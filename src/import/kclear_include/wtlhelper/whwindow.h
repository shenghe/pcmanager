#pragma once

#include <kuiwin/kuiimage.h>
#include <kuiwin/kuiskin.h>
#include "wh3statecheckimglst.h"

//////////////////////////////////////////////////////////////////////////
// Cached Owner Draw Window Implement
//////////////////////////////////////////////////////////////////////////

template <class T>
class CWHCachedOwnerDrawWindowImpl
{
protected:
    CWHCachedOwnerDrawWindowImpl()
        : m_bNeedRedraw(TRUE)
    {
    }

    virtual ~CWHCachedOwnerDrawWindowImpl()
    {
        if (!m_imgCache.IsNull())
            m_imgCache.DeleteObject();
    }

    BOOL m_bNeedRedraw;
    CKuiImage m_imgCache;

    void OnSize(UINT nType, CSize size)
    {
        SetMsgHandled(FALSE);

        if (nType == SIZE_MINIMIZED)
            return;

        if (!m_imgCache.IsNull())
            m_imgCache.DeleteObject();

        m_imgCache.CreateBitmap(size.cx, size.cy);

        NeedRedraw();
    }

    void OnPaint(CDCHandle /*dc*/)
    {
        CPaintDC dc(static_cast<T*>(this)->m_hWnd);

        if (m_bNeedRedraw)
        {
            CDC dcMem;

            dcMem.CreateCompatibleDC(dc);

            HBITMAP hbmpOld = dcMem.SelectBitmap(m_imgCache);

            static_cast<T*>(this)->DoPaint((HDC)dcMem);

            dcMem.SelectBitmap(hbmpOld);

            m_bNeedRedraw = FALSE;
        }

        m_imgCache.Draw(dc, 0, 0);
    }

    BOOL OnEraseKuignd(CDCHandle dc)
    {
        return TRUE;
    }

public:

    void NeedRedraw()
    {
        m_bNeedRedraw = TRUE;
        static_cast<T*>(this)->Invalidate(FALSE);
    }

    // Need Overide
    void DoPaint(CDCHandle dc)
    {
        ATLASSERT(FALSE);
    }

public:

	BEGIN_MSG_MAP_EX(CWHCachedOwnerDrawWindowImpl<T>)
        MSG_WM_SIZE(OnSize)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_ERASEBKGND(OnEraseKuignd)
	END_MSG_MAP()
};

//////////////////////////////////////////////////////////////////////////
// Resize Control Replacing Helper
//////////////////////////////////////////////////////////////////////////

template <class T>
class CWHControlReplacingHelperImpl
{
protected:

    void OnSize(UINT nType, CSize size)
    {
        SetMsgHandled(FALSE);

        if (nType == SIZE_MINIMIZED)
            return;

        CRect rcClient;

        static_cast<T*>(this)->GetClientRect(rcClient);

        static_cast<T*>(this)->ReplacingControls(rcClient);
    }

public:

    // Need Overide
    void ReplacingControls(CRect& rcClient)
    {
        ATLASSERT(FALSE);
    }

	BEGIN_MSG_MAP_EX(CWHControlReplacingHelperImpl<T>)
        MSG_WM_SIZE(OnSize)
	END_MSG_MAP()
};

#define BEGIN_REPLACING_CONTROLS()                          \
    void ReplacingControls(CRect& rcClient)                 \
    {                                                       \
        CWindow wndCtrl;                                    \

#define REPLACING_CONTROL_BASE(id, x, y, cx, cy, method)    \
        wndCtrl.Attach(::GetDlgItem(m_hWnd, id));           \
        wndCtrl.SetWindowPos(                               \
            NULL, x, y, cx, cy,                             \
            method | SWP_NOZORDER | SWP_NOACTIVATE);        \
        wndCtrl.Detach();                                   \

#define END_REPLACING_CONTROLS()                            \
    }                                                       \

#define PARENT_WIDTH    rcClient.Width()
#define PARENT_HEIGHT   rcClient.Height()

#define REPLACING_CONTROL(id, x, y, cx, cy)                 \
        REPLACING_CONTROL_BASE(id, x, y, cx, cy, 0)         \

#define REPLACING_CONTROL_POSITON(id, x, y)                 \
        REPLACING_CONTROL_BASE(id, x, y, 0, 0, SWP_NOSIZE)  \

//////////////////////////////////////////////////////////////////////////
// Skined Button Control
//////////////////////////////////////////////////////////////////////////

class CWHSkinedButton
    : public CWindowImpl<CWHSkinedButton, CButton>
    , public COwnerDraw<CWHSkinedButton>
{
public:
    CWHSkinedButton()
        : m_bTracking(FALSE)
        , m_bPushed(FALSE)
        , m_bHorzExtend(FALSE)
        , m_lHorzExSkinLeft(0)
        , m_crText(CLR_INVALID)
    {

    }

    BOOL SubclassWindow(HWND hWnd)
    {
        BOOL bRet = __super::SubclassWindow(hWnd);
        if (bRet)
            CWindow::ModifyStyle(0, BS_OWNERDRAW);

        return bRet;
    }

    void SetSkin(HBITMAP hbmpSkin, BOOL bSingle = FALSE, BOOL bHasDisable = FALSE, COLORREF crText = CLR_INVALID, BOOL bAutoResize = TRUE)
    {
        m_imgSkin.Attach(hbmpSkin);

        if (m_imgSkin.IsNull())
            return;

        m_imgSkin.SetTransparentMode(CKuiImage::ModeMaskColor);

        SIZE sizeImg;

        m_imgSkin.GetImageSize(sizeImg);

        if (!bSingle)
            sizeImg.cx /= bHasDisable ? 4 : 3;

        m_imgSkin.SetSubImageWidth(sizeImg.cx);

        if (bAutoResize)
            ResizeClient(sizeImg.cx, sizeImg.cy);

        m_bHorzExtend = FALSE;
        m_crText = crText;
    }

    void SetHorzExtendSkin(HBITMAP hbmpSkin, LONG lHorzExSkinLeft, BOOL bSingle = FALSE, BOOL bHasDisable = FALSE, COLORREF crText = CLR_INVALID, BOOL bAutoResize = TRUE)
    {
        SetSkin(hbmpSkin, bSingle, bHasDisable, crText, FALSE);

        m_lHorzExSkinLeft = lHorzExSkinLeft;

        if (bAutoResize)
        {
            SIZE sizeImg;

            m_imgSkin.GetImageSize(sizeImg);

            if (!bSingle)
                sizeImg.cx /= bHasDisable ? 4 : 3;

            CRect rcBtn;
            GetWindowRect(rcBtn);

            ResizeClient(rcBtn.Width(), sizeImg.cy);
        }

        m_bHorzExtend = TRUE;
    }

    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
    {
        int nSubImage = -1;
        CRect rcBtn = lpDrawItemStruct->rcItem;
        CDCHandle dc;

        dc.Attach(lpDrawItemStruct->hDC);

        if (m_bTracking)
        {
            if (m_bPushed)
                nSubImage = 2;
            else
                nSubImage = 1;
        }
        else
        {
            nSubImage = 0;
        }

        if (m_bHorzExtend)
        {
            CKuiSkinBase::HorzExtendDraw(dc, m_imgSkin, rcBtn, m_lHorzExSkinLeft, nSubImage);
        }
        else
        {
            m_imgSkin.Draw(lpDrawItemStruct->hDC, 0, 0, nSubImage);
        }

        if (CLR_INVALID != m_crText)
        {
            CString strText;

            GetWindowText(strText);

            dc.SetBkMode(TRANSPARENT);

            HFONT hftOld = dc.SelectFont(KuiFontPool::GetFont(KUIF_DEFAULTFONT));
            COLORREF crOld = dc.SetTextColor(m_crText);
            dc.DrawText(strText, strText.GetLength(), rcBtn, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            dc.SelectFont(hftOld);
            dc.SetTextColor(crOld);
        }
    }

protected:

    void OnLButtonDown(UINT nFlags, CPoint point)
    {
        SetCapture();

        m_bPushed = TRUE;

        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }

    void OnLButtonUp(UINT nFlags, CPoint point)
    {
        ReleaseCapture();

        m_bPushed = FALSE;

        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

        if (m_bTracking)
            GetParent().SendMessage(WM_COMMAND, GetDlgCtrlID(), (LPARAM)m_hWnd);
    }

    void OnMouseMove(UINT nFlags, CPoint point)
    {
        CRect rcBtn;

        GetClientRect(rcBtn);

        BOOL bTracking = rcBtn.PtInRect(point);

        if (bTracking != m_bTracking)
        {
            m_bPushed   = ((MK_LBUTTON & nFlags) == MK_LBUTTON);
            m_bTracking = bTracking;

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = m_hWnd;
            tme.dwFlags = TME_LEAVE;
            tme.dwHoverTime = 0;
            _TrackMouseEvent(&tme);

            RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        }
    }

    void OnMouseLeave()
    {
        if (m_bTracking)
        {
            m_bTracking = FALSE;
            m_bPushed = FALSE;

            RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        }
    }

    BOOL OnSetCursor(CWindow /*wnd*/, UINT /*nHitTest*/, UINT /*message*/)
    {
        SetCursor(::LoadCursor(NULL, IDC_HAND));

        return TRUE;
    }

    CKuiImage m_imgSkin;
    BOOL m_bTracking;
    BOOL m_bPushed;

    BOOL m_bHorzExtend;
    LONG m_lHorzExSkinLeft;
    COLORREF m_crText;

    BEGIN_MSG_MAP_EX(CWHSkinedButton)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_SETCURSOR(OnSetCursor)
        CHAIN_MSG_MAP_ALT(COwnerDraw<CWHSkinedButton>, 1)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()
};

template <class T>
class CWHRoundRectFrameHelper
{
protected:

    SIZE m_sizeWnd;

    void OnSize(UINT nType, CSize size)
    {
        T *pT = static_cast<T*>(this);

        if (nType == SIZE_MINIMIZED)
            return;

        if (size == m_sizeWnd)
            return;

        CRect rcWindow, rcClient;
        CRgn rgnWindow, rgnMinus, rgnAdd;

        pT->GetWindowRect(rcWindow);
        pT->GetClientRect(rcClient);
        pT->ClientToScreen(rcClient);

        rcClient.OffsetRect(- rcWindow.TopLeft());

        rgnWindow.CreateRectRgn(rcClient.left, rcClient.top + 2, rcClient.right, rcClient.bottom - 2);
        rgnAdd.CreateRectRgn(rcClient.left + 2, rcClient.top, rcClient.right - 2, rcClient.top + 1);
        rgnWindow.CombineRgn(rgnAdd, RGN_OR);
        rgnAdd.OffsetRgn(0, rcClient.Height() - 1);
        rgnWindow.CombineRgn(rgnAdd, RGN_OR);
        rgnAdd.SetRectRgn(rcClient.left + 1, rcClient.top + 1, rcClient.right - 1, rcClient.top + 2);
        rgnWindow.CombineRgn(rgnAdd, RGN_OR);
        rgnAdd.OffsetRgn(0, rcClient.Height() - 3);
        rgnWindow.CombineRgn(rgnAdd, RGN_OR);
//         rgnAdd.OffsetRgn(0, rcClient.Height());
//         rgnWindow.CombineRgn(rgnAdd, RGN_OR);

//         rgnMinus.CreateRectRgn(rcClient.left, rcClient.top, rcClient.left + 2, rcClient.top + 1);
//         rgnWindow.CombineRgn(rgnMinus, RGN_DIFF);
//         rgnMinus.OffsetRgn(rcClient.Width() - 2, 0);
//         rgnWindow.CombineRgn(rgnMinus, RGN_DIFF);
//         rgnMinus.OffsetRgn(0, rcClient.Height() - 1);
//         rgnWindow.CombineRgn(rgnMinus, RGN_DIFF);
//         rgnMinus.OffsetRgn(3 - rcClient.Width(), 0);
//         rgnWindow.CombineRgn(rgnMinus, RGN_DIFF);
//
//         rgnMinus.SetRectRgn(rcClient.left, rcClient.top, rcClient.left + 1, rcClient.top + 2);
//         rgnWindow.CombineRgn(rgnMinus, RGN_DIFF);
//         rgnMinus.OffsetRgn(rcClient.Width() - 1, 0);
//         rgnWindow.CombineRgn(rgnMinus, RGN_DIFF);
//         rgnMinus.OffsetRgn(0, rcClient.Height() - 2);
//         rgnWindow.CombineRgn(rgnMinus, RGN_DIFF);
//         rgnMinus.OffsetRgn(1 - rcClient.Width(), 0);
//         rgnWindow.CombineRgn(rgnMinus, RGN_DIFF);

        pT->SetWindowRgn(rgnWindow);

        m_sizeWnd = size;
    }

public:

    BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0)
    {
        BOOL bHandled = TRUE;

        switch(dwMsgMapID)
        {
        case 0:
            if (uMsg == WM_SIZE)
            {
                OnSize((UINT)wParam, _WTYPES_NS::CSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                lResult = 0;
            }
            break;
        }
        return FALSE;
    }
};

template <class T>
class CWHRoundRectDialog
    : public T
    , public CWHRoundRectFrameHelper<CWHRoundRectDialog<T>>
{
public:
    BEGIN_MSG_MAP_EX(CWHRoundRectDialog<T>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CWHRoundRectDialog<T>>)
        CHAIN_MSG_MAP(T)
    END_MSG_MAP()
};

template <class T>
class CWHDragMoveDlgHelper
{
protected:

public:

    BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0)
    {
        BOOL bHandled = TRUE;

        switch(dwMsgMapID)
        {
        case 0:
            if (uMsg == WM_LBUTTONDOWN)
            {
                T *pT = static_cast<T*>(this);

                if (0 == (pT->GetStyle() & (WS_MAXIMIZE | WS_MINIMIZE)))
                    pT->SendMessage(WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);

                pT->SetMsgHandled(TRUE);
            }
            break;
        }
        return FALSE;
    }
};

class CWHHeaderCtrl
    : public CWindowImpl<CWHHeaderCtrl, CHeaderCtrl>
{
public:
    ~CWHHeaderCtrl()
    {
        for(int i = m_iSortUp; i <= m_iSortDown; i++)
        {
            if(!m_bmSort[i].IsNull())
                m_bmSort[i].DeleteObject();
        }
    }

    typedef struct _FRG_PARAM
    {
        LONG lOffset;
        COLORREF crColor;
    } FRG_PARAM;

    void FrameRect(HDC hDC, RECT rect, COLORREF crColor)
    {
        HBRUSH hBrush = ::CreateSolidBrush(crColor);

        ::FrameRect(hDC, &rect, hBrush);

        ::DeleteObject(hBrush);
    }

    void Line(HDC hDC, int x1, int y1, int x2, int y2, COLORREF crColor)
    {
        HPEN hPen = ::CreatePen(PS_SOLID, 1, crColor), hOldPen = NULL;

        hOldPen = (HPEN)::SelectObject(hDC, hPen);

        ::MoveToEx(hDC, x1, y1, NULL);
        ::LineTo(hDC, x2, y2);

        ::SelectObject(hDC, hOldPen);

        ::DeleteObject(hPen);
    }

    void GradientFillRectV(HDC hdc, CRect rcFill, FRG_PARAM params[], int nCount)
    {
        GRADIENT_RECT gRect = {0, 1};
        TRIVERTEX vert[2] = {
            {rcFill.left, rcFill.top, 0, 0, 0, 0},
            {rcFill.right, rcFill.top, 0, 0, 0, 0}
        };
        int i = 0;

        for (i = 1; i < nCount && vert[0].y <= rcFill.bottom; i ++)
        {
            vert[0].y = vert[1].y;
            vert[1].y += params[i].lOffset;
            vert[0].Red     = GetRValue(params[i - 1].crColor) << 8;
            vert[0].Green   = GetGValue(params[i - 1].crColor) << 8;
            vert[0].Blue    = GetBValue(params[i - 1].crColor) << 8;
            vert[1].Red     = GetRValue(params[i].crColor) << 8;
            vert[1].Green   = GetGValue(params[i].crColor) << 8;
            vert[1].Blue    = GetBValue(params[i].crColor) << 8;
            CKuiSkinBase::GradientFill2(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
        }
    }

    void DrawHeaderBg(HDC hdc, CRect rcRect)
    {
        FRG_PARAM frgHeaderBg[2] = {
            {0, RGB(242, 247, 252)},
            {21, RGB(231, 235, 242)},
        };

        rcRect.bottom --;

        FrameRect(hdc, rcRect, RGB(0xFF, 0xFF, 0xFF));
        Line(hdc, rcRect.left, rcRect.bottom, rcRect.right, rcRect.bottom, RGB(195, 195, 195));

        rcRect.DeflateRect(1, 1);

        GradientFillRectV(hdc, rcRect, frgHeaderBg, 2);
    }

    void OnPaint(CDCHandle /*dc*/)
    {
        CPaintDC dc(m_hWnd);
        CRect rcClient, rcItem;
        int nItems = GetItemCount();
        HFONT hftOld = NULL;
        TCHAR szText[MAX_PATH] = {0};
        HDITEM hditem = {0};
        UINT uFormat = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;

        GetClientRect(&rcClient);
        rcClient.InflateRect(1, 0);

        dc.SetBkMode(TRANSPARENT);

        DrawHeaderBg(dc, rcClient);

        hftOld = dc.SelectFont(KuiFontPool::GetFont(KUIF_DEFAULTFONT));
        dc.SetTextColor(RGB(0, 0, 0));
        uFormat |= DT_CENTER;

        for (int i = 0; i < nItems; i++)
        {
            hditem.mask = HDI_TEXT | HDI_FORMAT;
            hditem.pszText = szText;
            hditem.cchTextMax = sizeof(szText);

            if (!GetItem(i, &hditem))
                continue;

            if (!GetItemRect(i, rcItem))
                continue;

            Line(dc, rcItem.right - 1, rcItem.top + 2, rcItem.right - 1, rcItem.bottom - 3, RGB(195, 195, 195));
            Line(dc, rcItem.right, rcItem.top + 2, rcItem.right, rcItem.bottom - 3, RGB(255, 255, 255));

            rcItem.DeflateRect(5, 0);
            if (rcItem.left < rcItem.right)
                dc.DrawText(szText, (int)wcslen(szText), &rcItem, uFormat);

            if (HDF_SORTDOWN == (hditem.fmt & HDF_SORTDOWN) && rcItem.Width() > 11)
            {
                CreateSortBitmaps();
                m_bmSort[m_iSortDown].Draw(dc, rcItem.right - 11, rcItem.top + (rcItem.Height() - 15) / 2);
            }
            else if (HDF_SORTUP == (hditem.fmt & HDF_SORTUP) && rcItem.Width() > 11)
            {
                CreateSortBitmaps();
                m_bmSort[m_iSortUp].Draw(dc, rcItem.right - 11, rcItem.top + (rcItem.Height() - 15) / 2);
            }
        }

        dc.SelectFont(hftOld);
    }

    enum
    {
        m_iSortUp = 0,        // index of sort bitmaps
        m_iSortDown = 1,
        m_nShellSortUpID = 133
    };

    CKuiImage m_bmSort[2];

    void CreateSortBitmaps()
    {
        bool bFree = false;
        LPCTSTR pszModule = _T("shell32.dll");
        HINSTANCE hShell = ::GetModuleHandle(pszModule);

        if (hShell == NULL)
        {
            hShell = ::LoadLibrary(pszModule);
            bFree = true;
        }

        if (hShell != NULL)
        {
            bool bSuccess = true;
            for(int i = m_iSortUp; i <= m_iSortDown; i++)
            {
                if(!m_bmSort[i].IsNull())
                    continue;
                m_bmSort[i] = (HBITMAP)::LoadImage(hShell, MAKEINTRESOURCE(m_nShellSortUpID + i),
                    IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
                if(m_bmSort[i].IsNull())
                {
                    bSuccess = false;
                    break;
                }
                m_bmSort[i].SetTransparentMode(CKuiImage::ModeMaskColor);
                m_bmSort[i].SetMaskColor(::GetSysColor(COLOR_3DFACE));
            }
            if(bFree)
                ::FreeLibrary(hShell);
            if(bSuccess)
                return;
        }
    }

    LRESULT OnLayout(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT lResult = 0L;
        LPHDLAYOUT pLayOut = (LPHDLAYOUT)lParam;
        RECT&      rect = *(pLayOut->prc);
        WINDOWPOS& pos	= *(pLayOut->pwpos);

        lResult = DefWindowProc(HDM_LAYOUT, wParam, lParam);

        pos.cy   = 24;
        rect.top = 24;

        return lResult;
    }

    void OnLButtonDblClk(UINT nFlags, CPoint point)
    {
        Invalidate();

        SetMsgHandled(FALSE);
    }

protected:

	BEGIN_MSG_MAP_EX(CWHHeaderCtrl)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
        MESSAGE_HANDLER_EX(HDM_LAYOUT, OnLayout)
	END_MSG_MAP()
};

#define WHLVN_ITEMSORT  (LVN_LAST - 1)

typedef struct tagWHLVNITEMSORT
{
    NMHDR hdr;
    int nCol;
    BOOL bDescending;
} WHLVNITEMSORT, *LPWHLVNITEMSORT;

class CWHListViewCtrl
    : public CWindowImpl<CWHListViewCtrl, CListViewCtrl, CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0>>
    , public CCustomDraw<CWHListViewCtrl>
{
public:
    CWHListViewCtrl()
        : m_nSortColumn(-1)
        , m_bSortDescending(FALSE)
        , m_nItemHeight(20)
    {
    }

    HWND Create(
        HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
        DWORD dwStyle = 0, DWORD dwExStyle = 0, _U_MENUorID MenuOrID = 0U,
        LPVOID lpCreateParam = NULL)
    {
        HWND hWnd = __super::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);

        if (!hWnd)
            return NULL;

        SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

        m_imglstCheck.Create();

        SetItemHeight();

        ShowCheckBox(FALSE);

        SetFont(KuiFontPool::GetFont(KUIF_DEFAULTFONT), FALSE);

        if (0 == (dwStyle & LVS_NOCOLUMNHEADER))
            m_hdrListVirus.SubclassWindow(GetHeader());

        return hWnd;
    }

    void SetItemHeight(int nHeight = 0)
    {
        CImageList imglst;

        imglst.Create(max(1, m_imgList.GetSubImageWidth()), nHeight ? nHeight : m_nItemHeight, ILC_COLOR24, 0, 1);

        __super::SetImageList(imglst, LVSIL_SMALL);
        __super::SetImageList(NULL, LVSIL_SMALL);

        imglst.Destroy();

        if (nHeight)
            m_nItemHeight = nHeight;
    }

    void ShowCheckBox(BOOL bShow)
    {
        if (bShow)
        {
            SetExtendedListViewStyle(LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
            __super::SetImageList(m_imglstCheck, LVSIL_STATE);

        }
        else
        {
            __super::SetImageList(NULL, LVSIL_STATE);
            SetExtendedListViewStyle(0, LVS_EX_CHECKBOXES);
        }
    }

    BOOL GetCheckState(int nItem)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            if (nItem >= GetItemCount() || nItem < 0)
                return FALSE;

            if (nItem >= (int)m_arrItemCheck.GetCount())
                return FALSE;

            return (2 == m_arrItemCheck[nItem]);
        }
        else
            return __super::GetCheckState(nItem);
    }

    BOOL SetCheckState(int nItem, BOOL bCheck, BOOL bForce = FALSE, BOOL bRedraw = TRUE)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            if (nItem >= GetItemCount() || nItem < 0)
                return FALSE;

            if (nItem >= (int)m_arrItemCheck.GetCount())
                m_arrItemCheck.SetCount(GetItemCount());

            if (bForce || 0 != m_arrItemCheck[nItem])
            {
                m_arrItemCheck[nItem] = bCheck ? 2 : 1;

                if (bRedraw)
                    RedrawItems(nItem, nItem);
            }

            return TRUE;
        }
        else
            return __super::SetCheckState(nItem, bCheck);
    }

    BOOL DisableItemCheck(int nItem, BOOL bRedraw = TRUE)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            if (nItem >= GetItemCount() || nItem < 0)
                return FALSE;

            if (nItem >= (int)m_arrItemCheck.GetCount())
                m_arrItemCheck.SetCount(GetItemCount());

            m_arrItemCheck[nItem] = 0;

            if (bRedraw)
                RedrawItems(nItem, nItem);

            return TRUE;
        }
        else
            return __super::SetItemState(nItem, INDEXTOSTATEIMAGEMASK(0), LVIS_STATEIMAGEMASK);
    }

    BOOL IsItemCheckDisabled(int nItem)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            if (nItem >= GetItemCount())
                return FALSE;

            if (nItem >= (int)m_arrItemCheck.GetCount())
                m_arrItemCheck.SetCount(GetItemCount());

            return 0 == m_arrItemCheck[nItem];
        }
        else
            return 0 == (__super::GetItemState(nItem, LVIS_STATEIMAGEMASK) >> 12);
    }

    BOOL IsAllItemChecked()
    {
        ATLASSERT(::IsWindow(m_hWnd));

        int nCount = GetItemCount();
        BOOL bAllCheck = TRUE;

        for (int i = 0; i < nCount; i ++)
        {
            if (!IsItemCheckDisabled(i))
                bAllCheck &= GetCheckState(i);
        }

        return bAllCheck;
    }

    BOOL IsAnyItemChecked()
    {
        ATLASSERT(::IsWindow(m_hWnd));

        int nCount = GetItemCount();

        for (int i = 0; i < nCount; i ++)
        {
            if (!IsItemCheckDisabled(i))
            {
                if (GetCheckState(i))
                    return TRUE;
            }
        }

        return FALSE;
    }

    BOOL CheckAllItems(BOOL bCheck, BOOL bForce = FALSE, BOOL bRedraw = TRUE)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        int nCount = GetItemCount();

        for (int i = 0; i < nCount; i ++)
            SetCheckState(i, bCheck, bForce, bRedraw);

        return TRUE;
    }

    void SetImageList(HBITMAP hImageList, int cx, COLORREF crMaskColor = RGB(0xFF, 0, 0xFF))
    {
        m_imgList.Attach(hImageList);
        m_imgList.SetTransparentMode(CKuiImage::ModeMaskColor);
        m_imgList.SetMaskColor(crMaskColor);
        m_imgList.SetSubImageWidth(cx);

        SetItemHeight();
    }

    DWORD_PTR GetItemData(int nItem) const
    {
        ATLASSERT(::IsWindow(m_hWnd));

        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            if (nItem >= GetItemCount() || nItem < 0)
                return 0;

            if (nItem >= (int)m_arrItemData.GetCount())
                return 0;

            return m_arrItemData[nItem];
        }
        else
            return __super::GetItemData(nItem);
    }

    BOOL SetItemData(int nItem, DWORD_PTR dwData)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            if (nItem >= GetItemCount() || nItem < 0)
                return FALSE;

            if (nItem >= (int)m_arrItemData.GetCount())
                m_arrItemData.SetCount(GetItemCount());

            m_arrItemData[nItem] = dwData;

            return TRUE;
        }
        else
            return __super::SetItemData(nItem, dwData);
    }

    BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
    {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));

        return TRUE;
    }

    DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
    {
        return CDRF_NOTIFYITEMDRAW;
    }

    DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW lpNMCustomDraw)
    {
        LPNMLVCUSTOMDRAW lpcd = (LPNMLVCUSTOMDRAW)lpNMCustomDraw;
        CRect rcItem, rcDraw;
        CKuiImage img;
        WTL::CDC dc;
        CString strText;
        BOOL bEnabled = IsWindowEnabled();

        GetItemRect((int)lpcd->nmcd.dwItemSpec, rcItem, LVIR_BOUNDS);

        img.CreateBitmap(rcItem.Width(), rcItem.Height());

        dc.CreateCompatibleDC(lpcd->nmcd.hdc);
        dc.SetWindowOrg(rcItem.TopLeft());

        dc.SetBkMode(TRANSPARENT);

        if (!bEnabled)
            dc.SetTextColor(RGB(0xAA, 0xAA, 0xAA));

        HBITMAP hbmpOld = dc.SelectBitmap(img);

        // Draw Background
//         COLORREF crBg = RGB(0xDD, 0xDD, 0xDD);
//         if (bEnabled)
//             crBg = ((lpcd->nmcd.dwItemSpec & 1) == 1) ? RGB(0xFA, 0xFA, 0xFA) : RGB(0xFF, 0xFF, 0xFF);

        COLORREF crBg = RGB(0xFF, 0xFF, 0xFF);

        if (LVIS_SELECTED == GetItemState((int)lpcd->nmcd.dwItemSpec, LVIS_SELECTED) && bEnabled)
            crBg = RGB(0xB9, 0xDB, 0xFF);

        dc.FillSolidRect(rcItem, crBg);

        GetItemRect((int)lpcd->nmcd.dwItemSpec, rcDraw, LVIR_SELECTBOUNDS);

        //rcDraw = rcItem;

        if (LVS_EX_CHECKBOXES == (LVS_EX_CHECKBOXES & GetExtendedListViewStyle()))
        {
            UINT uCheck = IsItemCheckDisabled((int)lpcd->nmcd.dwItemSpec) ? 0 : (GetCheckState((int)lpcd->nmcd.dwItemSpec) ? 2 : 1);

            SIZE size;
            CRect rcCheck = rcDraw;

            m_imglstCheck.GetIconSize(size);

            rcCheck.top += (rcDraw.Height() - size.cy) / 2;
            rcCheck.left += 2;

            rcDraw.left += size.cx;

            m_imglstCheck.Draw(dc, uCheck, rcCheck.TopLeft(), ILD_TRANSPARENT);
        }

        {
            CPen penLine;

            penLine.CreatePen(
                PS_SOLID,
                1,
                RGB(0xEA, 0xEA, 0xEA)
                );
            HPEN hpenOld = dc.SelectPen(penLine);
            dc.MoveTo(rcItem.left, rcItem.bottom - 1);
            dc.LineTo(rcItem.right, rcItem.bottom - 1);
            dc.SelectPen(hpenOld);
        }

/*
        if (LVIS_SELECTED == GetItemState((int)lpcd->nmcd.dwItemSpec, LVIS_SELECTED) && bEnabled)
        {
            CWHHeaderCtrl::FRG_PARAM frgSelBg[2] = {
                {0, RGB(242, 247, 252)},
                {rcDraw.Height() - 2, RGB(231, 235, 242)},
            };
            CPen penFrame;

            penFrame.CreatePen(
                PS_SOLID,
                1,
                RGB(209, 214, 221)
                );

            HPEN hpenOld = dc.SelectPen(penFrame);
            HBRUSH hbshOld = NULL, hbshNull = (HBRUSH)::GetStockObject(NULL_BRUSH);

            hbshOld = dc.SelectBrush(hbshNull);

            rcDraw.left ++;
            dc.RoundRect(rcDraw, CPoint(2, 2));

            rcDraw.DeflateRect(1, 1);

            m_hdrListVirus.GradientFillRectV(dc, rcDraw, frgSelBg, 2);

            dc.SelectBrush(hbshOld);
            dc.SelectPen(hpenOld);
        }
*/

        if (!m_imgList.IsNull())
        {
            LVITEM li = {LVIF_IMAGE, (int)lpcd->nmcd.dwItemSpec};
            if (GetItem(&li) && li.iImage != -1)
            {
                GetItemRect((int)lpcd->nmcd.dwItemSpec, rcDraw, LVIR_ICON);
                SIZE sizeImg;
                m_imgList.GetImageSize(sizeImg);
                m_imgList.Draw(dc, rcDraw.left - 2, rcDraw.top + (rcDraw.Height() - sizeImg.cy) / 2, li.iImage);
            }
        }

        // Draw Sub Items

        HFONT hftOld = dc.SelectFont(KuiFontPool::GetFont(KUIF_DEFAULTFONT));

        for (int nSubItem = 0; nSubItem < GetHeader().GetItemCount(); nSubItem ++)
        {
            GetSubItemRect((int)lpcd->nmcd.dwItemSpec, nSubItem, LVIR_LABEL, rcDraw);
            UINT uFormat = DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER;

            GetItemText((int)lpcd->nmcd.dwItemSpec, nSubItem, strText);

            LVCOLUMN colInfo = {LVCF_FMT};

            GetColumn(nSubItem, &colInfo);

            if (colInfo.fmt & LVCFMT_CENTER)
                uFormat |= DT_CENTER;
            else if (colInfo.fmt & LVCFMT_RIGHT)
                uFormat |= DT_RIGHT;

            dc.DrawText(strText, strText.GetLength(), rcDraw, uFormat);
        }

        dc.SelectFont(hftOld);
        dc.SelectBitmap(hbmpOld);

        img.Draw(lpcd->nmcd.hdc, rcItem.left, rcItem.top);

        img.DeleteObject();

        return CDRF_SKIPDEFAULT;
    }

    LRESULT OnLVNHeaderItemClick(LPNMHDR pnmh)
    {
        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            LPNMHEADER p = (LPNMHEADER)pnmh;
            if (p->iButton == 0)
            {
                int nOld = m_nSortColumn;

                WHLVNITEMSORT nms;
                nms.hdr.code = WHLVN_ITEMSORT;
                nms.hdr.hwndFrom = m_hWnd;
                nms.hdr.idFrom = GetDlgCtrlID();
                nms.bDescending = (m_nSortColumn == p->iItem) ? !m_bSortDescending : FALSE;
                nms.nCol = p->iItem;

                LRESULT lRet = ::SendMessage(GetParent(), WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);

                if (lRet)
                {
                    m_nSortColumn = p->iItem;
                    m_bSortDescending = nms.bDescending;

                    HDITEM hdi = {HDI_FORMAT};
                    m_hdrListVirus.GetItem(nOld, &hdi);
                    hdi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
                    m_hdrListVirus.SetItem(nOld, &hdi);
                    m_hdrListVirus.GetItem(m_nSortColumn, &hdi);
                    hdi.fmt |= m_bSortDescending ? HDF_SORTUP : HDF_SORTDOWN;
                    m_hdrListVirus.SetItem(m_nSortColumn, &hdi);
                }
            }
        }

        SetMsgHandled(FALSE);

        return 0;
    }

    BOOL OnEraseKuignd(CDCHandle dc)
    {
        int nTop = GetTopIndex();
        int nBottom = nTop + GetCountPerPage();

        nBottom = min(nBottom, GetItemCount() - 1);

        CRect rcClient, rcItemTop, rcItemBottom;
        COLORREF crBg = IsWindowEnabled() ? RGB(0xFF, 0xFF, 0xFF) : RGB(0xDD, 0xDD, 0xDD);

        GetClientRect(rcClient);
        GetItemRect(nTop, rcItemTop, LVIR_BOUNDS);
        GetItemRect(nBottom, rcItemBottom, LVIR_BOUNDS);

        dc.FillSolidRect(rcItemBottom.right, rcItemTop.top, rcClient.right - rcItemBottom.right, rcClient.bottom - rcItemTop.top, crBg);
        dc.FillSolidRect(rcClient.left, rcItemBottom.bottom, rcItemBottom.Width(), rcClient.bottom - rcItemBottom.bottom, crBg);

        return TRUE;
    }

    LRESULT OnLVNKeyDown(LPNMHDR pnmh)
    {
        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            LPNMLVKEYDOWN pnms = (LPNMLVKEYDOWN)pnmh;

            if (VK_SPACE == pnms->wVKey)
            {
                int nCurSel = GetSelectedIndex();
                if (-1 != nCurSel)
                {
                    BOOL bCheck = !GetCheckState(nCurSel);
                    SetCheckState(nCurSel, bCheck);
                    _SendItemCheckStateChangedNotify(nCurSel, bCheck);
                }
            }
        }
        else
            SetMsgHandled(FALSE);

        return 0;
    }

    LRESULT OnNMClick(LPNMHDR pnmh)
    {
        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            LPNMLISTVIEW pnms = (LPNMLISTVIEW)pnmh;
            LVHITTESTINFO hitinfo;

            hitinfo.pt = pnms->ptAction;

            int nItem = HitTest(&hitinfo);

            if(nItem != -1)
            {
                if( (hitinfo.flags & LVHT_ONITEM) == LVHT_ONITEMSTATEICON)
                {
                    BOOL bCheck = !GetCheckState(nItem);
                    SetCheckState(nItem, bCheck);
                    _SendItemCheckStateChangedNotify(nItem, bCheck);
                }
            }
        }
        else
            SetMsgHandled(FALSE);

        return 0;
    }

    void _SendItemCheckStateChangedNotify(int nItem, BOOL bCheck)
    {
        NMLISTVIEW nms = {0};

        nms.hdr.idFrom = GetDlgCtrlID();
        nms.hdr.hwndFrom = m_hWnd;
        nms.hdr.code = LVN_ITEMCHANGED;

        nms.iItem = nItem;
        nms.iSubItem = 0;
        nms.uOldState = INDEXTOSTATEIMAGEMASK(bCheck ? 1 : 2);
        nms.uNewState = INDEXTOSTATEIMAGEMASK(bCheck ? 2 : 1);

        ::SendMessage(::GetParent(m_hWnd), WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);
    }

protected:

    CWHHeaderCtrl m_hdrListVirus;
    CWH3StateCheckImageList m_imglstCheck;
    CKuiImage m_imgList;
    int m_nSortColumn;
    int m_nItemHeight;
    BOOL m_bSortDescending;
    CAtlArray<DWORD_PTR> m_arrItemData;
    CAtlArray<int> m_arrItemCheck;

    BEGIN_MSG_MAP_EX(CWHListViewCtrl)
        MSG_WM_ERASEBKGND(OnEraseKuignd)
        MSG_WM_SETCURSOR(OnSetCursor)
        NOTIFY_CODE_HANDLER_EX(HDN_ITEMCLICKA, OnLVNHeaderItemClick)
        NOTIFY_CODE_HANDLER_EX(HDN_ITEMCLICKW, OnLVNHeaderItemClick)
        REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_KEYDOWN, OnLVNKeyDown)
        REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_CLICK, OnNMClick)
        REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnNMClick)
        CHAIN_MSG_MAP_ALT(CCustomDraw<CWHListViewCtrl>, 1)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()
};

#define WHNM_TRAYICON 1

typedef struct _WHNMTRAYICON
{
    NMHDR hdr;
    UINT  uMsg;
} WHNMTRAYICON, *LPWHNMTRAYICON;

class CWHTrayIcon
    : public CWindowImpl<CWHTrayIcon>
{
public:
    CWHTrayIcon()
    {
    }

    ~CWHTrayIcon()
    {
        if (IsWindow())
            DestroyWindow();
    }

    BOOL Create(HWND hWndNotify, UINT uID, UINT uIconResID)
    {
        if (!IsWindow())
        {
            HWND hWnd = __super::Create(NULL, NULL, NULL, WS_POPUP);
            if (NULL == hWnd)
                return FALSE;
        }

        NOTIFYICONDATA nid = {sizeof(NOTIFYICONDATA)};
        HICON hTray = ::LoadIcon((HINSTANCE)(&__ImageBase), MAKEINTRESOURCE(uIconResID));

        nid.uFlags              = NIF_ICON | NIF_MESSAGE;
        nid.hIcon               = hTray;
        nid.hWnd                = m_hWnd;
        nid.uID                 = 0;
        nid.uCallbackMessage    = WM_APP;

        BOOL bRet = ::Shell_NotifyIcon(NIM_ADD, &nid);

        ::DestroyIcon(hTray);

        if (bRet)
        {
            m_hWndNotify = hWndNotify;
            m_uID        = uID;
            m_uIconResID = uIconResID;
        }

        return bRet;
    }

    BOOL SetTip(LPCTSTR lpszTip)
    {
        NOTIFYICONDATA nid = {sizeof(NOTIFYICONDATA)};

        nid.uFlags              = NIF_TIP;
        nid.hWnd                = m_hWnd;
        nid.uID                 = 0;
        ::StrCpyN(nid.szTip, lpszTip, 63);

        return ::Shell_NotifyIcon(NIM_MODIFY, &nid);
    }

    BOOL ShowBalloon(LPCTSTR lpszBalloonTitle,LPCTSTR lpszBalloonInfo,UINT uTimeout)
    {
        NOTIFYICONDATA IconData = {0};
        IconData.cbSize = sizeof(NOTIFYICONDATA);
        IconData.hWnd   = m_hWnd;
        IconData.uID    = 0;
        IconData.uFlags = NIF_INFO;

        lstrcpy(IconData.szInfo, lpszBalloonInfo);
        lstrcpy(IconData.szInfoTitle, lpszBalloonTitle);
        IconData.uTimeout   = uTimeout;
        IconData.dwState    = NIS_SHAREDICON;
        IconData.dwStateMask = 0;
        IconData.dwInfoFlags = NIIF_INFO;

        return Shell_NotifyIcon(NIM_MODIFY, &IconData);
    }

    BOOL SetIcon(UINT uIconResID)
    {
        NOTIFYICONDATA nid = {sizeof(NOTIFYICONDATA)};
        HICON hTray = ::LoadIcon((HINSTANCE)(&__ImageBase), MAKEINTRESOURCE(uIconResID));

        nid.uFlags              = NIF_ICON;
        nid.hIcon               = hTray;
        nid.hWnd                = m_hWnd;
        nid.uID                 = 0;

        BOOL bRet = ::Shell_NotifyIcon(NIM_MODIFY, &nid);

        ::DestroyIcon(hTray);

        return bRet;
    }

    BOOL Destroy()
    {
        NOTIFYICONDATA nid = {sizeof(NOTIFYICONDATA)};

        nid.uFlags              = 0;
        nid.hWnd                = m_hWnd;
        nid.uID                 = 0;

        return ::Shell_NotifyIcon(NIM_DELETE, &nid);
    }

protected:

    static const UINT WM_TASKBARCREATED;

    HWND m_hWndNotify;
    UINT m_uID;
    UINT m_uIconResID;

    void OnDestroy()
    {
        Destroy();
    }

    LRESULT OnTaskbarCreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
    {
        Create(m_hWndNotify, m_uID, m_uIconResID);

        return 0;
    }

    LRESULT OnTrayNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (m_hWndNotify && ::IsWindow(m_hWndNotify) && ::IsWindowEnabled(m_hWndNotify))
        {
            WHNMTRAYICON nms;
            nms.hdr.code = WHNM_TRAYICON;
            nms.hdr.hwndFrom = m_hWnd;
            nms.hdr.idFrom = m_uID;
            nms.uMsg = (UINT)lParam;

            LRESULT lRet = ::SendMessage(m_hWndNotify, WM_NOTIFY, (LPARAM)m_uID, (WPARAM)&nms);
        }

        return 0;
    }

    static UINT _RegisterTaskbarCreatedMessage()
    {
        UINT uMsg = ::RegisterWindowMessage(_T("TaskbarCreated"));

        typedef BOOL (WINAPI *pfn_ChangeWindowMessageFilter)(UINT, DWORD);
        #define MyMSGFLT_ADD 1

        pfn_ChangeWindowMessageFilter MyChangeWindowMessageFilter
            = (pfn_ChangeWindowMessageFilter)::GetProcAddress(::GetModuleHandle(L"user32.dll"), "ChangeWindowMessageFilter");

        if (MyChangeWindowMessageFilter)
            MyChangeWindowMessageFilter(uMsg, MyMSGFLT_ADD);

        return uMsg;
    }

public:
    BEGIN_MSG_MAP_EX(CWHTrayIcon)
        MSG_WM_DESTROY(OnDestroy)
        MESSAGE_HANDLER_EX(WM_TASKBARCREATED, OnTaskbarCreated)
        MESSAGE_HANDLER_EX(WM_APP, OnTrayNotify)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()
};

__declspec(selectany) const UINT CWHTrayIcon::WM_TASKBARCREATED = CWHTrayIcon::_RegisterTaskbarCreatedMessage();


class CWHLink
    : public CWindowImpl<CWHLink, CStatic>
{
public:
    CWHLink()
        : m_bTrackFlag(FALSE)
        , m_crBg(GetSysColor(COLOR_3DFACE))
    {

    }

    void SetBackground(COLORREF crBg)
    {
        m_crBg = crBg;
    }

    BOOL SubclassWindow(HWND hWnd)
    {
        BOOL bRet = __super::SubclassWindow(hWnd);

        if (bRet)
            ModifyStyle(0, SS_NOTIFY);

        return bRet;
    }

    void OnPaint(CDCHandle /*dc*/)
    {
        CString strCaption;
        CRect rcClient;
        CPaintDC dc(m_hWnd);

        GetWindowText(strCaption);
        GetClientRect(rcClient);

        dc.SetBkMode(TRANSPARENT);

        HFONT hftOld = dc.SelectFont(KuiFontPool::GetFont(FALSE, TRUE, FALSE));
        dc.FillSolidRect(rcClient, m_crBg);

        if (m_bTrackFlag)
        {
            dc.SetTextColor(RGB(0xFF, 0, 0));
        }
        else
        {
            dc.SetTextColor(RGB(0x30, 0x64, 0xA5));
        }

        dc.DrawText(strCaption, strCaption.GetLength(), rcClient, DT_SINGLELINE);

        dc.SetTextColor(RGB(0, 0, 0));

        dc.SelectFont(hftOld);
    }

    BOOL OnEraseKuignd(CDCHandle dc)
    {
        return TRUE;
    }

    void OnMouseMove(UINT nFlags, CPoint point)
    {
        if (!m_bTrackFlag)
        {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = m_hWnd;
            tme.dwFlags = TME_LEAVE;
            tme.dwHoverTime = 0;
            m_bTrackFlag = _TrackMouseEvent(&tme);

            Invalidate(FALSE);
        }
    }

    void OnMouseLeave()
    {
        m_bTrackFlag = FALSE;

        Invalidate(FALSE);
    }

    void OnLButtonDown(UINT nFlags, CPoint point)
    {
        SetCapture();
    }

    void OnLButtonUp(UINT nFlags, CPoint point)
    {
        ReleaseCapture();

        GetParent().SendMessage(WM_COMMAND, GetDlgCtrlID());
    }

    BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
    {
        ::SetCursor(::LoadCursor(NULL, IDC_HAND));

        return TRUE;
    }


protected:

    BOOL m_bTrackFlag;
    COLORREF m_crBg;

    BEGIN_MSG_MAP_EX(CWHLink)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_ERASEBKGND(OnEraseKuignd)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_SETCURSOR(OnSetCursor)
    END_MSG_MAP()
};

class CWHHtmlContainerWindow
    : public CWindowImpl<CWHHtmlContainerWindow>
    , public IDispEventImpl<IDYES, CWHHtmlContainerWindow>
{
public:
    CWHHtmlContainerWindow()
        : m_bNavigateError(FALSE)
        , m_crBg(RGB(0xFF, 0xFF, 0xFF))
    {
    }

    HWND Create(HWND hWndParent, UINT nID, BOOL bVisible = TRUE, COLORREF crBg = RGB(0xFF, 0xFF, 0xFF))
    {
        HWND hWnd = __super::Create(hWndParent, NULL, NULL, WS_CHILD/* | WS_CLIPCHILDREN*/ | (bVisible ? WS_VISIBLE : 0), 0, nID);
        if (!hWnd)
            return NULL;

        m_crBg = crBg;

        _CreateIEControl(hWnd);

        return hWnd;
    }

    void Show(LPCTSTR lpszURL)
    {
        m_bNavigateError = FALSE;

        m_strUrl = lpszURL;

        SetTimer(1, 500);

//         ::CloseHandle(::CreateThread(NULL, 0, _CreateCtrlProc, this, 0, NULL));
    }

	// Navi 失败后自动加载一个本地页面
	void Show2(LPCTSTR lpszUrl, LPCTSTR lpszErrorUrl)
	{
		if (lpszErrorUrl != NULL)
			m_strErrorUrl = lpszErrorUrl;

		Show(lpszUrl);
	}

    void SetBkColor(COLORREF color)
    {
        m_crBg = color;
        Invalidate();
    }

protected:

    CAxWindow m_wndIE;
    CString m_strUrl;
    BOOL m_bNavigateError;
    COLORREF m_crBg;
	CString	  m_strErrorUrl;

    static DWORD WINAPI _CreateCtrlProc(LPVOID pvParam)
    {
        CWHHtmlContainerWindow *pThis = (CWHHtmlContainerWindow *)pvParam;
        CComPtr<IWebBrowser2> piWebBrowser;
        CComVariant v;


        ::Sleep(1000);

        pThis->m_wndIE.QueryControl(&piWebBrowser);

        if (piWebBrowser)
        {
            piWebBrowser->put_RegisterAsDropTarget( VARIANT_FALSE );
            piWebBrowser->put_Silent( VARIANT_TRUE );
            piWebBrowser->Navigate(CComBSTR(pThis->m_strUrl), &v, &v, &v, &v);
        }

        return 0;
    }

    void WINAPI NavigateComplete2WebAdinfo(LPDISPATCH pDisp, VARIANT* URL)
    {
        if (!m_bNavigateError)
        {
            m_wndIE.ShowWindow(SW_SHOW);
        }
    }

    void WINAPI NavigateErrorWebAdinfo(
        IDispatch* pDisp,
        VARIANT* pvtURL,
        VARIANT* pvtFrame,
        VARIANT* pvtStatusCode,
        VARIANT_BOOL* pvbCancel
        )
    {
		BOOL bNaviErrorPage = FALSE;
		if (pvtURL!=NULL && !m_strErrorUrl.IsEmpty())
		{
			CComBSTR bstrUrl(pvtURL->bstrVal);
			if (bstrUrl == CComBSTR(m_strUrl))
			{
				CComPtr<IWebBrowser2> piWebBrowser;
				CComVariant v;

				m_wndIE.QueryControl(&piWebBrowser);

				if (piWebBrowser)
				{
                    piWebBrowser->put_RegisterAsDropTarget( VARIANT_FALSE );
					piWebBrowser->put_Silent( VARIANT_TRUE );
					piWebBrowser->Navigate(CComBSTR(m_strErrorUrl), &v, &v, &v, &v);
					bNaviErrorPage = TRUE;
				}
			}
		}


		if ( !bNaviErrorPage)
		{
			m_bNavigateError = TRUE;
		}
		*pvbCancel = VARIANT_TRUE;
    }

    void WINAPI NewWindow3WebAdinfo(
        IDispatch **ppDisp,
        VARIANT_BOOL *Cancel,
        DWORD dwFlags,
        BSTR bstrUrlContext,
        BSTR bstrUrl)
    {
        CString strUrl(bstrUrl);

        ::ShellExecute(NULL, L"open", strUrl, NULL, NULL, SW_SHOWNORMAL);

        *Cancel = TRUE;
    }

    void OnDestroy()
    {
        AtlAdviseSinkMap(this, FALSE);
        if (m_wndIE.IsWindow())
            m_wndIE.DestroyWindow();
    }

    BOOL OnEraseKuignd(CDCHandle dc)
    {
//         if (!m_wndIE.IsWindow() || !m_wndIE.IsWindowVisible())
//         {
//             CRect rcClient;
//
//             GetClientRect(rcClient);
//
//             dc.FillSolidRect(rcClient, m_crBg);
//             dc.SetBkMode(TRANSPARENT);
//             HFONT hftOld = dc.SelectFont(KuiFontPool::GetFont(KUIF_DEFAULTFONT));
//
//             CString strText = L"正在读取信息...";
//             dc.DrawText(strText, strText.GetLength(), rcClient, DT_SINGLELINE);
//
//             dc.SelectFont(hftOld);
//         }

        return TRUE;
    }

    void OnPaint(CDCHandle /*dc*/)
    {
        CPaintDC dc(m_hWnd);

        if (!m_wndIE.IsWindow() || !m_wndIE.IsWindowVisible())
        {
            CRect rcClient;

            GetClientRect(rcClient);

            dc.FillSolidRect(rcClient, m_crBg);
        }
    }

    void OnSize(UINT nType, CSize /*size*/)
    {
        if (m_wndIE.IsWindow())
        {
            CRect rcClient;

            GetClientRect(rcClient);

            m_wndIE.MoveWindow(rcClient);
        }
    }

    void OnTimer(UINT_PTR nIDEvent)
    {
        if (1 != nIDEvent)
            return;

        KillTimer(1);

        CComPtr<IWebBrowser2> piWebBrowser;
        CComVariant v;

        //::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

        m_wndIE.QueryControl(&piWebBrowser);

        if (piWebBrowser)
        {
            piWebBrowser->put_RegisterAsDropTarget( VARIANT_FALSE );
            piWebBrowser->put_Silent( VARIANT_TRUE );
            piWebBrowser->Navigate(CComBSTR(m_strUrl), &v, &v, &v, &v);
        }

        //::CoUninitialize();
    }

    void _CreateIEControl(HWND hWnd)
    {
        CRect rcClient;

        GetClientRect(rcClient);

        m_wndIE.Create(m_hWnd, rcClient, NULL, WS_CHILD | WS_CLIPCHILDREN);

        m_wndIE.CreateControl(L"Shell.Explorer");
        m_wndIE.SetDlgCtrlID(IDYES);

        AtlAdviseSinkMap(this, TRUE);
    }

	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
	{
		CComPtr<IWebBrowser2> pWebBrowser;
		m_wndIE.QueryControl(&pWebBrowser);

		if (pWebBrowser)
		{
			IServiceProvider* pServiceProvider = NULL;
			if (SUCCEEDED(pWebBrowser->QueryInterface(
				IID_IServiceProvider,
				(void**)&pServiceProvider)))
			{
				IOleWindow* pWindow = NULL;
				if (SUCCEEDED(pServiceProvider->QueryService(
					SID_SShellBrowser,
					IID_IOleWindow,
					(void**)&pWindow)))
				{
					HWND hwndBrowser = NULL;
					if (SUCCEEDED(pWindow->GetWindow(&hwndBrowser)))
					{
						OutputDebugString(_T("发送消息"));

						HWND hwnd = NULL;
						hwnd = ::FindWindowEx(hwndBrowser,NULL,TEXT("Shell DocObject View"),NULL);
						if (hwnd)
						{
							HWND hIE = ::FindWindowEx(hwnd,NULL,TEXT("Internet Explorer_Server"),NULL);

							if ( zDelta < 0 )
							{
								::SendMessage(hIE, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN,0),0);
							}
							else
							{
								::SendMessage(hIE, WM_VSCROLL, MAKEWPARAM(SB_LINEUP,0),0);
							}

						}

					}

					pWindow->Release();
				}
				pServiceProvider->Release();
			}
		}

		return FALSE;
	}

public:
    BEGIN_MSG_MAP(CWHHtmlContainerWindow)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_ERASEBKGND(OnEraseKuignd)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_SIZE(OnSize)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    BEGIN_SINK_MAP(CWHHtmlContainerWindow)
        SINK_ENTRY(IDYES, 252, NavigateComplete2WebAdinfo)
        SINK_ENTRY(IDYES, 271, NavigateErrorWebAdinfo)
        SINK_ENTRY(IDYES, 273, NewWindow3WebAdinfo)
    END_SINK_MAP()
};

class CWHEdit
    : public CWindowImpl<CWHEdit, CEdit>
{
public:
    CWHEdit()
        : m_crBg(CLR_DEFAULT)
    {

    }

    void SetBkColor(COLORREF crBg)
    {
        m_crBg = crBg;

        Invalidate(TRUE);
    }

protected:
    COLORREF m_crBg;

    HBRUSH OnReflectedCtlColor(CDCHandle dc, HWND /*hWnd*/)
    {
        if (CLR_DEFAULT == m_crBg)
        {
            SetMsgHandled(FALSE);
            return NULL;
        }

        dc.SetBkColor(m_crBg);

        return ::CreateSolidBrush(m_crBg);
    }

public:
    BEGIN_MSG_MAP(CWHHtmlContainerWindow)
        MSG_OCM_CTLCOLORSTATIC(OnReflectedCtlColor)
        MSG_OCM_CTLCOLOREDIT(OnReflectedCtlColor)
    END_MSG_MAP()
};

