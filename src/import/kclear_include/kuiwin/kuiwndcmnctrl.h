//////////////////////////////////////////////////////////////////////////
//   File Name: kuiwndcmnctrl.h
// Description: KuiWindow Definition
//     Creator: Zhang Xiaoxuan
//     Version: 2009.04.28 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "kuiwnd.h"
#include "kuiimage.h"
#include "kuitheme.h"
#include "kuiwndnotify.h"

//////////////////////////////////////////////////////////////////////////
// Spacing Control
// Only leave a space, faster than "div" (... a little)
//
// Usage: <spacing width=xx />
//
class CKuiSpacing : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiSpacing, "spacing")

public:

    // Do nothing
    void OnPaint(CDCHandle dc)
    {
    }

protected:
    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_PAINT(OnPaint)
    KUIWIN_END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////
// New line
// Panel control process return thing, so define a blank class
//
// Usage: <br />
//
class CKuiReturn : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiReturn, "br")
};

//////////////////////////////////////////////////////////////////////////
// Static Control
//
// Usage: <text>inner text example</text>
//
class CKuiStatic : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiStatic, "text")
};

//////////////////////////////////////////////////////////////////////////
// Link Control
// Only For Header Drag Test
// Usage: <link>inner text example</link>
//
class CKuiLink : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiLink, "link")
};

//////////////////////////////////////////////////////////////////////////
// Button Control
// Use id attribute to process click event
//
// Usage: <button id=xx>inner text example</button>
//
class CKuiButton : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiButton, "button")
public:

    virtual BOOL NeedRedrawWhenStateChange()
    {
        return TRUE;
    }
};

//////////////////////////////////////////////////////////////////////////
// Image Control
// Use src attribute specify a resource id
//
// Usage: <img src=xx />
//
class CKuiImageWnd : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiImageWnd, "img")
public:
    CKuiImageWnd()
        : m_nSubImageID(-1)
        , m_pSkin(NULL)
    {

    }

    virtual BOOL Load(TiXmlElement* pTiXmlElem)
    {
        if (!CKuiWindow::Load(pTiXmlElem))
            return FALSE;
//
//         m_pSkin = KuiSkin::GetSkin(m_strSkin);
//         if (m_pSkin && m_pSkin->IsClass(CKuiImageSkin::GetClassName()))
//             return TRUE;
//
// //         if (m_imgSrc.M_HOBJECT)
// //             return TRUE;

        return TRUE;
    }

    void OnPaint(CDCHandle dc)
    {
        if (m_pSkin)
            m_pSkin->Draw(dc, m_rcWindow, m_nSubImageID);
    }

    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
    {
        __super::OnNcCalcSize(bCalcValidRects, lParam);

        LPSIZE pSize = (LPSIZE)lParam;

        if (m_pSkin)
        {
            SIZE sizeImage;
            sizeImage = m_pSkin->GetSkinSize();

            if (sizeImage.cx)
                pSize->cx = sizeImage.cx;
            if (sizeImage.cy)
                pSize->cy = sizeImage.cy;

            return TRUE;
        }

        return 0;
    }

protected:
//     CKuiImageSkin m_imgSrc;
//     int m_nSubImageWidth;
    int m_nSubImageID;
    CKuiSkinBase *m_pSkin;

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_SKIN_ATTRIBUTE("skin", m_pSkin, TRUE)
//         KUIWIN_CHAIN_ATTRIBUTE(m_imgSrc, TRUE)
//         KUIWIN_UINT_ATTRIBUTE("src", m_imgSrc, TRUE)
//         KUIWIN_INT_ATTRIBUTE("subwidth", m_nSubImageWidth, TRUE)
        KUIWIN_INT_ATTRIBUTE("sub", m_nSubImageID, FALSE)
    KUIWIN_DECLARE_ATTRIBUTES_END()

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_PAINT(OnPaint)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
    KUIWIN_END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////
// Progress Control
// Use id attribute to process click event
//
// Usage: <button id=xx>inner text example</button>
//
class CKuiProgress : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiProgress, "progress")
public:
    CKuiProgress()
        : m_dwMinValue(0)
        , m_dwMaxValue(0)
        , m_dwValue(0)
        , m_bShowPercent(FALSE)
        , m_pSkinBg(NULL)
        , m_pSkinPos(NULL)
    {

    }

    virtual BOOL Load(TiXmlElement* pTiXmlElem)
    {
        if (!CKuiWindow::Load(pTiXmlElem))
            return FALSE;

//         CKuiSkinBase *pSkin = NULL;
//         pSkin = KuiSkin::GetSkin(m_strSkinBg);
//
//         if (pSkin)
//         {
//             if (pSkin->IsClass(CKuiSkinImgHorzExtend::GetClassName()))
//                 m_pSkinBg = (CKuiSkinImgHorzExtend *)pSkin;
//             else
//                 m_pSkinBg = NULL;
//         }
//
//         pSkin = KuiSkin::GetSkin(m_strSkinPos);
//
//         if (pSkin)
//         {
//             if (pSkin->IsClass(CKuiSkinImgHorzExtend::GetClassName()))
//                 m_pSkinPos = (CKuiSkinImgHorzExtend *)pSkin;
//             else
//                 m_pSkinPos = NULL;
//         }
//
        return TRUE;
    }

    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
    {
        __super::OnNcCalcSize(bCalcValidRects, lParam);

        LPSIZE pSize = (LPSIZE)lParam;

        if (m_pSkinBg)
        {
            SIZE sizeBg = m_pSkinBg->GetSkinSize();

            pSize->cy = sizeBg.cy;

            return TRUE;
        }

        return TRUE;
    }

    void OnPaint(CDCHandle dc)
    {
        KuiDCPaint kuidc;
        CPen penFrame;
        CString strPercent;
        double dPos = 0;

        BeforePaint(dc, kuidc);

        penFrame.CreatePen(
            PS_SOLID,
            1,
            RGB(0x70, 0x70, 0x70)
            );

        HPEN hpenOld = dc.SelectPen(penFrame);

        if (m_pSkinBg)
        {
            m_pSkinBg->Draw(dc, m_rcWindow, KuiWndState_Normal);
        }
        else
        {
            dc.RoundRect(m_rcWindow, CPoint(2, 2));
        }

        if (m_dwMaxValue == m_dwMinValue)
        {
            dPos = 0;
        }
        else
        {
            if (m_dwMaxValue < m_dwMinValue)
            {
                DWORD dwTemp = m_dwMaxValue;
                m_dwMaxValue = m_dwMinValue;
                m_dwMinValue = dwTemp;
            }

            if (m_dwValue < m_dwMinValue)
            {
                m_dwValue = m_dwMinValue;
            }
            else if (m_dwValue > m_dwMaxValue)
            {
                m_dwValue = m_dwMaxValue;
            }

            dPos = (double)(m_dwValue - m_dwMinValue) / (double)(m_dwMaxValue - m_dwMinValue);
        }

        if (m_bShowPercent)
        {
            strPercent.Format(_T("%d%%"), (int)(dPos * 100));
            dc.DrawText(strPercent, strPercent.GetLength(), m_rcWindow, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }

        CRect rcPosBar = m_rcWindow;

        if (m_pSkinPos)
        {
            SIZE sizePosBar = m_pSkinPos->GetSkinSize();

            rcPosBar.DeflateRect(2, (m_rcWindow.Height() - sizePosBar.cy) / 2);
            rcPosBar.right = rcPosBar.left + (LONG)(rcPosBar.Width() * dPos);

            m_pSkinPos->Draw(dc, rcPosBar, KuiWndState_Normal);
        }
        else
        {
            rcPosBar.DeflateRect(2, 2);
            rcPosBar.right = rcPosBar.left + (LONG)(rcPosBar.Width() * dPos);

            dc.RoundRect(rcPosBar, CPoint(2, 2));

            dc.FillSolidRect(rcPosBar, RGB(66, 97, 144));
        }

        dc.SelectPen(hpenOld);

        if (m_bShowPercent)
        {
            CRgn rgnClipOld, rgnClip;
            rgnClip.CreateRectRgnIndirect(rcPosBar);
            dc.GetClipRgn(rgnClipOld);
            dc.SelectClipRgn(rgnClip);

            if (CLR_INVALID == m_style.m_crBg)
                dc.SetTextColor(RGB(0xFF, 0xFF, 0xFF));
            else
                dc.SetTextColor(m_style.m_crBg);

            dc.DrawText(strPercent, strPercent.GetLength(), m_rcWindow, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

            dc.SetTextColor(m_style.m_crText);
            dc.SelectClipRgn(rgnClipOld);
        }

        AfterPaint(dc, kuidc);
    }

protected:
    DWORD m_dwMinValue;
    DWORD m_dwMaxValue;
    DWORD m_dwValue;
    BOOL m_bShowPercent;
//     CStringA m_strSkinBg;
//     CStringA m_strSkinPos;
    CKuiSkinBase *m_pSkinBg;
    CKuiSkinBase *m_pSkinPos;

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_PAINT(OnPaint)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
    KUIWIN_END_MSG_MAP()

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_SKIN_ATTRIBUTE("bgskin", m_pSkinBg, TRUE)
        KUIWIN_SKIN_ATTRIBUTE("posskin", m_pSkinPos, TRUE)
//         KUIWIN_STRING_ATTRIBUTE("bgskin", m_strSkinBg, TRUE)
//         KUIWIN_STRING_ATTRIBUTE("posskin", m_strSkinPos, TRUE)
        KUIWIN_DWORD_ATTRIBUTE("min", m_dwMinValue, FALSE)
        KUIWIN_DWORD_ATTRIBUTE("max", m_dwMaxValue, FALSE)
        KUIWIN_DWORD_ATTRIBUTE("value", m_dwValue, FALSE)
        KUIWIN_UINT_ATTRIBUTE("showpercent", m_bShowPercent, FALSE)
    KUIWIN_DECLARE_ATTRIBUTES_END()
};

//////////////////////////////////////////////////////////////////////////
// Image Button Control
//
// Usage: <imgbtn src=xx />
//
class CKuiImageBtnWnd : public CKuiImageWnd
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiImageBtnWnd, "imgbtn")
public:
    CKuiImageBtnWnd()
    {

    }

    virtual BOOL NeedRedrawWhenStateChange()
    {
        return TRUE;
    }

//     virtual BOOL Load(TiXmlElement* pTiXmlElem)
//     {
//         if (!CKuiWindow::Load(pTiXmlElem))
//             return FALSE;
//
//         if (m_imgSrc.M_HOBJECT)
//             return TRUE;
//
//         return FALSE;
//     }

    void OnPaint(CDCHandle dc)
    {
        //DWORD dwState = GetState();

        if (m_pSkin)
        {
            m_pSkin->Draw(
                dc, m_rcWindow,
                IIF_STATE4(GetState(), 0, 1, 2, 3)
                );
        }
    }

//     LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
//     {
//         LPSIZE pSize = (LPSIZE)lParam;
//
//         if (m_imgSrc.M_HOBJECT)
//         {
//             SIZE sizeImage;
//
//             m_imgSrc.GetImageSize(sizeImage);
//
//             pSize->cx = (m_imgSrc.GetSubImageWidth() != 0) ? m_imgSrc.GetSubImageWidth() : sizeImage.cx;
//             pSize->cy = sizeImage.cy;
//
//             return TRUE;
//         }
//
//         return FALSE;
//     }

protected:

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_PAINT(OnPaint)
//         MSG_WM_NCCALCSIZE(OnNcCalcSize)
    KUIWIN_END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////
// Real Window Control
// Binding a real window
//
// Usage: <realwnd ctrlid=xx />
//
class CKuiRealWnd : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiRealWnd, "realwnd")
public:
    CKuiRealWnd()
        : m_uRealWinDlgResID(0)
    {

    }

    virtual BOOL NeedRedrawWhenStateChange()
    {
        return FALSE;
    }

    void ShowRealWindow(BOOL bShow)
    {
        ::ShowWindow(_GetRealWindow(), bShow ? SW_SHOW : SW_HIDE);
    }

    void OnWindowPosChanged(LPWINDOWPOS lpWndPos)
    {
        CRect rcOldWnd = m_rcWindow;

        __super::OnWindowPosChanged(lpWndPos);

        if (rcOldWnd != m_rcWindow)
        {
            HWND hWndReal = _GetRealWindow();
            ::SetWindowPos(hWndReal, HWND_TOP, m_rcWindow.left, m_rcWindow.top, m_rcWindow.Width(), m_rcWindow.Height(), SWP_NOREDRAW);

            KUINMREALWNDRESIZED nms;
            nms.hdr.code = KUINM_REALWND_RESIZED;
            nms.hdr.hwndFrom = m_hWndContainer;
            nms.hdr.idFrom = ::GetDlgCtrlID(m_hWndContainer);
            nms.uItemID = GetCmdID();
            nms.rcWnd = m_rcWindow;

            LRESULT lRet = ::SendMessage(m_hWndContainer, WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);

            if (::IsWindowVisible(hWndReal))
            {
                CRect rcUpdate = m_rcWindow;
                rcUpdate.MoveToXY(0, 0);
                //::RedrawWindow(_GetRealWindow(), rcUpdate, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
//                 ::RedrawWindow(hWndReal, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ERASENOW);
                ::InvalidateRect(hWndReal, NULL, TRUE);
                ::SetWindowPos(hWndReal, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_FRAMECHANGED);
            }
        }
    }

    void OnShowWindow(BOOL bShow, UINT nStatus)
    {
        __super::OnShowWindow(bShow, nStatus);

        if (bShow)
        {
            // 如果父窗口或者父窗口的父窗口隐藏（主要是由于Tab切换），则不显示真实窗口
            CKuiWindow* pWnd = this;

            while (TRUE)
            {
                pWnd = KuiWnds::GetWindow(pWnd->GetParent());

                if (NULL == pWnd)
                    break;

                if (!pWnd->IsVisible())
                    return;
            }
        }

        ShowRealWindow(bShow);
    }

    void OnDestroy()
    {
        ::ShowWindow(_GetRealWindow(), SW_HIDE);
    }

    // Do nothing
    void OnPaint(CDCHandle dc)
    {
    }

protected:

    HWND _GetRealWindow()
    {
        return ::GetDlgItem(m_hWndContainer, m_uRealWinDlgResID);
    }

    UINT m_uRealWinDlgResID;

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_UINT_ATTRIBUTE("ctrlid", m_uRealWinDlgResID, FALSE)
    KUIWIN_DECLARE_ATTRIBUTES_END()

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_PAINT(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
        MSG_WM_SHOWWINDOW(OnShowWindow)
    KUIWIN_END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////
// Line Control
// Simple HTML "HR" tag
//
// Usage: <hr style=solid size=1 crbg=.../>
//
class CKuiLine : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiLine, "hr")

public:

    CKuiLine()
        : m_nPenStyle(PS_SOLID)
        , m_nLineSize(1)
    {
    }

    // Do nothing
    void OnPaint(CDCHandle dc)
    {
        CPen pen;

        pen.CreatePen(m_nPenStyle, m_nLineSize, m_style.m_crBg);

        HPEN hpenOld = dc.SelectPen(pen);

        dc.MoveTo(m_rcWindow.left, m_rcWindow.top/* + m_rcWindow.Height() / 2*/);
        dc.LineTo(m_rcWindow.right, m_rcWindow.bottom/*top + m_rcWindow.Height() / 2*/);

        dc.SelectPen(hpenOld);
    }

protected:
    int m_nPenStyle;
    int m_nLineSize;

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_INT_ATTRIBUTE("size", m_nLineSize, FALSE)
        KUIWIN_ENUM_ATTRIBUTE("style", int, FALSE)
            KUIWIN_ENUM_VALUE("solid", PS_SOLID)             // default
            KUIWIN_ENUM_VALUE("dash", PS_DASH)               /* -------  */
            KUIWIN_ENUM_VALUE("dot", PS_DOT)                 /* .......  */
            KUIWIN_ENUM_VALUE("dashdot", PS_DASHDOT)         /* _._._._  */
            KUIWIN_ENUM_VALUE("dashdotdot", PS_DASHDOTDOT)   /* _.._.._  */
        KUIWIN_ENUM_END(m_nPenStyle)
    KUIWIN_DECLARE_ATTRIBUTES_END()

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_PAINT(OnPaint)
    KUIWIN_END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////
// Check Box
//
// Usage: <check state=1>This is a check-box</check>
//
class CKuiCheckBox : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiCheckBox, "check")

    enum {
        CheckBoxSize = 13,
        CheckBoxSpacing = 4,
    };

public:

    CKuiCheckBox()
        : m_theme()
    {
    }

    void OnPaint(CDCHandle dc)
    {
        CRect rcCheckBox = m_rcWindow;
        rcCheckBox.right = rcCheckBox.left + CheckBoxSize;

        m_theme.OpenTheme(m_hWndContainer);

        if (m_theme.IsValid())
        {
            m_theme.DrawBackground(dc, _GetDrawState(), rcCheckBox);
        }
        else
        {
            dc.DrawFrameControl(rcCheckBox, DFC_BUTTON, _GetDrawState());
        }

        m_rcWindow.left += CheckBoxSize + CheckBoxSpacing;

        m_style.m_nTextAlign &= ~DT_BOTTOM;
        m_style.m_nTextAlign |= DT_VCENTER;

        __super::OnPaint(dc);

        m_rcWindow.left -= CheckBoxSize + CheckBoxSpacing;
    }

    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
    {
        LPSIZE pSize = (LPSIZE)lParam;

        __super::OnNcCalcSize(bCalcValidRects, lParam);

        pSize->cx += CheckBoxSize + CheckBoxSpacing;
        pSize->cy = max(pSize->cy, CheckBoxSize);

        return FALSE;
    }

protected:

    CKuiCheckBoxTheme m_theme;

    int m_nState;

    UINT _GetDrawState()
    {
        DWORD dwState = GetState();
        UINT uState = m_theme.IsValid() ? 0 : DFCS_BUTTONCHECK | DFCS_TRANSPARENT;

        if (m_theme.IsValid())
        {
            if (dwState & KuiWndState_Check)
            {
                if (dwState & KuiWndState_Disable)
                    uState = CBS_CHECKEDDISABLED;
                else if (dwState & KuiWndState_PushDown)
                    uState = CBS_CHECKEDPRESSED;
                else if (dwState & KuiWndState_Hover)
                    uState = CBS_CHECKEDHOT;
                else
                    uState = CBS_CHECKEDNORMAL;
            }
            else
            {
                if (dwState & KuiWndState_Disable)
                    uState = CBS_UNCHECKEDDISABLED;
                else if (dwState & KuiWndState_PushDown)
                    uState = CBS_UNCHECKEDPRESSED;
                else if (dwState & KuiWndState_Hover)
                    uState = CBS_UNCHECKEDHOT;
                else
                    uState = CBS_UNCHECKEDNORMAL;
            }
        }
        else
        {
            uState = DFCS_BUTTONCHECK | DFCS_TRANSPARENT;
            if (dwState & KuiWndState_Disable)
                uState |= DFCS_INACTIVE;
            if (dwState & KuiWndState_Hover)
                uState |= DFCS_HOT;
            if (dwState & KuiWndState_PushDown)
                uState |= DFCS_PUSHED;
            if (dwState & KuiWndState_Check)
                uState |= DFCS_CHECKED;
        }

        return uState;
    }

    virtual BOOL NeedRedrawWhenStateChange()
    {
        return TRUE;
    }

    void OnLButtonUp(UINT nFlags, CPoint point)
    {
        if (IsChecked())
            ModifyState(0, KuiWndState_Check);
        else
            ModifyState(KuiWndState_Check, 0);
    }

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_UINT_ATTRIBUTE("check", m_nState, FALSE)
    KUIWIN_DECLARE_ATTRIBUTES_END()

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_PAINT(OnPaint)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
        MSG_WM_LBUTTONUP(OnLButtonUp)
    KUIWIN_END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////
// Icon Control
// Use src attribute specify a resource id
//
// Usage: <icon src=xx />
//
class CKuiIconWnd : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiIconWnd, "icon")
public:
    CKuiIconWnd() : m_nSize(16), m_bOemIcon(FALSE)
    {
    }

    virtual BOOL Load(TiXmlElement* pTiXmlElem)
    {
        if (!CKuiWindow::Load(pTiXmlElem))
            return FALSE;

        _ReloadIcon();

        return TRUE;
    }

    void OnPaint(CDCHandle dc)
    {
        if (m_strResIDCurrent != m_strResID)
            _ReloadIcon();

        m_theIcon.DrawIconEx(dc, m_rcWindow.TopLeft(), m_rcWindow.Size());
    }

    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
    {
        LPSIZE pSize = (LPSIZE)lParam;

        pSize->cx = m_nSize;
        pSize->cy = m_nSize;

        return TRUE;
    }

    void AttachIcon(HICON hIcon)
    {
        m_theIcon = hIcon;
    }

protected:

    void _ReloadIcon()
    {
		KAppRes& appRes = KAppRes::Instance();
		m_theIcon = appRes.GetIcon(m_strResID);
        m_strResIDCurrent = m_strResID;
    }

    CIconHandle m_theIcon;
	std::string m_strResID;
    std::string m_strResIDCurrent;
    int m_nSize;
	BOOL m_bOemIcon;

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_STRING_ATTRIBUTE("src", m_strResID, FALSE)
        KUIWIN_UINT_ATTRIBUTE("oem", m_bOemIcon, FALSE)
        KUIWIN_INT_ATTRIBUTE("size", m_nSize, FALSE)
    KUIWIN_DECLARE_ATTRIBUTES_END()

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_PAINT(OnPaint)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
    KUIWIN_END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////
// Radio Box
//
// Usage: <radio state=1>This is a check-box</radio>
//
class CKuiRadioBox : public CKuiWindow
{
    KUIOBJ_DECLARE_CLASS_NAME(CKuiRadioBox, "radio")

    enum {
        RadioBoxSize = 12,
        RadioBoxSpacing = 2,
    };

public:

    CKuiRadioBox()
        : m_theme()
    {
    }

    ~CKuiRadioBox()
    {
        KuiWnds::UnregisterRadioGroup(this, m_strGroup);
    }

    virtual BOOL Load(TiXmlElement* pTiXmlElem)
    {
        if (!CKuiWindow::Load(pTiXmlElem))
            return FALSE;

        return KuiWnds::RegisterRadioGroup(this, m_strGroup);;
    }

    void OnPaint(CDCHandle dc)
    {
        CRect rcCheckBox(0, 0, RadioBoxSize, RadioBoxSize);

        rcCheckBox.MoveToXY(m_rcWindow.left, m_rcWindow.top + (m_rcWindow.Height() - RadioBoxSize) / 2);

        m_theme.OpenTheme(m_hWndContainer);

        if (m_theme.IsValid())
        {
            m_theme.DrawBackground(dc, _GetDrawState(), rcCheckBox);
        }
        else
        {
            dc.DrawFrameControl(rcCheckBox, DFC_BUTTON, _GetDrawState());
        }

        m_rcWindow.left += RadioBoxSize + RadioBoxSpacing;

        m_style.m_nTextAlign &= ~DT_BOTTOM;
        m_style.m_nTextAlign |= DT_VCENTER | DT_SINGLELINE;

        __super::OnPaint(dc);

        m_rcWindow.left -= RadioBoxSize + RadioBoxSpacing;
    }

    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
    {
        LPSIZE pSize = (LPSIZE)lParam;

        __super::OnNcCalcSize(bCalcValidRects, lParam);

        pSize->cx += RadioBoxSize + RadioBoxSpacing;
        pSize->cy = max(pSize->cy, RadioBoxSize);

        return FALSE;
    }

protected:

    CKuiRadioBoxTheme m_theme;

    int m_nState;
    CStringA m_strGroup;

    UINT _GetDrawState()
    {
        DWORD dwState = GetState();
        UINT uState = m_theme.IsValid() ? 0 : DFCS_BUTTONRADIO | DFCS_TRANSPARENT;

        if (m_theme.IsValid())
        {
            if (dwState & KuiWndState_Check)
            {
                if (dwState & KuiWndState_Disable)
                    uState = RBS_CHECKEDDISABLED;
                else if (dwState & KuiWndState_PushDown)
                    uState = RBS_CHECKEDPRESSED;
                else if (dwState & KuiWndState_Hover)
                    uState = RBS_CHECKEDHOT;
                else
                    uState = RBS_CHECKEDNORMAL;
            }
            else
            {
                if (dwState & KuiWndState_Disable)
                    uState = RBS_UNCHECKEDDISABLED;
                else if (dwState & KuiWndState_PushDown)
                    uState = RBS_UNCHECKEDPRESSED;
                else if (dwState & KuiWndState_Hover)
                    uState = RBS_UNCHECKEDHOT;
                else
                    uState = RBS_UNCHECKEDNORMAL;
            }
        }
        else
        {
            uState = DFCS_BUTTONRADIO | DFCS_TRANSPARENT;
            if (dwState & KuiWndState_Disable)
                uState |= DFCS_INACTIVE;
            if (dwState & KuiWndState_Hover)
                uState |= DFCS_HOT;
            if (dwState & KuiWndState_PushDown)
                uState |= DFCS_PUSHED;
            if (dwState & KuiWndState_Check)
                uState |= DFCS_CHECKED;
        }

        return uState;
    }

    virtual BOOL NeedRedrawWhenStateChange()
    {
        return TRUE;
    }

    void OnLButtonUp(UINT nFlags, CPoint point)
    {
        KuiWnds::SelectRadio(this, m_strGroup, NULL);
        //ModifyState(KuiWndState_Check, 0);
    }

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_UINT_ATTRIBUTE("check", m_nState, FALSE)
        KUIWIN_STRING_ATTRIBUTE("group", m_strGroup, FALSE)
    KUIWIN_DECLARE_ATTRIBUTES_END()

    KUIWIN_BEGIN_MSG_MAP()
        MSG_WM_PAINT(OnPaint)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
        MSG_WM_LBUTTONUP(OnLButtonUp)
    KUIWIN_END_MSG_MAP()
};
