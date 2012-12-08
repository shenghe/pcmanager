#include "StdAfx.h"
#include "DlgShutdownPrompt.h"
CDlgShutdownPrompt::CDlgShutdownPrompt(void)
{
	m_nTimer = 0;
	m_nCountDown = 30;
}

CDlgShutdownPrompt::~CDlgShutdownPrompt(void)
{
	
}

LRESULT CDlgShutdownPrompt::OnInitDialog( HWND /*hWnd*/, LPARAM /*lParam*/ )
{
	m_nTimer = SetTimer(TIMER_SHUTDOWN_PROMPT, 1000, NULL);
	_UpdateSeconds();
	return 0 ;
}

void CDlgShutdownPrompt::OnBkBtnShutdown()
{
	End(IDOK);
}

void CDlgShutdownPrompt::OnBkBtnClose()
{
	End( IDCANCEL );
}

void CDlgShutdownPrompt::OnTimer( UINT_PTR nIDEvent )
{
	if(nIDEvent==TIMER_SHUTDOWN_PROMPT)
	{
		-- m_nCountDown;
		if(m_nCountDown<0)
			End(IDOK);

		else
		{
			_UpdateSeconds();

		}
	}
	return;
}

void CDlgShutdownPrompt::End( UINT uRetCode )
{
	KillTimer( TIMER_SHUTDOWN_PROMPT );
	EndDialog( uRetCode );
}

void CDlgShutdownPrompt::_UpdateSeconds()
{
	CString str;
	str.Format(_T("%d"), m_nCountDown);
	SetItemText(30000, str);
}