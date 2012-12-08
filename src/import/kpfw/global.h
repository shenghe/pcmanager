////////////////////////////////////////////////////////////////////////////////
//      
//      File for kpfwtray
//      
//      File      : global.h
//      Version   : 1.0
//      Comment   : 全局的一些工具
//      
//      Create at : 2008-7-10
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 

#include "framework/kis_pathstr.h"
#include "framework/kis_os2.h"
//#include "kis/kpfw/kpfwtrayservice.h"
#include "regkeystring.h"

#define KPFWTRAY_APP				TEXT("kpfwtray.exe")

#define KPFWFRM_NAME				TEXT("kpfwfrm.exe")

inline ATL::CString GetRrmAppPath()
{
	WCHAR buf[MAX_PATH+1];
	if (GetModuleFileName(NULL, buf, MAX_PATH) == 0)
		return ATL::CString(TEXT(""));

	kis::KPath path(buf);
	path = path.GetPath();
	path.Append(TEXT("\\"));
	path.Append(KPFWFRM_NAME);
	return path.GetStr();
}

inline ATL::CString GetTrayAppPath()
{
	WCHAR buf[MAX_PATH+1];
	if (GetModuleFileName(NULL, buf, MAX_PATH) == 0)
		return ATL::CString(TEXT(""));

	kis::KPath path(buf);
	path = path.GetPath();
	path.Append(TEXT("\\"));
	path.Append(KPFWTRAY_APP);
	return path.GetStr();
}

inline ATL::CString GetSecurityPath()
{
	BOOL bRetCode = FALSE;
	HKEY hKey = NULL;
	DWORD dwType = REG_SZ;
	DWORD dwData = MAX_PATH * 2 * sizeof(WCHAR);
	LONG lRetCode = 0;
	WCHAR szBuf[MAX_PATH * 2] = { 0 };

	do 
	{
		lRetCode = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
			KIS_10_KIS_KEY, 
			0, 
			KEY_READ, 
			&hKey);
		if ( lRetCode != ERROR_SUCCESS )
		{
			::GetModuleFileName(NULL, szBuf, MAX_PATH * 2);
			*(wcsrchr(szBuf, _T('\\')) + 1) = '\0';
			break;
		}

		lRetCode = ::RegQueryValueEx(hKey, KIS_ANTIVIRUS_ITEM, NULL, &dwType, (BYTE*)szBuf, &dwData);
		if ( lRetCode != ERROR_SUCCESS )
			break;

		bRetCode = TRUE;
	} while(0);

	if ( hKey )
	{
		::RegCloseKey(hKey);
		hKey = NULL;
	}

	return ATL::CString(szBuf);
}

inline ATL::CString GetCommonSvrPath()
{
	BOOL bRetCode = FALSE;
	HKEY hKey = NULL;
	DWORD dwType = REG_SZ;
	DWORD dwData = MAX_PATH * 2 * sizeof(WCHAR);
	LONG lRetCode = 0;
	WCHAR szBuf[MAX_PATH * 2] = { 0 };

	do 
	{
		lRetCode = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
			KIS_10_KISCOMMON_KEY, 
			0, 
			KEY_READ, 
			&hKey);
		if ( lRetCode != ERROR_SUCCESS )
		{
			StringCchCopyW(szBuf, MAX_PATH * 2, _T("C:\\Program Files\\Common Files\\Kingsoft\\CommonService\\"));
			break;
		}

		lRetCode = ::RegQueryValueEx(hKey, KIS_ANTIVIRUS_ITEM, 0, &dwType, (BYTE*)szBuf, &dwData);
		if ( lRetCode != ERROR_SUCCESS )
			break;

		bRetCode = TRUE;

	} while(0);

	if ( hKey )
	{
		::RegCloseKey(hKey);
		hKey = NULL;
	}

	return ATL::CString(szBuf);
}