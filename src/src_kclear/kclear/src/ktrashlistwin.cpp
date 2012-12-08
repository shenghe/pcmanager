///////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "trashpublicfunc.h"
#include "ktrashlistwin.h"
#include "kscgui/kdraw.h"

//////////////////////////////////////////////////////////////////////////

CTrashCleanCtrl::CTrashCleanCtrl()
{
	KAppRes& appRes = KAppRes::Instance();
	m_bmpCheck = appRes.GetImage(DEF_IMG_TRASH_CHECK_BOX);
	m_bmpDetail = appRes.GetImage(DEF_IMG_TRASH_DETAIL);
	m_bmpShadow = appRes.GetImage(DEF_IMG_TRASH_SHADOW);
    m_hImageDetailBar = appRes.GetImagePlus(DEF_IMG_DETAIL_BAR);
	m_nPos = 0;
	counter = 0;
    m_bToolTipActive = FALSE;
    m_bEnbale = TRUE;
}

CTrashCleanCtrl::~CTrashCleanCtrl()
{

}
//////////////////////////////////////////////////////////////////////////
void CTrashCleanCtrl::Initialize(HWND hWnd)
{
	KAppRes& appRes = KAppRes::Instance();
	
	m_hNotifyHwnd = hWnd;

	m_iconList.Create(DEF_ICON_HEIGHT, DEF_ICON_HEIGHT, ILC_COLOR32 | ILC_MASK, 0,0);
	m_iconList.Add(appRes.GetImage("IDB_TRASH_WEB"), RGB(255, 0, 255));				// 0
	m_iconList.Add(appRes.GetImage("IDB_TRASH_VIDEO"), RGB(255, 0, 255));			// 1
	m_iconList.Add(appRes.GetImage("IDB_TRASH_FILE"), RGB(255, 0, 255));			// 2
	m_iconList.Add(appRes.GetImage("IDB_TRASH_RECYCLE_BIN"), RGB(255, 0, 255));		// 3
	m_iconList.Add(appRes.GetImage("IDB_TRASH_CUSTOM"), RGB(255, 0, 255));			// 4

	m_wndToolTip.Create(m_hWnd);
	m_wndToolTip.SetMaxTipWidth(400);
    CToolInfo ti(0, m_hWnd);
	m_wndToolTip.AddTool(&ti);
	m_wndToolTip.Activate(FALSE);
	m_wndToolTip.SetDelayTime(TTDT_AUTOPOP, 10000);
    m_nHoverIndex = -1;
    m_nSelectedIndex = -1;

	_RangeItems();
}

BOOL CTrashCleanCtrl::SubclassWindow(HWND hWnd)
{
	BOOL bRet = FALSE;

	bRet = CWindowImpl<CTrashCleanCtrl>::SubclassWindow(hWnd);
	GetClientRect(&m_rcClient);
	return bRet;
}
//////////////////////////////////////////////////////////////////////////
BOOL CTrashCleanCtrl::AddItem(TRASH_ITEM& item)
{
	m_vecTrashItems.push_back(item);
	return TRUE;
}

void CTrashCleanCtrl::SetCheckAll(BOOL bChecked)
{
	int iCount, jCount;

	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		m_vecTrashItems[iCount].bShowDetail = TRUE;
		for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
		{
			m_vecTrashItems[iCount].vecItems[jCount].bChecked = bChecked;
		}
	}

	_RangeItems();
}
void CTrashCleanCtrl::SetItemChecked(UINT nItemID, BOOL bChecked)
{
	int iCount, jCount;

	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
		{
			if (m_vecTrashItems[iCount].vecItems[jCount].nItemId == nItemID)
			{
				m_vecTrashItems[iCount].vecItems[jCount].bChecked = bChecked;
				break;
			}
		}
	}
}

void CTrashCleanCtrl::SetStatus(UINT nItemID /* = -1 */, UINT nStatus /* = Empty_Info */)
{
	int iCount, jCount;

	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		if (nItemID == -1 || m_vecTrashItems[iCount].uItemId == nItemID)
		{
			m_vecTrashItems[iCount].nStatus = nStatus;
		}
		for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
		{
			if (nItemID == -1 || m_vecTrashItems[iCount].vecItems[jCount].nItemId == nItemID)
			{
				m_vecTrashItems[iCount].vecItems[jCount].nStatus = nStatus;

				if (nStatus == Scaning || nStatus == Clearing)
				{
					if (m_vecTrashItems[iCount].nStatus == WaitScan)
					{
						m_vecTrashItems[iCount].nStatus = Scaning;
					}
					else if (m_vecTrashItems[iCount].nStatus == WaitClear)
					{
						m_vecTrashItems[iCount].nStatus = Clearing;
					}
				}
			}
		}	
		
		if (nStatus == FinishScan || nStatus == SkipRunning)
		{
			//
			if (_CheckChildItemAllFinish(&m_vecTrashItems[iCount]))
			{
				m_vecTrashItems[iCount].nStatus = FinishScan;
			}
		}

		if (nStatus == Scaning || nStatus == Clearing)
		{
			if (_CheckChildNoneSelected(&m_vecTrashItems[iCount]))
			{
				m_vecTrashItems[iCount].nStatus = Empty_Info;
			}
		}
	}


	Invalidate();
}

void CTrashCleanCtrl::SetItemEnable(BOOL bEnbale /* = TRUE */)
{
	// TODO ....
    m_bEnbale = bEnbale;
}

void CTrashCleanCtrl::SetItemStatusInfo(UINT nItemID,UINT nCount, UINT nFileCount, ULONGLONG nSize)
{
	// TODO ....
	int iCount, jCount;

	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		// 大标题的状态
		for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
		{
			if (m_vecTrashItems[iCount].vecItems[jCount].nItemId == nItemID)
			{
				m_vecTrashItems[iCount].vecItems[jCount].nFileNum = nFileCount;
				m_vecTrashItems[iCount].vecItems[jCount].uTotalSize = nSize;
				goto Clear0;
			}
		}
	}
Clear0:
	return;
}

void CTrashCleanCtrl::GetSelectedItems(std::vector<UINT>& selectItems)
{
	int iCount, jCount;
	selectItems.clear();

	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		if (!m_vecTrashItems[iCount].bShow)
			continue;
		for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
		{
			if (m_vecTrashItems[iCount].vecItems[jCount].bChecked)
			{
				selectItems.push_back(m_vecTrashItems[iCount].vecItems[jCount].nItemId);
			}
		}
	}

}

void CTrashCleanCtrl::GetItemName(UINT nItemID, CString& strName)
{
	int iCount, jCount;

	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		if (nItemID == m_vecTrashItems[iCount].uItemId)
		{
			strName = m_vecTrashItems[iCount].strItemName;
			goto Clear0;
		}
		for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
		{
			if (nItemID == m_vecTrashItems[iCount].vecItems[jCount].nItemId)
			{
				strName = m_vecTrashItems[iCount].vecItems[jCount].strName;
				goto Clear0;
			}
		}
	}
Clear0:
	return;
}

void CTrashCleanCtrl::SetItemShowDetail(UINT nItemID, BOOL bShow)
{
	int iCount, jCount;

	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		if (nItemID == m_vecTrashItems[iCount].uItemId)
		{
			m_vecTrashItems[iCount].bShowDetail = bShow;
			goto Clear0;
		}
		for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
		{
			if (nItemID == m_vecTrashItems[iCount].vecItems[jCount].nItemId)
			{
				m_vecTrashItems[iCount].bShowDetail = bShow;
				goto Clear0;
			}
		}

	}
Clear0:
	_RangeItems();
}
void CTrashCleanCtrl::SetItemShow(UINT nItemId, BOOL bShow)
{
	int iCount;

	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		if (nItemId == m_vecTrashItems[iCount].uItemId)
		{
			m_vecTrashItems[iCount].bShow = bShow;
			break;
		}
	}			
	_RangeItems();
}
BOOL CTrashCleanCtrl::GetItemShow(UINT nItemId)
{
    BOOL bRet = FALSE;
    int iCount;

    for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
    {
        if (nItemId == m_vecTrashItems[iCount].uItemId)
        {
            bRet = m_vecTrashItems[iCount].bShow;
            break;
        }
    }	

    return bRet;
}
BOOL CTrashCleanCtrl::CheckItemAllSelected()
{
	BOOL bRet = FALSE;
	int iCount, jCount;

	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		if (!m_vecTrashItems[iCount].bShow)
		{
			continue;
		}
		for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
		{
			if (!m_vecTrashItems[iCount].vecItems[jCount].bChecked)
			{
				goto Clear0;
			}
		}

	}

	bRet = TRUE;
Clear0:
	return bRet;
}
void CTrashCleanCtrl::ClearAllStatus()
{
	int iCount, jCount;

	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		m_vecTrashItems[iCount].nStatus = Empty_Info;
		for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
		{
			m_vecTrashItems[iCount].vecItems[jCount].nStatus = Empty_Info;
			m_vecTrashItems[iCount].vecItems[jCount].nFileNum = 0;
			m_vecTrashItems[iCount].vecItems[jCount].uTotalSize = 0;
		}

	}
}

BOOL CTrashCleanCtrl::GetDetailItemData(UINT nItemID, ULONGLONG& uSize, int& nCount)
{
    BOOL bRet = FALSE;
    int iCount, jCount;

    uSize = 0;
    nCount = 0;

    for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
    {
        for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
        {
            if (nItemID == m_vecTrashItems[iCount].vecItems[jCount].nItemId)
            {
                uSize = m_vecTrashItems[iCount].vecItems[jCount].uTotalSize;
                nCount = m_vecTrashItems[iCount].vecItems[jCount].nFileNum;
                bRet = TRUE;
                goto Clear0;
            }
        }
    }

Clear0:
    return bRet;
}

BOOL CTrashCleanCtrl::SetSelectedItem(UINT nItemID)
{
    BOOL bRet = FALSE;
    size_t iCount, jCount;
    SCROLLINFO si;
    int nItemHeiht = 0;

    if (nItemID < 0)
        goto Clear0;

    for (iCount = 0; iCount < m_vecTrashItems.size(); ++iCount)
    {
        if (!m_vecTrashItems[iCount].bShow)
        {
            continue;
        }
        for (jCount = 0; jCount < m_vecTrashItems[iCount].vecItems.size(); ++jCount)
        {
            if (nItemID == m_vecTrashItems[iCount].vecItems[jCount].nItemId)
            {
                m_nSelectedIndex = m_vecTrashItems[iCount].vecItems[jCount].nItemId;
                m_vecTrashItems[iCount].bShowDetail = TRUE;
                nItemHeiht = m_vecTrashItems[iCount].vecItems[jCount].rcCheck.top;
                if (nItemHeiht == 0)
                    nItemHeiht = m_vecTrashItems[iCount].rcBox.bottom;
                bRet = TRUE;
                break;
            }
        }
        if (bRet) break;
    }
    
    if (bRet)
    {
        si.cbSize = sizeof si;
        si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
        GetScrollInfo(SB_VERT, &si);

        si.fMask = SIF_POS;

        m_nPos = (nItemHeiht - (int)si.nPage / 4);

        if (m_nPos < 0)
            m_nPos = 0;

        si.nPos = m_nPos;

        SetScrollInfo(SB_VERT, &si, TRUE);
    }

Clear0:
    return bRet;
}
UINT CTrashCleanCtrl::GetItemParent(UINT nItemID)
{
    UINT nParentID = -1;
    int iCount, jCount;

    for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
    {
        for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
        {
            if (nItemID == m_vecTrashItems[iCount].vecItems[jCount].nItemId)
            {
                nParentID = m_vecTrashItems[iCount].uItemId;
                goto Clear0;
            }
        }
    }

Clear0:
    return nParentID;
}
//////////////////////////////////////////////////////////////////////////
//
void CTrashCleanCtrl::OnPaint(CDCHandle /*dcHandle*/)
{
	CPaintDC dc(m_hWnd);
	CBitmap memBmp;
	CDC memDC;
	CRect rcClient(m_rcClient);
	CString str;
// 	str.Format(L"paint times is %d\n", counter++);
// 	OutputDebugStr(str);
//	dc.FillSolidRect(m_rcRealClient, RGB(255, 255, 255));
    if (m_vecTrashItems.size() <= 0)
    {
        CString strWait = L"正在加载，请稍候...";
        HFONT hFont = KuiFontPool::GetFont(FALSE, FALSE, FALSE);
        HFONT oldFont = dc.SelectFont(hFont);
        CRect rcDraw(m_rcClient);
        rcDraw.top += 50;
        dc.FillSolidRect(m_rcRealClient, RGB(255, 255, 255));
        dc.DrawText(strWait, strWait.GetLength(), &rcDraw, DT_CENTER | DT_VCENTER);
        dc.SelectFont(oldFont);
        return;
    }
	memDC.CreateCompatibleDC(dc.m_hDC);
	memBmp.CreateCompatibleBitmap(dc.m_hDC, m_rcRealClient.Width(), m_rcRealClient.Height());
	memDC.SetBkMode(TRANSPARENT);
	SelectObject(memDC, memBmp);

	memDC.FillSolidRect(m_rcRealClient, RGB(255, 255,255));

	_OnRealPaint(memDC);

	m_nHeight = int((m_rcRealClient.Height() - m_rcClient.Height()) * (m_nPos * 1.0) / (m_rcRealClient.Height()));

	dc.BitBlt(0, 0, m_rcClient.Width(), m_rcClient.Height(), memDC, 0, m_nHeight, SRCCOPY);

	ReleaseDC(memDC);
}

BOOL CTrashCleanCtrl::OnEarseBKGND(CDCHandle dc)
{	
	return TRUE;
}

void CTrashCleanCtrl::OnSize(UINT uMsg, CSize size)
{
	GetClientRect(&m_rcClient);

	_RangeItems();
}
void CTrashCleanCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	BOOL bIn = FALSE;
	int iCount, jCount;
	CPoint hitPoint(point);

	hitPoint.y = point.y + LONG((m_rcRealClient.Height() - m_rcClient.Height()) * (m_nPos * 1.0) / (m_rcRealClient.Height()));
	SetFocus();	
	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		if (m_vecTrashItems[iCount].rcBox.PtInRect(hitPoint))
		{
			bIn = TRUE;
			CString des = m_vecTrashItems[iCount].strItemDes;
			if (m_strTip.CompareNoCase(des) != 0 || !m_bToolTipActive)
			{
                m_bToolTipActive = TRUE;
				m_strTip = des;
				m_wndToolTip.Activate(FALSE);
				m_wndToolTip.UpdateTipText((_U_STRINGorID)des, m_hWnd);
				m_wndToolTip.Activate(TRUE);	
			}
            if (m_nHoverIndex != m_vecTrashItems[iCount].uItemId)
            {
                m_nHoverIndex = m_vecTrashItems[iCount].uItemId;
                InvalidateRect(NULL);
            }
            goto Clear0;
		}
        if (m_vecTrashItems[iCount].bShowDetail)
        {
            for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
            {

                if (m_vecTrashItems[iCount].vecItems[jCount].rcCheck.PtInRect(hitPoint) ||
                    m_vecTrashItems[iCount].vecItems[jCount].rcSelected.PtInRect(hitPoint))
                {                
                    if (m_nSelectedIndex == m_vecTrashItems[iCount].vecItems[jCount].nItemId)
                    {
                        if (m_vecTrashItems[iCount].vecItems[jCount].rcDetail.PtInRect(hitPoint))
                        {
                            m_vecTrashItems[iCount].vecItems[jCount].bInDetail = TRUE;
                        }
                        else
                        {
                            m_vecTrashItems[iCount].vecItems[jCount].bInDetail = FALSE;
                        }

                        InvalidateRect(NULL);
                    }
                    if (m_nHoverIndex != m_vecTrashItems[iCount].vecItems[jCount].nItemId)
                    {
                        m_nHoverIndex = m_vecTrashItems[iCount].vecItems[jCount].nItemId;
                        InvalidateRect(NULL);
                    } 
                    goto Clear0;
                }
            }
        }
	}

    if (m_nHoverIndex != -1)
    {
        InvalidateRect(NULL);
    }
    m_nHoverIndex = -1;

Clear0:
	if (!bIn)
	{
		m_wndToolTip.Activate(FALSE);
        m_bToolTipActive = FALSE;
	}
    SetMsgHandled(FALSE);
}
void CTrashCleanCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	int iCount;
	CPoint hitPoint(point);

    SetFocus();

    if (!m_bEnbale)
        goto Clear0;

	hitPoint.y = point.y + LONG((m_rcRealClient.Height() - m_rcClient.Height()) * (m_nPos * 1.0) / (m_rcRealClient.Height()));

	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		if (m_vecTrashItems[iCount].rcBox.PtInRect(hitPoint)||m_vecTrashItems[iCount].rcIcon.PtInRect(hitPoint))
		{
			m_vecTrashItems[iCount].bShowDetail = !m_vecTrashItems[iCount].bShowDetail;
            m_nSelectedIndex = m_vecTrashItems[iCount].uItemId;
			
            _RangeItems();
			
            Invalidate();
			
            goto Clear0;
		}
		if (m_vecTrashItems[iCount].bShowDetail)
		{		
            if (_CheckInCheckDetail(hitPoint, &m_vecTrashItems[iCount]))
            {
                Invalidate();
                goto Clear0;
            }
			else if (_CheckInCheckBox(hitPoint, &m_vecTrashItems[iCount]) ||
                     _CheckInSelectArea(hitPoint, &m_vecTrashItems[iCount]))
			{
                Invalidate();
				goto Clear0;
			}
// 			if (m_vecTrashItems[iCount].rcIcon.PtInRect(hitPoint))
// 			{
// 				_CheckItemAll(&m_vecTrashItems[iCount]);
// 				Invalidate(FALSE);
// 				goto Clear0;
// 			}
		}
	}

    m_nSelectedIndex = -1;

Clear0:
	SetMsgHandled(FALSE);
}
void CTrashCleanCtrl::OnMouseLeave()
{
	SetMsgHandled(FALSE);
}
LRESULT CTrashCleanCtrl::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SCROLLINFO si;
	int vertPos;

	si.cbSize = sizeof si;
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo(SB_VERT, &si);

	vertPos = si.nPos;

	switch (LOWORD(wParam))
	{
	case SB_TOP:
		si.nPos = si.nMin;
		break;

	case SB_BOTTOM:
		si.nPos = si.nMax;
		break;

	case SB_LINEUP:
		si.nPos -= 1;
		break;

	case SB_LINEDOWN:
		si.nPos += 1;
		break;

	case SB_PAGEUP:
		si.nPos -= si.nPage;
		break;

	case SB_PAGEDOWN:
		si.nPos += si.nPage;
		break;

	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos;
		break;

	default:
		break;
	}

	si.fMask = SIF_ALL;

	m_nPos = abs(si.nPos);
	SetScrollInfo(SB_VERT, &si, TRUE);
	int nScrollHeight = int((m_rcRealClient.Height() - m_rcClient.Height()) * (m_nPos * 1.0) / (m_rcRealClient.Height()));
	int n = (m_nHeight - nScrollHeight);
	if (n > 1 || n < -1)
	{
		Invalidate();
	}
	bHandled = FALSE;
	return TRUE;
}

LRESULT CTrashCleanCtrl::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int zDelta = (short) HIWORD(wParam);

	SCROLLINFO si;

	si.cbSize = sizeof si;
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo(SB_VERT, &si);

	if (zDelta > 0)
	{
		si.nPos -= 27;
	}
	else
	{
		si.nPos += 27;
	}
	if (si.nPos < 0)
		si.nPos = 0;
	m_nPos = si.nPos;
	SetScrollInfo(SB_VERT, &si, TRUE);
	Invalidate();

	SetMsgHandled(FALSE);
	return FALSE;
}

LRESULT CTrashCleanCtrl::OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_wndToolTip.IsWindow())
	{
		MSG msg = { m_hWnd, uMsg, wParam, lParam };

		m_wndToolTip.RelayEvent(&msg);
	}

	SetMsgHandled(FALSE);

	return 0;
}
//////////////////////////////////////////////////////////////////////////
//
void CTrashCleanCtrl::_OnRealPaint(CDC& dc)
{
	int iCount;
	CRect rcDraw(0,0,0,0);
	int bLastShowDetail = FALSE;
	CRect rcBox;
    int nItemHeight;

//	m_nTotalHeight = DEF_TOP_TO_FIRST;
    m_nTotalHeight = 0;
	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		if (!m_vecTrashItems[iCount].bShow)
			continue;
        
        if (m_vecTrashItems[iCount].bShowDetail)
        {
            nItemHeight =  DEF_ITEM_THIN_HEIGHT;
        }
        else
        {
            nItemHeight =  DEF_ITEM_FAT_HEIGHT;
        }
        rcDraw.left = 0;
        rcDraw.right = m_rcClient.Width();
        rcDraw.top = m_nTotalHeight;
        rcDraw.bottom = rcDraw.top + nItemHeight;
        
        _DrawItemBKGround(dc, rcDraw, &m_vecTrashItems[iCount]);
        
		rcDraw.left = DEF_LEFT_TO_ICON;
		rcDraw.top = m_nTotalHeight;
		rcDraw.bottom = rcDraw.top + nItemHeight;
		rcDraw.right = m_rcClient.right;
		_DrawItemIcon(dc, rcDraw, &m_vecTrashItems[iCount]);

		_DrawShowDetailBar(dc, rcDraw, rcBox, &m_vecTrashItems[iCount]);
		::CopyRect(m_vecTrashItems[iCount].rcBox, rcBox);


		rcDraw.left = (DEF_LEFT_TO_DETAIL - 2);
		rcDraw.top = m_nTotalHeight;
		rcDraw.bottom = rcDraw.top + nItemHeight;
		rcDraw.right = m_rcClient.right;

        // 绘制大条目的文字信息
        _DrawTextInfor(dc, rcDraw, &m_vecTrashItems[iCount]);

        m_vecTrashItems[iCount].rcBox.left = m_rcClient.left;
        m_vecTrashItems[iCount].rcBox.right = m_rcClient.right;
        m_vecTrashItems[iCount].rcBox.top = m_nTotalHeight;
        m_vecTrashItems[iCount].rcBox.bottom = m_nTotalHeight + nItemHeight;


		m_nTotalHeight += nItemHeight;
		
		if (m_vecTrashItems[iCount].bShowDetail)
		{
			m_nTotalHeight += DEF_BETWEEN_SHADOW;
			rcDraw.left = DEF_LEFT_TO_ICON;
			rcDraw.top = m_nTotalHeight;
			_DrawShadowLine(dc, rcDraw);

			rcDraw.left = DEF_LEFT_TO_DETAIL;
			rcDraw.top = m_nTotalHeight;
			_DrawItem(dc, rcDraw, &m_vecTrashItems[iCount]);
			m_nTotalHeight += 1;
		}
// 		else
// 		{
// 			m_nTotalHeight += DEF_BETWEEN_ITEM;
// 		}
	}
}
void CTrashCleanCtrl::_DrawItem(CDC&dc, CRect& rcItem, LPTRASH_ITEM lpItem)
{
	int iCount;
	
	for (iCount = 0; iCount < (int)lpItem->vecItems.size(); ++iCount)
	{
        m_nTotalHeight += DEF_DETAIL_BETWEEN_HEIGHT;
		
        _DrawDetailItem(dc, rcItem, &lpItem->vecItems[iCount]);
		//
		m_nTotalHeight += DEF_DETAIL_ITEM_HEIGHT;
	}
    
    m_nTotalHeight += DEF_DETAIL_BETWEEN_HEIGHT + 1;

    _DrawLine(dc, CPoint(m_rcClient.left, m_nTotalHeight), CPoint(m_rcClient.right, m_nTotalHeight), RGB(227,237,247), PS_SOLID);

    m_nTotalHeight += 1;
}

void CTrashCleanCtrl::_DrawDetailItem(CDC& dc, CRect& rcItem, LPTrashDetailItem lpDetailItem)
{
	CRect rcDraw(rcItem);
    COLORREF clrText = DEF_TRASH_DETAIL_TEXT_CLR;

	HFONT hFont = KuiFontPool::GetFont(FALSE, FALSE, FALSE);
    
    rcDraw.left = rcItem.left;
    rcDraw.top = m_nTotalHeight;
    rcDraw.bottom = rcDraw.top + DEF_DETAIL_ITEM_HEIGHT;
    rcDraw.right = m_rcClient.right;

    _DrawCheckBox(dc, rcDraw, lpDetailItem->rcCheck, lpDetailItem);

    rcDraw.left = lpDetailItem->rcCheck.right + DEF_CHECK_TO_TEXT - 3;
    rcDraw.top = m_nTotalHeight - 3;
    rcDraw.bottom = rcDraw.top + DEF_DETAIL_ITEM_HEIGHT + 6;
    rcDraw.right = m_rcClient.right - 40;

    ::CopyRect(lpDetailItem->rcSelected, rcDraw);

    if (m_nSelectedIndex == lpDetailItem->nItemId)
    {
        _DrawSelectedBox(dc, rcDraw, lpDetailItem);

        clrText = RGB(0,0,0);
    }
    else if (m_nHoverIndex == lpDetailItem->nItemId)
    {
        _DrawHoverBox(dc, rcDraw, lpDetailItem);
    }

	rcDraw.left = lpDetailItem->rcCheck.right + DEF_CHECK_TO_TEXT;
	rcDraw.top = m_nTotalHeight;
	rcDraw.bottom = rcDraw.top + DEF_DETAIL_ITEM_HEIGHT;
	rcDraw.right = m_rcClient.right;
	_DrawText(dc, rcDraw, lpDetailItem->strName, clrText, hFont);

	lpDetailItem->rcCheck.right = rcDraw.right;
	// 根据状态 绘制 
	_DrawDetailItemStatus(dc, rcDraw, lpDetailItem);
}

void CTrashCleanCtrl::_DrawItemBKGround(CDC& dc, CRect& rcItem, LPTRASH_ITEM lpItem)
{
    CRect rcDraw(rcItem);

    if (lpItem->bShowDetail)
    {
        dc.FillSolidRect(&rcDraw, RGB(245, 250, 254));
    }
    else
    {
        if (m_nHoverIndex == lpItem->uItemId)
        {
            dc.FillSolidRect(&rcDraw, RGB(245, 250, 254));
        }
        // 绘制分割线
        _DrawLine(dc, CPoint(rcItem.left, rcItem.bottom-1), CPoint(rcItem.right, rcItem.bottom-1), RGB(227,237,247), PS_SOLID);
    }
}
void CTrashCleanCtrl::_DrawTextInfor(CDC& dc, CRect& rcItem, LPTRASH_ITEM lpItem)
{
    CRect rcDraw(rcItem);
    HFONT hFont = KuiFontPool::GetFont(TRUE, FALSE, FALSE);
    
    if (lpItem->bShowDetail)
    {
        _DrawText(dc, rcDraw, lpItem->strItemName, DEF_TRASH_DEFAULT_TEXT_CLR, hFont);
        ::CopyRect(lpItem->rcText, rcDraw);
    }
    else
    {
        rcDraw.top += (rcItem.Height() / 2) - (DEF_ICON_HEIGHT / 2);
        rcDraw.bottom = rcItem.top + (rcItem.Height() / 2);
        _DrawText(dc, rcDraw, lpItem->strItemName, DEF_TRASH_DEFAULT_TEXT_CLR, hFont);

        CRect rcBrief(rcItem);
        HFONT briefFont = KuiFontPool::GetFont(FALSE, FALSE, FALSE);
        rcBrief.top += (rcItem.Height() / 2) + 3;
        rcBrief.bottom = rcItem.top + (rcItem.Height() / 2) + (DEF_ICON_HEIGHT / 2);
        _DrawText(dc, rcBrief, lpItem->strBrief, DEF_TRASH_DETAIL_TEXT_CLR, briefFont);
    }
    // 绘制信息
    rcDraw.left = rcDraw.right;
    rcDraw.right = lpItem->rcBox.right;
    _DrawItemStatus(dc, rcDraw, lpItem);
}
void CTrashCleanCtrl::_DrawItemIcon(CDC& dc, CRect& rcItem, LPTRASH_ITEM lpItem)
{
    CRect rcDraw(rcItem);

    rcDraw.top = rcItem.top + (rcItem.Height() / 2 - DEF_ICON_HEIGHT / 2);

	m_iconList.Draw(dc, lpItem->nIconID, rcDraw.left, rcDraw.top, TRANSPARENT);
	lpItem->rcIcon.left = rcDraw.left;
	lpItem->rcIcon.right = rcDraw.left + DEF_ICON_HEIGHT;
	lpItem->rcIcon.top = rcDraw.top;
	lpItem->rcIcon.bottom = rcDraw.top + DEF_ICON_HEIGHT;
}

void CTrashCleanCtrl::_DrawCheckBox(CDC& dc, RECT& rcItem, CRect& rcBox, LPTrashDetailItem lpDetailItem)
{
	int nTop, nLeft;
	BITMAP bmp;
	CDC tempDC;
    int xSrc = 13;
	tempDC.CreateCompatibleDC(dc.m_hDC);

	if (m_bmpCheck == NULL)
		goto Clear0;

	m_bmpCheck.GetBitmap(&bmp);
	SelectObject(tempDC.m_hDC, m_bmpCheck);

	nTop = rcItem.top + (rcItem.bottom - rcItem.top - bmp.bmHeight) / 2;
	nLeft = rcItem.left; // ........................................
	if (rcItem.right < 5 + bmp.bmWidth / 4)
		goto Clear0;

	if (!lpDetailItem->bChecked)
	{
        if (m_nHoverIndex == lpDetailItem->nItemId)
        {
		    dc.BitBlt(nLeft, nTop, bmp.bmWidth / 4, bmp.bmHeight, tempDC.m_hDC, xSrc * 0, 0, SRCCOPY);
        }
        else
        {
            dc.BitBlt(nLeft, nTop, bmp.bmWidth / 4, bmp.bmHeight, tempDC.m_hDC, xSrc * 1, 0, SRCCOPY);
        }
    }
	else
	{
        if (m_nHoverIndex == lpDetailItem->nItemId)
        {
            dc.BitBlt(nLeft, nTop, bmp.bmWidth / 4, bmp.bmHeight, tempDC.m_hDC, xSrc * 2, 0, SRCCOPY);
        }
        else
        {
            dc.BitBlt(nLeft, nTop, bmp.bmWidth / 4, bmp.bmHeight, tempDC.m_hDC, xSrc * 3, 0, SRCCOPY);
        }
	}

	rcBox.left = nLeft;
	rcBox.right = nLeft + bmp.bmWidth / 4;
	rcBox.top = nTop - 2;
	rcBox.bottom = nTop + bmp.bmHeight;

Clear0:
	ReleaseDC(tempDC);
}

void CTrashCleanCtrl::_DrawText(CDC& memDC, CRect& rcItem, 
								LPCTSTR pszText , COLORREF colorText , HFONT hFont)
{
	if (NULL == pszText)
		return;

	CRect rcText = rcItem;

	memDC.SetBkMode(TRANSPARENT);
	HFONT hftOld = memDC.SelectFont(hFont);
	COLORREF crOld = memDC.SetTextColor(colorText);
	DWORD dwFormat = DT_SINGLELINE|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS|DT_VCENTER;

	CSize sizetemp;
	memDC.GetTextExtent(pszText, (int)_tcslen(pszText), &sizetemp);
	if(rcText.right - 20 >= rcText.left + sizetemp.cx)
		rcText.right = rcText.left + sizetemp.cx;
	else 
		rcText.right = rcText.right - 20;


//	rcText.OffsetRect(0,6);
	memDC.DrawText(pszText, (int)_tcslen(pszText), rcText, dwFormat);
	memDC.SelectFont(hftOld);
	memDC.SetTextColor(crOld);

	rcItem = rcText;
}
void CTrashCleanCtrl::_DrawLine(CDC& memDC, CPoint ptStart, CPoint ptEnd, COLORREF color, DWORD dwPenTyple /*=PS_SOLID*/)
{
    HDC hDC = memDC.m_hDC;
    if (NULL == hDC)
        return ;

    HPEN hPen = ::CreatePen(dwPenTyple , 1, color), hOldPen = NULL;

    hOldPen = (HPEN)::SelectObject(hDC, hPen);

    ::MoveToEx(hDC,ptStart.x, ptStart.y, NULL);
    ::LineTo(hDC, ptEnd.x, ptEnd.y);

    ::SelectObject(hDC, hOldPen);

    ::DeleteObject(hPen);
}
void CTrashCleanCtrl::_DrawTextRight(CDC& memDC, CRect& rcItem, 
								LPCTSTR pszText , COLORREF colorText , HFONT hFont)
{
	if (NULL == pszText)
		return;

	CRect rcText = rcItem;

	memDC.SetBkMode(TRANSPARENT);
	HFONT hftOld = memDC.SelectFont(hFont);
	COLORREF crOld = memDC.SetTextColor(colorText);
	DWORD dwFormat =  DT_RIGHT|DT_VCENTER|DT_SINGLELINE;

	CSize sizetemp;
	memDC.GetTextExtent(pszText, (int)_tcslen(pszText), &sizetemp);
	if(rcText.right - 20 >= rcText.left + sizetemp.cx)
		rcText.right = rcText.left + sizetemp.cx;
	else 
		rcText.right = rcText.right - 20;


	//	rcText.OffsetRect(0,6);
	memDC.DrawText(pszText, (int)_tcslen(pszText), rcText, dwFormat);
	memDC.SelectFont(hftOld);
	memDC.SetTextColor(crOld);

	rcItem = rcText;
}

void CTrashCleanCtrl::_DrawShadowLine(CDC& dc, CRect& rcItem)
{
	BITMAP bmp;
	CDC tempDC;

	tempDC.CreateCompatibleDC(dc.m_hDC);

	if (m_bmpShadow == NULL)
		goto Clear0;

	m_bmpShadow.GetBitmap(&bmp);
	SelectObject(tempDC.m_hDC, m_bmpShadow);

	dc.BitBlt(rcItem.left, rcItem.top, bmp.bmWidth, bmp.bmHeight, tempDC.m_hDC, 0, 0, SRCCOPY);
	
	//
//	m_nTotalHeight += bmp.bmHeight;

Clear0:
	ReleaseDC(tempDC);
}

void CTrashCleanCtrl::_DrawShowDetailBar(CDC& dc, CRect& rcItem, CRect& rcBox, LPTRASH_ITEM lpItem)
{
	int nTop, nLeft;
    int xSrc = 20;
	BITMAP bmp;
	CDC tempDC;
    Gdiplus::Graphics graphics(dc);

	tempDC.CreateCompatibleDC(dc.m_hDC);

	if (m_bmpDetail == NULL)
		goto Clear0;

    if (!m_hImageDetailBar)
        goto Clear0;

	m_bmpDetail.GetBitmap(&bmp);
	SelectObject(tempDC.m_hDC, m_bmpDetail);

//	nTop = rcItem.top + (rcItem.bottom - rcItem.top - bmp.bmHeight) / 2;
    nTop = rcItem.top + (rcItem.bottom - rcItem.top - m_hImageDetailBar->GetHeight()) / 2;
	nLeft = rcItem.right - DEF_DETAIL_BAR_TO_RIGHT; // ........................................
	
//     if (rcItem.right < 5 + bmp.bmWidth / 4)
// 		goto Clear0;
    
	if (!lpItem->bShowDetail)
	{
        if (lpItem->uItemId != m_nHoverIndex)
        {
/*            dc.BitBlt(nLeft, nTop, bmp.bmWidth / 4, bmp.bmHeight, tempDC.m_hDC, xSrc * 0, 0, SRCCOPY);*/
            graphics.DrawImage(m_hImageDetailBar, nLeft, nTop, xSrc * 0, 0,  
                            m_hImageDetailBar->GetWidth() / 4, m_hImageDetailBar->GetHeight(), Gdiplus::UnitPixel);
        }
        else
        {
/*		    dc.BitBlt(nLeft, nTop, bmp.bmWidth / 4, bmp.bmHeight, tempDC.m_hDC, xSrc * 1, 0, SRCCOPY);*/
            graphics.DrawImage(m_hImageDetailBar, nLeft, nTop, xSrc * 1, 0,  
                m_hImageDetailBar->GetWidth() / 4, m_hImageDetailBar->GetHeight(), Gdiplus::UnitPixel);
        }
	}
	else
	{
        if (lpItem->uItemId != m_nHoverIndex)
        {
/*            dc.BitBlt(nLeft, nTop, bmp.bmWidth / 4, bmp.bmHeight, tempDC.m_hDC, xSrc * 2, 0, SRCCOPY);*/
            graphics.DrawImage(m_hImageDetailBar, nLeft, nTop, xSrc * 2, 0,  
                m_hImageDetailBar->GetWidth() / 4, m_hImageDetailBar->GetHeight(), Gdiplus::UnitPixel);
        }
        else
        {
/*		    dc.BitBlt(nLeft, nTop, bmp.bmWidth / 4, bmp.bmHeight, tempDC.m_hDC, xSrc * 3, 0, SRCCOPY);*/
            graphics.DrawImage(m_hImageDetailBar, nLeft, nTop, xSrc * 3, 0,  
                m_hImageDetailBar->GetWidth() / 4, m_hImageDetailBar->GetHeight(), Gdiplus::UnitPixel);
        }
	}

	rcBox.left = nLeft;
	rcBox.right = nLeft + bmp.bmWidth / 2;
	rcBox.top = nTop;
	rcBox.bottom = nTop + bmp.bmHeight;

Clear0:
	ReleaseDC(tempDC);
}


void CTrashCleanCtrl::_DrawDetailItemStatus(CDC& dc,CRect& rcDraw,LPTrashDetailItem lpDetailItem)
{
	KAppRes& res = KAppRes::Instance();
	HFONT hFont = KuiFontPool::GetFont(FALSE, FALSE, FALSE);
    COLORREF clrText;

    if (m_nSelectedIndex == lpDetailItem->nItemId)
    {
        clrText = RGB(0,0,0);
    }
    else
    {
        clrText = DEF_TRASH_DETAIL_TEXT_CLR;
    }

	switch (lpDetailItem->nStatus)
	{
	case Empty_Info:
		break;
	case Scaning:
		{
			CString strFormat = res.GetString("IDS_TRASH_SCANING");
            CString strFileNum;
            CString strSize;
            CString str;
            
            GetFileCountText(lpDetailItem->nFileNum, strFileNum);
            GetFileSizeTextString(lpDetailItem->uTotalSize, strSize);
			
            rcDraw.left += 175;
			rcDraw.right = m_rcClient.right;
            str.Format(strFormat, strFileNum, strSize);

			_DrawText(dc, rcDraw, str, clrText, hFont);
		}
		break;
    case Clearing:
        {
            CString strFormat = res.GetString("IDS_TRASH_CLEANING");
            CString strFileNum;
            CString strSize;
            CString str;

            GetFileCountText(lpDetailItem->nFileNum, strFileNum);
            GetFileSizeTextString(lpDetailItem->uTotalSize, strSize);

            rcDraw.left += 175;
            rcDraw.right = m_rcClient.right;
            str.Format(strFormat, strFileNum, strSize);

            _DrawText(dc, rcDraw, str, clrText, hFont);
        }
        break;
	case FinishScan:
		{
// 			if (!lpDetailItem->bChecked)
// 				break;
            if (m_nSelectedIndex == lpDetailItem->nItemId && lpDetailItem->nFileNum > 0)
            {
                CRect rcDetail(rcDraw);
                rcDetail.right = m_rcClient.right - 40;
                _DrawCheckDetail(dc, rcDetail, lpDetailItem);
            }

			CString strFileNum;
			CString strCount;
			GetFileCountText(lpDetailItem->nFileNum, strCount);
			strFileNum.Format(L"%s个", strCount);
			rcDraw.left += 100;
			rcDraw.right = rcDraw.left + 100;
//			_DrawTextRight(dc, rcDraw, strFileNum, DEF_TRASH_DETAIL_TEXT_CLR, hFont);

			COLORREF oldClr = dc.SetTextColor(clrText);
			HFONT oldFont = dc.SelectFont(hFont);
			dc.DrawText(strFileNum, (int)_tcslen(strFileNum), rcDraw,  DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
			rcDraw.left += 70;
			rcDraw.right = rcDraw.left + 100;
			CString strSize;
			GetFileSizeTextString(lpDetailItem->uTotalSize, strSize);
			dc.DrawText(strSize, (int)_tcslen(strSize), rcDraw,  DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
//			_DrawTextRight(dc, rcDraw, strSize, DEF_TRASH_DETAIL_TEXT_CLR, hFont);
			dc.SelectFont(oldFont);
			dc.SetTextColor(oldClr);

		}
		break;
    case FinishClear:
        {
            CString strFinish = res.GetString("IDS_TRASH_FINISH_CLEAR");
            
            rcDraw.left += 175;
            rcDraw.right = m_rcClient.right;
            _DrawText(dc, rcDraw, strFinish, clrText, hFont);
        }
        break;
	case SkipRunning:
		{
			CString strSkip = res.GetString("IDS_TRASH_RUNNING");
			rcDraw.left += 175;
			rcDraw.right = m_rcClient.right;
			_DrawText(dc, rcDraw, strSkip, DEF_TRASH_STATUS_TEXT_CLR, hFont);
		}
		break;
	default:
		break;
	}
}
void CTrashCleanCtrl::_DrawItemStatus(CDC& dc, CRect& rcDraw, LPTRASH_ITEM lpItem)
{
	KAppRes& res = KAppRes::Instance();
	HFONT hFontNormal = KuiFontPool::GetFont(FALSE, FALSE, FALSE);

	switch (lpItem->nStatus)
	{
	case Empty_Info:
		break;
	case WaitScan:
		{
// 			CString str = res.GetString("IDS_WAIT_SCAN");
// 			_DrawText(dc, rcDraw, str, DEF_TRASH_STATUS_TEXT_CLR, hFontNormal);
		}
		break;
	case Scaning:
		{
// 			CString str = res.GetString("IDS_TRASH_SCANING");
// 			_DrawText(dc, rcDraw, str, DEF_TRASH_STATUS_TEXT_CLR, hFontNormal);
		}
		break;
	case FinishScan:
		{
			int iCount;
			ULONGLONG totalSize = 0;
			UINT totalCount = 0;
			int counter = 0;
			CString strStatus;
			for (iCount = 0; iCount < (int)lpItem->vecItems.size(); ++iCount)
			{
				if (lpItem->vecItems[iCount].bChecked && lpItem->vecItems[iCount].nStatus == FinishScan)
				{
					counter ++;
					totalSize += lpItem->vecItems[iCount].uTotalSize;
					totalCount += lpItem->vecItems[iCount].nFileNum;
				}
			}
			
			if (counter == 0)
				break;
			_GetItemStatusString(lpItem->uItemId, strStatus, counter, totalCount, totalSize);
			
			_DrawText(dc, rcDraw, strStatus, DEF_TRASH_STATUS_TEXT_CLR, hFontNormal);
		}
		break;
	default:
		break;
	}
}

void CTrashCleanCtrl::_DrawSelectedBox(CDC& dc, CRect& rcItem, LPTrashDetailItem lpDetailItem)
{
    CPen penBorder;
    CBrush brushBk;
    CRect rcSelect = rcItem;

    penBorder.CreatePen(PS_SOLID, 1, RGB(0x7d, 0xa2, 0xce));
    brushBk.CreateSolidBrush(RGB(0xeb, 0xf4, 0xfd));
    dc.SelectPen(penBorder);
    dc.SelectBrush(brushBk);
    dc.RoundRect(rcSelect, CPoint(5, 5));
    rcSelect.left += 2;
    rcSelect.right -= 2;
    rcSelect.top += 2;
    rcSelect.bottom -= 2;
    DrawGradualColorRect(dc, rcSelect, RGB(0xdd, 0xec, 0xfd), RGB(0xc2, 0xdc, 0xfd));

}

void CTrashCleanCtrl::_DrawHoverBox(CDC& dc, CRect& rcItem, LPTrashDetailItem lpDetailItem)
{
    CPen penBorder;
    CBrush brushBk;
    CRect rcSelect = rcItem;

    penBorder.CreatePen(PS_SOLID, 1, RGB(0xb9, 0xd7, 0xfc));
    brushBk.CreateSolidBrush(RGB(0xff, 0xff, 0xff));
    dc.SelectPen(penBorder);
    dc.SelectBrush(brushBk);
    dc.RoundRect(rcSelect, CPoint(5, 5));
    rcSelect.left += 2;
    rcSelect.right -= 2;
    rcSelect.top += 2;
    rcSelect.bottom -= 2;
    DrawGradualColorRect(dc, rcSelect, RGB(0xfb, 0xfc, 0xff), RGB(0xf2, 0xf8, 0xff));
}

void CTrashCleanCtrl::_DrawCheckDetail(CDC& dc, CRect& rcItem, LPTrashDetailItem lpDetailItem)
{
    HFONT hFont = KuiFontPool::GetFont(FALSE, TRUE, FALSE);
    CRect rcDraw(rcItem);
    COLORREF clrText;

    CString strText = KAppRes::Instance().GetString("IDS_TRASH_CHECK_DETAIL"); // 查看详情

    if (lpDetailItem->bInDetail && m_nHoverIndex == lpDetailItem->nItemId)
    {
       clrText = RGB(255, 23, 23);
    }
    else
    {
       clrText = RGB(77, 112, 150);
    }

    rcDraw.left = rcDraw.right - 70;

    _DrawText(dc, rcDraw, strText, clrText, hFont);

    ::CopyRect(lpDetailItem->rcDetail, rcDraw);

}
//////////////////////////////////////////////////////////////////////////
BOOL CTrashCleanCtrl::_CheckInCheckBox(CPoint& point, LPTRASH_ITEM lpItem)
{
	size_t iCount;
	BOOL bRet = FALSE;
	
	for (iCount = 0; iCount < lpItem->vecItems.size(); ++iCount)
	{
		if (lpItem->vecItems[iCount].rcCheck.PtInRect(point))
		{
			lpItem->vecItems[iCount].bChecked = !lpItem->vecItems[iCount].bChecked;
// 			InvalidateRect(lpItem->vecItems[iCount].rcCheck);
			::SendMessage(m_hNotifyHwnd, WM_SELCETED_CHECKED_CHANGE, NULL, NULL);
            m_nSelectedIndex = lpItem->vecItems[iCount].nItemId;
 			bRet = TRUE;
			goto Clear0;
		}
	}

Clear0:
	return bRet;
}

BOOL CTrashCleanCtrl::_CheckItemAll(LPTRASH_ITEM lpItem)
{
	BOOL bRet = TRUE;
	BOOL bChecked = FALSE;
	int iCount;
	
	for (iCount = 0; iCount < (int)lpItem->vecItems.size(); ++iCount)
	{
		if (lpItem->vecItems[iCount].bChecked == FALSE)
		{
			bChecked = TRUE;
			break;
		}
	}

	for (iCount = 0; iCount < (int)lpItem->vecItems.size(); ++iCount)
	{
		lpItem->vecItems[iCount].bChecked = bChecked;
	}	

//	if (bChecked)
	{
		::SendMessage(m_hNotifyHwnd, WM_SELCETED_CHECKED_CHANGE, NULL, NULL);
	}
	return bRet;
}
BOOL CTrashCleanCtrl::_CheckChildItemAllFinish(LPTRASH_ITEM lpItem)
{
	BOOL bRet = FALSE;
	int iCount;

	for (iCount = 0; iCount < (int)lpItem->vecItems.size(); ++iCount)
	{
		if (lpItem->vecItems[iCount].bChecked)
		{
			if (lpItem->vecItems[iCount].nStatus != FinishScan && 
				lpItem->vecItems[iCount].nStatus != SkipRunning)
			{
				goto Clear0;
			}
		}
	}
	
	bRet = TRUE;
Clear0:
	return bRet;
}
BOOL CTrashCleanCtrl::_CheckChildNoneSelected(LPTRASH_ITEM lpItem)
{
	BOOL bRet = FALSE;
	int iCount;

	for (iCount = 0; iCount < (int)lpItem->vecItems.size(); ++iCount)
	{
		if (lpItem->vecItems[iCount].bChecked)
		{
			goto Clear0;
		}
	}

	bRet = TRUE;
Clear0:
	return bRet;
}
BOOL CTrashCleanCtrl::_CheckInCheckDetail(CPoint& point, LPTRASH_ITEM lpItem)
{
    BOOL bRet = FALSE;
    size_t iCount;

    for (iCount = 0; iCount < lpItem->vecItems.size(); ++iCount)
    {
        if (lpItem->vecItems[iCount].rcDetail.PtInRect(point) && 
            lpItem->vecItems[iCount].nItemId == m_nSelectedIndex &&
            lpItem->vecItems[iCount].nStatus == FinishScan)
        {
            ::SendMessage(m_hNotifyHwnd, WM_TRASH_CHECK_DETAIL, (WPARAM)m_nSelectedIndex, NULL);
            lpItem->vecItems[iCount].bInDetail = FALSE;
            bRet = TRUE;
            goto Clear0;
        }
    }

Clear0:
    return bRet;
}

BOOL CTrashCleanCtrl::_CheckInSelectArea(CPoint& point, LPTRASH_ITEM lpItem)
{
    size_t iCount;
    BOOL bRet = FALSE;

    for (iCount = 0; iCount < lpItem->vecItems.size(); ++iCount)
    {
        if (lpItem->vecItems[iCount].rcSelected.PtInRect(point))
        {
            m_nSelectedIndex = lpItem->vecItems[iCount].nItemId;
            bRet = TRUE;
            goto Clear0;
        }
    }

Clear0:
    return bRet;
}
//////////////////////////////////////////////////////////////////////////
void CTrashCleanCtrl::_GetItemStatusString(UINT nItemID, CString& strStatus, 
										   int counter, UINT totalCount, ULONGLONG totalSize)
{
	KAppRes& res = KAppRes::Instance();
	CString strFormat;
	switch (nItemID)
	{
	case BROWER_TRASH:
		{
			strFormat = res.GetString("IDS_TRASH_BROWER_INFO");
			CString strCount;
			CString strTotalCount;
			CString strTotalSize;
			GetFileCountText(counter, strCount);
			GetFileCountText(totalCount, strTotalCount);
			GetFileSizeTextString(totalSize, strTotalSize);
			strStatus.Format(strFormat, strCount, 
				strTotalCount, strTotalSize);
		}
		break;

	case VIDEO_TRASH:
		{
			strFormat = res.GetString("IDS_TRASH_VEDIO_INFO");
			CString strCount;
			CString strTotalCount;
			CString strTotalSize;
			GetFileCountText(counter, strCount);
			GetFileCountText(totalCount, strTotalCount);
			GetFileSizeTextString(totalSize, strTotalSize);
			strStatus.Format(strFormat, strCount, 
				strTotalCount, strTotalSize);
		}
		break;

	case WIN_TRASH:
	case WIN_HUISHOUZHAN:
	case SYS_TRASH:
	default:
		{
			strFormat = res.GetString("IDS_TRASH_PUBLIC_INFO");
			CString strTotalCount;
			CString strTotalSize;
			GetFileCountText(totalCount, strTotalCount);
			GetFileSizeTextString(totalSize, strTotalSize);
			strStatus.Format(strFormat, 
				strTotalCount, strTotalSize);
		}
		break;
	}
}
void CTrashCleanCtrl::_RangeItems()
{
	int iCount, jCount;
	
//	m_nTotalHeight = DEF_TOP_TO_FIRST;
    m_nTotalHeight = 0;
	for (iCount = 0; iCount < (int)m_vecTrashItems.size(); ++iCount)
	{
		if (!m_vecTrashItems[iCount].bShow)
			continue;
	    if (m_vecTrashItems[iCount].bShowDetail)
        {
		    m_nTotalHeight += DEF_ITEM_THIN_HEIGHT;
        }
        else
        {
            m_nTotalHeight += DEF_ITEM_FAT_HEIGHT;
        }

		if (m_vecTrashItems[iCount].bShowDetail)
		{
            m_nTotalHeight += DEF_BETWEEN_SHADOW;
			for (jCount = 0; jCount < (int)m_vecTrashItems[iCount].vecItems.size(); ++jCount)
			{
                m_nTotalHeight += DEF_DETAIL_BETWEEN_HEIGHT;
				m_nTotalHeight += DEF_DETAIL_ITEM_HEIGHT;
			}
			m_nTotalHeight += DEF_DETAIL_BETWEEN_HEIGHT + 2;
		}
	}

    if (m_nTotalHeight - 3 < m_rcClient.Height())
	{
		m_nTotalHeight = m_rcClient.Height();
	}
	else
	{
		m_nTotalHeight += (int)(m_nTotalHeight * 0.4);
	}

	m_rcRealClient.left = m_rcClient.left;
	m_rcRealClient.top = m_rcClient.top;
	m_rcRealClient.right = m_rcClient.right;
	m_rcRealClient.bottom = m_nTotalHeight;
	
/*	m_nPos = 1;*/
	// 设置滚动条
	SCROLLINFO si;

	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	//GetScrollInfo(SB_VERT, &si);
	si.nMin = 1;
	si.nPos = m_nPos;
	si.nMax = m_nTotalHeight;
	si.nPage = m_rcClient.Height();

	SetScrollInfo(SB_VERT, &si, TRUE);
}
//////////////////////////////////////////////////////////////////////////