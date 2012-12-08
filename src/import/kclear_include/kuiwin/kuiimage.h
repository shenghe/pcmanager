//////////////////////////////////////////////////////////////////////////
//  Class Name: CKuiImage
// Description: Image Helper for KAV32 (Ver.2008)
//     Creator: Zhang Xiaoxuan
//     Version: 2007.10.19 - 1.0 - Create
//              2007.10.20 - 1.1 - Finish the draw method of 32bit bitmap
//                                 with alpha channel
//              2007.10.23 - 1.2 - support no MFC and no WTL
//              2007.11.01 - 1.3 - add some method for use:
//                                 CreateBitmap
//                                 Draw(HBITMAP, int, int, int)
//
//      Notice: 1. 32bit bitmap should be loaded by LoadDIBSection to
//                 get faster
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "kscbase/kscres.h"

#define abs(value) (value >= 0 ? value : -value)

#ifdef __AFXWIN_H__ // If MFC
    #define M_HOBJECT m_hObject
#else
    #ifdef __ATLGDI_H__ // If WTL
        #define M_HOBJECT m_hBitmap
    #else
        #define _KIMAGE_SDK_VER_
        #define M_HOBJECT m_hObject
    #endif
#endif

extern "C" IMAGE_DOS_HEADER __ImageBase;

class CKuiImage : public CBitmapHandle
{
public:

    CKuiImage(void)
        : m_lSubImageWidth(0)
        , m_nTransparentMode(ModeNone)
        , m_crMask(RGB(0xFF, 0, 0xFF))
    {
#ifdef _KIMAGE_SDK_VER_
        M_HOBJECT = NULL;
#endif
    }
    ~CKuiImage(void)
    {
#ifdef _KIMAGE_SDK_VER_
        if (M_HOBJECT)
        {
            ::DeleteObject(M_HOBJECT);
            m_hObject = NULL;
        }
#endif
    }

#ifdef _KIMAGE_SDK_VER_
    int GetBitmap(BITMAP* pBitMap)
    {
        if (NULL == M_HOBJECT)
            return -1;

        return ::GetObject(M_HOBJECT, sizeof(BITMAP), pBitMap);
    }

    BOOL Attach(HGDIOBJ hObject)
    {
        if (NULL == hObject)
            return FALSE;

        M_HOBJECT = hObject;

        return TRUE;
    }

    HGDIOBJ Detach()
    {
        HGDIOBJ hObject = M_HOBJECT;

        M_HOBJECT = NULL;

        return hObject;
    }

    BOOL DeleteObject()
    {
        if (NULL == M_HOBJECT)
            return FALSE;

        return ::DeleteObject(Detach());
    }

    operator HBITMAP() const
    {
        return (HBITMAP)m_hObject;
    }
#endif

    CKuiImage& operator=( __in const CKuiImage& src)
    {
        M_HOBJECT = src.M_HOBJECT;
        m_lSubImageWidth = src.m_lSubImageWidth;
        m_crMask = src.m_crMask;
        m_nTransparentMode = src.m_nTransparentMode;

        return( *this );
    }

    CKuiImage& operator=( __in const HBITMAP hBmp)
    {
        Attach(hBmp);

        return( *this );
    }

//     CKuiImage& operator=( __in const UINT uResID)
//     {
//         Attach(KuiBmpPool::GetBitmap(uResID));
//
//         return( *this );
//     }

	CKuiImage& operator=(__in const char* szId)
	{
		KAppRes& appRes = KAppRes::Instance();
		Attach(appRes.GetImage(szId));
		m_strResId = szId;
		return (*this);
	}

	std::string m_strResId;

    BOOL GetImageSize(SIZE &sizeImage)
    {
        BITMAP bmp;
        BOOL bRet = GetBitmap(&bmp);

        if (!bRet)
            return FALSE;

        sizeImage.cx = bmp.bmWidth;
        sizeImage.cy = bmp.bmHeight;

        return TRUE;
    }

    void SetSubImageWidth(LONG lWidth)
    {
        m_lSubImageWidth = lWidth;
    }

    LONG GetSubImageWidth()
    {
        return m_lSubImageWidth;
    }

    enum { ModeNone, ModeMaskColor, ModeAlpha };

    void SetTransparentMode(int nMode)
    {
        m_nTransparentMode = nMode;
    }

    void SetMaskColor(COLORREF crMask)
    {
        m_crMask = crMask;
    }

    BOOL LoadDIBSection(UINT nIDResource, HINSTANCE hInst = (HINSTANCE)&__ImageBase)
    {
        HGDIOBJ hLoad = ::LoadImage(hInst, MAKEINTRESOURCE(nIDResource), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        BOOL bResult = FALSE;

        if (!hLoad)
            return FALSE;

        Attach((HBITMAP)hLoad);

        return bResult;
    }

    BOOL LoadDIBSectionFromFile(LPCTSTR lpszFileName)
    {
        HGDIOBJ hLoad = ::LoadImage(NULL, lpszFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
        BOOL bResult = FALSE;

        if (!hLoad)
            return FALSE;

        Attach((HBITMAP)hLoad);

        return bResult;
    }

    BOOL CreateBitmap(int nWidth, int nHeight, COLORREF crBackground = CLR_INVALID)
    {
        HDC hDCDesktop = ::GetDC(NULL);
        HBITMAP hBmpCreate = ::CreateCompatibleBitmap(hDCDesktop, nWidth, nHeight);

        Attach(hBmpCreate);

        if (crBackground != CLR_INVALID)
        {
            HDC hDCDraw = ::CreateCompatibleDC(hDCDesktop);
            HGDIOBJ hbmpOld = ::SelectObject(hDCDraw, hBmpCreate);
            RECT rcDraw = {0, 0, nWidth, nHeight};
            COLORREF clrOld = ::SetBkColor(hDCDraw, crBackground);

            ATLASSERT(clrOld != CLR_INVALID);

            if(clrOld != CLR_INVALID)
            {
                ::ExtTextOut(hDCDraw, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, NULL);
                ::SetBkColor(hDCDraw, clrOld);
            }

            ::SelectObject(hDCDraw, hbmpOld);
            ::DeleteDC(hDCDraw);
        }

        ::ReleaseDC(NULL, hDCDesktop);

        return hBmpCreate != NULL;
    }

    BOOL BitBlt(HDC hDC, int x, int y, int nWidth, int nHeight, int xSrc, int ySrc, DWORD dwRop, int nSubImage = -1)
    {
        BOOL bRet = FALSE;
        HDC hDCDesktop = ::GetDC(NULL);
        HDC hDCDraw = ::CreateCompatibleDC(hDCDesktop);
        HGDIOBJ hbmpOld = ::SelectObject(hDCDraw, M_HOBJECT);

        ::ReleaseDC(NULL, hDCDesktop);

        if (-1 != nSubImage && 0 != m_lSubImageWidth)
        {
            xSrc += nSubImage * m_lSubImageWidth;
        }

        if (ModeNone == m_nTransparentMode)
            bRet = ::BitBlt(hDC, x, y, nWidth, nHeight, hDCDraw, xSrc, ySrc, dwRop);
        else if (ModeMaskColor == m_nTransparentMode)
            bRet = TransparentBlt2(hDC, x, y, nWidth, nHeight, hDCDraw, xSrc, ySrc, nWidth, nHeight, m_crMask);

        ::SelectObject(hDCDraw, hbmpOld);
        ::DeleteDC(hDCDraw);

        return bRet;
    }

    BOOL StretchBlt(HDC hDC, int x, int y, int nWidth, int nHeight, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop, int nSubImage = -1)
    {
        HDC hDCDesktop = ::GetDC(NULL);
        HDC hDCDraw = ::CreateCompatibleDC(hDCDesktop);
        HGDIOBJ hbmpOld = ::SelectObject(hDCDraw, M_HOBJECT);

        ::ReleaseDC(NULL, hDCDesktop);

        if (-1 != nSubImage && 0 != m_lSubImageWidth)
        {
            xSrc += nSubImage * m_lSubImageWidth;
        }

        BOOL bRet = FALSE;

        if (ModeNone == m_nTransparentMode)
            bRet = ::StretchBlt(hDC, x, y, nWidth, nHeight, hDCDraw, xSrc, ySrc, nSrcWidth, nSrcHeight, dwRop);
        else if (ModeMaskColor == m_nTransparentMode)
            bRet = TransparentBlt2(hDC, x, y, nWidth, nHeight, hDCDraw, xSrc, ySrc, nSrcWidth, nSrcHeight, m_crMask);

        ::SelectObject(hDCDraw, hbmpOld);
        ::DeleteDC(hDCDraw);

        return bRet;
    }

    BOOL Draw(HBITMAP hBmpDest, int nPosX, int nPosY, int nSubImage = -1/* All */)
    {
        HDC hDCDesktop = ::GetDC(NULL);
        HDC hDCDraw = ::CreateCompatibleDC(hDCDesktop);
        HGDIOBJ hbmpOld = ::SelectObject(hDCDraw, hBmpDest);

        BOOL bRet = Draw(hDCDraw, nPosX, nPosY, nSubImage);

        ::SelectObject(hDCDraw, hbmpOld);
        ::DeleteDC(hDCDraw);
        ::ReleaseDC(NULL, hDCDesktop);

        return bRet;
    }

    BOOL Draw(HDC hDC, int nPosX, int nPosY, int nSubImage = -1/* All */)
    {
        BOOL bResult = FALSE;
        int nDrawWidth = 0, nDrawHeight = 0, nSrcPosX = 0, nSrcPosY = 0;
        HDC hDCDesktop = NULL, hDCSrc = NULL;
        BITMAP bmp;
        HGDIOBJ hbmpOld = NULL;

        BLENDFUNCTION fnBlend = {AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA};

        if (NULL == M_HOBJECT)
            goto Exit0;

        if (0 == m_lSubImageWidth && -1 != nSubImage)
            goto Exit0;

        GetBitmap(&bmp);

        nDrawHeight = bmp.bmHeight;

        if (-1 == nSubImage)
        {
            nDrawWidth = bmp.bmWidth;
        }
        else
        {
            nDrawWidth = m_lSubImageWidth;
            nSrcPosX = nSubImage * m_lSubImageWidth;
        }

        if (ModeAlpha == m_nTransparentMode)
        {
            //_PreAlphaBlend((HBITMAP)m_hObject);

            bResult = AlphaBlend(hDC, nPosX, nPosY, nDrawWidth, nDrawHeight, (HBITMAP)M_HOBJECT, nSrcPosX, nSrcPosY);

            goto Exit0;
        }

        hDCDesktop = ::GetDC(NULL);
        hDCSrc = ::CreateCompatibleDC(hDCDesktop);
        ::ReleaseDC(NULL, hDCDesktop);

        hbmpOld = ::SelectObject(hDCSrc, M_HOBJECT);

        switch (m_nTransparentMode)
        {
        case ModeNone:
            bResult = ::BitBlt(hDC, nPosX, nPosY, nDrawWidth, nDrawHeight, hDCSrc, nSrcPosX, nSrcPosY, SRCCOPY);
        	break;

        case ModeMaskColor:
            bResult = TransparentBlt2(hDC, nPosX, nPosY, nDrawWidth, nDrawHeight, hDCSrc, nSrcPosX, nSrcPosY, nDrawWidth, nDrawHeight, m_crMask);
        	break;
        case ModeAlpha:
            // API AlphaBlend因为预乘，所以颜色会有所损失，干脆自己写了一个
            //::AlphaBlend(hDC, nPosX, nPosY, nDrawWidth, nDrawHeight, hDCSrc, nSrcPosX, nSrcPosY, nDrawWidth, nDrawHeight, fnBlend);
            break;

        default:
            bResult = FALSE;

            break;
        }

        ::SelectObject(hDCSrc, hbmpOld);

    Exit0:

        if (NULL != hDCSrc)
        {
            ::DeleteDC(hDCSrc);
            hDCSrc = NULL;
        }

        return bResult;
    }

	// 小于等于nAlpha的地方为透明，其他地方非透明
	HRGN	CreateHollyRgn(INT nAlpha)
	{
		return CreateHollyRgn((HBITMAP)M_HOBJECT, nAlpha);
	}

protected:

    LONG m_lSubImageWidth;
    int m_nTransparentMode;
    COLORREF m_crMask;

public:

#ifdef _KIMAGE_SDK_VER_
    HGDIOBJ m_hObject;
#endif

    // 预乘
    static BOOL AlphaPremultiplication(HBITMAP hbmpSrc)
    {
        LPVOID pBitsSrc = NULL;
        BYTE* psrc = NULL;
        BOOL bResult = FALSE;

        BITMAP bmpSrc;

        ::GetObject(hbmpSrc, sizeof(BITMAP), &bmpSrc);

        // Only support 32bit DIB section
        if (32 != bmpSrc.bmBitsPixel || NULL == bmpSrc.bmBits)
            goto Exit0;

        psrc = (BYTE *) bmpSrc.bmBits;

        // Just mix it
        for (int nPosY = 0; nPosY < abs(bmpSrc.bmHeight); nPosY ++)
        {
            for (int nPosX = bmpSrc.bmWidth; nPosX > 0; nPosX --)
            {
                BYTE alpha  = psrc[3];

                psrc[0] = (BYTE)((psrc[0] * alpha) / 255);
                psrc[1] = (BYTE)((psrc[1] * alpha) / 255);
                psrc[2] = (BYTE)((psrc[2] * alpha) / 255);

                psrc += 4;
            }
        }

Exit0:

        return bResult;
    }

	static HRGN CreateHollyRgn(HBITMAP hbmpSrc, INT nAlpha)
	{
		LPVOID pBitsSrc = NULL;
		BYTE* psrc = NULL;
		BOOL bResult = FALSE;
		HRGN hRgn = NULL;
		BITMAP bmpSrc;

		::GetObject(hbmpSrc, sizeof(BITMAP), &bmpSrc);

		// Only support 32bit DIB section
		if (32 != bmpSrc.bmBitsPixel || NULL == bmpSrc.bmBits)
			return NULL;

		psrc = (BYTE *) bmpSrc.bmBits;
		for (int nPosY = 0; nPosY < abs(bmpSrc.bmHeight); nPosY ++)
		{
			int nLastEmpty = -1;
			int nLastSolid = -1;

			for (int nPosX = 0; nPosX < bmpSrc.bmWidth;)
			{
				while ((nPosX < bmpSrc.bmWidth) && (psrc[3] <= nAlpha))
				{
					nPosX ++, psrc += 4;
				}
				int nLeft = nPosX;
				if (nPosX < bmpSrc.bmWidth)
				{
					while ((nPosX < bmpSrc.bmWidth) && (psrc[3] > nAlpha))
					{
						nPosX ++, psrc += 4;
					}
					int nRight = nPosX;
					// 找到一个RECT
					int nTop = bmpSrc.bmHeight - nPosY-1;
					int nBottom = nTop+1;

					HRGN hRgnNew = ::CreateRectRgn(nLeft, nTop, nRight, nBottom);
					if (!hRgn)
						hRgn = hRgnNew;
					else
					{
						if (::CombineRgn(hRgn, hRgn, hRgnNew, RGN_OR) == ERROR)
						{
							if (hRgn)
								::DeleteObject(hRgn);
							::DeleteObject(hRgnNew);
							return NULL;
						}
						::DeleteObject(hRgnNew);
					}
				}
			}
		}
		return hRgn;
	}

    static BOOL AlphaBlend(HDC hDC, int x, int y, int cx, int cy, HBITMAP hbmpSrc, int srcx, int srcy)
    {
        BITMAPINFOHEADER bmihDraw = { sizeof(BITMAPINFOHEADER), 0, 0, 1, 24 };
        LPVOID pBitsTemp = NULL, pBitsSrc = NULL;
        HBITMAP hBmpSrcTemp = NULL, hBmpTemp = NULL;
        HDC hMemDC = NULL, hMemDCDraw = NULL;
        HGDIOBJ hOld = NULL, hOldDraw = NULL;
        const BYTE* psrc = NULL;
        BYTE *pdst = NULL;
        BOOL bResult = FALSE;

        BITMAP bmpSrc;

        ::GetObject(hbmpSrc, sizeof(BITMAP), &bmpSrc);

        // Only support 32bit bitmap
        if (32 != bmpSrc.bmBitsPixel)
            goto Exit0;

        hMemDC = ::CreateCompatibleDC(hDC);

        bmihDraw.biWidth = cx;
        bmihDraw.biHeight = cy;

        if (bmpSrc.bmBits) // hbmpSrc is DIB section now
            pBitsSrc = bmpSrc.bmBits;
        else
        {   // Create a DIB section
            bmihDraw.biBitCount = 32;

            hBmpSrcTemp = ::CreateDIBSection(NULL, (BITMAPINFO *)&bmihDraw, DIB_RGB_COLORS, &pBitsSrc, NULL, NULL);
            if (hBmpSrcTemp == NULL)
                goto Exit0;

            hMemDCDraw = ::CreateCompatibleDC(hDC);

            hOldDraw = ::SelectObject(hMemDCDraw, hbmpSrc);
            hOld = ::SelectObject(hMemDC, hBmpSrcTemp);

            ::BitBlt(hMemDC, 0, 0, bmpSrc.bmWidth, bmpSrc.bmHeight, hMemDCDraw, 0, 0, SRCCOPY);

            ::SelectObject(hMemDC, hOld);
            ::SelectObject(hMemDCDraw, hOldDraw);

            ::DeleteDC(hMemDCDraw);

            bmihDraw.biBitCount = 24;
        }

        // Create a temp canvas for mix images
        hBmpTemp = ::CreateDIBSection(NULL, (BITMAPINFO *)&bmihDraw, DIB_RGB_COLORS, &pBitsTemp, NULL, NULL);

        if (hBmpTemp == NULL)
            goto Exit0;

        hOld = ::SelectObject(hMemDC, hBmpTemp);

        // Draw background to the temp canvas
        ::BitBlt(hMemDC, 0, 0, cx, cy, hDC, x, y, SRCCOPY);

        psrc = (const BYTE *) pBitsSrc;
        pdst = (BYTE *) pBitsTemp;

        // Just mix it
        for (int nPosY = 0; nPosY < abs(bmpSrc.bmHeight); nPosY ++)
        {
            for (int nPosX = bmpSrc.bmWidth; nPosX > 0; nPosX --)
            {
                if (bmpSrc.bmWidth - nPosX >= srcx && bmpSrc.bmWidth - nPosX < srcx + cx && nPosY >= srcy && nPosY < srcy + cy)
                {
                    BYTE alpha  = psrc[3];
                    BYTE salpha = 255 - alpha;

                    pdst[0] = (BYTE)((psrc[0] * alpha + pdst[0] * salpha) / 255);
                    pdst[1] = (BYTE)((psrc[1] * alpha + pdst[1] * salpha) / 255);
                    pdst[2] = (BYTE)((psrc[2] * alpha + pdst[2] * salpha) / 255);

                    pdst += 3;
                }

                psrc += 4;
            }

            pdst += ((4 - (cx * 3) % 4) & 3);
        }

        // Do the real thing
        bResult = ::BitBlt(hDC, x, y, cx, cy, hMemDC, 0, 0, SRCCOPY);

Exit0:

        if (hMemDC)
        {
            ::SelectObject(hMemDC, hOld);
            ::DeleteObject(hMemDC);
        }

        if (hBmpTemp)
        {
            ::DeleteObject(hBmpTemp);
        }

        if (hBmpSrcTemp)
            ::DeleteObject(hBmpSrcTemp);

        return bResult;
    }

    static BOOL TransparentBlt2(
        HDC hdcDest,            // 目标DC
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

        return TRUE;
    }
};
