////////////////////////////////////////////////////////////////////////////////
//      
//      File for splockhelper
//      
//      File      : kdrawframe.cpp
//      Version   : 1.0
//      Comment   : 封装绘制一般窗口的方法，使用上下左右共8张图片绘制底图的方法
//      
//      Create at : 2010-3-17
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "kdrawframe.h"
#include <atlpath.h>

KDrawFrame::KDrawFrame()
: m_nStatus(0)
{

}

KDrawFrame::~KDrawFrame()
{

}

BOOL KDrawFrame::Uninit()
{
	m_bmpLeftTop.DeleteObject();
	m_bmpLeftBoder.DeleteObject();
	m_bmpLeftBotton.DeleteObject();

	m_bmpRightTop.DeleteObject();
	m_bmpRightBoder.DeleteObject();
	m_bmpRightBotton.DeleteObject();

	m_bmpTopBoder.DeleteObject();
	m_bmpBottonBoder.DeleteObject();
	return FALSE;
}

void KDrawFrame::Draw(CDCHandle& dc, CRect rct, COLORREF clrBk)
{
	CBrush brush;
	brush.CreateSolidBrush(clrBk);
	HBRUSH hOldBrush = dc.SelectBrush(brush);
	// 绘制背景
	dc.Rectangle(&rct);

	int left, top, right, botton;
	CSize size = GetBmpSize(m_bmpLeftTop);
	left = size.cx;
	top = size.cy;

	size = GetBmpSize(m_bmpRightBotton);
	right = size.cx;
	botton = size.cy;

	CDC hTmpDC;
	hTmpDC.CreateCompatibleDC(dc);

	HBITMAP bOldbmp = hTmpDC.SelectBitmap(m_bmpLeftTop);
	dc.BitBlt(0, 0, left, top, hTmpDC, 0, 0, SRCCOPY);
	
	size = GetBmpSize(m_bmpTopBoder);
	hTmpDC.SelectBitmap(m_bmpTopBoder);
	dc.StretchBlt(left, 0, rct.right-right, top, hTmpDC, 0, 0, size.cx, size.cy, SRCCOPY);

	hTmpDC.SelectBitmap(m_bmpRightTop);
	dc.BitBlt(rct.right-right, 0, rct.right, top, hTmpDC, 0, 0, SRCCOPY);

	size = GetBmpSize(m_bmpLeftBoder);
	hTmpDC.SelectBitmap(m_bmpLeftBoder);
	dc.StretchBlt(0, top, left, rct.bottom-botton-top, hTmpDC, 0, 0, size.cx, size.cy, SRCCOPY);
	
	size = GetBmpSize(m_bmpRightBoder);
	hTmpDC.SelectBitmap(m_bmpRightBoder);
	dc.StretchBlt(rct.right-right, top, right, rct.bottom-botton-top, hTmpDC, 0, 0, size.cx, size.cy, SRCCOPY);

	hTmpDC.SelectBitmap(m_bmpLeftBotton);
	dc.BitBlt(0, rct.bottom-botton, left, botton, hTmpDC, 0, 0, SRCCOPY);

	size = GetBmpSize(m_bmpBottonBoder);
	hTmpDC.SelectBitmap(m_bmpBottonBoder);
	dc.StretchBlt(left, rct.bottom-botton, rct.right-right, botton, hTmpDC, 0, 0, size.cx, size.cy, SRCCOPY);

	hTmpDC.SelectBitmap(m_bmpRightBotton);
	dc.BitBlt(rct.right-right, rct.bottom-botton, rct.right, botton, hTmpDC, 0, 0, SRCCOPY);
	
	hTmpDC.SelectBitmap(bOldbmp);
	dc.SelectBrush(hOldBrush);
}

//extern HMODULE  g_hModule;
BOOL KDrawFrame::Init(int nValue /*= 0*/)
{
	TCHAR tszPath[MAX_PATH*2] = {0};
	GetModuleFileName( (HINSTANCE)&__ImageBase, tszPath, MAX_PATH * 2 );
	CPath path( tszPath );
	path.RemoveFileSpec();
	path.Append( TEXT("webui/splock/images/") );

	m_nStatus = nValue;

	if (-1 == m_nStatus)
	{
		LoadBmp(m_bmpLeftTop, ATL::CString(path+TEXT("red_top_left.bmp")));
		LoadBmp(m_bmpLeftBoder, ATL::CString(path+TEXT("red_left.bmp")));
		LoadBmp(m_bmpLeftBotton, ATL::CString(path+TEXT("red_bottom_left.bmp")));
		LoadBmp(m_bmpRightTop, ATL::CString(path+TEXT("red_top_right.bmp")));
		LoadBmp(m_bmpRightBoder, ATL::CString(path+TEXT("red_right.bmp")));
		LoadBmp(m_bmpRightBotton, ATL::CString(path+TEXT("red_bottom_right.bmp")));
		LoadBmp(m_bmpTopBoder, ATL::CString(path+TEXT("red_top_middle.bmp")));
		LoadBmp(m_bmpBottonBoder, ATL::CString(path+TEXT("red_bottom_middle.bmp")));
	}
	else
	{
		LoadBmp(m_bmpLeftTop, ATL::CString(path+TEXT("xzbh_top_left.bmp")));
		LoadBmp(m_bmpLeftBoder, ATL::CString(path+TEXT("xzbh_left.bmp")));
		LoadBmp(m_bmpLeftBotton, ATL::CString(path+TEXT("xzbh_bottom_left.bmp")));
		LoadBmp(m_bmpRightTop, ATL::CString(path+TEXT("xzbh_top_right.bmp")));
		LoadBmp(m_bmpRightBoder, ATL::CString(path+TEXT("xzbh_right.bmp")));
		LoadBmp(m_bmpRightBotton, ATL::CString(path+TEXT("xzbh_bottom_right.bmp")));
		LoadBmp(m_bmpTopBoder, ATL::CString(path+TEXT("xzbh_top_middle.bmp")));
		LoadBmp(m_bmpBottonBoder, ATL::CString(path+TEXT("xzbh_bottom_middle.bmp")));
		
	}		
	return TRUE;
}

void KDrawFrame::LoadBmp(CBitmap& bmp, LPCWSTR strFile)
{
	HBITMAP handle = (HBITMAP)::LoadImage( _AtlBaseModule.GetResourceInstance(),
		strFile, IMAGE_BITMAP, 
		0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
	if (handle)
	{
		bmp.Attach(handle);
	}
	return;
}

CSize KDrawFrame::GetBmpSize(CBitmap& bmp)
{
	CSize size;
	bmp.GetSize(size);
	size.cx = abs(size.cx);
	size.cy = abs(size.cy);
	return size;
}