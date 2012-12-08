#pragma once
#include "BeikeVulfix.h"
#include <wtlhelper/whwindow.h>
#include "BaseViewHandler.h"
#include "ViewVulHandler.h"
#include "BeikeVulfixUtils.h"
#include "ListViewCtrlEx.h"
#include "RichEditCtrlEx.h"

class CDlgMain;

enum SCAN_STATE
{
	STATE_CANCELED,
	STATE_SCANNING,
	STATE_REPAIRING,
	STATE_REPAIRDONE,
	STATE_REPAIRDONE_PART,
	STATE_REPAIRERROR,
	STATE_END,
};

class CViewVulHandler : public CBaseViewHandler<CDlgMain>
{
public:
	CViewVulHandler(CDlgMain &mainDlg);
	~CViewVulHandler(void);
	
public:
	BOOL Init();
	BOOL OnViewActived(INT nTabId, BOOL bActive);

	void _ReloadVulInfo();
protected:
	DWORD m_dwPos;
	CListViewCtrlEx m_wndListCtrlVul;
	CListViewCtrlEx m_wndListCtrlVulFixing;
	// 
	CRichEditCtrlEx m_ctlRichEdit;
	int m_nItemOfRightInfo;
	
	BOOL m_bRelateInfoShowing;
	
	// Vulfix
	int m_nCurrentVulFixTab;

	// scan 
	int m_nScanState, m_nTotalItem, m_nCurrentItem;
	int m_nRepairTotal, m_nRepairInstalled, m_nRepairDownloaded, m_nRepairProcessed;
	
	// 系统漏洞的TabChanged 
	BOOL OnBkTabVulFixSelChanged(int nTabItemIDOld, int nTabItemIDNew);

	void _LoadTabContent( int nTabItemIDNew );

	template<typename T>
	void _FillVulFixList(T func)
	{
		ResetListCtrl(m_wndListCtrlVul);
		
		const CSimpleArray<LPTUpdateItem>& arr = theEngine->m_pVulScan->GetResults();
		for(int i=0; i<arr.GetSize(); ++i)
		{
			LPTUpdateItem pItem = arr[i];
			if( !pItem->isIgnored && func(pItem->nWarnLevel) )
			{
				AppendItem2List( m_wndListCtrlVul, CreateListItem(pItem) );
			}
		}
		_DisplayRelateVulFixInfo(-1, m_nCurrentVulFixTab);
	}
	
	void OnTimer(UINT_PTR nIDEvent)
	{
		SetItemDWordAttribute(IDBK_PROGRESSBAR, "value", m_dwPos += 50, TRUE);
		if (m_dwPos == 10000)
			m_dwPos = -50;
	}
	

public:
	LRESULT OnVulFixEventHandle(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnListBoxVulFixNotify(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	void OnBkBtnSwitchRelateInfo();

	void OnBkBtnIgnoredVuls();
	void OnBkBtnInstalledVuls();

	void OnBkBtnSelectAll();
	void OnBkBtnSelectNone();
	void OnBkBtnIgnore();
	void OnBkBtnRepair();
	
	void _SetScanState( SCAN_STATE st );
	void _DisplayRelateVulFixInfo( int nItem, int nCurrentTab );
	int FindListItem( int nID );

	void _UpdateRepairTitle();
	void _SetRepairProgress( int nPos );
	void _SetScanProgress( int nPos );
	
	
	// 
	void OnBkBtnScan();
	void OnBkBtnCancelScan();
	void OnBkBtnCancelRepair();

	void OnBkBtnRunBackground();
	void OnBkBtnReboot();

	LRESULT OnScanDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRepaireDone(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


	LRESULT OnRichEditLink(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

public:

	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		// 系统漏洞 
		BK_NOTIFY_TAB_SELCHANGE(3003, OnBkTabVulFixSelChanged)
		BK_NOTIFY_ID_COMMAND(30001, OnBkBtnSwitchRelateInfo)

		BK_NOTIFY_ID_COMMAND(29900, OnBkBtnIgnoredVuls)
		BK_NOTIFY_ID_COMMAND(29901, OnBkBtnInstalledVuls)

		BK_NOTIFY_ID_COMMAND(30500, OnBkBtnSelectAll)
		BK_NOTIFY_ID_COMMAND(30501, OnBkBtnSelectNone)
		
		BK_NOTIFY_ID_COMMAND(30510, OnBkBtnRepair)
		BK_NOTIFY_ID_COMMAND(30511, OnBkBtnIgnore)
		BK_NOTIFY_ID_COMMAND(30512, OnBkBtnScan)

		// Repair 
		BK_NOTIFY_ID_COMMAND(31102, OnBkBtnScan)
		BK_NOTIFY_ID_COMMAND(31202, OnBkBtnCancelScan)
		BK_NOTIFY_ID_COMMAND(31302, OnBkBtnCancelRepair)
		BK_NOTIFY_ID_COMMAND(31303, OnBkBtnRunBackground)

		// 修复结果
		BK_NOTIFY_ID_COMMAND(31401, OnBkBtnReboot)
		BK_NOTIFY_ID_COMMAND(31402, OnBkBtnScan)
		BK_NOTIFY_ID_COMMAND(31501, OnBkBtnReboot)
		BK_NOTIFY_ID_COMMAND(31502, OnBkBtnScan)

		BK_NOTIFY_ID_COMMAND(31601, OnBkBtnScan)
	BK_NOTIFY_MAP_END()
	
	BEGIN_MSG_MAP_EX(CViewVulHandler)
	MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)		
		MSG_WM_TIMER(OnTimer)
		
		MESSAGE_HANDLER(WMH_SCAN_DONE, OnScanDone)
		MESSAGE_HANDLER(WMH_REPAIR_DONE, OnRepaireDone)	
		NOTIFY_ID_HANDLER(299, OnListBoxVulFixNotify)
		NOTIFY_HANDLER(30450, EN_LINK, OnRichEditLink)
	END_MSG_MAP()
};



