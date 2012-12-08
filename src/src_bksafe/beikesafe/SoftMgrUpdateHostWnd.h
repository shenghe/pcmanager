// MainDlg.h : interface of the CSoftMgrUpdateHostWnd class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "SoftMgrUpdateListView.h"

#define ITEM_HEIGHT			55
#define ITEMS_TITLE_HEIGHT	27

#define WM_USER_SET_CHECK_LIST		WM_USER + 102
#define WM_USER_GET_CHECK_LIST		WM_USER + 103

typedef CWinTraits<WS_CHILDWINDOW|WS_VISIBLE|WS_SYSMENU, 0> CHolderWndTraits;

class CSoftMgrUpdateHostWnd : public CWindowImpl<CSoftMgrUpdateHostWnd, CWindow, CHolderWndTraits>
{
public:
	CSoftMgrUpdateHostWnd();
	virtual ~CSoftMgrUpdateHostWnd();

	DECLARE_WND_CLASS(_T("Ksm_HolderWnd_by_Nuk07"))

	BEGIN_MSG_MAP_EX(CSoftMgrUpdateHostWnd)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_SETFOCUS(OnSetFocus)
		MESSAGE_HANDLER(WM_USER_CLICK_LIST_ITEM, OnClickListItemNotify)
		MESSAGE_HANDLER(WM_USER_CHECK_LIST_ITEM, OnCheckListItemNotify)
		MESSAGE_HANDLER(WM_USER_SET_CHECK_LIST, OnSetCheckList)
		MESSAGE_HANDLER(WM_USER_GET_CHECK_LIST, OnGetCheckList)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

public:
	int GetWindowMaxHeight();
	void SetCheckNotifyHwnd(HWND hWnd);
	void SetClickLinkCallback( ISoftMgrUpdateCallback* opCB );
	void AddItem(DWORD_PTR dwItemData);
	void UpdateCtrlWindow();
	CSoftListItemData* GetItemDataEx(DWORD dwIndex, HWND hWndList);
	void RefreshRight(int nIndex, HWND hWndList);
	void RefreshRight(LPCTSTR strSoftID);
	void RefreshIcon(LPCTSTR strSoftID);
	void UpdateSelectSoft();
	void DeleteAllItems();
	int GetCheck();
	int GetUpdateCount(int& nRecommend, int& nChoice);
	void UpdateSoft(LPCTSTR strSoftID);

protected:
	void OnPaint(CDCHandle dc);
	void OnClose();
	void OnDestroy();
	BOOL OnEraseBkgnd(CDCHandle dc);
	void OnSize(UINT nType, CSize size);
	void OnSetFocus(HWND hWnd);
	LRESULT OnClickListItemNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCheckListItemNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCheckList(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetCheckList(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	int OnCreate(LPCREATESTRUCT lpCreateStruct);

	void _InitCtrl();
	int _GetIgnoreLstCheck();
	int _GetFreeLstCheck();
	BOOL	IsKsafe( CSoftListItemData* pData );

private:
	CSoftMgrUpdateListView m_lstIgnore;
	CSoftMgrUpdateListView m_lstFree;

	HWND m_hWndCheckNotify;
	HFONT m_hFont;
};
