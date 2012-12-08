////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for shp routines file
//      
//      File      : shpitem.h
//      Comment   : simple html parser misc class
//					
//      Create at : 2008-09-24
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////
#pragma once 
#include <atlmisc.h>
#include <atlstr.h>

typedef ATL::CString	shpstr;
typedef COLORREF		shpclr;

#ifndef ASSERT
#define ASSERT assert
#endif

inline int chrToInt( TCHAR c );

COLORREF _ttoclr( TCHAR* ptszText);

bool draw_text_in_rect( HDC dc, LPCTSTR pszText, int nCnt, LPRECT pRect, int nLineSpace = 0 , BOOL bCalcRect = false );

void _draw_bmp( HDC hdc, CRect rect, HBITMAP bmp );

void _draw_bmp( HDC hdc, int xPos, int yPos, HBITMAP hbmp );

void _draw_bmp_mask( HDC hdc, CRect rect, HBITMAP bmp, shpclr clr_mask );

bool draw_rect_with_bmp( HDC hdc, CRect& rect, HBITMAP h_bmp );

void _draw_strech_bmp( HDC hdc, CRect& rect, HBITMAP h_bmp, HBITMAP h_bmp_left, HBITMAP h_bmp_right );