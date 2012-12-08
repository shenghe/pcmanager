
#pragma once

#include <runoptimize/interface.h>
#include <wtl/atlctrls.h>
#include "beikecolordef.h"
#include "loadruninfodlg.h"

#define __USE_BASE_LIST_VIEW	0

#define XX_RUN_LIST_ITEM_HEIGHT	45

#define XX_RUN_LIST_ITEM_HEIGHT_EXP	98
#define XX_RUN_LIST_ITEM_HEIGHT_COL	49

#define SUB_SHOW_TYPE_ALL		0
#define SUB_SHOW_TYPE_ENABLE	1
#define SUB_SHOW_TYPE_DISABLE	2

#define COL_STR_CPT_DEFAULT		RGB(0,0,0)
#define COL_STR_CPT_DISABLE		RGB(150,150,150)
#define COL_ITEM_SELECT			RGB(235,245,255)
#define COL_STR_STATE_ENABLE	RGB(0,153,0)
#define COL_STR_STATE_DISABLE	RGB(255,0,0)
#define COL_STR_ADVICE			RGB(0,0,0)
#define COL_STR_EXPAND			COL_STR_CPT_DISABLE
#define COL_STR_FILENOTEXIST	COL_STR_STATE_DISABLE

#define ICON_UNK_ID				-1
#define ICON_SYS_ID				-2
#define ICON_REMAIN				-3

#define TYPE_ITEM_MAIN			1
#define TYPE_ITEM_DESC			2
#define TYPE_ITEM_ADVICE		3
#define TYPE_ITEM_APPPATH		4

#define WM_USER_MSG_POST_CLICK	(WM_APP+1)

#define CLICK_INDEX_INVALID			-1
#define CLICK_INDEX_NULL			0
#define CLICK_INDEX_ENABLE			1
#define CLICK_INDEX_DELAY			2
#define CLICK_INDEX_DISABLE			3
#define CLICK_INDEX_DELETE			4
#define CLICK_INDEX_OPEN_FOLDER		5
#define CLICK_INDEX_CAPTION			6
#define CLICK_INDEX_LBL_FOLDER		7
#define CLICK_INDEX_STR_FOLDER		8
#define CLICK_INDEX_DESC			9

#define	CLICK_INDEX_MENUBTN			11

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
			rcMenuBtn	= CRect(0,0,0,0);
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
		CRect			rcMenuBtn;
		DWORD			nType;
		BOOL			bExpand;
	};

	CRunOptListView()
	{
		m_nShowType = KSRUN_TYPE_STARTUP;
		m_rcCaption	= CRect(58,8,108,20);
		m_rcDesc	= CRect(58,28,108,40);
		m_rcDesc2	= CRect(58,30,108,42);
		m_rcIcon	= CRect(10,(XX_RUN_LIST_ITEM_HEIGHT-32)/2,10+32,(XX_RUN_LIST_ITEM_HEIGHT+32)/2);
		m_rcAdvice	= CRect(58,48,108,60);
		m_rcAppPath	= CRect(58,48,108,60);
		m_opCallback= NULL;
		m_nSubShowType = SUB_SHOW_TYPE_ALL;
		m_bShowSystem= FALSE;
		m_hIconSys  = NULL;
		m_hIconUnk	= NULL;
		m_hIconRemian = NULL;
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

		if (m_hIconRemian!=NULL)
		{
			::DestroyIcon(m_hIconRemian);
			m_hIconRemian = NULL;
		}
	}

protected:
	CSimpleArray<_RUN_ITEM_DATA*> m_listArray;
	HICON						 m_hIconSys;
	HICON						 m_hIconUnk;
	HICON						 m_hIconRemian;
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
	CToolTipCtrl				 m_wndToolTip;
	DWORD						 m_pHoverTip;
	CBrush						 m_hBGBrush;
public:
	CLoadRunInfoDlg				 m_dlgMsg;

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
		MoveMsgWindowPos();
		SetMsgHandled(FALSE);
	}

	void MoveMsgWindowPos()
	{
#define XX_MSG_DLG_WIDTH	292
#define XX_MSG_DLG_HEIGHT	96
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
	}

	void CreateX()
	{
		m_dlgMsg.Create(m_hWnd);
		m_dlgMsg.ShowWindow(SW_HIDE);
		m_dlgMsg.ModifyStyle(0,WS_CHILD);
		m_dlgMsg.SetParent(m_hWnd);

		m_wndToolTip.Create(m_hWnd);
		CToolInfo ti(0, m_hWnd);
		m_wndToolTip.AddTool(ti);
		m_wndToolTip.Activate(TRUE);
		m_wndToolTip.SetMaxTipWidth(500);
		m_wndToolTip.SetDelayTime(TTDT_AUTOPOP,5*1000);
		m_wndToolTip.SetDelayTime(TTDT_INITIAL,500);
	}

	void ShowLoadDlg(BOOL bShow=TRUE)
	{
		if (bShow)
		{
			m_dlgMsg.SetItemVisible(1,TRUE);
			m_dlgMsg.SetItemVisible(2,FALSE);
			MoveMsgWindowPos();
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
		{
			if (pdata->pInfo->bFileIsExist == TRUE)
			{
				if (pdata->pInfo->bEnable == KSRUN_START_ENABLE || pdata->pInfo->bEnable == KSRUN_START_DELAY)
					return CLICK_INDEX_DISABLE;
				else
					return CLICK_INDEX_ENABLE;
			}
			else
				return CLICK_INDEX_DELETE;
		}
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
		else if (pdata->rcMenuBtn.PtInRect(ptx))
			return CLICK_INDEX_MENUBTN;
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

			if (iIndex == CLICK_INDEX_ENABLE || 
				iIndex == CLICK_INDEX_DELETE || 
				iIndex == CLICK_INDEX_DELAY  ||
				iIndex == CLICK_INDEX_DISABLE||
				iIndex == CLICK_INDEX_OPEN_FOLDER )
			{
				m_opCallback->OnClickLabel(iIndex,pInfo);
				bHandle = TRUE;
			}
			else if (iIndex == CLICK_INDEX_MENUBTN)
			{
				CMenu	menu;
				menu.CreatePopupMenu();
				menu.AppendMenu(MF_BYCOMMAND, CLICK_INDEX_ENABLE, BkString::Get(IDS_SYSOPT_4083));	//设为开机启动
				if (pdata->pInfo->nType == KSRUN_TYPE_STARTUP)
				{
					menu.AppendMenu(MF_BYCOMMAND, CLICK_INDEX_DELAY, BkString::Get(IDS_SYSOPT_4084));	//设为延迟启动
				}
				menu.AppendMenu(MF_BYCOMMAND, CLICK_INDEX_DISABLE, BkString::Get(IDS_SYSOPT_4085));	//禁止开机自启动
				if (pdata->pInfo->nType != KSRUN_TYPE_SERVICE)
				{
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_BYCOMMAND, CLICK_INDEX_DELETE, BkString::Get(IDS_SYSOPT_4086));	//删除启动项
				}
				
				if (pdata->pInfo->nType == KSRUN_TYPE_SERVICE)
					menu.EnableMenuItem(CLICK_INDEX_DELETE, MF_GRAYED);
				if (pdata->pInfo->bEnable == KSRUN_START_DISABELE)
					menu.EnableMenuItem(CLICK_INDEX_DISABLE, MF_GRAYED);
				if (pdata->pInfo->bEnable == KSRUN_START_ENABLE)
					menu.EnableMenuItem(CLICK_INDEX_ENABLE, MF_GRAYED);
				if (pdata->pInfo->bEnable == KSRUN_START_DELAY ||
					pdata->pInfo->CannotDelay_Sys() ||
					pdata->pInfo->nType != KSRUN_TYPE_STARTUP )
					menu.EnableMenuItem(CLICK_INDEX_DELAY, MF_GRAYED);

				if (pdata->pInfo->bFileIsExist == FALSE)
				{
					menu.EnableMenuItem(CLICK_INDEX_ENABLE, MF_GRAYED);
					menu.EnableMenuItem(CLICK_INDEX_DISABLE, MF_GRAYED);
					menu.EnableMenuItem(CLICK_INDEX_DELAY, MF_GRAYED);
				}

				CPoint	ptx;
				::GetCursorPos(&ptx);
				int i = menu.TrackPopupMenu(TPM_RETURNCMD,ptx.x,ptx.y, m_hWnd);

				if (i == CLICK_INDEX_ENABLE || 
					i == CLICK_INDEX_DELETE || 
					i == CLICK_INDEX_DELAY	||
					i == CLICK_INDEX_DISABLE)
				{
					m_opCallback->OnClickLabel(i,pdata->pInfo);
					bHandle = TRUE;
				}
				bHandle = TRUE;
			}

			if (!bHandle)
			{
				MsgDoExp(iItem);
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
				CMenu	menu;
				menu.CreatePopupMenu();
				menu.AppendMenu(MF_BYCOMMAND, CLICK_INDEX_ENABLE, BkString::Get(IDS_SYSOPT_4083));	//设为开机启动
				if (pdata->pInfo->nType == KSRUN_TYPE_STARTUP)
				{
					menu.AppendMenu(MF_BYCOMMAND, CLICK_INDEX_DELAY, BkString::Get(IDS_SYSOPT_4084));	//设为延迟启动
				}
				menu.AppendMenu(MF_BYCOMMAND, CLICK_INDEX_DISABLE, BkString::Get(IDS_SYSOPT_4085));	//禁止开机自启动
				if (pdata->pInfo->nType != KSRUN_TYPE_SERVICE)
				{
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_BYCOMMAND, CLICK_INDEX_DELETE, BkString::Get(IDS_SYSOPT_4086));	//删除启动项
				}

				if (pdata->pInfo->nType == KSRUN_TYPE_SERVICE)
					menu.EnableMenuItem(CLICK_INDEX_DELETE, MF_GRAYED);
				if (pdata->pInfo->bEnable == KSRUN_START_DISABELE)
					menu.EnableMenuItem(CLICK_INDEX_DISABLE, MF_GRAYED);
				if (pdata->pInfo->bEnable == KSRUN_START_ENABLE)
					menu.EnableMenuItem(CLICK_INDEX_ENABLE, MF_GRAYED);
				if (pdata->pInfo->bEnable == KSRUN_START_DELAY ||
					pdata->pInfo->CannotDelay_Sys() ||
					pdata->pInfo->nType != KSRUN_TYPE_STARTUP
					)
					menu.EnableMenuItem(CLICK_INDEX_DELAY, MF_GRAYED);
					
				if (pdata->pInfo->bFileIsExist == FALSE)
				{
					menu.EnableMenuItem(CLICK_INDEX_ENABLE, MF_GRAYED);
					menu.EnableMenuItem(CLICK_INDEX_DISABLE, MF_GRAYED);
					menu.EnableMenuItem(CLICK_INDEX_DELAY, MF_GRAYED);
				}

				CPoint	ptx;
				::GetCursorPos(&ptx);
				int i = menu.TrackPopupMenu(TPM_RETURNCMD,ptx.x,ptx.y, m_hWnd);

				if (i == CLICK_INDEX_ENABLE ||
					i == CLICK_INDEX_DELETE ||
					i == CLICK_INDEX_DISABLE||
					i == CLICK_INDEX_DELAY)
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
		BOOL bExpand = pdata->bExpand;
	
		ExpandAllItem(FALSE);

		if ( true )
		{
			if (!bExpand)
			{
				ExpandItem(iItem);
			}
			else
			{
				ExpandItem(iItem,FALSE);
			}
		}

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

	void ExpandItem(int iItem, BOOL bExp = TRUE)//TRUE
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
		if ( bExp && !pdata->bExpand)
		{
			pdata->bExpand = bExp;
			CListBox::SetItemHeight(iItem,XX_RUN_LIST_ITEM_HEIGHT_EXP);
		}
		else if ( !bExp && pdata->bExpand )
		{
			pdata->bExpand = bExp;
			CListBox::SetItemHeight(iItem,XX_RUN_LIST_ITEM_HEIGHT_COL);
		}
#endif
	}
	CKsafeRunInfo* GetDataInfo(int nItem)
	{
		_RUN_ITEM_DATA*	pParam = (_RUN_ITEM_DATA*)XBASE_GetItemData(nItem);

		if (pParam!=NULL && (DWORD)pParam != -1 )
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
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
			else if (iIndex == CLICK_INDEX_DELETE)
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
			else if (iIndex == CLICK_INDEX_OPEN_FOLDER )
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
			else if (iIndex == CLICK_INDEX_MENUBTN)
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
			else if (iIndex == CLICK_INDEX_DISABLE)
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
			
			if (iIndex == CLICK_INDEX_DESC||
				iIndex == CLICK_INDEX_LBL_FOLDER||
				iIndex == CLICK_INDEX_CAPTION)
			{
				if ( ((DWORD)kinfo!=m_pHoverTip) && !kinfo->strExePath.IsEmpty() )
				{
					CString	strInfo;

					if (iIndex == CLICK_INDEX_DESC)
					{
						strInfo.Append( kinfo->strDesc.IsEmpty()?BkString::Get(IDS_SYSOPT_4033):kinfo->strDesc );
						if (!kinfo->bFileIsExist)
							strInfo.Format(BkString::Get(IDS_SYSOPT_4064),CString(strInfo));
					}
					else if (iIndex == CLICK_INDEX_LBL_FOLDER )
						strInfo.Append( kinfo->strParam );
					else if (iIndex == CLICK_INDEX_CAPTION)
					{
						if (kinfo->bFileIsExist)
							strInfo.Append( kinfo->strDisplay );
						else
							strInfo.Format(BkString::Get(IDS_SYSOPT_4061),kinfo->strDisplay);
					}

					m_wndToolTip.SetMaxTipWidth(500);
					m_wndToolTip.UpdateTipText((LPCTSTR)strInfo,m_hWnd);
					m_pHoverTip = (DWORD)kinfo;
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
					m_hIconSys = ::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_RUN_SYSTEM));
				return m_hIconSys;
			}
			else if (nIndex==ICON_UNK_ID)
			{
				if (m_hIconUnk==NULL)
					m_hIconUnk = ::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_RUN_UNKNOWN));
				return m_hIconUnk;
			}
			else if (nIndex==ICON_REMAIN)
			{
				if (m_hIconRemian==NULL)
					m_hIconRemian = ::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_RUN_REMAIN));
				return m_hIconRemian;
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
		if (rcProbe.Width() <= rcArea.Width())
			rcOut.right	= rcOut.left + rcProbe.Width();
		else
			rcOut.right = rcArea.right;
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
		HFONT fntdef = BkFontPool::GetFont(BKF_DEFAULTFONT);
		HFONT fntBold = BkFontPool::GetFont(BKF_BOLDFONT);
		HFONT fntdeftmp = dcx.SelectFont(fntdef);
		clrOld = dcx.GetTextColor();

		if (pInfo && pItemData)
		{
			CRect	rcItemX = lpDrawItemStruct->rcItem;

			rcItemX.right = rcItemX.left + rcWin.Width()-20;
			
			if (lpDrawItemStruct->itemState&ODS_SELECTED)
				dcx.FillSolidRect(&rcItemX,COL_ITEM_SELECT);
			else
				dcx.FillSolidRect(&rcItemX,COL_DEFAULT_WHITE_BG);

			if (pItemData->bExpand)
			{
				CDC dcTmp;
				dcTmp.CreateCompatibleDC(dcx);
				HBITMAP hBmpCheck	= BkBmpPool::GetBitmap(IDB_RUNOPT_LIST_BG);
				HBITMAP hBmpOld		= dcTmp.SelectBitmap(hBmpCheck);
				dcx.StretchBlt( rcItemX.left, rcItemX.top+1, rcItemX.Width(),rcItemX.Height()-2, dcTmp, 0,0,1,98,SRCCOPY);
				dcTmp.SelectBitmap(hBmpOld);
			}

			CRect	rcFirst;
			CRect	rcSecond;
			int		nSecWidth = 310;

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
					::DrawIconEx(dcx,rcIcon.left,rcIcon.top,hIcon,32,32,0,NULL,DI_NORMAL);
				}

				dcx.SetTextColor((pInfo->bEnable == KSRUN_START_DISABELE)?COL_STR_CPT_DISABLE:COL_STR_CPT_DEFAULT);
				dcx.SetBkMode(TRANSPARENT);

				if (TRUE)
				{
					CRect	rcCaption=m_rcCaption;
					rcCaption.left+=rcFirst.left;
					rcCaption.top+=rcFirst.top;
					rcCaption.bottom=rcCaption.top+m_rcCaption.Height();
					rcCaption.right=rcFirst.right;

					CRect	rcProbe;
					
					dcx.SelectFont(fntBold);//启动项标题字体加粗
					if (pInfo->bFileIsExist == FALSE)
					{
						CString strDisplay_Red = pInfo->strDisplay;
						strDisplay_Red.Format(BkString::Get(IDS_SYSOPT_4061),CString(strDisplay_Red));
						dcx.SetTextColor(COL_STR_FILENOTEXIST);
						dcx.DrawText(strDisplay_Red,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
						dcx.DrawText(strDisplay_Red,-1,&rcCaption,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);
						dcx.SetTextColor((pInfo->bEnable == KSRUN_START_DISABELE)?COL_STR_CPT_DISABLE:COL_STR_CPT_DEFAULT);
					}
					else
					{
						dcx.DrawText(pInfo->strDisplay,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
						dcx.DrawText(pInfo->strDisplay,-1,&rcCaption,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);
					}
					dcx.SelectFont(fntdef);	//恢复默认字体
					m_rcCaption.right = rcFirst.Width();

					rcProbe.bottom = rcProbe.top+12;
					rcCaption.OffsetRect(-rcFirst.left, -rcFirst.top);
					calcLeftRECT(rcCaption,rcProbe,pItemData->rcCaption);		

					if (!pItemData->bExpand)
					{
						CRect rcDesc=m_rcDesc;//m_rcDesc2
						rcDesc.left+=rcFirst.left;
						rcDesc.top+=rcFirst.top;
						rcDesc.bottom=rcDesc.top+m_rcDesc.Height();
						rcDesc.right=rcFirst.right;

						CRect	rcProbe;
						CString	strShow;				
						strShow.Format(_T("%s"), pInfo->strDesc.IsEmpty()?BkString::Get(IDS_SYSOPT_4033):pInfo->strDesc);

						if (!pInfo->bFileIsExist)
							strShow.Format(BkString::Get(IDS_SYSOPT_4064),CString(strShow));

						dcx.DrawText(strShow,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
						dcx.DrawText(strShow,-1,&rcDesc,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);

						rcProbe.bottom = rcProbe.top+12;
						rcDesc.OffsetRect(-rcFirst.left, -rcFirst.top);
						calcLeftRECT(rcDesc,rcProbe,pItemData->rcDesc);
					}
				}
				if (TRUE)
				{
					DWORD	LENGTH_ADVICE	= 80;
					DWORD	LENGTH_STATE	= 48;
					DWORD	LENGTH_ACTION	= 25;
					DWORD	LENGTH_MARGIN	= 20;
					DWORD	LENGTH_PERCENT	= 48;

					DWORD	nTLength = LENGTH_ADVICE+LENGTH_STATE+LENGTH_ACTION+LENGTH_MARGIN*2;
					CRect	rcState=rcSecond;
					CString	strState;
					CString strAdvice;

					rcState.left = rcState.left + rcSecond.Width()-nTLength-125;
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
					//建议
					dcx.SetTextColor(COL_STR_ADVICE);
					strAdvice.Format(_T("%s"), pInfo->GetAdviceStr());
					CRect rcAdvice = rcState;
					rcAdvice.left += 8;
					rcAdvice.right+= 8;
					if (!pInfo->bFileIsExist)
					{
						dcx.SetTextColor(COL_STR_FILENOTEXIST);
						dcx.DrawText(strAdvice,-1,&rcAdvice,DT_VCENTER|DT_SINGLELINE|DT_CENTER);
						dcx.SetTextColor((pInfo->bEnable == KSRUN_START_DISABELE)?COL_STR_CPT_DISABLE:COL_STR_CPT_DEFAULT);
					}
					else
					{
						dcx.DrawText(strAdvice,-1,&rcAdvice,DT_VCENTER|DT_SINGLELINE|DT_CENTER);
					}

					//开启率
					CRect rcPercent = rcState;
					rcPercent.left = rcPercent.right + 14;
					rcPercent.right = rcPercent.left + LENGTH_PERCENT;
					CString strOpenRate;
					if (pInfo->strOpenRate.IsEmpty() == FALSE)
						strOpenRate = pInfo->strOpenRate;
					else
						strOpenRate = BkString::Get(IDS_SYSOPT_4094);
					dcx.DrawText(strOpenRate,-1,&rcPercent,DT_VCENTER|DT_SINGLELINE|DT_CENTER);

					//当前状态：
					int nTemLen = 0;
					if (pInfo->nType==KSRUN_TYPE_SERVICE)
					{
						LENGTH_STATE = pInfo->bEnable?50:60;
						LENGTH_ACTION = pInfo->bEnable?60:50;
						nTemLen = pInfo->bEnable?2:7;
						strState.Format(_T("%s"), pInfo->bEnable?BkString::Get(IDS_SYSOPT_4036):BkString::Get(IDS_SYSOPT_4037));
					}
					else
					{
						if (pInfo->bEnable == KSRUN_START_DELAY)
							strState.Format(_T("%s"), BkString::Get(IDS_SYSOPT_4092));					
						else
							strState.Format(_T("%s"), pInfo->bEnable?BkString::Get(IDS_SYSOPT_4034):BkString::Get(IDS_SYSOPT_4035));
					}

					rcState.left = rcState.right+LENGTH_MARGIN*4 + 7-nTemLen;
					rcState.right= rcState.left+LENGTH_STATE;

					COLORREF crColor = GetStateColor(pInfo->bEnable);
					dcx.SetTextColor(crColor);
					dcx.DrawText(strState,-1,&rcState,DT_VCENTER|DT_SINGLELINE|DT_CENTER);

					rcState.left = rcState.right+LENGTH_MARGIN;
					rcState.right= rcState.left+LENGTH_ACTION;

					//操作:
					dcx.SetTextColor(COL_DEFAULT_LINK);

					if ( pInfo->nType != KSRUN_TYPE_SERVICE )
					{
						if (pInfo->bFileIsExist)
							strState.Format(_T("%s"), 
								pInfo->bEnable == KSRUN_START_DISABELE?BkString::Get(IDS_SYSOPT_4039):BkString::Get(IDS_SYSOPT_4038));
						else
							strState.Format(_T("%s"), BkString::Get(IDS_SYSOPT_4062));
						rcState.left += 40;
						rcState.right+= 40;
					}
					else
					{
						if (pInfo->bFileIsExist)
							strState.Format(_T("%s"), (pInfo->bEnable == KSRUN_START_ENABLE)?BkString::Get(IDS_SYSOPT_4037):BkString::Get(IDS_SYSOPT_4036));
						else
							strState.Format(_T("%s"), BkString::Get(IDS_SYSOPT_4062));
					}

					HFONT hfnt = BkFontPool::GetFont(FALSE,TRUE,FALSE);
					HFONT hTmp = dcx.SelectFont(hfnt);
					dcx.DrawText(strState,-1,&rcState,DT_VCENTER|DT_SINGLELINE|DT_CENTER);
					dcx.SelectFont(hTmp);
					pItemData->rcButton = rcState;
					pItemData->rcButton.OffsetRect(-lpDrawItemStruct->rcItem.left,-lpDrawItemStruct->rcItem.top);

					//画扳手图标:
					if (pInfo->nType == KSRUN_TYPE_STARTUP)
					{
						CRect rcMenuBtn;
						DWORD LENGTH_MENU_BTN = 14;
						rcMenuBtn.left = rcState.right + 2;
						rcMenuBtn.top = rcState.top - 4;
						rcMenuBtn.bottom = rcMenuBtn.top + LENGTH_MENU_BTN;
						rcMenuBtn.right = rcMenuBtn.left + LENGTH_MENU_BTN;
						Gdiplus::Image *pImage = NULL;
						Gdiplus::Graphics grap(dcx.m_hDC);
						pImage = BkPngPool::Get(IDP_RUNOPT_LIST_MENU_BTN);
						Gdiplus::RectF rcDest(rcMenuBtn.left, rcMenuBtn.top, pImage->GetWidth(), pImage->GetHeight());
						grap.DrawImage(pImage, rcDest, 0, 0,
							pImage->GetWidth(), pImage->GetHeight(), Gdiplus::UnitPixel);
						pItemData->rcMenuBtn = rcMenuBtn;
						pItemData->rcMenuBtn.OffsetRect(-lpDrawItemStruct->rcItem.left,-lpDrawItemStruct->rcItem.top);
					}
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
						strShow.Format(BkString::Get(IDS_SYSOPT_4040), pInfo->GetAdviceStr() );
						dcx.DrawText(strShow,-1,&rcAdvice,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);
					}
					
					dcx.SetTextColor((pInfo->bEnable == KSRUN_START_DISABELE)?COL_STR_CPT_DISABLE:COL_STR_CPT_DEFAULT);
					if (TRUE)
					{
						CRect	rcDesc=m_rcDesc;//mm_rcDesc2
						rcDesc.left+=rcFirst.left;
						rcDesc.top+=rcFirst.top;
						rcDesc.bottom=rcDesc.top+m_rcDesc.Height();
						rcDesc.right=rcSecond.right;

						CRect	rcProbe;
						CString	strShow;								
						strShow.Format(_T("%s"), pInfo->strDesc.IsEmpty()?BkString::Get(IDS_SYSOPT_4033):pInfo->strDesc);
						
						if (!pInfo->bFileIsExist)
							strShow.Format(BkString::Get(IDS_SYSOPT_4064),CString(strShow));

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
						//开启率描述:
						CRect	rcProbe;
						CRect rcPercent = m_rcAppPath;
						rcPercent.left += rcFirst.left;
						rcPercent.top += rcFirst.top;
						rcPercent.bottom = rcPercent.top + m_rcDesc.Height();
						rcPercent.right = rcSecond.right-LENGTH_OPEN_FOLDER-MARGIN_OPEN_FOLDER*2;

						CString strPercent = GetOpenRateDesc(pInfo->nUserCount, pInfo->strOpenRate, pInfo->nType);
						dcx.DrawText(strPercent,-1,&rcPercent,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);

						//启动参数：
						CRect rcApp = m_rcAppPath;
						rcApp.left += rcFirst.left;
						rcApp.top += rcFirst.top+25;
						rcApp.bottom = rcApp.top + m_rcDesc.Height();
						rcApp.right = rcSecond.right-LENGTH_OPEN_FOLDER-MARGIN_OPEN_FOLDER*6;

						CString	strShow;				
						strShow.Format(BkString::Get(IDS_SYSOPT_4042), pInfo->strParam);
						dcx.DrawText(strShow,-1,&rcProbe,DT_CALCRECT|DT_VCENTER|DT_SINGLELINE|DT_LEFT);
						dcx.DrawText(strShow,-1,&rcApp,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_LEFT);

						//“打开所在目录”LinkText
						int nTemLen = 0;
						if (pInfo->nType == KSRUN_TYPE_SERVICE)
							nTemLen = pInfo->bEnable?3:-3;
						else
							nTemLen = 8;
						CRect rcOpenDir = rcApp;
						rcOpenDir.left = rcApp.right + MARGIN_OPEN_FOLDER*1 + 2 + nTemLen;
						rcOpenDir.right = rcOpenDir.left + LENGTH_OPEN_FOLDER*3/2;

						dcx.SetTextColor(COL_DEFAULT_LINK);
						dcx.SelectFont(BkFontPool::GetFont(BKF_UNDERLINE));	//LinkText字体加下划线
						strShow.Format(_T("%s"),BkString::Get(IDS_SYSOPT_4052));
						dcx.DrawText(strShow,-1,&rcOpenDir,DT_VCENTER|DT_SINGLELINE|DT_LEFT);
						pItemData->rcOpenDir = rcOpenDir;
						pItemData->rcOpenDir.OffsetRect(-lpDrawItemStruct->rcItem.left,-lpDrawItemStruct->rcItem.top);
												
						dcx.SelectFont(fntdef);	//恢复为默认字体			

						CRect rcAppX = rcApp;
						rcProbe.bottom = rcProbe.top+12;
						rcAppX.OffsetRect(-rcFirst.left, -rcFirst.top);
						calcLeftRECT(rcAppX,rcProbe,pItemData->rcCmdLine);
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

	COLORREF GetStateColor(int nState)
	{
		COLORREF crColor = COL_STR_CPT_DEFAULT;
		if (nState == KSRUN_START_DISABELE)
		{
			crColor = COL_STR_STATE_DISABLE;
		}
		if (nState == KSRUN_START_ENABLE)
		{
			crColor = COL_STR_STATE_ENABLE;
		}
		if (nState == KSRUN_START_DELAY)
		{
			crColor = COL_DEFAULT_LINK;
		}
		return crColor;
	}

	CString GetOpenRateDesc(DWORD dwUserCount, CString strOpenRate, int nType)
	{
		CString strDesc;
		if (strOpenRate.IsEmpty() || (dwUserCount == 0))
			strDesc = BkString::Get(IDS_SYSOPT_4089);
		else if (dwUserCount < 50)
			strDesc.Format((nType == KSRUN_TYPE_STARTUP)?(BkString::Get(IDS_SYSOPT_4088)):(BkString::Get(IDS_SYSOPT_4101)), strOpenRate);
		else if (dwUserCount > 50)
			strDesc.Format((nType == KSRUN_TYPE_STARTUP)?(BkString::Get(IDS_SYSOPT_4082)):(BkString::Get(IDS_SYSOPT_4087)), strOpenRate);
		return strDesc;
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
 		if (!pInfo->bFileIsExist)
 			m_mapIconIndex[pInfo] = ICON_REMAIN;
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
		if (m_nShowType!=nType)
		{
			m_bShowSystem = FALSE;
			DeleteAllItemX();
			for (int i=0; i<m_listArray.GetSize(); i++)
			{
				CKsafeRunInfo*	pInfoX = m_listArray[i]->pInfo;
				if (pInfoX->nType==nType && pInfoX->bSystem == m_bShowSystem)
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
					strFormat = BkString::Get(IDS_SYSOPT_4043);
				else if (m_nSubShowType==SUB_SHOW_TYPE_DISABLE)
					strFormat = BkString::Get(IDS_SYSOPT_4044);
				else if (m_nSubShowType==SUB_SHOW_TYPE_ENABLE)
					strFormat = BkString::Get(IDS_SYSOPT_4045);
			}
			else if ( m_nShowType==KSRUN_TYPE_SERVICE )
			{
				if (m_nSubShowType==SUB_SHOW_TYPE_ALL)
                    strFormat = BkString::Get(IDS_SYSOPT_4046);
				else if (m_nSubShowType==SUB_SHOW_TYPE_DISABLE)
                    strFormat = BkString::Get(IDS_SYSOPT_4047);
				else if (m_nSubShowType==SUB_SHOW_TYPE_ENABLE)
                    strFormat = BkString::Get(IDS_SYSOPT_4048);
			}
			else if ( m_nShowType==KSRUN_TYPE_TASK )
			{
				if (m_nSubShowType==SUB_SHOW_TYPE_ALL)
                    strFormat = BkString::Get(IDS_SYSOPT_4049);
				else if (m_nSubShowType==SUB_SHOW_TYPE_DISABLE)
                    strFormat = BkString::Get(IDS_SYSOPT_4050);
				else if (m_nSubShowType==SUB_SHOW_TYPE_ENABLE)
                    strFormat = BkString::Get(IDS_SYSOPT_4051);
			}

			m_dlgMsg.SetItemVisible(1,FALSE);
			m_dlgMsg.SetItemVisible(2,TRUE);
			m_dlgMsg.SetItemText(3,strFormat);
			MoveMsgWindowPos();
			m_dlgMsg.ShowWindow(SW_SHOW);
		}
		else
		{
			m_dlgMsg.ShowWindow(SW_HIDE);
		}
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
			if (pInfo->nType==nType && pInfo->bEnable &&pInfo->bFileIsExist)
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
