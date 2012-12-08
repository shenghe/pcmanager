#include "stdafx.h"
#include "beikesafe.h"
#include "SoftMgrUninstallListViewEx.h"
#include "beikecolordef.h"

#define SOFTMGR_UNINSTALL_LIST_ITEM_HEIGHT_EX 54
#define SOFTMGR_LIST_RIGHT_WIDTH 310

#define	COLOR_SOFTMGR_PLUG		RGB(210,100,0)
#define	COLOR_SOFTMGR_NOPLUG	RGB(10,150,0)
#define COLOR_MOUSE_ON_NAME		RGB(0,100,200)

#define STR_TIP_DETAIL          BkString::Get(IDS_SOFTMGR_8100)

CSoftMgrUninstallListViewEx::CSoftMgrUninstallListViewEx()
{
	m_rcIcon = CRect(10, 11, 42, 43);
	m_rcName = CRect(50, 9, 110, 23);
//	m_rcViewDetail = CRect(50, 33, 120, 45);
	m_rcSize = CRect(50, 33, 170, 45);
	m_rcFrequency = CRect(190, 33, 310, 45);
	m_rcWaitUninstall = CRect(0,0,0,0);
	m_rcDeleteDesktopIcon = CRect(0,0,0,0);
	m_rcRudimental = CRect(0,0,0,0);
	m_rcViewDetail = CRect(0,0,0,0);
	m_rcPath = CRect(0, 0, 0, 0);

	m_bStyleRudiment = FALSE;
//	m_bShowClearShortCut = FALSE;
	m_bDrawItem = FALSE;
	m_bMouseOn = FALSE;
	m_bMouseDown = FALSE;
	m_bMouseOnName = FALSE;
	m_bUninstalling = FALSE;

	m_bHinting = FALSE;

	m_pHoverTip = 0;

	m_nListID = -1;

	m_nListStyle = LIST_STYLE_NORMAL;

	m_fntNameOn.Attach( BkFontPool::GetFont(FALSE,TRUE,FALSE,0));
	m_fntNameNormal.Attach( BkFontPool::GetFont(TRUE, FALSE, FALSE, 0));
	m_fntPlug.Attach( BkFontPool::GetFont(BKF_DEFAULTFONT));
	m_fntDanger.Attach( BkFontPool::GetFont(BKF_BOLDFONT));
	m_fntDefault.Attach( BkFontPool::GetFont(BKF_DEFAULTFONT));
}

CSoftMgrUninstallListViewEx::~CSoftMgrUninstallListViewEx()
{
	DeleteAllItemsEx();

	m_fntNameOn.Detach();
	m_fntNameNormal.Detach();
	m_fntPlug.Detach();
	m_fntDanger.Detach();
	m_fntDefault.Detach();
}

void CSoftMgrUninstallListViewEx::OnSetFocus(HWND hWnd) 
{
	GetParent().GetParent().SetFocus();
	return;
}

LRESULT CSoftMgrUninstallListViewEx::OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_wndToolTip.IsWindow())
	{
		MSG msg = { m_hWnd, uMsg, wParam, lParam };
		m_wndToolTip.RelayEvent(&msg);
	}

	SetMsgHandled(FALSE);
	return 0;
}

void CSoftMgrUninstallListViewEx::CreateTipCtrl()
{
	m_wndToolTip.Create(m_hWnd);

	CToolInfo ti(0, m_hWnd);
	m_wndToolTip.AddTool(ti);
	m_wndToolTip.Activate(TRUE);
	m_wndToolTip.SetMaxTipWidth(300);
	m_wndToolTip.SetDelayTime(TTDT_AUTOPOP,1000);
}

HRESULT	CSoftMgrUninstallListViewEx::OnMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
{
	m_bMouseOn = FALSE;
	m_bMouseOnName = FALSE;
	Invalidate(FALSE);

	return S_OK;
}

HRESULT	CSoftMgrUninstallListViewEx::OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
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
		CUninstallSoftInfo *pData = GetItemDataEx(nIndex);
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

void CSoftMgrUninstallListViewEx::InitNormal()
{
	CRect rcTmp;
	GetClientRect(&rcTmp);

	m_rcStateBtn.right = rcTmp.Width() - 15;
	m_rcStateBtn.left = m_rcStateBtn.right - 65;
	m_rcStateBtn.top = 15;
	m_rcStateBtn.bottom = 40;

	m_rcViewDetail.right = m_rcStateBtn.right - 200;
	m_rcViewDetail.left = m_rcViewDetail.right - 60;
	m_rcViewDetail.top = 33;
	m_rcViewDetail.bottom = 45;
	/*if (m_bShowClearShortCut)
	{
		m_rcDeleteDesktopIcon.right = m_rcStateBtn.left - 20;
		m_rcDeleteDesktopIcon.left = m_rcDeleteDesktopIcon.right - 72;
		m_rcDeleteDesktopIcon.top = 33;
		m_rcDeleteDesktopIcon.bottom = 45;
	}
	else
	{
		m_rcDeleteDesktopIcon = CRect(0, 0, 0, 0);
	}*/

	m_rcUninsatll = CRect(0,0,0,0);
	m_rcWaitUninstall = CRect(0,0,0,0);
}

LRESULT	CSoftMgrUninstallListViewEx::OnCtlColor(HDC hDc, HWND hWnd)
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

CUninstallSoftInfo*	CSoftMgrUninstallListViewEx::GetItemDataEx(DWORD dwIndex)
{
	if (dwIndex < 0 || dwIndex >= (DWORD)m_arrData.GetSize())
	{
		return NULL;
	}

	for ( int i=0; i<m_arrData.GetSize(); i++)
	{
		if (m_arrData[i]->m_dwDataID == dwIndex)
			return m_arrData[i];
	}
	return NULL;
}

int CSoftMgrUninstallListViewEx::SetItemDataEx(DWORD dwIndex, DWORD_PTR dwItemData)
{
	CUninstallSoftInfo*	pItemData=(CUninstallSoftInfo*)dwItemData;
	ATLASSERT(pItemData!=NULL);

	for (int i= 0;i<m_arrData.GetSize();i++)
	{
		if (m_arrData[i]->m_dwDataID==pItemData->m_dwDataID)
		{
			ATLASSERT(FALSE);
			return -1;
		}
	}
	m_arrData.Add(pItemData);
	return SetItemData(dwIndex,(DWORD_PTR)pItemData->m_pImage);
}

BOOL CSoftMgrUninstallListViewEx::GetItemRect(int nItem, LPRECT lpRect, UINT nCode)
{
	return  CListViewCtrl::GetItemRect(nItem, lpRect, nCode);
}


HRESULT	CSoftMgrUninstallListViewEx::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
{
	ModifyStyle(0, LVS_SINGLESEL);

	POINT	pts;
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

	if ( nIndex >= 0 && nIndex < m_arrData.GetSize() && GetItemRect(nIndex, &rcItem, LVIR_BOUNDS) )
	{
		CUninstallSoftInfo*	pData = (CUninstallSoftInfo*)GetItemDataEx(nIndex);
		if(pData != NULL)
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
				pData->m_rcDeleteIcon.PtInRect(pts) ||
				pData->m_rcViewDetail.PtInRect( pts ) ||
				pData->m_rcFreq.PtInRect( pts ) ||
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

	return S_OK;		
}

HRESULT	CSoftMgrUninstallListViewEx::OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
{
	POINT	pts;
	CRect	rcItem;

	bMsgHandled = FALSE;
	pts.x = GET_X_LPARAM(lParam);
	pts.y = GET_Y_LPARAM(lParam);

	LVHITTESTINFO hti;
	hti.pt = pts;
	SubItemHitTest( &hti );
	int nIndex = hti.iItem;

	if ( nIndex >= 0 && nIndex < m_arrData.GetSize() && GetItemRect(nIndex, &rcItem, LVIR_BOUNDS) )
	{
		CUninstallSoftInfo*	pData = (CUninstallSoftInfo*)GetItemDataEx(nIndex);

		if ( pData != NULL )
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
				m_linkerCB->OnViewUninstallDetailEx(m_nListID, nIndex);
			}
			else if ( m_rcIcon.PtInRect(pts))
			{
				m_linkerCB->OnViewUninstallDetailEx(m_nListID, nIndex);
			}
			else if(m_rcStateBtn.PtInRect(pts))
			{
				if (m_bMouseDown)
				{
					m_linkerCB->OnUninstallEx(m_nListID, nIndex);
				}
			}
			else if ( pData->m_rcDeleteIcon.PtInRect(pts))
			{
				if ( pData->m_bShowDeleteIcon )
				{
					m_linkerCB->OnClearShortCutMenu(m_nListID, pData);
				}
			}
			else if ( pData->m_rcViewDetail.PtInRect( pts ))
			{
				m_linkerCB->OnViewUninstallDetailEx(m_nListID, nIndex);
			}
			else if ( pData->m_rcFreq.PtInRect(pts) )
			{
#if 0
				if ( pData->m_strFreq.IsEmpty() && pData->m_bCalcFreq )
				{
					m_linkerCB->OnStartProtection( m_nListID, pData);
				}
#endif
			}
		}
	}

	m_bMouseDown = FALSE;
	return S_OK;		
}

void CSoftMgrUninstallListViewEx::SetClickLinkCallback( ISoftMgrUnistallCallbackEx* opCB )
{
	m_linkerCB = opCB;
}

VOID CSoftMgrUninstallListViewEx::SetPaintItem(BOOL bDrawItem)
{
	m_bDrawItem = bDrawItem;
}

void CSoftMgrUninstallListViewEx::MeasureItem(LPMEASUREITEMSTRUCT lpMes)
{
	lpMes->itemHeight = SOFTMGR_UNINSTALL_LIST_ITEM_HEIGHT_EX;
}

HRESULT	CSoftMgrUninstallListViewEx::OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
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

	static CPoint ptLast;

	if (ptLast != pts)
		bMove=TRUE;

	ptLast = pts;

	LVHITTESTINFO hti;
	hti.pt = pts;
	SubItemHitTest( &hti );
	INT nIndex = hti.iItem;

	if ( nIndex >= 0 && nIndex < m_arrData.GetSize() && GetItemRect(nIndex, &rcItem, LVIR_BOUNDS) )
	{
		CUninstallSoftInfo*	pData = (CUninstallSoftInfo*)GetItemDataEx(nIndex);
		if(pData != NULL)
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

				// 仅在已安装状态即卸载按钮时，显示为手型
				//if ( pData->m_dwUninstalled == SOFT_INSTALLED )
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
				
				CRect rcStateBtn = m_rcStateBtn;
				rcStateBtn.OffsetRect(rcItem.left, rcItem.top);
				
				if(m_enumLastMSState != MOUSE_LASTON_STATEBTN)
				{
					InvalidateRect(rcStateBtn, FALSE);
					m_enumLastMSState = MOUSE_LASTON_STATEBTN;
				}
			}
			else if ( pData->m_rcViewDetail.PtInRect(pts))
			{
				::SetCursor(::LoadCursor(NULL,IDC_HAND));
				if (((DWORD)pData != m_pHoverTip))
				{
					m_wndToolTip.UpdateTipText(STR_TIP_DETAIL, m_hWnd);
					m_pHoverTip = (DWORD)pData;
				}
			}
			else if ( pData->m_rcDeleteIcon.PtInRect(pts))
			{
				if ( pData->m_bShowDeleteIcon )
				{
					::SetCursor(::LoadCursor(NULL,IDC_HAND));
				}
			}
			else if ( pData->m_rcFreq.PtInRect( pts ) )
			{
				if ( pData->m_strFreq.IsEmpty() && pData->m_bCalcFreq )
				{
//					::SetCursor( ::LoadCursor( NULL, IDC_HAND ));
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

	TRACKMOUSEEVENT   tme;   
	tme.cbSize   =   sizeof(tme);   
	tme.hwndTrack   =   m_hWnd;   
	tme.dwFlags   =   TME_LEAVE;   
	_TrackMouseEvent(&tme);  

	return S_OK;
}

void CSoftMgrUninstallListViewEx::_DrawItem(HDC hDC, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rcWnd;
	INT	  nColumWidth = 0;
	CUninstallSoftInfo*	pListData = NULL;
	pListData = GetItemDataEx(lpDrawItemStruct->itemID);

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
			if ( pListData->m_bIconLeg )
			{
				grap.DrawImage(pListData->m_pImage, rcIcon.left, rcIcon.top, rcIcon.Width(), rcIcon.Height());
			}
			else
			{
				pListData->m_pImage = NULL;
			}
		}

		if (!pListData->m_bIcon)
		{
			m_linkerCB->OnDownloadIconFromUninstallEx(m_nListID, pListData->m_strSoftID);
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
		rcName.right = rcItem.Width() - 300;
		pListData->m_rcName = rcName;
		if (rcProb.Width() < rcItem.Width() - 366)
		{
			pListData->m_rcName.right = rcName.left + rcProb.Width();
		}			
		rcName.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		dcx.DrawText(pListData->m_strName, -1, &rcName, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
		dcx.SetTextColor(clrOld);
		dcx.SelectFont(hFntTmp);

		CRect rcIntro;
		if (pListData->m_bRudimental)
		{
			m_rcSize = CRect(0,0,0,0);
			m_rcFrequency = CRect(0,0,0,0);
			m_rcDeleteDesktopIcon = CRect(0,0,0,0);
			m_rcRudimental = CRect(0,0,0,0);
			m_rcViewDetail = CRect(0,0,0,0);

			//软件简介
			CRect rcIntro;
			rcIntro = pListData->m_rcName;
			rcIntro.left = rcIntro.right + 10;
			dcx.SetTextColor(RGB(48, 100, 165));
			dcx.DrawText(BkString::Get(IDS_SOFTMGR_8107), -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcIntro.right = rcIntro.left + rcProb.Width();
			pListData->m_rcViewDetail = rcIntro;
			rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(BkString::Get(IDS_SOFTMGR_8107), -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
			dcx.SetTextColor(clrOld);

			m_rcPath = CRect(50, 33, 100, 45);
			CRect rcPath = m_rcPath;
			CRect rcTmp;
			dcx.DrawText(pListData->m_strPath, -1, &rcTmp, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcPath.right = rcItem.Width() - 304;
			if (rcPath.Width() < rcItem.Width() - 370)
			{
				rcPath.right = rcPath.left + rcTmp.Width();
			}
			rcPath.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(pListData->m_strPath, -1, &rcPath, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS );
		}
		else
		{
			//软件简介
	//		rcIntro = pListData->m_rcName;
	//		rcIntro.left = rcIntro.right + 10;
			InitNormal();
			rcIntro = m_rcViewDetail;
			dcx.SetTextColor(RGB(48, 100, 165));
			dcx.DrawText(BkString::Get(IDS_SOFTMGR_8108), -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcIntro.right = rcIntro.left + rcProb.Width();
			pListData->m_rcViewDetail = rcIntro;
			rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(BkString::Get(IDS_SOFTMGR_8108), -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
			dcx.SetTextColor(clrOld);

			m_rcSize = CRect(50, 33, 170, 45);
			m_rcFrequency = CRect(190, 33, 310, 45);

			if( pListData->loc.IsEmpty() )
			{
				CRect rcText = m_rcSize;
				CString strText;
				dcx.SetTextColor(RGB(255,0,0));
				strText = BkString::Get(IDS_SOFTMGR_8109);
				dcx.DrawText(strText, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcText.right = rcText.left + rcProb.Width();
				rcText.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(strText, -1, &rcText, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );	
				dcx.SetTextColor(clrOld);
				strText = BkString::Get(IDS_SOFTMGR_8110);
				rcText.left = rcText.right - 1;
				dcx.DrawText( strText, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcText.right = rcIntro.left + rcProb.Width();	
				dcx.DrawText(strText, -1, &rcText, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
			}
			else
			{
				//占用空间
				CString strSize;
				rcIntro = m_rcSize;
				if (pListData->m_strSize.IsEmpty())
				{
					if ( pListData->m_bCalcSizing )
					{
						strSize = BkString::Get(IDS_SOFTMGR_8111);
					}
					else
					{
						strSize = BkString::Get(IDS_SOFTMGR_8112);
					}
					dcx.DrawText( strSize, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
					rcIntro.right = rcIntro.left + rcProb.Width();	
					rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
					dcx.DrawText(strSize, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );	
				}
				else
				{
					strSize = BkString::Get(IDS_SOFTMGR_8113);
					dcx.DrawText( strSize, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
					rcIntro.right = rcIntro.left + rcProb.Width();	
					rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
					dcx.DrawText(strSize, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );	

		//			dcx.SetTextColor(RGB(255, 0, 0));
					rcIntro.left = rcIntro.right + 2;
					dcx.DrawText( pListData->m_strSize, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
					rcIntro.right = rcIntro.left + rcProb.Width();	
					dcx.DrawText( pListData->m_strSize, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );	
					dcx.SetTextColor(clrOld);
				}

				//使用频率
				CString strFreq;
				rcIntro = m_rcFrequency;
				if (pListData->m_strFreq.IsEmpty())
				{
					if (pListData->m_bCalcFreq)
					{
						//					dcx.SetTextColor(COLOR_MOUSE_ON_NAME);
						strFreq = BkString::Get(IDS_SOFTMGR_8114);
						dcx.DrawText(strFreq, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
						rcIntro.right = rcIntro.left + rcProb.Width();	
						pListData->m_rcFreq = rcIntro;
						rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
						dcx.DrawText(strFreq, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );	
						//					dcx.SetTextColor(clrOld);
#if 0
						CPen newPen;
						newPen.CreatePen(PS_SOLID,1,RGB(0, 100, 200)); //样式，宽度，颜色   
						HPEN hOldPen=dcx.SelectPen(newPen);  //保存画笔 
						dcx.MoveTo(rcIntro.left, rcIntro.bottom);
						dcx.LineTo(rcIntro.right, rcIntro.bottom);
						newPen.DeleteObject();
						dcx.SelectPen(hOldPen);
#endif
					}
					else
					{
						dcx.SetTextColor(RGB(255,0,0));
						strFreq = _T("！");
						dcx.DrawText(strFreq, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
						rcIntro.right = rcIntro.left + rcProb.Width();	
						pListData->m_rcFreq = rcIntro;
						rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
						dcx.DrawText(strFreq, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );	
						dcx.SetTextColor(clrOld);
						strFreq = BkString::Get(IDS_SOFTMGR_8115);
						rcIntro.left = rcIntro.right - 1;
						dcx.DrawText(strFreq, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
						rcIntro.right = rcIntro.left + rcProb.Width();	
						dcx.DrawText(strFreq, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );	

					}
				}
				else
				{	
					dcx.SetTextColor(RGB(0,0,0));
					strFreq = BkString::Get(IDS_SOFTMGR_8116);
					dcx.DrawText(strFreq, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
					rcIntro.right = rcIntro.left + rcProb.Width();	
					pListData->m_rcFreq = rcIntro;
					rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
					dcx.DrawText(strFreq, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );	
					dcx.SetTextColor(clrOld);

	//				dcx.SetTextColor(RGB(255, 0, 0));
					rcIntro.left = rcIntro.right + 2;
					dcx.DrawText( pListData->m_strFreq, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
					rcIntro.right = rcIntro.left + rcProb.Width();	
					dcx.DrawText( pListData->m_strFreq, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );	
					dcx.SetTextColor(clrOld);

				}
			}
		}


		{
			InitNormal();

			if ( pListData->m_bShowDeleteIcon )
			{
				m_rcDeleteDesktopIcon.right = m_rcStateBtn.left - 30;
				m_rcDeleteDesktopIcon.left = m_rcDeleteDesktopIcon.right - 94;
				m_rcDeleteDesktopIcon.top = 33;
				m_rcDeleteDesktopIcon.bottom = 45;
			}

			if (pListData->m_bNoUninstallInfo)
			{
				m_rcRudimental.right = rcItem.Width() - 40;
				m_rcRudimental.left = m_rcRudimental.right - 80;
				m_rcRudimental.top = 15;
				m_rcRudimental.bottom = 40;
				m_rcWaitUninstall = CRect(0,0,0,0);
				m_rcStateBtn = CRect(0,0,0,0);
				m_rcUninsatll = CRect(0,0,0,0);
			}
			else if (pListData->m_bUninstalling)
			{
				m_rcUninsatll.right = rcItem.Width() - 40;
				m_rcUninsatll.left = m_rcUninsatll.right - 80;
				m_rcUninsatll.top = 15;
				m_rcUninsatll.bottom = 40;
				m_rcWaitUninstall = CRect(0,0,0,0);
				m_rcStateBtn = CRect(0,0,0,0);
				m_rcRudimental = CRect(0,0,0,0);
			}
			else if(pListData->m_bWaitUninstall)
			{
				m_rcWaitUninstall.right = rcItem.Width() - 40;
				m_rcWaitUninstall.left = m_rcWaitUninstall.right - 80;
				m_rcWaitUninstall.top = 15;
				m_rcWaitUninstall.bottom = 40;
				m_rcUninsatll = CRect(0,0,0,0);
				m_rcStateBtn = CRect(0,0,0,0);
				m_rcRudimental = CRect(0,0,0,0);
			}

			//删除桌面图标
			rcIntro = m_rcDeleteDesktopIcon;
			dcx.SetTextColor(RGB(48, 100, 165));
			CString strDelete;
			if ( m_nListStyle == LIST_STYLE_NORMAL )
			{
				strDelete = _T("");
			}
			else if ( m_nListStyle == LIST_STYLE_DESKTOP )
			{
				if ( pListData->m_bShowDeleteIcon )
				{
					strDelete = BkString::Get(IDS_SOFTMGR_8117);
				}
				else
				{
					strDelete = _T("");
				}
			}
			else if ( m_nListStyle == LIST_STYLE_STARTMENU )
			{
				if ( pListData->m_bShowDeleteIcon )
				{
					strDelete = BkString::Get(IDS_SOFTMGR_8118);
				}
				else
				{
					strDelete = _T("");
				}
			}
			dcx.DrawText( strDelete, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcIntro.right = rcIntro.left + rcProb.Width();	
			pListData->m_rcDeleteIcon = rcIntro;
			rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText( strDelete, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
			if (pListData->m_bShowDeleteIcon)
			{
				CPen newPen;
				newPen.CreatePen(PS_SOLID,1,RGB(48, 100, 165)); //样式，宽度，颜色   
				HPEN hOldPen=dcx.SelectPen(newPen);  //保存画笔 
				dcx.MoveTo(rcIntro.left, rcIntro.bottom);
				dcx.LineTo(rcIntro.right, rcIntro.bottom);
				newPen.DeleteObject();
				dcx.SelectPen(hOldPen);
			}
			dcx.SetTextColor(clrOld);


			//等待卸载
			CRect rcTmpCalc;
			CRect rcWaitUninstall = m_rcWaitUninstall;
			if (pListData->m_bRudimental)
			{
				/*dcx.DrawText(_T("等待卸载中"), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				m_rcWaitUninstall.left = m_rcWaitUninstall.right - rcTmpCalc.Width();
				m_rcWaitUninstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(_T("等待卸载中"), -1, &m_rcWaitUninstall, DT_VCENTER | DT_SINGLELINE);*/
			}
			else
			{
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8105), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				m_rcWaitUninstall.left = m_rcWaitUninstall.right - rcTmpCalc.Width();
				m_rcWaitUninstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8105), -1, &m_rcWaitUninstall, DT_VCENTER | DT_SINGLELINE);
			}

			//正在卸载
			CRect rcUninstall = m_rcUninsatll;
			if (pListData->m_bRudimental)
			{
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8119), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcUninstall.left = rcUninstall.right - rcTmpCalc.Width();
				rcUninstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8119), -1, &rcUninstall, DT_VCENTER | DT_SINGLELINE);
			}
			else
			{
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8106), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcUninstall.left = rcUninstall.right - rcTmpCalc.Width();
				rcUninstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8106), -1, &rcUninstall, DT_VCENTER | DT_SINGLELINE);
			}

			//无卸载项
			CRect rcRudimental = m_rcRudimental;
			dcx.DrawText(BkString::Get(IDS_SOFTMGR_8120), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcRudimental.right = rcRudimental.left + rcTmpCalc.Width();
			rcRudimental.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(BkString::Get(IDS_SOFTMGR_8120), -1, &rcRudimental, DT_VCENTER | DT_SINGLELINE);

			//最右侧按钮
			Gdiplus::Image *pImage;
			CRect rcStateBtn = m_rcStateBtn;
			rcStateBtn.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);

			if (pListData->m_bRudimental)
			{
				pImage = BkPngPool::Get(IDP_SOFTMGR_CLEAR);
			}
			else if ( pListData->m_bSetup )
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

void CSoftMgrUninstallListViewEx::OnLButtonDBClk(UINT uMsg, CPoint ptx)
{
	LVHITTESTINFO hti;
	hti.pt = ptx;
	SubItemHitTest( &hti );

	CUninstallSoftInfo *pData = GetItemDataEx(hti.iItem);
	if( pData != NULL && hti.iSubItem==0 )
	{
		
		m_linkerCB->OnViewUninstallDetailEx(m_nListID, hti.iItem);
		
	}
}

void CSoftMgrUninstallListViewEx::DrawItem ( LPDRAWITEMSTRUCT lpDrawItemStruct )
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

void CSoftMgrUninstallListViewEx::RefreshItem(int nIndex)
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

void CSoftMgrUninstallListViewEx::RefreshIcon(int nIndex)
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

void CSoftMgrUninstallListViewEx::DeleteAllItemsEx()
{
	if(m_hWnd != NULL)
	{
		DeleteAllItems();
	}

	for (int i = 0; i < m_arrData.GetSize(); i++)
	{
		CUninstallSoftInfo *pListData = m_arrData[i];
		if (pListData != NULL)
		{
			pListData->m_dwDataID = -1;
		}
	}
	m_arrData.RemoveAll();		
}

void CSoftMgrUninstallListViewEx::DeleteItem(LPDELETEITEMSTRUCT lParam)
{
	ATLASSERT(lParam->itemID >= 0 && lParam->itemID < (UINT)m_arrData.GetSize());
	if (lParam->itemID < 0 || lParam->itemID >= (UINT)m_arrData.GetSize())
	{
		return;
	}

	CUninstallSoftInfo *pData = GetItemDataEx(lParam->itemID);
	if (pData != NULL)
	{
		delete pData;
		pData = NULL;
	}

	m_arrData.RemoveAt(lParam->itemID);
	CListViewCtrl::DeleteItem(lParam->itemID);
}

int CSoftMgrUninstallListViewEx::GetItemCount()
{
	return m_arrData.GetSize();
}

//BOOL CSoftMgrUninstallListViewEx::SetShowClearDesktopIcon(BOOL bShow /* = FALSE */)
//{
//	BOOL bOldShow = m_bShowClearShortCut;
//	m_bShowClearShortCut = bShow;
//
//	return bOldShow;
//}

void CSoftMgrUninstallListViewEx::SetListViewID(int nListId)
{
	ATLASSERT(nListId >= 0);
	m_nListID = nListId;
}

void CSoftMgrUninstallListViewEx::SetListStyle(BOOL bRudimental)
{
	m_bStyleRudiment = bRudimental;
}

void CSoftMgrUninstallListViewEx::SetListType(int nType /* = LIST_STYLE_NORMAL */)
{
	m_nListStyle = nType;
}