#include "stdafx.h"
#include "beikesafe.h"
#include "SoftMgrListView.h"

#define SOFTMGR_LIST_ITEM_HEIGHT 55
#define SOFTMGR_LIST_RIGHT_WIDTH 310

#define	COLOR_SOFTMGR_PLUG		RGB(200,0,0)
#define	COLOR_SOFTMGR_NOPLUG	RGB(10,150,0)
#define COLOR_MOUSE_ON_NAME		RGB(0,100,200)
#define COLOR_GRAYTEXT			RGB(159,159,159)

#define STR_BTN_TIP_CANCEL      BkString::Get(IDS_SOFTMGR_8055)
#define STR_BTN_TIP_CONTINUE    BkString::Get(IDS_SOFTMGR_8086)
#define STR_BTN_TIP_PAUSE       BkString::Get(IDS_SOFTMGR_8087)
#define STR_TIP_DETAIL          BkString::Get(IDS_SOFTMGR_8100)

void CSoftMgrListView::OnSize(UINT nType, CSize size)
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

	UpdateBtnRect();

	SetMsgHandled(FALSE);
}

HRESULT	CSoftMgrListView::OnMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
{
	m_bMouseOnPause = FALSE;
	m_bMouseOn = FALSE;
	m_bMouseOnCancel = FALSE;
	m_bMouseOnName = FALSE;
	Invalidate(FALSE);

	return S_OK;
}

HRESULT	CSoftMgrListView::OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
{
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

LRESULT CSoftMgrListView::OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_wndToolTip.IsWindow())
	{
		MSG msg = { m_hWnd, uMsg, wParam, lParam };
		m_wndToolTip.RelayEvent(&msg);
	}

	SetMsgHandled(FALSE);
	return 0;
}

void CSoftMgrListView::SetShowDlgState()
{
	if (GetItemCount()==0)
	{
		CString	strFormat;
		strFormat = BkString::Get(IDS_SOFTMGR_8101);
		m_dlgMsg.SetItemVisible(10,FALSE);
		m_dlgMsg.SetItemVisible(1,FALSE);
		m_dlgMsg.SetItemVisible(2,TRUE);
		m_dlgMsg.SetItemText(3,strFormat);
		m_dlgMsg.ShowWindow(SW_SHOW);
	}
	else
		m_dlgMsg.ShowWindow(SW_HIDE);
}

void CSoftMgrListView::SetLoadShow(BOOL bShow)
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

void CSoftMgrListView::CreateTipCtrl()
{
	if ( !m_dlgMsg.Load(IDR_BK_LISTTIP_MGR_DLG) )
		return;
	m_dlgMsg.Create(m_hWnd);
	SetBkColor(COL_DEFAULT_WHITE_BG);
	m_dlgMsg.ShowWindow(SW_HIDE);

	m_wndToolTip.Create(m_hWnd);

	CToolInfo ti(0, m_hWnd);
	m_wndToolTip.AddTool(ti);
	m_wndToolTip.Activate(TRUE);
	m_wndToolTip.SetMaxTipWidth(300);
	m_wndToolTip.SetDelayTime(TTDT_AUTOPOP,5*1000);
}

void CSoftMgrListView::InitNormal()
{
	CRect rcTmp;
	GetClientRect(&rcTmp);

	m_rcStateBtn.right = rcTmp.Width() - 15;
	m_rcStateBtn.left = m_rcStateBtn.right - 65;
	m_rcStateBtn.top = 15;
	m_rcStateBtn.bottom = 40;

	m_rcStar.right = m_rcStateBtn.left - 43;
	m_rcStar.left = m_rcStar.right - 75;
	m_rcStar.top = 9;
	m_rcStar.bottom = 23;

	m_rcMark.right = m_rcStateBtn.left - 40;
	m_rcMark.left = m_rcMark.right - 75;
	m_rcMark.top = 33;
	m_rcMark.bottom = 45;

	m_rcSize.right = m_rcStar.left - 30;
	m_rcSize.left = m_rcSize.right - 57;
	m_rcSize.top = m_rcStateBtn.top;
	m_rcSize.bottom = m_rcStateBtn.bottom;

	m_rcPlug.right = m_rcSize.left - 30;
	m_rcPlug.left = m_rcPlug.right - 48;
	m_rcPlug.top = 9;
	m_rcPlug.bottom = 23;

	m_rcCharge.right = m_rcPlug.left;
	m_rcCharge.left = m_rcCharge.right - 24;
	m_rcCharge.top = 9;
	m_rcCharge.bottom =23;

	m_rcState = CRect(0, 0, 0, 0);
	m_rcCancel = CRect(0, 0, 0, 0);	
	m_rcContinue = CRect(0, 0, 0, 0);
	m_rcProgress = CRect(0, 0, 0, 0);
	m_rcValue = CRect(0, 0, 0, 0);
	m_rcWaitInstall = CRect(0,0,0,0);
	m_rcWaitDownload = CRect(0,0,0,0);
	m_rcUseOnKey	= CRect(0,0,0,0);
}

void CSoftMgrListView::InitDownLoad()
{
	CRect rcTmp;
	GetClientRect(&rcTmp);


	m_rcStar = CRect(0, 0, 0, 0);
	m_rcMark = CRect(0, 0, 0, 0);

	m_rcStateBtn = CRect(0, 0, 0, 0);
	m_rcCharge = CRect(0, 0, 0, 0);
	m_rcPlug = CRect(0, 0, 0, 0);

	m_rcSize = CRect(0, 0, 0, 0);

	m_rcWaitInstall = CRect(0,0,0,0);
	m_rcWaitDownload = CRect(0,0,0,0);
	m_rcUseOnKey	 = CRect(0,0,0,0);

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

LRESULT	CSoftMgrListView::OnCtlColor(HDC hDc, HWND hWnd)
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

CSoftListItemData*	CSoftMgrListView::GetItemDataEx(DWORD dwIndex)
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

	if (m_pDataPageItem.m_dwID == dwIndex)
		return &m_pDataPageItem;

	return NULL;
}

int CSoftMgrListView::SetItemDataEx(DWORD dwIndex, DWORD_PTR dwItemData)
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
	return SetItemData(dwIndex,(DWORD_PTR)dwItemData); 
}

BOOL CSoftMgrListView::GetItemRect(int nItem, LPRECT lpRect, UINT nCode)
{
	return  CListViewCtrl::GetItemRect(nItem, lpRect, nCode);
}

HRESULT	CSoftMgrListView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
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
		if (htiSel.iItem != -1 && htiSel.iItem != nIndex)
		{
			SetItemState(htiSel.iItem, 0,  LVIS_SELECTED/*|LVIS_FOCUSED*/);
		}
	}
	if (nIndex != -1)
	{
		SetItemState(nIndex,  LVIS_SELECTED/*|LVIS_FOCUSED*/,  LVIS_SELECTED/*|LVIS_FOCUSED*/);
	//	__super::SetSelectionMark(nIndex);
	}

	if ( nIndex >= 0 && nIndex < m_arrData.GetSize() && GetItemRect(nIndex,&rcItem,LVIR_BOUNDS) )
	{
		CSoftListItemData*	pData = (CSoftListItemData*)GetItemDataEx(nIndex);
		if(pData != NULL)
		{
			if(pData->m_dwFlags & SOFTMGR_LISTITEM_TITLE)
				_LButtonDownPageItem(pData, pts, rcItem, bMsgHandled);
			else
				_LButtonDownSoftItem(pData, pts, rcItem, bMsgHandled);
		}
	}

	bMsgHandled = TRUE;
	return S_OK;		
}

void CSoftMgrListView::_LButtonDownSoftItem(CSoftListItemData* pData, POINT pts, CRect rcItem, BOOL &bMsgHandled )
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
			m_rcWaitInstall = CRect(0,0,0,0);
		}
		else if (pData->m_bInstalling)
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

			m_rcStar.right = m_rcStateBtn.left - 43;
			m_rcStar.left = m_rcStar.right - 75;
			m_rcStar.top = 9;
			m_rcStar.bottom = 23;

			m_rcMark.right = m_rcStateBtn.left - 40;
			m_rcMark.left = m_rcMark.right - 75;
			m_rcMark.top = 33;
			m_rcMark.bottom = 45;

			m_rcInstall = CRect(0,0,0,0);
		}

		m_rcState = CRect(0, 0, 0, 0);
		m_rcCancel = CRect(0, 0, 0, 0);	
		m_rcContinue = CRect(0, 0, 0, 0);
	}

	pts.x -= rcItem.left;
	pts.y -= rcItem.top;

	if ( m_rcCheck.PtInRect(pts) ||
		pData->m_rcName.PtInRect(pts) ||
		m_rcCancel.PtInRect(pts) ||
		m_rcContinue.PtInRect(pts) ||
		m_rcIcon.PtInRect(pts) ||
		m_rcInstall.PtInRect(pts) ||
		pData->m_rcFreeback.PtInRect(pts) ||
		pData->m_rcTautology.PtInRect(pts) ||
		m_rcMark.PtInRect(pts) || 
		m_rcStar.PtInRect(pts))
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

void CSoftMgrListView::_LButtonDownPageItem(CSoftListItemData* pData, POINT pts, CRect rcItem, BOOL &bMsgHandled )
{
	m_bMouseDown = TRUE;
	bMsgHandled = TRUE;
}

HRESULT	CSoftMgrListView::OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
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

	if ( nIndex >= 0 && nIndex < m_arrData.GetSize() && GetItemRect(nIndex,&rcItem,LVIR_BOUNDS) )
	{
		CSoftListItemData*	pData = (CSoftListItemData*)GetItemDataEx(nIndex);

		if ( pData )
		{
			if(pData->m_dwFlags & SOFTMGR_LISTITEM_TITLE)
				_LButtonUpPageItem(pData, pts, rcItem, nIndex);
			else
				_LButtonUpSoftItem(pData, pts, rcItem, nIndex);
		}
	}

	m_bMouseDown = FALSE;
	return S_OK;		
}

void CSoftMgrListView::_LButtonUpSoftItem(CSoftListItemData* pData, POINT pts, CRect rcItem, int nIndex)
{
	if (pData->m_bDownloading && !pData->m_bWaitDownload)
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
			m_rcWaitInstall = CRect(0,0,0,0);
		}
		else if (pData->m_bWaitInstall)
		{
			m_rcWaitInstall.right = rcTmp.Width() - 15;
			m_rcWaitInstall.left = m_rcWaitInstall.right - 57;
			m_rcWaitInstall.top = 19;
			m_rcWaitInstall.bottom = 39;
			m_rcInstall = CRect(0,0,0,0);
			m_rcStateBtn = CRect(0,0,0,0);
		}
		else if (pData->m_bInstalling)
		{
			m_rcInstall.right = rcTmp.Width() - 15;
			m_rcInstall.left = m_rcInstall.right - 57;
			m_rcInstall.top = 19;
			m_rcInstall.bottom = 39;
			m_rcWaitInstall = CRect(0,0,0,0);
			m_rcStateBtn = CRect(0,0,0,0);
		}
		else
		{
			m_rcStateBtn.right = rcTmp.Width() - 15;
			m_rcStateBtn.left = m_rcStateBtn.right - 65;
			m_rcStateBtn.top = 15;
			m_rcStateBtn.bottom = 40;
			m_rcWaitInstall = CRect(0,0,0,0);
			m_rcInstall = CRect(0,0,0,0);
		}
	}

	if (pData->m_bDownLoad)
	{
		CRect rcTmp;
		GetClientRect(&rcTmp);
		if (pData->m_bWaitInstall)
		{
			m_rcWaitInstall.right = rcTmp.Width() - 15;
			m_rcWaitInstall.left = m_rcWaitInstall.right - 57;
			m_rcWaitInstall.top = 19;
			m_rcWaitInstall.bottom = 39;
			m_rcInstall = CRect(0,0,0,0);
			m_rcStateBtn = CRect(0,0,0,0);
		}
		else if (pData->m_bInstalling)
		{
			m_rcInstall.right = rcTmp.Width() - 15;
			m_rcInstall.left = m_rcInstall.right - 57;
			m_rcInstall.top = 19;
			m_rcInstall.bottom = 39;
			m_rcWaitInstall = CRect(0,0,0,0);
			m_rcStateBtn = CRect(0,0,0,0);
		}
		else
		{
			m_rcStateBtn.right = rcTmp.Width() - 15;
			m_rcStateBtn.left = m_rcStateBtn.right - 65;
			m_rcStateBtn.top = 15;
			m_rcStateBtn.bottom = 40;

			m_rcStar.right = m_rcStateBtn.left - 43;
			m_rcStar.left = m_rcStar.right - 75;
			m_rcStar.top = 9;
			m_rcStar.bottom = 23;

			m_rcMark.right = m_rcStateBtn.left - 40;
			m_rcMark.left = m_rcMark.right - 75;
			m_rcMark.top = 33;
			m_rcMark.bottom = 45;

			m_rcWaitInstall = CRect(0,0,0,0);
			m_rcInstall = CRect(0,0,0,0);
		}

		m_rcState = CRect(0, 0, 0, 0);
		m_rcCancel = CRect(0, 0, 0, 0);	
		m_rcContinue = CRect(0, 0, 0, 0);
	}

	pts.x -= rcItem.left;
	pts.y -= rcItem.top;

	CRect rcTmp;
	if ( m_rcCheck.PtInRect(pts) )
	{
		pData->m_bCheck = !pData->m_bCheck;
		rcTmp = m_rcCheck;
		rcTmp.OffsetRect(rcItem.left, rcItem.top);
		//			InvalidateRect(rcTmp,FALSE);
	}
	else if ( pData->m_rcName.PtInRect(pts) )
	{
		m_linkerCB->OnViewDetail(nIndex);
	}
	else if ( pData->m_rcNewInfo.PtInRect(pts) )
	{
		m_linkerCB->OnViewNew(nIndex);
	}
	else if ( m_rcIcon.PtInRect(pts))
	{
		m_linkerCB->OnViewDetail(nIndex);
	}
	else if ( m_rcMark.PtInRect(pts))
	{
		m_linkerCB->OnDaquanSoftMark(nIndex);
	}
	else if ( m_rcStar.PtInRect(pts))
	{
		m_linkerCB->OnDaquanSoftMark(nIndex);
	}
	else if(m_rcStateBtn.PtInRect(pts))
	{
		if (m_bMouseDown)
		{
			m_linkerCB->OnDownLoad(nIndex);
		}
	}
	else if(m_rcCancel.PtInRect(pts))
	{
		if (m_bMouseDown && !pData->m_bDownLoad)
		{
			pData->m_bDownloading = FALSE;
			m_linkerCB->OnCancel(nIndex);
		}
	}
	else if (m_rcContinue.PtInRect(pts))
	{
		if (m_bMouseDown  && !pData->m_bDownLoad)
		{
			pData->m_bDownloading = TRUE;
			if(pData->m_bContinue)
			{
				m_linkerCB->OnPause(nIndex);
			}
			else
			{
				m_linkerCB->OnContinue(nIndex);
			}
		}
	}
	else if (pData->m_rcTautology.PtInRect(pts))
	{
		m_linkerCB->OnTautilogyFromList(nIndex);
	}
	else if (pData->m_rcFreeback.PtInRect(pts))
	{
		m_linkerCB->OnFreebackFromList(nIndex);
	}
}

void CSoftMgrListView::_LButtonUpPageItem(CSoftListItemData* pData, POINT pts, CRect rcItem, int nIndex)
{
	pts.x -= rcItem.left;
	pts.y -= rcItem.top;

	for (int i = 0 ; i < 14; i++)
	{
		CRect rcName = m_rcPageBtnEx[i];
		if (m_bPageBtnEnableEx[i] && rcName.PtInRect(pts))
		{
			if (m_nCurPage != m_nPageBtnEx[i])
			{
				m_nCurPage = m_nPageBtnEx[i];
				RefreshRight(nIndex);

				m_linkerCB->OnSwitchPage(m_nCurPage);
			}

			return;
		}
	}
}

void CSoftMgrListView::SetClickLinkCallback( ISoftMgrCallback* opCB )
{
	m_linkerCB = opCB;
}

VOID CSoftMgrListView::SetPaintItem(BOOL bDrawItem)
{
	m_bDrawItem = bDrawItem;
}

void CSoftMgrListView::MeasureItem(LPMEASUREITEMSTRUCT lpMes)
{
	lpMes->itemHeight = SOFTMGR_LIST_ITEM_HEIGHT;
}

HRESULT	CSoftMgrListView::OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
{
	POINT	pts;
	CRect	rcItem;

	m_bMouseOnName = FALSE;
	m_bMouseOnCancel = FALSE;
	m_bMouseOnPause = FALSE;
	m_bMouseOn = FALSE;
	m_bMouseOnFreeback = FALSE;
	m_bMouseOnTautology = FALSE;

	bMsgHandled = FALSE;
	pts.x		= GET_X_LPARAM(lParam);
	pts.y		= GET_Y_LPARAM(lParam);

	LVHITTESTINFO hti;
	hti.pt = pts;
	SubItemHitTest( &hti );
	INT nIndex = hti.iItem;

	if (nIndex >= 0 && nIndex < m_arrData.GetSize() && GetItemRect(nIndex,&rcItem,LVIR_BOUNDS) )
	{
		CSoftListItemData*	pData = (CSoftListItemData*)GetItemDataEx(nIndex);
		if(pData != NULL)
		{
			if(pData->m_dwFlags & SOFTMGR_LISTITEM_TITLE)
				_MouseMovePageItem(pData, pts, rcItem);
			else
				_MouseMoveSoftItem(pData, pts, rcItem);
		}
	}

	TRACKMOUSEEVENT   tme;   
	tme.cbSize   =   sizeof(tme);   
	tme.hwndTrack   =   m_hWnd;   
	tme.dwFlags   =   TME_LEAVE;   
	 _TrackMouseEvent(&tme);  

	return S_OK;
}

void CSoftMgrListView::_MouseMovePageItem(CSoftListItemData* pData, POINT pts, CRect rcItem)
{
	pts.x -= rcItem.left;
	pts.y -= rcItem.top;

	for (int i = 0 ; i < 14; i++)
	{
		CRect rcName = m_rcPageBtnEx[i];
		if (m_bPageBtnEnableEx[i] && m_nPageBtnEx[i] != m_nCurPage && rcName.PtInRect(pts))
		{
			::SetCursor(::LoadCursor(NULL,IDC_HAND));
		}
	}
}

void CSoftMgrListView::_MouseMoveSoftItem(CSoftListItemData* pData, POINT pts, CRect rcItem)
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
			m_rcWaitInstall = CRect(0,0,0,0);
		}
		else if (pData->m_bWaitInstall)
		{
			m_rcWaitInstall.right = rcTmp.Width() - 15;
			m_rcWaitInstall.left = m_rcWaitInstall.right - 57;
			m_rcWaitInstall.top = 19;
			m_rcWaitInstall.bottom = 39;
			m_rcWaitDownload = CRect(0,0,0,0);
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
			m_rcWaitInstall = CRect(0,0,0,0);
			m_rcStateBtn = CRect(0,0,0,0);
		}
		else
		{
			m_rcStateBtn.right = rcTmp.Width() - 15;
			m_rcStateBtn.left = m_rcStateBtn.right - 65;
			m_rcStateBtn.top = 15;
			m_rcStateBtn.bottom = 40;
			m_rcWaitInstall = CRect(0,0,0,0);
			m_rcWaitDownload = CRect(0,0,0,0);
			m_rcInstall = CRect(0,0,0,0);
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

			m_rcStar.right = m_rcStateBtn.left - 43;
			m_rcStar.left = m_rcStar.right - 75;
			m_rcStar.top = 9;
			m_rcStar.bottom = 23;

			m_rcMark.right = m_rcStateBtn.left - 40;
			m_rcMark.left = m_rcMark.right - 75;
			m_rcMark.top = 33;
			m_rcMark.bottom = 45;

			m_rcInstall = CRect(0,0,0,0);
		}

		m_rcState = CRect(0, 0, 0, 0);
		m_rcCancel = CRect(0, 0, 0, 0);	
		m_rcContinue = CRect(0, 0, 0, 0);
	}

	pts.x -= rcItem.left;
	pts.y -= rcItem.top;

	CRect rcName = pData->m_rcName;
	if (/* m_*/rcName.PtInRect(pts) )
	{
		m_pRefresh = (DWORD)pData;
		m_bMouseOnName = TRUE;
		::SetCursor(::LoadCursor(NULL,IDC_HAND));
		if (((DWORD)pData != m_pHoverTip))
		{
			m_wndToolTip.UpdateTipText(STR_TIP_DETAIL, m_hWnd);
			m_pHoverTip = (DWORD)pData;
		}

		rcName.OffsetRect(rcItem.left, rcItem.top);
		if(m_enumLastMSState != MOUSE_LASTON_NAME)
		{
			InvalidateRect(rcName,FALSE);
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
	else if ( m_rcMark.PtInRect(pts))
	{
		m_enumLastMSState = MOUSE_LASTON_MARK;
		::SetCursor(::LoadCursor(NULL, IDC_HAND));
	}
	else if ( m_rcStar.PtInRect(pts))
	{
		m_enumLastMSState = MOUSE_LASTON_MARK;
		::SetCursor(::LoadCursor(NULL, IDC_HAND));
	}
	else if ( pData->m_rcDes.PtInRect(pts))// Add by HuangRuiKun, Bug:48684
	{
		m_enumLastMSState = MOUSE_LASTON_DESCRIPTION;
		if (((DWORD)pData != m_pHoverTip))
		{
			m_wndToolTip.UpdateTipText((LPCTSTR)pData->m_strDescription, m_hWnd);
			m_pHoverTip = (DWORD)pData;
		}
	}
	else if ( pData->m_rcNewInfo.PtInRect(pts))
	{
		m_enumLastMSState = MOUSE_LASTON_NEWINFO;
		::SetCursor(::LoadCursor(NULL, IDC_HAND));
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
				InvalidateRect(rcContinue,FALSE);
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

	if((m_enumLastMSState == MOUSE_LASTON_NAME && !m_bMouseOnName) )
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
	else if(m_enumLastMSState == MOUSE_LASTON_CONTINUEBTN && !m_bMouseOnPause)
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
	else if(m_enumLastMSState == MOUSE_LASTON_TAUTOLOGY && !m_bMouseOnTautology)
	{
		CRect rcStateBtn = m_rcTautology;
		rcStateBtn.OffsetRect(rcItem.left, rcItem.top);
		InvalidateRect(rcStateBtn, FALSE);
		m_enumLastMSState = MOUSE_LASTON_NONE;
	}
	else if(m_enumLastMSState == MOUSE_LASTON_FREEBACK && !m_bMouseOnFreeback)
	{
		CRect rcStateBtn = m_rcFreeback;
		rcStateBtn.OffsetRect(rcItem.left, rcItem.top);
		InvalidateRect(rcStateBtn, FALSE);
		m_enumLastMSState = MOUSE_LASTON_NONE;
	}
}

void CSoftMgrListView::_DrawPageItem(HDC hDC, LPDRAWITEMSTRUCT lpDrawItemStruct)
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

	Gdiplus::Graphics grap(hDC);	

	Gdiplus::SolidBrush mySolidBrush(Gdiplus::Color(255,255,255,255));
	if (rcWnd.Height() > GetItemCount() * 54)
	{
		grap.FillRectangle(&mySolidBrush, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, nColumWidth, 
			lpDrawItemStruct->rcItem.bottom-lpDrawItemStruct->rcItem.top);
	}
	else
	{
		grap.FillRectangle(&mySolidBrush, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, nColumWidth - 15, 
			lpDrawItemStruct->rcItem.bottom-lpDrawItemStruct->rcItem.top);
	}

	CDCHandle		dcx;
	DWORD			nOldMod;
	HFONT			hFntTmp;
	COLORREF		clrOld;

	dcx.Attach(hDC);
	nOldMod = dcx.GetBkMode();
	dcx.SetBkMode(TRANSPARENT);
	hFntTmp = dcx.SelectFont(m_fntDefault);
	clrOld = dcx.GetTextColor();
	dcx.SetTextColor(COLOR_MOUSE_ON_NAME);

	for (int i = 0 ; i < 14; i++)
	{
		if (m_bPageBtnEnableEx[i])
		{
			CRect rcName = m_rcPageBtnEx[i];
			rcName.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			CString strText;
			if (i == 10)
				strText.Format(L"%d...", m_nPageBtnEx[i]);
			else if (i == 11)
				strText.Format(L"...%d", m_nPageBtnEx[i]);
			else if (i == 12)
				strText = L"上一页";
			else if (i == 13)
				strText = L"下一页";
			else
				strText.Format(L"%d", m_nPageBtnEx[i]);

			if (i > -1 && i < 10 && m_nCurPage == m_nPageBtnEx[i])
			{
				Gdiplus::Image *pImage = BkPngPool::Get(IDP_SOFTMGR_PAGE_INDEX_BK);
				Gdiplus::RectF rcDest(rcName.CenterPoint().x - pImage->GetWidth()/2,
					rcName.CenterPoint().y - pImage->GetHeight()/2 - 2, 
					pImage->GetWidth(), 
					pImage->GetHeight());
				grap.DrawImage(pImage, rcDest, 0, 0,
					pImage->GetWidth(), pImage->GetHeight(), Gdiplus::UnitPixel);
			}

			dcx.DrawText(strText, -1, &rcName, DT_CENTER|DT_VCENTER|DT_SINGLELINE );
		}
	}

	dcx.SetTextColor(clrOld);
	dcx.SelectFont(hFntTmp);
}

void CSoftMgrListView::_DrawSoftItem(HDC hDC, LPDRAWITEMSTRUCT lpDrawItemStruct)
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
			if (rcWnd.Height() > GetItemCount() * 54)
			{
				grap.FillRectangle(&mySolidBrush, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, nColumWidth, 
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
			if (rcWnd.Height() > GetItemCount() * 54)
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

		//画check box的状态
		/*CRect rcCheck = m_rcCheck;
		rcCheck.OffsetRect( lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		HBITMAP hBmpCheck	= BkBmpPool::GetBitmap(IDB_BITMAP_LISTCTRL_CHECK);
		HBITMAP hBmpOld		= dcTmp.SelectBitmap(hBmpCheck);
		if ( pListData->m_bCheck )
		{
			dcx.BitBlt( rcCheck.left, rcCheck.top, rcCheck.Width(),rcCheck.Height(), dcTmp, 0,0,SRCCOPY);
		}
		else
		{
			dcx.BitBlt( rcCheck.left, rcCheck.top, rcCheck.Width(),rcCheck.Height(), dcTmp, 13,0,SRCCOPY);
		}
		dcTmp.SelectBitmap(hBmpOld);*/

		//画图标
		CRect rcIcon = m_rcIcon;
		rcIcon.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		if (pListData->m_pImage != NULL)
		{
			grap.DrawImage(pListData->m_pImage, rcIcon.left, rcIcon.top, rcIcon.Width(), rcIcon.Height());
		}
		if (!pListData->m_bIcon)
		{
			m_linkerCB->OnDownLoadIcon(pListData->m_strSoftID);
		}

		int nTypeWidth = 0;
		if (m_bShowType)
		{
			CRect rcType;
			CString strType;
			strType.Format(L"[%s]", pListData->m_strTypeShort);
			dcx.DrawText(strType, -1, &rcType, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			nTypeWidth = rcType.Width();
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

		int nLablesWidth = 0;
		Gdiplus::Image *pImage = NULL;
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
		if ((pListData->m_attri&SA_New) == SA_New)
		{
			if (nLablesWidth != 0)
				nLablesWidth += 2;
			pImage = BkPngPool::Get(IDP_SOFTMGR_NEW_SOFT);
			nLablesWidth += pImage->GetWidth();
		}
		rcName.right = rcItem.Width() - 325 - nLablesWidth;

		pListData->m_rcName = rcName;
		if (rcProb.Width() < rcItem.Width() - 325 - nLablesWidth - 50 - nTypeWidth)
		{
			rcName.left += nTypeWidth;
			rcName.right = rcName.left + rcProb.Width();
			pListData->m_rcName.left += nTypeWidth;
			pListData->m_rcName.right = rcName.left + rcProb.Width();
		}	
		else
		{
			rcName.left += nTypeWidth;
			pListData->m_rcName.left += nTypeWidth;
		}
		if (m_bShowType)
		{
			CRect rcType;
			rcType = rcName;
			rcType.right = rcType.left;
			rcType.left = rcType.right - nTypeWidth;
			rcType.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);

			HFONT			hFntTypeOld;
			COLORREF		clTypeOld;
			hFntTypeOld = dcx.SelectFont(m_fntDefault);
			clTypeOld = dcx.SetTextColor(RGB( 50,101,166));
			CString strType;
			strType.Format(L"[%s]", pListData->m_strTypeShort);
			dcx.DrawText(strType, -1, &rcType, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
			dcx.SelectFont(hFntTypeOld);
			dcx.SetTextColor(clTypeOld);
		}
		rcName.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		dcx.DrawText(pListData->m_strName, -1, &rcName, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
		dcx.SetTextColor(clrOld);
		dcx.SelectFont(hFntTmp);

		clrOld = dcx.SetTextColor(RGB(122,122,122));
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
			nPosX += pImage->GetWidth();
		}
		if ((pListData->m_attri&SA_New) == SA_New)
		{
			nPosX += 2;
			pImage = BkPngPool::Get(IDP_SOFTMGR_NEW_SOFT);
			grap.DrawImage(pImage, nPosX, nPosY, pImage->GetWidth(), pImage->GetHeight());
		}

		//软件简介
		CRect rcIntro = m_rcDescription;
		dcx.DrawText(pListData->m_strDescription, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
		int nNewInfoWidth = 0;
		if (pListData->m_bUpdate)
			nNewInfoWidth = 52;
		rcIntro.right = rcItem.Width() - SOFTMGR_LIST_RIGHT_WIDTH - 15 - nNewInfoWidth;
		if (rcIntro.left + rcProb.Width() < rcIntro.right)
			rcIntro.right = rcIntro.left + rcProb.Width();
		pListData->m_rcDes = rcIntro;
		if (pListData->m_bUpdate)
		{
			COLORREF clOld = dcx.GetTextColor();
			dcx.SetTextColor(RGB( 47, 99,165));
			CRect rcNewInfo = CRect(rcIntro.right + 2, rcIntro.top, rcIntro.right + nNewInfoWidth, rcIntro.bottom);
			pListData->m_rcNewInfo = rcNewInfo;
			rcNewInfo.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(L"新版功能", -1, &rcNewInfo, DT_LEFT|DT_SINGLELINE|DT_END_ELLIPSIS );
			dcx.SetTextColor(clOld);
		}
		else
			pListData->m_rcNewInfo = CRect(0, 0, 0, 0);
		rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		dcx.DrawText(pListData->m_strDescription, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );

		//状态
		if(pListData->m_bDownloading && !pListData->m_bWaitDownload && !pListData->m_bUsingForOneKey ) //正在下载
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

				if (pListData->m_bWaitInstall)
				{
					m_rcWaitInstall.right = rcTmp.Width() - 15;
					m_rcWaitInstall.left = m_rcWaitInstall.right - 57;
					m_rcWaitInstall.top = 19;
					m_rcWaitInstall.bottom = 39;
					m_rcInstall = CRect(0,0,0,0);
					m_rcStateBtn = CRect(0,0,0,0);
				}
				else if (pListData->m_bInstalling)
				{
					m_rcInstall.right = rcTmp.Width() - 15;
					m_rcInstall.left = m_rcInstall.right - 57;
					m_rcInstall.top = 19;
					m_rcInstall.bottom = 39;
					m_rcWaitInstall = CRect(0,0,0,0);
					m_rcStateBtn = CRect(0,0,0,0);

					if (pListData->m_bLinkFailed)
					{
						m_rcFreeback = CRect(0,0,0,0);
						m_rcTautology = CRect(0,0,0,0);
					}
				}
				else
				{
					if (pListData->m_bLinkFailed)
					{
						m_rcStateBtn = CRect(0,0,0,0);
						m_rcStar = CRect(0,0,0,0);
						m_rcMark = CRect(0,0,0,0);
					}
					else
					{
						m_rcStateBtn.right = rcTmp.Width() - 15;
						m_rcStateBtn.left = m_rcStateBtn.right - 65;
						m_rcStateBtn.top = 15;
						m_rcStateBtn.bottom = 40;

						m_rcStar.right = m_rcStateBtn.left - 43;
						m_rcStar.left = m_rcStar.right - 75;
						m_rcStar.top = 9;
						m_rcStar.bottom = 23;

						m_rcMark.right = m_rcStateBtn.left - 40;
						m_rcMark.left = m_rcMark.right - 75;
						m_rcMark.top = 33;
						m_rcMark.bottom = 45;
					}

					m_rcSize.right = m_rcStar.left - 30;
					m_rcSize.left = m_rcSize.right - 57;
					m_rcSize.top = m_rcStateBtn.top;
					m_rcSize.bottom = m_rcStateBtn.bottom;

					m_rcProgress = CRect(0,0,0,0);
					m_rcWaitInstall = CRect(0,0,0,0);
					m_rcInstall = CRect(0,0,0,0);
				}

				m_rcState = CRect(0, 0, 0, 0);
				m_rcCancel = CRect(0, 0, 0, 0);	
				m_rcContinue = CRect(0, 0, 0, 0);

				//等待安装
				CRect rcWaitInstall = m_rcWaitInstall;
				CRect rcTmpCalc;
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8102), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcWaitInstall.left = rcWaitInstall.right - rcTmpCalc.Width();
				rcWaitInstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8102), -1, &rcWaitInstall, DT_VCENTER | DT_SINGLELINE);

				//正在安装
				if (m_rcInstall.IsRectEmpty() == FALSE)
				{
					CRect rcInstall = m_rcInstall;
					dcx.DrawText(BkString::Get(IDS_SOFTMGR_8085), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
					rcInstall.left = rcInstall.right - rcTmpCalc.Width();
					rcInstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
					dcx.DrawText(BkString::Get(IDS_SOFTMGR_8085), -1, &rcInstall, DT_VCENTER | DT_SINGLELINE);
				}

				//最右侧按钮
				CRect rcStateBtn = m_rcStateBtn;
				rcStateBtn.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				if (pListData->m_bSetup)
				{
					if ((pListData->m_attri&SA_Green) == SA_Green)
						pImage = BkPngPool::Get(IDP_SOFTMGR_DOWNLOADED);
					else
						pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_DOWN);
				}
				else
				{
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_SET);
				}

				if (m_bMouseOn)	//鼠标在按钮上
				{
					if(m_bMouseDown)
					{
						Gdiplus::RectF rcDest(rcStateBtn.left, rcStateBtn.top, rcStateBtn.Width(), rcStateBtn.Height());
						grap.DrawImage(pImage, rcDest,rcStateBtn.Width()*2, 0,
							rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
					}
					else
					{
						Gdiplus::RectF rcDest(rcStateBtn.left, rcStateBtn.top, rcStateBtn.Width(), rcStateBtn.Height());
						grap.DrawImage(pImage, rcDest,rcStateBtn.Width()*1, 0,
							rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
					}
				}
				else
				{
					Gdiplus::RectF rcDest(rcStateBtn.left, rcStateBtn.top, rcStateBtn.Width(), rcStateBtn.Height());
					grap.DrawImage(pImage, rcDest, 0, 0,
						rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
				}

				//画软件大小
				CRect rcSize = m_rcSize;
				rcSize.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(pListData->m_strSize, -1, &rcSize, DT_VCENTER | DT_SINGLELINE);

				//画评分
				if (m_rcStar.IsRectEmpty() == FALSE)
				{
					Gdiplus::Image *pImage = NULL;
					int nStep = m_rcStar.Width()/5;
					CRect rcStar = m_rcStar;
					rcStar.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
					for (int i = 0; i < 5; i++)
					{
						if ((i+1)*2 <= pListData->m_fMark)
							pImage = BkPngPool::Get(IDP_SOFTMGR_STAR);
						else if ((i+1)*2 - 1 <= pListData->m_fMark)
							pImage = BkPngPool::Get(IDP_SOFTMGR_STAR_HALF);
						else
							pImage = BkPngPool::Get(IDP_SOFTMGR_STAR_OFF);

						if (pImage)
						{
							Gdiplus::RectF rcDest(rcStar.left + i*nStep, rcStar.top, pImage->GetWidth(), pImage->GetHeight());
							grap.DrawImage(pImage, rcDest, 0, 0,
								pImage->GetWidth(), pImage->GetHeight(), Gdiplus::UnitPixel);
						}
					}

					COLORREF clOld = dcx.GetTextColor();
					dcx.SetTextColor(RGB( 47, 99,165));
					CRect rcMark = m_rcMark;
					rcMark.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
					CString strMark;
					strMark.Format(L"%.1f分 投票", pListData->m_fMark);
					dcx.DrawText(strMark, -1, &rcMark, DT_VCENTER | DT_LEFT/*DT_RIGHT*/ | DT_SINGLELINE);
					dcx.SetTextColor(clOld);
				}
			}

			//画进度条
			if (m_rcProgress.IsRectEmpty() == FALSE)
			{
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
				grap.FillRectangle(&brValue, rcProgress.left + 1, rcProgress.top, pListData->m_dwProgress, 8);//正在连接下载服务器

				//画进度值
				CString strTmp;
				CRect rcValue = m_rcValue;
				rcValue.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				strTmp.Format(_T("%d%%"), pListData->m_dwProgress);
				dcx.DrawText(strTmp, -1, rcValue, DT_VCENTER | DT_SINGLELINE);
			}

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
			if (pListData->m_bUsingForOneKey)
			{
				m_rcUseOnKey.right = rcTmp.Width() - 15;
				m_rcUseOnKey.left = m_rcUseOnKey.right - 57;
				m_rcUseOnKey.top = 19;
				m_rcUseOnKey.bottom = 39;

				m_rcInstall = CRect(0,0,0,0);
				m_rcStateBtn = CRect(0,0,0,0);
				m_rcWaitInstall = CRect(0,0,0,0);
				m_rcWaitDownload = CRect(0,0,0,0);
			}
			else if (pListData->m_bWaitDownload)
			{
				m_rcWaitDownload.right = rcTmp.Width() - 15;
				m_rcWaitDownload.left = m_rcWaitDownload.right - 57;
				m_rcWaitDownload.top = 19;
				m_rcWaitDownload.bottom = 39;
				m_rcInstall = CRect(0,0,0,0);
				m_rcStateBtn = CRect(0,0,0,0);
				m_rcWaitInstall = CRect(0,0,0,0);
			}
			else if (pListData->m_bWaitInstall)
			{
				m_rcWaitInstall.right = rcTmp.Width() - 15;
				m_rcWaitInstall.left = m_rcWaitInstall.right - 57;
				m_rcWaitInstall.top = 19;
				m_rcWaitInstall.bottom = 39;
				m_rcWaitDownload = CRect(0,0,0,0);
				m_rcInstall = CRect(0,0,0,0);
				m_rcStateBtn = CRect(0,0,0,0);
			}
			else if (pListData->m_bInstalling)
			{
				m_rcInstall.right = rcTmp.Width() - 15;
				m_rcInstall.left = m_rcInstall.right - 57;
				m_rcInstall.top = 19;
				m_rcInstall.bottom = 39;
				m_rcWaitDownload = CRect(0,0,0,0);
				m_rcWaitInstall = CRect(0,0,0,0);
				m_rcStateBtn = CRect(0,0,0,0);
			}
			else
			{
				m_rcStateBtn.right = rcTmp.Width() - 15;
				m_rcStateBtn.left = m_rcStateBtn.right - 65;
				m_rcStateBtn.top = 15;
				m_rcStateBtn.bottom = 40;
				m_rcWaitInstall = CRect(0,0,0,0);
				m_rcWaitDownload = CRect(0,0,0,0);
				m_rcInstall = CRect(0,0,0,0);
			}

			// 画正在被快速装机使用的文字描述
			if (m_rcUseOnKey.Width()!=0)
			{
				CRect rcOK = m_rcUseOnKey;
				rcOK.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText( L"正在安装", -1, &rcOK, DT_VCENTER | DT_SINGLELINE);
			}

			//画软件大小
			CRect rcSize = m_rcSize;
			rcSize.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(pListData->m_strSize, -1, &rcSize, DT_VCENTER | DT_SINGLELINE);

			//CString strTemp;
			//if (pListData->m_bCharge == TRUE && pListData->m_bPlug == TRUE)
			//	strTemp.Format(L"%s/%s", BkString::Get(IDS_SOFTMGR_8039), BkString::Get(IDS_SOFTMGR_8041));
			//else if (pListData->m_bCharge == TRUE && pListData->m_bPlug == FALSE)
			//	strTemp = BkString::Get(IDS_SOFTMGR_8039);
			//else if (pListData->m_bCharge == FALSE && pListData->m_bPlug == TRUE)
			//	strTemp = BkString::Get(IDS_SOFTMGR_8041);
			//if (strTemp.GetLength())
			//{
			//	CRect rcCharge = m_rcCharge;
			//	rcCharge.right = m_rcPlug.right;
			//	rcCharge.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			//	hFntTmp = dcx.SelectFont(m_fntDefault);
			//	dcx.SetTextColor(COLOR_SOFTMGR_PLUG);
			//	rcCharge.left -= 5;
			//	dcx.DrawText(strTemp, -1, &rcCharge, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			//	dcx.SetTextColor(clrOld);
			//	dcx.SelectFont(hFntTmp);
			//}

			//画评分
			Gdiplus::Image *pImage = NULL;
			int nStep = m_rcStar.Width()/5;
			CRect rcStar = m_rcStar;
			rcStar.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			for (int i = 0; i < 5; i++)
			{
				if ((i+1)*2 <= pListData->m_fMark)
					pImage = BkPngPool::Get(IDP_SOFTMGR_STAR);
				else if ((i+1)*2 - 1 <= pListData->m_fMark)
					pImage = BkPngPool::Get(IDP_SOFTMGR_STAR_HALF);
				else
					pImage = BkPngPool::Get(IDP_SOFTMGR_STAR_OFF);

				if (pImage)
				{
					Gdiplus::RectF rcDest(rcStar.left + i*nStep, rcStar.top, pImage->GetWidth(), pImage->GetHeight());
					grap.DrawImage(pImage, rcDest, 0, 0,
						pImage->GetWidth(), pImage->GetHeight(), Gdiplus::UnitPixel);
				}
			}

			dcx.SetTextColor(RGB( 47, 99,165));
			CRect rcMark = m_rcMark;
			rcMark.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			CString strMark;
			strMark.Format(L"%.1f分 投票", pListData->m_fMark);
			dcx.DrawText(strMark, -1, &rcMark, DT_VCENTER | DT_LEFT/*DT_RIGHT*/ | DT_SINGLELINE);

			dcx.SetTextColor(RGB(0, 0, 0));
			//等待下载
			CRect rcWaitDowload = m_rcWaitDownload;
			CRect rcTmpCalc;
			dcx.DrawText(BkString::Get(IDS_SOFTMGR_8103), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcWaitDowload.left = rcWaitDowload.right - rcTmpCalc.Width();
			rcWaitDowload.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(BkString::Get(IDS_SOFTMGR_8103), -1, &rcWaitDowload, DT_VCENTER | DT_SINGLELINE);

			//等待安装
			CRect rcWaitInstall = m_rcWaitInstall;
			dcx.DrawText(BkString::Get(IDS_SOFTMGR_8102), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcWaitInstall.left = rcWaitInstall.right - rcTmpCalc.Width();
			rcWaitInstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(BkString::Get(IDS_SOFTMGR_8102), -1, &rcWaitInstall, DT_VCENTER | DT_SINGLELINE);

			//正在安装
			if (m_rcInstall.IsRectEmpty() == FALSE)
			{
				CRect rcInstall = m_rcInstall;
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8085), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcInstall.left = rcInstall.right - rcTmpCalc.Width();
				rcInstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(BkString::Get(IDS_SOFTMGR_8085), -1, &rcInstall, DT_VCENTER | DT_SINGLELINE);
			}

			//最右侧按钮
			CRect rcStateBtn = m_rcStateBtn;
			rcStateBtn.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			if (pListData->m_bSetup)//已经安装
			{
				if (pListData->m_bUpdate)//需要升级
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_UPDATE);//BkPngPool::Get(IDP_LISTCTRL_BTN_DOWN);
				else//允许重新安装
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_DOWN);
			}
			else
			{
				if (pListData->m_bDownLoad)	// 没有安装但是已经下载
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_SET);
				else	// 没有安装也没有下载
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_NODOWN);
			}
			if (m_bMouseOn)	//鼠标在按钮上
			{
				if(m_bMouseDown)
				{
					Gdiplus::RectF rcDest(rcStateBtn.left, rcStateBtn.top, rcStateBtn.Width(), rcStateBtn.Height());
					grap.DrawImage(pImage, rcDest, rcStateBtn.Width()*2, 0,
						rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
				}
				else
				{
					Gdiplus::RectF rcDest(rcStateBtn.left, rcStateBtn.top, rcStateBtn.Width(), rcStateBtn.Height());
					grap.DrawImage(pImage, rcDest, rcStateBtn.Width(), 0,
						rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
				}
			}
			else
			{
				Gdiplus::RectF rcDest(rcStateBtn.left, rcStateBtn.top, rcStateBtn.Width(), rcStateBtn.Height());
				grap.DrawImage(pImage, rcDest, 0, 0,
					rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
			}

		}

		dcx.Detach();
	}
}

void CSoftMgrListView::DrawItem ( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if (!m_bDrawItem)
	{
		return;
	}

	if (lpDrawItemStruct->itemID >= 0xffff)
		return;

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

	CSoftListItemData*	pListData = GetItemDataEx(lpDrawItemStruct->itemID);

	if ( pListData )
	{
		if(pListData->m_dwFlags & SOFTMGR_LISTITEM_TITLE)
			_DrawPageItem(m_dcMem, lpDrawItemStruct);
		else
			_DrawSoftItem(m_dcMem, lpDrawItemStruct);
	}

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

void CSoftMgrListView::RefreshItem(int nIndex)
{
	ATLASSERT(nIndex < m_arrData.GetSize() && nIndex >= 0);
	if (nIndex < 0 || nIndex >= m_arrData.GetSize())
	{
		return;
	}

	CRect rcIndex;
	GetItemRect(nIndex, &rcIndex, LVIR_BOUNDS);
	CListViewCtrl::InvalidateRect(rcIndex, FALSE);
}

void CSoftMgrListView::RefreshProgress(int nIndex)
{
	ATLASSERT(nIndex < m_arrData.GetSize() && nIndex >= 0);
	if (nIndex < 0 || nIndex >= m_arrData.GetSize())
	{
		return;
	}

	CSoftListItemData *pData = (CSoftListItemData*)GetItemDataEx(nIndex);
	if (pData == NULL)
	{
		return;
	}

	CRect rcIndex;
	GetItemRect(nIndex, &rcIndex, LVIR_BOUNDS);
	rcIndex.left = rcIndex.right - SOFTMGR_LIST_RIGHT_WIDTH - 18;
	rcIndex.right -= 18;
	InvalidateRect(rcIndex, FALSE);
}

void CSoftMgrListView::RefreshButton(int nIndex)
{
	ATLASSERT(nIndex < m_arrData.GetSize() && nIndex >= 0);
	if (nIndex < 0 || nIndex >= m_arrData.GetSize() || (IsWindow() == FALSE))
	{
		return;
	}

	CSoftListItemData *pData = (CSoftListItemData*)GetItemDataEx(nIndex);
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

		if (pData->m_bDownLoad) //下载完成
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
	CRect rcTmp;
	rcTmp = m_rcStateBtn;
	rcTmp.OffsetRect(rcIndex.left, rcIndex.top);
	CListViewCtrl::InvalidateRect(rcTmp, FALSE);
}

void CSoftMgrListView::RefreshIcon(int nIndex)
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

void CSoftMgrListView::RefreshRight(int nIndex)
{
	ATLASSERT(nIndex < m_arrData.GetSize() && nIndex >= 0);
	if (nIndex < 0 || nIndex >= m_arrData.GetSize() || (IsWindow() == FALSE))
	{
		return;
	}

	CRect rcIndex;
	GetItemRect(nIndex, &rcIndex, LVIR_BOUNDS);
	rcIndex.left = rcIndex.right - SOFTMGR_LIST_RIGHT_WIDTH - 18;
	InvalidateRect(rcIndex, FALSE);
}

void CSoftMgrListView::DeleteAllItemsEx() //清空列表页删除数据
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

void CSoftMgrListView::ClearAllItemData() //只清空列表，不删除数据
{
	m_nNameSort = SORT_TYPE_UNKNOW;
	m_nSizeSort = SORT_TYPE_UNKNOW;
	m_nTimeSort = SORT_TYPE_UNKNOW;

	m_arrData.RemoveAll();	
	CListViewCtrl::DeleteAllItems();
}

void CSoftMgrListView::DeleteItem(LPDELETEITEMSTRUCT lParam)
{
	ATLASSERT(lParam->itemID >= 0 && lParam->itemID < (UINT)m_arrData.GetSize());
	if (lParam->itemID < 0 || lParam->itemID >= (UINT)m_arrData.GetSize())
	{
		return;
	}

	CSoftListItemData *pData = GetItemDataEx(lParam->itemID);
	if (pData != NULL)
	{
		if (pData->m_pImage != NULL)
		{
			delete pData->m_pImage;
			pData->m_pImage = NULL;
		}
		delete pData;
		pData = NULL;
	}

	m_arrData.RemoveAt(lParam->itemID);
}

int CSoftMgrListView::GetItemCount()
{
	return m_arrData.GetSize();
}

void CSoftMgrListView::AppendPageItem(int nCurPage, int nItemCount)
{
	m_nCurPage = nCurPage;
	m_nItemCount = nItemCount;

	int nMax = m_nItemCount/PAGE_ITEM_MAX_COUNT + ((m_nItemCount%PAGE_ITEM_MAX_COUNT > 0) ? 1 : 0);
	int nBegin = 0;
	if (nCurPage > 3)
	{
		if (nMax >= 9 && nCurPage >= nMax - 7)
			nBegin = nMax - 9;
		else
			nBegin = nCurPage - 2;
	}
	else if (nCurPage <= 3)
		nBegin = 1;

	for (int i = 0; i < 10; i++)
	{
		if (nBegin + i <= nMax)
		{
			m_nPageBtnEx[i] = nBegin + i;
			m_bPageBtnEnableEx[i] = TRUE;
		}
		else
		{
			m_nPageBtnEx[i] = 0;
			m_bPageBtnEnableEx[i] = FALSE;
		}
	}

	if (m_nPageBtnEx[0] > 1)
	{
		m_nPageBtnEx[10] = 1;
		m_bPageBtnEnableEx[10] = TRUE;
	}
	else
	{
		m_nPageBtnEx[10] = 0;
		m_bPageBtnEnableEx[10] = FALSE;
	}

	if (m_nPageBtnEx[9] > 0 && m_nPageBtnEx[9] < nMax)
	{
		m_nPageBtnEx[11] = nMax;
		m_bPageBtnEnableEx[11] = TRUE;
	}
	else
	{
		m_nPageBtnEx[11] = 0;
		m_bPageBtnEnableEx[11] = FALSE;
	}

	if (m_nCurPage - 1 > 0)
	{
		m_nPageBtnEx[12] = m_nCurPage - 1;
		m_bPageBtnEnableEx[12] = TRUE;
	}
	else
	{
		m_nPageBtnEx[12] = 0;
		m_bPageBtnEnableEx[12] = FALSE;
	}

	if (m_nCurPage + 1 <= nMax)
	{
		m_nPageBtnEx[13] = m_nCurPage + 1;
		m_bPageBtnEnableEx[13] = TRUE;
	}
	else
	{
		m_nPageBtnEx[13] = 0;
		m_bPageBtnEnableEx[13] = FALSE;
	}

	m_pDataPageItem.m_dwID = GetItemCount();
	m_pDataPageItem.m_dwFlags = SOFTMGR_LISTITEM_TITLE;

	InsertItem(GetItemCount(), _T(""));
	SetItemDataEx(GetItemCount(), (DWORD_PTR)&m_pDataPageItem);
	SetPaintItem(TRUE);
}

void CSoftMgrListView::UpdateBtnRect()
{
	CRect rcWnd;
	GetClientRect(&rcWnd);

	int nCount = 0;
	for (int i = 0; i < 10; i++)
	{
		if (m_bPageBtnEnableEx[i])
			nCount++;
	}
	int nPosX = rcWnd.CenterPoint().x - (PAGE_ITEM_WIDTH*nCount + PAGE_ITEM_SPACE*(nCount - 1))/2;
	for (int i = 0; i < 10; i++)
	{
		if (m_bPageBtnEnableEx[i])
		{
			m_rcPageBtnEx[i].left = nPosX + i*(PAGE_ITEM_WIDTH + PAGE_ITEM_SPACE);
			m_rcPageBtnEx[i].top = 12;
			m_rcPageBtnEx[i].bottom = 37;
			m_rcPageBtnEx[i].right = m_rcPageBtnEx[i].left + PAGE_ITEM_WIDTH;
		}
		else
		{
			int nPosTemp = 0;
			if (i - 1)
				nPosTemp = m_rcPageBtnEx[i - 1].right;
			else
				nPosTemp = nPosX;
			m_rcPageBtnEx[i].left = nPosTemp;
			m_rcPageBtnEx[i].top = 12;
			m_rcPageBtnEx[i].bottom = 37;
			m_rcPageBtnEx[i].right = nPosTemp;
		}
	}

	if (m_bPageBtnEnableEx[10])
	{
		m_rcPageBtnEx[10].left = nPosX - PAGE_ITEM_SPACE - PAGE_ITEM2_WIDTH;
		m_rcPageBtnEx[10].top = 12;
		m_rcPageBtnEx[10].bottom = 37;
		m_rcPageBtnEx[10].right = nPosX - PAGE_ITEM_SPACE;
	}
	else
	{
		m_rcPageBtnEx[10].left = nPosX;
		m_rcPageBtnEx[10].top = 12;
		m_rcPageBtnEx[10].bottom = 37;
		m_rcPageBtnEx[10].right = nPosX;
	}
	//if (m_bPageBtnEnableEx[12])
	{
		int nPosTemp = m_rcPageBtnEx[10].left;
		m_rcPageBtnEx[12].left = nPosTemp - PAGE_ITEM_SPACE - PAGE_ITEM2_WIDTH;
		m_rcPageBtnEx[12].top = 12;
		m_rcPageBtnEx[12].bottom = 37;
		m_rcPageBtnEx[12].right = nPosTemp - PAGE_ITEM_SPACE;
	}

	int nPosX2 = rcWnd.CenterPoint().x + (PAGE_ITEM_WIDTH*nCount + PAGE_ITEM_SPACE*(nCount - 1))/2;
	if (m_bPageBtnEnableEx[11])
	{
		m_rcPageBtnEx[11].left = nPosX2 + PAGE_ITEM_SPACE;
		m_rcPageBtnEx[11].top = 12;
		m_rcPageBtnEx[11].bottom = 37;
		m_rcPageBtnEx[11].right = nPosX2 + PAGE_ITEM_SPACE + PAGE_ITEM2_WIDTH;
	}
	else
	{
		m_rcPageBtnEx[11].left = nPosX2;
		m_rcPageBtnEx[11].top = 12;
		m_rcPageBtnEx[11].bottom = 37;
		m_rcPageBtnEx[11].right = nPosX2;
	}
	//if (m_bPageBtnEnableEx[13])
	{
		int nPosTemp = m_rcPageBtnEx[11].right;
		m_rcPageBtnEx[13].left = nPosTemp + PAGE_ITEM_SPACE;
		m_rcPageBtnEx[13].top = 12;
		m_rcPageBtnEx[13].bottom = 37;
		m_rcPageBtnEx[13].right = nPosTemp + PAGE_ITEM_SPACE + PAGE_ITEM2_WIDTH;
	}
}

BOOL CSoftMgrListView::OnEraseBkgnd(CDCHandle dc)
{
	RECT rc;
	GetClientRect(&rc);
	dc.FillSolidRect(&rc, RGB(255, 255, 255));

	return TRUE;
}

void CSoftMgrListView::OnLButtonDBClk(UINT uMsg, CPoint ptx)
{
	LVHITTESTINFO hti;
	hti.pt = ptx;
	SubItemHitTest( &hti );

	CSoftListItemData *pData = GetItemDataEx(hti.iItem);
	if( pData != NULL && hti.iSubItem==0 && (pData->m_dwFlags & SOFTMGR_LISTITEM_TITLE) != SOFTMGR_LISTITEM_TITLE)
	{
		m_linkerCB->OnViewDetail(hti.iItem);
	}
}

void CSoftMgrListView::OnSetFocus(HWND hWnd) 
{
//	GetParent().GetParent().SetFocus();
	return;
}

void CSoftMgrListView::SetShowType(BOOL bShow /*= FALSE*/)
{
	m_bShowType = bShow;
}

BOOL CSoftMgrListView::FindItem(CSoftListItemData *pData)
{
	if ( pData == NULL)
	{
		return FALSE;
	}

	CSoftListItemData *pItemData = NULL;
	for ( int i = 0; i < m_arrData.GetSize(); i++)
	{
		pItemData = m_arrData[i];
		if ( pItemData != NULL && pItemData->m_strSoftID == pData->m_strSoftID)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CSoftMgrListView::GetSoftType(int nListIndex)
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

void CSoftMgrListView::UpdateAll()
{
	if(m_hWnd != NULL)
		DeleteAllItems();

	for (int i=0; i < m_arrData.GetSize(); i++)
	{
		CSoftListItemData*	pItem = NULL;
		pItem = m_arrData[i];

		pItem->m_dwID = CListViewCtrl::GetItemCount();
		InsertItem(pItem->m_dwID, _T(""));
		//SetItemDataEx(pItem->m_dwID, (DWORD_PTR)pItem);
		SetItemData(pItem->m_dwID,(DWORD_PTR)pItem);
	}

	InvalidateRect(NULL);
}

int CSoftMgrListView::GetListCenterPos(int nListIndex)
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
