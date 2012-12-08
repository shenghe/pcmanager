//////////////////////////////////////////////////////////////////////////
//	
// FileName		:  SoftMgrUniExpandListView.h
// Comment		:  
// Creator		:  Huang RuiKun (Nuk)
// Create time	:  2010-9-7
//
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "beikesafe.h"
#include "beikecolordef.h"

#define XX_SOFT_UNI_LIST_ITEM_HEIGHT_EXP	52
#define XX_SOFT_UNI_LIST_ITEM_HEIGHT_COL	32

#define COL_SOFT_UNI_STR_CPT_DEFAULT		RGB(  0,  0,  0)
#define COL_STR_CPT_MOUSEON		RGB(  0,100,200)
#define COL_STR_CPT_DISABLE		RGB(150,150,150)
#define COL_STR_CPT_LINK		RGB(  0,109,255)
#define COL_SELECT_BG			RGB(235,245,255)//RGB(255,240,205)
#define COL_LINE				RGB(218,225,231)

#define DETAILS_OFFSET_X		5

#define CLICK_EXPAND_INVALID		-1
#define CLICK_EXPAND_NULL			0
#define CLICK_EXPAND_ENABLE			1
#define CLICK_EXPAND_MARK			2
#define CLICK_EXPAND_ICON			3
#define CLICK_EXPAND_CAPTION		4
#define CLICK_EXPAND_DESC			5

#define BTN_COLUMN_WIDTH	120
#define TIME_COLUMN_WIDTH	100
#define SIZE_COLUMN_WIDTH	100

struct SOFT_UNI_INFO
{
public:
	SOFT_UNI_INFO()
	{
		nSize = 0;
		nLastTime = 0;
		nRunCount = 0;
		bEnable = FALSE;
		bMouseDown = FALSE;
		bMouseOn = FALSE;
		pImg = NULL;
		bDefaultImg = FALSE;
		bShowInList = FALSE;
		hIcon = NULL;
		//nType = 0;

		strRunFrequency = L"未知";
		strSize = L"未知";
		strLastTime = L"未知";
		strType = L"未知";
		fMark = 0;
	}

#define STR_NEW_DETAIL_URL			L"http://baike.ijinshan.com/ksafe/client/v1_0/%s.html"
	CString GetInfoUrl()
	{
		CString		str;
		if (!strSoftID.IsEmpty())
			str.Format(STR_NEW_DETAIL_URL,strSoftID);
		return str;
	}

public:
	CString		strKey;
	CString		strSoftID;

	CString		strCaption;		//显示的名字
	CString		strSpellWhole;	//全拼，用于搜索
	CString		strSpellAcronym;//简拼，用于搜索
	CString		strDesc;		//描述
	CString		strSize;		//大小
	ULONGLONG	nSize;			//大小，用于排序
	CString		strLastTime;	//最后使用时间
	LONG		nLastTime;		//最后使用时间, 用于排序
	CString		strRunFrequency;//最近一个月的使用频率
	LONG		nRunCount;		//最近一个月的使用次数
	//LONG		nType;			//软件类型
	CString		strType;		//软件类型
	CString		strInfoUrl;		//详情连接
	CString		strMainPath;	//安装根目录
	BOOL		bDefaultImg;
	Gdiplus::Image*		pImg;			//图标
	HICON		hIcon;					//未知类型的卸载项目本地的图标

	BOOL		bEnable;		//是否卸载
	BOOL		bMouseDown;
	BOOL		bMouseOn;	
	BOOL		bShowInList;
	float		fMark;			//分数
};

struct SOFT_RUB_INFO
{
public:
	SOFT_RUB_INFO()
	{
		nType = 0;
		bCheck = FALSE;
	}

	int			nType;
	CString		strData;
	BOOL		bCheck;
};

class ILstSoftUniExpandCallback
{
public:
	virtual void OnUniExpandDownLoadIcon(CString strID) = 0;
	virtual BOOL OnClickSoftUniExpand(DWORD nIndex, SOFT_UNI_INFO* pInfo) = 0;
};

class CSoftMgrUniExpandListView : 
	public CWindowImpl<CSoftMgrUniExpandListView, CListBox>
	,public COwnerDraw<CSoftMgrUniExpandListView>
{
public:

	struct SOFT_UNI_ITEM
	{
	public:
		SOFT_UNI_ITEM()
		{
			nIndex		= -1;
			rcIcon		= CRect(0,0,0,0);
			rcCaption	= CRect(0,0,0,0);
			rcDesc		= CRect(0,0,0,0);
			rcStar		= CRect(0,0,0,0);
			rcMark		= CRect(0,0,0,0);
			rcRate		= CRect(0,0,0,0);
			rcButton	= CRect(0,0,0,0);
			bExpand		= FALSE;
			bMouseOnCaption = FALSE;
		}

		int				nIndex;
		SOFT_UNI_INFO*	pInfo;
		CRect			rcIcon;
		CRect			rcCaption;	//名字
		CRect			rcDesc;		//描述
		CRect			rcStar;		//分数星星
		CRect			rcMark;		//分数
		CRect			rcRate;		//频率
		CRect			rcButton;	//按钮
		BOOL			bExpand;
		BOOL			bMouseOnCaption;
	};

	enum SortType
	{
		SORT_TYPE_UNKNOW = 0,
		SORT_TYPE_UP,
		SORT_TYPE_DOWN,
	};

	CSoftMgrUniExpandListView()
	{
		m_rcIcon	= CRect(10,10,10+32,10+32);
		m_rcCaption	= CRect(50,8,100,20);
		m_rcDesc	= CRect(50,30,100,42);
		m_rcStar	= CRect(50,6,100,20);
		m_rcMark	= CRect(50,30,100,42);

		m_opCallback= NULL;
		m_pHoverTip = 0;

		m_fontNormal.Attach( BkFontPool::GetFont(BKF_DEFAULTFONT));
		m_fontBold.Attach( BkFontPool::GetFont(BKF_BOLDFONT));
		m_fontLink.Attach( BkFontPool::GetFont(BKF_UNDERLINE));

		m_nNameSort = SORT_TYPE_UNKNOW;
		m_nSizeSort = SORT_TYPE_UNKNOW;
		m_nTimeSort = SORT_TYPE_UNKNOW;
	}

	~CSoftMgrUniExpandListView(void)
	{
		ClearAll();

		m_fontNormal.Detach();
		m_fontBold.Detach();
		m_fontLink.Detach();
	}

	CSimpleArray<SOFT_UNI_ITEM*>& GetItemArray() 
	{ return m_arrItem; }

	BOOL GetSoftType(int nListIndex)
	{
		BOOL bResult = FALSE;

		switch(nListIndex)
		{
		case 0:
			{
				if (m_nNameSort == SORT_TYPE_UNKNOW)
				{
					m_nNameSort = SORT_TYPE_DOWN;
					return TRUE;
				}
				else if (m_nNameSort == SORT_TYPE_UP)
				{
					m_nNameSort = SORT_TYPE_DOWN;
					return TRUE;
				}
				else if (m_nNameSort == SORT_TYPE_DOWN)
				{
					m_nNameSort = SORT_TYPE_UP;
					return FALSE;
				}
			}
			break;
		case 1:
			if (m_nSizeSort == SORT_TYPE_UNKNOW)
			{
				m_nSizeSort = SORT_TYPE_DOWN;
				return TRUE;
			}
			else if (m_nSizeSort == SORT_TYPE_UP)
			{
				m_nSizeSort = SORT_TYPE_DOWN;
				return TRUE;
			}
			else if (m_nSizeSort == SORT_TYPE_DOWN)
			{
				m_nSizeSort = SORT_TYPE_UP;
				return FALSE;
			}
			break;
		case 2:
			if (m_nTimeSort == SORT_TYPE_UNKNOW)
			{
				m_nTimeSort = SORT_TYPE_DOWN;
				return TRUE;
			}
			else if (m_nTimeSort == SORT_TYPE_UP)
			{
				m_nTimeSort = SORT_TYPE_DOWN;
				return TRUE;
			}
			else if (m_nTimeSort == SORT_TYPE_DOWN)
			{
				m_nTimeSort = SORT_TYPE_UP;
				return FALSE;
			}
			break;
		}

		return bResult;
	}

	int GetListCenterPos(int nListIndex)
	{
		int nResult = 0;
		CRect rcClient;
		GetWindowRect(&rcClient);

		switch(nListIndex)
		{
		case 0:
			nResult = (rcClient.Width() -  BTN_COLUMN_WIDTH - TIME_COLUMN_WIDTH - SIZE_COLUMN_WIDTH)/2
				+ 25;
			break;
		case 1:
			nResult = rcClient.Width() -  BTN_COLUMN_WIDTH - TIME_COLUMN_WIDTH - SIZE_COLUMN_WIDTH
				+ SIZE_COLUMN_WIDTH/2
				+ 25;
			break;
		case 2:
			nResult = rcClient.Width() -  BTN_COLUMN_WIDTH - TIME_COLUMN_WIDTH
				+ TIME_COLUMN_WIDTH/2
				+ 25;
			break;
		}
		return nResult;
	}

	DECLARE_WND_CLASS(_T("Ksm_Uni_ExpandWnd_by_Nuk07"))

protected:
	CSimpleArray<SOFT_UNI_ITEM*> m_arrItem;

	CRect						 m_rcIcon;
	CRect						 m_rcCaption;
	CRect						 m_rcDesc;
	CRect						 m_rcStar;
	CRect						 m_rcMark;

	ILstSoftUniExpandCallback*	 m_opCallback;
	CToolTipCtrl				 m_wndToolTip;
	CBkDialogView				 m_dlgMsg;

	DWORD						 m_pHoverTip;
	CBrush						 m_hBGBrush;

	CFont		m_fontNormal;
	CFont		m_fontBold;	
	CFont		m_fontLink;	

	SortType	m_nNameSort;
	SortType	m_nSizeSort;
	SortType	m_nTimeSort;

	// 内存缓冲DC，bmp
	CDC			m_dcMem;
	CBitmap		m_bmpMem;


public:
	BEGIN_MSG_MAP(CSoftMgrUniExpandListView)   
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDBClk)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_SIZE(OnSize)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MSG_OCM_CTLCOLORLISTBOX(OnCtlColor)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CSoftMgrUniExpandListView>,1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()   

	LRESULT	OnCtlColor(HDC hDc, HWND hWnd)
	{
		if (hWnd==m_hWnd)
		{
			CDCHandle	dchan;
			dchan.Attach(hDc);

			dchan.SetBkMode(TRANSPARENT);
			SetMsgHandled(TRUE);

			if (m_hBGBrush.m_hBrush==NULL)
				m_hBGBrush.CreateSolidBrush(COL_DEFAULT_WHITE_BG);

			dchan.Detach();
			return (LRESULT)m_hBGBrush.m_hBrush;
		}
		SetMsgHandled(FALSE);
		return NULL;
	}

	LRESULT OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if(m_wndToolTip.IsWindow())
		{
			MSG msg = { m_hWnd, uMsg, wParam, lParam };
			m_wndToolTip.RelayEvent(&msg);
		}

		SetMsgHandled(FALSE);

		return 0;
	}

	void SetOpCallback(ILstSoftUniExpandCallback* pCallback)
	{
		m_opCallback = pCallback;
	}

	void GetCKSRunInfoArray(CSimpleArray<SOFT_UNI_INFO*>& arrayx)
	{
		for (int i=0; i < m_arrItem.GetSize(); i++)
		{
			if (m_arrItem[i]->pInfo)
			{
				arrayx.Add(m_arrItem[i]->pInfo);
			}
		}
	}

	void CreateCtrl()
	{
		if ( !m_dlgMsg.Load(IDR_BK_LISTTIP_MGR_DLG) )
			return;
		m_dlgMsg.Create(m_hWnd);
		m_dlgMsg.ShowWindow(SW_HIDE);

		m_wndToolTip.Create(m_hWnd);

		CToolInfo ti(0, m_hWnd);
		m_wndToolTip.AddTool(ti);
		m_wndToolTip.Activate(TRUE);
		m_wndToolTip.SetMaxTipWidth(500);
		m_wndToolTip.SetDelayTime(TTDT_AUTOPOP, 5*1000);
		m_wndToolTip.SetDelayTime(TTDT_INITIAL, 500);
	}

	int XBASE_HitTest(CPoint ptx)
	{
		BOOL bOut = FALSE;
		int i = (int)CListBox::ItemFromPoint(ptx,bOut);

		if (!bOut)
			return i;
		else
			return -1;
	}

	int XBASE_InsertItem(int iIndex, LPCTSTR lpstr)
	{
		return CListBox::InsertString(iIndex,lpstr);
	}

	BOOL XBASE_GetItemRect(int i, LPRECT lprc)
	{
		return CListBox::GetItemRect(i,lprc);
	}

	BOOL XBASE_DeleteItem(int i)
	{
		return CListBox::DeleteString((UINT)i);
	}

	int XBASE_GetItemCount()
	{
		return CListBox::GetCount();
	}

	void XBASE_DeleteAllItems()
	{
		CListBox::ResetContent();
	}

	DWORD_PTR XBASE_GetItemData(int nItem)
	{
		DWORD_PTR pvoid = CListBox::GetItemData(nItem);

		if ((DWORD)pvoid==-1)
			return NULL;

		return pvoid;
	}

	BOOL XBASE_SetItemData(int nItem,DWORD_PTR pItem)
	{
		return CListBox::SetItemData(nItem,pItem);
	}

	int LabelHitTest(CPoint ptx)
	{
		int iItem = XBASE_HitTest(ptx);

		CRect rcItem;
		SOFT_UNI_ITEM* pItem = (SOFT_UNI_ITEM*)XBASE_GetItemData(iItem);

		if (pItem==NULL)
			return CLICK_EXPAND_INVALID;
		
		XBASE_GetItemRect(iItem,&rcItem);
		ptx.Offset(-rcItem.left,-rcItem.top);

		if (pItem->rcCaption.PtInRect(ptx))
		{
			if (pItem->bMouseOnCaption == FALSE)
			{
				pItem->bMouseOnCaption = TRUE;
				RefreshRight(pItem->nIndex);
			}

			return CLICK_EXPAND_CAPTION;
		}
		else
		{
			if (pItem->bMouseOnCaption == TRUE)
			{
				pItem->bMouseOnCaption = FALSE;
				RefreshRight(pItem->nIndex);
			}
		}

		if (pItem->rcDesc.PtInRect(ptx))
			return CLICK_EXPAND_DESC;

		if (pItem->rcButton.PtInRect(ptx))
			return CLICK_EXPAND_ENABLE;
		
		if (pItem->rcIcon.PtInRect(ptx))
			return CLICK_EXPAND_ICON;

		if (pItem->pInfo->strSoftID.GetLength() != 0 &&
			(pItem->rcMark.PtInRect(ptx) || pItem->rcStar.PtInRect(ptx))
			)
		{
			return CLICK_EXPAND_MARK;
		}

		return CLICK_EXPAND_NULL;
	}

	void OnLButtonDBClk(UINT uMsg, CPoint ptx)
	{
		BOOL bHandle = FALSE;
		int iItem = XBASE_HitTest(ptx);

		SOFT_UNI_ITEM* pItem = (SOFT_UNI_ITEM*)XBASE_GetItemData(iItem);
		if (pItem!=NULL && pItem->pInfo)
		{
			SOFT_UNI_INFO*	pInfo = pItem->pInfo;
			int iIndex = CLICK_EXPAND_ICON;
			m_opCallback->OnClickSoftUniExpand(iIndex, pInfo);
		}

		SetMsgHandled(bHandle);
	}

	void OnLButtonDown(UINT uMsg, CPoint ptx)
	{
		//SetFocus();

		BOOL bHandle = FALSE;
		int iItem = XBASE_HitTest(ptx);
		SetCurSel(iItem);
		SOFT_UNI_ITEM* pItem = (SOFT_UNI_ITEM*)XBASE_GetItemData(iItem);
		if (pItem!=NULL && pItem->pInfo)
		{
			SOFT_UNI_INFO*	pInfo = pItem->pInfo;
			int iIndex = LabelHitTest(ptx);
			if (iIndex == CLICK_EXPAND_ENABLE)
			{
				if (pItem->bExpand)
				{
					pInfo->bMouseDown = TRUE;
					m_opCallback->OnClickSoftUniExpand(iIndex, pInfo);
					bHandle = TRUE;
				}
			}
			else if (iIndex == CLICK_EXPAND_ICON)
			{
				m_opCallback->OnClickSoftUniExpand(iIndex, pInfo);
			}
			else if (iIndex == CLICK_EXPAND_CAPTION)
			{
				m_opCallback->OnClickSoftUniExpand(iIndex, pInfo);
				pItem->bMouseOnCaption = FALSE;
				RefreshRight(pItem->nIndex);
			}
			else if (iIndex == CLICK_EXPAND_MARK)
			{
				m_opCallback->OnClickSoftUniExpand(iIndex, pInfo);
			}

 			if (!bHandle)
 			{
 				MsgDoExp(iItem);
 			}
		}		

		SetMsgHandled(TRUE);
		return;
	}

	void OnLButtonUp(UINT uMsg, CPoint ptx)
	{
		for (int i=0; i < m_arrItem.GetSize(); i++)
		{
			if (m_arrItem[i]->pInfo)
			{
				m_arrItem[i]->pInfo->bMouseDown = FALSE;
			}
		}

		Invalidate(FALSE);

		//int		iItem = XBASE_HitTest(ptx);

		//SOFT_UNI_ITEM* pItem = (SOFT_UNI_ITEM*)XBASE_GetItemData(iItem);
		//if (pItem!=NULL && pItem->pInfo)
		//{
		//	SOFT_UNI_INFO*	pInfo = pItem->pInfo;
		//	pInfo->bMouseDown = FALSE;
		//	
		//	Invalidate(FALSE);
		//}

		SetMsgHandled(FALSE);
	}

	HRESULT	OnMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
	{
		for (int i=0; i < m_arrItem.GetSize(); i++)
		{
			if (m_arrItem[i]->pInfo)
			{
				m_arrItem[i]->pInfo->bMouseDown = FALSE;
				m_arrItem[i]->pInfo->bMouseOn = FALSE;
			}
		}

		Invalidate(FALSE);

		return TRUE;
	}

	void MsgDoExp(int iItem)
	{
		SOFT_UNI_ITEM* pItem = (SOFT_UNI_ITEM*)XBASE_GetItemData(iItem);
		
		SetRedraw(FALSE);

		CPoint	ptx;
		::GetCursorPos(&ptx);

		CRect	rcOffset(0,0,0,0);
		ScreenToClient(rcOffset);
		ptx.Offset(rcOffset.left,rcOffset.top);
		int nShowIndex = XBASE_HitTest(ptx);
		BOOL bExpand = pItem->bExpand;
	
		ExpandAllItem(FALSE);
		ExpandItem(iItem);

		SetRedraw(TRUE);
		InvalidateRect(NULL);
	}

	void ExpandAllItem(BOOL bExp=TRUE)
	{
		for (int i=0; i < XBASE_GetItemCount(); i++)
		{
			ExpandItem(i,bExp);
		}
	}

	void ExpandItem(int iItem, BOOL bExp = TRUE)
	{
		SOFT_UNI_ITEM* pItem = (SOFT_UNI_ITEM*)XBASE_GetItemData(iItem);
		
		if (bExp && (pItem && !pItem->bExpand))
		{
			pItem->bExpand = TRUE;
			CListBox::SetItemHeight(iItem,XX_SOFT_UNI_LIST_ITEM_HEIGHT_EXP);
		}
		else if ( !bExp && (pItem && pItem->bExpand))
		{
			pItem->bExpand = FALSE;
			CListBox::SetItemHeight(iItem,XX_SOFT_UNI_LIST_ITEM_HEIGHT_COL);
		}
	}

	SOFT_UNI_INFO* GetDataInfo(int nItem)
	{
		SOFT_UNI_ITEM* pItem = (SOFT_UNI_ITEM*)XBASE_GetItemData(nItem);

		if (pItem!=NULL && (DWORD)pItem != -1 )
			return pItem->pInfo;
		else
			return NULL;
	}

	void OnMouseMove(UINT uMsg, CPoint ptx)
	{
		for (int i=0; i < m_arrItem.GetSize(); i++)
		{
			if (m_arrItem[i] && m_arrItem[i]->bMouseOnCaption)
			{
				m_arrItem[i]->bMouseOnCaption = FALSE;
				RefreshRight(i);
			}
		}

		int iItem = XBASE_HitTest(ptx);

		SOFT_UNI_INFO*	pInfo		= GetDataInfo(iItem);
		SOFT_UNI_ITEM*	pItem	= (SOFT_UNI_ITEM*)XBASE_GetItemData(iItem);
		
		if (pInfo && pItem)
		{
			int iIndex = LabelHitTest(ptx);
			if (iIndex == CLICK_EXPAND_ENABLE)
			{
				if (pItem->bExpand)
				{
					::SetCursor(::LoadCursor(NULL,IDC_HAND));
					pInfo->bMouseOn = TRUE;
				}
			}
			else if (iIndex == CLICK_EXPAND_ICON )
				::SetCursor(::LoadCursor(NULL,IDC_HAND));
			else if (iIndex == CLICK_EXPAND_CAPTION )
				::SetCursor(::LoadCursor(NULL,IDC_HAND));
			else if (iIndex == CLICK_EXPAND_MARK )
				::SetCursor(::LoadCursor(NULL,IDC_HAND));

			if (iIndex==CLICK_EXPAND_DESC || iIndex==CLICK_EXPAND_ICON || iIndex==CLICK_EXPAND_CAPTION)
			{
				if ( ((DWORD)pInfo!=m_pHoverTip))
				{
					CString	strInfo;

					if (iIndex==CLICK_EXPAND_DESC)
						strInfo.Append( pInfo->strDesc.IsEmpty()?L"暂时没有简介":pInfo->strDesc );
					else if (iIndex==CLICK_EXPAND_ICON || iIndex==CLICK_EXPAND_CAPTION)
						strInfo.Append( L"点击查看该软件简介及截图" );

					m_wndToolTip.SetMaxTipWidth(500);
					m_wndToolTip.UpdateTipText((LPCTSTR)strInfo,m_hWnd);
					m_pHoverTip = (DWORD)pInfo;
				}
			}
			else
			{
				if (m_pHoverTip!=0)
				{
					m_pHoverTip = 0;
					m_wndToolTip.UpdateTipText((LPCTSTR)NULL,m_hWnd);
					m_wndToolTip.Pop();
				}
			}
		}

		TRACKMOUSEEVENT   tme;   
		tme.cbSize   =   sizeof(tme);   
		tme.hwndTrack   =   m_hWnd;   
		tme.dwFlags   =   TME_LEAVE;   
		_TrackMouseEvent(&tme); 
	}

	void DeleteItem(LPDELETEITEMSTRUCT lParam)
	{
		return;
	}

	int GetItemIndexByKSInfo(SOFT_UNI_INFO* pItem)
	{
		for ( int i=0; i<XBASE_GetItemCount(); i++)
		{
			if (GetDataInfo(i)==pItem)
				return i;
		}
		return -1;
	}

	void calcLeftRECT(CRect rcArea, CRect rcProbe, CRect& rcOut)
	{
		rcOut.left		= rcArea.left;
		rcOut.right		= rcOut.left + rcProbe.Width();
		rcOut.top		= rcArea.top;
		rcOut.bottom	= rcOut.top + rcProbe.Height();
	}

	void calcCenterRECT(CRect rcArea, CRect rcProbe, CRect& rcOut)
	{
		rcOut.left		= rcArea.left + (rcArea.Width()-rcProbe.Width())/2;
		rcOut.right		= rcOut.left + rcProbe.Width();
		rcOut.top		= rcArea.top + (rcArea.Height()-rcProbe.Height())/2;
		rcOut.bottom	= rcOut.top + rcProbe.Height();
	}


	void _CreateMemDC(LPDRAWITEMSTRUCT lpDrawItemStruct )
	{
		CRect	rcItemX = lpDrawItemStruct->rcItem;
		if ( m_dcMem == NULL )
		{
			m_dcMem.CreateCompatibleDC(lpDrawItemStruct->hDC);
			m_bmpMem.CreateCompatibleBitmap(lpDrawItemStruct->hDC, rcItemX.Width(), rcItemX.Height());
		}
		else
		{
			BITMAP bmp;
			m_bmpMem.GetBitmap(&bmp);
			int nWidth = bmp.bmWidth;
			int nHeight = bmp.bmHeight; 
			if (nWidth != rcItemX.Width() || nHeight != rcItemX.Height())
			{
				DeleteObject(m_bmpMem);
				m_bmpMem = NULL;
				m_bmpMem.CreateCompatibleBitmap(lpDrawItemStruct->hDC, rcItemX.Width(), rcItemX.Height());
			}
		}

	}

	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		BOOL bRollBar = FALSE;
		CRect rcWnd;
		GetWindowRect(&rcWnd);
		if (rcWnd.Height() < GetAllCount() * XX_SOFT_UNI_LIST_ITEM_HEIGHT_EXP)
			bRollBar = TRUE;

		// 先创建内存DC
		_CreateMemDC(lpDrawItemStruct);


		SOFT_UNI_ITEM*	pItem = (SOFT_UNI_ITEM*)XBASE_GetItemData(lpDrawItemStruct->itemID);
		SOFT_UNI_INFO*	pInfo = GetDataInfo(lpDrawItemStruct->itemID);
		HFONT			hFontOld = NULL;
		COLORREF		clrOld;
		CRect			rcWin;
		CDCHandle		dcListWin;
		CBitmap			bmpTmp;
		CRect			rcItemDraw = lpDrawItemStruct->rcItem;
		CRect			rcItemX;
		HBITMAP			oldBmp;
		CDC&			dcx = m_dcMem;

		rcItemX.left	= 0;
		rcItemX.top		= 0;
		rcItemX.right   = rcItemDraw.Width();
		rcItemX.bottom  = rcItemDraw.Height();

		GetWindowRect(&rcWin);
		dcListWin.Attach(lpDrawItemStruct->hDC);


		oldBmp = dcx.SelectBitmap(m_bmpMem);
		hFontOld = dcx.SelectFont(m_fontNormal);
		CBrush Brush;
		Brush.CreateSolidBrush(RGB(225,225,255));
		dcx.SelectBrush(Brush);

		clrOld = dcx.GetTextColor();

		if (pInfo && pItem)
		{
			if (lpDrawItemStruct->itemState&ODS_SELECTED)
				dcx.FillSolidRect(&rcItemX, COL_SELECT_BG);
			else
				dcx.FillSolidRect(&rcItemX, COL_DEFAULT_WHITE_BG);

			{
				CRect	rcIcon=m_rcIcon;
				rcIcon.OffsetRect(rcItemX.left,rcItemX.top);

				Gdiplus::Graphics grap(dcx.m_hDC);
				grap.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
				if (!pItem->bExpand)
				{
					if ( pInfo->pImg )
					{
						Gdiplus::RectF rcDest(rcIcon.left, rcIcon.top, 16, 16);
						grap.DrawImage(pInfo->pImg, rcDest, 0, 0,
							pInfo->pImg->GetWidth(), pInfo->pImg->GetHeight(), Gdiplus::UnitPixel);
					}
					else if (pInfo->hIcon)
					{
						::DrawIconEx(dcx.m_hDC,rcIcon.left,rcIcon.top,pInfo->hIcon,16,16,0,NULL,DI_NORMAL);
					}

					pItem->rcIcon = CRect(rcIcon.left, rcIcon.top, rcIcon.left + 16, rcIcon.top + 16);
					pItem->rcIcon.OffsetRect(-rcItemX.left, -rcItemX.top);
				}
				else
				{
					if ( pInfo->pImg )
					{
						Gdiplus::RectF rcDest(rcIcon.left, rcIcon.top, 32, 32);
						grap.DrawImage(pInfo->pImg, rcDest, 0, 0,
							pInfo->pImg->GetWidth(), pInfo->pImg->GetHeight(), Gdiplus::UnitPixel);
					}
					else if (pInfo->hIcon)
					{
						::DrawIconEx(dcx.m_hDC,rcIcon.left,rcIcon.top,pInfo->hIcon,32,32,0,NULL,DI_NORMAL);
					}

					pItem->rcIcon = CRect(rcIcon.left, rcIcon.top, rcIcon.left + 32, rcIcon.top + 32);
					pItem->rcIcon.OffsetRect(-rcItemX.left, -rcItemX.top);
				}
				if (!pInfo->strSoftID.IsEmpty() && pInfo->bDefaultImg )
					m_opCallback->OnUniExpandDownLoadIcon(pInfo->strSoftID);

				grap.SetInterpolationMode(Gdiplus::InterpolationModeDefault);
			}

			dcx.SetBkMode(TRANSPARENT);
		
			{
				COLORREF clOld = dcx.GetTextColor();
				CRect	rcCaption=m_rcCaption;
				rcCaption.top+=rcItemX.top;
				rcCaption.bottom=rcCaption.top+m_rcCaption.Height();
				rcCaption.left+=rcItemX.left;
				rcCaption.right=rcItemX.right - BTN_COLUMN_WIDTH - TIME_COLUMN_WIDTH - SIZE_COLUMN_WIDTH;

				CRect	rcProbe;
				if (pItem->bExpand)
					hFontOld = dcx.SelectFont(m_fontBold);
				else
					hFontOld = dcx.SelectFont(m_fontNormal);
				if (pItem->bMouseOnCaption)
					dcx.SetTextColor(COL_STR_CPT_MOUSEON);
				else
					dcx.SetTextColor(COL_SOFT_UNI_STR_CPT_DEFAULT);
				dcx.DrawText(pInfo->strCaption,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
				if (rcCaption.left + rcProbe.right > rcCaption.right)
					rcProbe.right = rcCaption.right - rcCaption.left;
				dcx.DrawText(pInfo->strCaption,-1,&rcCaption,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);

				rcProbe.bottom = rcProbe.top+12;
				rcCaption.OffsetRect(-rcItemX.left, -rcItemX.top);
				calcLeftRECT(rcCaption,rcProbe,pItem->rcCaption);	
				dcx.SelectFont(hFontOld);
				dcx.SetTextColor(clOld);
			}

			clrOld = dcx.SetTextColor(RGB(122,122,122));

			{
				CRect	rcState=rcItemX;
				CString	strState;

				dcx.SelectFont(m_fontNormal);
				
				rcState.top		= rcItemX.top+((pItem->bExpand?XX_SOFT_UNI_LIST_ITEM_HEIGHT_EXP:XX_SOFT_UNI_LIST_ITEM_HEIGHT_COL)-12)/2;
				rcState.bottom	= rcState.top+12;
				rcState.right = rcItemX.right - BTN_COLUMN_WIDTH - TIME_COLUMN_WIDTH;
				rcState.left = rcState.right - SIZE_COLUMN_WIDTH;
				if (bRollBar)
					rcState.OffsetRect(17, 0);
				strState = pInfo->strSize;
				dcx.DrawText(strState, -1, &rcState, DT_VCENTER|DT_SINGLELINE|DT_CENTER);

				Gdiplus::Image *pImage = NULL;
				Gdiplus::Graphics grap(dcx.m_hDC);

				// 画频率
				rcState.top		= rcItemX.top+((pItem->bExpand?XX_SOFT_UNI_LIST_ITEM_HEIGHT_EXP:XX_SOFT_UNI_LIST_ITEM_HEIGHT_COL)-12)/2;
				rcState.bottom	= rcState.top+12;
				rcState.right= rcItemX.right - BTN_COLUMN_WIDTH + 5;
				rcState.left = rcState.right - TIME_COLUMN_WIDTH + 5;
				if (bRollBar)
					rcState.OffsetRect(17, 0);
				pItem->rcRate = rcState;
				pItem->rcRate.OffsetRect(-rcItemX.left,-rcItemX.top);
				dcx.DrawText(pInfo->strRunFrequency, -1, &rcState, DT_VCENTER|DT_SINGLELINE|DT_CENTER);

				//////////////////////////////////////////////////////////////////////////
				////画评分
				//int nStep = 15;
				//rcState = m_rcStar;
				//rcState.right= rcItemX.right - BTN_COLUMN_WIDTH + 5;
				//rcState.left = rcState.right - TIME_COLUMN_WIDTH + 5;
				//if (bRollBar)
				//	rcState.OffsetRect(17, 0);
				//pItem->rcStar = rcState;
				//pItem->rcStar.OffsetRect(-rcItemX.left,-rcItemX.top);
				//float fMark = pInfo->fMark;				//分数
				//for (int i = 0; i < 5; i++)
				//{
				//	if ((i+1)*2 <= fMark)
				//		pImage = BkPngPool::Get(IDP_SOFTMGR_STAR);
				//	else if ((i+1)*2 - 1 <= fMark)
				//		pImage = BkPngPool::Get(IDP_SOFTMGR_STAR_HALF);
				//	else
				//		pImage = BkPngPool::Get(IDP_SOFTMGR_STAR_OFF);

				//	if (pImage)
				//	{
				//		Gdiplus::RectF rcDest(rcState.left + i*nStep, rcState.top, pImage->GetWidth(), pImage->GetHeight());
				//		grap.DrawImage(pImage, rcDest, 0, 0,
				//			pImage->GetWidth(), pImage->GetHeight(), Gdiplus::UnitPixel);
				//	}
				//}

				//if (pInfo->strSoftID.GetLength() == 0)
				//	dcx.SetTextColor(RGB(128,128,128));
				//else
				//	dcx.SetTextColor(RGB( 47, 99,165));
				//rcState = m_rcMark;
				//rcState.right= rcItemX.right - BTN_COLUMN_WIDTH + 5;
				//rcState.left = rcState.right - TIME_COLUMN_WIDTH + 5;
				//if (bRollBar)
				//	rcState.OffsetRect(17, 0);
				//pItem->rcMark = rcState;
				//pItem->rcMark.OffsetRect(-rcItemX.left,-rcItemX.top);
				//CString strMark;
				//strMark.Format(L"%.1f分 投票", fMark);
				//dcx.DrawText(strMark, -1, &rcState, DT_VCENTER|DT_SINGLELINE|DT_LEFT);

				if (pItem->bExpand)
				{
					if (pInfo->bEnable)
						pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_UNINSTALLED);
					else
						pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_UNINSTALL);
					rcState.top		= rcItemX.CenterPoint().y - pImage->GetHeight()/2;
					rcState.bottom	= rcState.top + 25;
					rcState.left = rcItemX.right - BTN_COLUMN_WIDTH/2 - pImage->GetWidth()/6;
					rcState.right= rcState.left + pImage->GetWidth()/3;
					if (bRollBar)
						rcState.OffsetRect(17, 0);
					pItem->rcButton = rcState;
					pItem->rcButton.OffsetRect(-rcItemX.left,-rcItemX.top);
					//////////////////////////////////////////////////////////////////////////
					//if (pInfo->bMouseOn)	//鼠标在按钮上
					{
						if(pInfo->bMouseDown)
						{
							grap.DrawImage(pImage, rcState.left, rcState.top,rcState.Width()*2, 0,
								rcState.Width(), rcState.Height(), Gdiplus::UnitPixel);
						}
						else
						{
							grap.DrawImage(pImage, rcState.left, rcState.top,rcState.Width()*1, 0,
								rcState.Width(), rcState.Height(), Gdiplus::UnitPixel);
						}
					}
					//else
					//{
					//	grap.DrawImage(pImage, rcState.left, rcState.top,0, 0,
					//		rcState.Width(), rcState.Height(), Gdiplus::UnitPixel);
					//}
				}
			}

			if ( pItem->bExpand )
			{	
				//dcx.SetTextColor(COL_STR_CPT_DISABLE);

				CString	strShow;				
				strShow.Format(L"%s", pInfo->strDesc.IsEmpty()?L"暂时没有简介":pInfo->strDesc);

				CRect	rcProbe;
				dcx.DrawText(strShow,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);

				CRect	rcDesc = m_rcDesc;
				rcDesc.right = rcItemX.right - BTN_COLUMN_WIDTH - TIME_COLUMN_WIDTH - SIZE_COLUMN_WIDTH - DETAILS_OFFSET_X*2;
				if (rcDesc.right > rcDesc.left + rcProbe.Width())
					rcDesc.right = rcDesc.left + rcProbe.Width();
				rcDesc.top += rcItemX.top;
				rcDesc.bottom = rcDesc.top+m_rcDesc.Height();
				dcx.DrawText(strShow,-1,&rcDesc,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);

				rcProbe.bottom = rcProbe.top + m_rcDesc.Height();
				rcProbe.right = rcProbe.left + rcDesc.Width();
				rcDesc.OffsetRect(0, -rcItemX.top);
				calcLeftRECT(rcDesc,rcProbe,pItem->rcDesc);
			}
	
			{
				CPen	penx;
				penx.CreatePen(PS_SOLID,1,COL_LINE);
				HPEN	pentmp = dcx.SelectPen(penx);
				dcx.MoveTo(rcItemX.left,rcItemX.bottom-1);
				dcx.LineTo(rcItemX.right,rcItemX.bottom-1);
				dcx.SelectPen(pentmp);
			}

			dcListWin.BitBlt(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, rcItemX.Width(), rcItemX.Height(),
				dcx, 0,0, SRCCOPY
				);
		}
		dcx.SelectFont(hFontOld);
		dcx.SelectBitmap(oldBmp);
		dcListWin.Detach();
	}

	void MeasureItem(LPMEASUREITEMSTRUCT lParam)
	{
		SOFT_UNI_ITEM*	pItem = (SOFT_UNI_ITEM*)XBASE_GetItemData(lParam->itemID);

		if (/*TRUE || */(pItem && pItem->bExpand))
			lParam->itemHeight = XX_SOFT_UNI_LIST_ITEM_HEIGHT_EXP;
		else
			lParam->itemHeight = XX_SOFT_UNI_LIST_ITEM_HEIGHT_COL;
	}

	void InsertItemX(SOFT_UNI_INFO* pInfo)
	{
		int nPosNew = GetInsertIndex(pInfo);
		if (nPosNew == XBASE_GetItemCount())// 排重
		{
			int iPos = XBASE_InsertItem(nPosNew, pInfo->strKey);
			SOFT_UNI_ITEM*	pItem = NULL;
			pItem = new SOFT_UNI_ITEM;
			pItem->nIndex = nPosNew;
			pItem->pInfo = pInfo;	
			pItem->pInfo->bShowInList = TRUE;
			XBASE_SetItemData(iPos, (DWORD_PTR)pItem);

			m_arrItem.Add(pItem);
		}
	}

	BOOL isItemInList(SOFT_UNI_INFO* pInfo)
	{
		for (int i=0; i<XBASE_GetItemCount(); i++)
		{
			if ( pInfo == GetDataInfo(i) )
				return TRUE;
		}
		return FALSE;
	}

	int GetInsertIndex(SOFT_UNI_INFO* pInfo)
	{
		int i=0;
		for (i=0; i<XBASE_GetItemCount(); i++)
		{
			SOFT_UNI_INFO* pInfoTemp = GetDataInfo(i);
			if (pInfoTemp && pInfoTemp->strKey.CompareNoCase(pInfo->strKey) == 0)
				return i;
		}
		return i;
	}

	void ResetExpandState(BOOL bExp=FALSE)
	{
		for (int i=0; i<m_arrItem.GetSize(); i++)
		{
			m_arrItem[i]->bExpand = bExp;
		}
	}

	void DeleteItemByKSInfo(SOFT_UNI_INFO* pInfo)
	{
		for (int i=0; i<XBASE_GetItemCount(); i++)
		{
			SOFT_UNI_ITEM*	pItem = (SOFT_UNI_ITEM*)XBASE_GetItemData(i);
			
			if (pItem && pItem->pInfo==pInfo)
			{
				XBASE_DeleteItem(i);

				for (int k=0; k<m_arrItem.GetSize(); k++)
				{
					if ( pItem == m_arrItem[k])
					{
						m_arrItem.RemoveAt(k);
						break;
					}
				}
				delete pInfo;
				delete pItem;
				break;
			}
		}
	}

	int GetAllCount()
	{
		return m_arrItem.GetSize();
	}

	SOFT_UNI_ITEM* GetRunDataByIndex(int i)
	{
		return m_arrItem[i];
	}

	void ClearAll()
	{
		if (m_hWnd!=NULL)
			XBASE_DeleteAllItems();

		for (int i=0; i < m_arrItem.GetSize(); i++)
		{
			SOFT_UNI_ITEM*	pItem = (SOFT_UNI_ITEM*)m_arrItem[i];
			if (pItem)
			{
				pItem->pInfo->bShowInList = FALSE;
				delete pItem;
			}
		}
		m_arrItem.RemoveAll();

		m_nNameSort = SORT_TYPE_UNKNOW;
		m_nSizeSort = SORT_TYPE_UNKNOW;
		m_nTimeSort = SORT_TYPE_UNKNOW;
	}

	void UpdateAll()
	{
		if (m_hWnd!=NULL)
			XBASE_DeleteAllItems();

		for (int i=0; i < m_arrItem.GetSize(); i++)
		{
			SOFT_UNI_ITEM*	pItem = NULL;
			pItem = m_arrItem[i];
			pItem->nIndex = i;
			if (i == 0)
				pItem->bExpand = TRUE;
			else
				pItem->bExpand = FALSE;

			int nPosNew = XBASE_GetItemCount();
			int iPos = XBASE_InsertItem(nPosNew, pItem->pInfo->strKey);
			XBASE_SetItemData(iPos, (DWORD_PTR)pItem);
		}

		if (m_arrItem.GetSize())
			CListBox::SetItemHeight(0,XX_SOFT_UNI_LIST_ITEM_HEIGHT_EXP);

		InvalidateRect(NULL);
	}

	int GetShowCount()
	{
		return XBASE_GetItemCount();
	}

	void OnSize(UINT nType, CSize size)
	{
#define XX_MSG_DLG_WIDTH	300
#define XX_MSG_DLG_HEIGHT	100

		if (m_dlgMsg.IsWindow())
		{
			CRect	rc;
			GetWindowRect(&rc);

			m_dlgMsg.MoveWindow( 
				(rc.Width()-XX_MSG_DLG_WIDTH)/2, 
				(rc.Height()-XX_MSG_DLG_HEIGHT)/2,
				XX_MSG_DLG_WIDTH,
				XX_MSG_DLG_HEIGHT);
		}
		SetMsgHandled(FALSE);
	}

	void SetLoadShow(BOOL bShow)
	{
		if (bShow)
		{
			m_dlgMsg.SetItemVisible(1,TRUE);
			m_dlgMsg.ShowWindow(SW_SHOW);
		}
		else
		{
			m_dlgMsg.SetItemVisible(1,FALSE);
			m_dlgMsg.ShowWindow(SW_HIDE);
		}
	}

	void RefreshRight(LPCTSTR strKey)
	{
		for (int i=0; i<m_arrItem.GetSize(); i++)
		{
			SOFT_UNI_ITEM*	pItem = NULL;
			pItem = m_arrItem[i];

			if (pItem && pItem->pInfo && 
				pItem->pInfo->strKey.GetLength() != 0 &&
				pItem->pInfo->strKey.CompareNoCase(strKey) == 0)
			{
				RefreshRight(i);
				return;
			}
		}
	}

	void RefreshRight(int nIndex)
	{
		if (::IsWindow(m_hWnd))
		{
			CRect rcItem;
			XBASE_GetItemRect(nIndex, &rcItem);
			InvalidateRect(&rcItem, FALSE);
		}
	}
};