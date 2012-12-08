////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for WTL GUI routines file
//      
//      File      : tracktooltip.h
//      Comment   : 跟随鼠标的tooltip
//      
//      Create at : 2009-03-26
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once


#define TT_PADDING_LEFT		7
#define TT_PADDING_TOP		7
#define TT_PADDING_BOTTOM	7
#define TT_PADDING_RIGHT	7

#define	TIMER_OUT_ID		500
#define TIMER_ID_TIP		1024
#define TIMER_ID_TIP2       1025

class  KTrackTooltip : public CWindowImpl< KTrackTooltip >
{
public:
	KTrackTooltip():
	m_bTTVisble(FALSE),
	m_nIndex(-1),
	m_bRelay(FALSE),
	m_nTimeout(TIMER_OUT_ID),
	m_nPer1(0),
	m_nPer2(0)
	{

	}

	~KTrackTooltip()
	{
		UnInit();
	}

	BEGIN_MSG_MAP(KTrackTooltip)
		MESSAGE_HANDLER(WM_PAINT,				OnPaint)
		MESSAGE_HANDLER(WM_CREATE,				OnCreate)
		MESSAGE_HANDLER(WM_TIMER,				OnTimer)
	END_MSG_MAP()

	static DWORD GetWndStyle( DWORD dwStyle )
	{
		return WS_POPUP;
	}

	static DWORD GetWndExStyle( DWORD dwStyleEx )
	{
		return WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return TRUE;
	}

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if( wParam == TIMER_ID_TIP )
		{
			KillTimer( TIMER_ID_TIP );
			if( m_bTTVisble == FALSE )
			{
				ShowWindow( SW_HIDE );
			}
		}
		if( wParam == TIMER_ID_TIP2 )
		{
			KillTimer(TIMER_ID_TIP2);
			if (m_bTTVisble == TRUE)
			{
				ShowWindow(SW_SHOW);
			}
		}
		return TRUE;
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPaintDC dc( m_hWnd );

		CRect rect;
		GetClientRect( rect );
		HFONT hOldFont = dc.SelectFont( m_hFont );
		dc.SetBkMode( TRANSPARENT );

		dc.FillSolidRect( rect, RGB(0xFF, 0xFF, 0xE1 ) );
		CBrush brush( ::CreateSolidBrush(RGB(0x00, 0x00, 0x00)));
		dc.FrameRect( rect, brush );
		
		//dc.DrawText( m_strText, m_strText.GetLength(), rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		rect.left += TT_PADDING_LEFT * 2;
		rect.top += TT_PADDING_TOP;
		rect.right -= TT_PADDING_RIGHT;
		rect.left -= TT_PADDING_BOTTOM;

		//draw_text_in_rect( dc, m_strText, m_strText.GetLength(), rect, 2 );
		for (int n = 0; n< m_strText.GetSize(); n++)
		{
			if (n == 0 || n == 4 )
			{
				HFONT	fntOld=dc.SelectFont(KuiFontPool::GetFont(TRUE,FALSE,FALSE));
				//dc.DrawText( strTitle, -1, &rcItem, nFlag);
				DrawText(dc, m_strText[n], m_strText[n].GetLength(), rect, DT_LEFT);
				dc.SelectFont(fntOld);
			}
			else
			{
				CRect rcTemp;
				if (n == 3)
				{
					int nProSize = 0;
					nProSize = 60 - (int)((m_nPer1 * 60) / 100);
					rcTemp = rect;
					rcTemp.left += 65;
					rcTemp.right = rcTemp.left + 62;
					rcTemp.top += 2;
					rcTemp.bottom = rcTemp.top + 10;
					dc.FillSolidRect(rcTemp, RGB(222,222,222));
					rcTemp.top +=1;
					rcTemp.bottom -=1;
					rcTemp.left+=1;
					rcTemp.right -= nProSize;
					dc.FillSolidRect(rcTemp, RGB(64,64,140));
				}
				if (n == 8)
				{

					int nProSize = 0;
					nProSize = 60 - (int)((m_nPer2 * 60) / 100);
					rcTemp = rect;
					rcTemp.left += 65;
					rcTemp.right = rcTemp.left + 62;
					rcTemp.top += 2;
					rcTemp.bottom = rcTemp.top + 10;
					dc.FillSolidRect(rcTemp, RGB(222,222,222));
					rcTemp.top +=1;
					rcTemp.bottom -=1;
					rcTemp.left+=1;
					rcTemp.right -= nProSize;
					dc.FillSolidRect(rcTemp, RGB(64,64,140));
				}
				DrawText(dc, m_strText[n], m_strText[n].GetLength(), rect, DT_LEFT);
				dc.SelectFont(m_hFont);
			}
			rect.top += 20;
			rect.bottom += 20;
		}

		if( hOldFont )
			dc.SelectFont( hOldFont );

		return 0;
	}

	void SetRelay( BOOL bRelay, int nTimer )
	{
		m_bRelay = bRelay;
		m_nTimeout = nTimer;
	}


	void Init( HWND hParent )
	{
		if( IsWindow() )
			return;

		Create( hParent );
		ShowWindow( SW_HIDE );
//		m_strText = TEXT("  ");
		m_strText.RemoveAll();

		LOGFONT lf;
		RtlZeroMemory(&lf, sizeof(LOGFONT));
		memset(&lf, 0, sizeof(LOGFONT));
		lf.lfHeight = 12 ;
		lf.lfWeight = 400;
		lf.lfUnderline = FALSE;
		_tcscpy(lf.lfFaceName, TEXT("宋体") );
		m_hFont.CreateFontIndirect(&lf);

		return;
	}

	void UnInit()
	{
		if ( IsWindow() )
		{
			DestroyWindow( );
		}
	}

	BOOL ActiveTooltip( BOOL bActive )
	{		
		if ( bActive == TRUE && m_bTTVisble == TRUE )
		{
			return TRUE;
		}
		
		if ( IsWindow() )
		{
			if( bActive == TRUE )
			{
				SetWindowPos( NULL, 0, 0 , 0, 0, SWP_NOSIZE | SWP_NOMOVE |SWP_NOREPOSITION | SWP_SHOWWINDOW | SWP_NOACTIVATE );
				m_bTTVisble = TRUE;
			}
			else
			{
				if( m_bRelay )
					SetTimer( TIMER_ID_TIP, m_nTimeout, NULL );
				else
					ShowWindow( SW_HIDE );
				m_bTTVisble = FALSE;
			}
		}

		return TRUE;
	}

	// nIndex 用于btn类型优化。Heartbeat 里面不能优化，nIndex = -1
	BOOL ShowText( CPoint& point, CSimpleArray<CString>& pszShowText, int nIndex, int nPer1, int nPer2 )
	{
		m_nPer2 = nPer2;
		m_nPer1 = nPer1;
		if( IsWindow() )
		{
			/*if( abs(point.x - m_pt.x) > 2 || abs( point.y - m_pt.y) > 2 )*/
			{
				SetWindowPos( NULL, point.x + 16, point.y + 16 , 0, 0, SWP_NOSIZE | SWP_NOREPOSITION | SWP_SHOWWINDOW | SWP_NOACTIVATE );
				m_pt = point;
			}

			if( nIndex != m_nIndex || nIndex == -1 )
			{	
				UpdateTipText( pszShowText );
				m_nIndex = nIndex;
			}
			ShowWindow(SW_HIDE);

			SetTimer(TIMER_ID_TIP2, 1000, NULL);
		}

		return TRUE;
	}

	BOOL UpdateTipText( CSimpleArray<CString>& pszShowText )
	{
		if( pszShowText.GetSize() == 0 )
			return FALSE;
		
		m_strText.RemoveAll();
		m_strText = pszShowText;
// 		for (int n = 0; n <= pszShowText.GetSize() - 1; ++n)
// 		{
// 			m_strText.Add(pszShowText[n]);
// 		}

		ActiveTooltip( TRUE );

		CDC dc (::GetDC( m_hWnd ) );
		HFONT hOldFont = dc.SelectFont( m_hFont );
		SIZE size;
		
		size.cx = 200;
		size.cy = 190;

		if( hOldFont )
			dc.SelectFont( hOldFont );

		SetWindowPos( NULL, 0, 0, size.cx + TT_PADDING_LEFT + TT_PADDING_RIGHT,
			size.cy + TT_PADDING_BOTTOM + TT_PADDING_TOP, SWP_NOMOVE | SWP_NOREPOSITION | SWP_SHOWWINDOW | SWP_NOACTIVATE);

		Invalidate();

	
		return TRUE;
	}

private:
	BOOL						m_bTTVisble;
	CSimpleArray<CString>		m_strText;
	CFont						m_hFont;
	CPoint						m_pt;

	int							m_nIndex;	//用于优化效率
	BOOL						m_bRelay;
	int							m_nTimeout;

	int                         m_nPer1;
	int                         m_nPer2;
};