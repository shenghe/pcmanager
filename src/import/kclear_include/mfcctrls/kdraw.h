#ifndef _KAS_KDRAW_
#define _KAS_KDRAW_

typedef struct _FRG_PARAM 
{
    LONG lOffset;
    COLORREF crColor;
} FRG_PARAM;

namespace KDraw{

    void GradientFillRectV(CDC *pDC, CRect rcFill, FRG_PARAM params[], int nCount);
    void GradientFillRectH(CDC *pDC, CRect rcFill, FRG_PARAM params[], int nCount);

	void DrawGradualColorRect(
		HDC dc,
		const RECT& drawRC, 
		COLORREF clrFirst = RGB(255, 255, 255),
		COLORREF clrEnd = RGB(0, 0, 0),
		BOOL fVertical = TRUE
		); 
	void FillSolidRect(HDC hDC, const RECT& rcZone, COLORREF clr);

    HFONT GetDefaultFont(void);
    HFONT GetBoldFont(void);
    HFONT GetBigFont(void);
    HFONT GetExtraBigFont(void);
	int   GetDefaultFontSize();

    void DrawBitmap(HDC hdc, HBITMAP hBitmap, INT xStart, INT yStart);
    void DrawBitmap(HDC hdc, HBITMAP hBitmap, POINT ptDest, RECT rcSrc);

    void DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, INT xStart, INT yStart, COLORREF crTransparent = RGB(0xFF, 0, 0xFF));
    void DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, POINT ptDest, POINT ptSrc, SIZE sizeSrc, COLORREF crTransparent = RGB(0xFF, 0, 0xFF));

    void DrawCombineButton(
        HDC hdc, HBITMAP hBitmap, RECT rcSrc, LONG lBtnWidth, 
        LONG lSkinLeftSize, LONG lSkinRightSize, COLORREF crBackground = RGB(0xFF, 0xFF, 0xFF), COLORREF crTransparent = RGB(0xFF, 0, 0xFF)
        );

    void StretchBlt(HDC hdcDest, HBITMAP hBmpSrc, RECT rcDest, RECT rcSrc);

    void DrawFrameByBitmap(HDC hDC, RECT rcFrame, UINT uBmpID, SIZE sizeLT);
    void DrawTransparentFrameByBitmap(HDC hDC, RECT rcFrame, UINT uBmpID, SIZE sizeLT, COLORREF crTransparent = RGB(0xFF, 0, 0xFF));

    HBITMAP LoadBitmap(UINT uResID);
    SIZE GetBitmapSize(HBITMAP hbmp);

    void Line(HDC hDC, int x1, int y1, int x2, int y2, COLORREF crColor);
    void FrameRect(HDC hDC, RECT rect, COLORREF crColor);
    
    void StretchBtn(HDC hdcDest, HBITMAP hBmpSkin, int nBtnWidth, RECT rcSkin, int nSkinLWidth);
    
    HRGN CreateRgnFromBitmap(HBITMAP hBmp, int nWidth, int nHeight, COLORREF crTransparent = RGB(0xFF, 0, 0xFF));

    void TransparentBlt2(HDC hdc, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc, int xSrc, int ySrc, int cxSrc, int cySrc, UINT crTransparent);
};



#endif