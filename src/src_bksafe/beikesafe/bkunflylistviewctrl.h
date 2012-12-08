#pragma once
#include <wtlhelper/whwindow.h>

#define BKUNFLYLVN_ITEMSORT  (LVN_LAST - 1)

typedef struct tagBKUNFLYLVNITEMSORT
{
    NMHDR hdr;
    int nCol;
    BOOL bDescending;
} BKUNFLYLVNITEMSORT, *LPBKUNFLYLVNITEMSORT;

class CBkUnflyListViewCtrl
    : public CWindowImpl<CBkUnflyListViewCtrl, CListViewCtrl, CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0>>
    , public CCustomDraw<CBkUnflyListViewCtrl>
{
public:
    CBkUnflyListViewCtrl()
        : m_nSortColumn(-1)
        , m_bSortDescending(FALSE)
        , m_nItemHeight(20)
    {
    }

    HWND Create(
        HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL, 
        DWORD dwStyle = 0, DWORD dwExStyle = 0, _U_MENUorID MenuOrID = 0U, 
        LPVOID lpCreateParam = NULL)
    {
        HWND hWnd = __super::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);

        if (!hWnd)
            return NULL;

        SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

        m_imglstCheck.Create();

        SetItemHeight();

        ShowCheckBox(FALSE);

        SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT), FALSE);

        if (0 == (dwStyle & LVS_NOCOLUMNHEADER))
            m_hdrListVirus.SubclassWindow(GetHeader());

        return hWnd;
    }

    void SetItemHeight(int nHeight = 0)
    {
        CImageList imglst;

        imglst.Create(max(1, m_imgList.GetSubImageWidth()), nHeight ? nHeight : m_nItemHeight, ILC_COLOR24, 0, 1);

        __super::SetImageList(imglst, LVSIL_SMALL);
        __super::SetImageList(NULL, LVSIL_SMALL);

        imglst.Destroy();

        if (nHeight)
            m_nItemHeight = nHeight;
    }

    void ShowCheckBox(BOOL bShow)
    {
        if (bShow)
        {
            SetExtendedListViewStyle(LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
            __super::SetImageList(m_imglstCheck, LVSIL_STATE);

        }
        else
        {
            __super::SetImageList(NULL, LVSIL_STATE);
            SetExtendedListViewStyle(0, LVS_EX_CHECKBOXES);
        }
    }

    BOOL GetCheckState(int nItem)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            if (nItem >= GetItemCount() || nItem < 0)
                return FALSE;

            if (nItem >= (int)m_arrItemCheck.GetCount())
                return FALSE;

            return (2 == m_arrItemCheck[nItem]);
        }
        else
            return __super::GetCheckState(nItem);
    }

    BOOL SetCheckState(int nItem, BOOL bCheck, BOOL bForce = FALSE, BOOL bRedraw = TRUE)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            if (nItem >= GetItemCount() || nItem < 0)
                return FALSE;

            if (nItem >= (int)m_arrItemCheck.GetCount())
                m_arrItemCheck.SetCount(GetItemCount());

            if (bForce || 0 != m_arrItemCheck[nItem])
            {
                m_arrItemCheck[nItem] = bCheck ? 2 : 1;

                if (bRedraw)
                    RedrawItems(nItem, nItem);
            }

            return TRUE;
        }
        else
            return __super::SetCheckState(nItem, bCheck);
    }

    BOOL DisableItemCheck(int nItem, BOOL bRedraw = TRUE)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            if (nItem >= GetItemCount() || nItem < 0)
                return FALSE;

            if (nItem >= (int)m_arrItemCheck.GetCount())
                m_arrItemCheck.SetCount(GetItemCount());

            m_arrItemCheck[nItem] = 0;

            if (bRedraw)
                RedrawItems(nItem, nItem);

            return TRUE;
        }
        else
            return __super::SetItemState(nItem, INDEXTOSTATEIMAGEMASK(0), LVIS_STATEIMAGEMASK);
    }

    BOOL IsItemCheckDisabled(int nItem)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            if (nItem >= GetItemCount())
                return FALSE;

            if (nItem >= (int)m_arrItemCheck.GetCount())
                m_arrItemCheck.SetCount(GetItemCount());

            return 0 == m_arrItemCheck[nItem];
        }
        else
            return 0 == (__super::GetItemState(nItem, LVIS_STATEIMAGEMASK) >> 12);
    }

    BOOL IsAllItemChecked()
    {
        ATLASSERT(::IsWindow(m_hWnd));

        int nCount = GetItemCount();
        BOOL bAllCheck = TRUE;

        for (int i = 0; i < nCount; i ++)
        {
            if (!IsItemCheckDisabled(i))
                bAllCheck &= GetCheckState(i);
        }

        return bAllCheck;
    }

    BOOL IsAnyItemChecked()
    {
        ATLASSERT(::IsWindow(m_hWnd));

        int nCount = GetItemCount();

        for (int i = 0; i < nCount; i ++)
        {
            if (!IsItemCheckDisabled(i))
            {
                if (GetCheckState(i)) 
                    return TRUE;
            }
        }

        return FALSE;
    }

    BOOL CheckAllItems(BOOL bCheck, BOOL bForce = FALSE, BOOL bRedraw = TRUE)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        int nCount = GetItemCount();

        for (int i = 0; i < nCount; i ++)
            SetCheckState(i, bCheck, bForce, bRedraw);

        return TRUE;
    }

    void SetImageList(HBITMAP hImageList, int cx, COLORREF crMaskColor = RGB(0xFF, 0, 0xFF))
    {
        m_imgList.Attach(hImageList);
        m_imgList.SetTransparentMode(CBkImage::ModeMaskColor);
        m_imgList.SetMaskColor(crMaskColor);
        m_imgList.SetSubImageWidth(cx);

        SetItemHeight();
    }

    DWORD_PTR GetItemData(int nItem) const
    {
        ATLASSERT(::IsWindow(m_hWnd));

        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            if (nItem >= GetItemCount() || nItem < 0)
                return 0;

            if (nItem >= (int)m_arrItemData.GetCount())
                return 0;

            return m_arrItemData[nItem];
        }
        else
            return __super::GetItemData(nItem);
    }

    BOOL SetItemData(int nItem, DWORD_PTR dwData)
    {
        ATLASSERT(::IsWindow(m_hWnd));

        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            if (nItem >= GetItemCount() || nItem < 0)
                return FALSE;

            if (nItem >= (int)m_arrItemData.GetCount())
                m_arrItemData.SetCount(GetItemCount());

            m_arrItemData[nItem] = dwData;

            return TRUE;
        }
        else
            return __super::SetItemData(nItem, dwData);
    }

    BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
    {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));

        return TRUE;
    }

    DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
    {
        return CDRF_NOTIFYITEMDRAW;
    }

    DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW lpNMCustomDraw)
    {
        LPNMLVCUSTOMDRAW lpcd = (LPNMLVCUSTOMDRAW)lpNMCustomDraw;
        CRect rcItem, rcDraw;
        CBkImage img;
        WTL::CDC dc;
        CString strText;
        BOOL bEnabled = IsWindowEnabled();

        GetItemRect((int)lpcd->nmcd.dwItemSpec, rcItem, LVIR_BOUNDS);

        img.CreateBitmap(rcItem.Width(), rcItem.Height());

        dc.CreateCompatibleDC(lpcd->nmcd.hdc);
        dc.SetWindowOrg(rcItem.TopLeft());

        dc.SetBkMode(TRANSPARENT);

        if (!bEnabled)
            dc.SetTextColor(RGB(0xAA, 0xAA, 0xAA));

        HBITMAP hbmpOld = dc.SelectBitmap(img);

        // Draw Background
//         COLORREF crBg = RGB(0xDD, 0xDD, 0xDD);
//         if (bEnabled)
//             crBg = ((lpcd->nmcd.dwItemSpec & 1) == 1) ? RGB(0xFA, 0xFA, 0xFA) : RGB(0xFF, 0xFF, 0xFF);

        COLORREF crBg = RGB(0xFF, 0xFF, 0xFF);

        if (LVIS_SELECTED == GetItemState((int)lpcd->nmcd.dwItemSpec, LVIS_SELECTED) && bEnabled)
            crBg = RGB(0xB9, 0xDB, 0xFF);

        dc.FillSolidRect(rcItem, crBg);

        GetItemRect((int)lpcd->nmcd.dwItemSpec, rcDraw, LVIR_SELECTBOUNDS);

        //rcDraw = rcItem;

        if (LVS_EX_CHECKBOXES == (LVS_EX_CHECKBOXES & GetExtendedListViewStyle()))
        {
            UINT uCheck = IsItemCheckDisabled((int)lpcd->nmcd.dwItemSpec) ? 0 : (GetCheckState((int)lpcd->nmcd.dwItemSpec) ? 2 : 1);

            SIZE size;
            CRect rcCheck = rcDraw;

            m_imglstCheck.GetIconSize(size);

            rcCheck.top += (rcDraw.Height() - size.cy) / 2;
            rcCheck.left += 2;

            rcDraw.left += size.cx;

            m_imglstCheck.Draw(dc, uCheck, rcCheck.TopLeft(), ILD_TRANSPARENT);
        }

        {
            CPen penLine;

            penLine.CreatePen(
                PS_SOLID, 
                1, 
                RGB(0xEA, 0xEA, 0xEA)
                );
            HPEN hpenOld = dc.SelectPen(penLine);
            dc.MoveTo(rcItem.left, rcItem.bottom - 1);
            dc.LineTo(rcItem.right, rcItem.bottom - 1);
            dc.SelectPen(hpenOld);
        }

/*
        if (LVIS_SELECTED == GetItemState((int)lpcd->nmcd.dwItemSpec, LVIS_SELECTED) && bEnabled)
        {
            CWHHeaderCtrl::FRG_PARAM frgSelBg[2] = {
                {0, RGB(242, 247, 252)}, 
                {rcDraw.Height() - 2, RGB(231, 235, 242)}, 
            };
            CPen penFrame;

            penFrame.CreatePen(
                PS_SOLID, 
                1, 
                RGB(209, 214, 221)
                );

            HPEN hpenOld = dc.SelectPen(penFrame);
            HBRUSH hbshOld = NULL, hbshNull = (HBRUSH)::GetStockObject(NULL_BRUSH);

            hbshOld = dc.SelectBrush(hbshNull);

            rcDraw.left ++;
            dc.RoundRect(rcDraw, CPoint(2, 2));

            rcDraw.DeflateRect(1, 1);

            m_hdrListVirus.GradientFillRectV(dc, rcDraw, frgSelBg, 2);

            dc.SelectBrush(hbshOld);
            dc.SelectPen(hpenOld);
        }
*/

        if (!m_imgList.IsNull())
        {
            LVITEM li = {LVIF_IMAGE, (int)lpcd->nmcd.dwItemSpec};
            if (GetItem(&li) && li.iImage != -1)
            {
                GetItemRect((int)lpcd->nmcd.dwItemSpec, rcDraw, LVIR_ICON);
                SIZE sizeImg;
                m_imgList.GetImageSize(sizeImg);
                m_imgList.Draw(dc, rcDraw.left - 2, rcDraw.top + (rcDraw.Height() - sizeImg.cy) / 2, li.iImage);
            }
        }

        // Draw Sub Items

        HFONT hftOld = dc.SelectFont(BkFontPool::GetFont(BKF_DEFAULTFONT));

        for (int nSubItem = 0; nSubItem < GetHeader().GetItemCount(); nSubItem ++)
        {
            GetSubItemRect((int)lpcd->nmcd.dwItemSpec, nSubItem, LVIR_LABEL, rcDraw);
            UINT uFormat = DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER;

            GetItemText((int)lpcd->nmcd.dwItemSpec, nSubItem, strText);

            LVCOLUMN colInfo = {LVCF_FMT};

            GetColumn(nSubItem, &colInfo);

            if (colInfo.fmt & LVCFMT_CENTER)
                uFormat |= DT_CENTER;
            else if (colInfo.fmt & LVCFMT_RIGHT)
                uFormat |= DT_RIGHT;

			if ( 2 == nSubItem )
			{
				if ( BkString::Get(IDS_IEFIX_6110) == strText )
				{
					//lpcd->clrText = RGB(255,0,0);
					dc.SetTextColor( RGB(255,0,0) );
				}
				else if ( BkString::Get(IDS_IEFIX_6111) == strText )
				{
					dc.SetTextColor( RGB(0,129,0) );
				}
			}

            dc.DrawText(strText, strText.GetLength(), rcDraw, uFormat);
        }

        dc.SelectFont(hftOld);
        dc.SelectBitmap(hbmpOld);

        img.Draw(lpcd->nmcd.hdc, rcItem.left, rcItem.top);

        img.DeleteObject();

        return CDRF_SKIPDEFAULT;
    }

    LRESULT OnLVNHeaderItemClick(LPNMHDR pnmh)
    {
        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            LPNMHEADER p = (LPNMHEADER)pnmh;
            if (p->iButton == 0)
            {
                int nOld = m_nSortColumn;

                BKUNFLYLVNITEMSORT nms;
                nms.hdr.code = BKUNFLYLVN_ITEMSORT;
                nms.hdr.hwndFrom = m_hWnd;
                nms.hdr.idFrom = GetDlgCtrlID();
                nms.bDescending = (m_nSortColumn == p->iItem) ? !m_bSortDescending : FALSE;
                nms.nCol = p->iItem;

                LRESULT lRet = ::SendMessage(GetParent(), WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);

                if (lRet)
                {
                    m_nSortColumn = p->iItem;
                    m_bSortDescending = nms.bDescending;

                    HDITEM hdi = {HDI_FORMAT};
                    m_hdrListVirus.GetItem(nOld, &hdi);
                    hdi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
                    m_hdrListVirus.SetItem(nOld, &hdi);
                    m_hdrListVirus.GetItem(m_nSortColumn, &hdi);
                    hdi.fmt |= m_bSortDescending ? HDF_SORTUP : HDF_SORTDOWN;
                    m_hdrListVirus.SetItem(m_nSortColumn, &hdi);
                }
            }
        }

        SetMsgHandled(FALSE);

        return 0;
    }

    BOOL OnEraseBkgnd(CDCHandle dc)
    {
        int nTop = GetTopIndex();
        int nBottom = nTop + GetCountPerPage();

        nBottom = min(nBottom, GetItemCount() - 1);

        CRect rcClient, rcItemTop, rcItemBottom;
        COLORREF crBg = IsWindowEnabled() ? RGB(0xFF, 0xFF, 0xFF) : RGB(0xDD, 0xDD, 0xDD);

        GetClientRect(rcClient);
        GetItemRect(nTop, rcItemTop, LVIR_BOUNDS);
        GetItemRect(nBottom, rcItemBottom, LVIR_BOUNDS);

        dc.FillSolidRect(rcItemBottom.right, rcItemTop.top, rcClient.right - rcItemBottom.right, rcClient.bottom - rcItemTop.top, crBg);
        dc.FillSolidRect(rcClient.left, rcItemBottom.bottom, rcItemBottom.Width(), rcClient.bottom - rcItemBottom.bottom, crBg);

        return TRUE;
    }

    LRESULT OnLVNKeyDown(LPNMHDR pnmh)
    {
        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            LPNMLVKEYDOWN pnms = (LPNMLVKEYDOWN)pnmh;

            if (VK_SPACE == pnms->wVKey)
            {
                int nCurSel = GetSelectedIndex();
                if (-1 != nCurSel)
                {
                    BOOL bCheck = !GetCheckState(nCurSel);
                    SetCheckState(nCurSel, bCheck);
                    _SendItemCheckStateChangedNotify(nCurSel, bCheck);
                }
            }
        }
        else
            SetMsgHandled(FALSE);

        return 0;
    }

    LRESULT OnNMClick(LPNMHDR pnmh)
    {
        if (LVS_OWNERDATA == (LVS_OWNERDATA & GetStyle()))
        {
            LPNMLISTVIEW pnms = (LPNMLISTVIEW)pnmh;
            LVHITTESTINFO hitinfo;

            hitinfo.pt = pnms->ptAction;

            int nItem = HitTest(&hitinfo); 

            if(nItem != -1)
            {
                if( (hitinfo.flags & LVHT_ONITEM) == LVHT_ONITEMSTATEICON)
                {
                    BOOL bCheck = !GetCheckState(nItem);
                    SetCheckState(nItem, bCheck);
                    _SendItemCheckStateChangedNotify(nItem, bCheck);
                }
            }
        }
        else
            SetMsgHandled(FALSE);

        return 0;
    }

    void _SendItemCheckStateChangedNotify(int nItem, BOOL bCheck)
    {
        NMLISTVIEW nms = {0};

        nms.hdr.idFrom = GetDlgCtrlID();
        nms.hdr.hwndFrom = m_hWnd;
        nms.hdr.code = LVN_ITEMCHANGED;

        nms.iItem = nItem;
        nms.iSubItem = 0;
        nms.uOldState = INDEXTOSTATEIMAGEMASK(bCheck ? 1 : 2);
        nms.uNewState = INDEXTOSTATEIMAGEMASK(bCheck ? 2 : 1);

        ::SendMessage(::GetParent(m_hWnd), WM_NOTIFY, (LPARAM)nms.hdr.idFrom, (WPARAM)&nms);
    }

protected:

    CWHHeaderCtrl m_hdrListVirus;
    CWH3StateCheckImageList m_imglstCheck;
    CBkImage m_imgList;
    int m_nSortColumn;
    int m_nItemHeight;
    BOOL m_bSortDescending;
    CAtlArray<DWORD_PTR> m_arrItemData;
    CAtlArray<int> m_arrItemCheck;

    BEGIN_MSG_MAP_EX(CBkUnflyListViewCtrl)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_SETCURSOR(OnSetCursor)
        NOTIFY_CODE_HANDLER_EX(HDN_ITEMCLICKA, OnLVNHeaderItemClick)
        NOTIFY_CODE_HANDLER_EX(HDN_ITEMCLICKW, OnLVNHeaderItemClick)
        REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_KEYDOWN, OnLVNKeyDown)
        REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_CLICK, OnNMClick)
        REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnNMClick)
        CHAIN_MSG_MAP_ALT(CCustomDraw<CBkUnflyListViewCtrl>, 1)
        DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()
};