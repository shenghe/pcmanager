#pragma once
#include "../bkwinres.h"
#include "BeikeVulfix.h"
#include <wtlhelper/whwindow.h>

class CDlgShutdownPrompt 
	: public CBkDialogImpl<CDlgShutdownPrompt>
	, public CWHRoundRectFrameHelper<CDlgShutdownPrompt>
{
public:
	CDlgShutdownPrompt(void);
	~CDlgShutdownPrompt(void);

protected:
	LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
	void OnBkBtnShutdown();
	void OnBkBtnClose();
	
public:
	void OnTimer( UINT_PTR nIDEvent );

protected:
	void End( UINT uRetCode );
	void _UpdateSeconds();

protected:
	UINT_PTR m_nTimer;
	INT m_nCountDown;

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(30502, OnBkBtnShutdown)
		BK_NOTIFY_ID_COMMAND(30503, OnBkBtnClose)
	BK_NOTIFY_MAP_END()
	
	BEGIN_MSG_MAP_EX(CDlgShutdownPrompt)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CDlgShutdownPrompt>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CDlgShutdownPrompt>)
		MSG_WM_INITDIALOG(OnInitDialog)		
		MSG_WM_TIMER(OnTimer)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
};
