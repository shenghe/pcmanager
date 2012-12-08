#ifndef _SOFT_MGR_DOWNMGR_H_
#define _SOFT_MGR_DOWNMGR_H_

/*
* @file    softmgrDownloadListView.h
* @brief   软件管理--下载管理--正在下载
*/

#include "beikecolordef.h"
#include "SoftMgrItemData.h"

#define SOFTMGR_DOWNLOAD_LIST_ITEM_HEIGHT 54
#define SOFTMGR_LIST_RIGHT_WIDTH 310

#define	COLOR_SOFTMGR_PLUG		RGB(200,0,0)
#define	COLOR_SOFTMGR_NOPLUG	RGB(10,150,0)
#define COLOR_MOUSE_ON_NAME		RGB(0,100,200)

#define STR_BTN_TIP_CANCEL      BkString::Get(IDS_SOFTMGR_8055)
#define STR_BTN_TIP_CONTINUE    BkString::Get(IDS_SOFTMGR_8086)
#define STR_BTN_TIP_PAUSE       BkString::Get(IDS_SOFTMGR_8087)


class ISoftMgrDownloadCallback
{
public:
	virtual void OnMgrDownLoad(DWORD dwIndex) = 0;	//下载
	virtual void OnMgrContinue(DWORD dwIndex) = 0;	//继续
	virtual void OnMgrPause(DWORD dwindex) = 0;	//暂停
	virtual void OnMgrCancel(DWORD dwIndex) = 0;	//取消
	virtual void OnMgrFreebackFromList(DWORD dwIndex) = 0;
	virtual void OnMgrTautilogyFromList(DWORD dwIndex) = 0;
};

class CSoftMgrDownloadListView : public CWindowImpl<CSoftMgrDownloadListView, CListViewCtrl>, 
	public COwnerDraw<CSoftMgrDownloadListView>
{
public:
	CSoftMgrDownloadListView()
	{
		m_rcIcon = CRect(5, 11, 37, 43);
		m_rcName = CRect(45, 9, 105, 23);
		m_rcDescription = CRect(45, 33, 105, 45);

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

		m_fntNameOn.Attach( BkFontPool::GetFont(FALSE,TRUE,FALSE,0));
		m_fntNameNormal.Attach( BkFontPool::GetFont(TRUE, FALSE, FALSE, 0));
		m_fntPlug.Attach( BkFontPool::GetFont(BKF_DEFAULTFONT));
		m_fntDanger.Attach( BkFontPool::GetFont(BKF_BOLDFONT));
		m_fntDefault.Attach( BkFontPool::GetFont(BKF_DEFAULTFONT));
	}

	~CSoftMgrDownloadListView()
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
		MOUSE_LASTON_TAUTOLOGY
	};

public:
	BEGIN_MSG_MAP(CSoftMgrDownloadListView)  
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent) 
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MSG_OCM_CTLCOLORLISTBOX(OnCtlColor)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CSoftMgrDownloadListView>,1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()   

	CSimpleArray<CSoftListItemData*>& GetItemArray() 
	{ 
		return m_arrData;
	}

	void UpdateAll()
	{
		if(m_hWnd != NULL)
			DeleteAllItems();

		for (int i=0; i < m_arrData.GetSize(); i++)
		{
			CSoftListItemData*	pItem = NULL;
			pItem = m_arrData[i];

			pItem->m_dwID = CListViewCtrl::GetItemCount();
			InsertItem(pItem->m_dwID, _T(""));
			SetItemData(pItem->m_dwID,(DWORD_PTR)pItem);
		}

		InvalidateRect(NULL);
	}

	HRESULT	OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
	{
		bMsgHandled = FALSE;

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

	void CreateTipCtrl()
	{

		m_wndToolTip.Create(m_hWnd);

		CToolInfo ti(0, m_hWnd);
		m_wndToolTip.AddTool(ti);
		m_wndToolTip.Activate(TRUE);
		m_wndToolTip.SetMaxTipWidth(300);
		m_wndToolTip.SetDelayTime(TTDT_AUTOPOP,1000);
	}

	void InitNormal()
	{
		CRect rcTmp;
		GetClientRect(&rcTmp);

		m_rcStateBtn.right = rcTmp.Width() - 15;
		m_rcStateBtn.left = m_rcStateBtn.right - 65;
		m_rcStateBtn.top = 15;
		m_rcStateBtn.bottom = 40;

		m_rcPlug.right = m_rcStateBtn.left - 40;
		m_rcPlug.left = m_rcPlug.right - 48;
		m_rcPlug.top = m_rcStateBtn.top;
		m_rcPlug.bottom = m_rcStateBtn.bottom;

		m_rcCharge.right = m_rcPlug.left;
		m_rcCharge.left = m_rcCharge.right - 24;
		m_rcCharge.top = m_rcStateBtn.top;
		m_rcCharge.bottom = m_rcStateBtn.bottom;

		m_rcSize.right = m_rcCharge.left - 25;
		m_rcSize.left = m_rcSize.right - 70;
		m_rcSize.top = m_rcStateBtn.top;
		m_rcSize.bottom = m_rcStateBtn.bottom;

		m_rcState = CRect(0, 0, 0, 0);
		m_rcCancel = CRect(0, 0, 0, 0);	
		m_rcContinue = CRect(0, 0, 0, 0);
		m_rcProgress = CRect(0, 0, 0, 0);
		m_rcValue = CRect(0, 0, 0, 0);
		m_rcWaitDownload = CRect(0,0,0,0);
	}

	void InitDownLoad()
	{
		CRect rcTmp;
		GetClientRect(&rcTmp);

		m_rcStateBtn = CRect(0, 0, 0, 0);
		m_rcCharge = CRect(0, 0, 0, 0);
		m_rcPlug = CRect(0, 0, 0, 0);
		m_rcSize = CRect(0, 0, 0, 0);
		m_rcWaitDownload = CRect(0,0,0,0);

		m_rcCancel.right = rcTmp.Width() - 25;
		m_rcCancel.left = m_rcCancel.right - 16;
		m_rcCancel.top = 19;
		m_rcCancel.bottom = 35;

		m_rcContinue.right = m_rcCancel.left - 2;
		m_rcContinue.left = m_rcContinue.right - 16;
		m_rcContinue.top = m_rcCancel.top;
		m_rcContinue.bottom = m_rcCancel.bottom;

		m_rcState.right = m_rcContinue.left - 30;
		m_rcState.left = m_rcState.right - 60;
		m_rcState.top = 21;
		m_rcState.bottom = 33;

		m_rcValue.right = m_rcState.left - 5;
		m_rcValue.left = m_rcValue.right - 35;
		m_rcValue.top = m_rcState.top;
		m_rcValue.bottom = m_rcState.bottom;

		m_rcProgress.right = m_rcValue.left - 20;
		m_rcProgress.left = m_rcProgress.right - 102;
		m_rcProgress.top = 23;
		m_rcProgress.bottom = m_rcProgress.top + 8;
	}

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

	CSoftListItemData*	GetItemDataEx(DWORD dwIndex)
	{
		ATLASSERT(dwIndex >= 0 && dwIndex < (DWORD)m_arrData.GetSize());
		if (dwIndex < 0 || dwIndex >= (DWORD)m_arrData.GetSize())
		{
			return NULL;
		}

		for ( int i=0; i<m_arrData.GetSize(); i++)
		{
			if (m_arrData[i]->m_dwID == dwIndex)
				return m_arrData[i];
		}
		return NULL;
	}

	int SetItemDataEx(DWORD dwIndex, DWORD_PTR dwItemData)
	{
		CSoftListItemData*	pItemData=(CSoftListItemData*)dwItemData;
		ATLASSERT(pItemData!=NULL);

		for (int i= 0;i<m_arrData.GetSize();i++)
		{
			if (m_arrData[i]->m_dwID==pItemData->m_dwID)
			{
				ATLASSERT(FALSE);
				return -1;
			}
		}

		m_arrData.Add(pItemData);
		//??
		return SetItemData(dwIndex,(DWORD_PTR)pItemData->m_pImage); 
	}

	BOOL GetItemRect(int nItem, LPRECT lpRect, UINT nCode)
	{
		return  CListViewCtrl::GetItemRect(nItem, lpRect, nCode);
	}

	HRESULT	OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
	{

		POINT	pts;
		/*BOOL	bOutSide;*/
		CRect	rcItem;

		bMsgHandled = FALSE;
		pts.x		= GET_X_LPARAM(lParam);
		pts.y		= GET_Y_LPARAM(lParam);

		LVHITTESTINFO hti;
		hti.pt = pts;
		SubItemHitTest( &hti );
		INT nIndex = hti.iItem;

		if ( /*!bOutSide &&*/ nIndex >= 0 && nIndex < 0xffff && GetItemRect(nIndex,&rcItem,LVIR_BOUNDS) )
		{
			CSoftListItemData*	pData = (CSoftListItemData*)GetItemDataEx(nIndex);
			if(pData != NULL)
			{
				if (pData->m_bDownloading)
				{
					InitDownLoad();
					if(pData->m_bLinking)
					{
						if (pData->m_bLinkFailed)
						{
							m_rcFreeback.top = 18;
							m_rcFreeback.bottom = 39;
							m_rcTautology.top = 18;
							m_rcTautology.bottom = 39;
							m_rcCancel  = CRect(0,0,0,0);
						}
						else
						{
							m_rcFreeback = CRect(0,0,0,0);
							m_rcTautology = CRect(0,0,0,0);
						}
						m_rcLink = m_rcValue;
						m_rcContinue = CRect(0,0,0,0);
						m_rcValue = CRect(0,0,0,0);
						m_rcState = CRect(0,0,0,0);
					}
					else
					{
						m_rcLink = CRect(0,0,0,0);
						m_rcFreeback = CRect(0,0,0,0);
						m_rcTautology = CRect(0,0,0,0);
						InitDownLoad();
					}
				}
				else
				{
					InitNormal();

					CRect rcTmp;
					GetClientRect(&rcTmp);
					if (pData->m_bWaitDownload)
					{
						m_rcWaitDownload.right = rcTmp.Width() - 15;
						m_rcWaitDownload.left = m_rcWaitDownload.right - 57;
						m_rcWaitDownload.top = 19;
						m_rcWaitDownload.bottom = 39;
						m_rcInstall = CRect(0,0,0,0);
						m_rcStateBtn = CRect(0,0,0,0);
					}
					else if (pData->m_bInstalling)
					{
						m_rcInstall.right = rcTmp.Width() - 15;
						m_rcInstall.left = m_rcInstall.right - 57;
						m_rcInstall.top = 19;
						m_rcInstall.bottom = 39;
						m_rcWaitDownload = CRect(0,0,0,0);
						m_rcStateBtn = CRect(0,0,0,0);
					}
					else
					{
						m_rcStateBtn.right = rcTmp.Width() - 15;
						m_rcStateBtn.left = m_rcStateBtn.right - 65;
						m_rcStateBtn.top = 15;
						m_rcStateBtn.bottom = 40;
						m_rcInstall = CRect(0,0,0,0);
						m_rcWaitDownload = CRect(0,0,0,0);
					}
				}

				if ( pData )
				{
					pts.x -= rcItem.left;
					pts.y -= rcItem.top;

					if ( pData->m_rcName.PtInRect(pts) ||
						m_rcCancel.PtInRect(pts) ||
						m_rcContinue.PtInRect(pts) ||
						m_rcIcon.PtInRect(pts) ||
						m_rcInstall.PtInRect(pts) ||
						pData->m_rcFreeback.PtInRect(pts) ||
						pData->m_rcTautology.PtInRect(pts))
					{
						m_bMouseDown = TRUE;
						bMsgHandled = TRUE;
					}


					if ( m_rcStateBtn.PtInRect(pts) )
					{
						m_bMouseDown = TRUE;
						bMsgHandled = TRUE;
						CRect rcButton = m_rcStateBtn;
						rcButton.OffsetRect(rcItem.left, rcItem.top);
						InvalidateRect(rcButton, FALSE);
					}
				}
			}
		}

		return S_OK;		
	}

	HRESULT	OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
	{
		POINT	pts;
		/*BOOL	bOutSide;*/
		CRect	rcItem;

		bMsgHandled = FALSE;
		pts.x = GET_X_LPARAM(lParam);
		pts.y = GET_Y_LPARAM(lParam);

		LVHITTESTINFO hti;
		hti.pt = pts;
		SubItemHitTest( &hti );
		int nIndex = hti.iItem;

		if ( /*!bOutSide &&*/ nIndex >= 0 && nIndex < m_arrData.GetSize() && GetItemRect(nIndex,&rcItem,LVIR_BOUNDS) )
		{
			CSoftListItemData*	pData = (CSoftListItemData*)GetItemDataEx(nIndex);

			if ( pData )
			{
				if (pData->m_bDownloading)
				{
					InitDownLoad();
					if(pData->m_bLinking)
					{
						if (pData->m_bLinkFailed)
						{
							m_rcFreeback.top = 18;
							m_rcFreeback.bottom = 39;
							m_rcTautology.top = 18;
							m_rcTautology.bottom = 39;
							m_rcCancel  = CRect(0,0,0,0);
						}
						else
						{
							m_rcFreeback = CRect(0,0,0,0);
							m_rcTautology = CRect(0,0,0,0);
						}
						m_rcLink = m_rcValue;
						m_rcContinue = CRect(0,0,0,0);
						m_rcValue = CRect(0,0,0,0);
						m_rcState = CRect(0,0,0,0);
					}
					else
					{
						m_rcLink = CRect(0,0,0,0);
						m_rcFreeback = CRect(0,0,0,0);
						m_rcTautology = CRect(0,0,0,0);
						InitDownLoad();
					}
				}
				else
				{
					InitNormal();
					CRect rcTmp;
					GetClientRect(&rcTmp);
					if (pData->m_bWaitDownload)
					{
						m_rcWaitDownload.right = rcTmp.Width() - 15;
						m_rcWaitDownload.left = m_rcWaitDownload.right - 57;
						m_rcWaitDownload.top = 19;
						m_rcWaitDownload.bottom = 39;
						m_rcInstall = CRect(0,0,0,0);
						m_rcStateBtn = CRect(0,0,0,0);
					}
					else if (pData->m_bInstalling)
					{
						m_rcInstall.right = rcTmp.Width() - 15;
						m_rcInstall.left = m_rcInstall.right - 57;
						m_rcInstall.top = 19;
						m_rcInstall.bottom = 39;
						m_rcWaitDownload = CRect(0,0,0,0);
						m_rcStateBtn = CRect(0,0,0,0);
					}
					else
					{
						m_rcStateBtn.right = rcTmp.Width() - 15;
						m_rcStateBtn.left = m_rcStateBtn.right - 65;
						m_rcStateBtn.top = 15;
						m_rcStateBtn.bottom = 40;
						m_rcInstall = CRect(0,0,0,0);
						m_rcWaitDownload = CRect(0,0,0,0);
					}
				}

				if (pData->m_bDownLoad)
				{
					CRect rcTmp;
					GetClientRect(&rcTmp);
					if (pData->m_bInstalling)
					{
						m_rcInstall.right = rcTmp.Width() - 15;
						m_rcInstall.left = m_rcInstall.right - 57;
						m_rcInstall.top = 19;
						m_rcInstall.bottom = 39;
						m_rcStateBtn = CRect(0,0,0,0);
					}
					else
					{
						m_rcStateBtn.right = rcTmp.Width() - 15;
						m_rcStateBtn.left = m_rcStateBtn.right - 65;
						m_rcStateBtn.top = 15;
						m_rcStateBtn.bottom = 40;
						m_rcInstall = CRect(0,0,0,0);
					}

					m_rcState = CRect(0, 0, 0, 0);
					m_rcCancel = CRect(0, 0, 0, 0);	
					m_rcContinue = CRect(0, 0, 0, 0);
				}

				pts.x -= rcItem.left;
				pts.y -= rcItem.top;

				if(m_rcStateBtn.PtInRect(pts))
				{
					if (m_bMouseDown)
					{
						pData->m_bDownloading = TRUE;
						pData->m_bContinue = TRUE;
						pData->m_bPause = FALSE;
						m_linkerCB->OnMgrDownLoad(nIndex);
					}
				}
				else if(m_rcCancel.PtInRect(pts))
				{
					if (m_bMouseDown && !pData->m_bDownLoad)
					{
						pData->m_bDownloading = FALSE;
						m_linkerCB->OnMgrCancel(nIndex);
					}
				}
				else if (m_rcContinue.PtInRect(pts))
				{
					if (m_bMouseDown  && !pData->m_bDownLoad)
					{
						pData->m_bDownloading = TRUE;
						if(pData->m_bContinue)
						{
							pData->m_bPause = TRUE;
							pData->m_bContinue = FALSE;
							m_linkerCB->OnMgrPause(nIndex);
						}
						else
						{
							pData->m_bPause = FALSE;
							pData->m_bContinue = TRUE;
							m_linkerCB->OnMgrContinue(nIndex);
						}
					}
				}
				else if (pData->m_rcTautology.PtInRect(pts))
				{
					m_linkerCB->OnMgrTautilogyFromList(nIndex);
				}
				else if (pData->m_rcFreeback.PtInRect(pts))
				{
					m_linkerCB->OnMgrFreebackFromList(nIndex);
				}

				InvalidateRect(rcItem, FALSE);
			}
		}

		m_bMouseDown = FALSE;
		return S_OK;		
	}

	void SetClickLinkCallback( ISoftMgrDownloadCallback* opCB )
	{
		m_linkerCB = opCB;
	}

	VOID SetPaintItem(BOOL bDrawItem)
	{
		m_bDrawItem = bDrawItem;
	}

	void MeasureItem(LPMEASUREITEMSTRUCT lpMes)
	{
		lpMes->itemHeight = SOFTMGR_DOWNLOAD_LIST_ITEM_HEIGHT;
	}

	HRESULT	OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
	{
		POINT	pts;
		CRect	rcItem;
		BOOL	bOntip=FALSE;
		BOOL	bMove=FALSE;

		m_bMouseOnName = FALSE;
		m_bMouseOnCancel = FALSE;
		m_bMouseOnPause = FALSE;
		m_bMouseOn = FALSE;
		m_bMouseOnFreeback = FALSE;
		m_bMouseOnTautology = FALSE;

		bMsgHandled = FALSE;
		pts.x		= GET_X_LPARAM(lParam);
		pts.y		= GET_Y_LPARAM(lParam);

		static CPoint ptLast;

		if (ptLast != pts)
			bMove=TRUE;

		ptLast = pts;

		LVHITTESTINFO hti;
		hti.pt = pts;
		SubItemHitTest( &hti );
		INT nIndex = hti.iItem;

		if (/*bMove &&*/ nIndex >= 0 && nIndex < m_arrData.GetSize() && GetItemRect(nIndex,&rcItem,LVIR_BOUNDS) )
		{
			CSoftListItemData*	pData = (CSoftListItemData*)GetItemDataEx(nIndex);
			if(pData != NULL)
			{
				if (pData->m_bDownloading && !pData->m_bWaitDownload)//下载
				{
					InitDownLoad();
					if(pData->m_bLinking)
					{
						if (pData->m_bLinkFailed)
						{
							m_rcFreeback.top = 18;
							m_rcFreeback.bottom = 39;
							m_rcTautology.top = 18;
							m_rcTautology.bottom = 39;
							m_rcCancel  = CRect(0,0,0,0);
						}
						else
						{
							m_rcFreeback = CRect(0,0,0,0);
							m_rcTautology = CRect(0,0,0,0);
						}
						m_rcLink = m_rcValue;
						m_rcContinue = CRect(0,0,0,0);
						m_rcValue = CRect(0,0,0,0);
						m_rcState = CRect(0,0,0,0);
					}
					else
					{
						m_rcLink = CRect(0,0,0,0);
						m_rcFreeback = CRect(0,0,0,0);
						m_rcTautology = CRect(0,0,0,0);
						InitDownLoad();
					}
				}
				else //已安装
				{
					InitNormal();
					CRect rcTmp;
					GetClientRect(&rcTmp);
					if (pData->m_bWaitDownload)
					{
						m_rcWaitDownload.right = rcTmp.Width() - 15;
						m_rcWaitDownload.left = m_rcWaitDownload.right - 57;
						m_rcWaitDownload.top = 19;
						m_rcWaitDownload.bottom = 39;
						m_rcInstall = CRect(0,0,0,0);
						m_rcStateBtn = CRect(0,0,0,0);
					}
					else if (pData->m_bInstalling)
					{
						m_rcInstall.right = rcTmp.Width() - 15;
						m_rcInstall.left = m_rcInstall.right - 57;
						m_rcInstall.top = 19;
						m_rcInstall.bottom = 39;
						m_rcWaitDownload = CRect(0,0,0,0);
						m_rcStateBtn = CRect(0,0,0,0);
					}
					else
					{
						m_rcStateBtn.right = rcTmp.Width() - 15;
						m_rcStateBtn.left = m_rcStateBtn.right - 65;
						m_rcStateBtn.top = 15;
						m_rcStateBtn.bottom = 40;
						m_rcInstall = CRect(0,0,0,0);
						m_rcWaitDownload = CRect(0,0,0,0);
					}
				}

				if (pData->m_bDownLoad)//下载完成
				{
					//最右侧按钮
					CRect rcTmp;
					GetClientRect(&rcTmp);
					if (pData->m_bInstalling)
					{
						m_rcInstall.right = rcTmp.Width() - 15;
						m_rcInstall.left = m_rcInstall.right - 57;
						m_rcInstall.top = 19;
						m_rcInstall.bottom = 39;
						m_rcStateBtn = CRect(0,0,0,0);
					}
					else
					{
						m_rcStateBtn.right = rcTmp.Width() - 15;
						m_rcStateBtn.left = m_rcStateBtn.right - 65;
						m_rcStateBtn.top = 15;
						m_rcStateBtn.bottom = 40;
						m_rcInstall = CRect(0,0,0,0);
					}

					m_rcState = CRect(0, 0, 0, 0);
					m_rcCancel = CRect(0, 0, 0, 0);	
					m_rcContinue = CRect(0, 0, 0, 0);
				}

				if ( pData )
				{
					pts.x -= rcItem.left;
					pts.y -= rcItem.top;

					CRect rcName = pData->m_rcName;
					if (/* m_*/rcName.PtInRect(pts) )
					{
						m_bMouseOnName = TRUE;
						rcName.OffsetRect(rcItem.left, rcItem.top);
						if(m_enumLastMSState != MOUSE_LASTON_NAME)
						{
							InvalidateRect(rcName, FALSE);
							m_enumLastMSState = MOUSE_LASTON_NAME;
						}
					}
					else if ( m_rcIcon.PtInRect(pts))
					{
						m_enumLastMSState = MOUSE_LASTON_SOFTICON;
					}
					else if ( m_rcCancel.PtInRect(pts) )
					{
						if(!pData->m_bDownLoad)
						{
							m_enumLastMSState = MOUSE_LASTON_CANCELBTN;
							m_bMouseOnCancel = TRUE;
							::SetCursor(::LoadCursor(NULL,IDC_HAND));
							if (((DWORD)pData != m_pHoverTip))
							{	
								m_wndToolTip.UpdateTipText(STR_BTN_TIP_CANCEL,m_hWnd);
								m_pHoverTip = (DWORD)pData;
							}
						}
					}
					else if ( m_rcContinue.PtInRect(pts) )
					{
						if( !pData->m_bDownLoad)
						{
							m_bMouseOnPause = TRUE;
							::SetCursor(::LoadCursor(NULL,IDC_HAND));
							CRect rcContinue = m_rcContinue;
							rcContinue.OffsetRect(rcItem.left, rcItem.top);
							if(m_enumLastMSState != MOUSE_LASTON_CONTINUEBTN)
							{
								InvalidateRect(rcContinue, FALSE);
								m_enumLastMSState = MOUSE_LASTON_CONTINUEBTN;
							}
							if (((DWORD)pData != m_pHoverTip)) 
							{
								if (pData->m_bPause)
								{
									m_wndToolTip.UpdateTipText(STR_BTN_TIP_CONTINUE,m_hWnd);
								}
								else
								{
									m_wndToolTip.UpdateTipText(STR_BTN_TIP_PAUSE,m_hWnd);
								}
								m_pHoverTip = (DWORD)pData;
							}
						}
					}
					else if( m_rcStateBtn.PtInRect(pts))
					{
						m_bMouseOn = TRUE;
						::SetCursor(::LoadCursor(NULL, IDC_HAND));
						CRect rcStateBtn = m_rcStateBtn;
						rcStateBtn.OffsetRect(rcItem.left, rcItem.top);
						if(m_enumLastMSState != MOUSE_LASTON_STATEBTN)
						{
							InvalidateRect(rcStateBtn, FALSE);
							m_enumLastMSState = MOUSE_LASTON_STATEBTN;
						}
					}
					else if (pData->m_rcTautology.PtInRect(pts) )
					{
						m_bMouseOnTautology = TRUE;
						::SetCursor(::LoadCursor(NULL,IDC_HAND));
						CRect rcTautology = pData->m_rcTautology;
						rcTautology.OffsetRect(rcItem.left, rcItem.top);
						if(m_enumLastMSState != MOUSE_LASTON_TAUTOLOGY)
						{
							InvalidateRect(rcTautology, FALSE);
							m_enumLastMSState = MOUSE_LASTON_NAME;
						}
					}
					else if (pData->m_rcFreeback.PtInRect(pts) )
					{
						m_bMouseOnFreeback = TRUE;
						::SetCursor(::LoadCursor(NULL,IDC_HAND));
						CRect rcFreeback = pData->m_rcFreeback;
						rcFreeback.OffsetRect(rcItem.left, rcItem.top);
						if(m_enumLastMSState != MOUSE_LASTON_FREEBACK)
						{
							InvalidateRect(rcFreeback, FALSE);
							m_enumLastMSState = MOUSE_LASTON_NAME;
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

					if(m_enumLastMSState == MOUSE_LASTON_NAME && !m_bMouseOnName)
					{
						CRect rcName = pData->m_rcName;
						rcName.OffsetRect(rcItem.left, rcItem.top);
						InvalidateRect(rcName, FALSE);
						m_enumLastMSState = MOUSE_LASTON_NONE;
					}
					else if(m_enumLastMSState == MOUSE_LASTON_CANCELBTN && !m_bMouseOnCancel)
					{
						CRect rcCancel = m_rcCancel;
						rcCancel.OffsetRect(rcItem.left, rcItem.top);
						InvalidateRect(rcCancel, FALSE);
						m_enumLastMSState = MOUSE_LASTON_NONE;
					}
					else if(m_enumLastMSState == MOUSE_LASTON_CONTINUEBTN && !m_bMouseOnCancel)
					{
						CRect rcContinue = m_rcContinue;
						rcContinue.OffsetRect(rcItem.left, rcItem.top);
						InvalidateRect(rcContinue, FALSE);
						m_enumLastMSState = MOUSE_LASTON_NONE;
					}
					else if(m_enumLastMSState == MOUSE_LASTON_STATEBTN && !m_bMouseOn)
					{
						CRect rcStateBtn = m_rcStateBtn;
						rcStateBtn.OffsetRect(rcItem.left, rcItem.top);
						InvalidateRect(rcStateBtn, FALSE);
						m_enumLastMSState = MOUSE_LASTON_NONE;
					}
				}
			}
		}

		return S_OK;
	}


	void _DrawItem(HDC hDC, LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		CRect rcWnd;
		INT	  nColumWidth = 0;
		CSoftListItemData*	pListData = GetItemDataEx(lpDrawItemStruct->itemID);

		GetWindowRect(&rcWnd);
		nColumWidth = rcWnd.Width();

		if(nColumWidth <= 0)
		{
			nColumWidth = lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left;
		}

		if (pListData != NULL)
		{
			CDCHandle		dcx;
			CDC				dcTmp;
			HFONT			hFntTmp;
			DWORD			nOldMod;
			COLORREF		clrOld;
			Gdiplus::Pen	linePen(Gdiplus::Color(255, 223, 230, 233), 2);
			Gdiplus::Pen	penFrame(Gdiplus::Color(255, 255, 225, 194), 1);
			Gdiplus::Pen	penBK(Gdiplus::Color(255, 255, 255, 255), 3);

			Gdiplus::Graphics grap(hDC);			

			dcx.Attach(hDC);
			dcTmp.CreateCompatibleDC(dcx);

			nOldMod = dcx.GetBkMode();
			dcx.SetBkMode(TRANSPARENT);

			hFntTmp = dcx.SelectFont(m_fntDefault);
			clrOld = dcx.GetTextColor();

			//判断此项是否被选中
			BOOL bSelect = FALSE ;
			if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
				(lpDrawItemStruct->itemState & ODS_SELECTED))
			{
				bSelect = TRUE ;
			}

			//画选中状态
			if ( bSelect )
			{				
				Gdiplus::SolidBrush mySolidBrush(Gdiplus::Color(255,235,245,255/*255,240,205*/));
				if (rcWnd.Height() > GetItemCount()*54)
				{
					grap.FillRectangle(&mySolidBrush, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, nColumWidth , 
						lpDrawItemStruct->rcItem.bottom-lpDrawItemStruct->rcItem.top-1);
					grap.DrawLine(&linePen, lpDrawItemStruct->rcItem.left,lpDrawItemStruct->rcItem.bottom,
						nColumWidth, lpDrawItemStruct->rcItem.bottom);
				}
				else
				{
					grap.FillRectangle(&mySolidBrush, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, nColumWidth - 15 , 
						lpDrawItemStruct->rcItem.bottom-lpDrawItemStruct->rcItem.top-1);
					grap.DrawLine(&linePen, lpDrawItemStruct->rcItem.left,lpDrawItemStruct->rcItem.bottom,
						nColumWidth - 15, lpDrawItemStruct->rcItem.bottom);
				}

			}
			else
			{
				Gdiplus::SolidBrush mySolidBrush(Gdiplus::Color(255,255,255,255));
				if (rcWnd.Height() > GetItemCount()*54)
				{
					grap.FillRectangle(&mySolidBrush, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, nColumWidth, 
						lpDrawItemStruct->rcItem.bottom-lpDrawItemStruct->rcItem.top-1);
					grap.DrawLine(&linePen, lpDrawItemStruct->rcItem.left,lpDrawItemStruct->rcItem.bottom,
						nColumWidth, lpDrawItemStruct->rcItem.bottom);
				}
				else
				{
					grap.FillRectangle(&mySolidBrush, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, nColumWidth - 15, 
						lpDrawItemStruct->rcItem.bottom-lpDrawItemStruct->rcItem.top-1);
					grap.DrawLine(&linePen, lpDrawItemStruct->rcItem.left,lpDrawItemStruct->rcItem.bottom,
						nColumWidth - 15, lpDrawItemStruct->rcItem.bottom);
				}
			}

			//画图标
			CRect rcIcon = m_rcIcon;
			rcIcon.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			if (pListData->m_pImage != NULL)
			{
				grap.DrawImage(pListData->m_pImage, rcIcon.left, rcIcon.top, rcIcon.Width(), rcIcon.Height());
			}

			//画软件名称
			CRect rcItem;
			GetWindowRect(&rcItem);
			CRect rcName = m_rcName;
			CRect rcProb;
			hFntTmp = dcx.SelectFont(m_fntNameNormal);
			dcx.DrawText(pListData->m_strName, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcName.right = rcItem.Width() - SOFTMGR_LIST_RIGHT_WIDTH-10;

			Gdiplus::Image *pImage = NULL;
			int nLablesWidth = 0;
			if ((pListData->m_attri&SA_Green) == SA_Green)
			{
				pImage = BkPngPool::Get(IDP_SOFTMGR_GREEN_SOFT);
				nLablesWidth += pImage->GetWidth();
			}
			if (pListData->m_bCharge == TRUE)
			{
				if (nLablesWidth != 0)
					nLablesWidth += 2;
				pImage = BkPngPool::Get(IDP_SOFTMGR_CHARGE_SOFT);
				nLablesWidth += pImage->GetWidth();
			}
			if (pListData->m_bPlug == TRUE)
			{
				if (nLablesWidth != 0)
					nLablesWidth += 2;
				pImage = BkPngPool::Get(IDP_SOFTMGR_PLUGIN_SOFT);
				nLablesWidth += pImage->GetWidth();
			}
			rcName.right = rcItem.Width() - (SOFTMGR_LIST_RIGHT_WIDTH+10) - nLablesWidth;

			pListData->m_rcName = rcName;
			if (rcProb.Width() < rcItem.Width() - (SOFTMGR_LIST_RIGHT_WIDTH+10) - nLablesWidth - 50)
			{ 
				pListData->m_rcName.right = rcName.left + rcProb.Width();
				rcName.right = rcName.left + rcProb.Width();
			}			
			rcName.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(pListData->m_strName, -1, &rcName, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
			dcx.SelectFont(hFntTmp);

			//画标签
			int nPosY = rcName.top - 2;
			int nPosX = rcName.right;
			if ((pListData->m_attri&SA_Green) == SA_Green)
			{
				nPosX += 2;
				pImage = BkPngPool::Get(IDP_SOFTMGR_GREEN_SOFT);
				grap.DrawImage(pImage, nPosX, nPosY, pImage->GetWidth(), pImage->GetHeight());
				nPosX += pImage->GetWidth();
			}
			if (pListData->m_bCharge == TRUE)
			{
				nPosX += 2;
				pImage = BkPngPool::Get(IDP_SOFTMGR_CHARGE_SOFT);
				grap.DrawImage(pImage, nPosX, nPosY, pImage->GetWidth(), pImage->GetHeight());
				nPosX += pImage->GetWidth();
			}
			if (pListData->m_bPlug == TRUE)
			{
				nPosX += 2;
				pImage = BkPngPool::Get(IDP_SOFTMGR_PLUGIN_SOFT);
				grap.DrawImage(pImage, nPosX, nPosY, pImage->GetWidth(), pImage->GetHeight());
			}

			//软件安装包名称
			CRect rcIntro = m_rcDescription;
			dcx.DrawText(pListData->m_strFileName, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcIntro.right = rcItem.Width() - SOFTMGR_LIST_RIGHT_WIDTH-10;
			if (rcProb.Width() < rcItem.Width() - SOFTMGR_LIST_RIGHT_WIDTH - 70)
			{
				rcIntro.right = rcIntro.left + rcProb.Width();
			}
			rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(pListData->m_strFileName, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );

			//状态
			if(pListData->m_bDownloading && !pListData->m_bWaitDownload) //正在下载
			{
				InitDownLoad();
				if(pListData->m_bLinking)
				{
					if (pListData->m_bLinkFailed)
					{
						m_rcFreeback.top = 18;
						m_rcFreeback.bottom = 39;
						m_rcTautology.top = 18;
						m_rcTautology.bottom = 39;
						m_rcCancel  = CRect(0,0,0,0);
					}
					else
					{
						m_rcFreeback = CRect(0,0,0,0);
						m_rcTautology = CRect(0,0,0,0);
					}
					m_rcLink = m_rcValue;
					m_rcContinue = CRect(0,0,0,0);
					m_rcValue = CRect(0,0,0,0);
					m_rcState = CRect(0,0,0,0);
				}
				else
				{
					m_rcLink = CRect(0,0,0,0);
					m_rcFreeback = CRect(0,0,0,0);
					m_rcTautology = CRect(0,0,0,0);
					InitDownLoad();
				}

				Gdiplus::Image *pImage;

				if (pListData->m_bDownLoad) //下载完成
				{
					CRect rcTmp;
					GetClientRect(&rcTmp);

					if (pListData->m_bInstalling)
					{
						m_rcInstall.right = rcTmp.Width() - 15;
						m_rcInstall.left = m_rcInstall.right - 57;
						m_rcInstall.top = 19;
						m_rcInstall.bottom = 39;
						m_rcStateBtn = CRect(0,0,0,0);
					}
					else
					{
						m_rcStateBtn.right = rcTmp.Width() - 15;
						m_rcStateBtn.left = m_rcStateBtn.right - 65;
						m_rcStateBtn.top = 15;
						m_rcStateBtn.bottom = 40;
						m_rcInstall = CRect(0,0,0,0);
					}

					m_rcState = CRect(0, 0, 0, 0);
					m_rcCancel = CRect(0, 0, 0, 0);	
					m_rcContinue = CRect(0, 0, 0, 0);

					//正在安装
					CRect rcInstall = m_rcInstall;
					CRect rcTmpCalc;
					dcx.DrawText(BkString::Get(IDS_SOFTMGR_8088), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
					rcInstall.left = rcInstall.right - rcTmpCalc.Width();
					rcInstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
					dcx.DrawText(BkString::Get(IDS_SOFTMGR_8088), -1, &rcInstall, DT_VCENTER | DT_SINGLELINE);

					//最右侧按钮
					CRect rcStateBtn = m_rcStateBtn;
					rcStateBtn.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_SET);

					if (m_bMouseOn)	//鼠标在按钮上
					{
						if(m_bMouseDown)
						{
							grap.DrawImage(pImage, rcStateBtn.left, rcStateBtn.top,rcStateBtn.Width()*2, 0,
								rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
						}
						else
						{
							grap.DrawImage(pImage, rcStateBtn.left, rcStateBtn.top,rcStateBtn.Width()*1, 0,
								rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
						}
					}
					else
					{
						{
							//按钮的正常状态
							grap.DrawImage(pImage, rcStateBtn.left, rcStateBtn.top, 0, 0,
								rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
						}
					}
				}

				//画进度条
				CRect rcProgress = m_rcProgress;
				rcProgress.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				Gdiplus::Image *pImageValue = BkPngPool::Get(IDP_LISTCTRL_PROGRESS_VALUE);
				Gdiplus::Image *pImageBottom = BkPngPool::Get(IDP_LISTCTRL_PROGRESS_BOTTOM);
				grap.DrawImage(pImageBottom, rcProgress.left, rcProgress.top, 0, 0, 1, 8, Gdiplus::UnitPixel);
				Gdiplus::TextureBrush brBottom(pImageBottom, Gdiplus::WrapModeTile, 1, 0, 1, 8);
				brBottom.TranslateTransform((Gdiplus::REAL)rcProgress.left + 1, (Gdiplus::REAL)rcProgress.top);
				grap.FillRectangle(&brBottom, rcProgress.left + 1, rcProgress.top, 100, 8);
				grap.DrawImage(pImageBottom, rcProgress.left + 101, rcProgress.top, 2, 0, 1, 8, Gdiplus::UnitPixel);
				Gdiplus::TextureBrush brValue(pImageValue, Gdiplus::WrapModeTile, 0,0,1,8);
				brValue.TranslateTransform((Gdiplus::REAL)rcProgress.left + 1, (Gdiplus::REAL)rcProgress.top);
				if (pListData->m_bLinking)
				{
					grap.FillRectangle(&brValue, rcProgress.left + 1, rcProgress.top, pListData->m_dwProgress, 8);//正在连接下载服务器
				}
				else
				{
					grap.FillRectangle(&brValue, rcProgress.left + 1, rcProgress.top, pListData->m_dwProgress, 8);
				}

				//画进度值
				CString strTmp;
				CRect rcValue = m_rcValue;
				rcValue.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				strTmp.Format(_T("%d%%"), pListData->m_dwProgress);
				dcx.DrawText(strTmp, -1, rcValue, DT_VCENTER | DT_SINGLELINE);

				//画连接服务器提示
				CRect rcCalc;
				CRect rcLink = m_rcLink;
				rcLink.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				if (pListData->m_bLinkFailed)
				{
					dcx.DrawText(BkString::Get(IDS_SOFTMGR_8089), -1, &rcCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
					rcLink.right = rcLink.left + rcCalc.Width() + 2;
					dcx.DrawText(BkString::Get(IDS_SOFTMGR_8089), -1, &rcLink, DT_VCENTER | DT_SINGLELINE);
				}
				else
				{
					dcx.DrawText(BkString::Get(IDS_SOFTMGR_8090), -1, &rcCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
					rcLink.right = rcLink.left + rcCalc.Width() + 2;
					dcx.DrawText(BkString::Get(IDS_SOFTMGR_8090), -1, &rcLink, DT_VCENTER | DT_SINGLELINE);
				}

				//画暂停或者继续按钮
				CRect rcContinue = m_rcContinue;
				rcContinue.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				if (pListData->m_bContinue)//正在下载，那么画暂停按钮
				{
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_PAUSE);

					if (m_bMouseOnPause)
					{
						if (m_bMouseDown)
						{
							Gdiplus::RectF rcDest(rcContinue.left, rcContinue.top, rcContinue.Width(), rcContinue.Height());
							grap.DrawImage(pImage, rcDest, rcContinue.Width() * 0, 0,
								rcContinue.Width(), rcContinue.Height(), Gdiplus::UnitPixel);
						}
						else
						{
							Gdiplus::RectF rcDest(rcContinue.left, rcContinue.top, rcContinue.Width(), rcContinue.Height());
							grap.DrawImage(pImage, rcDest, rcContinue.Width() * 0, 0,
								rcContinue.Width(), rcContinue.Height(), Gdiplus::UnitPixel);
						}
					}
					else
					{
						Gdiplus::RectF rcDest(rcContinue.left, rcContinue.top, rcContinue.Width(), rcContinue.Height());
						grap.DrawImage(pImage, rcDest, 0, 0,
							rcContinue.Width(), rcContinue.Height(), Gdiplus::UnitPixel);
					}
				}
				else if (pListData->m_bPause)//暂停状态则画继续按钮
				{
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_CONTINUE);

					if (m_bMouseOnPause)
					{
						if (m_bMouseDown)
						{
							Gdiplus::RectF rcDest(rcContinue.left, rcContinue.top, rcContinue.Width(), rcContinue.Height());
							grap.DrawImage(pImage, rcDest, rcContinue.Width() * 0/*2*/, 0,
								rcContinue.Width(), rcContinue.Height(), Gdiplus::UnitPixel);
						}
						else
						{
							Gdiplus::RectF rcDest(rcContinue.left, rcContinue.top, rcContinue.Width(), rcContinue.Height());
							grap.DrawImage(pImage, rcDest, rcContinue.Width() * 0/*1*/, 0,
								rcContinue.Width(), rcContinue.Height(), Gdiplus::UnitPixel);
						}
					}
					else
					{
						Gdiplus::RectF rcDest(rcContinue.left, rcContinue.top, rcContinue.Width(), rcContinue.Height());
						grap.DrawImage(pImage, rcDest, 0, 0,
							rcContinue.Width(), rcContinue.Height(), Gdiplus::UnitPixel);
					}
				}

				CRect rcCancel = m_rcCancel;
				rcCancel.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_CANCEL);
				Gdiplus::RectF rectCancel(rcCancel.left, rcCancel.top, rcCancel.Width(), rcCancel.Height());
				grap.DrawImage(pImage, rectCancel, 0, 0,
					rcCancel.Width(), rcCancel.Height(), Gdiplus::UnitPixel);

				//画下载速度或者下载状态
				CRect rcState = m_rcState;
				rcState.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				if (pListData->m_bPause)
				{
					dcx.DrawText(BkString::Get(IDS_SOFTMGR_8091), -1, rcState, DT_VCENTER | DT_SINGLELINE);
				}
				else 
				{
					dcx.DrawText(pListData->m_strSpeed, -1, rcState, DT_VCENTER | DT_SINGLELINE);
				}

				//画反馈文字
				hFntTmp = dcx.SelectFont(m_fntNameOn);
				dcx.SetTextColor(COLOR_MOUSE_ON_NAME);
				CRect rcTmp;
				GetClientRect(&rcTmp);
				CRect rcFreeback = m_rcFreeback;
				rcFreeback.right = rcItem.Width() - 35;
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8092), -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcFreeback.left = rcFreeback.right - rcProb.Width();
				pListData->m_rcFreeback = rcFreeback;
				rcFreeback.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8092), -1, &rcFreeback, DT_VCENTER | DT_SINGLELINE);
				//画重试文字
				CRect rcTautology = m_rcTautology;
				rcTautology.right = rcFreeback.left - 5;
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8093), -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcTautology.left = rcTautology.right - rcProb.Width();
				pListData->m_rcTautology = rcTautology;
				rcTautology.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8093), -1, &rcTautology, DT_VCENTER | DT_SINGLELINE);
				dcx.SetTextColor(clrOld);
				dcx.SelectFont(hFntTmp);

			}
			else
			{
				InitNormal();
				m_rcInstall = CRect(0,0,0,0);

				CRect rcTmp;
				GetClientRect(&rcTmp);
				if (pListData->m_bWaitDownload)
				{
					m_rcWaitDownload.right = rcTmp.Width() - 15;
					m_rcWaitDownload.left = m_rcWaitDownload.right - 57;
					m_rcWaitDownload.top = 19;
					m_rcWaitDownload.bottom = 39;
					m_rcInstall = CRect(0,0,0,0);
					m_rcStateBtn = CRect(0,0,0,0);
				}
				else if (pListData->m_bInstalling)
				{
					m_rcInstall.right = rcTmp.Width() - 35;
					m_rcInstall.left = m_rcInstall.right - 57;
					m_rcInstall.top = 19;
					m_rcInstall.bottom = 39;
					m_rcStateBtn = CRect(0,0,0,0);
					m_rcWaitDownload = CRect(0,0,0,0);
				}

				//画软件大小
				CRect rcSize = m_rcSize;
				rcSize.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(pListData->m_strSize, -1, &rcSize, DT_VCENTER | DT_SINGLELINE);

				CRect rcCharge = m_rcCharge;
				rcCharge.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				//是否收费
				if (pListData->m_bCharge)
				{
					hFntTmp = dcx.SelectFont(m_fntDefault);
					dcx.SetTextColor(COLOR_SOFTMGR_PLUG);
					rcCharge.left -= 5;
					dcx.DrawText(BkString::Get(IDS_SOFTMGR_8094), -1, &rcCharge, DT_VCENTER | DT_SINGLELINE);
					dcx.SetTextColor(clrOld);
					dcx.SelectFont(hFntTmp);
				}
				else
				{
					hFntTmp = dcx.SelectFont(m_fntPlug);
					dcx.SetTextColor(COLOR_SOFTMGR_NOPLUG);
					rcCharge.left -= 5;
					dcx.DrawText(BkString::Get(IDS_SOFTMGR_8095), -1, &rcCharge, DT_VCENTER | DT_SINGLELINE);
					dcx.SetTextColor(clrOld);
					dcx.SelectFont(hFntTmp);
				}

				CRect rcPlug = m_rcPlug;
				rcPlug.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				//是否有插件
				if (pListData->m_bPlug)
				{
					hFntTmp = dcx.SelectFont(m_fntDefault);
					dcx.SetTextColor(COLOR_SOFTMGR_PLUG);
					if(pListData->m_bCharge)
					{
						rcPlug.left -= 5;
						dcx.DrawText(BkString::Get(IDS_SOFTMGR_8096), -1, &rcPlug, DT_VCENTER | DT_SINGLELINE);
					}
					else
						dcx.DrawText(BkString::Get(IDS_SOFTMGR_8097), -1, &rcPlug, DT_VCENTER | DT_SINGLELINE);
					dcx.SetTextColor(clrOld);
					dcx.SelectFont(hFntTmp);
				}
				else
				{
					hFntTmp = dcx.SelectFont(m_fntPlug);
					dcx.SetTextColor(COLOR_SOFTMGR_NOPLUG);
					if(pListData->m_bCharge)
					{
						rcPlug.left -= 5;
						dcx.DrawText(BkString::Get(IDS_SOFTMGR_8098), -1, &rcPlug, DT_VCENTER | DT_SINGLELINE);
					}
					else
						dcx.DrawText(BkString::Get(IDS_SOFTMGR_8099), -1, &rcPlug, DT_VCENTER | DT_SINGLELINE);
					dcx.SetTextColor(clrOld);
					dcx.SelectFont(hFntTmp);
				}	

				//正在安装
				CRect rcInstall = m_rcInstall;
				CRect rcTmpCalc;
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8085), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcInstall.left = rcInstall.right - rcTmpCalc.Width();
				rcInstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8085), -1, &rcInstall, DT_VCENTER | DT_SINGLELINE);

				//最右侧按钮
				Gdiplus::Image *pImage;
				CRect rcStateBtn = m_rcStateBtn;
				rcStateBtn.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				if (pListData->m_bSetup)//已经安装
				{
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_DOWN);
				}
				else
				{
					if (pListData->m_bDownLoad)	// 没有安装但是已经下载
					{
						pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_SET);
					}
					else	// 没有安装也没有下载
					{
						pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_NODOWN);
					}
				}
				if (m_bMouseOn)	//鼠标在按钮上
				{
					if(m_bMouseDown)
					{
						grap.DrawImage(pImage, rcStateBtn.left, rcStateBtn.top, rcStateBtn.Width()*2, 0,
							rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
					}
					else
					{
						grap.DrawImage(pImage, rcStateBtn.left, rcStateBtn.top, rcStateBtn.Width(), 0,
							rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
					}
				}
				else
				{
					//按钮的正常状态
					grap.DrawImage(pImage, rcStateBtn.left, rcStateBtn.top, 0, 0,
						rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
				}
			}

			dcx.Detach();
		}
	}
	void DrawItem ( LPDRAWITEMSTRUCT lpDrawItemStruct )
	{
		if (!m_bDrawItem)
		{
			return;
		}

		CRect rc;
		GetClientRect(&rc);

		if ( m_dcMem == NULL )
		{
			m_dcMem.CreateCompatibleDC(lpDrawItemStruct->hDC);
			m_bmpMem.CreateCompatibleBitmap(lpDrawItemStruct->hDC, rc.Width(), rc.Height());
		}
		else
		{
			BITMAP bmp;
			m_bmpMem.GetBitmap(&bmp);
			int nWidth = bmp.bmWidth;
			int nHeight = bmp.bmHeight; 
			if (nWidth != rc.Width() || nHeight != rc.Height())
			{
				DeleteObject(m_bmpMem);
				m_bmpMem = NULL;
				m_bmpMem.CreateCompatibleBitmap(lpDrawItemStruct->hDC, rc.Width(), rc.Height());
			}
		}

		CBrush Brush;
		Brush.CreateSolidBrush(RGB(225,225,255));
		m_dcMem.SelectBrush(Brush);

		HGDIOBJ hOldBitmap = ::SelectObject(m_dcMem, m_bmpMem);

		_DrawItem(m_dcMem, lpDrawItemStruct);

		BitBlt(lpDrawItemStruct->hDC, 
			lpDrawItemStruct->rcItem.left,
			lpDrawItemStruct->rcItem.top,
			lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left,
			lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top,
			m_dcMem,
			lpDrawItemStruct->rcItem.left,
			lpDrawItemStruct->rcItem.top,
			SRCCOPY
			);

		::SelectObject(m_dcMem, hOldBitmap);		
	}

	void RefreshItem(int nIndex)
	{
		ATLASSERT(nIndex < m_arrData.GetSize() && nIndex >= 0);
		if (nIndex < 0 || nIndex >= m_arrData.GetSize())
		{
			return;
		}

		CRect rcIndex;
		GetItemRect(nIndex, &rcIndex, LVIR_BOUNDS);
		InvalidateRect(rcIndex,FALSE);
	}

	void RefreshRight(int nIndex)
	{
		ATLASSERT(nIndex < m_arrData.GetSize() && nIndex >= 0);
		if (nIndex < 0 || nIndex >= m_arrData.GetSize())
		{
			return;
		}

		CRect rcIndex;
		GetItemRect(nIndex, &rcIndex, LVIR_BOUNDS);
		rcIndex.left = rcIndex.right - SOFTMGR_LIST_RIGHT_WIDTH - 18;
		InvalidateRect(rcIndex, FALSE);
	}
	void RefreshProgress(int nIndex)
	{
		ATLASSERT(nIndex < m_arrData.GetSize() && nIndex >= 0);
		if (nIndex < 0 || nIndex >= m_arrData.GetSize())
		{
			return;
		}

		CSoftListItemData *pData = GetItemDataEx(nIndex);
		if (pData == NULL)
		{
			return;
		}

		if (pData->m_bDownloading)//下载
		{
			InitDownLoad();
			if(pData->m_bLinking)
			{
				if (pData->m_bLinkFailed)
				{
					m_rcFreeback.top = 18;
					m_rcFreeback.bottom = 39;
					m_rcTautology.top = 18;
					m_rcTautology.bottom = 39;
					m_rcCancel  = CRect(0,0,0,0);
				}
				else
				{
					m_rcFreeback = CRect(0,0,0,0);
					m_rcTautology = CRect(0,0,0,0);
				}
				m_rcLink = m_rcValue;
				m_rcContinue = CRect(0,0,0,0);
				m_rcValue = CRect(0,0,0,0);
				m_rcState = CRect(0,0,0,0);
			}
			else
			{
				m_rcLink = CRect(0,0,0,0);
				m_rcFreeback = CRect(0,0,0,0);
				m_rcTautology = CRect(0,0,0,0);
				InitDownLoad();
			}
		}
		else //已安装
		{
			InitNormal();
			CRect rcTmp;
			GetClientRect(&rcTmp);
			if (pData->m_bInstalling)
			{
				m_rcInstall.right = rcTmp.Width() - 15;
				m_rcInstall.left = m_rcInstall.right - 57;
				m_rcInstall.top = 19;
				m_rcInstall.bottom = 39;
				m_rcStateBtn = CRect(0,0,0,0);
			}
			else
			{
				m_rcStateBtn.right = rcTmp.Width() - 15;
				m_rcStateBtn.left = m_rcStateBtn.right - 65;
				m_rcStateBtn.top = 15;
				m_rcStateBtn.bottom = 40;
				m_rcInstall = CRect(0,0,0,0);
			}
		}

		CRect rcIndex;
		GetItemRect(nIndex, &rcIndex, LVIR_BOUNDS);
		rcIndex.left = rcIndex.right - SOFTMGR_LIST_RIGHT_WIDTH - 10;
		rcIndex.right -= 18;
		InvalidateRect(rcIndex, FALSE);
#if 0
		CRect rcIndex;
		GetItemRect(nIndex, &rcIndex, LVIR_BOUNDS);
		CRect rcTmp;
		rcTmp = m_rcProgress;
		rcTmp.OffsetRect(rcIndex.left, rcIndex.top);
		InvalidateRect(rcTmp, FALSE);

		rcTmp = m_rcState;
		rcTmp.OffsetRect(rcIndex.left,rcIndex.top);
		InvalidateRect(rcTmp, FALSE);

		rcTmp = m_rcValue;
		rcTmp.OffsetRect(rcIndex.left,rcIndex.top);
		InvalidateRect(rcTmp, FALSE);

		rcTmp = m_rcLink;
		rcTmp.OffsetRect(rcIndex.left,rcIndex.top);
		InvalidateRect(rcTmp, FALSE);
#endif
	}

	void DeleteAllItemsEx() //清空列表页删除数据
	{
		if(m_hWnd != NULL)
		{
			DeleteAllItems();
		}

		for (int i = 0; i < m_arrData.GetSize(); i++)
		{
			CSoftListItemData *pListData = m_arrData[i];
			if (pListData != NULL)
			{
				delete pListData;
				pListData = NULL;
			}
		}
		m_arrData.RemoveAll();		
	}

	void ClearAllItemData() //只清空列表，不删除数据
	{
		m_arrData.RemoveAll();	
		CListViewCtrl::DeleteAllItems();
	}

	void DeleteItem(LPDELETEITEMSTRUCT lParam)
	{
		ATLASSERT(lParam->itemID >= 0 && lParam->itemID < (UINT)m_arrData.GetSize());
		if (lParam->itemID < 0 || lParam->itemID >= (UINT)m_arrData.GetSize())
		{
			return;
		}

		m_arrData.RemoveAt(lParam->itemID);
		CListViewCtrl::DeleteItem(lParam->itemID);
		ReQueueList(lParam->itemID);
	}

	int GetItemCount()
	{
		return m_arrData.GetSize();
	}

	void ReQueueList(int nIndex)
	{
		for (int i = 0; i < m_arrData.GetSize(); i++)
		{
			if (m_arrData[i]->m_dwID > nIndex)
			{
				m_arrData[i]->m_dwID--;
			}
		}
	}

private:
	CRect m_rcIcon;				//图标
	CRect m_rcName;				//软件名称
	CRect m_rcDescription;		//软件描述
	CRect m_rcSize;				//软件大小
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
	CRect m_rcInstall;
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

	CFont		m_fntNameOn;	// 软件名称字体(鼠标在上面）
	CFont		m_fntNameNormal;// 软件名称字体（正常情况下）
	CFont		m_fntPlug;		// 插件字体
	CFont		m_fntDefault;	// 一般字体
	CFont		m_fntDanger;	// 特殊提示的字体

	CSimpleArray<CSoftListItemData*>	m_arrData;
	ISoftMgrDownloadCallback			*m_linkerCB;
	CToolTipCtrl				 m_wndToolTip;

	DWORD						 m_pHoverTip;
	LASTSTATE					m_enumLastMSState;


	CDC	  m_dcMem;
	CBitmap m_bmpMem;
};

#endif