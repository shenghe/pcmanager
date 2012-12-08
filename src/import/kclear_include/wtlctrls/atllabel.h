/////////////////////////////////////////////////////////////////////////////
//
// Original CLabel Class written by Norm Almond
//
/////////////////////////////////////////////////////////////////////////////
//
// CLabel Version 1.2
//
// From now on I'll try to keep a log of fixes and enhancements...
// 
// The new feature were added due to the response of people.
// All I ask is to all you programmers out there, is if you add, fix or
// enhance this code, sent me a copy and I'll send the copy on to www.codeproject.com
//
// Happy Software Engineer :)
// 
// New features include:
//
// A. Support for 3D Fonts
// B. Support for background transparency
// C. More comments provided
// D. If alignment is 'centered' and the window text is seperated by '\r\n'
//	  the will be centered accordingly - requested by someone @ nasa ;)
// E. Support for font rotation.
// F. Respond to System Color Change
// G. OnPaint improved performance - using Double Buffering Technique
//
// Thanks to:
// Mark McDowell	- For suggestion on 'Increasing the flexibility of "hypertext" setting...'
// Erich Ruth		- For suggestion on 'Font Rotation'
//

/////////////////////////////////////////////////////////////////////////////
// CLabel Version 1.3
//
// A. Added SS_LEFTNOWORDWRAP to include wordwrap
// B. Fix repainting problem 
// C. Fix SetBkColor
// D. Added SS_CENTER

// Thanks to:
// Marius						- Added styling problem.
// Azing Vondeling & Broker		- Spotting painting Problem.
// Mel Stober					- Back Color & SS_CENTER
// 
/////////////////////////////////////////////////////////////////////////////
// CLabel Version 1.4
//
// A. Fix to transparency mode
// B. Added new SetText3DHiliteColor to change the 3D Font face color - default is white.
// 
// Thanks to:
// michael.groeger				- Spotting Transparency with other controls bug.
//
//
/////////////////////////////////////////////////////////////////////////////
// CLabel Version 1.5
//
// A. Sanity handle check
// B. Support Interface Charset
// C. Check compilition with _UNICODE
// D. Fix hyperlink feature
// E. Support default Dialog Font
// F. Inclusion of SS_OWNERDRAW via control creation and subclassing
// G. Modification to Text aligmnent code
// H. New background gradient fill function
// 
// Thanks to:
// Steve Kowald				- Using null handles 
// Alan Chan				- Supporting International Windows
// Dieter Fauth				- Request for default Dialog font
// Herb Illfelder			- Text Alignment code
// 
//
/////////////////////////////////////////////////////////////////////////////
// CLabel Version 1.5a
//
// A. Port to ATL / WTL (Rashid Thadha) 10/12/2000
// 
//
/////////////////////////////////////////////////////////////////////////////

#include "shellapi.h "

#if !defined(ATL_LABEL_H)
#define ATL_LABEL_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ATLLabel.h : header file
//

#define	NM_LINKCLICK	WM_APP + 0x200
#if (WINVER < 0x0500)
__declspec(selectany) struct
{
	enum { cxWidth = 32, cyHeight = 32 };
	int xHotSpot;
	int yHotSpot;
	unsigned char arrANDPlane[cxWidth * cyHeight / 8];
	unsigned char arrXORPlane[cxWidth * cyHeight / 8];
} _AtlLabel_CursorData = 
{
	5, 0, 
	{
		0xF9, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 
		0xF0, 0xFF, 0xFF, 0xFF, 0xF0, 0x3F, 0xFF, 0xFF, 0xF0, 0x07, 0xFF, 0xFF, 0xF0, 0x01, 0xFF, 0xFF, 
		0xF0, 0x00, 0xFF, 0xFF, 0x10, 0x00, 0x7F, 0xFF, 0x00, 0x00, 0x7F, 0xFF, 0x00, 0x00, 0x7F, 0xFF, 
		0x80, 0x00, 0x7F, 0xFF, 0xC0, 0x00, 0x7F, 0xFF, 0xC0, 0x00, 0x7F, 0xFF, 0xE0, 0x00, 0x7F, 0xFF, 
		0xE0, 0x00, 0xFF, 0xFF, 0xF0, 0x00, 0xFF, 0xFF, 0xF0, 0x00, 0xFF, 0xFF, 0xF8, 0x01, 0xFF, 0xFF, 
		0xF8, 0x01, 0xFF, 0xFF, 0xF8, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 
		0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0xC0, 0x00, 0x00, 0x06, 0xD8, 0x00, 0x00, 
		0x06, 0xDA, 0x00, 0x00, 0x06, 0xDB, 0x00, 0x00, 0x67, 0xFB, 0x00, 0x00, 0x77, 0xFF, 0x00, 0x00, 
		0x37, 0xFF, 0x00, 0x00, 0x17, 0xFF, 0x00, 0x00, 0x1F, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 
		0x0F, 0xFE, 0x00, 0x00, 0x07, 0xFE, 0x00, 0x00, 0x07, 0xFE, 0x00, 0x00, 0x03, 0xFC, 0x00, 0x00, 
		0x03, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	}
};
#endif //(WINVER < 0x0500)

// =============================================================================================
// CLabel window

class CLabel :	public CWindowImpl<CLabel, CStatic>      
{
// Construction
public:
	static enum FlashType {None, Text, Background };
	static enum Type3D { Raised, Sunken};
	static enum BackFillMode { Normal, Gradient };

	BEGIN_MSG_MAP(CLabel)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSysColorChange)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	END_MSG_MAP()

	// =============================================================================================
	// Function name	: CLabel::CLabel
	// Description	    : Default contructor
	// Return type		: 
	// =============================================================================================
	CLabel::CLabel() :	m_bPaintLabel(true), m_hBackBrush(NULL), m_crHiColor(0), m_crLoColor(0),
						m_bTimer(FALSE), m_bState(FALSE), m_bTransparent(FALSE), m_bLink(TRUE),
						m_hCursor(NULL), m_Type(None), m_bFont3d(FALSE), m_bNotifyParent(FALSE),
						m_bToolTips(FALSE), m_bRotation(FALSE), m_fillmode(Normal), m_hFont(NULL)
	{
		m_crText = GetSysColor(COLOR_WINDOWTEXT);
		m_cr3DHiliteColor =	RGB(255,255,255);		
		m_hwndBrush = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	}

	// =============================================================================================
	// Function name	: ~CLabel
	// Description	    : Destructor
	// Return type		: virtual 
	// =============================================================================================
	virtual ~CLabel()
	{
		if(m_hFont != NULL)
			::DeleteObject(m_hFont);
		if (m_hwndBrush)
			::DeleteObject(m_hwndBrush);

		// Stop Checking complaining
		if (m_hBackBrush)
			::DeleteObject(m_hBackBrush);
		
#if (WINVER < 0x0500)
		// It was created, not loaded, so we have to destroy it
		if(m_hCursor != NULL)
			::DestroyCursor(m_hCursor);
#endif //(WINVER < 0x0500)
	}

	// =============================================================================================
	// Function name	: SetBkColor
	// Description	    : Sets background color
	// Return type		: Reference to 'this' object
	// Argument         : COLORREF crBkgnd
	// Argument         : COLORREF crBkgndHigh = 0
	// Argument         : BackFillMode mode = Normal
	// =============================================================================================
	virtual CLabel& SetBkColor(COLORREF crBkgnd, COLORREF crBkgndHigh = 0, BackFillMode mode = Normal)
	{
		m_crLoColor = crBkgnd;
		m_crHiColor = crBkgndHigh;

		m_fillmode = mode;

		if (m_hBackBrush)
			::DeleteObject(m_hBackBrush);
		
		if (m_fillmode == Normal)
			m_hBackBrush = ::CreateSolidBrush(crBkgnd);

		UpdateSurface();

		return *this;
	}

	// =============================================================================================
	// Function name	: SetTextColor
	// Description	    : Sets the text color
	// Return type		: Reference to 'this' object
	// Argument         : COLORREF crText
	// =============================================================================================
	virtual CLabel& SetTextColor(COLORREF crText)
	{
		m_crText = crText;		
		UpdateSurface();
		
    return *this;
	}

	// =============================================================================================
	// Function name	: SetText
	// Description	    : Short cut to set window text - caption - label
	// Return type		: Reference to 'this' object
	// Argument         : const CString& strText
	// =============================================================================================
    virtual CLabel& SetText(const ATL::CString& strText)
	{
		SetWindowText(strText);
		UpdateSurface();

		return *this;
	}

	// =============================================================================================
	// Function name	: SetFontBold
	// Description	    : Sets the font ot bold 
	// Return type		: Reference to 'this' object
	// Argument         : BOOL bBold
	// =============================================================================================
	virtual CLabel& SetFontBold(BOOL bBold)
	{	

		m_lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
		ReconstructFont();
		UpdateSurface();
		return *this;
	}
	
	// =============================================================================================
	// Function name	: SetFontName
	// Description	    : Sets the fonts face name
	// Return type		: Reference to 'this' object
	// Argument         : const CString& strFont
	// Argument         : BYTE byCharSet = ANSI_CHARSET
	// =============================================================================================
    virtual CLabel& SetFontName(const ATL::CString& strFont, BYTE byCharSet = ANSI_CHARSET)
	{	
		m_lf.lfCharSet = byCharSet;

		_tcscpy(m_lf.lfFaceName,strFont);
		ReconstructFont();
		UpdateSurface();

		return *this;
	}
	
	// =============================================================================================
	// Function name	: SetFontUnderline
	// Description	    : Sets font underline attribue
	// Return type		: Reference to 'this' object
	// Argument         : BOOL bSet
	// =============================================================================================
	virtual CLabel& SetFontUnderline(BOOL bSet)
	{	
		m_lf.lfUnderline = bSet;
		ReconstructFont();
		UpdateSurface();

		return *this;
	}

	// =============================================================================================
	// Function name	: SetFontItalic
	// Description	    : Sets font italic attribute 
	// Return type		: Reference to 'this' object
	// Argument         : BOOL bSet
	// =============================================================================================
	virtual CLabel& SetFontItalic(BOOL bSet)
	{
		m_lf.lfItalic = bSet;
		ReconstructFont();
		UpdateSurface();

		return *this;	
	}

	// =============================================================================================
	// Function name	: SetFontSize
	// Description	    : Sets the font size 
	// Return type		: Reference to 'this' object
	// Argument         : int nSize
	// =============================================================================================
	virtual CLabel& SetFontSize(int nSize)
	{
		CFont cf;
		LOGFONT lf;

		cf.CreatePointFont(nSize * 10, m_lf.lfFaceName);
		cf.GetLogFont(&lf);

		m_lf.lfHeight = lf.lfHeight;
		m_lf.lfWidth  = lf.lfWidth;

		ReconstructFont();
		UpdateSurface();

		return *this;
	}

	// =============================================================================================
	// Function name	: SetSunken
	// Description	    : Sets sunken effect on border
	// Return type		: Reference to 'this' object
	// Argument         : BOOL bSet
	// =============================================================================================
	virtual CLabel& SetSunken(BOOL bSet)
	{

		if (!bSet)
			ModifyStyleEx(WS_EX_STATICEDGE,0,SWP_DRAWFRAME);
		else
			ModifyStyleEx(0,WS_EX_STATICEDGE,SWP_DRAWFRAME);
			
		return *this;	
	}

	// =============================================================================================
	// Function name	: SetBorder
	// Description	    : Toggles the border on/off
	// Return type		: Reference to 'this' object
	// Argument         : BOOL bSet
	// =============================================================================================
	virtual CLabel& SetBorder(BOOL bSet)
	{

		if (!bSet)
			ModifyStyle(WS_BORDER,0,SWP_DRAWFRAME);
		else
			ModifyStyle(0,WS_BORDER,SWP_DRAWFRAME);
			
		return *this;	
	}

	// =============================================================================================
	// Function name	: SetTransparent
	// Description	    : Sets the Label window to be transpaent
	// Return type		: Reference to 'this' object
	// Argument         : BOOL bSet
	// =============================================================================================
	virtual CLabel& SetTransparent(BOOL bSet)
	{

		m_bTransparent = bSet;
		ModifyStyleEx(0,WS_EX_TRANSPARENT); // Fix for transparency
		UpdateSurface();

		return *this;
	}

	// =============================================================================================
	// Function name	: FlashText
	// Description	    : As the function states
	// Return type		: Reference to 'this' object
	// Argument         : BOOL bActivate
	// =============================================================================================
	virtual CLabel& FlashText(BOOL bActivate)
	{
		if (m_bTimer)
			KillTimer(1);

		if (bActivate)
		{
			m_bState = FALSE;			
			m_bTimer = TRUE;			
			SetTimer(1,500);
			m_Type = Text;
		}
		else
			m_Type = None; // Fix

		return *this;
	}

	// =============================================================================================
	// Function name	: FlashBackground
	// Description	    : As the function states
	// Return type		: Reference to 'this' object
	// Argument         : BOOL bActivate
	// =============================================================================================
	virtual CLabel& FlashBackground(BOOL bActivate)
	{

		if (m_bTimer)
			KillTimer(1);

		if (bActivate)
		{
			m_bState = FALSE;

			m_bTimer = TRUE;
			SetTimer(1,500);

			m_Type = Background;
		}

		return *this;
	}

	// =============================================================================================
	// Function name	: SetLink
	// Description	    : Indicates the string is a link
	// Return type		: Reference to 'this' object
	// Argument         : BOOL bLink
	// Argument         : BOOL bNotifyParent
	// =============================================================================================
	virtual CLabel& SetLink(BOOL bLink, BOOL bNotifyParent)
	{

		m_bLink = bLink;
		m_bNotifyParent = bNotifyParent;

		if (bLink)
			ModifyStyle(0,SS_NOTIFY);
		else
			ModifyStyle(SS_NOTIFY,0);

		return *this;
	}

	// =============================================================================================
	// Function name	: SetLinkCursor
	// Description	    : Sets the internet browers link
	// Return type		: Reference to 'this' object
	// =============================================================================================
	virtual CLabel& SetLinkCursor()
	{
    // create or load a cursor
#if (WINVER >= 0x0500)
		m_hCursor = ::LoadCursor(NULL, IDC_HAND);
#else
		m_hCursor = ::CreateCursor(_Module.GetModuleInstance(), _AtlLabel_CursorData.xHotSpot, _AtlLabel_CursorData.yHotSpot, _AtlLabel_CursorData.cxWidth, _AtlLabel_CursorData.cyHeight, _AtlLabel_CursorData.arrANDPlane, _AtlLabel_CursorData.arrXORPlane);
#endif //!(WINVER >= 0x0500)
		ATLASSERT(m_hCursor != NULL);
		return *this;
	}

	// =============================================================================================
	// Function name	: SetFont3D
	// Description	    : Sets the 3D attribute of the font.
	// Return type		: Reference to 'this' object
	// Argument         : BOOL bSet
	// Argument         : Type3D type=Raised
	// =============================================================================================
	virtual CLabel& SetFont3D(BOOL bSet,Type3D type=Raised)
	{

		m_bFont3d = bSet;
		m_3dType = type;
		UpdateSurface();


		return *this;
	}

	// =============================================================================================
	// Function name	: SetRotationAngle
	// Description	    : Sets the rotation angle for the current font.
	// Return type		: Reference to 'this' object
	// Argument         : UINT nAngle
	// Argument         : BOOL bRotation
	// =============================================================================================
	virtual CLabel& SetRotationAngle(UINT nAngle,BOOL bRotation)
	{
		// Arrrrh...
		// Your looking in here why the font is rotating, aren't you?
		// Well try setting the font name to 'Arial' or 'Times New Roman'
		// Make the Angle 180 and set bRotation to true.
		//
		// Font rotation _ONLY_ works with TrueType fonts...
		//
		// 
		m_lf.lfEscapement = m_lf.lfOrientation = (nAngle * 10);
		m_bRotation = bRotation;
		
		ReconstructFont();		
		UpdateSurface();

		return *this;
	}

	// =============================================================================================
	// Function name	: SetText3DHiliteColor
	// Description	    : Sets the 3D font hilite color
	// Return type		: Reference to 'this' object
	// Argument         : COLORREF cr3DHiliteColor
	// =============================================================================================
	virtual CLabel& SetText3DHiliteColor(COLORREF cr3DHiliteColor)
	{
		m_cr3DHiliteColor = cr3DHiliteColor;
		UpdateSurface();


		return *this;
	}


	// =============================================================================================
	// Function name	: SubclassWindow
	// Description	    : 
	// Return type		: BOOL 
	// Argument         : HWND hWnd
	// =============================================================================================
	BOOL SubclassWindow(HWND hWnd)
	{
		BOOL bRet = CWindowImpl<CLabel, CStatic>::SubclassWindow(hWnd);
		if(bRet)
			Init();
		return bRet;
	}

protected:
	
	// =============================================================================================
	// Function name	: UpdateSurface
	// Description	    : 
	// Return type		: void 
	// =============================================================================================
	void UpdateSurface()
	{
		CRect (rc);
		GetWindowRect(rc);
		RedrawWindow();

		CWindow parent(GetParent());
		parent.ScreenToClient(rc);
		parent.InvalidateRect(rc,TRUE);
		parent.UpdateWindow();
	}

	// =============================================================================================
	// Function name	: ReconstructFont
	// Description	    : Helper function to build font after it was changed
	// Return type		: void 
	// =============================================================================================
	void ReconstructFont()
	{
		if(m_hFont != NULL)
		{
			::DeleteObject(m_hFont);
			m_hFont = NULL;
		}
		m_hFont = ::CreateFontIndirect(&m_lf);

		ATLASSERT(m_hFont);
	}

	// =============================================================================================
	// Function name	: DrawGradientFill
	// Description	    : Internal help function to gradient fill background
	// Return type		: void 
	// Argument         : CDC* pDC
	// Argument         : CRect* pRect
	// Argument         : COLORREF crStart
	// Argument         : COLORREF crEnd
	// Argument         : int nSegments
	// =============================================================================================
	void DrawGradientFill(CDC* pDC, CRect* pRect, COLORREF crStart, COLORREF crEnd, int nSegments)
	{
		// Get the starting RGB values and calculate the incremental
		// changes to be applied.

		COLORREF cr;
		int nR = GetRValue(crStart);
		int nG = GetGValue(crStart);
		int nB = GetBValue(crStart);

		int neB = GetBValue(crEnd);
		int neG = GetGValue(crEnd);
		int neR = GetRValue(crEnd);

		if(nSegments > pRect->Width())
			nSegments = pRect->Width();

		int nDiffR = (neR - nR);
		int nDiffG = (neG - nG);
		int nDiffB = (neB - nB);

		int ndR = 256 * (nDiffR) / (max(nSegments,1));
		int ndG = 256 * (nDiffG) / (max(nSegments,1));
		int ndB = 256 * (nDiffB) / (max(nSegments,1));

		nR *= 256;
		nG *= 256;
		nB *= 256;

		neR *= 256;
		neG *= 256;
		neB *= 256;

		int nCX = pRect->Width() / max(nSegments,1), nLeft = pRect->left, nRight;
		pDC->SelectStockPen(NULL_PEN);

		for (int i = 0; i < nSegments; i++, nR += ndR, nG += ndG, nB += ndB)
		{
			// Use special code for the last segment to avoid any problems
			// with integer division.

			if (i == (nSegments - 1))
				nRight = pRect->right;
			else
				nRight = nLeft + nCX;

			cr = RGB(nR / 256, nG / 256, nB / 256);
			
			{
				CBrush br;
				br.CreateSolidBrush(cr);
				HBRUSH pbrOld = pDC->SelectBrush(br);
				pDC->Rectangle(nLeft, pRect->top, nRight + 1, pRect->bottom);
				pDC->SelectBrush(pbrOld);
			}

			// Reset the left side of the drawing rectangle.

			nLeft = nRight;
		}
	}

	COLORREF		m_crText;
	COLORREF		m_cr3DHiliteColor;
	HBRUSH			m_hwndBrush;
	HBRUSH			m_hBackBrush;
	LOGFONT			m_lf;
	HFONT			m_hFont;
	ATL::CString			m_strText;
	BOOL			m_bState;
	BOOL			m_bTimer;
	BOOL			m_bLink;
	BOOL			m_bTransparent;
	BOOL			m_bFont3d;
	BOOL			m_bToolTips;
	BOOL			m_bNotifyParent;
	BOOL			m_bRotation;
	BOOL      m_bPaintLabel;
	FlashType		m_Type;
	HCURSOR			m_hCursor;
	Type3D			m_3dType;
	BackFillMode	m_fillmode;
	COLORREF		m_crHiColor;
	COLORREF		m_crLoColor;

protected:

	// =============================================================================================
	// Function name	: OnPaint
	// Description	    : Handles all the drawing code for the label
	//					: Called by Windows... not by USER
	// Return type		: LRESULT 
	// Argument         : UINT
	// Argument         : WPARAM wParam
	// Argument         : LPARAM
	// Argument         : BOOL& bHandled
	// =============================================================================================
	LRESULT OnPaint(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
	{
		if(!m_bPaintLabel)
		{
			bHandled = FALSE;
			return 1;
		}
		
		CPaintDC dc(m_hWnd); // device context for painting

		DWORD dwFlags = 0;

		CRect rc;
		GetClientRect(rc);
		TCHAR cValue[1000];
		GetWindowText(cValue, sizeof(cValue));
		ATL::CString strText(cValue);
		CBitmap bmp;

		// Set up for double buffering...	
		CDC* pDCMem;

		if (!m_bTransparent)
		{
			pDCMem = new CDC;
			pDCMem->CreateCompatibleDC(dc.m_hDC);
			bmp.CreateCompatibleBitmap(dc.m_hDC,rc.Width(),rc.Height());
			pDCMem->SelectBitmap(bmp);
		}
		else
		{
			pDCMem = &dc;
		}

		UINT nMode = pDCMem->SetBkMode(TRANSPARENT);

		COLORREF crText = pDCMem->SetTextColor(m_crText);
		HFONT OldTempFont = 0;
		if(m_hFont != NULL)
			OldTempFont = pDCMem->SelectFont(m_hFont);
		
		// Fill in backgound if not transparent
		if (!m_bTransparent)
		{
			if (m_fillmode == Normal)
			{
				CBrush br;
				
				if (m_hBackBrush != NULL)
					br.Attach(m_hBackBrush);
				else
					br.Attach(m_hwndBrush);
						
				pDCMem->FillRect(rc,br);

				br.Detach();
			}
			else // Gradient Fill
			{
				DrawGradientFill(pDCMem, &rc, m_crLoColor, m_crHiColor, 100);
			}

		}

		// If the text is flashing turn the text color on
		// then to the color of the window background.

		LOGBRUSH lb;
		ZeroMemory(&lb,sizeof(lb));

		// Stop Checking complaining
		if (m_hBackBrush)
			::GetObject(m_hBackBrush,sizeof(lb),&lb);


		// Something to do with flashing
		if (!m_bState && m_Type == Text)
			pDCMem->SetTextColor(lb.lbColor);

		DWORD style = GetStyle();
		
		switch (style & SS_TYPEMASK)
		{
			case SS_RIGHT: 
				dwFlags = DT_RIGHT | DT_WORDBREAK; 
				break; 
			
			case SS_CENTER: 
				dwFlags = SS_CENTER | DT_WORDBREAK;
				break;

			case SS_LEFTNOWORDWRAP: 
				dwFlags = DT_LEFT; 
				break;

			default: // treat other types as left
				case SS_LEFT: 
					dwFlags = DT_LEFT | DT_WORDBREAK; 
					break;
		}	

			
		// If the text centered make an assumtion that
		// the will want to center verticly as well
		if (style & SS_CENTERIMAGE)
		{
			dwFlags = DT_CENTER;

			// Apply 
			if (strText.Find(_T("\r\n")) == -1)
			{
				dwFlags |= DT_VCENTER;

				// And because DT_VCENTER only works with single lines
				dwFlags |= DT_SINGLELINE; 
			}
		}

		//
		// 3333   DDDDD
		//     3  D    D
		//   33   D     D    E F X 
		//     3  D    D
		// 3333   DDDDD
		//
		//
		if (m_bRotation)
		{
			int nAlign = pDCMem->SetTextAlign (TA_BASELINE);

			CPoint pt;
			GetViewportOrgEx (pDCMem->m_hDC,&pt) ;
			SetViewportOrgEx (pDCMem->m_hDC,rc.Width() / 2, rc.Height() / 2, NULL) ;
			pDCMem->TextOut (0, 0, strText) ;
			SetViewportOrgEx (pDCMem->m_hDC,pt.x / 2, pt.y / 2, NULL) ;
			pDCMem->SetTextAlign (nAlign);
		}
		else
		{
			pDCMem->DrawText(cValue, -1,rc,dwFlags);
			if (m_bFont3d)
			{
				pDCMem->SetTextColor(m_cr3DHiliteColor);

				if (m_3dType == Raised)
					rc.OffsetRect(-1,-1);
				else
					rc.OffsetRect(1,1);

				pDCMem->DrawText(cValue, -1, rc,dwFlags);
				m_3dType;
			}
		}

		// Restore DC's State
		pDCMem->SetBkMode(nMode);
        if (OldTempFont)
        {
            pDCMem->SelectFont(OldTempFont);
        }

		pDCMem->SetTextColor(crText);

		if (!m_bTransparent)
		{
			dc.BitBlt(0,0,rc.Width(),rc.Height(),pDCMem->m_hDC,0,0,SRCCOPY);
			delete pDCMem;
		}

		return 0;
	}

	// =============================================================================================
	// Function name	: OnTimer
	// Description	    : Used in conjunction with 'FLASH' functions
	// Return type		: LRESULT 
	// Argument         : UINT uMsg
	// Argument         : WPARAM wParam
	// Argument         : LPARAM lParam
	// Argument         : BOOL& bHandled
	// =============================================================================================
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{

		m_bState = !m_bState;

		UpdateSurface();
		
		return 0;
	}

	// =============================================================================================
	// Function name	: OnSysColorChange
	// Description	    : 
	// Return type		: LRESULT 
	// Argument         : UINT uMsg
	// Argument         : WPARAM wParam
	// Argument         : LPARAM lParam
	// Argument         : BOOL& bHandled
	// =============================================================================================
	LRESULT OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{

		if (m_hwndBrush)
			::DeleteObject(m_hwndBrush);

		m_hwndBrush = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));
		
		UpdateSurface();
		return 0;		
	}

	// =============================================================================================
	// Function name	: OnLButtonDown
	// Description	    : Called when a link is click on
	// Return type		: LRESULT 
	// Argument         : UINT uMsg
	// Argument         : WPARAM wParam
	// Argument         : LPARAM lParam
	// Argument         : BOOL& bHandled
	// =============================================================================================
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (!m_bNotifyParent) // Fix
		{
			TCHAR cValue[1000];
			GetWindowText(cValue, sizeof(cValue));
			ATL::CString strLink(cValue);
			ShellExecute(NULL,_T("open"),strLink, NULL, NULL, SW_SHOWNORMAL);
		}
		else
		{
			// To use notification in parent window
			// Respond to a OnNotify in parent and disassemble the message
			//
			NMHDR nm;

			nm.hwndFrom = m_hWnd;
			nm.idFrom  = GetDlgCtrlID();
			nm.code = NM_LINKCLICK;
			CWindow wnd(GetParent());
			wnd.SendMessage(WM_NOTIFY,nm.idFrom,(LPARAM) &nm);
		}
			
		return 0;
	}
	
	// =============================================================================================
	// Function name	: OnSetCursor
	// Description	    : Used in conjunction with 'LINK' function
	// Return type		: LRESULT 
	// Argument         : UINT uMsg
	// Argument         : WPARAM wParam
	// Argument         : LPARAM lParam
	// Argument         : BOOL& bHandled
	// =============================================================================================
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{

		if (m_hCursor)
		{
			::SetCursor(m_hCursor);
			return TRUE;
		}

		return 0;
	}


	// =============================================================================================
	// Function name	: Init
	// Description	    : 
	// Return type		: void 
	// =============================================================================================
	void Init()
	{
		ATLASSERT(::IsWindow(m_hWnd));

		// Check if we should paint a label
		TCHAR lpszBuffer[8];
		if(::GetClassName(m_hWnd, lpszBuffer, 8))
		{
			if(lstrcmpi(lpszBuffer, _T("static")) == 0)
			{
				ModifyStyle(0, SS_NOTIFY);	// we need this
				DWORD dwStyle = GetStyle() & 0x000000FF;
				if(dwStyle == SS_ICON || dwStyle == SS_BLACKRECT || dwStyle == SS_GRAYRECT || 
						dwStyle == SS_WHITERECT || dwStyle == SS_BLACKFRAME || dwStyle == SS_GRAYFRAME || 
						dwStyle == SS_WHITEFRAME || dwStyle == SS_OWNERDRAW || 
						dwStyle == SS_BITMAP || dwStyle == SS_ENHMETAFILE)
					m_bPaintLabel = FALSE;
			}
		}

		// set font
		if(m_bPaintLabel)
		{
			CWindow wnd = GetParent();
			CFontHandle font = wnd.GetFont();
			if(font.m_hFont != NULL)
			{
				font.GetLogFont(&m_lf);
				m_hFont = ::CreateFontIndirect(&m_lf);
			}
		}
	}
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(ATL_LABEL_H)
