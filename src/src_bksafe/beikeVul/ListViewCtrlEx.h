#pragma once
#include "BeikeVulfix.h"

enum E_SubItemType
{
	SUBITEM_TEXT,
	SUBITEM_LINK,
};

class CListViewCtrlEx : 
	public CWindowImpl<CListViewCtrlEx, CListViewCtrl>, 
	public COwnerDraw<CListViewCtrlEx>
{
	typedef CWindowImpl<CListViewCtrlEx, CListViewCtrl> _super;

protected:
	struct TListSubItem
	{
		TListSubItem() : type(SUBITEM_TEXT)
		{
			clr = RGB(0,0,0);
		}

		TListSubItem(LPCTSTR szText, E_SubItemType aType=SUBITEM_TEXT)
			: str(szText), type(aType)
		{
			clr = RGB(0,0,0);
			ATLASSERT(szText);
		}
		E_SubItemType type;
		CString str;
		COLORREF clr;
	};

	struct TListItem
	{
		bool checkbox;
		CSimpleArray<TListSubItem>	subItems;
	};

public:
	CListViewCtrlEx(void)
	{
		m_hWndObserver = NULL;
		m_nHoverItem = -1;
		m_fontLink.CreateFont(12,0,0,0,FW_NORMAL,0,1,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY ,DEFAULT_PITCH, _T("ËÎÌו"));
		m_bitmapCheck.LoadBitmap( IDB_BITMAP_LISTCTRL_CHECK );
	}
	~CListViewCtrlEx(void)
	{
		m_fontLink.DeleteObject();
		m_bitmapCheck.DeleteObject();
	}
	
protected:
	CFont m_fontLink;
	CBitmap m_bitmapCheck;
	int m_nHoverItem;

	CSimpleArray<TListItem> m_arrItems;
	HWND m_hWndObserver;
	
public:
	HWND Create(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL);
	BOOL GetCheckState(int nIndex) const;
	BOOL SetCheckState(int nItem, BOOL bCheck);
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
	int Append(LPCTSTR strItem, bool bCheckBox=false, E_SubItemType itemType=SUBITEM_TEXT);	
	int AppendSubItem(int nItem, LPCTSTR strItem, E_SubItemType itemType=SUBITEM_TEXT);	
	bool SetSubItem(int nItem, int nSubItem, LPCTSTR lpszItem, E_SubItemType itemType=SUBITEM_TEXT, BOOL bRedraw=TRUE);
	bool SetSubItemColor(int nItem, int nSubItem, COLORREF clr, BOOL bRedraw=TRUE);
	bool SetItemCheck(int nItem, bool bUseCheck);
	BOOL DeleteItem(int nItem);	
	BOOL DeleteAllItems();

public:
	BEGIN_MSG_MAP(CListViewCtrlEx)   
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CListViewCtrlEx>, 1)	
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	LRESULT OnClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);	
	LRESULT OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled );		
	void MeasureItem22(LPMEASUREITEMSTRUCT lpMes)
	{
		lpMes->itemHeight = 15*5;
	}
	
	void DrawItem ( LPDRAWITEMSTRUCT lpdis );
	VOID DeleteItem(LPDELETEITEMSTRUCT );

protected:
	const TListItem *_GetItemData(int nItem) const;
	TListItem *_GetItemData(int nItem);
	const TListSubItem *_GetSubItemData(int nItem, int nSubItem);
	BOOL _PtInSubItemCheckBox( const POINT& pt, int nItem );
	BOOL _GetSubItemLinkRect( int nItem, int nSubItem, LPCTSTR szText, RECT &rc );
	bool _PtInSubItemLink( const POINT &pt, int nItem, int nSubItem );
	int _DrawCheckBox( CDCHandle &dc, RECT &rcItem, BOOL bChecked );
	RECT _GetRectCheckBox( RECT &rcItem );
	void _InvalidateRect( int nItem, int nSubItem );

	BOOL _FireEvent(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
};
