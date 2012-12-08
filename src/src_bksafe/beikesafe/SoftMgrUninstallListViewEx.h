#pragma once

#include "SoftMgrItemData.h"


class ISoftMgrUnistallCallbackEx
{
public:
	virtual void OnUninstallEx(int nListId, DWORD dwIndex) = 0;
	virtual void OnViewUninstallDetailEx(int nListId, DWORD dwIndex) = 0;
	virtual void OnDownloadIconFromUninstallEx(int nListId, CString strID) = 0;
	virtual void OnClearShortCutMenu(int nListId, CUninstallSoftInfo *pData) = 0;
	virtual void OnViewRudimentalInfo(int nListId, CUninstallSoftInfo *pData) = 0;
	virtual void OnStartProtection( int nListId, CUninstallSoftInfo *pData ) = 0;
};

class CSoftMgrUninstallListViewEx : public CWindowImpl<CSoftMgrUninstallListViewEx, CListViewCtrl>, 
	public COwnerDraw<CSoftMgrUninstallListViewEx>
{
public:
	CSoftMgrUninstallListViewEx();

	~CSoftMgrUninstallListViewEx();

public:
	enum  //list风格
	{
		LIST_STYLE_NORMAL,		//一般
		LIST_STYLE_DESKTOP,		//桌面
		LIST_STYLE_STARTMENU	//开始菜单
	};

protected:
	enum LASTSTATE
	{
		MOUSE_LASTON_NONE,
		MOUSE_LASTON_STATEBTN,
		MOUSE_LASTON_NAME,
		MOUSE_LASTON_SOFTICON,
		MOUSE_LASTON_CLEARICON,
		MOUSE_LASTON_VIEWDETAIL,
	};

public:
	BEGIN_MSG_MAP(CSoftMgrUninstallListViewEx)  
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent) 
		MSG_WM_LBUTTONDBLCLK(OnLButtonDBClk)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_OCM_CTLCOLORLISTBOX(OnCtlColor)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CSoftMgrUninstallListViewEx>,1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP() 

public:
	void OnSetFocus(HWND hWnd);

	LRESULT OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void CreateTipCtrl();

	HRESULT	OnMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled );

	HRESULT	OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled );

	void InitNormal();

	LRESULT	OnCtlColor(HDC hDc, HWND hWnd);

	CUninstallSoftInfo*	GetItemDataEx(DWORD dwIndex);

	int SetItemDataEx(DWORD dwIndex, DWORD_PTR dwItemData);

	BOOL GetItemRect(int nItem, LPRECT lpRect, UINT nCode);

	HRESULT	OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled );

	HRESULT	OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled );

	void SetClickLinkCallback( ISoftMgrUnistallCallbackEx* opCB );

	VOID SetPaintItem(BOOL bDrawItem);

	void MeasureItem(LPMEASUREITEMSTRUCT lpMes);

	HRESULT	OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled );

	void _DrawItem(HDC hDC, LPDRAWITEMSTRUCT lpDrawItemStruct);

	void OnLButtonDBClk(UINT uMsg, CPoint ptx);

	void DrawItem ( LPDRAWITEMSTRUCT lpDrawItemStruct );

	void RefreshItem(int nIndex);

	void RefreshIcon(int nIndex);

	void DeleteAllItemsEx();

	void DeleteItem(LPDELETEITEMSTRUCT lParam);

	int GetItemCount();

	//Must be set after init immediately.
//	BOOL SetShowClearDesktopIcon(BOOL bShow = FALSE);
	void SetListViewID(int nListId);
	void SetListStyle(BOOL bRudimental);//是否残留类型
	void SetListType(int nType = LIST_STYLE_NORMAL);

public:
	BOOL		m_bHinting;

private:
	CRect m_rcIcon;				//图标
	CRect m_rcName;				//软件名称
//	CRect m_rcDescription;		//软件描述
	CRect m_rcFrequency;		//使用频率
	CRect m_rcSize;				//软件大小
	CRect m_rcStateBtn;			//状态按钮
	CRect m_rcUninsatll;
	CRect m_rcWaitUninstall;
	CRect m_rcViewDetail;		//查看软件详情
	CRect m_rcDeleteDesktopIcon;//删除桌面图标
	CRect m_rcRudimental;		//残留
	CRect m_rcPath;				//路径

	BOOL m_bDrawItem;			//是否有需要重绘制的项
	BOOL m_bMouseOn;			//鼠标是否在状态按钮上
	BOOL m_bMouseDown;			//鼠标是否按下
	BOOL m_bMouseOnName;		//鼠标在软件名称上面
	BOOL m_bUninstalling;		//是否正在删除
//	BOOL m_bShowClearShortCut;	//是否显示删除桌面图标

	

	BOOL m_bStyleRudiment;		//是否为残留类型

	CFont		m_fntNameOn;	// 软件名称字体(鼠标在上面）
	CFont		m_fntNameNormal;// 软件名称字体（正常情况下）
	CFont		m_fntPlug;		// 插件字体
	CFont		m_fntDefault;	// 一般字体
	CFont		m_fntDanger;	// 特殊提示的字体

	CBrush	m_hBGBrush;			//背景刷

	int m_nListID;

	CSimpleArray<CUninstallSoftInfo*>	m_arrData;
	ISoftMgrUnistallCallbackEx			*m_linkerCB;
	CToolTipCtrl				 m_wndToolTip;

	DWORD						 m_pHoverTip;
	DWORD						m_pRefresh;
	LASTSTATE					m_enumLastMSState;

	int		m_nListStyle;

	CDC	  m_dcMem;
	CBitmap m_bmpMem;
};
