//////////////////////////////////////////////////////////////////////////
//   File Name: kuiwndtabctrl.h
// Description: Tab Control
//     Creator: Zhang Xiaoxuan
//     Version: 2009.6.8 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "kuiwndnotify.h"

class CKuiTab : public CKuiDialog
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiTab, "tab")

public:

    LPCTSTR GetTitle()
    {
        return m_strTitle;
    }

    void SetTitle(LPCTSTR lpszTitle)
    {
        m_strTitle = lpszTitle;
    }

protected:

    CString m_strTitle;

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_TSTRING_ATTRIBUTE("title", m_strTitle, FALSE)
    KUIWIN_DECLARE_ATTRIBUTES_END()
};

class CKuiTabCtrl : public CKuiContainerWnd
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiTabCtrl, "tabctrl")

protected:
    CAtlList<CKuiTab *> m_lstPages;
    int m_nHoverTabItem;
    int m_nCurrentPage;
    int m_nTabSpacing;
    int m_nTabPos;
    int m_nFramePos;
    int m_nTabWidth;
    int m_nTabHeight;
    CKuiSkinBase *m_pSkinTab;
    CKuiSkinBase *m_pSkinFrame;
    CKuiSkinBase *m_pSkinIcon;
    CKuiSkinBase *m_pSkinSplitter;
    CRect m_rcClient;
    POINT m_ptIcon;
    POINT m_ptText;
    int m_nTabAlign;

    enum {
        AlignTop,
        AlignLeft,
    };

public:

    CKuiTabCtrl()
        : m_nCurrentPage(0)
        , m_pSkinTab(NULL)
        , m_pSkinFrame(NULL)
        , m_pSkinIcon(NULL)
        , m_pSkinSplitter(NULL)
        , m_nTabSpacing(0)
        , m_nTabPos(0)
        , m_nFramePos(0)
        , m_nTabWidth(0)
        , m_nTabHeight(0)
        , m_nHoverTabItem(-1)
        , m_nTabAlign(AlignTop)
    {
        m_ptIcon.x = m_ptIcon.y = 0;
        m_ptText.x = m_ptText.y = 0;
    }

    int GetCurSel()
    {
        return m_nCurrentPage;
    }

	BOOL DeleteTabItemFormList(int nItem)
	{
		if (nItem > (int)m_lstPages.GetCount() - 1)
			return FALSE;

		m_lstPages.RemoveAt(m_lstPages.FindIndex(nItem));
		return TRUE;
	}

    BOOL SetCurSel(int nIndex)
    {
        int nOldPage = m_nCurrentPage;

        KUINMTABSELCHANGE nms;
        nms.hdr.code = KUINM_TAB_SELCHANGE;
        nms.hdr.hwndFrom = m_hWndContainer;
        nms.hdr.idFrom = ::GetDlgCtrlID(m_hWndContainer);
        nms.uTabID = GetCmdID();
        nms.uTabItemIDNew = nIndex;
        nms.uTabItemIDOld = nOldPage;
        nms.bCancel = FALSE;

        LRESULT lRet = ::SendMessage(m_hWndContainer, WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);

        if (nms.bCancel)
            return FALSE;

        if (m_nCurrentPage != nIndex)
        {
            if (nIndex < 0 || nIndex >= (int)m_lstPages.GetCount())
                return FALSE;

            CRect rcItem;

            GetTabItemRect(m_nCurrentPage, rcItem);
            NotifyInvalidateRect(rcItem);
            GetTabItemRect(nIndex, rcItem);
            NotifyInvalidateRect(rcItem);

            GetTab(m_nCurrentPage)->KuiSendMessage(WM_SHOWWINDOW, (WPARAM)FALSE);

            m_nCurrentPage = nIndex;

            GetTab(nIndex)->KuiSendMessage(WM_SHOWWINDOW, (WPARAM)TRUE);

            WINDOWPOS WndPos = {0};

            WndPos.x = m_rcClient.left;
            WndPos.y = m_rcClient.top;
            WndPos.cx = m_rcClient.Width();
            WndPos.cy = m_rcClient.Height();
            GetTab(nIndex)->KuiSendMessage(WM_WINDOWPOSCHANGED, NULL, (LPARAM)&WndPos);

            NotifyInvalidateRect(m_rcClient);
        }

        return TRUE;
    }

    BOOL SetTabTitle(int nIndex, LPCTSTR lpszTitle)
    {
        CRect rcItem;

        CKuiTab* pTab = GetTab(nIndex);
        if (pTab)
        {
            pTab->SetTitle(lpszTitle);
            GetTabItemRect(nIndex, rcItem);
            NotifyInvalidateRect(rcItem);

            return TRUE;
        }

        return FALSE;
    }

    BOOL LoadChilds(TiXmlElement* pTiXmlChildElem)
    {
        BOOL bFirstPage = TRUE;

        KuiSendMessage(WM_DESTROY);

        for (TiXmlElement* pXmlChild = pTiXmlChildElem; NULL != pXmlChild; pXmlChild = pXmlChild->NextSiblingElement())
        {
            CKuiTab *pNewChildWindow = CKuiTab::CheckAndNew(pXmlChild->Value());
            if (!pNewChildWindow)
                continue;

            pNewChildWindow->SetParent(m_hKuiWnd);
            pNewChildWindow->SetContainer(m_hWndContainer);
            pNewChildWindow->Load(pXmlChild);
            pNewChildWindow->SetAttribute("pos", "0,0,-0,-0", TRUE);

            pNewChildWindow->KuiSendMessage(WM_SHOWWINDOW, bFirstPage);
            bFirstPage = FALSE;

            m_lstPages.AddTail(pNewChildWindow);
        }

        return TRUE;
    }

    void SetContainer(HWND hWndContainer)
    {
        __super::SetContainer(hWndContainer);

        POSITION pos = m_lstPages.GetHeadPosition();

        while (pos != NULL)
        {
            CKuiTab *pKuiWndChild = m_lstPages.GetNext(pos);

            if (pKuiWndChild)
            {
                pKuiWndChild->SetContainer(hWndContainer);
            }
        }
    }

    BOOL GetTabItemRect(int nIndex, CRect &rcItem)
    {
        if (nIndex < 0 || nIndex >= (int)m_lstPages.GetCount())
            return FALSE;

        SIZE size = {0, 0};

        if (m_pSkinTab)
            size = m_pSkinTab->GetSkinSize();

        if (0 != m_nTabHeight)
            size.cy = m_nTabHeight;
        if (0 != m_nTabWidth)
            size.cx = m_nTabWidth;

        rcItem.SetRect(m_rcWindow.left, m_rcWindow.top, m_rcWindow.left + size.cx, m_rcWindow.top + size.cy);

        switch (m_nTabAlign)
        {
        case AlignTop:
            rcItem.OffsetRect(m_nTabPos + (m_nTabWidth + m_nTabSpacing) * nIndex, 0);
            break;
        case AlignLeft:
            rcItem.OffsetRect(0, m_nTabPos + (m_nTabHeight + m_nTabSpacing) * nIndex);
            break;
        }

        return TRUE;
    }

    CKuiTab* GetTab(int nIndex)
    {
        if (nIndex < 0 || nIndex >= (int)m_lstPages.GetCount())
            return NULL;

        return m_lstPages.GetAt(m_lstPages.FindIndex(nIndex));
    }

    // Create children
    virtual BOOL Load(TiXmlElement* pTiXmlElem)
    {
        if (!CKuiWindow::Load(pTiXmlElem))
            return FALSE;

        BOOL bRet = LoadChilds(pTiXmlElem->FirstChildElement());
        if (!bRet)
            return FALSE;

//         CKuiSkinBase *pSkin = NULL;
//         pSkin = KuiSkin::GetSkin(m_strSkinTab);
//
//         if (pSkin)
//         {
//             if (pSkin->IsClass(CKuiSkinImgHorzExtend::GetClassName()))
//                 m_pSkinTab = (CKuiSkinImgHorzExtend *)pSkin;
//             else
//                 m_pSkinTab = NULL;
//         }
//
//         pSkin = KuiSkin::GetSkin(m_strSkinFrame);
//
//         if (pSkin)
//         {
//             if (pSkin->IsClass(CKuiSkinImgFrame::GetClassName()))
//                 m_pSkinFrame = (CKuiSkinImgFrame *)pSkin;
//             else
//                 m_pSkinFrame = NULL;
//         }

        return TRUE;
    }

    // Hittest children
    virtual HKUIWND KuiGetHWNDFromPoint(POINT ptHitTest, BOOL bOnlyText)
    {
        if (m_rcWindow.PtInRect(ptHitTest))
        {
            if (m_rcClient.PtInRect(ptHitTest))
                return GetTab(m_nCurrentPage)->KuiGetHWNDFromPoint(ptHitTest, bOnlyText);
            else
                return GetKuiHWnd();
        }
        else
            return NULL;
    }

    CKuiWindow* FindChildByCmdID(UINT uCmdID)
    {
        CKuiWindow *pChildFind = NULL;
        POSITION pos = m_lstPages.GetHeadPosition();

        while (pos != NULL)
        {
            CKuiTab *pKuiWndChild = m_lstPages.GetNext(pos);

            if (pKuiWndChild)
            {
                if (uCmdID == pKuiWndChild->GetCmdID())
                    return pKuiWndChild;

                pChildFind = pKuiWndChild->FindChildByCmdID(uCmdID);
                if (pChildFind)
                    return pChildFind;
            }
        }

        return NULL;
    }

    void ShowAllRealWindowChilds(BOOL bShow)
    {
        int nPageCount = (int)m_lstPages.GetCount();

        for (int i = 0; i < nPageCount; i ++)
        {
            GetTab(i)->ShowAllRealWindowChilds((i == m_nCurrentPage) ? bShow : FALSE);
        }
    }

    virtual BOOL RedrawRegion(CDCHandle& dc, CRgn& rgn)
    {
        if (__super::RedrawRegion(dc, rgn))
        {
            CKuiTab *pCurPage = GetTab(m_nCurrentPage);
            if (pCurPage)
                pCurPage->RedrawRegion(dc, rgn);

            return TRUE;
        }

        return FALSE;
    }

    void OnDestroy()
    {
        POSITION pos = m_lstPages.GetHeadPosition();

        while (pos != NULL)
        {
            CKuiTab *pKuiWndChild = m_lstPages.GetNext(pos);

            pKuiWndChild->KuiSendMessage(WM_DESTROY);

            delete pKuiWndChild;
        }

        m_lstPages.RemoveAll();
    }

    void OnPaint(CDCHandle dc)
    {
        int nPageCount = (int)m_lstPages.GetCount();
        CRect rcItem;
        SIZE sizeTab = {0, 0}, sizeIcon = {0, 0}, sizeSplitter = {0, 0};
        CString strTabText;

        rcItem = m_rcWindow;

        if (m_pSkinTab)
            sizeTab = m_pSkinTab->GetSkinSize();

        if (m_pSkinIcon)
            sizeIcon = m_pSkinIcon->GetSkinSize();

        if (m_pSkinSplitter)
            sizeSplitter = m_pSkinSplitter->GetSkinSize();

        if (0 != m_nTabHeight)
            sizeTab.cy = m_nTabHeight;

        if (0 != m_nTabWidth)
            sizeTab.cx = m_nTabWidth;

        switch (m_nTabAlign)
        {
        case AlignTop:
            rcItem.top += sizeTab.cy + m_nFramePos;
            break;
        case AlignLeft:
            rcItem.left += sizeTab.cx + m_nFramePos;
            break;
        }

        if (m_pSkinFrame)
            m_pSkinFrame->Draw(dc, rcItem, KuiWndState_Normal);

        dc.SetBkMode(TRANSPARENT);

        if (CLR_INVALID != m_style.m_crText)
            dc.SetTextColor(m_style.m_crText);

        HFONT hFontOld = NULL;

        if (NULL != m_style.m_ftText)
            hFontOld = dc.SelectFont(m_style.m_ftText);

        for (int i = 0; i < nPageCount; i ++)
        {
            GetTabItemRect(i, rcItem);

            if (m_pSkinSplitter && i != nPageCount - 1)
            {
                CRect rcDraw;

                switch (m_nTabAlign)
                {
                case AlignTop:
                    rcDraw.SetRect(rcItem.right, rcItem.top, rcItem.right + m_nTabSpacing, rcItem.bottom);
                    rcDraw.NormalizeRect();
                    rcDraw.DeflateRect((rcDraw.Width() - sizeSplitter.cx) / 2, 0);
                    break;
                case AlignLeft:
                    rcDraw.SetRect(rcItem.left, rcItem.bottom, rcItem.right, rcItem.bottom + m_nTabSpacing);
                    rcDraw.NormalizeRect();
                    rcDraw.DeflateRect(0, (rcDraw.Height() - sizeSplitter.cy) / 2);
                    break;
                }

                m_pSkinSplitter->Draw(dc, rcDraw, -1);
            }

            if (i == m_nCurrentPage)
                continue;

            if (m_pSkinTab)
                m_pSkinTab->Draw(dc, rcItem, (i != m_nHoverTabItem) ? KuiWndState_Normal : KuiWndState_Hover);

            if (m_pSkinIcon)
            {
                CRect rcDraw = rcItem;
                rcDraw.OffsetRect(m_ptIcon);
                m_pSkinIcon->Draw(dc, rcDraw, i);
            }

            rcItem.OffsetRect(m_ptText);

            strTabText = GetTab(i)->GetTitle();
            dc.DrawText(strTabText, strTabText.GetLength(), rcItem, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_TABSTOP);
        }

        if (NULL != m_style.m_ftHover)
            hFontOld = (NULL == hFontOld) ? dc.SelectFont(m_style.m_ftHover) : NULL;

        if (CLR_INVALID != m_style.m_crHoverText)
            dc.SetTextColor(m_style.m_crHoverText);

        for (int i = 0; i < nPageCount; i ++)
        {
            if (i != m_nCurrentPage)
                continue;

            GetTabItemRect(i, rcItem);

            if (m_pSkinTab)
                m_pSkinTab->Draw(dc, rcItem, KuiWndState_PushDown);

            if (m_pSkinIcon)
            {
                CRect rcDraw = rcItem;
                rcDraw.OffsetRect(m_ptIcon);
                m_pSkinIcon->Draw(dc, rcDraw, i);
            }

            rcItem.OffsetRect(m_ptText);

            strTabText = GetTab(i)->GetTitle();
            dc.DrawText(strTabText, strTabText.GetLength(), rcItem, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_TABSTOP);

            break;
        }

        if (NULL != hFontOld)
            dc.SelectFont(hFontOld);
    }

    void OnWindowPosChanged(LPWINDOWPOS lpWndPos)
    {
        CKuiWindow::OnWindowPosChanged(lpWndPos);

        SIZE sizeTab = {0, 0};

        if (m_pSkinTab)
            sizeTab = m_pSkinTab->GetSkinSize();

        if (0 != m_nTabHeight)
            sizeTab.cy = m_nTabHeight;
        if (0 != m_nTabWidth)
            sizeTab.cx = m_nTabWidth;

        m_rcClient = m_rcWindow;
        switch (m_nTabAlign)
        {
        case AlignTop:
            m_rcClient.top += sizeTab.cy + m_nFramePos;
            break;
        case AlignLeft:
            m_rcClient.left += sizeTab.cx + m_nFramePos;
            break;
        }

        m_rcClient.DeflateRect(m_style.m_nMarginX, m_style.m_nMarginY);

        for (int i = 0; i < (int)m_lstPages.GetCount(); i ++)
        {
            WINDOWPOS WndPos = *lpWndPos;

            WndPos.x = m_rcClient.left;
            WndPos.y = m_rcClient.top;
            WndPos.cx = m_rcClient.Width();
            WndPos.cy = m_rcClient.Height();

            CKuiTab *pCurPage = GetTab(i);
            if (pCurPage)
                pCurPage->KuiSendMessage(WM_WINDOWPOSCHANGED, NULL, (LPARAM)&WndPos);
        }

    }

    void OnLButtonDown(UINT nFlags, CPoint point)
    {
        CRect rcItem;

        for (int i = 0; i < (int)m_lstPages.GetCount(); i ++)
        {
            if (i == m_nCurrentPage)
                continue;

            GetTabItemRect(i, rcItem);

            if (rcItem.PtInRect(point))
            {
                SetCurSel(i);
                break;
            }
        }
    }

    void OnShowWindow(BOOL bShow, UINT nStatus)
    {
        __super::OnShowWindow(bShow, nStatus);

        GetTab(m_nCurrentPage)->KuiSendMessage(WM_SHOWWINDOW, (WPARAM)bShow);
    }

    void OnMouseMove(UINT nFlags, CPoint point)
    {
        CRect rcItem;
        int nHoverTabItem = -1;

        for (int i = 0; i < (int)m_lstPages.GetCount(); i ++)
        {
            if (i == m_nCurrentPage)
                continue;

            GetTabItemRect(i, rcItem);

            if (rcItem.PtInRect(point))
            {
                NotifyInvalidateRect(rcItem);
                nHoverTabItem = i;
                break;
            }
        }

        if (nHoverTabItem != m_nHoverTabItem)
        {
            if (-1 != m_nHoverTabItem)
            {
                GetTabItemRect(m_nHoverTabItem, rcItem);
                NotifyInvalidateRect(rcItem);
            }

            m_nHoverTabItem = nHoverTabItem;
        }
    }

    void OnMouseLeave()
    {
        if (-1 != m_nHoverTabItem)
        {
            CRect rcItem;
            GetTabItemRect(m_nHoverTabItem, rcItem);
            NotifyInvalidateRect(rcItem);
            m_nHoverTabItem = -1;
        }
    }

    virtual void SetCursor()
    {
        ::SetCursor(::LoadCursor(NULL, (-1 != m_nHoverTabItem) ? IDC_HAND : IDC_ARROW));
    }

protected:

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_PAINT(OnPaint)
        MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_SHOWWINDOW(OnShowWindow)
    KUIWIN_END_MSG_MAP()

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_INT_ATTRIBUTE("cursel", m_nCurrentPage, FALSE)
        KUIWIN_INT_ATTRIBUTE("tabwidth", m_nTabWidth, FALSE)
        KUIWIN_INT_ATTRIBUTE("tabheight", m_nTabHeight, FALSE)
        KUIWIN_INT_ATTRIBUTE("tabpos", m_nTabPos, FALSE)
        KUIWIN_INT_ATTRIBUTE("tabspacing", m_nTabSpacing, FALSE)
        KUIWIN_SKIN_ATTRIBUTE("tabskin", m_pSkinTab, FALSE)
        KUIWIN_SKIN_ATTRIBUTE("frameskin", m_pSkinFrame, FALSE)
        KUIWIN_SKIN_ATTRIBUTE("iconskin", m_pSkinIcon, FALSE)
        KUIWIN_SKIN_ATTRIBUTE("splitterskin", m_pSkinSplitter, FALSE)
        KUIWIN_INT_ATTRIBUTE("framepos", m_nFramePos, FALSE)
        KUIWIN_INT_ATTRIBUTE("icon-x", m_ptIcon.x, FALSE)
        KUIWIN_INT_ATTRIBUTE("icon-y", m_ptIcon.y, FALSE)
        KUIWIN_INT_ATTRIBUTE("text-x", m_ptText.x, FALSE)
        KUIWIN_INT_ATTRIBUTE("text-y", m_ptText.y, FALSE)
        KUIWIN_ENUM_ATTRIBUTE("tabalign", int, TRUE)
            KUIWIN_ENUM_VALUE("top", AlignTop)
            KUIWIN_ENUM_VALUE("left", AlignLeft)
        KUIWIN_ENUM_END(m_nTabAlign)
    KUIWIN_DECLARE_ATTRIBUTES_END()
};
