#ifndef _SOFT_MGR_LISTVIEW_H_
#define _SOFT_MGR_LISTVIEW_H_

#include "beikecolordef.h"
#include "SoftMgrItemData.h"

enum E_SoftMgrListItemType
{
	SOFTMGR_LISTITEM_TITLE		= 1,
};

#define PAGE_ITEM_MAX_COUNT 50
#define PAGE_ITEM_WIDTH		25
#define PAGE_ITEM2_WIDTH	50
#define PAGE_ITEM_SPACE		4

#define BTN_COLUMN_WIDTH	120
#define TIME_COLUMN_WIDTH	100
#define SIZE_COLUMN_WIDTH	100

class ISoftMgrCallback
{
public:
	virtual void OnViewDetail(DWORD dwIndex) = 0; //查看软件详情
	virtual void OnViewNew(DWORD dwIndex) = 0; //查看软件更新
	virtual void OnDownLoad(DWORD dwIndex) = 0;	//下载
	virtual void OnContinue(DWORD dwIndex) = 0;	//继续
	virtual void OnPause(DWORD dwindex) = 0;	//暂停
	virtual void OnCancel(DWORD dwIndex) = 0;	//取消
	virtual void OnDownLoadIcon(CString strID) = 0;
	virtual void OnFreebackFromList(DWORD dwIndex) = 0;
	virtual void OnTautilogyFromList(DWORD dwIndex) = 0;
	virtual void OnSwitchPage(DWORD dwPage) = 0;
	virtual void OnDaquanSoftMark(DWORD dwIndex) = 0;
};

class CSoftMgrListView : public CWindowImpl<CSoftMgrListView, CListViewCtrl>, 
	public COwnerDraw<CSoftMgrListView>
{
public:
	CSoftMgrListView()
	{
		m_bShowType = FALSE;
		//m_rcCheck = CRect(10, 20, 23, 33);
		//m_rcIcon = CRect(28, 11, 60, 43);
		//m_rcName = CRect(68, 9, 128, 23);
		m_rcCheck = CRect(0,0,0,0);
		m_rcIcon = CRect(10, 11, 42, 43);
		m_rcName = CRect(50, 9, 110, 23);
		m_rcDescription = CRect(50, 33, 110, 45);

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
		m_nCurPage = 0;
		m_nItemCount = 0;

		m_fntNameOn.Attach( BkFontPool::GetFont(FALSE,TRUE,FALSE,0));
		m_fntNameNormal.Attach( BkFontPool::GetFont(TRUE, FALSE, FALSE, 0));
		m_fntPlug.Attach( BkFontPool::GetFont(BKF_DEFAULTFONT));
		m_fntDanger.Attach( BkFontPool::GetFont(BKF_BOLDFONT));
		m_fntDefault.Attach( BkFontPool::GetFont(BKF_DEFAULTFONT));

		m_nNameSort = SORT_TYPE_UNKNOW;
		m_nSizeSort = SORT_TYPE_UNKNOW;
		m_nTimeSort = SORT_TYPE_UNKNOW;
	}

	~CSoftMgrListView()
	{
		if(m_hWnd != NULL)
		{
			DeleteAllItems();
		}

		m_fntNameOn.Detach();
		m_fntNameNormal.Detach();
		m_fntPlug.Detach();
		m_fntDanger.Detach();
		m_fntDefault.Detach();
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
		MOUSE_LASTON_DESCRIPTION,
		MOUSE_LASTON_MARK,
		MOUSE_LASTON_NEWINFO,
	};

	enum SortType
	{
		SORT_TYPE_UNKNOW = 0,
		SORT_TYPE_UP,
		SORT_TYPE_DOWN,
	};

public:
	BEGIN_MSG_MAP(CSoftMgrListView)  
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
		CHAIN_MSG_MAP_ALT(COwnerDraw<CSoftMgrListView>,1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()   

	void SetShowType(BOOL bShow = FALSE);
	void OnSetFocus(HWND hWnd);
	HRESULT	OnMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled );
	BOOL OnEraseBkgnd(CDCHandle dc);
	HRESULT	OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled );
	void OnLButtonDBClk(UINT uMsg, CPoint ptx);

	LRESULT OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnSize(UINT nType, CSize size);
	void SetLoadShow(BOOL bShow=TRUE);
	void CreateTipCtrl();
	void SetShowDlgState();

	void InitNormal();
	void InitDownLoad();

	LRESULT	OnCtlColor(HDC hDc, HWND hWnd);

	CSoftListItemData*	GetItemDataEx(DWORD dwIndex);

	int SetItemDataEx(DWORD dwIndex, DWORD_PTR dwItemData);

	BOOL GetItemRect(int nItem, LPRECT lpRect, UINT nCode);
	HRESULT	OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled );

	HRESULT	OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled );

	void SetClickLinkCallback( ISoftMgrCallback* opCB );

	VOID SetPaintItem(BOOL bDrawItem);

	void MeasureItem(LPMEASUREITEMSTRUCT lpMes);

	HRESULT	OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled );

	void DrawItem ( LPDRAWITEMSTRUCT lpDrawItemStruct );

	void SetCheckedAllItem(BOOL bChecked);

	void RefreshItem(int nIndex);

	void RefreshProgress(int nIndex);

	void RefreshButton(int nIndex);
	void RefreshRight(int nIndex);
	void RefreshIcon(int nIndex);

	BOOL FindItem(CSoftListItemData *pData);

	void DeleteAllItemsEx(); //清空列表页删除数据

	void ClearAllItemData(); //只清空列表，不删除数据
	void DeleteItem(LPDELETEITEMSTRUCT lParam);

	int GetItemCount();
	void AppendPageItem(int nCurPage, int nItemCount);
	void UpdateBtnRect();

	CSimpleArray<CSoftListItemData*>& GetItemArray() 
	{ return m_arrData; }
	BOOL GetSoftType(int nListIndex);
	void UpdateAll();
	int GetListCenterPos(int nListIndex);

protected:
	void _DrawSoftItem(HDC hDC, LPDRAWITEMSTRUCT lpDrawItemStruct);
	void _DrawPageItem(HDC hDC, LPDRAWITEMSTRUCT lpDrawItemStruct);

	void _MouseMoveSoftItem(CSoftListItemData* pData, POINT pts, CRect rcItem);
	void _MouseMovePageItem(CSoftListItemData* pData, POINT pts, CRect rcItem);
	void _LButtonDownSoftItem(CSoftListItemData* pData, POINT pts, CRect rcItem, BOOL &bMsgHandled );
	void _LButtonDownPageItem(CSoftListItemData* pData, POINT pts, CRect rcItem, BOOL &bMsgHandled );
	void _LButtonUpSoftItem(CSoftListItemData* pData, POINT pts, CRect rcItem, int nIndex);
	void _LButtonUpPageItem(CSoftListItemData* pData, POINT pts, CRect rcItem, int nIndex);

public:
	SortType	m_nNameSort;
	SortType	m_nSizeSort;
	SortType	m_nTimeSort;

	BOOL m_bShowType;			//是否在名字前显示类型
	CRect m_rcCheck;			//选择框
	CRect m_rcIcon;				//图标
	CRect m_rcName;				//软件名称
	CRect m_rcDescription;		//软件描述
	CRect m_rcSize;				//软件大小
	CRect m_rcStar;				//分数星星
	CRect m_rcMark;				//分数
	CRect m_rcStateBtn;			//状态按钮
	CRect m_rcCancel;			//取消按钮
	CRect m_rcContinue;			//继续按钮
	CRect m_rcState;			//状态或者下载速度
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
	CRect m_rcUseOnKey;			//正在被快速装机使用时屏蔽按钮,显示正在安装

	BOOL m_bDrawItem;			//是否有需要重绘制的项
	CBrush	m_hBGBrush;			//背景刷
	BOOL m_bMouseOn;			//鼠标是否在状态按钮上
	BOOL m_bMouseDown;			//鼠标是否按下
	BOOL m_bMouseOnPause;		//鼠标在暂停按钮上
	BOOL m_bMouseOnCancel;		//鼠标在取消按钮上
	BOOL m_bMouseOnName;		//鼠标在软件名称上面
	BOOL m_bMouseOnFreeback;
	BOOL m_bMouseOnTautology;

	CFont		m_fntNameOn;	// 软件名称字体(鼠标在上面）
	CFont		m_fntNameNormal;// 软件名称字体（正常情况下）
	CFont		m_fntPlug;		// 插件字体
	CFont		m_fntDefault;	// 一般字体
	CFont		m_fntDanger;	// 特殊提示的字体

	CSimpleArray<CSoftListItemData*>	m_arrData;
	ISoftMgrCallback			*m_linkerCB;
	CToolTipCtrl				 m_wndToolTip;
	CBkDialogView				 m_dlgMsg;

	DWORD						 m_pHoverTip;
	DWORD						m_pRefresh;
	LASTSTATE					m_enumLastMSState;

	CDC	  m_dcMem;
	CBitmap m_bmpMem;

	CSoftListItemData m_pDataPageItem;
	int m_nCurPage;
	int m_nItemCount;
	BOOL m_bPageBtnEnableEx[14];
	CRect m_rcPageBtnEx[14];
	int m_nPageBtnEx[14];
};

#endif