//////////////////////////////////////////////////////////////////////////
//
//	
//
//

#pragma once
#include <atlscrl.h>
#include <atlctrls.h>
typedef struct tagComLVItemBase
{
	int nHeight;

}ComLVItemBase;

interface IListParent
{
	virtual UINT DrawItem( CDCHandle& dc, UINT uItemIndex, CRect& rtItem, BOOL bSelect ) = 0;
	virtual UINT MeasureItem( UINT uItemIndex ) = 0;
	virtual BOOL ItemHitTest( UINT uItemIndex, int xOffset, int yOffset, UINT uMsg ) = 0;
	virtual LPCTSTR GetTipText( UINT uItemIndex, int xOffset, int yOffset ) = 0;
};

class  CListViewMgr : public CScrollWindowImpl< CListViewMgr >
{
public:

	CListViewMgr();
	~CListViewMgr();


	BEGIN_MSG_MAP(CListViewMgr)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent)
		MESSAGE_HANDLER(WM_CREATE,				OnCreate)
		MESSAGE_HANDLER(WM_DESTROY,				OnDestroy)
		MESSAGE_HANDLER(WM_LBUTTONDOWN,			OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK,		OnLButtonDown)
		MESSAGE_HANDLER(WM_RBUTTONUP,			OnRButtonUp)
		MESSAGE_HANDLER(WM_ERASEBKGND,			OnEraseBKGnd)
		MESSAGE_HANDLER(WM_SETCURSOR,			OnSetCursor)
		MESSAGE_HANDLER(WM_MOUSEMOVE,			OnMouseMove)
		MESSAGE_HANDLER(WM_SIZE,				OnSize)
		CHAIN_MSG_MAP(CScrollWindowImpl< CListViewMgr >)
	END_MSG_MAP()

	LRESULT OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBKGnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnVScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

	static LPCTSTR GetWndClassName()
	{
		return _T("LIST_VIEW_MGR");
	}

	void SetCallBack( IListParent* );
	void SetItemCount( UINT nCnt, BOOL bReDraw = TRUE );
	void DoPaint( CDCHandle dc );
	void DoPaintGnd( CDCHandle dc );
	UINT GetItemCount( );
	UINT _HitTest( int xPos, int yPos, CRect& rtItemOut );
	void ReScrollSize( BOOL bReDraw = TRUE );

private:

	IListParent*					m_pListParent;
	UINT							m_uItemCount;
	UINT							m_uItemSelect;
	CToolTipCtrl					m_wndToolTip;


protected:

	void CreateToolTip();
};
