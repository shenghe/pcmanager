#include "stdafx.h"
#include "kdraw.h"
#include "kbitmapmapmfc.h"

#pragma comment(lib, "msimg32.lib")


class GlobalGdiObject
{
public:
    GlobalGdiObject()
    {
		m_hftDefault   = NULL;
		m_hftBold      = NULL;
		m_hftBig       = NULL;
		m_hftExtraBig  = NULL;
		m_bWindowsIsNT = FALSE;
		m_nDefFontSize = 0;
    }

    ~GlobalGdiObject()
    {
        ::DeleteObject(m_hftBold);
        ::DeleteObject(m_hftBig);
        ::DeleteObject(m_hftExtraBig);
    }


	HFONT GetDefaultFont() 
	{
		Init();
		return m_hftDefault;
	}

	HFONT GetBoldFont() 
	{
		Init();
		return m_hftBold;
	}

	HFONT GetBigFont() 
	{
		Init();
		return m_hftBig;
	}

	HFONT GetExtraBigFont() 
	{
		Init();
		return m_hftExtraBig;
	}

	int GetDefaultFontSize()
	{
		if( m_nDefFontSize != 0 )
			return m_nDefFontSize;


		Init();

		LOGFONT lf = {0};

		if( m_hftDefault != NULL)
		{
			::GetObject(m_hftDefault, sizeof(LOGFONT), &lf );
			m_nDefFontSize =  abs( lf.lfHeight );
		}

		return m_nDefFontSize;
	}

	BOOL  IsWindowsNT() 
	{
		Init();
		return m_bWindowsIsNT;
	}
	

private:
	void Init()
	{
		if( m_hftDefault != NULL )
			return;


//		CKSFont* pKSFont = NULL;
// 		if( g_pKSLang != NULL )
// 		{
// 			pKSFont = g_pKSLang->GetFont();
// 			if( pKSFont != NULL )
// 			{
// 				m_hftDefault  = (HFONT)pKSFont->GetSafeHandle();
// 			}
// 
// 			pKSFont = g_pKSLang->GetFont( IDF_KAS_BOLD );
// 			if( pKSFont != NULL )
// 			{
// 				m_hftBold     = (HFONT)pKSFont->GetSafeHandle();
// 			}
// 
// 			pKSFont = g_pKSLang->GetFont( IDF_KAS_BIG );
// 			if( pKSFont != NULL )
// 			{
// 				m_hftBig      = (HFONT)pKSFont->GetSafeHandle();
// 			}
// 
// 			pKSFont = g_pKSLang->GetFont( IDF_KAS_EXTRABIG );
// 			if( pKSFont != NULL )
// 			{
// 				m_hftExtraBig = (HFONT)pKSFont->GetSafeHandle();
// 			}
// 		}
// 		else
		{
			LOGFONT logFont;
			m_hftDefault = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);


			::GetObject(m_hftDefault, sizeof(LOGFONT), &logFont);
			logFont.lfWeight = FW_NORMAL;
			m_hftBold = ::CreateFontIndirect(&logFont);


			logFont.lfWidth = 0;
			logFont.lfHeight -= 1; //= -13;
			m_hftBig = ::CreateFontIndirect(&logFont);


			logFont.lfHeight -= 5; //= -18;
			m_hftExtraBig = ::CreateFontIndirect(&logFont);
		}


		m_bWindowsIsNT = (::GetVersion() < 0x80000000);
	}


    HFONT m_hftDefault;
    HFONT m_hftBold;
    HFONT m_hftBig;
    HFONT m_hftExtraBig;

	int  m_nDefFontSize;
    BOOL m_bWindowsIsNT;
};

GlobalGdiObject g_globalGdiObj;
CKBitmapMapMFC g_bmpMap;

void KDraw::TransparentBlt2( HDC hdcDest,   // 目标DC
					int nXOriginDest,       // 目标X偏移
					int nYOriginDest,       // 目标Y偏移
					int nWidthDest,         // 目标宽度
					int nHeightDest,        // 目标高度
					HDC hdcSrc,             // 源DC
					int nXOriginSrc,        // 源X起点
					int nYOriginSrc,        // 源Y起点
					int nWidthSrc,          // 源宽度
					int nHeightSrc,         // 源高度
					UINT crTransparent      // 透明色,COLORREF类型
					)
{
	HBITMAP hOldImageBMP = NULL, hImageBMP = NULL, hOldMaskBMP = NULL, hMaskBMP = NULL;
	HDC     hImageDC = NULL, hMaskDC = NULL;

    if ( g_globalGdiObj.IsWindowsNT() )
    {
        ::TransparentBlt(
            hdcDest, 
            nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, 
            hdcSrc, 
            nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, 
            crTransparent
            );

        return;
    }

    hImageBMP = ::CreateCompatibleBitmap(hdcDest, nWidthDest, nHeightDest);   // 创建兼容位图
    hMaskBMP = ::CreateBitmap(nWidthDest, nHeightDest, 1, 1, NULL);           // 创建单色掩码位图
    hImageDC = ::CreateCompatibleDC(hdcDest);
    hMaskDC = ::CreateCompatibleDC(hdcDest);
    
    hOldImageBMP = (HBITMAP)::SelectObject(hImageDC, hImageBMP);
    hOldMaskBMP = (HBITMAP)::SelectObject(hMaskDC, hMaskBMP);

	// 将源DC中的位图拷贝到临时DC中
	if (nWidthDest == nWidthSrc && nHeightDest == nHeightSrc)
        ::BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY);
	else
        ::StretchBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, 
					hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);

	// 设置透明色
    ::SetBkColor(hImageDC, crTransparent);

	// 生成透明区域为白色，其它区域为黑色的掩码位图
    ::BitBlt(hMaskDC, 0, 0, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCCOPY);

	// 生成透明区域为黑色，其它区域保持不变的位图
    ::SetBkColor(hImageDC, RGB(0,0,0));
    ::SetTextColor(hImageDC, RGB(255,255,255));
    ::BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);

	// 透明部分保持屏幕不变，其它部分变成黑色
    ::SetBkColor(hdcDest,RGB(255,255,255));
    ::SetTextColor(hdcDest,RGB(0,0,0));
    ::BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);

	// "或"运算,生成最终效果
    ::BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCPAINT);

	// 清理、恢复	
    ::SelectObject(hImageDC, hOldImageBMP);
    ::DeleteDC(hImageDC);
    ::SelectObject(hMaskDC, hOldMaskBMP);
    ::DeleteDC(hMaskDC);
    ::DeleteObject(hImageBMP);
    ::DeleteObject(hMaskBMP);
}

void KDraw::GradientFillRectV(CDC *pDC, CRect rcFill, FRG_PARAM params[], int nCount)
{
    GRADIENT_RECT gRect = {0, 1};
    TRIVERTEX vert[2] = {
        {rcFill.left, rcFill.top, 0, 0, 0, 0}, 
        {rcFill.right, rcFill.top, 0, 0, 0, 0}
    };
    int i = 0;

    for (i = 1; i < nCount && vert[0].y <= rcFill.bottom; i ++)
    {
        vert[0].y = vert[1].y;
        vert[1].y += params[i].lOffset;
        vert[0].Red     = GetRValue(params[i - 1].crColor) << 8;
        vert[0].Green   = GetGValue(params[i - 1].crColor) << 8;
        vert[0].Blue    = GetBValue(params[i - 1].crColor) << 8;
        vert[1].Red     = GetRValue(params[i].crColor) << 8;
        vert[1].Green   = GetGValue(params[i].crColor) << 8;
        vert[1].Blue    = GetBValue(params[i].crColor) << 8;
        ::GradientFill(pDC->GetSafeHdc(), vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
    }
}

void KDraw::GradientFillRectH(CDC *pDC, CRect rcFill, FRG_PARAM params[], int nCount)
{
    GRADIENT_RECT gRect = {0, 1};
    TRIVERTEX vert[2] = {
        {rcFill.left, rcFill.top, 0, 0, 0, 0}, 
        {rcFill.left, rcFill.bottom, 0, 0, 0, 0}
    };
    int i = 0;

    for (i = 1; i < nCount && vert[0].x <= rcFill.right; i ++)
    {
        vert[0].x = vert[1].x;
        vert[1].x += params[i].lOffset;
        vert[0].Red     = GetRValue(params[i - 1].crColor) << 8;
        vert[0].Green   = GetGValue(params[i - 1].crColor) << 8;
        vert[0].Blue    = GetBValue(params[i - 1].crColor) << 8;
        vert[1].Red     = GetRValue(params[i].crColor) << 8;
        vert[1].Green   = GetGValue(params[i].crColor) << 8;
        vert[1].Blue    = GetBValue(params[i].crColor) << 8;
        ::GradientFill(pDC->GetSafeHdc(), vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
    }
}

HFONT KDraw::GetDefaultFont(void)
{
    return g_globalGdiObj.GetDefaultFont();
}

HFONT KDraw::GetBoldFont(void)
{
    return g_globalGdiObj.GetBoldFont();
}

HFONT KDraw::GetBigFont(void)
{
    return g_globalGdiObj.GetBigFont();
}

HFONT KDraw::GetExtraBigFont(void)
{
    return g_globalGdiObj.GetExtraBigFont();
}

int KDraw::GetDefaultFontSize()
{
	return g_globalGdiObj.GetDefaultFontSize();
}


void KDraw::DrawBitmap(HDC hdc, HBITMAP hBitmap, INT xStart, INT yStart)
{
    BITMAP bmpInfo;
    HDC dcDraw = ::CreateCompatibleDC(hdc);
    HBITMAP hBmpOld = (HBITMAP)::SelectObject(dcDraw, hBitmap);

    ::GetObject(hBitmap, sizeof(BITMAP), &bmpInfo);
    ::BitBlt(
        hdc, xStart, yStart, bmpInfo.bmWidth, bmpInfo.bmHeight, 
        dcDraw, 0, 0, SRCCOPY
        );

    ::SelectObject(dcDraw, hBmpOld);
    ::DeleteDC(dcDraw);
}

void KDraw::DrawBitmap(HDC hdc, HBITMAP hBitmap, POINT ptDest, RECT rcSrc)
{
    HDC dcDraw = ::CreateCompatibleDC(hdc);
    HBITMAP hBmpOld = (HBITMAP)::SelectObject(dcDraw, hBitmap);

    ::BitBlt(
        hdc, ptDest.x, ptDest.y, rcSrc.right - rcSrc.left, rcSrc.bottom - rcSrc.top, 
        dcDraw, rcSrc.left, rcSrc.top, SRCCOPY
        );

    ::SelectObject(dcDraw, hBmpOld);
    ::DeleteDC(dcDraw);
}

void KDraw::DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, INT xStart, INT yStart, COLORREF crTransparent)
{
    BITMAP bmpInfo;
    HDC dcDraw = ::CreateCompatibleDC(hdc);
    HBITMAP hBmpOld = (HBITMAP)::SelectObject(dcDraw, hBitmap);

    ::GetObject(hBitmap, sizeof(BITMAP), &bmpInfo);

    TransparentBlt2(
        hdc, xStart, yStart, bmpInfo.bmWidth, bmpInfo.bmHeight, 
        dcDraw, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, crTransparent
        );

    ::SelectObject(dcDraw, hBmpOld);
    ::DeleteDC(dcDraw);
}

void KDraw::DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, POINT ptDest, POINT ptSrc, SIZE sizeSrc, COLORREF crTransparent)
{
    HDC dcDraw = ::CreateCompatibleDC(hdc);
    HBITMAP hBmpOld = (HBITMAP)::SelectObject(dcDraw, hBitmap);

    TransparentBlt2(
        hdc, ptDest.x, ptDest.y, sizeSrc.cx, sizeSrc.cy, 
        dcDraw, ptSrc.x, ptSrc.y, sizeSrc.cx, sizeSrc.cy, crTransparent
        );

    ::SelectObject(dcDraw, hBmpOld);
    ::DeleteDC(dcDraw);
}

HBITMAP KDraw::LoadBitmap(UINT uResID)
{
    return g_bmpMap[uResID];
}

SIZE KDraw::GetBitmapSize(HBITMAP hbmp)
{
    BITMAP bmpInfo;
    SIZE size;

    ::GetObject(hbmp, sizeof(BITMAP), &bmpInfo);

    size.cx = bmpInfo.bmWidth;
    size.cy = bmpInfo.bmHeight;

    return size;
}

void KDraw::DrawFrameByBitmap(HDC hDC, RECT rcFrame, UINT uBmpID, SIZE sizeLT)
{
    BITMAP bmpInfo;
    HBITMAP hBitmap = g_bmpMap[uBmpID];
    HDC dcDraw = ::CreateCompatibleDC(hDC);
    HBITMAP hBmpOld = (HBITMAP)::SelectObject(dcDraw, hBitmap);
    SIZE sizeRB, sizeCenter;

    if (NULL == hBitmap)
        return;

    ::GetObject(hBitmap, sizeof(BITMAP), &bmpInfo);

    sizeRB.cx = bmpInfo.bmWidth - sizeLT.cx - 3;
    sizeRB.cy = bmpInfo.bmHeight - sizeLT.cy - 3;
    sizeCenter.cx = rcFrame.right - rcFrame.left - bmpInfo.bmWidth + 3;
    sizeCenter.cy = rcFrame.bottom - rcFrame.top - bmpInfo.bmHeight + 3;

    // TopLeft
    ::BitBlt(
        hDC, 
        rcFrame.left, rcFrame.top, sizeLT.cx, sizeLT.cy, 
        dcDraw, 0, 0, SRCCOPY
        );

    // TopRight
    ::BitBlt(
        hDC, 
        rcFrame.right - sizeRB.cx, rcFrame.top, sizeRB.cx, sizeLT.cy, 
        dcDraw, sizeLT.cx + 3, 0, SRCCOPY
        );
    
    // BottomLeft
    ::BitBlt(
        hDC, 
        rcFrame.left, rcFrame.bottom - sizeRB.cy, sizeLT.cx, sizeRB.cy, 
        dcDraw, 0, sizeLT.cy + 3, SRCCOPY
        );

    // BottomRight
    ::BitBlt(
        hDC, 
        rcFrame.right - sizeRB.cx, rcFrame.bottom - sizeRB.cy, sizeRB.cx, sizeRB.cy, 
        dcDraw, sizeLT.cx + 3, sizeLT.cy + 3, SRCCOPY
        );

    // Top
    ::StretchBlt(
        hDC, 
        rcFrame.left + sizeLT.cx, rcFrame.top, sizeCenter.cx, sizeLT.cy, 
        dcDraw, sizeLT.cx + 1, 0, 1, sizeLT.cy, 
        SRCCOPY
        );

    // Left
    ::StretchBlt(
        hDC, 
        rcFrame.left, rcFrame.top + sizeLT.cy, sizeLT.cx, sizeCenter.cy, 
        dcDraw, 0, sizeLT.cy + 1, sizeLT.cx, 1, 
        SRCCOPY
        );

    // Right
    ::StretchBlt(
        hDC, 
        rcFrame.right - sizeRB.cx, rcFrame.top + sizeLT.cy, sizeRB.cx, sizeCenter.cy, 
        dcDraw, sizeLT.cx + 3, sizeLT.cy + 1, sizeRB.cx, 1, 
        SRCCOPY
        );

    // Bottom
    ::StretchBlt(
        hDC, 
        rcFrame.left + sizeLT.cx, rcFrame.bottom - sizeRB.cy, sizeCenter.cx, sizeRB.cy, 
        dcDraw, sizeLT.cx + 1, sizeLT.cy + 3, 1, sizeRB.cy, 
        SRCCOPY
        );

    ::SelectObject(dcDraw, hBmpOld);
    ::DeleteDC(dcDraw);
}

void KDraw::DrawTransparentFrameByBitmap(HDC hDC, RECT rcFrame, UINT uBmpID, SIZE sizeLT, COLORREF crTransparent/* = RGB(0xFF, 0, 0xFF)*/)
{
    BITMAP bmpInfo;
    HBITMAP hBitmap = g_bmpMap[uBmpID];
    HDC dcDraw = ::CreateCompatibleDC(hDC);
    HBITMAP hBmpOld = (HBITMAP)::SelectObject(dcDraw, hBitmap);
    SIZE sizeRB, sizeCenter;

    if (NULL == hBitmap)
        return;

    ::GetObject(hBitmap, sizeof(BITMAP), &bmpInfo);

    sizeRB.cx = bmpInfo.bmWidth - sizeLT.cx - 3;
    sizeRB.cy = bmpInfo.bmHeight - sizeLT.cy - 3;
    sizeCenter.cx = rcFrame.right - rcFrame.left - bmpInfo.bmWidth + 3;
    sizeCenter.cy = rcFrame.bottom - rcFrame.top - bmpInfo.bmHeight + 3;

    // TopLeft
    TransparentBlt2(
        hDC, 
        rcFrame.left, rcFrame.top, sizeLT.cx, sizeLT.cy, 
        dcDraw, 
        0, 0, sizeLT.cx, sizeLT.cy, 
        crTransparent
        );

    // TopRight
    TransparentBlt2(
        hDC, 
        rcFrame.right - sizeRB.cx, rcFrame.top, sizeRB.cx, sizeLT.cy, 
        dcDraw, 
        sizeLT.cx + 3, 0, sizeRB.cx, sizeLT.cy, 
        crTransparent
        );
    
    // BottomLeft
    TransparentBlt2(
        hDC, 
        rcFrame.left, rcFrame.bottom - sizeRB.cy, sizeLT.cx, sizeRB.cy, 
        dcDraw, 
        0, sizeLT.cy + 3, sizeLT.cx, sizeRB.cy, 
        crTransparent
        );

    // BottomRight
    TransparentBlt2(
        hDC, 
        rcFrame.right - sizeRB.cx, rcFrame.bottom - sizeRB.cy, sizeRB.cx, sizeRB.cy, 
        dcDraw, 
        sizeLT.cx + 3, sizeLT.cy + 3, sizeRB.cx, sizeRB.cy, 
        crTransparent
        );

    // Top
    TransparentBlt2(
        hDC, 
        rcFrame.left + sizeLT.cx, rcFrame.top, 
        sizeCenter.cx, sizeLT.cy, 
        dcDraw, sizeLT.cx + 1, 0, 1, sizeLT.cy, 
        crTransparent
        );

    // Left
    TransparentBlt2(
        hDC, 
        rcFrame.left, rcFrame.top + sizeLT.cy, 
        sizeLT.cx, sizeCenter.cy, 
        dcDraw, 0, sizeLT.cy + 1, sizeLT.cx, 1, 
        crTransparent
        );

    // Right
    TransparentBlt2(
        hDC, 
        rcFrame.right - sizeRB.cx, rcFrame.top + sizeLT.cy, 
        sizeRB.cx, sizeCenter.cy, 
        dcDraw, sizeLT.cx + 3, sizeLT.cy + 1, sizeRB.cx, 1, 
        crTransparent
        );

    // Bottom
    TransparentBlt2(
        hDC, 
        rcFrame.left + sizeLT.cx, rcFrame.bottom - sizeRB.cy, 
        sizeCenter.cx, sizeRB.cy, 
        dcDraw, sizeLT.cx + 1, sizeLT.cy + 3, 1, sizeRB.cy, 
        crTransparent
        );

    ::SelectObject(dcDraw, hBmpOld);
    ::DeleteDC(dcDraw);
}

void KDraw::Line(HDC hDC, int x1, int y1, int x2, int y2, COLORREF crColor)
{
    HPEN hPen = ::CreatePen(PS_SOLID, 1, crColor), hOldPen = NULL;

    hOldPen = (HPEN)::SelectObject(hDC, hPen);

    ::MoveToEx(hDC, x1, y1, NULL);
    ::LineTo(hDC, x2, y2);

    ::SelectObject(hDC, hOldPen);

    ::DeleteObject(hPen);
}

void KDraw::FrameRect(HDC hDC, RECT rect, COLORREF crColor)
{
    HBRUSH hBrush = ::CreateSolidBrush(crColor);

    ::FrameRect(hDC, &rect, hBrush);

    ::DeleteObject(hBrush);
}

void KDraw::StretchBlt(HDC hdcDest, HBITMAP hBmpSrc, RECT rcDest, RECT rcSrc)
{
    HDC hdcMem = ::CreateCompatibleDC(hdcDest);
    HBITMAP hbmpOld = (HBITMAP)::SelectObject(hdcMem, hBmpSrc);

    ::StretchBlt(
        hdcDest, 
        rcDest.left, rcDest.top, rcDest.right - rcDest.left, rcDest.bottom - rcDest.top, 
        hdcMem, 
        rcSrc.left, rcSrc.top, rcSrc.right - rcSrc.left, rcSrc.bottom - rcSrc.top, 
        SRCCOPY
        );

    ::SelectObject(hdcMem, hbmpOld);

    ::DeleteDC(hdcMem);
}

void KDraw::DrawCombineButton(
    HDC hdc, HBITMAP hBitmap, RECT rcSrc, LONG lBtnWidth, 
    LONG lSkinLeftSize, LONG lSkinRightSize, COLORREF crBackground, COLORREF crTransparent
    )
{
    HDC hdcMem = ::CreateCompatibleDC(hdc);
    HBITMAP hbmpOld = (HBITMAP)::SelectObject(hdcMem, hBitmap);
    RECT rcDraw = {0, 0, lBtnWidth, rcSrc.bottom - rcSrc.top};

    ::FillRect(hdc, &rcDraw, (HBRUSH)::GetStockObject(WHITE_BRUSH));

    TransparentBlt2(
        hdc, 
        0, 0, lSkinLeftSize, rcSrc.bottom - rcSrc.top, 
        hdcMem, 
        rcSrc.left, rcSrc.top, lSkinLeftSize, rcSrc.bottom - rcSrc.top, 
        crTransparent
        );

    TransparentBlt2(
        hdc, 
        lSkinLeftSize, 0, lBtnWidth - lSkinLeftSize - lSkinRightSize, rcSrc.bottom - rcSrc.top, 
        hdcMem, 
        rcSrc.left + lSkinLeftSize, rcSrc.top, 1, rcSrc.bottom - rcSrc.top, 
        crTransparent
        );

    TransparentBlt2(
        hdc, 
        lBtnWidth - lSkinRightSize, 0, lSkinRightSize, rcSrc.bottom - rcSrc.top, 
        hdcMem, 
        rcSrc.left + lSkinLeftSize + 1, rcSrc.top, lSkinRightSize, rcSrc.bottom - rcSrc.top, 
        crTransparent
        );

    ::SelectObject(hdcMem, hbmpOld);

    ::DeleteDC(hdcMem);
}

void KDraw::StretchBtn(HDC hdcDest, HBITMAP hBmpSkin, int nBtnWidth, RECT rcSkin, int nSkinLWidth)
{
    CRect rcSrc, rcDest;
    POINT ptDest;
    
    ptDest.x = ptDest.y = 0;
    rcSrc.SetRect(0, rcSkin.top, nSkinLWidth, rcSkin.top + rcSkin.bottom - rcSkin.top);
    KDraw::DrawBitmap(hdcDest, hBmpSkin, ptDest, rcSrc);

    ptDest.x = nBtnWidth - (rcSkin.right - rcSkin.left - nSkinLWidth - 1);
    rcDest.SetRect(nSkinLWidth, 0, ptDest.x, rcSkin.bottom - rcSkin.top);
    rcSrc.SetRect(nSkinLWidth, rcSkin.top, nSkinLWidth + 1, rcSkin.top + rcSkin.bottom - rcSkin.top);
    KDraw::StretchBlt(hdcDest, hBmpSkin, rcDest, rcSrc);

    rcSrc.SetRect(nSkinLWidth + 1, rcSkin.top, rcSkin.right - rcSkin.left, rcSkin.top + rcSkin.bottom - rcSkin.top);
    KDraw::DrawBitmap(hdcDest, hBmpSkin, ptDest, rcSrc);
}

HRGN KDraw::CreateRgnFromBitmap(HBITMAP hBmp, int nWidth, int nHeight, COLORREF crTransparent)
{
    int x, y, nStart, nFirst = 0;
    CRgn rgnLine, rgnTemp;
    HDC hdcBmp;
    HBITMAP hBmpOld = NULL;
    COLORREF crPoint;
    HRGN hRgnMask = ::CreateRectRgn(0, 0, 1, 1);

    hdcBmp = ::CreateCompatibleDC(NULL);
    hBmpOld = (HBITMAP)::SelectObject(hdcBmp, hBmp);

    rgnTemp.CreateRectRgn(0, 0, 0, 0);
    rgnLine.CreateRectRgn(0, 0, 0, 0);

    for (y = 0; y < nHeight; y ++)
    {
        nStart = -1;

        for (x = 0; x <= nWidth; x ++)
        {
            if (x == nWidth)
                crPoint = crTransparent;
            else
                crPoint = ::GetPixel(hdcBmp, x, y);

            if (crTransparent != crPoint && -1 == nStart)
            {
                nStart = x;
            }

            if (crTransparent == crPoint && -1 != nStart)
            {
                rgnLine.SetRectRgn(nStart, y, x, y + 1);

                if (1 == nFirst)
                {
                    ::CombineRgn(hRgnMask, rgnLine, rgnTemp, RGN_OR);
                }
                else
                {
                    ::CombineRgn(hRgnMask, rgnLine, NULL, RGN_COPY);
                    nFirst = 1;
                }

                ::CombineRgn(rgnTemp, hRgnMask, NULL, RGN_COPY);

                nStart = -1;
            }
        }
    }

    ::SelectObject(hdcBmp, hBmpOld);
    ::DeleteDC(hdcBmp);

    return hRgnMask;
}

//////////////////////////////////////////////////////////////////////////

void KDraw::DrawGradualColorRect(
	HDC dc,
	const RECT& drawRC, 
	COLORREF clrFirst,
	COLORREF clrEnd,
	BOOL fVertical
	) 
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

void KDraw::FillSolidRect(HDC hDC, const RECT& rcZone, COLORREF clr)
{
	COLORREF clrOld = ::SetBkColor(hDC, clr);
	if(clrOld != CLR_INVALID)
	{
		::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcZone, NULL, 0, NULL);
		::SetBkColor(hDC, clrOld);
	}
}

//////////////////////////////////////////////////////////////////////////
