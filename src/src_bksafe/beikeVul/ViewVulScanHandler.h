#pragma once

#include "BeikeVulfix.h"
#include <wtlhelper/whwindow.h>
#include <process.h>
#include <algorithm>
#include <functional>
#include "BaseViewHandler.h"
#include "BeikeVulfixUtils.h"
#include "ListViewCtrlEx.h"
#include "RichEditCtrlEx.h"

class CDlgMain;
class CViewVulScanHandler : public CBaseViewHandler<CDlgMain>
{
public:
	CViewVulScanHandler(CDlgMain &mainDlg);
	
	BOOL Init();
	BOOL OnViewActived(INT nTabId, BOOL bActive);
	
	void StartScan();
	int _AppendVulItem( T_VulListItemData * pVulItem );
	LRESULT OnVulFixEventHandle(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void _SetScanProgress( int nPos );

	void _SetRepairProgress( int nPos );

	void _UpdateRepairTitle();
	int FindListItem(int nID);

protected:
	int m_nPos;		// Progress pos 
	CListViewCtrlEx m_wndListCtrl;
	CRichEditCtrlEx m_ctlRichEdit;

	//
	int m_nItemOfRightInfo;
	
	// scan 
	int m_nScanState, m_nTotalItem, m_nCurrentItem;
	
	// Repair 
	int m_nRepairTotal, m_nRepairInstalled, m_nRepairDownloaded, m_nRepairProcessed;
	
	int m_nDashPage;
	
	void ShowDashPage(int nID, BOOL showIntro=TRUE);
	void OnTimer(UINT_PTR nIDEvent)
	{
		SetItemDWordAttribute(IDBK_PROGRESSBAR, "value", m_nPos += 50, TRUE);
		if (m_nPos == 10000)
			m_nPos = -50;
	}
	
	void OnBtnStartScan();
	void OnBtnCancelScan();
	void OnBtnViewVulsDetail();
	void OnBtnViewSoftVulsDetail();

	void OnBtnRepair();
	
	void OnBtnCancelRepair();
	void _SafeTerminateThread(HANDLE &hThread);
	void OnBtnRunBackground();
	void OnBtnRebootNow();
	void OnBtnRebootLater();
	
public:
	LRESULT OnScanStart(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnScanDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRepaireDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnListBoxNotify(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnRichEditLink(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

protected:
	void _DisplayRelateVulInfo( int nItem );

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(1000, OnBtnStartScan)
		BK_NOTIFY_ID_COMMAND(1001, OnBtnCancelScan)
		BK_NOTIFY_ID_COMMAND(1005, OnBtnRepair)
		BK_NOTIFY_ID_COMMAND(1006, OnBtnCancelRepair)
		BK_NOTIFY_ID_COMMAND(1007, OnBtnRunBackground)
		
		BK_NOTIFY_ID_COMMAND(1010, OnBtnViewVulsDetail)
		BK_NOTIFY_ID_COMMAND(1011, OnBtnViewSoftVulsDetail)
		
		BK_NOTIFY_ID_COMMAND(1020, OnBtnRebootNow)
		BK_NOTIFY_ID_COMMAND(1021, OnBtnRebootLater)
	BK_NOTIFY_MAP_END()
	
	BEGIN_MSG_MAP_EX(CViewVulScanHandler)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MSG_WM_TIMER(OnTimer)
		
		MESSAGE_HANDLER(WMH_SCAN_START, OnScanStart)
		MESSAGE_HANDLER(WMH_SCAN_DONE, OnScanDone)
		MESSAGE_HANDLER(WMH_REPAIR_DONE, OnRepaireDone)
		NOTIFY_ID_HANDLER(9001, OnListBoxNotify)
		NOTIFY_HANDLER(24150, EN_LINK, OnRichEditLink)
	END_MSG_MAP()
};

