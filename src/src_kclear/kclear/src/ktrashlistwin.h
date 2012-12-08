//////////////////////////////////////////////////////////////////////////
// Create Author: zhang zexin
// Create Date: 2010-12-16
// Description: 第二版本垃圾清理主界面中的列表
// Modify Record:
//////////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////////
#include <gdiplus.h>
//////////////////////////////////////////////////////////////////////////

#define DEF_TRASH_STATUS_TEXT_CLR		RGB(207, 0, 0)
#define DEF_TRASH_DETAIL_TEXT_CLR		RGB(103, 103, 103)
#define DEF_TRASH_DEFAULT_TEXT_CLR		RGB(0, 0, 0)

#define DEF_IMG_TRASH_CHECK_BOX				"IDB_TRASH_CHECK_BOX"
#define DEF_IMG_TRASH_SHADOW				"IDB_TRASH_SHADOW"
#define DEF_IMG_TRASH_DETAIL				"IDB_SHOW_DETAIL"
#define DEF_IMG_DETAIL_BAR                  "IDP_SHOW_DETAIL_BAR"

#define DEF_DETAIL_BAR_TO_RIGHT				96	// 展示详细内容的按钮，左边到右侧窗口的距离
#define DEF_LEFT_TO_ICON					20  // 大项到左侧距离
#define DEF_LEFT_TO_DETAIL					60  // 小项到左侧的距离
#define DEF_TOP_TO_FIRST					12  // 顶到第一项图标的距离
#define DEF_BETWEEN_ITEM					16  // 大项不展开时的间距
#define DEF_ITEM_THIN_HEIGHT				41  // 大项展开的每条的高度
#define DEF_ITEM_FAT_HEIGHT                 56  // 大项收缩后每条的高度
#define DEF_BETWEEN_DETAIL_ITEM				4	// 小项间距
#define DEF_DETAIL_ITEM_HEIGHT				13  // 小项每条的高度
#define DEF_DETAIL_BETWEEN_HEIGHT           8
#define DEF_BETWEEN_SHADOW					1   // 大项展开时，上端阴影到大项条目的距离
#define DEF_CHECK_TO_TEXT					9

#define DEF_ICON_HEIGHT                     32
//////////////////////////////////////////////////////////////////////////


enum ItemStates{
	Empty_Info,
	Scaning,
	NotChecked,
	Clearing,
	FinishScan,
    FinishClear,
	WaitScan,
	WaitClear,
	SkipRunning
};

typedef struct _TRASH_DETAIL_ITEM
{
	UINT nItemId;
	UINT nFileNum;
	ULONGLONG uTotalSize;
	CString strName;
	BOOL bChecked;
    BOOL bHover;
	CRect rcCheck;
    CRect rcSelected;
    CRect rcDetail;
    BOOL bInDetail;
	UINT nStatus;
	_TRASH_DETAIL_ITEM()
	{
		uTotalSize = 0;
		nFileNum = 0;
		bChecked = FALSE;
        bHover = FALSE;
		nStatus = Empty_Info;
        bInDetail = FALSE;
	}
}TrashDetailItem, *LPTrashDetailItem;

typedef struct _TRASH_ITEM
{
	UINT uItemId;
	BOOL bShow;
	BOOL bShowDetail;
	BOOL bStatus;
	CString strItemName;
	CString strItemDes;
	CString strStatus;
    CString strBrief;
//	HBITMAP bmpItem;
	CRect rcBox;
	CRect rcText;
	CRect rcIcon;
	UINT nStatus;
	int nIconID;
	std::vector<TrashDetailItem> vecItems;
	_TRASH_ITEM()
	{
		bShow = TRUE;
		bStatus = FALSE;
		bShowDetail = FALSE;
		nStatus = Empty_Info;
	}
}TRASH_ITEM, *LPTRASH_ITEM;

//////////////////////////////////////////////////////////////////////////
class CTrashCleanCtrl : public CWindowImpl<CTrashCleanCtrl>
{
public:
	DECLARE_WND_CLASS(L"TrashCleanCtrl");

	CTrashCleanCtrl();
	virtual ~CTrashCleanCtrl();

public:

	void Initialize(HWND hWnd);

	BOOL SubclassWindow(HWND hWnd);

public:

	BOOL AddItem(TRASH_ITEM& item);

	void SetCheckAll(BOOL bChecked);
	
	void SetItemChecked(UINT nItemID, BOOL bChecked);

	void SetStatus(UINT nItemID = -1, UINT nStatus = Empty_Info);
	//
	void SetItemEnable(BOOL bEnbale = TRUE);

	void SetItemStatusInfo(UINT nItemID, UINT nCount, UINT nFileCount, ULONGLONG nSize);

	void GetSelectedItems(std::vector<UINT>& selectItems);

	void GetItemName(UINT nItemID, CString& strName);

	void SetItemShowDetail(UINT nItemID, BOOL bShow = TRUE);

	void SetItemShow(UINT nItemId, BOOL bShow = TRUE);

    BOOL GetItemShow(UINT nItemId);

	BOOL CheckItemAllSelected();

	void ClearAllStatus();

    BOOL GetDetailItemData(UINT nItemID, ULONGLONG& uSize, int& nCount);

    BOOL SetSelectedItem(UINT nItemID);

    // 返回 id
    UINT GetItemParent(UINT nItemID);

public:

	BEGIN_MSG_MAP_EX(CTrashCleanCtrl)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER_EX(WM_MOUSEWHEEL, OnMouseWheel)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_ERASEBKGND(OnEarseBKGND)
		MSG_WM_SIZE(OnSize)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
        DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnPaint(CDCHandle dc);
	BOOL OnEarseBKGND(CDCHandle dc);
	void OnSize(UINT nType, CSize size);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnMouseLeave();

protected:

	void _OnRealPaint(CDC& dc);

	void _DrawItem(CDC& dc, CRect& rcItem, LPTRASH_ITEM lpItem);
	void _DrawDetailItem(CDC& dc, CRect& rcItem, LPTrashDetailItem lpDetailItem);

    void _DrawItemBKGround(CDC& dc, CRect& rcItem, LPTRASH_ITEM lpItem);
    void _DrawTextInfor(CDC& dc, CRect& rcItem, LPTRASH_ITEM lpItem);
	void _DrawItemIcon(CDC& dc, CRect& rcItem, LPTRASH_ITEM lpItem);
	void _DrawCheckBox(CDC& dc, RECT& rcItem, CRect& rcBox, LPTrashDetailItem lpDetailItem);
	void _DrawText(CDC& memDC, CRect& rcItem, LPCTSTR pszText , COLORREF colorText , HFONT hFont);
	void _DrawTextRight(CDC& memDC, CRect& rcItem, LPCTSTR pszText , COLORREF colorText , HFONT hFont);
	void _DrawShadowLine(CDC& dc, CRect& rcItem);
	void _DrawShowDetailBar(CDC& dc, CRect& rcItem, CRect& rcBox, LPTRASH_ITEM lpItem);
    void _DrawLine(CDC& memDC, CPoint ptStart, CPoint ptEnd, COLORREF color, DWORD dwPenTyple = PS_SOLID);

    void _DrawSelectedBox(CDC& dc, CRect& rcItem, LPTrashDetailItem lpDetailItem);
    void _DrawHoverBox(CDC& dc, CRect& rcItem, LPTrashDetailItem lpDetailItem);

    void _DrawCheckDetail(CDC& dc, CRect& rcDraw, LPTrashDetailItem lpDetailItem);

	void _DrawDetailItemStatus(CDC& dc,CRect& rcDraw,LPTrashDetailItem lpDetailItem);
	void _DrawItemStatus(CDC& dc, CRect& rcDraw, LPTRASH_ITEM lpItem);
	
	BOOL _CheckInCheckBox(CPoint& point, LPTRASH_ITEM lpItem);
	BOOL _CheckItemAll(LPTRASH_ITEM lpItem);
	BOOL _CheckChildItemAllFinish(LPTRASH_ITEM lpItem);
	BOOL _CheckChildNoneSelected(LPTRASH_ITEM lpItem);
    BOOL _CheckInCheckDetail(CPoint& point, LPTRASH_ITEM lpItem);
    BOOL _CheckInSelectArea(CPoint& point, LPTRASH_ITEM lpItem);

	void _GetItemStatusString(UINT nItemID, CString& strStatus, int counter, UINT totalCount, ULONGLONG totalSize);
	void _RangeItems();

protected:

	std::vector<TRASH_ITEM> m_vecTrashItems;
	CBitmap m_bmpShadow;
	CBitmap m_bmpCheck;
	CBitmap m_bmpDetail;
	CRect m_rcClient;
	CRect m_rcRealClient;
	int m_nTotalHeight;
	int m_nPos;
	int m_nHeight;

	CImageList		m_iconList;
	CToolTipCtrl m_wndToolTip;
	CString m_strTip;
	int counter;
	HWND m_hNotifyHwnd;

    UINT m_nHoverIndex;
    UINT m_nSelectedIndex;

    BOOL m_bToolTipActive;
    Gdiplus::Image* m_hImageDetailBar;

    BOOL m_bEnbale;
};