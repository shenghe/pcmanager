#include "stdafx.h"
#include "vdirctrl.h"
#include "kscbase/kscres.h"
#include "kscgui/kdraw.h"
#include "kuires/kuifontpool.h"
#include "bigfilehelper.h"
#include "kuires.h"

//////////////////////////////////////////////////////////////////////////

static const UINT g_nTitleHeight = 24;
static const UINT g_nItemHeight = 44;

//////////////////////////////////////////////////////////////////////////

CVirtualDirCtrl::CVirtualDirCtrl()
{
    m_nSelect       = 0;
    m_nSubSelect    = 0;
    m_nScrollOffset = 0;
    m_nScrollLimit  = 0;
}

CVirtualDirCtrl::~CVirtualDirCtrl()
{
}

//////////////////////////////////////////////////////////////////////////

void CVirtualDirCtrl::Init()
{
    
}

void CVirtualDirCtrl::UnInit()
{

}
//////////////////////////////////////////////////////////////////////////

size_t CVirtualDirCtrl::AddItem(HICON hIcon, const CString& strTitle)
{
    VDirSet dirSet;
    KAutoLock lock(m_lock);

    dirSet.hIcon = hIcon;
    dirSet.strTitle = strTitle;
    dirSet.bHasScroll = FALSE;
    m_vBigSets.push_back(dirSet);

    return m_vBigSets.size() - 1;
}

size_t CVirtualDirCtrl::AddSubItem(
    size_t nItem,
    const CString& strName, 
    const CString& strComment, 
    UINT nPercent, 
    BOOL bShowPercent
    )
{
    KAutoLock lock(m_lock);

    if (nItem < m_vBigSets.size())
    {
        VDir dir;
        dir.strName = strName;
        dir.strComment = strComment;
        dir.nPercent = nPercent;
        dir.bShowPercent = bShowPercent;
        dir.pData = NULL;
        m_vBigSets[nItem].vDirs.push_back(dir);

        return m_vBigSets[nItem].vDirs.size() - 1;
    }
    else
    {
        return size_t(-1);
    }
}

void CVirtualDirCtrl::UpdateSubItem(
    size_t nItem, 
    size_t nSubItem, 
    const CString& strName, 
    const CString& strComment, 
    UINT nPercent, 
    BOOL bShowPercent
    )
{
    KAutoLock lock(m_lock);

    if (nItem < m_vBigSets.size())
    {
        if (nSubItem < m_vBigSets[nItem].vDirs.size())
        {
            m_vBigSets[nItem].vDirs[nSubItem].strName = strName;
            m_vBigSets[nItem].vDirs[nSubItem].strComment = strComment;
            m_vBigSets[nItem].vDirs[nSubItem].nPercent = nPercent;
            m_vBigSets[nItem].vDirs[nSubItem].bShowPercent = bShowPercent;
        }
    }
}

void CVirtualDirCtrl::Select(size_t nItem, size_t nSubItem)
{
    m_nSelect = nItem;
    m_nSubSelect = nSubItem;
}

void CVirtualDirCtrl::SetItemData(size_t nItem, size_t nSubItem, PVOID pData)
{
    KAutoLock lock(m_lock);

    if (nItem < m_vBigSets.size())
    {
        if (nSubItem < m_vBigSets[nItem].vDirs.size())
        {
            m_vBigSets[nItem].vDirs[nSubItem].pData = pData;
        }
    }
}

PVOID CVirtualDirCtrl::GetItemData(size_t nItem, size_t nSubItem)
{
    PVOID retval = NULL;
    KAutoLock lock(m_lock);

    if (nItem < m_vBigSets.size())
    {
        if (nSubItem < m_vBigSets[nItem].vDirs.size())
            retval = m_vBigSets[nItem].vDirs[nSubItem].pData;
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////

void CVirtualDirCtrl::DoPaint(CDCHandle dc)
{
    size_t i;

    GetClientRect(m_rcClient);
    dc.FillSolidRect(m_rcClient, RGB(0xff, 0xff, 0xff));
    dc.SetBkMode(TRANSPARENT);

    for (i = 0; i < m_vBigSets.size(); ++i)
    {
        PaintVDirSet(dc, i);
    }
}

//////////////////////////////////////////////////////////////////////////

void CVirtualDirCtrl::PaintVDirSet(CDCHandle& dc, size_t idx)
{
    UINT nOffsetY = GetPaintOffsetY(idx);
    UINT nHeight = GetPaintHeight(idx);
    CRect rcDraw;
    size_t i;
    int nWidth = m_rcClient.Width();
    CFontHandle boldFont = KuiFontPool::GetFont(TRUE, FALSE, FALSE);
    CFontHandle normalFont = KuiFontPool::GetFont(FALSE, FALSE, FALSE);
    BOOL bHasScroll = FALSE;

    if (nHeight + nOffsetY > (UINT)m_rcClient.Height())
    {
        bHasScroll = TRUE;
    }
    else
    {
        if (1 == idx)
        {
            m_nScrollOffset = 0;
            m_nScrollLimit = 0;
        }
    }

    m_vBigSets[idx].bHasScroll = bHasScroll;

    // 绘制标题
    rcDraw.left = 0;
    rcDraw.right = m_rcClient.Width();
    rcDraw.top = nOffsetY;
    rcDraw.bottom = nOffsetY + g_nTitleHeight;
    dc.FillSolidRect(rcDraw, RGB(0xeb, 0xf4, 0xfd));
    rcDraw.left += 1;
    rcDraw.right -= 1;
    rcDraw.top += 1;
    rcDraw.bottom -= 1;
    DrawGradualColorRect(dc, rcDraw, RGB(0xdd, 0xec, 0xfd), RGB(0xc2, 0xdc, 0xfd));
    rcDraw.left = 0;
    rcDraw.right = nWidth;
    rcDraw.top -= 1;
    rcDraw.bottom += 1;
    dc.SetTextColor(RGB(0x4e, 0x70, 0xa0));
    dc.SelectFont(boldFont);
    dc.DrawText(
        m_vBigSets[idx].strTitle, 
        m_vBigSets[idx].strTitle.GetLength(),
        rcDraw,
        DT_SINGLELINE|DT_CENTER|DT_VCENTER
        );
    rcDraw.top = rcDraw.bottom - 1;
    dc.FillSolidRect(rcDraw, RGB(0x96, 0xae, 0xca));

    if (bHasScroll)
    {
        // 画滚动箭头
        COLORREF dwColor = m_nScrollOffset ? RGB(0x7d, 0xa2, 0xce) : RGB(0xee, 0xee, 0xee);
        CRect rcUp = rcDraw;
        rcUp.top = nOffsetY + g_nTitleHeight;
        rcUp.bottom = rcUp.top + 10;
        m_rcUp = rcUp;
        POINT pt[3];
        pt[0].x = rcUp.left + rcUp.Width() / 2;
        pt[0].y = rcUp.top + 4;
        pt[1].x = pt[0].x - 6;
        pt[1].y = pt[0].y + 6;
        pt[2].x = pt[0].x + 6;
        pt[2].y = pt[0].y + 6;
        CPen penUp;
        CBrush brushUp;
        penUp.CreatePen(PS_SOLID, 1, dwColor);
        brushUp.CreateSolidBrush(dwColor);
        dc.SelectPen(penUp);
        dc.SelectBrush(brushUp);
        dc.Polygon(pt, 3);
    }
    else
    {
        m_rcUp.top = 0;
        m_rcUp.bottom = 0;
    }

    dc.SelectFont(normalFont);
    dc.SetTextColor(RGB(0,0,0));
    m_nScrollLimit = m_nScrollOffset;
    if (1 == idx)
    {
        i = m_nScrollOffset;
    }
    else
    {
        i = 0;
    }
    for (; i < m_vBigSets[idx].vDirs.size(); ++i)
    {
        if (1 == idx)
            rcDraw.top = nOffsetY + g_nTitleHeight + (i - m_nScrollOffset) * g_nItemHeight + m_rcUp.Height();
        else
            rcDraw.top = nOffsetY + g_nTitleHeight + i * g_nItemHeight + m_rcUp.Height();

        rcDraw.bottom = rcDraw.top + g_nItemHeight;

        if (rcDraw.bottom > m_rcClient.Height())
        {
            m_vBigSets[idx].vDirs[i].bVisual = FALSE;
            m_nScrollLimit++;
            continue;
        }
        else
        {
            m_vBigSets[idx].vDirs[i].bVisual = TRUE;
        }

        if (idx == m_nSelect && i == m_nSubSelect)
        {
            // 绘制选中背景
            CRect rcSelect = rcDraw;
            CPen penBorder;
            CBrush brushBk;

            penBorder.CreatePen(PS_SOLID, 1, RGB(0x7d, 0xa2, 0xce));
            brushBk.CreateSolidBrush(RGB(0xeb, 0xf4, 0xfd));
            rcSelect.top += 4;
            rcSelect.bottom -= 4;
            rcSelect.left += 4;
            rcSelect.right = nWidth - 4;
            dc.SelectPen(penBorder);
            dc.SelectBrush(brushBk);
            dc.RoundRect(rcSelect, CPoint(3, 3));
            rcSelect.left += 2;
            rcSelect.right -= 2;
            rcSelect.top += 2;
            rcSelect.bottom -= 2;
            DrawGradualColorRect(dc, rcSelect, RGB(0xdd, 0xec, 0xfd), RGB(0xc2, 0xdc, 0xfd));
            //rcSelect.left = nWidth / 2;
            
            //DrawGradualColorRect(dc, rcSelect, RGB(0xbf, 0xbf, 0xbf), RGB(0xff, 0xff, 0xff), FALSE);
        }

        // 绘制图标
        if (m_vBigSets[idx].vDirs[i].bShowPercent)
        {
            dc.DrawIcon(12, rcDraw.top + 2, m_vBigSets[idx].hIcon);
        }
        else
        {
            dc.DrawIcon(12, rcDraw.top + 6, m_vBigSets[idx].hIcon);
        }

        if (m_vBigSets[idx].vDirs[i].bShowPercent)
        {
            // 绘制文字
            CString strDraw;
            strDraw.Format(_T("%s %s"), m_vBigSets[idx].vDirs[i].strName, m_vBigSets[idx].vDirs[i].strComment);
            CRect rcText = rcDraw;
            rcText.left += 48;
            rcText.right -= 8;
            rcText.bottom = rcDraw.top + rcDraw.Height() * 3 / 5;
            dc.DrawText(strDraw, strDraw.GetLength(), rcText, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);

            // 绘进度条
            CPen borderPen;
            CBrush brushBk;
            CRect rcProgress = rcDraw;
            rcProgress.left += 48;
            rcProgress.right -= 12;
            rcProgress.top = rcDraw.top + rcDraw.Height() * 3 / 5;
            rcProgress.bottom = rcProgress.top + 8;
            borderPen.CreatePen(PS_SOLID, 1, RGB(187,191,192));
            brushBk.CreateSolidBrush(RGB(255,255,255));
            dc.SelectPen(borderPen);
            dc.SelectBrush(brushBk);
            dc.Rectangle(rcProgress);
            rcProgress.left += 2;
            rcProgress.right -= 2;
            rcProgress.top += 2;
            rcProgress.bottom -= 2;
            dc.FillSolidRect(rcProgress, RGB(223,221,222));
            rcProgress.right = rcProgress.left + rcProgress.Width() * m_vBigSets[idx].vDirs[i].nPercent / 100;
            dc.FillSolidRect(rcProgress, RGB(148,210,101));
        }
        else
        {
            // 绘制文字
            CString strDraw;
            strDraw.Format(_T("%s %s"), m_vBigSets[idx].vDirs[i].strName, m_vBigSets[idx].vDirs[i].strComment);
            CRect rcText = rcDraw;
            rcText.left += 48;
            rcText.right -= 8;
            dc.DrawText(strDraw, strDraw.GetLength(), rcText, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
        }

        if (i + 1 == m_vBigSets[idx].vDirs.size() && idx + 1 != m_vBigSets.size())
        {
            // 画最低下面的线
            rcDraw.top = rcDraw.bottom - 1;
            dc.FillSolidRect(rcDraw, RGB(0x96, 0xae, 0xca));
        }

        if (i + 1 != m_vBigSets[idx].vDirs.size())
        {
            // 画分割线
            rcDraw.top = rcDraw.bottom - 1;
            rcDraw.left = 6;
            rcDraw.right = nWidth / 2;
            DrawGradualColorRect(dc, rcDraw, RGB(255,255,255), RGB(0xd6, 0xe5, 0xf5), FALSE);
            rcDraw.left = nWidth / 2;
            rcDraw.right = nWidth - 6;
            DrawGradualColorRect(dc, rcDraw, RGB(0xd6, 0xe5, 0xf5), RGB(255,255,255), FALSE);
            //dc.FillSolidRect(rcDraw, RGB(0xd6, 0xe5, 0xf5));
            rcDraw.left = 0;
            rcDraw.right = nWidth;
        }
    }

    if (bHasScroll)
    {
        // 画滚动箭头
        COLORREF dwColor = (m_nScrollLimit > m_nScrollOffset) ? RGB(0x7d, 0xa2, 0xce) : RGB(0xee, 0xee, 0xee);
        CRect rcDown = rcDraw;
        rcDown.top = m_rcClient.bottom - 10;
        rcDown.bottom = m_rcClient.bottom;
        m_rcDown = rcDown;
        POINT pt[3];
        pt[0].x = rcDown.left + rcDown.Width() / 2;
        pt[0].y = rcDown.bottom - 6;
        pt[1].x = pt[0].x - 6;
        pt[1].y = pt[0].y - 6;
        pt[2].x = pt[0].x + 6;
        pt[2].y = pt[0].y - 6;
        CPen penUp;
        CBrush brushUp;
        penUp.CreatePen(PS_SOLID, 1, dwColor);
        brushUp.CreateSolidBrush(dwColor);
        dc.SelectPen(penUp);
        dc.SelectBrush(brushUp);
        dc.Polygon(pt, 3);
    }
    else
    {
        m_rcDown.top = 0;
        m_rcDown.bottom = 0;
    }
}

UINT CVirtualDirCtrl::GetPaintOffsetY(size_t idx)
{
    UINT retval;
    size_t i, j, current = 0;

    for (i = 0; i < m_vBigSets.size(); ++i)
    {
        if (idx == i)
        {
            break;
        }

        for (j = 0; j < m_vBigSets[i].vDirs.size(); ++j)
        {
            current++;    
        }
    }

    retval = UINT(idx * g_nTitleHeight + current * g_nItemHeight);

    return retval;
}

UINT CVirtualDirCtrl::GetPaintHeight(size_t idx)
{
    return g_nTitleHeight + m_vBigSets[idx].vDirs.size() * g_nItemHeight;
}

//////////////////////////////////////////////////////////////////////////

WORD CVirtualDirCtrl::HitTest(CPoint pt)
{
    LONG retval = -1;
    size_t i, j;
    UINT nPos1, nPos2;
    size_t current = 0;
    int nScrollHeight = 0;

    for (i = 0; i < m_vBigSets.size(); ++i)
    {
        for (j = 0; j < m_vBigSets[i].vDirs.size(); ++j)
        {
            nScrollHeight = m_vBigSets[i].bHasScroll ? 10 : 0;
            nPos1 = g_nTitleHeight * (i + 1) + g_nItemHeight * current + nScrollHeight;
            nPos2 = g_nTitleHeight * (i + 1) + g_nItemHeight * (current + 1) + nScrollHeight;

//             if (!m_vBigSets[i].vDirs[j].bVisual)
//                 break;

            if (pt.y > (LONG)nPos1 && pt.y < (LONG)nPos2)
            {
                if (1 == i)
                    j += m_nScrollOffset;

                retval = MAKEWORD(j, i);
                goto clean0;
            }

            current++;
        }
    }

clean0:
    return (WORD)retval;
}

//////////////////////////////////////////////////////////////////////////

void CVirtualDirCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
    WORD nIndex = HitTest(point);
    size_t nSelect, nSubSelect;
	SetMsgHandled(FALSE);
    if (m_rcDown.Height() && PtInRect(m_rcDown, point))
    {
        if (m_nScrollOffset < m_nScrollLimit)
        {
            m_nScrollOffset += 1;
            InvalidateRect(NULL);
        }
        return;
    }

    if (m_rcUp.Height() && PtInRect(m_rcUp, point))
    {
        if (m_nScrollOffset > 0)
        {
            m_nScrollOffset -= 1;
            InvalidateRect(NULL);
        }
        return;
    }

    if (WORD(-1) == nIndex)
        goto clean0;

    nSelect = nIndex >> 8;
    nSubSelect = nIndex & 0xFF;

    if (m_nSelect != nSelect || m_nSubSelect != nSubSelect)
    {
        m_nSelect = nSelect;
        m_nSubSelect = nSubSelect;
        InvalidateRect(NULL);
    }

    GetParent().GetParent().PostMessage(
        WM_COMMAND, 
        MAKELPARAM(ID_CTL_BIGFILE_LISTLEFT, nIndex),
        (LPARAM)m_hWnd
        );

clean0:
    return;
}

void CVirtualDirCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
//     WORD nIndex = HitTest(point);
// 
//     if (-1 == nIndex)
//         goto clean0;
// 
// clean0:
//     return;
}

//////////////////////////////////////////////////////////////////////////
