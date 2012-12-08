#pragma once

//////////////////////////////////////////////////////////////////////////
// Rich Text Control
//
class CKuiRichText : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiRichText, "richtext")

public:
    CKuiRichText()
        : m_XmlElemChilds("")
        , m_dwFont(0)
        , m_bLinkLoaded(FALSE)
        , m_posHover(NULL)
        , m_posPushDown(NULL)
    {

    }

    virtual ~CKuiRichText()
    {
    }

    virtual BOOL Load(TiXmlElement* pTiXmlElem)
    {
        if (!CKuiWindow::Load(pTiXmlElem))
            return FALSE;


        return LoadChilds(pTiXmlElem);
    }

    BOOL LoadChilds(TiXmlElement* pTiXmlElem)
    {
        m_lstLink.RemoveAll();
        m_bLinkLoaded = FALSE;
        m_posHover = NULL;

        m_XmlElemChilds = *pTiXmlElem;

        return TRUE;
    }

    virtual void SetCursor()
    {
        ::SetCursor(::LoadCursor(NULL, (0 != m_posHover) ? IDC_HAND : IDC_ARROW));
    }

protected:

    void OnWindowPosChanged(LPWINDOWPOS lpWndPos)
    {
        __super::OnWindowPosChanged(lpWndPos);

        m_lstLink.RemoveAll();
        m_bLinkLoaded = FALSE;
        m_posHover = NULL;
    }

    void OnPaint(CDCHandle dc)
    {
        HFONT hftOld = dc.SelectFont(KuiFontPool::GetFont(LOWORD(m_dwFont)));

        CRect rcDraw = m_rcWindow;
        CRgn rgnDraw;

        rcDraw.DeflateRect(m_style.m_nMarginX, m_style.m_nMarginY);
        rgnDraw.CreateRectRgnIndirect(rcDraw);

        int nDC = dc.SaveDC();
        dc.SelectClipRgn(rgnDraw);

        POINT pt = rcDraw.TopLeft();

        _DrawElement(dc, &m_XmlElemChilds, LOWORD(m_dwFont), pt);

        _DrawLinks(dc);

        dc.RestoreDC(nDC);

        dc.SelectFont(hftOld);

        m_bLinkLoaded = TRUE;
    }

    void OnMouseMove(UINT nFlags, CPoint point)
    {
        POSITION posHover = NULL;
        POSITION pos = m_lstLink.GetHeadPosition();

        while (pos)
        {
            posHover = pos;
            const _LinkInfo& link = m_lstLink.GetNext(pos);

            if (link.rcLink.PtInRect(point))
            {
                if (m_posHover != posHover)
                {
                    if (m_posHover)
                    {
                        _LinkInfo& linkOldHover = m_lstLink.GetAt(m_posHover);
                        NotifyInvalidateRect(linkOldHover.rcLink);
                    }

                    m_posHover = posHover;
                    NotifyInvalidateRect(link.rcLink);
                }

                return;
            }
        }

        OnMouseLeave();
    }

    void OnMouseLeave()
    {
        if (m_posHover)
        {
            const _LinkInfo& linkOldHover = m_lstLink.GetAt(m_posHover);
            NotifyInvalidateRect(linkOldHover.rcLink);
        }

        m_posHover = NULL;
    }

    void OnLButtonDown(UINT nFlags, CPoint point)
    {
        if (m_posHover)
        {
            m_posPushDown = m_posHover;
        }
    }

    void OnLButtonUp(UINT nFlags, CPoint point)
    {
        if (m_posPushDown && m_posHover == m_posPushDown)
        {
            const _LinkInfo& link = m_lstLink.GetAt(m_posPushDown);

            KUINMCOMMAND nms;
            nms.hdr.code = KUINM_COMMAND;
            nms.hdr.hwndFrom = GetContainer();
            nms.hdr.idFrom = ::GetDlgCtrlID(nms.hdr.hwndFrom);
            nms.uItemID = link.uCmdID;
            nms.szItemClass = GetObjectClass();

            LRESULT lRet = ::SendMessage(::GetParent(nms.hdr.hwndFrom), WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);
        }

        m_posPushDown = NULL;
    }

    void _AddLink(CDCHandle &dc, TiXmlElement *pElemChild, POINT &pt)
    {
        _LinkInfo newLink;

        HFONT hftOld = NULL;

        if (NULL != m_styleLink.m_ftText)
            hftOld = dc.SelectFont(m_styleLink.m_ftText);

        newLink.strText = CA2T(pElemChild->GetText(), CP_UTF8);

        SIZE sz = {0, 0};

        dc.GetTextExtent(newLink.strText, newLink.strText.GetLength(), &sz);

        if (NULL != m_styleLink.m_ftText)
            dc.SelectFont(hftOld);

        if (pt.x + sz.cx > m_rcWindow.right - m_style.m_nMarginX)
        {
            pt.x = m_rcWindow.left + m_style.m_nMarginX;
            pt.y += m_style.m_nLineSpacing;
        }

        newLink.rcLink.SetRect(pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy);

        pt.x = newLink.rcLink.right + m_style.m_nSpacing;

        pElemChild->Attribute("id", (int *)&newLink.uCmdID);

        m_lstLink.AddTail(newLink);
    }

    void _DrawElement(CDCHandle &dc, TiXmlElement *pElemChild, WORD wFont, POINT &pt)
    {
        TiXmlNode *pNodeChild = NULL;

        WORD wNewFont = wFont;
        COLORREF crTextOld = CLR_INVALID;

        BOOL bOnlyCalcPoint = FALSE;

        if (0 == stricmp(pElemChild->Value(), "sp"))
        {
            _DrawSpace(dc, pt);
            return;
        }
        else if (0 == stricmp(pElemChild->Value(), "br"))
        {
            pt.x = m_rcWindow.left + m_style.m_nMarginX;
            pt.y += m_style.m_nLineSpacing;
        }
        else if (0 == stricmp(pElemChild->Value(), "b"))
        {
            wNewFont = wFont | KUIF_BOLD;
        }
        else if (0 == stricmp(pElemChild->Value(), "i"))
        {
            wNewFont = wFont | KUIF_ITALIC;
        }
        else if (0 == stricmp(pElemChild->Value(), "u"))
        {
            wNewFont = wFont | KUIF_UNDERLINE;
        }
        else if (0 == stricmp(pElemChild->Value(), "c"))
        {
            crTextOld = dc.SetTextColor(CKuiObject::HexStringToColor(pElemChild->Attribute("color")));
        }
        else if (0 == stricmp(pElemChild->Value(), "a"))
        {
            if (!m_bLinkLoaded)
            {
                _AddLink(dc, pElemChild, pt);

                return;
            }

            bOnlyCalcPoint = TRUE;
        }

        dc.SelectFont(KuiFontPool::GetFont(wNewFont));

        while (pNodeChild = pElemChild->IterateChildren(pNodeChild))
        {
            _DrawNode(dc, pNodeChild, wNewFont, pt, bOnlyCalcPoint);
        }

        dc.SelectFont(KuiFontPool::GetFont(wFont));

        if (CLR_INVALID != crTextOld)
            dc.SetTextColor(crTextOld);
    }

    void _DrawSpace(CDCHandle &dc, POINT &pt)
    {
        CSize size;
        dc.GetTextExtent(_T(" "), 1, &size);
        dc.TextOut(pt.x, pt.y, _T(" "));
        pt.x += size.cx;
    }

    void _DrawLinks(CDCHandle &dc)
    {
        BOOL bDisabled = IsDisabled(TRUE);
        BOOL bHover = FALSE;
        POSITION pos = m_lstLink.GetHeadPosition();

        if (NULL != m_styleLink.m_ftText)
            dc.SelectFont(m_styleLink.m_ftText);

        if (bDisabled)
        {
            if (CLR_INVALID != m_styleLink.m_crDisabledText)
                dc.SetTextColor(m_styleLink.m_crDisabledText);
        }
        else
        {
            if (CLR_INVALID != m_styleLink.m_crText)
                dc.SetTextColor(m_styleLink.m_crText);
        }

        while (pos)
        {
            if (pos == m_posHover)
            {
                if (NULL != m_styleLink.m_ftText)
                    dc.SelectFont(m_styleLink.m_ftHover);

                if (CLR_INVALID != m_styleLink.m_crText)
                    dc.SetTextColor(m_styleLink.m_crHoverText);

                bHover = TRUE;
            }

            const _LinkInfo& link = m_lstLink.GetNext(pos);

            dc.TextOut(link.rcLink.left, link.rcLink.top, link.strText);

            if (bHover)
            {
                if (NULL != m_styleLink.m_ftText)
                    dc.SelectFont(m_styleLink.m_ftText);

                if (CLR_INVALID != m_styleLink.m_crText)
                    dc.SetTextColor(m_styleLink.m_crText);

                bHover = FALSE;
            }
        }
    }

    void _DrawNode(CDCHandle &dc, TiXmlNode *pNodeChild, WORD wFont, POINT &pt, BOOL bOnlyCalcPoint = FALSE)
    {
        switch (pNodeChild->Type())
        {
        case TiXmlNode::ELEMENT:
            {
                TiXmlElement *pElemChild = pNodeChild->ToElement();

                _DrawElement(dc, pElemChild, wFont, pt);
            }
            break;

        case TiXmlNode::TEXT:
            {
                CString strText = CA2T(pNodeChild->Value(), CP_UTF8);
                LPCTSTR lpszDraw = strText;
                int nFullLength = strText.GetLength(), nLength = 0;

                while (TRUE)
                {
                    nLength = nFullLength - nLength;
                    nFullLength = nLength;
                    SIZE sz = {0, 0};

                    while (nLength > 0)
                    {
                        dc.GetTextExtent(lpszDraw, nLength, &sz);
                        if (pt.x + sz.cx <= m_rcWindow.right - m_style.m_nMarginX)
                            break;

                        nLength --;
                    }

                    if (!bOnlyCalcPoint)
                        dc.TextOut(pt.x, pt.y, lpszDraw, nLength);

                    if (nFullLength == nLength)
                    {
                        pt.x += sz.cx;
                        break;
                    }

                    pt.x = m_rcWindow.left + m_style.m_nMarginX;
                    pt.y += m_style.m_nLineSpacing;
                    lpszDraw += nLength;
                }

                pt.x += m_style.m_nSpacing;
            }
            break;
        }
    }

protected:

    TiXmlElement m_XmlElemChilds;
    DWORD m_dwFont;
    KuiStyle m_styleLink;
    BOOL m_bLinkLoaded;
    POSITION m_posHover;
    POSITION m_posPushDown;

    class _LinkInfo
    {
    public:
        CString strText;
        UINT    uCmdID;
        CRect   rcLink;
    };

    CAtlList<_LinkInfo> m_lstLink;

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_HEX_ATTRIBUTE("defaultfont", m_dwFont, FALSE)
        KUIWIN_STYLE_ATTRIBUTE("linkclass", m_styleLink, FALSE)
    KUIWIN_DECLARE_ATTRIBUTES_END()

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_PAINT(OnPaint)
        MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
    KUIWIN_END_MSG_MAP()
};
