//////////////////////////////////////////////////////////////////////////
//
//  Create Author: ZhangZexin
//  Create Date  : 2011-03-04
//  Description  : 垃圾清理界面，清理完成时，提示跳过那些软件的控件
//////////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////////

typedef struct  _SKIP_ITEM  
{
    UINT nItemID;
    CString strName;
    CString strIconPath;
    CString strProcess;
    ULONGLONG uSize;
    ULONGLONG uAddSize; // 额外添加的大小，如在 ie项中sogou项已经存在，那么其余扫描出的大小则加到这一项中。
    CRect rcItem;
    _SKIP_ITEM()
    {
        rcItem.SetRect(0,0,0,0);
        nItemID = -1;
        uSize = 0;
        uAddSize = 0;
    }
} TRASH_SKIP_ITEM, *LPTRASH_SKIP_ITEM;

//////////////////////////////////////////////////////////////////////////
class CTrashSkipCtrl : public CWindowImpl<CTrashSkipCtrl>
{
public:
    DECLARE_WND_CLASS(_T("TrashSkipCtrl"));

    CTrashSkipCtrl(void);
    virtual ~CTrashSkipCtrl(void);

//////////////////////////////////////////////////////////////////////////
#define DEF_ITEM_WIDTH              103
#define DEF_ITEM_HEIGHT             103
#define DEF_HORI_SPACE              50
#define DEF_VERT_SPACE              16
#define DEF_HEAD_HEIGHT             32
#define DEF_LEFT_SPACE              25
#define DEF_RIGHT_SPACE             40
//////////////////////////////////////////////////////////////////////////
public:

    void Initialize(HWND hWnd);

    BOOL SubclassWindow(HWND hWnd);

public:
    void Clear();

    BOOL AddItem(TRASH_SKIP_ITEM item);

    BOOL AddItem(UINT id, CString& strName, CString& strProcess, ULONGLONG uSize);

    size_t GetSize();

    BOOL GetItemByProcessName(CString& strPorcess, LPTRASH_SKIP_ITEM& lpItem);

    BOOL RemoveItemByProcess(LPCTSTR szProcess);

    void StartCountdown();

    void StopCountdown();

protected:
    CRect m_rcClient;
    CRect m_rcRealClient;
    std::vector<TRASH_SKIP_ITEM> m_vecItems;
    HWND m_hNotifyHwnd;

    HICON m_iconIE;
    HICON m_iconUnknow;
    HBITMAP m_bmpHeadBack;
    Gdiplus::Image* m_imgSoftOff;

    int m_nHoverIndex;
    int m_nSelectIndex;

    int m_nPos;
    int m_nHeight;

    int m_nCountdownNum;

public:

    BEGIN_MSG_MAP_EX(CTrashSkipCtrl)
        MSG_WM_SETCURSOR(OnSetCursor)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_ERASEBKGND(OnEarseBKGND)
        MSG_WM_SIZE(OnSize)
        MSG_WM_TIMER(OnTimer)

        MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
        MESSAGE_HANDLER_EX(WM_MOUSEWHEEL, OnMouseWheel)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

    LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void OnPaint(CDCHandle dc);
    LRESULT OnEarseBKGND(CDCHandle dc);
    void OnMouseMove(UINT nFlags, CPoint point);
    LRESULT OnSetCursor(CWindow wnd, UINT nHitTest, UINT message);
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonUp(UINT nFlags, CPoint point);
    void OnSize(UINT nType, CSize size);

    void OnTimer(UINT_PTR nIDEvent);

protected:
    BOOL _GetTotalSize(ULONGLONG& uTotalSize);

    void _OnRealPaint(CDC& memDC);

    BOOL _GetItemIcon(LPCTSTR lpFileName, HICON& icon);

    void _DrawText(CDC& memDC, CRect& rcItem, LPCTSTR pszText , COLORREF colorText , HFONT hFont);

    void _DrawLine(CDC& memDC, CPoint ptStart, CPoint ptEnd, COLORREF color, DWORD dwPenTyple = PS_SOLID);
    
    void _DrawHeader(CDC& dc, CRect& rcDraw);

    void _DrawSoftItem(CDC& dc, CRect& rcDraw, int nItemIndex);

    void _DrawItemBackgnd(CDC& dc, CRect rcDraw, int nItemIndex);

    void _DrawItemText(CDC& dc, CRect rcDraw, LPCTSTR szText);

    BOOL _PointInClose(CPoint pt);

    void _RandItems();
};
