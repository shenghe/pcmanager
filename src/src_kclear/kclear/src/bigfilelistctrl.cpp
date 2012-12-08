#include "stdafx.h"
#include "bigfilelistctrl.h"
#include "kscmaindlg.h"
#include "uihandler_bigfile.h"
#include "kscbase/kscres.h"
#include "bigfilehelper.h"
#include "kscgui/kdraw.h"
#include "iconhelper.h"

//////////////////////////////////////////////////////////////////////////

CIconCache g_IconCache;

//////////////////////////////////////////////////////////////////////////

CBigFileListCtrl::CBigFileListCtrl(void* pOwner)
{
    m_pOwner        = pOwner;
    m_nItemHeight   = 24;
    m_nHeaderHeight = 24;
    m_nHotItem      = -1;
    m_nHotLink      = -1;
    m_bMoreLink     = FALSE;
    m_bSysFilter    = TRUE;
}

CBigFileListCtrl::~CBigFileListCtrl()
{

}

//////////////////////////////////////////////////////////////////////////

BOOL CBigFileListCtrl::OnEraseBkgnd(CDCHandle dc)
{
    CRect rcClient;
    GetClientRect(rcClient);
    dc.FillSolidRect(rcClient, RGB(0xff, 0xff, 0xff));
    return TRUE;
}

void CBigFileListCtrl::Init()
{
    m_wndToolTip.Create(m_hWnd);
    m_wndToolTip.SetMaxTipWidth(400);
    m_wndToolTip.AddTool(m_hWnd);
    m_wndToolTip.Activate(FALSE);
    m_wndToolTip.SetDelayTime(0, 1000);

    ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
    m_hdr.SubclassWindow(GetHeader());
    m_hdr.SetDlgCtrlID(1000);
    //m_hdr.EnableWindow(FALSE);
}

BOOL CBigFileListCtrl::SubclassWindow(HWND hWnd)
{
    BOOL retval = FALSE;
    retval = CBigFileListCtrlBase::SubclassWindow(hWnd);
    Init();
    return retval;
}

void CBigFileListCtrl::AddLink(const CString& strLink)
{
    LinkInfo link;
    link.strLink = strLink;
    m_vLinks.push_back(link);
}

void CBigFileListCtrl::SetNotifyHwnd(HWND hNotifyWnd)
{
    m_hNotifyWnd = hNotifyWnd;
}

void CBigFileListCtrl::SetMoreItem(DWORD dwMoreItems)
{
    m_dwMoreItems = dwMoreItems;
}

void CBigFileListCtrl::SetSysFilterEnable(BOOL bEnable)
{
    m_bSysFilter = bEnable;
}

BOOL CBigFileListCtrl::GetSysFilterEnable()
{
    return m_bSysFilter;
}

void CBigFileListCtrl::DoDrawItem(CDCHandle dc, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CRect rcItem = lpDrawItemStruct->rcItem;
    size_t idx = (size_t)lpDrawItemStruct->itemData;
    int nItem = lpDrawItemStruct->itemID;
    BOOL bSelect = lpDrawItemStruct->itemState & ODS_SELECTED;
    CRect rcDraw;
    CVirtualDirManager* pDir = (CVirtualDirManager*)m_pOwner;
    CString strFilePath, strFile, strDir;
    HICON hIcon = NULL, hIconCache = NULL;
    int nCurrentEnd = rcItem.left;
    double percent;
    CRect rcClient;
    CFontHandle underlineFont = KuiFontPool::GetFont(FALSE, TRUE, FALSE);
    CFontHandle normalFont = KuiFontPool::GetFont(FALSE, FALSE, FALSE);

    // 绘制背景
    {
        rcDraw = rcItem;
        GetClientRect(rcClient);
        if (rcClient.right > rcDraw.right)
            rcDraw.right = rcClient.right;
        dc.FillSolidRect(rcDraw, RGB(0xff,0xff,0xff));
    }

    if (idx == -1)
    {
        CString strMore;
        dc.SelectFont(underlineFont);
        dc.SetTextColor(RGB(0x4d, 0x70, 0x96));
        strMore.Format(
            //_T("已经显示了%d个文件，还有%d个没有显示，点击此处查看全部"), 
            _T("还有%d个没有显示，点击此处查看全部"),
            //GetItemCount() - 1, 
            m_dwMoreItems);
        rcDraw = rcItem;
        dc.FillSolidRect(rcDraw, RGB(0xff, 0xff, 0xff));
        int nLeft = rcItem.left + GetColumnWidth(0) + GetColumnWidth(1);
        int nRight = nLeft + GetColumnWidth(2);
        rcDraw.left = nLeft;
        rcDraw.right = nRight;
        dc.DrawText(strMore, strMore.GetLength(), rcDraw, DT_LEFT|DT_BOTTOM|DT_SINGLELINE|DT_CALCRECT);
        if (rcDraw.right > nRight)
            rcDraw.right = nRight;
        dc.DrawText(strMore, strMore.GetLength(), rcDraw, DT_LEFT|DT_BOTTOM|DT_SINGLELINE|DT_END_ELLIPSIS);
        m_rcMore.left = rcDraw.left;
        m_rcMore.right = rcDraw.right;
        m_rcMore.top = 0;
        m_rcMore.bottom = m_nItemHeight;
        return;
    }

    if (idx == -2)
    {
        CString strNone;
        dc.SelectFont(normalFont);
        dc.SetTextColor(RGB(0, 0, 0));
        strNone = _T("没有发现文件，您可以重新扫描。");
        rcDraw = rcItem;
        dc.FillSolidRect(rcDraw, RGB(0xff, 0xff, 0xff));
        rcDraw.left = rcItem.left + GetColumnWidth(0) + GetColumnWidth(1);
        rcDraw.right = rcDraw.left + GetColumnWidth(2);
        dc.DrawText(strNone, strNone.GetLength(), rcDraw, DT_LEFT|DT_BOTTOM|DT_SINGLELINE|DT_END_ELLIPSIS);
        return;
    }

    percent = (double)pDir->m_vCacheFiles[idx].qwFileSize / pDir->m_vCacheFiles[0].qwFileSize;
    rcDraw = rcItem;
    dc.SetTextColor(RGB(0,0,0));
    if (bSelect)
    {
        CPen penBorder;
        CBrush brushBk;
        CRect rcSelect = rcDraw;

        penBorder.CreatePen(PS_SOLID, 1, RGB(0x7d, 0xa2, 0xce));
        brushBk.CreateSolidBrush(RGB(0xeb, 0xf4, 0xfd));
        dc.SelectPen(penBorder);
        dc.SelectBrush(brushBk);
        dc.RoundRect(rcSelect, CPoint(3, 3));
        rcSelect.left += 2;
        rcSelect.right -= 2;
        rcSelect.top += 2;
        rcSelect.bottom -= 2;
        DrawGradualColorRect(dc, rcSelect, RGB(0xdd, 0xec, 0xfd), RGB(0xc2, 0xdc, 0xfd));
    }
    else
    {
        dc.FillSolidRect(rcDraw, RGB(0xff, 0xff, 0xff));
    }
    {
        // 画背景进度条
        CRect rcProgress = rcDraw;
        rcProgress.left += 2;
        rcProgress.right -= 2;
        rcProgress.top += 2;
        rcProgress.bottom -= 2;
        rcProgress.right = rcProgress.left + int(rcProgress.Width() * percent);
        COLORREF color1 = RGB(0xff, 0xf4, 0xcc);
        COLORREF color2 = RGB(0xff, 0xde, 0x5c);
        COLORREF color3 = RGB(0xff, 0xf4 + (0xde - 0xf4) * percent, 0xcc + (0x5c - 0xcc) * percent);
        DrawGradualColorRect(dc, rcProgress, color1, color3, FALSE);
    }
    //rcDraw.top = rcDraw.bottom;
    //rcDraw.bottom += 1;
    //dc.FillSolidRect(rcDraw, RGB(0xea, 0xe9, 0xe1));

    dc.SetBkMode(TRANSPARENT);

    strFilePath = pDir->m_vCacheFiles[idx].strFilePath;
    strFile = strFilePath.Right(strFilePath.GetLength() - strFilePath.ReverseFind(_T('\\')) - 1);
    strDir = strFilePath.Left(strFilePath.GetLength());

    // 绘制文件名
    hIconCache = g_IconCache.GetIcon(strFilePath);
    if (hIconCache)
    {
        hIcon = hIconCache;
    }
    else
    {
        hIcon = bigfilehelper::GetIconFromFilePath(strFilePath);
    }
    rcDraw = rcItem;
    rcDraw.left = nCurrentEnd;
    nCurrentEnd += GetColumnWidth(0);
    rcDraw.right = nCurrentEnd;
    if (rcDraw.Width() > 8)
    {
        dc.DrawIconEx(rcDraw.left + 4, rcDraw.top + (rcDraw.Height() - 16) / 2, hIcon, 16, 16);
    }
    if (rcDraw.Width() > 24)
    {
        rcDraw.left += 24;
        BOOL bFilter = CBigfileFilter::Instance().IsFileInFilter(strFilePath);
        BOOL bfilterEnable = GetSysFilterEnable();
        if(bFilter && bfilterEnable)
            dc.SetTextColor(RGB(0xcc, 0xcc, 0xcc));
        dc.DrawText(strFile, strFile.GetLength(), rcDraw, DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_END_ELLIPSIS);
    }

    // 绘制进度条
    rcDraw.left = nCurrentEnd;
    nCurrentEnd += GetColumnWidth(1);
    rcDraw.right = nCurrentEnd;
    if (rcDraw.Width() > 8)
    {
        CString strSize;
        bigfilehelper::GetFileSizeString(pDir->m_vCacheFiles[idx].qwFileSize, strSize);
        dc.DrawText(strSize, strSize.GetLength(), rcDraw, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
    }
//     if (rcDraw.Width() > 48)
//     {
//         CPen penBorder;
//         CBrush brushProgress;
//         CRect rcProgress = rcDraw;
//         int nLength;
//         rcProgress.left += 40;
//         rcProgress.right -= 4;
//         rcProgress.top += (rcDraw.Height() - 6) / 2;
//         rcProgress.bottom -= (rcDraw.Height() - 6) / 2;
//         penBorder.CreatePen(PS_SOLID, 1, RGB(0xb5, 0xb5, 0xb5));
//         brushProgress.CreateSolidBrush(RGB(0xf3, 0xf6, 0xfd));
//         dc.SelectPen(penBorder);
//         dc.SelectBrush(brushProgress);
//         dc.RoundRect(rcProgress, CPoint(3, 3));
//     }

    // 绘制路径
    rcDraw.left = nCurrentEnd;
    nCurrentEnd += GetColumnWidth(2);
    rcDraw.right = nCurrentEnd;

    if (rcDraw.Width() > 8)
    {
        dc.DrawText(strDir, strDir.GetLength(), rcDraw, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);   
    }

    // 绘制控制
    rcDraw.left = nCurrentEnd;
    nCurrentEnd += GetColumnWidth(3);
    rcDraw.right = nCurrentEnd;

    if (rcDraw.Width() > 4)
    {
        CRect rcLink;
        for (size_t i = 0; i < m_vLinks.size(); ++i)
        {
            CString strLink = m_vLinks[i].strLink;
            rcLink = rcDraw;
            rcLink.left = rcDraw.left + 4 + 40 * (long)i;
            rcLink.top += 4;
            rcLink.bottom += 4;
            dc.SelectFont(underlineFont);
            dc.SetTextColor(RGB(0x4d, 0x70, 0x96));
            dc.DrawText(strLink, strLink.GetLength(), rcLink, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_CALCRECT);
            if (rcLink.right > rcDraw.right)
                rcLink.right = rcDraw.right;
            dc.DrawText(strLink, strLink.GetLength(), rcLink, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);
            rcLink.top = (rcLink.top - m_nHeaderHeight) % m_nItemHeight;
            rcLink.bottom = (rcLink.bottom - m_nHeaderHeight) % m_nItemHeight;
            m_vLinks[i].rcLink = rcLink;
        }
    }

    // 绘制
    //dc.DrawText(strFilePath, strFilePath.GetLength(), rcDraw, DT_SINGLELINE|DT_VCENTER|DT_LEFT);

    if (hIcon != hIconCache)
    {
        DestroyIcon(hIcon);
    }
}

void CBigFileListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
//     RECT rect = { 0 };
//     GetClientRect(&rect);
//     CMemoryDC dcMem(lpDrawItemStruct->hDC, rect);
//     dcMem.FillSolidRect(&rect, RGB(0xff, 0xff, 0xff));
//    DoDrawItem(dcMem.m_hDC, lpDrawItemStruct);
    DoDrawItem(lpDrawItemStruct->hDC, lpDrawItemStruct);
}

void CBigFileListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    lpMeasureItemStruct->itemHeight = m_nItemHeight;
}

int CBigFileListCtrl::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
    return 0;
}

void CBigFileListCtrl::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
{

}

//////////////////////////////////////////////////////////////////////////

LRESULT CBigFileListCtrl::OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (m_wndToolTip.IsWindow())
    {
        MSG msg = { m_hWnd, uMsg, wParam, lParam };

        m_wndToolTip.RelayEvent(&msg);
    }

    SetMsgHandled(FALSE);

    return 0;
}

LRESULT CBigFileListCtrl::OnMouseLeave()
{
//     LDLISTVIEW_MSG_NOTIFY msgNotify;
//     FILL_NOTIFY_MSG_STRUCT(WM_MOUSELEAVE, 0,0, m_dwCtrlID, msgNotify);
//     
//     m_wndToolTip.Activate(FALSE);
//     m_wndToolTip.DelTool(m_hWnd, 1000);
//     ::SendMessage(m_hNotifyHwnd, MSG_USER_LDLISTVIEWCTRL_NOTIFY, 0, (LPARAM)&msgNotify);

    return 0;
}

void CBigFileListCtrl::OnVScroll(UINT nSBCode, UINT nPos, HWND hWnd)
{
    if (SB_TOP == nSBCode || SB_ENDSCROLL == nSBCode)
    {
        Invalidate();
    }

    SetMsgHandled(FALSE);
}

LRESULT CBigFileListCtrl::OnMouseWhell(UINT fwKey, short zDela,CPoint pt)
{
    SetMsgHandled(FALSE);

    return 0;
}

void CBigFileListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
    int nItem, nLink;
    CVirtualDirManager* pDir = (CVirtualDirManager*)m_pOwner;
    nLink = IsMouseInLink(point, nItem);

    if (-1 == nItem)
    {
        m_wndToolTip.Activate(FALSE);
        goto clean0;
    }

    if (nLink != -1)
    {
        m_nHotItem = nItem;
        m_nHotLink = nLink;
    }
    else
    {
        m_nHotItem = -1;
        m_nHotLink = -1;
    }

    if (pDir)
    {
        BigFileInfo fileInfo;
        size_t idx = (size_t)GetItemData(nItem);
        if (idx != -1 && idx != -2)
        {
            pDir->GetBigFileInfo(idx, fileInfo);
            if (m_strTip.CompareNoCase(fileInfo.strFilePath) != 0)
            {
                m_strTip = fileInfo.strFilePath;
                m_wndToolTip.Activate(FALSE);
                m_wndToolTip.UpdateTipText((_U_STRINGorID)fileInfo.strFilePath, m_hWnd);
                m_wndToolTip.Activate(TRUE);
            }
        }
    }

clean0:
    IsMouseInMore(point, nItem);
    SetMsgHandled(FALSE);
}

void CBigFileListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{

}

void CBigFileListCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
    int nItem = HitTest(point);
    if (nItem != -1)
    {
        SetMsgHandled(TRUE);
        ::PostMessage(m_hNotifyWnd, WM_BIGFILE_LIST_CONTEXT, nItem, 0);
    }

    SetMsgHandled(FALSE);
}

void CBigFileListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    int nItem = HitTest(point);

    if (nItem != -1)
    {
        ::PostMessage(m_hNotifyWnd, WM_BIGFILE_LIST_DBCLICK, nItem, 0);
    }

    SetMsgHandled(FALSE);
}

void CBigFileListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_bMoreLink)
    {
        ::PostMessage(m_hNotifyWnd, WM_BIGFILE_LIST_MORE, 0, 0);
    }

    if (m_nHotLink != -1 && m_nHotItem != -1)
    {
        ::PostMessage(m_hNotifyWnd, WM_BIGFILE_LIST_LINK, m_nHotItem, m_nHotLink);
    }

    SetMsgHandled(FALSE);
}

//////////////////////////////////////////////////////////////////////////

int CBigFileListCtrl::HitTest(CPoint pt)
{
    LVHITTESTINFO hitinfo;

    hitinfo.pt = pt;
    return __super::HitTest(&hitinfo);
}

//////////////////////////////////////////////////////////////////////////

BOOL CBigFileListCtrl::IsMouseInMore(CPoint pt, int& nItem)
{
    BOOL retval = FALSE;

    nItem = HitTest(pt);
    if (-1 == nItem)
        goto clean0;

    if (!m_dwMoreItems)
        goto clean0;

    if (nItem != GetItemCount() - 1)
        goto clean0;

    pt.y = (pt.y - m_nHeaderHeight) % m_nItemHeight;

    if (PtInRect(m_rcMore, pt))
    {
        retval = TRUE;
        SetCursor(LoadCursor(NULL, IDC_HAND));
        goto clean0;
    }

clean0:
    m_bMoreLink = retval;
    return retval;
}

int CBigFileListCtrl::IsMouseInLink(CPoint pt, int& nItem)
{
    int retval = -1;
    CPoint pt2 = pt;
    size_t i;

    nItem = HitTest(pt);
    if (-1 == nItem)
        goto clean0;
        
    pt2.y = (pt2.y - m_nHeaderHeight) % m_nItemHeight;

    for (i = 0; i < m_vLinks.size(); ++i)
    {
        if (PtInRect(m_vLinks[i].rcLink, pt2))
        {
            retval = (int)i;
            SetCursor(LoadCursor(NULL, IDC_HAND));
            goto clean0;
        }
    }

clean0:
    return retval;
}

BOOL CBigFileListCtrl::DeleteItem(int nItem)
{
    return __super::DeleteItem(nItem);
}

//////////////////////////////////////////////////////////////////////////
