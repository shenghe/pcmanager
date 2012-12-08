#pragma once


#include <wtlhelper/whwindow.h>
#include "beikesafe.h"

class CksWebFixingDlg
	: public CBkDialogImpl<CksWebFixingDlg>
	, public CWHRoundRectFrameHelper<CksWebFixingDlg>
{
public:
	CksWebFixingDlg()
		: CBkDialogImpl<CksWebFixingDlg>(IDR_K_IEFIXING_DLG)
	{
		m_nProgress = 0;
	}
	~CksWebFixingDlg(void)
	{

	}
public:
	void OnTimer(UINT_PTR nIdEvent)
	{
		if ( nIdEvent == 1 )
		{
			m_nProgress += 5;
			if ( m_nProgress > 100 )
				m_nProgress = 0;

			CStringA		strX;
			strX.Format( "%d", m_nProgress);
			this->SetItemAttribute(1, "value",strX);
		}
	}

	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{
		m_nProgress = 0;
		SetTimer(1,100);
		return TRUE;
	}

public:
	WPARAM	m_wParam;
	LPARAM	m_lParam;

protected:
	DWORD	m_nProgress;
public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CksWebFixingDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CksWebFixingDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CksWebFixingDlg>)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()
};
