#pragma once

//////////////////////////////////////////////////////////////////////////

#include "kscgui/kdraw.h"

//////////////////////////////////////////////////////////////////////////

class CBigFileListHdr;
typedef CWindowImpl<CBigFileListHdr, CHeaderCtrl> CBigFileListHdrBase;

//////////////////////////////////////////////////////////////////////////

class CBigFileListHdr 
    : public CBigFileListHdrBase
    , public CDoubleBufferImpl<CBigFileListHdr>
{
public:
    BEGIN_MSG_MAP(CBigFileListHdr)
        MSG_WM_SIZE(OnSize)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
        MESSAGE_HANDLER(HDM_LAYOUT, OnHdmLayout)
        REFLECTED_NOTIFY_CODE_HANDLER(HDN_ENDTRACK, OnEndTrack)
        CHAIN_MSG_MAP(CDoubleBufferImpl<CBigFileListHdr>)
    END_MSG_MAP()

    void Init()
    {
        GetClientRect(&m_rcClient);
    }

    BOOL SubclassWindow(HWND hWnd)
    {
        BOOL retval = FALSE;
        retval = CBigFileListHdrBase::SubclassWindow(hWnd);

        Init();
        return retval;
    }

    LRESULT OnEndTrack(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
    {
        GetParent().InvalidateRect(NULL);
        return 0L;
    }

    LRESULT OnHdmLayout(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LRESULT retval;
        HDLAYOUT* pHdLayout = (HDLAYOUT*)lParam;

        retval = DefWindowProc(uMsg, wParam, lParam);
        if (pHdLayout)
        {
            pHdLayout->pwpos->cy = 24;
            pHdLayout->prc->top = 24;
            pHdLayout->prc->bottom -= 24;
        }

        return 0;
    }

    LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = TRUE;
        return 0;
    }

    void DoPaint(CDCHandle dc)
    {
        CRect rcTemp;
        CFontHandle font = KuiFontPool::GetFont(FALSE, FALSE, FALSE);
        CRect rcSplit, rcHalf;
        int nItemCount = GetItemCount();

        GetClientRect(&m_rcClient);
        rcTemp = m_rcClient;
        rcTemp.bottom -= 1;
        rcHalf = rcTemp;
        rcHalf.bottom = rcTemp.Height() / 2;
        DrawGradualColorRect(dc, rcHalf, RGB(255,255,255), RGB(248,253,255));
        rcHalf.top = rcHalf.bottom;
        rcHalf.bottom = m_rcClient.bottom - 1;
        DrawGradualColorRect(dc, rcHalf, RGB(229,246,254), RGB(238,249,255));
        rcTemp.top = m_rcClient.bottom - 1;
        rcTemp.bottom = m_rcClient.bottom;
        dc.FillSolidRect(rcTemp, RGB(150,174,202));

        dc.SetBkMode(TRANSPARENT);
        dc.SetTextColor(RGB(0,0,0));
        dc.SelectFont(font);
        for (int i = 0; i < nItemCount; ++i)
        {
            HDITEM hdi;
            CString strText;

            hdi.mask = HDI_TEXT;
            hdi.pszText = strText.GetBuffer(MAX_PATH);
            hdi.cchTextMax = MAX_PATH;

            GetItem(i, &hdi);
            strText.ReleaseBuffer();
            GetItemRect(i, rcTemp);

            dc.DrawText(strText, strText.GetLength(), rcTemp, 
                DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

            rcSplit.left = rcTemp.right - 2;
            rcSplit.right = rcTemp.right - 1;
            rcSplit.top = rcTemp.top + 4;
            rcSplit.bottom = rcTemp.bottom - 3;
            dc.FillSolidRect(rcSplit, RGB(162,186,212));
            rcSplit.left = rcTemp.right - 1;
            rcSplit.right = rcTemp.right - 0;
            dc.FillSolidRect(rcSplit, RGB(255,255,255));
        }
    }

    void OnSize(UINT nType, CSize size)
    {
        GetClientRect(&m_rcClient);
    }

private:
    RECT m_rcClient;
};

//////////////////////////////////////////////////////////////////////////

#include "kclearmsg.h"

//////////////////////////////////////////////////////////////////////////

class CBigFileListCtrl;
typedef CWindowImpl<CBigFileListCtrl, CListViewCtrl> CBigFileListCtrlBase;

//////////////////////////////////////////////////////////////////////////

typedef struct tagLinkInfo {
    CString strLink;
    CRect rcLink;
} LinkInfo;

//////////////////////////////////////////////////////////////////////////

class CBigFileListCtrl
    : public CBigFileListCtrlBase
    , public COwnerDraw<CBigFileListCtrl>
{
public:
    CBigFileListCtrl(void* pOwner);
    ~CBigFileListCtrl();

    void Init();
    BOOL SubclassWindow(HWND hWnd);

    void AddLink(const CString& strLink);

    // вт╩Ф
    void DoDrawItem(CDCHandle dc, LPDRAWITEMSTRUCT lpDrawItemStruct);
    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
    void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);

    void SetSysFilterEnable(BOOL bEnable);
    BOOL GetSysFilterEnable();

    BOOL OnEraseBkgnd(CDCHandle dc);

    BOOL DeleteItem(int nItem);

    void SetNotifyHwnd(HWND hNotifyWnd);

    void SetMoreItem(DWORD dwMoreItems);

    BEGIN_MSG_MAP_EX(CBigFileListCtrl)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_RBUTTONDOWN(OnRButtonDown)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        CHAIN_MSG_MAP_ALT(COwnerDraw<CBigFileListCtrl>,1)
    END_MSG_MAP()

protected:
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonDblClk(UINT nFlags, CPoint point);
    void OnMouseMove(UINT nFlags, CPoint point);
    void OnRButtonDown(UINT nFlags, CPoint point);
    void OnLButtonUp(UINT nFlags, CPoint point);
    LRESULT OnNMClick(LPNMHDR pnmh);
    LRESULT OnNMDClick(LPNMHDR pnmh);
    void OnVScroll(UINT nSBCode, UINT nPos, HWND hWnd);
    LRESULT OnMouseWhell(UINT fwKey, short zDela, CPoint pt);
    BOOL OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
    LRESULT OnReflectedNotifyRangeHandlerEX(LPNMHDR pnmh);
    LRESULT OnMouseHover(WPARAM wParam, CPoint ptPos);
    LRESULT OnMouseLeave();
    LRESULT OnWmLBtnColunm( UINT uMsg, WPARAM wParam, LPARAM lParam );
    LRESULT OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

    int IsMouseInLink(CPoint pt, int& nItem);
    BOOL IsMouseInMore(CPoint pt, int& nItem);
    int HitTest(CPoint pt);

protected:
    CToolTipCtrl m_wndToolTip;
    HWND m_hNotifyWnd;
    void* m_pOwner;
    std::vector<LinkInfo> m_vLinks;
    CRect m_rcMore;
    int m_nItemHeight;
    int m_nHeaderHeight;
    CBigFileListHdr m_hdr;
    int m_nHotItem;
    int m_nHotLink;
    BOOL m_bMoreLink;
    DWORD m_dwMoreItems;
    CString m_strTip;
    BOOL m_bSysFilter;
};

//////////////////////////////////////////////////////////////////////////
