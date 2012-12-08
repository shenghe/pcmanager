#pragma once
#include <wtlhelper/whwindow.h>
#include "BeikeVulfixUtils.h"

class CDlgOption 
	: public CBkRichWinImpl<CDlgOption>
	, public CWHRoundRectFrameHelper<CDlgOption>
{
public:
	CDlgOption(void);
	~CDlgOption(void);
	
public:
	CString m_strPath;
	
protected:
	LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
	void OnBkBtnReset();
	void OnBkBtnCancel();
	void OnBkBtnOK();
	
	void OnBkBtnOpenDir();
	void OnBkBtnChangeDir();
	
protected:
	CEdit m_ctlEdit;
	CFont m_font;

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(10000, OnBkBtnCancel)
		BK_NOTIFY_ID_COMMAND(10101, OnBkBtnOK)		
		BK_NOTIFY_ID_COMMAND(10102, OnBkBtnReset)
		
		BK_NOTIFY_ID_COMMAND(20101, OnBkBtnChangeDir)
		BK_NOTIFY_ID_COMMAND(20102, OnBkBtnOpenDir)
	BK_NOTIFY_MAP_END()
	
	BEGIN_MSG_MAP_EX(CDlgMain)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkRichWinImpl<CDlgOption>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CDlgOption>)
		MSG_WM_INITDIALOG(OnInitDialog)		
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
};
