#pragma once
#include "BeikeVulfix.h"
#include <wtlhelper/whwindow.h>
#include "BaseViewHandler.h"
#include "BeikeVulfixUtils.h"
#include "ListViewCtrlEx.h"
#include "RichEditCtrlEx.h"

class CInnerView;

#define IDC_RICHVIEW_WIN_VULFIX            1001


class CViewSoftVulHandler : public CBaseViewHandler<CInnerView>
{
public:
	CViewSoftVulHandler(CInnerView &mainDlg);
	~CViewSoftVulHandler(void);
	
public:
	BOOL Init(HWND hWndParent);
	BOOL OnViewActived(INT nTabId, BOOL bActive);
	void _SetDisplayState(TScanSoftState st, TRepairSubState subst=REPAIRSTATE_ING);

protected:
	BOOL m_firstInited;
	DWORD m_dwPos;
	CListViewCtrlEx m_wndListCtrlVul, m_wndListCtrlVulFixing;
	BOOL m_bRelateInfoShowing;
	CRichEditCtrlEx m_ctlRichEdit;
	int m_nCurrentRelateInfoItem;
	
	// scan 
	int m_nScanState, m_nTotalItem, m_nCurrentItem;
	// Repair 
	int m_nRepairTotal, m_nRepairInstalled, m_nRepairDownloaded, m_nRepairProcessed;
		
	void OnTimer(UINT_PTR nIDEvent)
	{
		SetItemDWordAttribute(IDBK_PROGRESSBAR, "value", m_dwPos += 50, TRUE);
		if (m_dwPos == 10000)
			m_dwPos = -50;
	}
	
	// BK HANDLER
	void OnBkBtnSwitchRelateInfo();
	void OnBkBtnSelectAll();
	void OnBkBtnSelectNone();
	void OnBkBtnSelectAllSuggested();
	void OnBkBtnSelectAllOptional();

	void OnBkBtnScan();
	void OnBkBtnCancelScan();
	
	void OnBkBtnBeginRepair();
	void OnBkBtnCancelRepair();
	void OnBkBtnRunBackground();
	void OnBkBtnReboot();
	
	void OnBkBtnEnableRelateCOM();
	void OnBkBtnDisableRelateCOM();

	void OnBkBtnViewIgnored();
	void OnBkBtnViewInstalled();

public:
	// WM HANDLER
	LRESULT OnListBoxVulFixNotify(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnVulFixEventHandle(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	LRESULT OnScanStart(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnScanDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRepaireDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRichEditLink(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

protected:
	int FindListItem( int nID );
	void _DisplayRelateVulFixInfo( int nItem );
	void _SetScanProgress( int nPos );
	void _SetRepairProgress( int nPos );
	void _UpdateRepairTitle();
	void _EnableRelateCOM( BOOL bEnable );
	
public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN_VULFIX)
		// 系统漏洞 
		BK_NOTIFY_ID_COMMAND(30001, OnBkBtnSwitchRelateInfo)
		
		BK_NOTIFY_ID_COMMAND(30012, OnBkBtnSelectAll)
		BK_NOTIFY_ID_COMMAND(30013, OnBkBtnSelectNone)

		BK_NOTIFY_ID_COMMAND(30014, OnBkBtnSelectAllSuggested)
		BK_NOTIFY_ID_COMMAND(30015, OnBkBtnSelectAllOptional)		

		BK_NOTIFY_ID_COMMAND(30016, OnBkBtnBeginRepair)
		BK_NOTIFY_ID_COMMAND(30017, OnBkBtnScan)
		
		// 扫描中 
		BK_NOTIFY_ID_COMMAND(30052, OnBkBtnCancelScan)
		
		// 漏洞修复
		BK_NOTIFY_ID_COMMAND(30103, OnBkBtnCancelRepair)
		BK_NOTIFY_ID_COMMAND(30104, OnBkBtnRunBackground)

		BK_NOTIFY_ID_COMMAND(30110, OnBkBtnReboot)
		BK_NOTIFY_ID_COMMAND(30111, OnBkBtnScan)

		BK_NOTIFY_ID_COMMAND(30120, OnBkBtnReboot)
		BK_NOTIFY_ID_COMMAND(30121, OnBkBtnScan)
		
		BK_NOTIFY_ID_COMMAND(30130, OnBkBtnScan)	

		// 查看已忽略/已安装
		BK_NOTIFY_ID_COMMAND(30021, OnBkBtnViewIgnored)	
		BK_NOTIFY_ID_COMMAND(30022, OnBkBtnViewInstalled)	

	BK_NOTIFY_MAP_END()
	
	BEGIN_MSG_MAP_EX(CViewSoftVulHandler)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN_VULFIX)		
		MSG_WM_TIMER(OnTimer)	
		
		MESSAGE_HANDLER(WMH_SCAN_START, OnScanStart)
		MESSAGE_HANDLER(WMH_SCAN_DONE, OnScanDone)
		MESSAGE_HANDLER(WMH_REPAIR_DONE, OnRepaireDone)	
		MESSAGE_HANDLER(WMH_SCAN_SOFT_DONE, OnScanDone)
		MESSAGE_RANGE_HANDLER(WMH_VULFIX_BASE, WMH_VULFIX_BASE+EVulfix_Task_Error, OnVulFixEventHandle)
		
		NOTIFY_ID_HANDLER(30011, OnListBoxVulFixNotify)
		NOTIFY_HANDLER(30040, EN_LINK, OnRichEditLink)
	END_MSG_MAP()
};




