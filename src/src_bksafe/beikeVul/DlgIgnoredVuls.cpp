#include "StdAfx.h"
#include "DlgIgnoredVuls.h"
#include "BeikeVulfixEngine.h"

static int AppendItemIgnoredList( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	CString strTitle;
	strTitle.Format(_T("KB%d"), pVulItem->nID);
	
	int nItem = listCtrl.Append( GetLevelDesc(pVulItem->nWarnLevel), true );
	listCtrl.AppendSubItem(nItem, pVulItem->strPubDate);
	listCtrl.AppendSubItem(nItem, strTitle);
	listCtrl.AppendSubItem(nItem, pVulItem->strName);
	listCtrl.AppendSubItem(nItem, _T("更多信息"), SUBITEM_LINK);
	listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);
	return nItem;
}

CDlgIgnoredVuls::CDlgIgnoredVuls(void)
{
	m_bDirty = FALSE;
}

CDlgIgnoredVuls::~CDlgIgnoredVuls(void)
{
}

LRESULT CDlgIgnoredVuls::OnInitDialog( HWND /*hWnd*/, LPARAM /*lParam*/ )
{
	m_wndListCtrlVul.Create( 
		GetViewHWND(), NULL, NULL, 
		WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL , 
		0, 30010, NULL);
	{
		m_wndListCtrlVul.InsertColumn(0, _T("选择"), LVCFMT_LEFT, 80);
		m_wndListCtrlVul.InsertColumn(1, _T("发布时间"), LVCFMT_LEFT, 100);
		m_wndListCtrlVul.InsertColumn(2, _T("补丁名称"), LVCFMT_LEFT, 100);
		m_wndListCtrlVul.InsertColumn(3, _T("补丁描述"), LVCFMT_LEFT, 250);
		m_wndListCtrlVul.InsertColumn(4, _T("更多信息"), LVCFMT_LEFT, 100);
	}
	PostMessage(WMH_INIT_SCAN, 0, 0);
	return 0;
}

void CDlgIgnoredVuls::OnBkBtnClose()
{
	EndDialog(m_bDirty ? IDOK : IDCANCEL);
}

void CDlgIgnoredVuls::OnShowWindow( BOOL bShow, UINT nStatus )
{
	
}

void CDlgIgnoredVuls::OnBkBtnSelectAll()
{
	SetListCheckedAll(m_wndListCtrlVul, true);
}

void CDlgIgnoredVuls::OnBkBtnSelectNone()
{
	SetListCheckedAll(m_wndListCtrlVul, false);
}

void CDlgIgnoredVuls::OnBkBtnCancelIgnore()
{
	CSimpleArray<int> arr;
	if(GetListCheckedItems(m_wndListCtrlVul, arr))
	{
		m_bDirty = TRUE;
		theEngine->IgnoreVuls(arr, false);
		ListDeleteItems(m_wndListCtrlVul, arr);
	}
	_UpdateTitle();
}

void CDlgIgnoredVuls::OnBkBtnCancelScan()
{
	theEngine->CancelScanIgnored();
	SetItemVisible(201, FALSE);
	SetItemVisible(300, TRUE);
}

void CDlgIgnoredVuls::OnBkBtnScan()
{
	theEngine->ScanIgnored( m_hWnd );
	SetItemVisible(201, TRUE);
	SetItemVisible(300, FALSE);
}

LRESULT CDlgIgnoredVuls::OnInitScan( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	OnBkBtnScan();
	return 0;
}

LRESULT CDlgIgnoredVuls::OnDoneScan( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	SetItemVisible(201, FALSE);
	SetItemVisible(300, TRUE);
	
	int count = 0;
	const CSimpleArray<LPTUpdateItem>& arr = theEngine->m_pVulScanIgnored->GetResults();
	ResetListCtrl(m_wndListCtrlVul);
	for(int i=0; i<arr.GetSize(); ++i)
	{
		LPTUpdateItem pItem = arr[i];
		if(pItem->isIgnored)
		{
			++count;
			AppendItemIgnoredList( m_wndListCtrlVul, CreateListItem(pItem) );
		}
	}
	
	_UpdateTitle();

	return 0;
}

LRESULT CDlgIgnoredVuls::OnVulFixEventHandle( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
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

void CDlgIgnoredVuls::_SetScanProgress( int nPos )
{
	SetItemDWordAttribute(112, "value", nPos, TRUE);
}

void CDlgIgnoredVuls::_UpdateTitle()
{
	CString strTitle;
	strTitle.Format(_T("找到 %d 个被忽略的补丁。您可以取消忽略，之后它们将可以被重新安装"), m_wndListCtrlVul.GetItemCount() );
	SetItemText(30000, strTitle);
}
