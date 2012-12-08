#pragma once
#include <wtlhelper/whwindow.h>
#include "BeikeVulfixUtils.h"
#include "ListViewCtrlEx.h"

class CDlgIgnoredVuls 
	: public CBkRichWinImpl<CDlgIgnoredVuls>
	, public CWHRoundRectFrameHelper<CDlgIgnoredVuls>
{
public:
	CDlgIgnoredVuls(void);
	~CDlgIgnoredVuls(void);
	
protected:
	LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
	void OnBkBtnClose();
	void OnBkBtnSelectAll();
	void OnBkBtnSelectNone();
	void OnBkBtnCancelIgnore();
	void OnBkBtnCancelScan();	
	void OnBkBtnScan();

	void OnShowWindow(BOOL bShow, UINT nStatus);
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if( nChar==_T('0') )
		{
			SetItemVisible(201, TRUE);
			SetItemVisible(300, FALSE);
		}
		else if(nChar==_T('1'))
		{
			SetItemVisible(201, FALSE);
			SetItemVisible(300, TRUE);
		}
	}

	LRESULT OnInitScan(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDoneScan(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnVulFixEventHandle(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	

protected:
	void _SetScanProgress( int nPos );
	void _UpdateTitle();

public:
	BOOL m_bDirty;	// 是否已经修改

protected:
	CListViewCtrlEx m_wndListCtrlVul;
	int m_nScanState, m_nTotalItem, m_nCurrentItem;

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(10000, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(30500, OnBkBtnSelectAll)
		BK_NOTIFY_ID_COMMAND(30501, OnBkBtnSelectNone)
		BK_NOTIFY_ID_COMMAND(30502, OnBkBtnCancelIgnore)
		BK_NOTIFY_ID_COMMAND(30503, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(30504, OnBkBtnScan)
		
		BK_NOTIFY_ID_COMMAND(1001, OnBkBtnCancelScan)
		
	BK_NOTIFY_MAP_END()
	
	BEGIN_MSG_MAP_EX(CDlgMain)
	MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkRichWinImpl<CDlgIgnoredVuls>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CDlgIgnoredVuls>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_CHAR(OnChar)

		MESSAGE_HANDLER(WMH_INIT_SCAN, OnInitScan)
		MESSAGE_HANDLER(WMH_SCAN_FIXED_DONE, OnDoneScan)		
		MESSAGE_RANGE_HANDLER(WMH_VULFIX_BASE, WMH_VULFIX_BASE+EVulfix_Task_Error, OnVulFixEventHandle)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
};
