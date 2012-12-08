#ifndef _SOFTGMR_LISTVIEW_UNINSTALL_H_
#define	_SOFTGMR_LISTVIEW_UNINSTALL_H_

#include "beikecolordef.h"
#include "SoftMgrItemData.h"
#include <common/utility.h>

#define XX_SOFTMGR_TIMER	1
#define SOFTMGR_UNINSTALL_LIST_ITEM_HEIGHT 55
#define SOFTMGR_LIST_RIGHT_WIDTH 310

#define	COLOR_SOFTMGR_NOPLUG	RGB(10,150,0)
#define COLOR_MOUSE_ON_NAME		RGB(0,100,200)

#define STR_TIP_DETAIL          BkString::Get(IDS_SOFTMGR_8100)

class ISoftMgrUnistallCallback
{
public:
	virtual void OnUninstall(DWORD dwIndex) = 0;
	virtual void OnViewUninstallDetail(DWORD dwIndex) = 0;
	virtual void OnDownloadIconFromUninstall(CString strID) = 0;
};

class CSoftMgrUninstallListView : public CWindowImpl<CSoftMgrUninstallListView, CListViewCtrl>, 
	public COwnerDraw<CSoftMgrUninstallListView>
{
public:
	CSoftMgrUninstallListView()
	{
		/*m_rcIcon = CRect(28, 11, 60, 43);
		m_rcName = CRect(68, 9, 128, 23);
		m_rcDescription = CRect(68, 33, 128, 45);*/
		m_rcIcon = CRect(10, 11, 42, 43);
		m_rcName = CRect(50, 9, 110, 23);
		m_rcDescription = CRect(50, 33, 110, 45);
		m_rcWaitUninstall = CRect(0,0,0,0);

		m_bDrawItem = FALSE;
		m_bMouseOn = FALSE;
		m_bMouseDown = FALSE;
		m_bMouseOnName = FALSE;
		m_bUninstalling = FALSE;

		m_bHinting = FALSE;

		m_pHoverTip = 0;

		m_fntNameOn.Attach( BkFontPool::GetFont(FALSE,TRUE,FALSE,0));
		m_fntNameNormal.Attach( BkFontPool::GetFont(TRUE, FALSE, FALSE, 0));
		m_fntPlug.Attach( BkFontPool::GetFont(BKF_DEFAULTFONT));
		m_fntDanger.Attach( BkFontPool::GetFont(BKF_BOLDFONT));
		m_fntDefault.Attach( BkFontPool::GetFont(BKF_DEFAULTFONT));
	}

	~CSoftMgrUninstallListView()
	{
		DeleteAllItemsEx();

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
		MOUSE_LASTON_SOFTICON
	};

public:
	BEGIN_MSG_MAP(CSoftMgrUninstallListView)  
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent) 
		MSG_WM_LBUTTONDBLCLK(OnLButtonDBClk)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_OCM_CTLCOLORLISTBOX(OnCtlColor)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CSoftMgrUninstallListView>,1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()   

	void OnSetFocus(HWND hWnd) 
	{
		GetParent().GetParent().SetFocus();
		return;
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

	HRESULT	OnMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
	{
		m_bMouseOn = FALSE;
		m_bMouseOnName = FALSE;
		Invalidate(FALSE);

		return S_OK;
	}

	HRESULT	OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
	{

		if ( m_bHinting )
		{
			bMsgHandled = TRUE;
			return S_OK;
		}

		POINT pts;
		GetCursorPos(&pts);
		CRect rcWnd;
		GetWindowRect(&rcWnd);
		pts.x -= rcWnd.left;
		pts.y -= rcWnd.top;

		LVHITTESTINFO hti;
		hti.pt = pts;
		SubItemHitTest( &hti );
		INT nIndex = hti.iItem;

		if (nIndex >= 0)
		{
			CSoftListItemData *pData = GetItemDataEx(nIndex);
			if (pData != NULL)
			{
				m_bMouseOnName = FALSE;
				CRect rcName = pData->m_rcName;
				CRect rcItem;
				GetItemRect(nIndex, &rcItem, LVIR_BOUNDS);
				rcName.OffsetRect(rcItem.left, rcItem.top);
				InvalidateRect(rcName, FALSE);
			}
		}

		bMsgHandled = FALSE;

		return S_OK;
	}

	void InitNormal()
	{
		CRect rcTmp;
		GetClientRect(&rcTmp);

		m_rcStateBtn.right = rcTmp.Width() - 15;
		m_rcStateBtn.left = m_rcStateBtn.right - 65;
		m_rcStateBtn.top = 15;
		m_rcStateBtn.bottom = 40;

		m_rcUninsatll = CRect(0,0,0,0);
		m_rcWaitUninstall = CRect(0,0,0,0);
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
		return SetItemData(dwIndex,(DWORD_PTR)pItemData->m_pImage);
	}

	BOOL GetItemRect(int nItem, LPRECT lpRect, UINT nCode)
	{
		return  CListViewCtrl::GetItemRect(nItem, lpRect, nCode);
	}


	HRESULT	OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
	{
		ModifyStyle(0, LVS_SINGLESEL);

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

		LVITEM htiSel;
		if(GetSelectedItem(&htiSel))
		{
			if (htiSel.iItem != -1)
			{
				SetItemState(htiSel.iItem, 0,  LVIS_SELECTED|LVIS_FOCUSED);
			}
		}
		if (nIndex != -1)
		{
			SetItemState(nIndex,  LVIS_SELECTED|LVIS_FOCUSED,  LVIS_SELECTED|LVIS_FOCUSED);
		}

		if ( /*!bOutSide &&*/ nIndex >= 0 && nIndex < 0xffff && GetItemRect(nIndex,&rcItem,LVIR_BOUNDS) )
		{
			CSoftListItemData*	pData = (CSoftListItemData*)GetItemDataEx(nIndex);
			if(pData != NULL)
			{
				if ( pData )
				{
					InitNormal();

					if (pData->m_bUninstalling)
					{
						m_rcUninsatll.right = rcItem.Width() - 25;
						m_rcUninsatll.left = m_rcUninsatll.right - 65;
						m_rcUninsatll.top = 15;
						m_rcUninsatll.bottom = 40;
						m_rcWaitUninstall = CRect(0,0,0,0);
						m_rcStateBtn = CRect(0,0,0,0);
					}
					else if(pData->m_bWaitUninstall)
					{
						m_rcWaitUninstall.right = rcItem.Width() - 25;
						m_rcWaitUninstall.left = m_rcWaitUninstall.right - 65;
						m_rcWaitUninstall.top = 15;
						m_rcWaitUninstall.bottom = 40;
						m_rcUninsatll = CRect(0,0,0,0);
						m_rcStateBtn = CRect(0,0,0,0);
					}

					pts.x -= rcItem.left;
					pts.y -= rcItem.top;

					if ( pData->m_rcName.PtInRect(pts) ||
						m_rcIcon.PtInRect(pts))
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
				InitNormal();

				if (pData->m_bUninstalling)
				{
					m_rcUninsatll.right = rcItem.Width() - 25;
					m_rcUninsatll.left = m_rcUninsatll.right - 65;
					m_rcUninsatll.top = 15;
					m_rcUninsatll.bottom = 40;
					m_rcWaitUninstall = CRect(0,0,0,0);
					m_rcStateBtn = CRect(0,0,0,0);
				}
				else if(pData->m_bWaitUninstall)
				{
					m_rcWaitUninstall.right = rcItem.Width() - 25;
					m_rcWaitUninstall.left = m_rcWaitUninstall.right - 65;
					m_rcWaitUninstall.top = 15;
					m_rcWaitUninstall.bottom = 40;
					m_rcUninsatll = CRect(0,0,0,0);
					m_rcStateBtn = CRect(0,0,0,0);
				}


				pts.x -= rcItem.left;
				pts.y -= rcItem.top;

				if ( pData->m_rcName.PtInRect(pts) )
				{
					m_linkerCB->OnViewUninstallDetail(nIndex);
				}
				else if ( m_rcIcon.PtInRect(pts))
				{
					m_linkerCB->OnViewUninstallDetail(nIndex);
				}
				else if(m_rcStateBtn.PtInRect(pts))
				{
					if (m_bMouseDown)
					{
						pData->m_bDownloading = TRUE;
						pData->m_bContinue = TRUE;
						pData->m_bPause = FALSE;
						m_linkerCB->OnUninstall(nIndex);
					}
				}

				InvalidateRect(rcItem, FALSE);
			}
		}

		m_bMouseDown = FALSE;
		return S_OK;		
	}

	void SetClickLinkCallback( ISoftMgrUnistallCallback* opCB )
	{
		m_linkerCB = opCB;
	}

	VOID SetPaintItem(BOOL bDrawItem)
	{
		m_bDrawItem = bDrawItem;
	}

	void MeasureItem(LPMEASUREITEMSTRUCT lpMes)
	{
		lpMes->itemHeight = SOFTMGR_UNINSTALL_LIST_ITEM_HEIGHT;
	}

	HRESULT	OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
	{
		POINT	pts;
		CRect	rcItem;
		BOOL	bOntip=FALSE;
		BOOL	bMove=FALSE;

		m_bMouseOnName = FALSE;
		m_bMouseOn = FALSE;
		bMsgHandled = FALSE;
		pts.x		= GET_X_LPARAM(lParam);
		pts.y		= GET_Y_LPARAM(lParam);

//		this->SetFocus();

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
				if ( pData )
				{
					pts.x -= rcItem.left;
					pts.y -= rcItem.top;

					m_pRefresh = (DWORD)pData;

					if ( pData->m_rcName.PtInRect(pts) )
					{
						m_bMouseOnName = TRUE;
						::SetCursor(::LoadCursor(NULL,IDC_HAND));
						if (((DWORD)pData != m_pHoverTip))
						{
							m_wndToolTip.UpdateTipText(STR_TIP_DETAIL, m_hWnd);
							m_pHoverTip = (DWORD)pData;
						}
						CRect rcName = pData->m_rcName;
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
						::SetCursor(::LoadCursor(NULL, IDC_HAND));
						if (((DWORD)pData != m_pHoverTip))
						{
							m_wndToolTip.UpdateTipText(STR_TIP_DETAIL, m_hWnd);
							m_pHoverTip = (DWORD)pData;
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
					else if(m_enumLastMSState == MOUSE_LASTON_STATEBTN && !m_bMouseOn)
					{
						m_bMouseOn = FALSE;
						CRect rcStateBtn = m_rcStateBtn;
						rcStateBtn.OffsetRect(rcItem.left, rcItem.top);
						InvalidateRect(rcStateBtn, FALSE);
						m_enumLastMSState = MOUSE_LASTON_NONE;
					}
				}
			}
		}

		TRACKMOUSEEVENT   tme;   
		tme.cbSize   =   sizeof(tme);   
		tme.hwndTrack   =   m_hWnd;   
		tme.dwFlags   =   TME_LEAVE;   
		_TrackMouseEvent(&tme);  

		return S_OK;
	}

	void _DrawItem(HDC hDC, LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		CRect rcWnd;
		INT	  nColumWidth = 0;
		CSoftListItemData*	pListData = NULL;

//		if ( lpDrawItemStruct->itemID >=0 && lpDrawItemStruct->itemID <= 0xffff )
		{
			pListData = GetItemDataEx(lpDrawItemStruct->itemID);
		}

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
				Gdiplus::SolidBrush mySolidBrush(Gdiplus::Color(255,255,240,205));
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
			CImageList imageList = GetImageList(LVSIL_SMALL);
			if (pListData->m_pImage != NULL)
			{
				grap.DrawImage(pListData->m_pImage, rcIcon.left, rcIcon.top, rcIcon.Width(), rcIcon.Height());
			}
			if (!pListData->m_bIcon)
			{
				CString strIconPath;
				if(SUCCEEDED(CAppPath::Instance().GetLeidianDataPath(strIconPath)))
				{
					strIconPath = strIconPath.Left(strIconPath.ReverseFind(_T('\\')) + 1);
					strIconPath.Append(_T("ksoft\\icon\\") + pListData->m_strSoftID + _T(".png"));
				}
				if (PathFileExists(strIconPath))
				{
					pListData->m_pImage = Gdiplus::Image::FromFile(strIconPath);
					if (pListData->m_pImage != NULL)
					{
						grap.DrawImage(pListData->m_pImage, rcIcon.left, rcIcon.top, rcIcon.Width(), rcIcon.Height());
						pListData->m_bIcon = TRUE;
						RefreshIcon(lpDrawItemStruct->itemID);
					}
				}
				else
				{
					m_linkerCB->OnDownloadIconFromUninstall(pListData->m_strSoftID);
				}
			}

			//画软件名称
			CRect rcItem;
			GetWindowRect(&rcItem);
			CRect rcName = m_rcName;
			CRect rcProb;
			if (m_bMouseOnName && (m_pRefresh == (DWORD)pListData))
			{
				hFntTmp = dcx.SelectFont(m_fntNameNormal);
				dcx.SetTextColor(COLOR_MOUSE_ON_NAME);
			}
			else
			{
				dcx.SetTextColor(RGB(0,0,0));
				hFntTmp = dcx.SelectFont(m_fntNameNormal);
			}
			dcx.DrawText(pListData->m_strName, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcName.right = rcItem.Width() - 120;
			pListData->m_rcName = rcName;
			if (rcProb.Width() < rcItem.Width() - 186)
			{
				pListData->m_rcName.right = rcName.left + rcProb.Width();
			}			
			rcName.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(pListData->m_strName, -1, &rcName, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
			dcx.SetTextColor(clrOld);
			dcx.SelectFont(hFntTmp);

			//软件简介
			CRect rcIntro = m_rcDescription;
			dcx.DrawText(pListData->m_strDescription, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcIntro.right = rcItem.Width() - 120;
			if (rcProb.Width() < rcItem.Width() - 186)
			{
				rcIntro.right = rcIntro.left + rcProb.Width();
			}
			rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(pListData->m_strDescription, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );

			{
				InitNormal();

				if (pListData->m_bUninstalling)
				{
					m_rcUninsatll.right = rcItem.Width() - 40;
					m_rcUninsatll.left = m_rcUninsatll.right - 65;
					m_rcUninsatll.top = 15;
					m_rcUninsatll.bottom = 40;
					m_rcWaitUninstall = CRect(0,0,0,0);
					m_rcStateBtn = CRect(0,0,0,0);
				}
				else if(pListData->m_bWaitUninstall)
				{
					m_rcWaitUninstall.right = rcItem.Width() - 40;
					m_rcWaitUninstall.left = m_rcWaitUninstall.right - 65;
					m_rcWaitUninstall.top = 15;
					m_rcWaitUninstall.bottom = 40;
					m_rcUninsatll = CRect(0,0,0,0);
					m_rcStateBtn = CRect(0,0,0,0);
				}

				//等待卸载
				CRect rcTmpCalc;
				CRect rcWaitUninstall = m_rcWaitUninstall;
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8105), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				m_rcWaitUninstall.left = m_rcWaitUninstall.right - rcTmpCalc.Width();
				m_rcWaitUninstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8105), -1, &m_rcWaitUninstall, DT_VCENTER | DT_SINGLELINE);

				//正在卸载
				CRect rcUninstall = m_rcUninsatll;
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8106), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcUninstall.left = rcUninstall.right - rcTmpCalc.Width();
				rcUninstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8106), -1, &rcUninstall, DT_VCENTER | DT_SINGLELINE);

				//最右侧按钮
				Gdiplus::Image *pImage;
				CRect rcStateBtn = m_rcStateBtn;
				rcStateBtn.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);


				if ( pListData->m_bSetup )
				{
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_UNINSTALL);
				}
				else
				{
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_UNINSTALLED);
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

	void OnLButtonDBClk(UINT uMsg, CPoint ptx)
	{
		LVHITTESTINFO hti;
		hti.pt = ptx;
		SubItemHitTest( &hti );

		CSoftListItemData *pData = GetItemDataEx(hti.iItem);
		if( pData != NULL && hti.iSubItem==0 )
		{
			m_linkerCB->OnViewUninstallDetail(hti.iItem);
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
		if (nIndex < 0 || nIndex >= m_arrData.GetSize() || (IsWindow() == FALSE))
		{
			return;
		}

		CRect rcIndex;
		GetItemRect(nIndex, &rcIndex, LVIR_BOUNDS);
		InvalidateRect(rcIndex, FALSE);
	}

	void RefreshIcon(int nIndex)
	{
		ATLASSERT(nIndex < m_arrData.GetSize() && nIndex >= 0);
		if (nIndex < 0 || nIndex >= m_arrData.GetSize() || (IsWindow() == FALSE))
		{
			return;
		}

		CRect rcItem;
		CRect rcIcon = m_rcIcon;
		GetItemRect(nIndex, &rcItem, LVIR_BOUNDS);
		rcIcon.OffsetRect(rcItem.left, rcItem.top);
		InvalidateRect(rcIcon, FALSE);
	}

	void DeleteAllItemsEx()
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

	void DeleteItem(LPDELETEITEMSTRUCT lParam)
	{
		ATLASSERT(lParam->itemID >= 0 && lParam->itemID < (UINT)m_arrData.GetSize());
		if (lParam->itemID < 0 || lParam->itemID >= (UINT)m_arrData.GetSize())
		{
			return;
		}

		CSoftListItemData *pData = GetItemDataEx(lParam->itemID);
		if (pData != NULL)
		{
			delete pData;
			pData = NULL;
		}

		m_arrData.RemoveAt(lParam->itemID);
		CListViewCtrl::DeleteItem(lParam->itemID);
	}

	int GetItemCount()
	{
		return m_arrData.GetSize();
	}

public:
	BOOL		m_bHinting;

private:
	CRect m_rcIcon;				//图标
	CRect m_rcName;				//软件名称
	CRect m_rcDescription;		//软件描述
	CRect m_rcSize;				//软件大小
	CRect m_rcStateBtn;			//状态按钮
	CRect m_rcUninsatll;
	CRect m_rcWaitUninstall;

	BOOL m_bDrawItem;			//是否有需要重绘制的项
	CBrush	m_hBGBrush;			//背景刷
	BOOL m_bMouseOn;			//鼠标是否在状态按钮上
	BOOL m_bMouseDown;			//鼠标是否按下
	BOOL m_bMouseOnName;		//鼠标在软件名称上面
	BOOL m_bUninstalling;

	CFont		m_fntNameOn;	// 软件名称字体(鼠标在上面）
	CFont		m_fntNameNormal;// 软件名称字体（正常情况下）
	CFont		m_fntPlug;		// 插件字体
	CFont		m_fntDefault;	// 一般字体
	CFont		m_fntDanger;	// 特殊提示的字体

	CSimpleArray<CSoftListItemData*>	m_arrData;
	ISoftMgrUnistallCallback			*m_linkerCB;
	CToolTipCtrl				 m_wndToolTip;

	DWORD						 m_pHoverTip;
	DWORD						m_pRefresh;
	LASTSTATE					m_enumLastMSState;


	CDC	  m_dcMem;
	CBitmap m_bmpMem;
};

#endif