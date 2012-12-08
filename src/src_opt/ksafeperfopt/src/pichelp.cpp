////////////////////////////////////////////////////////////////////////////////
//      
//      File for kpfwfrm
//      
//      File      : pichelp.cpp
//      Version   : 1.0
//      Comment   : Í¼Æ¬±£´æ£¬Í¼Æ¬Ìî³ä
//      
//      Create at : 2008-5-7
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "pichelp.h"

BOOL SaveBitmapToFile(HDC hMemDC,HBITMAP hBitmap, LPCTSTR pszFile)
{
	int wBitCount = 32;

	BITMAP             Bitmap;  
	BITMAPFILEHEADER   bmfHdr;    
	BITMAPINFOHEADER   bi;

	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap); 

	bi.biSize = sizeof(BITMAPINFOHEADER);    
	bi.biWidth = Bitmap.bmWidth;    
	bi.biHeight = Bitmap.bmHeight;  
	bi.biPlanes = 1;    
	bi.biBitCount = wBitCount;    
	bi.biCompression = BI_RGB;    
	bi.biSizeImage = 0;  
	bi.biXPelsPerMeter = 0;    
	bi.biYPelsPerMeter = 0;    
	bi.biClrUsed = 0;    
	bi.biClrImportant = 0; 

	DWORD dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight; 

	HANDLE hDib = GlobalAlloc(GHND,dwBmBitsSize+sizeof(BITMAPINFOHEADER)); 
	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);    
	*lpbi = bi;

	GetDIBits(hMemDC, hBitmap, 0, (UINT)Bitmap.bmHeight,  
		(LPSTR)lpbi + sizeof(BITMAPINFOHEADER), (BITMAPINFO *)lpbi, DIB_RGB_COLORS);    

	HANDLE fh = CreateFile(pszFile,
		GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);   

	if (fh == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwDIBSize   =   sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmBitsSize;

	bmfHdr.bfType = 0x4D42;    
	bmfHdr.bfSize = dwDIBSize;  
	bmfHdr.bfReserved1 = 0;    
	bmfHdr.bfReserved2 = 0;    
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);    

	DWORD dwWritten;

	WriteFile(fh,   (LPSTR)&bmfHdr,   sizeof(BITMAPFILEHEADER),   &dwWritten,   NULL);    
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);    
	GlobalUnlock(hDib);    
	GlobalFree(hDib);    
	CloseHandle(fh);
	return TRUE;
}

BOOL SaveIconBmpToFile(HICON hIcon, LPCTSTR pszFile, COLORREF clrBkGnd )
{
	if (hIcon == NULL || pszFile == NULL)
		return FALSE;

	CClientDC dcDesk(GetDesktopWindow());

	CIconHandle icon = hIcon;

	ICONINFO iconInfo = { 0 };

	if ( !icon.GetIconInfo( &iconInfo ) )
		return FALSE;

	CBitmapHandle bmpIcon = iconInfo.hbmColor;

	BITMAP bmpInfo = { 0 };
	bmpIcon.GetBitmap(&bmpInfo);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap( dcDesk, bmpInfo.bmWidth, bmpInfo.bmHeight);

	CDC dcMem;
	dcMem.CreateCompatibleDC( dcDesk );

	HBITMAP hOldBmp = dcMem.SelectBitmap( bmp );

	RECT rcBkGnd = { 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight };

	dcMem.FillSolidRect(&rcBkGnd , clrBkGnd);

	::DrawIconEx( dcMem, 0, 0, 
		hIcon, bmpInfo.bmWidth, bmpInfo.bmHeight,
		0, NULL,  DI_NORMAL );

	dcMem.SelectBitmap( hOldBmp );


	SaveBitmapToFile(dcMem,  bmp, pszFile);

	//must delete bitmap object
	if (iconInfo.hbmColor)
		::DeleteObject( iconInfo.hbmColor );

	//must delete bitmap object
	if ( iconInfo.hbmMask )
		::DeleteObject( iconInfo.hbmMask );

	return TRUE;
}


HBITMAP GetBmpFromIcon( HICON hIcon, HWND hWnd, COLORREF clrBkGnd )
{
	ICONINFO IconInfo;
	::GetIconInfo( hIcon, &IconInfo );

	CBitmapHandle bmpIcon = IconInfo.hbmColor;

	CDC dc;
	dc.Attach( ::GetDC(hWnd) );

	CDC dcMem;
	dcMem.CreateCompatibleDC( dc );

	BITMAP bmpInfo = { 0 };
	bmpIcon.GetBitmap(&bmpInfo);

	CBitmapHandle bmp;
	bmp.CreateCompatibleBitmap( dc, bmpInfo.bmWidth, bmpInfo.bmHeight);

	dcMem.SelectBitmap( bmp );

	RECT rcBkGnd = { 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight };

	dcMem.FillSolidRect(&rcBkGnd , clrBkGnd);

	::DrawIconEx( dcMem, 0, 0, 
		hIcon, bmpInfo.bmWidth, bmpInfo.bmHeight,
		0, NULL,  DI_NORMAL );

	if ( IconInfo.hbmColor )
		DeleteObject( IconInfo.hbmColor );

	if ( IconInfo.hbmMask )
		DeleteObject( IconInfo.hbmMask );

	return bmp;
};

BOOL DrawRectWithBmp( CBitmap& bmp, CRect& rect, CDCHandle& dc )
{
	BOOL bRetCode = FALSE;
	CDC ComDC;
	BITMAP bmpInfo;
	HBITMAP hOldbmp = NULL;

	ComDC.CreateCompatibleDC( dc );
	bmp.GetBitmap( &bmpInfo );
	hOldbmp = ComDC.SelectBitmap( bmp );

	for ( int i = rect.left; i <= rect.right; i = i + bmpInfo.bmWidth )
	{
		bRetCode = ::BitBlt(   dc,
			i,
			rect.top,
			bmpInfo.bmWidth,
			bmpInfo.bmHeight,
			ComDC,
			0,
			0,
			SRCCOPY );
	}

	if ( hOldbmp)
	{
		ComDC.SelectBitmap( hOldbmp );
	}

	return bRetCode;
}



void DrawBmp( HDC hdc, CRect& rect, HBITMAP hbmp )
{
	CBitmapHandle hBmp( hbmp );
	CDC ComDC;
	ComDC.CreateCompatibleDC( hdc );
	BITMAP bmp;
	hBmp.GetBitmap( &bmp );

	ComDC.SelectBitmap( hBmp );

	if ( bmp.bmWidth == rect.Width() && bmp.bmHeight == rect.Height() )
	{
		::BitBlt( hdc,
			rect.left,
			rect.top,
			rect.Width(),
			rect.Height(),
			ComDC,
			0,
			0,
			SRCCOPY );
	}
	else
	{
		::StretchBlt(   hdc,
			rect.left,
			rect.top,
			rect.right - rect.left,
			rect.bottom - rect.top,
			ComDC,
			0,
			0,
			bmp.bmWidth,
			bmp.bmHeight,
			SRCCOPY );
	}
	
}

void DrawBmpBit( HDC hdc, int xPos, int yPos, HBITMAP hbmp )
{
	CBitmapHandle hBmp( hbmp );
	CDC ComDC;
	ComDC.CreateCompatibleDC( hdc );
	BITMAP bmp;
	hBmp.GetBitmap( &bmp );

	ComDC.SelectBitmap( hBmp );

	::BitBlt( hdc,
		xPos,
		yPos,
		bmp.bmWidth,
		bmp.bmHeight,
		ComDC,
		0,
		0,
		SRCCOPY );
}

void DrawBmpBitTransparent( HDC hdc, int xPos, int yPos, HBITMAP hbmp, COLORREF clrMask )
{
	CBitmapHandle hBmp( hbmp );
	CDC ComDC;
	ComDC.CreateCompatibleDC( hdc );
	BITMAP bmp;
	hBmp.GetBitmap( &bmp );

	ComDC.SelectBitmap( hBmp );

	::TransparentBlt( hdc,
		xPos,
		yPos,
		bmp.bmWidth,
		bmp.bmHeight,
		ComDC,
		0,
		0,
		bmp.bmWidth,
		bmp.bmHeight,
		clrMask );
}

void DrawBorder( CRect& rect, HDC hdc, CBitmap& bmpLeft, CBitmap& bmpRight, CBitmap& bmpTop, CBitmap& bmpBottom, HBITMAP hbmpBg )
{

	BITMAP infoLeft, infoRight, infoTop, infoBottom;
	CRect rect_draw(rect);

	if ( bmpLeft.IsNull() || bmpRight.IsNull() || bmpTop.IsNull() || bmpBottom.IsNull() )
	{
		return;
	}

	bmpLeft.GetBitmap( infoLeft );
	rect_draw.right = rect_draw.left + infoLeft.bmWidth;
	DrawBmp( hdc, rect_draw, bmpLeft );

	rect_draw.right = rect.right;
	bmpRight.GetBitmap( infoRight );
	rect_draw.left = rect_draw.right - infoRight.bmWidth;
	DrawBmp( hdc, rect_draw, bmpRight );

	rect_draw.top = rect.top;
	rect_draw.left = rect.left + infoLeft.bmWidth;
	rect_draw.right = rect.right - infoRight.bmWidth;
	bmpTop.GetBitmap( infoTop );
	rect_draw.bottom = rect.top + infoTop.bmHeight;
	DrawBmp( hdc, rect_draw, bmpTop );

	rect_draw.bottom  = rect.bottom;
	bmpBottom.GetBitmap( infoBottom );
	rect_draw.top = rect_draw.bottom - infoBottom.bmHeight;
	DrawBmp( hdc, rect_draw, bmpBottom );

	if ( hbmpBg )
	{
		rect_draw.bottom = rect.bottom - infoBottom.bmHeight;
		rect_draw.top	= rect.top + infoTop.bmHeight;

		DrawBmp( hdc, rect_draw, hbmpBg );
	}
}

void DrawStrechBkG( CRect& rect, HDC hdc, CBitmap& bmpLeft, CBitmap& bmpCenter, CBitmap& bmpRight )
{
	CRect rectDraw(rect);
	BITMAP infoLeft, infoRight;
	bmpLeft.GetBitmap( infoLeft );
	rectDraw.right = rectDraw.left + infoLeft.bmWidth;

	DrawBmp( hdc, rectDraw, bmpLeft );

	bmpRight.GetBitmap( infoRight );
	rectDraw.left = rectDraw.right;
	rectDraw.right = rect.right - infoRight.bmWidth;

	DrawBmp( hdc, rectDraw, bmpCenter );

	rectDraw.left = rectDraw.right;
	rectDraw.right = rect.right;
	DrawBmp( hdc, rectDraw, bmpRight );
}

void DrawStrechBorder( HDC dc, CRect& rect, CBitmap& bmpLeft, CBitmap& bmpRight, CBitmap& bmpTop, CBitmap& bmpBottom,
				 CBitmap& bmpLeftTop, CBitmap& bmpLeftBottom, CBitmap& bmpRightTop, CBitmap& bmpRightBottom )
{
	CRect rectDraw(rect);
	BITMAP infoLeft, infoRight, infoTop, infoBottom;
	BITMAP infoLB, infoRT, infoRB;

	bmpTop.GetBitmap( &infoTop );
	rectDraw.bottom = rectDraw.top + infoTop.bmHeight;
	DrawBmp( dc, rectDraw, bmpTop );

	bmpLeft.GetBitmap( &infoLeft );
	rectDraw.right = rectDraw.left + infoLeft.bmWidth;
	rectDraw.bottom = rect.bottom;
	DrawBmp( dc, rectDraw, bmpLeft );

	bmpBottom.GetBitmap( &infoBottom );
	rectDraw.right = rect.right;
	rectDraw.top = rectDraw.bottom - infoBottom.bmHeight;
	DrawBmp( dc, rectDraw, bmpBottom );

	bmpRight.GetBitmap( &infoRight );
	rectDraw.top = rect.top;
	rectDraw.left = rectDraw.right - infoRight.bmWidth;
	DrawBmp( dc, rectDraw, bmpRight );

	DrawBmpBit( dc, rect.left, rect.top, bmpLeftTop );

	bmpLeftBottom.GetBitmap( &infoLB );
	DrawBmpBit( dc, rect.left, rect.bottom - infoLB.bmHeight, bmpLeftBottom );

	bmpRightTop.GetBitmap( &infoRT );
	DrawBmpBit( dc, rect.right - infoRT.bmWidth, rect.top, bmpRightTop );

	bmpRightBottom.GetBitmap( &infoRB );
	DrawBmpBit( dc, rect.right - infoRB.bmWidth, rect.bottom - infoRB.bmHeight, bmpRightBottom );
}