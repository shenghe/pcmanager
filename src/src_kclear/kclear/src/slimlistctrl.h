//////////////////////////////////////////////////////////////////////////
// Create Author: zhang zexin
// Create Date: 2011-01-17
// Description: 系统盘瘦身的窗口显示的列表（windows减肥）
// Modify Record:
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////
#include "slimcallback.h"
#include <map>
#include "kscgui/kdraw.h"
#include "kscbase/ksclock.h"
//////////////////////////////////////////////////////////////////////////
#define ITEM_ROW_HEIGHT         52
/////////////////////////////////////////////////////////////////////////

typedef struct _SLIM_ITEM
{
    UINT nItemID;
    BOOL bChecked;
    BOOL bEnable;
    CRect rcCheck;
    CRect rcButton;
    CRect rcLink;
    CString strName;
    int nIconIndex;
    CString strBrief;
    CString strDescription;
    ULONGLONG uSizeDisk;
    ULONGLONG uSizeSlim;
    SlimMethod nTreatment;
    BOOL bInit;

    _SLIM_ITEM()
    {
        nTreatment = SLIM_DELETE_FILE;
        uSizeDisk = 0;
        uSizeSlim = 0;
        bChecked = FALSE;
        nIconIndex = -1;
        bEnable = FALSE;
        bInit = FALSE;
    }

//    _SLIM_ITEM& operator=(const _SLIM_ITEM& slim)
//    {
// 
//    }
}SLIM_DRAW_ITEM, *LPSLIM_DRAW_ITEM;
//////////////////////////////////////////////////////////////////////////
class CSysSlimListHdr;
typedef CWindowImpl<CSysSlimListHdr, CHeaderCtrl> CSysSlimListHdrBase;
//////////////////////////////////////////////////////////////////////////
class CSysSlimListHdr 
    : public CSysSlimListHdrBase
    , public CDoubleBufferImpl<CSysSlimListHdr>
{
public:
    BEGIN_MSG_MAP(CSysSlimListHdr)
        MSG_WM_SIZE(OnSize)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
        MESSAGE_HANDLER(HDM_LAYOUT, OnHdmLayout)
//        REFLECTED_NOTIFY_CODE_HANDLER(HDN_ENDTRACK, OnEndTrack)
        CHAIN_MSG_MAP(CDoubleBufferImpl<CSysSlimListHdr>)
    END_MSG_MAP()

    void Init()
    {
        GetClientRect(&m_rcClient);
    }

    BOOL SubclassWindow(HWND hWnd)
    {
        BOOL retval = FALSE;
        retval = CSysSlimListHdrBase::SubclassWindow(hWnd);

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
class CSlimListCtrl : public CWindowImpl<CSlimListCtrl, CListViewCtrl>, 
                      public COwnerDraw<CSlimListCtrl>
{
public:

    CSlimListCtrl(void);
    virtual ~CSlimListCtrl(void);

public:

    void Initialize();

    void AddItem(SLIM_DRAW_ITEM& item);

    BOOL SubclassWindow(HWND hWnd);
    void SetNotifyHwnd(HWND hWnd);
    void DeleteItem(int nItem);
    void DeleteItem(LPDELETEITEMSTRUCT lpdis);
    BOOL RemoveAll();
    void DrawItem(LPDRAWITEMSTRUCT lpdis);
    void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

    void SetEnable(BOOL bEnable);
    void SetItemStatus(int nID, BOOL bEnable, ULONGLONG uSizeAfterSlim, ULONGLONG uSizeOnDisk, BOOL bInit = TRUE);
    
public:

    BEGIN_MSG_MAP(CSlimListCtrl)
        MSG_WM_SIZE(OnSize)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        CHAIN_MSG_MAP_ALT(COwnerDraw<CSlimListCtrl>, 1)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    BOOL OnEraseBkgnd(CDCHandle dc);
    void OnLButtonDown(UINT nFlags, CPoint point);	
    void OnMouseMove(UINT nFlags, CPoint point);
    void OnSize(UINT nType, CSize size);
    LRESULT OnBeginTrack(WPARAM wParam, LPNMHDR lParam, BOOL& bHandled);
	LRESULT OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
    CSysSlimListHdr m_hdr;
    HWND m_hNotifyHwnd;
    CToolTipCtrl m_wndToolTip;
    BOOL m_bToolTipActive;
    CString m_strTip;
    CBitmap m_checkBmp;
    CImageList		m_iconList;
    CImageList      m_diableIconList;   // 和 m_iconList 是 一一对应关系，一个为正常显示，一个为disable显示
    CRect m_rcButton;
    CRect m_rcLink;
    CRect m_rcDraw;

    BOOL m_bEnable;
    int m_nHoverIndex;
    int m_nSelectedIndex;
    BOOL m_bCursorHand;
    BOOL m_bInButton;
    BOOL m_bSelectBtn;

    std::vector<SLIM_DRAW_ITEM> m_vSlimItems;

protected:

    BOOL _GetItemData(UINT nItem, SLIM_DRAW_ITEM& item);
    int _HitTest(CPoint pt);


    void _DrawItemText(CDCHandle& dc, int nItem);
    void _DrawItemBKGround(CDCHandle& dc, int nItem, BOOL bSelected);
    void _DrawCheckBox(CDCHandle& dc, int nItem);
    void _DrawIcon(CDCHandle& dc, int nItem);
    void _DrawItemSize(CDCHandle& dc,int nItem);
    void _DrawItemTreat(CDCHandle& dc, int nItem);
    void _DrawItemPath(CDCHandle& dc, int nItem);

    void _DrawText(CDCHandle& dc, CRect& rcItem, LPCTSTR pszText , COLORREF colorText , HFONT hFont);
    void _DrawLine(CDCHandle& memDC, CPoint ptStart, CPoint ptEnd, COLORREF color, DWORD dwPenTyple /*=PS_SOLID*/);

    int _GetIconIndexByID(int nID);

private:
    KLock m_lock;
};
