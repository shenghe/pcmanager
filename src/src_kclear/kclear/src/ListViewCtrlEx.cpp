#include "stdafx.h"
#include "ListViewCtrlEx.h"
#include <algorithm>

#define COLOR_LIST_LINK				RGB(40,100,165)	
#define LEFT_MARGIN_TEXT_COLUMN		2

void CreateBitmapMask( CBitmap &pBmpSource, CBitmap &pBmpMask, COLORREF clrpTransColor, int iTransPixelX, int iTransPixelY)
{
	BITMAP bm;
	
	// Get the dimensions of the source bitmap
	pBmpSource.GetBitmap(&bm);

	// Create the mask bitmap
	//pBmpMask.DeleteObject();
	pBmpMask.CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL);
	
	// We will need two DCs to work with. One to hold the Image
	// (the source), and one to hold the mask (destination).
	// When blitting onto a monochrome bitmap from a color, pixels
	// in the source color bitmap that are equal to the background
	// color are blitted as white. All the remaining pixels are
	// blitted as black.

	CDC hdcSrc, hdcDst;
	
	hdcSrc.CreateCompatibleDC(NULL);
	hdcDst.CreateCompatibleDC(NULL);

	// Load the bitmaps into memory DC
	HBITMAP hbmSrcT = hdcSrc.SelectBitmap(pBmpSource);
	HBITMAP hbmDstT = hdcDst.SelectBitmap(pBmpMask);
	
	// Dynamically get the transparent color
	COLORREF clrTrans;
	if (clrpTransColor == NULL)
	{
		// User did not specify trans color so get it from bmp
		clrTrans = hdcSrc.GetPixel(iTransPixelX, iTransPixelY);
	}
	else
	{
		clrTrans = clrpTransColor;
	}
	
	// Change the background to trans color
	COLORREF clrSaveBk  = hdcSrc.SetBkColor(clrTrans);

	// This call sets up the mask bitmap.
	hdcDst.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);

	// Now, we need to paint onto the original image, making
	// sure that the "transparent" area is set to black. What
	// we do is AND the monochrome image onto the color Image
	// first. When blitting from mono to color, the monochrome
	// pixel is first transformed as follows:
	// if  1 (black) it is mapped to the color set by SetTextColor().
	// if  0 (white) is is mapped to the color set by SetBkColor().
	// Only then is the raster operation performed.

	COLORREF clrSaveDstText = hdcSrc.SetTextColor(RGB(255,255,255));
	hdcSrc.SetBkColor(RGB(0,0,0));

	hdcSrc.BitBlt(0,0,bm.bmWidth, bm.bmHeight, hdcDst,0,0,SRCAND);

	// Clean up by deselecting any objects, and delete the
	// DC's.
	hdcDst.SetTextColor(clrSaveDstText);

	hdcSrc.SetBkColor(clrSaveBk);
	hdcSrc.SelectBitmap(hbmSrcT);
	hdcDst.SelectBitmap(hbmDstT);

	hdcSrc.DeleteDC();
	hdcDst.DeleteDC();
}

CHeaderCtrlEx::CHeaderCtrlEx()
{
	m_nHeight = -1;
	KAppRes& appRes = KAppRes::Instance();
	m_bitmapHeader.Attach(appRes.GetImage("IDB_BITMAP_LISTCTRL_HEADER"));
	m_nFillFlag = -1;
	m_bUp = TRUE;
    m_bDrag = FALSE;
//	m_bitmapHeader.Attach( KuiBmpPool::GetBitmap(IDB_BITMAP_LISTCTRL_HEADER));
}

CHeaderCtrlEx::~CHeaderCtrlEx()
{
	//m_font.Detach();
	m_bitmapHeader.Detach();
}

void CHeaderCtrlEx::SetHeight(INT nHeight)
{
	m_nHeight = nHeight;
}

void CHeaderCtrlEx::DisableHeaderFlag()
{
	m_nFillFlag = -1;
}

void CHeaderCtrlEx::DoPaint(CDCHandle dc)
{	
 	CDC dcTemp;
 	dcTemp.CreateCompatibleDC( dc );
	HBITMAP hOldBitmap = dcTemp.SelectBitmap(m_bitmapHeader);

	HFONT hOldFont = dc.SelectFont(GetFont());
	dc.SetBkMode( TRANSPARENT );
	
	RECT rc = {0};
	GetClientRect( &rc );	
	dc.StretchBlt(rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, dcTemp, 0, 0, 3, 20, SRCCOPY);
	int nItems = GetItemCount();
	for(int i=0; i<nItems; ++i)
	{
		CRect rcItem;
		CRect rcAngle;
		GetItemRect(i, &rcItem);
		
		// Draw split 
		dc.StretchBlt(rcItem.right-3, rcItem.top, 3, rcItem.Height(), dcTemp, 3, 0, 3, 20, SRCCOPY);
		
		// Draw text
		rcItem.DeflateRect(2, 0);
		TCHAR buf[255] = {0};
		HDITEM hdItem = {0};
		
		hdItem.mask	 = HDI_TEXT | HDI_FORMAT | HDI_ORDER;
		hdItem.pszText = buf;
		hdItem.cchTextMax = 255;
		
		GetItem(i, &hdItem);
		UINT uFormat = DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER;
		UINT uAlign = DT_LEFT;
		if(hdItem.fmt & HDF_CENTER)
			uAlign = DT_CENTER;
		else if(hdItem.fmt & HDF_RIGHT)
			uAlign = DT_RIGHT;
		uFormat |= uAlign;
		
		dc.DrawText(buf, -1, rcItem, uFormat);
		
		rcAngle = rcItem;
		if ( m_nFillFlag == i)
		{
			if (m_bUp)
			{
				HBRUSH hBrush = ::CreateSolidBrush(RGB(255,0,0));

				GRADIENT_TRIANGLE gRect[1] ={ {0, 1, 2}
				};
				
				TRIVERTEX vert[3] = {
					{rcAngle.right - 10,rcAngle.bottom - 15, 0, 0, 0, 0}, 
					{rcAngle.right - 20,rcAngle.bottom - 15, 0, 0, 0, 0},
					{rcAngle.right - 15,rcAngle.bottom - 10, 0, 0, 0, 0},};

					COLORREF clrFirst = RGB(120,227,248);
					COLORREF clrSecond = RGB(99,212,236);
					COLORREF clrThird = RGB(74,194,222);

					vert[0].Red     = GetRValue(clrFirst) << 8;
					vert[0].Green   = GetGValue(clrFirst) << 8;
					vert[0].Blue    = GetBValue(clrFirst) << 8;
					vert[1].Red     = GetRValue(clrSecond) << 8;
					vert[1].Green   = GetGValue(clrSecond) << 8;
					vert[1].Blue    = GetBValue(clrSecond) << 8;
					vert[2].Red     = GetRValue(clrThird) << 8;
					vert[2].Green   = GetGValue(clrThird) << 8;
					vert[2].Blue    = GetBValue(clrThird) << 8;

					BOOL bret = ::GradientFill(dc, vert, 3, &gRect, 1, GRADIENT_FILL_TRIANGLE);
			}
			if (!m_bUp)
			{
				HBRUSH hBrush = ::CreateSolidBrush(RGB(255,0,0));

				GRADIENT_TRIANGLE gRect[1] ={ {0, 1, 2}
				};

				TRIVERTEX vert[3] = {
					{rcAngle.right - 10,rcAngle.bottom - 10, 0, 0, 0, 0}, 
					{rcAngle.right - 20,rcAngle.bottom - 10, 0, 0, 0, 0},
					{rcAngle.right - 15,rcAngle.bottom - 18, 0, 0, 0, 0},};

					COLORREF clrFirst = RGB(120,227,248);
					COLORREF clrSecond = RGB(99,212,236);
					COLORREF clrThird = RGB(74,194,222);

					vert[0].Red     = GetRValue(clrFirst) << 8;
					vert[0].Green   = GetGValue(clrFirst) << 8;
					vert[0].Blue    = GetBValue(clrFirst) << 8;
					vert[1].Red     = GetRValue(clrSecond) << 8;
					vert[1].Green   = GetGValue(clrSecond) << 8;
					vert[1].Blue    = GetBValue(clrSecond) << 8;
					vert[2].Red     = GetRValue(clrThird) << 8;
					vert[2].Green   = GetGValue(clrThird) << 8;
					vert[2].Blue    = GetBValue(clrThird) << 8;

					BOOL bret = ::GradientFill(dc, vert, 3, &gRect, 1, GRADIENT_FILL_TRIANGLE);
			}
		}

	}
	dc.SelectFont(hOldFont);	
	dcTemp.SelectBitmap(hOldBitmap);	
}

LRESULT CHeaderCtrlEx::OnHDMLayout(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lResult = CWindowImpl<CHeaderCtrlEx, CHeaderCtrl>::DefWindowProc(uMsg, wParam, lParam);
	HD_LAYOUT &hdl = *( HD_LAYOUT * ) lParam;
	RECT *prc = hdl.prc;
	WINDOWPOS *pwpos = hdl.pwpos;
	int nHeight = (int)(pwpos->cy);
	if(m_nHeight>=0)
		nHeight = m_nHeight;
	pwpos->cy = nHeight;
	prc->top = nHeight;
	return lResult;
}

LRESULT CHeaderCtrlEx::OnMouseMove( UINT nFlags, CPoint point )
{
	SetMsgHandled(FALSE);
	return S_OK;
}

void CHeaderCtrlEx::OnLButtonDown(UINT nFlags, CPoint point)
{
    int i = -1;
	i = GetClickItem(point);  

    if(i!=-1 && i!= GetItemCount()-1)
    {
        m_nFillFlag = i;
        m_bUp = !m_bUp;
        m_bDrag = FALSE;
       ::SendMessage(m_hNotifyHwnd, MSG_CLR_COLUNM_CLICK, (WPARAM)i, (LPARAM)0);
    }
    else
    {
        m_bDrag = TRUE;
    }
    
	Invalidate();
	SetMsgHandled(FALSE);
	return;
}

 void CHeaderCtrlEx::OnLButtonUp(UINT nFlags, CPoint point)
 {
     if(m_bDrag)
     {
  //       ::SendMessage(m_hNotifyHwnd, MSG_CLR_COLUNM_CLICK, (WPARAM)(GetItemCount()-1), (LPARAM)0);
     }
     m_bDrag = FALSE;
     Invalidate();
     SetMsgHandled(FALSE);
     return;
 }

int CHeaderCtrlEx::GetClickItem(CPoint point)
{
	int nCount = -1;
	nCount = GetItemCount();
	for (int i = 0; i<nCount; i++)
	{
		CRect rc;
		GetItemRect(i, &rc);
		if (point.x > rc.left + 20 && point.x < rc.right - 20 )
		{
			return i;
		}
	}
	return -1;
}

int CHeaderCtrlEx::GetClickEdge(CPoint point)
{
    int nCount = -1;
    nCount = GetItemCount();
    
    CRect rc;
    GetItemRect(nCount-1, &rc);
    if (point.x > rc.right - 10 && point.x < rc.right )
    {
            return 1;
    }
 
    return 0;

}

HWND CHeaderCtrlEx::SetNotifyHwnd(HWND hWnd)
{
	HWND wnd = m_hNotifyHwnd;
	m_hNotifyHwnd = hWnd;

	return wnd;
}

CListViewCtrlEx::CListViewCtrlEx( void )
{
	m_hWndObserver = NULL;
	m_nHoverItem = -1;

	m_fontLink.Attach( KuiFontPool::GetFont(FALSE,TRUE,FALSE,0) );
	m_fontBold.Attach( KuiFontPool::GetFont(TRUE,FALSE,FALSE) );
	m_fontTitle.Attach( KuiFontPool::GetFont(TRUE,FALSE,FALSE,4) );
	m_fontDef.Attach( KuiFontPool::GetFont(KUIF_DEFAULTFONT));
	//m_bitmapCheck.Attach( KuiBmpPool::GetBitmap(IDB_BITMAP_LISTCTRL_CHECK) );
	//m_bitmapExpand.Attach( KuiBmpPool::GetBitmap(IDB_BITMAP_MINUS_PLUS) );
	//m_bitmapRadio.Attach( KuiBmpPool::GetBitmap(IDB_BITMAP_LISTCTRL_RADIO) );
	CreateBitmapMask(m_bitmapRadio, m_bitmapRadioMask, 0, 0, 0);
	//m_strEmptyString = BkString::Get(IDS_VULFIX_5163);
}

CListViewCtrlEx::~CListViewCtrlEx( void )
{
	m_fontLink.Detach();
	m_fontBold.Detach();
	m_fontTitle.Detach();
	m_bitmapCheck.Detach();
	m_bitmapExpand.Detach();
	m_bitmapRadio.Detach();
	m_fontDef.Detach();
	m_bitmapRadioMask.DeleteObject();
}

HWND CListViewCtrlEx::Create( HWND hWndParent, _U_RECT rect /*= NULL*/, LPCTSTR szWindowName /*= NULL*/, DWORD dwStyle /*= 0*/, DWORD dwExStyle /*= 0*/, _U_MENUorID MenuOrID /*= 0U*/, LPVOID lpCreateParam /*= NULL*/ )
{
	HWND hWnd = _super::Create(hWndParent, rect, szWindowName, dwStyle|LVS_OWNERDRAWFIXED, dwExStyle, MenuOrID, lpCreateParam);
	_super::SetExtendedListViewStyle( LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT|_super::GetExtendedListViewStyle() );
	
	m_ctlHeader.SubclassWindow( GetHeader().m_hWnd );
	m_ctlHeader.SetHeight(25);
	m_ctlHeader.ModifyStyle(HDS_FULLDRAG, 0, 0);
	m_ctlHeader.SetFont(KuiFontPool::GetFont(FALSE, FALSE, FALSE));

	_super::SetBkColor(BACKGROUND_COLOR);
	return hWnd;
}

BOOL CListViewCtrlEx::GetCheckState( int nIndex, BOOL *pIsRadio ) const
{
	const TListItem *pItem = _GetItemData(nIndex);
	if( pItem && (pItem->dwFlags&(LISTITEM_CHECKBOX|LISTITEM_RADIOBOX)) )
	{
		if(pIsRadio && pItem->dwFlags&LISTITEM_RADIOBOX)
			*pIsRadio = TRUE;
		return _super::GetCheckState( nIndex );
	}
	return FALSE;
}

BOOL CListViewCtrlEx::SetCheckState( int nItem, BOOL bCheck )
{
	TListItem *pItem = _GetItemData( nItem );
	if( pItem && (pItem->dwFlags&LISTITEM_CHECKBOX) )
	{
		if( bCheck )
			ClearRadioCheck();
		return _super::SetCheckState( nItem, bCheck );
	}
	return FALSE;
}

int CListViewCtrlEx::SetRadioState( int nItem, BOOL bCheck )
{
	TListItem *pItem = _GetItemData( nItem );
	if( pItem && (pItem->dwFlags&LISTITEM_RADIOBOX) )
	{
		if( bCheck )
			CleanCheck( nItem );
		return _super::SetCheckState( nItem, bCheck );
	}
	return -1;
}

void CListViewCtrlEx::ClearRadioCheck()
{
	for(int i=0; i<_super::GetItemCount(); ++i)
	{
		TListItem *pItem = _GetItemData( i );
		if( pItem->dwFlags & LISTITEM_RADIOBOX )
		{
			_super::SetCheckState(i, FALSE);
		}
	}
}

void CListViewCtrlEx::CleanCheck( int nItem/*=-1 */ )
{
	for(int i=0; i<_super::GetItemCount(); ++i)
	{
		if(nItem!=i)
			_super::SetCheckState( i, FALSE );
	}
}

void CListViewCtrlEx::CheckAll()
{
	ClearRadioCheck();
	for(int i=0; i<_super::GetItemCount(); ++i)
	{
		TListItem *pItem = _GetItemData( i );
		if( pItem && (pItem->dwFlags&LISTITEM_CHECKBOX) )
		{
			_super::SetCheckState( i, TRUE );
		}
	}
}

void CListViewCtrlEx::SetObserverWindow( HWND hWnd )
{
	m_hWndObserver = hWnd;
}

int CListViewCtrlEx::AppendTitle( LPCTSTR strItem, COLORREF clr, UINT uFlags)
{
//	return Append(strItem, LISTITEM_TITLE);

	int nItem = _super::AddItem(_super::GetItemCount(), 0, _T(""));
	TListItem *pitem = new TListItem;
	pitem->dwFlags = LISTITEM_TITLE | uFlags;
	pitem->clrBg	=RGB(248,248,248);
	pitem->bBold = uFlags & LISTITEM_BOLD;
	pitem->nLeftmargin = 15;
	
	TListSubItem subItem(strItem, SUBITEM_TEXT);
	subItem.clr = clr;
	pitem->subItems.push_back( subItem );
	m_arrItems.push_back( pitem );
	return nItem;
}

int CListViewCtrlEx::Append( LPCTSTR strItem, DWORD dwFlags /*=0*/, E_SubItemType itemType/*=SUBITEM_TEXT*/ )
{
	int nItem = _super::AddItem(_super::GetItemCount(), 0, _T(""));
	TListItem *pitem = new TListItem;
	pitem->dwFlags = dwFlags;
	pitem->subItems.push_back( TListSubItem(strItem, itemType) );
	m_arrItems.push_back( pitem );
	return nItem;
}

int CListViewCtrlEx::AppendTitleItem(int nItem, LPCTSTR strItem, CRect rc, E_SubItemType itemType, COLORREF clr, LPCTSTR szURL )
{
	BOOL bValid = nItem>=0 && nItem < (int)m_arrItems.size();
	ATLASSERT(bValid);
	if( !bValid )
		return -1;
	TListSubItem  isubitem(strItem, rc, itemType);
	if(szURL)
		isubitem.strUrl = szURL;
	isubitem.clr = clr;
	m_arrItems[nItem]->subItems.push_back( isubitem );
	return (int)m_arrItems[nItem]->subItems.size();
}

int CListViewCtrlEx::AppendSubItem( int nItem, LPCTSTR strItem, E_SubItemType itemType/*=SUBITEM_TEXT*/ )
{
	BOOL bValid = nItem >=0 && nItem < (int)m_arrItems.size();
	ATLASSERT(bValid);
	if( !bValid )
		return -1;
	m_arrItems[nItem]->subItems.push_back( TListSubItem(strItem, itemType) );
	return (int)m_arrItems[nItem]->subItems.size();
}

bool CListViewCtrlEx::SetSubItem( int nItem, int nSubItem, LPCTSTR lpszItem, E_SubItemType itemType/*=SUBITEM_TEXT*/, BOOL bRedraw )
{
	if (nItem >=0 && nItem < (int)m_arrItems.size())
	{
		std::vector<TListSubItem> &subItems = m_arrItems[nItem]->subItems;
		if (nSubItem >= 0 && nSubItem < (int)subItems.size())
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

bool CListViewCtrlEx::GetSubItemText( int nItem, int nSubItem, CString &str )
{
	if (nItem >= 0 && nItem < (int)m_arrItems.size())
	{
		std::vector<TListSubItem> &subItems = m_arrItems[nItem]->subItems;
		if (nSubItem >= 0 && nSubItem < (int)subItems.size())
		{
			str = subItems[nSubItem].str; 
			return true;
		}
	}
	return false;
}

bool CListViewCtrlEx::SetSubItemColor( int nItem, int nSubItem, COLORREF clr, BOOL bRedraw  )
{
	if (nItem >= 0 && nItem < (int)m_arrItems.size())
	{
		std::vector<TListSubItem> &subItems = m_arrItems[nItem]->subItems;
		if (nSubItem >= 0 && nSubItem < (int)subItems.size())
		{
			subItems[nSubItem].clr = clr;
			if(bRedraw)
				_InvalidateRect(nItem, nSubItem);
			return true;
		}
	}
	return false;
}

bool CListViewCtrlEx::SetItemCheckX( int nItem, DWORD dwFlags )
{
	TListItem *pItem = _GetItemData(nItem);
	if(pItem)
	{
		if( dwFlags & LISTITEM_CHECKBOX )
			pItem->dwFlags |= LISTITEM_CHECKBOX;
		else if(dwFlags & LISTITEM_RADIOBOX)
			pItem->dwFlags |= LISTITEM_RADIOBOX;
		else
		{
			DWORD dwFlags = LISTITEM_CHECKBOX|LISTITEM_RADIOBOX;
			pItem->dwFlags &= ~dwFlags;
			_super::SetCheckState( nItem, FALSE );
		}
	}
	return pItem!=NULL;
}

BOOL CListViewCtrlEx::DeleteItem( int nItem )
{
	TListItemPtrs::iterator it = m_arrItems.begin();
	if(nItem>0)
		std::advance(it, nItem);
	delete *it;
	m_arrItems.erase( it );
	return _super::DeleteItem( nItem );
}

VOID CListViewCtrlEx::DeleteItem( LPDELETEITEMSTRUCT )
{
	ATLASSERT(FALSE);	
}

BOOL CListViewCtrlEx::DeleteAllItems()
{
	for(TListItemPtrs::iterator it=m_arrItems.begin(); it!=m_arrItems.end(); ++it)
		delete *it;
	m_arrItems.clear();
	return _super::DeleteAllItems();
}

LRESULT CListViewCtrlEx::OnClick( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
	bHandled = TRUE;
	
	LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) pnmh;

	// - title collapsed
	TListItem *pItem = _GetItemData(lpnmitem->iItem);
	if(!pItem)
		return 0;

	if(pItem->dwFlags & LISTITEM_TITLE)
	{
		if(pItem->dwFlags & LISTITEM_EXPANDABLE)
		{
			INT iItem = lpnmitem->iItem;
			RECT rcItem = {0};
			if( GetItemRect(iItem, &rcItem, LVIR_BOUNDS) )
			{
				_ExpandItem(pItem, lpnmitem->iItem, pItem->_isclapsed);
				InvalidateRect( &rcItem );
			}		
			return 0;
		}
	}
	else 
	{
		// - link check 
		int iSubItem = lpnmitem->iSubItem;
		if(_PtInSubItemLink(lpnmitem->ptAction, lpnmitem->iItem, iSubItem) )
			_FireEvent(WMH_LISTEX_LINK, lpnmitem->iItem, iSubItem);
		else if( pItem->dwFlags & (LISTITEM_CHECKBOX|LISTITEM_RADIOBOX) )
		{
			// - check box 
			BOOL bDirty = FALSE;
			if (pItem->dwFlags&LISTITEM_RADIOBOX)
			{
				bDirty = TRUE;
				SetRadioState( lpnmitem->iItem, !_super::GetCheckState(lpnmitem->iItem) );
			}
			else if (_PtInSubItemCheckBox(lpnmitem->ptAction, lpnmitem->iItem))
			{
				bDirty = TRUE;
				SetCheckState( lpnmitem->iItem, !_super::GetCheckState(lpnmitem->iItem) );
			}
			
			if( bDirty )
			{
				CRect	rcItem;
				GetItemRect( lpnmitem->iItem, &rcItem, LVIR_BOUNDS);
				InvalidateRect( rcItem );
			}
		}
	}
	return 0;
}

void CListViewCtrlEx::OnPaint( CDCHandle )
{
	if(m_arrItems.empty())
	{
		CPaintDC dc(m_hWnd);
		RECT rc = {0};
		GetClientRect( &rc );
		//dc.FillSolidRect(&rc, RGB(0,0,255));
		rc.top += 50;
		HFONT hOldFont = dc.SelectFont(m_fontDef);
		dc.DrawText(m_strEmptyString, -1, &rc, DT_SINGLELINE|DT_LEFT|DT_CENTER);
		dc.SelectFont( hOldFont );
	}
	else
	{
		SetMsgHandled(FALSE);
	}
}

void CListViewCtrlEx::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect	rcItem;
	SetMsgHandled(FALSE);
	
	LVHITTESTINFO hti;
	hti.pt = point;
	SubItemHitTest( &hti );
	if( m_nHoverItem!=hti.iItem )
	{
		m_nHoverItem = hti.iItem;
		_FireEvent(WMH_LISTEX_HOVERCHANGED, m_nHoverItem);
	}
}

BOOL CListViewCtrlEx::OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
{
	CPoint pt;
	GetCursorPos( &pt );
	ScreenToClient( &pt );

	LVHITTESTINFO hti;
	hti.pt = pt;
	SubItemHitTest( &hti );
	bool bInLink = _PtInSubItemLink(pt, hti.iItem, hti.iSubItem);
	::SetCursor(::LoadCursor(NULL, bInLink?IDC_HAND:IDC_ARROW));
	return TRUE;
}

void CListViewCtrlEx::MeasureItem( LPMEASUREITEMSTRUCT lpMes )
{
	/*
	const TListItem *pItem = _GetItemData( lpMes->itemID );
	if(pItem)
	{
		if(pItem->dwFlags&LISTITEM_TITLE)
			lpMes->itemHeight = 30;
		else
			lpMes->itemHeight = 28;
	}
	*/

	lpMes->itemHeight = 24;
}

void CListViewCtrlEx::DrawItem( LPDRAWITEMSTRUCT lpdis )
{
	int nItem = lpdis->itemID;
	const TListItem *pItem = _GetItemData( nItem );
	if(!pItem)
		return;
	if(pItem->dwFlags & LISTITEM_TITLE)
	{
		_DrawTitleItem(lpdis, pItem);
	}
	else
	{
		_DrawNormalItem(lpdis, pItem);
	}
	return;
}

void CListViewCtrlEx::SetEmptyString( LPCTSTR szEmptyString )
{
	if(szEmptyString)
		m_strEmptyString = szEmptyString;
	else
		m_strEmptyString = _T("");
}

void CListViewCtrlEx::ExpandGroup( INT nItem, BOOL bExpand )
{
	// - title collapsed
	TListItem *pItem = _GetItemData( nItem );
	if(!pItem)
		return; 
	
	if( (pItem->dwFlags & LISTITEM_TITLE)
		&& (pItem->dwFlags & LISTITEM_EXPANDABLE) )
	{
		BOOL needRedraw = _ExpandItem(pItem, nItem, bExpand);
		RECT rcItem = {0};
		if( GetItemRect(nItem, &rcItem, LVIR_BOUNDS) )
			InvalidateRect( &rcItem );		
	}
}

void CListViewCtrlEx::_DrawTitleItem( LPDRAWITEMSTRUCT lpdis, const TListItem *pItem )
{
	ATLASSERT(pItem);
	
	if(pItem->subItems.empty())
		return ;

	int	nWinWidth=lpdis->rcItem.right-lpdis->rcItem.left;
	CRect	rcWindows;
	GetWindowRect(rcWindows);
	if ( nWinWidth > rcWindows.Width())
		nWinWidth = rcWindows.Width()-20;

	int nItem = lpdis->itemID;
	
	CDCHandle dc;
	dc.Attach(lpdis->hDC);
	dc.FillSolidRect( &lpdis->rcItem, pItem->clrBg);
	HFONT	hOldFont = dc.SelectFont(m_fontDef);
	COLORREF clrOld = dc.GetTextColor();
	COLORREF clrDef = clrOld;
	
	// 
	RECT rcItem = lpdis->rcItem;
	if( pItem->dwFlags&LISTITEM_EXPANDABLE )
	{
		//3 + 9 + 3
		if(rcItem.left>-12)
		{
			CDC	dcTmp;
			dcTmp.CreateCompatibleDC(dc);
			HBITMAP hBmpOld	= dcTmp.SelectBitmap(m_bitmapExpand);				
			RECT rcMinus = _GetRectMinus(rcItem);
			dc.BitBlt( rcMinus.left, rcMinus.top, 9, 9, dcTmp, pItem->_isclapsed? 9:0, 0, SRCCOPY);
			dcTmp.SelectBitmap(hBmpOld);
		}
	}
	
	for (int i = 0; i < (int)pItem->subItems.size(); i++)
	{
		bool	bVCenter=TRUE;
		const TListSubItem& subItem = pItem->subItems[i];
		CRect rcItem = subItem.rcOffset;
		if ( i == 0 )
		{
			rcItem = lpdis->rcItem;

			if(pItem->nTopMargin>=0)
			{
				rcItem.top += pItem->nTopMargin;
				rcItem.bottom -= 0;
				bVCenter=FALSE;
			}
			else
			{
				rcItem.top += 2;
				rcItem.bottom -= 2;
			}
			rcItem.left+= pItem->nLeftmargin;
		}
		else
		{
			if ( rcItem.left < 0 )
			{
				rcItem.left = nWinWidth+rcItem.left;
			}
			if (rcItem.right < 0)
			{
				rcItem.right = nWinWidth+rcItem.right;
			}
			rcItem.OffsetRect( lpdis->rcItem.left, lpdis->rcItem.top);
		}

		if ( subItem.type == SUBITEM_TEXT )
		{
			dc.SetTextColor( subItem.clr);
			dc.SelectFont(m_fontDef);
		}
		else if ( subItem.type == SUBITEM_LINK )
		{
			dc.SelectFont(m_fontLink);
			dc.SetTextColor(COLOR_LIST_LINK);
		}

		CString strTitle = subItem.str;	
		DWORD	nFlag=DT_SINGLELINE|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS;
		if(bVCenter)
			nFlag|=DT_VCENTER;

		if (i==0&&pItem->bBold||pItem->nHeightAdd!=0)
		{
			HFONT	fntOld=dc.SelectFont(KuiFontPool::GetFont(pItem->bBold,FALSE,FALSE,pItem->nHeightAdd));
			dc.DrawText( strTitle, -1, &rcItem, nFlag);
			dc.SelectFont(fntOld);
		}
		else
			dc.DrawText( strTitle, -1, &rcItem, nFlag);
	}

	CPen	pex;
	pex.CreatePen(PS_SOLID,1,pItem->clrBtmGapLine);
	HPEN	penOld = dc.SelectPen(pex);
	dc.MoveTo( lpdis->rcItem.left, lpdis->rcItem.bottom-1 );
	dc.LineTo( lpdis->rcItem.right, lpdis->rcItem.bottom-1 );
	
	dc.SetTextColor(clrOld);
	dc.SelectPen(penOld);
	dc.SelectFont(hOldFont);
	dc.Detach();
}

void CListViewCtrlEx::_DrawNormalItem( LPDRAWITEMSTRUCT lpdis, const TListItem *pItem )
{
	ATLASSERT(pItem);
	int nItem = lpdis->itemID;

	CDCHandle dc;
	dc.Attach(lpdis->hDC);

	HFONT	hOldFont = dc.SelectFont(m_fontDef);

	BOOL bSelect = FALSE ;
	if ((lpdis->itemAction | ODA_SELECT) &&
		(lpdis->itemState & ODS_SELECTED))
	{
		bSelect = TRUE ;
	}

	if ( bSelect )
		dc.FillSolidRect( &lpdis->rcItem, RGB(185,219,255));
	else
		dc.FillSolidRect( &lpdis->rcItem, pItem->clrBg);

	// draw check box 
	if( pItem->dwFlags&(LISTITEM_CHECKBOX|LISTITEM_RADIOBOX) )
		_DrawCheckBox(dc, lpdis->rcItem, _super::GetCheckState(nItem), pItem->dwFlags);

	COLORREF oldClr = dc.GetTextColor();
	for (int i = 0; i < (int)pItem->subItems.size(); ++i)
	{
		CRect	rcSubItem;
		DWORD	nMarginWidth = 0;
		CRect	rcBounds;
		GetSubItemRect(nItem, i, LVIR_LABEL, &rcSubItem);

		nMarginWidth = LEFT_MARGIN_TEXT_COLUMN+3;
		
		if(i==0)
		{
			if( pItem->dwFlags&(LISTITEM_CHECKBOX|LISTITEM_RADIOBOX) )
			{
				nMarginWidth+=rcSubItem.left;
			}
			else
			{
				rcSubItem.left -= 19;
				nMarginWidth+=5;
			}
		}
		
#define DT_FLAGS_DRAW_TEXT		(DT_SINGLELINE|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS|DT_VCENTER)

		rcSubItem.left += LEFT_MARGIN_TEXT_COLUMN;
		rcSubItem.right -= 3;
		const TListSubItem &subItem = pItem->subItems[i];
		if(subItem.type==SUBITEM_LINK)
		{
			dc.SelectFont(m_fontLink);
			dc.SetTextColor(COLOR_LIST_LINK);

			CRect	rcProbeItem;
			dc.DrawText( subItem.str, -1, &rcProbeItem, DT_SINGLELINE|DT_LEFT|DT_NOPREFIX|DT_VCENTER|DT_CALCRECT);
			dc.DrawText( subItem.str, -1, &rcSubItem, DT_FLAGS_DRAW_TEXT);

			DWORD nMaxWidth = rcProbeItem.Width()+nMarginWidth;
			_SetColumnNeedWidth(i,nMaxWidth);
		}
		else
		{
			dc.SetTextColor( subItem.clr );
			dc.DrawText( subItem.str, -1, &rcSubItem, DT_SINGLELINE|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS|DT_VCENTER);

			CRect	rcProbeItem;
			dc.DrawText( subItem.str, -1, &rcProbeItem, DT_SINGLELINE|DT_LEFT|DT_NOPREFIX|DT_VCENTER|DT_CALCRECT);
			DWORD nMaxWidth = rcProbeItem.Width()+nMarginWidth;
			_SetColumnNeedWidth(i,nMaxWidth);
		}
	}
	
	CPen	penx;
	penx.CreatePen(PS_SOLID,1,pItem->clrBtmGapLine);
	HPEN	penOld = dc.SelectPen(penx);
	dc.MoveTo( lpdis->rcItem.left, lpdis->rcItem.bottom-1 );
	dc.LineTo( lpdis->rcItem.right, lpdis->rcItem.bottom-1);
	dc.SelectPen(penOld);

	dc.SelectFont(hOldFont);
	dc.SetTextColor(oldClr);	
	dc.Detach();
}

const CListViewCtrlEx::TListSubItem * CListViewCtrlEx::_GetSubItemData( int nItem, int nSubItem )
{
	if (nItem >= 0 && nItem < (int)m_arrItems.size())
	{
		const std::vector<TListSubItem> &subItems = m_arrItems[nItem]->subItems;
		if (nSubItem >= 0 && nSubItem < (int)subItems.size())
		{
			return &subItems[nSubItem];
		}
	}
	return NULL;
}

const CListViewCtrlEx::TListItem * CListViewCtrlEx::_GetItemData( int nItem ) const
{
	if (nItem >= 0 && nItem < (int)m_arrItems.size())
		return m_arrItems[nItem];
	return NULL;
}

CListViewCtrlEx::TListItem * CListViewCtrlEx::_GetItemData( int nItem )
{
	if (nItem >= 0 && nItem < (int)m_arrItems.size())
		return m_arrItems[nItem];
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
		if( pItem->dwFlags&(LISTITEM_CHECKBOX|LISTITEM_RADIOBOX) )
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

	rc.left += LEFT_MARGIN_TEXT_COLUMN;
	CDC dc;
	dc.Attach( hDC );
	HFONT hOldFont = dc.SelectFont(m_fontLink);
	dc.DrawText( szText, -1, &rc, DT_CALCRECT|DT_SINGLELINE|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS|DT_VCENTER );
	dc.SelectFont(hOldFont);
	dc.Detach();
	ReleaseDC( hDC );
	return TRUE;
}

bool CListViewCtrlEx::_PtInSubItemLink( const POINT &pt, int nItem, int& nSubItem )
{
	if( nItem>=0 && nItem<_super::GetItemCount() )
	{
		TListItem *pItem = _GetItemData(nItem);
		ATLASSERT(pItem);

		if ( pItem->dwFlags & LISTITEM_TITLE )
		{
			for (int i = 0; i < (int)pItem->subItems.size(); i++)
			{
				const TListSubItem&	subItem = pItem->subItems[i];
				if ( subItem.type == SUBITEM_LINK)
				{
					CRect	rcItem;
					CRect	rcLink = subItem.rcOffset;
					GetItemRect(nItem,rcItem, LVIR_BOUNDS);
					rcLink.OffsetRect(rcItem.left, rcItem.top);
					if ( rcLink.PtInRect(pt) )
					{
						nSubItem = i;
						return true;
					}
				}
			}
		}
		else if (nSubItem >= 0 && nSubItem < (int)pItem->subItems.size())
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

int CListViewCtrlEx::_DrawCheckBox( CDCHandle &dc, RECT &rcItem, BOOL bChecked, DWORD dwFlags )
{
	if(rcItem.left>-16)
	{
		CDC	dcTmp;
		dcTmp.CreateCompatibleDC(dc);
		
		RECT rcCheckBox = _GetRectCheckBox( rcItem );
		HBITMAP hBmpOld = NULL;
		int x=rcCheckBox.left, y=rcCheckBox.top, nWidth=13, nHeight=13, xSrc=bChecked ? 0 : 13, ySrc=0;
		
		if(dwFlags&LISTITEM_CHECKBOX)
		{
			hBmpOld	= dcTmp.SelectBitmap(m_bitmapCheck);
			dc.BitBlt(x, y, nWidth, nHeight, dcTmp, xSrc, ySrc, SRCCOPY);
		}
		else
		{
			// Need Mask 
			hBmpOld	= dcTmp.SelectBitmap(m_bitmapRadioMask);
			dc.BitBlt(x, y, nWidth, nHeight, dcTmp, xSrc, ySrc, SRCAND);
			dcTmp.SelectBitmap(m_bitmapRadio);
			dc.BitBlt(x, y, nWidth, nHeight, dcTmp, xSrc, ySrc, SRCPAINT);
		}

		dcTmp.SelectBitmap(hBmpOld);
		dcTmp.DeleteDC();
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


RECT CListViewCtrlEx::_GetRectMinus( RECT &rcItem )
{
	INT nHeight = rcItem.bottom - rcItem.top;
	int nTop = rcItem.top + (nHeight-9)/2;
	RECT rcMinus = {rcItem.left+3, nTop, rcItem.left+3+9, nTop+9};	
	return rcMinus;
}

void CListViewCtrlEx::_InvalidateRect( int nItem, int nSubItem )
{
	RECT rcItem = {0};
	//_super::GetItemRect(nItem, &rcItem, LVIR_BOUNDS);
	_super::GetSubItemRect(nItem, nSubItem, LVIR_LABEL, &rcItem);
	InvalidateRect( &rcItem );
}

BOOL CListViewCtrlEx::_FireEvent( UINT message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/ )
{
	if(m_hWndObserver)
		return ::PostMessage(m_hWndObserver, message, wParam, lParam);
	else
		return GetParent().PostMessage(message, wParam, lParam);
}

void CListViewCtrlEx::_SetColumnNeedWidth( int i, DWORD nWidth )
{
	if ((int)nWidth<=0)
	{
		return;
	}

	DWORD	nOrgWidth = 0;
	if ( m_columnMaxWidth.Lookup(i, nOrgWidth) )
	{
		if ( nOrgWidth < nWidth )
			m_columnMaxWidth.SetAt(i,nWidth);
	}
	else
		m_columnMaxWidth.SetAt(i,nWidth);
}

LRESULT CListViewCtrlEx::OnTrackDbClickHeader( int idCtrl, LPNMHDR pnmhx, BOOL& bHandled )
{
	DWORD		nWidth	= 0;
	LPNMHEADER	pnmh	= (LPNMHEADER)pnmhx;

	bHandled = FALSE;
	INT col = pnmh->iItem;
	if ( m_columnMaxWidth.Lookup(col,nWidth) )
	{
		SetColumnWidth(pnmh->iItem,nWidth);
		InvalidateRect(NULL);
		bHandled = TRUE;
		return S_OK;
	}

	return S_OK;
}

void CListViewCtrlEx::Reset()
{
	m_columnMaxWidth.RemoveAll();
}

void CListViewCtrlEx::_RedrawTitle()
{
	for ( int i = 0; i < GetItemCount(); i++)
	{	
		TListItem*	pItem  = (TListItem*)_GetItemData(i);

		if ( pItem && pItem->dwFlags&LISTITEM_TITLE)
		{
			CListViewCtrl::RedrawItems(i,i);
		}
	}
}

LRESULT CListViewCtrlEx::OnTrackMoveHeader( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
	bHandled = TRUE;
	//_RedrawTitle();
	//InvalidateRect(NULL);
	return S_OK;
}

LRESULT CListViewCtrlEx::OnHeaderEndTrack( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
	//bHandled = FALSE;
	bHandled = TRUE;
	InvalidateRect(NULL);
	return S_OK;
}

BOOL CListViewCtrlEx::_ExpandItem( TListItem * pItem, INT iItem, BOOL expand )
{
	if(expand && pItem->_isclapsed)
	{
		// expand if collapsed 		
		_super::SetRedraw(FALSE);

		TListItemPtrs::iterator it = std::find(m_arrItems.begin(), m_arrItems.end(), pItem);
		ATLASSERT( it!=m_arrItems.end() );
		++it;		
		m_arrItems.insert(it, pItem->_clapsed_items.begin(), pItem->_clapsed_items.end());

		INT nNext = iItem + 1;
		for (int i = 0; i < (int)pItem->_clapsed_items.size(); ++i)
		{
			_super::AddItem(nNext, 0, _T(""));
			_super::SetItemData( nNext, pItem->_clapsed_items[i]->_itemData );
			++nNext;
		}
		_super::SelectItem( iItem );
		pItem->_clapsed_items.clear();
		pItem->_isclapsed = FALSE;

		_super::SetRedraw(TRUE);
#if 0 
		_tcprintf(_T("-ListCtrlEx expanded %d\n"), _super::GetItemCount());
		for(int i=0; i<_super::GetItemCount(); ++i)
		{
			_tcprintf(_T("%d - %s %x\n"), i, m_arrItems[i]->subItems[1].str, _super::GetItemData(i));
		}
#endif 
		return TRUE;
	}
	else if(!expand && !pItem->_isclapsed)
	{
		_super::SetRedraw(FALSE);
		// collapse if expanded
		int nEnd = iItem;
		for( int i=iItem+1; i<_super::GetItemCount(); ++i)
		{
			TListItem *p = _GetItemData( i );
			if(p->dwFlags & LISTITEM_TITLE)
				break;
			
			p->_itemData = _super::GetItemData( i );
			pItem->_clapsed_items.push_back( p );
			nEnd = i;
		}
	
		for(int i=nEnd; i > iItem; --i)
		{
			TListItemPtrs::iterator it = m_arrItems.begin();
			if(i>0)
				std::advance(it, i);
			_super::DeleteItem( i );
			m_arrItems.erase( it ); 
		}
		pItem->_isclapsed = TRUE;
		_super::SetRedraw(TRUE);
		return TRUE;
	}
	return FALSE;
}
