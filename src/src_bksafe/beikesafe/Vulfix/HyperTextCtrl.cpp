#include "StdAfx.h"
#include "HyperTextCtrl.h"

#define COLOR_NORMAL RGB(0,0,0)
#define COLOR_LINK RGB(40,100,165)
#define COLOR_LINK_HOVER RGB(0xff,0,0)

CHyperTextCtrl::CHyperTextCtrl(void)
{
#if 1
	m_font.Attach( BkFontPool::GetFont(BKF_DEFAULTFONT));
	m_fontLink.Attach( BkFontPool::GetFont(FALSE,TRUE,FALSE,0) );
	m_fontBold.Attach( BkFontPool::GetFont(TRUE,FALSE,FALSE) );
#else
	m_font.CreateFont(12,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY ,DEFAULT_PITCH, _T("ËÎÌו"));
	m_fontLink.CreateFont(12,0,0,0,FW_NORMAL,0,1,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY ,DEFAULT_PITCH, _T("ËÎÌו"));
	m_fontBold.CreateFont(12,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY ,DEFAULT_PITCH, _T("ËÎÌו"));
#endif 
	m_bOverControl = FALSE;
	m_bLinkRectValid = FALSE;
	m_bShowHandle = FALSE;
	m_dwFlags = DT_LEFT;
	m_clrBackground = RGB(0xfb,0xfc,0xfd);
}

CHyperTextCtrl::~CHyperTextCtrl(void)
{
#if 1
	m_font.Detach();
	m_fontLink.Detach();
	m_fontBold.Detach();
#else
	m_font.DeleteObject();
	m_fontLink.DeleteObject();
	m_fontBold.DeleteObject();
#endif
}

void CHyperTextCtrl::SetMText( LPCTSTR szText )
{
	m_bLinkRectValid = FALSE;
	m_tt.clear();
	m_tt_rects.clear();
	m_textparser.Parse( szText, m_tt );
	Invalidate();
}

void CHyperTextCtrl::SetBackgroudColor( COLORREF rgb )
{
	m_clrBackground = rgb;	
}

void CHyperTextCtrl::SetFlags( UINT uFlags )
{
	m_dwFlags = uFlags;
}

BOOL CHyperTextCtrl::GetCurrentLinkID( CPoint point, INT &id )
{
	INT nIdx = _GetHoverLinkIndex(point);
	ATLTRACE(_T("down : %d\n"), nIdx);
	if(nIdx>=0)
	{
		TextPart &t = m_tt[ nIdx ];
		attributes::iterator it = t.attrs.find(_T("id"));
		if( it==t.attrs.end() )
			return FALSE;
		id = _ttoi( it->second.c_str() );
		return TRUE;
	}
	return FALSE;
}

void CHyperTextCtrl::ReleaseMouse()
{
	m_bOverControl = FALSE;
	ReleaseCapture();
	RedrawWindow();
}

void CHyperTextCtrl::OnPaint( CDCHandle )
{
	CPaintDC dcPaint(m_hWnd);
	{
		CRect rcClient;
		GetClientRect( &rcClient );

		CPoint pt;
		GetCursorPos( &pt );
		ScreenToClient( &pt );
		
		CDC memDC;
		CBitmap memBitmap;
		memDC.CreateCompatibleDC( dcPaint );
		memBitmap.CreateCompatibleBitmap( dcPaint, rcClient.Width(), rcClient.Height() );
		HFONT hOldFont = memDC.SelectFont( m_font );
		HBITMAP hOldBitmap = memDC.SelectBitmap( memBitmap );
		memDC.SetBkMode( TRANSPARENT );
		
		if( !m_bLinkRectValid )
			_RecalcItemRects();
		
		memDC.FillSolidRect( &rcClient, m_clrBackground );
		int nItems = m_tt.size();
		for(int i=0; i<nItems; ++i)
		{
			TextPart &t = m_tt[i];
			RECT rcItem = m_tt_rects[i];
			UINT uFlags = DT_SINGLELINE|DT_LEFT|DT_NOCLIP;
			if(t.isLink())
			{
				BOOL hover = m_bOverControl && PtInRect( &rcItem, pt );
				memDC.SelectFont( hover ? m_fontLink : m_font );
				memDC.SetTextColor( hover ? COLOR_LINK_HOVER : COLOR_LINK );
			}
			else if(t.isBold())
			{
				memDC.SelectFont( m_fontBold );
				memDC.SetTextColor( COLOR_NORMAL );
			}
			else
			{
				memDC.SelectFont( m_font );
				memDC.SetTextColor( COLOR_NORMAL );
			}
			memDC.DrawText( t.val.c_str(), -1, &rcItem, uFlags);
		}
		dcPaint.BitBlt(0,0, rcClient.Width(), rcClient.Height(), memDC, 0, 0, SRCCOPY); 

		memDC.SelectFont(hOldFont);
		memDC.SelectBitmap( hOldBitmap );
		memBitmap.DeleteObject();
		memDC.DeleteDC();
	}
}

BOOL CHyperTextCtrl::OnSetCursor( CWindow wnd, UINT nHitTest, UINT message )
{
	BOOL bShowHandle=_GetHoverLinkIndex()!=-1;
	::SetCursor(::LoadCursor(NULL, bShowHandle?IDC_HAND:IDC_ARROW));
	return TRUE;
}

void CHyperTextCtrl::OnSize( UINT nType, CSize size )
{
	m_bLinkRectValid = FALSE;
}

void CHyperTextCtrl::OnMouseMove( UINT nFlags, CPoint point )
{
	SetMsgHandled(FALSE);
	if (m_bOverControl)        // Cursor is currently over control
	{
		INT nidx = _GetTTIndex(point) ;
		if(nidx!=m_nLastLink)
		{
			RedrawWindow();
			m_nLastLink = nidx;
		}
		
		BOOL toShowHandle = FALSE;
		if(nidx>=0)
		{
			if(m_tt[nidx].isLink())
				toShowHandle = TRUE;
		}

		if(toShowHandle)
		{
			if(!m_bShowHandle)
				::SetCursor(::LoadCursor(NULL,IDC_HAND));
			m_bShowHandle = TRUE;
		}
		else
		{
			if(m_bShowHandle)
				::SetCursor(::LoadCursor(NULL,IDC_ARROW));
			m_bShowHandle = FALSE;
		}

		CRect rect;
		GetClientRect(rect);
		if (!rect.PtInRect(point))
		{
			ReleaseMouse();
			return;
		}
	}
	else                      // Cursor has just moved over control
	{
		m_nLastLink = -1;
		m_bOverControl = TRUE;
		m_bShowHandle = FALSE;
		RedrawWindow();
		SetCapture();
	}
}

void CHyperTextCtrl::_RecalcItemRects()
{
	m_tt_rects.clear();

	RECT rc = {0};
	GetClientRect( &rc );
	
	CDC dc = GetWindowDC();
	HFONT hOldFont = dc.SelectFont( m_font );
	
	INT nOffsetX = rc.left;
	INT nOffsetY = rc.top;
	int nItems = m_tt.size();
	for(int i=0; i<nItems; ++i)
	{
		TextPart &t = m_tt[i];
		dc.SelectFont( t.isBold() ? m_fontBold:m_font );
		CRect rcItem;
		dc.DrawText( t.val.c_str(), -1, &rcItem, DT_CALCRECT);
		rcItem.MoveToXY(nOffsetX, nOffsetY);
		nOffsetX = rcItem.right;
		m_tt_rects.push_back( rcItem );
	}
	dc.SelectFont(hOldFont);
	
	if( m_dwFlags&DT_RIGHT && !m_tt_rects.empty() )
	{
		INT w = m_tt_rects.rbegin()->right - m_tt_rects.begin()->left;
		INT o = rc.right - rc.left - w;
		for( RECTS::iterator it=m_tt_rects.begin(); it!=m_tt_rects.end(); ++it)
		{
			it->left += o;
			it->right += o;
		}
	}
	m_bLinkRectValid = TRUE;
}

INT CHyperTextCtrl::_GetTTIndex( CPoint point )
{
	if( !m_bLinkRectValid )
		_RecalcItemRects();
	for( int n=0; n<m_tt_rects.size(); ++n)
	{
		if( PtInRect( &m_tt_rects[n], point) )
		{
			return n;
		}
	}
	return -1;
}

INT CHyperTextCtrl::_GetHoverLinkIndex()
{
	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );
	return _GetHoverLinkIndex(point);
}

INT CHyperTextCtrl::_GetHoverLinkIndex( CPoint point )
{
	INT nidx = _GetTTIndex(point) ;
	if(nidx>=0)
	{
		if(m_tt[nidx].isLink())
			return nidx;
	}
	return -1;
}