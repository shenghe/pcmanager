////////////////////////////////////////////////////////////////////////////////
//      
//      File for kpfwfrm
//      
//      File      : bmpbtn.h
//      Version   : 1.0
//      Comment   : ÍøïÚÍ¼ÐÎ°´Å¥¿Ø¼þ
//      
//      Create at : 2008-09-19
//      Create by : chenguicheng
//      
///////////////////////////////////////////////////////////////////////////////

#pragma once 
#include <wtl/atlctrls.h>
#include <wtl/atlctrlx.h>
#include <atlstr.h>

class CBmpBtn : public CBitmapButtonImpl<CBmpBtn>
{
public:

	DECLARE_WND_SUPERCLASS(_T("WTL_BitmapButton"), GetWndClassName())

	CBmpBtn(DWORD dwExtendedStyle = BMPBTN_AUTOSIZE, HIMAGELIST hImageList = NULL)
		:m_hCursor(NULL),m_pFont(NULL),
		CBitmapButtonImpl<CBmpBtn>(dwExtendedStyle, hImageList)
	{
	}

	~CBmpBtn()
	{
		if( m_hCursor )
			::DestroyCursor(m_hCursor);
	}

	void SetBtnText( LPCTSTR lpText )
	{
		m_strText = lpText;
	}

	void	SetFont(CFont* pFont)
	{
		m_pFont = pFont;
	}

	BEGIN_MSG_MAP(CBmpBtn)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		CHAIN_MSG_MAP(CBitmapButtonImpl<CBmpBtn>)
	END_MSG_MAP()


	void DoPaint(CDCHandle dc)
	{

		ATLASSERT(m_ImageList.m_hImageList != NULL);   // image list must be set
		ATLASSERT(m_nImage[0] != -1);                  // main bitmap must be set

		// set bitmap according to the current button state
		int nImage = -1;
		bool bHover = IsHoverMode();
		if(!IsWindowEnabled())
			nImage = m_nImage[_nImageDisabled];
		else if(m_fPressed == 1)
			nImage = m_nImage[_nImagePushed];
		else if((!bHover && m_fFocus == 1) || (bHover && m_fMouseOver == 1))
			nImage = m_nImage[_nImageFocusOrHover];
		if(nImage == -1)   // not there, use default one
			nImage = m_nImage[_nImageNormal];

		// draw the button image
		int xyPos = 0;
		if((m_fPressed == 1) && ((m_dwExtendedStyle & (BMPBTN_AUTO3D_SINGLE | BMPBTN_AUTO3D_DOUBLE)) != 0) && (m_nImage[_nImagePushed] == -1))
			xyPos = 1;
		m_ImageList.Draw(dc, nImage, xyPos, xyPos, ILD_NORMAL);

		// draw 3D border if required
		if((m_dwExtendedStyle & (BMPBTN_AUTO3D_SINGLE | BMPBTN_AUTO3D_DOUBLE)) != 0)
		{
			RECT rect;
			GetClientRect(&rect);

			if(m_fPressed == 1)
				dc.DrawEdge(&rect, ((m_dwExtendedStyle & BMPBTN_AUTO3D_SINGLE) != 0) ? BDR_SUNKENOUTER : EDGE_SUNKEN, BF_RECT);
			else if(!bHover || m_fMouseOver == 1)
				dc.DrawEdge(&rect, ((m_dwExtendedStyle & BMPBTN_AUTO3D_SINGLE) != 0) ? BDR_RAISEDINNER : EDGE_RAISED, BF_RECT);

			if(!bHover && m_fFocus == 1)
			{
				::InflateRect(&rect, -2 * ::GetSystemMetrics(SM_CXEDGE), -2 * ::GetSystemMetrics(SM_CYEDGE));
				dc.DrawFocusRect(&rect);
			}
		}

		if( m_strText.GetLength() > 0 )
		{
			RECT rect;
			GetClientRect( &rect );
			HFONT hOldFont = NULL;
		
			if (!m_pFont)
				hOldFont = dc.SelectFont ( (HFONT)::GetStockObject(DEFAULT_GUI_FONT) );
			else
				hOldFont = dc.SelectFont(*m_pFont);

			dc.SetBkMode( TRANSPARENT );

			dc.DrawText( m_strText, m_strText.GetLength(), &rect, DT_VCENTER|DT_CENTER|DT_SINGLELINE );

			if ( hOldFont )
				dc.SelectFont( hOldFont );
		}
	}

	HRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if ( m_hCursor )
		{
			::SetCursor( m_hCursor );
		}
		return TRUE;
	}

	void SetBmp( LPCTSTR pszNormal, LPCTSTR pszDown = NULL, LPCTSTR pszOver = NULL, LPCTSTR hDisable = NULL )
	{
		if (LoadBmp(pszNormal, m_hBmpNormal) && 
			LoadBmp(pszOver, m_hBmpHover) && 
			LoadBmp(pszDown, m_hBmpDown) )
		{
			SetBmpHandle(m_hBmpNormal, m_hBmpDown, m_hBmpHover);
		}
	}

	void SetBmpHandle( HBITMAP hNormal, HBITMAP hDown = NULL, HBITMAP hOver = NULL, HBITMAP hDisable = NULL )
	{
		CImageList ImageList;
		BITMAP bm = {0};
		CBitmapHandle bitmap = hNormal;
		int iNormal = -1;
		int iDown = -1;
		int iOver = -1;
		int iDisable = -1;

		bitmap.GetBitmap( &bm );
		ImageList.Create( bm.bmWidth, bm.bmHeight, ILC_COLOR24, 4, 1);
		
		if ( hNormal )
		{
			iNormal = 0;
			ImageList.Add( hNormal );
			::DeleteObject( hNormal );
			hNormal = NULL;
		}

		if ( hOver )
		{
			iOver = 1;
			ImageList.Add( hOver );
			::DeleteObject( hOver );
			hOver = NULL;
		}

		if ( hDown )
		{
			iDown = 2;
			ImageList.Add( hDown );
			::DeleteObject( hDown );
			hDown = NULL;
		}
		
		if ( hDisable )
		{
			iDisable = 3;
			ImageList.Add( hDisable );
			::DeleteObject( hDisable );
			hDisable = NULL;
		}

		SetBitmapButtonExtendedStyle( BMPBTN_HOVER );
		HIMAGELIST hImageListPre = SetImageList( ImageList );

		if ( hImageListPre )
			ImageList_Destroy( hImageListPre );

		SetImages( iNormal, iDown ,iOver, iDisable );

		SetBitmapButtonExtendedStyle( BMPBTN_HOVER );

		//ImageList.Destroy();
	}

	BOOL SetBtnCursor(LPCTSTR szCursorId)
	{
		// Destroy any previous cursor
		if (m_hCursor != NULL)
		{
			::DestroyCursor(m_hCursor);
			m_hCursor = NULL;
		}	

		// If we want a cursor
		if (szCursorId)
		{		
			// Load icon resource
			m_hCursor = (HCURSOR)::LoadCursor(
				NULL,
				szCursorId
				);
			// If something wrong then return FALSE
			if (m_hCursor == NULL)
			{
				return FALSE;
			}
		}

		return TRUE;
	}

protected:

	BOOL	LoadBmp(LPCTSTR strBmp, CBitmap& bmp)
	{
		HBITMAP handle = (HBITMAP)::LoadImage( _AtlBaseModule.GetResourceInstance(),
			strBmp, IMAGE_BITMAP, 
			0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
		if (handle)
		{
			bmp.Attach(handle);
			return TRUE;
		}
		return FALSE;
	}

private:

	HCURSOR m_hCursor;
	ATL::CString m_strText;

	CBitmap	m_hBmpNormal;
	CBitmap	m_hBmpHover;
	CBitmap	m_hBmpDown;
	CFont*	m_pFont;
};