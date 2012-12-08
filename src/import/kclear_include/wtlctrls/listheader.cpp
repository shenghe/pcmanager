// ListHeader.cpp : implementation file
//

#include "stdafx.h"
#include "listHeader.h"
#include "kdraw.h"


/////////////////////////////////////////////////////////////////////////////
// CListHeader

CListHeader::CListHeader()
{
    m_clrBkGnd = Default_BKG_RGB;
    m_clrTextColor = Default_TEXT_RGB;
    m_strTitle = L"";
}

CListHeader::~CListHeader()
{
}

LRESULT CListHeader::OnLayOut(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lRes = 0L;
    bHandled = TRUE;
    HD_LAYOUT *playout	=(HD_LAYOUT*)lParam;
    RECT&      rect = *(playout->prc);
    WINDOWPOS& pos	= *(playout->pwpos);

    lRes = DefWindowProc(HDM_LAYOUT, wParam, lParam);


    m_headHight = 0;
    pos.cy   = m_headHight;
    rect.top = m_headHight;

    return lRes;

}


extern void DrawHeaderBg(CDC *pDC, CRect rcRect, LONG lFirstItemSize)
{
    // 	CRect rcTemp = rcRect;
    // 
    // 	KDraw::DrawGradualColorRect(pDC->m_hDC, rcRect, RGB(254,254,254), RGB(243,243,243));
    // 	rcTemp.top = rcRect.bottom - 1;
    // 	rcTemp.bottom = rcRect.bottom;
    // 	KDraw::FillSolidRect(pDC->m_hDC, rcTemp, RGB(130,135,144));
}

void CListHeader::OnPaint(HDC hDC) 
{
    CPaintDC dc(m_hWnd); 
    CRect rcClient;

    GetClientRect(&rcClient);

    CBrush brush;
    brush.CreateSolidBrush(m_clrBkGnd);

    HGDIOBJ pOldFont = SelectObject((HDC)dc,GetFont());
    dc.SetTextColor(m_clrTextColor);
    dc.SetBkMode(TRANSPARENT);
    DrawHeaderBg(&dc, rcClient, -1);
    rcClient.DeflateRect(5, 1, 1, 2);
    dc.DrawText(m_strTitle, m_strTitle.GetLength() ,&rcClient, DT_LEFT | DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    SelectObject((HDC)dc,pOldFont);
}



void CListHeader::OnMouseMove(UINT nFlags, CPoint point) 
{

}

int CListHeader::OnMouseActivate(HWND pDesktopWnd, UINT nHitTest, UINT message) 
{
    return false;
}
