#include "StdAfx.h"
#include "HotfixUtils.h"
#include "Utils.h"
#include "EnvUtils.h"
#include "ItemFilter.h"
#include <bksafe/bksafeconfig.h>
#include <vulfix/BeikeUtils.h>
#include <common/registrywow.h>

BOOL RequireUsingInterface()
{
	// X64用接口，Vista RTM 用接口
#ifndef _DEBUG
	INT nWinVer = 0, nSP = 0;
    GetWinVerInfo(nWinVer, nSP);
	//return (nWinVer==WINVISTA && IsWin64()) || (nWinVer==WINVISTA && 0 == nSP);
	return IsWin64() || (nWinVer==WINVISTA && 0 == nSP);
#else
	return IsWin64();
#endif
}

DBFilterBase * CreateOSFilter( INT nWinVer, DWORD dwFlags )
{
	DBFilterBase * filterOS=NULL;
	if( nWinVer>=WINVISTA )
		filterOS = new DBFilterVista();
	else
		filterOS = new DBFilterXp();
	
	return filterOS;
}

BOOL InitOSFilter( DBFilterBase *pFilter, INT nWinVer, DWORD dwFlags )
{
	if(!pFilter)
		return FALSE;
	
	if( 0==(dwFlags&VULSCAN_EXPRESS_SCAN) )
	{
		dwFlags = 0;
		if( RequireUsingInterface() )
			dwFlags = VULSCAN_USE_IUPDATE;
	}
	pFilter->Init( dwFlags );
	return TRUE;
}

#define STR_IMMUN_SUBKEY_FMT _T("SOFTWARE\\Microsoft\\Internet Explorer\\ActiveX Compatibility\\%s")
#define STR_IMMUN_KEY_FLAG	_T("Compatibility Flags")
#define STR_IMMUN_KEY_BAKUP	_T("ImmueFlagBakup")
static const DWORD dwImmunYes = 0x400;
static const DWORD dwImmunNo = 0;
BOOL EnableCom(LPCTSTR lpClsid, BOOL bEnable)
{
	if( NULL == lpClsid )
		return FALSE;
	
	CRegistryWow<REGISTRY_WOW> reg;
	CString strSubkey; 
	strSubkey.Format(STR_IMMUN_SUBKEY_FMT, lpClsid);

	DWORD dwType = REG_DWORD;
	DWORD dwValue = dwImmunNo;
	DWORD dwValLen = sizeof(dwValue);
	if( ERROR_SUCCESS!=reg.GetValue(HKEY_LOCAL_MACHINE, strSubkey, STR_IMMUN_KEY_FLAG, &dwType, &dwValue, &dwValLen) )
		return FALSE;

	if(bEnable) // 启用COM, 取消免疫
	{
		if( (dwValue&dwImmunYes)==dwImmunYes ) // 被免疫了 
		{
			// 获取备份的Flag 
			reg.GetValue(HKEY_LOCAL_MACHINE, strSubkey, STR_IMMUN_KEY_BAKUP, &dwType, &dwValue, &dwValLen);
			dwValue &= ~dwImmunYes;

			reg.SetValue(HKEY_LOCAL_MACHINE, strSubkey, STR_IMMUN_KEY_FLAG, REG_DWORD, &dwValue, sizeof(dwValue));
			reg.DeleteValue(HKEY_LOCAL_MACHINE, strSubkey, STR_IMMUN_KEY_BAKUP);
		}
	}
	else // 禁用 COM, 免疫 
	{
		if( (dwValue&dwImmunYes)!=dwImmunYes )
		{
			reg.SetValue(HKEY_LOCAL_MACHINE, strSubkey, STR_IMMUN_KEY_BAKUP, REG_DWORD, &dwValue, sizeof(dwValue));
			dwValue = dwImmunYes;
			reg.SetValue(HKEY_LOCAL_MACHINE, strSubkey, STR_IMMUN_KEY_FLAG, REG_DWORD, &dwValue, sizeof(dwValue));
		}
	}
	return TRUE;
}

BOOL GetComState( LPCTSTR lpClsid, BOOL &bEnabled )
{
	ATLASSERT(lpClsid);
	CRegistryWow<REGISTRY_WOW> reg;

	CString strSubkey; 
	strSubkey.Format(STR_IMMUN_SUBKEY_FMT, lpClsid);

	DWORD dwType = REG_DWORD;
	DWORD dwValue = dwImmunNo;
	DWORD dwValLen = sizeof(dwValue);
	if( ERROR_SUCCESS!=reg.GetValue(HKEY_LOCAL_MACHINE, strSubkey, STR_IMMUN_KEY_FLAG, &dwType, &dwValue, &dwValLen) )
		return FALSE;

	bEnabled = (dwValue&dwImmunYes)!=dwImmunYes;	
	return TRUE;
}

void SafeTerminateThread( HANDLE &hThread )
{
	DWORD dwExitCode;
	if( GetExitCodeThread( hThread , &dwExitCode ) )
	{
		if( dwExitCode == STILL_ACTIVE )
		{
			SuspendThread( hThread ) ;
			TerminateThread( hThread , 0 ) ;
		}
	}
	CloseHandle(hThread);
	hThread = NULL;
}

BOOL WriteVulConfig( LPCTSTR lpSectionName, LPCTSTR lpKeyName, LPCTSTR lpString )
{
	CString ignoredfilename;
	BKSafeConfig::Get_Vulfix_IniFile( ignoredfilename );
	return WritePrivateProfileString(lpSectionName, lpKeyName, lpString, ignoredfilename);
}

BOOL ReadVulConfig( LPCTSTR lpSectionName, LPCTSTR lpKeyName, CString& strValue, LPCTSTR szDefault )
{
	CString ignoredfilename;
	BKSafeConfig::Get_Vulfix_IniFile( ignoredfilename );
	TCHAR buf[MAX_PATH] = {0};
	GetPrivateProfileString(lpSectionName, lpKeyName, szDefault, buf, MAX_PATH-1, ignoredfilename);
	strValue = buf;
	return TRUE;
}

LCID FixLocale(LCID id)
{
	LCID old = ::GetThreadLocale();
	if(id==0)
		::SetThreadLocale( 0x0804 ); 
	return old;
}
