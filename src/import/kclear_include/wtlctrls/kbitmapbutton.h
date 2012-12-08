#ifndef _K_BITMAP_BUTTON_H_
#define _K_BITMAP_BUTTON_H_
#pragma once
#ifndef __ATLCTRLX_H__
#error KBitmapButton.h requires atlctrlx.h to be included first
#endif
#include "ImageHelpers.h"
//#include <atlstr.h>


class KBitmapButton : 
    public CBitmapButtonImpl<KBitmapButton>
{
public:
	//DECLARE_WND_CLASS(_T("OWN_KBitmapButton_Cr"))

    KBitmapButton(DWORD dwExtendedStyle = BMPBTN_AUTOSIZE, HIMAGELIST hImageList = NULL) : 
        CBitmapButtonImpl<KBitmapButton>(dwExtendedStyle, hImageList),
        m_hCursor(NULL), m_nWeight(92)
    { }

    BEGIN_MSG_MAP(KBitmapButton)
        //MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessage)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMessage)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)

        CHAIN_MSG_MAP(CBitmapButtonImpl<KBitmapButton>)
    END_MSG_MAP()


	void DoPaint(CDCHandle dc)
	{
		CBitmapButtonImpl<KBitmapButton>::DoPaint(dc);

		CRect rc;
		GetClientRect(&rc);

		if (!m_strTitle.IsEmpty())
		{		
			// Draw the button's title
			// If button is pressed then "press" title also
			rc.OffsetRect(0, 1);
			if(m_fPressed == 1)
			{
				rc.OffsetRect(1, 1);
			}

			LOGFONT logFont = { 0 };
			logFont.lfCharSet = DEFAULT_CHARSET;
			logFont.lfHeight = m_nWeight;
			if ( m_strFont.GetLength() > 0 )
			{
				::memset(logFont.lfFaceName, 0, LF_FACESIZE);
				::_tcsncpy(logFont.lfFaceName, m_strFont.GetBuffer(), LF_FACESIZE - 1);
			}
			logFont.lfWeight = FW_NORMAL;
			CFont fnt;
			fnt.CreatePointFontIndirect( &logFont );

			HFONT hOldFont = dc.SelectFont(fnt);


			// Center text
			CRect centerRect = rc;
			dc.DrawText(m_strTitle, -1, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
			rc.OffsetRect(
				(centerRect.Width() - rc.Width()) / 2, 
				(centerRect.Height() - rc.Height()) / 2);

			dc.SetBkMode(TRANSPARENT);

			if(!IsWindowEnabled())
			{
				rc.OffsetRect(1, 1);
				dc.SetTextColor(::GetSysColor(COLOR_3DHILIGHT));
				dc.DrawText(m_strTitle, -1, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				rc.OffsetRect(-1, -1);
				dc.SetTextColor(::GetSysColor(COLOR_3DSHADOW));
				dc.DrawText(m_strTitle, -1, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
			else
			{
				bool bHover = IsHoverMode();
				if ((bHover && m_fMouseOver == 1) || m_fPressed) 
				{
					dc.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
					dc.SetBkColor(::GetSysColor(COLOR_BTNFACE));
				} 
				else 
				{
					dc.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
					dc.SetBkColor(::GetSysColor(COLOR_BTNFACE));
				}
				dc.DrawText(m_strTitle, -1, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}

			dc.SelectFont(hOldFont);
		}
	}

    LRESULT OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		bHandled = FALSE;

        if (m_hCursor != NULL && IsWindowEnabled())
            ::SetCursor(m_hCursor);

		
        return 0;
    }

	LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		CRect	rc;
		POINT	pt = { 0, 0 };

		GetCursorPos(&pt);
		ScreenToClient(&pt);
		
		GetClientRect(rc);
		if( ::PtInRect(rc, pt) )
		{
			return TRUE;
		}

		bHandled = FALSE;
		return FALSE;
	}
    
    HCURSOR SetCursor(UINT uID)
    {
        HCURSOR hRet = m_hCursor;
        
		m_hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(uID));
		ATLASSERT(m_hCursor != NULL);

		if (m_hCursor != NULL && IsWindowEnabled())
			::SetCursor(m_hCursor);

        return hRet;
    }

    HIMAGELIST CreateImageList(
        UINT uResourceID,
        int cx, 
        int cy,
        COLORREF clrMask = RGB(255, 0, 255),
        UINT flags = ILC_COLOR16 | ILC_MASK,
		bool bPNG = false)
    {
        HIMAGELIST hImageList = CreateBitmapImageList(
            uResourceID, cx, cy,
            clrMask, flags, bPNG);
        
        ATLASSERT(hImageList != NULL);

        return SetImageList(hImageList);
    }


    static HIMAGELIST CreateBitmapImageList(
        UINT uResourceID,
        int cx, 
        int cy,
        COLORREF clrMask = RGB(255, 0, 255),
        UINT flags = ILC_COLOR16 | ILC_MASK,
		bool bPNG = false)
    {
        HIMAGELIST hImageList = NULL;    
        HBITMAP    hBitbmp    = NULL;

        int nRetCode = -1;

        hImageList = ImageList_Create(cx, cy, flags, 1, 1);
        if (hImageList == NULL)
            goto Exit0;

		HBITMAP hBmp = NULL;
		if (bPNG)
		{
			hBmp = AtlLoadGdiplusImage(uResourceID, _T("PNG"));
		}
		else
		{
	        hBmp = ::LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(uResourceID));
		}

        if (hBmp == NULL)
            goto Exit0;

        nRetCode = ImageList_AddMasked(hImageList, hBmp, clrMask);
        if (nRetCode < 0)
            goto Exit0;

        return hImageList;
    Exit0:
        if (hImageList != NULL)
            ImageList_Destroy(hImageList);

        if (hBmp != NULL)
            DeleteObject(hBmp);

        return NULL;
    }

	void SetBtnImages(UINT uNormal, UINT uDown, UINT uHover, UINT uDisable)
	{
		HIMAGELIST hImageList = NULL;    
		HBITMAP    hBitbmp    = NULL;

		int nRetCode = -1;
		
		CBitmap bmpNormal = AtlLoadGdiplusImage(uNormal, _T("PNG"));
		CBitmap bmpDown = AtlLoadGdiplusImage(uDown, _T("PNG"));
		CBitmap bmpHover = AtlLoadGdiplusImage(uHover, _T("PNG"));
		CBitmap	bmpDisable = AtlLoadGdiplusImage(uDisable, _T("PNG"));

		BITMAP bmpInfo = { 0 };
		bmpNormal.GetBitmap(&bmpInfo);

		hImageList = ImageList_Create(bmpInfo.bmWidth, bmpInfo.bmHeight, ILC_COLOR24, 1, 1);
		if (hImageList == NULL)
			goto Exit0;

		nRetCode = ImageList_AddMasked(hImageList, bmpNormal, 0);
		if (nRetCode < 0)
			goto Exit0;

		nRetCode = ImageList_AddMasked(hImageList, bmpDown, 0);
		if (nRetCode < 0)
			goto Exit0;

		nRetCode = ImageList_AddMasked(hImageList, bmpHover, 0);
		if (nRetCode < 0)
			goto Exit0;

		nRetCode = ImageList_AddMasked(hImageList, bmpDisable, 0);
		if (nRetCode < 0)
			goto Exit0;

		SetImageList(hImageList);
		return;
Exit0:
		if (hImageList != NULL)
			ImageList_Destroy(hImageList);

		return;
	}

	void SetTitle(UINT uID)
	{
		m_strTitle.LoadString(uID);
		Invalidate();
	}

    CString GetTitle()
    {
        return m_strTitle;
       
    }

	void SetFont(const TCHAR* lpszFont, int nWeight)
	{
		if ( lpszFont != NULL )
			m_strFont = lpszFont;

		m_nWeight = nWeight;
	}


private:

    HCURSOR		m_hCursor;
	CString		m_strTitle;
	CString		m_strFont;
	int			m_nWeight;
};


#endif	// #ifndef _K_BITMAP_BUTTON_H_
