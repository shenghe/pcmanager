#include "stdafx.h"
#include "beikesafe.h"
#include "SoftMgrRubbishListView.h"

#define SOFTMGR_RUBBISH_LIST_ITEM_HEIGHT 54
#define SOFTMGR_RUBBISH_LIST_RIGHT_WIDTH 310

#define	COLOR_SOFTMGR_PLUG		RGB(210,100,0)
#define	COLOR_SOFTMGR_NOPLUG	RGB(10,150,0)
#define COLOR_MOUSE_ON_NAME		RGB(0,100,200)


CSoftMgrRubbishListView::CSoftMgrRubbishListView()
{
	m_rcCheck = CRect(10, 20, 23, 33);
	m_rcIcon = CRect(28, 11, 60, 43);
	m_rcName = CRect(68, 9, 128, 23);
	m_rcDescription = CRect(68, 33, 128, 45);
	m_rcState = CRect(0,0,0,0);

	m_bDrawItem = FALSE;
	m_bMouseOn = FALSE;
	m_bMouseDown = FALSE;
	m_bMouseOnName = FALSE;

	m_fntNameOn.Attach( BkFontPool::GetFont(FALSE,TRUE,FALSE,0));
	m_fntNameNormal.Attach( BkFontPool::GetFont(TRUE, FALSE, FALSE, 0));
	m_fntDefault.Attach( BkFontPool::GetFont(BKF_DEFAULTFONT));
}

CSoftMgrRubbishListView::~CSoftMgrRubbishListView()
{
	if(m_hWnd != NULL)
	{
		DeleteAllItems();
	}

	m_fntNameOn.Detach();
	m_fntNameNormal.Detach();
	m_fntDefault.Detach();
}

HRESULT	CSoftMgrRubbishListView::OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bMsgHandled )
{
	bMsgHandled = FALSE;

	return S_OK;
}

void CSoftMgrRubbishListView::InitNormal()
{
	CRect rcTmp;
	GetClientRect(&rcTmp);

	m_rcStateBtn.right = rcTmp.Width() - 15;
	m_rcStateBtn.left = m_rcStateBtn.right - 65;
	m_rcStateBtn.top = 14;
	m_rcStateBtn.bottom = 40;

	m_rcState.right = m_rcStateBtn.left - 20;
	m_rcState.left = m_rcState.right - 36;
	m_rcState.top = m_rcStateBtn.top;
	m_rcState.bottom = m_rcStateBtn.bottom;

	m_rcSize.right = m_rcStateBtn.left - 70;
	m_rcSize.left = m_rcSize.right - 70;
	m_rcSize.top = m_rcStateBtn.top;
	m_rcSize.bottom = m_rcStateBtn.bottom;
}

LRESULT	CSoftMgrRubbishListView::OnCtlColor(HDC hDc, HWND hWnd)
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

CSoftListItemData*	CSoftMgrRubbishListView::GetItemDataEx(DWORD dwIndex)
{
	ATLASSERT(dwIndex >= 0 && dwIndex < (UINT)m_arrData.GetSize());
	if (dwIndex < 0 || dwIndex >= (UINT)m_arrData.GetSize())
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

int CSoftMgrRubbishListView::SetItemDataEx(DWORD dwIndex, DWORD_PTR dwItemData)
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

BOOL CSoftMgrRubbishListView::GetItemRect(int nItem, LPRECT lpRect, UINT nCode)
{
	return  CListViewCtrl::GetItemRect(nItem, lpRect, nCode);
}

HRESULT	CSoftMgrRubbishListView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
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
			InitNormal();
			CRect rcTmp;
			GetClientRect(&rcTmp);

			pts.x -= rcItem.left;
			pts.y -= rcItem.top;

			if ( m_rcCheck.PtInRect(pts) )
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

HRESULT	CSoftMgrRubbishListView::OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
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
			CRect rcTmp;
			GetClientRect(&rcTmp);

			pts.x -= rcItem.left;
			pts.y -= rcItem.top;

			if ( m_rcCheck.PtInRect(pts) )
			{
				pData->m_bCheck = !pData->m_bCheck;
				m_linkerCB->OnLbuttonUpNotifyEx(nIndex);
				CRect rcCheck = m_rcCheck;
				rcCheck.OffsetRect(rcItem.left, rcItem.top);
				InvalidateRect(rcCheck, FALSE);
			}
			else if(m_rcStateBtn.PtInRect(pts))
			{
				if (m_bMouseDown)
				{
					pData->m_bDownloading = TRUE;
					pData->m_bContinue = TRUE;
					pData->m_bPause = FALSE;
					m_linkerCB->OnResume( pData->m_strSoftID );
				}
			}
		}
	}

	m_bMouseDown = FALSE;
	return S_OK;		
}

void CSoftMgrRubbishListView::SetClickLinkCallback( ISoftMgrRubbishCallback* opCB )
{
	m_linkerCB = opCB;
}

VOID CSoftMgrRubbishListView::SetPaintItem(BOOL bDrawItem)
{
	m_bDrawItem = bDrawItem;
}

void CSoftMgrRubbishListView::MeasureItem(LPMEASUREITEMSTRUCT lpMes)
{
	lpMes->itemHeight = SOFTMGR_RUBBISH_LIST_ITEM_HEIGHT;
}

HRESULT	CSoftMgrRubbishListView::OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bMsgHandled )
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
	int nIndex = hti.iItem;

	if (/*bMove &&*/ nIndex >= 0 && nIndex < m_arrData.GetSize() && GetItemRect(nIndex,&rcItem,LVIR_BOUNDS) )
	{
		CSoftListItemData*	pData = (CSoftListItemData*)GetItemDataEx(nIndex);
		if(pData != NULL)
		{	
			InitNormal();
			CRect rcTmp;
			GetClientRect(&rcTmp);

			pts.x -= rcItem.left;
			pts.y -= rcItem.top;

			if( m_rcStateBtn.PtInRect(pts))
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

			if(m_enumLastMSState == MOUSE_LASTON_STATEBTN && !m_bMouseOn)
			{
				m_bMouseOn = FALSE;
				CRect rcStateBtn = m_rcStateBtn;
				rcStateBtn.OffsetRect(rcItem.left, rcItem.top);
				InvalidateRect(rcStateBtn, FALSE);
				m_enumLastMSState = MOUSE_LASTON_NONE;
			}
		}
	}

	return S_OK;
}

void CSoftMgrRubbishListView::_DrawItem(HDC hDC, LPDRAWITEMSTRUCT lpDrawItemStruct)
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

		InitNormal();
		CRect rcTmp;
		GetClientRect(&rcTmp);

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
		CRect rcCheck = m_rcCheck;
		rcCheck.OffsetRect( lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		if ( bSelect )
		{				
			Gdiplus::SolidBrush mySolidBrush(Gdiplus::Color(255,235,245,255/*255,240,205*/));
			if (rcWnd.Height() > GetItemCount()*54)
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

		//画check box的状态
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
		dcTmp.SelectBitmap(hBmpOld);

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
		if (m_bMouseOnName)
		{
			hFntTmp = dcx.SelectFont(m_fntNameNormal);
		}
		else
		{
			hFntTmp = dcx.SelectFont(m_fntNameNormal);
		}
		dcx.DrawText(pListData->m_strName, -1, &rcProb, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);

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
		rcName.right = rcItem.Width() - 240 - nLablesWidth;

		pListData->m_rcName = rcName;
		if (rcProb.Width() < rcItem.Width() - 240 - nLablesWidth - 68)
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
		rcIntro.right = rcItem.Width() - 240;
		if (rcProb.Width() < rcItem.Width() - SOFTMGR_RUBBISH_LIST_RIGHT_WIDTH)
		{
			rcIntro.right = rcIntro.left + rcProb.Width();
		}
		rcIntro.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		dcx.DrawText(pListData->m_strFileName, -1, &rcIntro, DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );


		//画软件大小
		CRect rcSize = m_rcSize;
		rcSize.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		dcx.DrawText(pListData->m_strSize, -1, &rcSize, DT_VCENTER | DT_SINGLELINE);

		//状态
		CRect rcInstall = m_rcState;
		CRect rcTmpCalc;
		dcx.DrawText(BkString::Get(IDS_SOFTMGR_8104), -1, &rcTmpCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
		rcInstall.left = rcInstall.right - rcTmpCalc.Width();
		rcInstall.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		dcx.DrawText(BkString::Get(IDS_SOFTMGR_8104), -1, &rcInstall, DT_VCENTER | DT_SINGLELINE);

		//最右侧按钮
		CRect rcStateBtn = m_rcStateBtn;
		rcStateBtn.OffsetRect(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		
		pImage = BkPngPool::Get(IDP_SOFTMGR_REVERT);

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
			//按钮的正常状态
			grap.DrawImage(pImage, rcStateBtn.left, rcStateBtn.top, 0, 0,
				rcStateBtn.Width(), rcStateBtn.Height(), Gdiplus::UnitPixel);
		}
		dcx.Detach();
	}
}

void CSoftMgrRubbishListView::DrawItem ( LPDRAWITEMSTRUCT lpDrawItemStruct )
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

void CSoftMgrRubbishListView::SetCheckedAllItem(BOOL bChecked)
{
	for (int i = 0; i < m_arrData.GetSize(); i++)
	{
		CSoftListItemData*	pListData = GetItemDataEx(i);
		pListData->m_bCheck = bChecked;

		CRect rcCheck = m_rcCheck;
		CRect rcItem;
		GetItemRect(i, &rcItem, LVIR_BOUNDS);
		rcCheck.OffsetRect(rcItem.left, rcItem.top);
		InvalidateRect(rcCheck, FALSE);
	}
}

void CSoftMgrRubbishListView::RefreshItemCheckedState(int nIndex)
{
	CRect rcCheck = m_rcCheck;
	CRect rcItem;
	GetItemRect(nIndex, &rcItem, LVIR_BOUNDS);
	rcCheck.OffsetRect(rcItem.left, rcItem.top);
	InvalidateRect(rcCheck, FALSE);
}

void CSoftMgrRubbishListView::RefreshItem(int nIndex)
{
	ATLASSERT(nIndex < m_arrData.GetSize() && nIndex >= 0);
	if (nIndex < 0 || nIndex >= m_arrData.GetSize())
	{
		return;
	}

	CRect rcIndex;
	GetItemRect(nIndex, &rcIndex, LVIR_BOUNDS);
	InvalidateRect(rcIndex, FALSE);
}

void CSoftMgrRubbishListView::DeleteAllItemsEx() //清空列表页删除数据
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

void CSoftMgrRubbishListView::ClearAllItemData() //只清空列表，不删除数据
{
	m_arrData.RemoveAll();	
	CListViewCtrl::DeleteAllItems();
}

void CSoftMgrRubbishListView::DeleteItem(LPDELETEITEMSTRUCT lParam)
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
	ReQueueList(lParam->itemID);
}

void CSoftMgrRubbishListView::ReQueueList(int nIndex)
{
	for (int i = 0; i < m_arrData.GetSize(); i++)
	{
		if (m_arrData[i]->m_dwID > nIndex)
		{
			m_arrData[i]->m_dwID--;
		}
	}
}

int CSoftMgrRubbishListView::GetItemCount()
{
	return m_arrData.GetSize();
}

CSimpleArray<CSoftListItemData*>& CSoftMgrRubbishListView::GetItemArray() 
{ 
	return m_arrData;
}

void CSoftMgrRubbishListView::UpdateAll()
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