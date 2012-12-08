// beikesafe.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "beikesafe.h"
#include <miniutil/fileversion.h>
#include "beikesafemaindlg.h"
#include <bksafe/bksafeconfig.h>
#include "bksafeddewnd.h"
#include "SetupDlg.h"
#include "beikesafeinstallkavenginedlg.h"
#include <safemon/safetrayshell.h>
#include <comproxy/bkservicecall.h>
#include <runoptimize/interface.h>
#include <runoptimize\restorerunitem.h>
#include "kws\kactivate.h"
#include "KDump/KDump.h"
#include "bksafetabctrl.h" 
#include "kws/util.h"

CBeikeSafeModule _Module;

DWORD g_dwMainThreadId = 0;

typedef struct _MY_TOKEN_MANDATORY_LABEL {
	SID_AND_ATTRIBUTES Label;
} MY_TOKEN_MANDATORY_LABEL, *PMY_TOKEN_MANDATORY_LABEL;


typedef struct _MY_TOKEN_LINKED_TOKEN {
	HANDLE LinkedToken;
} MY_TOKEN_LINKED_TOKEN, *PMY_TOKEN_LINKED_TOKEN;


typedef enum _MY_TOKEN_INFORMATION_CLASS {
	/*
	TokenUser = 1,
	TokenGroups,
	TokenPrivileges,
	TokenOwner,
	TokenPrimaryGroup,
	TokenDefaultDacl,
	TokenSource,
	TokenType,
	TokenImpersonationLevel,
	TokenStatistics,

	TokenRestrictedSids,
	TokenSessionId,
	TokenGroupsAndPrivileges,
	TokenSessionReference,
	TokenSandBoxInert,
	TokenAuditPolicy,
	TokenOrigin,*/
	MYTokenElevationType  = 18, // MaxTokenInfoClass
	MYTokenLinkedToken,
	MYTokenElevation,

	MYTokenHasRestrictions,
	MYTokenAccessInformation,
	MYTokenVirtualizationAllowed,
	MYTokenVirtualizationEnabled,
	MYTokenIntegrityLevel,
	MYTokenUIAccess,
	MYTokenMandatoryPolicy,
	MYTokenLogonSid,
	//MaxTokenInfoClass  // MaxTokenInfoClass should always be the last enum
} MY_TOKEN_INFORMATION_CLASS, *PMY_TOKEN_INFORMATION_CLASS;

#ifndef SECURITY_MANDATORY_HIGH_RID
#define SECURITY_MANDATORY_HIGH_RID                 (0x00003000L)
#endif 

#ifndef SE_GROUP_INTEGRITY
#define SE_GROUP_INTEGRITY                 (0x00000020L)
#endif

BOOL NeedServiceCall()
{
    BOOL bRet = FALSE, bNeedSvcCall = FALSE;
    HANDLE hToken = NULL;
    DWORD dwBytesNeeded = 0;
    MY_TOKEN_MANDATORY_LABEL *pTIL = NULL;

    if (!RunTimeHelper::IsVista())
        goto Exit0;

    bRet = ::OpenProcessToken(::GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
    if (NULL == hToken)
        return FALSE;

    bRet = ::GetTokenInformation(
        hToken,
        (TOKEN_INFORMATION_CLASS)MYTokenIntegrityLevel,
        NULL, 0, &dwBytesNeeded);

    pTIL = (MY_TOKEN_MANDATORY_LABEL *)new BYTE[dwBytesNeeded];
    if (!pTIL)
        goto Exit0;

    bRet = ::GetTokenInformation(
        hToken,
        (TOKEN_INFORMATION_CLASS)MYTokenIntegrityLevel,
        pTIL, dwBytesNeeded, &dwBytesNeeded);
    if (!bRet || !pTIL)
        goto Exit0;

    SID* pSid = static_cast<SID*>(pTIL->Label.Sid);
    if (!pSid)
        goto Exit0;

    if (SECURITY_MANDATORY_HIGH_RID > pSid->SubAuthority[0])
        bNeedSvcCall = TRUE;

Exit0:

    if (NULL != pTIL)
    {
        delete[] (LPBYTE)pTIL;
        pTIL = NULL;
    }

    if (hToken)
    {
        ::CloseHandle(hToken);
        hToken = NULL;
    }

    return bNeedSvcCall;
}

void AllowDropFile()
{
    typedef BOOL (WINAPI *FN_ChangeWindowMessageFilter)(UINT, DWORD);

    HMODULE hModUser = ::GetModuleHandle(L"user32.dll");

    FN_ChangeWindowMessageFilter pfnChangeWindowMessageFilter = (FN_ChangeWindowMessageFilter)::GetProcAddress(hModUser, "ChangeWindowMessageFilter");

    if (pfnChangeWindowMessageFilter)
    {
        pfnChangeWindowMessageFilter(0x0049/*WM_COPYGLOBALDATA*/, 1/*MSGFLT_ADD*/);
        pfnChangeWindowMessageFilter(WM_DROPFILES, 1);
    }
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
#   ifdef _DEBUG

    //bkconsole theConsole;
 	CString str;
 	str.Format(L"%d", nCmdShow);
 	//::MessageBox(NULL, str, NULL, 1);

#   endif


	_Module.m_nCmdShow = nCmdShow;

	HRESULT hRet = E_FAIL;

	g_dwMainThreadId = ::GetCurrentThreadId();

    
    hRet = _Module.Init(hInstance);
    if (FAILED(hRet))
        return -1;

    hRet = _Module.ParseCommandLine(lpstrCmdLine);
    if (FAILED(hRet))
        return -2;

    if (!_Module.CheckIntegrityLevel())
        return -3;

    if (hRet == S_FALSE)
        return 0;

	AtlInitCommonControls( ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_WIN95_CLASSES );
    ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	_Module.InitRichEdit2();
    _Module.Main();

    ::CoUninitialize();

// #   ifdef _DEBUG
//     bkconsole::getch();
// #   endif

    return 0;
}

HRESULT CBeikeSafeModule::Init(HINSTANCE hInstance)
{
    HRESULT hRet = __super::Init(NULL, hInstance);
    if (FAILED(hRet))
        return hRet;

    DWORD dwRet = ::GetModuleFileName(NULL, m_strAppFileName.GetBuffer(MAX_PATH + 1), MAX_PATH);
    if (0 == dwRet)
    {
        m_strAppFileName.ReleaseBuffer(0);

        hRet = E_FAIL;
    }
    else
    {
        m_strAppFileName.ReleaseBuffer();

        m_strAppPath = m_strAppFileName;
        m_strAppPath.Truncate(m_strAppPath.ReverseFind(L'\\') + 1);
    }

    CString strVerDllFileName = m_strAppPath + L"ksafever.dll";

    _GetPEProductVersion(strVerDllFileName, m_strAppVersion);

	KActivate::SetAppVersion(m_strAppVersion);
    m_lAppVersionNumber = ::StrToLong(m_strAppVersion.Mid(m_strAppVersion.ReverseFind(L'.') + 1));

    CRegKey reg;

    // 共存1, 此处要修改
    reg.Create(HKEY_LOCAL_MACHINE, KOpKsafeReg::ReplaceRegStr(L"SOFTWARE\\KSafe").c_str());
    WCHAR szTemp[2];

    dwRet = 1;
    
    LONG lRet = reg.QueryStringValue(L"Install Path", szTemp, &dwRet);
    if (ERROR_FILE_NOT_FOUND == lRet)
    {
        reg.SetStringValue(L"Install Path", m_strAppPath);
    }

	GetInstallPID(m_strInstallPID);
	GetOrgPID( m_strOrgOem );

    return hRet;
}

LPCTSTR CBeikeSafeModule::GetPID()
{
	return m_strInstallPID;
}

LPCTSTR CBeikeSafeModule::GetOrgOem()
{
	return m_strOrgOem;
}

BOOL CBeikeSafeModule::GetInstallPID(CString & strPid)
{
	TCHAR	szPrePartner[MAX_PATH] = _T("");
	TCHAR	szPartner[MAX_PATH] = _T("");
	DWORD	dwSize = sizeof(szPrePartner);
	DWORD	dwType = REG_SZ;

	// 共存1 此处要修改RsEGPATH_COOP宏
	SHGetValue( HKEY_LOCAL_MACHINE, (KOpKsafeReg::ReplaceRegStr(REGPATH_COOP)).c_str(), REGKEY_PREOEM, &dwType, &szPrePartner, &dwSize );
	dwSize = sizeof(szPartner);dwType = REG_SZ;
	SHGetValue( HKEY_LOCAL_MACHINE, KOpKsafeReg::ReplaceRegStr(REGPATH_COOP).c_str(), REGKEY_OEM, &dwType, &szPartner, &dwSize );

	if( szPrePartner[0] == _T('\0') )
	{
		StringCbPrintf( szPrePartner, sizeof(szPrePartner)-1, _T("%s"), _T("h_lvse") );
		SHSetValue( HKEY_LOCAL_MACHINE, KOpKsafeReg::ReplaceRegStr(REGPATH_COOP).c_str(), REGKEY_PREOEM, REG_SZ, szPrePartner, (1+(DWORD)_tcslen(szPrePartner))*sizeof(TCHAR) );
		SHSetValue( HKEY_LOCAL_MACHINE, KOpKsafeReg::ReplaceRegStr(REGPATH_COOP).c_str(), REGKEY_OEMNAME, REG_SZ, szPrePartner, (1+(DWORD)_tcslen(szPrePartner))*sizeof(TCHAR) );
	}

	strPid = szPrePartner;

	if( _tcslen(szPartner) == 0 || _tcsicmp( szPartner, szPrePartner )!=0 )
	{
		return TRUE; //Need RegUser;
	}

	return FALSE;
}

BOOL CBeikeSafeModule::GetOrgPID(CString & strPid)
{
	TCHAR	szPrePartner[MAX_PATH] = _T("");
	TCHAR	szPartner[MAX_PATH] = _T("");
	DWORD	dwSize = sizeof(szPrePartner);
	DWORD	dwType = REG_SZ;

	SHGetValue( HKEY_LOCAL_MACHINE, KOpKsafeReg::ReplaceRegStr(REGPATH_COOP).c_str(), REGKEY_ORGOEM, &dwType, &szPrePartner, &dwSize );
	dwSize = sizeof(szPartner);dwType = REG_SZ;

	if( szPrePartner[0] == _T('\0') )
	{
		StringCbPrintf( szPrePartner, sizeof(szPrePartner)-1, _T("%s"), _T("h_lvse") );
	}

	strPid = szPrePartner;

	return FALSE;
}

HRESULT CBeikeSafeModule::ParseCommandLine(LPCWSTR lpszCmdLine)
{
    HRESULT hRet = S_OK;

    BOOL bRet = _CmdLine.Analyze(lpszCmdLine);
    if (!bRet)
        return E_FAIL;

    return hRet;
}

void CBeikeSafeModule::CheckUnknowFile()
{
	if( BKSafeConfig::GetUnKnownFileOpen() == 1 )
	{
		CRegKey reg;
		TCHAR tszFilePath[MAX_PATH] = {0};
		::GetModuleFileName( NULL, tszFilePath, MAX_PATH );
		CPath pathFile( tszFilePath );
		pathFile.RemoveFileSpec();
		pathFile.Append( STMGR_UNKNOWN_FILENAME );
		CString strParam;
		strParam.Format( TEXT("\"%s\" \"%%1\""), pathFile.m_strPath );
		LONG lRes = reg.Create( HKEY_CLASSES_ROOT, TEXT("Unknown\\shell\\openas\\command") );
		if (lRes == ERROR_SUCCESS)
			lRes = reg.SetStringValue( NULL, strParam, REG_EXPAND_SZ );
		reg.Close();

		if( RunTimeHelper::IsVista() )
		{
			lRes = reg.Create( HKEY_CLASSES_ROOT, TEXT("Unknown\\shell\\opendlg\\command") );
			if (lRes == ERROR_SUCCESS)
				lRes = reg.SetStringValue( NULL, strParam, REG_EXPAND_SZ );
		}
	}
}

UINT_PTR CBeikeSafeModule::Main()
{
    BOOL bNavigate = FALSE;
    CString strNavigate;

	/*主程序初始化最开始注册Dump*/
	KDump::Instance().Install();

    if ( TRUE /*_CmdLine.HasParam(L"{69DD4969-E6C4-42d9-A508-105DDA13CE40}")*/)
    {
        CString strPath;

        GetModuleFileName((HMODULE)&__ImageBase, strPath.GetBuffer(MAX_PATH + 10), MAX_PATH);
        strPath.ReleaseBuffer();
        strPath.Truncate(strPath.ReverseFind(L'\\') + 1);
        strPath += L"res\\ksafe";

		if ( PathFileExists(strPath) )
			BkResManager::SetResourcePath(strPath);
    }

	if (_CmdLine.HasParam(CMDPARAM_NAME_SETUCACHE))
	{
        _InitUIResource();

		CString strHwnd = _CmdLine[CMDPARAM_NAME_SETUCACHE].String();
		HWND	hWnd = (HWND)_ttoi(strHwnd);

		CSetupDlg dlgSetup;

		if (IsWindow(hWnd))
		{
			dlgSetup.DoModal(hWnd);
		}
		else
		{
			dlgSetup.DoModal(NULL);
		}

		CheckUnknowFile();

		return 0;
	}
	if (_CmdLine.HasParam(CMDPARAM_NAME_SAFEMONITOR))
	{
		//if (NULL != ::FindWindow(NULL, L"金山卫士实时保护设置"))
		//{
		//	return 0;
		//}

		//_InitUIResource();

		//CString strHwnd = _CmdLine[CMDPARAM_NAME_SAFEMONITOR].String();
		//HWND	hWnd = (HWND)_ttoi(strHwnd);

		//CBKSafeMonitorDlg SM(NULL);

		//if (IsWindow(hWnd))
		//{
		//	SM.DoModal(hWnd);
		//}
		//else
		//{
		//	SM.DoModal(NULL);
		//}

		return 0;
	}
    else if (_CmdLine.HasParam(CMDPARAM_NAME_INSTALL_LOCAL_ENGINE))
    {
        CAtlFileMapping<BkSafe_Instance_Data> mapping;
        BOOL bAlreadyExisted = FALSE;

        HRESULT hRet = mapping.MapSharedMem(sizeof(BkSafe_Instance_Data), L"{FF08A752-3C75-426f-84F0-379B74666273}", &bAlreadyExisted);
        if (FAILED(hRet))
            return 0;

        if (bAlreadyExisted)
        {
            HWND hWndActive = ((BkSafe_Instance_Data *)mapping)->hActiveWnd;

            if (::IsIconic(hWndActive))
                ::PostMessage(hWndActive, WM_SYSCOMMAND, SC_RESTORE | HTCAPTION, 0);

            ::SetForegroundWindow(hWndActive);
            ::SetFocus(hWndActive);

            return 0;
        }
        else
        {
            _InitUIResource();

            CBeikeSafeInstallKavEngineDlg dlg(((BkSafe_Instance_Data *)mapping)->hActiveWnd);

            dlg.DoModal();
        }

        return 0;
	}
/*
	else if (_CmdLine.HasParam(CMDPARAM_NAME_SETTING))
	{//调用设置窗口
		if (NULL != ::FindWindow(NULL, L"设置"))
		{
			return 0;
		}

		_InitUIResource();

		CString strHwnd = _CmdLine[CMDPARAM_NAME_SETTING].String();
		HWND	hWnd = (HWND)_ttoi(strHwnd);

		CBeikeSafeSettingDlg Setting;

		if (IsWindow(hWnd))
		{
			Setting.DoModal(0, hWnd);
		}
		else
		{
			Setting.DoModal(0, NULL);
		}

		return 0;
	}*/


	if (_CmdLine.HasParam(CMDPARAM_NAME_UNINSTALL))
	{
		if (TRUE)
		{
			ReSetUnKnownFileLink();

			//::SHSetValue(HKEY_CLASSES_ROOT, L"Unknown\\shell\\openas\\command",NULL,REG_EXPAND_SZ,lpDefValue,_tcslen(lpDefValue)*sizeof(TCHAR));
		}

		if (TRUE)
		{
			// 恢复启动项
			RunLogItem* pSetting = NULL;
			CRestoreRunner customSetting;
			IRunOptimizeEng2*	pEng = CRunOptLoader::GetGlobalPtr()->GetOptEng2();
			int nDisableCount = customSetting.GetCount();
			CKsafeRunInfo RunInfo;
			
			if (pEng)
			{
				for (int nLoop = 0; nLoop < nDisableCount; nLoop ++)
				{
					pSetting = customSetting.GetItem(nLoop);

					if (pSetting->dwNewState == KSRUN_START_DELAY)
					{
						RunInfo.nType = pSetting->nType;
						RunInfo.strName = pSetting->strName;
						RunInfo.bSystem = pSetting->bSystem;

						if (RunInfo.nType == KSRUN_TYPE_TASK)
						{
							RunInfo.nJobType = pSetting->nExtType;
						}

						BOOL bRet = pEng->ControlRun(TRUE, &RunInfo);
					}
				}
				//恢复已延迟的启动项
				pEng->RestoreRun(KSRUN_TYPE_RESTORE_DELAY_RUN, NULL);
				//恢复系统设置项到初始状态
				pEng->RestoreSysCfg(KSAFE_SYSCONFIG_RESTORE_INIT, NULL);

				if (IDYES == ::MessageBox(NULL, _T("是否将使用金山卫士禁止的启动项重新设为开机自启动"), _T("卸载"), MB_YESNO))
					pEng->RestoreRun(KSRUN_TYPE_RESTORE_ALL,NULL);	//恢复所有启动项

				CRunOptLoader::GetGlobalPtr()->FreeOptEng(pEng);
			}
		}

		return 0;
	}
    if (_CmdLine.HasParam(CMDPARAM_NAME_NAVIGATE))
    {
        bNavigate = TRUE;
        strNavigate = _CmdLine[CMDPARAM_NAME_NAVIGATE].String();

        CString strTempNavigate = strNavigate;

        BOOL bRet = CBkSafeDDEWindow::PostDDEMessage(strNavigate, strNavigate.GetLength());
        if (!bRet)
		{
			if ( strNavigate.CompareNoCase( BKSFNS_UI L"_" BKSFNS_SOFTMGR) == 0 )
			{
				CBkSafeTabChangeCtrl::Instance().Push(TAB_SWITCH_SOFTMGR,TAB_SWITCH_SOFTMGR_FROM_DESKTOP);
			}
			Navigate(strTempNavigate);
		}
    }
	// restart [8/10/2010 zhangbaoliang]
	if (_CmdLine.HasParam(CMDPARAM_NAME_RESTART))
	{
		_RestartSelf();
		return 0;
	}

    // 判断前一实例
    if (!CheckInstance())
    {
//         // 如果需要调用功能
//         if (bNavigate)
//             CBkSafeDDEWindow::PostDDEMessage(strNavigate, strNavigate.GetLength());

        return 0;
    }

    UINT_PTR uRet = 0;

    if (m_bShowMainUI)
    {
        _InitUIResource();

        AllowDropFile();

		CBeikeSafeMainDlg dlgMain;
		CBkSafeDDEWindow wndDDE;

		wndDDE.Create();

		m_pNavigator = &dlgMain;

		if (bNavigate)
			dlgMain.SetDefaultNavigate(strNavigate);

		uRet = dlgMain.DoModal(NULL);

        m_pNavigator = NULL;
    }

    return uRet;
}

CBkNavigator* CBeikeSafeModule::OnNavigate(CString &strChildName)
{
    CBkNavigator *pChild = NULL;

    if (0 == strChildName.CompareNoCase(BKSFNS_UI))
    {
        pChild = m_pNavigator;
    }
    else if (0 == strChildName.CompareNoCase(BKSFNS_TRAYSTART))
    {
        m_bShowMainUI = FALSE;

        CBkSafeProtectionNotifyWindow::NotifyTrayRestart();
    }
    else if (0 == strChildName.CompareNoCase(BKSFNS_URL))
    {
        m_bShowMainUI = FALSE;

        pChild = &m_UrlNavigator;
    }
    else if (0 == strChildName.CompareNoCase(BKSFNS_JOIN_SKYLARK))
    {
        BKSafeConfig::SetAutoReportUnknown(TRUE);

        CBkUtility util;

        util.Initialize();
        util.SettingChange();
        util.Uninitialize();
		CString strMsg;
        strMsg.Format(BkString::Get(IDS_JOIN_SKYLARK_NOTICE));
		CBkSafeMsgBox msg;
		CRect rc(0, 0, 80, 60);
		msg.ShowPanelMsg(strMsg, &rc, NULL,  MB_OK | MB_ICONINFORMATION, NULL);

//        CBkSafeMsgBox::Show(L"已经加入云安全计划\n\n您可以在“设置->查杀木马”中关闭此项", NULL, MB_OK | MB_ICONINFORMATION);

        if (m_pNavigator)
            m_pNavigator->Navigate(BKSFNS_SETTING_CHANGE);
    }
    else if (0 == strChildName.CompareNoCase(BKSFNS_QUIT))
    {
        ::ExitProcess(0);
    }

    return pChild;
}

BOOL CBeikeSafeModule::CheckInstance()
{
    BOOL bRet = FALSE;
    HRESULT hRet = E_FAIL;
    BOOL bAlreadyExisted = FALSE;

    if (NULL != m_mapping.GetHandle())
        return TRUE;

	CString strWndClassName = BKSAFE_DDE_WND_CLASS;

	std::wstring strValue;

	if (KisPublic::Instance()->Init())
	{
		int nRet = KisPublic::Instance()->KQueryOEMLPCWSTR(KIS::KOemKsfe::oemc_OtherCheckName, strValue);
		if (nRet)
			strWndClassName.Append(strValue.c_str());
	}
	

    hRet = m_mapping.MapSharedMem(sizeof(BkSafe_Instance_Data), strWndClassName, &bAlreadyExisted);
    if (FAILED(hRet))
        return bRet;

    if (bAlreadyExisted)
    {
        HWND hWndActive = ((BkSafe_Instance_Data *)m_mapping)->hActiveWnd;
		
		if( IsWindowRunBackGround(hWndActive) )
		{
			SetWindowRunBackGround(hWndActive, FALSE);

			CSafeMonitorTrayShell shell;
			shell.SetTrayTipInfo( NULL, 10, TRUE );
		}

        if (::IsIconic(hWndActive))
            ::SendMessage(hWndActive, WM_SYSCOMMAND, SC_RESTORE | HTCAPTION, 0);
				
        ::SetWindowPos(hWndActive, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
        ::SetForegroundWindow(hWndActive);
        ::SetFocus(hWndActive);

        goto Exit0;
    }

    bRet = TRUE;

Exit0:

    return bRet;
}

void CBeikeSafeModule::SetActiveWindow(HWND hWndActive)
{
    BkSafe_Instance_Data *pInstance = (BkSafe_Instance_Data *)m_mapping;

    if (pInstance)
        pInstance->hActiveWnd = hWndActive;
}

void CBeikeSafeModule::_GetFileInfo(BYTE *pbyInfo, LPCWSTR lpszFileInfoKey, LPWSTR lpszFileInfoRet)
{
    BOOL bRet = FALSE;
    UINT uCount = 0;
    UINT uRetSize = MAX_PATH - 1;
    LPWSTR lpszValue = NULL;

    struct LANGANDCODEPAGE
    {
        WORD wLanguage;
        WORD wCodePage;
    } *lpTranslate;

    bRet = ::VerQueryValue(pbyInfo, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &uCount);
    if (!bRet || uCount == 0)
        return;

    CString strSubBlock;

    strSubBlock.Format(
        L"\\StringFileInfo\\%04x%04x\\%s",
        lpTranslate[0].wLanguage,
        lpTranslate[0].wCodePage, 
        lpszFileInfoKey
        );
    bRet = ::VerQueryValue(
        pbyInfo, (LPWSTR)(LPCWSTR)strSubBlock, 
        (LPVOID *)&lpszValue, &uRetSize
        );
    if (!bRet)
        return;

    wcsncpy(lpszFileInfoRet, lpszValue, wcslen(lpszValue) + 1);
}

BOOL CBeikeSafeModule::_GetPEProductVersion(LPCWSTR lpszFileName, CString &strProductVersion)
{
    BOOL bResult = FALSE;
    BOOL bRet = FALSE;
    DWORD dwHandle          = 0;
    DWORD dwFileInfoSize    = 0;
    VS_FIXEDFILEINFO *pFixFileInfo = NULL;
    BYTE *pbyInfo           = NULL;

    dwFileInfoSize = ::GetFileVersionInfoSize(lpszFileName, &dwHandle);
    if (0 == dwFileInfoSize)
        goto Exit0;

    pbyInfo = new BYTE[dwFileInfoSize];
    if (!pbyInfo)
        goto Exit0;

    bRet = ::GetFileVersionInfo(lpszFileName, dwHandle, dwFileInfoSize, pbyInfo);
    if (!bRet)
        goto Exit0;

    _GetFileInfo(pbyInfo, L"ProductVersion", strProductVersion.GetBuffer(MAX_PATH + 1));
    strProductVersion.ReleaseBuffer();

    bResult = TRUE;

Exit0:

    if (pbyInfo)
    {
        delete[] pbyInfo;
        pbyInfo = NULL;
    }

    return bResult;
}

LPCTSTR CBeikeSafeModule::GetAppFilePath()
{
	return m_strAppFileName;
}

LPCTSTR CBeikeSafeModule::GetAppDirPath()
{
	return m_strAppPath;
}

LPCTSTR CBeikeSafeModule::GetProductVersion()
{
    return m_strAppVersion;
}

VOID CBeikeSafeModule::InitRichEdit2()
{
	if ( m_hModRichEdit2 == NULL )
		m_hModRichEdit2 = ::LoadLibrary(_T("RICHED20.DLL"));
}

BOOL CBeikeSafeModule::CheckIntegrityLevel()
{
    BOOL bNeedSvcCall = NeedServiceCall();
    CString strCmdLine;

    _CmdLine.GetCmdLine(strCmdLine);

    if (!bNeedSvcCall)
        return TRUE;

    CBkServiceCall svccall;

    ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

    HRESULT hRet = svccall.Initialize();
    if (FAILED(hRet))
    {
//         CString strText;
//         strText.Format(L"svccall.Initialize() Failed 0x%08X", hRet);
//         ::MessageBox(0, strText, 0, 0);
        ::CoUninitialize();
        goto _ShowUAC;
    }

    hRet = svccall.Execute(m_strAppFileName, strCmdLine, TRUE);

    svccall.Uninitialize();

    ::CoUninitialize();

    if (FAILED(hRet))
    {
//         CString strText;
//         strText.Format(L"svccall.Execute() Failed 0x%08X", hRet);
//         ::MessageBox(0, strText, 0, 0);
        goto _ShowUAC;
    }

    return FALSE;

_ShowUAC:

    BOOL bRet = _CmdLine.Execute(m_strAppFileName, TRUE, FALSE);

    if (!bRet)
        ::MessageBox(0, L"请使用管理员账户运行本软件", 0, 0);

    return FALSE;
}

void CBeikeSafeModule::_InitUIResource()
{
//         if (RunTimeHelper::IsVista())
//             BkFontPool::SetDefaultFont(_T("微软雅黑"), -12);
//         else

    BkString::Load(IDR_BK_STRING_DEF);

    BkFontPool::SetDefaultFont(BkString::Get(IDS_APP_FONT), -12);

    BkSkin::LoadSkins(IDR_BK_SKIN_DEF);
    BkStyle::LoadStyles(IDR_BK_STYLE_DEF);
}

void CBeikeSafeModule::_RestartSelf()
{
	// if( _IsCalledBySelf() )
	{
		CSafeMonitorTrayShell tray;
		HWND hWndTray = tray.GetWnd();
		HWND hWndUpload = tray.GetUploadTrayWnd();
		HWND hWndMain = ::FindWindow(BKSAFE_DDE_WND_CLASS, NULL);;//BeikeSafe::GetMainWindow();

		HANDLE hProcTray = _GetWindowProcess(hWndTray);
		HANDLE hProcMain = _GetWindowProcess(hWndMain);

		if(hWndTray)
		{
			
//			::MessageBox(NULL, L"Exit Tray", NULL, 1);
			tray.ExitTray(TRUE, TRUE);
			::PostMessage(hWndUpload, WM_CLOSE, 0, 0);
		}

		INT nHandles = 0;
		HANDLE szHandles[2]= {NULL, NULL};
		if(hProcMain)
			szHandles[nHandles++] = hProcMain;
		if(hProcTray)
			szHandles[nHandles++] = hProcTray;

		if(nHandles>0)
		{
			DWORD dwWait = WaitForMultipleObjects(nHandles, szHandles, TRUE, INFINITE);
			ATLASSERT(dwWait==WAIT_OBJECT_0);
		}

		if(hProcMain)
			CloseHandle(hProcMain);
		if(hProcTray)
			CloseHandle(hProcTray);

		// Start !! 
//		::MessageBox(NULL, L"Restart", NULL, 1);
		ShellExecute(NULL, _T("open"), GetAppFilePath(), NULL, NULL, SW_SHOW);
	}
}

HANDLE CBeikeSafeModule::_GetWindowProcess( HWND hWnd )
{
	if(hWnd==NULL)
		return NULL;
	DWORD dwPid = 0;
	GetWindowThreadProcessId(hWnd, &dwPid);
	if(dwPid)
	{
		return OpenProcess(SYNCHRONIZE, FALSE, dwPid);
	}
	return NULL;
}

int CBeikeSafeModule::ReSetUnKnownFileLink()
{
	int nRet = 0;

	LPCTSTR lpDefValue = L"%SystemRoot%\\system32\\rundll32.exe %SystemRoot%\\system32\\shell32.dll,OpenAs_RunDLL %1";
	TCHAR pszValue[1024] = {0};

	CString strKeyValue;
	strKeyValue.Format(L"%s", m_strAppPath);
	strKeyValue += L"kpcfileopen.exe";
	//获取未知文件关联，看看是不是我们修改的，如果是，就该回去
	DWORD dwType = REG_EXPAND_SZ, dwSize = sizeof(pszValue);
	if (ERROR_SUCCESS == ::SHGetValue(HKEY_CLASSES_ROOT, L"Unknown\\shell\\openas\\command", NULL, &dwType, (PBYTE)pszValue, &dwSize))
	{
		if (NULL != StrStrI(pszValue, strKeyValue))
		{
			//修改UNKONW的关联项
			::SHSetValue(HKEY_CLASSES_ROOT, L"Unknown\\shell\\openas\\command", NULL,REG_EXPAND_SZ,lpDefValue,_tcslen(lpDefValue)*sizeof(TCHAR));
		}
	}

	//win7下新增加的
	if (WINVER_WIN7 == GetWindowVersion())
	{
		ZeroMemory(pszValue, sizeof(pszValue));
		dwType = REG_EXPAND_SZ, dwSize = sizeof(pszValue);

		if (ERROR_SUCCESS == ::SHGetValue(HKEY_CLASSES_ROOT, L"Unknown\\shell\\opendlg\\command", NULL, &dwType, (PBYTE)pszValue, &dwSize))
		{
			if (NULL != StrStrI(pszValue, strKeyValue))
			{
				// 另外增加 [2/24/2011 zhangbaoliang]
				::SHSetValue(HKEY_CLASSES_ROOT, L"Unknown\\shell\\opendlg\\command",NULL,REG_EXPAND_SZ,lpDefValue,_tcslen(lpDefValue)*sizeof(TCHAR));
				LPCTSTR pszValue = L"{e44e9428-bdbc-4987-a099-40dc8fd255e7}";
				::SHSetValue(HKEY_CLASSES_ROOT, L"Unknown\\shell\\openas\\command", L"DelegateExecute", REG_SZ, pszValue, _tcslen(pszValue)*sizeof(TCHAR));
			}
		}
	}


	return nRet;
}