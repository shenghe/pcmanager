#pragma once

/*
* @file    softmgrRubbishListView.h
* @brief   软件管理--下载管理--垃圾箱
*/


#include "beikecolordef.h"
#include "SoftMgrItemData.h"

class ISoftMgrRubbishCallback
{
public:
	virtual void OnResume(CString strId) = 0;	//还原
	virtual void OnLbuttonUpNotifyEx(DWORD dwIndex) = 0;
};

class CSoftMgrRubbishListView : public CWindowImpl<CSoftMgrRubbishListView, CListViewCtrl>, 
	public COwnerDraw<CSoftMgrRubbishListView>
{
public:
	CSoftMgrRubbishListView();
	~CSoftMgrRubbishListView();

protected:
	enum LASTSTATE
	{
		MOUSE_LASTON_NONE,
		MOUSE_LASTON_STATEBTN,
		MOUSE_LASTON_NAME,
		MOUSE_LASTON_CANCELBTN,
		MOUSE_LASTON_CONTINUEBTN,
		MOUSE_LASTON_SOFTICON,
		MOUSE_LASTON_FREEBACK,
		MOUSE_LASTON_TAUTOLOGY
	};

public:
	BEGIN_MSG_MAP(CSoftMgrRubbishListView)  
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MSG_OCM_CTLCOLORLISTBOX(OnCtlColor)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CSoftMgrRubbishListView>,1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()   

	CSimpleArray<CSoftListItemData*>& GetItemArray();
	void UpdateAll();

	HRESULT	OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled );

	void InitNormal();

	LRESULT	OnCtlColor(HDC hDc, HWND hWnd);

	CSoftListItemData*	GetItemDataEx(DWORD dwIndex);

	int SetItemDataEx(DWORD dwIndex, DWORD_PTR dwItemData);

	BOOL GetItemRect(int nItem, LPRECT lpRect, UINT nCode);

	HRESULT	OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled );

	HRESULT	OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled );

	void SetClickLinkCallback( ISoftMgrRubbishCallback* opCB );

	VOID SetPaintItem(BOOL bDrawItem);

	void MeasureItem(LPMEASUREITEMSTRUCT lpMes);

	HRESULT	OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled );

	void _DrawItem(HDC hDC, LPDRAWITEMSTRUCT lpDrawItemStruct);

	void DrawItem ( LPDRAWITEMSTRUCT lpDrawItemStruct );

	void SetCheckedAllItem(BOOL bChecked);

	void RefreshItemCheckedState(int nIndex);

	void RefreshItem(int nIndex);

	void DeleteAllItemsEx(); //清空列表页删除数据

	void ClearAllItemData(); //只清空列表，不删除数据

	void DeleteItem(LPDELETEITEMSTRUCT lParam);

	void ReQueueList(int nIndex);

	int GetItemCount();

private:
	CRect m_rcCheck;			//选择框
	CRect m_rcIcon;				//图标
	CRect m_rcName;				//软件名称
	CRect m_rcDescription;		//软件描述
	CRect m_rcSize;				//软件大小
	CRect m_rcStateBtn;			//状态按钮
	CRect m_rcState;			//安装

	BOOL m_bDrawItem;			//是否有需要重绘制的项
	CBrush	m_hBGBrush;			//背景刷
	BOOL m_bMouseOn;			//鼠标是否在状态按钮上
	BOOL m_bMouseDown;			//鼠标是否按下
	BOOL m_bMouseOnName;		//鼠标在软件名称上面

	CFont		m_fntNameOn;	// 软件名称字体(鼠标在上面）
	CFont		m_fntNameNormal;// 软件名称字体（正常情况下）
	CFont		m_fntDefault;	// 一般字体

	CSimpleArray<CSoftListItemData*>	m_arrData;
	ISoftMgrRubbishCallback			*m_linkerCB;

	LASTSTATE					m_enumLastMSState;
	CDC	  m_dcMem;
	CBitmap m_bmpMem;
};
