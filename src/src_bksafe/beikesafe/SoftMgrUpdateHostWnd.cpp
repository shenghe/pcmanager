
#include "stdafx.h"
#include "beikesafe.h"
#include "SoftMgrUpdateHostWnd.h"

CSoftMgrUpdateHostWnd::CSoftMgrUpdateHostWnd()
{
	m_hFont = CreateFont(
		12,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_NORMAL,				   // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		L"宋体");                 // lpszFacename
	m_hWndCheckNotify = NULL;
}

CSoftMgrUpdateHostWnd::~CSoftMgrUpdateHostWnd()
{
}

int CSoftMgrUpdateHostWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CenterWindow();

	_InitCtrl();

	return 0;
}

#define COL_UPDATE_CAPTION_BG		RGB(0xdc,0xee,0xff)

void CSoftMgrUpdateHostWnd::OnPaint(CDCHandle dc)
{
	PAINTSTRUCT ps;
	HDC hDC = ::BeginPaint( m_hWnd, &ps );
	Gdiplus::Graphics grap(hDC);	
	Gdiplus::Image* pImgDotBlue = BkPngPool::Get(IDP_PNG_SOFT_NECESS_POINT);
	Gdiplus::Image* pImgDotGreen = BkPngPool::Get(IDB_SOFTMGR_DOT);
	int nSize = 0;
	if (pImgDotBlue)
		nSize = pImgDotBlue->GetWidth();

	WTL::CRect rcClient;
	GetClientRect(&rcClient);

	CDC dcClient(hDC);
	//dcClient.FillSolidRect(&rcClient, RGB(255, 255, 255));

	HFONT hFontOld = (HFONT)SelectObject( hDC, m_hFont );
	::SetBkMode( hDC, TRANSPARENT );

	::SetTextColor(hDC, RGB(0,0,0));
	if (m_lstIgnore.GetItemCount())
	{
		CRect rcIgnoreLst;
		m_lstIgnore.GetWindowRect(rcIgnoreLst);
		ScreenToClient(&rcIgnoreLst);
		CRect rcIgnoreTitle;
		rcIgnoreTitle = rcIgnoreLst;
		rcIgnoreTitle.bottom = rcIgnoreLst.top;
		rcIgnoreTitle.top = rcIgnoreTitle.bottom - ITEMS_TITLE_HEIGHT;
		dcClient.FillSolidRect(&rcIgnoreTitle, COL_UPDATE_CAPTION_BG);

		if (pImgDotGreen)
		{
			grap.DrawImage(pImgDotBlue, rcIgnoreTitle.left + (ITEMS_TITLE_HEIGHT - nSize)/2, 
				rcIgnoreTitle.top + (ITEMS_TITLE_HEIGHT - nSize)/2, 
				nSize, nSize);
		}

		rcIgnoreTitle.left += (ITEMS_TITLE_HEIGHT/2 + nSize);
		CString strIgnoreTitle;
		strIgnoreTitle.Format(L"推荐升级( %d )，建议您立即升级", m_lstIgnore.GetItemCount());
		::DrawText( hDC, 
			strIgnoreTitle,
			strIgnoreTitle.GetLength(),
			&(rcIgnoreTitle),
			DT_LEFT|DT_VCENTER|DT_SINGLELINE );

		CRect rcProb;
		::DrawText(hDC, L"推荐升级( ", -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
		CRect rcNum;
		rcNum = rcIgnoreTitle;
		rcNum.left += rcProb.Width();
		::SetTextColor(hDC, RGB(228, 16, 15));
		CString strNum;
		strNum.Format(L"%d", m_lstIgnore.GetItemCount());
		::DrawText( hDC, 
			strNum,
			-1,
			&(rcNum),
			DT_LEFT|DT_VCENTER|DT_SINGLELINE );
	}

	::SetTextColor(hDC, RGB(0,0,0));
	if (m_lstFree.GetItemCount())
	{
		CRect rcFreeLst;
		m_lstFree.GetWindowRect(rcFreeLst);
		ScreenToClient(&rcFreeLst);
		CRect rcFreeTitle;
		rcFreeTitle = rcFreeLst;
		rcFreeTitle.bottom = rcFreeLst.top;
		rcFreeTitle.top = rcFreeTitle.bottom - ITEMS_TITLE_HEIGHT;
		dcClient.FillSolidRect(&rcFreeTitle, COL_UPDATE_CAPTION_BG);

		if (pImgDotBlue)
		{
			grap.DrawImage(pImgDotBlue, rcFreeTitle.left + (ITEMS_TITLE_HEIGHT - nSize)/2, 
				rcFreeTitle.top + (ITEMS_TITLE_HEIGHT - nSize)/2, 
				nSize, nSize);
		}

		rcFreeTitle.left += (ITEMS_TITLE_HEIGHT/2 + nSize);
		CString strFreeTitle;
		strFreeTitle.Format(L"可选升级( %d )，以下软件包括测试版，收费版，您可以根据需要选择升级", m_lstFree.GetItemCount());
		::DrawText( hDC, 
			strFreeTitle,
			strFreeTitle.GetLength(),
			&(rcFreeTitle),
			DT_LEFT|DT_VCENTER|DT_SINGLELINE );

		CRect rcProb;
		::DrawText(hDC, L"可选升级( ", -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
		CRect rcNum;
		rcNum = rcFreeTitle;
		rcNum.left += rcProb.Width();
		::SetTextColor(hDC, RGB(228, 16, 15));
		CString strNum;
		strNum.Format(L"%d", m_lstFree.GetItemCount());
		::DrawText( hDC, 
			strNum,
			-1,
			&(rcNum),
			DT_LEFT|DT_VCENTER|DT_SINGLELINE );
	}

	SelectObject( hDC, hFontOld );

	::EndPaint( m_hWnd, &ps );
}

void CSoftMgrUpdateHostWnd::OnClose()
{
	SetMsgHandled(false);
}

void CSoftMgrUpdateHostWnd::OnDestroy()
{
	SetMsgHandled(false);
}

void CSoftMgrUpdateHostWnd::_InitCtrl()
{
	CRect rcWnd;
	GetClientRect(&rcWnd);

	m_lstIgnore.Create(m_hWnd, NULL, NULL,
		WS_CHILD|LVS_REPORT|LVS_NOCOLUMNHEADER|LVS_NOSORTHEADER|LVS_OWNERDRAWFIXED|LVS_SINGLESEL,
		0 , IDC_LST_IGNORE);
	m_lstIgnore.CreateTipCtrl();
	m_lstIgnore.ShowScrollBar(SB_HORZ, FALSE);
	m_lstIgnore.InsertColumn(0, _T(""), LVCFMT_LEFT, 0);
	m_lstIgnore.SetColumnWidth(0, rcWnd.Width());
	m_lstIgnore.ShowWindow(SW_SHOW);

	m_lstFree.Create(m_hWnd, NULL, NULL, 
		WS_CHILD|LVS_REPORT|LVS_NOCOLUMNHEADER|LVS_NOSORTHEADER|LVS_OWNERDRAWFIXED|LVS_SINGLESEL, 
		0 , IDC_LST_FREE);
	m_lstFree.CreateTipCtrl();
	m_lstFree.ShowScrollBar(SB_HORZ, FALSE);
	m_lstFree.InsertColumn(0, _T(""), LVCFMT_LEFT, 0);
	m_lstFree.SetColumnWidth(0, rcWnd.Width());
	m_lstFree.ShowWindow(SW_SHOW);
}

int CSoftMgrUpdateHostWnd::_GetIgnoreLstCheck()
{
	CSoftListItemData* pData = NULL;
	int nCount = 0;
	int nCountCheck = 0;
	int nCheckFree = BST_CHECKED;

	nCount = m_lstIgnore.GetItemCount();
	for (int i = 0; i<nCount; i++)
	{
		pData = m_lstIgnore.GetItemDataEx(i);
		if (pData && pData->m_bCheck)
			nCountCheck++;
	}

	if (nCount == 0)
		nCheckFree = BST_CHECKED;
	else
	{
		if (nCountCheck > 0)
		{
			if (nCount == nCountCheck)
				nCheckFree = BST_CHECKED;
			else if (nCount > nCountCheck)
				nCheckFree = BST_INDETERMINATE;
		}
		else
			nCheckFree = BST_UNCHECKED;
	}

	return nCheckFree;
}

int CSoftMgrUpdateHostWnd::_GetFreeLstCheck()
{
	CSoftListItemData* pData = NULL;
	int nCount = 0;
	int nCountCheck = 0;
	int nCheckFree = BST_CHECKED;

	nCount = m_lstFree.GetItemCount();
	for (int i = 0; i<nCount; i++)
	{
		pData = m_lstFree.GetItemDataEx(i);
		if (pData && pData->m_bCheck)
			nCountCheck++;
	}

	if (nCount == 0)
		nCheckFree = BST_CHECKED;
	else
	{
		if (nCountCheck > 0)
		{
			if (nCount == nCountCheck)
				nCheckFree = BST_CHECKED;
			else if (nCount > nCountCheck)
				nCheckFree = BST_INDETERMINATE;
		}
		else
			nCheckFree = BST_UNCHECKED;
	}

	return nCheckFree;
}

BOOL CSoftMgrUpdateHostWnd::OnEraseBkgnd(CDCHandle dc)
{
	return TRUE;
}

void CSoftMgrUpdateHostWnd::OnSize(UINT nType, CSize size)
{
	if (!m_lstIgnore.IsWindow() || !m_lstFree.IsWindow())
		return ;

	CRect rcClient;
	GetClientRect(&rcClient);

	int nTop = 0;

	CRect rcLstIgnore(0, 
		nTop + ITEMS_TITLE_HEIGHT, 
		rcClient.Width(), 
		nTop + ITEMS_TITLE_HEIGHT + m_lstIgnore.GetItemCount()*ITEM_HEIGHT);
	m_lstIgnore.SetColumnWidth(0, rcClient.Width());
	m_lstIgnore.Invalidate();
	m_lstIgnore.MoveWindow(&rcLstIgnore);

	nTop = (m_lstIgnore.GetItemCount()>0 ? ITEMS_TITLE_HEIGHT : 0) + m_lstIgnore.GetItemCount()*ITEM_HEIGHT;
	CRect rcLstFree(0, 
		nTop + ITEMS_TITLE_HEIGHT, 
		rcClient.Width(), 
		nTop + ITEMS_TITLE_HEIGHT + m_lstFree.GetItemCount()*ITEM_HEIGHT);
	m_lstFree.SetColumnWidth(0, rcClient.Width());
	m_lstFree.Invalidate();
	m_lstFree.MoveWindow(&rcLstFree);
}

void CSoftMgrUpdateHostWnd::OnSetFocus(HWND hWnd) 
{
	GetParent().SetFocus();
	return;
}

LRESULT CSoftMgrUpdateHostWnd::OnClickListItemNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hWndList = (HWND)wParam;
	LVITEM htiSel = {0};

	if (hWndList == m_lstIgnore.m_hWnd)
	{
		if(m_lstFree.GetSelectedItem(&htiSel))
		{
			if (htiSel.iItem != -1)
			{
				m_lstFree.SetItemState(htiSel.iItem, 0,  LVIS_SELECTED|LVIS_FOCUSED);
			}
		}
	}
	else if (hWndList == m_lstFree.m_hWnd)
	{
		if(m_lstIgnore.GetSelectedItem(&htiSel))
		{
			if (htiSel.iItem != -1)
			{
				m_lstIgnore.SetItemState(htiSel.iItem, 0,  LVIS_SELECTED|LVIS_FOCUSED);
			}
		}
	}

	return TRUE;
}

LRESULT CSoftMgrUpdateHostWnd::OnCheckListItemNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int nCheckIgnore = _GetIgnoreLstCheck();
	int nCheckFree = _GetFreeLstCheck();

	if (!m_hWndCheckNotify)
		m_hWndCheckNotify = GetParent().GetParent();

	if (nCheckFree == BST_CHECKED)
		::PostMessage(m_hWndCheckNotify, WM_USER_SET_CHECK_LIST, USCT_FREE, BST_CHECKED);
	else
		::PostMessage(m_hWndCheckNotify, WM_USER_SET_CHECK_LIST, USCT_FREE, BST_UNCHECKED);

	if (nCheckIgnore == BST_CHECKED && nCheckFree == BST_CHECKED)
		::PostMessage(m_hWndCheckNotify, WM_USER_SET_CHECK_LIST, USCT_ALL, BST_CHECKED);
	else
		::PostMessage(m_hWndCheckNotify, WM_USER_SET_CHECK_LIST, USCT_ALL, BST_UNCHECKED);

	return TRUE;
}

LRESULT CSoftMgrUpdateHostWnd::OnSetCheckList(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch(wParam)
	{
	case USCT_ALL:
		{
			CSoftListItemData* pData = NULL;
			int nCount = 0;
			
			if (lParam == BST_CHECKED)
			{
				nCount = m_lstIgnore.GetItemCount();
				for (int i = 0; i<nCount; i++)
				{
					pData = m_lstIgnore.GetItemDataEx(i);
					if (pData)
					{
						pData->m_bCheck = TRUE;
						m_lstIgnore.SetItemData(i, (DWORD_PTR)pData);
					}
				}

				nCount = m_lstFree.GetItemCount();
				for (int i = 0; i<nCount; i++)
				{
					pData = m_lstFree.GetItemDataEx(i);
					if (pData)
					{
						pData->m_bCheck = TRUE;
						m_lstFree.SetItemData(i, (DWORD_PTR)pData);
					}
				}
			}
			else if (lParam == BST_UNCHECKED)
			{
				nCount = m_lstIgnore.GetItemCount();
				for (int i = 0; i<nCount; i++)
				{
					pData = m_lstIgnore.GetItemDataEx(i);
					if (pData)
					{
						pData->m_bCheck = FALSE;
						m_lstIgnore.SetItemData(i, (DWORD_PTR)pData);
					}
				}

				nCount = m_lstFree.GetItemCount();
				for (int i = 0; i<nCount; i++)
				{
					pData = m_lstFree.GetItemDataEx(i);
					if (pData)
					{
						pData->m_bCheck = FALSE;
						m_lstFree.SetItemData(i, (DWORD_PTR)pData);
					}
				}
			}
			m_lstIgnore.Invalidate();
			m_lstFree.Invalidate();
		}
		break;
	case USCT_FREE:
		{
			CSoftListItemData* pData = NULL;
			int nCount = 0;

			if (lParam == BST_CHECKED)
			{
				nCount = m_lstFree.GetItemCount();
				for (int i = 0; i<nCount; i++)
				{
					pData = m_lstFree.GetItemDataEx(i);
					if (pData)
					{
						pData->m_bCheck = TRUE;
						m_lstFree.SetItemData(i, (DWORD_PTR)pData);
					}
				}
			}
			else if (lParam == BST_UNCHECKED)
			{
				nCount = m_lstFree.GetItemCount();
				for (int i = 0; i<nCount; i++)
				{
					pData = m_lstFree.GetItemDataEx(i);
					if (pData)
					{
						pData->m_bCheck = FALSE;
						m_lstFree.SetItemData(i, (DWORD_PTR)pData);
					}
				}
			}
			m_lstFree.Invalidate();
		}
		break;
	}
	return TRUE;
}

LRESULT CSoftMgrUpdateHostWnd::OnGetCheckList(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == 7)
	{
		int nCheckIgnore = _GetIgnoreLstCheck();
		int nCheckFree = _GetFreeLstCheck();

		if (nCheckIgnore == BST_CHECKED && nCheckFree == BST_CHECKED)
			return BST_CHECKED;
	}

	return BST_UNCHECKED;
}

int CSoftMgrUpdateHostWnd::GetWindowMaxHeight()
{
	int nResult = 0;

	if (m_lstIgnore.IsWindow())
		nResult += (m_lstIgnore.GetItemCount()>0?ITEMS_TITLE_HEIGHT:0) + m_lstIgnore.GetItemCount()*ITEM_HEIGHT;

	if (m_lstFree.IsWindow())
		nResult += (m_lstFree.GetItemCount()>0?ITEMS_TITLE_HEIGHT:0) + m_lstFree.GetItemCount()*ITEM_HEIGHT;

	return nResult;
}

void CSoftMgrUpdateHostWnd::SetCheckNotifyHwnd(HWND hWnd)
{
	m_hWndCheckNotify = hWnd;
}

void CSoftMgrUpdateHostWnd::SetClickLinkCallback( ISoftMgrUpdateCallback* opCB )
{
	if (m_lstIgnore.IsWindow())
		m_lstIgnore.SetClickLinkCallback(opCB);
	if (m_lstFree.IsWindow())
		m_lstFree.SetClickLinkCallback(opCB);
}

void CSoftMgrUpdateHostWnd::AddItem(DWORD_PTR dwItemData)
{
	CSoftListItemData *pData = (CSoftListItemData *)dwItemData;
	int nType = ST_INVALID;
	if (pData->m_bCharge == FALSE)
	{
		nType = ST_FREE;
		if ((pData->m_attri&SA_Beta) == SA_Beta)
			nType = ST_BETA;
	}
	else
		nType = ST_PAY;

	if (pData->m_bIgnore)
		nType = ST_IGNORE;

	if ((nType == ST_BETA || nType == ST_PAY) && (pData->m_attri&SA_Major) != SA_Major)
	{
		pData->m_dwIDUpdate = m_lstFree.GetItemCount();

		m_lstFree.InsertItem(m_lstFree.GetItemCount(), _T(""));
		m_lstFree.SetItemDataEx(m_lstFree.GetItemCount(), (DWORD_PTR)pData);
		m_lstFree.SetPaintItem(TRUE);
	}
	else
	{
		pData->m_dwIDUpdate = m_lstIgnore.GetItemCount();

		m_lstIgnore.InsertItem(m_lstIgnore.GetItemCount(), _T(""));
		m_lstIgnore.SetItemDataEx(m_lstIgnore.GetItemCount(), (DWORD_PTR)pData);
		m_lstIgnore.SetPaintItem(TRUE);
	}
}

void CSoftMgrUpdateHostWnd::UpdateCtrlWindow()
{
	CRect rcWnd;
	GetClientRect(&rcWnd);

	int nTop = 0;
	CRect rcLstIgnore(0, 
		nTop + ITEMS_TITLE_HEIGHT, 
		rcWnd.Width(), 
		nTop + ITEMS_TITLE_HEIGHT + m_lstIgnore.GetItemCount()*ITEM_HEIGHT);
	m_lstIgnore.MoveWindow(&rcLstIgnore);

	nTop = (m_lstIgnore.GetItemCount()>0 ? ITEMS_TITLE_HEIGHT : 0) + m_lstIgnore.GetItemCount()*ITEM_HEIGHT;
	CRect rcLstFree(0, 
		nTop + ITEMS_TITLE_HEIGHT, 
		rcWnd.Width(), 
		nTop + ITEMS_TITLE_HEIGHT + m_lstFree.GetItemCount()*ITEM_HEIGHT);
	m_lstFree.MoveWindow(&rcLstFree);

	Invalidate();
}

CSoftListItemData* CSoftMgrUpdateHostWnd::GetItemDataEx(DWORD dwIndex, HWND hWndList)
{
	if (m_lstIgnore && m_lstIgnore.IsWindow() && m_lstIgnore.m_hWnd == hWndList)
	{
		return m_lstIgnore.GetItemDataEx(dwIndex);
	}
	else if (m_lstFree && m_lstFree.IsWindow() && m_lstFree.m_hWnd == hWndList)
	{
		return m_lstFree.GetItemDataEx(dwIndex);
	}

	return NULL;
}	

void CSoftMgrUpdateHostWnd::RefreshRight(int nIndex, HWND hWndList)
{
	if (m_lstIgnore && m_lstIgnore.IsWindow() && m_lstIgnore.m_hWnd == hWndList)
	{
		m_lstIgnore.RefreshRight(nIndex);
	}
	else if (m_lstFree && m_lstFree.IsWindow() && m_lstFree.m_hWnd == hWndList)
	{
		m_lstFree.RefreshRight(nIndex);
	}
}

void CSoftMgrUpdateHostWnd::RefreshRight(LPCTSTR strSoftID)
{
	int nCount = 0;

	nCount = m_lstIgnore.GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		CSoftListItemData *pItemData = m_lstIgnore.GetItemDataEx(i);
		if (pItemData != NULL && pItemData->m_strSoftID == strSoftID)
		{
			m_lstIgnore.RefreshRight(i);
			return;
		}
	}

	nCount = m_lstFree.GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		CSoftListItemData *pItemData = m_lstFree.GetItemDataEx(i);
		if (pItemData != NULL && pItemData->m_strSoftID == strSoftID)
		{
			m_lstFree.RefreshRight(i);
			return;
		}
	}
}

void CSoftMgrUpdateHostWnd::RefreshIcon(LPCTSTR strSoftID)
{
	CSoftListItemData* pData = NULL;
	int nCount = 0;

	nCount = m_lstIgnore.GetItemCount();
	for (int i = 0; i<nCount; i++)
	{
		pData = m_lstIgnore.GetItemDataEx(i);
		if (pData && pData->m_strSoftID == strSoftID)
		{
			m_lstIgnore.RefreshIcon(i);
			goto Exit1;
		}
	}

	nCount = m_lstFree.GetItemCount();
	for (int i = 0; i<nCount; i++)
	{
		pData = m_lstFree.GetItemDataEx(i);
		if (pData && pData->m_strSoftID == strSoftID)
		{
			m_lstFree.RefreshIcon(i);
			goto Exit1;
		}
	}

Exit1:
	return ;
}

void CSoftMgrUpdateHostWnd::UpdateSelectSoft()
{
	CSoftListItemData* pData = NULL;
	int nCount = 0;
	BOOL bUpdateKsafe = FALSE;
	int nKsafeIndex = -1;
	HWND hWnd = NULL;

	nCount = m_lstIgnore.GetItemCount();
	for (int i = 0; i<nCount; i++)
	{
		pData = m_lstIgnore.GetItemDataEx(i);
		if (pData && pData->m_bCheck)
		{
			if( IsKsafe( pData ) )
			{
				bUpdateKsafe = TRUE;
				nKsafeIndex = i;
				hWnd = m_lstIgnore.m_hWnd;
			}
			else
			{
				ISoftMgrUpdateCallback* opCB = m_lstIgnore.GetClickLinkCallback();
				if (opCB)
					opCB->OnUpdateDownLoad(i, m_lstIgnore.m_hWnd);
			}
			
			
		}
	}

	nCount = m_lstFree.GetItemCount();
	for (int i = 0; i<nCount; i++)
	{
		pData = m_lstFree.GetItemDataEx(i);
		if (pData && pData->m_bCheck)
		{
			if( IsKsafe( pData) )
			{
				bUpdateKsafe = TRUE;
				nKsafeIndex = i;
				hWnd = m_lstIgnore.m_hWnd;
			}
			else
			{
				ISoftMgrUpdateCallback* opCB = m_lstFree.GetClickLinkCallback();
				if (opCB)
					opCB->OnUpdateDownLoad(i, m_lstFree.m_hWnd);
			}
		}
	}

	//界面上延迟安装下载卫士
	if( bUpdateKsafe )
	{
		if( hWnd == m_lstIgnore.m_hWnd )
		{
			ISoftMgrUpdateCallback* opCB = m_lstIgnore.GetClickLinkCallback();
			if (opCB)
				opCB->OnUpdateDownLoad( nKsafeIndex, m_lstIgnore.m_hWnd);
		}
		else
		{
			ISoftMgrUpdateCallback* opCB = m_lstFree.GetClickLinkCallback();
			if (opCB)
				opCB->OnUpdateDownLoad( nKsafeIndex, m_lstFree.m_hWnd);
		}
	}
}

BOOL CSoftMgrUpdateHostWnd::IsKsafe( CSoftListItemData* pData )
{
	if( pData->m_strSoftID == TEXT("60000411")
		|| pData->m_strName == TEXT("金山卫士") )
	{
		return TRUE;
	}

	return FALSE;
}

void CSoftMgrUpdateHostWnd::DeleteAllItems()
{
	m_lstIgnore.DeleteAllItemsEx();
	m_lstFree.DeleteAllItemsEx();
}

int CSoftMgrUpdateHostWnd::GetCheck()
{
	int nCheckIgnore = _GetIgnoreLstCheck();
	int nCheckFree = _GetFreeLstCheck();

	if (m_lstIgnore.GetItemCount() == 0)
		nCheckIgnore = BST_UNCHECKED;
	if (m_lstFree.GetItemCount() == 0)
		nCheckFree = BST_UNCHECKED;

	if (nCheckIgnore == BST_CHECKED && nCheckFree == BST_CHECKED)
		return BST_CHECKED;
	else if (nCheckIgnore == BST_UNCHECKED && nCheckFree == BST_UNCHECKED)
		return BST_UNCHECKED;
	else
		return BST_INDETERMINATE;
}

int CSoftMgrUpdateHostWnd::GetUpdateCount(int& nRecommend, int& nChoice)
{
	CSoftListItemData* pData = NULL;
	int nCount = 0;

	nRecommend = 0;
	nCount = m_lstIgnore.GetItemCount();
	for (int i = 0; i<nCount; i++)
	{
		pData = m_lstIgnore.GetItemDataEx(i);
		if (pData && pData->m_bUpdate)
			nRecommend++;
	}

	nChoice = 0;
	nCount = m_lstFree.GetItemCount();
	for (int i = 0; i<nCount; i++)
	{
		pData = m_lstFree.GetItemDataEx(i);
		if (pData && pData->m_bUpdate)
			nChoice++;
	}

	return nRecommend + nChoice;
}

void CSoftMgrUpdateHostWnd::UpdateSoft(LPCTSTR strSoftID)
{
	CSoftListItemData* pData = NULL;
	int nCount = 0;

	nCount = m_lstIgnore.GetItemCount();
	for (int i = 0; i<nCount; i++)
	{
		pData = m_lstIgnore.GetItemDataEx(i);
		if (pData && pData->m_strSoftID == strSoftID)
		{
			m_lstIgnore.UpdateSoft(i, m_lstIgnore.m_hWnd);
			goto Exit1;
		}
	}

	nCount = m_lstFree.GetItemCount();
	for (int i = 0; i<nCount; i++)
	{
		pData = m_lstFree.GetItemDataEx(i);
		if (pData && pData->m_strSoftID == strSoftID)
		{
			m_lstFree.UpdateSoft(i, m_lstFree.m_hWnd);
			goto Exit1;
		}
	}

Exit1:
	return ;
}