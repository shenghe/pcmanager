
#pragma once

class CTitleTipEx : public CWindowImpl< CTitleTipEx >
{
public:
	CTitleTipEx()
	{
		m_hWndParent = NULL;
		m_bShowThemed = TRUE;
	}
	
	~CTitleTipEx()
	{
	}
	
	DECLARE_WND_CLASS_EX( _T( "TitleTip" ), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_SAVEBITS, COLOR_WINDOW )

protected:
	HWND m_hWndParent;
	BOOL m_bShowThemed;
	CString m_strToolTip;
	
	COLORREF m_rgbBackground;
	COLORREF m_rgbTextColour;
	COLORREF m_rgbBorderOuter;
	COLORREF m_rgbBorderInner;
	COLORREF m_rgbBackgroundTop;
	COLORREF m_rgbBackgroundBottom;
	
	CFont m_fntTitleFont;
	CToolTipCtrl m_ttToolTip;
	
public:
	BOOL Create( HWND hWndParent, BOOL bShowThemed )
	{
		m_hWndParent = hWndParent;
		m_bShowThemed = bShowThemed;
		
		m_rgbBackground = GetSysColor( COLOR_INFOBK );
		m_rgbTextColour = ( m_bShowThemed && CTheme::IsThemingSupported() ) ? GetSysColor( COLOR_WINDOWTEXT ) : GetSysColor( COLOR_INFOTEXT );		
		m_rgbBorderOuter = RGB( 220, 220, 220 );
		m_rgbBorderInner = RGB( 245, 245, 245 );
		m_rgbBackgroundTop = RGB( 250, 250, 250 );
		m_rgbBackgroundBottom = RGB( 235, 235, 235 );
		
		if ( CWindowImpl< CTitleTipEx >::Create( hWndParent, NULL, NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST ) == NULL )
			return FALSE;
		
		// create the tooltip
		if ( !m_ttToolTip.Create( m_hWnd ) )
			return FALSE;
		m_ttToolTip.SetMaxTipWidth( SHRT_MAX );
		
		// get system message font
		CLogFont logFont;
		logFont.SetMessageBoxFont();
		if ( !m_fntTitleFont.IsNull() )
			m_fntTitleFont.DeleteObject();
		return ( m_fntTitleFont.CreateFontIndirect( &logFont ) != NULL );
	}
	
	BOOL Show( CRect& rcRect, LPCTSTR lpszItemText, LPCTSTR lpszToolTip )
	{
		CString strItemText = lpszItemText;
		
		if ( !IsWindow() || strItemText.IsEmpty() )
			return FALSE;
		
		m_strToolTip = lpszToolTip;
		SetWindowText( strItemText );
		
		CClientDC dcClient( m_hWnd );
		
		HFONT hOldFont = dcClient.SelectFont( m_fntTitleFont );
			
		CRect rcTextExtent( rcRect );
				
		// calculate item text extent...
		dcClient.DrawText( strItemText, strItemText.GetLength(), rcTextExtent, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_CALCRECT );
		
		dcClient.SelectFont( hOldFont );
		
		// do not show titletip if entire text is visible
		if ( rcTextExtent.Width() <= rcRect.Width() - 1 )
			return FALSE;
		
		if ( m_strToolTip.IsEmpty() )
			m_ttToolTip.Activate( FALSE );
		else
		{
			m_ttToolTip.Activate( TRUE );
			m_ttToolTip.AddTool( m_hWnd, (LPCTSTR)m_strToolTip.Left( SHRT_MAX ) );
		}
		
		// show titletip at new location
		if ( !SetWindowPos( NULL, rcRect.left - 4, rcRect.top, rcTextExtent.Width() + 11, rcRect.Height(), SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS ) )
			return FALSE;
		
		SetCapture();
		
		return TRUE;
	}
	
	BOOL Hide()
	{
		if ( GetCapture() == m_hWnd )
			ReleaseCapture();
		return IsWindow() ? ShowWindow( SW_HIDE ) : FALSE;
	}
	
	BEGIN_MSG_MAP(CTitleTipEx)
		MSG_WM_DESTROY(OnDestroy)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseRange)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_PAINT(OnPaint)
	END_MSG_MAP()
	
	void OnDestroy()
	{
		if ( m_ttToolTip.IsWindow() )
			m_ttToolTip.DestroyWindow();
		m_ttToolTip.m_hWnd = NULL;
	}
	
	LRESULT OnMouseRange( UINT nMessage, WPARAM wParam, LPARAM lParam )
	{
		SetMsgHandled( FALSE );
		
		if ( m_ttToolTip.IsWindow() )
		{
			MSG msgRelay = { m_hWnd, nMessage, wParam, lParam };
			m_ttToolTip.RelayEvent( &msgRelay );
		}
		
		CPoint ptMouse( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
		ClientToScreen( &ptMouse );
		
		if ( nMessage == WM_MOUSEMOVE )
		{
			CRect rcWindow;
			GetWindowRect( rcWindow );		
			if ( !rcWindow.PtInRect( ptMouse ) )
				Hide();
			return 0;
		}		
										
		CWindow wndParent( m_hWndParent );
		UINT nHitTest = (UINT)wndParent.SendMessage( WM_NCHITTEST, 0, MAKELPARAM( ptMouse.x, ptMouse.y ) );
		
		// forward notifcation through to parent
		if ( nHitTest == HTCLIENT )
		{
			wndParent.ScreenToClient( &ptMouse );
			wndParent.PostMessage( nMessage, wParam, MAKELPARAM( ptMouse.x, ptMouse.y ) );
		}
		else
		{
			switch ( nMessage )
			{
				case WM_LBUTTONDOWN:	wndParent.PostMessage( WM_NCLBUTTONDOWN, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_LBUTTONUP:		wndParent.PostMessage( WM_NCLBUTTONUP, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_LBUTTONDBLCLK:	wndParent.PostMessage( WM_NCLBUTTONDBLCLK, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_RBUTTONDOWN:	wndParent.PostMessage( WM_NCRBUTTONDOWN, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_RBUTTONUP:		wndParent.PostMessage( WM_NCRBUTTONUP, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_RBUTTONDBLCLK:	wndParent.PostMessage( WM_NCRBUTTONDBLCLK, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_MBUTTONDOWN:	wndParent.PostMessage( WM_NCMBUTTONDOWN, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_MBUTTONUP:		wndParent.PostMessage( WM_NCMBUTTONUP, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
				case WM_MBUTTONDBLCLK:	wndParent.PostMessage( WM_NCMBUTTONDBLCLK, nHitTest, MAKELPARAM( ptMouse.x, ptMouse.y ) );
										break;
			}
		}

		return 0;
	}
	
	BOOL OnEraseBkgnd( HDC dc ) 
	{
		return TRUE;
	}
	
	void OnPaint( HDC )
	{
		CPaintDC dcPaint( m_hWnd );
		
		int nContextState = dcPaint.SaveDC();
		
		CRect rcClient;
		GetClientRect( rcClient );
		
		CRect rCTitleTipEx( rcClient );
					
		if ( m_bShowThemed && CTheme::IsThemingSupported() )
		{
			CPen penBorder;
			penBorder.CreatePen( PS_SOLID, 1, m_rgbBorderOuter );
			
			dcPaint.SelectPen( penBorder );
			dcPaint.SelectStockBrush( HOLLOW_BRUSH );
			
			dcPaint.RoundRect( rCTitleTipEx, CPoint( 5, 5 ) );
			rCTitleTipEx.DeflateRect( 1, 1 );
			
			CPen penInnerBorder;
			penInnerBorder.CreatePen( PS_SOLID, 1, m_rgbBorderInner );
			dcPaint.SelectPen( penInnerBorder );
			
			dcPaint.RoundRect( rCTitleTipEx, CPoint( 2, 2 ) );
			rCTitleTipEx.DeflateRect( 1, 1 );
			
			GRADIENT_RECT grdRect = { 0, 1 };
			TRIVERTEX triVertext[ 2 ] = {
											rCTitleTipEx.left,
											rCTitleTipEx.top,
											GetRValue( m_rgbBackgroundTop ) << 8,
											GetGValue( m_rgbBackgroundTop ) << 8,
											GetBValue( m_rgbBackgroundTop ) << 8,
											0x0000,			
											rCTitleTipEx.right,
											rCTitleTipEx.bottom,
											GetRValue( m_rgbBackgroundBottom ) << 8,
											GetGValue( m_rgbBackgroundBottom ) << 8,
											GetBValue( m_rgbBackgroundBottom ) << 8,
											0x0000
										};
			
			dcPaint.GradientFill( triVertext, 2, &grdRect, 1, GRADIENT_FILL_RECT_V );
		}
		else
		{
			dcPaint.SetBkColor( m_rgbBackground );
			dcPaint.ExtTextOut( rCTitleTipEx.left, rCTitleTipEx.top, ETO_OPAQUE, rCTitleTipEx, _T( "" ), 0, NULL );
			
			CBrush bshTitleFrame;
			bshTitleFrame.CreateSolidBrush( m_rgbTextColour );
			dcPaint.FrameRect( rCTitleTipEx, bshTitleFrame );
		}
		
		int nTextLength = GetWindowTextLength() + 1;
		CString strItemText;
		GetWindowText( strItemText.GetBuffer( nTextLength ), nTextLength );
		strItemText.ReleaseBuffer();
		
		dcPaint.SelectFont( m_fntTitleFont );
		dcPaint.SetTextColor( m_rgbTextColour );
		dcPaint.SetBkMode( TRANSPARENT );
		
		CRect rcItemText( rcClient );
		rcItemText.OffsetRect( 4, 0 );
		
		dcPaint.DrawText( strItemText, strItemText.GetLength(), rcItemText, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER );
	
		dcPaint.RestoreDC( nContextState );
	}
};
