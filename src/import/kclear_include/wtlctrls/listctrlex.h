//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////

#include "dragdrop.h"
#include "droparrows.h"
#include "titletip.h"
#include "listedit.h"
#include "listcombo.h"
#include "listdate.h"

#include "guidef.h"
#include "gradient.h"
#include "listtypes.h"

//////////////////////////////////////////////////////////////////////////

using namespace std;

//////////////////////////////////////////////////////////////////////////

struct CListColumnEx
{
	CString m_strText;
	int m_nWidth;
	BOOL m_bFixed;
	UINT m_nFormat;
	UINT m_nFlags;
	int m_nImage;
	int m_nIndex;	
	CListArray <CString> m_aComboList;
};

//////////////////////////////////////////////////////////////////////////

template <class T>
class CListImplEx : public CWindowImpl<CListImplEx<T>>,
				  public CDoubleBufferImpl<CListImplEx<T>>
{
public:
	CListImplEx() : m_bEnableMultiSelection(false)
	{
		mbShowHeader = TRUE;
		mbShowThemed = FALSE;
		m_bSortAscending = FALSE;
		m_bButtonDown = FALSE;
		m_bMouseOver = FALSE;
		m_bColumnSizing = FALSE;
		m_bBeginSelect = FALSE;
		m_bSingleSelect = FALSE;
		m_bFocusSubItem = FALSE;
		m_bGroupSelect = FALSE;
		m_bEnableHorizScroll = FALSE;
		m_bEnableVertScroll = FALSE;
		m_bShowHorizScroll = TRUE;
		m_bShowVertScroll = TRUE;
		m_bShowSort = TRUE;
		m_bResizeTimer = FALSE;
		m_bDragDrop = FALSE;
		m_bSmoothScroll = TRUE;
		m_bEditItem = FALSE;
		m_bScrolling = FALSE;
		m_bScrollDown = FALSE;
		m_bTileBackground = FALSE;
		m_nMouseWheelScroll = 3;
		m_nTotalWidth = 0;
		mnHeaderHeight = 0;
		mnItemHeight = 0;
		m_nFirstSelected = NULL_ITEM;
		m_nFocusItem = NULL_ITEM;
		m_nFocusSubItem = NULL_SUBITEM;
		m_nHotItem = NULL_ITEM;
		m_nHotSubItem = NULL_SUBITEM;
		m_nTitleTipItem = NULL_ITEM;
		m_nTitleTipSubItem = NULL_SUBITEM;
		m_nSortColumn = NULL_COLUMN;
		m_nHighlightColumn = NULL_COLUMN;
		m_nDragColumn = NULL_COLUMN;
		m_nHotColumn = NULL_COLUMN;
		m_nHotDivider = NULL_COLUMN;
		m_nColumnSizing = NULL_COLUMN;
		m_nScrollOffset = 0;
		m_nScrollDelta = 0;
		m_nScrollUnit = 0;
		m_nStartScrollPos = 0;
		m_nStartSize = 0;
		m_nStartPos = 0;
		m_ptDownPoint = 0;
		m_ptSelectPoint = 0;
		m_rcGroupSelect = 0;
		m_dwSearchTick = 0;
		m_dwScrollTick = 0;
		m_strSearchString = _T( "" );
	}
	
	~CListImplEx()
	{
	}

protected:
	BOOL mbShowHeader;
	BOOL mbShowThemed;
	BOOL m_bShowSort;
	BOOL m_bSortAscending;
	BOOL m_bButtonDown;
	BOOL m_bMouseOver;
	BOOL m_bColumnSizing;
	BOOL m_bBeginSelect;
	BOOL m_bSingleSelect;
	BOOL m_bFocusSubItem;
	BOOL m_bGroupSelect;
	BOOL m_bShowHorizScroll;
	BOOL m_bShowVertScroll;
	BOOL m_bEnableHorizScroll;
	BOOL m_bEnableVertScroll;
	BOOL m_bResizeTimer;
	BOOL m_bDragDrop;
	BOOL m_bSmoothScroll;
	BOOL m_bEditItem;
	BOOL m_bScrolling;
	BOOL m_bScrollDown;
	BOOL m_bTileBackground;
	CPoint m_ptDownPoint;
	CPoint m_ptSelectPoint;
	CRect m_rcGroupSelect;
	int mnItemHeight;
	int mnHeaderHeight;
	int m_nFirstSelected;
	int m_nFocusItem;
	int m_nFocusSubItem;
	int m_nHotItem;
	int m_nHotSubItem;
	int m_nTitleTipItem;
	int m_nTitleTipSubItem;
	int m_nMouseWheelScroll;
	int m_nTotalWidth;
	int m_nSortColumn;
	int m_nDragColumn;
	int m_nHighlightColumn;
	int m_nHotColumn;
	int m_nHotDivider;
	int m_nColumnSizing;
	int m_nScrollOffset;
	int m_nScrollDelta;
	int m_nScrollUnit;
	int m_nStartScrollPos;
	int m_nStartSize;
	int m_nStartPos;
	DWORD m_dwSearchTick;
	DWORD m_dwScrollTick;
	CString m_strSearchString;
	CBitmap m_bmpScrollList;
	CBitmap m_bmpBackground;
	
	UINT m_nHeaderClipboardFormat;
	
	COLORREF mBKColor;
	COLORREF mHeaderBKColor;
	COLORREF mHeaderBorderColor;
	COLORREF mHeaderTextColor;
	COLORREF mHeaderHighlightColor;
	COLORREF mSelectedItemColor;
	COLORREF mSelectedTextColor;
	COLORREF mItemTextColor;
	COLORREF mSelectOuterColor;
	COLORREF mSelectInnerColor;
	COLORREF mSelectTopColor;
	COLORREF mSelectBottomColor;
	COLORREF mNoFocusTopColor;
	COLORREF mNoFocusBottomColor;
	COLORREF mNoFocusOuterColor;
	COLORREF mNoFocusInnerColor;
	COLORREF mFocusTopColor;
	COLORREF mFocusBottomColor;
	COLORREF mProgressTopColor;
	COLORREF mProgressBottomColor;
	COLORREF mItemFocusColor;
	COLORREF mHyperLinkColor;
	
	CTheme mHeaderTheme;
	CTheme m_thmProgress;
	CCursor mDividerCursor;
	CCursor mHandleCursor;
	CFont m_fntListFont;
	CFont m_fntUnderlineFont;
	CImageList mCtrlImageList;
	CImageList m_ilItemImages;
	//CDragDrop < CListImplEx > m_oleDragDrop;
	CToolTipCtrl m_ttToolTip;
	CDropArrowsEx m_wndDropArrows;
	CTitleTipEx m_wndTitleTip;
	CListEditEx m_wndItemEdit;
	CListComboEx m_wndItemCombo;
	CListDate m_wndItemDate;
	
	CListArray < CListColumnEx > m_aColumns;
	set < int > m_setSelectedItems;

	bool	m_bEnableMultiSelection;

public:
	BOOL SubclassWindow( HWND hWnd )
	{
		T* pT = static_cast<T*>(this);
		return CWindowImpl< CListImplEx >::SubclassWindow( hWnd ) ? pT->Initialise() : FALSE;
	}
	
	void RegisterClass()
	{
		T* pT = static_cast<T*>(this);
		pT->GetWndClassInfo().m_wc.lpfnWndProc = m_pfnSuperWindowProc;
		pT->GetWndClassInfo().Register( &m_pfnSuperWindowProc );
	}
	
	BOOL Initialise()
	{
		if ( !mCtrlImageList.CreateFromImage( IDB_LISTITEMS, 16, 0, RGB( 255, 0, 255 ), IMAGE_BITMAP, LR_CREATEDIBSECTION ) )
			return FALSE;
		
		if ( mDividerCursor.LoadSysCursor( IDC_SIZEWE ) == NULL )
			return FALSE;

		if (mHandleCursor.LoadSysCursor(IDC_ARROW) == NULL)
			return FALSE;
		
		if ( !LoadSettings() )
			return FALSE;
			
		//m_oleDragDrop.Register( this );
		//m_oleDragDrop.AddTargetFormat( m_nHeaderClipboardFormat );
		//m_oleDragDrop.AddSourceFormat( m_nHeaderClipboardFormat );
		
		if ( !m_ttToolTip.Create( m_hWnd ) )
			return FALSE;

		m_ttToolTip.SetMaxTipWidth( SHRT_MAX );
		
		return TRUE;
	}

	// 行被选中时的背景色
	void SetSelectedItemColor(const COLORREF& color)
	{
		mSelectedItemColor = color;
	}

	// 设置是否启用多选
	void EnableMultiSelection(bool bEnable)
	{
		m_bEnableMultiSelection = bEnable;
	}
	
	// 在此处修改各颜色
	BOOL LoadSettings()
	{
		mBKColor = GetSysColor( COLOR_WINDOW );
		mHeaderBKColor = GetSysColor( COLOR_BTNFACE );
		mHeaderBorderColor = CONTROL_BORDER_COLOR;
		mHeaderTextColor = GetSysColor( COLOR_WINDOWTEXT );
		mHeaderHighlightColor = RGB( 130, 140, 180 );

		 // 选中该行时的背景色
		mSelectedItemColor = RGB(222, 239, 214);				//RGB(185, 219, 255);
		mSelectedTextColor = GetSysColor( COLOR_WINDOWTEXT );
		mItemTextColor = GetSysColor( COLOR_WINDOWTEXT );
		mSelectOuterColor = RGB( 170, 200, 245 );
		mSelectInnerColor = RGB( 230, 250, 250 );
		mSelectTopColor = RGB( 210, 240, 250 );
		mSelectBottomColor = RGB( 185, 215, 250 );
		mNoFocusTopColor = RGB( 250, 250, 250 );
		mNoFocusBottomColor = RGB( 235, 235, 235 );
		mNoFocusOuterColor = RGB( 220, 220, 220 );
		mNoFocusInnerColor = RGB( 245, 245, 245 );
		mFocusTopColor = RGB( 235, 245, 245 );
		mFocusBottomColor = RGB( 225, 235, 245 );
		mProgressTopColor = RGB( 170, 240, 170 );
		mProgressBottomColor = RGB( 45, 210, 50 );
		mItemFocusColor = RGB( 180, 190, 210 );
		mHyperLinkColor = RGB( 0, 0, 200 );
		
		m_nHeaderClipboardFormat = RegisterClipboardFormat( _T( "HEADERCLIPBOARDFORMAT" ) );
		
		SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &m_nMouseWheelScroll, 0 );
		
		CLogFont logFont;
		
		gGuiFont.mFont.GetLogFont(logFont);

		if ( !m_fntListFont.IsNull() )
			m_fntListFont.DeleteObject();

		if ( m_fntListFont.CreateFontIndirect( &logFont ) == NULL )
			return FALSE;
		
		logFont.lfUnderline = BYTE(TRUE);

		if ( !m_fntUnderlineFont.IsNull() )
			m_fntUnderlineFont.DeleteObject();

		if ( m_fntUnderlineFont.CreateFontIndirect( &logFont ) == NULL )
			return FALSE;
		
		CClientDC dcClient( m_hWnd );
		
		HFONT hOldFont = dcClient.SelectFont( m_fntListFont );
		
		CSize sizeExtent;
		if ( !dcClient.GetTextExtent( _T( "Height" ), -1, &sizeExtent ) )
			return FALSE;
		
		dcClient.SelectFont( hOldFont );
		
		// has system font changed
		if ( mnItemHeight != sizeExtent.cy + ITEM_HEIGHT_MARGIN )
		{
			mnItemHeight = sizeExtent.cy + ITEM_HEIGHT_MARGIN;
			mnHeaderHeight = mnItemHeight;
			
			// create drop arrows window
			if ( m_wndDropArrows.IsWindow() )
				m_wndDropArrows.DestroyWindow();			
			if ( !m_wndDropArrows.Create( m_hWnd, mnHeaderHeight, TRUE ) )
				return FALSE;
		}
		
		// open header theme
		if ( !mHeaderTheme.IsThemeNull() )
			mHeaderTheme.CloseThemeData();
		mHeaderTheme.OpenThemeData( m_hWnd, L"HEADER" );
		
		// open progress theme
		if ( !m_thmProgress.IsThemeNull() )
			m_thmProgress.CloseThemeData();
		m_thmProgress.OpenThemeData( m_hWnd, L"PROGRESS" );
		
		// create titletip window
		if ( m_wndTitleTip.IsWindow() )
			m_wndTitleTip.DestroyWindow();			
		if ( !m_wndTitleTip.Create( m_hWnd, mbShowThemed ) )
			return FALSE;
		
		return TRUE;
	}
	
	void ShowThemed( BOOL bShowThemed = TRUE )
	{
		mbShowThemed = bShowThemed;
		LoadSettings();
		Invalidate();
	}
	
	void ShowHeader( BOOL bShowHeader = TRUE )
	{
		mbShowHeader = bShowHeader;		
		ResetScrollBars();
		Invalidate();
	}

	void ShowHeaderSort( BOOL bShowSort = FALSE )
	{
		m_bShowSort = bShowSort;
		Invalidate();
	}

	void SetSingleSelect( BOOL bSingleSelect = TRUE )
	{
		m_bSingleSelect = bSingleSelect;
		Invalidate();
	}
	
	void SetFocusSubItem( BOOL bFocusSubItem = TRUE )
	{
		m_bFocusSubItem = bFocusSubItem;
		Invalidate();
	}
	
	void SetDragDrop( BOOL bDragDrop = TRUE )
	{
		m_bDragDrop = bDragDrop;
	}
	
	void SetSmoothScroll( BOOL bSmoothScroll = TRUE )
	{
		m_bSmoothScroll = bSmoothScroll;
	}
	
	void SetBackgroundImage( HBITMAP hBackgroundImage, BOOL bTileImage = FALSE )
	{
		m_bmpBackground = hBackgroundImage;
		m_bTileBackground = bTileImage;
	}
	
	void SetImageList( CImageList& ilItemImages )
	{
		m_ilItemImages = ilItemImages;
	}
	
	UINT ValidateFlags( UINT nFlags )
	{
		if ( nFlags & ITEM_FLAGS_CENTRE )
			nFlags &= ~( ITEM_FLAGS_LEFT | ITEM_FLAGS_RIGHT );
		if ( nFlags & ITEM_FLAGS_RIGHT )
			nFlags &= ~ITEM_FLAGS_LEFT;
		if ( nFlags & ITEM_FLAGS_DATE_ONLY )
			nFlags &= ~ITEM_FLAGS_TIME_ONLY;
		if ( nFlags & ( ITEM_FLAGS_EDIT_NUMBER | ITEM_FLAGS_EDIT_FLOAT ) )
			nFlags &= ~ITEM_FLAGS_EDIT_UPPER;
		if ( !( nFlags & ( ITEM_FLAGS_EDIT_NUMBER | ITEM_FLAGS_EDIT_FLOAT ) ) )
			nFlags &= ~( ITEM_FLAGS_EDIT_NEGATIVE | ITEM_FLAGS_EDIT_OPERATOR );
		if ( nFlags & ITEM_FLAGS_COMBO_EDIT )
			nFlags &= ~( ITEM_FLAGS_DATE_ONLY | ITEM_FLAGS_TIME_ONLY | ITEM_FLAGS_DATETIME_NONE );
		return nFlags;
	}
	
	void AddColumn( CListColumnEx& listColumn )
	{
		// minimum column width
		if ( listColumn.m_strText.IsEmpty() && listColumn.m_nImage != ITEM_IMAGE_NONE )
		{
			CSize sizeIcon;
			mCtrlImageList.GetIconSize( sizeIcon );
			listColumn.m_nWidth = sizeIcon.cx + 5;
			listColumn.m_nFlags |= ITEM_FLAGS_CENTRE;
		}
		
		// correct incompatible flag mask values
		listColumn.m_nFlags = ValidateFlags( listColumn.m_nFlags );
		
		// initial data index
		listColumn.m_nIndex = GetColumnCount();
		
		m_aColumns.Add( listColumn );
		
		ResetScrollBars();
		Invalidate();
	}
	
	void AddColumn( LPCTSTR lpszText, int nWidth = 0, int nImage = ITEM_IMAGE_NONE, BOOL bFixed = FALSE, UINT nFormat = ITEM_FORMAT_NONE, UINT nFlags = ITEM_FLAGS_NONE )
	{
		CListColumnEx listColumn;
		listColumn.m_strText = lpszText;
		listColumn.m_nWidth = nWidth;
		listColumn.m_bFixed = bFixed;
		listColumn.m_nFormat = nFormat;
		listColumn.m_nFlags = nFlags;
		listColumn.m_nImage = nImage;
		AddColumn( listColumn );
	}
	
	int GetColumnCount()
	{
		return m_aColumns.GetSize();
	}
	
	BOOL GetColumn( int nColumn, CListColumnEx& listColumn )
	{
		if ( nColumn < 0 || nColumn >= GetColumnCount() ) 
			return FALSE;
		listColumn = m_aColumns[ nColumn ];
		return TRUE;
	}
		
	int GetTotalWidth( BOOL bRecalc = FALSE )
	{
		if ( bRecalc )
		{
			m_nTotalWidth = 0;
			for ( int nColumn = 0; nColumn < GetColumnCount(); nColumn++ )
				m_nTotalWidth += GetColumnWidth( nColumn );
		}		
		return m_nTotalWidth - 1;
	}
	
	int GetTotalHeight()
	{
		T* pT = static_cast<T*>(this);
		return max( ( pT->GetItemCount() * mnItemHeight ) + ( mbShowHeader ? mnHeaderHeight : 0 ), 1 );
	}
	
	BOOL SetColumnWidth( int nColumn, int nWidth )
	{
		if ( nColumn < 0 || nColumn >= GetColumnCount() ) 
			return FALSE;
		
		if ( !m_aColumns[ nColumn ].m_bFixed )
		{
			m_aColumns[ nColumn ].m_nWidth = nWidth;
			
			ResetScrollBars();
			Invalidate();
		}
		
		return TRUE;
	}

	void SetColumnText(
		int nColumn,
		const WCHAR *psText)
	{
		if (nColumn < 0 || nColumn >= GetColumnCount()) 
			return;

		m_aColumns[nColumn].m_strText = psText;
	}

	int GetColumnWidth( int nColumn )
	{
		CListColumnEx listColumn;
		return GetColumn( nColumn, listColumn ) ? listColumn.m_nWidth : 0;
	}
	
	int GetColumnIndex( int nColumn )
	{
		CListColumnEx listColumn;
		return GetColumn( nColumn, listColumn ) ? listColumn.m_nIndex : 0;
	}
	
	int IndexToOrder( int nIndex )
	{
		for ( int nColumn = 0; nColumn < GetColumnCount(); nColumn++ )
		{
			if ( GetColumnIndex( nColumn ) == nIndex )
				return nColumn;
		}
		return -1;
	}
	
	BOOL SetColumnFormat( int nColumn, UINT nFormat, UINT nFlags = ITEM_FLAGS_NONE )
	{
		if ( nColumn < 0 || nColumn >= GetColumnCount() ) 
			return FALSE;
		m_aColumns[ nColumn ].m_nFormat = nFormat;
		m_aColumns[ nColumn ].m_nFlags = ValidateFlags( nFlags );
		return TRUE;
	}
	
	BOOL SetColumnFormat( int nColumn, UINT nFormat, UINT nFlags, CListArray < CString >& aComboList )
	{
		if ( nColumn < 0 || nColumn >= GetColumnCount() ) 
			return FALSE;
		m_aColumns[ nColumn ].m_nFormat = nFormat;
		m_aColumns[ nColumn ].m_nFlags = ValidateFlags( nFlags );
		m_aColumns[ nColumn ].m_aComboList = aComboList;
		return TRUE;
	}
	
	UINT GetColumnFormat( int nColumn )
	{
		CListColumnEx listColumn;
		return GetColumn( nColumn, listColumn ) ? listColumn.m_nFormat : ITEM_FORMAT_NONE;
	}
	
	UINT GetColumnFlags( int nColumn )
	{
		CListColumnEx listColumn;
		return GetColumn( nColumn, listColumn ) ? listColumn.m_nFlags : ITEM_FLAGS_NONE;
	}
	
	BOOL GetColumnComboList( int nColumn, CListArray < CString >& aComboList )
	{
		CListColumnEx listColumn;
		if ( !GetColumn( nColumn, listColumn ) )
			return FALSE;
		aComboList = listColumn.m_aComboList;
		return !aComboList.IsEmpty();		
	}
	
	BOOL GetColumnRect( int nColumn, CRect& rcColumn )
	{
		if ( nColumn < 0 || nColumn >= GetColumnCount() ) 
			return FALSE;
			
		GetClientRect( rcColumn );
		rcColumn.bottom = mnHeaderHeight;
		
		for ( int nColumnOrder = 0; nColumnOrder < GetColumnCount(); nColumnOrder++ )
		{
			int nWidth = GetColumnWidth( nColumnOrder );
			
			if ( nColumn == nColumnOrder )
			{
				rcColumn.right = rcColumn.left + nWidth;
				break;
			}
			
			rcColumn.left += nWidth;
		}
		
		// offset column by scroll position
		rcColumn.OffsetRect( -GetScrollPos( SB_HORZ ), 0 );
		
		return TRUE;
	}
	
	BOOL AddItem()
	{
		ResetScrollBars();
		return Invalidate();
	}
	
	BOOL DeleteItem( int nItem )
	{
		m_setSelectedItems.erase( nItem );		
		ResetScrollBars();
		return Invalidate();
	}
	
	BOOL DeleteAllItems()
	{
		m_setSelectedItems.clear();		
		ResetScrollBars();
		return Invalidate();
	}
	
	int GetItemCount()
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return 0;
	}
	
	CString GetItemText( int nItem, int nSubItem )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return _T( "" );
	}
	
	BOOL GetItemDate( int nItem, int nSubItem, SYSTEMTIME& stItemDate )
	{
		T* pT = static_cast<T*>(this);
		
		ZeroMemory( &stItemDate, sizeof( SYSTEMTIME ) );
		
		CString strItemText = pT->GetItemText( nItem, nSubItem );
		if ( strItemText.IsEmpty() )
			return FALSE;
		
		// get date-time from item text: yyyymmddhhmmss
		stItemDate.wYear = _ttoi( strItemText.Left( 4 ) );
		stItemDate.wMonth = _ttoi( strItemText.Mid( 4, 2 ) );
		stItemDate.wDay = _ttoi( strItemText.Mid( 6, 2 ) );
		stItemDate.wHour = _ttoi( strItemText.Mid( 8, 2 ) );
		stItemDate.wMinute = _ttoi( strItemText.Mid( 10, 2 ) );
		stItemDate.wSecond = _ttoi( strItemText.Mid( 12, 2 ) );
		stItemDate.wMilliseconds = 0;
		
		return TRUE;
	}
	
	int GetItemImage( int nItem, int nSubItem )
	{
		return ITEM_IMAGE_NONE; // may be implemented in a derived class
	}
	
	virtual UINT GetItemFormat( int nItem, int nSubItem )
	{
		return GetColumnFormat( IndexToOrder( nSubItem ) ); // may be implemented in a derived class
	}
	
	virtual UINT GetItemFlags( int nItem, int nSubItem )
	{
		return GetColumnFlags( IndexToOrder( nSubItem ) ); // may be implemented in a derived class
	}
	
	BOOL GetItemComboList( int nItem, int nSubItem, CListArray < CString >& aComboList )
	{
		return GetColumnComboList( IndexToOrder( nSubItem ), aComboList ); // may be implemented in a derived class
	}
	
	HFONT GetItemFont( int nItem, int nSubItem )
	{
		return m_fntListFont; // may be implemented in a derived class
	}
	
	BOOL GetItemColor( int nItem, int nSubItem, COLORREF& BKColor, COLORREF& TextColor )
	{
		BKColor = mBKColor;
		TextColor = mItemTextColor;
		return TRUE;
	}
	
	CString GetItemToolTip( int nItem, int nSubItem )
	{
		return _T( "" ); // may be implemented in a derived class
	}
	
	BOOL SetItemText( int nItem, int nSubItem, LPCTSTR lpszText )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}

	virtual BOOL SetItemFlags( 
		int nItem, 
		int nSubItem, 
		UINT unFlags)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}

	int GetItemExpandCount(
		int nItem)
	{
		T *pT = static_cast<T*>(this);

		if (nItem > -1)
		{
			CSubItemEx listSubItem;

			if (!pT->GetSubItem(nItem, 0, listSubItem))
				return FALSE;

			return listSubItem.mExpandList.GetSize();
		}else
		{
			int nCount = 0;

			for (int nLoop = 0; nLoop < pT->GetItemCount(); nLoop ++)
			{
				UINT unFormat = GetItemFlags(nLoop, 0);
				if (unFormat == ITEM_FLAGS_HIDE)
					nCount ++;
			}

			return nCount;
		}
	}

	BOOL IsItemHide(
		int nItem)
	{
		T *pT = static_cast<T*>(this);

		CSubItemEx listSubItem;

		if (!pT->GetSubItem(nItem, 0, listSubItem))
			return FALSE;

		if (listSubItem.mExpandList.GetSize() == 0)
			return FALSE;

		UINT unFlags = pT->GetItemFlags(listSubItem.mExpandList[0], 0);
		if (unFlags == ITEM_FLAGS_HIDE)
			return TRUE;
		else
			return FALSE;
	}

	BOOL GetItemExpandList(
		int nItem,
		CListArray<int> &ExpandList)
	{
		T* pT = static_cast<T*>(this);

		CSubItemEx listSubItem;

		if (!pT->GetSubItem(nItem, 0, listSubItem))
			return FALSE;

		ExpandList = listSubItem.mExpandList;

		return TRUE;
	}

	BOOL SetItemComboIndex( int nItem, int nSubItem, int nIndex )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL SetItemDate( int nItem, int nSubItem, SYSTEMTIME& stItemDate )
	{
		T* pT = static_cast<T*>(this);
		
		// set date-time in format (yyyymmddhhmmss)
		CString strFormatDate;
		strFormatDate.Format( _T( "%04d%02d%02d%02d%02d%02d" ), stItemDate.wYear, stItemDate.wMonth, stItemDate.wDay, stItemDate.wHour, stItemDate.wMinute, stItemDate.wSecond );
		
		return pT->SetItemText( nItem, nSubItem, strFormatDate );
	}
	
	BOOL SetItemImage( int nItem, int nSubItem, int nImage )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL SetItemFormat( int nItem, int nSubItem, UINT nFormat, UINT nFlags = ITEM_FLAGS_NONE )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL SetItemFormat( int nItem, int nSubItem, UINT nFormat, UINT nFlags, CListArray < CString >& aComboList )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL SetItemFont( int nItem, int nSubItem, HFONT hFont )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL SetItemColor( int nItem, int nSubItem, COLORREF BKColor, COLORREF TextColor )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
		
	void ReverseItems()
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
	}
	
	void SortItems( int nColumn, BOOL bAscending )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
	}
	
	BOOL GetItemRect( int nItem, int nSubItem, CRect& rcItem )
	{
		T* pT = static_cast<T*>(this);
		
		int nTopItem = GetTopItem();		
		if ( nItem < nTopItem || nItem >= pT->GetItemCount() || nItem >= nTopItem + GetCountPerPage() )
			return FALSE;
		
		CRect rcClient;
		GetClientRect( rcClient );
		
		// calculate item rect based on scroll position
		rcItem = rcClient;
		rcItem.top = ( mbShowHeader ? mnHeaderHeight : 0 ) + ( ( nItem - nTopItem ) * mnItemHeight );
		rcItem.bottom = rcItem.top + mnItemHeight;
		rcItem.right = min( rcClient.right, GetTotalWidth() );
		
		if ( nSubItem != NULL_SUBITEM )
		{
			CRect rcColumn;
			if ( !GetColumnRect( nSubItem, rcColumn ) )
				return FALSE;

			rcItem.left = rcColumn.left;
			rcItem.right = rcColumn.right;
		}
		
		return TRUE;
	}
	
	BOOL GetItemRect( int nItem, CRect& rcItem )
	{
		return GetItemRect( nItem, NULL_SUBITEM, rcItem );
	}
	
	BOOL InvalidateItem( int nItem, int nSubItem = NULL_SUBITEM )
	{
		CRect rcItem;
		return GetItemRect( nItem, nSubItem, rcItem ) ? InvalidateRect( rcItem ) : FALSE;
	}

	BOOL InvalidateHeader()
	{
		if ( !mbShowHeader )
			return TRUE;
		CRect rcHeader;
		if ( !GetClientRect( rcHeader ) )
			return FALSE;
		rcHeader.bottom = mnHeaderHeight;
		return InvalidateRect( rcHeader );	
	}
	
	int GetTopItem()
	{
		return (int)( GetScrollPos( SB_VERT ) / mnItemHeight );
	}
	
	int GetCountPerPage( BOOL bPartial = TRUE )
	{
		CRect rcClient;
		GetClientRect( rcClient );
		rcClient.top = ( mbShowHeader ? mnHeaderHeight : 0 );
		
		// calculate number of items per control height (include partial item)
		div_t divHeight = div( rcClient.Height(), mnItemHeight );
			
		// round up to nearest item count
		return max( bPartial && divHeight.rem > 0 ? divHeight.quot + 1 : divHeight.quot, 1 );
	}
	
	BOOL IsItemVisible( int nItem, int nSubItem = NULL_SUBITEM, BOOL bPartial = TRUE )
	{
		T* pT = static_cast<T*>(this);
		
		int nTopItem = GetTopItem();
		if ( nItem < nTopItem || nItem >= pT->GetItemCount() )
			return FALSE;
		
		// check whether item is visible
		if ( nItem < nTopItem || nItem >= nTopItem + GetCountPerPage( bPartial ) )
			return FALSE;
		
		// check whether subitem is visible
		if ( m_bFocusSubItem && nSubItem != NULL_SUBITEM )
		{
			CRect rcColumn;
			if ( !GetColumnRect( nSubItem, rcColumn ) )
				return FALSE;
			
			CRect rcClient;
			GetClientRect( rcClient );
			
			if ( rcColumn.left < rcClient.left || rcColumn.right > rcClient.right )
				return FALSE;
		}
		
		return TRUE;
	}
	
	BOOL EnsureItemVisible( int nItem, int nSubItem = NULL_SUBITEM )
	{
		if ( IsItemVisible( nItem, nSubItem, FALSE ) )
			return TRUE;
		
		HideTitleTip();
		
		CRect rcClient;
		GetClientRect( rcClient );
		rcClient.top = ( mbShowHeader ? mnHeaderHeight : 0 );
		
		CRect rcItem;
		rcItem.top = ( mbShowHeader ? mnHeaderHeight : 0 ) + ( ( nItem - GetTopItem() ) * mnItemHeight );
		rcItem.bottom = rcItem.top + mnItemHeight;
		
		if ( rcItem.top < rcClient.top || rcItem.bottom > rcClient.bottom )
		{
			int nScrollItem = NULL_ITEM;
			
			// scroll list up/down to include item
			if ( rcItem.top < rcClient.top || rcItem.Height() > rcClient.Height() )
				nScrollItem = nItem;
			else if ( rcItem.bottom > rcClient.bottom )
				nScrollItem = nItem - ( GetCountPerPage( FALSE ) - 1 );
			
			if ( nScrollItem != NULL_ITEM )
				SetScrollPos( SB_VERT, nScrollItem * mnItemHeight );
		}
		
		if ( m_bFocusSubItem && nSubItem != NULL_SUBITEM )
		{
			CRect rcColumn;
			if ( !GetColumnRect( nSubItem, rcColumn ) )
				return FALSE;
			
			CRect rcClient;
			GetClientRect( rcClient );

			int nScrollPos = 0;

			// scroll list left/right to include subitem
			if ( rcColumn.Width() > rcClient.Width() || rcColumn.left < 0 )
				nScrollPos = rcColumn.left;
			else if ( rcColumn.right > rcClient.right )
				nScrollPos = rcColumn.right - rcClient.right;

			if ( nScrollPos != 0 )
				SetScrollPos( SB_HORZ, GetScrollPos( SB_HORZ ) + nScrollPos );
		}
		
		return Invalidate();
	}
	
	void ShowScrollBar( int nScrollBar, BOOL bShow = TRUE )
	{
		switch ( nScrollBar )
		{
			case SB_HORZ:	
				m_bShowHorizScroll = bShow;
				break;

			case SB_VERT:	
				m_bShowVertScroll = bShow;
				break;

			case SB_BOTH:	
				m_bShowHorizScroll = bShow;
				m_bShowVertScroll = bShow;
				break;
		}
		
		ResetScrollBars();
		Invalidate();
	}
	
	void ResetScrollBars( int nScrollBar = SB_BOTH, int nScrollPos = -1, BOOL bRecalc = TRUE )
	{
		T* pT = static_cast<T*>(this);
		
		CRect rcClient;
		GetClientRect( rcClient );
		
		SCROLLINFO infoScroll;
		infoScroll.cbSize = sizeof( SCROLLINFO );
		infoScroll.fMask = nScrollPos < 0 ? SIF_PAGE | SIF_RANGE : SIF_PAGE | SIF_RANGE | SIF_POS;
		infoScroll.nPos = nScrollPos;
		infoScroll.nMin = 0;
		
		if ( ( nScrollBar == SB_BOTH || nScrollBar == SB_VERT ) && m_bShowVertScroll )
		{
			infoScroll.nMax = ( pT->GetItemCount() * mnItemHeight ) + ( mbShowHeader ? mnHeaderHeight : 0 );
			infoScroll.nPage = rcClient.Height() - ( mbShowHeader ? mnHeaderHeight : 0 );
			
			// are we within client range?
			if ( (UINT)infoScroll.nMax <= infoScroll.nPage + ( mbShowHeader ? mnHeaderHeight : 0 ) )
				infoScroll.nMax = 0;
				    
			// set vertical scroll bar
			m_bEnableVertScroll = SetScrollInfo( SB_VERT, &infoScroll, TRUE ) ? ( infoScroll.nMax > 0 ) : FALSE;
		}
		
		if ( ( nScrollBar == SB_BOTH || nScrollBar == SB_HORZ ) && m_bShowHorizScroll )
		{
			infoScroll.nMax = GetTotalWidth( bRecalc );
			infoScroll.nPage = rcClient.Width();
			
			// are we within client range?
			if ( infoScroll.nPage >= (UINT)infoScroll.nMax )
				infoScroll.nMax = 0;
				
			// set horizontal scroll bar
			m_bEnableHorizScroll = SetScrollInfo( SB_HORZ, &infoScroll, TRUE ) ? ( infoScroll.nMax > (int)infoScroll.nPage ) : FALSE;
		}
	}

	BOOL IsScrollBarVisible( int nScrollBar )
	{
		switch ( nScrollBar )
		{
			case SB_HORZ:	
				return m_bEnableHorizScroll;

			case SB_VERT:	
				return m_bEnableVertScroll;

			case SB_BOTH:	
				return ( m_bEnableHorizScroll && m_bEnableVertScroll );

			default:		
				return FALSE;
		}
	}
	
	BOOL ResetSelected()
	{
		m_setSelectedItems.clear();
		m_nFocusItem = NULL_ITEM;
		m_nFocusSubItem = NULL_SUBITEM;
		m_nFirstSelected = NULL_ITEM;
		return Invalidate();
	}
	
	BOOL SelectItem( int nItem, int nSubItem = NULL_SUBITEM, UINT nFlags = 0 )
	{
		T* pT = static_cast<T*>(this);
		
		if ( nItem < 0 || nItem >= pT->GetItemCount() )
			return FALSE;
		
		BOOL bSelectItem = TRUE;
		BOOL bSelectRange = !m_bSingleSelect && ( nFlags & MK_SHIFT );
		BOOL bNewSelect = !( bSelectRange || ( nFlags & MK_CONTROL ) );
		BOOL bEnsureVisible = FALSE;

		// are we starting a new select sequence?
		if ( bNewSelect || bSelectRange )
		{
			// are we simply reselecting the same item?
			if ( m_setSelectedItems.size() == 1 && *m_setSelectedItems.begin() == nItem )
			{
				bSelectItem = FALSE;
				m_nFirstSelected = nItem;
				m_nFocusItem = nItem;
				m_nFocusSubItem = nSubItem;
			}
			else
				m_setSelectedItems.clear();
		}
		else // we adding to or removing from select sequence
		{
			if ( m_bSingleSelect )
				m_setSelectedItems.clear();
			
			set < int >::iterator posSelectedItem = m_setSelectedItems.find( nItem );
			
			// is this item already selected?
			if ( posSelectedItem != m_setSelectedItems.end() )
			{
				bSelectItem = FALSE;
				m_setSelectedItems.erase( posSelectedItem );				
				m_nFirstSelected = nItem;
				m_nFocusItem = nItem;
				m_nFocusSubItem = m_setSelectedItems.size() > 1 ? NULL_SUBITEM : nSubItem;
			}
		}
		
		// are we adding this item to the select sequence?
		if ( bSelectItem )
		{
			bEnsureVisible = TRUE;
			
			if ( bSelectRange )
			{
				if ( m_nFirstSelected == NULL_ITEM )
					m_nFirstSelected = nItem;
					
				for ( int nSelectedItem = min( m_nFirstSelected, nItem ); nSelectedItem <= max( m_nFirstSelected, nItem ); nSelectedItem++ )
					m_setSelectedItems.insert( nSelectedItem );
			}
			else
			{
				m_nFirstSelected = nItem;
				m_setSelectedItems.insert( nItem );
			}
			
			m_nFocusItem = nItem;
			m_nFocusSubItem = m_setSelectedItems.size() > 1 ? NULL_SUBITEM : nSubItem;
			
			// notify parent of selected item
			NotifyParent( m_nFocusItem, m_nFocusSubItem, LCN_SELECTED );
		}
		
		// start visible timer (scrolls list to partially hidden item)
		if ( !IsItemVisible( nItem, m_setSelectedItems.size() > 1 ? NULL_SUBITEM : nSubItem, FALSE ) )
			SetTimer( ITEM_VISIBLE_TIMER, ITEM_VISIBLE_PERIOD );
		else if ( m_nFocusItem != NULL_ITEM && m_nFocusSubItem != NULL_SUBITEM )
			EditItem( m_nFocusItem, m_nFocusSubItem );

		return Invalidate();
	}
	
	BOOL IsSelected( int nItem )
	{
		set < int >::iterator posSelectedItem = m_setSelectedItems.find( nItem );
		return ( posSelectedItem != m_setSelectedItems.end() );
	}
	
	BOOL GetSelectedItems( CListArray < int >& aSelectedItems )
	{
		aSelectedItems.RemoveAll();
		for ( set < int >::iterator posSelectedItem = m_setSelectedItems.begin(); posSelectedItem != m_setSelectedItems.end(); ++posSelectedItem )
			aSelectedItems.Add( *posSelectedItem );
		return !aSelectedItems.IsEmpty();
	}
	
	BOOL SetFocusItem( int nItem, int nSubItem = NULL_SUBITEM )
	{
		m_nFocusItem = nItem;
		m_nFocusSubItem = nSubItem;		
		return EnsureItemVisible( m_nFocusItem, m_nFocusSubItem );
	}

	BOOL GetFocusItem( int& nItem, int& nSubItem )
	{
		nItem = IsSelected( m_nFocusItem ) ? m_nFocusItem : ( m_setSelectedItems.empty() ? NULL_ITEM : *m_setSelectedItems.begin() );
		nSubItem = !m_bFocusSubItem || nItem == NULL_ITEM ? NULL_SUBITEM : m_nFocusSubItem;
		return ( nItem != NULL_ITEM );
	}
	
	int GetFocusItem()
	{
		return IsSelected( m_nFocusItem ) ? m_nFocusItem : ( m_setSelectedItems.empty() ? NULL_ITEM : *m_setSelectedItems.begin() );
	}
	
	BOOL HitTestHeader( CPoint point, int& nColumn, UINT& nFlags )
	{
		// reset hittest flags
		nFlags = HITTEST_FLAG_NONE;
		
		if ( !mbShowHeader )
			return FALSE;
		
		CRect rcClient;
		if ( !GetClientRect( rcClient ) )
			return FALSE;
		
		// are we over the header?
		if ( point.y < rcClient.top || point.y > mnHeaderHeight )
			return FALSE;
		
		int nDividerPos = 0;
		int nColumnCount = GetColumnCount();
	
		// get hit-test subitem
		for ( nColumn = 0; nColumn < nColumnCount; nColumn++ )
		{
			int nColumnWidth = GetColumnWidth( nColumn );
			nDividerPos += nColumnWidth;

			// offset divider position with current scroll position
			int nRelativePos = nDividerPos - GetScrollPos( SB_HORZ );

			// are we over the divider zone?
			if ( point.x >= nRelativePos - DRAG_HEADER_OFFSET - 1 && point.x <= nRelativePos + DRAG_HEADER_OFFSET )
			{
				nFlags |= HITTEST_FLAG_HEADER_DIVIDER;
				
				// are we to the left of the divider (or over last column divider)?
				if ( ( point.x >= nRelativePos - DRAG_HEADER_OFFSET - 1 && point.x < nRelativePos ) || nColumn + 1 >= nColumnCount - 1 )
				{
					nFlags |= HITTEST_FLAG_HEADER_LEFT;
					return TRUE;
				}

				// find last zero-length column after this column
				for ( int nNextColumn = nColumn + 1; nNextColumn < nColumnCount; nNextColumn++ )
				{
					if ( GetColumnWidth( nNextColumn ) > 0 )
						break;
					nColumn = nNextColumn;
				}
				
				nFlags |= HITTEST_FLAG_HEADER_RIGHT;

				return TRUE;
			}

			// are we over a column?
			if ( point.x > nRelativePos - nColumnWidth && point.x < nRelativePos )
				return TRUE;
		}	
		
		return FALSE;
	}
	
	BOOL HitTest( 
		CPoint point, 
		int &nItem, 
		int &nSubItem)
	{
		T *pT = static_cast<T *>(this);
		
		if (point.y < (mbShowHeader ? mnHeaderHeight : 0))
			return FALSE;
		
		nItem = GetTopItem() + (int)((point.y - (mbShowHeader ? mnHeaderHeight : 0)) / mnItemHeight);

		if (nItem < 0 || nItem >= pT->GetItemCount())
			return FALSE;
		
		UINT unFlags = GetItemFlags(nItem, 0);
		if (unFlags & ITEM_FLAGS_HIDE)
			return FALSE;

		int nTotalWidth = 0;
		int nColumnCount = GetColumnCount();
	
		for (nSubItem = 0; nSubItem < nColumnCount; nSubItem ++)
		{
			int nColumnWidth = GetColumnWidth(nSubItem);
			nTotalWidth += nColumnWidth;

			int nRelativePos = nTotalWidth - GetScrollPos(SB_HORZ);

			if ( point.x > nRelativePos - nColumnWidth && point.x < nRelativePos )
				return TRUE;
		}
		
		return FALSE;
	}
	
	BOOL AutoSizeColumn( int nColumn )
	{
		T* pT = static_cast<T*>(this);
		
		CListColumnEx listColumn;
		if ( !GetColumn( nColumn, listColumn ) || listColumn.m_bFixed )
			return FALSE;
			
		CClientDC dcClient( m_hWnd );
		HFONT hOldFont = dcClient.SelectFont( m_fntListFont );
		
		// set to column text width if zero-length
		CSize sizeExtent;
		if ( !dcClient.GetTextExtent( listColumn.m_strText, -1, &sizeExtent ) )
			return FALSE;
		
		int nMaxWidth = sizeExtent.cx + ITEM_WIDTH_MARGIN;
		
		CSize sizeIcon = 0;
		if ( !m_ilItemImages.IsNull() )
			m_ilItemImages.GetIconSize( sizeIcon );
		
		// calculate maximum column width required
		for ( int nItem = 0; nItem < pT->GetItemCount(); nItem++ )
		{
			if ( !dcClient.GetTextExtent( pT->GetItemText( nItem, listColumn.m_nIndex ), -1, &sizeExtent ) )
				return FALSE;
			
			if ( !m_ilItemImages.IsNull() && pT->GetItemImage( nItem, listColumn.m_nIndex ) != ITEM_IMAGE_NONE )
				sizeExtent.cx += sizeIcon.cx;
			
			nMaxWidth = max( nMaxWidth, (int)sizeExtent.cx + ITEM_WIDTH_MARGIN );
		}
		
		dcClient.SelectFont( hOldFont );
		
		return SetColumnWidth( nColumn, nMaxWidth );	
	}
	
	void ResizeColumn( BOOL bColumnScroll = FALSE )
	{
		HideTitleTip();
		
		int nCurrentPos = GET_X_LPARAM( GetMessagePos() );
		
		CRect rcClient;
		GetClientRect( rcClient );
		int nScrollLimit = GetTotalWidth() - rcClient.Width();
		
		if ( bColumnScroll )
		{
			// have we finished scrolling list to accommodate new column size?
			if ( !m_bColumnSizing || !m_bEnableHorizScroll || nCurrentPos - m_nStartScrollPos > 0 )
			{
				KillTimer( RESIZE_COLUMN_TIMER );
				
				// reset resize start point
				m_nStartPos = nCurrentPos;
				m_bResizeTimer = FALSE;
			}
			else if ( nCurrentPos < m_nStartPos && GetScrollPos( SB_HORZ ) >= nScrollLimit )
			{
				// reset start column size
				m_nStartSize = max( GetColumnWidth( m_nColumnSizing ) + ( nCurrentPos - m_nStartScrollPos ), 0 );
				
				// resize column
				SetColumnWidth( m_nColumnSizing, m_nStartSize );
			}
		}
		else
		{
			int nColumnSize = max( m_nStartSize + ( nCurrentPos - m_nStartPos ), 0 );
			
			// are we scrolled fully to the right and wanting to reduce the size of a column?
			if ( m_bEnableHorizScroll && GetScrollPos( SB_HORZ ) >= nScrollLimit && nColumnSize < GetColumnWidth( m_nColumnSizing ) )
			{
				if ( !m_bResizeTimer )
				{
					// only start the scroll timer once
					m_bResizeTimer = TRUE;

					// set new start scroll position
					m_nStartScrollPos = nCurrentPos;

					// start column resize / scroll timer
					SetTimer( RESIZE_COLUMN_TIMER, RESIZE_COLUMN_PERIOD );
				}
			}
			else
			{
				// resizing is done in scroll timer (if started)
				if ( !m_bResizeTimer )
					SetColumnWidth( m_nColumnSizing, nColumnSize );
			}
		}
	}
	
	void DragColumn()
	{
		HideTitleTip();
		
		CRect rcColumn;
		if ( !GetColumnRect( m_nHighlightColumn, rcColumn ) )
			return;
		
		CRect HeaderItemRect( rcColumn );
		HeaderItemRect.MoveToXY( 0, 0 );
		
		CListColumnEx listColumn;
		if ( !GetColumn( m_nHighlightColumn, listColumn ) )
			return;
		
		// store drag column
		m_nDragColumn = m_nHighlightColumn;
		
		CClientDC dcClient( m_hWnd );
		
		CDC HeaderDC;
		HeaderDC.CreateCompatibleDC( dcClient );
		
		int nContextState = HeaderDC.SaveDC();
		
		// create drag header bitmap
		CBitmapHandle bmpHeader;
		bmpHeader.CreateCompatibleBitmap( dcClient, HeaderItemRect.Width(), HeaderItemRect.Height() );
		HeaderDC.SelectBitmap( bmpHeader );
		
		if ( mbShowThemed && !mHeaderTheme.IsThemeNull() )
			mHeaderTheme.DrawThemeBackground( HeaderDC, HP_HEADERITEM, HIS_PRESSED, HeaderItemRect, NULL );
		else
		{
			HeaderDC.SetBkColor( mHeaderBKColor );
			HeaderDC.ExtTextOut( HeaderItemRect.left, HeaderItemRect.top, ETO_OPAQUE, HeaderItemRect, _T( "" ), 0, NULL );

			CBrush FrameBrush;

			FrameBrush.CreateSolidBrush(mHeaderBorderColor);
			HeaderDC.FrameRect(HeaderItemRect, FrameBrush);
		}
		
		CRect rcHeaderText( HeaderItemRect );
		rcHeaderText.left += m_nHighlightColumn == 0 ? 4 : 3;
		rcHeaderText.OffsetRect( 0, 1 );
		
		// margin header text
		rcHeaderText.DeflateRect( 4, 0, 5, 0 );
		
		// has this header item an associated image?
		if ( listColumn.m_nImage != ITEM_IMAGE_NONE )
		{
			CSize sizeIcon;
			mCtrlImageList.GetIconSize( sizeIcon );
			
			CRect rcHeaderImage;
			rcHeaderImage.left = listColumn.m_strText.IsEmpty() ? ( ( rcHeaderText.left + rcHeaderText.right ) / 2 ) - ( sizeIcon.cx / 2 ) - ( ( !mbShowThemed || mHeaderTheme.IsThemeNull() ) ? 0 : 1 ) : rcHeaderText.left;
			rcHeaderImage.right = min( rcHeaderImage.left + sizeIcon.cx, HeaderItemRect.right );
			rcHeaderImage.top = ( ( HeaderItemRect.top + HeaderItemRect.bottom ) / 2 ) - ( sizeIcon.cy / 2 );
			rcHeaderImage.bottom = min( rcHeaderImage.top + sizeIcon.cy, HeaderItemRect.bottom );
				
			mCtrlImageList.DrawEx( listColumn.m_nImage, HeaderDC, rcHeaderImage, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT );

			// offset header text (for image)
			rcHeaderText.left += sizeIcon.cx + 4;
		}
		
		HeaderDC.SelectFont( m_fntListFont );
		HeaderDC.SetTextColor( mHeaderTextColor );
		HeaderDC.SetBkMode( TRANSPARENT );

		UINT nFormat = DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS;

		if ( listColumn.m_nFlags & ITEM_FLAGS_CENTRE )
			nFormat |= DT_CENTER;
		else if ( listColumn.m_nFlags & ITEM_FLAGS_RIGHT )
			nFormat |= DT_RIGHT;
		else
			nFormat |= DT_LEFT;
			
		// draw header text
		if ( !listColumn.m_strText.IsEmpty() )
			HeaderDC.DrawText( listColumn.m_strText, listColumn.m_strText.GetLength(), rcHeaderText, nFormat );

		HeaderDC.RestoreDC( nContextState );
		
		SHDRAGIMAGE shDragImage;
		ZeroMemory( &shDragImage, sizeof( SHDRAGIMAGE ) );
		
		shDragImage.sizeDragImage.cx = HeaderItemRect.Width();
		shDragImage.sizeDragImage.cy = HeaderItemRect.Height();
		shDragImage.ptOffset.x = rcColumn.Width() / 2;
		shDragImage.ptOffset.y = rcColumn.Height() / 2;
		shDragImage.hbmpDragImage = bmpHeader;
		shDragImage.crColorKey = mBKColor;
		
		// start header drag operation
		//m_oleDragDrop.DoDragDrop( &shDragImage, DROPEFFECT_MOVE );
		
		// hide drop arrows after moving column
		m_wndDropArrows.Hide();
		
		if ( m_bButtonDown )
		{
			ReleaseCapture();
			m_bButtonDown = FALSE;
			m_bBeginSelect = FALSE;
			m_ptDownPoint = 0;
			m_ptSelectPoint = 0;
		}
		
		// finish moving a column
		if ( m_nHighlightColumn != NULL_COLUMN )
		{
			m_nHighlightColumn = NULL_COLUMN;
			InvalidateHeader();
		}
		
		m_nDragColumn = NULL_COLUMN;
	}
	
	BOOL DropColumn( CPoint point )
	{
		if ( !mbShowHeader )
			return FALSE;
			
		m_nHotDivider = NULL_COLUMN;
		m_nHotColumn = NULL_COLUMN;
		UINT nHeaderFlags = HITTEST_FLAG_NONE;
		
		// are we over the header?
		if ( HitTestHeader( point, m_nHotColumn, nHeaderFlags ) )
		{
			CRect rcColumn;
			if ( !GetColumnRect( m_nHotColumn, rcColumn ) )
				return FALSE;
			m_nHotDivider = point.x < ( ( rcColumn.left + rcColumn.right ) / 2 ) ? m_nHotColumn : m_nHotColumn + 1;
			
			if ( m_nHotDivider == m_nDragColumn || m_nHotDivider == m_nDragColumn + 1 )
				m_nHotDivider = NULL_COLUMN;
		}
		
		if ( m_nHotDivider != NULL_COLUMN )
		{
			CRect rcHeader;
			GetClientRect( rcHeader );
			rcHeader.bottom = mnHeaderHeight;
		
			CPoint ptDivider( 0, rcHeader.Height() / 2 );
			
			CRect rcColumn;
			int nColumnCount = GetColumnCount();
			
			// set closest divider position
			if ( GetColumnRect( m_nHotDivider < nColumnCount ? m_nHotDivider : nColumnCount - 1, rcColumn ) )
				ptDivider.x = m_nHotDivider < nColumnCount ? rcColumn.left : rcColumn.right;
			
			ClientToScreen( &ptDivider );
			
			// track drop window
			m_wndDropArrows.Show( ptDivider );
			return TRUE;
		}
		
		m_wndDropArrows.Hide();
		
		return FALSE;
	}
	
	BOOL SortColumn( int nColumn )
	{
		T* pT = static_cast<T*>(this);
		
		if ( !mbShowHeader || !m_bShowSort )
			return FALSE;
			
		BOOL bReverseSort = FALSE;
		int nSortIndex = GetColumnIndex( nColumn );
		
		CWaitCursor curWait;
		
		if ( nSortIndex != m_nSortColumn )
		{
			// sort by new column
			m_bSortAscending = TRUE;
			m_nSortColumn = nSortIndex;
			pT->SortItems( m_nSortColumn, m_bSortAscending );
		}
		else
		{
			// toggle sort order if sorting same column
			m_bSortAscending = !m_bSortAscending;
			pT->ReverseItems();
		}
			
		return ResetSelected();
	}
	
	BOOL GetSortColumn( int& nColumn, BOOL& bAscending )
	{
		if ( !mbShowHeader || !m_bShowSort || m_nSortColumn == NULL_COLUMN )
			return FALSE;
		nColumn = m_nSortColumn;
		bAscending = m_bSortAscending;
		return TRUE;
	}
	
	BOOL DragItem()
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
		return FALSE;
	}
	
	BOOL GroupSelect( CPoint point )
	{
		HideTitleTip();
		
		int nHorzScroll = GetScrollPos( SB_HORZ );
		int nVertScroll = GetScrollPos( SB_VERT );
		
		m_rcGroupSelect.left = min( m_ptSelectPoint.x, point.x + nHorzScroll );
		m_rcGroupSelect.right = max( m_ptSelectPoint.x, point.x + nHorzScroll );
		m_rcGroupSelect.top = min( m_ptSelectPoint.y, point.y + nVertScroll );
		m_rcGroupSelect.bottom = max( m_ptSelectPoint.y, point.y + nVertScroll );
		
		if ( m_rcGroupSelect.IsRectEmpty() )
			return FALSE;
		
		// select items in group
		AutoSelect( point );
		
		// start auto scroll timer
		SetTimer( ITEM_AUTOSCROLL_TIMER, ITEM_SCROLL_PERIOD );
		
		DWORD dwCurrentTick = GetTickCount();
		
		// timer messages are a low priority, therefore we need to simulate the timer when moving the mouse
		if ( ( dwCurrentTick - m_dwScrollTick ) > ITEM_SCROLL_PERIOD - 10 )
		{
			if ( AutoScroll( point ) )
				m_dwScrollTick = dwCurrentTick;
		}
		
		// redraw list immediately
		return RedrawWindow();
	}
	
	void AutoSelect( CPoint point )
	{
		m_setSelectedItems.clear();
			
		if ( m_rcGroupSelect.left < GetTotalWidth() )
		{
			int nHorzScroll = GetScrollPos( SB_HORZ );
			int nVertScroll = GetScrollPos( SB_VERT );
			
			CRect rcGroupSelect( m_rcGroupSelect );
			rcGroupSelect.OffsetRect( -nHorzScroll, -nVertScroll );
		
			int nTopItem = GetTopItem();
			int nLastItem = nTopItem + ( ( rcGroupSelect.bottom - ( mbShowHeader ? mnHeaderHeight : 0 ) ) / mnItemHeight );
			nTopItem += ( ( rcGroupSelect.top - ( mbShowHeader ? mnHeaderHeight : 0 ) ) / mnItemHeight ) - ( ( rcGroupSelect.top < 0 ) ? 1 : 0 );
			
			for ( int nItem = nTopItem; nItem <= nLastItem; nItem++ )
			{
				if ( m_setSelectedItems.empty() )
					m_nFirstSelected = nItem;
				m_setSelectedItems.insert( nItem );
				
				m_nFocusItem = nItem;
				m_nFocusSubItem = NULL_SUBITEM;
			}
		}

		if ( m_setSelectedItems.empty() )
			m_nFirstSelected = NULL_ITEM;
	}
	
	BOOL AutoScroll( CPoint point )
	{
		CRect rcClient;
		GetClientRect( rcClient );
		rcClient.top = ( mbShowHeader ? mnHeaderHeight : 0 );
		
		int nHorzScroll = GetScrollPos( SB_HORZ );
		int nVertScroll = GetScrollPos( SB_VERT );

		BOOL bAutoScroll = FALSE;
		
		if ( point.y < rcClient.top )
		{
			SendMessage( WM_VSCROLL, MAKEWPARAM( SB_LINEUP, 0 ) );
			int nAutoScroll = GetScrollPos( SB_VERT );
			if ( nVertScroll != nAutoScroll )
			{
				m_rcGroupSelect.top = rcClient.top + nAutoScroll - 1;
				m_rcGroupSelect.bottom = max( m_ptSelectPoint.y, point.y + nAutoScroll - 1 );
				bAutoScroll = TRUE;
			}
		}
		if ( point.y > rcClient.bottom )
		{
			SendMessage( WM_VSCROLL, MAKEWPARAM( SB_LINEDOWN, 0 ) );
			int nAutoScroll = GetScrollPos( SB_VERT );
			if ( nVertScroll != nAutoScroll )
			{
				m_rcGroupSelect.top = min( m_ptSelectPoint.y, point.y + nAutoScroll + 1 );
				m_rcGroupSelect.bottom = rcClient.bottom + nAutoScroll + 1;
				bAutoScroll = TRUE;
			}
		}
		if ( point.x < rcClient.left )
		{
			SendMessage( WM_HSCROLL, MAKEWPARAM( SB_LINELEFT, 0 ) );
			int nAutoScroll = GetScrollPos( SB_HORZ );
			if ( nHorzScroll != nAutoScroll )
			{
				m_rcGroupSelect.left = rcClient.left + nAutoScroll - 1;
				m_rcGroupSelect.right = max( m_ptSelectPoint.x, point.x + nAutoScroll - 1 );
				bAutoScroll = TRUE;
			}
		}
		if ( point.x > rcClient.right )
		{
			SendMessage( WM_HSCROLL, MAKEWPARAM( SB_LINERIGHT, 0 ) );
			int nAutoScroll = GetScrollPos( SB_HORZ );
			if ( nHorzScroll != nAutoScroll )
			{
				m_rcGroupSelect.left = min( m_ptSelectPoint.x, point.x + nAutoScroll + 1 );
				m_rcGroupSelect.right = rcClient.right + nAutoScroll + 1;
				bAutoScroll = TRUE;
			}
		}
		
		// was scrolling performed?
		return bAutoScroll;
	}
	
	BOOL BeginScroll( int nBeginItem, int nEndItem )
	{
		T* pT = static_cast<T*>(this);
		
		// any scroll required?
		if ( nBeginItem == nEndItem )
			return FALSE;
		
		// calculate scroll offset
		m_nScrollOffset = abs( nEndItem - nBeginItem ) * mnItemHeight;
		m_nScrollUnit = min( max( m_nScrollOffset / mnItemHeight, ITEM_SCROLL_UNIT_MIN ), ITEM_SCROLL_UNIT_MAX );
		m_nScrollDelta = ( m_nScrollOffset - m_nScrollUnit ) / m_nScrollUnit;
		m_bScrollDown = ( nBeginItem < nEndItem );
		
		CClientDC dcClient( m_hWnd );
		
		CDC dcScrollList;
		dcScrollList.CreateCompatibleDC( dcClient );
		
		int nContextState = dcScrollList.SaveDC();
		
		CRect rcScrollList;
		GetClientRect( rcScrollList );
		rcScrollList.bottom = ( GetCountPerPage() + abs( nEndItem - nBeginItem ) ) * mnItemHeight;
		
		if ( !m_bmpScrollList.IsNull() )
			m_bmpScrollList.DeleteObject();
		m_bmpScrollList.CreateCompatibleBitmap( dcClient, rcScrollList.Width(), rcScrollList.Height() ); 
		dcScrollList.SelectBitmap( m_bmpScrollList );
		
		pT->DrawBkgnd( dcScrollList.m_hDC );
		
		CRect rcItem;
		rcItem.left = -GetScrollPos( SB_HORZ );
		rcItem.right = GetTotalWidth();
		rcItem.top = 0;
		rcItem.bottom = rcItem.top;
		
		// draw all visible items into bitmap
		for ( int nItem = min( nBeginItem, nEndItem ); nItem < pT->GetItemCount(); rcItem.top = rcItem.bottom, nItem++ )
		{
			rcItem.bottom = rcItem.top + mnItemHeight;
			
			if ( rcItem.top > rcScrollList.bottom )
				break;
			
			// may be implemented in a derived class
			pT->DrawItem( dcScrollList.m_hDC, nItem, rcItem );
		}
		
		dcScrollList.RestoreDC( nContextState );
		
		ScrollList();
		
		// start scrolling timer
		SetTimer( ITEM_SCROLL_TIMER, ITEM_SCROLL_PERIOD );
		
		return TRUE;
	}
	
	BOOL EndScroll()
	{
		KillTimer( ITEM_SCROLL_TIMER );
		if ( !m_bmpScrollList.IsNull() )
			m_bmpScrollList.DeleteObject();
		return Invalidate();
	}
	
	BOOL ScrollList()
	{
		if ( m_nScrollOffset <= m_nScrollUnit )
			m_nScrollOffset--;
		else
		{
			m_nScrollOffset -= m_nScrollDelta;
			if ( m_nScrollOffset < m_nScrollDelta )
				m_nScrollOffset = m_nScrollUnit;
		}
		
		if ( m_bmpScrollList.IsNull() || m_nScrollOffset < 0 )
			return FALSE;
		
		CClientDC dcClient( m_hWnd );
	
		CDC dcScrollList;
		dcScrollList.CreateCompatibleDC( dcClient );
		
		CRect rcClient;
		GetClientRect( rcClient );
		rcClient.top = ( mbShowHeader ? mnHeaderHeight : 0 );
		
		HBITMAP hOldBitmap = dcScrollList.SelectBitmap( m_bmpScrollList );
		
		CSize sizScrollBitmap;
		m_bmpScrollList.GetSize( sizScrollBitmap );
		
		// draw scrolled list
		dcClient.BitBlt( 0, rcClient.top, rcClient.Width(), rcClient.Height(), dcScrollList, 0, m_bScrollDown ? ( sizScrollBitmap.cy - ( GetCountPerPage() * mnItemHeight ) - m_nScrollOffset ) : m_nScrollOffset, SRCCOPY );

		dcScrollList.SelectBitmap( hOldBitmap );
		
		return TRUE;
	}
	
	BOOL EditItem( int nItem, int nSubItem = NULL_SUBITEM )
	{
		T* pT = static_cast<T*>(this);
		
		if ( !EnsureItemVisible( nItem, nSubItem ) )
			return FALSE;
		
		if ( GetFocus() != m_hWnd )
			return FALSE;
		
		CRect rCSubItemEx;
		if ( !GetItemRect( nItem, nSubItem, rCSubItemEx ) )
			return FALSE;
		
		int nIndex = GetColumnIndex( nSubItem );
		if ( pT->GetItemFlags( nItem, nIndex ) & ITEM_FLAGS_READ_ONLY )
			return TRUE;
		
		switch ( pT->GetItemFormat( nItem, nIndex ) )
		{
			case ITEM_FORMAT_EDIT:				
				m_bEditItem = TRUE;
				if ( !RedrawWindow() )
					return FALSE;

				if ( !m_wndItemEdit.Create( m_hWnd, nItem, nSubItem, rCSubItemEx, pT->GetItemFlags( nItem, nIndex ), pT->GetItemText( nItem, nIndex ) ) )
					return FALSE;

				break;

			case ITEM_FORMAT_DATETIME:			
				{
					m_bEditItem = TRUE;
					if ( !RedrawWindow() )
						return FALSE;

					SYSTEMTIME stItemDate;
					GetItemDate( nItem, nIndex, stItemDate );
					if ( !m_wndItemDate.Create( m_hWnd, nItem, nSubItem, rCSubItemEx, pT->GetItemFlags( nItem, nIndex ), stItemDate ) )
						return FALSE;
				}
				break;

			case ITEM_FORMAT_COMBO:				
				{
					m_bEditItem = TRUE;
					if ( !RedrawWindow() )
						return FALSE;

					CListArray < CString > aComboList;
					if ( !pT->GetItemComboList( nItem, nIndex, aComboList ) )
						return FALSE;

					if ( !m_wndItemCombo.Create( m_hWnd, nItem, nSubItem, rCSubItemEx, pT->GetItemFlags( nItem, nIndex ), pT->GetItemText( nItem, nIndex ), mbShowThemed, aComboList ) )
						return FALSE;
				}
				break;
		}

		return TRUE;
	}
	
	CString FormatDate( SYSTEMTIME& stFormatDate )
	{
		if ( stFormatDate.wYear == 0 )
			return _T( "" );
		
		// format date to local format
		TCHAR szDateFormat[ DATE_STRING ];
		return GetDateFormat( LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stFormatDate, NULL, szDateFormat, DATE_STRING ) == 0 ? _T( "" ) : szDateFormat;
	}
	
	CString FormatTime( SYSTEMTIME& stFormatDate )
	{
		SYSTEMTIME stFormatTime = stFormatDate;
		stFormatTime.wYear = 0;
		stFormatTime.wMonth = 0;
		stFormatTime.wDay = 0;
		
		// format time to local format
		TCHAR szTimeFormat[ DATE_STRING ];
		return GetTimeFormat( LOCALE_USER_DEFAULT, 0, &stFormatTime, NULL, szTimeFormat, DATE_STRING ) == 0 ? _T( "" ) : szTimeFormat;
	}
	
	void NotifyParent( int nItem, int nSubItem, int nMessage )
	{
		T* pT = static_cast<T*>(this);
		
		CListNotify listNotify;
		listNotify.m_hdrNotify.hwndFrom = pT->m_hWnd;
		listNotify.m_hdrNotify.idFrom = pT->GetDlgCtrlID();
		listNotify.m_hdrNotify.code = nMessage;
		listNotify.m_nItem = nItem;
		listNotify.m_nSubItem = GetColumnIndex( nSubItem );
		listNotify.m_nExitChar = 0;
		listNotify.m_lpszItemText = NULL;
		listNotify.m_lpItemDate = NULL;

		// forward notification to parent
		FORWARD_WM_NOTIFY( pT->GetParent(), listNotify.m_hdrNotify.idFrom, &listNotify.m_hdrNotify, ::SendMessage );
	}
	
	BOOL ShowTitleTip( CPoint point, int nItem, int nSubItem )
	{
		T* pT = static_cast<T*>(this);
		
		// do not show titletip if editing
		if ( m_bEditItem )
			return FALSE;
		
		// is titletip already shown for this item?
		if ( nItem == m_nTitleTipItem && nSubItem == m_nTitleTipSubItem )
			return FALSE;
		
		CRect rCSubItemEx;
		if ( !GetItemRect( nItem, nSubItem, rCSubItemEx ) )
		{
			HideTitleTip();
			return FALSE;
		}
		
		int nIndex = GetColumnIndex( nSubItem );
		CRect rcItemText( rCSubItemEx );
				
		// margin item text
		rcItemText.left += nSubItem == 0 ? 4 : 3;
		rcItemText.DeflateRect( 4, 0 );
		
		// offset item text (for image)
		if ( !m_ilItemImages.IsNull() && pT->GetItemImage( nItem, nIndex ) != ITEM_IMAGE_NONE )
		{
			CSize sizeIcon;
			m_ilItemImages.GetIconSize( sizeIcon );
			rcItemText.left += sizeIcon.cx + 4;
		}
				
		// is current cursor position over item text (not over an icon)?
		if ( !rcItemText.PtInRect( point ) )
			return FALSE;
		
		CString strItemText;
		
		switch ( pT->GetItemFormat( nItem, nIndex ) )
		{
			case ITEM_FORMAT_CHECKBOX:
			case ITEM_FORMAT_CHECKBOX_3STATE:	
			case ITEM_FORMAT_PROGRESS:			
				break; // no titletip for checkboxes or progress

			case ITEM_FORMAT_DATETIME:			
				{
					SYSTEMTIME stItemDate;
					if ( !GetItemDate( nItem, nIndex, stItemDate ) )
						break;
													
					UINT nItemFlags = pT->GetItemFlags( nItem, nIndex );
					if ( nItemFlags & ITEM_FLAGS_DATE_ONLY )
						strItemText = FormatDate( stItemDate );
					else if ( nItemFlags & ITEM_FLAGS_TIME_ONLY )
						strItemText = FormatTime( stItemDate );
					else
						strItemText = FormatDate( stItemDate ) + _T( " " ) + FormatTime( stItemDate );
				}
				break;

			default:
				strItemText = pT->GetItemText( nItem, nIndex );
				break;
		}

		if ( strItemText.IsEmpty() )
		{
			HideTitleTip();
			return FALSE;
		}
		
		ClientToScreen( rcItemText );
		if ( !m_wndTitleTip.Show( rcItemText, strItemText, pT->GetItemToolTip( nItem, nIndex ) ) )
		{
			HideTitleTip();
			return FALSE;
		}
		
		m_nTitleTipItem = nItem;
		m_nTitleTipSubItem = nSubItem;
						
		return TRUE;
	}
	
	BOOL HideTitleTip( BOOL bResetItem = TRUE )
	{
		if ( bResetItem )
		{
			m_nTitleTipItem = NULL_ITEM;
			m_nTitleTipSubItem = NULL_SUBITEM;
		}
		return m_wndTitleTip.Hide();
	}

	BEGIN_MSG_MAP(CListImplEx)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_GETDLGCODE(OnGetDlgCode)
		MSG_WM_SIZE(OnSize)
		MSG_WM_HSCROLL(OnHScroll)
		MSG_WM_VSCROLL(OnVScroll)
		MSG_WM_CANCELMODE(OnCancelMode)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseRange)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MSG_WM_RBUTTONUP(OnRButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_SYSKEYDOWN(OnSysKeyDown)
		MSG_WM_SETTINGCHANGE(OnSettingsChange)
		MSG_WM_SYSCOLORCHANGE(OnSettingsChange)
		MSG_WM_FONTCHANGE(OnSettingsChange)
		MSG_WM_THEMECHANGED(OnSettingsChange)
		NOTIFY_CODE_HANDLER_EX(LCN_ENDEDIT,OnEndEdit)
		MESSAGE_HANDLER(WM_CTLCOLORLISTBOX, OnCtlColorListBox)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnCtlColorListBox)
		CHAIN_MSG_MAP(CDoubleBufferImpl<CListImplEx>)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	int OnCreate( 
		LPCREATESTRUCT lpCreateStruct)
	{
		T* pT = static_cast<T*>(this);
		return pT->Initialise() ? 0 : -1;
	}

	void OnDestroy()
	{
		//m_oleDragDrop.Revoke();
		
		if ( m_wndDropArrows.IsWindow() )
			m_wndDropArrows.DestroyWindow();
		
		if ( m_wndTitleTip.IsWindow() )
			m_wndTitleTip.DestroyWindow();
		
		if ( m_wndItemEdit.IsWindow() )
			m_wndItemEdit.DestroyWindow();
		
		if ( m_wndItemCombo.IsWindow() )
			m_wndItemCombo.DestroyWindow();
		
		if ( m_wndItemDate.IsWindow() )
			m_wndItemDate.DestroyWindow();
		
		if ( m_ttToolTip.IsWindow() )
			m_ttToolTip.DestroyWindow();
	}
	
	void OnSetFocus( HWND hOldWnd )
	{
		Invalidate();
	}
	
	void OnKillFocus( HWND hNewWnd )
	{
		Invalidate();
	}
	
	UINT OnGetDlgCode( LPMSG lpMessage )
	{
		return DLGC_WANTARROWS | DLGC_WANTTAB | DLGC_WANTCHARS;
	}
	
	void OnSize( UINT nType, CSize size )
	{
		// stop any pending scroll
		EndScroll();
		
		// end any pending edit
		if ( m_bEditItem )
			SetFocus();
			
		ResetScrollBars( SB_BOTH, -1, FALSE );
		Invalidate();
	}
	
	void OnHScroll( int nSBCode, short nPos, HWND hScrollBar )
	{
		// stop any pending scroll
		EndScroll();
		
		// end any pending edit
		if ( m_bEditItem )
			SetFocus();
		
		HideTitleTip();
			
		CRect rcClient;
		GetClientRect( rcClient );
		
		int nScrollPos = GetScrollPos( SB_HORZ );

		switch ( nSBCode )
		{
			case SB_LEFT:			
				nScrollPos = 0;
				break;

			case SB_LINELEFT:		
				nScrollPos = max( nScrollPos - ITEM_SCROLL_OFFSET, 0 );
				break;

			case SB_PAGELEFT:		
				nScrollPos = max( nScrollPos - rcClient.Width(), 0 );
				break;

			case SB_RIGHT:			
				nScrollPos = rcClient.Width();
				break;

			case SB_LINERIGHT:		
				nScrollPos = min( nScrollPos + ITEM_SCROLL_OFFSET, GetTotalWidth() );
				break;

			case SB_PAGERIGHT:		
				nScrollPos = min( nScrollPos + rcClient.Width(), GetTotalWidth() );
				break;

			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:		
				{
					SCROLLINFO infoScroll;
					ZeroMemory( &infoScroll, sizeof( SCROLLINFO ) );
					infoScroll.cbSize = sizeof( SCROLLINFO );
					infoScroll.fMask = SIF_TRACKPOS;
										
					// get 32-bit scroll position
					if ( !GetScrollInfo( SB_HORZ, &infoScroll ) )
						return;
										
					// has scroll position changed?
					if ( infoScroll.nTrackPos == nScrollPos )
						return;
										
					nScrollPos = infoScroll.nTrackPos;
				}
				break;

			default:				
				return;
		}

		ResetScrollBars( SB_HORZ, nScrollPos, FALSE );	
		Invalidate();
	}

	void OnVScroll( int nSBCode, short nPos, HWND hScrollBar )
	{
		T* pT = static_cast<T*>(this);
		
		// end any pending edit
		if ( m_bEditItem )
			SetFocus();
		
		HideTitleTip();

		CRect rcClient;
		GetClientRect( rcClient );
		rcClient.top = ( mbShowHeader ? mnHeaderHeight : 0 );
		
		int nScrollPos = GetScrollPos( SB_VERT );
		BOOL bScrollList = m_bSmoothScroll;
		
		switch ( nSBCode )
		{
			case SB_TOP:			
				nScrollPos = 0;
				bScrollList = FALSE;
				break;

			case SB_LINEUP:			
				nScrollPos = max( nScrollPos - mnItemHeight, 0 );
				break;

			case SB_PAGEUP:			
				nScrollPos = max( nScrollPos - rcClient.Height(), 0 );
				break;

			case SB_BOTTOM:			
				nScrollPos = pT->GetItemCount() * mnItemHeight;
				bScrollList = FALSE;
				break;

			case SB_LINEDOWN:		
				nScrollPos += mnItemHeight;
				break;

			case SB_PAGEDOWN:		
				nScrollPos += rcClient.Height();
				break;

			case SB_THUMBTRACK:
			case SB_THUMBPOSITION:	
				{
					SCROLLINFO infoScroll;
					ZeroMemory( &infoScroll, sizeof( SCROLLINFO ) );
					infoScroll.cbSize = sizeof( SCROLLINFO );
					infoScroll.fMask = SIF_TRACKPOS;
										
					// get 32-bit scroll position
					if ( !GetScrollInfo( SB_VERT, &infoScroll ) )
						return;
										
					// has scroll position changed?
					if ( infoScroll.nTrackPos == nScrollPos )
						return;
										
					nScrollPos = infoScroll.nTrackPos;
					bScrollList = FALSE;
				}
				break;

			case SB_ENDSCROLL:		
				m_bScrolling = FALSE;

			default:				
				return;
		}
		
		// store original top item before scrolling
		int nTopItem = GetTopItem();
		ResetScrollBars( SB_VERT, nScrollPos, FALSE );
		
		if ( bScrollList && !m_bScrolling )
			m_bScrolling = BeginScroll( nTopItem, GetTopItem() );
		else
			EndScroll();
	}
	
	void OnCancelMode() 
	{
		if ( m_bButtonDown )
			ReleaseCapture();
	
		HideTitleTip();
		m_wndDropArrows.Hide();
		m_nDragColumn = NULL_COLUMN;
		m_nHighlightColumn = NULL_COLUMN;
	}
	
	LRESULT OnMouseRange( UINT nMessage, WPARAM wParam, LPARAM lParam )
	{
		if ( m_ttToolTip.IsWindow() )
		{
			MSG msgRelay = { m_hWnd, nMessage, wParam, lParam };
			m_ttToolTip.RelayEvent( &msgRelay );
		}
		SetMsgHandled( FALSE );
		return 0;
	}
	
	void OnLButtonDown( UINT nFlags, CPoint point )
	{
		T* pT = static_cast<T*>(this);
		
		HideTitleTip( FALSE );
		
		m_bButtonDown = TRUE;
		m_ptDownPoint = point;
		m_ptSelectPoint = CPoint( point.x + GetScrollPos( SB_HORZ ), point.y + GetScrollPos( SB_VERT ) );

		// stop any pending scroll
		EndScroll();
		
		SetFocus();
		
		// capture all mouse input
		// 检查是否启用多选.若未启用，禁用用鼠标拖动选择的功能
		if ( m_bEnableMultiSelection )
			SetCapture();

		if ( !m_bEnableMultiSelection )
			m_bButtonDown = FALSE;
		
		int nColumn = NULL_COLUMN;
		UINT nHeaderFlags = HITTEST_FLAG_NONE;
		
		// are we over the header?
		if ( HitTestHeader( point, nColumn, nHeaderFlags ) )
		{
			// 若用户是在标题栏上拖曳，允许操作
			SetCapture();
			m_bButtonDown = TRUE;

			CListColumnEx listColumn;
			if ( !GetColumn( nColumn, listColumn ) )
				return;
				
			if ( !listColumn.m_bFixed && ( nHeaderFlags & HITTEST_FLAG_HEADER_DIVIDER ) )
			{
				SetCursor( mDividerCursor );			
				// begin column resizing
				m_bColumnSizing = TRUE;
				m_nColumnSizing = nColumn;
				m_nStartSize = listColumn.m_nWidth;
				m_nStartPos = GET_X_LPARAM( GetMessagePos() );
			}
			else
			{
				m_nHighlightColumn = nColumn;
				InvalidateHeader();
			}
			
			return;			
		}
		
		int nItem = NULL_ITEM;
		int nSubItem = NULL_SUBITEM;
		
		if ( !HitTest( point, nItem, nSubItem ) )
		{
			m_nFirstSelected = NULL_ITEM;
			m_bBeginSelect = TRUE;		
		}
		else
		{
			SetCursor( mHandleCursor );

			// 检查是否启用多选.若未启用，清除对ctl, shift的检查
			if ( !m_bEnableMultiSelection )
				nFlags &= ~(MK_SHIFT | MK_CONTROL);

			if ( !( nFlags & MK_SHIFT ) && !( nFlags & MK_CONTROL ) && nSubItem != 0 )
			{
				m_bBeginSelect = TRUE;
				m_nFirstSelected = nItem;
			}
			
			// only select item if not already selected
			if ( ( nFlags & MK_SHIFT ) || ( nFlags & MK_CONTROL ) || !IsSelected( nItem ) || m_setSelectedItems.size() <= 1 )
				SelectItem( nItem, nSubItem, nFlags );
			
			int nIndex = GetColumnIndex( nSubItem );
			if ( !( pT->GetItemFlags( nItem, nIndex ) & ITEM_FLAGS_READ_ONLY ) )
			{
				switch ( pT->GetItemFormat( nItem, nIndex ) )
				{
					case ITEM_FORMAT_CHECKBOX:			
						m_bBeginSelect = FALSE;

						pT->SetItemCheck(nItem, nIndex, !pT->GetItemCheck(nItem, nIndex));

						NotifyParent( nItem, nSubItem, LCN_MODIFIED );
						InvalidateItem( nItem );
						break;

					case ITEM_FORMAT_CHECKBOX_3STATE:	
						{
							m_bBeginSelect = FALSE;
															
							int nCheck = pT->GetItemCheck(nItem, nIndex);
							if (nCheck < 0)
								pT->SetItemCheck(nItem, nIndex, 0);
							else if (nCheck > 0)
								pT->SetItemCheck(nItem, nIndex, -1);
							else
								pT->SetItemCheck(nItem, nIndex, 1);
															
							NotifyParent( nItem, nSubItem, LCN_MODIFIED );
							InvalidateItem( nItem );
						}
						break;

					case ITEM_FORMAT_HYPERLINK:			
						m_bBeginSelect = FALSE;
						SetCursor( mHandleCursor );
						NotifyParent( nItem, nSubItem, LCN_HYPERLINK );
						break;
				}
			}
		}
	}
	
	void OnLButtonUp( 
		UINT nFlags, 
		CPoint point)
	{
		if (m_bButtonDown)
			ReleaseCapture();
		
		if (m_bColumnSizing || m_nHighlightColumn != NULL_COLUMN)
		{
			if (!m_bColumnSizing && m_nHighlightColumn != NULL_COLUMN)
				SortColumn(m_nHighlightColumn);
			
			m_bColumnSizing = FALSE;
			m_nColumnSizing = NULL_COLUMN;
			m_nHighlightColumn = NULL_COLUMN;
			m_nStartSize = 0;
			m_nStartPos = 0;

			InvalidateHeader();
		}
		
		m_bBeginSelect = FALSE;
		m_bButtonDown = FALSE;
		m_ptDownPoint = 0;
		m_ptSelectPoint = 0;
		
		if (m_bGroupSelect)
		{
			m_bGroupSelect = FALSE;
			Invalidate();
		}else
		{
			int nItem = NULL_ITEM;
			int nSubItem = NULL_SUBITEM;
			
			BOOL bRetCode = HitTest(point, nItem, nSubItem);
			if (bRetCode == TRUE)
			{
				UINT unFormat = GetItemFormat(nItem, 0);
				if (unFormat == ITEM_FORMAT_FULL_LINE)
				{
					CListArray<int> ExpandList;

					GetItemExpandList(nItem, ExpandList);

					if (ExpandList.GetSize() > 0)
					{
						UINT unFlags = GetItemFlags(ExpandList[0], 0);
						if (unFlags == ITEM_FLAGS_HIDE)
							unFlags = 0;
						else
							unFlags = ITEM_FLAGS_HIDE;

						for (int nLoop = 0; nLoop < ExpandList.GetSize(); nLoop ++)
							SetItemFlags(ExpandList[nLoop], 0, unFlags);

						Invalidate();
					}
				}

				if (IsSelected(nItem) && m_setSelectedItems.size() > 1 && 
					!(nFlags & MK_SHIFT) && 
					!(nFlags & MK_CONTROL))
				{
					SelectItem(nItem, nSubItem, nFlags);
				}				
			}
				
			NotifyParent(nItem, nSubItem, LCN_LEFTCLICK);
		}
	}
	
	void OnLButtonDblClk( UINT nFlags, CPoint point ) 
	{
		HideTitleTip( FALSE );
		
		// handle double-clicks (for drawing)
		SendMessage( WM_LBUTTONDOWN, 0, MAKELPARAM( point.x, point.y ) );

		int nColumn = NULL_COLUMN;
		UINT nHeaderFlags = HITTEST_FLAG_NONE;
		
		// resize column if double-click on a divider
		if ( HitTestHeader( point, nColumn, nHeaderFlags ) && ( nHeaderFlags & HITTEST_FLAG_HEADER_DIVIDER ) )
			AutoSizeColumn( nColumn );
		
		int nItem = NULL_ITEM;
		int nSubItem = NULL_SUBITEM;
		
		HitTest( point, nItem, nSubItem );
		
		// notify parent of double-clicked item
		NotifyParent( nItem, nSubItem, LCN_DBLCLICK );
	}
	
	void OnRButtonDown( UINT nFlags, CPoint point ) 
	{
		// stop any pending scroll
		EndScroll();
		
		SetFocus();
		
		HideTitleTip( FALSE );
		
		int nItem = NULL_ITEM;
		int nSubItem = NULL_SUBITEM;
		
		SetCursor( mHandleCursor );

		if ( HitTest( point, nItem, nSubItem ) && !IsSelected( nItem ) )
			SelectItem( nItem, nSubItem, nFlags );
	}

	void OnRButtonUp( UINT nFlags, CPoint point )
	{
		int nItem = NULL_ITEM;
		int nSubItem = NULL_SUBITEM;
		
		if ( !HitTest( point, nItem, nSubItem ) )
			ResetSelected();
		
		// notify parent of right-click item
		NotifyParent( nItem, nSubItem, LCN_RIGHTCLICK );
	}
	
	void OnMouseMove( UINT nFlags, CPoint point )
	{
		T* pT = static_cast<T*>(this);
		
		if ( !( nFlags & MK_LBUTTON ) )
		{
			if ( m_bButtonDown )
				ReleaseCapture();
			
			m_bButtonDown = FALSE;
		}
		
		if ( !m_bMouseOver )
		{
			m_bMouseOver = TRUE;
			
			TRACKMOUSEEVENT trkMouse;
			trkMouse.cbSize = sizeof( TRACKMOUSEEVENT );
			trkMouse.dwFlags = TME_LEAVE;
			trkMouse.hwndTrack = m_hWnd;
			
			// notify when the mouse leaves button
			_TrackMouseEvent( &trkMouse );
		}
		
		if ( m_bButtonDown )
		{
			// are we resizing a column?
			if ( m_bColumnSizing )
			{
				ResizeColumn();
				return;
			}
			
			// are we beginning to drag a column? 
			if ( m_nHighlightColumn != NULL_COLUMN && ( point.x < m_ptDownPoint.x - DRAG_HEADER_OFFSET || point.x > m_ptDownPoint.x + DRAG_HEADER_OFFSET || point.y < m_ptDownPoint.y - DRAG_HEADER_OFFSET || point.y > m_ptDownPoint.y + DRAG_HEADER_OFFSET ) )
			{
				DragColumn();
				return;
			}
			
			// are we beginning a group select or dragging an item?
			if ( point.x < m_ptDownPoint.x - DRAG_ITEM_OFFSET || point.x > m_ptDownPoint.x + DRAG_ITEM_OFFSET || point.y < m_ptDownPoint.y - DRAG_ITEM_OFFSET || point.y > m_ptDownPoint.y + DRAG_ITEM_OFFSET )
			{
				if ( m_bBeginSelect || !m_bDragDrop )
					m_bGroupSelect = ( !m_bSingleSelect && !m_bEditItem );
				else
				{
					int nItem = NULL_ITEM;
					int nSubItem = NULL_SUBITEM;
					
					if ( HitTest( point, nItem, nSubItem ) )
					{
						// select the drag item (if not already selected)
						if ( !IsSelected( nItem ) )
							SelectItem( nItem, nSubItem, nFlags );
						
						// begin drag item operation
						pT->DragItem();
					}
				}
			}							
			
			if ( m_bGroupSelect )
			{
				GroupSelect( point );
				return;
			}
		}
		else
		{
			int nColumn = NULL_COLUMN;
			UINT nHeaderFlags = HITTEST_FLAG_NONE;
			
			// are we over the header?
			BOOL bHitTestHeader = HitTestHeader( point, nColumn, nHeaderFlags );
			
			if ( mbShowThemed && !mHeaderTheme.IsThemeNull() )
			{
				if ( bHitTestHeader && m_nHotColumn != nColumn )
				{
					m_nHotColumn = nColumn;
					InvalidateHeader();
				}
				else if ( !bHitTestHeader && m_nHotColumn != NULL_COLUMN )
				{
					m_nHotColumn = NULL_COLUMN;
					InvalidateHeader();
				}
			}
			
			if ( bHitTestHeader )
			{
				HideTitleTip();
				CListColumnEx listColumn;
				if ( GetColumn( nColumn, listColumn ) && !listColumn.m_bFixed && ( nHeaderFlags & HITTEST_FLAG_HEADER_DIVIDER ) )
					SetCursor( mDividerCursor );
				return;
			}
			
			int nItem = NULL_ITEM;
			int nSubItem = NULL_SUBITEM;
			
			if ( !HitTest( point, nItem, nSubItem ) )
			{
				if ( m_nHotItem != NULL_ITEM && m_nHotSubItem != NULL_SUBITEM )
				{
					// redraw old hot item
					int nIndex = GetColumnIndex( m_nHotSubItem );
					if ( pT->GetItemFormat( m_nHotItem, nIndex ) == ITEM_FORMAT_HYPERLINK && !( pT->GetItemFlags( m_nHotItem, nIndex ) & ITEM_FLAGS_READ_ONLY ) )
						InvalidateItem( m_nHotItem, m_nHotSubItem );
				}
				
				m_ttToolTip.Activate( FALSE );
				m_ttToolTip.DelTool( m_hWnd, TOOLTIP_TOOL_ID );
					
				m_nHotItem = NULL_ITEM;
				m_nHotSubItem = NULL_SUBITEM;
				HideTitleTip();
			}
			else
			{
				SetCursor( mHandleCursor );

				if ( nItem != m_nHotItem || nSubItem != m_nHotSubItem )
				{
					// redraw old hot item
					int nIndex = GetColumnIndex( m_nHotSubItem );
					if ( pT->GetItemFormat( m_nHotItem, nIndex ) == ITEM_FORMAT_HYPERLINK && !( pT->GetItemFlags( m_nHotItem, nIndex ) & ITEM_FLAGS_READ_ONLY ) )
						InvalidateItem( m_nHotItem, m_nHotSubItem );
					
					m_nHotItem = nItem;
					m_nHotSubItem = nSubItem;
				}
				
				int nIndex = GetColumnIndex( m_nHotSubItem );
				UINT nItemFormat = pT->GetItemFormat( m_nHotItem, nIndex );
				UINT nItemFlags = pT->GetItemFlags( m_nHotItem, nIndex );
				
				if (nItemFormat == ITEM_FORMAT_FULL_LINE)
					return;

				// draw new hot hyperlink item
				if ( nItemFormat == ITEM_FORMAT_HYPERLINK && !( nItemFlags & ITEM_FLAGS_READ_ONLY ) )
				{
					InvalidateItem( m_nHotItem, m_nHotSubItem );
					SetCursor( mHandleCursor );
				}
				
				// get tooltip for this item
				CString strToolTip = pT->GetItemToolTip( m_nHotItem, nIndex );
				
				CRect rCSubItemEx;
				if ( !strToolTip.IsEmpty() && GetItemRect( m_nHotItem, rCSubItemEx ) )
				{
					m_ttToolTip.Activate( TRUE );
					m_ttToolTip.AddTool( m_hWnd, (LPCTSTR)strToolTip.Left( SHRT_MAX ), rCSubItemEx, TOOLTIP_TOOL_ID );
				}
				else
				{
					m_ttToolTip.Activate( FALSE );
					m_ttToolTip.DelTool( m_hWnd, TOOLTIP_TOOL_ID );
				}
				
				// show titletips for this item
				ShowTitleTip( point, m_nHotItem, m_nHotSubItem );
			}
		}
	}
	
	void OnMouseLeave()
	{
		m_bMouseOver = FALSE;
		
		if ( m_nHotColumn != NULL_COLUMN )
		{
			m_nHotColumn = NULL_COLUMN;
			InvalidateHeader();
		}
		
		if ( m_nHotItem != NULL_ITEM || m_nHotSubItem != NULL_SUBITEM )
		{
			m_nHotItem = NULL_ITEM;
			m_nHotSubItem = NULL_SUBITEM;
			Invalidate();
		}
	}
	
	BOOL OnMouseWheel( UINT nFlags, short nDelta, CPoint point )
	{
		HideTitleTip();
		
		// end any pending edit
		if ( m_bEditItem )
			SetFocus();
		
		int nRowsScrolled = m_nMouseWheelScroll * nDelta / 120;
		int nScrollPos = GetScrollPos( SB_VERT );
		
		if ( nRowsScrolled > 0 )
			nScrollPos = max( nScrollPos - ( nRowsScrolled * mnItemHeight ), 0 );
		else
			nScrollPos += ( -nRowsScrolled * mnItemHeight );
		
		ResetScrollBars( SB_VERT, nScrollPos, FALSE );
		Invalidate();
		
		return TRUE;
	}
	
	void OnTimer( UINT nIDEvent)
	{
		switch ( nIDEvent )
		{
			case RESIZE_COLUMN_TIMER:	
				ResizeColumn( TRUE );
				break;

			case ITEM_VISIBLE_TIMER:	
				{
					KillTimer( ITEM_VISIBLE_TIMER );
											
					int nFocusItem = NULL_ITEM;
					int nFocusSubItem = NULL_SUBITEM;
											
					// get current focus item
					if ( !GetFocusItem( nFocusItem, nFocusSubItem ) )
						break;
											
					// make sure current focus item is visible before editing
					if ( !EditItem( nFocusItem, nFocusSubItem ) )
						break;
				}
				break;

			case ITEM_AUTOSCROLL_TIMER:	
				if ( !m_bGroupSelect )
				{
					KillTimer( ITEM_AUTOSCROLL_TIMER );
				}else
				{
					DWORD dwPoint = GetMessagePos();
					CPoint ptMouse( GET_X_LPARAM( dwPoint ), GET_Y_LPARAM( dwPoint ) );
					ScreenToClient( &ptMouse );
		
					// automatically scroll when group selecting
					AutoScroll( ptMouse );
					AutoSelect( ptMouse );
				}
				break;

			case ITEM_SCROLL_TIMER:		
				if ( !ScrollList() )
					EndScroll();

				break;
		}
	}
	
	void OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags )
	{
		T* pT = static_cast<T*>(this);
		
		// stop any pending scroll
		EndScroll();
		
		BOOL bCtrlKey = ( ( GetKeyState( VK_CONTROL ) & 0x8000 ) != 0 );
		BOOL bShiftKey = ( ( GetKeyState( VK_SHIFT ) & 0x8000 ) != 0 );
		
		CRect rcClient;
		GetClientRect( rcClient );
		
		int nFocusItem = NULL_ITEM;
		int nFocusSubItem = NULL_SUBITEM;
		GetFocusItem( nFocusItem, nFocusSubItem );
		
		switch ( nChar )
		{
			case VK_DOWN:	
				SetFocusItem( min( nFocusItem + 1, pT->GetItemCount() - 1 ), nFocusSubItem );
				break;

			case VK_UP:		
				SetFocusItem( max( nFocusItem - 1, 0 ), nFocusSubItem );
				break;

			case VK_NEXT:	
				SetFocusItem( min( nFocusItem + GetCountPerPage( FALSE ) - 1, pT->GetItemCount() - 1 ), nFocusSubItem );
				break;

			case VK_PRIOR:	
				SetFocusItem( max( nFocusItem - GetCountPerPage( FALSE ) + 1, 0 ), nFocusSubItem );
				break;

			case VK_HOME:	
				SetFocusItem( 0, nFocusSubItem );
				break;

			case VK_END:	
				SetFocusItem( pT->GetItemCount() - 1, nFocusSubItem );
				break;

			case VK_LEFT:	
				if ( m_bFocusSubItem )
					SetFocusItem( nFocusItem, max( nFocusSubItem - 1, 0 ) );
				else
					SetScrollPos( SB_HORZ, max( GetScrollPos( SB_HORZ ) - ( bCtrlKey ? ITEM_SCROLL_OFFSET * 10 : ITEM_SCROLL_OFFSET ), 0 ) );

				break;

			case VK_RIGHT:	
				if ( m_bFocusSubItem )
					SetFocusItem( nFocusItem, min( nFocusSubItem + 1, GetColumnCount() - 1 ) );
				else
					SetScrollPos( SB_HORZ, min( GetScrollPos( SB_HORZ ) + ( bCtrlKey ? ITEM_SCROLL_OFFSET * 10 : ITEM_SCROLL_OFFSET ), rcClient.Width() ) );

				break;

			case VK_TAB:	
				if ( !bCtrlKey && m_bFocusSubItem )
					SetFocusItem( nFocusItem, bShiftKey ? max( nFocusSubItem - 1, 0 ) : min( nFocusSubItem + 1, GetColumnCount() - 1 ) );

				break;

			default:		
				if ( nChar == VK_SPACE )
				{
					int nIndex = GetColumnIndex( nFocusSubItem );
					if ( !( pT->GetItemFlags( nFocusItem, nIndex ) & ITEM_FLAGS_READ_ONLY ) )
					{
						switch ( pT->GetItemFormat( nFocusItem, nIndex ) )
						{
							case ITEM_FORMAT_CHECKBOX:			
								m_bBeginSelect = FALSE;

								pT->SetItemCheck(nFocusItem, nFocusSubItem, !pT->GetItemCheck(nFocusItem, nFocusSubItem));

								NotifyParent( nFocusItem, nFocusSubItem, LCN_MODIFIED );
								InvalidateItem( nFocusItem );
								break;

							case ITEM_FORMAT_CHECKBOX_3STATE:	
								{
									m_bBeginSelect = FALSE;

									int nCheck = pT->GetItemCheck(nFocusItem, nFocusSubItem);
									if (nCheck < 0)
										pT->SetItemCheck(nFocusItem, nFocusSubItem, 0);
									else if (nCheck > 0)
										pT->SetItemCheck(nFocusItem, nFocusSubItem, -1);
									else
										pT->SetItemCheck(nFocusItem, nFocusSubItem, 1);

									NotifyParent( nFocusItem, nFocusSubItem, LCN_MODIFIED );
									InvalidateItem( nFocusItem );
								}
								return;
							}
						}
					}
							
					if ( bCtrlKey && nChar == _T( 'A' ) && m_bEnableMultiSelection )
					{
						m_setSelectedItems.clear();
						for ( int nItem = 0; nItem < pT->GetItemCount(); nItem++ )
							m_setSelectedItems.insert( nItem );

						Invalidate();
						return;
					}
							
					if ( !bCtrlKey && iswprint( nChar ) && iswupper( nChar ) )
					{
						int nSortIndex = GetColumnIndex( m_nSortColumn );
						int nStartItem = nFocusItem + 1;
						DWORD dwCurrentTick = GetTickCount();
								
						CString strStart;
						strStart += nChar;
								
						// has there been another keypress since last search period?
						if ( ( dwCurrentTick - m_dwSearchTick ) < SEARCH_PERIOD )
						{
							if ( m_strSearchString.Left( 1 ) != strStart )
								m_strSearchString += nChar;
									
							CString strFocusText = pT->GetItemText( nFocusItem, nSortIndex );
									
							// are we continuing to type characters under current focus item?
							if ( m_strSearchString.GetLength() > 1 && m_strSearchString.CompareNoCase( strFocusText.Left( m_strSearchString.GetLength() ) ) == 0 )
							{
								m_dwSearchTick = GetTickCount();
								return;
							}
						}else
						{
							if ( m_strSearchString.Left( 1 ) != strStart )
								nStartItem = 0;

							m_strSearchString = strStart;
						}
								
						m_dwSearchTick = GetTickCount();
								
						// scan for next search string
						for ( int nFirst = nStartItem; nFirst < pT->GetItemCount(); nFirst++ )
						{
							CString strItemText = pT->GetItemText( nFirst, nSortIndex );
									
							if ( m_strSearchString.CompareNoCase( strItemText.Left( m_strSearchString.GetLength() ) ) == 0 )
							{
								SelectItem( nFirst, nFocusSubItem, TRUE );
								EnsureItemVisible( nFirst, nFocusSubItem );
								return;
							}
						}
								
						// re-scan from top if not found search string
						for ( int nSecond = 0; nSecond < pT->GetItemCount(); nSecond++ )
						{
							CString strItemText = pT->GetItemText( nSecond, nSortIndex );
									
							if ( m_strSearchString.CompareNoCase( strItemText.Left( m_strSearchString.GetLength() ) ) == 0 )
							{
								SelectItem( nSecond, nFocusSubItem, TRUE );
								EnsureItemVisible( nSecond, nFocusSubItem );
								return;
							}
						}
					}
					return;
		}

		if ( !bCtrlKey )
			SelectItem( m_nFocusItem, m_nFocusSubItem, bShiftKey ? MK_SHIFT : 0 );
	}
	
	void OnSysKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags )
	{
		HideTitleTip( FALSE );
		SetMsgHandled( FALSE );		
	}
	
	void OnSettingsChange( UINT nFlags, LPCTSTR lpszSection )
	{
		OnSettingsChange();
	}
	
	void OnSettingsChange()
	{
		LoadSettings();		
		ResetScrollBars();
		Invalidate();
	}
	
	LRESULT OnCtlColorListBox( UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		return DefWindowProc( nMsg, wParam, lParam );
	}
	
	LRESULT OnEndEdit( LPNMHDR lpNMHDR )
	{
		T* pT = static_cast<T*>(this);
		CListNotify *pListNotify = reinterpret_cast<CListNotify *>( lpNMHDR );
		
		m_bEditItem = FALSE;
		int nIndex = GetColumnIndex( pListNotify->m_nSubItem );
		
		switch ( pListNotify->m_nExitChar )
		{
			case VK_ESCAPE:	
				break; // do nothing

			case VK_DELETE:	
				pT->SetItemText( pListNotify->m_nItem, nIndex, _T( "" ) );
				NotifyParent( pListNotify->m_nItem, pListNotify->m_nSubItem, LCN_MODIFIED );
				break;

			default:		
				if ( pListNotify->m_lpItemDate == NULL )
				{
					pT->SetItemText( pListNotify->m_nItem, nIndex, pListNotify->m_lpszItemText );
				}else
				{
					if ( _ttoi( pListNotify->m_lpszItemText ) == 0 )
						pT->SetItemText( pListNotify->m_nItem, nIndex, _T( "" ) );
					else
						pT->SetItemDate( pListNotify->m_nItem, nIndex, *pListNotify->m_lpItemDate );
				}

				if ( pListNotify->m_nExitChar == VK_TAB )
					PostMessage( WM_KEYDOWN, (WPARAM)VK_TAB );

				NotifyParent( pListNotify->m_nItem, pListNotify->m_nSubItem, LCN_MODIFIED );
				break;
		}
		
		InvalidateItem( pListNotify->m_nItem );
		
		return 0;	
	}
	
	DWORD OnDragEnter( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point )
	{
		DWORD dwEffect = DROPEFFECT_NONE;
		
		if ( FormatEtc.cfFormat == m_nHeaderClipboardFormat )
		{
			LPBYTE lpDragHeader = (LPBYTE)GlobalLock( StgMedium.hGlobal );
			if ( lpDragHeader == NULL )
				return DROPEFFECT_NONE;
			
			// dragged column must originate from this control
			if ( *( (HWND*)lpDragHeader ) == m_hWnd )
				dwEffect = DropColumn( point ) ? DROPEFFECT_MOVE : DROPEFFECT_NONE;
			
			GlobalUnlock( StgMedium.hGlobal );
		}
		
		return dwEffect;
	}
	
	DWORD OnDragOver( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwKeyState, CPoint point )
	{
		DWORD dwEffect = DROPEFFECT_NONE;
		
		if ( FormatEtc.cfFormat == m_nHeaderClipboardFormat )
		{
			LPBYTE lpDragHeader = (LPBYTE)GlobalLock( StgMedium.hGlobal );
			if ( lpDragHeader == NULL )
				return DROPEFFECT_NONE;
			
			// dragged column must originate from this control
			if ( *( (HWND*)lpDragHeader ) == m_hWnd )
				dwEffect = DropColumn( point ) ? DROPEFFECT_MOVE : DROPEFFECT_NONE;
			
			GlobalUnlock( StgMedium.hGlobal );
		}
		
		return dwEffect;
	}
	
	BOOL OnDrop( FORMATETC& FormatEtc, STGMEDIUM& StgMedium, DWORD dwEffect, CPoint point )
	{
		if ( FormatEtc.cfFormat == m_nHeaderClipboardFormat )
		{
			if ( m_nDragColumn != NULL_COLUMN && m_nHotDivider != NULL_COLUMN )
			{
				CListColumnEx listColumn;
				if ( !GetColumn( m_nDragColumn, listColumn ) )
					return FALSE;
			
				// move column to new position
				m_aColumns.RemoveAt( m_nDragColumn );
				m_aColumns.InsertAt( ( m_nDragColumn < m_nHotColumn ? ( m_nHotDivider == 0 ? 0 : m_nHotDivider - 1 ) : m_nHotDivider ), listColumn );
				Invalidate();
			}

			return TRUE;
		}			
		
		// not supported
		return FALSE;
	}
	
	void OnDragLeave()
	{
	}
	
	BOOL OnRenderData( FORMATETC& FormatEtc, STGMEDIUM *pStgMedium, BOOL bDropComplete )
	{
		if ( FormatEtc.cfFormat == m_nHeaderClipboardFormat )
		{
			pStgMedium->tymed = TYMED_HGLOBAL;
			pStgMedium->hGlobal = GlobalAlloc( GMEM_MOVEABLE, sizeof( HWND ) );
			if ( pStgMedium->hGlobal == NULL )
				return FALSE;
			
			LPBYTE lpDragHeader = (LPBYTE)GlobalLock( pStgMedium->hGlobal );
			if ( lpDragHeader == NULL )
				return FALSE;
			
			// store this window handle
			*( (HWND*)lpDragHeader ) = m_hWnd;
			
			GlobalUnlock( pStgMedium->hGlobal );
		
			return TRUE;
		}
			
		return FALSE;
	}
	
	void DoPaint( 
		CDCHandle PaintDC)
	{
		T *pT = static_cast<T *>(this);
		
		int nContextState = PaintDC.SaveDC();
		
 		pT->DrawBkgnd(PaintDC);
		pT->DrawList(PaintDC);
		pT->DrawSelect(PaintDC);
		pT->DrawHeader(PaintDC);
		
		PaintDC.RestoreDC(nContextState);
	}
	
	void DrawBkgnd( 
		CDCHandle PaintDC)
	{
		CRect ClipRect;
		if (PaintDC.GetClipBox(ClipRect) == ERROR)
			return;
		
		PaintDC.SetBkColor(mBKColor);
		PaintDC.ExtTextOut(ClipRect.left, ClipRect.top, ETO_OPAQUE, ClipRect, _T(""), 0, NULL);
		
		CRect ClientRect;

		GetClientRect(ClientRect);
		ClientRect.top = ( mbShowHeader ? mnHeaderHeight : 0 );
		
		if ( !m_bmpBackground.IsNull() && ClipRect.bottom > ClientRect.top )
		{
			CSize sizBackground;
			m_bmpBackground.GetSize( sizBackground );
		
			CDC dcBackgroundImage;
			dcBackgroundImage.CreateCompatibleDC( PaintDC );
		
			HBITMAP hOldBitmap = dcBackgroundImage.SelectBitmap( m_bmpBackground );
			
			if ( m_bTileBackground )
			{
				// calculate tile image maximum rows and columns
				ldiv_t divRows = div( (long)ClientRect.Height(), (long)sizBackground.cy );
				int nTileRows = divRows.rem > 0 ? divRows.quot + 1 : divRows.quot;
				ldiv_t divColumns = div( (long)ClientRect.Width(), (long)sizBackground.cx );
				int nTileColumns = divColumns.rem > 0 ? divColumns.quot + 1 : divColumns.quot;
				
				// draw tiled background image
				for ( int nRow = 0; nRow <= nTileRows; nRow++ )
				{
					for ( int nColumn = 0; nColumn <= nTileColumns; nColumn++ )
						PaintDC.BitBlt( nColumn * sizBackground.cx, nRow * sizBackground.cy, sizBackground.cx, sizBackground.cy, dcBackgroundImage, 0, 0, SRCCOPY );
				}
			}
			else
			{
				CRect rcCentreImage( ClientRect );
				
				// horizontally centre image if smaller than the client width
				if ( sizBackground.cx < ClientRect.Width() )
				{
					rcCentreImage.left = ( ClientRect.Width() / 2 ) - (int)( sizBackground.cx / 2 );
					rcCentreImage.right = rcCentreImage.left + sizBackground.cx;
				}
				
				// vertically centre image if smaller than the client height
				if ( sizBackground.cy + 16 < ClientRect.Height() )
				{
					rcCentreImage.top = ( ClientRect.Height() / 2 ) - (int)( ( sizBackground.cy + 16 ) / 2 );
					rcCentreImage.bottom = rcCentreImage.top + sizBackground.cy;
				}
				
				// draw centred background image
				PaintDC.BitBlt( rcCentreImage.left, rcCentreImage.top, rcCentreImage.Width(), rcCentreImage.Height(), dcBackgroundImage, 0, 0, SRCCOPY );
			}

			dcBackgroundImage.SelectBitmap( hOldBitmap );
		}		
	}
	
	void DrawHeader( 
		CDCHandle PaintDC)
	{
		if (!mbShowHeader)
			return;
		
		CRect ClipRect;
		if (PaintDC.GetClipBox(ClipRect) == ERROR)
			return;
		
		CRect HeaderRect;
		GetClientRect(HeaderRect);
		HeaderRect.bottom = mnHeaderHeight;
		
		if (ClipRect.top > HeaderRect.bottom)
			return;
		
		if (!mbShowThemed || mHeaderTheme.IsThemeNull())
		{
			CGradient Gradient(CSize(HeaderRect.Width(), HeaderRect.Height()));
			Gradient.PrepareVertical(PaintDC.m_hDC, RGB(255, 255, 255), RGB(243, 243, 243));
			Gradient.Draw(PaintDC.m_hDC, HeaderRect.left, HeaderRect.top, 0, 0, HeaderRect.Width(), HeaderRect.Height(), SRCCOPY);
		}
			
		CRect HeaderItemRect(HeaderRect);
		HeaderItemRect.OffsetRect(-GetScrollPos(SB_HORZ), 0);
		
		int nHeaderWidth = 0;
		
		for ( int nColumn = 0, nColumnCount = GetColumnCount(); nColumn < nColumnCount; HeaderItemRect.left = HeaderItemRect.right, nColumn++ )
		{
			CListColumnEx listColumn;
			if ( !GetColumn( nColumn, listColumn ) )
				break;
			
			HeaderItemRect.right = HeaderItemRect.left + listColumn.m_nWidth;
			nHeaderWidth += HeaderItemRect.Width();
			
			if ( HeaderItemRect.right < ClipRect.left )
				continue;

			if ( HeaderItemRect.left > ClipRect.right )
				break;
			
			if ( mbShowThemed && !mHeaderTheme.IsThemeNull() )
			{
				mHeaderTheme.DrawThemeBackground( 
					PaintDC, 
					HP_HEADERITEM, 
					nColumn == m_nHighlightColumn ? HIS_PRESSED : ( nColumn == m_nHotColumn ? HIS_HOT : HIS_NORMAL ), 
					HeaderItemRect, 
					NULL);
			}
			else
			{
				if (nColumn == m_nHighlightColumn)
				{
					PaintDC.SetBkColor( mHeaderHighlightColor );
					PaintDC.ExtTextOut( HeaderItemRect.left, HeaderItemRect.top, ETO_OPAQUE, HeaderItemRect, _T( "" ), 0, NULL );
				}

				CPen BorderPen;
				BorderPen.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHIGHLIGHT));

				CPen BorderShadowPen;
				BorderShadowPen.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));

				PaintDC.SelectPen(BorderShadowPen);
				PaintDC.MoveTo(HeaderItemRect.right - 1, HeaderItemRect.top + 4);
				PaintDC.LineTo(HeaderItemRect.right - 1, mnHeaderHeight - 4);

				PaintDC.SelectPen(BorderPen);
				PaintDC.MoveTo(HeaderItemRect.right, HeaderItemRect.top + 4);
				PaintDC.LineTo(HeaderItemRect.right, mnHeaderHeight - 4);
			}
			
			CRect HeaderTextRect( HeaderItemRect );
			HeaderTextRect.left += nColumn == 0 ? 4 : 3;
			
			BOOL bShowArrow = m_bShowSort && ( HeaderItemRect.Width() > 15 );
			
			if ( bShowArrow && !listColumn.m_bFixed && listColumn.m_nIndex == m_nSortColumn )
				HeaderTextRect.right -= 15;
			
			HeaderTextRect.DeflateRect( 4, 0, 5, 0 );
			
			if ( listColumn.m_nImage != ITEM_IMAGE_NONE )
			{
				CSize IconSize;
				mCtrlImageList.GetIconSize( IconSize );
				
				CRect HeaderImageRect;
				HeaderImageRect.left = listColumn.m_strText.IsEmpty() ? ( ( HeaderTextRect.left + HeaderTextRect.right ) / 2 ) - ( IconSize.cx / 2 ) - ( ( !mbShowThemed || mHeaderTheme.IsThemeNull() ) ? 0 : 1 ) : HeaderTextRect.left;
				HeaderImageRect.right = min( HeaderImageRect.left + IconSize.cx, HeaderItemRect.right - 2 );
				HeaderImageRect.top = ( ( HeaderItemRect.top + HeaderItemRect.bottom ) / 2 ) - ( IconSize.cy / 2 );
				HeaderImageRect.bottom = min( HeaderImageRect.top + IconSize.cy, HeaderItemRect.bottom );
				
				mCtrlImageList.DrawEx( listColumn.m_nImage, PaintDC, HeaderImageRect, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT );

				HeaderTextRect.left += IconSize.cx + 4;
			}
			
			PaintDC.SelectFont( m_fntListFont );
			PaintDC.SetTextColor( mHeaderTextColor );
			PaintDC.SetBkMode( TRANSPARENT );

			UINT nFormat = DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS;

			if ( listColumn.m_nFlags & ITEM_FLAGS_CENTRE )
				nFormat |= DT_CENTER;
			else if ( listColumn.m_nFlags & ITEM_FLAGS_RIGHT )
				nFormat |= DT_RIGHT;
			else
				nFormat |= DT_LEFT;
			
			if ( !HeaderTextRect.IsRectEmpty() && !listColumn.m_strText.IsEmpty() )
				PaintDC.DrawText( listColumn.m_strText, listColumn.m_strText.GetLength(), HeaderTextRect, nFormat );

			// draw sorting arrow
			if ( bShowArrow && !listColumn.m_bFixed && listColumn.m_nIndex == m_nSortColumn )
			{
				CSize sizeIcon;
				mCtrlImageList.GetIconSize( sizeIcon );
				
				CRect rcSortArrow;
				rcSortArrow.left = HeaderTextRect.right + 4;
				rcSortArrow.right = min( rcSortArrow.left + sizeIcon.cx, HeaderItemRect.right );
				rcSortArrow.top = HeaderItemRect.Height() / 2 - 3;
				rcSortArrow.bottom = min( rcSortArrow.top + sizeIcon.cy, HeaderItemRect.bottom );
					
				mCtrlImageList.DrawEx( m_bSortAscending ? ITEM_IMAGE_UP : ITEM_IMAGE_DOWN, PaintDC, rcSortArrow, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT );
			}
		}
		
		// draw remaining blank header
		if ( mbShowThemed && !mHeaderTheme.IsThemeNull() && nHeaderWidth < HeaderRect.Width() )
			mHeaderTheme.DrawThemeBackground( 
			PaintDC, 
			HP_HEADERITEM, 
			HIS_NORMAL, 
			CRect(nHeaderWidth, HeaderRect.top, HeaderRect.right + 2, HeaderRect.bottom), 
			NULL);

		if ( ( !mbShowThemed || mHeaderTheme.IsThemeNull() ) && nHeaderWidth > 0 )
		{
			CPen LinePen;

			LinePen.CreatePen(PS_SOLID, 1, mHeaderBorderColor);

			PaintDC.SelectPen(LinePen);
			PaintDC.MoveTo(HeaderRect.left, HeaderRect.bottom);
			PaintDC.LineTo(HeaderRect.right, HeaderRect.bottom);
		}
	}
	
	void DrawRoundRect( CDCHandle dcPaint, CRect& rcRect, COLORREF rgbOuter, COLORREF rgbInner )
	{
		CRect rcRoundRect( rcRect );
					
		CPen penBorder;
		penBorder.CreatePen( PS_SOLID, 1, rgbOuter );
		CBrush bshInterior;
		bshInterior.CreateSolidBrush( mBKColor );
		
		dcPaint.SelectPen( penBorder );
		dcPaint.SelectBrush( bshInterior );
		
		dcPaint.RoundRect( rcRoundRect, CPoint( 5, 5 ) );
		rcRoundRect.DeflateRect( 1, 1 );
		
		CPen penInnerBorder;
		penInnerBorder.CreatePen( PS_SOLID, 1, rgbInner );
		dcPaint.SelectPen( penInnerBorder );
		
		dcPaint.RoundRect( rcRoundRect, CPoint( 2, 2 ) );
	}
	
	void DrawGradient( CDCHandle dcPaint, CRect& rcRect, COLORREF rgbTop, COLORREF rgbBottom )
	{
		GRADIENT_RECT grdRect = { 0, 1 };
		TRIVERTEX triVertext[ 2 ] = 
		{
			rcRect.left,
			rcRect.top,
			GetRValue( rgbTop ) << 8,
			GetGValue( rgbTop ) << 8,
			GetBValue( rgbTop ) << 8,
			0x0000,			
			rcRect.right,
			rcRect.bottom,
			GetRValue( rgbBottom ) << 8,
			GetGValue( rgbBottom ) << 8,
			GetBValue( rgbBottom ) << 8,
			0x0000
		};
		
		dcPaint.GradientFill( triVertext, 2, &grdRect, 1, GRADIENT_FILL_RECT_V );
	}
	
	void DrawList( 
		CDCHandle PaintDC)
	{
		T *pT = static_cast<T *>(this);
		
		CRect ClipRect;
		if (PaintDC.GetClipBox(ClipRect) == ERROR)
			return;
		
		CRect ItemRect;

		ItemRect.left = -GetScrollPos(SB_HORZ);
		ItemRect.right = GetTotalWidth();
		ItemRect.top = (mbShowHeader ? mnHeaderHeight : 0);
		ItemRect.bottom = ItemRect.top;
		
		for (int nItem = GetTopItem(); nItem < pT->GetItemCount(); ItemRect.top = ItemRect.bottom, nItem ++)
		{
			ItemRect.bottom = ItemRect.top + mnItemHeight;
			
			if (ItemRect.bottom < ClipRect.top || ItemRect.right < ClipRect.left)
				continue;			

			if (ItemRect.top > ClipRect.bottom || ItemRect.left > ClipRect.right)
				break;
			
			pT->DrawItem(PaintDC, nItem, ItemRect);
		}
	}
	
	void DrawItem( 
		CDCHandle PaintDC, 
		int nItem, 
		CRect &ItemRect)
	{
		T *pT = static_cast<T *>(this);
		
		CRect ClipRect;
		if (PaintDC.GetClipBox(ClipRect) == ERROR)
			return;
			
		UINT nItemFlags = pT->GetItemFlags(nItem, 0);
		if (nItemFlags & ITEM_FLAGS_HIDE)
			return;

		int nFocusItem = NULL_ITEM;
		int nFocusSubItem = NULL_SUBITEM;

		GetFocusItem(nFocusItem, nFocusSubItem);
		
		BOOL bSelectedItem = IsSelected(nItem);
		BOOL bControlFocus = (GetFocus() == m_hWnd || m_bEditItem);
		
		CPen LinePen;

		LinePen.CreatePen(PS_SOLID, 1, RGB(234, 233, 225));

		if (bSelectedItem)
		{
			if (mbShowThemed && !mHeaderTheme.IsThemeNull())
			{
				DrawRoundRect( 
					PaintDC, 
					ItemRect, 
					bControlFocus ? mSelectOuterColor : mNoFocusOuterColor, 
					bControlFocus ? mSelectInnerColor : mNoFocusInnerColor);
				
				CRect SelectRect(ItemRect);
				SelectRect.DeflateRect(2, 2);
				
				DrawGradient( 
					PaintDC, 
					SelectRect, 
					bControlFocus ? mSelectTopColor : mNoFocusTopColor, 
					bControlFocus ? mSelectBottomColor : mNoFocusBottomColor);
			}else
			{
				PaintDC.SelectPen((HPEN)LinePen);

				PaintDC.MoveTo(ItemRect.left, ItemRect.top);
				PaintDC.LineTo(ItemRect.right, ItemRect.top);

				ItemRect.top ++;
				PaintDC.FillSolidRect(ItemRect, mSelectedItemColor);
				ItemRect.top --;
			}
		}
		
		PaintDC.SelectPen((HPEN)LinePen);

		PaintDC.MoveTo(ItemRect.left, ItemRect.bottom);
		PaintDC.LineTo(ItemRect.right, ItemRect.bottom);

		CRect SubItemRect( ItemRect );
		SubItemRect.right = SubItemRect.left;
		
		for ( int nSubItem = 0, nColumnCount = GetColumnCount(); nSubItem < nColumnCount; SubItemRect.left = SubItemRect.right + 1, nSubItem++ )
		{
			CListColumnEx ColumnList;
			if ( !GetColumn( nSubItem, ColumnList ) )
				break;
			
			SubItemRect.right = SubItemRect.left + ColumnList.m_nWidth - 1;
			
			if ( SubItemRect.right < ClipRect.left || SubItemRect.Width() == 0 )
				continue;

			if ( SubItemRect.left > ClipRect.right )
				break;
			
			CString sItemText = pT->GetItemText( nItem, ColumnList.m_nIndex );
			int nItemImage = pT->GetItemImage( nItem, ColumnList.m_nIndex );
			UINT nItemFormat = pT->GetItemFormat( nItem, ColumnList.m_nIndex );
			UINT nItemFlags = pT->GetItemFlags( nItem, ColumnList.m_nIndex );
			
			if ( nItemFormat == ITEM_FORMAT_CUSTOM )
			{
				pT->DrawCustomItem( PaintDC, nItem, nSubItem, SubItemRect );
				return;
			}
			
			BOOL bFocusSubItem = ( m_bFocusSubItem && nFocusItem == nItem && nFocusSubItem == nSubItem );
			
			COLORREF BKColor = mBKColor;
			COLORREF TextColor = mItemTextColor;
			
			if ( bFocusSubItem )
			{
				if ( mbShowThemed && !mHeaderTheme.IsThemeNull() )
				{
					if ( bControlFocus )
					{
						DrawRoundRect( PaintDC, SubItemRect, bControlFocus ? mSelectOuterColor : mNoFocusOuterColor, bControlFocus ? mSelectInnerColor : mNoFocusInnerColor );
				
						CRect rcSelect( SubItemRect );
						rcSelect.DeflateRect( 2, 2 );
				
						if ( !m_bEditItem )
						{
							DrawGradient( PaintDC, rcSelect, mFocusTopColor, mFocusBottomColor );
						}else
						{
							PaintDC.SetBkColor( mBKColor );
							PaintDC.ExtTextOut( rcSelect.left, rcSelect.top, ETO_OPAQUE, rcSelect, _T( "" ), 0, NULL );
						}
					}
				}else
				{
					PaintDC.SetBkColor( m_bEditItem ? mBKColor : mItemFocusColor );
					PaintDC.ExtTextOut( SubItemRect.left, SubItemRect.top, ETO_OPAQUE, SubItemRect, _T( "" ), 0, NULL );
					
					if ( m_bEditItem )
					{
						CBrush bshSelectFrame;
						bshSelectFrame.CreateSolidBrush( mItemFocusColor );
						PaintDC.FrameRect( SubItemRect, bshSelectFrame );
					}
				}
			}else if ( pT->GetItemColor( nItem, nSubItem, BKColor, TextColor ) && BKColor != mBKColor )
			{
				CPen penBorder;
				penBorder.CreatePen( PS_SOLID, 1, BKColor );
				CBrush bshInterior;
				bshInterior.CreateSolidBrush( BKColor );
				
				PaintDC.SelectPen( penBorder );
				PaintDC.SelectBrush( bshInterior );
				
				PaintDC.RoundRect( SubItemRect, CPoint( 3, 3 ) );
			}
			
			CRect ItemTextRect( SubItemRect );
			
			ItemTextRect.left += nSubItem == 0 ? 4 : 3;
			ItemTextRect.DeflateRect( 4, 0 );
			
			if ( !m_ilItemImages.IsNull() && nItemImage != ITEM_IMAGE_NONE && ( !m_bEditItem || ( m_bEditItem && !bFocusSubItem ) ) )
			{
				CSize sizeIcon;
				m_ilItemImages.GetIconSize( sizeIcon );
				
				CRect rcItemImage;
				rcItemImage.left = sItemText.IsEmpty() ? ( ( ItemTextRect.left + ItemTextRect.right ) / 2 ) - ( sizeIcon.cx / 2 ) - ( ( !mbShowThemed || mHeaderTheme.IsThemeNull() ) ? 0 : 1 ) : ItemTextRect.left;
				rcItemImage.right = min( rcItemImage.left + sizeIcon.cx, SubItemRect.right );
				rcItemImage.top = ( ( SubItemRect.top + SubItemRect.bottom ) / 2 ) - ( sizeIcon.cy / 2 );
				rcItemImage.bottom = min( rcItemImage.top + sizeIcon.cy, SubItemRect.bottom );
				
				m_ilItemImages.DrawEx( nItemImage, PaintDC, rcItemImage, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT );
					
				ItemTextRect.left += sizeIcon.cx + 4;
			}
			
			if ( ItemTextRect.IsRectEmpty() )
				continue;
			
			PaintDC.SelectFont( pT->GetItemFont( nItem, nSubItem ) );
			//PaintDC.SetTextColor( ( ( !m_bShowThemed || m_thmHeader.IsThemeNull() ) && bSelectedItem && !bFocusSubItem ) ? mSelectedTextColor : TextColor );
			PaintDC.SetTextColor(TextColor);
			PaintDC.SetBkMode( TRANSPARENT );

			UINT nFormat = DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS;

			if ( nItemFlags & ITEM_FLAGS_CENTRE )
				nFormat |= DT_CENTER;
			else if ( nItemFlags & ITEM_FLAGS_RIGHT )
				nFormat |= DT_RIGHT;
			else
				nFormat |= DT_LEFT;
		
			switch ( nItemFormat )
			{
				case ITEM_FORMAT_DATETIME:			
					if ( !sItemText.IsEmpty() )
					{
						SYSTEMTIME stItemDate;
						if ( !GetItemDate( nItem, ColumnList.m_nIndex, stItemDate ) )
							break;
														
						CString strItemDate;
						if ( nItemFlags & ITEM_FLAGS_DATE_ONLY )
							strItemDate = FormatDate( stItemDate );
						else if ( nItemFlags & ITEM_FLAGS_TIME_ONLY )
							strItemDate = FormatTime( stItemDate );
						else
							strItemDate = FormatDate( stItemDate ) + _T( " " ) + FormatTime( stItemDate );															

						PaintDC.DrawText( strItemDate, strItemDate.GetLength(), ItemTextRect, nFormat );
					}
					break;

				case ITEM_FORMAT_CHECKBOX:			
				case ITEM_FORMAT_CHECKBOX_3STATE:	
					{
						CSize IconSize;
						mCtrlImageList.GetIconSize(IconSize);
				
						CRect CheckBoxRect;

						CheckBoxRect.left = ItemTextRect.left;
						CheckBoxRect.right = CheckBoxRect.left + IconSize.cx;
						CheckBoxRect.top = ( (SubItemRect.top + SubItemRect.bottom) / 2 ) - (IconSize.cy / 2);
						CheckBoxRect.bottom = min(CheckBoxRect.top + IconSize.cy, SubItemRect.bottom);
											
						int nCheckValue = pT->GetItemCheck(nItem, ColumnList.m_nIndex);
																
						if (nItemFormat == ITEM_FORMAT_CHECKBOX)
						{
							mCtrlImageList.DrawEx( 
								nCheckValue > 0 ? ITEM_IMAGE_CHECK_ON : ITEM_IMAGE_CHECK_OFF, 
								PaintDC, 
								CheckBoxRect, 
								CLR_DEFAULT, 
								CLR_DEFAULT, 
								ILD_TRANSPARENT);
						}else
						{
							int nCheckImage = ITEM_IMAGE_3STATE_UNDEF;
							if (nCheckValue < 0)
								nCheckImage = ITEM_IMAGE_3STATE_OFF;
							else if (nCheckValue > 0)
								nCheckImage = ITEM_IMAGE_3STATE_ON;

							mCtrlImageList.DrawEx(nCheckImage, PaintDC, CheckBoxRect, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT);
						}

						if ( !sItemText.IsEmpty() )
						{
							ItemTextRect.left += IconSize.cx + 4;
							PaintDC.DrawText( sItemText, sItemText.GetLength(), ItemTextRect, nFormat );
						}
					}													
					break;

				case ITEM_FORMAT_PROGRESS:			
					if ( mbShowThemed && !m_thmProgress.IsThemeNull() && !( nItemFlags & ITEM_FLAGS_PROGRESS_SOLID ) )
					{
						CRect rcProgress( SubItemRect );
						rcProgress.DeflateRect( 3, 2 );
						m_thmProgress.DrawThemeBackground( PaintDC, PP_BAR, 0, rcProgress, NULL );
														
						rcProgress.DeflateRect( 3, 3, 2, 3 );
						rcProgress.right = rcProgress.left + (int)( (double)rcProgress.Width() * ( ( max( min( _tstof( sItemText ), 100 ), 0 ) ) / 100.0 ) );
						m_thmProgress.DrawThemeBackground( PaintDC, PP_CHUNK, 0, rcProgress, NULL );
					}else
					{
						CRect rcProgress( SubItemRect );
						rcProgress.DeflateRect( 3, 2 );
																												
						rcProgress.DeflateRect( 3, 3 );
						rcProgress.right = rcProgress.left + (int)( (double)rcProgress.Width() * ( ( max( min( _tstof( sItemText ), 100 ), 0 ) ) / 100.0 ) );
						DrawGradient( PaintDC, rcProgress, mProgressTopColor, mProgressBottomColor );
					}					
					break;

				case ITEM_FORMAT_HYPERLINK:			
					if ( nItem == m_nHotItem && nSubItem == m_nHotSubItem && !( nItemFlags & ITEM_FLAGS_READ_ONLY ) )
					{
						PaintDC.SelectFont( m_fntUnderlineFont );
						PaintDC.SetTextColor( mHyperLinkColor );
					}

				case ITEM_FORMAT_FULL_LINE:
					ItemRect.top ++;
					PaintDC.FillSolidRect(ItemRect, BKColor);
					ItemRect.top --;

					PaintDC.SelectPen((HPEN)LinePen);

					PaintDC.MoveTo(ItemRect.left, ItemRect.top);
					PaintDC.LineTo(ItemRect.right, ItemRect.top);

					if (GetItemExpandCount(nItem) > 0)
					{
						CSize IconSize;
						mCtrlImageList.GetIconSize(IconSize);

						CRect IconRect;

						IconRect.left = ItemTextRect.left - 2;
						IconRect.right = IconRect.left + IconSize.cx;
						IconRect.top = ( (SubItemRect.top + SubItemRect.bottom) / 2 ) - (IconSize.cy / 2);
						IconRect.bottom = min(IconRect.top + IconSize.cy, SubItemRect.bottom);

						int nIcon = ITEM_IMAGE_EXPAND;

						if (IsItemHide(nItem) == TRUE)
							nIcon = ITEM_IMAGE_COLLAPSE;

						mCtrlImageList.DrawEx(nIcon, PaintDC, IconRect, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT);
					}

					ItemTextRect = ItemRect;
					ItemTextRect.left += 28;
					//not break;

				default:							
					if ( !sItemText.IsEmpty() )
						PaintDC.DrawText( sItemText, sItemText.GetLength(), ItemTextRect, nFormat );

					break;
			}

			if (nItemFormat == ITEM_FORMAT_FULL_LINE)
				break;
		}
	}
	
	void DrawSelect( CDCHandle dcPaint )
	{
		if ( !m_bGroupSelect )
			return;
		
		int nHorzScroll = GetScrollPos( SB_HORZ );
		int nVertScroll = GetScrollPos( SB_VERT );
		
		CRect rcGroupSelect( m_rcGroupSelect );
		rcGroupSelect.OffsetRect( -nHorzScroll, -nVertScroll );
		
		CRect rcClient;
		GetClientRect( rcClient );
		rcClient.top = ( mbShowHeader ? mnHeaderHeight : 0 );
		
		// limit box to list client area if scrolled to limits
		if ( nHorzScroll > ( GetTotalWidth() - rcClient.Width() ) )
			rcGroupSelect.right = min( rcClient.right, rcGroupSelect.right );
		if ( nHorzScroll == 0 )
			rcGroupSelect.left = max( rcClient.left, rcGroupSelect.left );
		if ( nVertScroll > ( GetTotalHeight() - rcClient.Height() ) )
			rcGroupSelect.bottom = min( rcClient.bottom, rcGroupSelect.bottom );
		if ( nVertScroll == 0 )
			rcGroupSelect.top = max( rcClient.top, rcGroupSelect.top );
		
		// limit bitmap to client area
		CRect rcSelectArea( rcGroupSelect );
		rcSelectArea.IntersectRect( rcSelectArea, rcClient );
		
		CDC dcBackground;
		dcBackground.CreateCompatibleDC( dcPaint );
		
		int nBackgroundContext = dcBackground.SaveDC();
		
		CBitmap bmpBackground;
		bmpBackground.CreateCompatibleBitmap( dcPaint, rcSelectArea.Width(), rcSelectArea.Height() ); 
		dcBackground.SelectBitmap( bmpBackground );
		
		// take a copy of existing backgroud
		dcBackground.BitBlt( 0, 0, rcSelectArea.Width(), rcSelectArea.Height(), dcPaint, rcSelectArea.left, rcSelectArea.top, SRCCOPY );
		
		CDC dcGroupSelect;
		dcGroupSelect.CreateCompatibleDC( dcPaint );
		
		int nGroupSelectContext = dcGroupSelect.SaveDC();
		
		CBitmap bmpGroupSelect;
		bmpGroupSelect.CreateCompatibleBitmap( dcPaint, rcSelectArea.Width(), rcSelectArea.Height() ); 
		dcGroupSelect.SelectBitmap( bmpGroupSelect );
		
		// draw group select box
		dcGroupSelect.SetBkColor( ( !mbShowThemed || mHeaderTheme.IsThemeNull() ) ? mItemFocusColor : mSelectedItemColor );
		dcGroupSelect.ExtTextOut( 0, 0, ETO_OPAQUE, CRect( CPoint( 0 ), rcSelectArea.Size() ), _T( "" ), 0, NULL );
		
		BLENDFUNCTION blendFunction;
		blendFunction.BlendOp = AC_SRC_OVER;
		blendFunction.BlendFlags = 0;
		blendFunction.SourceConstantAlpha = 180;
		blendFunction.AlphaFormat = 0;
		
		// blend existing background with selection box
		dcGroupSelect.AlphaBlend( 0, 0, rcSelectArea.Width(), rcSelectArea.Height(), dcBackground, 0, 0, rcSelectArea.Width(), rcSelectArea.Height(), blendFunction ); 
		
		// draw blended selection box
		dcPaint.BitBlt( rcSelectArea.left, rcSelectArea.top, rcSelectArea.Width(), rcSelectArea.Height(), dcGroupSelect, 0, 0, SRCCOPY );
		
		// draw selection box frame
		CBrush bshSelectFrame;
		bshSelectFrame.CreateSolidBrush( ( !mbShowThemed || mHeaderTheme.IsThemeNull() ) ? mItemTextColor : mSelectedItemColor );
		dcPaint.FrameRect( rcGroupSelect, bshSelectFrame );
		
		dcBackground.RestoreDC( nBackgroundContext );
		dcGroupSelect.RestoreDC( nGroupSelectContext );
	}
	
	void DrawCustomItem( CDCHandle dcPaint, int nItem, int nSubItem, CRect& rCSubItemEx )
	{
		ATLASSERT( FALSE ); // must be implemented in a derived class
	}
};

struct CSubItemEx
{
	CString sText;
	int nCheck;
	int nImage;
	UINT unFormat;
	UINT unFlags;
	HFONT hFont;
	COLORREF BKColor;
	COLORREF TextColor;

	CListArray<CString> mComboList;
	CListArray<int> mExpandList;
};

template <class TData = DWORD>
struct CListItemEx
{
	CListArray <CSubItemEx> mSubItems;
	CString msToolTip;
	TData mtData;	
};

template < class TData >
class CListCtrlDataEx : public CListImplEx< CListCtrlDataEx< TData > >
{
public:
	DECLARE_WND_CLASS( _T( "ListCtrl" ) )

public:
	CListArray <CListItemEx<TData>> mItems;
	
public:
	int AddItem( 
		CListItemEx<TData> &listItem)
	{
		if (!mItems.Add(listItem))
			return -1;

		return CListImplEx<CListCtrlDataEx>::AddItem() ? GetItemCount() - 1 : -1;
	}

    int InsertItem(CListItemEx<TData> &listItem)
    {
        if (!mItems.InsertAt(0, listItem))
            return -1;

        CListImplEx<CListCtrlDataEx>::AddItem();

        return 0;
    }

    int InsertItem( 
        LPCTSTR lpszText, 
        int nImage = ITEM_IMAGE_NONE, 
        UINT nFormat = ITEM_FORMAT_NONE, 
        UINT nFlags = ITEM_FLAGS_NONE)
    {
        CSubItemEx listSubItem;

        listSubItem.nImage = ITEM_IMAGE_NONE;
        listSubItem.nCheck = 0;
        listSubItem.unFormat = ITEM_FORMAT_NONE;
        listSubItem.unFlags = ValidateFlags( nFlags );
        listSubItem.hFont = NULL;
        listSubItem.BKColor = mBKColor;
        listSubItem.TextColor = mItemTextColor;

        CListItemEx< TData > listItem;
        for ( int nSubItem = 0; nSubItem < GetColumnCount(); nSubItem++ )
            listItem.mSubItems.Add( listSubItem );

        // set item details for first subitem
        listItem.mSubItems[0].sText = lpszText;
        listItem.mSubItems[0].nImage = nImage;
        listItem.mSubItems[0].unFormat = nFormat;

        return InsertItem( listItem );
    }
	
	int AddItem( 
		LPCTSTR lpszText, 
		int nImage = ITEM_IMAGE_NONE, 
		UINT nFormat = ITEM_FORMAT_NONE, 
		UINT nFlags = ITEM_FLAGS_NONE)
	{
		CSubItemEx listSubItem;

		listSubItem.nImage = ITEM_IMAGE_NONE;
		listSubItem.nCheck = 0;
		listSubItem.unFormat = ITEM_FORMAT_NONE;
		listSubItem.unFlags = ValidateFlags( nFlags );
		listSubItem.hFont = NULL;
		listSubItem.BKColor = mBKColor;
		listSubItem.TextColor = mItemTextColor;
		
		CListItemEx< TData > listItem;
		for ( int nSubItem = 0; nSubItem < GetColumnCount(); nSubItem++ )
			listItem.mSubItems.Add( listSubItem );
			
		// set item details for first subitem
		listItem.mSubItems[0].sText = lpszText;
		listItem.mSubItems[0].nImage = nImage;
		listItem.mSubItems[0].unFormat = nFormat;
		
		return AddItem( listItem );
	}
	
	BOOL DeleteItem( int nItem )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		return mItems.RemoveAt( nItem ) ? CListImplEx< CListCtrlDataEx >::DeleteItem( nItem ) : FALSE;
	}
	
	BOOL DeleteAllItems()
	{
		mItems.RemoveAll();
		return CListImplEx< CListCtrlDataEx >::DeleteAllItems();
	}
	
	int GetItemCount()
	{
		return mItems.GetSize();
	}
	
	BOOL GetItem( 
		int nItem, 
		CListItemEx<TData> &listItem)
	{
		if (nItem < 0 || nItem >= GetItemCount()) 
			return FALSE;

		listItem = mItems[nItem];

		return TRUE;
	}
	
	BOOL GetSubItem( 
		int nItem, 
		int nSubItem, 
		CSubItemEx &listSubItem )
	{
		CListItemEx<TData> listItem;
		if (!GetItem(nItem, listItem))
			return FALSE;

		if (nSubItem < 0 || nSubItem >= (int)listItem.mSubItems.GetSize())
			return FALSE;

		listSubItem = listItem.mSubItems[nSubItem];

		return TRUE;
	}
	
	CString GetItemText( int nItem, int nSubItem )
	{
		CSubItemEx listSubItem;
		return GetSubItem( nItem, nSubItem, listSubItem ) ? listSubItem.sText : _T( "" );
	}
	
	int GetItemImage( int nItem, int nSubItem )
	{
		CSubItemEx listSubItem;
		return GetSubItem( nItem, nSubItem, listSubItem ) ? listSubItem.nImage : ITEM_IMAGE_NONE;
	}
	
	virtual UINT GetItemFormat( int nItem, int nSubItem )
	{
		CSubItemEx listSubItem;
		if ( !GetSubItem( nItem, nSubItem, listSubItem ) )
			return FALSE;

		return listSubItem.unFormat == ITEM_FORMAT_NONE ? GetColumnFormat( IndexToOrder( nSubItem ) ) : listSubItem.unFormat;
	}
	
	virtual UINT GetItemFlags( int nItem, int nSubItem )
	{
		CSubItemEx listSubItem;
		if ( !GetSubItem( nItem, nSubItem, listSubItem ) )
			return FALSE;
		return listSubItem.unFlags == ITEM_FLAGS_NONE ? GetColumnFlags( IndexToOrder( nSubItem ) ) : listSubItem.unFlags;
	}
	
	BOOL GetItemComboList( int nItem, int nSubItem, CListArray < CString >& aComboList )
	{
		CSubItemEx listSubItem;
		if ( !GetSubItem( nItem, nSubItem, listSubItem ) )
			return FALSE;

		aComboList = listSubItem.mComboList;
		return aComboList.IsEmpty() ? GetColumnComboList( IndexToOrder( nSubItem ), aComboList ) : !aComboList.IsEmpty();
	}

	HFONT GetItemFont( int nItem, int nSubItem )
	{
		CSubItemEx listSubItem;
		if ( !GetSubItem( nItem, nSubItem, listSubItem ) )
			return FALSE;
		return listSubItem.hFont == NULL ? CListImplEx< CListCtrlDataEx >::GetItemFont( nItem, nSubItem ) : listSubItem.hFont;
	}
	
	BOOL GetItemColor( 
		int nItem, 
		int nSubItem, 
		COLORREF &BKColor, 
		COLORREF &TextColor)
	{
		CSubItemEx listSubItem;
		if (!GetSubItem(nItem, nSubItem, listSubItem))
			return FALSE;

		BKColor = listSubItem.BKColor;
		TextColor = listSubItem.TextColor;

		return TRUE;
	}
	
	CString GetItemToolTip( int nItem, int nSubItem )
	{
		CListItemEx< TData > listItem;
		return GetItem( nItem, listItem ) ? listItem.msToolTip : _T( "" );
	}
	
	BOOL GetItemData( int nItem, TData& tData )
	{
		CListItemEx< TData > listItem;
		if ( !GetItem( nItem, listItem ) )
			return FALSE;
		tData = listItem.mtData;
		return TRUE;
	}
	
	BOOL SetItemCheck( int nItem, int nSubItem, int nCheckValue )
	{
		if (nItem < 0 || nItem >= GetItemCount()) 
			return FALSE;

		if (nSubItem < 0 || nSubItem >= (int)mItems[nItem].mSubItems.GetSize())
			return FALSE;

		mItems[nItem].mSubItems[nSubItem].nCheck = nCheckValue;

		return TRUE;
	}

	virtual BOOL SetItemFlags( 
		int nItem, 
		int nSubItem, 
		UINT unFlags)
	{
		if (nItem < 0 || nItem >= GetItemCount()) 
			return FALSE;

		if (nSubItem < 0 || nSubItem >= (int)mItems[nItem].mSubItems.GetSize())
			return FALSE;

		mItems[nItem].mSubItems[nSubItem].unFlags = unFlags;

		return TRUE;
	}

	int GetItemCheck(int nItem, int nSubItem)
	{
		if (nItem < 0 || nItem >= GetItemCount()) 
			return -2;

		if (nSubItem < 0 || nSubItem >= (int)mItems[nItem].mSubItems.GetSize())
			return -2;

		return mItems[nItem].mSubItems[nSubItem].nCheck;
	}

	BOOL SetItemText( int nItem, int nSubItem, LPCTSTR lpszText )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;

		if ( nSubItem < 0 || nSubItem >= (int)mItems[ nItem ].mSubItems.GetSize() )
			return FALSE;

		mItems[ nItem ].mSubItems[ nSubItem ].sText = lpszText;

		return TRUE;
	}
	
	void AddItemExpand(
		int nItem,
		int nExpandItem)
	{
		mItems[nItem].mSubItems[0].mExpandList.Add(nExpandItem);
	}

	BOOL SetItemComboIndex( int nItem, int nSubItem, int nIndex )
	{
		CListArray < CString > aComboList;
		if ( !GetItemComboList( nItem, nSubItem, aComboList ) )
			return FALSE;
		return SetItemText( nItem, nSubItem, nIndex < 0 || nIndex >= aComboList.GetSize() ? _T( "" ) : aComboList[ nIndex ] );
	}
	
	BOOL SetItemImage( int nItem, int nSubItem, int nImage )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		if ( nSubItem < 0 || nSubItem >= (int)mItems[ nItem ].mSubItems.GetSize() )
			return FALSE;
		mItems[ nItem ].mSubItems[ nSubItem ].m_nImage = nImage;
		return TRUE;
	}
	
	BOOL SetItemFormat( int nItem, int nSubItem, UINT nFormat, UINT nFlags = ITEM_FLAGS_NONE )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		if ( nSubItem < 0 || nSubItem >= (int)mItems[ nItem ].mSubItems.GetSize() )
			return FALSE;
		mItems[ nItem ].mSubItems[ nSubItem ].m_nFormat = nFormat;
		mItems[ nItem ].mSubItems[ nSubItem ].m_nFlags = nFlags;
		return TRUE;
	}
	
	BOOL SetItemFormat( int nItem, int nSubItem, UINT nFormat, UINT nFlags, CListArray < CString >& aComboList )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		if ( nSubItem < 0 || nSubItem >= (int)mItems[ nItem ].mSubItems.GetSize() )
			return FALSE;
		mItems[ nItem ].mSubItems[ nSubItem ].m_nFormat = nFormat;
		mItems[ nItem ].mSubItems[ nSubItem ].m_nFlags = nFlags;
		mItems[ nItem ].mSubItems[ nSubItem ].m_aComboList = aComboList;
		return TRUE;
	}
	
	BOOL SetItemFont( int nItem, int nSubItem, HFONT hFont )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		if ( nSubItem < 0 || nSubItem >= (int)mItems[ nItem ].mSubItems.GetSize() )
			return FALSE;
		mItems[ nItem ].mSubItems[ nSubItem ].m_hFont = hFont;
		return TRUE;
	}
	
	BOOL SetItemColor( int nItem, int nSubItem, COLORREF BKColor, COLORREF TextColor )
	{
		if ( nItem < 0 || nItem >= GetItemCount() )
			return FALSE;

		if ( nSubItem < 0 || nSubItem >= (int)mItems[ nItem ].mSubItems.GetSize() )
			return FALSE;

		if (BKColor != -1)
			mItems[nItem].mSubItems[nSubItem].BKColor = BKColor;

		if (TextColor != -1)
			mItems[nItem].mSubItems[nSubItem].TextColor = TextColor;

		return TRUE;
	}
	
	void ReverseItems()
	{
		mItems.Reverse();
	}
	
	class CompareItem
	{
	public:
		CompareItem( int nColumn ) : m_nColumn( nColumn ) {}
		inline bool operator() ( const CListItemEx< TData >& listItem1, const CListItemEx< TData >& listItem2 )
		{
			return ( listItem1.mSubItems[ m_nColumn ].sText.Compare( listItem2.mSubItems[ m_nColumn ].sText ) < 0 );
		}
		
	protected:
		int m_nColumn;
	};
	
	void SortItems( int nColumn, BOOL bAscending )
	{
		mItems.Sort( CompareItem( nColumn ) );
	}
	
	BOOL SetItemToolTip( int nItem, LPCTSTR lpszToolTip )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		mItems[ nItem ].msToolTip = lpszToolTip;
		return TRUE;
	}
	
	BOOL SetItemData( int nItem, TData tData )
	{
		if ( nItem < 0 || nItem >= GetItemCount() ) 
			return FALSE;
		mItems[ nItem ].mtData = tData;
		return TRUE;
	}
};

//typedef CListCtrlDataEx< DWORD > CListCtrlEx;
