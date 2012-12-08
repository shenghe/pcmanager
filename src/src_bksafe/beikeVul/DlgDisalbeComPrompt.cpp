#include "StdAfx.h"
#include "DlgDisalbeComPrompt.h"
#include "BeikeVulfixEngine.h"

CDlgDisalbeComPrompt::CDlgDisalbeComPrompt(void)
{
}

CDlgDisalbeComPrompt::~CDlgDisalbeComPrompt(void)
{
}

LRESULT CDlgDisalbeComPrompt::OnInitDialog( HWND /*hWnd*/, LPARAM /*lParam*/ )
{
	m_wndListCtrlVul.Create( 
		GetViewHWND(), NULL, NULL, 
		WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 
		0, 30010, NULL);
	{
		m_wndListCtrlVul.InsertColumn(0, _T("软件名称"), LVCFMT_LEFT, 250);
		m_wndListCtrlVul.InsertColumn(1, _T("COM 组件类型"), LVCFMT_LEFT, 350);
	}
	
	// 
	for(int i=0; i<m_arrSoftItems.GetSize(); ++i)
	{
		T_VulListItemData *pItem = m_arrSoftItems[i];
		int nItem = m_wndListCtrlVul.Append(pItem->strName, true);
		m_wndListCtrlVul.AppendSubItem(nItem, pItem->strDesc);
		m_wndListCtrlVul.SetItemData(nItem, (DWORD_PTR)pItem);
	}
	return 0;
}

void CDlgDisalbeComPrompt::OnBkBtnClose()
{
	EndDialog(IDCANCEL);
}

void CDlgDisalbeComPrompt::OnBkBtnRepair()
{
	ATLASSERT(theEngine->m_pSoftVulScan);
	bool hasFixed = false;
	for(int i=0; i<m_wndListCtrlVul.GetItemCount(); ++i)
	{
		if(m_wndListCtrlVul.GetCheckState(i))
		{
			T_VulListItemData *pItem = (T_VulListItemData*) m_wndListCtrlVul.GetItemData( i );	
			ATLASSERT(pItem);
			
			theEngine->m_pSoftVulScan->EnableVulCOM( pItem->nID, FALSE );
			
			hasFixed = true;
		}
	}
	if(!hasFixed)
	{
		::MessageBox(m_hWnd, _T("请选择要修复的选项"), NULL, MB_OK);
	}
	else
	{
		EndDialog(IDOK);
	}
}

void CDlgDisalbeComPrompt::SetItems( const CSimpleArray<T_VulListItemData*> &arrItems )
{
	m_arrSoftItems.RemoveAll();
	CopySimpleArray(arrItems, m_arrSoftItems);
}