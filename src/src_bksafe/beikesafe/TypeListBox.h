#pragma once

#include "stdafx.h"
#include <bkres/bkres.h>

//////////////////////////////////////////////////////////////////////////
// Type List Box Control
//

// 定义用到的颜色常量
#define		COL_TYPE_LIST_BG_NORMAL		RGB( 0xE3, 0xED, 0xF8 )
#define		COL_TYPE_LIST_BG_SELECTED	RGB( 0x42, 0x90, 0xDA )
#define		COL_TYPE_LIST_BG_MOUSE_IN	RGB( 0xB4, 0xD9, 0xFF )

#define		COL_TYPE_LIST_BORDER_LINE	RGB( 0xA3, 0xBA, 0xD2 )
#define		COL_TYPE_LIST_LOWER_LINE	RGB( 0xB5, 0xD3, 0xDE )
#define		COL_TYPE_LIST_UPPER_LINE	RGB( 0xFF, 0xFF, 0xFF )

#define		COL_TYPE_NAME_NORMAL		RGB( 0x00, 0x00, 0x00 )



#define		MOUSE_IN_ITEM_MAGIC_NUMBER		2000

#define		SCROLL_BTN_HEIGHT		20
#define		SCROLL_BTN_WIDTH		134

#define TYPE_ITEM_HEIGHT	27

#define SC_UP 2 //Up scroll
#define SC_DOWN 3 //Down Scroll

#define SC_NORMAL					0 // Normal scroll
#define SC_MOUSEOVER				1 // mouse over
#define SC_PRESSED					2 // The scroll is pressed
#define SC_DISABLED					3 // The scroll is disabled


class CTypeListItemData
{
public:
	CTypeListItemData( int nOffset, DWORD Id, COLORREF colType, LPCTSTR TypeName, int inumber )
	{
		nTextOffset = nOffset;
		nId			= Id;
		colTypeName = colType;
		strTypeName = TypeName;
		nNumber		= inumber;
	}

	virtual	~CTypeListItemData()
	{
	
	}

public:
	
	int			nTextOffset;		// 显示文字的偏移
	CString		strTypeName;		// 分类名
	COLORREF	colTypeName;		// 可以对不同的分类设置颜色
	DWORD		nId;				// 分类的ID
	int			nNumber;			// 数量  -1表示不显示
};


class IClickCallback
{
public:
	virtual void	OnClick( int nListId, CTypeListItemData * pData) = 0;
};

class CTypeListBox : 
	public CWindowImpl<CTypeListBox, CListBox>, 
	public COwnerDraw<CTypeListBox>
{
public:
	CTypeListBox(void)
	{
		m_ClickCB = NULL;
		
		// 内部使用的标志变量
		m_bNoPaintItem = FALSE;
		m_bTrackFlag = FALSE;
		m_bMouseInScrollUp = FALSE;
		m_bMouseInScrollDown = FALSE;
		m_bNcTrackFlag = FALSE;
		m_bScroll = FALSE;
		m_MaxHeight = 0;
		m_nScrollTopestIndex = -1;
		m_nSelectID = -1;
		m_bNoScrollBtn = TRUE;

		m_bSetFocusOnMouseMove = TRUE;

		m_fntNormal.Attach( BkFontPool::GetFont( BKF_DEFAULTFONT ) );
		m_fntSelected.Attach( BkFontPool::GetFont( TRUE, FALSE, FALSE, 0 ) );

	}

	~CTypeListBox(void)
	{
		m_fntSelected.Detach();
		m_fntNormal.Detach();
	}

protected:

	CBrush	m_hBGBrush;

	BOOL	m_bSetFocusOnMouseMove;

	CFont		m_fntNormal;
	CFont		m_fntSelected;	// 被点击选中后的字体

	BOOL	m_bNoPaintItem;
	BOOL	m_bTrackFlag;

	BOOL	m_bMouseInScrollUp;
	BOOL	m_bMouseInScrollDown;
	BOOL	m_bNcTrackFlag;

	BOOL    m_bNoScrollBtn;
	BOOL	m_bScroll;
	int		m_MaxHeight;

	int		m_nScrollTopestIndex;		// 当listbox中的数据滚动到最高点时显示在最上面的行的索引

	CRect	m_ItemRect;
	CRect	m_LastRect;

	IClickCallback*	m_ClickCB;

	int		m_nListId;					// 创建ListBox时使用的ID， 保存它用于OnClick回调时区分同一个窗体中的不同ListBox
	int		m_nSelectID;

	CSimpleArray<CTypeListItemData*>	m_arrData;

public:
	BEGIN_MSG_MAP(CTypeListBox)   
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_NCPAINT, OnNcPaint)
		MESSAGE_HANDLER(WM_NCHITTEST, OnNcHitTest)
		MESSAGE_HANDLER(WM_NCLBUTTONDOWN, OnNcLButtonDown)
		MESSAGE_HANDLER(WM_NCLBUTTONDBLCLK, OnNcLButtonDown)
		MESSAGE_HANDLER(WM_NCMOUSEMOVE, OnNcMouseMove)
		MESSAGE_HANDLER(WM_NCMOUSELEAVE, OnNcMouseLeave)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_NCCALCSIZE(OnNcCalcSize)
		MSG_OCM_CTLCOLORLISTBOX(OnCtlColor)
		MSG_WM_SETFOCUS(OnSetFocus)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CTypeListBox>,1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()   

	void ShowScrollBtn(BOOL bShow = TRUE)
	{
		m_bNoScrollBtn = bShow;
	}

	void OnSetFocus(HWND hWnd) 
	{
		GetParent().GetParent().SetFocus();
		return;
	}

	LRESULT	OnCtlColor(HDC hDc, HWND hWnd)
	{
		if (hWnd==m_hWnd)
		{
			CDCHandle	dchan;
			dchan.Attach(hDc);

			SetMsgHandled(TRUE);

			if (m_hBGBrush.m_hBrush==NULL)
				m_hBGBrush.CreateSolidBrush(COL_TYPE_LIST_BG_NORMAL);

			DrawBorder();

			dchan.Detach();
			return (LRESULT)m_hBGBrush.m_hBrush;
		}
		SetMsgHandled(FALSE);
		return NULL;
	}

	void SetFocusOnMouseMove( BOOL bSetFocus )
	{
		m_bSetFocusOnMouseMove = bSetFocus;
	}

	void SetNumber( int nIndex,  int n )
	{
		CTypeListItemData * p = GetItemDataX( nIndex );

		p->nNumber = n;

		CRect rt;
		if ( GetItemRect( nIndex, &rt ) )
		{
			InvalidateRect( &rt );
		}
	}

	void SetCallBack( IClickCallback * pCallBack )
	{
		m_ClickCB = pCallBack;
	}

	void	SetListID( int nListId )
	{
		m_nListId = nListId;
	}

	HRESULT	OnLButtonDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
	{
		POINT	pts;
		pts.x		= GET_X_LPARAM(lParam);
		pts.y		= GET_Y_LPARAM(lParam);

		for ( int i=0; i<this->GetCount(); i++ )
		{
			CRect r;
			GetItemRect( i, r );
			if (PtInRect(&r, pts))
			{
				m_nSelectID = i;
				SetCurSel(m_nSelectID);
				Invalidate(FALSE);
				break;
			}
		}

		return S_OK;
	}

	HRESULT	OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
	{
		POINT	pts;
		BOOL	bOutSide;

		pts.x		= GET_X_LPARAM(lParam);
		pts.y		= GET_Y_LPARAM(lParam);

		INT nIndex;
		if ( m_nSelectID != -1 )
		{
			nIndex = m_nSelectID;
			CListBox::ItemFromPoint(pts,bOutSide);
		}
		else
		{
			nIndex = CListBox::ItemFromPoint(pts,bOutSide);
		}
		
		if ( !bOutSide && nIndex >= 0 && nIndex < 0xffff )
		{
			CTypeListItemData*	pData = (CTypeListItemData*)GetItemDataX(nIndex);
			if ( m_ClickCB != NULL )
			{
				m_ClickCB->OnClick( m_nListId, pData );
			}		
		}

		bMsgHandled = FALSE;
		return	S_OK;
	}

	HRESULT	OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
	{
		bMsgHandled = TRUE;

		// 首先刷新一下鼠标悬停区域的颜色
		if ( m_ItemRect.top != MOUSE_IN_ITEM_MAGIC_NUMBER )
		{
			m_ItemRect.bottom = m_ItemRect.top = MOUSE_IN_ITEM_MAGIC_NUMBER;
			InvalidateRect( m_LastRect );
			m_LastRect = m_ItemRect;
		}

		if ( wParam & 0x10000000 )
		{
			// rotated backward
			SendMessage( WM_VSCROLL, SB_LINEDOWN );
		}
		else
		{
			// rotated forward
			SendMessage( WM_VSCROLL, SB_LINEUP );
		}

		DrawScrolls( SC_UP, SC_NORMAL );
		DrawScrolls( SC_DOWN, SC_NORMAL );

		return S_OK;
	}

	HRESULT	OnMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
	{
		 m_bTrackFlag = FALSE;

		 m_ItemRect.bottom = m_ItemRect.top = MOUSE_IN_ITEM_MAGIC_NUMBER;
		 InvalidateRect( m_LastRect );
		 m_LastRect = m_ItemRect;

		 return S_OK;
	}

	void OnTimer(UINT_PTR nIDEvent)
	{
		//Gets the state of the left button to see if it is pressed
		short result=GetKeyState(VK_LBUTTON);

		if (nIDEvent==1){ //Up timer

			//If it returns negative then it is pressed
			if ( result >= 0 ) 
			{ //No longer pressed
				
				KillTimer(1); 
				DrawScrolls( SC_UP, SC_MOUSEOVER ); 
			}
			else
			{
				SendMessage(WM_VSCROLL,MAKEWPARAM(SB_LINEUP,0),0); 
				DrawScrolls(SC_UP, SC_PRESSED);
				DrawScrolls(SC_DOWN, SC_NORMAL);
			}
			
		}
		else 
		{ //Down timer

			if ( result >= 0 ) 
			{
				KillTimer(2);  
				DrawScrolls(SC_DOWN,SC_MOUSEOVER); 
			} 
			else
			{
				SendMessage(WM_VSCROLL,MAKEWPARAM(SB_LINEDOWN,0),0);
				DrawScrolls( SC_UP, SC_NORMAL );
				DrawScrolls( SC_DOWN, SC_PRESSED );
			}

		}
	}

	// 再试一下， 或者这个的消息映射方式也应该改成 Message_Handler 
	LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)		
	{
		NCCALCSIZE_PARAMS * lpncsp = ( NCCALCSIZE_PARAMS * ) lParam;

		int		nClientHight = lpncsp->rgrc[0].bottom - lpncsp->rgrc[0].top;
		int		nAllItemHight = 0;
		BOOL	bScroll = FALSE;

		for ( int i=0; i<this->GetCount(); i++ )
		{
			nAllItemHight += GetItemHeight( i );
		}

		if ( m_bNoScrollBtn && nAllItemHight + SCROLL_BTN_HEIGHT > nClientHight )
		{
			m_MaxHeight = lpncsp->rgrc[0].bottom - lpncsp->rgrc[0].top - 1;

			lpncsp->rgrc[0].top += SCROLL_BTN_HEIGHT;
			lpncsp->rgrc[0].bottom -= SCROLL_BTN_HEIGHT;
			bScroll = TRUE;
			nClientHight -= SCROLL_BTN_HEIGHT * 2;
		}
		else
		{
			bScroll = FALSE;
			m_MaxHeight = 0;
		}

		// 确定 m_nScrollTopestIndex
		int		nItemHeight = GetItemHeight( 0 );
		int		nDisplayCount;
		if ( nItemHeight > 0 )
		{
			nDisplayCount = nClientHight / nItemHeight;
		}
		else
		{
			nDisplayCount = 0;
		}
			
		m_nScrollTopestIndex = GetCount() - nDisplayCount;

		
		m_bScroll = bScroll;

		SendMessage( WM_NCPAINT );		// 修改后必须重绘一次才生效

		// CListBox::OnNcCalcSize(bCalcValidRects, lParam);
		return 0;
	}

	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

		// 获取List当前的位置， 相对于屏幕左上角
		RECT irect = { 0 };
		GetWindowRect(&irect);

		// 获得当前鼠标位置
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if ( lRet == HTNOWHERE )
		{
			if ( pt.x > irect.left && pt.x < irect.right )
			{
				if ( pt.y > irect.top && pt.y < irect.top + SCROLL_BTN_HEIGHT )
				{
					lRet = HTVSCROLL;
				}
				else if ( pt.y > irect.bottom - SCROLL_BTN_HEIGHT && pt.y < irect.bottom )
				{
					lRet = HTHSCROLL;
				}
			}
		}

		return lRet;
	}

	LRESULT OnNcLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{

		CString str;
		str.Format(_T("123"));
		OutputDebugString(str);

		// 获取List当前的位置， 相对于屏幕左上角
		RECT irect = { 0 };
		GetWindowRect(&irect);

		// 获得当前鼠标位置
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if ( pt.x > irect.left && pt.x < irect.right )
		{
			if ( pt.y > irect.top && pt.y < irect.top + SCROLL_BTN_HEIGHT )
			{
				DrawScrolls( SC_UP, SC_PRESSED );
				SendMessage(WM_VSCROLL,MAKEWPARAM(SB_LINEUP,0),0);  
				DrawScrolls( SC_DOWN, SC_NORMAL );

				DrawBorder();

				SetTimer( 1, 150, NULL );		
			}
			else if ( pt.y > irect.bottom - SCROLL_BTN_HEIGHT && pt.y < irect.bottom )
			{
				DrawScrolls( SC_DOWN, SC_PRESSED );
				SendMessage(WM_VSCROLL,MAKEWPARAM(SB_LINEDOWN,0),0);
				DrawScrolls( SC_UP, SC_NORMAL );

				DrawBorder();

				SetTimer( 2, 150, NULL );
			}
		}

		return DefWindowProc(uMsg, wParam, lParam);
	//	return 0;
	}

	LRESULT OnNcMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{

		// 获取List当前的位置， 相对于屏幕左上角
		RECT irect = { 0 };
		GetWindowRect(&irect);

		if ( !m_bNcTrackFlag )
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.hwndTrack = this->m_hWnd;
			tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
			tme.dwHoverTime = 0;
			m_bNcTrackFlag = _TrackMouseEvent(&tme);
		}

		// 获得当前鼠标位置
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if ( pt.x > irect.left && pt.x < irect.right )
		{
			if ( pt.y > irect.top && pt.y < irect.top + SCROLL_BTN_HEIGHT )
			{
				if ( !m_bMouseInScrollUp )
				{
					DrawScrolls( SC_UP, SC_MOUSEOVER );
					m_bMouseInScrollUp = TRUE;
				}		
			}
			else if ( pt.y > irect.bottom - SCROLL_BTN_HEIGHT && pt.y < irect.bottom )
			{
				if ( !m_bMouseInScrollDown)
				{
					DrawScrolls( SC_DOWN, SC_MOUSEOVER );
					m_bMouseInScrollDown = TRUE;
				}
			}
		}

		return DefWindowProc(uMsg, wParam, lParam);
		// return 0;
	}

	HRESULT	OnNcMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
	{
		m_bNcTrackFlag = FALSE;

		if ( m_bMouseInScrollUp )
		{
			DrawScrolls( SC_UP, SC_NORMAL );
			m_bMouseInScrollUp = FALSE;
		}

		if ( m_bMouseInScrollDown )
		{
			DrawScrolls( SC_DOWN, SC_NORMAL );
			m_bMouseInScrollDown = FALSE;
		}

		return	S_OK;
	}

	HRESULT	OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
	{
		POINT	pts;
		CRect	rcItem;
		CRect	rcOut( -1, -1, -1, -1 );
		BOOL	bOutSide = FALSE;
		BOOL	bOntip=FALSE;
		BOOL	bMove=FALSE;

		if (!m_bTrackFlag)
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.hwndTrack = this->m_hWnd;
			tme.dwFlags = TME_LEAVE;
			tme.dwHoverTime = 0;
			m_bTrackFlag = _TrackMouseEvent(&tme);

			if ( m_bSetFocusOnMouseMove )
			{
//				this->SetFocus();
			}	
		}

		bMsgHandled = FALSE;
		pts.x		= GET_X_LPARAM(lParam);
		pts.y		= GET_Y_LPARAM(lParam);

		int nIndex = -1;
		for ( int i=0; i<this->GetCount(); i++ )
		{
			CRect r;
			GetItemRect( i, r );
			if (PtInRect(&r, pts))
			{
				nIndex = i;
				break;
			}
		}
		if ( nIndex >= 0 && nIndex < 0xffff && GetItemRect(nIndex,&rcItem) )
		{
			if ( m_ItemRect == rcItem )
			{
				return	S_OK;
			}
			else
			{
				m_ItemRect = rcItem;
				InvalidateRect( &rcItem );
			}
		}
		else if (nIndex == -1)
		{
			if ( m_ItemRect == rcOut )
			{
				return	S_OK;
			}
			else
			{
				m_ItemRect = rcOut;
				InvalidateRect( m_LastRect );
			}
		}

		return	S_OK;
	}

	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		static BOOL before=FALSE;
		if ( !before ) 
		{
			// 指在真正显示这个测栏的时候被呼叫
			SetWindowPos(NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE); 
			before=TRUE;
		}

		DrawBorder();

		DrawScrolls(SC_UP,SC_NORMAL);
		DrawScrolls(SC_DOWN,SC_NORMAL);
	
		return 0;
	}


	void DrawScrolls(UINT WhichOne, UINT State)
	{

		if ( !m_bScroll )
		{
			return;
		}

		CRect rect;
		GetClientRect(rect); //Gets the dimensions

		if ( rect.bottom - rect.top > m_MaxHeight )
		{
			return;
		}

		CDC iDC;
		HDC	h = GetDC();
		
		CDCHandle	han;
		han.Attach( h );
		iDC.CreateCompatibleDC( han );

		int		nCurrentTopIndex = GetTopIndex();

		if ( rect.bottom > 0 )
		{
			if (WhichOne==SC_UP)
			{ 
				//The one to draw is the up one
				//Draws the scroll up
				rect.top = -SCROLL_BTN_HEIGHT;
				rect.bottom = 0;
				
				HBITMAP hBmpCheck	= BkBmpPool::GetBitmap( IDB_TYPELIST_SCROLL_UP );
				HBITMAP hBmpOld		= iDC.SelectBitmap(hBmpCheck);

				if ( nCurrentTopIndex == 0 )
				{
					State = SC_DISABLED;
				}

				han.BitBlt( rect.left, rect.top, rect.Width(),rect.Height(), iDC, State * SCROLL_BTN_WIDTH, 0, SRCCOPY );

				iDC.SelectBitmap(hBmpOld);
			}
			else
			{ 
				//Needs to draw down
				rect.top = rect.bottom;
				rect.bottom = rect.bottom + SCROLL_BTN_HEIGHT;

				HBITMAP hBmpCheck	= BkBmpPool::GetBitmap( IDB_TYPELIST_SCROLL_DOWN );
				HBITMAP hBmpOld		= iDC.SelectBitmap(hBmpCheck);

				if ( nCurrentTopIndex == m_nScrollTopestIndex )
				{
					State = SC_DISABLED;
				}

				han.BitBlt( rect.left, rect.top, rect.Width() - 1,rect.Height(), iDC, State * SCROLL_BTN_WIDTH, 0, SRCCOPY );
				iDC.SelectBitmap(hBmpOld);

			} 

			HPEN	hPenTmp;
			CPen	pen_border;
			pen_border.CreatePen( PS_SOLID, 1, COL_TYPE_LIST_BORDER_LINE );
			hPenTmp = han.SelectPen( pen_border );

			han.MoveTo( rect.right - 1, rect.top );
			han.LineTo( rect.right - 1, rect.bottom );

			// 恢复
			han.SelectPen( hPenTmp );
		}  
		
		han.Detach();
		ReleaseDC( h );

		InvalidateRect( &rect );
	}

	void DrawBorder()
	{
		CDC iDC;
		HDC	h = GetDC();

		CDCHandle	han;
		han.Attach( h );
		iDC.CreateCompatibleDC( han );

		CRect rect;
		GetClientRect(rect); //Gets the dimensions

		HPEN	hPenTmp;
		CPen	pen_border;
		pen_border.CreatePen( PS_SOLID, 1, COL_TYPE_LIST_BORDER_LINE );
		hPenTmp = han.SelectPen( pen_border );

		int		nIndex = GetCurSel();
		CRect	SelRect;
		int	nret = GetItemRect( nIndex, &SelRect );
		if ( nret != LB_ERR )
		{
			//if ( rect.top < SelRect.top && SelRect.bottom < rect.bottom )
			//{
			//	han.MoveTo( rect.right - 1, rect.top );
			//	han.LineTo( rect.right - 1, SelRect.top );

			//	han.MoveTo( rect.right -1, SelRect.bottom );
			//	han.LineTo( rect.right - 1, rect.bottom );
			//}
			//else if ( SelRect.bottom <= rect.top || SelRect.top >= rect.bottom  )
			//{
			//	han.MoveTo( rect.right - 1, rect.top );
			//	han.LineTo( rect.right - 1, rect.bottom );
			//}
			//else if ( SelRect.top <= rect.top && SelRect.bottom > rect.top )
			//{
			//	han.MoveTo( rect.right - 1, SelRect.bottom );
			//	han.LineTo( rect.right - 1, rect.bottom );
			//}
			//else if ( SelRect.top < rect.bottom && SelRect.bottom > rect.bottom )
			//{
			//	han.MoveTo( rect.right - 1, rect.top );
			//	han.LineTo( rect.right - 1, SelRect.top );
			//}

			// 不再画出边线的缺口
			han.MoveTo( rect.right - 1, rect.top );
			han.LineTo( rect.right - 1, rect.bottom );
			
		}
		else
		{
			han.MoveTo( rect.right - 1, rect.top );
			han.LineTo( rect.right - 1, rect.bottom );
		}

		// 恢复
		han.SelectPen( hPenTmp );
	
		han.Detach();
		ReleaseDC( h );
	}

	void GetCurSelTypeName( CString & strTypeName )
	{
		CTypeListItemData * p = GetItemDataX( m_nSelectID );
		if ( p != NULL )
		{
			strTypeName = p->strTypeName;
		}
		else
		{
			strTypeName = _T( "" );
		}
	}

	void SetCurSelTypeName( CString strTypeName, BOOL bOnClick = FALSE)
	{
		for ( int i=0; i<m_arrData.GetSize(); i++ )
		{
			CTypeListItemData * p = GetItemDataX( i );
			if ( p != NULL && p->strTypeName == strTypeName )
			{
				m_nSelectID = i;
				SetCurSel( i );
				if (bOnClick)
				{
					m_ClickCB->OnClick(m_nListId, p);
				}
				break;
			}
		}
	}

	void SetNumberByName( CString strTypeName, int nNumber )
	{
		for ( int i=0; i<m_arrData.GetSize(); i++ )
		{
			CTypeListItemData * p = GetItemDataX( i );
			if ( p != NULL && p->strTypeName == strTypeName )
			{
				p->nNumber = nNumber;
				break;
			}
		}
	}

	int GetNumberByName( CString strTypeName )
	{
		for ( int i=0; i<m_arrData.GetSize(); i++ )
		{
			CTypeListItemData * p = GetItemDataX( i );
			if ( p != NULL && p->strTypeName == strTypeName )
			{
				return p->nNumber;
			}
		}

		return 0;		// 没找到
	}

	void DrawItem ( LPDRAWITEMSTRUCT lpdis )
	{
		if ( m_bNoPaintItem )
		{
			return;
		}

		CRect rcWindow;
		GetWindowRect( rcWindow );

		CTypeListItemData * pListData	= GetItemDataX( lpdis->itemID );
		if ( pListData != NULL )
		{
			CDCHandle	dcx;
			CDC			dcTmp;
			HFONT		hFntTmp;
			DWORD		colTxtTmp;
			DWORD		nOldMod;

			dcx.Attach( lpdis->hDC );
			dcTmp.CreateCompatibleDC( dcx );

			nOldMod = dcx.GetBkMode();
			dcx.SetBkMode( TRANSPARENT );

			hFntTmp = dcx.SelectFont( m_fntNormal );
			colTxtTmp = dcx.GetTextColor();

			// 判断当前的item是否被选中
			BOOL	bSelect = FALSE;
			if ( ( lpdis->itemAction | ODA_SELECT )
				&& ( lpdis->itemState & ODS_SELECTED ) )
			{
				bSelect = TRUE;
				m_nSelectID = lpdis->itemID;
				dcx.SelectFont( m_fntSelected );
			}

			if ( bSelect )
			{
				dcx.FillSolidRect( &( lpdis->rcItem ), COL_TYPE_LIST_BG_SELECTED );
			}
			else if ( m_ItemRect == lpdis->rcItem )
			{
				dcx.FillSolidRect( &( lpdis->rcItem ), COL_TYPE_LIST_BG_MOUSE_IN );
				if ( m_LastRect != m_ItemRect )
				{
					InvalidateRect( m_LastRect );
					m_LastRect = m_ItemRect;
				}
			}
			else
			{
				dcx.FillSolidRect( &( lpdis->rcItem ), COL_TYPE_LIST_BG_NORMAL );
			}

			if ( bSelect )
			{
				dcx.SetTextColor( RGB( 0xFF, 0xFF, 0xFF ) );
			}
			else
			{
				// 设置文字颜色
				dcx.SetTextColor( pListData->colTypeName );
			}
			

			CRect	rcText = lpdis->rcItem;
			rcText.left += pListData->nTextOffset;

			CString		strDisplay;
			if ( pListData->nNumber == -1 )
			{
				strDisplay = pListData->strTypeName;
			}
			else
			{
				strDisplay.Format( _T("%s(%d)"), pListData->strTypeName, pListData->nNumber );
			}
			dcx.DrawText( strDisplay, -1, rcText, DT_VCENTER | DT_LEFT | DT_SINGLELINE );

			// 画线
			HPEN	hPenTmp;
			CPen	pen_up;

			pen_up.CreatePen( PS_SOLID, 1, COL_TYPE_LIST_UPPER_LINE );	
			
			hPenTmp = dcx.SelectPen( pen_up );

			if ( lpdis->itemID != GetTopIndex() && !bSelect )
			{
				dcx.MoveTo( lpdis->rcItem.left + 1, lpdis->rcItem.top );
				dcx.LineTo( lpdis->rcItem.right - 1, lpdis->rcItem.top );
			}
						
			CPen	pen_down;
			if ( !bSelect )
			{
				pen_down.CreatePen( PS_SOLID, 1, COL_TYPE_LIST_LOWER_LINE );
			}
			else
			{
				pen_down.CreatePen( PS_SOLID, 1, COL_TYPE_LIST_BORDER_LINE );
			}
			
			dcx.SelectPen( pen_down );

			
			dcx.MoveTo( lpdis->rcItem.left + 1, lpdis->rcItem.bottom - 1 );

			if ( bSelect )
			{
				dcx.LineTo( lpdis->rcItem.right, lpdis->rcItem.bottom - 1 );
			}
			else
			{
				if ( lpdis->itemID != GetCount() - 1 )
				{
					dcx.LineTo( lpdis->rcItem.right - 1, lpdis->rcItem.bottom - 1 );
				}		
			}
				

			CPen	pen_border;
			pen_border.CreatePen( PS_SOLID, 1, COL_TYPE_LIST_BORDER_LINE );
			dcx.SelectPen( pen_border );
		
			if ( !bSelect )
			{
				dcx.MoveTo( lpdis->rcItem.right - 1, lpdis->rcItem.top );
				dcx.LineTo( lpdis->rcItem.right - 1, lpdis->rcItem.bottom + 1 );
			}
			

			// 恢复
			dcx.SelectPen( hPenTmp );
			dcx.SetTextColor( colTxtTmp );
			dcx.SelectFont( hFntTmp );
			dcx.SetBkMode( nOldMod );

		}
	}

	int SetItemDataX(int nIndex, DWORD_PTR dwItemData)
	{
		CTypeListItemData*	pItemData=(CTypeListItemData*)dwItemData;
		ATLASSERT(pItemData!=NULL);

		for (int i= 0;i<m_arrData.GetSize();i++)
		{
			if ( m_arrData[i]->nId == pItemData->nId )
			{
				ATLASSERT(FALSE);
				return -1;
			}
		}

		m_arrData.Add(pItemData);
		return SetItemData( nIndex, ( DWORD_PTR ) pItemData->nId );
	}

	CTypeListItemData*	GetItemDataX(int i)
	{
		DWORD	nItemId = (DWORD)GetItemData(i);

		for ( int i=0; i<m_arrData.GetSize(); i++)
		{
			if ( m_arrData[i] != NULL && m_arrData[i]->nId == nItemId)
				return m_arrData[i];
		}
		return NULL;
	}
	
	VOID DeleteAllItems()
	{
		for ( INT i= 0; i < m_arrData.GetSize(); i++)
		{
			CTypeListItemData*	pData = m_arrData[i];
			if ( pData )
			{
				delete pData;
			}
		}

		m_arrData.RemoveAll();
		CListBox::ResetContent();
	}

	void RefreshItem(int nIndex)
	{
		ATLASSERT(nIndex >= 0 && nIndex < m_arrData.GetSize());
		if (nIndex < 0 || nIndex >= m_arrData.GetSize() || (IsWindow() == FALSE))
		{
			return;
		}

		CRect rcItem;
		GetItemRect(nIndex, &rcItem);
		InvalidateRect(&rcItem, FALSE);
	}

	BOOL CheckItemExist(CString strText)
	{
		for (int i = 0; i < m_arrData.GetSize(); i++)
		{
			CTypeListItemData*	pData = NULL;
			pData = m_arrData[i];
			if ( pData != NULL && pData->strTypeName == strText)
			{
				return TRUE;
			}
		}

		return FALSE;
	}
};