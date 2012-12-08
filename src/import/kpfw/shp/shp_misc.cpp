////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for shp routines file
//      
//      File      : shpitem.cpp
//      Comment   : simple html parser misc class
//					
//      Create at : 2008-09-24
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "shp_misc.h"
#include <assert.h>

int chrToInt( TCHAR c )
{
	int index = c;
	index = index - 48;
	if ( index > 9 )
		index = index - 7;
	return index;
}

COLORREF _ttoclr( TCHAR* ptszText)	
{
	TCHAR* pText = _tcsupr( ptszText );

	int nHigh = chrToInt( *pText );
	int nLow = chrToInt( *(++pText) );
	int r = nHigh * 16 + nLow; 

	nHigh = chrToInt( *(++pText) );
	nLow = chrToInt( *(++pText) );
	int g = nHigh * 16 + nLow;

	nHigh = chrToInt( *(++pText) );
	nLow = chrToInt( *(++pText) );
	int b = nHigh * 16 + nLow;
	return RGB( r, g, b );
}


bool draw_text_in_rect( HDC dc, LPCTSTR pszText, int nCnt, LPRECT pRect, int nLineSpace, BOOL bCalcRect )
{
	ASSERT(dc);
	LPTSTR pText = (LPTSTR)pszText;
	TEXTMETRIC tm;
	int nWidth = 0;
	int nRectWidth = pRect->right - pRect->left;
	int nTop = pRect->top;
	int nWordCnt = 0;

	::GetTextMetrics( dc, &tm );

	for( int i = 0; i < nCnt; i ++ )
	{
		if ( *(pszText + i) >=0 && *(pszText + i) <= 255 )
			nWidth += tm.tmAveCharWidth;
		else
			nWidth += tm.tmAveCharWidth * 2;

		if ( nWidth > nRectWidth )
		{
			if ( !bCalcRect )
				::TextOut( dc, pRect->left, nTop, pText, nWordCnt  );

			pText +=  nWordCnt ;

			nWidth = 0;
			i = i -1;
			nTop += tm.tmHeight + tm.tmExternalLeading + nLineSpace;
			nWordCnt = 0;
		}
		else
		{
			nWordCnt++;
		}
	}

	if ( nWidth > 0  && !bCalcRect)
	{
		::TextOut( dc, pRect->left, nTop, pText, _tcslen(pText) );
	}

	if ( bCalcRect )
	{
		pRect->bottom = nTop + tm.tmHeight + tm.tmExternalLeading ;
	}

	return TRUE;
}

void _draw_bmp( HDC hdc, CRect rect, HBITMAP hbmp )
{
	CBitmapHandle hBmp( hbmp );
	CDC ComDC;
	HBITMAP hOldBmp = NULL;
	ComDC.CreateCompatibleDC( hdc );
	BITMAP bmp;
	hBmp.GetBitmap( &bmp );

	hOldBmp = ComDC.SelectBitmap( hBmp );

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

	ComDC.SelectBitmap( hOldBmp );
}

void _draw_bmp( HDC hdc, int xPos, int yPos, HBITMAP hbmp )
{
	CBitmapHandle hBmp( hbmp );
	CDC ComDC;
	HBITMAP hOldBmp = NULL;
	ComDC.CreateCompatibleDC( hdc );
	BITMAP bmp;
	hBmp.GetBitmap( &bmp );

	hOldBmp = ComDC.SelectBitmap( hBmp );

	::BitBlt(   hdc,
		0,
		0,
		bmp.bmWidth,
		bmp.bmHeight,
		ComDC,
		xPos,
		yPos,
		SRCCOPY );

	ComDC.SelectBitmap( hOldBmp );
}

void _draw_bmp_mask( HDC hdc, CRect rect, HBITMAP hbmp, shpclr clr_mask )
{
	CBitmapHandle hBmp( hbmp );
	CDC ComDC;
	HBITMAP hOldBmp = NULL;
	ComDC.CreateCompatibleDC( hdc );
	BITMAP bmp;
	hBmp.GetBitmap( &bmp );

	hOldBmp = ComDC.SelectBitmap( hBmp );

	::TransparentBlt(   hdc,
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		ComDC,
		0,
		0,
		bmp.bmWidth,
		bmp.bmHeight,
		clr_mask );

	ComDC.SelectBitmap( hOldBmp );
}

void _draw_strech_bmp( HDC hdc, CRect& rect, HBITMAP h_bmp, HBITMAP h_bmp_left, HBITMAP h_bmp_right )
{
	CBitmapHandle hBmp( h_bmp );
	CBitmapHandle hBmpLeft( h_bmp_left );
	CBitmapHandle hBmpRight( h_bmp_right );
	CRect rect_draw(rect);

	CDC ComDC;
	ComDC.CreateCompatibleDC( hdc );
	BITMAP bmp, bmpLeft, bmpRight;
	hBmp.GetBitmap( &bmp );
	hBmpLeft.GetBitmap( &bmpLeft );
	hBmpRight.GetBitmap( &bmpRight );

	rect_draw.right = rect_draw.left + bmpLeft.bmWidth;
	_draw_bmp( hdc, rect_draw, h_bmp_left );

	rect_draw.left = rect_draw.right;
	rect_draw.right = rect.right - bmpRight.bmWidth;
	draw_rect_with_bmp( hdc, rect_draw, h_bmp );

	rect_draw.right = rect.right;
	rect_draw.left = rect_draw.right - bmpRight.bmWidth;
	_draw_bmp( hdc, rect_draw, h_bmp_right);
}

bool draw_rect_with_bmp( HDC hdc, CRect& rect, HBITMAP h_bmp )
{
	bool bRetCode = false;
	CDC ComDC;
	BITMAP bmpInfo;
	HBITMAP hOldbmp = NULL;
	CDCHandle dc(hdc);
	CBitmapHandle bmp(h_bmp);

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