
#include "stdafx.h"
#include "beikesafe.h"
#include "SoftMgrUpdateRollWnd.h"

CSoftMgrUpdateRollWnd::CSoftMgrUpdateRollWnd()
{
	m_pHostWnd = NULL;
	m_nScrollPos = 0;
}

CSoftMgrUpdateRollWnd::~CSoftMgrUpdateRollWnd()
{
}

int CSoftMgrUpdateRollWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// center the dialog on the screen
	CenterWindow();

	//
	m_pHostWnd = new CSoftMgrUpdateHostWnd;
	m_pHostWnd->Create(m_hWnd);

	_ReplaceScroll();

	return 0;
}

void CSoftMgrUpdateRollWnd::OnSize(UINT nType, CSize size)
{
	_ReplaceScroll();
}

void CSoftMgrUpdateRollWnd::_ReplaceScroll()
{
	if (!m_pHostWnd || !m_pHostWnd->IsWindow())
		return;

	CRect rcClient;
	GetClientRect(&rcClient);

	//
	CRect rcHost;
	rcHost = rcClient;
	rcHost.bottom = m_pHostWnd->GetWindowMaxHeight();
	m_pHostWnd->MoveWindow(rcHost);
	m_pHostWnd->ShowWindow(SW_SHOW);

	//
	SCROLLINFO si = {0};
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin   = 0;
	si.nMax   = m_pHostWnd->GetWindowMaxHeight();
	si.nPos   = 0;
	si.nPage   = rcClient.Height();
	::SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE); 

	m_nScrollPos = 0;
}

void CSoftMgrUpdateRollWnd::OnPaint(CDCHandle dc)
{
	PAINTSTRUCT ps;
	HDC hDC = ::BeginPaint( m_hWnd, &ps );

	WTL::CRect rcClient;
	GetClientRect(&rcClient);

	CDC dcClient(hDC);
	dcClient.FillSolidRect(&rcClient, RGB(255, 255, 255));

	::EndPaint( m_hWnd, &ps );
}

void CSoftMgrUpdateRollWnd::OnClose()
{
	SetMsgHandled(false);
}

void CSoftMgrUpdateRollWnd::OnDestroy()
{
	SetMsgHandled(false);
}

void CSoftMgrUpdateRollWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar)
{
	int nCurPos = GetScrollPos(SB_VERT);
	int nOldPos = nCurPos;

	SCROLLINFO siVert = {0};
	siVert.cbSize = sizeof(SCROLLINFO);
	siVert.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
	::GetScrollInfo(m_hWnd, SB_VERT, &siVert);

	SetFocus();

	switch (nSBCode)
	{
	case SB_TOP:
		nCurPos = 0;
		break;
	case SB_BOTTOM:
		nCurPos = siVert.nMax;
		break;
	case SB_LINEUP:
		nCurPos = max(nCurPos - ITEM_HEIGHT, 0);
		break;
	case SB_LINEDOWN:
		nCurPos = min(nCurPos + ITEM_HEIGHT, siVert.nMax - (int)siVert.nPage );
		break;
	case SB_PAGEUP:
		nCurPos = max(nCurPos - (int)siVert.nPage, 0);
		break;
	case SB_PAGEDOWN:
		nCurPos = min(nCurPos + (int)siVert.nPage, siVert.nMax - (int)siVert.nPage);
		break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		nCurPos = min(siVert.nTrackPos, siVert.nMax - (int)siVert.nPage);
		break;
	}
	if (nOldPos != nCurPos)
	{
		SetScrollPos(SB_VERT, nCurPos);

		if (m_pHostWnd && m_pHostWnd->IsWindow())
			m_pHostWnd->ScrollWindow(0, m_nScrollPos - nCurPos, NULL);

		m_nScrollPos = nCurPos;
	}

	SetMsgHandled(false);
}

BOOL CSoftMgrUpdateRollWnd::OnEraseBkgnd(CDCHandle dc)
{
	return TRUE;
}

BOOL CSoftMgrUpdateRollWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	//
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		WTL::CRect rcClient;
		GetClientRect(&rcClient);
		int nMaxHeight = m_pHostWnd->GetWindowMaxHeight();
		if (rcClient.Height() > nMaxHeight)
			return TRUE;
	}

	//
	int nCurPos = GetScrollPos(SB_VERT);
	int nOldPos = nCurPos;

	SCROLLINFO siVert = {0};
	siVert.cbSize = sizeof(SCROLLINFO);
	siVert.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
	::GetScrollInfo(m_hWnd, SB_VERT, &siVert);

	SetFocus();

	if (zDelta > 0) 
		nCurPos = max(nCurPos - ITEM_HEIGHT, 0);
	else
		nCurPos = min(nCurPos + ITEM_HEIGHT, siVert.nMax - (int)siVert.nPage);

	if (nOldPos != nCurPos)
	{
		SetScrollPos(SB_VERT, nCurPos);

		if (m_pHostWnd && m_pHostWnd->IsWindow())
			m_pHostWnd->ScrollWindow(0, m_nScrollPos - nCurPos, NULL);

		m_nScrollPos = nCurPos;
	}

	return TRUE;
}

void CSoftMgrUpdateRollWnd::SetCheckNotifyHwnd(HWND hWnd)
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		m_pHostWnd->SetCheckNotifyHwnd(hWnd);
	}
}

void CSoftMgrUpdateRollWnd::SetClickLinkCallback( ISoftMgrUpdateCallback* opCB )
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		m_pHostWnd->SetClickLinkCallback(opCB);
	}
}

void CSoftMgrUpdateRollWnd::AddItem(DWORD_PTR dwItemData)
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		m_pHostWnd->AddItem(dwItemData);
	}
}

void CSoftMgrUpdateRollWnd::UpdateCtrlWindow()
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		m_pHostWnd->UpdateCtrlWindow();
		_ReplaceScroll();
	}
}

CSoftListItemData* CSoftMgrUpdateRollWnd::GetItemDataEx(DWORD dwIndex, HWND hWndList)
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		return m_pHostWnd->GetItemDataEx(dwIndex, hWndList);
	}

	return NULL;
}

void CSoftMgrUpdateRollWnd::RefreshRight(int nIndex, HWND hWndList)
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		return m_pHostWnd->RefreshRight(nIndex, hWndList);
	}
}

void CSoftMgrUpdateRollWnd::RefreshRight(LPCTSTR strSoftID)
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		return m_pHostWnd->RefreshRight(strSoftID);
	}
}

void CSoftMgrUpdateRollWnd::RefreshIcon(LPCTSTR strSoftID)
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		return m_pHostWnd->RefreshIcon(strSoftID);
	}
}

void CSoftMgrUpdateRollWnd::UpdateSelectSoft()
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		return m_pHostWnd->UpdateSelectSoft();
	}
}

void CSoftMgrUpdateRollWnd::DeleteAllItems()
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		return m_pHostWnd->DeleteAllItems();
	}
}

int CSoftMgrUpdateRollWnd::GetCheck()
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		return m_pHostWnd->GetCheck();
	}

	return 0;
}

int CSoftMgrUpdateRollWnd::GetUpdateCount(int& nRecommend, int& nChoice)
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		return m_pHostWnd->GetUpdateCount(nRecommend, nChoice);
	}

	return 0;
}

void CSoftMgrUpdateRollWnd::UpdateSoft(LPCTSTR strSoftID)
{
	if (m_pHostWnd && m_pHostWnd->IsWindow())
	{
		return m_pHostWnd->UpdateSoft(strSoftID);
	}
}