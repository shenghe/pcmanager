//////////////////////////////////////////////////////////////////////////
//   File Name: kuiwndpanel.h
// Description: CKuiPanel Definition, Virtual Container of KuiWindows
//     Creator: Zhang Xiaoxuan
//     Version: 2009.04.28 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "kuiwndcmnctrl.h"
#include "kuiwndrichtxt.h"

class CKuiPanel : public CKuiContainerWnd
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiPanel, "div")

public:
    CKuiPanel()
        : m_bOnlyDrawChild(FALSE)
    {
    }

protected:

    CAtlList<CKuiWindow *> m_lstWndChild;
    BOOL m_bOnlyDrawChild;

protected:

    CKuiWindow* _CreateKuiWindowByName(LPCSTR lpszName);

    // Vert-Align Process
    // Called after this line composed
    void _ProcessLineVAlign(POSITION posLineStart, POSITION posLineEnd, LONG lLineTop, LONG lLineBottom)
    {
        CRect rcChild;

        if (m_uVAlign == VAlign_Top)
            return;

        POSITION posLine = posLineStart;
        int nNewTop = 0;

        while (posLine != posLineEnd)
        {
            CKuiWindow *pKuiWndLine = m_lstWndChild.GetNext(posLine);

            pKuiWndLine->GetRect(rcChild);

            if (m_uVAlign == VAlign_Middle)
                nNewTop = lLineTop + (lLineBottom - lLineTop - rcChild.Height()) / 2;
            else if(m_uVAlign == VAlign_Bottom)
                nNewTop = lLineBottom - rcChild.Height();

            rcChild.MoveToY(nNewTop);
            pKuiWndLine->Move(rcChild);
        }
    }

    // Composing .....
    void _ComposingPanel(LPWINDOWPOS lpWndPos)
    {
        POSITION pos = m_lstWndChild.GetHeadPosition(), posPrev = NULL, posThisLineStart = NULL;
        WINDOWPOS WndPos;
        CRect rcChild;

        LONG lNextLineTop = lpWndPos->y + m_style.m_nMarginY + m_style.m_nLineSpacing;
        LONG lMaxRight = m_rcWindow.right;
        LONG lMaxBottom = m_rcWindow.top;
        BOOL bFirstInBlock = TRUE, bNewLine = FALSE;

        memcpy(&WndPos, lpWndPos, sizeof(WINDOWPOS));

        WndPos.x += m_style.m_nMarginX;
        WndPos.y += m_style.m_nMarginY;
        WndPos.cx = ((GetPositionType() & SizeX_Specify) ? m_lSpecifyWidth : WndPos.cx) - m_style.m_nMarginX * 2;
        WndPos.cy -= m_style.m_nMarginY * 2;

        while (pos != NULL)
        {
            posPrev = pos;

            CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);

            if (!pKuiWndChild->IsVisible())
                continue;

            if (Position_Float & pKuiWndChild->GetPositionType())
                continue;

            if (bFirstInBlock)
                posThisLineStart = posPrev;

            // Return
            if (pKuiWndChild->IsClass(CKuiReturn::GetClassName()))
            {
                _ProcessLineVAlign(posThisLineStart, posPrev, WndPos.y, lNextLineTop);
                posThisLineStart = posPrev;

                WndPos.x = lpWndPos->x + m_style.m_nMarginX;
                WndPos.y = lNextLineTop + m_style.m_nLineSpacing;
                WndPos.cx = ((GetPositionType() & SizeX_Specify) ? m_lSpecifyWidth : lpWndPos->cx) - m_style.m_nMarginX * 2;
                WndPos.cy = lpWndPos->y + lpWndPos->cy - WndPos.y - m_style.m_nMarginY;

                bFirstInBlock = FALSE;

                continue;
            }

            if (WndPos.cx == 0)
            {
                bNewLine = TRUE;
            }
            else
            {
                pKuiWndChild->KuiSendMessage(WM_WINDOWPOSCHANGED, NULL, (LPARAM)&WndPos);
                pKuiWndChild->GetRect(rcChild);

                bNewLine = FALSE;

                // Is this line full?
                if ((rcChild.right > WndPos.x + WndPos.cx || rcChild.right <= rcChild.left) && !bFirstInBlock)
                {
                    bNewLine = TRUE;
                    _ProcessLineVAlign(posThisLineStart, posPrev, WndPos.y, lNextLineTop);
                    posThisLineStart = posPrev;
                }
            }

            bFirstInBlock = FALSE;

            if (bNewLine)
            {
                // Reposition this control to next line
                WndPos.x = lpWndPos->x + m_style.m_nMarginX;
                WndPos.y = lNextLineTop + m_style.m_nLineSpacing;
                WndPos.cx = ((GetPositionType() & SizeX_Specify) ? m_lSpecifyWidth : lpWndPos->cx) - m_style.m_nMarginX * 2;
                WndPos.cy = lpWndPos->y + lpWndPos->cy - WndPos.y - m_style.m_nMarginY;

                pKuiWndChild->KuiSendMessage(WM_WINDOWPOSCHANGED, NULL, (LPARAM)&WndPos);
                pKuiWndChild->GetRect(rcChild);
            }

            // Next control position
            WndPos.x += rcChild.Width() + m_style.m_nSpacing;
            WndPos.cx -= rcChild.Width() + m_style.m_nSpacing;

            if (lNextLineTop < rcChild.bottom)
            {
                lNextLineTop = rcChild.bottom;
            }

            if (lMaxRight < rcChild.right + m_style.m_nMarginX)
            {
                lMaxRight = rcChild.right + m_style.m_nMarginX;
            }
        }

        // Process last line
        _ProcessLineVAlign(posThisLineStart, NULL, WndPos.y, lNextLineTop);

        switch (GetPositionType() & SizeX_Mask)
        {
        case SizeX_FitParent:
            m_rcWindow.right = lpWndPos->x + lpWndPos->cx;
            break;
        case SizeX_FitContent:
            m_rcWindow.right = lMaxRight;
            break;
        case SizeX_Specify:
            m_rcWindow.right = lpWndPos->x + m_lSpecifyWidth;
            break;
        }

        switch (GetPositionType() & SizeY_Mask)
        {
        case SizeY_FitParent:
            m_rcWindow.bottom = lpWndPos->y + lpWndPos->cy;
            break;
        case SizeY_FitContent:
            m_rcWindow.bottom = lNextLineTop + m_style.m_nMarginY;
            break;
        case SizeY_Specify:
            m_rcWindow.bottom = lpWndPos->y + m_lSpecifyHeight;
            break;
        }

        // Process float children here
        pos = m_lstWndChild.GetHeadPosition();

        while (pos != NULL)
        {
            posPrev = pos;

            CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);

            if (!pKuiWndChild->IsVisible())
                continue;

            if (0 == (Position_Float & pKuiWndChild->GetPositionType()))
                continue;

            memcpy(&WndPos, lpWndPos, sizeof(WINDOWPOS));

            WndPos.x += m_style.m_nMarginX;
            WndPos.y += m_style.m_nMarginY;
            WndPos.cx = ((GetPositionType() & SizeX_Specify) ? m_lSpecifyWidth : WndPos.cx) - m_style.m_nMarginX * 2;
            WndPos.cy -= m_style.m_nMarginY * 2;

            pKuiWndChild->KuiSendMessage(WM_WINDOWPOSCHANGED, NULL, (LPARAM)&WndPos);
            pKuiWndChild->GetRect(rcChild);

            if (Align_Right & pKuiWndChild->GetAlign())
            {
                WndPos.x = WndPos.x + WndPos.cx - rcChild.Width();
            }
            else if (Align_Center & pKuiWndChild->GetAlign())
            {
                WndPos.x = WndPos.x + (WndPos.cx - rcChild.Width()) / 2;
            }

            if (VAlign_Bottom & pKuiWndChild->GetAlign())
            {
                WndPos.y = WndPos.y + WndPos.cy - rcChild.Height();
            }
            else if (VAlign_Middle & pKuiWndChild->GetAlign())
            {
                WndPos.y = WndPos.y + (WndPos.cy - rcChild.Height()) / 2;
            }

            WndPos.cx = rcChild.Width();
            WndPos.cy = rcChild.Height();

            pKuiWndChild->KuiSendMessage(WM_WINDOWPOSCHANGED, NULL, (LPARAM)&WndPos);

        }
    }

public:

    BOOL LoadChilds(TiXmlElement* pTiXmlChildElem)
    {
        KuiSendMessage(WM_DESTROY);

        BOOL bVisible = IsVisible(TRUE);

        for (TiXmlElement* pXmlChild = pTiXmlChildElem; NULL != pXmlChild; pXmlChild = pXmlChild->NextSiblingElement())
        {
            CKuiWindow *pNewChildWindow = _CreateKuiWindowByName(pXmlChild->Value());
            if (!pNewChildWindow)
                continue;

//             int nID = 0;
//             pXmlChild->Attribute("id", &nID);

            pNewChildWindow->SetParent(m_hKuiWnd);
            pNewChildWindow->SetContainer(m_hWndContainer);
            pNewChildWindow->Load(pXmlChild);

//             if (pNewChildWindow->IsVisible())
//                 pNewChildWindow->KuiSendMessage(WM_SHOWWINDOW, bVisible);

            m_lstWndChild.AddTail(pNewChildWindow);
        }

        return TRUE;
    }

    // Create children
    virtual BOOL Load(TiXmlElement* pTiXmlElem)
    {
        if (!CKuiWindow::Load(pTiXmlElem))
            return FALSE;

        return LoadChilds(pTiXmlElem->FirstChildElement());
    }

    // Hittest children
    virtual HKUIWND KuiGetHWNDFromPoint(POINT ptHitTest, BOOL bOnlyText)
    {
        if (m_rcWindow.PtInRect(ptHitTest))
        {
            if (m_bOnlyDrawChild)
                return m_hKuiWnd;

            HKUIWND hKuiWndChild = NULL;

            POSITION pos = m_lstWndChild.GetTailPosition();

            while (pos != NULL)
            {
                CKuiWindow *pKuiWndChild = m_lstWndChild.GetPrev(pos);
                if (!pKuiWndChild->IsVisible() || pKuiWndChild->IsTransparent())
                    continue;

                if (Position_Float == (pKuiWndChild->GetPositionType() & Position_Float))
                {
                    hKuiWndChild = pKuiWndChild->KuiGetHWNDFromPoint(ptHitTest, bOnlyText);

                    if (hKuiWndChild)
                        break;
                }
            }

            if (hKuiWndChild)
                return hKuiWndChild;

            pos = m_lstWndChild.GetTailPosition();

            while (pos != NULL)
            {
                CKuiWindow *pKuiWndChild = m_lstWndChild.GetPrev(pos);
                if (!pKuiWndChild->IsVisible() || pKuiWndChild->IsTransparent())
                    continue;

                if (0 == (pKuiWndChild->GetPositionType() & Position_Float))
                {
                    hKuiWndChild = pKuiWndChild->KuiGetHWNDFromPoint(ptHitTest, bOnlyText);

                    if (hKuiWndChild)
                        break;
                }
            }

            if (hKuiWndChild)
                return hKuiWndChild;
            else
                return m_hKuiWnd;
        }
        else
            return NULL;
    }

    void SetContainer(HWND hWndContainer)
    {
        __super::SetContainer(hWndContainer);

        POSITION pos = m_lstWndChild.GetHeadPosition();

        while (pos != NULL)
        {
            CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);

            if (pKuiWndChild)
            {
                pKuiWndChild->SetContainer(hWndContainer);
            }
        }
    }

    CKuiWindow* FindChildByCmdID(UINT uCmdID)
    {
        CKuiWindow *pChildFind = NULL;

        POSITION pos = m_lstWndChild.GetHeadPosition();

        while (pos != NULL)
        {
            CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);

            if (pKuiWndChild->GetCmdID() == uCmdID)
                return pKuiWndChild;
        }

        pos = m_lstWndChild.GetHeadPosition();

        while (pos != NULL)
        {
            CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);

            if (pKuiWndChild->IsContainer())
            {
                pChildFind = ((CKuiContainerWnd *)pKuiWndChild)->FindChildByCmdID(uCmdID);
                if (pChildFind)
                    return pChildFind;
            }
        }

        return NULL;
    }

    virtual BOOL RedrawRegion(CDCHandle& dc, CRgn& rgn)
    {
        if (__super::RedrawRegion(dc, rgn))
        {
            POSITION pos = m_lstWndChild.GetHeadPosition();
            KuiDCPaint KuiDC;

            BeforePaint(dc, KuiDC);

            while (pos != NULL)
            {
                CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);

                if (pKuiWndChild && pKuiWndChild->IsVisible())
                {
                    pKuiWndChild->RedrawRegion(dc, rgn);
                }
            }

            AfterPaint(dc, KuiDC);

            return TRUE;
        }

        return FALSE;
    }

    void ShowAllRealWindowChilds(BOOL bShow);

    void OnWindowPosChanged(LPWINDOWPOS lpWndPos)
    {
        // Calc default panel size
        CKuiWindow::OnWindowPosChanged(lpWndPos);

        _ComposingPanel(lpWndPos);

    }

    int OnCreate(LPCREATESTRUCT /*lpCreateStruct*/)
    {
        POSITION pos = m_lstWndChild.GetHeadPosition();

        while (pos != NULL)
        {
            CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);

            pKuiWndChild->OnCreate(NULL);
        }

        return TRUE;
    }

    void OnDestroy()
    {
        POSITION pos = m_lstWndChild.GetHeadPosition();

        while (pos != NULL)
        {
            CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);

            pKuiWndChild->KuiSendMessage(WM_DESTROY);

            delete pKuiWndChild;
        }

        m_lstWndChild.RemoveAll();
    }

    BOOL OnEraseKuignd(CDCHandle dc)
    {
//         CKuiWindow::OnEraseKuignd(dc);

        POSITION pos = m_lstWndChild.GetHeadPosition();

        BOOL bDisabled = IsDisabled(), bIsChildDisabled = FALSE;

        while (pos != NULL)
        {
            CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);
            if (!pKuiWndChild->IsVisible())
                continue;

            if (bDisabled)
            {
                bIsChildDisabled = pKuiWndChild->IsDisabled();

                if (!bIsChildDisabled)
                    pKuiWndChild->ModifyState(KuiWndState_Disable, 0);
            }

            pKuiWndChild->KuiSendMessage(WM_ERASEBKGND, (WPARAM)(HDC)dc);

            if (bDisabled && !bIsChildDisabled)
            {
                pKuiWndChild->ModifyState(0, KuiWndState_Disable);
            }
        }

        return TRUE;
    }

    void OnPaint(CDCHandle dc)
    {
        return;

        HFONT hftOld = NULL, hftDraw = NULL;
        COLORREF crOld = CLR_INVALID, crOldBg = CLR_INVALID, crDraw = CLR_INVALID;
        CRect rcText = m_rcWindow;
        int nOldKuiMode = OPAQUE;

        if (!m_style.m_strSkinName.IsEmpty())
            nOldKuiMode = dc.SetBkMode(TRANSPARENT);
        if (CLR_INVALID != m_style.m_crBg)
            crOldBg = dc.SetBkColor(m_style.m_crBg);

        if (m_style.m_ftText)
            hftDraw = m_style.m_ftText;

        if (m_style.m_crText != CLR_INVALID)
            crDraw = m_style.m_crText;

        if (KuiWndState_Hover == (GetState() & KuiWndState_Hover))
        {
            if (m_style.m_ftHover)
                hftDraw = m_style.m_ftHover;

            if (m_style.m_crHoverText != CLR_INVALID)
                crDraw = m_style.m_crHoverText;
        }

        if (hftDraw)
            hftOld = dc.SelectFont(hftDraw);

        if (crDraw != CLR_INVALID)
            crOld = dc.SetTextColor(crDraw);
//         int nOldKuiMode = OPAQUE;
//         COLORREF crOldBg = CLR_INVALID;
//         COLORREF crOldText = CLR_INVALID;
//
//         if (!m_style.m_strSkinName.IsEmpty())
//             nOldKuiMode = dc.SetBkMode(TRANSPARENT);
//         if (CLR_INVALID != m_style.m_crBg)
//             crOldBg = dc.SetBkColor(m_style.m_crBg);
//         if (CLR_INVALID != m_style.m_crText)
//             crOldText = dc.SetTextColor(m_style.m_crText);

        POSITION pos = m_lstWndChild.GetHeadPosition();

        BOOL bDisabled = IsDisabled(), bIsChildDisabled = FALSE;

        while (pos != NULL)
        {
            CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);

            if (!pKuiWndChild->IsVisible())
                continue;

            if (bDisabled)
            {
                bIsChildDisabled = pKuiWndChild->IsDisabled();

                if (!bIsChildDisabled)
                    pKuiWndChild->ModifyState(KuiWndState_Disable, 0);
            }

            pKuiWndChild->KuiSendMessage(WM_PAINT, (WPARAM)(HDC)dc);

            if (bDisabled && !bIsChildDisabled)
            {
                pKuiWndChild->ModifyState(0, KuiWndState_Disable);
            }
        }

        if (hftDraw)
            dc.SelectFont(hftOld);

        if (crDraw != CLR_INVALID)
            dc.SetTextColor(crOld);

        if (!m_style.m_strSkinName.IsEmpty())
            dc.SetBkMode(nOldKuiMode);
        if (CLR_INVALID != m_style.m_crBg)
            dc.SetBkColor(crOldBg);
    }

    void OnMouseMove(UINT nFlags, CPoint point)
    {
        POSITION pos = m_lstWndChild.GetHeadPosition();
        CRect rcChild;

        while (pos != NULL)
        {
            CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);
            if (!pKuiWndChild->IsVisible())
                continue;

            pKuiWndChild->GetRect(rcChild);

            if (rcChild.PtInRect(point))
            {
                pKuiWndChild->KuiSendMessage(WM_MOUSEMOVE, (WPARAM)nFlags, (LPARAM)MAKELONG(point.x, point.y));
            }
        }
    }

    void OnShowWindow(BOOL bShow, UINT nStatus)
    {
        __super::OnShowWindow(bShow, nStatus);

        ShowAllRealWindowChilds(bShow);
    }

protected:

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_INT_ATTRIBUTE("onlydrawchild", m_bOnlyDrawChild, FALSE)
    KUIWIN_DECLARE_ATTRIBUTES_END()

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_PAINT(OnPaint)
//         MSG_WM_ERASEBKGND(OnEraseKuignd)
        MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_SHOWWINDOW(OnShowWindow)
    KUIWIN_END_MSG_MAP()
};

#include "kuiwnddlg.h"
#include "kuiwndtabctrl.h"

inline CKuiWindow* CKuiPanel::_CreateKuiWindowByName(LPCSTR lpszName)
{
    CKuiWindow *pNewWindow = NULL;

//     pNewWindow = CKuiPanel::CheckAndNew(lpszName);
//     if (pNewWindow)
//         return pNewWindow;

    pNewWindow = CKuiDialog::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

    pNewWindow = CKuiStatic::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

    pNewWindow = CKuiButton::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

//     pNewWindow = CKuiSpacing::CheckAndNew(lpszName);
//     if (pNewWindow)
//         return pNewWindow;

    pNewWindow = CKuiImageWnd::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

//     pNewWindow = CKuiReturn::CheckAndNew(lpszName);
//     if (pNewWindow)
//         return pNewWindow;

    pNewWindow = CKuiProgress::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

    pNewWindow = CKuiImageBtnWnd::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

    pNewWindow = CKuiRealWnd::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

    pNewWindow = CKuiLine::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

    pNewWindow = CKuiCheckBox::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

    pNewWindow = CKuiIconWnd::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

    pNewWindow = CKuiTabCtrl::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

    pNewWindow = CKuiRadioBox::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

    pNewWindow = CKuiRichText::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

    pNewWindow = CKuiLink::CheckAndNew(lpszName);
    if (pNewWindow)
        return pNewWindow;

    return NULL;
}

inline void CKuiPanel::ShowAllRealWindowChilds(BOOL bShow)
{
    POSITION pos = m_lstWndChild.GetHeadPosition();

    BOOL bVisible = IsVisible(TRUE);

    while (pos != NULL)
    {
        CKuiWindow *pKuiWndChild = m_lstWndChild.GetNext(pos);

        BOOL bChildVisible = bVisible && pKuiWndChild->IsVisible();

//         if (!pKuiWndChild->IsVisible())
//             continue;

        if (pKuiWndChild->IsClass(CKuiRealWnd::GetClassName()))
        {
            ((CKuiRealWnd *)pKuiWndChild)->ShowRealWindow(bShow && bChildVisible);
        }
        else if (pKuiWndChild->IsClass(CKuiPanel::GetClassName()) || pKuiWndChild->IsClass(CKuiDialog::GetClassName()))
        {
            ((CKuiPanel *)pKuiWndChild)->ShowAllRealWindowChilds(bShow && bChildVisible);
        }
        else if (pKuiWndChild->IsClass(CKuiTabCtrl::GetClassName()))
        {
            ((CKuiTabCtrl *)pKuiWndChild)->ShowAllRealWindowChilds(bShow && bChildVisible);
        }
    }
}
