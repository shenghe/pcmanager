// MiniButton.cpp : implementation file
//

#include "stdafx.h"
#include "minibutton.h"
#include "kdraw.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

/////////////////////////////////////////////////////////////////////////////
// CMiniButton

HBITMAP CMiniButton::ms_hbmpNormal = NULL;
HBITMAP CMiniButton::ms_hbmpHover = NULL;
HBITMAP CMiniButton::ms_hbmpDown = NULL;
HBITMAP CMiniButton::ms_hbmpDisable = NULL;
CSize CMiniButton::ms_sizeMiniBtn;

CMiniButton::CMiniButton()
{
}

CMiniButton::~CMiniButton()
{
}


BEGIN_MESSAGE_MAP(CMiniButton, CxSkinButton)
	//{{AFX_MSG_MAP(CMiniButton)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMiniButton message handlers

void CMiniButton::_CreateMiniSkin()
{
    CWindowDC dc(this);
    HBITMAP hbmpSkin = ::LoadBitmap(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_BTN_MINI)); // Only use once, so don't use map
    HBITMAP hbmpOld = NULL;
    SIZE sizeSkin = KDraw::GetBitmapSize(hbmpSkin);
    CDC dcMem;
    CRect rcSkin;
    LONG lLeftWidth = 4;

    ms_sizeMiniBtn.cx = 70;
    ms_sizeMiniBtn.cy = sizeSkin.cy / 4;

    dcMem.CreateCompatibleDC(&dc);

    ms_hbmpNormal = ::CreateCompatibleBitmap(dc, ms_sizeMiniBtn.cx, ms_sizeMiniBtn.cy);
    ms_hbmpDown = ::CreateCompatibleBitmap(dc, ms_sizeMiniBtn.cx, ms_sizeMiniBtn.cy);
    ms_hbmpHover = ::CreateCompatibleBitmap(dc, ms_sizeMiniBtn.cx, ms_sizeMiniBtn.cy);
    ms_hbmpDisable = ::CreateCompatibleBitmap(dc, ms_sizeMiniBtn.cx, ms_sizeMiniBtn.cy);

    rcSkin.SetRect(0, 0, sizeSkin.cx, ms_sizeMiniBtn.cy);

    hbmpOld = (HBITMAP)dcMem.SelectObject(ms_hbmpNormal);
    KDraw::StretchBtn(dcMem, hbmpSkin, ms_sizeMiniBtn.cx, rcSkin, lLeftWidth);
    rcSkin.OffsetRect(0, ms_sizeMiniBtn.cy);

    dcMem.SelectObject(ms_hbmpDown);
    KDraw::StretchBtn(dcMem, hbmpSkin, ms_sizeMiniBtn.cx, rcSkin, lLeftWidth);
    rcSkin.OffsetRect(0, ms_sizeMiniBtn.cy);

    dcMem.SelectObject(ms_hbmpHover);
    KDraw::StretchBtn(dcMem, hbmpSkin, ms_sizeMiniBtn.cx, rcSkin, lLeftWidth);
    rcSkin.OffsetRect(0, ms_sizeMiniBtn.cy);

    dcMem.SelectObject(ms_hbmpDisable);
    KDraw::StretchBtn(dcMem, hbmpSkin, ms_sizeMiniBtn.cx, rcSkin, lLeftWidth);

    dcMem.SelectObject(hbmpOld);

    ::DeleteObject(hbmpSkin);
}

void CMiniButton::PreSubclassWindow() 
{
	if (!ms_hbmpNormal)
        _CreateMiniSkin();

    SetSkin(ms_hbmpNormal, ms_hbmpDown, ms_hbmpHover, ms_hbmpDisable, NULL, 0, 0);
	
	CxSkinButton::PreSubclassWindow();
}

void CMiniButton::OnSize(UINT nType, int cx, int cy) 
{
	CxSkinButton::OnSize(nType, cx, cy);
}

BOOL CMiniButton::Create(LPCTSTR lpszWindowName, const RECT& rect, CWnd* pParentWnd, UINT nID) 
{
	BOOL bRet = CWnd::Create(_T("BUTTON"), lpszWindowName, WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, rect, pParentWnd, nID, NULL);

	if (!ms_hbmpNormal)
        _CreateMiniSkin();

    SetSkin(ms_hbmpNormal, ms_hbmpDown, ms_hbmpHover, NULL, NULL, 0, 0, 0);

    return bRet;
}
