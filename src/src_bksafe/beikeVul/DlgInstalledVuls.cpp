#include "StdAfx.h"
#include "DlgInstalledVuls.h"
#include "BeikeVulfixEngine.h"

CDlgInstalledVuls::CDlgInstalledVuls(void)
{
	m_nTimerID = 0;
}

CDlgInstalledVuls::~CDlgInstalledVuls(void)
{

}

LRESULT CDlgInstalledVuls::OnInitDialog( HWND /*hWnd*/, LPARAM /*lParam*/ )
{
	m_wndListCtrlVul.Create( 
		GetViewHWND(), NULL, NULL, 
		//WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_NOCOLUMNHEADER, 
		WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL , 
		0, 30010, NULL);
	{
		m_wndListCtrlVul.InsertColumn(0, _T("安装时间"), LVCFMT_LEFT, 100);
		m_wndListCtrlVul.InsertColumn(1, _T("补丁名称"), LVCFMT_LEFT, 100);
		m_wndListCtrlVul.InsertColumn(2, _T("补丁描述"), LVCFMT_LEFT, 250);
		m_wndListCtrlVul.InsertColumn(3, _T("更多信息"), LVCFMT_LEFT, 100);
	}
	m_wndListCtrlVul.SetObserverWindow( m_hWnd );
	PostMessage(WMH_INIT_SCAN, 0, 0);
	return 0 ;
}

void CDlgInstalledVuls::OnBkBtnClose()
{
	OnBkBtnCancelScan();
	EndDialog(IDOK);
}

void CDlgInstalledVuls::OnBkBtnOpenDir()
{
	CString strPath;
	GetDownloadPath( strPath );
	ShellExecute(NULL, _T("open"), strPath, NULL, NULL, SW_SHOW);
}

void CDlgInstalledVuls::OnBkBtnCancelScan()
{
	_SafeKillTimer();
	theEngine->CancelScanFixed();
}

void CDlgInstalledVuls::OnShowWindow( BOOL bShow, UINT nStatus )
{
	
}

LRESULT CDlgInstalledVuls::OnInitScan( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if(!m_nTimerID)
		m_nTimerID = SetTimer(TIMER_RUNNING, 200, NULL);
	theEngine->ScanFixed( m_hWnd );
	return 0;
}

static int AppendItemInstalledList( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	CString strTitle;
	strTitle.Format(_T("KB%d(%d)"), pVulItem->nID, pVulItem->nWarnLevel);
	
	int nItem = listCtrl.Append( pVulItem->strPubDate, false );
	listCtrl.AppendSubItem(nItem, strTitle);
	listCtrl.AppendSubItem(nItem, pVulItem->strName);
	listCtrl.AppendSubItem(nItem, _T("更多信息"), SUBITEM_LINK);
	listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);
	return nItem;
}

LRESULT CDlgInstalledVuls::OnDoneScan( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	SetItemVisible(201, FALSE);
	SetItemVisible(300, TRUE);

	const CSimpleArray<LPTUpdateItem>& arr = theEngine->m_pIVulFixed->GetResults();
	
	CString strTitle;
	strTitle.Format(_T("已安装系统漏洞补丁 %d 个"), arr.GetSize() );
	SetItemText(30000, strTitle);
	
	ResetListCtrl(m_wndListCtrlVul);
	for(int i=0; i<arr.GetSize(); ++i)
	{
		LPTUpdateItem pItem = arr[i];
		AppendItemInstalledList( m_wndListCtrlVul, CreateListItem(pItem) );
	}
	return 0;
}

LRESULT CDlgInstalledVuls::OnListLinkClicked( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/ )
{
	ATLTRACE(_T("LinkClicked %d, %d\n"), wParam, lParam);

	T_VulListItemData *pItem = (T_VulListItemData *)m_wndListCtrlVul.GetItemData( wParam );
	if(pItem)
	{
		ShellExecute(NULL, _T("open"), pItem->strWebPage, NULL, NULL, SW_SHOW);
	}

	return 0;
}

LRESULT CDlgInstalledVuls::OnVulFixEventHandle( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	int evt = uMsg - WMH_VULFIX_BASE;
	if(evt==EVulfix_ScanBegin||evt==EVulfix_ScanProgress)
	{
		if(evt==EVulfix_ScanBegin)
		{
			if(wParam==VTYPE_WINDOWS)
				m_nScanState = 0;
			else 
				++m_nScanState;
			m_nTotalItem = lParam;
			m_nCurrentItem = 0;
		}
		else //EVulfix_ScanProgress:
		{
			if(lParam>(m_nCurrentItem+50))
			{
				m_nCurrentItem = lParam;
				int nPos = m_nScanState*100 + (m_nCurrentItem*100)/m_nTotalItem;
				_SetScanProgress(nPos);
			}
		}
	}
	return 0;
}

void CDlgInstalledVuls::_SetScanProgress( int nPos )
{
	SetItemDWordAttribute(112, "value", nPos, TRUE);
}

void CDlgInstalledVuls::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
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
	else if(nChar==_T('2'))
	{
		const CSimpleArray<LPTUpdateItem>& arr = theEngine->m_pIVulFixed->GetResults();
		for(int i=0; i<arr.GetSize(); ++i)
		{
			LPTUpdateItem pItem = arr[i];
			AppendItemInstalledList( m_wndListCtrlVul, CreateListItem(pItem) );
		}
	}
}

void CDlgInstalledVuls::_SafeKillTimer()
{
	if(m_nTimerID>0)
	{
		KillTimer(TIMER_RUNNING);
		m_nTimerID = 0;
	}
}

void CDlgInstalledVuls::OnTimer( UINT_PTR nIDEvent )
{
	if(nIDEvent==TIMER_RUNNING)
	{
		static int nSubImage = 0;
		nSubImage = ++nSubImage%8;
		SetItemIntAttribute(20180, "sub", nSubImage);
	}
}
