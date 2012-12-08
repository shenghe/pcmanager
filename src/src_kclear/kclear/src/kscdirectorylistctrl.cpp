// DirectoryListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "kscdirectorylistctrl.h"
#include "resource.h"
#include "msgdefine.h"

LPCTSTR GetFileSizeText(__int64 fNum)
{
    static TCHAR szResult[60] = { 0 };

    if (fNum < 1000)   // Byte
    {
        _stprintf(szResult, TEXT("%dB"), fNum);
    }
    else if (fNum < 1024 * 1000)  // KB
    {
        double dfNum = (double)fNum / 1024;

        _stprintf(szResult, 
            dfNum == double(int(dfNum)) ? TEXT("%.0fKB") : TEXT("%.0fKB"), 
            dfNum);
    }
    else if (fNum < 1024 * 1024 * 1000) // MB
    {
        double dfNum = (double)fNum / (1024 * 1024);

        _stprintf(szResult, 
            dfNum == double(int(dfNum)) ? TEXT("%.0fMB") : TEXT("%.0fMB"), 
            dfNum);

    }
    else // GB
    {
        double dfNum = (double)fNum / (1024 * 1024 * 1024);

        _stprintf(szResult, 
            dfNum == double(int(dfNum)) ? TEXT("%.0fGB") : TEXT("%.0fGB"), 
            dfNum);
    }    

    if (szResult[0] == TCHAR("-"))
    {

    }
    return szResult;
}


CDirectoryListCtrl::CDirectoryListCtrl()
{
    m_clrSelect       = RGB(228, 245, 255);
    m_clrSelectBorder = RGB(0 , 0, 128);
    m_clrTextColor    = RGB(0, 0, 0);
    m_clrSelectTextColor = RGB(0, 0, 0);
    m_clrBorder       = RGB(165, 165, 165);
    m_hImageList = NULL;
    m_currentTSize = 0;
    m_nCurrentIndex = -1;

}

CDirectoryListCtrl::~CDirectoryListCtrl()
{
    ::DestroyIcon(m_hIconRegedit);
}

/////////////////////////////////////////////////////////////////////////////
// CDirectoryListCtrl message handlers

void CDirectoryListCtrl::InitListCtrl()
{
    

    ModifyStyle(0, 
        LVS_REPORT|LVS_SHOWSELALWAYS|LVS_SINGLESEL|LVS_OWNERDATA|LVS_OWNERDRAWFIXED);

    ModifyStyleEx(WS_EX_CLIENTEDGE, 0);

    SetExtendedListViewStyle( 
        LVS_EX_FULLROWSELECT| LVS_EX_INFOTIP );
   

    CHeaderCtrl pHeader = GetHeader();
    m_wndHeaderCtrl.SubclassWindow(pHeader.m_hWnd);
    SetWindowPos(NULL, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

    m_imagelist.Create(32, 32, ILC_COLOR32 | ILC_MASK, 1, 0);
    CBitmap bmpFILE;
    bmpFILE.LoadBitmap(IDB_BMP_FILE);
    //bmpFILE = AtlLoadGdiplusImage(IDB_BMP_FILE, _T("PNG"));
    m_imagelist.Add((HBITMAP)bmpFILE,RGB(255,255,255));
    m_hIconRegedit = ::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDR_FILE));



}

void CDirectoryListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
     CClientDC dc(m_hWnd);

    int nSaveDC = dc.SaveDC();

    SelectObject((HDC)dc,KuiFontPool::GetFont(KUIF_DEFAULTFONT));

    RECT rcItem     = lpDrawItemStruct->rcItem;
    int  nitemIndex = lpDrawItemStruct->itemID;
    bool bSelected = (lpDrawItemStruct->itemState & ODS_SELECTED) != 0;
    bool bFocus    = (lpDrawItemStruct->itemState & ODS_FOCUS) != 0;

    CRect rcClient;
    GetClientRect(&rcClient);

    rcItem.right = rcClient.right;

    DIR_LIST_ITEM& item = m_itemArray[nitemIndex];

    CRect rcSubItem;

    GetSubItemRect(nitemIndex, 0, LVIR_LABEL, rcSubItem);

    
    RECT rcBkGnd = rcSubItem;
    rcBkGnd.left = 0;
    rcBkGnd.bottom -= 2;
    
    SetBkMode((HDC)dc,TRANSPARENT);
    dc.FillSolidRect(&rcBkGnd, 
        bSelected? 
    RGB(190,207,223):RGB(243,247,253));

    CPen penleft;
    penleft.CreatePen( PS_SOLID, 1, RGB(160,186,209));
    SelectObject(dc, penleft );
    dc.MoveTo(rcBkGnd.left,rcBkGnd.top);
    dc.LineTo(rcBkGnd.left,rcBkGnd.bottom);


    rcBkGnd.left += 5;
    //rcBkGnd.top += 4;

    POINT pt = { rcBkGnd.left , rcBkGnd.top };
    
    SIZE tmpSize = {32,32};
    
    dc.DrawIconEx(pt,m_hIconRegedit,tmpSize);
    
    rcSubItem.left += 35;
    rcSubItem.top +=10; 
     
    CString strName = item.strCaption;
    strName += L"(";
    dc.DrawText(strName, strName.GetLength(),
        &rcSubItem, 
        DT_SINGLELINE );

    

    CSize  size;
    dc.GetTextExtent(strName,-1,&size);

    CString strSize(GetFileSizeText(item.ulSize));
    RECT rcSize = rcSubItem;
    rcSize.left += size.cx;

    dc.DrawText(strSize, strSize.GetLength(),
        &rcSize, 
        DT_SINGLELINE );

    dc.GetTextExtent(strSize,-1,&size);
    rcSize.left += size.cx;
    dc.DrawText(L")",1,
        &rcSize, 
        DT_SINGLELINE );


    //rcSubItem.top = rcSubItem.top + size.cy;
    
//     rcSubItem.bottom = rcSubItem.bottom-5;
//     rcSubItem.top = rcSubItem.bottom-6;
//     rcSubItem.right = rcSubItem.left + 100;
// 
//     HBRUSH hBrush = ::CreateSolidBrush(RGB(176,176,176));
// 
//     ::FrameRect((HDC)dc, &rcSubItem, hBrush);
// 
//     int CurPos = 100 - (int)(((double)item.ulSize/m_currentTSize)*100);
// 
//     if(CurPos >= 95)
//         CurPos = 95;
// 
//     if(CurPos <= 5)
//         CurPos = 5;
// 
//     rcSubItem.left = rcSubItem.left+1;
//     rcSubItem.top = rcSubItem.top+1;
//     rcSubItem.right = rcSubItem.right-CurPos;
//     rcSubItem.bottom = rcSubItem.bottom-1;

    
//     GRADIENT_RECT gRect[3] ={ {0, 1},
//         {1, 2},
//         {2, 3},
//     };
//  
//      TRIVERTEX vert[4] = {
//          {rcSubItem.left,rcSubItem.top, 0, 0, 0, 0}, 
//          {rcSubItem.right,rcSubItem.top+3, 0, 0, 0, 0},
//          {rcSubItem.left,rcSubItem.top+4, 0, 0, 0, 0},
//          {rcSubItem.right,rcSubItem.bottom, 0, 0, 0, 0}};
//  
//      COLORREF clrFirst = RGB(120,227,248);
//      COLORREF clrSecond = RGB(99,212,236);
//      COLORREF clrThird = RGB(99,212,236);
//      COLORREF clrEnd = RGB(5,140,167);
//  
//      vert[0].Red     = GetRValue(clrFirst) << 8;
//      vert[0].Green   = GetGValue(clrFirst) << 8;
//      vert[0].Blue    = GetBValue(clrFirst) << 8;
//      vert[1].Red     = GetRValue(clrSecond) << 8;
//      vert[1].Green   = GetGValue(clrSecond) << 8;
//      vert[1].Blue    = GetBValue(clrSecond) << 8;
//      vert[2].Red     = GetRValue(clrThird) << 8;
//      vert[2].Green   = GetGValue(clrThird) << 8;
//      vert[2].Blue    = GetBValue(clrThird) << 8;
//      vert[3].Red     = GetRValue(clrEnd) << 8;
//      vert[3].Green   = GetGValue(clrEnd) << 8;
//      vert[3].Blue    = GetBValue(clrEnd) << 8;
//  
//  
//      ::GradientFill(dc, vert, 4, &gRect, 3, GRADIENT_FILL_RECT_V);

    
     


     CPen newPen,newPenAno;
     newPen.CreatePen( PS_SOLID, 1, RGB(163,186,210));
     newPenAno.CreatePen(PS_SOLID,1,RGB(255,255,255));
 
     SelectObject(dc, newPen );
     dc.MoveTo(rcItem.left,rcItem.bottom-2);
     dc.LineTo(rcItem.right,rcItem.bottom-2);
 
     SelectObject(dc, newPenAno );
     dc.MoveTo(rcItem.left,rcItem.bottom-1);
     dc.LineTo(rcItem.right,rcItem.bottom-1);



             
    dc.RestoreDC(nSaveDC);
    ::DeleteObject(newPen);
    //::DeleteObject(hBrush);
    ::DeleteObject(newPenAno);
    ::DeleteObject(penleft);

}


void CDirectoryListCtrl::AutoColumnWidth()
{
    SetColumnWidth(0,142);
    RedrawItems(0,(int)m_itemArray.size()-1);
    //Invalidate();
}

void CDirectoryListCtrl::AddItem(LPCTSTR pszName, ULONGLONG ulSize, int nIndex,CString strDesc)
{

    DIR_LIST_ITEM item;
    item.strCaption = pszName;
    item.ulSize     = ulSize;
    m_currentTSize += ulSize;
    item.nIndex = nIndex;
    item.strDesc = strDesc;
    m_itemArray.push_back(item);
    m_nCurrentIndex++;
    SetItemCountEx((int)m_itemArray.size(), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
    RedrawItems(0,m_nCurrentIndex);
}



void CDirectoryListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
    SetMsgHandled(FALSE);
    CRect rcHeadClient;
    m_wndHeaderCtrl.GetClientRect(rcHeadClient);

    if (point.y < rcHeadClient.bottom)
        return;


    UINT uFlag  = 0;

    LVHITTESTINFO pHitTestInfo = {0};
    pHitTestInfo.pt = point;
    pHitTestInfo.flags = 0;
    
    int  nIndex = HitTest(&pHitTestInfo);
    if(nIndex < 0)
    {
        SetMsgHandled(TRUE);
    }
    else
    {
        RedrawItems(0,(int)m_itemArray.size()-1);
        ::SendMessage(m_hwParent, MSG_USER_DIRLIST_CHANGE,(WPARAM)m_itemArray[nIndex].nIndex, (LPARAM)m_itemArray[nIndex].strDesc.GetBuffer());
    }

    
        
}

LRESULT CDirectoryListCtrl::OnHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    LRESULT lResult = -1;
    bHandled = TRUE;
    LPLVHITTESTINFO pHitTestInfo = (LPLVHITTESTINFO)lParam;

    lResult = DefWindowProc(LVM_HITTEST, wParam, lParam);

    return lResult;
}





void CDirectoryListCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
    CRect rcHeadClient;
    m_wndHeaderCtrl.GetClientRect(rcHeadClient);

    if (point.y < rcHeadClient.bottom)
        return;

    UINT uFlag  = 0;

    int  nIndex = HitTest(point, &uFlag);
    if (nIndex < 0)
    {
        SetMsgHandled(TRUE);
    }

}


void CDirectoryListCtrl::OnPaint(HDC rgn) 
{
    SetMsgHandled(FALSE);
    int nCurrentCount = (int)m_itemArray.size();
    CWindowDC dc(m_hWnd);
    CRect rcWindow;

    GetWindowRect(&rcWindow);

    int nSaveDC = dc.SaveDC();

    COLORREF crBg = RGB(255, 255, 255);
     
    GetClientRect(rcWindow);

    rcWindow.top += (nCurrentCount*34);
       
    dc.FillSolidRect(&rcWindow, crBg);
    dc.RestoreDC(nSaveDC);
   //  RedrawItems(0,m_itemArray.size()-1);
  
}

void CDirectoryListCtrl::OnNcPaint(HRGN rgn) 
{
    
    SetMsgHandled(FALSE);
}