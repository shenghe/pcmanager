#pragma once

#include "beikecolordef.h"
#include "SoftMgrItemData.h"

#define WM_USER_CLICK_LIST_ITEM		WM_USER + 100
#define WM_USER_CHECK_LIST_ITEM		WM_USER + 101

class ISoftMgrUpdateCallback
{
public:
	virtual void OnUpdateViewDetail(DWORD dwIndex, HWND hWndList) = 0; //查看软件详情
	virtual void OnUpdateViewNew(DWORD dwIndex, HWND hWndList) = 0; //查看软件更新
	virtual void OnUpdateDownLoad(DWORD dwIndex, HWND hWndList) = 0;	//下载
	virtual void OnBtnClick( DWORD dwIndex, int nType ) = 0;
	virtual void OnUpdateContinue(DWORD dwIndex, HWND hWndList) = 0;	//继续
	virtual void OnUpdatePause(DWORD dwIndex, HWND hWndList) = 0;	//暂停
	virtual void OnUpdateCancel(DWORD dwIndex, HWND hWndList) = 0;	//取消
	virtual void OnUpdateDownLoadIcon(CString strID) = 0;
	virtual void OnUpdateFreebackFromList(DWORD dwIndex, HWND hWndList) = 0;
	virtual void OnUpdateTautilogyFromList(DWORD dwIndex, HWND hWndList) = 0;
	virtual void OnUpdateSoftMark(DWORD dwIndex, HWND hWndList) = 0;
	virtual void OnUpdateIgnore(DWORD dwIndex, HWND hWndList) = 0;
};

class CSoftMgrUpdateListView : public CWindowImpl<CSoftMgrUpdateListView, CListViewCtrl>, 
	public COwnerDraw<CSoftMgrUpdateListView>
{
public:
	DECLARE_WND_CLASS(_T("Ksm_Update_ListView_by_Nuk07"))

	CSoftMgrUpdateListView()
	{
		m_rcIgnore = CRect(0,0,0,0);
		m_rcCheck = CRect(10, 20, 10 + 13, 20 + 13);
		m_rcIcon = CRect(30, 11, 62, 43);
		m_rcName = CRect(70, 9, 130, 23);
		m_rcDescription = CRect(70, 33, 130, 45);
		m_rcNewInfo = CRect(132, 33, 182, 45);

		m_rcTautology = CRect(0,0,0,0);
		m_rcFreeback = CRect(0,0,0,0);
		m_rcInstall = CRect(0,0,0,0);
		m_rcWaitDownload = CRect(0,0,0,0);

		m_bDrawItem = FALSE;
		m_bMouseOn = FALSE;
		m_bMouseDown = FALSE;
		m_bMouseOnPause = FALSE;
		m_bMouseOnCancel = FALSE;
		m_bMouseOnName = FALSE;

		m_pHoverTip = 0;

		m_fntNormal.Attach( BkFontPool::GetFont(BKF_DEFAULTFONT));
		m_fntBold.Attach( BkFontPool::GetFont(BKF_BOLDFONT));
		m_fntUnderLine.Attach( BkFontPool::GetFont(BKF_UNDERLINE));
	}

	~CSoftMgrUpdateListView()
	{
		if(m_hWnd != NULL)
			DeleteAllItems();

		m_fntNormal.Detach();
		m_fntBold.Detach();
		m_fntUnderLine.Detach();
	}
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
		MOUSE_LASTON_TAUTOLOGY,
		MOUSE_LASTON_MARK,
	};

public:
	BEGIN_MSG_MAP(CSoftMgrUpdateListView)  
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent) 
		MSG_WM_LBUTTONDBLCLK(OnLButtonDBClk)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MSG_OCM_CTLCOLORLISTBOX(OnCtlColor)
		MSG_WM_SIZE(OnSize)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_SETCURSOR(OnSetCursor)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CSoftMgrUpdateListView>,1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()   

	void OnSize(UINT nType, CSize size);
	void OnSetFocus(HWND hWnd);
	HRESULT	OnMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled );
	BOOL OnEraseBkgnd(CDCHandle dc);
	BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message);
	HRESULT	OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled );
	void OnLButtonDBClk(UINT uMsg, CPoint ptx);
	HRESULT	OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled );
	LRESULT	OnCtlColor(HDC hDc, HWND hWnd);
	HRESULT	OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled );
	HRESULT	OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled );

	LRESULT OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	void SetLoadShow(BOOL bShow=TRUE);
	void CreateTipCtrl();
	void SetShowDlgState();

	void InitNormal();
	void InitDownLoad();

	CSoftListItemData*	GetItemDataEx(DWORD dwIndex);
	int SetItemDataEx(DWORD dwIndex, DWORD_PTR dwItemData);

	BOOL GetItemRect(int nItem, LPRECT lpRect, UINT nCode);
	void SetClickLinkCallback( ISoftMgrUpdateCallback* opCB );
	ISoftMgrUpdateCallback* GetClickLinkCallback();
	VOID SetPaintItem(BOOL bDrawItem);
	void MeasureItem(LPMEASUREITEMSTRUCT lpMes);
	void DrawItem ( LPDRAWITEMSTRUCT lpDrawItemStruct );
	BOOL FindItem(CSoftListItemData *pData);
	void DeleteAllItemsEx(); //清空列表页删除数据
	void ClearAllItemData(); //只清空列表，不删除数据
	void DeleteItem(LPDELETEITEMSTRUCT lParam);
	int GetItemCount();

	void SetCheckedAllItem(BOOL bChecked);
	void RefreshItem(int nIndex);
	void RefreshProgress(int nIndex);
	void RefreshButton(int nIndex);
	void RefreshRight(int nIndex);
	void RefreshIcon(int nIndex);
	void UpdateSoft(int nIndex, HWND hWnd);
protected:
	void _DrawItem(HDC hDC, LPDRAWITEMSTRUCT lpDrawItemStruct);
	
public:
	CRect m_rcIgnore;			//不在提醒
	CRect m_rcCheck;			//选择框
	CRect m_rcIcon;				//图标
	CRect m_rcName;				//软件名称
	CRect m_rcDescription;		//软件描述
	CRect m_rcNewInfo;			//新版功能
	CRect m_rcSize;				//软件大小
	CRect m_rcStar;				//分数星星
	CRect m_rcMark;				//分数
	CRect m_rcStateBtn;			//状态按钮
	CRect m_rcCancel;			//取消按钮
	CRect m_rcContinue;			//继续按钮
	CRect m_rcState;			//状态或者下载速度
	CRect m_rcLocalVer;			//当前版本
	CRect m_rcNewVer;			//更新版本
	CRect m_rcPlug;				//插件
	CRect m_rcCharge;			//收费
	CRect m_rcProgress;			//下载进度条
	CRect m_rcValue;			//进度值
	CRect m_rcLink;				//正在连接服务器
	CRect m_rcFreeback;			//反馈
	CRect m_rcTautology;		//重试
	CRect m_rcInstall;			//安装
	CRect m_rcWaitInstall;		//等待安装
	CRect m_rcWaitDownload;		//等待下载

	BOOL m_bDrawItem;			//是否有需要重绘制的项
	CBrush	m_hBGBrush;			//背景刷
	BOOL m_bMouseOn;			//鼠标是否在状态按钮上
	BOOL m_bMouseDown;			//鼠标是否按下
	BOOL m_bMouseOnPause;		//鼠标在暂停按钮上
	BOOL m_bMouseOnCancel;		//鼠标在取消按钮上
	BOOL m_bMouseOnName;		//鼠标在软件名称上面
	BOOL m_bMouseOnFreeback;
	BOOL m_bMouseOnTautology;

	CFont		m_fntNormal;	// 常规
	CFont		m_fntBold;		// 粗体
	CFont		m_fntUnderLine;		

	CSimpleArray<CSoftListItemData*>	m_arrData;
	ISoftMgrUpdateCallback			*m_linkerCB;
	CToolTipCtrl				 m_wndToolTip;
	//CBkDialogView				 m_dlgMsg;

	DWORD						 m_pHoverTip;
	DWORD						m_pRefresh;
	LASTSTATE					m_enumLastMSState;

	CDC	  m_dcMem;
	CBitmap m_bmpMem;
};