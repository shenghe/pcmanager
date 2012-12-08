// ListHeader.cpp : implementation file
//

#include "stdafx.h"
#include "listHeader.h"
#include "kdraw.h"
#include "trashpublicfunc.h"

/////////////////////////////////////////////////////////////////////////////
// CListHeader

CListHeader::CListHeader()
{
    m_clrBkGnd = Default_BKG_RGB;
    m_clrTextColor = Default_TEXT_RGB;
    
}

CListHeader::~CListHeader()
{
}

BEGIN_MESSAGE_MAP(CListHeader, CHeaderCtrl)
	//{{AFX_MSG_MAP(CListHeader)
    ON_MESSAGE(HDM_LAYOUT, OnLayOut)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListHeader message handlers

LRESULT CListHeader::OnLayOut(WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = 0L;
    
    HD_LAYOUT *playout	=(HD_LAYOUT*)lParam;
    RECT&      rect = *(playout->prc);
    WINDOWPOS& pos	= *(playout->pwpos);
    
    lRes = DefWindowProc(HDM_LAYOUT, wParam, lParam);

    SYSTEM_VERSION     m_eSysVer;
    KAEGetSystemVersion(&m_eSysVer);
    CString StrSuffix;
    if(m_eSysVer == enumSystem_Win_7)
        m_headHight = 0;
    else m_headHight = 0;
    pos.cy   = m_headHight;
    rect.top = m_headHight;
    
    return lRes;

}


extern void DrawHeaderBg(CDC *pDC, CRect rcRect, LONG lFirstItemSize)
{
	CRect rcTemp = rcRect;

	KDraw::DrawGradualColorRect(pDC->m_hDC, rcRect, RGB(254,254,254), RGB(243,243,243));
	rcTemp.top = rcRect.bottom - 1;
	rcTemp.bottom = rcRect.bottom;
	KDraw::FillSolidRect(pDC->m_hDC, rcTemp, RGB(130,135,144));
}

void CListHeader::OnPaint() 
{
	CPaintDC dc(this); 
    CRect rcClient;

    GetClientRect(&rcClient);

    CBrush brush;
    brush.CreateSolidBrush(m_clrBkGnd);

    int nItems = GetItemCount();
    ASSERT(nItems >= 0);

    CFont* pOldFont = dc.SelectObject(GetFont());
    dc.SetTextColor(m_clrTextColor);
    dc.SetBkMode(TRANSPARENT);
    DrawHeaderBg(&dc, rcClient, -1);
    rcClient.DeflateRect(5, 1, 1, 2);
    dc.DrawText(m_strTitle, &rcClient, DT_LEFT | DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    dc.SelectObject(pOldFont);
}

void CListHeader::PreSubclassWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
    m_strTitle.LoadString(IDS_CHOICE_DIR);
    EnableWindow(FALSE);
	CHeaderCtrl::PreSubclassWindow();
}

void CListHeader::OnMouseMove(UINT nFlags, CPoint point) 
{
	
}

int CListHeader::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message) 
{
	return false;
}
