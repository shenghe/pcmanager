//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "trashpublicfunc.h"
#include "ktrashskipctrl.h"
#include "kclearmsg.h"
#include "kscgui/kdraw.h"

//////////////////////////////////////////////////////////////////////////

CTrashSkipCtrl::CTrashSkipCtrl(void)
{
    m_nHoverIndex = -1;
    m_nSelectIndex = -1;
    m_nPos = 0;
    m_nHeight = 0;
    m_nCountdownNum = -1;
}

CTrashSkipCtrl::~CTrashSkipCtrl(void)
{
    DestroyIcon(m_iconIE);
    DestroyIcon(m_iconUnknow);
//    DestroyObject(m_bmpHeadBack);
}

void CTrashSkipCtrl::Initialize(HWND hWnd)
{
    KAppRes& res = KAppRes::Instance();

    m_hNotifyHwnd = hWnd;
    
    m_iconIE = res.GetIcon("IDI_BROWER_IE");
    m_iconUnknow = res.GetIcon("IDI_UNKNOW_BROWER");
    m_bmpHeadBack = res.GetImage("IDB_SKIP_HEAD_BACK");
    m_imgSoftOff = res.GetImagePlus("IDP_SKIP_SOFT_OFF");
}

BOOL CTrashSkipCtrl::SubclassWindow(HWND hWnd)
{
    BOOL bRet = FALSE;

    bRet = CWindowImpl<CTrashSkipCtrl>::SubclassWindow(hWnd);
    GetClientRect(&m_rcClient);
    return bRet;
}
//////////////////////////////////////////////////////////////////////////
BOOL CTrashSkipCtrl::AddItem(TRASH_SKIP_ITEM item)
{
    BOOL bRet = FALSE;
    std::vector<TRASH_SKIP_ITEM>::iterator ite;

    if (item.strIconPath.IsEmpty() && !item.strProcess.IsEmpty())
    {
        GetProcessPathFromName(item.strProcess, item.strIconPath);
    }

    m_vecItems.push_back(item);

    _RandItems();
    return bRet = TRUE;
}
BOOL CTrashSkipCtrl::AddItem(UINT id, CString& strName, CString& strProcess, ULONGLONG uSize)
{
    TRASH_SKIP_ITEM item;
    item.nItemID = id;
    item.strName = strName;
    item.strProcess = strProcess;
    item.uSize = uSize;

//    GetProcessNamesByID(id, vecProName);

    if (!strProcess.IsEmpty())
    {
        GetProcessPathFromName(strProcess, item.strIconPath);
    }

    m_vecItems.push_back(item);
    
    _RandItems();
    return TRUE;
}

void CTrashSkipCtrl::Clear()
{
    m_vecItems.clear();   
}
size_t CTrashSkipCtrl::GetSize()
{
    return m_vecItems.size();
}

BOOL CTrashSkipCtrl::GetItemByProcessName(CString& strProcess, LPTRASH_SKIP_ITEM& lpItem)
{
    BOOL bRet = FALSE;
    size_t iCount;

    if (strProcess.IsEmpty())
        goto Clear0;

    for (iCount = 0; iCount < m_vecItems.size(); ++iCount)
    {
        if (m_vecItems[iCount].strProcess.CompareNoCase(strProcess) == 0)
        {
            lpItem = &m_vecItems[iCount];
            bRet = TRUE;     
            goto Clear0;
        }
    }
Clear0:
    return bRet;
}

BOOL CTrashSkipCtrl::RemoveItemByProcess(LPCTSTR szProcess)
{
    BOOL bRet = FALSE;
    std::vector<TRASH_SKIP_ITEM>::iterator ite;
    
    if (szProcess == NULL)
        goto Clear0;

    for (ite = m_vecItems.begin(); ite != m_vecItems.end(); ++ite)
    {
        TRASH_SKIP_ITEM skipItem = (*ite);
        if (skipItem.strProcess.CompareNoCase(szProcess) == 0)
        {
            m_vecItems.erase(ite);
            bRet = TRUE;     
            goto Clear0;
        }
    }
Clear0:
    _RandItems();
    return bRet;
}

void CTrashSkipCtrl::StartCountdown()
{
    m_nCountdownNum = 3;
    
    InvalidateRect(NULL);

    SetTimer(DEF_TRASH_SKIP_RESCAN_EVENT, 1000, NULL);
}

void CTrashSkipCtrl::StopCountdown()
{
    KillTimer(DEF_TRASH_SKIP_RESCAN_EVENT);
    m_nCountdownNum = -1;
}
//////////////////////////////////////////////////////////////////////////
void CTrashSkipCtrl::OnPaint(CDCHandle dcHandle)
{
    CPaintDC dc(m_hWnd);
    CBitmap memBmp;
    CDC memDC;
    CRect rcDraw(m_rcRealClient);
    
    memDC.CreateCompatibleDC(dc.m_hDC);
    memBmp.CreateCompatibleBitmap(dc.m_hDC, rcDraw.Width(), rcDraw.Height());
    memDC.SetBkMode(TRANSPARENT);
    SelectObject(memDC, memBmp);
    
    memDC.FillSolidRect(rcDraw, RGB(0xff, 0xff, 0xff));

    _OnRealPaint(memDC);

    m_nHeight = int((m_rcRealClient.Height() - m_rcClient.Height()) * (m_nPos * 1.0) / (m_rcRealClient.Height()));

    dc.BitBlt(0, 0, m_rcClient.Width(), m_rcClient.Height(), memDC.m_hDC, 0, m_nHeight, SRCCOPY);

    ReleaseDC(memDC);
}
LRESULT CTrashSkipCtrl::OnEarseBKGND(CDCHandle dc)
{
    return TRUE;
}
void CTrashSkipCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
    size_t iCount;

    for (iCount = 0; iCount < m_vecItems.size(); ++iCount)
    {
        if (m_vecItems[iCount].rcItem.PtInRect(point))
        {
            m_nHoverIndex = (int)iCount;
            InvalidateRect(NULL);
            break;
        }
    }

    if (iCount == m_vecItems.size() && iCount != 0)
    {
        if (m_nHoverIndex != -1)
            InvalidateRect(NULL);
        m_nHoverIndex = -1;
    }
}
LRESULT CTrashSkipCtrl::OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
{
    CPoint pt;
    BOOL bLink = FALSE;
    GetCursorPos( &pt );
    ScreenToClient( &pt );

    bLink = _PointInClose(pt);
    ::SetCursor(::LoadCursor(NULL, bLink?IDC_HAND:IDC_ARROW));

    return TRUE;
}
void CTrashSkipCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
    size_t iItem = 0;
    
    for (iItem = 0; iItem < m_vecItems.size(); ++iItem)
    {
        if (m_vecItems[iItem].rcItem.PtInRect(point))
        {
            m_nSelectIndex = (int)iItem;
            InvalidateRect(NULL);
            break;
        }
    }

    if (iItem == m_vecItems.size() && iItem != 0)
    {
        if (m_nSelectIndex != -1)
            InvalidateRect(NULL);
        m_nSelectIndex = -1;
    }
}

void CTrashSkipCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
    size_t iItem;
    for (iItem = 0; iItem < m_vecItems.size(); ++iItem)
    {
        if (m_vecItems[iItem].rcItem.PtInRect(point) &&
            m_nSelectIndex == (int)iItem)
        {
            SendMessage(m_hNotifyHwnd, WM_TRASH_CLOSE_TASK,
            (WPARAM)m_vecItems[iItem].nItemID, (LPARAM)m_vecItems[iItem].strProcess.GetBuffer());
            break;
        }
    }
    m_nSelectIndex = -1;   
    if (iItem == m_vecItems.size() && iItem != 0)
        InvalidateRect(NULL);
}
void CTrashSkipCtrl::OnSize(UINT nType, CSize size)
{
    GetClientRect(&m_rcClient);
    _RandItems();
}

void CTrashSkipCtrl::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == DEF_TRASH_SKIP_RESCAN_EVENT)
    {            
        m_nCountdownNum --;
        if (m_nCountdownNum < 0)
        {
            //KillTimer(DEF_TRASH_SKIP_RESCAN_EVENT);
            m_nCountdownNum = -1;
            ::PostMessage(m_hNotifyHwnd, WM_TRASH_RESCAN,NULL, NULL);
        }
        else
        {
            InvalidateRect(NULL);
        }
    }
    SetMsgHandled(FALSE);
}

LRESULT CTrashSkipCtrl::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

LRESULT CTrashSkipCtrl::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam)
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


//////////////////////////////////////////////////////////////////////////
BOOL CTrashSkipCtrl::_GetItemIcon(LPCTSTR lpFileName, HICON& icon)
{
    BOOL bRet = FALSE;
    KAppRes& res = KAppRes::Instance();

    if (lpFileName == NULL)
        goto Clear0;

    icon = ExtractIcon(NULL, lpFileName, 0);

Clear0:
    return bRet;
}
//////////////////////////////////////////////////////////////////////////
void CTrashSkipCtrl::_OnRealPaint(CDC& memDC)
{
    CRect rcDraw(m_rcClient);
    
    _DrawHeader(memDC, rcDraw);

    rcDraw.top = DEF_HEAD_HEIGHT + DEF_VERT_SPACE;
    rcDraw.bottom = rcDraw.top + DEF_ITEM_HEIGHT;
    rcDraw.left = DEF_LEFT_SPACE - (DEF_HORI_SPACE + DEF_ITEM_WIDTH);
    rcDraw.right = rcDraw.left + DEF_ITEM_WIDTH;

    for (size_t iCount = 0; iCount < m_vecItems.size(); ++iCount)
    {
        if ((rcDraw.right + DEF_RIGHT_SPACE + DEF_ITEM_WIDTH + DEF_HORI_SPACE) <= m_rcClient.right)
        {
            rcDraw.left += (DEF_HORI_SPACE + DEF_ITEM_WIDTH);
            rcDraw.right = rcDraw.left + DEF_ITEM_WIDTH;
        }
        else
        {
            rcDraw.left = DEF_LEFT_SPACE;
            rcDraw.top += DEF_ITEM_HEIGHT + DEF_VERT_SPACE;
            rcDraw.right = rcDraw.left + DEF_ITEM_WIDTH;
            rcDraw.bottom = rcDraw.top + DEF_ITEM_HEIGHT;
        }
        _DrawSoftItem(memDC, rcDraw, (int)iCount);
    }
}

void CTrashSkipCtrl::_DrawHeader(CDC& dc, CRect& rcDraw)
{
    CDC dcTemp;
    CRect rcBack(rcDraw);
    HFONT hFont;
    CString strNum;

    dcTemp.CreateCompatibleDC( dc );
    HBITMAP hOldBitmap = dcTemp.SelectBitmap(m_bmpHeadBack);

    rcBack.bottom = DEF_HEAD_HEIGHT;
    dc.StretchBlt(rcBack.left, rcBack.top, rcBack.Width(), rcBack.Height(), dcTemp, 0, 0, 1, DEF_HEAD_HEIGHT, SRCCOPY);

    rcDraw.top = 0;
    rcDraw.left = DEF_LEFT_SPACE;
    rcDraw.bottom = rcDraw.top + DEF_HEAD_HEIGHT;

    hFont = KuiFontPool::GetFont(FALSE, FALSE, FALSE, 0);
//     _DrawText(dc, rcDraw, L"清理时已跳过", RGB(0,0,0), hFont);
// 
//     strNum.Format(L"%d", GetSize());
// 
//     rcDraw.left = rcDraw.right + 5;
//     rcDraw.right = m_rcClient.right;
// 
//     _DrawText(dc, rcDraw, strNum, RGB(255,0,0), hFont);
// 
//     rcDraw.left = rcDraw.right + 5;
     rcDraw.right = m_rcClient.right;

     if (m_nCountdownNum >= 0)
     {
        CString strFormat = L"%d ";
        CString strText;
        _DrawText(dc, rcDraw, L"已关闭所有程序，",RGB(0,0,0), hFont);
        rcDraw.left = rcDraw.right;
        rcDraw.right = m_rcClient.right;

        strText.Format(strFormat, m_nCountdownNum);
        _DrawText(dc, rcDraw, strText, RGB(0xE4,0x5C,0x27), KuiFontPool::GetFont(TRUE, FALSE, FALSE, 2));
        rcDraw.left = rcDraw.right;
        rcDraw.right = m_rcClient.right;
        _DrawText(dc, rcDraw, L"秒后自动重新扫描！", RGB(0,0,0), hFont);
     }
     else
     {
         _DrawText(dc, rcDraw, L"关闭下列程序再重新扫描，还可以释放约", RGB(0,0,0), hFont);

         ULONGLONG totalSize = 0;
         CString strSize;

         _GetTotalSize(totalSize);

         GetFileSizeTextString(totalSize, strSize);
         rcDraw.left = rcDraw.right + 5;
         rcDraw.right = m_rcClient.right;

         _DrawText(dc, rcDraw, strSize, RGB(255,0,0), hFont);

         rcDraw.left = rcDraw.right + 5;
         rcDraw.right = m_rcClient.right;

         _DrawText(dc, rcDraw, L"磁盘空间。", RGB(0,0,0), hFont);
     }
}

void CTrashSkipCtrl::_DrawItemBackgnd(CDC& dc, CRect rcDraw, int nItemIndex)
{
    CRect rcBack(rcDraw);
    CRect rcCloseBtn;
    COLORREF clrFirst;
    COLORREF clrEnd;
    COLORREF oldbr;
    COLORREF oldPen;

//     HBRUSH hBrush;
//     hBrush = CreateSolidBrush(RGB(0xCA, 0xD1, 0xD7));
    rcBack.top += 6;
    rcBack.right -= 6;

    SelectObject(dc.m_hDC, GetStockObject(DC_PEN));
    SelectObject(dc.m_hDC, GetStockObject(DC_BRUSH));

    oldbr = dc.SetDCBrushColor(RGB(0xFA, 0xFF, 0xFF));
    oldPen = dc.SetDCPenColor(RGB(0xCA, 0xD1, 0xD7));
    
    if (nItemIndex == m_nHoverIndex && m_nSelectIndex == -1)
    {
        clrFirst = RGB(0xFF, 0xFF, 0xFD);
        clrEnd = RGB(0xE6, 0xEE, 0xF8);  
    }
    else if (nItemIndex == m_nSelectIndex)
    {
        clrFirst = RGB(0xC6, 0xE2, 0xFD);
        clrEnd = RGB(0xF8, 0xFB, 0xFD);    
    }
    else
    {
        clrFirst = RGB(0xFF, 0xFF, 0xFD);
        clrEnd = RGB(0xEE, 0xF7, 0xFE);
    }

    dc.RoundRect(rcBack, CPoint(10,10));
    
    rcBack.left += 2;
    rcBack.top += 2;
    rcBack.right -= 2;
    rcBack.bottom -= 2;

    DrawGradualColorRect(dc, rcBack, clrFirst, clrEnd);
    
    dc.SetDCBrushColor(oldbr);
    dc.SetDCPenColor(oldPen);


//    ::DeleteObject(hBrush);
}
void CTrashSkipCtrl::_DrawSoftItem(CDC& dc, CRect& rcDraw, int nItemIndex)
{
    TRASH_SKIP_ITEM item = m_vecItems[nItemIndex];
    HICON hIcon = NULL;
    CRect rcCloseBtn(rcDraw);
    int xSrc;

    _DrawItemBackgnd(dc, rcDraw, nItemIndex);

    Gdiplus::Graphics graphics(dc);

    rcCloseBtn.left = rcDraw.right - 25;
    rcCloseBtn.bottom = rcDraw.top + 25;
    if (m_nHoverIndex == nItemIndex && m_nSelectIndex == -1)
    {
        xSrc = 25;
    }
    else if (m_nSelectIndex == nItemIndex)
    {
        xSrc = 50;
    }
    else
    {
        xSrc = 0;
    }
    graphics.DrawImage(m_imgSoftOff, rcCloseBtn.left, rcCloseBtn.top, xSrc, 0, 25, 25, Gdiplus::UnitPixel);

    ::CopyRect(m_vecItems[nItemIndex].rcItem, rcCloseBtn);


    _GetItemIcon(item.strIconPath, hIcon);

    CRect rcIcon(rcDraw);

    rcIcon.left += (DEF_ITEM_WIDTH - 6 - 32) / 2;
    rcIcon.top += 24;

    if (!hIcon)
    {            
        KAppRes& res = KAppRes::Instance();

        if (item.nItemID == BROWER_IE)
        {
            hIcon = m_iconIE;
        }
        else
        {
            hIcon = m_iconUnknow;
        }
        dc.DrawIcon(rcIcon.left, rcIcon.top, hIcon);
    }
    else
    {
        dc.DrawIconEx(rcIcon.left, rcIcon.top, hIcon, 32, 32);
    }

    _DrawItemText(dc, rcDraw, item.strName);

    if (hIcon && (hIcon != m_iconIE && hIcon != m_iconUnknow))
    {
        DestroyIcon(hIcon);
    }
}

void CTrashSkipCtrl::_DrawItemText(CDC& dc, CRect rcDraw, LPCTSTR szText)
{
    COLORREF clrText = RGB(0x7A, 0x83, 0x88);
    COLORREF oldClr = dc.SetTextColor(clrText);
    HFONT hFont = KuiFontPool::GetFont(FALSE, FALSE, FALSE);
    HFONT oldFont = dc.SelectFont(hFont); 
    CString strText;
    DWORD dwFormat = DT_SINGLELINE|DT_CENTER|DT_NOPREFIX|DT_END_ELLIPSIS|DT_VCENTER;

    if (szText == NULL)
        goto Clear0;

    strText = szText;
    
    rcDraw.top += 65;
    rcDraw.bottom = rcDraw.top + 15;
    
    dc.SetBkMode(TRANSPARENT);
    int iFind = strText.Find(_T(' '));
    if (iFind != -1)
    {
        CString strTop = strText.Mid(0, iFind);
        CString strDown = strText.Mid(iFind + 1);

        dc.DrawText(strTop, -1, &rcDraw, dwFormat);
        
        rcDraw.top += 16;
        rcDraw.bottom += 16;
        dc.DrawText(strDown, -1, &rcDraw, dwFormat);
    }
    else
    {
        dc.DrawText(strText, -1, &rcDraw, dwFormat);
    }

Clear0:
    dc.SelectFont(oldFont);
    dc.SetTextColor(oldClr);
    return;
}

void CTrashSkipCtrl::_DrawText(CDC& memDC, CRect& rcItem, 
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
void CTrashSkipCtrl::_DrawLine(CDC& memDC, CPoint ptStart, CPoint ptEnd, COLORREF color, DWORD dwPenTyple /*=PS_SOLID*/)
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
BOOL CTrashSkipCtrl::_GetTotalSize(ULONGLONG& uTotalSize)
{
    size_t iCount;
    uTotalSize = 0;
    
    for (iCount = 0; iCount < m_vecItems.size(); ++iCount)
    {
        if (m_vecItems[iCount].nItemID == BROWER_IE)
        {
            uTotalSize += m_vecItems[iCount].uSize;
            break;
        }
    }

    for (iCount = 0; iCount < m_vecItems.size(); ++iCount)
    {
        if (m_vecItems[iCount].nItemID != BROWER_IE)
        {
            uTotalSize += m_vecItems[iCount].uSize;
            uTotalSize += m_vecItems[iCount].uAddSize;
        }
        else
        {
            uTotalSize += m_vecItems[iCount].uAddSize;
        }
    }

    return TRUE;
}

BOOL CTrashSkipCtrl::_PointInClose(CPoint pt)
{
    BOOL bRet = FALSE;
    
    for (size_t t = 0; t < m_vecItems.size(); ++t)
    {
        if (m_vecItems[t].rcItem.PtInRect(pt))
        {
            bRet = TRUE;
            goto Clear0;
        }
    }

Clear0:
    return bRet;
}

void CTrashSkipCtrl::_RandItems()
{
    int nLine;
    int nWidth = m_rcClient.Width();
    int nOneWidth = (DEF_ITEM_WIDTH + DEF_HORI_SPACE);
    int nOneLineNum = (nWidth - DEF_LEFT_SPACE - DEF_RIGHT_SPACE)/ nOneWidth;
    int nNum = (int)GetSize();
    int nHeight = (DEF_VERT_SPACE + DEF_ITEM_HEIGHT);
    int nTotalHeight;
    
    if (nOneLineNum <= 0)
        return;

    nLine = nNum / nOneLineNum;

    if (nNum % nOneLineNum)
    {
        nLine += 1;
    }
    nTotalHeight = (nLine * nHeight + DEF_HEAD_HEIGHT);
    if (m_rcClient.Height() > nTotalHeight)
    {
        if (m_rcRealClient.Height() != m_rcClient.Height())
        {
            ::CopyRect(m_rcRealClient, m_rcClient);
            SCROLLINFO si;

            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL;
            //GetScrollInfo(SB_VERT, &si);
            si.nMin = 1;
            si.nPos = m_nPos;
            si.nMax = 1;
            si.nPage = m_rcClient.Height();

            SetScrollInfo(SB_VERT, &si, TRUE);
        }
    }
    else
    {
        if (m_rcRealClient.Height() < nTotalHeight)
        {
            m_rcRealClient.bottom = nTotalHeight;

            SCROLLINFO si;

            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL;
            //GetScrollInfo(SB_VERT, &si);
            si.nMin = 1;
            si.nPos = m_nPos;
            si.nMax = nTotalHeight;
            si.nPage = m_rcClient.Height();

            SetScrollInfo(SB_VERT, &si, TRUE);
        }
    }
}
//////////////////////////////////////////////////////////////////////////
