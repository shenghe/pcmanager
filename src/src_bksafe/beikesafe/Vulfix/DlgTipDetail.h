//////////////////////////////////////////////////////////////////////////
//
// Create Date   : 2011-03-22
// Create Author : luciffer.j (zhangzexin@kingsoft.com)
// Description   : View the vulnerability details.
//////////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////////

#include "BeikeVulfixUtils.h"

//////////////////////////////////////////////////////////////////////////

#define DEF_VUL_TIP_DETAIL_WIDTH  406
#define DEF_VUL_TIP_DETAIL_HEIGHT 180
//////////////////////////////////////////////////////////////////////////

class CDlgTipDetail
    : public CWindowImpl<CDlgTipDetail>
{

public:
    CDlgTipDetail(void);
    virtual~CDlgTipDetail(void);

    BOOL SubclassWindow(HWND hWnd)
    {
        BOOL bRet = CWindowImpl<CDlgTipDetail>::SubclassWindow(hWnd);

        return bRet;
    }
public:
    void Initilize(HWND hNotifyHwnd, T_VulListItemData* pItemData, CPoint popPoint);

protected:
    LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/);
    void OnPaint(CDCHandle dc);
    LRESULT OnEraseBkgnd(CDCHandle dc);
    void OnMouseMove(UINT nFlags, CPoint point);
    BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message);
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnKillFocus(HWND hWnd);

public:

    BEGIN_MSG_MAP_EX(CDlgTipDetail)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_SETCURSOR(OnSetCursor)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_KILLFOCUS(OnKillFocus)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

protected:
    CRect m_rcCloseBtn;
    CRect m_rcQuestion;
    CRect m_rcIgnoreVul;
    CRect m_rcOfficial;
    CRect m_rcDetail;
    CRect m_rcClient;
    CRect m_rcHead;

    T_VulListItemData* m_pItemData;
    HWND m_hNotifyHwnd;

    int m_nCloseState;

protected:
    BOOL _PtInLink(const CPoint pt);
    int _CountHeight();

    inline void _DrawGradualColorRect(
        HDC dc,
        const RECT& drawRC, 
        COLORREF clrFirst = RGB(255, 255, 255),
        COLORREF clrEnd = RGB(0, 0, 0),
        BOOL fVertical = TRUE);

    void _DrawLine(CDC& memDC, CPoint ptStart, CPoint ptEnd,
        COLORREF color, DWORD dwPenTyple = PS_SOLID);

    void _DrawText(CDC& memDC, CRect& rcItem, 
        LPCTSTR pszText , COLORREF colorText , HFONT hFont);
    
};
