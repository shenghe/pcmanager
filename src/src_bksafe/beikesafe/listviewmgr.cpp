
#include "stdafx.h"
#include "listviewmgr.h"
#include <atlframe.h>

CListViewMgr::CListViewMgr()
:m_pListParent(NULL),
m_uItemCount(0),
m_uItemSelect(-1)
{

}

CListViewMgr::~CListViewMgr()
{

}

LRESULT CListViewMgr::OnVScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled )
{
	bHandled = FALSE;
	Invalidate();
	return TRUE;
}

LRESULT CListViewMgr::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CreateToolTip();

	return TRUE;
}

LRESULT CListViewMgr::OnEraseBKGnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CDCHandle dc( (HDC)wParam );
	DoPaintGnd( dc );
	return TRUE;
}

LRESULT CListViewMgr::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ReScrollSize();
	bHandled = FALSE;
	return TRUE;
}

void CListViewMgr::ReScrollSize( BOOL bReDraw )
{
	CPoint point;
	GetScrollOffset( point );
	SetItemCount( GetItemCount(), FALSE );
	SetScrollOffset( point, TRUE );
}

LRESULT CListViewMgr::OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_wndToolTip.IsWindow())
	{
		MSG msg = { m_hWnd, uMsg, wParam, lParam };

		m_wndToolTip.RelayEvent(&msg);
	}

	SetMsgHandled(FALSE);

	return 0;
}

LRESULT CListViewMgr::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if( m_wndToolTip.IsWindow() )
		m_wndToolTip.DestroyWindow();

	return TRUE;
}

UINT CListViewMgr::_HitTest( int xPos, int yPos, CRect& rtItemOut )
{
	int nRet = -1;

	if( !m_pListParent )
		return nRet;

	CRect rtClient;
	GetClientRect(&rtClient);

	CPoint ptOffset;
	GetScrollOffset( ptOffset );

	CRect rtItem(rtClient);
	rtItem.top -= ptOffset.y;
	rtItem.bottom -= ptOffset.y;

	UINT uHeight = 0;
	for( UINT i = 0; i < m_uItemCount; i++ )
	{
		uHeight = m_pListParent->MeasureItem( i );
		rtItem.bottom = rtItem.top + uHeight;

		if( rtItem.bottom <= rtClient.top )
		{
			rtItem.top = rtItem.bottom;
			continue;
		}

		if( rtItem.top >= rtClient.bottom )
		{
			rtItem.top = rtItem.bottom;
			break;
		}

		if( rtItem.PtInRect( CPoint(xPos, yPos) ) )
		{
			rtItemOut = rtItem;
			nRet = i;
			break;
		}

		rtItem.top = rtItem.bottom;
	}
	
	return nRet;
}


void CListViewMgr::SetCallBack( IListParent* pParent )
{
	m_pListParent = pParent;
}

void CListViewMgr::SetItemCount( UINT nCnt, BOOL bReDraw )
{
	CRect rtClient;
	GetClientRect(&rtClient);
	m_uItemCount = nCnt;
	UINT uHeight = 0;

	for( UINT i = 0; i < m_uItemCount; i++ )
	{
		uHeight += m_pListParent->MeasureItem( i );
	}

	if( nCnt == 0 )
		m_uItemSelect = -1;

	if( uHeight > 0 )
		SetScrollSize( rtClient.Width() - ::GetSystemMetrics(SM_CXVSCROLL), uHeight, bReDraw );
	else
		SetScrollSize( rtClient.Width() - ::GetSystemMetrics(SM_CXVSCROLL), 1, bReDraw );
}

UINT CListViewMgr::GetItemCount( )
{
	return m_uItemCount;
}

void CListViewMgr::DoPaint( CDCHandle dc )
{

}

LRESULT CListViewMgr::OnSetCursor(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	UINT uCode = LOWORD(lParam);
	UINT uMouseMsg = HIWORD(lParam);
	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	if( uMouseMsg == 0 || uCode != HTCLIENT )
	{
		bHandled = FALSE;
		return FALSE;
	}

	if( uMouseMsg >= WM_MOUSEFIRST && uMouseMsg <= WM_MOUSELAST )
	{
		CRect rtItem;
		int nIndex = _HitTest( point.x, point.y, rtItem );
		if ( nIndex != -1 )
		{
			if( m_pListParent->ItemHitTest( nIndex, point.x - rtItem.left , point.y - rtItem.top , uMsg ) )
				SetCursor( LoadCursor( NULL, IDC_HAND) );
			else
				bHandled = FALSE;
		}
		else
		{
			bHandled = FALSE;
		}
	}
	else
	{
		bHandled = FALSE;
	}
	return TRUE;
}

LRESULT CListViewMgr::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	CRect rtItem;
	int nIndex = _HitTest( pt.x, pt.y, rtItem );

	static CPoint ptLast;

	if (ptLast == pt)
		return TRUE;

	ptLast = pt;

	if( nIndex != -1 )
	{
		LPCTSTR pszTip = m_pListParent->GetTipText( nIndex, pt.x - rtItem.left , pt.y - rtItem.top );
		if( pszTip )
		{
			m_wndToolTip.SetMaxTipWidth(500);
			m_wndToolTip.UpdateTipText((LPCTSTR)pszTip,m_hWnd);
		}
		else
		{
			m_wndToolTip.UpdateTipText((LPCTSTR)NULL,m_hWnd);
			m_wndToolTip.Pop();
		}
	}
	else
	{
		m_wndToolTip.UpdateTipText((LPCTSTR)NULL,m_hWnd);
		m_wndToolTip.Pop();
	}

	return TRUE;
}

void CListViewMgr::CreateToolTip()
{
	if ( m_wndToolTip.IsWindow() )
		return;

	if ( IsWindow() )
	{
		m_wndToolTip.Create(m_hWnd);
		CToolInfo ti(0, m_hWnd);
		m_wndToolTip.AddTool(ti);
		m_wndToolTip.Activate(TRUE);
		m_wndToolTip.SetMaxTipWidth(500);
		m_wndToolTip.SetDelayTime(TTDT_AUTOMATIC,500);
		//m_wndToolTip.SetDelayTime(TTDT_AUTOPOP,1000);
		//m_wndToolTip.SetDelayTime(TTDT_INITIAL,500);
	}
}

LRESULT CListViewMgr::OnRButtonUp(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	CRect rtItem;
	int nIndex = _HitTest( pt.x, pt.y, rtItem );
	BOOL bSelect = FALSE;
	if( nIndex != -1 )
	{
		if( m_pListParent )
		{
			bSelect = m_pListParent->ItemHitTest( nIndex, pt.x - rtItem.left , pt.y - rtItem.top, uMsg );
		}

		if( bSelect && m_uItemSelect != nIndex )
		{
			m_uItemSelect = nIndex;
			Invalidate();
		}

		SetFocus();
	}

	return TRUE;
}

LRESULT CListViewMgr::OnLButtonDown(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	CRect rtItem;
	int nIndex = _HitTest( pt.x, pt.y, rtItem );
	BOOL bSelect = FALSE;
	if( nIndex != -1 )
	{
		if( m_pListParent )
		{
			bSelect = m_pListParent->ItemHitTest( nIndex, pt.x - rtItem.left , pt.y - rtItem.top, uMsg );
		}

		SetFocus();
	}

	if( bSelect && m_uItemSelect != nIndex )
	{
		m_uItemSelect = nIndex;
		Invalidate();
	}

	return TRUE;
}

void CListViewMgr::DoPaintGnd( CDCHandle dc )
{
	if( !m_pListParent )
		return;

	CRect rtClient;
	GetClientRect(&rtClient);
	CMemoryDC dcMem( dc.m_hDC, rtClient );
	int nTop = 0;

	ATLASSERT( m_pListParent );

	CPoint ptOffset;
	GetScrollOffset( ptOffset );

	CDCHandle dcItem( dcMem );

	dcItem.FillSolidRect( rtClient, RGB(0xFF, 0xFF, 0xFF) );
	CRect rtItem(rtClient);
	rtItem.top -= ptOffset.y;
	rtItem.bottom -= ptOffset.y;
	UINT uHeight = 0;
	for( UINT i = 0; i < m_uItemCount; i++ )
	{
		uHeight = m_pListParent->MeasureItem( i );
		rtItem.bottom = rtItem.top + uHeight;
		
		if( rtItem.bottom <= rtClient.top )
		{
			rtItem.top = rtItem.bottom;
			continue;
		}

		if( rtItem.top >= rtClient.bottom )
		{
			rtItem.top = rtItem.bottom;
			break;
		}
		
		m_pListParent->DrawItem( dcItem, i, rtItem, m_uItemSelect == i );
		rtItem.top = rtItem.bottom;
	}
}
