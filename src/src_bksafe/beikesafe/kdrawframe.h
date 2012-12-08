////////////////////////////////////////////////////////////////////////////////
//      
//      File for splockhelper
//      
//      File      : kdrawframe.h
//      Version   : 1.0
//      Comment   : 封装绘制一般窗口的方法，使用上下左右共8张图片绘制底图的方法
//      
//      Create at : 2010-3-17
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include <wtl/atlapp.h>
#include <wtl/atlgdi.h>
#include <wtl/atlmisc.h>

class KDrawFrame
{
public:
	KDrawFrame();
	~KDrawFrame();
	BOOL		Init(int nValue = 1);
	BOOL		Uninit();
	void		Draw(CDCHandle& dc, CRect rct, COLORREF clrBk = RGB(235, 248, 222));

	static KDrawFrame& Instance()
	{
		static KDrawFrame _inst;
		return _inst;
	}

private:
	void		LoadBmp(CBitmap& bmp, LPCWSTR strFile);
	CSize		GetBmpSize(CBitmap& bmp);

private:
	CBitmap		m_bmpLeftTop;
	CBitmap		m_bmpLeftBoder;
	CBitmap		m_bmpLeftBotton;

	CBitmap		m_bmpRightTop;
	CBitmap		m_bmpRightBoder;
	CBitmap		m_bmpRightBotton;

	CBitmap		m_bmpTopBoder;
	CBitmap		m_bmpBottonBoder;

	int        m_nStatus;

};