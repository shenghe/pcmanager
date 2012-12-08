#ifndef _REALWND_DETAIL_H_
#define _REALWND_DETAIL_H_

#include "SoftMgrListView.h"


class CDetailSoftMgrDlg:public CWindowImpl<CDetailSoftMgrDlg>
{
public:
	CDetailSoftMgrDlg()
	{
		m_pImage = NULL;
		m_hIcon  = NULL;
		m_bSmall = FALSE;
	}

	~CDetailSoftMgrDlg()
	{
	}

public:
	void OnPaint(CDCHandle dc)
	{
		CPaintDC dcPaint(m_hWnd);

		OnPrint((HDC)dcPaint, 0);
	}

	void OnPrint(CDCHandle dc, UINT uFlags)
	{
		CRect rcIcon;
		GetClientRect(rcIcon);
		if (m_pImage)
		{
			Gdiplus::Graphics grap(dc);

			Gdiplus::SolidBrush mySolidBrush(Gdiplus::Color(255,252,253,254));
			grap.FillRectangle(&mySolidBrush, rcIcon.left, rcIcon.top, rcIcon.Width(), rcIcon.Height());

			if (m_bSmall)
				grap.DrawImage( m_pImage, rcIcon.left, rcIcon.top, 16, 16);
			else
				grap.DrawImage( m_pImage, rcIcon.left, rcIcon.top, 32, 32);
		}
		else if (m_hIcon)
		{
			dc.FillSolidRect(rcIcon.left,rcIcon.top,rcIcon.Width(),rcIcon.Height(),RGB(252,253,254));

			if (m_bSmall)
				::DrawIconEx( dc.m_hDC,rcIcon.left, rcIcon.top, m_hIcon, 16,16,0,NULL,DI_NORMAL);
			else
				::DrawIconEx( dc.m_hDC,rcIcon.left, rcIcon.top, m_hIcon, 32,32,0,NULL,DI_NORMAL);
		}
	}

	BOOL OnEraseBkgnd(CDCHandle dc)
	{
		return TRUE;
	}

	void SetDataRes(Gdiplus::Image *pData)
	{
		m_pImage = pData;
	}

	void SetDataICON(HICON hIcon)
	{
		m_hIcon = hIcon;
	}

	void SetSmall(BOOL bSmall)
	{
		m_bSmall = bSmall;
	}

public:
	BEGIN_MSG_MAP(CDetailSoftMgrDlg)  
		MSG_WM_PRINT(OnPrint)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP() 

private:
	Gdiplus::Image *m_pImage;
	HICON			m_hIcon;
	BOOL			m_bSmall;
};


#endif