////////////////////////////////////////////////////////////////////////////////
//      
//      File for kpfwfrm
//      
//      File      : pichelp.h
//      Version   : 1.0
//      Comment   : Í¼Æ¬±£´æ
//      
//      Create at : 2008-5-7
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////


#pragma once 

BOOL SaveBitmapToFile(HDC hMemDC,HBITMAP hBitmap, LPCTSTR pszFile);

BOOL SaveIconBmpToFile(HICON hIcon, LPCTSTR pszFile, COLORREF clrBkGnd = 0x00FFFFFF );

//add by chenguicheng
HBITMAP GetBmpFromIcon( HICON hIcon, HWND hWnd, COLORREF clrBkGnd = RGB(255,255,255) );

BOOL DrawRectWithBmp( CBitmap& bmp, CRect& rect, CDCHandle& dc );


void DrawBmp( HDC hdc, CRect& rect, HBITMAP hbmp );

void DrawBmpBit( HDC hdc, int xPos, int yPos, HBITMAP hbmp );

void DrawBmpBitTransparent( HDC hdc, int xPos, int yPos, HBITMAP hbmp, COLORREF clrMask );

void DrawBorder( CRect& rect, HDC hdc, CBitmap& bmpLeft, CBitmap& bmpRight, CBitmap& bmpTop, CBitmap& bmpBottom, HBITMAP hbmpBg = NULL );

void DrawStrechBkG( CRect& rect, HDC hdc, CBitmap& bmpLeft, CBitmap& bmpCenter, CBitmap& bmpRight );
void DrawStrechBorder( HDC dc, CRect& rect, CBitmap& bmpLeft, CBitmap& bmpRight, CBitmap& bmpTop, CBitmap& bmpBottom,
				 CBitmap& bmpLeftTop, CBitmap& bmpLeftBottom, CBitmap& bmpRightTop, CBitmap& bmpRightBottom );