#pragma once

#include <atlcrack.h>
#include <vector>
#include "msgdefine.h"
const COLORREF BACKGROUND_COLOR=RGB(0xfb,0xfc,0xfd);
enum {
	WMH_INIT_SCAN = WM_USER + 0x100,
	WMH_SCAN_START,
	WMH_SCAN_DONE,
	WMH_REPAIR_DONE,
	WMH_SCAN_FIXED_BEGIN,
	WMH_SCAN_FIXED_DONE,
	WMH_SCAN_SOFT_BEGIN,
	WMH_SCAN_SOFT_DONE,
	WMH_VULFIX_BASE = WM_USER + 0x200,

	WMH_LISTEX_HOVERCHANGED = WM_USER + 0x300,
	WMH_LISTEX_LINK
};

class CHeaderCtrlEx 
    : public CWindowImpl<CHeaderCtrlEx, CHeaderCtrl>
    , public CDoubleBufferImpl<CHeaderCtrlEx>
{
public:
	CHeaderCtrlEx();
	virtual ~CHeaderCtrlEx();
	void SetHeight(INT nHeight);
	HWND SetNotifyHwnd(HWND hWnd);
	void DisableHeaderFlag();
    void DoPaint(CDCHandle dc);

protected:
	LRESULT OnHDMLayout(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT	OnMouseMove(UINT nFlags, CPoint point);

	void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonUp(UINT nFlags, CPoint point);
	int GetClickItem(CPoint point);
    int GetClickEdge(CPoint point);

	void OnVScroll(UINT nSBCode, UINT nPos, HWND hWnd)
	{
		 	
	    Invalidate();
		 	
		SetMsgHandled(FALSE);
	}

protected:
	CBitmap m_bitmapHeader;
	CFont m_font;
	INT m_nHeight;
	HWND m_hNotifyHwnd;

	CRBMap<int, BOOL>	m_mapHeaderDrag;

	int m_nFillFlag;
	BOOL m_bUp;
    BOOL m_bDrag;

public:
	BEGIN_MSG_MAP_EX(CHeaderCtrlEx) 
		//MSG_WM_PAINT(OnPaint)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		//MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
       /// MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_VSCROLL(OnVScroll)
		MSG_WM_HSCROLL(OnVScroll)
		MESSAGE_HANDLER(HDM_LAYOUT, OnHDMLayout)
        CHAIN_MSG_MAP(CDoubleBufferImpl<CHeaderCtrlEx>)
	END_MSG_MAP()
};

enum E_SubItemType
{
	SUBITEM_TEXT,
	SUBITEM_LINK,
};

enum E_ListItemType
{
	LISTITEM_TITLE		= 1,
	LISTITEM_CHECKBOX	= 2,
	LISTITEM_RADIOBOX	= 4,
	LISTITEM_EXPANDABLE	= 8,
	LISTITEM_BOLD		= 16,
};

enum E_ListTitleType
{
	LISTTITLE_DEFAULT = 0x1,

	LISTITEM_THIRD_LINE_TOP = 0x2,
	LISTITEM_THIRD_LINE_MID = 0x4,
	LISTITEM_THIRD_LINE_BTM = 0x8,
	
	//æ”÷–
	LISTTILTE_CENTER=0x10,
};

class CListViewCtrlEx : 
	public CWindowImpl<CListViewCtrlEx, CListViewCtrl>, 
	public COwnerDraw<CListViewCtrlEx>
{
	typedef CWindowImpl<CListViewCtrlEx, CListViewCtrl> _super;

public:
	struct TListItem;
	struct TListSubItem;
	typedef std::vector<TListItem*> TListItemPtrs;
	typedef std::vector<TListSubItem> TListSubItems;
	struct TListSubItem
	{
		TListSubItem() : type(SUBITEM_TEXT)
		{
			nMarginLeft = 2;
			clr = RGB(0,0,0);
			rcOffset = CRect(0,0,0,0);
		}

		TListSubItem(LPCTSTR szText, CRect rc, E_SubItemType aType=SUBITEM_TEXT)
			: str(szText), type(aType), rcOffset(rc)
		{
		}

		TListSubItem(LPCTSTR szText, E_SubItemType aType=SUBITEM_TEXT)
			: str(szText), type(aType)
		{
			rcOffset = CRect(0,0,0,0);
			nMarginLeft = 2;
			clr = RGB(0,0,0);
			ATLASSERT(szText);
		}
		E_SubItemType type;
		CString str;
		COLORREF clr;
		int	nMarginLeft;
		CRect	rcOffset;
		CString	strUrl;
	};
	
	struct TListItem
	{
	public:
		TListItem()
		{
			dwFlags		=0;
			clrBg		=BACKGROUND_COLOR;
			clrBtmGapLine=RGB(234,233,225);
			nLeftmargin	=10;
			nTopMargin	=-1;
			bBold		=FALSE;
			nHeightAdd	=0;

			_isclapsed = FALSE;
		}
		~TListItem()
		{
			for(TListItemPtrs::iterator it=_clapsed_items.begin(); it!=_clapsed_items.end(); ++it)
				delete *it;
			_clapsed_items.clear();
		}
		
		int			nLeftmargin;
		int			nTopMargin;
		BOOL		bBold;
		int			nHeightAdd;
		DWORD		dwFlags;
		COLORREF	clrBg;
		COLORREF	clrBtmGapLine;
		TListSubItems subItems;
		
		BOOL _isclapsed;
		TListItemPtrs _clapsed_items;
		DWORD_PTR _itemData;
	};

public:
	CListViewCtrlEx(void);
	~CListViewCtrlEx(void);
	CHeaderCtrlEx m_ctlHeader;
	
protected:
	CFont m_fontLink, m_fontBold, m_fontTitle, m_fontDef;
	CBitmap m_bitmapCheck, m_bitmapExpand, m_bitmapRadio;
	CBitmap m_bitmapRadioMask;
	int m_nHoverItem;
	
	TListItemPtrs m_arrItems;
	HWND m_hWndObserver;

	CString m_strEmptyString;
	
public:
	HWND Create(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL);
	BOOL GetCheckState(int nIndex, BOOL *pIsRadio=NULL) const;
	BOOL SetCheckState(int nItem, BOOL bCheck);
	int SetRadioState(int nItem, BOOL bCheck);	// -1: not valid , 0 : not checked, 1: checked 
	
	void ClearRadioCheck();
	void CleanCheck( int nItem=-1 );
	void CheckAll( );
	template<typename Func> void CheckAll(Func fn)
	{
		ClearRadioCheck();
		for(int i=0; i<_super::GetItemCount(); ++i)
		{
			T_VulListItemData *pItemData = (T_VulListItemData*)_super::GetItemData(i);
			TListItem *pItem = _GetItemData( i );
			if( pItem && (pItem->dwFlags&LISTITEM_CHECKBOX) )
			{
				_super::SetCheckState( i, fn(pItemData) );
			}
		}
	}
	void SetObserverWindow(HWND hWnd);
	
protected:
	DWORD SetExtendedListViewStyle(DWORD dwExStyle, DWORD dwExMask = 0)
	{
		ATLASSERT(FALSE);
		return _super::SetExtendedListViewStyle(dwExStyle, dwExMask);
	}
	int AddItem(int nItem, int nSubItem, LPCTSTR strItem, int nImageIndex = -1)
	{
		ATLASSERT(FALSE);
		return _super::AddItem(nItem, nSubItem, strItem, nImageIndex);
	}
	BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
	{
		ATLASSERT(FALSE);
		return _super::SetItemText(nItem, nSubItem, lpszText);
	}
	
public:
	void Reset();

	int AppendTitle(LPCTSTR strItem, COLORREF clr, UINT uFlags=0);	
	int AppendTitleItem(int nItem, LPCTSTR strItem, CRect rc, E_SubItemType itemType, COLORREF clr, LPCTSTR szURL);
	int Append(LPCTSTR strItem, DWORD dwFlags=0, E_SubItemType itemType=SUBITEM_TEXT);	
	int AppendSubItem(int nItem, LPCTSTR strItem, E_SubItemType itemType=SUBITEM_TEXT);	
	bool SetSubItem(int nItem, int nSubItem, LPCTSTR lpszItem, E_SubItemType itemType=SUBITEM_TEXT, BOOL bRedraw=TRUE);
	bool GetSubItemText(int nItem, int nSubItem, CString &str);
	bool SetSubItemColor(int nItem, int nSubItem, COLORREF clr, BOOL bRedraw=TRUE);
	bool SetItemCheckX(int nItem, DWORD dwFlags);
	BOOL DeleteItem(int nItem);	
	BOOL DeleteAllItems();
	void SetEmptyString(LPCTSTR szEmptyString);
	void ExpandGroup(INT nItem, BOOL bExpand);
	
public:
	BEGIN_MSG_MAP(CListViewCtrlEx)   
		MSG_WM_PAINT(OnPaint)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_SETCURSOR(OnSetCursor)
		NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
		NOTIFY_CODE_HANDLER(HDN_DIVIDERDBLCLICK , OnTrackDbClickHeader)
		NOTIFY_CODE_HANDLER(HDN_TRACK , OnTrackMoveHeader)
		NOTIFY_CODE_HANDLER(HDN_ENDTRACK , OnHeaderEndTrack)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CListViewCtrlEx>, 1)	
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	void OnPaint(CDCHandle dc);
	void OnMouseMove(UINT nFlags, CPoint point);
	BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message);
	LRESULT OnClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		
	LRESULT OnTrackDbClickHeader(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);	
	LRESULT OnTrackMoveHeader(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);	
	LRESULT OnHeaderEndTrack(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	
	void MeasureItem(LPMEASUREITEMSTRUCT lpMes);
	void DrawItem ( LPDRAWITEMSTRUCT lpdis );
	VOID DeleteItem(LPDELETEITEMSTRUCT );

public:
	const TListItem *_GetItemData(int nItem) const;
	TListItem *_GetItemData(int nItem);
	const TListSubItem *_GetSubItemData(int nItem, int nSubItem);

protected:
	void _DrawTitleItem( LPDRAWITEMSTRUCT lpdis, const TListItem *pItem );
	void _DrawNormalItem( LPDRAWITEMSTRUCT lpdis, const TListItem *pItem );
	BOOL _PtInSubItemCheckBox( const POINT& pt, int nItem );
	BOOL _GetSubItemLinkRect( int nItem, int nSubItem, LPCTSTR szText, RECT &rc );
	bool _PtInSubItemLink( const POINT &pt, int nItem, int & nSubItem );
	int _DrawCheckBox( CDCHandle &dc, RECT &rcItem, BOOL bChecked, DWORD dwFlags );
	RECT _GetRectCheckBox( RECT &rcItem );
	RECT _GetRectMinus( RECT &rcItem );
	void _InvalidateRect( int nItem, int nSubItem );
	BOOL _FireEvent(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

	void _SetColumnNeedWidth(int i, DWORD nWidth);
	void _RedrawTitle();
	BOOL _ExpandItem( TListItem * pItem, INT iItem, BOOL expand );
protected:
	CRBMap<int,DWORD>	m_columnMaxWidth;
};
