#pragma once
#include "SoftMgrUpdateHostWnd.h"

typedef CWinTraits<WS_BORDER|WS_CHILDWINDOW|WS_VISIBLE|WS_SYSMENU, 0> CRollWndTraits;

//////////////////////////////////////////////////////////////////////////
class CSoftMgrUpdateRollWnd : public CWindowImpl<CSoftMgrUpdateRollWnd, CWindow, CHolderWndTraits>
{
public:
	CSoftMgrUpdateRollWnd();
	virtual ~CSoftMgrUpdateRollWnd();

	DECLARE_WND_CLASS(_T("Ksm_RollWnd_by_Nuk07"))

	BEGIN_MSG_MAP_EX(CSoftMgrUpdateRollWnd)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_VSCROLL(OnVScroll)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_SIZE(OnSize)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
	END_MSG_MAP()

public:
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
	void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar);
	BOOL OnEraseBkgnd(CDCHandle dc);
	void OnSize(UINT nType, CSize size);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	int OnCreate(LPCREATESTRUCT lpCreateStruct);

	void _ReplaceScroll();

public:
	CSoftMgrUpdateHostWnd* m_pHostWnd;
	int m_nScrollPos;
};
