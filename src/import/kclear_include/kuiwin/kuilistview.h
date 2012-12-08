//////////////////////////////////////////////////////////////////////////
//  Class Name: CKuiDlgView
// Description: Dialog View, Real Container of KuiWindow
//     Creator: ZhangXiaoxuan
//     Version: 2010.5.11 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "kuidlgview.h"

//////////////////////////////////////////////////////////////////////////
// Beike List View Notify

// Get Display Info Notify
#define KUILVM_GET_DISPINFO  1

typedef struct _KUILVMGETDISPINFO
{
    NMHDR       hdr;
    int         nListItemID;
    BOOL        bHover;
    BOOL        bSelect;
} KUILVMGETDISPINFO, *LPKUILVMGETDISPINFO;

// Item Click Notify
#define KUILVM_ITEMCLICK     2

typedef struct _KUILVMITEMCLICK
{
    NMHDR       hdr;
    int         nListItemID;
    UINT        uCmdID;
} KUILVMITEMCLICK, *LPKUILVMITEMCLICK;

//////////////////////////////////////////////////////////////////////////

typedef CWinTraits<WS_CHILD | WS_TABSTOP | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_NOCOLUMNHEADER, 0> CKuiListViewTraits;

template <class T, class TBase = CListViewCtrl, class TWinTraits = CKuiListViewTraits>
class CKuiListViewImpl
    : public ATL::CWindowImpl<T, TBase, TWinTraits>
    , public CKuiViewImpl<T>
    , public CCustomDraw<T>
{
    friend CKuiViewImpl;

protected:

    typedef ATL::CWindowImpl<T, TBase, TWinTraits>  __baseClass;
    typedef CKuiListViewImpl<T, TBase, TWinTraits>   __thisClass;

    CKuiDialog       m_kuiListTemplate;
    CToolTipCtrl    m_wndToolTip;
    BOOL            m_bXmlLoaded;
    CKuiImage        m_imgMem;
    int             m_nItemHeight;

    // Only one hover control
    HKUIWND          m_hKuiWndHover;
    // Only one pushdown control
    HKUIWND          m_hKuiWndPushDown;

    int             m_nHoverItem;
    int             m_nPushItem;
    int             m_nLastDrawItem;

    // Tracking flag
    BOOL            m_bTrackFlag;

    BOOL            m_bCaptureSetted;
public:

    CKuiListViewImpl()
        : m_bXmlLoaded(FALSE)
        , m_nItemHeight(0)
        , m_hKuiWndHover(NULL)
        , m_hKuiWndPushDown(NULL)
        , m_bTrackFlag(FALSE)
        , m_bCaptureSetted(FALSE)
        , m_nHoverItem(-1)
        , m_nPushItem(-1)
        , m_nLastDrawItem(-1)
    {
    }

    ~CKuiListViewImpl()
    {
        m_imgMem.DeleteObject();
    }

    HWND Create(
        HWND hWndParent, _U_RECT rect = NULL, DWORD dwStyle = 0,
        DWORD dwExStyle = 0, _U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
    {
        if (NULL != m_hWnd)
            return m_hWnd;

        HWND hWnd = __baseClass::Create(hWndParent, rect, NULL, dwStyle, dwExStyle, MenuOrID, lpCreateParam);

        if (hWnd)
        {
            SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

            InsertColumn(0, L"", LVCFMT_LEFT, 1);

            m_kuiListTemplate.SetContainer(m_hWnd);

            m_wndToolTip.Create(hWnd);
            m_wndToolTip.SetMaxTipWidth(400);

            CToolInfo ti(0, hWnd);
            m_wndToolTip.AddTool(ti);
            m_wndToolTip.Activate(TRUE);
        }

        return hWnd;
    };

    HWND Create(HWND hWndParent, _U_MENUorID MenuOrID)
    {
        return Create(hWndParent, NULL, 0, 0, MenuOrID, NULL);
    };

    BOOL Load(UINT uResID)
    {
        CStringA strXml;

        BOOL bRet = KuiResUtil::LoadKuiXmlResource(uResID, strXml);

        if (!bRet)
            return FALSE;

        return SetXml(strXml);
    }

    BOOL XmlLoaded()
    {
        return m_bXmlLoaded;
    }

    BOOL SetXml(LPCSTR lpszXml)
    {
        CStringA strValue;

        TiXmlDocument xmlDoc;

        m_bXmlLoaded    = FALSE;

        m_kuiListTemplate.KuiSendMessage(WM_DESTROY);

        { // Free stack
            xmlDoc.Parse(lpszXml, NULL, TIXML_ENCODING_UTF8);
        }

        if (xmlDoc.Error())
        {
//             _Redraw();
            return FALSE;
        }

        TiXmlElement *pXmlRootElem = xmlDoc.RootElement();

        strValue = pXmlRootElem->Value();
        if (strValue != "listitem")
        {
//             _Redraw();
            return FALSE;
        }

        int nItemHeight = 0;
        pXmlRootElem->Attribute("height", &nItemHeight);

        if (0 != nItemHeight)
        {
            _SetItemHeight(nItemHeight);

            m_kuiListTemplate.Load(pXmlRootElem);
            CStringA strPos;
            strPos.Format("0,0,-0,%d", nItemHeight);
            m_kuiListTemplate.SetAttribute("pos", strPos, TRUE);
        }

        m_bXmlLoaded = TRUE;

        if (!m_hWnd)
            return TRUE;

//         _Redraw();

//         m_hKuiWndHover = NULL;
//         m_hKuiWndPushDown = NULL;

        return TRUE;
    }

    void RedrawItem(int nItem)
    {
        _RedrawItem(nItem, TRUE);
    }

    CKuiWindow* FindChildByCmdID(UINT uCmdID)
    {
        return m_kuiListTemplate.FindChildByCmdID(uCmdID);
    }

    BOOL SetItemText(UINT uItemID, LPCTSTR lpszText)
    {
        return CKuiViewImpl<T>::SetItemText(uItemID, lpszText);
    }

//     BOOL SetItemCount(int nItems)
//     {
//         SetColumnWidth(0, 1);
//
//         BOOL bRet = __super::SetItemCount(nItems);
//
//         _ResizeColumn();
//
//         return bRet;
//     }

public:

    DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
    {
        return CDRF_NOTIFYITEMDRAW;
    }

    DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW lpNMCustomDraw)
    {
        LPNMLVCUSTOMDRAW lpcd = (LPNMLVCUSTOMDRAW)lpNMCustomDraw;

        KuiWinManager::EnterPaintLock();

        int nItem = (int)lpcd->nmcd.dwItemSpec;

        if (m_nLastDrawItem != nItem)
        {
            _GetItemDisplayInfo(nItem);
        }

        CDC dcMem;
        CDCHandle dcMemHandle;
        HDC hDCDesktop = ::GetDC(NULL);
        dcMem.CreateCompatibleDC(hDCDesktop);
        ::ReleaseDC(NULL, hDCDesktop);
        HBITMAP hbmpOld = dcMem.SelectBitmap(m_imgMem);

        HFONT hftOld = dcMem.SelectFont(KuiFontPool::GetFont(KUIF_DEFAULTFONT));

        dcMem.SetBkMode(TRANSPARENT);

        CRgn rgnNull;

        dcMemHandle.Attach(dcMem);

        CKuiWindow *pWndHover = NULL, *pWndPush = NULL;

        if (nItem != m_nHoverItem && NULL != m_hKuiWndHover)
        {
            pWndHover = KuiWnds::GetWindow(m_hKuiWndHover);
            pWndHover->ModifyState(0, KuiWndState_Hover);
        }

        if (nItem != m_nPushItem && NULL != m_hKuiWndPushDown)
        {
            pWndPush = KuiWnds::GetWindow(m_hKuiWndPushDown);
            pWndPush->ModifyState(0, KuiWndState_PushDown);
        }

        m_kuiListTemplate.RedrawRegion(dcMemHandle, rgnNull);

        if (pWndHover)
            pWndHover->ModifyState(KuiWndState_Hover, 0);

        if (pWndPush)
            pWndPush->ModifyState(KuiWndState_PushDown, 0);

        dcMemHandle.Detach();

        dcMem.SelectFont(hftOld);
        dcMem.SelectBitmap(hbmpOld);

        KuiWinManager::LeavePaintLock();

        CRect rcItem;
        TBase::GetItemRect(nItem, rcItem, LVIR_BOUNDS);

        //CDCHandle dc = lpcd->nmcd.hdc;

        //dc.FillSolidRect(rcItem, RGB(0xCC, 0xCC, 0xCC));

        m_imgMem.Draw(lpcd->nmcd.hdc, rcItem.left, rcItem.top);

        return CDRF_SKIPDEFAULT;
    }

protected:

    BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
    {
        return FALSE;
    }

    void _RedrawItem(int nItem, BOOL bReload = FALSE)
    {
        if (-1 == nItem)
            return;

        if (bReload)
            m_nLastDrawItem = -1;

        ATLTRACE(L"RedrawItem(%d)\r\n", nItem);

        RedrawItems(nItem, nItem);
    }

    void _SetItemHeight(int nHeight)
    {
        CImageList imglst;

        imglst.Create(1, nHeight - 1, ILC_COLOR24, 0, 1);

        __super::SetImageList(imglst, LVSIL_SMALL);
        __super::SetImageList(NULL, LVSIL_SMALL);

        imglst.Destroy();

        m_nItemHeight = nHeight;

        CRect rcClient;

        GetClientRect(rcClient);

        if (rcClient.Width() == 0)
            return;

        if (m_imgMem.M_HOBJECT)
            m_imgMem.DeleteObject();

        m_imgMem.CreateBitmap(rcClient.Width(), m_nItemHeight, RGB(0, 0, 0));
    }

    void _ModifyWndState(CKuiWindow *pWnd, DWORD dwStateAdd, DWORD dwStateRemove)
    {
        pWnd->ModifyState(dwStateAdd, dwStateRemove);
    }

    void _GetItemDisplayInfo(int nItem)
    {
        KUILVMGETDISPINFO nms;
        nms.hdr.code    = KUILVM_GET_DISPINFO;
        nms.hdr.hwndFrom = m_hWnd;
        nms.hdr.idFrom  = GetDlgCtrlID();
        nms.bHover      = nItem == m_nHoverItem;
        nms.bSelect     = nItem == CListViewCtrl::GetSelectedIndex();
        nms.nListItemID = nItem;
        ::SendMessage(GetParent(), WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);

        m_nLastDrawItem = nItem;
    }

    void _InvalidateControl(CKuiWindow *pWnd, BOOL bCheckVisible = TRUE)
    {
    }

    int _HitTest(CPoint& point)
    {
        UINT uFlags = 0;
        int nItem = CListViewCtrl::HitTest(point, &uFlags);

        HKUIWND hKuiWndHitTest = NULL;

        if (-1 != nItem)
        {
            CRect rcItem;
            TBase::GetItemRect(nItem, rcItem, LVIR_BOUNDS);
            point -= rcItem.TopLeft();
        }

        return nItem;
    }

    void _ResizeColumn()
    {
        CRect rcClient;

        GetWindowRect(rcClient);

        if (rcClient.Width() == 0)
            return;

        if (m_imgMem.M_HOBJECT)
            m_imgMem.DeleteObject();

        rcClient.MoveToXY(0, 0);
        rcClient.right -= ::GetSystemMetrics(SM_CXVSCROLL);

        m_imgMem.CreateBitmap(rcClient.Width(), m_nItemHeight, RGB(0, 0, 0));

        SetColumnWidth(0, rcClient.Width());

        WINDOWPOS WndPos = { 0, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_SHOWWINDOW };

        m_kuiListTemplate.KuiSendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&WndPos);
    }

    void OnSize(UINT nType, CSize /*size*/)
    {
        _ResizeColumn();
    }

    void OnMouseMove(UINT nFlags, CPoint point)
    {
        if (!m_bTrackFlag)
        {
            m_nLastDrawItem = -1;

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = m_hWnd;
            tme.dwFlags = TME_LEAVE;
            tme.dwHoverTime = 0;
            m_bTrackFlag = _TrackMouseEvent(&tme);
        }

        HKUIWND hKuiWndHitTest = NULL;
        int nItem = _HitTest(point);

        if (-1 != nItem)
            hKuiWndHitTest = m_kuiListTemplate.KuiGetHWNDFromPoint(point, TRUE);

        if (m_nHoverItem != nItem)
        {
            CKuiWindow* pWndHoverOld = KuiWnds::GetWindow(m_hKuiWndHover);

            if (pWndHoverOld && !pWndHoverOld->IsDisabled(TRUE))
            {
                _ModifyWndState(pWndHoverOld, 0, KuiWndState_Hover);
                pWndHoverOld->KuiSendMessage(WM_MOUSELEAVE);
            }

            m_hKuiWndHover = NULL;

            int nOldHoverItem = m_nHoverItem;

            m_nHoverItem = nItem;

            _RedrawItem(nOldHoverItem, TRUE);
            _RedrawItem(m_nHoverItem, TRUE);
            m_wndToolTip.UpdateTipText(L"", m_hWnd);
        }

        CKuiWindow* pWndHover = KuiWnds::GetWindow(hKuiWndHitTest);

        if (pWndHover)
        {
            pWndHover->KuiSendMessage(WM_MOUSEMOVE, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));

            if (hKuiWndHitTest != m_hKuiWndHover)
            {
                CRect rcInvalidate;

                if (NULL != m_hKuiWndHover)
                {
                    CKuiWindow* pWndHoverOld = KuiWnds::GetWindow(m_hKuiWndHover);

                    if (pWndHoverOld && !pWndHoverOld->IsDisabled(TRUE))
                    {
                        _ModifyWndState(pWndHoverOld, 0, KuiWndState_Hover);
                        if (pWndHoverOld->NeedRedrawWhenStateChange())
                            _RedrawItem(m_nHoverItem);
                        pWndHoverOld->KuiSendMessage(WM_MOUSELEAVE);
                    }
                }

                if (!pWndHover->IsDisabled(TRUE))
                {
                    _ModifyWndState(pWndHover, KuiWndState_Hover, 0);
                    if (pWndHover->NeedRedrawWhenStateChange())
                        _RedrawItem(m_nHoverItem);
                }

                m_hKuiWndHover = hKuiWndHitTest;

                if (pWndHover)
                {
                    _GetItemDisplayInfo(m_nHoverItem);
                    m_wndToolTip.UpdateTipText(pWndHover->GetToolTipText(), m_hWnd);
                }
            }
        }
    }

    void OnMouseLeave()
    {
        m_wndToolTip.UpdateTipText(L"", m_hWnd);

        m_bTrackFlag = FALSE;

        if (m_hKuiWndHover)
        {
            CKuiWindow* pWndHover = KuiWnds::GetWindow(m_hKuiWndHover);
            if (pWndHover)
            {
                _ModifyWndState(pWndHover, 0, KuiWndState_Hover);
                pWndHover->KuiSendMessage(WM_MOUSELEAVE);
            }

            m_hKuiWndHover = NULL;
        }

//         if (m_hKuiWndPushDown)
//         {
//             CKuiWindow* pWndPushdown = KuiWnds::GetWindow(m_hKuiWndPushDown);
//             if (pWndPushdown)
//                 _ModifyWndState(pWndPushdown, 0, KuiWndState_PushDown);
//
//             m_hKuiWndPushDown = NULL;
//         }

        _RedrawItem(m_nHoverItem, TRUE);

        m_nHoverItem = -1;
    }

    void OnLButtonDown(UINT nFlags, CPoint point)
    {
        SetFocus();

        HKUIWND hKuiWndHitTest = NULL;
        int nItem = _HitTest(point), nSelectItem = CListViewCtrl::GetSelectedIndex();

        if (-1 != nItem)
            hKuiWndHitTest = m_kuiListTemplate.KuiGetHWNDFromPoint(point, TRUE);

        CListViewCtrl::SelectItem(nItem);

        if (nItem != nSelectItem)
        {
            int nOldSelect = nSelectItem;
            nSelectItem = nItem;
            _RedrawItem(nOldSelect, TRUE);
        }

        CKuiWindow* pWndPushDown = KuiWnds::GetWindow(hKuiWndHitTest);

        if (pWndPushDown)
        {
            if (pWndPushDown->IsDisabled(TRUE))
                return;

            if (!m_bCaptureSetted)
            {
                SetCapture();
                m_bCaptureSetted = TRUE;

                if (hKuiWndHitTest != m_hKuiWndHover)
                {
                    // Hover和Pushdown不同的原因是：鼠标一直没动，界面刷新，切换了鼠标所在位置的控件
                    //ATLASSERT(FALSE);
                    m_hKuiWndHover = hKuiWndHitTest;
                }

                m_hKuiWndPushDown = hKuiWndHitTest;

                _ModifyWndState(pWndPushDown, KuiWndState_PushDown, 0);

                m_nPushItem = nItem;

                pWndPushDown->KuiSendMessage(WM_LBUTTONDOWN, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));

                if (pWndPushDown->NeedRedrawWhenStateChange())
                    _RedrawItem(nItem);
            }
        }
    }

    void OnLButtonUp(UINT nFlags, CPoint point)
    {
        BOOL bNotifyClick = FALSE;
        UINT uCmdID = 0;
        CKuiWindow* pWndClick = NULL;

        if (m_bCaptureSetted)
        {
            ReleaseCapture();
            m_bCaptureSetted = FALSE;
        }

        if (m_hKuiWndPushDown)
        {
            pWndClick = KuiWnds::GetWindow(m_hKuiWndPushDown);

            if (pWndClick)
            {
                _ModifyWndState(pWndClick, 0, KuiWndState_PushDown);

                if (m_hKuiWndPushDown == m_hKuiWndHover && m_nHoverItem == m_nPushItem)
                {
                    if (m_nLastDrawItem != m_nPushItem)
                        _GetItemDisplayInfo(m_nPushItem);

                    pWndClick->KuiSendMessage(WM_LBUTTONUP, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));

                    LPCTSTR lpszUrl = pWndClick->GetLinkUrl();
                    if (lpszUrl && lpszUrl[0])
                    {
                        HINSTANCE hRet = ::ShellExecute(NULL, L"open", lpszUrl, NULL, NULL, SW_SHOWNORMAL);
                    }
                    else if (pWndClick->GetCmdID())
                    {
                        bNotifyClick = TRUE;
                        uCmdID = pWndClick->GetCmdID();
                    }
                }

                if (bNotifyClick)
                {
                    KUILVMITEMCLICK nms;
                    nms.hdr.code    = KUILVM_ITEMCLICK;
                    nms.hdr.hwndFrom = m_hWnd;
                    nms.hdr.idFrom  = GetDlgCtrlID();
                    nms.nListItemID = m_nPushItem;
                    nms.uCmdID      = uCmdID;

                    LRESULT lRet = ::SendMessage(GetParent(), WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);
                }

                _RedrawItem(m_nPushItem, pWndClick->IsClass("check") || pWndClick->IsClass("radio"));
                m_nPushItem = -1;
            }

            m_hKuiWndPushDown = NULL;
        }

    }

    BOOL OnSetCursor(CWindow /*wnd*/, UINT nHitTest, UINT message)
    {
        if (m_hKuiWndHover)
        {
            CKuiWindow *pKuiWndHover = KuiWnds::GetWindow(m_hKuiWndHover);

            if (pKuiWndHover && !pKuiWndHover->IsDisabled(TRUE))
            {
                pKuiWndHover->SetCursor();
                return TRUE;
            }
        }

        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));

        return TRUE;
    }

    BOOL OnEraseKuignd(CDCHandle dc)
    {
        int nTop = GetTopIndex();
        int nBottom = nTop + GetCountPerPage();

        nBottom = min(nBottom, GetItemCount() - 1);

        CRect rcClient, rcItemTop, rcItemBottom;
        COLORREF crBg = RGB(0xFF, 0xFF, 0xFF);

        GetClientRect(rcClient);
        TBase::GetItemRect(nTop, rcItemTop, LVIR_BOUNDS);
        TBase::GetItemRect(nBottom, rcItemBottom, LVIR_BOUNDS);

        dc.FillSolidRect(rcItemBottom.right, rcItemTop.top, rcClient.right - rcItemBottom.right, rcClient.bottom - rcItemTop.top, crBg);
        dc.FillSolidRect(rcClient.left, rcItemBottom.bottom, rcItemBottom.Width(), rcClient.bottom - rcItemBottom.bottom, crBg);

        return TRUE;
    }

    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
    {
        DefWindowProc();

        ScreenToClient(&pt);

        OnMouseMove(nFlags, pt);

        return FALSE;
    }

    LRESULT OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if(m_wndToolTip.IsWindow())
        {
            MSG msg = { m_hWnd, uMsg, wParam, lParam };

            m_wndToolTip.RelayEvent(&msg);
        }

        SetMsgHandled(FALSE);

        return 0;
    }

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {

    }

protected:

    BEGIN_MSG_MAP_EX(CKuiListViewImpl)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent)
        MSG_WM_ERASEBKGND(OnEraseKuignd)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDown)
        MSG_WM_SIZE(OnSize)
        MSG_WM_SETCURSOR(OnSetCursor)
        MSG_WM_KEYDOWN(OnKeyDown)
        CHAIN_MSG_MAP_ALT(CCustomDraw<T>, 1)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()
};

class CKuiListView : public CKuiListViewImpl<CKuiListView>
{
};
