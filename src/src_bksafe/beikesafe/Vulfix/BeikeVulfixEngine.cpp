#include "StdAfx.h"
#include "BeikeVulfixEngine.h"
#include <algorithm>
#include <functional>
#include <process.h>
#include "BeikeVulfixUtils.h"
#include <bksafe/bksafeconfig.h>
#include "../beikesafemsgbox.h"
#include <vulfix/Defines.h>

CVulEngine *theEngine = NULL;

class CWindowVulfixObserver : public IVulfixObserver
{
public:
	CWindowVulfixObserver(HWND hWnd) : m_hWnd(hWnd)
	{
		m_bSmartInstallBegined = FALSE;
		//ATLASSERT(m_hWnd && ::IsWindow(m_hWnd));
	}
	
	virtual void OnVulfixEvent( TVulFixEventType evt, int nKbId, DWORD dwParam1, DWORD dwParam2 )
	{
		//DEBUG_TRACE(_T("LeakEvent %d, %d (%d, %d)\n"), evt, nKbId, dwParam1, dwParam2);

		/*
		EVulfix_Error,				// 错误				
		EVulfix_ScanBegin,			// Scan 开始		nType, Num 
		EVulfix_ScanProgress,		// Scan 进度		nType, nCurrentItem 

		EVulfix_DownloadBegin,		// 下载开始			id  currentnum,	totalnum  
		EVulfix_DownloadProcess,	// 下载进度			id, downloadedsize, fullsize
		EVulfix_DownloadEnd,		// 下载结束			id, currentnum, down.result

		EVulfix_InstallBegin,		// 安装开始			id, currentnum,	totalnum 
		EVulfix_InstallEnd,			// 安装结束			id, currentnum, install.result 

		EVulfix_Task_Complete,		// 任务完全完成 
		EVulfix_Task_Error,			// 任务有错, 可能部分完成 
		*/
        if(EVulfix_InstallBegin==evt)
            m_bSmartInstallBegined = FALSE;
        else if(EVulfix_DownloadError==evt || EVulfix_DownloadEnd==evt || EVulfix_InstallError==evt || EVulfix_InstallBeginUser==evt || EVulfix_InstallEnd==evt || EVulfix_Download_Check_Error==evt )
        {

            if( EVulfix_InstallBeginUser==evt )
                m_bSmartInstallBegined = TRUE;
		}
		::PostMessage( m_hWnd, WMH_VULFIX_BASE+evt, nKbId, dwParam1 );
	}

protected:
	HWND m_hWnd;
	BOOL m_bSmartInstallBegined;
};

DWORD WINAPI CVulEngine::ThreadFunc_Scan( LPVOID lpParam )
{
	HWND hWnd = (HWND)lpParam;
	theEngine->_ScanVul(hWnd);	
	return 0;
}

DWORD WINAPI CVulEngine::ThreadFunc_Repair( LPVOID lpParam )
{
	HWND hWnd = (HWND)lpParam;
	theEngine->_RepairVul(hWnd);	
	return 0;
}

DWORD WINAPI CVulEngine::ThreadFunc_ScanFixed( LPVOID lpParam )
{
	HWND hWnd = (HWND)lpParam;
	theEngine->_ScanFixed( hWnd	);
	return 0;
}

DWORD WINAPI CVulEngine::ThreadFunc_ScanIngored( LPVOID lpParam )
{
	HWND hWnd = (HWND)lpParam;
	theEngine->_ScanIgnored( hWnd );
	return 0;
}

DWORD WINAPI CVulEngine::ThreadFunc_ScanSuperseded( LPVOID lpParam )
{
	HWND hWnd = (HWND)lpParam;
	theEngine->_ScanSuperseded( hWnd );
	return 0;
}

CVulEngine::CVulEngine()
    : m_hModuleVulFixEngine(NULL)
    , m_pfnCreateVulFix(NULL)
    , m_pfnCreateVulFixed(NULL)
    , m_pfnCreateSoftVulFix(NULL)
    , m_pfnCreateRepairVul(NULL)
	, m_pfnCreateObj(NULL)
	, m_pVulEnv(NULL)
{
	m_bUseInterface = TRUE;
	
	m_pRepairVul = NULL;
	
	m_pVulScan = NULL;
	m_hThreadVulScan = NULL;
	m_hThreadVulRepair = NULL;
	m_bVulScanCanceled = FALSE;
	m_isRepairing = FALSE;

	m_bRepairCanceled = FALSE;
	// 
	m_pIVulFixed = NULL;
	m_pIVulFixedSoft = NULL;
	m_hThreadVulFixed = NULL;
	m_bVulFixedCanceled = FALSE;
	
	// superseded
	m_pVulScanSuperseded = NULL;
	m_hThreadSuperseded = NULL;
	m_bVulScanSuperseded = FALSE;

	// ignored
	m_pVulScanIgnored = NULL;
	m_hThreadVulIgnored = NULL;
	m_bVulIgnoredCanceled = FALSE;

	// 
	m_hThreadSoftVul = NULL;
	m_bCanceledSoftVul = FALSE;

	// express scan
	m_pVulExpressScan = NULL;

	m_fixLog.Load();
}

CVulEngine::~CVulEngine()
{
    if (m_hModuleVulFixEngine)
    {
        ::FreeLibrary(m_hModuleVulFixEngine);
        m_hModuleVulFixEngine = NULL;
    }
}

void CVulEngine::SetUseIUpdate( BOOL bUseIUpdate )
{
	m_bUseInterface = bUseIUpdate;
}

void CVulEngine::IgnoreVuls( int nKB, bool bIgnore )
{
	if(m_pVulScan==NULL)
	{
		m_pVulScan = CreateVulFix();
	}
	if(m_pVulScan)
	{
		m_pVulScan->Ignore(nKB, bIgnore);
		m_pVulScan->PersistIgnored();
	}
}

void CVulEngine::IgnoreVuls( CSimpleArray<int> &arr, bool bIgnore )
{
	if(m_pVulScan==NULL)
	{
		m_pVulScan = CreateVulFix();
	}
	if(m_pVulScan)
	{
		for(int i=0; i<arr.GetSize(); ++i)
		{
			m_pVulScan->Ignore(arr[i], bIgnore);
		}
		m_pVulScan->PersistIgnored();
	}
}

bool CVulEngine::ScanVul(HWND hWnd)
{
	if(m_hThreadVulScan)
		_SafeTerminateThread( m_hThreadVulScan, FALSE );

	m_bVulScanCanceled = FALSE;
	m_hThreadVulScan = CreateThread(NULL, 0, ThreadFunc_Scan, (void*)hWnd, 0, NULL);
	return m_hThreadVulScan!=NULL;
}

bool CVulEngine::CancelScanVul()
{
	if(!m_hThreadVulScan)
		return false;
	
	m_bVulScanCanceled = TRUE;
	m_pVulScan->Cancel();
	_SafeTerminateThread( m_hThreadVulScan, FALSE );
	return true;
}

void CVulEngine::_ScanVul(HWND hWnd)
{
	DWORD dwFlags = _GetScanFlags();
	CWindowVulfixObserver observer( hWnd);
	if(!m_pVulScan)
		m_pVulScan = CreateVulFix();
	
	HRESULT hr = E_POINTER;
	if(m_pVulScan)
	{
		m_pVulScan->SetObserver(&observer);
		
		hr = m_pVulScan->Scan(dwFlags);
		DEBUG_TRACE(_T("CVulEngine::_ScanVul ScanVul %x(%x) \n"), hr, dwFlags);
		m_pVulScan->SetObserver(NULL);
	}
	_RelayMessage(hWnd, WMH_SCAN_DONE, m_bVulScanCanceled, hr);
}

bool CVulEngine::ScanFixed(HWND hWnd)
{
	if(m_hThreadVulFixed)
		_SafeTerminateThread( m_hThreadVulFixed, FALSE );
	
	m_bVulFixedCanceled = FALSE;
	m_hThreadVulFixed = (HANDLE) CreateThread(NULL, 0, ThreadFunc_ScanFixed, (void*)hWnd, 0, NULL);
	return m_hThreadVulFixed!=NULL;
}

void CVulEngine::CancelScanFixed()
{
	if(!m_hThreadVulFixed)
		return;

	m_bVulFixedCanceled = TRUE;
	m_pIVulFixedSoft->Cancel();
	m_pIVulFixed->Cancel();
	_SafeTerminateThread( m_hThreadVulFixed, FALSE );
}

void CVulEngine::_ScanFixed(HWND hWnd)
{
	DWORD dwFlags = _GetScanFlags();

	if(!m_pIVulFixed)
		m_pIVulFixed = CreateVulFixed();
	if(!m_pIVulFixedSoft)
		m_pIVulFixedSoft = CreateSoftVulFix();
	
	HRESULT hr = E_POINTER;
	if(m_pIVulFixed && m_pIVulFixedSoft)
	{
		CWindowVulfixObserver observer(hWnd);
		m_pIVulFixed->SetObserver(&observer);
		m_pIVulFixedSoft->SetObserver(&observer);
		_RelayMessage(hWnd, WMH_SCAN_FIXED_BEGIN, 0, 0 );
		hr = m_pIVulFixed->Scan( dwFlags );
		if( SUCCEEDED(hr) && !m_bVulFixedCanceled )
			hr = m_pIVulFixedSoft->Scan( dwFlags );
		m_pIVulFixed->SetObserver(NULL);
		m_pIVulFixedSoft->SetObserver(NULL);
	}
	_RelayMessage(hWnd, WMH_SCAN_FIXED_DONE, m_bVulFixedCanceled, hr );
}

bool CVulEngine::ScanSuperseded(HWND hWnd)
{
	if(m_hThreadSuperseded)
		_SafeTerminateThread( m_hThreadSuperseded, FALSE );

	m_bVulScanSuperseded = FALSE;
	m_hThreadSuperseded = CreateThread(NULL, 0, ThreadFunc_ScanSuperseded, (void*)hWnd, 0, NULL);
	return m_hThreadSuperseded!=NULL;
}

bool CVulEngine::CancelScanSuperseded()
{
	if(!m_hThreadSuperseded)
		return false;

	m_bVulScanSuperseded = TRUE;
	m_pVulScanSuperseded->Cancel();
	_SafeTerminateThread( m_hThreadSuperseded, FALSE );
	return true;
}

void CVulEngine::_ScanSuperseded(HWND hWnd)
{
	DWORD dwFlags = _GetScanFlags();
	CWindowVulfixObserver observer( hWnd);
	if(!m_pVulScanSuperseded)
		m_pVulScanSuperseded = CreateVulFix();
	
	HRESULT hr = E_POINTER;
	if(m_pVulScanSuperseded)
	{
		m_pVulScanSuperseded->SetObserver(&observer);
		
		_RelayMessage(hWnd, WMH_SCAN_FIXED_BEGIN, 0, 0 );
		hr = m_pVulScanSuperseded->Scan(dwFlags);
		m_pVulScanSuperseded->SetObserver(NULL);
	}
	_RelayMessage(hWnd, WMH_SCAN_FIXED_DONE, m_bVulScanSuperseded, hr);
}

bool CVulEngine::ScanIgnored( HWND hWnd )
{
	if(m_hThreadVulIgnored)
		_SafeTerminateThread( m_hThreadVulIgnored, FALSE );
	
	m_bVulIgnoredCanceled = FALSE;
	m_hThreadVulIgnored = (HANDLE) CreateThread(NULL, 0, ThreadFunc_ScanIngored, (void*)hWnd, 0, NULL);
	return m_hThreadVulIgnored!=NULL;
}

bool CVulEngine::CancelScanIgnored()
{
	if(!m_hThreadVulIgnored)
		return false;
	m_bVulIgnoredCanceled = TRUE;
	m_pVulScanIgnored->Cancel();
	_SafeTerminateThread( m_hThreadVulIgnored, FALSE );
	return true;
}

void CVulEngine::_ScanIgnored( HWND hWnd )
{
	DWORD dwFlags = _GetScanFlags();
	if(!m_pVulScanIgnored)
		m_pVulScanIgnored = CreateVulFix();

	HRESULT hr = E_POINTER;
	if(m_pVulScanIgnored)
	{
		CWindowVulfixObserver observer(hWnd);
		m_pVulScanIgnored->SetObserver(&observer);
		_RelayMessage(hWnd, WMH_SCAN_FIXED_BEGIN, 0, 0 );
		hr = m_pVulScanIgnored->Scan( dwFlags );
		m_pVulScanIgnored->SetObserver(NULL);
	}	
	_RelayMessage(hWnd, WMH_SCAN_FIXED_DONE, m_bVulIgnoredCanceled, hr );
}

BOOL CVulEngine::CheckRepairPrequisite(BOOL hasOfficeVul)
{
	try
	{
		return _CheckDiskFreeSpace() && _CheckUpdateRelatedSvc() && (!hasOfficeVul || _CheckNeedCloseOffice());
	}
	catch (...)
	{
	}
	return TRUE;	
}

bool CVulEngine::RepairAll( HWND hWnd, const CSimpleArray<int> &arrVulIds, const CSimpleArray<int> &arrSoftVulIds )
{
	if(arrVulIds.GetSize()==0 && arrSoftVulIds.GetSize()==0)
	{
		return false;
	}
	
	m_bRepairCanceled = FALSE;	
	if(m_hThreadVulRepair!=NULL)
		_SafeTerminateThread( m_hThreadVulRepair, FALSE );	
	
	m_arrRepairVulIds.RemoveAll();
	m_arrRepairVulSoftIds.RemoveAll();
	CopySimpleArray(arrVulIds, m_arrRepairVulIds);
	CopySimpleArray(arrSoftVulIds, m_arrRepairVulSoftIds);
	m_hThreadVulRepair = (HANDLE) CreateThread(NULL, 0, ThreadFunc_Repair, (void*)hWnd, 0, NULL);

	m_isRepairing = TRUE;
	return true;
}

bool CVulEngine::CancelRepair()
{
	if(!m_hThreadVulRepair)
		return false;
	
	m_bRepairCanceled = TRUE;
	m_pRepairVul->StopRepair();
	_SafeTerminateThread( m_hThreadVulRepair, FALSE );
	//_RelayMessage( NULL, WMH_REPAIR_DONE, 1, 0);
	m_isRepairing = FALSE;
	return true;
}

void CVulEngine::_RepairVul(HWND hWnd)
{
	ATLASSERT(m_pVulScan);
	CWindowVulfixObserver observer(hWnd);
	if(!m_pRepairVul)
		m_pRepairVul = CreateRepairVul();
	HRESULT hr = E_POINTER;
	if(m_pRepairVul)
	{
		m_pRepairVul->SetObserver( &observer );
		m_pRepairVul->Reset();
		m_pRepairVul->AddSoftVul(m_pVulScan, m_arrRepairVulSoftIds);
		m_pRepairVul->AddVul(m_pVulScan, m_arrRepairVulIds);

		CString strPath, strImportPath;
		GetDownloadPath( strPath );
		BKSafeConfig::Get_Vulfix_Download_LocalPath( strImportPath );
		BOOL bImportLocal = BKSafeConfig::Get_Vulfix_Download_UseLocal();
		BOOL bDownloadIfImportError = BKSafeConfig::Get_Vulfix_Download_DownloadIfNotExists();
		DWORD dwFlags = 0;
		if(bImportLocal && !strImportPath.IsEmpty())
		{
			dwFlags |= VULFLAG_REPAIR_IMPORT_LOCAL;
			if(bDownloadIfImportError)
				dwFlags |= VULFLAG_REPAIR_DOWNLOAD_IF_FAIL_IMPORT;
		}
		
		hr = m_pRepairVul->Repair(strPath, strImportPath, dwFlags);
		m_pRepairVul->WaitForRepairDone();	
		m_pVulScan->SetObserver(NULL);
	}
	m_isRepairing = FALSE;
	_RelayMessage( hWnd, WMH_REPAIR_DONE, m_bRepairCanceled, hr);
}

void CVulEngine::_SafeTerminateThread( HANDLE &hThread, BOOL bForceKill )
{
	DWORD dwExitCode;
	if( GetExitCodeThread( hThread , &dwExitCode ) )
	{
		if( dwExitCode == STILL_ACTIVE && bForceKill )
		{
			SuspendThread( hThread ) ;
			TerminateThread( hThread , 0 ) ;
		}
	}	
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	hThread = NULL;
}

void CVulEngine::_RelayMessage( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	if(hWnd && ::IsWindow(hWnd))
	{
		::PostMessage(hWnd, Msg, wParam, lParam );
	}
	else
	{
		ATLASSERT(FALSE);
	}
}

DWORD CVulEngine::_GetScanFlags( )
{
	DWORD dwFlags = 0;
	if(m_bUseInterface)
		dwFlags |= VULSCAN_USE_IUPDATE;
#ifdef _DEBUG
	dwFlags = 0;
#endif
	return dwFlags;
}

IVulfix* CVulEngine::CreateVulFix()
{
    if (NULL == m_hModuleVulFixEngine)
    {
        if (!_InitFunctions())
            return NULL;
    }

    if (NULL == m_pfnCreateVulFix)
        return NULL;

    return m_pfnCreateVulFix(BEIKESAFE_VULSCAN_DLL_VERSION);
}

IVulfixed* CVulEngine::CreateVulFixed()
{
    if (NULL == m_hModuleVulFixEngine)
    {
        if (!_InitFunctions())
            return NULL;
    }

    if (NULL == m_pfnCreateVulFixed)
        return NULL;

    return m_pfnCreateVulFixed(BEIKESAFE_VULSCAN_DLL_VERSION);
}

ISoftVulfix* CVulEngine::CreateSoftVulFix()
{
    if (NULL == m_hModuleVulFixEngine)
    {
        if (!_InitFunctions())
            return NULL;
    }

    if (NULL == m_pfnCreateSoftVulFix)
        return NULL;

    return m_pfnCreateSoftVulFix(BEIKESAFE_VULSCAN_DLL_VERSION);
}

IRepairVul* CVulEngine::CreateRepairVul()
{
    if (NULL == m_hModuleVulFixEngine)
    {
        if (!_InitFunctions())
            return NULL;
    }

    if (NULL == m_pfnCreateRepairVul)
        return NULL;

    return m_pfnCreateRepairVul(BEIKESAFE_VULSCAN_DLL_VERSION);
}

BOOL CVulEngine::_InitFunctions()
{
	if( m_hModuleVulFixEngine )
		return TRUE;

    CString strDllPath =_Module.GetAppDirPath();
    m_hModuleVulFixEngine = ::LoadLibrary(strDllPath + L"ksafevul.dll");
    if (NULL == m_hModuleVulFixEngine)
        return FALSE;

    m_pfnCreateVulFix = (FnCreateVulFix)::GetProcAddress(m_hModuleVulFixEngine, FUNCNAME_CREATEVULFIX);
    if (NULL == m_pfnCreateVulFix)
        goto Exit0;
    m_pfnCreateVulFixed = (FnCreateVulFixed)::GetProcAddress(m_hModuleVulFixEngine, FUNCNAME_CREATEVULFIXED);
    if (NULL == m_pfnCreateVulFixed)
        goto Exit0;
    m_pfnCreateSoftVulFix = (FnCreateSoftVulFix)::GetProcAddress(m_hModuleVulFixEngine, FUNCNAME_CREATESOFTVULFIX);
    if (NULL == m_pfnCreateSoftVulFix)
        goto Exit0;
    m_pfnCreateRepairVul = (FnCreateRepairVul)::GetProcAddress(m_hModuleVulFixEngine, FUNCNAME_CREATEREPAIRVUL);
    if (NULL == m_pfnCreateRepairVul)
        goto Exit0;
	m_pfnCreateObj = (FnCreateObject)::GetProcAddress(m_hModuleVulFixEngine, FUNCNAME_CreateObject);
	if( !m_pfnCreateObj )
		goto Exit0;
	m_pfnCreateObj( __uuidof(IVulEnvironment), (void**)&m_pVulEnv );
	if( !m_pVulEnv )
		goto Exit0;

    return TRUE;

Exit0:
    FreeLibrary(m_hModuleVulFixEngine);
    m_hModuleVulFixEngine = NULL;

    return FALSE;
}

int CVulEngine::ExpressScan()
{
	if(m_pVulExpressScan==NULL)
	{
		m_pVulExpressScan = CreateVulFix();
	}
	if(m_pVulExpressScan)
	{
		return m_pVulExpressScan->ExpressScanSystem();
	}
	return 0;
}

BOOL CVulEngine::IsUsingInterface()
{
	BOOL bRet = FALSE;
	if( m_pVulEnv )
		m_pVulEnv->IsUsingInterface( &bRet );
	return bRet;
}

BOOL CVulEngine::IsSystemSupported()
{
	BOOL bRet = TRUE;
	if( m_pVulEnv )
		m_pVulEnv->IsSystemSupport( &bRet );
	return bRet;
}

BOOL CVulEngine::_CheckDiskFreeSpace()
{
	CString strDisk = _T("C:\\");
	TCHAR szPath[MAX_PATH+1] = {0};
	if( SHGetSpecialFolderPath(NULL, szPath, CSIDL_WINDOWS, FALSE) )
	{
		strDisk.SetString(szPath, 3);
	}
	ULARGE_INTEGER   lpuse = {0}; 
	ULARGE_INTEGER   lptotal = {0};
	ULARGE_INTEGER   lpfree = {0};
	GetDiskFreeSpaceEx(strDisk,&lpuse,&lptotal,&lpfree);
	const int MEGA = 1024 * 1024;
	const int SPACE_LIMIT = MEGA * 100;
	if(lpfree.QuadPart<SPACE_LIMIT)
	{
		LPCTSTR szBtnClean = BkString::Get(IDS_VULFIX_5000);
		LPCTSTR szBtnCancel = BkString::Get(IDS_MSGBOX_CANCEL);

		LPCTSTR szMsgPrompt = BkString::Get(IDS_VULFIX_5001);

		CBkSafeMsgBox dlg;		
		dlg.AddButton( szBtnClean, IDYES);
		dlg.AddButton( szBtnCancel, IDCANCEL);
		UINT nRet = dlg.ShowMsg(szMsgPrompt, NULL, MB_BK_CUSTOM_BUTTON|MB_ICONQUESTION, NULL);
		if(nRet==IDYES)
		{
			// C:\WINDOWS\\system32\\cleanmgr.exe 
			LPCTSTR szCmd = _T("cleanmgr.exe");
			::ShellExecute(NULL,_T("open"), szCmd, NULL, NULL, SW_SHOW);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CVulEngine::_CheckUpdateRelatedSvc()
{
	BOOL bCryptExists = TRUE;
	if(IsCryptServiceExists(bCryptExists))
	{
		if(!bCryptExists)
		{
			INT nHasFailedInstallCryptSvc = BKSafeConfig::Get_Vulfix_ProfileInt(_T("Main"), _T("FailInstallCryptSvc"), 0);
			LPCTSTR szBtnInstall = BkString::Get(IDS_VULFIX_5002);
			LPCTSTR szBtnReport = BkString::Get(IDS_VULFIX_5003);
			LPCTSTR szBtnCancel = BkString::Get(IDS_MSGBOX_CANCEL);

			LPCTSTR szMsgPrompt = BkString::Get(IDS_VULFIX_5004);
			LPCTSTR szMsgInstallFail = BkString::Get(IDS_VULFIX_5005);

			CBkSafeMsgBox dlg;		
			dlg.AddButton( szBtnInstall, IDYES);
			dlg.AddButton( szBtnCancel, IDCANCEL);
			UINT nRet = dlg.ShowMsg(szMsgPrompt, NULL, MB_BK_CUSTOM_BUTTON|MB_ICONQUESTION, NULL);
			if(nRet==IDYES)
			{
				BOOL bInstallOK = InstallCryptService();
				if(!bInstallOK)
				{
					BKSafeConfig::Set_Vulfix_ProfileInt(_T("Main"), _T("FailInstallCryptSvc"), 1);

					CBkSafeMsgBox dlg;		
					dlg.AddButton( szBtnReport, IDYES);
					dlg.AddButton( szBtnCancel, IDCANCEL);
					UINT nRet = dlg.ShowMsg(szMsgInstallFail, NULL, MB_BK_CUSTOM_BUTTON|MB_ICONEXCLAMATION, NULL);
					if(nRet==IDYES)
					{
						::ShellExecute(NULL,_T("open"), _T("http://bbs.ijinshan.com/thread-1013-1-1.html"), NULL, NULL, SW_SHOW);
						return FALSE;
					}
				}
			}
		}
	}
	EnableUpdateRelatedService();
	return TRUE;
}

BOOL CVulEngine::_CheckNeedCloseOffice()
{
	CString strOfficeProducts;
	if(IsOfficeProcessExists(strOfficeProducts))
	{
		LPCTSTR szBtnGo = BkString::Get(IDS_VULFIX_5006);
		LPCTSTR szBtnLater = BkString::Get(IDS_VULFIX_5007);
		LPCTSTR szMsgPrompt = BkString::Get(IDS_VULFIX_5008);
		CString strMsg;
		strMsg.Format(szMsgPrompt, strOfficeProducts);

		CBkSafeMsgBox dlg;		
		dlg.AddButton( szBtnGo, IDYES);
		dlg.AddButton( szBtnLater, IDCANCEL);
		UINT nRet = dlg.ShowMsg(strMsg, NULL, MB_BK_CUSTOM_BUTTON|MB_ICONWARNING, NULL);
		return nRet==IDYES;
	}
	return TRUE;
}
