//////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "kclearmsg.h"
#include "slimhelper.h"
#include "slimlistctrl.h"
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CSlimListCtrl::CSlimListCtrl(void)
{
    KAppRes& appRes = KAppRes::Instance();
    m_checkBmp = appRes.GetImage("IDB_BITMAP_LISTCTRL_CHECK");

    m_bCursorHand = FALSE;
    m_bInButton = FALSE;
    m_bSelectBtn = FALSE;
}

CSlimListCtrl::~CSlimListCtrl(void)
{
}
//////////////////////////////////////////////////////////////////////////
void CSlimListCtrl::Initialize()
{
    KAppRes& appRes = KAppRes::Instance(); 
    COLORREF clrBK = RGB(255, 255, 255);

    // Icon List
	m_iconList.Create(32, 32, ILC_COLOR32 | ILC_MASK, -1,0);
    m_diableIconList.Create(32, 32, ILC_COLOR24 | ILC_MASK, -1,0);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_DRIVER"), clrBK);				// 0
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_DRIVER_DIS"), clrBK);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_HELP"), clrBK);				// 1
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_HELP_DIS"), clrBK);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_INSTALL_UPDATE"), clrBK);		// 2
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_INSTALL_UPDATE_DIS"), clrBK);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_JAPAN"), clrBK);				// 3
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_JAPAN_DIS"), clrBK);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_KOREAN"), clrBK);				// 4
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_KOREAN_DIS"), clrBK);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_MICR"), clrBK);				// 5
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_MICR_DIS"), clrBK);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_MUSIC"), clrBK);				// 6
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_MUSIC_DIS"), clrBK);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_PICTURE"), clrBK);				// 7
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_PICTURE_DIS"), clrBK);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_TRAD"), clrBK);				// 8
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_TRAD_DIS"), clrBK);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_TV"), clrBK);				// 9
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_TV_DIS"), clrBK);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_VADIO"), clrBK);				// 10
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_VADIO_DIS"), clrBK);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_WALL"), clrBK);				// 11
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_WALL_DIS"), clrBK);

    m_iconList.Add(appRes.GetImage("IDB_SLIM_WIN_UPDATE"), clrBK);				// 12
    m_diableIconList.Add(appRes.GetImage("IDB_SLIM_WIN_UPDATE_DIS"), clrBK);
 
    //
    m_wndToolTip.Create(m_hWnd);
    m_wndToolTip.SetMaxTipWidth(400);
    CToolInfo ti(0, m_hWnd);
    m_wndToolTip.AddTool(&ti);
    m_wndToolTip.Activate(FALSE);
    m_wndToolTip.SetDelayTime(TTDT_AUTOPOP, 10000);
    m_bToolTipActive = FALSE;
    m_nHoverIndex = -1;
    m_nSelectedIndex = -1;

    ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
    m_hdr.SubclassWindow(GetHeader());
    m_hdr.SetDlgCtrlID(1000);
    m_hdr.EnableWindow(FALSE);

}

BOOL CSlimListCtrl::SubclassWindow(HWND hWnd)
{
    BOOL bRet = FALSE;
    bRet = CWindowImpl<CSlimListCtrl, CListViewCtrl>::SubclassWindow(hWnd);

    return bRet;   
}
void CSlimListCtrl::SetNotifyHwnd(HWND hWnd)
{
    m_hNotifyHwnd = hWnd;
}
void CSlimListCtrl::DeleteItem(int nItem)
{
    
}
void CSlimListCtrl::DeleteItem(LPDELETEITEMSTRUCT lpdis)
{

}
BOOL CSlimListCtrl::RemoveAll()
{
    return FALSE;
}

void CSlimListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight = ITEM_ROW_HEIGHT;
}

void CSlimListCtrl::SetEnable(BOOL bEnable)
{
    m_bEnable = bEnable;
}

void CSlimListCtrl::SetItemStatus(int nID, BOOL bEnable, ULONGLONG uSizeAfterSlim, ULONGLONG uSizeOnDisk, BOOL bInit)
{
    int iCount;

    for (iCount = 0; iCount < (int)m_vSlimItems.size(); ++iCount)
    {
        if (m_vSlimItems[iCount].nItemID == nID)
        {
            KAutoLock lock(m_lock);
            CRect rect;
            GetItemRect(iCount, rect, LVIR_BOUNDS);
            m_vSlimItems[iCount].bInit = bInit;
            m_vSlimItems[iCount].uSizeDisk = uSizeOnDisk;
            m_vSlimItems[iCount].uSizeSlim = uSizeAfterSlim;
            m_vSlimItems[iCount].bEnable = bEnable;
            
            InvalidateRect(rect);
            break;
        }
    }

    
}

//////////////////////////////////////////////////////////////////////////
void CSlimListCtrl::AddItem(SLIM_DRAW_ITEM& item)
{
    int nCount = 0;
    if (!item.strName.IsEmpty())
    {
        KAutoLock lock(m_lock);
        item.nIconIndex = _GetIconIndexByID(item.nItemID);
        m_vSlimItems.push_back(item);
    }
    nCount = (int)m_vSlimItems.size();
    SetItemCountEx(nCount, LVSICF_NOINVALIDATEALL);
}
//////////////////////////////////////////////////////////////////////////
void CSlimListCtrl::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
    CDCHandle dc;
    UINT nItem = lpdis->itemID;
    SLIM_DRAW_ITEM currentItem;
    CRect rcDraw(lpdis->rcItem);
    BOOL bSelected;

    dc = lpdis->hDC;
    
    bSelected = lpdis->itemState & ODS_SELECTED;
    
    if (!_GetItemData(nItem, currentItem))
    {
        goto Clear0; //无效的行，返回
    }

    dc.SetBkMode(TRANSPARENT);

    _DrawItemBKGround(dc, nItem, bSelected);
    
//    _DrawCheckBox(dc, nItem);

    _DrawIcon(dc, nItem);

    _DrawItemText(dc, nItem);

    _DrawItemSize(dc, nItem);

    _DrawItemPath(dc, nItem);
    
    _DrawItemTreat(dc, nItem);

Clear0:
    dc.Detach();
    return;
}

void CSlimListCtrl::_DrawItemText(CDCHandle& dc, int nItem)
{
    CRect rcItem;
    CRect rcDraw;
    SLIM_DRAW_ITEM item;
    HFONT nameFont = KuiFontPool::GetFont(TRUE, FALSE, FALSE);
    HFONT briefFont = KuiFontPool::GetFont(FALSE, FALSE, FALSE);
    COLORREF clrName;
    COLORREF clrBrief;

    if (!_GetItemData(nItem, item))
        return;

    GetItemRect(nItem, &rcItem, LVIR_BOUNDS);
    ::CopyRect(rcDraw, rcItem);

    rcDraw.left = 70;
    rcDraw.top += 8;
    rcDraw.bottom = rcDraw.top + 18;
    
    if (item.bEnable && m_bEnable)
    {
        clrName = RGB(0,0,0);
        clrBrief  = RGB(109,109,109);
    }
    else
    {
        clrName = RGB(129,129,129);
        clrBrief = RGB(186,186,186);
    }
    {
        KAutoLock lock(m_lock);
        LV_COLUMN lvc;
        lvc.mask = LVCF_FMT | LVCF_WIDTH |LVCF_SUBITEM;
        GetColumn(0, &lvc);

        CopyRect(m_vSlimItems[nItem].rcCheck, rcItem);
        m_vSlimItems[nItem].rcCheck.right = lvc.cx;
    }
    
    _DrawText(dc, rcDraw, item.strName, clrName, nameFont);

    rcDraw.right = m_vSlimItems[nItem].rcCheck.right - 10;
    rcDraw.top = rcItem.top;
    rcDraw.top += (rcItem.bottom - rcItem.top) / 2;
    rcDraw.bottom = rcDraw.top + 18;

    _DrawText(dc, rcDraw, item.strDescription, clrBrief, briefFont);
   
}
void CSlimListCtrl::_DrawItemBKGround(CDCHandle& dc, int nItem, BOOL bSelected)
{
    CRect rcItem;
    if (nItem < 0)
        return;
    
    GetItemRect(nItem, &rcItem, LVIR_BOUNDS);
    rcItem.right = GetColumnWidth(0) + GetColumnWidth(1) + GetColumnWidth(2) + GetColumnWidth(3);

    if (bSelected)
    {
        dc.FillSolidRect(&rcItem, RGB(245,250,254));
    }
    else
    {
        dc.FillSolidRect(&rcItem, RGB(255,255,255));
    }

    // 绘制分割线
    _DrawLine(dc, CPoint(rcItem.left, rcItem.bottom-1), CPoint(rcItem.right, rcItem.bottom-1), RGB(219,219,219), PS_SOLID);

}
void CSlimListCtrl::_DrawCheckBox(CDCHandle& dc, int nItem)
{
    int nTop, nLeft;
    BITMAP bmp;
    CDC memDC;
    CRect rcBox;
    CRect rcItem;
    SLIM_DRAW_ITEM item;
    
    memDC.CreateCompatibleDC(dc.m_hDC);

    if (!_GetItemData(nItem, item))
        goto Clear0;

    GetItemRect(nItem, rcItem, LVIR_BOUNDS);

    if (m_checkBmp == NULL)
        goto Clear0;

    m_checkBmp.GetBitmap(&bmp);
    SelectObject(memDC.m_hDC, m_checkBmp);

    nTop = rcItem.top + (rcItem.bottom - rcItem.top - bmp.bmHeight) / 2;

    if (rcItem.right < 5 + bmp.bmWidth / 2)
        goto Clear0;

    nLeft = 12;

    if (item.bChecked)
    {
        dc.BitBlt(nLeft, nTop, bmp.bmWidth / 2, bmp.bmHeight, memDC.m_hDC, 0, 0, SRCCOPY);
    }
    else
    {
        dc.BitBlt(nLeft, nTop, bmp.bmWidth / 2, bmp.bmHeight, memDC.m_hDC, 13, 0, SRCCOPY);
    }

    rcBox.left = nLeft;
    rcBox.right = nLeft + bmp.bmWidth / 2;
    rcBox.top = nTop - 2;
    rcBox.bottom = nTop + bmp.bmHeight;
    
    item.rcCheck;
Clear0:
//    ReleaseDC(memDC);
    memDC.DeleteDC();
}
void CSlimListCtrl::_DrawIcon(CDCHandle& dc, int nItem)
{
    CRect rcItem;
    int nTop;
    int nLeft;
    SLIM_DRAW_ITEM item;

    if (!_GetItemData(nItem, item))
        return;

    if (item.nIconIndex == -1)
        return;

    GetItemRect(nItem, &rcItem, LVIR_BOUNDS);

    nTop = rcItem.top + (rcItem.Height() / 2 - 32 / 2);
    nLeft = (70 / 2 - 32/2);
    
    if (item.bEnable && m_bEnable)
    {
        m_iconList.Draw(dc, item.nIconIndex, nLeft, nTop, TRANSPARENT);
    }
    else
    {
        m_diableIconList.Draw(dc, item.nIconIndex, nLeft, nTop, TRANSPARENT);
    }

}
void CSlimListCtrl::_DrawItemSize(CDCHandle& dc, int nItem)
{
    CRect rcItem;
    SLIM_DRAW_ITEM item;
    CString strDiskSize;
    CString strSlimSize;
    COLORREF clrText;
    CRect rcDraw;

    if (!_GetItemData(nItem, item))
        return;

    GetItemRect(nItem, &rcItem, LVIR_BOUNDS);

    if (item.bInit)
    {
        // 大小
//        if (item.uSizeDisk >= 1* 1024 * 1024)
        {
            slimhelper::GetFileSizeTextString(item.uSizeDisk, strDiskSize);
            slimhelper::GetFileSizeTextString(item.uSizeSlim, strSlimSize);
        }
    }
    else
    {
        // 计算中
        strDiskSize = L"计算中";
        strSlimSize = L"计算中";
    }

    if (item.bEnable && m_bEnable)
    {
        clrText = RGB(0,0,0);
    }
    else
    {
        clrText = RGB(186,186,186);
    }

    CopyRect(rcDraw, rcItem);
    LV_COLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH |LVCF_SUBITEM;
    for (int nCol = 0; GetColumn(nCol, &lvc); ++ nCol)
    {
        if (nCol != 0)
            rcDraw.left = rcDraw.right;
        
        rcDraw.right = rcDraw.left + lvc.cx;

        if (nCol == 1)
        {
            rcDraw.right -= 60;
            COLORREF clrOld = dc.SetTextColor(clrText);
            DrawText(dc, strDiskSize,(int) wcslen(strDiskSize), rcDraw, DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
            dc.SetTextColor(clrOld);
        }
//         if (nCol == 2)
//         {
//             COLORREF clrOld = dc.SetTextColor(clrText);
//             DrawText(dc, strSlimSize,(int) wcslen(strSlimSize), rcDraw, DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
//             dc.SetTextColor(clrOld);
//             break;
//         }
    }

}
void CSlimListCtrl::_DrawItemTreat(CDCHandle& dc, int nItem)
{
    CRect rcItem;
    CRect rcDraw;
    SLIM_DRAW_ITEM item;
    CString strText;
    COLORREF clrText;
    COLORREF clrTop;
    COLORREF clrDown;
    COLORREF oldcolor;
    COLORREF oldPen;
    HBRUSH btnBr;

    if (!_GetItemData(nItem, item))
        return;

    GetItemRect(nItem, &rcItem, LVIR_BOUNDS);
    rcItem.right = GetColumnWidth(0) + 
                   GetColumnWidth(1) + 
                   GetColumnWidth(2) + 
                   GetColumnWidth(3);

    ::CopyRect(rcDraw, rcItem);

    rcDraw.left = rcItem.right - 90;
    rcDraw.right = rcItem.right - 20;
    rcDraw.top = rcItem.top + 15;
    rcDraw.bottom = rcItem.bottom - 15;

    SelectObject(dc.m_hDC, GetStockObject(DC_PEN));
    SelectObject(dc.m_hDC, GetStockObject(DC_BRUSH));
    oldcolor = dc.SetDCBrushColor(RGB(255, 255,255));
    oldPen = dc.SetDCPenColor(RGB(125, 158, 188));
    btnBr = CreateSolidBrush(RGB(125, 158, 188));

    CRect rcBorder(rcDraw.left-1, rcDraw.top - 1, rcDraw.right + 1, rcDraw.bottom + 1);

    dc.FillRect(rcBorder, btnBr);

    dc.SetDCBrushColor(oldcolor);
    dc.SetDCPenColor(oldPen);
    
    ::DeleteObject(btnBr);

    if (nItem == m_nSelectedIndex && m_bSelectBtn && item.bEnable)
    {
        clrTop = RGB(198, 226, 253);
        clrDown = RGB(251, 252, 253);
    }
    else if (nItem == m_nHoverIndex && m_bInButton && item.bEnable)
    {
        clrTop = RGB(254, 254, 254);
        clrDown = RGB(238, 251, 254);
    }
    else
    {
        clrTop = RGB(254,254,254);
        clrDown = RGB(221,239,251);
    }
    DrawGradualColorRect(dc, rcDraw, clrTop, clrDown);

    CopyRect(m_rcButton, rcDraw);
    CRect rcHdr;
    m_hdr.GetClientRect(&rcHdr);
    m_rcButton.top = (rcDraw.top - rcHdr.Height()) % ITEM_ROW_HEIGHT;
    m_rcButton.bottom = (rcDraw.bottom - rcHdr.Height()) % ITEM_ROW_HEIGHT;
    
    {
        KAutoLock lock(m_lock);
        ::CopyRect(m_vSlimItems[nItem].rcButton, rcDraw);
    }

    if (item.bEnable && m_bEnable)
    {
        clrText = RGB(0,0,0);
    }
    else
    {
        clrText = RGB(111,111,113);
    }   
    
    if (item.nTreatment == SLIM_DELETE_FILE)
    {
        if (item.bEnable)
        {
            strText = L"删 除";
        }
        else
        {
            strText = L"已删除";
            rcDraw.left -= 5;
        }
    }
    else if (item.nTreatment == SLIM_COMPRESS_FILE)
    {
        if (item.bEnable)
        {
            strText = L"压 缩";
        }
        else
        {
            strText = L"已压缩";
            rcDraw.left -= 5;
        }
    }
    
    rcDraw.left += 20;

    _DrawText(dc, rcDraw, strText, clrText, KuiFontPool::GetFont(FALSE,FALSE,FALSE));
    // 根据类型绘制文字 删除/已删除  压缩/已压缩
    //_DrawText(dc, rcDraw, L"操作", RGB(0,0,0), KuiFontPool::GetFont(FALSE,FALSE,FALSE));    



}
void CSlimListCtrl::_DrawItemPath(CDCHandle& dc, int nItem)
{
    CRect rcItem;
    CRect rcDraw;
    SLIM_DRAW_ITEM item;
    COLORREF clrText;

    if (!_GetItemData(nItem, item))
        return;

    GetItemRect(nItem, &rcItem, LVIR_BOUNDS);

    if (item.bEnable && m_bEnable)
    {
        clrText = RGB(79,123,150);
    }
    else
    {
        clrText = RGB(186,186,186);
    }

    CopyRect(rcDraw, rcItem);
    LV_COLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH |LVCF_SUBITEM;
    for (int nCol = 0; GetColumn(nCol, &lvc); ++ nCol)
    {
        if (nCol != 0)
            rcDraw.left = rcDraw.right;

        rcDraw.right = rcDraw.left + lvc.cx;
        if (nCol == 2)
        {
            HFONT hFont = KuiFontPool::GetFont(FALSE, TRUE, FALSE);
            rcDraw.left += 30;
            CString strText = L"定位";
            _DrawText(dc, rcDraw, strText, clrText, hFont);
            
            {
                KAutoLock lock(m_lock);
                CRect rcHdr;
                m_hdr.GetClientRect(rcHdr);
                CopyRect(m_rcLink, rcDraw);
                m_rcLink.top = (rcDraw.top - rcHdr.Height() + 5) % ITEM_ROW_HEIGHT;
                m_rcLink.bottom = (rcDraw.bottom - rcHdr.Height() - 5) % ITEM_ROW_HEIGHT;
                CopyRect(m_vSlimItems[nItem].rcLink, rcDraw);
            }
        }
    }


}
void CSlimListCtrl::_DrawText(CDCHandle& dc, CRect& rcItem, LPCTSTR pszText , COLORREF colorText , HFONT hFont)
{
    if (NULL == pszText)
        return;

    CRect rcText = rcItem;

    HFONT hftOld = dc.SelectFont(hFont);
    COLORREF crOld = dc.SetTextColor(colorText);
    DWORD dwFormat = DT_SINGLELINE|DT_LEFT|DT_NOPREFIX|DT_END_ELLIPSIS|DT_VCENTER;

    CSize sizetemp;
    dc.GetTextExtent(pszText, (int)_tcslen(pszText), &sizetemp);
    if(rcText.right - 5 >= rcText.left + sizetemp.cx)
        rcText.right = rcText.left + sizetemp.cx;
    else 
        rcText.right = rcText.right - 5;


    //	rcText.OffsetRect(0,6);
    dc.DrawText(pszText, (int)_tcslen(pszText), rcText, dwFormat);
    dc.SelectFont(hftOld);
    dc.SetTextColor(crOld);

    rcItem = rcText;
}
void CSlimListCtrl::_DrawLine(CDCHandle& memDC, CPoint ptStart, CPoint ptEnd, COLORREF color, DWORD dwPenTyple /*=PS_SOLID*/)
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
//////////////////////////////////////////////////////////////////////////
BOOL CSlimListCtrl::OnEraseBkgnd(CDCHandle dc)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    dc.FillSolidRect(rcClient, RGB(255,255,255));
    return TRUE;
}
void CSlimListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
    int nItem = _HitTest(point);
    SLIM_DRAW_ITEM item;
    CRect rcHeader;

    m_hdr.GetClientRect(&rcHeader);

    if (!m_bEnable)
        goto Clear0;

    m_nSelectedIndex = nItem;
    
    if (!_GetItemData(nItem, item))
        goto Clear0;

    if (!item.bEnable)
        goto Clear0;

    point.y = (point.y - rcHeader.Height()) % ITEM_ROW_HEIGHT;

    if (m_rcButton.PtInRect(point))
    {
//         if (!m_bSelectBtn)
//         {
//             InvalidateRect(item.rcButton);
//         }
        m_bSelectBtn = TRUE;
        ::PostMessage(m_hNotifyHwnd, SLIM_WM_ITEM_TREAT, nItem, NULL);
    }
    else
    {
        m_bSelectBtn = FALSE;
    } 
    
    if (m_rcLink.PtInRect(point))
    {
        ::PostMessage(m_hNotifyHwnd, SLIM_WM_ITEM_LINK, nItem, NULL);
    }
Clear0:
    SetMsgHandled(FALSE);
}
void CSlimListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
    int nItem = _HitTest(point);
    SLIM_DRAW_ITEM slimItem;
    BOOL bIn = FALSE;
    CRect rcHdr;

    SetFocus();
    m_nHoverIndex = nItem;

    if (!_GetItemData(nItem, slimItem))
        goto Clear0;

    if (!slimItem.rcButton.PtInRect(point) && m_bSelectBtn)
    {
        m_bSelectBtn = FALSE;
        Invalidate();
    }

    if (!slimItem.bEnable)
        goto Clear0;

    if (slimItem.rcCheck.PtInRect(point))
    {
        bIn = TRUE;
        CString des = slimItem.strDescription;
        if (m_strTip.CompareNoCase(des) != 0 || !m_bToolTipActive)
        {
            m_bToolTipActive = TRUE;
            m_strTip = des;
            m_wndToolTip.Activate(FALSE);
            m_wndToolTip.UpdateTipText((_U_STRINGorID)des, m_hWnd);
            m_wndToolTip.Activate(TRUE);	
         }
    }
    m_hdr.GetClientRect(&rcHdr);
    point.y = (point.y - rcHdr.Height()) % ITEM_ROW_HEIGHT;

    if (m_rcLink.PtInRect(point))
    {
        m_bCursorHand = TRUE;
        ::SetCursor(::LoadCursor(NULL,   MAKEINTRESOURCE(IDC_HAND)));  
    }
    else if (m_rcButton.PtInRect(point))
    {
        if (!m_bInButton)
        {
            InvalidateRect(slimItem.rcButton);
        }
        ::SetCursor(::LoadCursor(NULL,   MAKEINTRESOURCE(IDC_HAND)));
        m_bCursorHand = TRUE;
        m_bInButton = TRUE;
    }
    else
    {
        if (m_bCursorHand || m_bInButton)
        {
            m_bCursorHand = FALSE;
            m_bInButton = FALSE;
            InvalidateRect(slimItem.rcButton);
            ::SetCursor(::LoadCursor(NULL,   MAKEINTRESOURCE(IDC_ARROW))); 
        }
    }

Clear0:
    if (!bIn)
    {
        m_wndToolTip.Activate(FALSE);
        m_bToolTipActive = FALSE;
    }
    SetMsgHandled(FALSE);
}
void CSlimListCtrl::OnSize(UINT nType, CSize size)
{
    CRect rcClient;
    int nWidth;

    GetClientRect(&rcClient);
    
    nWidth = rcClient.right - GetColumnWidth(1) - GetColumnWidth(2) - GetColumnWidth(3);

    if (nWidth - 1 > 0)
    {
        SetColumnWidth(0, nWidth - 1);

        Invalidate();
    }
    
    // 设置横向滚动条不显示
    ShowScrollBar(SB_HORZ, FALSE);
    EnableScrollBar(SB_BOTH, ESB_DISABLE_DOWN);
}
LRESULT CSlimListCtrl::OnBeginTrack(WPARAM wParam, LPNMHDR lParam, BOOL& bHandled)
{
    // 表头不可以拖动
    bHandled = TRUE;
    return TRUE;
}
LRESULT CSlimListCtrl::OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

int CSlimListCtrl::_HitTest(CPoint pt)
{
    LVHITTESTINFO hitinfo;
    hitinfo.pt = pt;
    int nItem = __super::HitTest(&hitinfo); 

    return nItem;
}
BOOL CSlimListCtrl::_GetItemData(UINT nItem, SLIM_DRAW_ITEM& item)
{
    BOOL bRet = FALSE;
    KAutoLock lock(m_lock);
    if (nItem < 0 || (int)m_vSlimItems.size() <= nItem)
        goto Clear0;

    item = m_vSlimItems[nItem];

    bRet = TRUE;
Clear0:
    return bRet;
}
int CSlimListCtrl::_GetIconIndexByID(int nID)
{
    int nIndex = -1;   

    switch (nID)
    {
    case 0: // 壁纸
        nIndex = 11;
        break;
    case 1: // 示例电视录制文件
        nIndex = 9;
        break;
    case 2: // 示例音乐
        nIndex = 6;
        break;
    case 3: // 示例图片
        nIndex = 7;
        break;
    case 4: // 示例视频
        nIndex = 10;
        break;
    case 5: // 日文输入法
        nIndex = 3;
        break;
    case 6: // 韩文输入法
        nIndex = 4;
        break;
    case 7: // 繁体中文输入法
        nIndex = 8;
        break;
    case 8: // 微软拼音输入法
        nIndex = 5;
        break;
    case 9:// 帮助文件
        nIndex = 1;
        break;
    case 10:// 系统升级补丁备份
        nIndex = 12;
        break;
    case 11:// 软件安装补丁备份
        nIndex = 2;
        break;
    case 12:// 系统驱动备份
        nIndex = 0;
        break;
    default:
        break;
    }
    return nIndex;
}
//////////////////////////////////////////////////////////////////////////