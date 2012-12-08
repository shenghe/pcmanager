
#include "stdafx.h"
#include "tinyxml\tinyxml.h"
#include "bksoftmgruninstall.h"
#include "beikesafesoftmgrHeader.h"
#include "BkSoftMgrPowerSweepDlg.h"


LRESULT CBkSafeSoftMgrUnstDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	SendMessage(WM_SOFTMGR_UNINSTALL_BEGIN,NULL,NULL);
	SendMessage(WM_SOFTMGR_UNINSTALL_ANALYSIS,(WPARAM)TRUE,NULL);
	m_pdataSweep = m_pSoftUnstall->DataSweep2( m_strSoftKey );

	return 0;
}

void CBkSafeSoftMgrUnstDlg::OnSetFocus(HWND wndOld)
{
	if ( !m_bShowSoftWnd ) 
		return;

	if ( !::IsWindow(m_hSoftWnd) )
		return;

	if ( m_hSoftWnd == wndOld )
		return;

	::SetForegroundWindow(m_hSoftWnd);
	return;
}


void CBkSafeSoftMgrUnstDlg::OnKillFocus(HWND wndFocus)
{
	if ( !m_bShowSoftWnd ) 
		return;

	if ( ::IsWindow(m_hSoftWnd) )
		return;

	m_hSoftWnd = wndFocus;
	return;
}


void CBkSafeSoftMgrUnstDlg::OnClose()
{
	if (!m_bSweeping)
	{
		EndDialog(IDOK);
		return;
	}

	m_bCancel	= TRUE;
	m_pdataSweep->DataCancelUninst( m_strSoftKey );
	//EndDialog(IDOK);
}


void CBkSafeSoftMgrUnstDlg::OnSweep()
{
	ATLASSERT(m_pPowerSweepdlg==NULL);
	m_pPowerSweepdlg = new CBeikeSoftMgrPowerSweepDlg(m_pHandler);
	m_pPowerSweepdlg->CopySoftRubInfo( m_arrSoftRubData, m_pdataSweep, m_strCaption );
	m_pPowerSweepdlg->SetDlgTittle(m_strCaption);
	m_pPowerSweepdlg->DoModal();
	delete m_pPowerSweepdlg;
	m_pPowerSweepdlg = NULL;

	if (!m_bSweeping)
		EndDialog(IDOK);
}


// 定时器消息
void CBkSafeSoftMgrUnstDlg::OnTimer(UINT_PTR nIdEvent)
{
	if ( m_nAnimateID != 0 )
	{
		SetItemIntAttribute(m_nAnimateID, "sub", m_nLoopValue % 8);
		++m_nLoopValue;
	}
}


LRESULT	CBkSafeSoftMgrUnstDlg::OnBegin(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_bSweeping = TRUE;
	m_nAnimateID = 0;
	m_nLoopValue = 0;
	m_nRemaintNum = 0;
	m_bShowSoftWnd = false;
	m_hSoftWnd = NULL;
	m_bCancel	= FALSE;

	//EnableItem(IDC_SOFTMGR_UNIN_FINISH, FALSE);
	::SetTimer(m_hWnd, TIMER_SOFT_UNINSTALL, 200, NULL);

	return 0;
}


LRESULT	CBkSafeSoftMgrUnstDlg::OnEnd(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//EnableItem(IDC_SOFTMGR_UNIN_FINISH, TRUE);
	::KillTimer(m_hWnd, TIMER_SOFT_UNINSTALL);
	return 0;
}



LRESULT	CBkSafeSoftMgrUnstDlg::OnAnalysis(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( wParam )
	{
		m_nAnimateID = IDC_SOFTMGR_UNIN_ANI_ANALYSIS;
		SetItemVisible(IDC_SOFTMGR_UNIN_ANALYSIS, FALSE);
		SetItemVisible(IDC_SOFTMGR_UNIN_ANI_ANALYSIS, TRUE);
	}
	else
	{
		m_nAnimateID = 0;
		SetItemVisible(IDC_SOFTMGR_UNIN_ANI_ANALYSIS, FALSE);
		SetItemVisible(IDC_SOFTMGR_UNIN_ANALYSIS_OK, TRUE);
		SetItemVisible(IDC_SOFTMGR_UNIN_ANALYSIS_TXT, TRUE);
	}

	return 0;
}


LRESULT	CBkSafeSoftMgrUnstDlg::OnInvokeUninstall(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD	nRet = (DWORD)lParam;
	if ( wParam )
	{
		m_bShowSoftWnd = true;
		m_hSoftWnd = NULL;

		m_hSoftWnd = ::GetForegroundWindow();
		if ( m_hSoftWnd == m_hWnd )
			m_hSoftWnd = NULL;

		m_nAnimateID = IDC_SOFTMGR_UNIN_ANI_INVOKE;
		SetItemVisible(DLG_SOFTMGR_UNIN_INVOKE, FALSE);

		SetItemVisible(DLG_SOFTMGR_UNIN_INVOKING, TRUE);
		SetItemVisible(IDC_SOFTMGR_UNIN_ANI_INVOKE, TRUE);
	}
	else
	{
		m_bShowSoftWnd = false;
		m_hSoftWnd = NULL;

		m_nAnimateID = 0;
		SetItemVisible(IDC_SOFTMGR_UNIN_ANI_INVOKE, FALSE);
		SetItemVisible(IDC_SOFTMGR_UNIN_INVOKE_OK, TRUE);

		if ( nRet == 0 )
			SetItemVisible(IDC_SOFTMGR_UNIN_INVOKE_FAILED, TRUE);
		else if ( nRet == 1 )
			SetItemVisible(IDC_SOFTMGR_UNIN_INVOKE_TXT, TRUE);
		else
			SetItemVisible(IDC_SOFTMGR_UNIN_INVOKE_CANCEL, TRUE);

		m_strMainDir  = m_pdataSweep->GetMainDir();

	}

	return 0;
}


LRESULT	CBkSafeSoftMgrUnstDlg::OnCheckRemaint(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( wParam )
	{
		m_nAnimateID = IDC_SOFTMGR_UNIN_ANI_SWEEP;
		SetItemVisible(DLG_SOFTMGR_UNIN_SWEEP_NORMAL, FALSE);
		SetItemVisible(DLG_SOFTMGR_UNIN_SWEEPING, TRUE);
		SetItemVisible(IDC_SOFTMGR_UNIN_ANI_SWEEP, TRUE);
	}
	else
	{
		m_nAnimateID = 0;

		SetItemVisible(DLG_SOFTMGR_UNIN_SWEEPING, FALSE);

		if ( m_nRemaintNum <= 0 )
		{
			SetItemVisible(DLG_SOFTMGR_UNIN_SWEEP_NOREMAINT, TRUE);
			m_bSoftUnistOK = TRUE;
		}
		else if ( m_nRemaintNum < 15 )
			SetItemVisible(DLG_SOFTMGR_UNIN_SWEEP_REMAINT, TRUE);
		else
			SetItemVisible(DLG_SOFTMGR_UNIN_SWEEP_TOOMUCH, TRUE);
	}

	return 0;
}


LRESULT	CBkSafeSoftMgrUnstDlg::OnGetRemaint(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( (int)wParam >= 0 )
		m_nRemaintNum = (int)wParam;

	return 0;
}


LRESULT	CBkSafeSoftMgrUnstDlg::OnPowerSweep(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}


LRESULT CBkSafeSoftMgrUnstDlg::OnUnistallEnd( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (!m_pdataSweep)
		return S_OK;

	m_bSoftUnistOK = !m_pdataSweep->DataSoftExists();
	m_bSweeping = FALSE;

	if ( m_pPowerSweepdlg == NULL && !m_bMulSoftInDir && !m_bTooMuchFile )
		EndDialog(IDOK);

	return S_OK;
}

LRESULT CBkSafeSoftMgrUnstDlg::OnMultiSoftInDir( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (!m_pdataSweep)
		return S_OK;

	if (m_bTooMuchFile)
		return S_OK;

	m_bMulSoftInDir = TRUE;

	m_nAnimateID = 0;
	SetItemVisible(DLG_SOFTMGR_UNIN_SWEEPING, FALSE);
	SetItemVisible(DLG_SOFTMGR_UNIN_SWEEP_MULTI_IN_DIR, TRUE);
	::KillTimer(m_hWnd, TIMER_SOFT_UNINSTALL);

	m_strMainDir = m_pdataSweep->GetMainDir();

	return S_OK;
}

void CBkSafeSoftMgrUnstDlg::OnOpenMainDir()
{
	if ( m_strMainDir.IsEmpty() )
		return;

	if ( !::PathIsDirectory(m_strMainDir) )
		return;

	::ShellExecute(NULL, _T("open"), m_strMainDir, NULL, NULL, SW_SHOW);
}

LRESULT CBkSafeSoftMgrUnstDlg::OnTooMuchFileInDir( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	m_bTooMuchFile	= TRUE;
	m_ullSize		= m_pdataSweep->GetRubbishSize();
	m_strMainDir	= m_pdataSweep->GetMainDir();

	m_nAnimateID = 0;
	SetItemVisible(DLG_SOFTMGR_UNIN_SWEEPING, FALSE);
	SetItemVisible(DLG_SOFTMGR_UNIN_SWEEP_TOO_MUCH, TRUE);
	::KillTimer(m_hWnd, TIMER_SOFT_UNINSTALL);

	return S_OK;
}
