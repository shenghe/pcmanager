#pragma once
#include <wtlhelper/whwindow.h>
#include "BeikeVulfixUtils.h"
#include "ListViewCtrlEx.h"

class CDlgInstalledVuls 
	: public CBkRichWinImpl<CDlgInstalledVuls>
	, public CWHRoundRectFrameHelper<CDlgInstalledVuls>
{
public:
	CDlgInstalledVuls(void);
	~CDlgInstalledVuls(void);

protected:
	LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
	void OnBkBtnClose();
	void OnBkBtnOpenDir();
	void OnBkBtnCancelScan();	

	void OnShowWindow(BOOL bShow, UINT nStatus);
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	LRESULT OnInitScan(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDoneScan(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnListLinkClicked(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnVulFixEventHandle(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	void OnTimer(UINT_PTR nIDEvent);

protected:
	void _SetScanProgress( int nPos );
	void _SafeKillTimer();

protected:
	CListViewCtrlEx m_wndListCtrlVul;
	int m_nScanState, m_nTotalItem, m_nCurrentItem;
	INT_PTR m_nTimerID;

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(10000, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(30503, OnBkBtnClose)

		BK_NOTIFY_ID_COMMAND(30001, OnBkBtnOpenDir)
		BK_NOTIFY_ID_COMMAND(1001, OnBkBtnCancelScan)
	BK_NOTIFY_MAP_END()
	
	BEGIN_MSG_MAP_EX(CDlgMain)

	MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkRichWinImpl<CDlgInstalledVuls>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CDlgInstalledVuls>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_CHAR(OnChar)
		
		MESSAGE_HANDLER(WMH_INIT_SCAN, OnInitScan)
		MESSAGE_HANDLER(WMH_SCAN_FIXED_DONE, OnDoneScan)
		MESSAGE_HANDLER(WMH_LISTEX_LINK, OnListLinkClicked)
		MESSAGE_RANGE_HANDLER(WMH_VULFIX_BASE, WMH_VULFIX_BASE+EVulfix_Task_Error, OnVulFixEventHandle)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()
};
