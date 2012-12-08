#include "StdAfx.h"
#include "DlgMain.h"
#include "DlgOption.h"

CDlgMain::CDlgMain( void )
{
	m_firstLoaded = TRUE;
	m_nCurrentTab = 0;
	m_bNoDisturb = FALSE;
	m_bReparing = FALSE;
	m_nItemID = -1;
	m_nTimer = 0;
}

CDlgMain::~CDlgMain(void)
{
}

void CDlgMain::RunBackGround()
{
	::ShowWindow(m_hWnd, SW_SHOWMINIMIZED);
}

void CDlgMain::ShutDownComputer( BOOL toReboot )
{
	BOOL bExit = TRUE;
	if(m_bReparing)
	{
		bExit = ::MessageBox(m_hWnd, _T("有正在修复的项, 是否退出? "), NULL, MB_YESNO)==IDYES;
	}
	
	if(!bExit)
		return; 

#ifdef _DEBUG
	::MessageBox(NULL, _T(""), _T("ShutDownComputer"), MB_OK);
#else
	UINT flags = toReboot ? EWX_REBOOT : EWX_SHUTDOWN;
	::ExitWindowsEx( flags | EWX_FORCE, 0);
#endif
}

void CDlgMain::SetNoDisturb( BOOL bNoDisturb, BOOL bReparing, int uItemID )
{
	m_nItemID = uItemID;
	if(bNoDisturb)
	{
		if(!m_nTimer)
			m_nTimer = SetTimer(TIMER_RUNNING, 200, NULL);
	}
	else
	{
		if(m_nTimer)
		{
			KillTimer(TIMER_RUNNING);
			m_nTimer = 0;
		}
	}
	m_bNoDisturb = bNoDisturb;
	m_bReparing = bReparing;
}

void CDlgMain::OnTimer( UINT_PTR nIDEvent )
{
#if 0 
	if(nIDEvent==TIMER_RUNNING)
	{
		if(m_nItemID>0)
		{
			static int nSubImage = 0;
			nSubImage = ++nSubImage%8;
			SetItemIntAttribute(m_nItemID, "sub", nSubImage);
		}		
	}
	return;
#endif
}

void CDlgMain::OnBkBtnClose()
{
	EndDialog(IDCANCEL);
}

void CDlgMain::OnBkBtnMin()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE | HTCAPTION, 0);
}

void CDlgMain::OnBkBtnMax()
{
	if (WS_MAXIMIZE == (GetStyle() & WS_MAXIMIZE))
	{
		SendMessage(WM_SYSCOMMAND, SC_RESTORE | HTCAPTION, 0);
		SetItemIntAttribute(IDBK_BTN_MAX, "src", IDB_BTN_SYS_MAX);
	}
	else
	{
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE | HTCAPTION, 0);
		SetItemIntAttribute(IDBK_BTN_MAX, "src", IDB_BTN_SYS_RESTORE);
	}
}

void CDlgMain::OnBkBtnOption()
{
	CDlgOption dlg;
	dlg.Load(IDR_BK_OPTION_DIALOG);
	GetDownloadPath( dlg.m_strPath );
	if( IDOK==dlg.DoModal() )
	{
		if( !CreateDir(dlg.m_strPath, TRUE) )
		{
			CString str;
			str.Format(_T("创建目录\"%s\"失败, 没有修改路径"), dlg.m_strPath);
			::MessageBox(m_hWnd, str, NULL, MB_OK);
		}
		else
		{
			SaveDownloadPath( dlg.m_strPath );
		}
	}
}

LRESULT CDlgMain::OnInitDialog( HWND /*hWnd*/, LPARAM /*lParam*/ )
{
	m_viewMain.Create(GetViewHWND(), 1001);
	ATLASSERT( m_viewMain.Load( IDR_BK_VULDLG_MAIN ) );

	//m_viewSoftVul.Init( m_viewMain.m_hWnd );
	m_viewMain.Init();
	
	PostMessage( WMH_SCAN_START, 0, 0);
	return TRUE;
}

void CDlgMain::OnShowWindow( BOOL bShow, UINT nStatus )
{
	
}

BOOL CDlgMain::OnBkTabMainSelChange( int nTabItemIDOld, int nTabItemIDNew )
{
	if(m_bNoDisturb)
		return FALSE;
	
	m_nCurrentTab = nTabItemIDNew;
	return TRUE;
}

LRESULT CDlgMain::OnScanDone( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{

	return 0;
}

LRESULT CDlgMain::OnRepaireDone( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{

	return 0;
}

LRESULT CDlgMain::OnVulFixEventHandle( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	return 0;
}
