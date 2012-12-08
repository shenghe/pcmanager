#include "StdAfx.h"
#include "BeikeVulfixEngine.h"
#include <algorithm>
#include <functional>
#include <process.h>
#include "DlgMain.h"
#include "BeikeVulfixUtils.h"

class CWindowVulfixObserver : public IVulfixObserver
{
public:
	CWindowVulfixObserver(CDlgMain &dlgMain, HWND hWnd) : m_dlgMain(dlgMain), m_hWnd(hWnd)
	{
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
		if(m_hWnd && ::IsWindow(m_hWnd))
			::PostMessage( m_hWnd, WMH_VULFIX_BASE+evt, nKbId, dwParam1 );
		else
			m_dlgMain.PostMessage( WMH_VULFIX_BASE+evt, nKbId, dwParam1 );
	}

protected:
	CDlgMain &m_dlgMain;
	HWND m_hWnd;
};

#if 0 

unsigned int __stdcall ThreadFunc_Scan( void *lpParam )
{
	HWND hWnd = (HWND)lpParam;
	theEngine->_ScanVul(hWnd);	
	return 0;
}

unsigned int __stdcall ThreadFunc_Repair( void *lpParam )
{
	HWND hWnd = (HWND)lpParam;
	theEngine->_RepairVul(hWnd);	
	return 0;
}

unsigned int __stdcall ThreadFunc_ScanFixed( void *lpParam )
{
	HWND hWnd = (HWND)lpParam;
	theEngine->_ScanFixed( hWnd	);
	return 0;
}

#else

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

#endif

CVulEngine::CVulEngine( CDlgMain &mainDlg ) : m_MainDlg (mainDlg)
{
	m_pRepairVul = NULL;
	
	m_pVulScan = NULL;
	m_pSoftVulScan = NULL;
	m_hThreadVulScan = NULL;
	m_hThreadVulRepair = NULL;
	m_bRepairCanceled = FALSE;
	
	// 
	m_pIVulFixed = NULL;
	m_hThreadVulFixed = NULL;
	m_bVulFixedCanceled = FALSE;
	

	// ignored
	m_pVulScanIgnored = NULL;
	m_hThreadVulIgnored = NULL;
	m_bVulIgnoredCanceled = FALSE;

	// 
	m_hThreadSoftVul = NULL;
	m_bCanceledSoftVul = FALSE;
}

CVulEngine::~CVulEngine()
{
}

void CVulEngine::IgnoreVuls( CSimpleArray<int> &arr, bool bIgnore )
{
	if(m_pVulScan==NULL)
	{
		m_pVulScan = CreateVulFix();
	}
	for(int i=0; i<arr.GetSize(); ++i)
	{
		m_pVulScan->Ignore(arr[i], bIgnore);
	}
	m_pVulScan->PersistIgnored();
}

bool CVulEngine::ScanVul(HWND hWnd)
{
	if(m_hThreadVulScan!=NULL)
	{
		_SafeTerminateThread( m_hThreadVulScan );
	}
	//m_hThreadVulScan = (HANDLE) _beginthreadex(NULL, 0, ThreadFunc_Scan, (void*)hWnd, 0, NULL);
	m_hThreadVulScan = CreateThread(NULL, 0, ThreadFunc_Scan, (void*)hWnd, 0, NULL);
	return m_hThreadVulScan!=NULL;
}

bool CVulEngine::CancelScanVul()
{
	if(!m_hThreadVulScan)
		return false;
	_SafeTerminateThread( m_hThreadVulScan );
	return true;
}

void CVulEngine::_ScanVul(HWND hWnd)
{
	CWindowVulfixObserver observer(m_MainDlg, hWnd);
	
	if(m_pVulScan==NULL)
	{
		m_pVulScan = CreateVulFix();
	}
	m_pVulScan->SetObserver(&observer);
	m_pVulScan->Scan();

	if(m_pSoftVulScan==NULL)
	{
		m_pSoftVulScan = CreateSoftVulFix();
	}
	
	m_pVulScan->SetObserver(&observer);
	m_pSoftVulScan->Scan();
	_RelayMessage(hWnd, WMH_SCAN_DONE, 0, 0);
	
	m_pVulScan->SetObserver(NULL);
	m_pSoftVulScan->SetObserver(NULL);
}

bool CVulEngine::RepairAll( HWND hWnd, const CSimpleArray<int> &arrVulIds, const CSimpleArray<int> &arrSoftVulIds )
{
	m_bRepairCanceled = FALSE;
	if(m_hThreadVulRepair!=NULL)
		_SafeTerminateThread( m_hThreadVulRepair );
	
	if(arrVulIds.GetSize()==0 && arrSoftVulIds.GetSize()==0)
	{
		return false;
	}
	
	m_arrRepairVulIds.RemoveAll();
	m_arrRepairVulSoftIds.RemoveAll();
	CopySimpleArray(arrVulIds, m_arrRepairVulIds);
	CopySimpleArray(arrSoftVulIds, m_arrRepairVulSoftIds);
	m_hThreadVulRepair = (HANDLE) CreateThread(NULL, 0, ThreadFunc_Repair, (void*)hWnd, 0, NULL);
	return true;
}

bool CVulEngine::CancelRepair()
{
	if(!m_hThreadVulRepair)
		return false;
	
	m_bRepairCanceled = TRUE;
	m_pRepairVul->StopRepair();
	_SafeTerminateThread( m_hThreadVulRepair );
	//_RelayMessage( NULL, WMH_REPAIR_DONE, 1, 0);
	return true;
}

void CVulEngine::_RepairVul(HWND hWnd)
{
	ATLASSERT(m_pVulScan && m_pSoftVulScan);
	CWindowVulfixObserver observer(m_MainDlg, hWnd);
	if(!m_pRepairVul)
		m_pRepairVul = CreateRepairVul();
	m_pRepairVul->SetObserver( &observer );
	m_pRepairVul->AddVul(m_pVulScan, m_arrRepairVulIds);
	m_pRepairVul->AddSoftVul(m_pSoftVulScan, m_arrRepairVulSoftIds);

	CString strPath;
	GetDownloadPath( strPath );
	m_pRepairVul->Repair(strPath, 0);
	m_pRepairVul->WaitForRepairDone();	
	
	_RelayMessage( hWnd, WMH_REPAIR_DONE, m_bRepairCanceled, 0);
	m_pVulScan->SetObserver(NULL);
	m_pSoftVulScan->SetObserver(NULL);
}

bool CVulEngine::ScanFixed(HWND hWnd)
{
	m_bVulFixedCanceled = FALSE;
	if(m_hThreadVulFixed!=NULL)
	{
		_SafeTerminateThread( m_hThreadVulFixed );
	}
	m_hThreadVulFixed = (HANDLE) CreateThread(NULL, 0, ThreadFunc_ScanFixed, (void*)hWnd, 0, NULL);
	return m_hThreadVulFixed!=NULL;
}

void CVulEngine::CancelScanFixed()
{
	m_bVulFixedCanceled = FALSE;
}

void CVulEngine::_ScanFixed(HWND hWnd)
{
	if(!m_pIVulFixed)
		m_pIVulFixed = CreateVulFixed();
	
	CWindowVulfixObserver observer(m_MainDlg, hWnd);
	m_pIVulFixed->SetObserver(&observer);
	_RelayMessage(hWnd, WMH_SCAN_FIXED_BEGIN, 0, 0 );
	m_pIVulFixed->Scan();
	_RelayMessage(hWnd, WMH_SCAN_FIXED_DONE, m_bVulFixedCanceled, 0 );
	m_pIVulFixed->SetObserver(NULL);
}

void CVulEngine::WaitScanDone()
{
	if(!m_hThreadVulScan)
		return;
	_SafeTerminateThread( m_hThreadVulScan );
}

void CVulEngine::_SafeTerminateThread( HANDLE &hThread )
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

bool CVulEngine::ScanIgnored( HWND hWnd )
{
	m_bVulIgnoredCanceled = FALSE;
	if(m_hThreadVulIgnored!=NULL)
	{
		_SafeTerminateThread( m_hThreadVulIgnored );
	}
	m_hThreadVulIgnored = (HANDLE) CreateThread(NULL, 0, ThreadFunc_ScanIngored, (void*)hWnd, 0, NULL);
	return m_hThreadVulIgnored!=NULL;
}

bool CVulEngine::CancelScanIgnored()
{
	m_bVulIgnoredCanceled = TRUE;
	return true;
}

void CVulEngine::_ScanIgnored( HWND hWnd )
{
	if(!m_pVulScanIgnored)
		m_pVulScanIgnored = CreateVulFix();
	
	CWindowVulfixObserver observer(m_MainDlg, hWnd);
	m_pVulScanIgnored->SetObserver(&observer);
	_RelayMessage(hWnd, WMH_SCAN_FIXED_BEGIN, 0, 0 );
	m_pVulScanIgnored->Scan();
	_RelayMessage(hWnd, WMH_SCAN_FIXED_DONE, m_bVulIgnoredCanceled, 0 );
	m_pVulScanIgnored->SetObserver(NULL);
}

void CVulEngine::_RelayMessage( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	if(hWnd && ::IsWindow(hWnd))
	{
		::PostMessage(hWnd, Msg, wParam, lParam );
	}
	else
	{
		m_MainDlg.PostMessage( Msg, wParam, (LPARAM)hWnd );
	}
}
