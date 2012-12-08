// bksafesvc.cpp : Implementation of WinMain


#include "stdafx.h"
#include "resource.h"
#include "_idl_gen/bksafesvc.h"
#include <stdio.h>
#include "bksafesvcmodule.h"
#include "bkengex.h"
#include "reportfiledb.h"
#include "instmanager.h"
#include "common/runtimelog.h"
#include "common/AccessHelper.h"
#include "avploader.h"
#include "kpfw/fwproxy_public.h"
#include "kpfw/ktdifilt_public.h"
#include "scom/scom/ksdll.h"
#include "kpfw/antiarp/AntiArpInterface.h"
//#ifndef _DEBUG
//#include <miniutil/dumpreporter.h>
//#include <miniutil/dumpreporterclient.h>
//#endif

LONG CbksafesvcModule::ms_lObjectCount = 0;
CbksafesvcModule* _AtlModule = NULL;

#define CMDLINE_PARAM_NAME_INSTKENG         L"instkeng"
#define CMDLINE_PARAM_NAME_UNINSTKENG       L"uninstkeng"
#define CMDLINE_PARAM_NAME_RELOAD_AVSIGN    L"reloadksign"
#define CMDLINE_PARAM_NAME_UPDATE_WHITELIST L"updatewhitelist"
#define CMDLINE_PARAM_NAME_INST_KPFW_DRIVER    L"inst_kpfw_driver"
#define CMDLINE_PARAM_NAME_UNINST_KPFW_DRIVER    L"uninst_kpfw_driver"
#define CMDLINE_PARAM_NAME_UNINST_KARPFW_DRIVER  L"uninst_karpfw_drive"

void BKRegSave(void);
static void FixService(BOOL bInstall);
BOOL LoadInterface(const GUID& iid,  void** pInterface );
BOOL InstallDriverSvr(DWORD dwEnable, DWORD dwRequest);
BOOL UnInstallDriverSvr();
BOOL UnInstallArpDriverSvr();
//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
                                LPTSTR /*lpCmdLine*/, int nShowCmd)
{
    CBkCmdLine  _CmdLine;
    HRESULT     hr = S_OK;
    int         nRet = 0;
    BOOL        bRet = _CmdLine.Analyze( ::GetCommandLine() );
    if ( !bRet )
    {
        return nRet;
    }


    BKRegSave();
    //BKEngLoad();

    if ( _CmdLine.HasParam( CMDLINE_PARAM_NAME_SEVICE ) 
        || _CmdLine.HasParam( CMDLINE_PARAM_NAME_DEBUG ) )
    {
        CRunTimeLog::InitLogFile();
        CRunTimeLog::WriteLog(WINMOD_LLVL_CRITICAL, L"[main] SetLogLevel %lu", CRunTimeLog::GetLogFile().GetMaxLogLevel());

//         hr = BkScanInitialize();
//         if ( FAILED( hr ) )
//         {
//             goto Exit0;
//         }
// 
//         if ( SUCCEEDED( CInsRecycle::Instance().Initialize() ) )
//         {
//             CInsRecycle::Instance().StartRunning();
//         }
//     }
//     else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_INSTKENG))
//     {
//         CInstManager::LoadAvEngine();
//         goto Exit1;
//     }
//     else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_UNINSTKENG))
//     {
//         CInstManager::UnloadAvEngine();
//         goto Exit1;
//     }
//     else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_RELOAD_AVSIGN))
//     {
//         CInstManager::ReloadAvSign();
//         goto Exit1;
//     }
//     else if ( _CmdLine.HasParam( CMDLINE_PARAM_NAME_UPDATE_WHITELIST ) )
//     {
//         CInstManager::UpdateWhiteList();
//         goto Exit1;
//     }
    }
    else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_INSTKENG))
    {
        goto Exit1;
    }
    else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_UNINSTKENG))
    {
        goto Exit1;
    }
    else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_RELOAD_AVSIGN))
    {
        goto Exit1;
    }
    else if ( _CmdLine.HasParam( CMDLINE_PARAM_NAME_UPDATE_WHITELIST ) )
    {
        goto Exit1;
    }
	else if ( _CmdLine.HasParam( CMDLINE_PARAM_NAME_INST_KPFW_DRIVER ) )
	{
		goto Exit1;
	}
	else if ( _CmdLine.HasParam( CMDLINE_PARAM_NAME_UNINST_KPFW_DRIVER ) )
	{
		UnInstallDriverSvr();
		goto Exit1;
	}

    else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_UNINST_KARPFW_DRIVER))
    {
        UnInstallArpDriverSvr();
        goto Exit1;
    }
	else if ( _CmdLine.HasParam(L"install") )
	{
		FixService(TRUE);
	}
	else if ( _CmdLine.HasParam(L"uninstall") )
	{
		FixService(FALSE);
	}

    _AtlModule = new CbksafesvcModule;
    if ( _AtlModule )
    {
        nRet = _AtlModule->WinMain(nShowCmd);
    }
Exit1:
//      if ( _CmdLine.HasParam( CMDLINE_PARAM_NAME_SEVICE ) 
//          || _CmdLine.HasParam( CMDLINE_PARAM_NAME_DEBUG ) )
//      {
// 
//          CInsRecycle::Instance().NotifyStop();
//          CInsRecycle::Instance().WaitExit(INFINITE);
//          CInsRecycle::Instance().Uninitialize();
// 
//          CReportFileDB::Instance().Uninitialize();
//          BkScanUninitialize();
//      }

    return nRet;
}

void BKRegSave(void)
{
    CRegKey reg;
    LONG lRetCode   = ERROR_SUCCESS;
    // 共存0，注册表路径需要替换
    LPCTSTR lpszKeyName = _T("SOFTWARE\\KSafe\\KXEngine\\KxEScanSystem\\ksecore.config.top\\appconfig\\kse.kspfeng.kspolfile");

    lRetCode = reg.Open(HKEY_LOCAL_MACHINE, KOpKsafeReg::ReplaceRegStr(lpszKeyName).c_str());
    if(ERROR_SUCCESS != lRetCode)
    {
        lRetCode = reg.Create(HKEY_LOCAL_MACHINE, lpszKeyName);
        if(ERROR_SUCCESS != lRetCode)
        {
            goto Exit0;
        }

        lRetCode = reg.Open(HKEY_LOCAL_MACHINE, lpszKeyName);
        if(ERROR_SUCCESS != lRetCode)
        {
            goto Exit0;
        }
    }

    reg.SetStringValue(_T("cfgval"), _T("..\\KEng\\signs.ini"));
Exit0:
    reg.Close();
}


static void FixService(BOOL bInstall)
{
	{
		//@1
		CAccessHepler accesser;

		accesser.AddRegAccess(HKEY_CLASSES_ROOT, _T("CLSID\\{C313E554-97AB-49F9-988F-04DF64CD0451}"), KEY_ALL_ACCESS, TRUE);
		accesser.AddRegAccess(HKEY_CLASSES_ROOT, _T("TypeLib\\{C09A3ECC-2185-4F08-A17D-3EE687E05774}"), KEY_ALL_ACCESS, TRUE);
		accesser.AddRegAccess(HKEY_CLASSES_ROOT, _T("AppID\\{E44A3E87-876D-46BB-8831-836A4C74918B}"), KEY_ALL_ACCESS, TRUE);
		accesser.AddRegAccess(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\services\\KSafeSvc"), KEY_ALL_ACCESS, TRUE);		
	}

	{
		TCHAR szData[MAX_PATH];
		DWORD cbData = MAX_PATH * sizeof (TCHAR);
		DWORD dwDataType;
		LRESULT lResult;
		BOOL bDelete = FALSE;

		lResult = SHGetValue(HKEY_CLASSES_ROOT, _T("CLSID\\{C313E554-97AB-49F9-988F-04DF64CD0451}\\LocalServer32"), NULL, &dwDataType, szData, &cbData);
		if ( lResult == ERROR_SUCCESS && dwDataType == REG_SZ )
		{
			szData[MAX_PATH - 1] = 0;

			TCHAR szFilePath[MAX_PATH];

			GetModuleFileName(NULL, szFilePath, MAX_PATH);
			if ( StrStrI(szData, szFilePath) == NULL )
			{
				bDelete = TRUE;
			}
		}
		else
		{
			bDelete = TRUE;
		}

		if ( bDelete )
		{
			SHDeleteKey(HKEY_CLASSES_ROOT, _T("CLSID\\{C313E554-97AB-49F9-988F-04DF64CD0451}"));
		}
	}
}

BOOL UnInstallDriverSvr()
{
	ULONG status = E_FAIL;
	IFwInstaller *FwInstaller = NULL;
	if( LoadInterface(__uuidof(IFwInstaller), (VOID**)&FwInstaller) )
	{
		status = FwInstaller->UnInstall();
		if (status != S_OK)
		{
			wprintf(TEXT("卸载驱动失败：%08x"), status);
		}
	}
	return (status == S_OK);
}

BOOL UnInstallArpDriverSvr()
{
    ULONG ulStat = E_FAIL;
    BOOL bRunning = FALSE;
    KComInterfacePtr<IArpInstaller>  pArpInstaller;
    pArpInstaller.LoadInterface(_T("arpproxy.dll"));
    if (NULL != pArpInstaller)
    {
        pArpInstaller->IsRuning(&bRunning);
        if (bRunning)
        {
            ulStat = pArpInstaller->UnInstall();
            if (S_OK != ulStat)
            {
                wprintf(TEXT("卸载ARP驱动失败：%08x"), ulStat);
            }
        }
    }
 
    return (ulStat == S_OK);

}

BOOL InstallDriverSvr(DWORD dwEnable, DWORD dwRequest)
{
	ULONG status = E_FAIL;
	IFwInstaller *FwInstaller = NULL;
	if( LoadInterface(__uuidof(IFwInstaller), (VOID**)&FwInstaller) )
	{
		status = FwInstaller->Install();
		if( status == S_OK )
		{
			FwInstaller->SetConfig( dwEnable, dwRequest );
		}
		else
		{	
			wprintf(TEXT("KTdiDriverProxy::InstallDriverSvr install failed:%08x"), status);
		}
	}
	else
	{
		wprintf(TEXT("KTdiDriverProxy::InstallDriverSvr 无法获取安装接口"));
	}

	wprintf(TEXT("%s:%08X"), status == S_OK ? "安装成功" : "安装失败", status );
	if( FwInstaller != NULL )
		FwInstaller->Release();

	return status == S_OK;
}

BOOL LoadInterface(const GUID& iid,  void** pInterface )
{
	static KSDll dll;
	static BOOL b = FALSE;

	if (!b)
	{
		WCHAR wszProxy[MAX_PATH + 1] = { 0 };

		GetModuleFileName(NULL, wszProxy, MAX_PATH);
		*(wcsrchr(wszProxy, L'\\')) = 0L;
		wcscat_s(wszProxy, MAX_PATH, L"\\fwproxy.dll");

		if (S_OK == dll.Open(wszProxy))
			b = TRUE;
	}

	if (!b)
		return FALSE;

	return dll.GetClassObject(iid, iid, pInterface) == S_OK;
}


