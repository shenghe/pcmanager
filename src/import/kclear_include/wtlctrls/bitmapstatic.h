//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////

class CBitmapStatic : public CWindowImpl<CBitmapStatic, CStatic>
{
public:
	CBitmapStatic()
	{
		mpImageList = NULL;
		mnIndex = -1;
	}

	DECLARE_WND_SUPERCLASS(0, GetWndClassName())

	BEGIN_MSG_MAP(CBitmapStatic)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

	LRESULT OnPaint(
		UINT uMsg, 
		WPARAM wParam, 
		LPARAM lParam, 
		BOOL &bHandled)
	{
		if (mpImageList == NULL || mnIndex == -1)
			return 0;

	    CPaintDC PaintDC(m_hWnd);
		CRect WndRect;

		GetClientRect(&WndRect);

		mpImageList->Draw(PaintDC, mnIndex, 0, 0, ILD_TRANSPARENT);

		return 0;
	}

public:
	int mnIndex;
	CImageList *mpImageList;
};

//////////////////////////////////////////////////////////////////////////
