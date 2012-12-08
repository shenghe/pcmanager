//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "vulfix/BeikeUtils.h"
#include "dlgtipdetail.h"
#include "bkmsgdefine.h"
//////////////////////////////////////////////////////////////////////////

CDlgTipDetail::CDlgTipDetail(void)
{
    m_nCloseState = -1;
}

CDlgTipDetail::~CDlgTipDetail(void)
{
}
//////////////////////////////////////////////////////////////////////////
void CDlgTipDetail::Initilize(HWND hNotifyHwnd, T_VulListItemData* pItemData, CPoint popPoint)
{
    m_hNotifyHwnd = hNotifyHwnd;
    m_pItemData = pItemData;

    ::MoveWindow(m_hWnd, popPoint.x, popPoint.y + 10, DEF_VUL_TIP_DETAIL_WIDTH,_CountHeight(),TRUE);
    
    GetClientRect(&m_rcClient);
    m_rcHead.SetRect(0,0,DEF_VUL_TIP_DETAIL_WIDTH, 60);
    m_rcCloseBtn.SetRect(DEF_VUL_TIP_DETAIL_WIDTH - 17, 10,DEF_VUL_TIP_DETAIL_WIDTH - 8, 19);
}
//////////////////////////////////////////////////////////////////////////
LRESULT CDlgTipDetail::OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/)
{
    return TRUE;
}
LRESULT CDlgTipDetail::OnEraseBkgnd(CDCHandle dc)
{
    return TRUE;
}
void CDlgTipDetail::OnPaint(CDCHandle dc)
{
    PAINTSTRUCT ps;
    HDC hDC = ::BeginPaint( m_hWnd, &ps );
    Gdiplus::Graphics grap(hDC);
    Gdiplus::Image *pImg = NULL;
    CDC memDC(hDC);
    int nDownHeightPos = m_rcClient.Height() - 27;

    memDC.SetBkMode(TRANSPARENT);

    pImg = BkPngPool::Get(IDP_VUL_TIP_HEAD);
    grap.DrawImage(pImg, 0,0);

    pImg = BkPngPool::Get(IDP_VUL_TIP_DOWN);
    grap.DrawImage(pImg, 0, nDownHeightPos);

    pImg = BkPngPool::Get(IDP_VUL_TIP_CLOSE);
    if (m_nCloseState == -1)
    {
        grap.DrawImage(pImg, m_rcCloseBtn.left, m_rcCloseBtn.top);
    }
    CRect rcGrant(0,84, DEF_VUL_TIP_DETAIL_WIDTH, nDownHeightPos);
    _DrawGradualColorRect(memDC, rcGrant,RGB(0xE4, 0xF0, 0xFE), RGB(0xD7, 0xE6, 0xF9), TRUE);
    
    _DrawLine(memDC, CPoint(0,0), CPoint(0,m_rcClient.Height()), RGB(0x87, 0x98, 0xA8));
    
    _DrawLine(memDC, CPoint(DEF_VUL_TIP_DETAIL_WIDTH - 1, 0), CPoint(DEF_VUL_TIP_DETAIL_WIDTH - 1, m_rcClient.Height()), RGB(0x87, 0x98, 0xA8));

    HFONT hHeadFont = BkFontPool::GetFont(TRUE, FALSE, FALSE, 1);
    HFONT hNormalFont = BkFontPool::GetFont(FALSE, FALSE, FALSE);
    CString strTitle;
    CString strTmp = BkString::Get(IDS_VULFIX_5019);
    FormatKBString(m_pItemData->nID, strTitle);
    strTitle =  strTmp + _T(":") + strTitle;
    CRect rcText(13, 10, 220, 25);
    CRect rcTemp;
    _DrawText(memDC, rcText, strTitle, RGB(0x0, 0x0, 0x0), hHeadFont);

    rcText.SetRect(15, 32, 200, 47);
    strTmp = BkString::Get(IDS_VULFIX_5021);
    strTitle = strTmp + _T(":") + m_pItemData->strPubDate;
    _DrawText(memDC, rcText, strTitle, RGB(0x44, 0x61, 0x83), hNormalFont);
    
    // 大小
    rcText.SetRect(275,32,DEF_VUL_TIP_DETAIL_WIDTH,47);
    FormatSizeString(m_pItemData->nFileSize, strTitle);
    strTmp = BkString::Get(IDS_VULFIX_5124);
    strTitle = strTmp + strTitle;
    _DrawText(memDC, rcText, strTitle, RGB(0x44, 0x61, 0x83), hNormalFont);
 
    // 绘制小点 和 描述文字
    pImg = BkPngPool::Get(IDP_VUL_TIP_POINT);
    CRect rcContent(15, 72, DEF_VUL_TIP_DETAIL_WIDTH, 72 + 15);
    
    if (!m_pItemData->strMSID.IsEmpty())
    {
        grap.DrawImage(pImg, rcContent.left, rcContent.top + 5);
        strTmp = BkString::Get(IDS_VULFIX_5123);
        strTmp += m_pItemData->strMSID;
        rcContent.left += 10;
        _DrawText(memDC, rcContent, strTmp, RGB(0,0,0), hNormalFont);
        rcContent.top += 17;
        rcContent.bottom += 17;
        rcContent.left = 15;
        rcContent.right = DEF_VUL_TIP_DETAIL_WIDTH;
    }

    grap.DrawImage(pImg, rcContent.left, rcContent.top + 5);
    rcContent.left += 10;
    strTmp = BkString::Get(IDS_VULFIX_5125);
    _DrawText(memDC, rcContent, strTmp, RGB(0,0,0), hNormalFont);

    rcContent.left = rcContent.right;
    rcContent.right = DEF_VUL_TIP_DETAIL_WIDTH;
    CString strAff = m_pItemData->strAffects;
    if (strAff.GetLength() < 25)
    {
        _DrawText(memDC, rcContent, strAff, RGB(255, 0, 0), hNormalFont);
    }
    else
    {
        strTmp = strAff.Mid(0, 25);
        _DrawText(memDC, rcContent, strTmp, RGB(255, 0, 0), hNormalFont);
        strAff = strAff.Mid(25);

        while(1)
        {
            if (strAff.GetLength() > 0)
            {
                int len = strAff.GetLength();
                rcContent.top += 17;
                rcContent.bottom += 17;
                rcContent.left = 25;
                rcContent.right = DEF_VUL_TIP_DETAIL_WIDTH;
                if (strAff.GetLength() > 30)
                    len = 30;
                strTmp = strAff.Mid(0, len);
                _DrawText(memDC, rcContent, strTmp, RGB(255, 0, 0), hNormalFont);
                strAff = strAff.Mid(len);
            }
            else
            {
                break;
            }
        }
    }

    // 描述

    rcContent.top += 17;
    rcContent.bottom += 17;
    rcContent.left = 15;
    rcContent.right = DEF_VUL_TIP_DETAIL_WIDTH;

    grap.DrawImage(pImg, rcContent.left, rcContent.top + 5);
    rcContent.left += 10;
    strTmp = BkString::Get(IDS_VULFIX_5172);
    _DrawText(memDC, rcContent, strTmp, RGB(0,0,0), hNormalFont);

    rcContent.left = rcContent.right;
    rcContent.right = DEF_VUL_TIP_DETAIL_WIDTH;
    CString strDes = m_pItemData->strDesc;
    if (strDes.GetLength() < 25)
    {
        _DrawText(memDC, rcContent, strDes, RGB(0, 0, 0), hNormalFont);
    }
    else
    {
        HFONT oldFont = memDC.SelectFont(hNormalFont);;
        int len = 25;
        strTmp = strDes.Mid(0, len);
        int nCount = 0;
        while(1)
        {
            CSize nSize;
            memDC.GetTextExtent(strTmp, -1, &nSize);
            if (rcContent.left + nSize.cx < DEF_VUL_TIP_DETAIL_WIDTH - 30)
            {
                if (len < strDes.GetLength())
                {
                    ++len;
                    strTmp = strDes.Mid(0, len);
                }
                else
                {
                    break;
                }
            }
            else break;
        }
        _DrawText(memDC, rcContent, strTmp, RGB(0, 0, 0), hNormalFont);
        strDes = strDes.Mid(len);

        while(1)
        {
            if (nCount >= 2)
            {
                break;
            }
            if (strDes.GetLength() > 0)
            {
                int len = strDes.GetLength();
                rcContent.left = 25;
                rcContent.right = DEF_VUL_TIP_DETAIL_WIDTH;
                if (strDes.GetLength() > 30)
                    len = 30;
                else 
                    break;
                strTmp = strDes.Mid(0, len);
                // 文字可能是混合的，从个数到计算长度来确定本行的个数。
                while(1)
                {
                    CSize nSize;
                    memDC.GetTextExtent(strTmp, -1, &nSize);
                    if (rcContent.left + nSize.cx < DEF_VUL_TIP_DETAIL_WIDTH - 30)
                    {
                        if (len < strDes.GetLength())
                        {
                            ++len;
                            strTmp = strDes.Mid(0, len);
                        }
                        else
                        {
                            break;
                        }
                    }
                    else break;
                }

                if (len == strDes.GetLength())
                    break;
                rcContent.top += 17;
                rcContent.bottom += 17;
                
                _DrawText(memDC, rcContent, strTmp, RGB(0, 0, 0), hNormalFont);
                strDes = strDes.Mid(len);
            }
            else
            {
                break;
            }
            nCount ++;
        }
        memDC.SelectFont(oldFont);
    }
    CSize nSize;
    rcContent.top += 17;
    rcContent.bottom += 17;
    rcContent.left = 25;
    rcContent.right = DEF_VUL_TIP_DETAIL_WIDTH;
    memDC.GetTextExtent(strDes, -1, &nSize);
    if (rcContent.left + nSize.cx > DEF_VUL_TIP_DETAIL_WIDTH - 90)
        rcContent.right = DEF_VUL_TIP_DETAIL_WIDTH - 90;
    _DrawText(memDC, rcContent, strDes, RGB(0, 0, 0), hNormalFont);

    rcContent.left = rcContent.right;
    rcContent.right = DEF_VUL_TIP_DETAIL_WIDTH;
    _DrawText(memDC, rcContent, BkString::Get(IDS_VULFIX_5128), RGB(0x33, 0x62, 0x9A), BkFontPool::GetFont(FALSE, TRUE ,FALSE));
    ::CopyRect(m_rcDetail, rcContent);
    
    // draw link
    HFONT hLinkFont = BkFontPool::GetFont(FALSE, TRUE, FALSE);
    CRect rcLink;
    COLORREF clrLink = RGB(0x33, 0x62, 0x9A);
    
    rcLink.SetRect(15, m_rcClient.Height() - 20, 300, m_rcClient.Height() - 5);
    _DrawText(memDC, rcLink, BkString::Get(IDS_VULFIX_5170), clrLink, hLinkFont);
    ::CopyRect(m_rcQuestion, rcLink);

    rcLink.left = 250;
    rcLink.right = DEF_VUL_TIP_DETAIL_WIDTH;
    _DrawText(memDC, rcLink, BkString::Get(IDS_VULFIX_5129), clrLink, hLinkFont);
    ::CopyRect(m_rcOfficial, rcLink);

    rcLink.left = 325;
    rcLink.right = DEF_VUL_TIP_DETAIL_WIDTH;
    _DrawText(memDC, rcLink, BkString::Get(IDS_VULFIX_5171), clrLink, hLinkFont);
    ::CopyRect(m_rcIgnoreVul, rcLink);

	::EndPaint( m_hWnd, &ps );
}
void CDlgTipDetail::OnMouseMove(UINT nFlags, CPoint point)
{

}
void CDlgTipDetail::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_rcCloseBtn.PtInRect(point))
    {
        ::PostMessage(m_hNotifyHwnd, MSG_USER_VUL_TIP_DESTORY, NULL, NULL);
    }
    else if (m_rcDetail.PtInRect(point))
    {
        if(!m_pItemData->strWebPage.IsEmpty())
            ShellExecute(NULL, _T("open"), m_pItemData->strWebPage, NULL, NULL, SW_SHOW);
    }
    else if (m_rcIgnoreVul.PtInRect(point))
    {
        ::PostMessage(m_hNotifyHwnd, MSG_USER_VUL_IGNORE, (WPARAM)m_pItemData->nID, NULL);
        ::PostMessage(m_hNotifyHwnd, MSG_USER_VUL_TIP_DESTORY, NULL, NULL);
    }
    else if (m_rcOfficial.PtInRect(point))
    {
        if(!m_pItemData->strDownloadUrl.IsEmpty())
            ShellExecute(NULL, _T("open"), m_pItemData->strDownloadUrl, NULL, NULL, SW_SHOW);
    }
    else if (m_rcQuestion.PtInRect(point))
    {
        CString strName;
        LPCTSTR pslash = _tcsrchr(m_pItemData->strDownloadUrl, _T('/'));
        if(pslash && _tcslen(pslash)>0)
        {
            strName = ++pslash;
        }
        CString strUrl;
        strUrl.Format(_T("http://www.ijinshan.com/safe/leak_help.html?fr=client&kb=%s"), strName);
        ::ShellExecute(NULL,_T("open"), strUrl, NULL, NULL, SW_SHOW);
    }


    if (m_rcHead.PtInRect(point))
    {
        PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
    }
}
void CDlgTipDetail::OnKillFocus(HWND hWnd)
{
    ::PostMessage(m_hNotifyHwnd, MSG_USER_VUL_TIP_DESTORY, NULL, NULL);
}
BOOL CDlgTipDetail::OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
{
    CPoint pt;
    BOOL bLink = FALSE;
    GetCursorPos( &pt );
    ScreenToClient( &pt );

    bLink = _PtInLink(pt);
    ::SetCursor(::LoadCursor(NULL, bLink?IDC_HAND:IDC_ARROW));
    return TRUE;
}
//////////////////////////////////////////////////////////////////////////
int CDlgTipDetail::_CountHeight()
{
    int nHeight = DEF_VUL_TIP_DETAIL_HEIGHT;
    
    int nAffLen = m_pItemData->strAffects.GetLength() + 5;
    int nDesLen = m_pItemData->strDesc.GetLength() + 5;
    int nCount = -1;

    nCount += nAffLen / 30;
    nCount += nDesLen / 30;

    if (nCount > 0)
    {
        if (nCount > 5)
            nCount = 5;
        nHeight += nCount * 20;
    }
    return nHeight;
}
BOOL CDlgTipDetail::_PtInLink(const CPoint pt)
{
    BOOL bRet = FALSE;
    if (m_rcCloseBtn.PtInRect(pt))
    {
        bRet = TRUE;
        goto Clear0;
    }
    else if (m_rcQuestion.PtInRect(pt))
    {
        bRet = TRUE;
    }
    else if (m_rcOfficial.PtInRect(pt))
    {
        bRet = TRUE;
    }
    else if (m_rcIgnoreVul.PtInRect(pt))
    {
        bRet = TRUE;
    }
    else if (m_rcDetail.PtInRect(pt))
    {
        bRet = TRUE;
    }
Clear0:
    return bRet;
}
void CDlgTipDetail::_DrawGradualColorRect(HDC dc, const RECT& drawRC, 
                                          COLORREF clrFirst,
                                          COLORREF clrEnd,
                                          BOOL fVertical)
{
    if (fVertical)
    {
        GRADIENT_RECT gRect = {0, 1};

        TRIVERTEX vert[2] = {
            {drawRC.left , drawRC.top   , 0, 0, 0, 0}, 
            {drawRC.right, drawRC.bottom, 0, 0, 0, 0} };

            vert[0].Red     = GetRValue(clrFirst) << 8;
            vert[0].Green   = GetGValue(clrFirst) << 8;
            vert[0].Blue    = GetBValue(clrFirst) << 8;
            vert[1].Red     = GetRValue(clrEnd) << 8;
            vert[1].Green   = GetGValue(clrEnd) << 8;
            vert[1].Blue    = GetBValue(clrEnd) << 8;

            ::GradientFill(dc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
    }
    else
    {
        GRADIENT_RECT gRect = {0, 1};

        TRIVERTEX vert[2] = {
            {drawRC.left , drawRC.top   , 0, 0, 0, 0}, 
            {drawRC.right, drawRC.bottom, 0, 0, 0, 0} };

            vert[0].Red     = GetRValue(clrFirst) << 8;
            vert[0].Green   = GetGValue(clrFirst) << 8;
            vert[0].Blue    = GetBValue(clrFirst) << 8;
            vert[1].Red     = GetRValue(clrEnd) << 8;
            vert[1].Green   = GetGValue(clrEnd) << 8;
            vert[1].Blue    = GetBValue(clrEnd) << 8;

            ::GradientFill(dc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
    }
}

void CDlgTipDetail::_DrawLine(CDC& memDC, CPoint ptStart, CPoint ptEnd, 
                              COLORREF color, DWORD dwPenTyple /*=PS_SOLID*/)
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

void CDlgTipDetail::_DrawText(CDC& memDC, CRect& rcItem, 
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
    if(rcText.right - 15 >= rcText.left + sizetemp.cx)
        rcText.right = rcText.left + sizetemp.cx;
    else 
        rcText.right = rcText.right - 15;


    //	rcText.OffsetRect(0,6);
    memDC.DrawText(pszText, (int)_tcslen(pszText), rcText, dwFormat);
    memDC.SelectFont(hftOld);
    memDC.SetTextColor(crOld);

    rcItem = rcText;
}
//////////////////////////////////////////////////////////////////////////

