#pragma once
#include <wtlhelper/whwindow.h>
#include "BeikeVulfixUtils.h"
#include "ListViewCtrlEx.h"

class CDlgDisalbeComPrompt 
	: public CBkRichWinImpl<CDlgDisalbeComPrompt>
	, public CWHRoundRectFrameHelper<CDlgDisalbeComPrompt>
{
public:
	CDlgDisalbeComPrompt(void);
	~CDlgDisalbeComPrompt(void);
	
protected:
	LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
	
	void OnBkBtnClose();
	void OnBkBtnRepair();
	
public:
	void SetItems( const CSimpleArray<T_VulListItemData*> &arrItems );

protected:
	CListViewCtrlEx m_wndListCtrlVul;
	CSimpleArray<T_VulListItemData*> m_arrSoftItems;

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(10000, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(30001, OnBkBtnRepair)
		BK_NOTIFY_ID_COMMAND(30002, OnBkBtnClose)
	BK_NOTIFY_MAP_END()
	
	BEGIN_MSG_MAP_EX(CDlgMain)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkRichWinImpl<CDlgDisalbeComPrompt>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CDlgDisalbeComPrompt>)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
};
