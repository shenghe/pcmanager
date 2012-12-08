#ifndef KCLEAR_EXTLIST_H_
#define KCLEAR_EXTLIST_H_

//////////////////////////////////////////////////////////////////////////

#include "kscgui/kdraw.h"

//////////////////////////////////////////////////////////////////////////

typedef struct tagExtEntry {
    std::wstring name;
    std::wstring description;
    bool checked;
} ExtEntry;

//////////////////////////////////////////////////////////////////////////

#include "kclearmsg.h"

//////////////////////////////////////////////////////////////////////////

class CExtListHdr;
typedef CWindowImpl<CExtListHdr, CHeaderCtrl> CExtListHdrBase;

class CExtListHdr 
    : public CExtListHdrBase
    , public CDoubleBufferImpl<CExtListHdr>
{
public:
    BEGIN_MSG_MAP(CExtListHdr)
        MSG_WM_SIZE(OnSize)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
        MESSAGE_HANDLER(HDM_LAYOUT, OnHdmLayout)
        REFLECTED_NOTIFY_CODE_HANDLER(HDN_ENDTRACK, OnEndTrack)
        CHAIN_MSG_MAP(CDoubleBufferImpl<CExtListHdr>)
    END_MSG_MAP()

    void Init()
    {
        GetClientRect(&m_rcClient);
    }

    BOOL SubclassWindow(HWND hWnd)
    {
        BOOL retval = FALSE;
        retval = CExtListHdrBase::SubclassWindow(hWnd);

        Init();
        return retval;
    }

    LRESULT OnEndTrack(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
    {
        GetParent().InvalidateRect(NULL);
        return 0L;
    }

    LRESULT OnHdmLayout(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LRESULT retval;
        HDLAYOUT* pHdLayout = (HDLAYOUT*)lParam;

        retval = DefWindowProc(uMsg, wParam, lParam);
        if (pHdLayout)
        {
            pHdLayout->pwpos->cy = 0;
            pHdLayout->prc->top = 0;
            pHdLayout->prc->bottom -= 12;
        }

        return 0;
    }

    LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = TRUE;
        return 0;
    }

    void DoPaint(CDCHandle dc)
    {
        CRect rcTemp;
        CFontHandle font = KuiFontPool::GetFont(FALSE, FALSE, FALSE);
        CRect rcSplit, rcHalf;
        int nItemCount = GetItemCount();

        GetClientRect(&m_rcClient);
        rcTemp = m_rcClient;
        rcTemp.bottom -= 1;
        rcHalf = rcTemp;
        rcHalf.bottom = rcTemp.Height() / 2;
        DrawGradualColorRect(dc, rcHalf, RGB(255,255,255), RGB(248,253,255));
        rcHalf.top = rcHalf.bottom;
        rcHalf.bottom = m_rcClient.bottom - 1;
        DrawGradualColorRect(dc, rcHalf, RGB(229,246,254), RGB(238,249,255));
        rcTemp.top = m_rcClient.bottom - 1;
        rcTemp.bottom = m_rcClient.bottom;
        dc.FillSolidRect(rcTemp, RGB(150,174,202));

        dc.SetBkMode(TRANSPARENT);
        dc.SetTextColor(RGB(0,0,0));
        dc.SelectFont(font);
        for (int i = 0; i < nItemCount; ++i)
        {
            HDITEM hdi;
            CString strText;

            hdi.mask = HDI_TEXT;
            hdi.pszText = strText.GetBuffer(MAX_PATH);
            hdi.cchTextMax = MAX_PATH;

            GetItem(i, &hdi);
            strText.ReleaseBuffer();
            GetItemRect(i, rcTemp);

            dc.DrawText(strText, strText.GetLength(), rcTemp, 
                DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

            rcSplit.left = rcTemp.right - 2;
            rcSplit.right = rcTemp.right - 1;
            rcSplit.top = rcTemp.top + 4;
            rcSplit.bottom = rcTemp.bottom - 3;
            dc.FillSolidRect(rcSplit, RGB(162,186,212));
            rcSplit.left = rcTemp.right - 1;
            rcSplit.right = rcTemp.right - 0;
            dc.FillSolidRect(rcSplit, RGB(255,255,255));
        }
    }

    void OnSize(UINT nType, CSize size)
    {
        GetClientRect(&m_rcClient);
    }

private:
    RECT m_rcClient;
};

//////////////////////////////////////////////////////////////////////////

class CExtListCtrl;
typedef CWindowImpl<CExtListCtrl, CListViewCtrl> CExtListCtrlBase;

class CExtListCtrl 
    : public CExtListCtrlBase
    , public COwnerDraw<CExtListCtrl>
{
public:
    CExtListCtrl(std::vector<ExtEntry>& extList) : m_extList(extList)
    {
        m_nItemHeight = 24;
        m_nHeaderHeight = 0;
    }

    ~CExtListCtrl()
    {

    }

    void Init()
    {
        m_bmpChkBox = KAppRes::Instance().GetImage("IDB_BITMAP_LISTCTRL_CHECK");

        m_wndToolTip.Create(m_hWnd);
        m_wndToolTip.SetMaxTipWidth(400);
        m_wndToolTip.AddTool(m_hWnd);
        m_wndToolTip.Activate(FALSE);
        m_wndToolTip.SetDelayTime(0, 1000);

        ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
        m_hdr.SubclassWindow(GetHeader());
        m_hdr.SetDlgCtrlID(1000);
    }

    BOOL SubclassWindow(HWND hWnd)
    {
        BOOL retval = FALSE;
        retval = CExtListCtrlBase::SubclassWindow(hWnd);
        Init();
        return retval;
    }

    // ×Ô»æ
    void DoDrawItem(CDCHandle dc, LPDRAWITEMSTRUCT lpDrawItemStruct)
    {
        CRect rcItem = lpDrawItemStruct->rcItem;
        CRect rcName;
        CString strText;
        int nItem = lpDrawItemStruct->itemID;
        BOOL bSelect = lpDrawItemStruct->itemState & ODS_SELECTED;
        bool bChecked = m_extList[nItem].checked;
        BOOL bEnable = IsWindowEnabled();

        // »æÖÆ±³¾°
        {
            CRect rcDraw = rcItem;
            CRect rcClient;
            GetClientRect(rcClient);
            if (rcClient.right > rcDraw.right)
                rcDraw.right = rcClient.right;
            dc.FillSolidRect(rcDraw, bEnable ? RGB(0xff,0xff,0xff) : RGB(0xf0, 0xf0, 0xf0));
        }

        if (bSelect && bEnable)
        {
            CPen penBorder;
            CBrush brushBk;
            CRect rcSelect = rcItem;

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

        if (GetColumnWidth(0) > 22)
        {
            CDC memdc;
            CRect rcHot;
            memdc.CreateCompatibleDC(dc);
            memdc.SelectBitmap(m_bmpChkBox);

            dc.BitBlt(rcItem.left + 4, rcItem.top + (m_nItemHeight - 13) / 2, 
                13, 13, memdc, bChecked ? 0 : 13, 0, SRCCOPY);
            rcHot.left = rcItem.left;
            rcHot.top = 0;
            rcHot.right = rcHot.left + 21;
            rcHot.bottom = m_nItemHeight;
            m_rcCheckBox = rcHot;
        }

        rcName = rcItem;
        rcName.right = rcName.left + GetColumnWidth(0);
        rcName.left += 22;

        if (rcName.right > rcName.left)
        {
            GetItemText(nItem, 0, strText);
            dc.SelectFont(GetFont());
            dc.SetTextColor(bEnable ? RGB(0,0,0) : RGB(0x9b, 0x9b, 0x9b));
            dc.DrawText(strText, strText.GetLength(), rcName, 
                DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
        }

        rcName.left = rcName.right;
        rcName.right = rcName.left + GetColumnWidth(1);
        GetItemText(nItem, 1, strText);
        dc.DrawText(strText, strText.GetLength(), rcName, 
            DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
    }

    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
    {
        DoDrawItem(lpDrawItemStruct->hDC, lpDrawItemStruct);
    }

    void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
    {
        lpMeasureItemStruct->itemHeight = m_nItemHeight;
    }

    int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
    {
        return 0;
    }

    void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
    {
        // NOTHING;
    }

    BOOL OnEraseBkgnd(CDCHandle dc)
    {
        CRect rcClient;
        BOOL bEnable;

        bEnable = IsWindowEnabled();
        GetClientRect(rcClient);
        dc.FillSolidRect(rcClient, bEnable ? RGB(0xff, 0xff, 0xff) : RGB(0xf0, 0xf0, 0xf0));
        return TRUE;
    }

    BOOL DeleteItem(int nItem);

    BEGIN_MSG_MAP_EX(CExtListCtrl)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_ENABLE(OnEnable)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnToolTipEvent)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        CHAIN_MSG_MAP_ALT(COwnerDraw<CExtListCtrl>,1)
        END_MSG_MAP()

protected:
    void OnLButtonDown(UINT nFlags, CPoint point)
    {
        int nItem;
        
        if (IsMouseInCheckBox(point, nItem))
        {
            m_extList[nItem].checked = !m_extList[nItem].checked;
			if (_IsNoOneChecked())
			{
				//
				::SendMessage(GetParent().GetParent().m_hWnd, WM_NONE_CHECKED, NULL, NULL);
			}
            InvalidateRect(NULL);
        }

        SetMsgHandled(FALSE);
    }

    void OnMouseMove(UINT nFlags, CPoint point)
    {
        
        SetMsgHandled(FALSE);
    }

    //void OnLButtonUp(UINT nFlags, CPoint point);
    //LRESULT OnNMClick(LPNMHDR pnmh);
    //LRESULT OnNMDClick(LPNMHDR pnmh);
    //void OnVScroll(UINT nSBCode, UINT nPos, HWND hWnd);
    //LRESULT OnMouseWhell(UINT fwKey, short zDela, CPoint pt);
    BOOL OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
    {

    }

    //LRESULT OnReflectedNotifyRangeHandlerEX(LPNMHDR pnmh);
    LRESULT OnMouseHover(WPARAM wParam, CPoint ptPos)
    {

    }

    LRESULT OnMouseLeave()
    {
        return 0;
    }

    LRESULT OnToolTipEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (m_wndToolTip.IsWindow())
        {
            MSG msg = { m_hWnd, uMsg, wParam, lParam };

            m_wndToolTip.RelayEvent(&msg);
        }

        SetMsgHandled(FALSE);

        return 0;
    }

	LRESULT OnEnable(WPARAM wParam)
	{
		BOOL bEnable = (BOOL) wParam;
		
		if (bEnable)
		{
			if (_IsNoOneChecked() && m_extList.size() > 0)
			{
				m_extList[0].checked = TRUE;
			}
		}
		SetMsgHandled(FALSE);
		return 0;
	}

	BOOL _IsNoOneChecked()
	{
		BOOL bRet = FALSE;
		int i;

		for (i = 0; i < (int)m_extList.size(); ++i)
		{
			if (m_extList[i].checked)
			{
				goto clear0;
			}
		}
		bRet = TRUE;	
clear0:
		return bRet;
	}

    BOOL IsMouseInCheckBox(CPoint pt, int& nItem)
    {
        BOOL retval = FALSE;
        CPoint pt2 = pt;

        nItem = HitTest(pt);
        if (-1 == nItem)
            goto clean0;

        pt2.y = (pt2.y - m_nHeaderHeight) % m_nItemHeight;
        if (PtInRect(m_rcCheckBox, pt2))
        {
            retval = TRUE;
            //SetCursor(LoadCursor(NULL, IDC_HAND));
            goto clean0;
        }

clean0:
        return retval;
    }

    int HitTest(CPoint pt)
    {
        LVHITTESTINFO hitinfo;

        hitinfo.pt = pt;
        return __super::HitTest(&hitinfo);
    }

protected:
    CToolTipCtrl m_wndToolTip;
    CRect m_rcCheckBox;
    int m_nItemHeight;
    int m_nHeaderHeight;
    CExtListHdr m_hdr;
    int m_nHotItem;
    int m_nHotLink;
    CString m_strTip;
    std::vector<ExtEntry>& m_extList;
    CBitmapHandle m_bmpChkBox;
};

//////////////////////////////////////////////////////////////////////////

#endif // KCLEAR_EXTLIST_H_
