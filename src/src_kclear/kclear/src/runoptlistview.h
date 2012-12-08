
#pragma once

#include "kclear/irunoptimize.h"
#include "3rdparty/wtl/atlctrls.h"
#include "kuicolordef.h"
#include "kuires/kuifontpool.h"

#define __USE_BASE_LIST_VIEW	0

#define XX_RUN_LIST_ITEM_HEIGHT	45

#define XX_RUN_LIST_ITEM_HEIGHT_EXP	80
#define XX_RUN_LIST_ITEM_HEIGHT_COL	50


#define SUB_SHOW_TYPE_ALL		0
#define SUB_SHOW_TYPE_ENABLE	1
#define SUB_SHOW_TYPE_DISABLE	2

#define COL_STR_CPT_DEFAULT		RGB(0,0,0)
#define COL_STR_CPT_DISABLE		RGB(150,150,150)

#define COL_STR_STATE_ENABLE	RGB(0,153,0)
#define COL_STR_STATE_DISABLE	RGB(255,0,0)
#define COL_STR_ADVICE			RGB(0,0,0)
#define COL_STR_EXPAND			COL_STR_CPT_DISABLE

// #define COL_LNK_ACTION_DISABLE	RGB(128,10,10)
// #define COL_LNK_ACTION_DISABLE	RGB(128,10,10)

#define ICON_UNK_ID				-1
#define ICON_SYS_ID				-2

#define TYPE_ITEM_MAIN			1
#define TYPE_ITEM_DESC			2
#define TYPE_ITEM_ADVICE		3
#define TYPE_ITEM_APPPATH		4

#define WM_USER_MSG_POST_CLICK	(WM_APP+1)

#define CLICK_INDEX_INVALID			-1
#define CLICK_INDEX_NULL			0
#define CLICK_INDEX_ENABLE			1
#define CLICK_INDEX_DELETE			2
#define CLICK_INDEX_OPEN_FOLDER		3
#define CLICK_INDEX_CAPTION			4
#define CLICK_INDEX_LBL_FOLDER		5
#define CLICK_INDEX_STR_FOLDER		6
#define CLICK_INDEX_DESC			7

class IListOpCallback
{
public:
	virtual BOOL OnClickLabel(DWORD nIndex, CKsafeRunInfo* pInfo) = 0;
};

class CRunOptListView : 
#if __USE_BASE_LIST_VIEW
	public CWindowImpl<CRunOptListView, CListViewCtrl>
#else
	public CWindowImpl<CRunOptListView, CListBox>
#endif
	,public COwnerDraw<CRunOptListView>
{
public:

	struct _RUN_ITEM_DATA
	{
	public:
		_RUN_ITEM_DATA()
		{
			rcCaption	= CRect(0,0,0,0);
			rcDesc		= CRect(0,0,0,0);
			rcButton	= CRect(0,0,0,0);
			rcDelete	= CRect(0,0,0,0);
			rcOpenDir	= CRect(0,0,0,0);
			rcCmdLine	= CRect(0,0,0,0);
			nType		= TYPE_ITEM_MAIN;
			bExpand		= FALSE;
		}

		CKsafeRunInfo*	pInfo;
		CRect			rcCaption;
		CRect			rcDesc;
		CRect			rcButton;
		CRect			rcDelete;
		CRect			rcOpenDir;
		CRect			rcCmdLine;
		DWORD			nType;
		BOOL			bExpand;
	};

	CRunOptListView()
	{
		m_nShowType = KSRUN_TYPE_STARTUP;
		m_rcCaption	= CRect(50,8,100,20);
		m_rcDesc	= CRect(50,25,100,37);
		m_rcDesc2	= CRect(50,30,100,42);
		m_rcIcon	= CRect(10,(XX_RUN_LIST_ITEM_HEIGHT-32)/2,10+32,(XX_RUN_LIST_ITEM_HEIGHT+32)/2);
		m_rcAdvice	= CRect(50,45,100,57);
		m_rcAppPath	= CRect(50,52,100,64);
		m_opCallback= NULL;
		m_nSubShowType = SUB_SHOW_TYPE_ALL;
		m_bShowSystem= FALSE;
		m_hIconSys  = NULL;
		m_hIconUnk	= NULL;
		m_pHoverTip = 0;
	}

	~CRunOptListView(void)
	{
		ClearAll();
	
		if (m_hIconSys!=NULL)
		{
			::DestroyIcon(m_hIconSys);
			m_hIconSys = NULL;
		}

		if (m_hIconUnk!=NULL)
		{
			::DestroyIcon(m_hIconUnk);
			m_hIconUnk = NULL;
		}
	}

protected:
	CSimpleArray<_RUN_ITEM_DATA*> m_listArray;
	HICON						 m_hIconSys;
	HICON						 m_hIconUnk;
	CSimpleArray<HICON>			 m_listICON;
	CAtlMap<CKsafeRunInfo*,int>  m_mapIconIndex;
	DWORD						 m_nShowType;
	CRect						 m_rcCaption;
	CRect						 m_rcDesc;
	CRect						 m_rcDesc2;
	CRect						 m_rcIcon;
	CRect						 m_rcButton;
	CRect						 m_rcAdvice;
	CRect						 m_rcAppPath;
	CRect						 m_rcDelete;
	IListOpCallback*			 m_opCallback;
	DWORD						 m_nSubShowType;
	BOOL						 m_bShowSystem;
	CKuiDialogView				 m_dlgMsg;
	CToolTipCtrl				 m_wndToolTip;
	DWORD						 m_pHoverTip;
	CBrush						 m_hBGBrush;

public:
	BEGIN_MSG_MAP(CRunOptListView)   
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_RBUTTONUP(OnRButtonUp)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_SIZE(OnSize)
		MSG_OCM_CTLCOLORLISTBOX(OnCtlColor)
		MESSAGE_HANDLER_EX(WM_USER_MSG_POST_CLICK,OnMsgPostClick)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CRunOptListView>,1)
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

	LRESULT OnMsgPostClick(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		MsgDoExp((int)wParam);
		return S_OK;
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

	void SetOpCallback(IListOpCallback* pCallback)
	{
		m_opCallback = pCallback;
	}

	void GetCKSRunInfoArray(CSimpleArray<CKsafeRunInfo*>& arrayx)
	{
		for (int i=0; i < m_listArray.GetSize(); i++)
		{
			if (m_listArray[i]->pInfo)
			{
				arrayx.Add(m_listArray[i]->pInfo);
			}
		}
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

	void CreateX()
	{
		if ( !m_dlgMsg.Load(128) )
			return;
		m_dlgMsg.Create(m_hWnd);
	//yyx	SetBkColor(COL_DEFAULT_WHITE_BG);
		m_dlgMsg.ShowWindow(SW_HIDE);

		m_wndToolTip.Create(m_hWnd);

		CToolInfo ti(0, m_hWnd);
		m_wndToolTip.AddTool(ti);
		m_wndToolTip.Activate(TRUE);
		m_wndToolTip.SetMaxTipWidth(500);
		m_wndToolTip.SetDelayTime(TTDT_AUTOPOP,5*1000);
		m_wndToolTip.SetDelayTime(TTDT_INITIAL,500);
	}

	void SetLoadShow(BOOL bShow=TRUE)
	{
		if (bShow)
		{
			m_dlgMsg.SetItemVisible(1,TRUE);
			m_dlgMsg.SetItemVisible(2,FALSE);
			m_dlgMsg.ShowWindow(SW_SHOW);
		}
	}

	int XBASE_HitTest(CPoint ptx)
	{
#if __USE_BASE_LIST_VIEW
		LVHITTESTINFO hti;
		hti.pt = ptx;
		SubItemHitTest( &hti );
		return hti.iItem;
#else
		BOOL	bOut = FALSE;
		int i = (int)CListBox::ItemFromPoint(ptx,bOut);

		if (!bOut)
			return i;
		else
			return -1;
#endif

	}

	int XBASE_InsertItem(int iIndex, LPCTSTR lpstr)
	{
#if __USE_BASE_LIST_VIEW
		return CListViewCtrl::InsertItem(iIndex,lpstr);
#else
		return CListBox::InsertString(iIndex,lpstr);
#endif
	}

	BOOL XBASE_GetItemRect(int i, LPRECT lprc)
	{
#if __USE_BASE_LIST_VIEW
		return CListViewCtrl::GetItemRect(i,lprc,LVIR_BOUNDS);
#else
		return CListBox::GetItemRect(i,lprc);
#endif
	}


	BOOL XBASE_DeleteItem(int i)
	{
#if __USE_BASE_LIST_VIEW
		return CListViewCtrl::DeleteItem(i);
#else
		return CListBox::DeleteString((UINT)i);
#endif
	}

	int XBASE_GetItemCount()
	{
#if __USE_BASE_LIST_VIEW
		return CListViewCtrl::GetItemCount();
#else
		return CListBox::GetCount();
#endif
	}

	BOOL XBASE_DeleteAllItems()
	{
#if __USE_BASE_LIST_VIEW
		return CListViewCtrl::DeleteAllItems();
#else
		CListBox::ResetContent();
		return TRUE;
#endif
	}

	DWORD_PTR XBASE_GetItemData(int nItem)
	{
#if __USE_BASE_LIST_VIEW
		return CListViewCtrl::GetItemData(nItem);
#else
		DWORD_PTR pvoid = CListBox::GetItemData(nItem);

		if ((DWORD)pvoid==-1)
			return NULL;
		return pvoid;
#endif
	}

	BOOL XBASE_SetItemData(int nItem,DWORD_PTR pdata)
	{
#if __USE_BASE_LIST_VIEW
		return CListViewCtrl::SetItemData(nItem,pdata);
#else
		return CListBox::SetItemData(nItem,pdata);
#endif
	}

	BOOL XBASE_SetItemText(int nItem, LPCTSTR lpstr)
	{
#if __USE_BASE_LIST_VIEW
		return CListViewCtrl::SetItemText(nItem,0,lpstr);
#else
		return FALSE;
#endif
	}

	int LabelHitTest(CPoint ptx)
	{
		int		iItem = XBASE_HitTest(ptx);

		CRect	rcItem;
		_RUN_ITEM_DATA* pdata = (_RUN_ITEM_DATA*)XBASE_GetItemData(iItem);

		if (pdata==NULL)
			return CLICK_INDEX_INVALID;
		
		XBASE_GetItemRect(iItem,&rcItem);
		ptx.Offset(-rcItem.left,-rcItem.top);

		if (pdata->rcButton.PtInRect(ptx))
			return CLICK_INDEX_ENABLE;
		else if (pdata->rcCaption.PtInRect(ptx))
			return CLICK_INDEX_CAPTION;
		else if (pdata->rcDelete.PtInRect(ptx))
			return CLICK_INDEX_DELETE;
		else if (pdata->rcDesc.PtInRect(ptx))
			return CLICK_INDEX_DESC;
		else if (pdata->rcOpenDir.PtInRect(ptx) )
			return CLICK_INDEX_OPEN_FOLDER;
		else if (pdata->rcCmdLine.PtInRect(ptx) )
			return CLICK_INDEX_LBL_FOLDER;
		else
			return CLICK_INDEX_NULL;
	}

	void OnLButtonDown(UINT uMsg, CPoint ptx)
	{
		BOOL	bHandle = FALSE;
		int		iItem = XBASE_HitTest(ptx);

		_RUN_ITEM_DATA* pdata = (_RUN_ITEM_DATA*)XBASE_GetItemData(iItem);

		if (pdata!=NULL && pdata->pInfo)
		{
			CKsafeRunInfo*	pInfo = pdata->pInfo;
			int iIndex = LabelHitTest(ptx);

			if (iIndex == CLICK_INDEX_ENABLE || iIndex== CLICK_INDEX_OPEN_FOLDER)
			{
				m_opCallback->OnClickLabel(iIndex,pInfo);
				bHandle = TRUE;
			}
			
			if (!bHandle)
			{
				if (CListBox::GetCurSel()==LB_ERR || iItem==CListBox::GetCurSel())
					PostMessage(WM_USER_MSG_POST_CLICK,(WPARAM)iItem,NULL);
			}
		}
	
		SetMsgHandled(bHandle);
		return;
	}

	void OnRButtonUp(UINT uMsg, CPoint ptx)
	{
		int i = XBASE_HitTest(ptx);
		_RUN_ITEM_DATA* pdata = (_RUN_ITEM_DATA*)XBASE_GetItemData(i);
		if (i>=0 && pdata)
		{
			int iIndex = LabelHitTest(ptx);
			if (iIndex!=CLICK_INDEX_ENABLE&&
				iIndex!=CLICK_INDEX_OPEN_FOLDER)
			{
				CString	strFormat;
				if (pdata->pInfo->nType==KSRUN_TYPE_SERVICE)
					strFormat = (pdata->pInfo->bEnable)?_T("开机不启动"):_T("开机启动");
				else
					strFormat = (pdata->pInfo->bEnable)?_T("禁止启动"):_T("允许启动");

				CMenu	menu;
				menu.CreatePopupMenu();
				menu.AppendMenu(MF_BYCOMMAND,CLICK_INDEX_OPEN_FOLDER,_T("打开所在文件夹"));
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_BYCOMMAND,CLICK_INDEX_ENABLE,strFormat);
				menu.AppendMenu(MF_BYCOMMAND,CLICK_INDEX_DELETE,_T("删除此项"));
				CPoint	ptx;
				::GetCursorPos(&ptx);
				int i = menu.TrackPopupMenu(TPM_RETURNCMD,ptx.x,ptx.y, m_hWnd);

				if (i==CLICK_INDEX_ENABLE||
					i==CLICK_INDEX_DELETE||
					i==CLICK_INDEX_OPEN_FOLDER)
				{
					m_opCallback->OnClickLabel(i,pdata->pInfo);
				}
			}
		}
		SetMsgHandled(TRUE);
	}

	void OnLButtonUp(UINT uMsg, CPoint ptx)
	{
		SetMsgHandled(FALSE);
	}

	void MsgDoExp(int iItem)
	{
		_RUN_ITEM_DATA* pdata = (_RUN_ITEM_DATA*)XBASE_GetItemData(iItem);
		
		SetRedraw(FALSE);

		CPoint	ptx;
		::GetCursorPos(&ptx);

		CRect	rcOffset(0,0,0,0);
		ScreenToClient(rcOffset);
		ptx.Offset(rcOffset.left,rcOffset.top);
		int nShowIndex = XBASE_HitTest(ptx);
	

		if ( true )
		{
			if (!pdata->bExpand)
			{
				ExpandAllItem(TRUE);
				//	ExpandItem(iItem);
			}
			else
			{
				ExpandAllItem(FALSE);

				//	ExpandItem(iItem,FALSE);
			}
		}

		int nTopIndex = CListBox::GetTopIndex();
		int nItemHeight = (pdata->bExpand?XX_RUN_LIST_ITEM_HEIGHT_EXP:XX_RUN_LIST_ITEM_HEIGHT_COL);
		int nCursel = CListBox::GetCurSel();
		int nNewPtSel = ptx.y/nItemHeight;
		CRect	rcClinet;

		nNewPtSel+=nTopIndex;
		nTopIndex = nTopIndex+(nCursel-nNewPtSel);

		GetClientRect(&rcClinet);
		int npage = rcClinet.Height()/nItemHeight;

		if (nTopIndex < 0 )
			nTopIndex = 0;
		CListBox::SetTopIndex(nTopIndex);
	

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
		_RUN_ITEM_DATA* pdata = (_RUN_ITEM_DATA*)XBASE_GetItemData(iItem);
		
#if __USE_BASE_LIST_VIEW
		if (bExp && !pdata->bExpand)
		{
			if (TRUE)
			{
				_RUN_ITEM_DATA* pNewData = new _RUN_ITEM_DATA(*pdata);
				pNewData->nType = TYPE_ITEM_DESC;
				XBASE_InsertItem(iItem+1,NULL);
				XBASE_SetItemData(iItem+1, (DWORD_PTR)pNewData);
			}
			if (TRUE)
			{
				_RUN_ITEM_DATA* pNewData = new _RUN_ITEM_DATA(*pdata);
				pNewData->nType = TYPE_ITEM_ADVICE;
				XBASE_InsertItem(iItem+2,NULL);
				XBASE_SetItemData(iItem+2, (DWORD_PTR)pNewData);
			}
			if (FALSE)
			{
				_RUN_ITEM_DATA* pNewData = new _RUN_ITEM_DATA(*pdata);
				pNewData->nType = TYPE_ITEM_APPPATH;
				XBASE_InsertItem(iItem+3,NULL);
				XBASE_SetItemData(iItem+3, (DWORD_PTR)pNewData);
			}
			pdata->bExpand = TRUE;
		}
		else if ( !bExp && pdata->bExpand )
		{
			if (TRUE)
			{
				_RUN_ITEM_DATA* pNewdata = (_RUN_ITEM_DATA*)XBASE_GetItemData(iItem+1);
				delete pNewdata;
				XBASE_DeleteItem(iItem+1);
			}
			if (TRUE)
			{
				_RUN_ITEM_DATA* pNewdata = (_RUN_ITEM_DATA*)XBASE_GetItemData(iItem+1);
				delete pNewdata;
				XBASE_DeleteItem(iItem+1);
			}
			if (FALSE)
			{
				_RUN_ITEM_DATA* pNewdata = (_RUN_ITEM_DATA*)XBASE_GetItemData(iItem+1);
				delete pNewdata;
				XBASE_DeleteItem(iItem+1);
			}
			pdata->bExpand = FALSE;
		}

#else
		if (bExp && !pdata->bExpand)
		{
			pdata->bExpand = TRUE;
			CListBox::SetItemHeight(iItem,XX_RUN_LIST_ITEM_HEIGHT_EXP);
		}
		else if ( !bExp && pdata->bExpand )
		{
			pdata->bExpand = FALSE;
			CListBox::SetItemHeight(iItem,XX_RUN_LIST_ITEM_HEIGHT_COL);
		}
#endif
	}
	CKsafeRunInfo* GetDataInfo(int nItem)
	{
		_RUN_ITEM_DATA*	pParam = (_RUN_ITEM_DATA*)XBASE_GetItemData(nItem);

		if (pParam!=NULL && (DWORD)(ULONG_PTR)pParam != -1 )
			return pParam->pInfo;
		else
			return NULL;
	}

	void OnMouseMove(UINT uMsg, CPoint ptx)
	{
		int			iItem = XBASE_HitTest(ptx);

		CKsafeRunInfo*	kinfo		= GetDataInfo(iItem);
		_RUN_ITEM_DATA*	pItemData	= (_RUN_ITEM_DATA*)XBASE_GetItemData(iItem);
		
		if (kinfo && pItemData)
		{
			int iIndex = LabelHitTest(ptx);
			if (iIndex == CLICK_INDEX_ENABLE)
				::SetCursor(::LoadCursor(NULL,IDC_HAND));
			else if (iIndex == CLICK_INDEX_OPEN_FOLDER )
				::SetCursor(::LoadCursor(NULL,IDC_HAND));
			
			if (iIndex==CLICK_INDEX_DESC||
				iIndex==CLICK_INDEX_LBL_FOLDER)
			{
				if ( ((DWORD)(ULONG_PTR)kinfo!=m_pHoverTip) && !kinfo->strExePath.IsEmpty() )
				{
					CString	strInfo;

					if (iIndex==CLICK_INDEX_DESC)
						strInfo.Append( kinfo->strDesc.IsEmpty()?_T("暂时没有简介"):kinfo->strDesc );
					else if (iIndex==CLICK_INDEX_LBL_FOLDER )
						strInfo.Append( kinfo->strParam );

					m_wndToolTip.SetMaxTipWidth(500);
					m_wndToolTip.UpdateTipText((LPCTSTR)strInfo,m_hWnd);
					m_pHoverTip = (DWORD)(ULONG_PTR)kinfo;
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
	}

	void DeleteItem(LPDELETEITEMSTRUCT lParam)
	{
		return;
	}
	HICON FindIcon(CKsafeRunInfo* pinfo)
	{
		int nIndex = 0;
		
		if ( m_mapIconIndex.Lookup(pinfo,nIndex) )
		{
			if (nIndex==ICON_SYS_ID)
			{
				if (m_hIconSys==NULL)
					m_hIconSys = ::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(102));
				return m_hIconSys;
			}
			else if (nIndex==ICON_UNK_ID)
			{
				if (m_hIconUnk==NULL)
					m_hIconUnk = ::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(103));
				return m_hIconUnk;
			}
			if (m_listICON.GetSize() > nIndex && nIndex>=0 )
				return m_listICON[nIndex];
			else
				return NULL;
		}
		return NULL;
	}

	int GetItemIndexByKSInfo(CKsafeRunInfo* pdata)
	{
		for ( int i=0; i<XBASE_GetItemCount(); i++)
		{
			if (GetDataInfo(i)==pdata)
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

	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		_RUN_ITEM_DATA*	pItemData = (_RUN_ITEM_DATA*)XBASE_GetItemData(lpDrawItemStruct->itemID);
		CKsafeRunInfo*	pInfo = GetDataInfo(lpDrawItemStruct->itemID);
		CDCHandle		dcx;
		COLORREF		clrOld;
		CRect			rcWin;

		GetWindowRect(&rcWin);
		dcx.Attach(lpDrawItemStruct->hDC);
		HFONT fntdef = KuiFontPool::GetFont(KUIF_DEFAULTFONT);

		HFONT fntdeftmp = dcx.SelectFont(fntdef);
		clrOld = dcx.GetTextColor();

		if (pInfo && pItemData)
		{
			CRect	rcItemX = lpDrawItemStruct->rcItem;

			rcItemX.right = rcItemX.left + rcWin.Width()-20;
			
			if (lpDrawItemStruct->itemState&ODS_SELECTED)
				dcx.FillSolidRect(&rcItemX,RGB(216,241,212));
			else
				dcx.FillSolidRect(&rcItemX,COL_DEFAULT_WHITE_BG);

			CRect	rcFirst;
			CRect	rcSecond;
			int		nSecWidth = (pInfo->nType==KSRUN_TYPE_SERVICE)?250:200;

			rcFirst = rcItemX;
			rcFirst.right -= nSecWidth;
			rcSecond= rcItemX;
			rcSecond.left = rcFirst.right;

			if (pItemData->nType == TYPE_ITEM_MAIN )
			{
				HICON	hIcon = FindIcon(pInfo);
				if (hIcon)
				{
					CRect	rcIcon=m_rcIcon;
					rcIcon.OffsetRect(rcFirst.left,rcFirst.top);
					//dcx.DrawIcon(rcIcon.left,rcIcon.top,hIcon);
					::DrawIconEx(dcx,rcIcon.left,rcIcon.top,hIcon,32,32,0,NULL,DI_NORMAL);
				}

				dcx.SetTextColor(pInfo->bEnable?COL_STR_CPT_DEFAULT:COL_STR_CPT_DISABLE);
				dcx.SetBkMode(TRANSPARENT);

				if (TRUE)
				{
					CRect	rcCaption=m_rcCaption;
					rcCaption.left+=rcFirst.left;
					rcCaption.top+=rcFirst.top;
					rcCaption.bottom=rcCaption.top+m_rcCaption.Height();
					rcCaption.right=rcFirst.right;

					CRect	rcProbe;
					dcx.DrawText(pInfo->strDisplay,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
					dcx.DrawText(pInfo->strDisplay,-1,&rcCaption,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);
					m_rcCaption.right = rcFirst.Width();

					rcProbe.bottom = rcProbe.top+12;
					rcCaption.OffsetRect(-rcFirst.left, -rcFirst.top);
					calcLeftRECT(rcCaption,rcProbe,pItemData->rcCaption);		

					if (!pItemData->bExpand)
					{
						CRect	rcDesc=m_rcDesc;
						rcDesc.left+=rcFirst.left;
						rcDesc.top+=rcFirst.top;
						rcDesc.bottom=rcDesc.top+m_rcDesc.Height();
						rcDesc.right=rcFirst.right;

						CRect	rcProbe;
						CString	strShow;				
						strShow.Format(_T("%s"), pInfo->strDesc.IsEmpty()?_T("暂时没有简介"):pInfo->strDesc);
						dcx.DrawText(strShow,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
						dcx.DrawText(strShow,-1,&rcDesc,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);

						rcProbe.bottom = rcProbe.top+12;
						rcDesc.OffsetRect(-rcFirst.left, -rcFirst.top);
						calcLeftRECT(rcDesc,rcProbe,pItemData->rcDesc);
					}
				}
				if (TRUE)
				{
					// #define LENGTH_ADVICE	0//80
					// #define LENGTH_STATE	38
					// #define LENGTH_ACTION	25
					// #define LENGTH_MARGIN	20

					DWORD	LENGTH_ADVICE	= 80;
					DWORD	LENGTH_STATE	= 38;
					DWORD	LENGTH_ACTION	= 25;
					DWORD	LENGTH_MARGIN	= 20;

					if (pInfo->nType==KSRUN_TYPE_SERVICE)
					{
						if (pInfo->bEnable)
						{
							LENGTH_STATE  = 50;
							LENGTH_ACTION = 60;
						}
						else
						{
							LENGTH_STATE  = 60;
							LENGTH_ACTION = 50;
						}
					}

					DWORD	nTLength = LENGTH_ADVICE+LENGTH_STATE+LENGTH_ACTION+LENGTH_MARGIN*2;
					CRect	rcState=rcSecond;
					CString	strState;

					rcState.left = rcState.left + rcSecond.Width()-10-nTLength;
					rcState.right= rcState.left+LENGTH_ADVICE;

					if ( !pItemData->bExpand)
					{
						rcState.top		= rcSecond.top+(XX_RUN_LIST_ITEM_HEIGHT_COL-12)/2;
						rcState.bottom	= rcState.top+12;
					}
					else
					{
						rcState.top += m_rcCaption.top;
						rcState.bottom = rcState.top+12;
					}

					dcx.SetTextColor(COL_STR_ADVICE);
					strState.Format(_T("%s"), pInfo->GetAdviceStr());
					dcx.DrawText(strState,-1,&rcState,DT_VCENTER|DT_SINGLELINE|DT_CENTER);

					rcState.left = rcState.right+LENGTH_MARGIN;
					rcState.right= rcState.left+LENGTH_STATE;

					

					dcx.SetTextColor(pInfo->bEnable?COL_STR_STATE_ENABLE:COL_STR_STATE_DISABLE);

					if ( pInfo->nType != KSRUN_TYPE_SERVICE )
						strState.Format(_T("%s"), pInfo->bEnable?_T("已允许"):_T("已禁止"));
					else
						strState.Format(_T("%s"), pInfo->bEnable?_T("开机启动"):_T("开机不启动"));

					dcx.DrawText(strState,-1,&rcState,DT_VCENTER|DT_SINGLELINE|DT_CENTER);

					rcState.left = rcState.right+LENGTH_MARGIN;
					rcState.right= rcState.left+LENGTH_ACTION;

					dcx.SetTextColor(COL_DEFAULT_LINK);

					if ( pInfo->nType != KSRUN_TYPE_SERVICE )
						strState.Format(_T("%s"), pInfo->bEnable?_T("禁止"):_T("允许"));
					else
						strState.Format(_T("%s"), pInfo->bEnable?_T("开机不启动"):_T("开机启动"));

					HFONT hfnt = KuiFontPool::GetFont(FALSE,TRUE,FALSE);
					HFONT hTmp = dcx.SelectFont(hfnt);
					dcx.DrawText(strState,-1,&rcState,DT_VCENTER|DT_SINGLELINE|DT_CENTER);
					dcx.SelectFont(hTmp);
					pItemData->rcButton = rcState;
					pItemData->rcButton.OffsetRect(-lpDrawItemStruct->rcItem.left,-lpDrawItemStruct->rcItem.top);
				}

				if ( pItemData->bExpand )
				{	
					if ( FALSE )
					{
						CRect	rcAdvice=m_rcAdvice;
						rcAdvice.left+=rcFirst.left;
						rcAdvice.top+=rcFirst.top;
						rcAdvice.bottom=rcAdvice.top+m_rcDesc.Height();
						rcAdvice.right=rcFirst.right;

						CString	strShow;				
						strShow.Format(_T("操作建议：%s"), pInfo->GetAdviceStr() );
						dcx.DrawText(strShow,-1,&rcAdvice,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);
					}
					
					dcx.SetTextColor(COL_STR_EXPAND);
					if (TRUE)
					{
						CRect	rcDesc=m_rcDesc2;
						rcDesc.left+=rcFirst.left;
						rcDesc.top+=rcFirst.top;
						rcDesc.bottom=rcDesc.top+m_rcDesc.Height();
						rcDesc.right=rcSecond.right;

						CRect	rcProbe;
						CString	strShow;				
						strShow.Format(_T("项目简介：%s"), pInfo->strDesc.IsEmpty()?_T("暂时没有简介"):pInfo->strDesc);
						dcx.DrawText(strShow,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
						dcx.DrawText(strShow,-1,&rcDesc,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);

						rcProbe.bottom = rcProbe.top+12;
						rcDesc.OffsetRect(-rcFirst.left, -rcFirst.top);
						calcLeftRECT(rcDesc,rcProbe,pItemData->rcDesc);
					}

					if ( TRUE )
					{
#define MARGIN_OPEN_FOLDER	10
#define LENGTH_OPEN_FOLDER	50
						CRect	rcApp=m_rcAppPath;
						rcApp.left+=rcFirst.left;
						rcApp.top+=rcFirst.top;
						rcApp.bottom=rcApp.top+m_rcDesc.Height();
						rcApp.right=rcSecond.right-LENGTH_OPEN_FOLDER-MARGIN_OPEN_FOLDER*2;

						CString	strShow;				
						CRect	rcProbe;
						strShow.Format(_T("启动参数：%s"), pInfo->strParam);
						dcx.DrawText(strShow,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
						dcx.DrawText(strShow,-1,&rcApp,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);

						CRect rcAppX = rcApp;
						rcProbe.bottom = rcProbe.top+12;
						rcAppX.OffsetRect(-rcFirst.left, -rcFirst.top);
						calcLeftRECT(rcAppX,rcProbe,pItemData->rcCmdLine);

						/*
						CRect	rcOpen=rcApp;

						if (rcProbe.Width() > rcApp.Width() )
							rcOpen.left  = rcApp.right+MARGIN_OPEN_FOLDER;
						else
							rcOpen.left  = rcApp.left+rcProbe.Width()+MARGIN_OPEN_FOLDER;

						rcOpen.right = rcOpen.left+LENGTH_OPEN_FOLDER;

						strShow.Format(_T("查看文件"));
						dcx.SetTextColor(COL_DEFAULT_LINK);
						HFONT hfnt = BkFontPool::GetFont(FALSE,TRUE,FALSE);
						HFONT hTmp = dcx.SelectFont(hfnt);
						dcx.DrawText(strShow,-1,rcOpen,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);
						dcx.SelectFont(hTmp);

						rcOpen.OffsetRect(-rcFirst.left, -rcFirst.top);
						pItemData->rcOpenDir = rcOpen;
						*/
					}
				}
			}
	
			if ( (pItemData->nType==TYPE_ITEM_MAIN) ||
				 (pItemData->nType==TYPE_ITEM_APPPATH) )
			{
				CPen	penx;
				penx.CreatePen(PS_SOLID,1,RGB(200,200,200));
				HPEN	pentmp = dcx.SelectPen(penx);
				dcx.MoveTo(rcItemX.left,rcItemX.bottom-1);
				dcx.LineTo(rcItemX.right,rcItemX.bottom-1);
				dcx.SelectPen(pentmp);
			}
		}
		dcx.SelectFont(fntdeftmp);
		dcx.Detach();
	}

	void MeasureItem(LPMEASUREITEMSTRUCT lParam)
	{
		_RUN_ITEM_DATA*	px = (_RUN_ITEM_DATA*)XBASE_GetItemData(lParam->itemID);

		if (px && px->bExpand)
			lParam->itemHeight = XX_RUN_LIST_ITEM_HEIGHT_EXP;
		else
			lParam->itemHeight = XX_RUN_LIST_ITEM_HEIGHT_COL;
	}

	void InsertItemX(CKsafeRunInfo* pInfo, HICON hIcon)
	{
		_RUN_ITEM_DATA*	pX = new _RUN_ITEM_DATA;
		pX->pInfo = pInfo;		
		
		if (IsItemCanShow(pInfo,m_nShowType,m_nSubShowType,m_bShowSystem))
		{
			int iPos = XBASE_InsertItem(GetInsertIndex(pInfo),pInfo->strDisplay);
			XBASE_SetItemText(iPos,pInfo->strDisplay);
			XBASE_SetItemData(iPos,(DWORD_PTR)pX);
		}

		m_listArray.Add(pX);
		if (hIcon!=NULL)
		{
			m_mapIconIndex[pInfo] = m_listICON.GetSize();
			m_listICON.Add(hIcon);
		}
		else if (pInfo->bSystem)
			m_mapIconIndex[pInfo] = ICON_SYS_ID;
		else
			m_mapIconIndex[pInfo] = ICON_UNK_ID;
	}

	BOOL isItemInList(CKsafeRunInfo* pInfo)
	{
		for (int i=0; i<XBASE_GetItemCount(); i++)
		{
			if ( pInfo == GetDataInfo(i) )
				return TRUE;
		}
		return FALSE;
	}

	BOOL IsItemCanShow(CKsafeRunInfo* pInfo, DWORD nShowType, DWORD nSubShowType, BOOL bShowSys)
	{
		if (pInfo->nType==nShowType)
		{
			if (pInfo->nType==KSRUN_TYPE_SERVICE )
			{
				if ( (!bShowSys&&pInfo->bSystem) )
					return FALSE;
			}

			if (nSubShowType==SUB_SHOW_TYPE_ALL)
				return TRUE;
			else if (nSubShowType==SUB_SHOW_TYPE_ENABLE)
				return (pInfo->bEnable);
			else if (nSubShowType==SUB_SHOW_TYPE_DISABLE)
				return (!pInfo->bEnable);
		}
		return FALSE;
	}

	int GetInsertIndex(CKsafeRunInfo* pInfo)
	{
		int i=0;
		for (i=0; i<XBASE_GetItemCount(); i++)
		{
			CKsafeRunInfo* pItemInfo = GetDataInfo(i);
			if ( pInfo->bEnable && !pItemInfo->bEnable )
				return i;
			else if( (pItemInfo->bEnable&&pInfo->bEnable) || (!pInfo->bEnable&&!pItemInfo->bEnable) )
			{
				if ( pInfo->strDisplay.CompareNoCase(pItemInfo->strDisplay) <= 0 )
					return i;
			}
		}
		return i;
	}

	DWORD GetShowType()
	{
		return m_nShowType;
	}

	void SetShowSystem(BOOL bShow)
	{
		m_bShowSystem = bShow;
		SetShowType(m_nShowType,m_nSubShowType);
	}

	void SetSubShowType(DWORD nsubType)
	{
		SetShowType(m_nShowType,nsubType);
	}

	void ResetExpandState(BOOL bExp=FALSE)
	{
		for (int i=0; i<m_listArray.GetSize(); i++)
		{
			m_listArray[i]->bExpand = bExp;
		}
	}

	void SetShowType(DWORD nType, DWORD nSubShowType=SUB_SHOW_TYPE_ALL)
	{
		SetRedraw(FALSE);
		ResetExpandState();
		if (m_nShowType!=nType)
		{
			m_bShowSystem = FALSE;
			DeleteAllItemX();
			for (int i=0; i<m_listArray.GetSize(); i++)
			{
				CKsafeRunInfo*	pInfoX = m_listArray[i]->pInfo;
				if (pInfoX->nType==nType && !pInfoX->bSystem)
				{
					int k = XBASE_InsertItem(GetInsertIndex(pInfoX),pInfoX->strDisplay);
					XBASE_SetItemText(k,pInfoX->strDisplay);
					XBASE_SetItemData(k,(DWORD_PTR)m_listArray[i]);
				}
			}
		}
		else
		{
			for (int i=XBASE_GetItemCount()-1; i>=0;i--)
			{
				if (!IsItemCanShow(GetDataInfo(i),nType,nSubShowType,m_bShowSystem))
				{
					XBASE_DeleteItem(i);
				}
			}

			for ( int i= 0; i<m_listArray.GetSize(); i++)
			{
				CKsafeRunInfo*	pInfoX = m_listArray[i]->pInfo;
				if (IsItemCanShow(pInfoX,nType,nSubShowType,m_bShowSystem))
				{
					if (!isItemInList(pInfoX))
					{
						int iPos = GetInsertIndex(pInfoX);
						iPos = XBASE_InsertItem(iPos,pInfoX->strDisplay);
						XBASE_SetItemData(iPos,(DWORD_PTR)m_listArray[i]);
					}
				}
			}
		}
		SetRedraw(TRUE);

		m_nShowType = nType;
		m_nSubShowType = nSubShowType;
		SetShowDlgState();
	}

	void SetShowDlgState()
	{
		if (XBASE_GetItemCount()==0)
		{
			CString	strFormat;

			if (m_nShowType==KSRUN_TYPE_STARTUP)
			{
				if (m_nSubShowType==SUB_SHOW_TYPE_ALL)
					strFormat = _T("启动项中没有数据!");
				else if (m_nSubShowType==SUB_SHOW_TYPE_DISABLE)
					strFormat = _T("没有已禁止的启动项数据!");
				else if (m_nSubShowType==SUB_SHOW_TYPE_ENABLE)
					strFormat = _T("没有已允许的启动项数据!");
			}
			else if ( m_nShowType==KSRUN_TYPE_SERVICE )
			{
				if (m_nSubShowType==SUB_SHOW_TYPE_ALL)
					strFormat = _T("服务项中没有数据!");
				else if (m_nSubShowType==SUB_SHOW_TYPE_DISABLE)
					strFormat = _T("没有已禁止的服务项数据!");
				else if (m_nSubShowType==SUB_SHOW_TYPE_ENABLE)
					strFormat = _T("没有已允许的服务项数据!");
			}
			else if ( m_nShowType==KSRUN_TYPE_TASK )
			{
				if (m_nSubShowType==SUB_SHOW_TYPE_ALL)
					strFormat = _T("计划任务中没有数据!");
				else if (m_nSubShowType==SUB_SHOW_TYPE_DISABLE)
					strFormat = _T("没有已禁止的计划任务数据!");
				else if (m_nSubShowType==SUB_SHOW_TYPE_ENABLE)
					strFormat = _T("没有已允许的计划任务数据!");
			}

			m_dlgMsg.SetItemVisible(10,10==m_nShowType);
			m_dlgMsg.SetItemVisible(1,FALSE);
			m_dlgMsg.SetItemVisible(2,TRUE);
			m_dlgMsg.SetItemText(3,strFormat);
			m_dlgMsg.ShowWindow(SW_SHOW);
		}
		else
			m_dlgMsg.ShowWindow(SW_HIDE);
	}

	void DeleteItemByKSInfo(CKsafeRunInfo* pInfo)
	{
		for (int i=0; i<XBASE_GetItemCount(); i++)
		{
			_RUN_ITEM_DATA*	px = (_RUN_ITEM_DATA*)XBASE_GetItemData(i);
			
			if (px && px->pInfo==pInfo)
			{
				XBASE_DeleteItem(i);

				for (int k=0; k<m_listArray.GetSize(); k++)
				{
					if ( px == m_listArray[k])
					{
						m_listArray.RemoveAt(k);
						break;
					}
				}
				delete pInfo;
				delete px;
				break;
			}
		}
	}

	void DeleteAllItemX()
	{
		for ( int i=0; i < XBASE_GetItemCount(); i++)
		{
			_RUN_ITEM_DATA*	px = (_RUN_ITEM_DATA*)XBASE_GetItemData(i);

			if (px->nType!=TYPE_ITEM_MAIN)
				delete px;
		}
		XBASE_DeleteAllItems();
	}

	int GetAllCount()
	{
		return m_listArray.GetSize();
	}

	_RUN_ITEM_DATA* GetRunDataByIndex(int i)
	{
		return m_listArray[i];
	}

	void ClearAll()
	{
		if (m_hWnd!=NULL)
			DeleteAllItemX();
		for (int i=0; i<m_listArray.GetSize(); i++)
		{
			_RUN_ITEM_DATA*	px = (_RUN_ITEM_DATA*)m_listArray[i];

			if (px && px->pInfo)
				delete px->pInfo;
			
			if (px)
				delete px;
		}
		m_listArray.RemoveAll();

		for (int i=0; i<m_listICON.GetSize(); i++)
		{
			if (m_listICON[i]!=NULL)
				::DestroyIcon(m_listICON[i]);
		}
		m_mapIconIndex.RemoveAll();
		m_listICON.RemoveAll();
	}

	int GetShowCount()
	{
		return XBASE_GetItemCount();
	}

	BOOL IsShowSystem()
	{
		return m_bShowSystem;
	}

	int GetEnableTypeCount(DWORD nType, BOOL bIncSystem = FALSE)//包含系统
	{
		int iCount = 0;
		for (int i=0; i<m_listArray.GetSize(); i++)
		{
			CKsafeRunInfo*	pInfo = m_listArray[i]->pInfo;
			if (pInfo->nType==nType && pInfo->bEnable )
			{
				if (nType==KSRUN_TYPE_SERVICE)
				{
					if (!bIncSystem)
					{
						if (!pInfo->bSystem)
							iCount++;
					}
					else
						iCount++;

				}
				else
					iCount++;
			}
		}
		return iCount;
	}

	int GetDefShowTypeCount(DWORD nType, BOOL bIncSystem = FALSE)//包含系统
	{
		int iCount = 0;
		for (int i=0; i<m_listArray.GetSize(); i++)
		{
			CKsafeRunInfo*	pInfo = m_listArray[i]->pInfo;
			if (pInfo->nType==nType)
			{
				if (nType==KSRUN_TYPE_SERVICE)
				{
					if (!bIncSystem)
					{
						if (!pInfo->bSystem)
							iCount++;
					}
					else
						iCount++;
				}
				else
					iCount++;
			}
		}
		return iCount;
	}
};
