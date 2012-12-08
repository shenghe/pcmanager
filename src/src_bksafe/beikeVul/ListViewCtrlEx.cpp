#include "stdafx.h"
#include "ListViewCtrlEx.h"

HWND CListViewCtrlEx::Create( HWND hWndParent, _U_RECT rect /*= NULL*/, LPCTSTR szWindowName /*= NULL*/, DWORD dwStyle /*= 0*/, DWORD dwExStyle /*= 0*/, _U_MENUorID MenuOrID /*= 0U*/, LPVOID lpCreateParam /*= NULL*/ )
{
	HWND hWnd = _super::Create(hWndParent, rect, szWindowName, dwStyle|LVS_OWNERDRAWFIXED, dwExStyle, MenuOrID, lpCreateParam);
	_super::SetExtendedListViewStyle( LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT|_super::GetExtendedListViewStyle() );
	return hWnd;
}

BOOL CListViewCtrlEx::GetCheckState( int nIndex ) const
{
	const TListItem *pItem = _GetItemData(nIndex);
	if(pItem && pItem->checkbox)
		return _super::GetCheckState( nIndex );
	return FALSE;
}

BOOL CListViewCtrlEx::SetCheckState( int nItem, BOOL bCheck )
{
	TListItem *pItem = _GetItemData( nItem );
	if(pItem && pItem->checkbox)
		return _super::SetCheckState( nItem, bCheck );
	return FALSE;
}

void CListViewCtrlEx::SetObserverWindow( HWND hWnd )
{
	m_hWndObserver = hWnd;
}

int CListViewCtrlEx::Append( LPCTSTR strItem, bool bCheckBox/*=false*/, E_SubItemType itemType/*=SUBITEM_TEXT*/ )
{
	int nItem = _super::AddItem(_super::GetItemCount(), 0, _T(""));
	TListItem item;
	item.checkbox = bCheckBox;
	item.subItems.Add( TListSubItem(strItem, itemType) );
	m_arrItems.Add( item );
	return nItem;
}

int CListViewCtrlEx::AppendSubItem( int nItem, LPCTSTR strItem, E_SubItemType itemType/*=SUBITEM_TEXT*/ )
{
	BOOL bValid = nItem>=0 && nItem<m_arrItems.GetSize();
	ATLASSERT(bValid);
	if( !bValid )
		return -1;
	m_arrItems[nItem].subItems.Add( TListSubItem(strItem, itemType) );
	return m_arrItems[nItem].subItems.GetSize();
}

bool CListViewCtrlEx::SetSubItem( int nItem, int nSubItem, LPCTSTR lpszItem, E_SubItemType itemType/*=SUBITEM_TEXT*/, BOOL bRedraw )
{
	if(nItem>=0 && nItem<m_arrItems.GetSize())
	{
		CSimpleArray<TListSubItem> &subItems = m_arrItems[nItem].subItems;
		if(nSubItem>=0 && nSubItem<subItems.GetSize())
		{
			subItems[nSubItem].type = itemType;
			subItems[nSubItem].str = lpszItem;				
			if(bRedraw)
				_InvalidateRect(nItem, nSubItem);
			return true;
		}
	}
	return false;
}

bool CListViewCtrlEx::SetSubItemColor( int nItem, int nSubItem, COLORREF clr, BOOL bRedraw  )
{
	if(nItem>=0 && nItem<m_arrItems.GetSize())
	{
		CSimpleArray<TListSubItem> &subItems = m_arrItems[nItem].subItems;
		if(nSubItem>=0 && nSubItem<subItems.GetSize())
		{
			subItems[nSubItem].clr = clr;
			if(bRedraw)
				_InvalidateRect(nItem, nSubItem);
			return true;
		}
	}
	return false;
}

bool CListViewCtrlEx::SetItemCheck( int nItem, bool bUseCheck )
{
	TListItem *pItem = _GetItemData(nItem);
	if(pItem)
	{
		pItem->checkbox = bUseCheck;
		if(!bUseCheck)
			_super::SetCheckState( nItem, FALSE );
	}
	return pItem!=NULL;
}

BOOL CListViewCtrlEx::DeleteItem( int nItem )
{
	m_arrItems.RemoveAt(nItem);
	return _super::DeleteItem( nItem );
}

VOID CListViewCtrlEx::DeleteItem( LPDELETEITEMSTRUCT )
{

}
BOOL CListViewCtrlEx::DeleteAllItems()
{
	m_arrItems.RemoveAll();
	return _super::DeleteAllItems();
}

LRESULT CListViewCtrlEx::OnClick( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
	bHandled = FALSE;
	
	LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) pnmh;
	CRect	rcItem;
	if( _PtInSubItemCheckBox(lpnmitem->ptAction, lpnmitem->iItem) )
	{
		_super::SetCheckState(lpnmitem->iItem, !_super::GetCheckState(lpnmitem->iItem) );
		InvalidateRect( rcItem );
		return 0;
	}

	if(_PtInSubItemLink(lpnmitem->ptAction, lpnmitem->iItem, lpnmitem->iSubItem) )
		_FireEvent(WMH_LISTEX_LINK, lpnmitem->iItem, lpnmitem->iSubItem);

	return 0;
}

LRESULT CListViewCtrlEx::OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
{
	POINT	pts;
	CRect	rcItem;

	bMsgHandled = FALSE;
	pts.x		= GET_X_LPARAM(lParam);
	pts.y		= GET_Y_LPARAM(lParam);

	LVHITTESTINFO hti;
	hti.pt = pts;
	SubItemHitTest( &hti );
	if( m_nHoverItem!=hti.iItem )
	{
		m_nHoverItem = hti.iItem;
		_FireEvent(WMH_LISTEX_HOVERCHANGED, m_nHoverItem);
	}

	bool bInLink = _PtInSubItemLink(pts, hti.iItem, hti.iSubItem);
	::SetCursor(::LoadCursor(NULL, bInLink?IDC_HAND:IDC_ARROW));
	return 0;
}

void CListViewCtrlEx::DrawItem( LPDRAWITEMSTRUCT lpdis )
{
	DWORD nId = lpdis->CtlID;
	int nItem = lpdis->itemID;

	const TListItem *pItem = _GetItemData( nItem );
	if(!pItem)
		return; 

	CDCHandle dc;
	dc.Attach(lpdis->hDC);

	BOOL bSelect = FALSE ;
	if ((lpdis->itemAction | ODA_SELECT) &&
		(lpdis->itemState & ODS_SELECTED))
	{
		bSelect = TRUE ;
	}

	if ( bSelect )
		dc.FillSolidRect( &lpdis->rcItem, RGB(216,241,212));
	else
		dc.FillSolidRect( &lpdis->rcItem, RGB(255,255,255));

	// draw check box 
	if(pItem->checkbox)
		_DrawCheckBox(dc, lpdis->rcItem, _super::GetCheckState(nItem));

	COLORREF oldClr = dc.GetTextColor();
	for(int i=0; i<pItem->subItems.GetSize(); ++i)
	{
		RECT rcSubItem;
		GetSubItemRect(nItem, i, LVIR_LABEL, &rcSubItem);

		if(i==0 && !pItem->checkbox)
		{
			rcSubItem.left -= 19;
		}

		rcSubItem.right -= 3;
		const TListSubItem &subItem = pItem->subItems[i];
		if(subItem.type==SUBITEM_LINK)
		{
			HFONT	hOldFont = dc.SelectFont(m_fontLink);
			dc.SetTextColor(RGB(0,0,200));
			dc.DrawText( subItem.str, -1, &rcSubItem, DT_SINGLELINE|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS);
			dc.SelectFont(hOldFont);
		}
		else
		{
			dc.SetTextColor( subItem.clr );
			dc.DrawText( subItem.str, -1, &rcSubItem, DT_SINGLELINE|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS);
		}
	}
	dc.SetTextColor(oldClr);	
	dc.Detach();
}

const CListViewCtrlEx::TListSubItem * CListViewCtrlEx::_GetSubItemData( int nItem, int nSubItem )
{
	if(nItem>=0 && nItem<m_arrItems.GetSize())
	{
		const CSimpleArray<TListSubItem> &subItems = m_arrItems[nItem].subItems;
		if(nSubItem>=0 && nSubItem<subItems.GetSize())
		{
			return &subItems[nSubItem];
		}
	}
	return NULL;
}

const CListViewCtrlEx::TListItem * CListViewCtrlEx::_GetItemData( int nItem ) const
{
	if(nItem>=0 && nItem<m_arrItems.GetSize())
		return &m_arrItems[nItem];
	return NULL;
}

CListViewCtrlEx::TListItem * CListViewCtrlEx::_GetItemData( int nItem )
{
	if(nItem>=0 && nItem<m_arrItems.GetSize())
		return &m_arrItems[nItem];
	return NULL;
}

BOOL CListViewCtrlEx::_PtInSubItemCheckBox( const POINT& pt, int nItem )
{
	RECT rcItem = {0};
	if( nItem>=0 && nItem<_super::GetItemCount() && GetItemRect(nItem, &rcItem, LVIR_BOUNDS) )
	{
		TListItem *pItem = _GetItemData(nItem);
		ATLASSERT(pItem);

		// 
		if(pItem->checkbox)
		{
			RECT rcCheckBox = _GetRectCheckBox( rcItem );
			return PtInRect(&rcCheckBox, pt);
		}
	}
	return FALSE;
}

BOOL CListViewCtrlEx::_GetSubItemLinkRect( int nItem, int nSubItem, LPCTSTR szText, RECT &rc )
{
	_super::GetSubItemRect( nItem, nSubItem, LVIR_LABEL, &rc );
	HDC hDC = GetWindowDC(); 
	if(!hDC)
		return FALSE;

	CDC dc;
	dc.Attach( hDC );
	HFONT hOldFont = dc.SelectFont(m_fontLink);
	dc.DrawText( szText, -1, &rc, DT_CALCRECT|DT_SINGLELINE|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS );
	dc.SelectFont(hOldFont);
	dc.Detach();
	ReleaseDC( hDC );
	return TRUE;
}

bool CListViewCtrlEx::_PtInSubItemLink( const POINT &pt, int nItem, int nSubItem )
{
	if( nItem>=0 && nItem<_super::GetItemCount() )
	{
		TListItem *pItem = _GetItemData(nItem);
		ATLASSERT(pItem);
		if(nSubItem>=0 && nSubItem<pItem->subItems.GetSize())
		{
			const TListSubItem &subItem = pItem->subItems[nSubItem];
			if( subItem.type==SUBITEM_LINK )
			{
				// Check if in LINK Rect 
				RECT rcLink = {0};
				return _GetSubItemLinkRect(nItem, nSubItem, subItem.str, rcLink) && PtInRect(&rcLink, pt);
			}
		}
	}
	return false;
}

int CListViewCtrlEx::_DrawCheckBox( CDCHandle &dc, RECT &rcItem, BOOL bChecked )
{
	if(rcItem.left>-16)
	{
		CDC	dcTmp;
		dcTmp.CreateCompatibleDC(dc);
		HBITMAP hBmpOld	= dcTmp.SelectBitmap(m_bitmapCheck);

		RECT rcCheckBox = _GetRectCheckBox( rcItem );
		if ( bChecked )
			dc.BitBlt( rcCheckBox.left, rcCheckBox.top, 13, 13, dcTmp, 0,0,SRCCOPY);
		else
			dc.BitBlt( rcCheckBox.left, rcCheckBox.top, 13, 13, dcTmp, 13,0,SRCCOPY);

		dcTmp.SelectBitmap(hBmpOld);
	}
	return 3 + 13 + 3;
}

RECT CListViewCtrlEx::_GetRectCheckBox( RECT &rcItem )
{
	INT nHeight = rcItem.bottom - rcItem.top;
	int nTop = rcItem.top + (nHeight-13)/2;
	RECT rcCheckBox = {rcItem.left+3, nTop, rcItem.left+3+13, nTop+13};
	return rcCheckBox;
}

void CListViewCtrlEx::_InvalidateRect( int nItem, int nSubItem )
{
	RECT rcItem = {0};
	_super::GetItemRect(nItem, &rcItem, LVIR_BOUNDS);
	//_super::GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, &rcItem);
	InvalidateRect( &rcItem );
}

BOOL CListViewCtrlEx::_FireEvent( UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/ )
{
	if(m_hWndObserver)
		return ::PostMessage(m_hWndObserver, message, wParam, lParam);
	else
		return GetParent().PostMessage(message, wParam, lParam);
}