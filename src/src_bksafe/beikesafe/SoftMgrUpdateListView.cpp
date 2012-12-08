#include "stdafx.h"
#include "beikesafe.h"
#include "SoftMgrUpdateListView.h"

#define SOFTMGR_LIST_ITEM_HEIGHT 55
#define SOFTMGR_LIST_RIGHT_WIDTH 310

#define	COLOR_SOFTMGR_PLUG		RGB(200,0,0)
#define	COLOR_SOFTMGR_NOPLUG	RGB(10,150,0)
#define COLOR_MOUSE_ON_NAME		RGB(0,100,200)
#define	COLOR_SOFTMGR_NOUPDATE	RGB(  0, 78,152)
#define COLOR_MOUSE_ON_MARK		RGB( 47, 99,165)
#define COLOR_GRAY				RGB(128,128,128)

#define STR_BTN_TIP_CANCEL	_T("取消")
#define STR_BTN_TIP_CONTINUE	_T("继续下载")
#define STR_BTN_TIP_PAUSE	_T("暂停")
#define STR_TIP_DETAIL	_T("点击查看该软件简介及截图")

void CSoftMgrUpdateListView::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
}

HRESULT	CSoftMgrUpdateListView::OnMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
{
	m_bMouseOnPause = FALSE;
	m_bMouseOn = FALSE;
	m_bMouseOnCancel = FALSE;
	m_bMouseOnName = FALSE;
	Invalidate(FALSE);

	return S_OK;
}

HRESULT	CSoftMgrUpdateListView::OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
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

LRESULT CSoftMgrUpdateListView::OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_wndToolTip.IsWindow())
	{
		MSG msg = { m_hWnd, uMsg, wParam, lParam };
		m_wndToolTip.RelayEvent(&msg);
	}

	SetMsgHandled(FALSE);
	return 0;
}

void CSoftMgrUpdateListView::SetShowDlgState()
{
	//if (GetItemCount()==0)
	//{
	//	CString	strFormat;
	//	strFormat = _T("正在加载，请稍候...");
	//	m_dlgMsg.SetItemVisible(10,FALSE);
	//	m_dlgMsg.SetItemVisible(1,FALSE);
	//	m_dlgMsg.SetItemVisible(2,TRUE);
	//	m_dlgMsg.SetItemText(3,strFormat);
	//	m_dlgMsg.ShowWindow(SW_SHOW);
	//}
	//else
	//	m_dlgMsg.ShowWindow(SW_HIDE);
}

void CSoftMgrUpdateListView::SetLoadShow(BOOL bShow)
{
	//if (bShow)
	//{
	//	m_dlgMsg.SetItemVisible(1,TRUE);
	//	m_dlgMsg.ShowWindow(SW_SHOW);
	//}
	//else
	//{
	//	m_dlgMsg.SetItemVisible(1,FALSE);
	//	m_dlgMsg.ShowWindow(SW_HIDE);
	//}
}

void CSoftMgrUpdateListView::CreateTipCtrl()
{
	//if ( !m_dlgMsg.Load(IDR_BK_LISTTIP_MGR_DLG) )
	//	return;
	//m_dlgMsg.Create(m_hWnd);
	//m_dlgMsg.ShowWindow(SW_HIDE);

	SetBkColor(COL_DEFAULT_WHITE_BG);

	m_wndToolTip.Create(m_hWnd);

	CToolInfo ti(0, m_hWnd);
	m_wndToolTip.AddTool(ti);
	m_wndToolTip.Activate(TRUE);
	m_wndToolTip.SetMaxTipWidth(300);
	m_wndToolTip.SetDelayTime(TTDT_AUTOPOP,1000);
}

void CSoftMgrUpdateListView::InitNormal()
{
	CRect rcTmp;
	GetClientRect(&rcTmp);

	m_rcIgnore.right = rcTmp.Width() - 5;
	m_rcIgnore.left = m_rcIgnore.right - 62;
	m_rcIgnore.top = 15;
	m_rcIgnore.bottom = 37;

	m_rcStateBtn.right = rcTmp.Width() - 75;
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

	//m_rcStar.right = m_rcStateBtn.left - 43;
	//m_rcStar.left = m_rcStar.right - 75;
	//m_rcStar.top = 9;
	//m_rcStar.bottom = 23;
	m_rcStar = CRect(0, 0, 0, 0);
	m_rcMark = CRect(0, 0, 0, 0);	
	//m_rcMark.right = m_rcStateBtn.left - 40;
	//m_rcMark.left = m_rcMark.right - 75;
	//m_rcMark.top = 33;
	//m_rcMark.bottom = 45;

	m_rcSize.right = m_rcStateBtn.left - 43 - 75 - 30 + 100;
	m_rcSize.left = m_rcSize.right - 57;
	m_rcSize.top = m_rcStateBtn.top;
	m_rcSize.bottom = m_rcStateBtn.bottom;

	m_rcLocalVer.right = rcTmp.Width() - 75 - 65 - 43 - 75 - 30 - 57 - 14 + 80;
	m_rcLocalVer.left = m_rcLocalVer.right - 150;
	m_rcLocalVer.top = 9;
	m_rcLocalVer.bottom = 23;

	m_rcNewVer.right = rcTmp.Width() - 75 - 65 - 43 - 75 - 30 - 57 - 14 + 80;
	m_rcNewVer.left = m_rcNewVer.right - 150;
	m_rcNewVer.top = 33;
	m_rcNewVer.bottom = 45;

	m_rcState = CRect(0, 0, 0, 0);
	m_rcCancel = CRect(0, 0, 0, 0);	
	m_rcContinue = CRect(0, 0, 0, 0);
	m_rcProgress = CRect(0, 0, 0, 0);
	m_rcValue = CRect(0, 0, 0, 0);
	m_rcWaitInstall = CRect(0,0,0,0);
	m_rcWaitDownload = CRect(0,0,0,0);
}

void CSoftMgrUpdateListView::InitDownLoad()
{
	CRect rcTmp;
	GetClientRect(&rcTmp);

	m_rcStar = CRect(0, 0, 0, 0);
	m_rcMark = CRect(0, 0, 0, 0);
	m_rcStateBtn = CRect(0, 0, 0, 0);
	m_rcIgnore = CRect(0, 0, 0, 0);
	m_rcCharge = CRect(0, 0, 0, 0);
	m_rcPlug = CRect(0, 0, 0, 0);
	m_rcSize = CRect(0, 0, 0, 0);
	m_rcWaitInstall = CRect(0,0,0,0);
	m_rcWaitDownload = CRect(0,0,0,0);

	m_rcLocalVer.right = rcTmp.Width() - 75 - 65 - 43 - 75 - 30 - 57 - 14 + 80;
	m_rcLocalVer.left = m_rcLocalVer.right - 150;
	m_rcLocalVer.top = 9;
	m_rcLocalVer.bottom = 23;

	m_rcNewVer.right = rcTmp.Width() - 75 - 65 - 43 - 75 - 30 - 57 - 14 + 80;
	m_rcNewVer.left = m_rcNewVer.right - 150;
	m_rcNewVer.top = 33;
	m_rcNewVer.bottom = 45;

	m_rcCancel.right = rcTmp.Width() - 10;
	m_rcCancel.left = m_rcCancel.right - 16;
	m_rcCancel.top = 19;
	m_rcCancel.bottom = 35;

	m_rcContinue.right = m_rcCancel.left - 2;
	m_rcContinue.left = m_rcContinue.right - 16;
	m_rcContinue.top = m_rcCancel.top;
	m_rcContinue.bottom = m_rcCancel.bottom;

	m_rcState.right = m_rcContinue.left - 25;
	m_rcState.left = m_rcState.right - 60;
	m_rcState.top = 21;
	m_rcState.bottom = 33;

	m_rcValue.right = m_rcState.left - 5;
	m_rcValue.left = m_rcValue.right - 35;
	m_rcValue.top = m_rcState.top;
	m_rcValue.bottom = m_rcState.bottom;

	m_rcProgress.right = m_rcValue.left - 10;
	m_rcProgress.left = m_rcProgress.right - 102;
	m_rcProgress.top = 23;
	m_rcProgress.bottom = m_rcProgress.top + 8;
}

LRESULT	CSoftMgrUpdateListView::OnCtlColor(HDC hDc, HWND hWnd)
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

CSoftListItemData*	CSoftMgrUpdateListView::GetItemDataEx(DWORD dwIndex)
{
//	ATLASSERT(dwIndex >= 0 && dwIndex < (DWORD)m_arrData.GetSize());
	if (dwIndex < 0 || dwIndex >= (DWORD)m_arrData.GetSize())
	{
		return NULL;
	}

	for ( int i=0; i<m_arrData.GetSize(); i++)
	{
		CSoftListItemData* pTemp = m_arrData[i];
		//if (m_arrData[i] && m_arrData[i]->m_dwID == dwIndex)
		if (m_arrData[i] && m_arrData[i]->m_dwIDUpdate == dwIndex)
			return m_arrData[i];
	}
	return NULL;
}

int CSoftMgrUpdateListView::SetItemDataEx(DWORD dwIndex, DWORD_PTR dwItemData)
{
	CSoftListItemData*	pItemData=(CSoftListItemData*)dwItemData;
	ATLASSERT(pItemData!=NULL);

	for (int i= 0;i<m_arrData.GetSize();i++)
	{
		//if (m_arrData[i]->m_dwID==pItemData->m_dwID)
		if (m_arrData[i]->m_dwIDUpdate==pItemData->m_dwIDUpdate)
		{
			ATLASSERT(FALSE);
			return -1;
		}
	}

	m_arrData.Add(pItemData);
	return SetItemData(dwIndex,(DWORD_PTR)dwItemData); 
}

BOOL CSoftMgrUpdateListView::GetItemRect(int nItem, LPRECT lpRect, UINT nCode)
{
	return  CListViewCtrl::GetItemRect(nItem, lpRect, nCode);
}

HRESULT	CSoftMgrUpdateListView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
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
			SetItemState(htiSel.iItem, 0,  LVIS_SELECTED|LVIS_FOCUSED);
		}
	}
	if (nIndex != -1)
	{
		SetItemState(nIndex,  LVIS_SELECTED|LVIS_FOCUSED,  LVIS_SELECTED|LVIS_FOCUSED);
		::PostMessage(GetParent(), WM_USER_CLICK_LIST_ITEM, (WPARAM)m_hWnd, nIndex);
	}

	if ( /*!bOutSide &&*/ nIndex >= 0 && nIndex < m_arrData.GetSize() && GetItemRect(nIndex,&rcItem,LVIR_BOUNDS) )
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
					m_rcIgnore = CRect(0,0,0,0);
					m_rcWaitInstall = CRect(0,0,0,0);
				}
				else if (pData->m_bInstalling)
				{
					m_rcInstall.right = rcTmp.Width() - 15;
					m_rcInstall.left = m_rcInstall.right - 57;
					m_rcInstall.top = 19;
					m_rcInstall.bottom = 39;
					m_rcStateBtn = CRect(0,0,0,0);
					m_rcIgnore = CRect(0,0,0,0);
				}
				else
				{
					m_rcStateBtn.right = rcTmp.Width() - 75;
					m_rcStateBtn.left = m_rcStateBtn.right - 65;
					m_rcStateBtn.top = 15;
					m_rcStateBtn.bottom = 40;

					m_rcIgnore.right = rcTmp.Width() - 5;
					m_rcIgnore.left = m_rcIgnore.right - 62;
					m_rcIgnore.top = 15;
					m_rcIgnore.bottom = 37;

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
					m_rcIgnore = CRect(0,0,0,0);
				}
				else
				{
					m_rcStateBtn.right = rcTmp.Width() - 75;
					m_rcStateBtn.left = m_rcStateBtn.right - 65;
					m_rcStateBtn.top = 15;
					m_rcStateBtn.bottom = 40;

					//m_rcStar.right = m_rcStateBtn.left - 43;
					//m_rcStar.left = m_rcStar.right - 75;
					//m_rcStar.top = 9;
					//m_rcStar.bottom = 23;
					m_rcStar = CRect(0, 0, 0, 0);
					m_rcMark = CRect(0, 0, 0, 0);	
					//m_rcMark.right = m_rcStateBtn.left - 40;
					//m_rcMark.left = m_rcMark.right - 75;
					//m_rcMark.top = 33;
					//m_rcMark.bottom = 45;

					m_rcIgnore.right = rcTmp.Width() - 5;
					m_rcIgnore.left = m_rcIgnore.right - 62;
					m_rcIgnore.top = 15;
					m_rcIgnore.bottom = 37;

					m_rcInstall = CRect(0,0,0,0);
				}

				m_rcState = CRect(0, 0, 0, 0);
				m_rcCancel = CRect(0, 0, 0, 0);	
				m_rcContinue = CRect(0, 0, 0, 0);
			}

			pts.x -= rcItem.left;
			pts.y -= rcItem.top;

			if ( m_rcCheck.PtInRect(pts) ||
				m_rcNewInfo.PtInRect(pts) ||
				pData->m_rcName.PtInRect(pts) ||
				m_rcCancel.PtInRect(pts) ||
				m_rcContinue.PtInRect(pts) ||
				m_rcIcon.PtInRect(pts) ||
				m_rcInstall.PtInRect(pts) ||
				pData->m_rcFreeback.PtInRect(pts) ||
				pData->m_rcTautology.PtInRect(pts) || 
				m_rcMark.PtInRect(pts) || 
				m_rcStar.PtInRect(pts) ||
				m_rcIgnore.PtInRect(pts))
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

HRESULT	CSoftMgrUpdateListView::OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
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
					m_rcIgnore = CRect(0,0,0,0);
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
					m_rcIgnore = CRect(0,0,0,0);
				}
				else if (pData->m_bInstalling)
				{
					m_rcInstall.right = rcTmp.Width() - 15;
					m_rcInstall.left = m_rcInstall.right - 57;
					m_rcInstall.top = 19;
					m_rcInstall.bottom = 39;
					m_rcWaitInstall = CRect(0,0,0,0);
					m_rcStateBtn = CRect(0,0,0,0);
					m_rcIgnore = CRect(0,0,0,0);
				}
				else
				{
					m_rcStateBtn.right = rcTmp.Width() - 75;
					m_rcStateBtn.left = m_rcStateBtn.right - 65;
					m_rcStateBtn.top = 15;
					m_rcStateBtn.bottom = 40;

					m_rcIgnore.right = rcTmp.Width() - 5;
					m_rcIgnore.left = m_rcIgnore.right - 62;
					m_rcIgnore.top = 15;
					m_rcIgnore.bottom = 37;

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
					m_rcIgnore = CRect(0,0,0,0);
				}
				else if (pData->m_bInstalling)
				{
					m_rcInstall.right = rcTmp.Width() - 15;
					m_rcInstall.left = m_rcInstall.right - 57;
					m_rcInstall.top = 19;
					m_rcInstall.bottom = 39;
					m_rcWaitInstall = CRect(0,0,0,0);
					m_rcStateBtn = CRect(0,0,0,0);
					m_rcIgnore = CRect(0,0,0,0);
				}
				else
				{
					m_rcStateBtn.right = rcTmp.Width() - 75;
					m_rcStateBtn.left = m_rcStateBtn.right - 65;
					m_rcStateBtn.top = 15;
					m_rcStateBtn.bottom = 40;

					//m_rcStar.right = m_rcStateBtn.left - 43;
					//m_rcStar.left = m_rcStar.right - 75;
					//m_rcStar.top = 9;
					//m_rcStar.bottom = 23;
					m_rcStar = CRect(0, 0, 0, 0);
					m_rcMark = CRect(0, 0, 0, 0);	
					//m_rcMark.right = m_rcStateBtn.left - 40;
					//m_rcMark.left = m_rcMark.right - 75;
					//m_rcMark.top = 33;
					//m_rcMark.bottom = 45;

					m_rcIgnore.right = rcTmp.Width() - 5;
					m_rcIgnore.left = m_rcIgnore.right - 62;
					m_rcIgnore.top = 15;
					m_rcIgnore.bottom = 37;

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
				InvalidateRect(rcTmp,FALSE);
				::PostMessage(GetParent(), WM_USER_CHECK_LIST_ITEM, (WPARAM)m_hWnd, pData->m_bCheck);
			}
			else if ( pData->m_rcName.PtInRect(pts) )
			{
				m_linkerCB->OnUpdateViewDetail(nIndex, m_hWnd);
			}
			else if ( m_rcNewInfo.PtInRect(pts) )
			{
				m_linkerCB->OnUpdateViewNew(nIndex, m_hWnd);
			}
			else if ( m_rcIcon.PtInRect(pts))
			{
				m_linkerCB->OnUpdateViewDetail(nIndex, m_hWnd);
			}
			else if ( m_rcMark.PtInRect(pts))
			{
				m_linkerCB->OnUpdateSoftMark(nIndex, m_hWnd);
			}
			else if ( m_rcStar.PtInRect(pts))
			{
				m_linkerCB->OnUpdateSoftMark(nIndex, m_hWnd);
			}
			else if(m_rcStateBtn.PtInRect(pts))
			{
				if (m_bMouseDown)
				{
					m_linkerCB->OnUpdateDownLoad(nIndex, m_hWnd);
					m_linkerCB->OnBtnClick( nIndex, 0 );
				}
			}
			else if(m_rcCancel.PtInRect(pts))
			{
				if (m_bMouseDown && !pData->m_bDownLoad)
				{
					pData->m_bDownloading = FALSE;
					m_linkerCB->OnUpdateCancel(nIndex, m_hWnd);
				}
			}
			else if (m_rcContinue.PtInRect(pts))
			{
				if (m_bMouseDown  && !pData->m_bDownLoad)
				{
					pData->m_bDownloading = TRUE;
					if(pData->m_bContinue)
					{
						m_linkerCB->OnUpdatePause(nIndex, m_hWnd);
					}
					else
					{
						m_linkerCB->OnUpdateContinue(nIndex, m_hWnd);
					}
				}
			}
			else if (pData->m_rcTautology.PtInRect(pts))
			{
				m_linkerCB->OnUpdateTautilogyFromList(nIndex, m_hWnd);
			}
			else if (pData->m_rcFreeback.PtInRect(pts))
			{
				m_linkerCB->OnUpdateFreebackFromList(nIndex, m_hWnd);
			}
			else if (m_rcIgnore.PtInRect(pts))
			{
				m_linkerCB->OnUpdateIgnore(nIndex, m_hWnd);
			}
		}
	}

	CRect rcTmp;
	GetClientRect(&rcTmp);
	InvalidateRect(&rcTmp);

	m_bMouseDown = FALSE;
	return S_OK;		
}

void CSoftMgrUpdateListView::SetClickLinkCallback( ISoftMgrUpdateCallback* opCB )
{
	m_linkerCB = opCB;
}

ISoftMgrUpdateCallback* CSoftMgrUpdateListView::GetClickLinkCallback()
{
	return m_linkerCB;
}

VOID CSoftMgrUpdateListView::SetPaintItem(BOOL bDrawItem)
{
	m_bDrawItem = bDrawItem;
}

void CSoftMgrUpdateListView::MeasureItem(LPMEASUREITEMSTRUCT lpMes)
{
	lpMes->itemHeight = SOFTMGR_LIST_ITEM_HEIGHT;
}

HRESULT	CSoftMgrUpdateListView::OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
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

//	this->SetFocus();

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
					m_rcIgnore = CRect(0,0,0,0);
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
					m_rcIgnore = CRect(0,0,0,0);
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
					m_rcIgnore = CRect(0,0,0,0);
				}
				else
				{
					m_rcStateBtn.right = rcTmp.Width() - 75;
					m_rcStateBtn.left = m_rcStateBtn.right - 65;
					m_rcStateBtn.top = 15;
					m_rcStateBtn.bottom = 40;

					m_rcIgnore.right = rcTmp.Width() - 5;
					m_rcIgnore.left = m_rcIgnore.right - 62;
					m_rcIgnore.top = 15;
					m_rcIgnore.bottom = 37;

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
					m_rcIgnore = CRect(0,0,0,0);
				}
				else if (pData->m_bWaitInstall)
				{
					m_rcStar = CRect(0,0,0,0);
					m_rcMark = CRect(0,0,0,0);
					m_rcStateBtn = CRect(0,0,0,0);
					m_rcIgnore = CRect(0,0,0,0);
					m_rcInstall = CRect(0,0,0,0);
				}
				else
				{
					m_rcStateBtn.right = rcTmp.Width() - 75;
					m_rcStateBtn.left = m_rcStateBtn.right - 65;
					m_rcStateBtn.top = 15;
					m_rcStateBtn.bottom = 40;

					//m_rcStar.right = m_rcStateBtn.left - 43;
					//m_rcStar.left = m_rcStar.right - 75;
					//m_rcStar.top = 9;
					//m_rcStar.bottom = 23;
					m_rcStar = CRect(0, 0, 0, 0);
					m_rcMark = CRect(0, 0, 0, 0);	
					//m_rcMark.right = m_rcStateBtn.left - 40;
					//m_rcMark.left = m_rcMark.right - 75;
					//m_rcMark.top = 33;
					//m_rcMark.bottom = 45;

					m_rcIgnore.right = rcTmp.Width() - 5;
					m_rcIgnore.left = m_rcIgnore.right - 62;
					m_rcIgnore.top = 15;
					m_rcIgnore.bottom = 37;

					m_rcInstall = CRect(0,0,0,0);
				}

				m_rcState = CRect(0, 0, 0, 0);
				m_rcCancel = CRect(0, 0, 0, 0);	
				m_rcContinue = CRect(0, 0, 0, 0);
			}

			pts.x -= rcItem.left;
			pts.y -= rcItem.top;

			CRect rcName = pData->m_rcName;
			if ( m_rcCheck.PtInRect(pts))
			{
				::SetCursor(::LoadCursor(NULL,IDC_HAND));
			}
			else if ( m_rcNewInfo.PtInRect(pts))
			{
				::SetCursor(::LoadCursor(NULL,IDC_HAND));
			}
			else if (pData->m_rcName.PtInRect(pts) )
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
			else if ( m_rcIgnore.PtInRect(pts))
			{
				::SetCursor(::LoadCursor(NULL,IDC_HAND));
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
	}

	TRACKMOUSEEVENT   tme;   
	tme.cbSize   =   sizeof(tme);   
	tme.hwndTrack   =   m_hWnd;   
	tme.dwFlags   =   TME_LEAVE;   
	 _TrackMouseEvent(&tme);  

	return S_OK;
}

void CSoftMgrUpdateListView::_DrawItem(HDC hDC, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rcWnd;
	INT	  nColumWidth = 0;
	CSoftListItemData*	pListData = GetItemDataEx(lpDrawItemStruct->itemID);

	GetWindowRect(&rcWnd);

	nColumWidth = rcWnd.Width();
	if(nColumWidth <= 0)
		nColumWidth = lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left;

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

		hFntTmp = dcx.SelectFont(m_fntNormal);
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
			Gdiplus::Color clBk = 0;
			//if (pListData->m_bIgnore)
			//	clBk = Gdiplus::Color(255,255,255,220);
			//else
				clBk = Gdiplus::Color(255,255,255,255);
			Gdiplus::SolidBrush mySolidBrush(clBk);
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
		CRect rcCheck = m_rcCheck;
		rcCheck.OffsetRect( lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		HBITMAP hCheckBitmp = BkBmpPool::GetBitmap(IDB_BITMAP_LISTCTRL_CHECK);
		BITMAP bmp = {0};
		GetObject( hCheckBitmp, sizeof(BITMAP), &bmp); 
		CDC dcDraw;
		dcDraw.CreateCompatibleDC(dcx.m_hDC);
		SelectObject(dcDraw.m_hDC, hCheckBitmp);
		if (pListData->m_bCheck)
		{
			dcx.BitBlt(rcCheck.left, rcCheck.top, bmp.bmHeight, bmp.bmHeight, dcDraw.m_hDC, 0, 0, SRCCOPY);
		}
		else
		{
			dcx.BitBlt(rcCheck.left, rcCheck.top, bmp.bmHeight, bmp.bmHeight, dcDraw.m_hDC, 13, 0, SRCCOPY);
		}

		//画图标
		CRect rcIcon = m_rcIcon;
		rcIcon.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		if (pListData->m_pImage != NULL)
		{
			grap.DrawImage(pListData->m_pImage, rcIcon.left, rcIcon.top, rcIcon.Width(), rcIcon.Height());
		}
		if (!pListData->m_bIcon)
		{
			m_linkerCB->OnUpdateDownLoadIcon(pListData->m_strSoftID);
		}

		//画软件名称
		CRect rcItem;
		GetWindowRect(&rcItem);
		CRect rcName = m_rcName;
		CRect rcProb;
		hFntTmp = dcx.SelectFont(m_fntBold);
		if (m_bMouseOnName && (m_pRefresh == (DWORD)pListData))
			dcx.SetTextColor(COLOR_MOUSE_ON_NAME);
		else
			dcx.SetTextColor(RGB(0,0,0));
		dcx.DrawText(pListData->m_strName, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);

		int nLablesWidth = 0;
		Gdiplus::Image *pImage = NULL;

		if ((pListData->m_attri&SA_Major) == SA_Major)
		{
			pImage = BkPngPool::Get(IDP_SOFTMGR_IMPORTANT_SOFT);
			nLablesWidth += pImage->GetWidth();
		}
		if ((pListData->m_attri&SA_Green) == SA_Green)
		{
			if (nLablesWidth != 0)
				nLablesWidth += 2;
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
		rcName.right = rcItem.Width() - 415 - nLablesWidth;

		pListData->m_rcName = rcName;
		if (rcProb.Width() < rcItem.Width() - 415 - nLablesWidth - 70)
		{
			pListData->m_rcName.right = rcName.left + rcProb.Width();
			rcName.right = rcName.left + rcProb.Width();
		}
		rcName.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);

		dcx.DrawText(pListData->m_strName, -1, &rcName, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
		dcx.SetTextColor(clrOld);
		dcx.SelectFont(hFntTmp);

		clrOld = dcx.SetTextColor(RGB(122,122,122));
		int nPosY = rcName.top - 2;
		int nPosX = rcName.right;
		if ((pListData->m_attri&SA_Major) == SA_Major)
		{
			nPosX += 2;
			pImage = BkPngPool::Get(IDP_SOFTMGR_IMPORTANT_SOFT);
			grap.DrawImage(pImage, nPosX, nPosY, pImage->GetWidth(), pImage->GetHeight());
			nPosX += pImage->GetWidth();
		}
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

		//发布时间
		if (pListData->m_strPublished.IsEmpty() == FALSE)
		{
			CString strIntro;
			strIntro = pListData->m_strPublished;
			CRect rcIntro = m_rcDescription;
			dcx.DrawText(strIntro, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcIntro.right = rcItem.Width() - SOFTMGR_LIST_RIGHT_WIDTH-15 - 53;
			if (rcProb.Width() < rcItem.Width() - SOFTMGR_LIST_RIGHT_WIDTH - 81 - 53)
			{
				rcIntro.right = rcIntro.left + rcProb.Width();
			}
			rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(strIntro, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
		}

		//新版功能
		//dcx.SelectFont(m_fntUnderLine);
		{
			COLORREF clOld = dcx.GetTextColor();
			dcx.SetTextColor(RGB( 47, 99,165));
			CRect rcNew = m_rcNewInfo;
			rcNew.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(L"新版功能", -1, &rcNew, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
			dcx.SetTextColor(clOld);
		}

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
				}
				else
				{
					m_rcStateBtn.right = rcTmp.Width() - 75;
					m_rcStateBtn.left = m_rcStateBtn.right - 65;
					m_rcStateBtn.top = 15;
					m_rcStateBtn.bottom = 40;

					//m_rcStar.right = m_rcStateBtn.left - 43;
					//m_rcStar.left = m_rcStar.right - 75;
					//m_rcStar.top = 9;
					//m_rcStar.bottom = 23;
					m_rcStar = CRect(0, 0, 0, 0);
					m_rcMark = CRect(0, 0, 0, 0);	
					//m_rcMark.right = m_rcStateBtn.left - 40;
					//m_rcMark.left = m_rcMark.right - 75;
					//m_rcMark.top = 33;
					//m_rcMark.bottom = 45;

					m_rcSize.right = m_rcStateBtn.left - 43 - 75 - 30 + 100;
					m_rcSize.left = m_rcSize.right - 57;
					m_rcSize.top = m_rcStateBtn.top;
					m_rcSize.bottom = m_rcStateBtn.bottom;

					m_rcIgnore.right = rcTmp.Width() - 5;
					m_rcIgnore.left = m_rcIgnore.right - 62;
					m_rcIgnore.top = 15;
					m_rcIgnore.bottom = 37;

					m_rcProgress = CRect(0, 0, 0, 0);
					m_rcWaitInstall = CRect(0,0,0,0);
					m_rcInstall = CRect(0,0,0,0);
				}

				m_rcState = CRect(0, 0, 0, 0);
				m_rcCancel = CRect(0, 0, 0, 0);	
				m_rcContinue = CRect(0, 0, 0, 0);

				//不再提醒
				{
					COLORREF clOld = dcx.GetTextColor();
					CString strIgnore;
					if (pListData->m_bIgnore)
					{
						dcx.SetTextColor(RGB(  3,128,  2));
						strIgnore = L"恢复提醒";
					}
					else
					{
						dcx.SetTextColor(RGB( 47, 99,165));
						strIgnore = L"不再提醒";
					}
					CRect rcIgnore = m_rcIgnore;
					rcIgnore.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
					dcx.DrawText(strIgnore, -1, &rcIgnore, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
					dcx.SetTextColor(clOld);
				}

				CRect rcTmpCalc;
				if (!m_rcWaitInstall.IsRectEmpty())
				{
					//等待安装
					CRect rcWaitInstall = m_rcWaitInstall;
					dcx.DrawText(_T("等待安装"), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
					rcWaitInstall.left = rcWaitInstall.right - rcTmpCalc.Width();
					rcWaitInstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
					dcx.DrawText(_T("等待安装"), -1, &rcWaitInstall, DT_VCENTER | DT_SINGLELINE);
				}

				if (!m_rcInstall.IsRectEmpty())
				{
					//正在安装
					CRect rcInstall = m_rcInstall;
					dcx.DrawText(_T("正在安装"), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
					rcInstall.left = rcInstall.right - rcTmpCalc.Width();
					rcInstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
					dcx.DrawText(_T("正在安装"), -1, &rcInstall, DT_VCENTER | DT_SINGLELINE);
				}

				//最右侧按钮
				CRect rcStateBtn = m_rcStateBtn;
				rcStateBtn.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				if (pListData->m_bSetup)
				{
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_UPDATE);//BkPngPool::Get(IDP_LISTCTRL_BTN_DOWN);
				}
				else
				{
					pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_UPDATE);//BkPngPool::Get(IDP_LISTCTRL_BTN_SET);
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

				//画软件大小
				CRect rcSize = m_rcSize;
				rcSize.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(pListData->m_strSize, -1, &rcSize, DT_VCENTER | DT_LEFT/*DT_RIGHT*/ | DT_SINGLELINE);

				////画评分
				//if (m_rcStar.IsRectEmpty() == FALSE)
				//{
				//	int nStep = m_rcStar.Width()/5;
				//	CRect rcStar = m_rcStar;
				//	rcStar.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				//	Gdiplus::Image *pImage = NULL;
				//	for (int i = 0; i < 5; i++)
				//	{
				//		if ((i+1)*2 <= pListData->m_fMark)
				//			pImage = BkPngPool::Get(IDP_SOFTMGR_STAR);
				//		else if ((i+1)*2 - 1 <= pListData->m_fMark)
				//			pImage = BkPngPool::Get(IDP_SOFTMGR_STAR_HALF);
				//		else
				//			pImage = BkPngPool::Get(IDP_SOFTMGR_STAR_OFF);

				//		if (pImage)
				//		{
				//			Gdiplus::RectF rcDest(rcStar.left + i*nStep, rcStar.top, pImage->GetWidth(), pImage->GetHeight());
				//			grap.DrawImage(pImage, rcDest, 0, 0,
				//				pImage->GetWidth(), pImage->GetHeight(), Gdiplus::UnitPixel);
				//		}
				//	}

				//	dcx.SetTextColor(RGB( 47, 99,165));
				//	CRect rcMark = m_rcMark;
				//	rcMark.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				//	CString strMark;
				//	strMark.Format(L"%.1f分 投票", pListData->m_fMark);
				//	dcx.DrawText(strMark, -1, &rcMark, DT_VCENTER | DT_LEFT/*DT_RIGHT*/ | DT_SINGLELINE);
				//}
			}

			//画版本对比
			CRect rcLocalVer = m_rcLocalVer;
			rcLocalVer.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			CString strLocalVer;
			strLocalVer.Format(L"当前版本:%s", pListData->m_strVersion);
			dcx.DrawText(strLocalVer, -1, &rcLocalVer, DT_TOP | DT_LEFT | DT_SINGLELINE|DT_END_ELLIPSIS);

			if (pListData->m_strNewVersion.IsEmpty() == FALSE)
			{
				CRect rcNewVer = m_rcNewVer;
				rcNewVer.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				CString strNewVer;
				strNewVer.Format(L"可更新至:%s", pListData->m_strNewVersion);
				dcx.DrawText(strNewVer, -1, &rcNewVer, DT_TOP | DT_LEFT | DT_SINGLELINE|DT_END_ELLIPSIS);
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
				dcx.DrawText(_T("下载连接失败"), -1, &rcCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcLink.right = rcLink.left + rcCalc.Width() + 2;
				dcx.DrawText(_T("下载连接失败"), -1, &rcLink, DT_VCENTER | DT_SINGLELINE);
			}
			else
			{
				dcx.DrawText(_T("正在连接下载服务器"), -1, &rcCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcLink.right = rcLink.left + rcCalc.Width() + 2;
				dcx.DrawText(_T("正在连接下载服务器"), -1, &rcLink, DT_VCENTER | DT_SINGLELINE);
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
				dcx.DrawText(_T("暂停中"), -1, rcState, DT_VCENTER | DT_SINGLELINE);
			}
			else 
			{
				dcx.DrawText(pListData->m_strSpeed, -1, rcState, DT_VCENTER | DT_SINGLELINE);
			}

			//画反馈文字
			hFntTmp = dcx.SelectFont(m_fntNormal);
			dcx.SetTextColor(COLOR_MOUSE_ON_NAME);
			CRect rcTmp;
			GetClientRect(&rcTmp);
			CRect rcFreeback = m_rcFreeback;
			rcFreeback.right = rcItem.Width() - 35;
			dcx.DrawText(_T("反馈"), -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcFreeback.left = rcFreeback.right - rcProb.Width();
			pListData->m_rcFreeback = rcFreeback;
			rcFreeback.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(_T("反馈"), -1, &rcFreeback, DT_VCENTER | DT_SINGLELINE);
			//画重试文字
			CRect rcTautology = m_rcTautology;
			rcTautology.right = rcFreeback.left - 5;
			dcx.DrawText(_T("重试"), -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rcTautology.left = rcTautology.right - rcProb.Width();
			pListData->m_rcTautology = rcTautology;
			rcTautology.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(_T("重试"), -1, &rcTautology, DT_VCENTER | DT_SINGLELINE);
			dcx.SetTextColor(clrOld);
			dcx.SelectFont(hFntTmp);
		}
		else// 常规状态
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
				m_rcWaitInstall = CRect(0,0,0,0);
				m_rcIgnore = CRect(0,0,0,0);
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
				m_rcIgnore = CRect(0,0,0,0);
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
				m_rcIgnore = CRect(0,0,0,0);
			}
			else
			{
				m_rcStateBtn.right = rcTmp.Width() - 75;
				m_rcStateBtn.left = m_rcStateBtn.right - 65;
				m_rcStateBtn.top = 15;
				m_rcStateBtn.bottom = 40;
				m_rcWaitInstall = CRect(0,0,0,0);
				m_rcWaitDownload = CRect(0,0,0,0);
				m_rcInstall = CRect(0,0,0,0);
			}

			//画版本对比
			CRect rcLocalVer = m_rcLocalVer;
			rcLocalVer.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			CString strLocalVer;
			strLocalVer.Format(L"当前版本:%s", pListData->m_strVersion);
			dcx.DrawText(strLocalVer, -1, &rcLocalVer, DT_TOP | DT_LEFT | DT_SINGLELINE|DT_END_ELLIPSIS);

			if (pListData->m_strNewVersion.IsEmpty() == FALSE)
			{
				CRect rcNewVer = m_rcNewVer;
				rcNewVer.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				CString strNewVer;
				strNewVer.Format(L"可更新至:%s", pListData->m_strNewVersion);
				dcx.DrawText(strNewVer, -1, &rcNewVer, DT_TOP | DT_LEFT | DT_SINGLELINE|DT_END_ELLIPSIS);
			}

			//画软件大小
			CRect rcSize = m_rcSize;
			rcSize.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(pListData->m_strSize, -1, &rcSize, DT_VCENTER | DT_LEFT/*DT_RIGHT*/ | DT_SINGLELINE);

			////画评分
			//int nStep = m_rcStar.Width()/5;
			//CRect rcStar = m_rcStar;
			//rcStar.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			//Gdiplus::Image *pImage = NULL;
			//for (int i = 0; i < 5; i++)
			//{
			//	if ((i+1)*2 <= pListData->m_fMark)
			//		pImage = BkPngPool::Get(IDP_SOFTMGR_STAR);
			//	else if ((i+1)*2 - 1 <= pListData->m_fMark)
			//		pImage = BkPngPool::Get(IDP_SOFTMGR_STAR_HALF);
			//	else
			//		pImage = BkPngPool::Get(IDP_SOFTMGR_STAR_OFF);
			//	
			//	if (pImage)
			//	{
			//		Gdiplus::RectF rcDest(rcStar.left + i*nStep, rcStar.top, pImage->GetWidth(), pImage->GetHeight());
			//		grap.DrawImage(pImage, rcDest, 0, 0,
			//			pImage->GetWidth(), pImage->GetHeight(), Gdiplus::UnitPixel);
			//	}
			//}

			//dcx.SetTextColor(RGB( 47, 99,165));
			//CRect rcMark = m_rcMark;
			//rcMark.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			//CString strMark;
			//strMark.Format(L"%.1f分 投票", pListData->m_fMark);
			//dcx.DrawText(strMark, -1, &rcMark, DT_VCENTER | DT_LEFT/*DT_RIGHT*/ | DT_SINGLELINE);

			//不再提醒
			CString strIgnore;
			if (pListData->m_bIgnore)
			{
				dcx.SetTextColor(RGB(  3,128,  2));
				strIgnore = L"恢复提醒";
			}
			else
			{
				dcx.SetTextColor(RGB( 47, 99,165));
				strIgnore = L"不再提醒";
			}
			CRect rcIgnore = m_rcIgnore;
			rcIgnore.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
			dcx.DrawText(strIgnore, -1, &rcIgnore, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			CRect rcTmpCalc;
			if (!m_rcWaitDownload.IsRectEmpty())
			{
				//等待下载
				CRect rcWaitDowload = m_rcWaitDownload;
				dcx.DrawText(_T("等待下载"), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcWaitDowload.left = rcWaitDowload.right - rcTmpCalc.Width();
				rcWaitDowload.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(_T("等待下载"), -1, &rcWaitDowload, DT_VCENTER | DT_SINGLELINE);
			}

			if (!m_rcWaitInstall.IsRectEmpty())
			{
				//等待安装
				CRect rcWaitInstall = m_rcWaitInstall;
				dcx.DrawText(_T("等待安装"), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcWaitInstall.left = rcWaitInstall.right - rcTmpCalc.Width();
				rcWaitInstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(_T("等待安装"), -1, &rcWaitInstall, DT_VCENTER | DT_SINGLELINE);
			}

			if (!m_rcInstall.IsRectEmpty())
			{
				//正在安装
				CRect rcInstall = m_rcInstall;
				dcx.DrawText(_T("正在安装"), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
				rcInstall.left = rcInstall.right - rcTmpCalc.Width();
				rcInstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
				dcx.DrawText(_T("正在安装"), -1, &rcInstall, DT_VCENTER | DT_SINGLELINE);
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
				pImage = BkPngPool::Get(IDP_LISTCTRL_BTN_UPDATE);
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
				//按钮的正常状态
				Gdiplus::RectF rcDest(rcStateBtn.left, rcStateBtn.top, rcStateBtn.Width(), rcStateBtn.Height());
				grap.DrawImage(pImage, rcDest, 0, 0,
					rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
			}
		}

		dcx.Detach();
	}
}

void CSoftMgrUpdateListView::DrawItem ( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if (!m_bDrawItem)
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

void CSoftMgrUpdateListView::RefreshItem(int nIndex)
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

void CSoftMgrUpdateListView::RefreshProgress(int nIndex)
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

void CSoftMgrUpdateListView::RefreshButton(int nIndex)
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
				m_rcStateBtn.right = rcTmp.Width() - 75;
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
			m_rcStateBtn.right = rcTmp.Width() - 75;
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

void CSoftMgrUpdateListView::RefreshIcon(int nIndex)
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

void CSoftMgrUpdateListView::RefreshRight(int nIndex)
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

void CSoftMgrUpdateListView::DeleteAllItemsEx() //清空列表页删除数据
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
			//delete pListData;
			//pListData = NULL;
			pListData->m_dwDataID = -1;
		}
	}
	m_arrData.RemoveAll();		
}

void CSoftMgrUpdateListView::ClearAllItemData() //只清空列表，不删除数据
{
	m_arrData.RemoveAll();	
	CListViewCtrl::DeleteAllItems();
}

void CSoftMgrUpdateListView::DeleteItem(LPDELETEITEMSTRUCT lParam)
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

int CSoftMgrUpdateListView::GetItemCount()
{
	return m_arrData.GetSize();
}

BOOL CSoftMgrUpdateListView::OnEraseBkgnd(CDCHandle dc)
{
	return TRUE;
}

BOOL CSoftMgrUpdateListView::OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
{
	SetMsgHandled(false);

	return TRUE;
}

void CSoftMgrUpdateListView::OnLButtonDBClk(UINT uMsg, CPoint ptx)
{
	LVHITTESTINFO hti;
	hti.pt = ptx;
	SubItemHitTest( &hti );

	CSoftListItemData *pData = GetItemDataEx(hti.iItem);
	if( pData != NULL && hti.iSubItem==0 )
	{
		m_linkerCB->OnUpdateViewDetail(hti.iItem, m_hWnd);
	}
}

void CSoftMgrUpdateListView::OnSetFocus(HWND hWnd) 
{
	GetParent().GetParent().SetFocus();
	return;
}

BOOL CSoftMgrUpdateListView::FindItem(CSoftListItemData *pData)
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

void CSoftMgrUpdateListView::UpdateSoft(int nIndex, HWND hWnd)
{
	m_linkerCB->OnUpdateDownLoad(nIndex, hWnd);
}