#pragma once
#include <wtlhelper/whwindow.h>
#include "BeikeVulfixUtils.h"
#include "ListViewCtrlEx.h"
#include "HyperTextCtrl.h"

class CDlgViewDetail 
	: public CBkDialogImpl<CDlgViewDetail>
	, public CWHRoundRectFrameHelper<CDlgViewDetail>
{
public:
	CDlgViewDetail(void);
	~CDlgViewDetail(void);
	
public:
	void SetInitTab(INT nTab);

protected:
	LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
	LRESULT OnInitScan(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnListLinkClicked(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	
	void OnBkBtnClose();
	void OnBkBtnOpenHotfixDiretory();
	void OnBkBtnSelectAll();
	void OnBkBtnSelectNone();
	void OnBkBtnCancelIgnore();
	BOOL OnBkTabMainSelChange(int nTabItemIDOld, int nTabItemIDNew);
	LRESULT OnDoneScan(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnVulFixEventHandle(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
    void OnBkBtnExport();

protected:
	void _AppendItem2ListCtrl( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem );
	void _FillFixed();
	void _UpdateFixedTitle();
	void _FillIgnored();
	void _UpdateIgnoredTitle();	
	void _FillSuperseded();	
	void _FillInvalid();
	void _EnableIgnoreButton( BOOL bChecked );
	void _EnableIgnoreSelectLink( BOOL bChecked );

public:
	BOOL m_bDirty;	// 是否已经修改
	BOOL m_bDisplayInstallDate;
	
protected:
	INT m_nTab, m_nCurrentTab;
	BOOL m_bLoadedFixed, m_bLoadedIgnored, m_bLoadedSuperseded, m_bLoadedInvalid;
	CListViewCtrlEx m_wndListFixed, m_wndListIgnored, m_wndListSuperseded, m_wndListInvalid;
	CHyperTextCtrl m_ctlTitleFixed, m_ctlTitleIgnored, m_ctlTitleSuperseded, m_ctlTitleInvalid;

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		BK_NOTIFY_TAB_SELCHANGE(IDC_TAB_MAIN, OnBkTabMainSelChange)
		BK_NOTIFY_ID_COMMAND(20503, OnBkBtnClose)
				
		BK_NOTIFY_ID_COMMAND(20002, OnBkBtnOpenHotfixDiretory)
		BK_NOTIFY_ID_COMMAND(30500, OnBkBtnSelectAll)
		BK_NOTIFY_ID_COMMAND(30501, OnBkBtnSelectNone)
		BK_NOTIFY_ID_COMMAND(30502, OnBkBtnCancelIgnore)
        BK_NOTIFY_ID_COMMAND(2001,  OnBkBtnExport)
	BK_NOTIFY_MAP_END()
	
	BEGIN_MSG_MAP_EX(CDlgViewDetail)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CDlgViewDetail>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CDlgViewDetail>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER(WMH_INIT_SCAN, OnInitScan)
		MESSAGE_HANDLER(WMH_LISTEX_LINK, OnListLinkClicked)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

};
