//
//	Class:		CPPHtmlDrawer
//
//	Compiler:	Visual C++
//	Tested on:	Visual C++ 6.0
//				Visual C++ .NET 2003
//
//	Version:	See GetVersionC() or GetVersionI()
//
//	Created:	xx/xxxx/2004
//	Updated:	21/November/2004
//
//	Author:		Eugene Pustovoyt	pustovoyt@mail.ru
//
//	Disclaimer
//	----------
//	THIS SOFTWARE AND THE ACCOMPANYING FILES ARE DISTRIBUTED "AS IS" AND WITHOUT
//	ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED. NO REPONSIBILITIES FOR POSSIBLE
//	DAMAGES OR EVEN FUNCTIONALITY CAN BE TAKEN. THE USER MUST ASSUME THE ENTIRE
//	RISK OF USING THIS SOFTWARE.
//
//	Terms of use
//	------------
//	THIS SOFTWARE IS FREE FOR PERSONAL USE OR FREEWARE APPLICATIONS.
//	IF YOU WISH TO THANK MY WORK, YOU MAY DONATE ANY SUM OF MONEY TO ME 
//  FOR SUPPORT OF DEVELOPMENT OF THIS CLASS.
//	IF YOU USE THIS SOFTWARE IN COMMERCIAL OR SHAREWARE APPLICATIONS YOU
//	ARE GENTLY ASKED TO DONATE ANY SUM OF MONEY TO THE AUTHOR:
//
//--- History ------------------------------ 
// 2004/03/01  *** Releases version 1.0 ***
//------------------------------------------
//		2004/04/13	[ADD] Added a "speed" parameter to a <ilst> tag for animation
//		2004/04/20	[ADD] For non-MFC program added STL class CStdString
//		2004/05/05	[ADD] Added an EnableOutput method
//					[FIX] Fixed an error in SetImageList method (thanks to topus)
//------------------------------------------
// 2004/05/05  *** Releases version 1.1 ***
//------------------------------------------
//		2004/06/06	[FIX] Fixed an error on select a reference to the handle of the font
//							instead select a handle of the font
//		2004/06/24	[ADD] Added SetMaxWidth and GetMaxWidth methods for supporting a text wrapping
//					[ADD] Added EnableTextWrap and IsTextWrapEnabled methods to sets a text wrap
//							mode or to retrieves a mode status.
//					[ADD] Added common character entities.
//		2004/06/25	[ADD] Added SetTabSize method.
//					[ADD] Implemented a support to output a justified text.
//					[ADD] Added new <justify> tag to output a justified text.
//		2004/07/18	[ADD] Support a disabled draw in the tables
//		2004/09/07	[FIX] Fixed minor errors of drawing table's cells 
//		2004/10/13	[FIX] The last line of the paragraph alignment on fields now is not applied
//		2004/10/20	[FIX] Fixed error in GetLengthUnit method. Thanks to Reinhard Steiner
//		2004/10/26	[FIX] Corrected work of the justify for the multifont text in one line
//		 			[ADD] Support a word wrapping for the text with the images
//		2004/10/28	[ADD] Now cx and cy parameters of the <ilst> tag is an optional information
//------------------------------------------
// 2004/10/30  *** Releases version 1.2 ***
//------------------------------------------
//		2004/11/30	[FIX] Fixed an error in the drawing nested tables
//		2004/12/05	[CHN] Replaces CImageList to HIMAGELIST
//					[CHN] Replaces MFC classes (CSize, CPoint, CRect) to API structures
//		2004/12/14	[FIX] Fixed an error in determinates of the hyperlink area
//					[FIX] Fixed an error in drawing of the horizontal line (<hr> tag)
//		2005/01/15	[ADD] Not fixed columns will resize to the width of the client area
//		2005/03/01	[FIX] Fixed an error of the imagelist drawing
/////////////////////////////////////////////////////////////////////
//
// "GotoURL" function by Stuart Patterson
// As seen in the August, 1997 Windows Developer's Journal.
// Copyright 1997 by Miller Freeman, Inc. All rights reserved.
// Modified by Chris Maunder to use TCHARs instead of chars.
//
// "Default hand cursor" from Paul DiLascia's Jan 1998 MSJ article.
// Modified by Zorglab to use standard hand cursor on WinMe,2k,XP
//
/////////////////////////////////////////////////////////////////////

#ifndef _PPHTMLDRAWER_H_
#define _PPHTMLDRAWER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4786)

#include "ppdrawmanager.h"
#include "atlconv.h"
#include <shellapi.h>
#pragma comment(lib, "comctl32.lib")

#include <vector>
#include <map>
//#include "skinapp.h"					//统一资源Load

#define CPPString	CString
#define PPHTMLDRAWER_NO_HOVERLINK	-2	//A hot area is not exist under the cursor
#define PPHTMLDRAWER_BREAK_CHARS	_T(" -.,!:;)}]?") //A set of the chars to break line in the text wrap mode

enum 
{
	MODE_DRAW			= 0,
	MODE_FIRSTPASS,
	MODE_SECONDPASS
};

/////////////////////////////////////////////////////////////////////////////
// CPPHtmlDrawer
class CPPHtmlDrawer
{
public:
	CPPHtmlDrawer()
	{
		m_nNumPass = MODE_FIRSTPASS;

		m_hInstDll = NULL;
		m_bFreeInstDll = FALSE;
		m_hDC = NULL;
		m_hImageList = NULL;

		m_csCallbackRepaint.hWnd = NULL;
		m_csCallbackRepaint.nMessage = 0;
		m_csCallbackRepaint.lParam = 0;
		m_csCallbackRepaint.wParam = 0;

		m_csCallbackLink.hWnd = NULL;
		m_csCallbackLink.nMessage = 0;
		m_csCallbackLink.lParam = 0;
		m_csCallbackLink.wParam = 0;

		//	m_clrShadow = PPHTMLDRAWER_SHADOW_COLOR;

		m_hLinkCursor = NULL; // No cursor as yet
		m_nHoverIndexLink = PPHTMLDRAWER_NO_HOVERLINK;

		_SetListOfTags();
		_SetListSpecChars();
		_SetTableOfColors();
		_SetDefaultCursor();
		EnableEscapeSequences();
		SetMaxWidth(0);
		//	EnableTextWrap(FALSE); //A text warpping was disabled by default
		//	EnableTextWrap(TRUE); //A text warpping was disabled by default
		SetImageShadow(4, 4);
		SetTabSize(32);
		_SetDefaultCssStyles();
		EnableOutput();
		SetDisabledColor(::GetSysColor(COLOR_BTNSHADOW));
	}

	virtual ~CPPHtmlDrawer()
	{
		SetResourceDll(NULL);

		if (NULL != m_hLinkCursor)
		{
			::DestroyCursor(m_hLinkCursor);
			m_hLinkCursor = NULL;
		}

		if (NULL != m_hImageList)
			::DeleteObject(m_hImageList);
	}


public:
	//////////////////////////////////////////////////////////////////////////
	// 需要处理的消息
	void OnLButtonDown(LPPOINT lpClient)
	{
		//	TRACE (_T("OnLButtonDown()\n"));

		int nLink = _PtInHyperlink(lpClient);
		if (nLink >= 0)
		{
			//Hyperlink under the mouse pointer
			_JumpToHyperlink(nLink);
		} //if
	} //End OnLButtonDown

	BOOL OnSetCursor(LPPOINT lpClient)
	{
		int nLink = _PtInHyperlink(lpClient);
		if (nLink >= 0)
		{
			STRUCT_HYPERLINK link = m_arrLinks [nLink];
			if (m_nHoverIndexLink != link.nIndexLink)
			{
				m_nHoverIndexLink = link.nIndexLink;
				_CallbackOnRepaint(m_nHoverIndexLink);
				//Redraw Window
			} //if

			if (!link.sHyperlink.IsEmpty() && (NULL != m_hLinkCursor))
			{
				::SetCursor(m_hLinkCursor);
				return TRUE;
			} //if
		}
		else if (m_nHoverIndexLink != PPHTMLDRAWER_NO_HOVERLINK)
		{
			m_nHoverIndexLink = PPHTMLDRAWER_NO_HOVERLINK;
			_CallbackOnRepaint(m_nHoverIndexLink);
			//Redraw Window
		} //if

		return FALSE;
	} //End OnSetCursor

	BOOL OnTimer()
	{
		BOOL bRedraw = FALSE;
		if (m_arrAni.size() > 0)
		{
			for (UINT i = 0; i < m_arrAni.size(); ++i)
			{
				STRUCT_ANIMATION & sa = m_arrAni [i];
				if (sa.nMaxImages > 0)
				{
					sa.nTimerCount ++;
					if (sa.nTimerCount >= sa.nSpeed)
					{
						sa.nTimerCount = 0;
						sa.nIndex ++;
						if (sa.nIndex >= sa.nMaxImages)
							sa.nIndex = 0;
						bRedraw = TRUE;
					} //if
					m_arrAni [i] = sa;
				} //if
			} //for
		} //if

		return bRedraw;
	} //End of OnTimer


	/////////////////////////////////////////////////////////////////////////////////////////
	// 
	void Draw(HDC hDC, LPCTSTR lpszHtml, LPPOINT lpPoint)
	{
		//ENG: Preparing an output text
		SIZE size;
		PrepareOutput(hDC, lpszHtml, &size);

		//ENG: If output was disabled
		if (!size.cx || !size.cy)
			return;

		//ENG: Calculates an output area
		RECT rect;
		rect.left = lpPoint->x;
		rect.top = lpPoint->y;
		rect.right = rect.left + size.cx;
		rect.bottom = rect.top + size.cy;

		//ENG: Output a prepared text
		DrawPreparedOutput(hDC, &rect);
	} //End Draw

	void PrepareOutput(HDC hDC, LPCTSTR lpszHtml, LPSIZE lpSize)
	{
		//ENG: Copy initial parameters
		m_hDC = hDC;

		//ENG: Reset text justification
		::SetTextJustification(m_hDC, 0, 0);

		RECT rect;
		rect.left = rect.right = rect.top = rect.bottom = 0;
		//	if (m_bIsTextWrapEnabled)
		rect.right = m_nMaxWidth;
		m_csHtmlText = lpszHtml;
		_ReplaceSpecChars();
		lpSize->cx = lpSize->cy = 0;

		//ENG: If prepared text wasn't empty then return
		if (!m_csHtmlText.IsEmpty())
		{
			//ENG: Sets a prepare mode
			m_nNumPass = MODE_FIRSTPASS;

			m_arrTables.clear();

			//ENG: Prepares to real output
			_DrawHtml(lpSize, &rect);

			if (!lpSize->cx && !lpSize->cy)
				m_csHtmlText.Empty();

			//Cuts a tooltip if his real width more than m_nMaxWidth
			if (m_nMaxWidth/*m_bIsTextWrapEnabled*/ && (lpSize->cx > m_nMaxWidth))
				lpSize->cx = m_nMaxWidth;

			lpSize->cx ++;
			lpSize->cy ++;
		} //if
	} //End PrepareOutput

	////////////////////////////////////////////////////////////////////
	// DrawPreparedOutput()
	//		Draw a string prepared by PrepareOutput method.
	//------------------------------------------------------------------
	// Parameters:
	//		hDC				- Device Context to drawing 
	//		lpRect			- Pointer to RECT structure contains a bounding rectangle of
	//						  drawing area.
	////////////////////////////////////////////////////////////////////
	void DrawPreparedOutput(HDC hDC, LPCRECT lpRect)
	{
		//ENG: If prepared text was empty then return
		if (m_csHtmlText.IsEmpty())
			return;

		//ENG: Copy initial parameters
		m_hDC = hDC;
		SIZE size = {0, 0};

		//ENG: Sets a output mode
		m_nNumPass = MODE_DRAW;

		RECT rect = *lpRect;
		//	if (((rect.right - rect.left) > m_nMaxWidth) && m_bIsTextWrapEnabled)
		//		rect.right = rect.left + m_nMaxWidth;

		//ENG: Real output the prepared string
		_DrawHtml(&size, &rect);
	} //End of DrawPreparedOutput

	void SetHyperlinkCursor(HCURSOR hCursor = NULL)
	{
		if ((m_hLinkCursor == hCursor) && (NULL != m_hLinkCursor))
			return;

		if (NULL != m_hLinkCursor)
		{
			::DestroyCursor(m_hLinkCursor);
			m_hLinkCursor = NULL;
		} //if


		if (NULL == hCursor)
			_SetDefaultCursor();
		else
			m_hLinkCursor = hCursor;
	} //End SetHyperlinkCursor

	HCURSOR GetHyperlinkCursor() const
	{
		return m_hLinkCursor;
	} //End GetHyperlinkCursor

	/////////////////////////////////////////////////////////////////////
	// SetCallbackHyperlink
	// This function sets or removes the notification messages from the control before display.
	//
	// Parameters:
	//	hWnd [in] -    If non-NULL the control will be send the notification 
	//				   to specified window
	//				   Else the notification will not send
	///////////////////////////////////////////////////////////////////////
	void SetCallbackHyperlink(HWND hWnd, UINT nMessage, LPARAM lParam = 0)
	{
		//	TRACE(_T("SetCallbackHyperlink()\n"));

		m_csCallbackLink.hWnd = hWnd;
		if (NULL == hWnd)
		{
			m_csCallbackLink.nMessage = 0;
			m_csCallbackLink.lParam = 0;
		}
		else
		{
			m_csCallbackLink.nMessage = nMessage;
			m_csCallbackLink.lParam = lParam;
		} //if
	} //End SetCallbackHyperlink

	void SetCallbackRepaint(HWND hWnd, UINT nMessage, LPARAM lParam = 0)
	{
		//	TRACE(_T("_SetCallbackRepaint()\n"));

		m_csCallbackRepaint.hWnd = hWnd;
		if (NULL == hWnd)
		{
			m_csCallbackRepaint.nMessage = 0;
			m_csCallbackRepaint.lParam = 0;
		}
		else
		{
			m_csCallbackRepaint.nMessage = nMessage;
			m_csCallbackRepaint.lParam = lParam;
		} //if
	} //End SetCallbackRepaint

	/////////////////////////////////////////////////////////////////////////////
	//  CPPToolTip::SetImageList (public member function)
	//    sets the image list to tooltip
	//
	//  Parameters :
	//		nIdBitmap	[in] - Resource IDs of the bitmap to be associated with the image list
	//		cx			[in] - Dimensions of each image, in pixels.
	//		cy			[in] - Dimensions of each image, in pixels.
	//		nCount		[in] - Number of images that the image list initially contains.
	//		crMask		[in] - Color used to generate a mask. Each pixel of this color in the 
	//						   specified bitmap is changed to black, and the corresponding 
	//						   bit in the mask is set to one.
	//  Returns :
	//		None
	//
	/////////////////////////////////////////////////////////////////////////////
	void SetImageList(UINT nIdBitmap, int cx, int cy, int nCount, COLORREF crMask = RGB(255, 0, 255))
	{
		// Load bitmap
		HBITMAP hBitmap = _GetBitmapFromResources(nIdBitmap);
		SetImageList(hBitmap, cx, cy, nCount, crMask);
	} //End SetImageList

	/////////////////////////////////////////////////////////////////////////////
	//  CPPToolTip::SetImageList (public member function)
	//    sets the image list to tooltip
	//
	//  Parameters :
	//		hBitmap		[in] - Handle of the bitmap to be associated with the image list
	//		cx			[in] - Dimensions of each image, in pixels.
	//		cy			[in] - Dimensions of each image, in pixels.
	//		nCount		[in] - Number of images that the image list initially contains.
	//		crMask		[in] - Color used to generate a mask. Each pixel of this color in the 
	//						   specified bitmap is changed to black, and the corresponding 
	//						   bit in the mask is set to one.
	//  Returns :
	//		None
	//
	/////////////////////////////////////////////////////////////////////////////
	void SetImageList(HBITMAP hBitmap, int cx, int cy, int nCount, COLORREF crMask = RGB(255, 0, 255))
	{
		//ENG: Removes previously image list
		if (NULL != m_hImageList)
			::DeleteObject(m_hImageList);

		//ENG: If don't need to create a new image list
		if (NULL == hBitmap)
			return;

		// Ensure that the common control DLL is loaded. 
		InitCommonControls(); 

		m_hImageList = ImageList_Create(cx, cy, ILC_COLOR32 | ILC_MASK, nCount, 1);
		ImageList_AddMasked(m_hImageList, hBitmap, crMask);
		m_szImageList.cx = cx;
		m_szImageList.cy = cy;
	} //End SetImageList

	/////////////////////////////////////////////////////////////////////////////
	//  CPPToolTip::GetImageList (public member function)
	//    gets the image list from tooltip
	//
	//  Parameters :
	//		sz		   [out] - Dimensions of each image, in pixels.
	//  Returns :
	//		A pointer to a CImageList object
	//
	/////////////////////////////////////////////////////////////////////////////
	//CImageList * _GetImageList(CSize & sz)
	//{
	//	sz = m_szImageList;
	//	return &m_ImageList;
	//} //End GetImageList

	void EnableEscapeSequences(BOOL bEnable = TRUE)
	{
		m_bEnableEscapeSequences = bEnable;
	}

	void LoadResourceDll(LPCTSTR lpszPathDll, DWORD dwFlags = 0)
	{
		HINSTANCE hInst = NULL;
		if (NULL != lpszPathDll)
			hInst = ::LoadLibraryEx(lpszPathDll, NULL, dwFlags);

		SetResourceDll(hInst);

		if (NULL != hInst)
			m_bFreeInstDll = TRUE;
	} //End LoadResourceDll

	void SetResourceDll(HINSTANCE hInstDll = NULL)
	{
		if (NULL != m_hInstDll)
		{
			if (!m_bFreeInstDll)
				return;
			::FreeLibrary(m_hInstDll);
			m_hInstDll = NULL;
		} //if

		m_bFreeInstDll = FALSE;

		if (NULL != hInstDll)
			m_hInstDll = hInstDll;
	} //End SetResourceDll


	CPPDrawManager * GetDrawManager()
	{
		return &m_drawmanager;
	} //End GetDrawManager

	void SetCssStyles(LPCTSTR lpszCssString = NULL)
	{
		m_mapStyles.clear(); //removes previously styles

		if (NULL == lpszCssString)
		{
			_SetDefaultCssStyles();
		}
		else
		{
			CPPString str = (CPPString)lpszCssString;
			m_strCssStyles = str;

			CPPString strName;
			CPPString strProperty;

			int nBegin;
			TCHAR chSymbol;
			int nIndex = 0;

			while (nIndex < str.GetLength())
			{
				//Passes a space in begin string
				if (_GetIndexNextAlphaNum(str, nIndex))
				{
					nBegin = nIndex;
					//Searching end of the style name
					chSymbol = _GetIndexNextChars(str, nIndex, _T(" {"));
					if ((nIndex > nBegin) && (0 != chSymbol))
					{
						strName = str.Mid(nBegin, nIndex - nBegin);
						if (!strName.IsEmpty())
						{
							if (chSymbol != _T(' '))
								nIndex --;
							chSymbol = _GetIndexNextChars(str, nIndex, _T("{"));
							if (0 != chSymbol)
							{
								nBegin = nIndex + 1;
								chSymbol = _GetIndexNextChars(str, nIndex, _T("}"));
								if ((nIndex > nBegin) && (0 != chSymbol))
								{
									strProperty = str.Mid(nBegin, nIndex - nBegin);
									SetTextStyle(strName, strProperty);
								} //if
							} //if
						} //if
					} //if
				} //if
			} //while
		} //if
	}

	void SetCssStyles(DWORD dwIdCssString, LPCTSTR lpszPathDll = NULL)
	{
		CPPString str;
		if (NULL == lpszPathDll)
			str = _GetStringFromResource(dwIdCssString);
		else
			str = _GetStringFromDll(dwIdCssString, lpszPathDll);
		SetCssStyles(str);
	}


	LPCTSTR GetCssStyles()
	{
		return (LPCTSTR)m_strCssStyles;
	} //End GetCssStyles

	void SetTextStyle(LPCTSTR lpszStyleName, LPCTSTR lpszStyleValue)
	{
		CPPString name = (CPPString)lpszStyleName;
		name.MakeLower();
		iter_mapStyles iterMap = m_mapStyles.find(name);

		if (iterMap != m_mapStyles.end())
		{
			//Modifies 
			iterMap->second = (CPPString)lpszStyleValue;
		}
		else
		{
			//Add new
			m_mapStyles.insert(std::make_pair(name, (CPPString)lpszStyleValue));
		} //if
	} //End SetTextStyle

	////////////////////////////////////////////////////////////////////
	// _SetTooltipShadow()
	//		Sets a image's shadow.
	//------------------------------------------------------------------
	// Parameters:
	//		nOffsetX, 
	//		nOffsetY		- The offsets of the tooltip's shadow from the tooltip's window.
	//		nDarkenPercent	- So far as colors under the shadow will be darken (0 - 100)
	//      bGradient		- TRUE to use a gradient shadow.
	//		nDepthX,
	//		nDepthY			- The gradient depths of the tooltip's shadow.
	////////////////////////////////////////////////////////////////////
	void SetImageShadow(int nOffsetX, int nOffsetY, BYTE nDarkenPercent = 50, 
		BOOL bGradient = TRUE, int nDepthX = 7, int nDepthY = 7)
	{
		m_szOffsetShadow.cx = nOffsetX;
		m_szOffsetShadow.cy = nOffsetY;
		m_szDepthShadow.cx = nDepthX;
		m_szDepthShadow.cy = nDepthY;
		m_nDarkenShadow = min(100, nDarkenPercent);
		m_bGradientShadow = bGradient;
		BYTE nColor = ::MulDiv(255, 100 - m_nDarkenShadow, 100);
		m_crShadow = RGB(nColor, nColor, nColor);
	} //End of SetTooltipShadow


	void EnableOutput(BOOL bEnable = TRUE)
	{
		m_bIsEnable = bEnable;
	} //End of EnableOutput

	void SetDisabledColor(COLORREF color)
	{
		m_crDisabled = color;
	}
	CPPString GetResCommandPrompt(UINT nID, UINT nNumParam = 0)
	{
		CPPString str = _GetStringFromResource(nID);
		if (!str.IsEmpty())
		{
			int nFirst = 0;
			int nLast = nFirst;
			UINT nCount = 0;
			while ((nCount <= nNumParam) && (nFirst < str.GetLength()))
			{
				nLast = str.Find(_T('\n'), nFirst);
				if (nLast < 0)
				{
					//Char wasn't found
					if (nCount == nNumParam)
						str = str.Mid(nFirst, str.GetLength() - nFirst);
					else
						str.Empty();

					return str;
				}
				else
				{
					//Char was found
					if (nCount == nNumParam)
					{
						str = str.Mid(nFirst, nLast - nFirst);
						return str;
					}
					else
					{
						nFirst = nLast + 1;
					} //if
				} //if
				nCount ++;
			} //while
		} //if

		return _T("");
	} //End of GetResCommandPrompt

	void SetMaxWidth(int nWidth = 0) {m_nMaxWidth = nWidth;}; //Sets the maximum width of the output window.
	int  GetMaxWidth() {return m_nMaxWidth;}; //Gets the maximum width of the output window.
//	void EnableTextWrap(BOOL bEnable = TRUE){
//		m_bIsTextWrapEnabled = bEnable;};
//	BOOL IsTextWrapEnabled() {return m_bIsTextWrapEnabled;};

	void SetTabSize(int nSize) {m_nTabSize = nSize;};


protected:
	enum{	ALIGN_LEFT = 0,
			ALIGN_CENTER,
			ALIGN_RIGHT,
			ALIGN_JUSTIFY
		};
	
	enum{	ALIGN_TOP = 0,
			ALIGN_VCENTER,
			ALIGN_BOTTOM,
			ALIGN_BASELINE
		};

	enum{	LINK_NONE = 0,
			LINK_HREF,
			LINK_MESSAGE
		};

	enum{	TEXT_TRANSFORM_NONE = 0,
			TEXT_TRANSFORM_UPPERCASE,
			TEXT_TRANSFORM_LOWERCASE,
			TEXT_TRANSFORM_CAPITALIZE
		};

	enum{	BORDER_STYLE_NONE = 0,
			BORDER_STYLE_SOLID,
			BORDER_STYLE_DOTTED,
			BORDER_STYLE_DASHED,
			BORDER_STYLE_DOUBLE,
		};

	enum {	TAG_NONE = 0,
			TAG_BOLD,
			TAG_ITALIC,
			TAG_UNDERLINE,
			TAG_STRIKEOUT,
			TAG_FONT,
			TAG_HLINE,
			TAG_NEWLINE,
			TAG_TABULATION,
			TAG_LEFT,
			TAG_CENTER,
			TAG_RIGHT,
			TAG_JUSTIFY,
			TAG_BASELINE,
			TAG_TOP,
			TAG_VCENTER,
			TAG_BOTTOM,
			TAG_BITMAP,
			TAG_ICON,
			TAG_IMAGELIST,
			TAG_STRING,
			TAG_NEWSTYLE,
			TAG_SPAN,
			TAG_HYPERLINK
		};

#pragma pack(1)
	typedef struct _STRUCT_TAGPROP
	{
		DWORD dwTagIndex;	// The hot rect of the hyperlink
		CPPString strTagName;	// The type of the hyperlink
	} STRUCT_TAGPROP;
#pragma pack()

#pragma pack(1)
	typedef struct _STRUCT_ANIMATION
	{
		int nIndex;		//The current index of the image
		int nMaxImages; //The max images in the bitmap
		int nTimerCount;//The current time position
		int nSpeed;		//The speed of animation
	} STRUCT_ANIMATION;
#pragma pack()

#pragma pack(1)
	typedef struct _STRUCT_HYPERLINK
	{
		RECT rcArea;		// The hot rect of the hyperlink
		int nTypeLink;		// The type of the hyperlink
		int nIndexLink;		// The index of the hyperlink
		CPPString sHyperlink; // The hyperlink
	} STRUCT_HYPERLINK;
#pragma pack()
	
#pragma pack(1)
	typedef struct _STRUCT_CHANGESTYLE 
	{
		CPPString strTag;		//The name of the last opened tag
		
		//Font
		int  nSizeFont;		//The height of the logic font
		int	 nWeightFont;	//The weight of the logic font
		BOOL bItalicFont;	//Is italic logic font?
		BOOL bUnderlineFont;//Is underline logic font?
		BOOL bStrikeOutFont;//Is strikeout logic font?
		BOOL bOverlineFont; //Is overline logic font?
		CPPString sFaceFont;  //The face name of the logic font
		
		//Color		
		COLORREF crText;	//The foreground color 
		COLORREF crBkgnd;	//The background color (also begin for the gradient)
		COLORREF crBorderLight;	//The border color
		COLORREF crBorderDark;	//The border color
		COLORREF crMidBkgnd;//The middle background color
		COLORREF crEndBkgnd;//The end background color

		//Fill
		int  nBkMode;		//The background mode for the text (TRANSPARENT, OPAQUE)
		int  nFillBkgnd;	//The fill effect of the background
		CPPString strNameResBk;

		//Align
		int  nHorzAlign;	//The horizontal align
		int  nVertAlign;	//The vertical align
		
		//Border
		int  nBorderStyle;	//The border style
		int  nBorderWidth;	//The width of the border

		//Cell Sizes
		int nCellWidth;		//The width of the cell
		int nCellHeight;	//The height of the cell
		BOOL bCellWidthPercent; //The width value in the percent
		BOOL bCellHeightPercent; //The height value in the percent

		//Text
		int  nTextTransform;//Transformation of the text (NONE, UPPERCASE, LOWERCASE, CAPITALIZE)

		int nMargin;		//Margins
		
		int nPadding;		//Padding
		
		//Hyperlink
		int  nTypeLink;		//The type of the link (NONE, HREF, MESSAGE)
		CPPString sHyperlink; //The additional parameter for the link
	} STRUCT_CHANGESTYLE; 
#pragma pack()
	
#pragma pack(1)
	typedef struct _STRUCT_IMAGE
	{
		int			nIndexImageList;//image's index of the image list
		int			nIdRes;			//ID resource from app
		int			nIdDll;			//ID resource from dll
		int			nHandle;		//handle of the resource
		int			cx;				//horizontal size of image
		int			cy;				//vertical size of image
		int			nWidth;			//width of image
		int			nHeight;		//height of image
		int         nSpeed;			//speed for animation
		UINT		nStyles;		//styles of image
		UINT		nHotStyles;		//hot styles of image
		BOOL        bUseMask;		//
		BOOL		bPercentWidth;
		BOOL		bPercentHeight;
		COLORREF	crMask;			//color of mask
		CPPString	strSkinRes;		//ksgui中的图象名称				//冰峰
		CPPString	strSrcFile;		//path on the source file
		CPPString   strPathDll;		//path on the resource dll
	} STRUCT_IMAGE;
#pragma pack()
	
#pragma pack(1)
	typedef struct _STRUCT_CALLBACK
	{
		HWND		hWnd;
		UINT		nMessage;		// Message identifier
		WPARAM		wParam;
		LPARAM		lParam;
	} STRUCT_CALLBACK;
#pragma pack()

#pragma pack(1)
	typedef struct _STRUCT_HTMLLINE
	{
		int  nWidthLine;
		int  nHeightLine;
		int  nDescentLine;
		int  nAddPercentWidth;
		int  nHorzAlign;
		int  nSpaceChars;	//a count of space chars in the line
		BOOL bWrappedLine;	//TRUE if text was wrapped in the current line
	} STRUCT_HTMLLINE;
#pragma pack()

#pragma pack(1)
	typedef struct _STRUCT_CELL
	{
		int   nColSpan;			//-1 = Cell isn't used, >0 - How much columns was spaned
		int   nRowSpan;			//-1 = Cell isn't used, >0 - How much rows was spaned
		SIZE  szText;			//Real size of the text's area
		SIZE  szCell;			//Real size of the cell
		BOOL  bFixedWidth;		//TRUE if width of this cell was fixed
//		vecHtmlLine vecLines;
//		int   nWidth;			//Width of the cell
//		int   nHeight;			//Height of the cell
//		BOOL  bWidthPercent;	//if TRUE nWidth member in a percents
//		BOOL  bHeightPercent;	//if TRUE nHeight member in a percents
	} STRUCT_CELL;
#pragma pack()

	//Cells of Table
	typedef std::vector<STRUCT_CELL> vecRow;	//Alone row
	typedef std::vector<vecRow> vecTable;		//Vector of the rows is a table
	typedef std::vector<int> vecSize;			//Width of the columns or height of the rows
	typedef std::vector<BOOL> vecFlag;			//Flags for fixed widthes of the columns

#pragma pack(1)
	typedef struct _STRUCT_TABLE
	{
		vecTable  cells;	//Info about each cell of the table
		vecSize   width;	//Dimensions of the width of the columns
		vecSize	  height;	//Dimensions of the height of the rows
		vecFlag   fixed_width; //
	} STRUCT_TABLE;
#pragma pack()

	typedef std::vector<STRUCT_TABLE> vecTables;
	vecTables m_arrTables;	//All tables
	int   m_nCurTable;	  //The current index of the table
	int   m_nCurTableRow; //The current row of the table

	STRUCT_CALLBACK	m_csCallbackRepaint; //Callback for repaint HTML drawer
	STRUCT_CALLBACK	m_csCallbackLink; //Callback for hyperlink message
	STRUCT_CHANGESTYLE m_defStyle;
	STRUCT_HTMLLINE m_hline;

	CPPDrawManager m_drawmanager;
	
	//Values of the system context
	HIMAGELIST m_hImageList;
	SIZE m_szImageList;

	HINSTANCE m_hInstDll;
	BOOL m_bFreeInstDll;

	HCURSOR m_hLinkCursor;
	HFONT m_hOldFont;
	int m_nOldBkMode;
	COLORREF m_crOldText;
	COLORREF m_crOldBk;

//	BOOL m_bIsTextWrapEnabled;			//Is text wrap enabled
	BOOL		m_bIsEnable;			//TRUE for fullcolor output, FALSE for disabled output
	COLORREF	m_crDisabled;
	RECT		m_rcOutput;				//Output rectangle
	HDC			m_hDC;					//Device context to output or to prepare
	CPPString	m_csHtmlText;			//String to output

//	COLORREF m_clrShadow;

	int	  m_nNumPass;	//The number or type of the pass

	int   m_nTabSize;	// The max size for the each tabulation
	int   m_nMaxWidth;	// The max width for wrapping output
	int   m_nCurLine;   // The current drawing line
	int   m_nNumCurTable; //The number of the current table

	int m_nHoverIndexLink;	//The index of the link under the mouse
	int m_nCurIndexLink;
	int m_nCurIndexAni;		//The index of the animation
	BOOL m_bLastValueIsPercent;
	BOOL m_bEnableEscapeSequences; // 

	//Shadow of the image
	BOOL m_bGradientShadow;
	SIZE m_szOffsetShadow;
	SIZE m_szDepthShadow;
	BYTE m_nDarkenShadow;
	COLORREF m_crShadow;

	TEXTMETRIC m_tm;

	LOGFONT m_lfDefault;		//Default font
	HFONT m_hFont;

	//Wrapper string
	CPPString m_strPrefix;		//Prefix string 
	CPPString m_strPostfix;		//Postfix string
	CPPString m_strCssStyles;

	//Vectors
	typedef std::vector<STRUCT_HTMLLINE> vecHtmlLine;
	vecHtmlLine m_arrHtmlLine;

	//Vector of the stack
	typedef std::vector<STRUCT_CHANGESTYLE> arrStack;
	arrStack m_arrStack;

	//Vector of the hyperlinks
	typedef std::vector<STRUCT_HYPERLINK> arrLink;
	arrLink m_arrLinks;

	typedef std::vector<STRUCT_ANIMATION> arrAni;
	arrAni m_arrAni;

	//Map of the colors by name
	typedef std::map<CPPString, COLORREF> mapColors;
	typedef std::map<CPPString, COLORREF>::iterator iterMapColors;
	mapColors m_mapColors;

	//Map of the styles
	typedef std::map<CPPString, CPPString> mapStyles;
	typedef std::map<CPPString, CPPString>::iterator iter_mapStyles;
	mapStyles m_mapStyles;
	mapStyles m_mapSpecChars;

	//Map of the colors by name
	typedef std::map<CPPString, STRUCT_TAGPROP> mapTags;
	typedef mapTags::iterator iterMapTags;
	mapTags m_mapTags;

private:

	HICON _GetIconFromResources(DWORD dwID, int nWidth = 0, int nHeight = 0) const
	{
		if (0 == dwID) return NULL;

		// Find correct resource handle
#ifdef _MFC_VER
		HINSTANCE hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(dwID), RT_GROUP_ICON);
#else
		HINSTANCE hInstResource = ::GetModuleHandle(NULL);
#endif
		// Set icon when the mouse is IN the button
		HICON hIcon = (HICON)::LoadImage(hInstResource, MAKEINTRESOURCE(dwID), IMAGE_ICON, nWidth, nHeight, LR_DEFAULTCOLOR);

		return hIcon;
	}

	HICON _GetIconFromFile(LPCTSTR lpszPath, int nWidth = 0, int nHeight = 0) const
	{
		HICON hIcon = (HICON)::LoadImage(NULL, lpszPath, IMAGE_ICON, nWidth, nHeight, LR_LOADFROMFILE | LR_DEFAULTCOLOR);

		return hIcon;
	}

	HICON _GetIconFromDll(DWORD dwID, int nWidth = 0, int nHeight = 0, LPCTSTR lpszPathDll = NULL) const
	{
		if (0 == dwID) return NULL;

		HICON hIcon = NULL;

		HINSTANCE hInstDll = NULL;
		BOOL bNewDll = FALSE;

		if (NULL == lpszPathDll)
		{
			if (NULL != m_hInstDll)
				hInstDll = m_hInstDll;
		}
		else
		{
			//Load New Library
			hInstDll = ::LoadLibraryEx(lpszPathDll, NULL, 0);
			if (NULL != hInstDll)
				bNewDll = TRUE;	
		}

		if (NULL != hInstDll)
		{
			hIcon = (HICON)::LoadImage(hInstDll, MAKEINTRESOURCE(dwID), IMAGE_ICON, nWidth, nHeight, LR_DEFAULTCOLOR);

			if (bNewDll)
				::FreeLibrary(hInstDll);
		}

		return hIcon;
	}

	HBITMAP _GetBitmapFromResources(DWORD dwID) const
	{
		if (0 == dwID) return NULL;

		// Load bitmap
		CBitmap bitmap;					//冰峰
		bitmap.LoadBitmap(dwID);
		return bitmap.Detach();	
	}

	HBITMAP _GetBitmapFromSkinResources(LPCTSTR szSkinRes) const
	{
		//return SkinLoadBitmap(szSkinRes);
		return NULL;
	}
	HBITMAP _GetBitmapFromFile(LPCTSTR lpszPath) const
	{
		HBITMAP hBitmap = (HBITMAP)::LoadImage(NULL, lpszPath, IMAGE_BITMAP,
			0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

		return hBitmap;
	}

	HBITMAP _GetBitmapFromDll(DWORD dwID, LPCTSTR lpszPathDll = NULL) const
	{
		if (0 == dwID) return NULL;

		HBITMAP hBitmap = NULL;

		HINSTANCE hInstDll = NULL;
		BOOL bNewDll = FALSE;

		if (NULL == lpszPathDll)
		{
			if (NULL != m_hInstDll)
				hInstDll = m_hInstDll;
		}
		else
		{
			//Load New Library
			hInstDll = ::LoadLibraryEx(lpszPathDll, NULL, 0);
			if (NULL != hInstDll)
				bNewDll = TRUE;	
		}

		if (NULL != hInstDll)
		{
			hBitmap = (HBITMAP)::LoadImage(hInstDll, MAKEINTRESOURCE(dwID), IMAGE_BITMAP,
				0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

			if (bNewDll)
				::FreeLibrary(hInstDll);
		}

		return hBitmap;
	}

	CPPString _GetStringFromResource(DWORD dwID) const
	{
		if (0 == dwID) return _T("");

		CPPString str;
		str.LoadString(dwID);

		return str;
	}

	CPPString _GetStringFromDll(DWORD dwID, LPCTSTR lpszPathDll = NULL) const
	{
		if (0 == dwID) return _T("");

		CPPString str = _T("");

		HINSTANCE hInstDll = NULL;
		BOOL bNewDll = FALSE;

		if (NULL == lpszPathDll)
		{
			if (NULL != m_hInstDll)
				hInstDll = m_hInstDll;
		}
		else
		{
			//Load New Library
			hInstDll = ::LoadLibraryEx(lpszPathDll, NULL, 0);
			if (NULL != hInstDll)
				bNewDll = TRUE;	
		}

		if (NULL != hInstDll)
		{
#ifdef _UNICODE
#define CHAR_FUDGE 1    // one TCHAR unused is good enough
#else
#define CHAR_FUDGE 2    // two BYTES unused for case of DBC last char
#endif
			// try fixed buffer first (to avoid wasting space in the heap)
			TCHAR szTemp[256];

			DWORD dwLen = ::LoadString(hInstDll, dwID, szTemp, (sizeof(szTemp) * sizeof(TCHAR)));
			// If resource not found (or ::LoadString failure)
			if (0 != dwLen) 
			{
				if ((sizeof(szTemp) * sizeof(TCHAR)) - dwLen > CHAR_FUDGE)
				{
					str = szTemp;
				} // if
				else
				{
					// try buffer size of 512, then larger size until entire string is retrieved
					int nSize = 256;
					do
					{
						nSize += 256;
						dwLen = ::LoadString(hInstDll, dwID, str.GetBuffer(nSize-1), nSize);
					} while (nSize - dwLen <= CHAR_FUDGE);
					str.ReleaseBuffer();
				}
#undef CHAR_FUDGE
			}

			if (bNewDll)
				::FreeLibrary(hInstDll);
		}
		return str;
	}

	///////////////////////////////////////////////////////////
	// Get tooltip string for menu and toolbar items from the 
	// resources of the application.
	// 
	// Parameters:
	//		nID - Resource ID of the string
	//		nNumParam - Which parameter will gets:
	//					 0=long,
	//					 1=short,
	//					 2=disable
	//
	//
	// Format prompt string:  long prompt \n short prompt \n disable prompt
	////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// 
	void _SetListSpecChars()
	{
		_AddSpecChar(_T("&amp;"), _T("&"));			// ampersand
		_AddSpecChar(_T("&bull;"), _T("\x95\0"));	// bullet  NOT IN MS SANS SERIF
		_AddSpecChar(_T("&copy;"), _T("\xA9\0"));	// copyright
		//	AddSpecChar(_T("&euro;"), _T("\x80\0"));	// euro sign IN NOT CYRILLIC FONTS
		_AddSpecChar(_T("&euro;"), _T("\x88\0"));	// euro sign IN CYRILLIC FONTS
		_AddSpecChar(_T("&gt;"), _T(">"));			// greater than
		_AddSpecChar(_T("&iquest;"), _T("\xBF\0"));	// inverted question mark
		_AddSpecChar(_T("&lt;"), _T("<<"));			// less than
		_AddSpecChar(_T("&nbsp;"), _T(" "));			// nonbreaking space
		_AddSpecChar(_T("&para;"), _T("\xB6\0"));	// paragraph sign
		_AddSpecChar(_T("&pound;"), _T("\xA3\0"));	// pound sign
		_AddSpecChar(_T("&quot;"), _T("\""));		// quotation mark
		_AddSpecChar(_T("&reg;"), _T("\xAE\0"));		// registered trademark
		_AddSpecChar(_T("&trade;"), _T("\x99\0"));	// trademark NOT IN MS SANS SERIF
	} //End of _SetListSpecChars

	void _AddSpecChar(LPCTSTR lpszAlias, LPCTSTR lpszValue)
	{
		iter_mapStyles iter = m_mapSpecChars.find(lpszAlias);

		if (iter != m_mapSpecChars.end())
			iter->second = lpszValue;		//Modifies
		else
			m_mapSpecChars.insert(std::make_pair(lpszAlias, lpszValue)); //Add new
	} //End of AddSpecialChar

	void _ReplaceSpecChars()
	{
		CPPString sAlias, sValue;
		for (iter_mapStyles iter = m_mapSpecChars.begin(); iter != m_mapSpecChars.end(); ++iter)
		{
			sAlias = iter->first;
			sValue = iter->second;
			m_csHtmlText.Replace(sAlias, sValue);
		} //for

		m_csHtmlText.Remove(_T('\r'));
		if (!m_bEnableEscapeSequences)
		{
			//ENG: Remove escape sequences
			m_csHtmlText.Remove(_T('\n'));
			m_csHtmlText.Remove(_T('\t'));
		}
		else
		{
			//ENG: Replace escape sequences to HTML tags
			m_csHtmlText.Replace(_T("\n"), _T("<br>"));
			m_csHtmlText.Replace(_T("\t"), _T("<t>"));
		} //if
	} //End of ReplaceSpecChars

	/////////////////////////////////////////////////////////////////////////////
	// 
	void _SetListOfTags()
	{
		_AddTagToList(_T("b"), TAG_BOLD, _T("bold"));
		_AddTagToList(_T("i"), TAG_ITALIC, _T("italic"));
		_AddTagToList(_T("em"), TAG_ITALIC, _T("italic"));
		_AddTagToList(_T("u"), TAG_UNDERLINE, _T("underline"));
		_AddTagToList(_T("s"), TAG_STRIKEOUT, _T("strikeout"));
		_AddTagToList(_T("strike"), TAG_STRIKEOUT, _T("strikeout"));
		_AddTagToList(_T("font"), TAG_FONT, _T("font"));
		_AddTagToList(_T("hr"), TAG_HLINE, _T(""));
		_AddTagToList(_T("br"), TAG_NEWLINE, _T(""));
		_AddTagToList(_T("\n"), TAG_NEWLINE, _T(""));
		_AddTagToList(_T("t"), TAG_TABULATION, _T(""));
		_AddTagToList(_T("\t"), TAG_TABULATION, _T(""));
		_AddTagToList(_T("left"), TAG_LEFT, _T("left"));
		_AddTagToList(_T("center"), TAG_CENTER, _T("center"));
		_AddTagToList(_T("right"), TAG_RIGHT, _T("right"));
		_AddTagToList(_T("justify"), TAG_JUSTIFY, _T("justify"));
		_AddTagToList(_T("baseline"), TAG_BASELINE, _T("baseline"));
		_AddTagToList(_T("top"), TAG_TOP, _T("top"));
		_AddTagToList(_T("vcenter"), TAG_VCENTER, _T("vcenter"));
		_AddTagToList(_T("middle"), TAG_VCENTER, _T("vcenter"));
		_AddTagToList(_T("bottom"), TAG_BOTTOM, _T("vcenter"));
		_AddTagToList(_T("bmp"), TAG_BITMAP, _T(""));
		_AddTagToList(_T("icon"), TAG_ICON, _T(""));
		_AddTagToList(_T("ilst"), TAG_IMAGELIST, _T(""));
		_AddTagToList(_T("string"), TAG_STRING, _T(""));
		_AddTagToList(_T("body"), TAG_NEWSTYLE, _T("body"));
		_AddTagToList(_T("h1"), TAG_NEWSTYLE, _T("h1"));
		_AddTagToList(_T("h2"), TAG_NEWSTYLE, _T("h2"));
		_AddTagToList(_T("h3"), TAG_NEWSTYLE, _T("h3"));
		_AddTagToList(_T("h4"), TAG_NEWSTYLE, _T("h4"));
		_AddTagToList(_T("h5"), TAG_NEWSTYLE, _T("h5"));
		_AddTagToList(_T("h6"), TAG_NEWSTYLE, _T("h6"));
		_AddTagToList(_T("code"), TAG_NEWSTYLE, _T("code"));
		_AddTagToList(_T("pre"), TAG_NEWSTYLE, _T("pre"));
		_AddTagToList(_T("big"), TAG_NEWSTYLE, _T("big"));
		_AddTagToList(_T("small"), TAG_NEWSTYLE, _T("small"));
		_AddTagToList(_T("sub"), TAG_NEWSTYLE, _T("sub"));
		_AddTagToList(_T("sup"), TAG_NEWSTYLE, _T("sup"));
		_AddTagToList(_T("span"), TAG_SPAN, _T("span"));
		_AddTagToList(_T("a"), TAG_HYPERLINK, _T("link"));
	} //End of _SetListOfTags

	////////////////////////////////////////////////////////////////////////
	// Format for the new tags:
	//		lpszName		- a tag's name in the HTML string
	//		dwTagIndex		- ID of the tag
	//		lpszFullName	- a custom name if tag must be closing. Empty if not.  
	////////////////////////////////////////////////////////////////////////
	void _AddTagToList(LPCTSTR lpszName, DWORD dwTagIndex, LPCTSTR lpszFullName)
	{
		STRUCT_TAGPROP tp;
		tp.dwTagIndex = dwTagIndex;
		tp.strTagName = lpszFullName;

		iterMapTags iterMap = m_mapTags.find(lpszName);

		if (iterMap != m_mapTags.end())
			iterMap->second = tp; //Modifies
		else
			m_mapTags.insert(std::make_pair(lpszName, tp)); //Add new
	} //End of _AddTagToList

	DWORD _GetTagFromList(CPPString sTagName, CPPString & strFullName, BOOL & bCloseTag)
	{
		strFullName.Empty();

		bCloseTag = (sTagName.GetAt(0) == _T('/')) ? TRUE : FALSE;
		if (bCloseTag)
			sTagName = sTagName.Mid(1);

		iterMapTags iterMap = m_mapTags.find(sTagName);

		if (iterMap != m_mapTags.end())
		{
			STRUCT_TAGPROP tp = iterMap->second;
			strFullName = tp.strTagName;

			return tp.dwTagIndex;
		} //if

		return TAG_NONE;
	} //End of GetTagFromList

	///////////////////////////////////////////////////////
	// 
	///////////////////////////////////////////////////////
	void _SetTableOfColors()
	{
		//Frequency used
		_SetColorName(_T("aqua"), RGB(0x00, 0xFF, 0xFF)); 
		_SetColorName(_T("black"), RGB(0x00, 0x00, 0x00)); 
		_SetColorName(_T("blue"), RGB(0x00, 0x00, 0xFF)); 
		_SetColorName(_T("brown"), RGB(0xA5, 0x2A, 0x2A)); 
		_SetColorName(_T("cyan"), RGB(0x00, 0xFF, 0xFF));
		_SetColorName(_T("gold"), RGB(0xFF, 0xD7, 0x00)); 
		_SetColorName(_T("gray"), RGB(0x80, 0x80, 0x80)); 
		_SetColorName(_T("green"), RGB(0x00, 0x80, 0x00)); 
		_SetColorName(_T("magenta"), RGB(0xFF, 0x00, 0xFF)); 
		_SetColorName(_T("maroon"), RGB(0x80, 0x00, 0x00)); 
		_SetColorName(_T("navy"), RGB(0x00, 0x00, 0x80)); 
		_SetColorName(_T("olive"), RGB(0x80, 0x80, 0x00)); 
		_SetColorName(_T("orange"), RGB(0xFF, 0xA5, 0x00)); 
		_SetColorName(_T("pink"), RGB(0xFF, 0xC0, 0xCB)); 
		_SetColorName(_T("purple"), RGB(0x80, 0x00, 0x80)); 
		_SetColorName(_T("red"), RGB(0xFF, 0x00, 0x00)); 
		_SetColorName(_T("silver"), RGB(0xC0, 0xC0, 0xC0)); 
		_SetColorName(_T("snow"), RGB(0xFF, 0xFA, 0xFA)); 
		_SetColorName(_T("violet"), RGB(0xEE, 0x82, 0xEE)); 
		_SetColorName(_T("white"), RGB(0xFF, 0xFF, 0xFF)); 
		_SetColorName(_T("yellow"), RGB(0xFF, 0xFF, 0x00)); 

		//Common Used
		_SetColorName(_T("aliceblue"), RGB(0xF0, 0xF8, 0xFF)); 
		_SetColorName(_T("antiquewhite"), RGB(0xFA, 0xEB, 0xD7)); 
		_SetColorName(_T("aquamarine"), RGB(0x7F, 0xFF, 0xD4)); 
		_SetColorName(_T("azure"), RGB(0xF0, 0xFF, 0xFF)); 
		_SetColorName(_T("beige"), RGB(0xF5, 0xF5, 0xDC)); 
		_SetColorName(_T("bisque"), RGB(0xFF, 0xE4, 0xC4));
		_SetColorName(_T("blanchedalmond"), RGB(0xFF, 0xEB, 0xCD)); 
		_SetColorName(_T("blueviolet"), RGB(0x8A, 0x2B, 0xE2)); 
		_SetColorName(_T("burlywood"), RGB(0xDE, 0xB8, 0x87)); 
		_SetColorName(_T("cadetblue"), RGB(0x5F, 0x9E, 0xA0)); 
		_SetColorName(_T("chartreuse"), RGB(0x7F, 0xFF, 0x00)); 
		_SetColorName(_T("chocolate"), RGB(0xD2, 0x69, 0x1E)); 
		_SetColorName(_T("coral"), RGB(0xFF, 0x7F, 0x50)); 
		_SetColorName(_T("cornflowerblue"), RGB(0x64, 0x95, 0xED)); 
		_SetColorName(_T("cornsilk"), RGB(0xFF, 0xF8, 0xDC)); 
		_SetColorName(_T("crimson"), RGB(0xDC, 0x14, 0x3C)); 
		_SetColorName(_T("darkblue"), RGB(0x00, 0x00, 0x8B)); 
		_SetColorName(_T("darkcyan"), RGB(0x00, 0x8B, 0x8B)); 
		_SetColorName(_T("darkgoldenrod"), RGB(0xB8, 0x86, 0x0B)); 
		_SetColorName(_T("darkgray"), RGB(0xA9, 0xA9, 0xA9)); 
		_SetColorName(_T("darkgreen"), RGB(0x00, 0x64, 0x00)); 
		_SetColorName(_T("darkkhaki"), RGB(0xBD, 0xB7, 0x6B)); 
		_SetColorName(_T("darkmagenta"), RGB(0x8B, 0x00, 0x8B)); 
		_SetColorName(_T("darkolivegreen"), RGB(0x55, 0x6B, 0x2F)); 
		_SetColorName(_T("darkorange"), RGB(0xFF, 0x8C, 0x00)); 
		_SetColorName(_T("darkorchid"), RGB(0x99, 0x32, 0xCC)); 
		_SetColorName(_T("darkred"), RGB(0x8B, 0x00, 0x00)); 
		_SetColorName(_T("darksalmon"), RGB(0xE9, 0x96, 0x7A)); 
		_SetColorName(_T("darkseagreen"), RGB(0x8F, 0xBC, 0x8B)); 
		_SetColorName(_T("darkslateblue"), RGB(0x48, 0x3D, 0x8B)); 
		_SetColorName(_T("darkslategray"), RGB(0x2F, 0x4F, 0x4F)); 
		_SetColorName(_T("darkturquoise"), RGB(0x00, 0xCE, 0xD1)); 
		_SetColorName(_T("darkviolet"), RGB(0x94, 0x00, 0xD3)); 
		_SetColorName(_T("deeppink"), RGB(0xFF, 0x14, 0x93)); 
		_SetColorName(_T("deepskyblue"), RGB(0x00, 0xBF, 0xFF)); 
		_SetColorName(_T("dimgray"), RGB(0x69, 0x69, 0x69)); 
		_SetColorName(_T("dodgerblue"), RGB(0x1E, 0x90, 0xFF)); 
		_SetColorName(_T("firebrick"), RGB(0xB2, 0x22, 0x22)); 
		_SetColorName(_T("floralwhite"), RGB(0xFF, 0xFA, 0xF0)); 
		_SetColorName(_T("forestgreen"), RGB(0x22, 0x8B, 0x22)); 
		_SetColorName(_T("fuchsia"), RGB(0xFF, 0x00, 0xFF)); 
		_SetColorName(_T("gainsboro"), RGB(0xDC, 0xDC, 0xDC)); 
		_SetColorName(_T("ghostwhite"), RGB(0xF8, 0xF8, 0xFF)); 
		_SetColorName(_T("goldenrod"), RGB(0xDA, 0xA5, 0x20)); 
		_SetColorName(_T("greenyellow"), RGB(0xAD, 0xFF, 0x2F)); 
		_SetColorName(_T("honeydew"), RGB(0xF0, 0xFF, 0xF0)); 
		_SetColorName(_T("hotpink"), RGB(0xFF, 0x69, 0xB4)); 
		_SetColorName(_T("indianred"), RGB(0xCD, 0x5C, 0x5C)); 
		_SetColorName(_T("indigo"), RGB(0x4B, 0x00, 0x82)); 
		_SetColorName(_T("ivory"), RGB(0xFF, 0xFF, 0xF0)); 
		_SetColorName(_T("khaki"), RGB(0xF0, 0xE6, 0x8C)); 
		_SetColorName(_T("lavender"), RGB(0xE6, 0xE6, 0xFA)); 
		_SetColorName(_T("lavenderblush"), RGB(0xFF, 0xF0, 0xF5)); 
		_SetColorName(_T("lawngreen"), RGB(0x7C, 0xFC, 0x00)); 
		_SetColorName(_T("lemonchiffon"), RGB(0xFF, 0xFA, 0xCD)); 
		_SetColorName(_T("lightblue"), RGB(0xAD, 0xD8, 0xE6)); 
		_SetColorName(_T("lightcoral"), RGB(0xF0, 0x80, 0x80)); 
		_SetColorName(_T("lightcyan"), RGB(0xE0, 0xFF, 0xFF));
		_SetColorName(_T("lightgoldenrodyellow"), RGB(0xFA, 0xFA, 0xD2)); 
		_SetColorName(_T("lightgreen"), RGB(0x90, 0xEE, 0x90)); 
		_SetColorName(_T("lightgrey"), RGB(0xD3, 0xD3, 0xD3)); 
		_SetColorName(_T("lightpink"), RGB(0xFF, 0xB6, 0xC1)); 
		_SetColorName(_T("lightsalmon"), RGB(0xFF, 0xA0, 0x7A)); 
		_SetColorName(_T("lightseagreen"), RGB(0x20, 0xB2, 0xAA)); 
		_SetColorName(_T("lightskyblue"), RGB(0x87, 0xCE, 0xFA)); 
		_SetColorName(_T("lightslategray"), RGB(0x77, 0x88, 0x99)); 
		_SetColorName(_T("lightsteelblue"), RGB(0xB0, 0xC4, 0xDE));
		_SetColorName(_T("lightyellow"), RGB(0xFF, 0xFF, 0xE0)); 
		_SetColorName(_T("lime"), RGB(0x00, 0xFF, 0x00)); 
		_SetColorName(_T("limegreen"), RGB(0x32, 0xCD, 0x32)); 
		_SetColorName(_T("linen"), RGB(0xFA, 0xF0, 0xE6)); 
		_SetColorName(_T("mediumaquamarine"), RGB(0x66, 0xCD, 0xAA)); 
		_SetColorName(_T("mediumblue"), RGB(0x00, 0x00, 0xCD)); 
		_SetColorName(_T("mediumorchid"), RGB(0xBA, 0x55, 0xD3)); 
		_SetColorName(_T("mediumpurple"), RGB(0x93, 0x70, 0xDB)); 
		_SetColorName(_T("mediumseagreen"), RGB(0x3C, 0xB3, 0x71)); 
		_SetColorName(_T("mediumslateblue"), RGB(0x7B, 0x68, 0xEE)); 
		_SetColorName(_T("mediumspringgreen"), RGB(0x00, 0xFA, 0x9A)); 
		_SetColorName(_T("mediumturquoise"), RGB(0x48, 0xD1, 0xCC)); 
		_SetColorName(_T("mediumvioletred"), RGB(0xC7, 0x15, 0x85)); 
		_SetColorName(_T("midnightblue"), RGB(0x19, 0x19, 0x70)); 
		_SetColorName(_T("mintcream"), RGB(0xF5, 0xFF, 0xFA)); 
		_SetColorName(_T("mistyrose"), RGB(0xFF, 0xE4, 0xE1)); 
		_SetColorName(_T("moccasin"), RGB(0xFF, 0xE4, 0xB5)); 
		_SetColorName(_T("navajowhite"), RGB(0xFF, 0xDE, 0xAD)); 
		_SetColorName(_T("oldlace"), RGB(0xFD, 0xF5, 0xE6)); 
		_SetColorName(_T("olivedrab"), RGB(0x6B, 0x8E, 0x23)); 
		_SetColorName(_T("orangered"), RGB(0xFF, 0x45, 0x00)); 
		_SetColorName(_T("orchid"), RGB(0xDA, 0x70, 0xD6)); 
		_SetColorName(_T("palegoldenrod"), RGB(0xEE, 0xE8, 0xAA)); 
		_SetColorName(_T("palegreen"), RGB(0x98, 0xFB, 0x98)); 
		_SetColorName(_T("paleturquoise"), RGB(0xAF, 0xEE, 0xEE)); 
		_SetColorName(_T("palevioletred"), RGB(0xDB, 0x70, 0x93)); 
		_SetColorName(_T("papayawhip"), RGB(0xFF, 0xEF, 0xD5));
		_SetColorName(_T("peachpuff"), RGB(0xFF, 0xDA, 0xB9)); 
		_SetColorName(_T("peru"), RGB(0xCD, 0x85, 0x3F)); 
		_SetColorName(_T("plum"), RGB(0xDD, 0xA0, 0xDD)); 
		_SetColorName(_T("powderblue"), RGB(0xB0, 0xE0, 0xE6)); 
		_SetColorName(_T("rosybrown"), RGB(0xBC, 0x8F, 0x8F)); 
		_SetColorName(_T("royalblue"), RGB(0x41, 0x69, 0xE1)); 
		_SetColorName(_T("saddlebrown"), RGB(0x8B, 0x45, 0x13)); 
		_SetColorName(_T("salmon"), RGB(0xFA, 0x80, 0x72)); 
		_SetColorName(_T("sandybrown"), RGB(0xF4, 0xA4, 0x60)); 
		_SetColorName(_T("seagreen"), RGB(0x2E, 0x8B, 0x57)); 
		_SetColorName(_T("seashell"), RGB(0xFF, 0xF5, 0xEE)); 
		_SetColorName(_T("sienna"), RGB(0xA0, 0x52, 0x2D)); 
		_SetColorName(_T("skyblue"), RGB(0x87, 0xCE, 0xEB)); 
		_SetColorName(_T("slateblue"), RGB(0x6A, 0x5A, 0xCD)); 
		_SetColorName(_T("slategray"), RGB(0x70, 0x80, 0x90)); 
		_SetColorName(_T("springgreen"), RGB(0x00, 0xFF, 0x7F)); 
		_SetColorName(_T("steelblue"), RGB(0x46, 0x82, 0xB4)); 
		_SetColorName(_T("tan"), RGB(0xD2, 0xB4, 0x8C)); 
		_SetColorName(_T("teal"), RGB(0x00, 0x80, 0x80)); 
		_SetColorName(_T("thistle"), RGB(0xD8, 0xBF, 0xD8)); 
		_SetColorName(_T("tomato"), RGB(0xFF, 0x63, 0x47)); 
		_SetColorName(_T("turquoise"), RGB(0x40, 0xE0, 0xD0)); 
		_SetColorName(_T("wheat"), RGB(0xF5, 0xDE, 0xB3)); 
		_SetColorName(_T("whitesmoke"), RGB(0xF5, 0xF5, 0xF5)); 
		_SetColorName(_T("yellowgreen"), RGB(0x9A, 0xCD, 0x32));

		//Systems colors
		_SetColorName(_T("activeborder"), ::GetSysColor(COLOR_ACTIVEBORDER)); 
		_SetColorName(_T("activecaption"), ::GetSysColor(COLOR_ACTIVECAPTION)); 
		_SetColorName(_T("appworkspace"), ::GetSysColor(COLOR_APPWORKSPACE)); 
		_SetColorName(_T("background"), ::GetSysColor(COLOR_BACKGROUND)); 
		_SetColorName(_T("buttonface"), ::GetSysColor(COLOR_BTNFACE)); 
		_SetColorName(_T("buttonhighlight"), ::GetSysColor(COLOR_BTNHILIGHT)); 
		_SetColorName(_T("buttonshadow"), ::GetSysColor(COLOR_BTNSHADOW)); 
		_SetColorName(_T("buttontext"), ::GetSysColor(COLOR_BTNTEXT)); 
		_SetColorName(_T("captiontext"), ::GetSysColor(COLOR_CAPTIONTEXT)); 
		_SetColorName(_T("graytext"), ::GetSysColor(COLOR_GRAYTEXT)); 
		_SetColorName(_T("highlight"), ::GetSysColor(COLOR_HIGHLIGHT)); 
		_SetColorName(_T("highlighttext"), ::GetSysColor(COLOR_HIGHLIGHTTEXT)); 
		_SetColorName(_T("inactiveborder"), ::GetSysColor(COLOR_INACTIVEBORDER)); 
		_SetColorName(_T("inactivecaption"), ::GetSysColor(COLOR_INACTIVECAPTION)); 
		_SetColorName(_T("inactivecaptiontext"), ::GetSysColor(COLOR_INACTIVECAPTIONTEXT)); 
		_SetColorName(_T("infobackground"), ::GetSysColor(COLOR_INFOBK)); 
		_SetColorName(_T("infotext"), ::GetSysColor(COLOR_INFOTEXT)); 
		_SetColorName(_T("menu"), ::GetSysColor(COLOR_MENU)); 
		_SetColorName(_T("menutext"), ::GetSysColor(COLOR_MENUTEXT)); 
		_SetColorName(_T("scrollbar"), ::GetSysColor(COLOR_SCROLLBAR)); 
		_SetColorName(_T("threeddarkshadow"), ::GetSysColor(COLOR_3DDKSHADOW)); 
		_SetColorName(_T("threedface"), ::GetSysColor(COLOR_3DFACE)); 
		_SetColorName(_T("threedhighlight"), ::GetSysColor(COLOR_3DHIGHLIGHT)); 
		_SetColorName(_T("threedlightshadow"), ::GetSysColor(COLOR_3DLIGHT)); 
		_SetColorName(_T("threedshadow"), ::GetSysColor(COLOR_3DSHADOW)); 
		_SetColorName(_T("window"), ::GetSysColor(COLOR_WINDOW)); 
		_SetColorName(_T("windowframe"), ::GetSysColor(COLOR_WINDOWFRAME)); 
		_SetColorName(_T("windowtext"), ::GetSysColor(COLOR_WINDOWTEXT)); 
	} //End SetTableOfColors

	void _SetColorName(LPCTSTR lpszColorName, COLORREF color)
	{
		iterMapColors iterMap = m_mapColors.find(lpszColorName);

		if (iterMap != m_mapColors.end())
			iterMap->second = color; //Modifies
		else
			m_mapColors.insert(std::make_pair(lpszColorName, color)); //Add new
	} //End _SetColorName

	COLORREF _GetColorByName(LPCTSTR lpszColorName, COLORREF crDefColor = RGB(0, 0, 0))
	{
		if (m_bIsEnable)
		{
			iterMapColors iterMap = m_mapColors.find(lpszColorName);

			if (iterMap != m_mapColors.end())
				crDefColor = iterMap->second;
		}
		else
		{
			//For disabled output
			crDefColor = m_crDisabled;
		} //if
		return crDefColor;
	} //End GetColorByName

	/////////////////////////////////////////////////////////////////
	// Gets the system tooltip's logfont
	/////////////////////////////////////////////////////////////////
	LPLOGFONT _GetSystemToolTipFont() const
	{
		static LOGFONT lf;

		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
			return FALSE;

		memcpy(&lf, &(ncm.lfStatusFont), sizeof(LOGFONT));

		return &lf; 
	} //End GetSystemToolTipFont

	////////////////////////////////////////////
	// Check a pointer over the hyperlink
	//   In: lpPoint - the coordinates of the mouse pointer 
	//  Out: -1 - hyperlink not found
	//       index of the hyperlink
	////////////////////////////////////////////
	int _PtInHyperlink(LPPOINT lpPoint)
	{
		for (UINT i = 0; i < m_arrLinks.size(); ++i)
		{
			STRUCT_HYPERLINK & link = m_arrLinks [i];
			if ((link.rcArea.left <= lpPoint->x) && (link.rcArea.right >= lpPoint->x) &&
				(link.rcArea.top <= lpPoint->y) && (link.rcArea.bottom >= lpPoint->y))
				return i;
		} //for
		return -1;
	} //End PtInHyperlink

	void _JumpToHyperlink(int nLink)
	{
		STRUCT_HYPERLINK & link = m_arrLinks [nLink];
		//	TRACE(_T("Jump to Hyperlink number = %d\n"), nLink);
		if (!link.sHyperlink.IsEmpty())
		{
			switch (link.nTypeLink)
			{
			case LINK_HREF:
				_GotoURL(link.sHyperlink);
				break;
			case LINK_MESSAGE:
				_CallbackOnClickHyperlink(link.sHyperlink);
				break;
			} //switch
		} //if
	} //End JumpToHyperlink


	void _CallbackOnRepaint(int nIndexLink)
	{
		//	TRACE(_T("CallbackOnRepaint()\n")); 

		if ((NULL == m_csCallbackRepaint.hWnd) || !m_csCallbackRepaint.nMessage)
			return; 

		::SendMessage(m_csCallbackRepaint.hWnd, m_csCallbackRepaint.nMessage, (LPARAM)nIndexLink, m_csCallbackRepaint.lParam);  
	} //End CallbackOnRepaint

	void _CallbackOnClickHyperlink(LPCTSTR sLink)
	{
		//	TRACE(_T("CallbackOnClickHyperlink()\n")); 

		if ((NULL == m_csCallbackLink.hWnd) || !m_csCallbackLink.nMessage)
			return; 

		::SendMessage(m_csCallbackLink.hWnd, m_csCallbackLink.nMessage, (LPARAM)sLink, m_csCallbackLink.lParam);  	
	} //if CallbackOnClickHyperlink

	HINSTANCE _GotoURL(LPCTSTR url, int showcmd = SW_SHOW)
	{
		SetHyperlinkCursor(NULL);

		TCHAR key[MAX_PATH + MAX_PATH];

		// First try ShellExecute()
		HINSTANCE result = ::ShellExecute(NULL, _T("open"), url, NULL, NULL, showcmd);
		// If it failed, get the .htm regkey and lookup the program
		if ((UINT)(UINT_PTR)result <= HINSTANCE_ERROR) 
		{

			if (_GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS) 
			{
				lstrcat(key, _T("\\shell\\open\\command"));

				if (_GetRegKey(HKEY_CLASSES_ROOT, key, key) == ERROR_SUCCESS)
				{
					TCHAR *pos;
					pos = _tcsstr(key, _T("\"%1\""));
					if (pos == NULL) 
					{                     // No quotes found
						pos = _tcsstr(key, _T("%1"));      // Check for %1, without quotes 
						if (pos == NULL)                   // No parameter at all...
							pos = key+lstrlen(key)-1;
						else
							*pos = '\0';                   // Remove the parameter
					}
					else
						*pos = '\0';                       // Remove the parameter

					lstrcat(pos, _T(" "));
					lstrcat(pos, url);

					USES_CONVERSION;
					result = (HINSTANCE)(UINT_PTR) WinExec(T2A(key),showcmd);
				} //if
			} //if
		} //if
		return result;
	} //End GotoURL

	LONG _GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
	{
		HKEY hkey;
		LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

		if (retval == ERROR_SUCCESS) {
			long datasize = MAX_PATH;
			TCHAR data[MAX_PATH];
			RegQueryValue(hkey, NULL, data, &datasize);
			lstrcpy(retdata,data);
			RegCloseKey(hkey);
		} //if

		return retval;
	} //End GetRegKey

	/////////////////////////////////////////////////////////////////
	void _DrawHtml (LPSIZE lpSize, LPCRECT lpRect)
	{
		//ENG: Bounding rectangle of a current area for output
		RECT rcArea;
		rcArea.left = lpRect->left;
		rcArea.right = lpRect->right;
		rcArea.top = lpRect->top;
		rcArea.bottom = lpRect->bottom;

		SIZE szArea;
		szArea.cx = szArea.cy = 0;

		if (MODE_FIRSTPASS == m_nNumPass)
		{
			//ENG: In preparing mode clears an auxiliary tables
			m_arrLinks.clear();
			//		m_arrTable.clear();
			m_arrHtmlLine.clear();
			//		m_arrTableSizes.clear();
			m_arrAni.clear();
		} //if

		m_nCurLine = 0;
		m_nCurTable = -1;
		m_nNumCurTable = -1;
		m_nCurIndexLink = -1;
		m_nCurIndexAni = -1;

		//ENG: Clear stack of tags
		m_arrStack.clear();

		int nIndex = 0;
		int nBegin;
		CPPString strText;

		//ENG: Applies a default styles
		_SetDefaultStyles(m_defStyle);
		_SelectNewHtmlStyle(_T("body"), m_defStyle);

		//ENG: Creates a default font
		m_lfDefault.lfHeight = m_defStyle.nSizeFont;
		m_lfDefault.lfWidth = 0;
		m_lfDefault.lfOrientation = 0;
		m_lfDefault.lfEscapement = 0;
		m_lfDefault.lfWeight = m_defStyle.nWeightFont;
		m_lfDefault.lfItalic = m_defStyle.bItalicFont;
		m_lfDefault.lfStrikeOut = m_defStyle.bStrikeOutFont;
		m_lfDefault.lfUnderline = m_defStyle.bUnderlineFont;
		m_lfDefault.lfCharSet = DEFAULT_CHARSET;
		m_lfDefault.lfOutPrecision = OUT_DEFAULT_PRECIS;
		m_lfDefault.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		m_lfDefault.lfQuality = DEFAULT_QUALITY;
		m_lfDefault.lfPitchAndFamily = FF_DONTCARE;
		_tcscpy (m_lfDefault.lfFaceName, m_defStyle.sFaceFont);
		m_hFont = ::CreateFontIndirect(&m_lfDefault);

		//ENG: Remember a current context setting
		m_hOldFont = (HFONT)::SelectObject(m_hDC, m_hFont);
		m_nOldBkMode = ::SetBkMode(m_hDC, m_defStyle.nBkMode);
		m_crOldText = ::SetTextColor(m_hDC, m_defStyle.crText);
		m_crOldBk = ::SetBkColor(m_hDC, m_defStyle.crBkgnd);
		::GetTextMetrics(m_hDC, &m_tm);

		while (nIndex < m_csHtmlText.GetLength())
		{
			//ENG: Searching a begin of table
			nBegin = nIndex;
			BOOL bFoundTable = _SearchTag(m_csHtmlText, nIndex, _T("table"));

			//ENG: Gets a text before a table
			strText = m_csHtmlText.Mid(nBegin, nIndex - nBegin);

			//ENG: If text before a table is exist
			if (!strText.IsEmpty())
			{
				//ENG: Add a tag BODY around of a output text
				//			strText = _T("<body>") + strText + _T("</body>");

				//ENG: Output a text before of a table
				szArea = _DrawHtmlString(strText, &rcArea);

				//ENG: Updates a output area size
				lpSize->cx = max(lpSize->cx, szArea.cx);
				lpSize->cy += szArea.cy;
				if (MODE_DRAW == m_nNumPass)
					rcArea.top += szArea.cy;
			} //if

			//ENG: If table was found
			if (bFoundTable)
			{
				//ENG: Searching an end of the table
				nBegin = nIndex;
				nIndex += 6;
				_SearchEndOfTable(m_csHtmlText, nIndex);

				//ENG: Cuts a text of a table
				strText = m_csHtmlText.Mid(nBegin, nIndex - nBegin);

				//ENG: Output a table
				szArea = _DrawHtmlTable(strText, &rcArea);

				//ENG: Updates a output area size
				lpSize->cx = max(lpSize->cx, szArea.cx);
				lpSize->cy += szArea.cy;
				if (MODE_DRAW == m_nNumPass)
					rcArea.top += szArea.cy;
			} //if
		} //while

		//ENG: Restore context setting
		::SetBkMode(m_hDC, m_nOldBkMode);
		::SetBkColor(m_hDC, m_crOldBk);
		::SetTextColor(m_hDC, m_crOldText);
		::SelectObject(m_hDC, m_hOldFont);

		//ENG: Clear stack of tags
		m_arrStack.clear();

		//ENG: Delete a font
		::DeleteObject(m_hFont);
	} //End of DrawHtml


	SIZE _DrawHtmlTable (CPPString & sTable, LPCRECT lpRect)
	{
		//ENG: Jump to the next table
		m_nCurTable++;

		int i;
		UINT pos;
		SIZE size = {0, 0};
		SIZE szTable;
		RECT rcTable = {0, 0, 0, 0};
		RECT rcRow;

		if (MODE_FIRSTPASS == m_nNumPass) 
		{
			//ENG: Get size of the table
			szTable = _GetTableDimensions(sTable);

			STRUCT_TABLE st;
			STRUCT_CELL sc;
			sc.nRowSpan = 0;
			sc.nColSpan = 0;
			//		sc.bHeightPercent = FALSE;
			//		sc.bWidthPercent = FALSE;
			//		sc.nHeight = 0;
			//		sc.nWidth = 0;
			sc.szText.cx = sc.szText.cy = sc.szCell.cx = sc.szCell.cy = 0;
			sc.bFixedWidth = FALSE;

			//ENG: Creates a template of an empty table
			vecRow rows;
			for (i = 0; i < szTable.cx; i++)
			{
				rows.push_back(sc);
				st.width.push_back(0);
				st.fixed_width.push_back(FALSE);
			} //for
			for (i = 0; i < szTable.cy; i++)
			{
				st.cells.push_back(rows);
				st.height.push_back(0);
			} //for

			//ENG: Add a new table
			m_arrTables.push_back(st);
		} //if

		//ENG: Gets an info about a current table
		int nIndexTable = m_nCurTable;
		STRUCT_TABLE cur_table = m_arrTables [nIndexTable];

		szTable.cx = (int)cur_table.width.size();
		szTable.cy = (int)cur_table.height.size();

		//ENG: Applies styles of <table> tag
		m_defStyle.strTag = _T("table");
		_StoreRestoreStyle(FALSE);
		_SelectNewHtmlStyle(m_defStyle.strTag, m_defStyle);

		//ENG: Passes a tag body and get a properties of the tag
		int nIndex = 0;
		CPPString sTag;
		_SearchNextTag(sTable, sTag, nIndex);
		CPPString sProperties = _SplitTag(sTag);

		//ENG: Analyses a properties of the tag
		_AnalyseCellParam(sProperties, m_defStyle, TRUE);
		_UpdateContext();

		if (MODE_FIRSTPASS != m_nNumPass)
		{
			//ENG: Gets a real size of the table
			rcTable.left = lpRect->left;
			rcTable.top = rcTable.bottom = lpRect->top;

			int nWidthTable = m_defStyle.nPadding + (int)cur_table.width.size() - 1;
			for (pos = 0; pos < cur_table.width.size(); ++pos)
				nWidthTable += cur_table.width [pos] + m_defStyle.nPadding;
			rcTable.bottom += m_defStyle.nPadding + (int)cur_table.height.size() - 1;
			for (pos = 0; pos < cur_table.height.size(); ++pos)
				rcTable.bottom += cur_table.height [pos] + m_defStyle.nPadding;

			if (CPPDrawManager::PEN_DOUBLE == m_defStyle.nBorderStyle)
			{
				nWidthTable += 6;
				rcTable.bottom += 6;
			}
			else
			{
				nWidthTable += m_defStyle.nBorderWidth * 2;
				rcTable.bottom += m_defStyle.nBorderWidth * 2;
			} //if

			//ENG: Horizontal align of the table
			int nRealWidth = lpRect->right - lpRect->left;

			if (nWidthTable < nRealWidth)
			{
				int nDelta = nRealWidth - nWidthTable;
				int nNotFixedColumns = 0;
				for (pos = 0; pos < cur_table.fixed_width.size(); ++pos)
				{
					if (!cur_table.fixed_width [pos])
						nNotFixedColumns++;
				} //for
				for (pos = 0; (pos < cur_table.fixed_width.size()) && (nNotFixedColumns > 0); ++pos)
				{
					if (!cur_table.fixed_width [pos])
					{
						int nStep = nDelta / nNotFixedColumns;
						cur_table.width [pos] += nStep;
						nDelta -= nStep;
						nNotFixedColumns--;
						nWidthTable += nStep;
					} //if
				} //for
			} //if

			if (nWidthTable < nRealWidth)
			{
				switch (m_defStyle.nHorzAlign)
				{
				case ALIGN_RIGHT:
					rcTable.left = lpRect->right - nWidthTable;
					break;
				case ALIGN_CENTER:
					rcTable.left += (nRealWidth - nWidthTable) / 2;
					break;
				} //switch
			} //if
			rcTable.right = rcTable.left + nWidthTable;

			//Calculate the real column's width and row's height
			//		if (CPPDrawManager::PEN_DOUBLE == m_defStyle.nBorderStyle)
			//			rcTable.bottom += m_defStyle.nBorderWidth * 6;
			//		else
			//			rcTable.bottom += m_defStyle.nBorderWidth * 2;
		} //if

		//Draw table border
		if (MODE_DRAW == m_nNumPass)
		{
			if (m_defStyle.nFillBkgnd >= 0)
			{
				m_drawmanager.FillEffect(m_hDC, m_defStyle.nFillBkgnd, &rcTable, 
					m_defStyle.crBkgnd, m_defStyle.crMidBkgnd, m_defStyle.crEndBkgnd,
					5);
			}
			else if (!m_defStyle.strNameResBk.IsEmpty())
			{
				_DrawBackgroundImage(m_hDC, rcTable.left, rcTable.top, rcTable.right - rcTable.left, rcTable.bottom - rcTable.top, m_defStyle.strNameResBk);
			} //if
			if (m_defStyle.nBorderWidth > 0)
			{
				if (m_bIsEnable)
				{
					m_drawmanager.DrawRectangle(m_hDC, &rcTable, m_defStyle.crBorderLight, m_defStyle.crBorderDark,
						m_defStyle.nBorderStyle, m_defStyle.nBorderWidth);
				}
				else
				{
					m_drawmanager.DrawRectangle(m_hDC, &rcTable, m_crDisabled, m_crDisabled,
						m_defStyle.nBorderStyle, m_defStyle.nBorderWidth);
				} //if
			} //if
		} //if

		rcRow = rcTable;

		if (MODE_FIRSTPASS != m_nNumPass)
		{
			if (CPPDrawManager::PEN_DOUBLE == m_defStyle.nBorderStyle)
			{
				rcRow.left += 3;
				rcRow.top  += 3;
				rcRow.right -= 3;
				rcRow.bottom -= 3;
			}
			else
			{
				rcRow.left += m_defStyle.nBorderWidth;
				rcRow.top  += m_defStyle.nBorderWidth;
				rcRow.right -= m_defStyle.nBorderWidth;
				rcRow.bottom -= m_defStyle.nBorderWidth;
			}
		} //if

		if (szTable.cx && szTable.cy)
		{
			int nNewRow = 0;
			int nEndRow;
			CPPString sTagName, sTagParam, sRow;
			for (i = 0; i < szTable.cy; ++i)
			{
				//ENG: Searching a begin of the row
				if (_SearchTag(sTable, nNewRow, _T("tr")))
				{
					//ENG: The begin of the row was found. Searching end of the row
					nEndRow = nNewRow;
					_SearchEndOfRow(sTable, nEndRow);
					//ENG: The end of the row was found
					sRow = sTable.Mid(nNewRow, nEndRow - nNewRow);

					//ENG: Draw a row of the table
					_DrawHtmlTableRow(sRow, &rcRow, cur_table, i);

					//ENG: Jump to char after the end of the row
					nNewRow = nEndRow + 5;
				} //if
			} //for
		} //if

		if (MODE_DRAW != m_nNumPass)
		{
			//ENG: Analysing cell's width
			for (i = 1; i <= szTable.cx; i++)
			{
				for (int y = 0; y < szTable.cy; y++)
				{
					vecRow & row = cur_table.cells [y];
					for (int x = 0; x < szTable.cx; x++)
					{
						STRUCT_CELL & sc = row [x];
						if (sc.nColSpan == i)
						{
							if (i == 1)
							{
								cur_table.width [x] = max (cur_table.width [x], sc.szCell.cx);
								if (sc.bFixedWidth)
									cur_table.fixed_width [x] = TRUE;
							}
							else
							{
								int span_width = 0;
								for (int z = 0; z < i; z++)
								{
									span_width += cur_table.width [x + z];
									if (sc.bFixedWidth)
										cur_table.fixed_width [x + z] = TRUE;
								} //for

								if (span_width < sc.szText.cx)
								{
									int step = (sc.szCell.cx - span_width) / i;
									cur_table.width [x + i - 1] += (sc.szCell.cx - span_width) % i;
									for (int z = 0; z < i; z++)
										cur_table.width [x + z] += step;
								} //if
							} //if
						} //if
					} //for
				} //for
			} //for

			//ENG: Analysing cell's height
			for (i = 1; i <= szTable.cy; i++)
			{
				for (int y = 0; y < szTable.cy; y++)
				{
					vecRow & row = cur_table.cells [y];
					for (int x = 0; x < szTable.cx; x++)
					{
						STRUCT_CELL & sc = row [x];
						if (sc.nRowSpan == i)
						{
							if (i == 1)
								cur_table.height [y] = max (cur_table.height [y], sc.szCell.cy);
							else
							{
								int span_height = 0;
								for (int z = 0; z < i; z++)
									span_height += cur_table.height [y + z];

								if (span_height < sc.szCell.cy)
								{
									int step = (sc.szCell.cy - span_height) / i;
									cur_table.height [y] += (sc.szCell.cy - span_height) % i;
									for (int z = 0; z < i; z++)
										cur_table.height [y + z] += step;
								} //if
							} //if
						} //if
					} //for
				} //for
			} //for

			size.cx += m_defStyle.nPadding + szTable.cx - 1;
			size.cy += m_defStyle.nPadding + szTable.cy - 1;
			for (i = 0; i < szTable.cx; i++)
				size.cx += cur_table.width [i] + m_defStyle.nPadding;
			for (i = 0; i < szTable.cy; i++)
				size.cy += cur_table.height [i] + m_defStyle.nPadding;

			if (CPPDrawManager::PEN_DOUBLE == m_defStyle.nBorderStyle)
			{
				size.cx += m_defStyle.nBorderWidth * 6;
				size.cy += m_defStyle.nBorderWidth * 6;
			}
			else
			{
				size.cx += m_defStyle.nBorderWidth * 2;
				size.cy += m_defStyle.nBorderWidth * 2;
			} //if

			//		size.cx = GetTableWidth(strTable, 0, size.cx, TRUE);
		}
		else
		{
			size.cx = rcTable.right - rcTable.left;
			size.cy = rcTable.bottom - rcTable.top;
		} //if

		//ENG: Stores a current table
		m_arrTables [nIndexTable] = cur_table;

		//ENG: Restore styles before <table> tag
		m_defStyle.strTag = _T("table");
		if (_StoreRestoreStyle(TRUE))
			_UpdateContext();

		return size;
	} //End DrawHtmlTable

	///////////////////////////////////////////////////////////////////////////////
	// DrawHtmlTableRow
	//	Draw a row of the table
	//-----------------------------------------------------------------------------
	// Parameters:
	//		sRow	- a text of the cell with the tags. For example: "<tr>...</tr>"
	//		lpRect	- a bounding rectangle for the row
	//		st		- the info about current table
	//		nRow	- the current row of the table
	///////////////////////////////////////////////////////////////////////////////
	//
	///////////////////////////////////////////////////////////////////////////////
	void _DrawHtmlTableRow(CPPString & sRow, LPCRECT lpRect, STRUCT_TABLE & st, int nRow)
	{
		//ENG: Applies styles of <tr> tag
		m_defStyle.strTag = _T("tr");
		_StoreRestoreStyle(FALSE);
		_SelectNewHtmlStyle(m_defStyle.strTag, m_defStyle);

		int nCol = 0;
		int i;
		vecRow & row = st.cells [nRow];

		//ENG: Passes a tag body and get a properties of the tag
		int nIndex = 0;
		CPPString sTag;
		_SearchNextTag(sRow, sTag, nIndex);
		CPPString sProperties = _SplitTag(sTag);

		//ENG: Analyses a properties of the tag
		_AnalyseCellParam(sProperties, m_defStyle, FALSE);
		_UpdateContext();

		while (nIndex < sRow.GetLength())
		{
			int nEndRow = nIndex;
			int nNewCell = nIndex;
			//ENG: Search an end of the cell or a begin of the nested table
			_SearchTag(sRow, nEndRow, _T("/tr"));
			_SearchTag(sRow, nNewCell, _T("td"));
			if (nNewCell < nEndRow)
			{
				//ENG: Search an existing cell
				STRUCT_CELL * sc2 = &row [nCol];
				while ((sc2->nColSpan < 0) && (nCol < (int)row.size())) 
				{
					nCol++;
					sc2 = &row [nCol];
				} //while
				STRUCT_CELL & sc = row [nCol];
				//ENG: Searching the end of the cell
				nIndex = nNewCell;
				_SearchEndOfCell(sRow, nIndex);
				CPPString sCell = sRow.Mid(nNewCell, nIndex - nNewCell);

				RECT rcCell = {0, 0, 0, 0};
				if (MODE_FIRSTPASS != m_nNumPass)
				{
					//ENG: Gets a real rectangle to draw a cell
					rcCell = *lpRect;
					rcCell.left += m_defStyle.nPadding;
					for (i = 0; i < nCol; i++)
						rcCell.left += st.width [i] + m_defStyle.nPadding + 1;
					rcCell.right = rcCell.left;
					for (i = 0; i < sc.nColSpan; i++)
						rcCell.right += st.width [nCol + i];
					rcCell.right += (sc.nColSpan - 1) * (m_defStyle.nPadding + 1);

					rcCell.top += m_defStyle.nPadding;
					for (i = 0; i < nRow; i++)
						rcCell.top += st.height [i] + m_defStyle.nPadding + 1;
					rcCell.bottom = rcCell.top;
					for (i = 0; i < sc.nRowSpan; i++)
						rcCell.bottom += st.height [nRow + i];
					rcCell.bottom += (sc.nRowSpan - 1) * (m_defStyle.nPadding + 1);

					//ENG: cellspacing - margins from table's edge to the cell's edge
					//				rcCell.left += m_defStyle.nPadding;
					//				rcCell.top += m_defStyle.nPadding;
					//				rcCell.right -= m_defStyle.nPadding;
					//				rcCell.bottom -= m_defStyle.nPadding;
				} //if

				_DrawHtmlTableCell(sCell, &rcCell, sc);

				if (MODE_DRAW != m_nNumPass)
				{
					//ENG: Add a cellspacing
					//				sc.szCell.cx += m_defStyle.nPadding + m_defStyle.nPadding;
					//				sc.szCell.cy += m_defStyle.nPadding + m_defStyle.nPadding;

					//ENG: Stores a span cells
					int nColSpan = sc.nColSpan + nCol;
					int nRowSpan = sc.nRowSpan + nRow;
					for (i = nCol + 1; i < nColSpan; i++)
					{
						STRUCT_CELL & scTemp = row [i];
						scTemp.nColSpan = -1;
						scTemp.nRowSpan = -1;
					} //for
					for (i = nRow + 1; i < nRowSpan; i++)
					{
						vecRow & rowTemp = st.cells [i];
						STRUCT_CELL & scTemp = rowTemp [nCol];
						scTemp.nColSpan = -1;
						scTemp.nRowSpan = -1;
					} //for
				} //if
				nCol += sc.nColSpan;
			}
			else
			{
				nIndex = sRow.GetLength();
			} //if
		} //while

		//ENG: Restore styles before <tr> tag
		m_defStyle.strTag = _T("tr");
		if (_StoreRestoreStyle(TRUE))
			_UpdateContext();

	} //End of DrawHtmlTableRow

	///////////////////////////////////////////////////////////////////////////////
	// DrawHtmlTableCell
	//	Draw a table's cell
	//-----------------------------------------------------------------------------
	// Parameters:
	//		sCell	- a text of the cell with the tags. For example: "<td>...</td>"
	//		lpRect	- a bounding rectangle for cell
	//		sc		- the info about current cell
	///////////////////////////////////////////////////////////////////////////////
	//
	///////////////////////////////////////////////////////////////////////////////
	void _DrawHtmlTableCell(CPPString & sCell, LPCRECT lpRect, STRUCT_CELL & sc)
	{
		if (MODE_DRAW != m_nNumPass)
		{
			sc.szText.cx = 0;
			sc.szText.cy = 0;
		} //if

		RECT rcCell = *lpRect;
		RECT rcText;

		//ENG: Applies styles of <td> tag
		m_defStyle.strTag = _T("td");
		_StoreRestoreStyle(FALSE);
		_SelectNewHtmlStyle(m_defStyle.strTag, m_defStyle);

		//ENG: Passes a tag body and get a properties of the tag
		int nIndex = 0;
		CPPString sTag;
		_SearchNextTag(sCell, sTag, nIndex);
		CPPString sProperties = _SplitTag(sTag);

		//ENG: Analyses a properties of the tag
		m_defStyle.nCellWidth = m_defStyle.nCellHeight = 0;
		m_defStyle.bCellWidthPercent = m_defStyle.bCellHeightPercent = FALSE;
		SIZE szSpan = _AnalyseCellParam(sProperties, m_defStyle, FALSE);

		if (MODE_FIRSTPASS == m_nNumPass)
		{
			//ENG: Stores a cell span info
			sc.nColSpan = szSpan.cx;
			sc.nRowSpan = szSpan.cy;
			//ENG: Stores an info about the recommended cell sizes
			//		sc.nWidth = m_defStyle.nCellWidth;
			//		sc.bWidthPercent = m_defStyle.bCellWidthPercent;
			//		sc.nHeight = m_defStyle.nCellHeight;
			//		sc.bHeightPercent = m_defStyle.bCellHeightPercent;
			//ENG: 
			sc.szText.cx = m_defStyle.nCellWidth;
			//		sc.szText.cy = m_defStyle.nCellHeight;
			sc.szText.cy = 0;

			if (m_defStyle.nCellWidth > 0)
				sc.bFixedWidth = TRUE;

			rcText = rcCell;
			rcText.right = rcText.left + sc.szText.cx;
			rcText.bottom = rcText.top + sc.szText.cy;
		}
		else if (MODE_DRAW == m_nNumPass)
		{
			//ENG: cellspacing - margins from table's edge to the cell's edge
			rcText = rcCell;

			if (m_defStyle.nFillBkgnd >= 0)
			{
				//ENG: Filling cell background
				m_drawmanager.FillEffect(m_hDC, m_defStyle.nFillBkgnd, &rcText, 
					m_defStyle.crBkgnd, m_defStyle.crMidBkgnd, m_defStyle.crEndBkgnd, 5);
			} //if

			//Draws the border
			if (m_bIsEnable) 
				m_drawmanager.DrawRectangle(m_hDC, &rcText, m_defStyle.crBorderDark, m_defStyle.crBorderLight, m_defStyle.nBorderStyle);
			else 
				m_drawmanager.DrawRectangle(m_hDC, &rcText, m_crDisabled, m_crDisabled, m_defStyle.nBorderStyle);

			//ENG: cellpadding - margin from cell's edge to the inside cell text
			rcText.left += m_defStyle.nMargin + m_defStyle.nBorderWidth;
			rcText.top += m_defStyle.nMargin + m_defStyle.nBorderWidth;
			rcText.right -= m_defStyle.nMargin + m_defStyle.nBorderWidth;
			rcText.bottom -= m_defStyle.nMargin + m_defStyle.nBorderWidth;

			//Vertical align
			switch (m_defStyle.nVertAlign)
			{
			case ALIGN_BOTTOM:
				rcText.top = rcText.bottom - sc.szText.cy;
				break;
			case ALIGN_VCENTER:
				rcText.top += (rcText.bottom - rcText.top - sc.szText.cy) / 2;
				break;
			} //switch
		} //if

		//ENG: Draws a cell
		while(nIndex < sCell.GetLength())
		{
			int nEndCell = nIndex;
			int nNewTable = nIndex;
			//ENG: Search an end of the cell or a begin of the nested table
			_SearchTag(sCell, nEndCell, _T("/td"));
			_SearchTag(sCell, nNewTable, _T("table"));
			//ENG: Gets a nearly index of the tag
			int nNearlyTag = min(nEndCell, nNewTable);
			SIZE szTemp = {0, 0};
			if (nNearlyTag > nIndex)
			{
				//ENG: If between the last index and the current index there is a text
				CPPString sText = sCell.Mid(nIndex, nNearlyTag - nIndex);
				szTemp = _DrawHtmlString(sText, &rcText);
				nIndex = nNearlyTag;
			} //if
			else if (nNewTable < nEndCell)
			{
				//ENG: A nested table was found
				nIndex = nNewTable;
				_SearchEndOfTable(sCell, nIndex);
				CPPString sTable = sCell.Mid(nNewTable, nIndex - nNewTable);
				szTemp = _DrawHtmlTable(sTable, &rcText); 
			}
			else
			{
				//ENG: Alas, it is the end of the cell
				nIndex = sCell.GetLength();
			} //if

			if (MODE_DRAW != m_nNumPass)
			{
				//ENG: On first and second passes we are calculate the dimensions of the cell
				sc.szText.cx = max(szTemp.cx, sc.szText.cx);
				sc.szText.cy += szTemp.cy;
			} //if
			rcText.top += szTemp.cy;
		} //while

		if (MODE_DRAW != m_nNumPass)
		{
			//ENG: On first and second passes we are calculate the dimensions of the cell
			sc.szCell.cx = max(m_defStyle.nCellWidth, sc.szText.cx);
			sc.szCell.cy = max(m_defStyle.nCellHeight, sc.szText.cy);

			//ENG: Add the margins of the text from the cell's edges
			sc.szCell.cx += 2 * (m_defStyle.nMargin + m_defStyle.nBorderWidth);
			sc.szCell.cy += 2 * (m_defStyle.nMargin + m_defStyle.nBorderWidth);
		} //if

		//ENG: Restore styles before <td> tag
		m_defStyle.strTag = _T("td");
		if (_StoreRestoreStyle(TRUE))
			_UpdateContext();
	}

	SIZE _DrawHtmlString (CPPString & sHtml, LPCRECT lpRect)
	{
		SIZE szTextArea = {0, 0};

		COLORREF clrShadow = m_bIsEnable ? m_crShadow : _GetColorByName(_T(""));

		//ENG: For any string we are add a <body> tag as wrapper
		sHtml = _T("<body>") + sHtml;
		sHtml += _T("</body>");

		//ENG: Bounding rectangle for a full text
		m_rcOutput.top = lpRect->top;
		m_rcOutput.left = lpRect->left;
		m_rcOutput.bottom = lpRect->bottom;
		m_rcOutput.right = lpRect->right;

		//ENG: The width of the bounding rectangle
		int nTextWrapWidth = m_rcOutput.right - m_rcOutput.left;

		//ENG: A current position for output
		POINT ptOutput;
		ptOutput.x = lpRect->left;
		ptOutput.y = lpRect->top;

		//	szTextArea.cx = szTextArea.cy = 0;
		//	m_szOutput.cx = m_szOutput.cy = 0;

		//	m_szOutput = CSize(0, 0);

		//ENG: If a text is empty
		//	if (str.IsEmpty())
		//	{
		//		szTextArea.cx = szTextArea.cy = 0;
		//		return;
		//	} //if

		int nFirstLine = m_nCurLine;

		//	POINT pt;
		//	pt.x = lpRect->left;
		//	pt.y = lpRect->top;

		int y;
		SIZE sz;

		CPPString sText			= _T("");
		CPPString sTag			= _T("");				//String of the tag
		CPPString sProperties	= _T("");				//String of the tag's property
		CPPString sParameter	= _T("");
		CPPString sValue		= _T("");

		BOOL bCloseTag = FALSE;							//TRUE if tag have symbol '\'

		//ENG: Initializing a new line
		ptOutput.x = _InitNewLine(ptOutput.x);
		int nBeginLineX = ptOutput.x;
		int nSpacesInLine = m_hline.nSpaceChars;
		int nRealWidth = m_hline.nWidthLine;

		int nIndex = 0;
		int nBegin = 0;
		int i = 0;
		while (i < sHtml.GetLength())
		{
			//ENG: Searching a first tag
			sText = _SearchNextTag(sHtml, sTag, i);
			sProperties = _SplitTag(sTag);

			//ENG: Before a tag was exist a text
			if (!sText.IsEmpty())
			{
				//ENG: Transform text
				switch (m_defStyle.nTextTransform)
				{
				case TEXT_TRANSFORM_UPPERCASE:
					//ENG: All chars make upper
					sText.MakeUpper();
					break;
				case TEXT_TRANSFORM_LOWERCASE:
					//ENG: All chars make lower
					sText.MakeLower();
					break;
				case TEXT_TRANSFORM_CAPITALIZE:
					//ENG: Each first char of a word to upper
					sText.MakeLower();
					for (nIndex = 0; nIndex < sText.GetLength(); nIndex++)
					{
						if ((sText.GetAt(nIndex) >= _T('a')) && (sText.GetAt(nIndex) <= _T('z')))
						{
							if ((0 == nIndex) || (_T(' ') == sText.GetAt(nIndex - 1)))
								sText.SetAt(nIndex, sText.GetAt(nIndex) - _T('a') + _T('A'));
						} //if
					} //if
					break;
				} //switch

				while (!sText.IsEmpty())
				{
					//ENG: Reset an additional interval for space chars
					::SetTextJustification(m_hDC, 0, 0);

					//ENG: Gets a size a output text
					::GetTextExtentPoint32(m_hDC, sText, sText.GetLength(), &sz);

					//ENG: Gets a real top coordinate to output with vertical alignment
					y = _VerticalAlignText(ptOutput.y, sz.cy);

					CPPString sTemp = sText;
					int nMaxSize = nTextWrapWidth - ptOutput.x + m_rcOutput.left;

					if (m_nMaxWidth && ((nMaxSize - sz.cx) < 0) && nTextWrapWidth)
					{
						//ENG: Text wrap was enabled and text out for a bounding rectangle
						int nRealSize = nMaxSize;
						sTemp = _GetWordWrap(sText, nTextWrapWidth, nRealSize);
						sz.cx = nRealSize;
					}
					else
					{
						sText.Empty();
					} //if

					if (MODE_DRAW == m_nNumPass)
					{
						if (sz.cx)
						{
							if ((0 == (nRealWidth - sz.cx)) && (_T(' ') == sTemp.GetAt(sTemp.GetLength() - 1)))
							{
								//ENG: Removes the right space chars for the last output in line
								sTemp.TrimRight();
								nSpacesInLine = _GetCountOfChars(sTemp);
								SIZE szTemp;
								::GetTextExtentPoint32(m_hDC, sTemp, sTemp.GetLength(), &szTemp);
								nRealWidth -= (sz.cx - szTemp.cx);
							} //if

							if ((ALIGN_JUSTIFY == m_hline.nHorzAlign) && m_hline.bWrappedLine)
								::SetTextJustification(m_hDC, nMaxSize - nRealWidth, nSpacesInLine);
							nRealWidth -= sz.cx;

							//ENG: Gets a size a output text
							::GetTextExtentPoint32(m_hDC, sTemp, sTemp.GetLength(), &sz);

							//ENG: Stores a current area as a hyperlink area if it available
							_StoreHyperlinkArea(ptOutput.x, y, ptOutput.x + sz.cx, y + sz.cy);

							//ENG: Real output a text
							::TextOut(m_hDC, ptOutput.x, y, sTemp, sTemp.GetLength());
							nSpacesInLine -= _GetCountOfChars(sTemp);

							//ENG: If sets an overline style then draw a line over the text
							if (m_defStyle.bOverlineFont)
							{
								HPEN hpenOverline = ::CreatePen(PS_SOLID, (m_defStyle.nWeightFont >= FW_BOLD) ? 2 : 1, m_defStyle.crText);
								HPEN hOldPen = (HPEN)::SelectObject(m_hDC, hpenOverline);
								::MoveToEx(m_hDC, ptOutput.x, y, NULL);
								::LineTo(m_hDC, ptOutput.x + sz.cx, y);
								::SelectObject(m_hDC, hOldPen);
							} //if
						} //if
					}
					else
					{
						//ENG: Stores a last horizontal alignment
						m_hline.nHorzAlign = m_defStyle.nHorzAlign;

						//ENG:
						//RUS:
						m_hline.nSpaceChars += _GetCountOfChars(sTemp);
					} //if

					//ENG: Moves to a right of the outputed text
					ptOutput.x += sz.cx;
					if (!sText.IsEmpty())
					{
						//ENG: Not all text was printed (cause text wrap)
						m_hline.bWrappedLine = TRUE;
						_Tag_NewLine(&ptOutput, 1, &szTextArea);
						nBeginLineX = ptOutput.x;
						nSpacesInLine = m_hline.nSpaceChars;
						nRealWidth = m_hline.nWidthLine;
					}
				} //while
			} //if

			//ENG: If tag was found then analyzing ...
			if (!sTag.IsEmpty())
			{
				//ENG: Reset temporary parameters
				m_defStyle.strTag.Empty();
				bCloseTag = FALSE;

				//ENG: Get Tag's name
				nIndex = 0;

				//ENG: Searching a tag's value
				DWORD dwTag = _GetTagFromList(sTag, m_defStyle.strTag, bCloseTag);

				//ENG: If a tag was found in a list of the tags
				if (TAG_NONE != dwTag)
				{
					//ENG: If it is a style tag 
					if (!m_defStyle.strTag.IsEmpty())
					{
						//ENG: Checks on permissibility of tag
						if (_StoreRestoreStyle(bCloseTag))
						{
							//ENG: If it isn't a close tag
							if (!bCloseTag)
							{
								//ENG: Processing a tag
								switch (dwTag)
								{
								case TAG_BOLD:
									m_defStyle.nWeightFont <<= 1;
									if (m_defStyle.nWeightFont > FW_BLACK)
										m_defStyle.nWeightFont = FW_BLACK;
									break;
								case TAG_ITALIC:
									m_defStyle.bItalicFont = m_defStyle.bItalicFont ? FALSE : TRUE;
									break;
								case TAG_UNDERLINE:
									m_defStyle.bUnderlineFont = m_defStyle.bUnderlineFont ? FALSE : TRUE;
									break;
								case TAG_STRIKEOUT:
									m_defStyle.bStrikeOutFont = m_defStyle.bStrikeOutFont ? FALSE : TRUE;
									break;
								case TAG_FONT:
									//Search parameters
									while (nIndex < sProperties.GetLength())
									{
										//ENG: Searching a parameters of a tag
										sValue = _GetNextProperty(sProperties, nIndex, sParameter);
										//ENG: If a parameter was found
										if (!sParameter.IsEmpty())
										{
											//ENG: Processing a parameters of a tag
											if (sParameter == _T("face"))
												m_defStyle.sFaceFont = _GetStyleString(sValue, m_defStyle.sFaceFont);
											else if (sParameter == _T("size"))
												m_defStyle.nSizeFont = _GetLengthUnit(sValue, m_defStyle.nSizeFont, TRUE);
											else if (sParameter == _T("color"))
											{
												if (m_bIsEnable)
													m_defStyle.crText = _GetStyleColor(sValue, m_defStyle.crText);
												else
													m_defStyle.crText = _GetColorByName(_T(""));
											}
											else if (sParameter == _T("style"))
												_GetStyleFontShortForm(sValue);
											else if (sParameter == _T("weight"))
												m_defStyle.nWeightFont = _GetStyleFontWeight(sValue, m_defStyle.nWeightFont);
											else if (sParameter == _T("bkgnd"))
											{
												if (((sValue == _T("transparent")) && sValue.IsEmpty()) || !m_bIsEnable)
												{
													m_defStyle.nBkMode = TRANSPARENT;
												}
												else
												{
													m_defStyle.nBkMode = OPAQUE;
													m_defStyle.crBkgnd = _GetStyleColor(sValue, m_defStyle.crBkgnd);
												} //if
											} //if
										} //if
									} //while
									break;
								case TAG_LEFT:
									m_defStyle.nHorzAlign = ALIGN_LEFT;
									break;
								case TAG_CENTER:
									m_defStyle.nHorzAlign = ALIGN_CENTER;
									break;
								case TAG_RIGHT:
									m_defStyle.nHorzAlign = ALIGN_RIGHT;
									break;
								case TAG_JUSTIFY:
									m_defStyle.nHorzAlign = ALIGN_JUSTIFY;
									break;
								case TAG_BASELINE:
									m_defStyle.nVertAlign = ALIGN_BASELINE;
									break;
								case TAG_TOP:
									m_defStyle.nVertAlign = ALIGN_TOP;
									break;
								case TAG_VCENTER:
									m_defStyle.nVertAlign = ALIGN_VCENTER;
									break;
								case TAG_BOTTOM:
									m_defStyle.nVertAlign = ALIGN_BOTTOM;
									break;
								case TAG_NEWSTYLE:
									_SelectNewHtmlStyle(sTag, m_defStyle);
									break;
								case TAG_SPAN:
									while (nIndex < sProperties.GetLength())
									{
										//ENG: Searching a parameters of a tag
										sValue = _GetNextProperty(sProperties, nIndex, sParameter);
										//ENG: If a parameter was found
										if (sParameter == _T("class"))
											_SelectNewHtmlStyle(_T(".") + _GetStyleString(sValue, _T("")), m_defStyle);
									} //while
									break;
								case TAG_HYPERLINK:
									//ENG: A default values
									m_defStyle.nTypeLink = LINK_MESSAGE;
									m_defStyle.sHyperlink.Empty();
									while (nIndex < sProperties.GetLength())
									{
										//ENG: Searching a parameters of a tag
										sValue = _GetNextProperty(sProperties, nIndex, sParameter);
										//ENG: If a parameter was found
										if (!sParameter.IsEmpty())
										{
											//ENG: Processing a parameters of a tag
											if (sParameter == _T("href"))
											{
												m_defStyle.nTypeLink = LINK_HREF;
												m_defStyle.sHyperlink = _GetStyleString(sValue, _T(""));
											} //if
											if (sParameter == _T("msg"))
											{
												m_defStyle.nTypeLink = LINK_MESSAGE;
												m_defStyle.sHyperlink = _GetStyleString(sValue, _T(""));
											} //if
										} //if
									} //while
									//ENG: Gets a index of a current link
									m_nCurIndexLink ++;
									//ENG: If a mouse over this link
									if (m_nCurIndexLink == m_nHoverIndexLink)
										_SelectNewHtmlStyle(_T("a:hover"), m_defStyle);
									else
										_SelectNewHtmlStyle(_T("a:link"), m_defStyle);
									break;
								} //switch
							} //if
							//ENG: Update a device context
							_UpdateContext();
						} //if
					}
					else 
					{
						BOOL bPercent;
						BOOL bShadow;
						BOOL bAutoDelete;
						int nWidth, nNum;

						STRUCT_IMAGE si;
						STRUCT_CHANGESTYLE csTemp; //Temporary structure
						STRUCT_ANIMATION sa;

						SIZE szReal;
						HBITMAP hBitmap = NULL;;
						HICON hIcon = NULL;

						DWORD nMaxCol, nMaxRow;
						UINT nIdRes, nIdDll;

						//ENG: Processing a tag
						switch (dwTag)
						{
						case TAG_HLINE:
							//ENG: Draws the horizontal line
							csTemp = m_defStyle;
							csTemp.nBorderWidth = 1;
							//ENG: Applies a new styles for <hr> tag
							_SelectNewHtmlStyle(_T("hr"), csTemp);
							nWidth = 100;
							bPercent = TRUE;

							while (nIndex < sProperties.GetLength())
							{
								//ENG: Searching a parameters of a tag
								sValue = _GetNextProperty(sProperties, nIndex, sParameter);
								//ENG: If a parameter was found
								if (!sParameter.IsEmpty())
								{
									//ENG: Processing a parameters of a tag
									if (sParameter == _T("width"))
									{
										bPercent = _IsPercentableValue(sValue);
										nWidth = _GetLengthUnit(sValue, 100);
									}
									else if (sParameter == _T("size"))
										csTemp.nBorderWidth = _GetLengthUnit(sValue, csTemp.nBorderWidth);
									else if (sParameter == _T("color"))
									{
										if (m_bIsEnable)
											csTemp.crText = _GetStyleColor(sValue, csTemp.crText);
										else
											csTemp.crText = _GetColorByName(_T(""));
									}
								} //if
							} //while

							if (bPercent)
							{
								if (MODE_FIRSTPASS == m_nNumPass)
								{
									m_hline.nAddPercentWidth += nWidth;
									nWidth = 1;
								}
								else nWidth = ::MulDiv(lpRect->right - lpRect->left, nWidth, 100);
							} //if

							if (MODE_FIRSTPASS == m_nNumPass)
							{
								m_hline.nHeightLine = max(m_hline.nHeightLine, csTemp.nBorderWidth + 8);
								m_hline.nHorzAlign = m_defStyle.nHorzAlign; //Store a last horizontal alignment
							}
							else if (MODE_DRAW == m_nNumPass)
							{
								m_drawmanager.DrawLine(m_hDC, ptOutput.x, ptOutput.y + m_hline.nHeightLine / 2, 
									ptOutput.x + nWidth, ptOutput.y + m_hline.nHeightLine / 2, 
									csTemp.crText, CPPDrawManager::PEN_SOLID, csTemp.nBorderWidth);
							} //if
							ptOutput.x += nWidth;
							break;
						case TAG_NEWLINE:
							//ENG: New line
							nNum = 1;
							if (!sProperties.IsEmpty())
							{
								sProperties = sProperties.Mid(1);
								nNum = _GetLengthUnit(sProperties, nNum);
							} //if
							m_hline.bWrappedLine = FALSE;
							_Tag_NewLine(&ptOutput, nNum, &szTextArea);
							nBeginLineX = ptOutput.x;
							nSpacesInLine = m_hline.nSpaceChars;
							nRealWidth = m_hline.nWidthLine;
							break;
						case TAG_TABULATION:
							//ENG: Tabulation
							nNum = 1;
							if (!sProperties.IsEmpty())
							{
								sProperties = sProperties.Mid(1);
								nNum = _GetLengthUnit(sProperties, nNum);
							} //if
							_Tag_Tabulation(&ptOutput, nNum);
							break;
						case TAG_BITMAP:
							//-----------------------------
							//Draws the bitmap 
							//-----------------------------
							//ENG: Default Parameters
							si.nIdRes = 0;
							si.nIdDll = 0;
							si.nHandle = 0;
							si.nWidth = 100;
							si.bPercentWidth = TRUE;
							si.nHeight = 100;
							si.bPercentHeight = TRUE;
							si.crMask = RGB(255, 0, 255);
							si.bUseMask = FALSE;
							si.nStyles = 0;
							si.nHotStyles = 0;
							si.strSkinRes.Empty();
							si.strSrcFile.Empty();
							si.strPathDll.Empty();

							//ENG: Searching image parameters
							_AnalyseImageParam(sProperties, si);

							//ENG: If a image's source was specified
							if (si.nIdRes || si.nIdDll || si.nHandle || !si.strSkinRes.IsEmpty() || !si.strSrcFile.IsEmpty())
							{
								//ENG: Sets a autodelete flag of the image object
								bAutoDelete = TRUE;

								//ENG: Gets a handle of the image
								if (si.nIdRes)
									hBitmap = _GetBitmapFromResources(si.nIdRes);
								else if (!si.strSkinRes.IsEmpty())
									hBitmap = _GetBitmapFromSkinResources(si.strSkinRes);
								else if (!si.strSrcFile.IsEmpty())
									hBitmap = _GetBitmapFromFile(si.strSrcFile);
								else if (si.nIdDll)
									hBitmap = _GetBitmapFromDll(si.nIdDll, si.strPathDll);
								else if (si.nHandle)
								{
									hBitmap = (HBITMAP)(INT_PTR)si.nHandle;
									//ENG: If an image handle specified, disables autodelete
									bAutoDelete = FALSE;
								} //if

								//ENG: If a handle of an image was retrieved
								if (NULL != hBitmap)
								{
									//ENG: Image with shadow or not?
									bShadow = _IsImageWithShadow(si);

									//ENG: Retrieves an original size of an image
									m_drawmanager.GetSizeOfBitmap(hBitmap, &sz);

									//ENG: Retrieves an output size
									if (si.bPercentWidth) si.nWidth = ::MulDiv(sz.cx, si.nWidth, 100);
									if (si.bPercentHeight) si.nHeight = ::MulDiv(sz.cy, si.nHeight, 100);

									//ENG: If a shadow was enabled then set a real size
									if (si.nWidth && si.nHeight && bShadow)
									{
										sz.cx = si.nWidth + m_szOffsetShadow.cx;
										sz.cy = si.nHeight + m_szOffsetShadow.cy;
									} //if

									int nMaxSize = nTextWrapWidth - ptOutput.x + m_rcOutput.left;
									if (m_nMaxWidth && ((nMaxSize - sz.cx) < 0) && nTextWrapWidth) 
									{
										//ENG: Not all text was printed (cause text wrap) 
										m_hline.bWrappedLine = TRUE;
										_Tag_NewLine(&ptOutput, 1, &szTextArea);
										nBeginLineX = ptOutput.x;
										nSpacesInLine = m_hline.nSpaceChars;
										nRealWidth = m_hline.nWidthLine;
									} //if
									nRealWidth -= sz.cx;

									//ENG: Store a last horizontal alignment
									if (MODE_FIRSTPASS == m_nNumPass) 
										m_hline.nHorzAlign = m_defStyle.nHorzAlign;

									//ENG: Retrieves a vertical coordinates of drawing area
									y = _VerticalAlignImage(ptOutput.y, si.nHeight);

									//ENG: If an image is exist and not prepare mode
									if (si.nWidth && si.nHeight && (MODE_DRAW == m_nNumPass))
									{
										//ENG: Add an output area to hyperlink list if needed
										_StoreHyperlinkArea(ptOutput.x, y, ptOutput.x + sz.cx, y + sz.cy);

										//ENG: If a mouse over an image then applies a hot styles
										if (m_defStyle.nTypeLink != LINK_NONE)
										{
											if (m_nCurIndexLink == m_nHoverIndexLink)
												si.nStyles = si.nHotStyles;
										} //if

										if (!m_bIsEnable)
											si.nStyles = (si.nStyles & 0xFF00) | IMAGE_EFFECT_MONOCHROME;

										//ENG: Drawing an image
										m_drawmanager.DrawBitmap(m_hDC, ptOutput.x, y, si.nWidth, si.nHeight, hBitmap, 
											si.bUseMask, si.crMask, si.nStyles, 
											bShadow, 
											m_szOffsetShadow.cx, m_szOffsetShadow.cy, 
											m_szDepthShadow.cx, m_szDepthShadow.cy, 
											clrShadow);
									} //if

									//ENG: Moves to a right of the outputed image
									ptOutput.x += sz.cx; //si.nWidth;

									//ENG: If needed delete a handle of an image
									if (bAutoDelete)
										::DeleteObject(hBitmap);
								} //if
							} //if
							break;
						case TAG_ICON:
							//-----------------------------
							//Draws the icon
							//-----------------------------
							//ENG: Default Parameters
							si.nIdRes = 0;
							si.nIdDll = 0;
							si.nHandle = 0;
							si.nWidth = 100;
							si.bPercentWidth = TRUE;
							si.nHeight = 100;
							si.bPercentHeight = TRUE;
							si.nStyles = 0;
							si.nHotStyles = 0;
							si.strSrcFile.Empty();
							si.strPathDll.Empty();

							//ENG: Searching image parameters
							_AnalyseImageParam(sProperties, si);

							//ENG: If a image's source was specified
							if (si.nIdRes || si.nIdDll || si.nHandle || !si.strSrcFile.IsEmpty())
							{
								//ENG: Sets a autodelete flag of the image object
								bAutoDelete = TRUE;

								sz.cx = si.nWidth;
								sz.cy = si.nHeight;
								if (si.bPercentWidth) sz.cx = ::MulDiv(::GetSystemMetrics(SM_CXICON), si.nWidth, 100);
								if (si.bPercentHeight) sz.cy = ::MulDiv(::GetSystemMetrics(SM_CYICON), si.nHeight, 100);

								//ENG: Gets a handle of the image
								if (si.nIdRes)
									hIcon = _GetIconFromResources(si.nIdRes, sz.cx, sz.cy);
								else if (!si.strSrcFile.IsEmpty())
									hIcon = _GetIconFromFile(si.strSrcFile, sz.cx, sz.cy);
								else if (si.nIdDll)
									hIcon = _GetIconFromDll(si.nIdDll, sz.cx, sz.cy, si.strPathDll);
								else if (si.nHandle)
								{
									hIcon = (HICON)(INT_PTR)si.nHandle;

									//ENG: If an image handle specified, disables autodelete
									bAutoDelete = FALSE;
								} //if

								//ENG: If a handle of an image was retrieved
								if (NULL != hIcon)
								{
									//ENG: Image with shadow or not?
									BOOL bShadow = _IsImageWithShadow(si);

									//ENG: Retrieves an original size of an image
									m_drawmanager.GetSizeOfIcon(hIcon, &sz);
									si.nWidth = sz.cx;
									si.nHeight = sz.cy;

									//ENG: Retrieves an output size
									//									if (si.bPercentWidth) si.nWidth = ::MulDiv(sz.cx, si.nWidth, 100);
									//									if (si.bPercentHeight) si.nHeight = ::MulDiv(sz.cy, si.nHeight, 100);

									//ENG: If a shadow was enabled then set a real size
									if (si.nWidth && si.nHeight && bShadow)
									{
										sz.cx = si.nWidth + m_szOffsetShadow.cx;
										sz.cy = si.nHeight + m_szOffsetShadow.cy;
									} //if

									int nMaxSize = nTextWrapWidth - ptOutput.x + m_rcOutput.left;
									if (m_nMaxWidth && ((nMaxSize - sz.cx) < 0) && nTextWrapWidth) 
									{
										//ENG: Not all text was printed (cause text wrap) 
										m_hline.bWrappedLine = TRUE;
										_Tag_NewLine(&ptOutput, 1, &szTextArea);
										nBeginLineX = ptOutput.x;
										nSpacesInLine = m_hline.nSpaceChars;
										nRealWidth = m_hline.nWidthLine;
									} //if
									nRealWidth -= sz.cx;

									//ENG: Store a last horizontal alignment
									if (MODE_FIRSTPASS == m_nNumPass) 
										m_hline.nHorzAlign = m_defStyle.nHorzAlign;

									//ENG: Retrieves a vertical coordinates of drawing area
									y = _VerticalAlignImage(ptOutput.y, si.nHeight);

									//ENG: If an image is exist and not prepare mode
									if (si.nWidth && si.nHeight && (MODE_DRAW == m_nNumPass))
									{
										//ENG: Add an output area to hyperlink list if needed
										_StoreHyperlinkArea(ptOutput.x, y, ptOutput.x + sz.cx, y + sz.cy);

										//ENG: If a mouse over an image then applies a hot styles
										if (m_defStyle.nTypeLink != LINK_NONE)
										{
											if (m_nCurIndexLink == m_nHoverIndexLink)
												si.nStyles = si.nHotStyles;
										} //if

										if (!m_bIsEnable)
											si.nStyles = (si.nStyles & 0xFF00) | IMAGE_EFFECT_MONOCHROME;

										//ENG: Drawing an image
										m_drawmanager.DrawIcon(m_hDC, ptOutput.x, y, si.nWidth, si.nHeight, hIcon, si.nStyles, 
											bShadow, 
											m_szOffsetShadow.cx, m_szOffsetShadow.cy, 
											m_szDepthShadow.cx, m_szDepthShadow.cy, 
											clrShadow);
									} //if
									//ENG: Moves to a right of the outputed image
									ptOutput.x += sz.cx; //si.nWidth;

									//ENG: If needed delete a handle of an image
									if (bAutoDelete) 
										::DestroyIcon(hIcon);
								} //if
							} //if
							break;
						case TAG_IMAGELIST:
							//-----------------------------
							//Draws the icon from image list
							//-----------------------------
							//ENG: Default Parameters
							si.nIndexImageList = 0;
							si.nIdRes = 0;
							si.nIdDll = 0;
							si.nHandle = 0;
							si.nWidth = 100;
							si.bPercentWidth = TRUE;
							si.nHeight = 100;
							si.bPercentHeight = TRUE;
							si.nSpeed = 0;
							si.bUseMask = FALSE;
							si.crMask = RGB(255, 0, 255);
							si.cx = 0;//GetSystemMetrics(SM_CXICON);
							si.cy = 0;//GetSystemMetrics(SM_CYICON);
							si.nStyles = 0;
							si.nHotStyles = 0;
							si.strSrcFile.Empty();
							si.strPathDll.Empty();

							//ENG: Searching image parameters
							_AnalyseImageParam(sProperties, si);

							//ENG: Image with shadow or not?
							bShadow = _IsImageWithShadow(si);

							if (si.nIdRes || si.nIdDll || si.nHandle || !si.strSrcFile.IsEmpty())
							{
								//ENG: Sets a autodelete flag of the image object
								bAutoDelete = TRUE;

								//ENG: Gets a handle of the image
								if (si.nIdRes)
									hBitmap = _GetBitmapFromResources(si.nIdRes);
								else if (!si.strSrcFile.IsEmpty())
									hBitmap = _GetBitmapFromFile(si.strSrcFile);
								else if (si.nIdDll)
									hBitmap = _GetBitmapFromDll(si.nIdDll, si.strPathDll);
								else if (si.nHandle)
								{
									hBitmap = (HBITMAP)(INT_PTR)si.nHandle;
									//ENG: If an image handle specified, disables autodelete
									bAutoDelete = FALSE;
								} //if

								//ENG: If a handle of an image was retrieved
								if (NULL != hBitmap)
								{
									//ENG: Retrieves an original size of an image
									m_drawmanager.GetSizeOfBitmap(hBitmap, &sz);

									//ENG: Creates a no specified sizes
									if (!si.cx && !si.cy)
										si.cx = si.cy = min(sz.cx, sz.cy);
									else if (!si.cx)
										si.cx = si.cy;
									else if (!si.cy)
										si.cy = si.cx;

									//ENG: Retrieves an output size
									if (si.bPercentWidth) si.nWidth = ::MulDiv(si.cx, si.nWidth, 100);
									if (si.bPercentHeight) si.nHeight = ::MulDiv(si.cy, si.nHeight, 100);

									//ENG: If a shadow was enabled then set a real size
									szReal.cx = si.nWidth;
									szReal.cy = si.nHeight;
									if (si.nWidth && si.nHeight && bShadow)
									{
										szReal.cx += m_szOffsetShadow.cx;
										szReal.cy += m_szOffsetShadow.cy;
									} //if

									//ENG: Gets a max columns and rows of the images on the bitmap
									nMaxCol = sz.cx / si.cx;
									nMaxRow = sz.cy / si.cy;

									if (si.nSpeed)
									{
										if (MODE_FIRSTPASS == m_nNumPass)
										{
											sa.nIndex = si.nIndexImageList;
											sa.nMaxImages = nMaxCol * nMaxRow;
											sa.nSpeed = si.nSpeed;
											sa.nTimerCount = 0;
											m_arrAni.push_back(sa);
										}
										else if (MODE_DRAW == m_nNumPass)
										{
											m_nCurIndexAni ++;
											sa = m_arrAni [m_nCurIndexAni];
											si.nIndexImageList = sa.nIndex;
										} //if
									} //if

									//ENG: If a specified index of image is a legitimate value
									if ((si.nIndexImageList < (int)(nMaxCol * nMaxRow)) && nMaxCol && nMaxRow)
									{
										int nMaxSize = nTextWrapWidth - ptOutput.x + m_rcOutput.left;
										if (m_nMaxWidth && ((nMaxSize - szReal.cx) < 0) && nTextWrapWidth) 
										{
											//ENG: Not all text was printed (cause text wrap) 
											m_hline.bWrappedLine = TRUE;
											_Tag_NewLine(&ptOutput, 1, &szTextArea);
											nBeginLineX = ptOutput.x;
											nSpacesInLine = m_hline.nSpaceChars;
											nRealWidth = m_hline.nWidthLine;
										} //if
										nRealWidth -= szReal.cx;

										//ENG: Store a last horizontal alignment
										if (MODE_FIRSTPASS == m_nNumPass) 
											m_hline.nHorzAlign = m_defStyle.nHorzAlign;

										//ENG: Retrieves a vertical coordinates of drawing area
										y = _VerticalAlignImage(ptOutput.y, szReal.cy);

										//ENG: If an image is exist and not prepare mode
										if (si.nWidth && si.nHeight && (MODE_DRAW == m_nNumPass))
										{
											//ENG: Add an output area to hyperlink list if needed
											_StoreHyperlinkArea(ptOutput.x, y, ptOutput.x + szReal.cx, y + szReal.cy);

											//ENG: If a mouse over an image then applies a hot styles
											if (m_defStyle.nTypeLink != LINK_NONE)
											{
												if (m_nCurIndexLink == m_nHoverIndexLink)
													si.nStyles = si.nHotStyles;
											} //if

											if (!m_bIsEnable)
												si.nStyles = (si.nStyles & 0xFF00) | IMAGE_EFFECT_MONOCHROME;

											//ENG: Drawing an image
											m_drawmanager.DrawImageList(m_hDC, ptOutput.x, y, si.nWidth, si.nHeight, hBitmap,
												si.nIndexImageList, si.cx, si.cy,
												si.bUseMask, si.crMask, si.nStyles, 
												bShadow, 
												m_szOffsetShadow.cx, m_szOffsetShadow.cy, 
												m_szDepthShadow.cx, m_szDepthShadow.cy, 
												clrShadow);
										} //if

										//ENG: Moves to a right of the outputed image
										ptOutput.x += szReal.cx;
									} //if

									//ENG: If needed delete a handle of an image
									if (bAutoDelete)
										::DeleteObject(hBitmap);
								} //if
							}
							else if (NULL != m_hImageList)
							{
								// Ensure that the common control DLL is loaded. 
								InitCommonControls(); 

								if ((int)si.nIndexImageList < ImageList_GetImageCount(m_hImageList))
								{
									hIcon = ImageList_ExtractIcon(NULL, m_hImageList, si.nIndexImageList);
									if (NULL != hIcon)
									{
										sz.cx = si.nWidth;
										sz.cy = si.nHeight;
										if (si.bPercentWidth) sz.cx = ::MulDiv(m_szImageList.cx, si.nWidth, 100);
										if (si.bPercentHeight) sz.cy = ::MulDiv(m_szImageList.cy, si.nHeight, 100);

										szReal.cx = sz.cx;
										szReal.cy = sz.cy;
										if (sz.cx && sz.cy && bShadow)
										{
											szReal.cx += m_szOffsetShadow.cx;
											szReal.cy += m_szOffsetShadow.cy;
										} //if

										int nMaxSize = nTextWrapWidth - ptOutput.x + m_rcOutput.left;
										if (m_nMaxWidth && ((nMaxSize - szReal.cx) < 0) && nTextWrapWidth) 
										{
											//ENG: Not all text was printed (cause text wrap) 
											m_hline.bWrappedLine = TRUE;
											_Tag_NewLine(&ptOutput, 1, &szTextArea);
											nBeginLineX = ptOutput.x;
											nSpacesInLine = m_hline.nSpaceChars;
											nRealWidth = m_hline.nWidthLine;
										} //if
										nRealWidth -= sz.cx;

										if (MODE_FIRSTPASS == m_nNumPass) 
											m_hline.nHorzAlign = m_defStyle.nHorzAlign; //Store a last horizontal alignment
										y = _VerticalAlignImage(ptOutput.y, szReal.cy);
										if (sz.cx && sz.cy && (MODE_DRAW == m_nNumPass))
										{
											_StoreHyperlinkArea(ptOutput.x, y, ptOutput.x + szReal.cx, y + szReal.cy);

											if (m_defStyle.nTypeLink != LINK_NONE)
											{
												if (m_nCurIndexLink == m_nHoverIndexLink)
													si.nStyles = si.nHotStyles;
											} //if

											if (!m_bIsEnable)
												si.nStyles = (si.nStyles & 0xFF00) | IMAGE_EFFECT_MONOCHROME;

											m_drawmanager.DrawIcon(m_hDC, ptOutput.x, y, 
												sz.cx, sz.cy, hIcon, si.nStyles, 
												bShadow, 
												m_szOffsetShadow.cx, m_szOffsetShadow.cy, 
												m_szDepthShadow.cx, m_szDepthShadow.cy, 
												clrShadow);
											::DestroyIcon(hIcon);
										} //if
										ptOutput.x += szReal.cx;
									} //if
								} //if
							} //if
							break;
						case TAG_STRING:
							//-----------------------------
							//Draws the string
							//-----------------------------
							nIdRes = 0;
							nIdDll = 0;
							sText.Empty();
							while (nIndex < sProperties.GetLength())
							{
								//ENG: Searching a parameters of a tag
								sValue = _GetNextProperty(sProperties, nIndex, sParameter);
								//ENG: If a parameter was found
								if (!sParameter.IsEmpty())
								{
									if (sParameter == _T("idres"))
										nIdRes = _GetLengthUnit(sValue, nIdRes);
									else if (sParameter == _T("iddll"))
										nIdRes = _GetLengthUnit(sValue, nIdDll);
									else if (sParameter == _T("srcdll"))
										sText = _GetStyleString(sValue, sText);
								} //if
							} //while
							if (nIdRes || nIdDll)
							{
								if (nIdRes)
									sText = _GetStringFromResource(nIdRes);
								else if (nIdDll)
									sText = _GetStringFromDll(nIdDll, sText);

								if (!sText.IsEmpty())
								{
									::GetTextExtentPoint32(m_hDC, sText, sText.GetLength(), &sz);
									if (MODE_FIRSTPASS == m_nNumPass) m_hline.nHorzAlign = m_defStyle.nHorzAlign; //Store a last horizontal alignment
									y = _VerticalAlignText(ptOutput.y, sz.cy);
									if (MODE_DRAW == m_nNumPass)
									{
										_StoreHyperlinkArea(ptOutput.x, y, ptOutput.x + sz.cx, y + sz.cy);
										::TextOut(m_hDC, ptOutput.x, y, sText, sText.GetLength());
									} //if
									ptOutput.x += sz.cx;
								} //if
							} //if
							break;
						} //switch
					} //if
				} //if
			} //if
		} //for
		if (nBeginLineX != ptOutput.x)
		{
			m_hline.bWrappedLine = FALSE;
			_Tag_NewLine(&ptOutput, 1, &szTextArea);
		}

		//ENG: Reset an additional interval for space chars
		::SetTextJustification(m_hDC, 0, 0);

		szTextArea.cy = ptOutput.y - lpRect->top;

		//Adds the percent's length to the line's length
		for (i = nFirstLine; i < m_nCurLine; i++)
		{
			m_hline = m_arrHtmlLine [i];
			if (0 != m_hline.nAddPercentWidth)
			{
				m_hline.nWidthLine += ::MulDiv(m_hline.nAddPercentWidth, szTextArea.cx, 100);
				szTextArea.cx = max(szTextArea.cx, m_hline.nWidthLine);
			} //if
		} //for
		//
		//	if (NULL != lpSize)
		//	{
		//		szTextArea.cx = m_szOutput.cx;
		//		szTextArea.cy = m_szOutput.cy;
		//	} //if
		return szTextArea;
	} //End DrawHtmlString

	void _StoreHyperlinkArea(int left, int top, int right, int bottom)
	{
		if (m_defStyle.nTypeLink != LINK_NONE)
		{
			STRUCT_HYPERLINK link;
			link.rcArea.left = left;
			link.rcArea.top = top;
			link.rcArea.right = right;
			link.rcArea.bottom = bottom;
			link.sHyperlink = m_defStyle.sHyperlink;
			link.nTypeLink = m_defStyle.nTypeLink;
			link.nIndexLink = m_nCurIndexLink;
			m_arrLinks.push_back(link);
		} //if
	} //StoreHyperlinkArea

	void _SelectNewHtmlStyle(LPCTSTR lpszNameStyle, STRUCT_CHANGESTYLE & cs)
	{
		//Unpack a new styles
		_UnpackTextStyle(_GetTextStyle(lpszNameStyle), cs);
	}

	BOOL _StoreRestoreStyle(BOOL bRestore)
	{
		BOOL bOk = FALSE;
		if (bRestore)
		{
			//Restore styles
			if (m_arrStack.size() > 0)
			{
				STRUCT_CHANGESTYLE cs = m_arrStack.back();
				if (cs.strTag == m_defStyle.strTag)
				{
					m_defStyle = cs;
					m_arrStack.pop_back();
					bOk = TRUE;
				} //if
			} //if
			m_defStyle.strTag.Empty();
		}
		else 
		{
			m_arrStack.push_back(m_defStyle);
			bOk = TRUE;
		} //if

		return bOk;
	} //End StoreRestoreStyle

	void _UpdateContext()
	{
		::SelectObject(m_hDC, m_hOldFont);
		::DeleteObject(m_hFont);
		m_lfDefault.lfHeight = m_defStyle.nSizeFont;
		m_lfDefault.lfWeight = m_defStyle.nWeightFont;
		m_lfDefault.lfItalic = m_defStyle.bItalicFont;
		m_lfDefault.lfStrikeOut = m_defStyle.bStrikeOutFont;
		m_lfDefault.lfUnderline = m_defStyle.bUnderlineFont;
		_tcscpy (m_lfDefault.lfFaceName, m_defStyle.sFaceFont);
		m_hFont = ::CreateFontIndirect(&m_lfDefault);
		m_hOldFont = (HFONT)::SelectObject(m_hDC, m_hFont);
		::GetTextMetrics(m_hDC, &m_tm);

		::SetBkMode(m_hDC, m_defStyle.nBkMode);
		::SetTextColor(m_hDC, m_defStyle.crText);
		::SetBkColor(m_hDC, m_defStyle.crBkgnd);
	} //End UpdateContext

	int _VerticalAlignText(int y, int nHeight)
	{
		//Vertical align
		if (MODE_FIRSTPASS == m_nNumPass)
		{
			//If calculate then don't output text
			m_hline.nDescentLine = max(m_hline.nDescentLine, nHeight - m_tm.tmAscent);
			m_hline.nHeightLine = max(m_hline.nHeightLine, m_tm.tmAscent);
		}
		else if (MODE_DRAW == m_nNumPass)
		{
			switch (m_defStyle.nVertAlign)
			{
			case ALIGN_VCENTER:
				y += (m_hline.nHeightLine - m_tm.tmHeight) / 2;
				break;
			case ALIGN_BASELINE:
				y += m_hline.nHeightLine - m_hline.nDescentLine - m_tm.tmAscent;
				break;
			case ALIGN_BOTTOM:
				y += m_hline.nHeightLine - m_tm.tmAscent;
				break;
			} //switch
		} //if
		return y;
	} //End VerticalAlignText

	int _VerticalAlignImage(int y, int nHeight)
	{
		//Vertical align
		if (MODE_FIRSTPASS == m_nNumPass)
		{
			//If calculate then don't output text
			m_hline.nHeightLine = max(m_hline.nHeightLine, nHeight);
		}
		else if (MODE_DRAW == m_nNumPass)
		{
			switch (m_defStyle.nVertAlign)
			{
			case ALIGN_VCENTER:
				y += (m_hline.nHeightLine - nHeight) / 2;
				break;
			case ALIGN_BASELINE:
				y += m_hline.nHeightLine - m_hline.nDescentLine - nHeight;
				break;
			case ALIGN_BOTTOM:
				y += m_hline.nHeightLine - nHeight;
				break;
			} //switch
		} //if
		return y;
	} //End VerticalAlignImage

	void _Tag_NewLine(LPPOINT lpPoint, int nNum, LPSIZE lpSize)
	{
		//New line
		if (nNum <= 0)
			nNum = 1;

		if (MODE_FIRSTPASS == m_nNumPass)
		{
			if (!m_hline.nHeightLine)
				m_hline.nHeightLine = m_tm.tmHeight;
			lpSize->cx = max(lpSize->cx, lpPoint->x - m_rcOutput.left);
			m_hline.nWidthLine = lpPoint->x - m_rcOutput.left; //Adds the real length of the lines
			m_hline.nHeightLine += m_hline.nDescentLine; //Adds the real height of the lines
			m_arrHtmlLine [m_nCurLine] = m_hline;
		} //if

		m_nCurLine ++;

		lpPoint->y += m_hline.nHeightLine * nNum;
		lpPoint->x = _InitNewLine(m_rcOutput.left);	
	} //End Tag_NewLine

	int _InitNewLine(int x)
	{
		if (MODE_FIRSTPASS == m_nNumPass)
		{
			//ENG: Creates a new line with default parameters
			m_hline.nAddPercentWidth = 0;
			m_hline.nDescentLine = 0;
			m_hline.nHeightLine = 0;
			m_hline.nWidthLine = 0;
			m_hline.nHorzAlign = m_defStyle.nHorzAlign;
			m_hline.nSpaceChars = 0;
			m_arrHtmlLine.push_back(m_hline);
		}
		else if (MODE_DRAW == m_nNumPass)
		{
			//ENG: Gets the data of the first line and converts the percent value to the real width
			m_hline = m_arrHtmlLine [m_nCurLine];
			int nRealWidth = m_rcOutput.right - m_rcOutput.left;

			if (m_hline.nAddPercentWidth)
				m_hline.nWidthLine += ::MulDiv(nRealWidth, m_hline.nAddPercentWidth, 100);

			if ((ALIGN_JUSTIFY == m_hline.nHorzAlign) && m_hline.bWrappedLine)
				::SetTextJustification(m_hDC, nRealWidth - m_hline.nWidthLine, m_hline.nSpaceChars);
			else
				::SetTextJustification(m_hDC, 0, 0);

			//ENG: Horizontal coordinate of the begin output
			switch (m_hline.nHorzAlign)
			{
			case ALIGN_CENTER:
				x = m_rcOutput.left + (nRealWidth - m_hline.nWidthLine) / 2;
				break;
			case ALIGN_RIGHT:
				x = m_rcOutput.left + nRealWidth - m_hline.nWidthLine;
				break;
			} //switch
		} //if
		return x;
	} //End of InitNewLine

	void _Tag_Tabulation(LPPOINT lpPoint, int nNum)
	{
		//Tabulation
		if (!nNum)
			nNum = 1;
		int nWidth = (lpPoint->x - m_rcOutput.left) % m_nTabSize;
		if (nWidth)
		{
			//aligns with tab
			lpPoint->x += m_nTabSize - nWidth;
			nNum --;
		} //if
		lpPoint->x += (nNum * m_nTabSize);
	} //End Tag_Tabulation


	// The following appeared in Paul DiLascia's Jan 1998 MSJ articles.
	// It loads a "hand" cursor from the winhlp32.exe module
	void _SetDefaultCursor()
	{
		if (m_hLinkCursor == NULL)                // No cursor handle - load our own
		{
#ifdef IDC_HAND
			//This code was added from Zorglab's comments to hyperlink control from Chris Maunder
			m_hLinkCursor = ::LoadCursor(NULL, IDC_HAND); // Load Windows' hand cursor
			if (m_hLinkCursor != NULL)                    // if not available, load it from winhlp32.exe
				return;
#endif //IDC_HAND
			// Get the windows directory
			CPPString strWndDir;
			GetWindowsDirectory(strWndDir.GetBuffer(MAX_PATH), MAX_PATH);
			strWndDir.ReleaseBuffer();

			strWndDir += _T("\\winhlp32.exe");
			// This retrieves cursor #106 from winhlp32.exe, which is a hand pointer
			HMODULE hModule = LoadLibrary(strWndDir);
			if (hModule) 
			{
				HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
				if (hHandCursor)
					m_hLinkCursor = CopyIcon(hHandCursor);
			} //if
			FreeLibrary(hModule);
		} //if
	} //End SetDefaultCursor


	BOOL _IsImageWithShadow(_STRUCT_IMAGE & si)
	{
		DWORD dwStyles = si.nStyles | si.nHotStyles;
		if ((dwStyles & IMAGE_EFFECT_MONO_SHADOW) || 
			(dwStyles & IMAGE_EFFECT_GRADIENT_SHADOW))
			return TRUE;

		return FALSE;
	}

	///////////////////////////////////////////////////////////////////////////
	// Map of the styles
	void _SetDefaultCssStyles()
	{
		CPPString str = _T("");
		str += _T("body {font-size: 10pt; color:black; font-family:Verdana}\r\n");
		str += _T("p {font-size: 10pt; color:black; font-family:Verdana; font-weight:bold}\r\n");
		str += _T("h1 {font-size: 14pt; color:black; font-family:Verdana; font-weight:bold}\r\n");
		str += _T("h2 {font-size: 13pt; color:#ff9900; font-family:Verdana; font-weight:bold}\r\n");
		str += _T("h3 {font-size: 12pt; color:#ff9900; font-family:Arial; font-weight:bold}\r\n");
		str += _T("h4 {font-size: 10pt; color:black; font-family:Verdana; font-weight:bold}\r\n");
		str += _T("h5 {font-size: 9pt; color:#ff9900; font-family:Verdana; font-weight:bold}\r\n");
		str += _T("h6 {font-size: 65%; color:#626262; font-family:Verdana; font-weight:normal}\r\n");
		str += _T("pre {font-size: 9pt; font-family:\"Courier\"; background-color:#fbedbb}\r\n");
		str += _T("code {color:#990000; font-family:Arial}\r\n");
		str += _T("a:link {text-decoration:none; color:blue}\r\n");
		str += _T("a:hover {text-decoration:underline; color:blue}\r\n");
		str += _T("sub {font-size:65%; vertical-align:bottom}\r\n");
		str += _T("sup {font-size:65%; vertical-align:top}\r\n");
		str += _T("big {font-size:125%}\r\n");
		str += _T("small {font-size:75%}\r\n");
		str += _T(".cpp-comment {color:green; font-style:italic}\r\n");
		//	str += _T("td {text-align:center; color:#ff0000; vertical-align:middle}\r\n");
		//	str += _T("table {padding:2; border-width:1; color:red}\r\n");

		SetCssStyles(str);
	} //End SetDefaultCssStyle

	LPCTSTR _GetTextStyle(LPCTSTR lpszStyleName)
	{
		CPPString name = (CPPString)lpszStyleName;
		name.MakeLower();
		iter_mapStyles iterMap = m_mapStyles.find(name);

		if (iterMap != m_mapStyles.end())
			return (LPCTSTR)iterMap->second;

		//Not found
		return NULL;
	} //End GetTextStyle


	void _RemoveTextStyle(LPCTSTR lpszStyleName)
	{
		CPPString name = (CPPString)lpszStyleName;
		name.MakeLower();
		iter_mapStyles iterMap = m_mapStyles.find(name);

		if (iterMap == m_mapStyles.end())
			return; //item was not found

		m_mapStyles.erase(iterMap);
	} //End RemoveTextStyle

	void _AddToTextStyle(LPCTSTR lpszStyleName, LPCTSTR lpszAddStyle)
	{
	} //End AddToTextStyle

	void _UnpackTextStyle(CPPString strStyle, _STRUCT_CHANGESTYLE & cs)
	{
		//Gets a string
		strStyle.MakeLower();
		if (strStyle.IsEmpty())
			return;

		CPPString strName;
		CPPString strParameter;

		int nBegin;
		TCHAR chSymbol;
		int nIndex = 0;
		CPPString str;

		while (nIndex < strStyle.GetLength())
		{
			//Passes a space in begin string
			if (_GetIndexNextAlphaNum(strStyle, nIndex))
			{
				nBegin = nIndex;
				//Searching end of the style name
				chSymbol = _GetIndexNextChars(strStyle, nIndex, _T(" :"));
				if (0 != chSymbol)
				{
					//Gets a property's name
					strName = strStyle.Mid(nBegin, nIndex - nBegin);

					//Gets a property's value
					strParameter = _GetParameterString(strStyle, nIndex, _T(':'));

					//Analyzing name
					if (strName == _T("font-size"))
					{
						cs.nSizeFont = _GetLengthUnit(strParameter, cs.nSizeFont, TRUE);
					}
					else if (strName == _T("font-family"))
					{
						if (!strParameter.IsEmpty())
							cs.sFaceFont = strParameter;
					}
					else if (strName == _T("font-style"))
					{
						cs.bItalicFont = _GetStyleFontStyle(strParameter, cs.bItalicFont);
					}
					else if (strName == _T("font-weight"))
					{
						cs.nWeightFont = _GetStyleFontWeight(strParameter, cs.nWeightFont);
					}
					else if (strName == _T("text-align"))
					{
						cs.nHorzAlign = _GetStyleHorzAlign(strParameter, cs.nHorzAlign);
					}
					else if (strName == _T("text-transform"))
					{
						cs.nTextTransform = _GetStyleTextTransform(strParameter, cs.nTextTransform);
					}
					else if (strName == _T("color"))
					{
						if (m_bIsEnable)
							cs.crText = _GetStyleColor(strParameter, cs.crText);
						else
							cs.crText = _GetColorByName(_T(""));
					}
					else if (strName == _T("background-color"))
					{
						if (((strParameter == _T("transparent")) && strParameter.IsEmpty()) || !m_bIsEnable)
						{
							cs.nBkMode = TRANSPARENT;
						}
						else
						{
							cs.nBkMode = OPAQUE;
							cs.crBkgnd = _GetStyleColor(strParameter, cs.crBkgnd);
						} //if
					}
					else if (strName == _T("text-decoration"))
					{
						_StyleTextDecoration(strParameter, cs);
					}
					else if (strName == _T("vertical-align"))
					{
						cs.nVertAlign = _GetStyleVertAlign(strParameter, cs.nVertAlign);
					}
					else if (strName == _T("border-color"))
					{
						if (m_bIsEnable)
							cs.crBorderLight = _GetStyleColor(strParameter, cs.crBorderLight);
						else
							cs.crBorderLight = _GetColorByName(_T(""));
						cs.crBorderDark = cs.crBorderLight;
					}
					else if ((strName == _T("border-width")) || (strName == _T("size")))
					{
						cs.nBorderWidth = _StyleBorderWidth(strParameter, cs.nBorderWidth);
						if (!cs.nBorderWidth)
							cs.nBorderStyle = CPPDrawManager::PEN_NULL;
						else if (CPPDrawManager::PEN_NULL == cs.nBorderStyle)
							cs.nBorderStyle = CPPDrawManager::PEN_SOLID;
					}
					else if (strName == _T("border-style"))
					{
						cs.nBorderStyle = _StyleBorder(strParameter, cs.nBorderStyle);
						if ((CPPDrawManager::PEN_NULL != cs.nBorderStyle) && !cs.nBorderWidth)
							cs.nBorderWidth = 1;
					}
					else if (strName == _T("margin"))
					{
						cs.nMargin = _GetLengthUnit(strParameter, cs.nMargin);
					}
					else if (strName == _T("padding"))
					{
						cs.nPadding = _GetLengthUnit(strParameter, cs.nPadding);
					} //if
				} //if
			} //if
		} //while
	} //End UnpackTextStyle

	BOOL _GetStyleFontStyle(CPPString & str, BOOL bDefault)
	{
		if ((str == _T("normal")) || str.IsEmpty())
		{
			bDefault = FALSE;
		}
		else if ((str == _T("italic")) || (str == _T("oblique"))) 
		{
			bDefault = TRUE;
		} //if

		return bDefault;
	} //End _GetStyleFontStyle

	int _GetStyleFontWeight(CPPString & str, int nDefault)
	{
		if ((str == _T("normal")) || str.IsEmpty())
		{
			nDefault = FW_NORMAL;
		}
		else if (str == _T("bold"))
		{
			nDefault = FW_BOLD;
		}
		else if (str == _T("bolder"))
		{
			nDefault = 900;
		}
		else if (str == _T("lighter"))
		{
			nDefault = 100;
		}
		else
		{
			nDefault = _ttoi(str);
		} //if

		return nDefault;
	} //End GetStyleFontWeight

	int _GetStyleHorzAlign(CPPString & str, int nDefault)
	{
		if ((str == _T("left")) || str.IsEmpty())
		{
			nDefault = ALIGN_LEFT;
		}
		else if (str == _T("center"))
		{
			nDefault = ALIGN_CENTER;
		}
		else if (str == _T("right"))
		{
			nDefault = ALIGN_RIGHT;
		}

		return nDefault;
	} //End GetStyleHorzAlign

	int _GetStyleVertAlign(CPPString & str, int nDefault)
	{
		if ((str == _T("baseline")) || str.IsEmpty())
		{
			nDefault = ALIGN_BASELINE;
		}
		else if ((str == _T("middle")) || (str == _T("vcenter")))
		{
			nDefault = ALIGN_VCENTER;
		}
		else if (str == _T("top"))
		{
			nDefault = ALIGN_TOP;
		}
		else if (str == _T("bottom"))
		{
			nDefault = ALIGN_BOTTOM;
		}

		return nDefault;
	} //End GetStyleVertAlign

	int _GetStyleTextTransform(CPPString & str, int nDefault)
	{
		if ((str == _T("none")) || str.IsEmpty())
		{
			nDefault = TEXT_TRANSFORM_NONE;
		}
		else if (str == _T("uppercase"))
		{
			nDefault = TEXT_TRANSFORM_UPPERCASE;
		}
		else if (str == _T("lowercase"))
		{
			nDefault = TEXT_TRANSFORM_LOWERCASE;
		}
		else if (str == _T("capitalize"))
		{
			nDefault = TEXT_TRANSFORM_CAPITALIZE;
		}

		return nDefault;
	}

	COLORREF _GetStyleColor(CPPString & str, COLORREF crDefault)
	{
		//	if (!m_bIsEnable)
		//		return GetColorByName("");

		if (!str.IsEmpty())
		{
			if (str.GetAt(0) == _T('#'))
			{
				if (str.GetLength() == 7)
				{
					CPPString strHex = _T("0x");
					strHex += str.Mid(5, 2);
					strHex += str.Mid(3, 2);
					strHex += str.Mid(1, 2);
					crDefault = (COLORREF)_tcstoul(strHex, 0, 0);
				} //if
			}
			else if ((str.GetAt(0) >= '0') && (str.GetAt(0) <= '9'))
				crDefault = (COLORREF)_tcstoul(str, 0, 0);
			else
				crDefault = _GetColorByName(str, crDefault);
		} //if 

		return crDefault;
	} //End GetStyleColor

	int _GetLengthUnit(CPPString & str, int nDefault, BOOL bFont = FALSE)
	{
		if (str.IsEmpty())
			return nDefault;

		if (_IsPercentableValue(str))
		{
			//Percent value
			int percent = _ttoi(str.Left(str.GetLength() - 1));
			return ::MulDiv(nDefault, percent, 100);
		} //if

		int nSign = 0;
		if (str.GetAt(0) == _T('+')) nSign = 1;
		else if (str.GetAt(0) == _T('-')) nSign = -1;

		if (0 != nSign) str = str.Right(str.GetLength() - 1);

		//ENG: This code fragment fixed by Reinhard Steiner(2004/10/20).
		int nValue = _ttoi(str);
		CPPString strUnit;
		if(str.GetLength() >= 2)
			strUnit = str.Right(2);

		if (strUnit == _T("px"))		nDefault = nValue;
		else if (strUnit == _T("ex"))
		{
			SIZE szText;
			CPPString strText = _T("x");
			::GetTextExtentPoint32(m_hDC, strText, strText.GetLength(), &szText);
			nDefault = nValue * szText.cy;
		}
		else if (strUnit == _T("em"))	nDefault = nValue * m_tm.tmHeight;
		else
		{
			//Gets pixel in inch
			nValue *= ::GetDeviceCaps(m_hDC, LOGPIXELSY);
			if (strUnit == _T("in"))		nDefault = nValue;
			else if (strUnit == _T("cm"))	nDefault = (int)((double)nValue / 2.54);
			else if (strUnit == _T("mm"))	nDefault = (int)((double)nValue / 25.4);
			else if (strUnit == _T("pt"))	nDefault = nValue / 72;
			else if (strUnit == _T("pc"))	nDefault = nValue / 6;
			else
			{
				nValue = _tcstoul(str, 0, 0);//_ttoi(str);
				if ((nValue > 0) && (nValue < 8) && bFont)
				{
					int nSize [] = {8, 10, 12, 14, 18, 24, 36};
					nDefault = nSize [nValue - 1];
				}
				else
				{
					nDefault = nValue;
				} //if
			} //if
		} //if

		return nDefault;
	} //End GetLengthUnit

	void _StyleTextDecoration(CPPString & str, _STRUCT_CHANGESTYLE & cs)
	{
		if (str.IsEmpty())
			str = _T("none");

		int nBegin = 0;
		int nEnd = 0;
		CPPString strTemp;
		while (nBegin < str.GetLength())
		{
			if (_GetIndexNextAlphaNum(str, nBegin))
			{
				nEnd = nBegin;
				_GetIndexNextChars(str, nEnd, _T(" ,"));
				strTemp = str.Mid(nBegin, nEnd - nBegin);
				nBegin = nEnd;
				if (strTemp == _T("none"))
				{
					cs.bUnderlineFont = FALSE;
					cs.bStrikeOutFont = FALSE;
					cs.bOverlineFont = FALSE;
				}
				else if (strTemp == _T("underline"))
				{
					cs.bUnderlineFont = TRUE;
				}
				else if (strTemp == _T("line-through"))
				{
					cs.bStrikeOutFont = TRUE;
				}
				else if (strTemp == _T("overline"))
				{
					cs.bOverlineFont = TRUE;
				}  //if
			} //if
		} //while
	} //End StyleTextDecoration

	int _StyleBorderWidth(CPPString & str, int nDefault)
	{
		if (str ==_T("thin"))		nDefault = ::MulDiv(75, nDefault, 100);
		else if (str ==_T("thick"))	nDefault = ::MulDiv(125, nDefault, 100);
		else if (str !=_T("medium"))nDefault = _GetLengthUnit(str, nDefault);

		return nDefault;
	} //End StyleBorderWidth

	int _StyleBorder(CPPString & str, int nDefault)
	{
		if ((str == _T("none")) || str.IsEmpty()) nDefault = CPPDrawManager::PEN_NULL;
		else if (str == _T("solid")) nDefault = CPPDrawManager::PEN_SOLID;
		else if (str == _T("dotted")) nDefault = CPPDrawManager::PEN_DOT;
		else if (str == _T("dashed")) nDefault = CPPDrawManager::PEN_DASH;
		else if (str == _T("double")) nDefault = CPPDrawManager::PEN_DOUBLE;

		return nDefault;
	} //End StyleBorder

	void _SetDefaultStyles(_STRUCT_CHANGESTYLE & cs)
	{
		m_defStyle.strTag.Empty();		//The name of the last opened tag

		//Font
		m_defStyle.nSizeFont = 16;		//The height of the logic font
		m_defStyle.nWeightFont = FW_NORMAL;	//The weight of the logic font
		m_defStyle.bItalicFont = FALSE;	//Is italic logic font?
		m_defStyle.bUnderlineFont = FALSE;//Is underline logic font?
		m_defStyle.bStrikeOutFont = FALSE;//Is strikeout logic font?
		m_defStyle.bOverlineFont = FALSE; //Is overline logic font?
		m_defStyle.sFaceFont = _T("Verdana");  //The face name of the logic font

		//Color		
		m_defStyle.crText = RGB (0, 0, 0);	//The foreground color 
		m_defStyle.crBkgnd = RGB (255, 255, 255);	//The background color (also begin for the gradient)
		m_defStyle.crBorderLight = RGB (0, 0, 0);	//The border color
		m_defStyle.crBorderDark = RGB (0, 0, 0);	//The border color
		m_defStyle.crMidBkgnd = RGB (255, 255, 255);//The middle background color
		m_defStyle.crEndBkgnd = RGB (255, 255, 255);//The end background color

		//Fill
		m_defStyle.nBkMode = TRANSPARENT;		//The background mode for the text (TRANSPARENT, OPAQUE)
		m_defStyle.nFillBkgnd = -1;	//The fill effect of the background
		m_defStyle.strNameResBk.Empty();

		//Align
		m_defStyle.nHorzAlign = ALIGN_LEFT;	//The horizontal align
		m_defStyle.nVertAlign = ALIGN_BASELINE;	//The vertical align

		//Border
		m_defStyle.nBorderStyle = CPPDrawManager::PEN_NULL;	//The border style
		m_defStyle.nBorderWidth = 0;	//The width of the border

		//Text
		m_defStyle.nTextTransform = TEXT_TRANSFORM_NONE;//Transformation of the text (NONE, UPPERCASE, LOWERCASE, CAPITALIZE)

		//Margins
		m_defStyle.nMargin = 2;

		//Padding
		m_defStyle.nPadding = 0;

		//Hyperlink
		m_defStyle.nTypeLink = LINK_NONE;		//The type of the link (NONE, HREF, MESSAGE)
		m_defStyle.sHyperlink.Empty(); //The additional parameter for the link
	} //SetDefaultStyles

	/////////////////////////////////////////////////////////////////
	// Search body of the next tag
	//---------------------------------------------------------------
	// Parameters:
	//     In: str    - a string with html text
	//         nIndex - an index of the first char to the searching in the string
	//    Out: nIndex - an index of the char in the string after found tag's text
	//         strTag - a string contained the tag's text if was found
	// Return: A string before found tag's text 
	/////////////////////////////////////////////////////////////////
	CPPString _SearchNextTag(CPPString & str, CPPString & strTag, int & nIndex)
	{
		int nBegin;
		CPPString sText = _T("");
		strTag.Empty();

		while (nIndex < str.GetLength())
		{
			nBegin = nIndex;
			//Searching a chars of the begin tag
			nIndex = str.Find(_T("<"), nIndex);
			if (nIndex < 0)
				nIndex = str.GetLength();					//A tag wasn't found
			sText += str.Mid(nBegin, nIndex - nBegin);
			if (nIndex < str.GetLength())
			{
				//May be it is a begin of the tag?
				if ((nIndex < (str.GetLength() - 1)) && (_T('<') != str.GetAt(nIndex + 1)))
				{
					//Yes of cause!!!
					strTag = _GetTagBody(str, nIndex);
					return sText;
				}
				//No, it is a char '<'
				sText += _T("<");
				nIndex += 2;
				break;
			} //if
		} //while
		return sText;
	} //End SearchNextTag

	/////////////////////////////////////////////////////////////////
	// _GetTagBody
	//	Gets a name of tag with a parameters
	//---------------------------------------------------------------
	// Parameters:
	//	[in]
	//		str		-	a string with html text
	//		nIndex	-   an index of the begin of the tag. 
	//	[out]
	//		nIndex  -	an index of char after the tag
	//---------------------------------------------------------------
	// Return values:
	//	A tag's name .
	/////////////////////////////////////////////////////////////////
	CPPString _GetTagBody(CPPString & str, int & nIndex)
	{
		CPPString sTagName = _T("");
		//ENG: Search the tag's end 
		int nEndOfTag = str.Find(_T('>'), nIndex);
		//ENG: The tag's end was found. Passes a tag's begin char ('<')
		nIndex++;
		if (nEndOfTag > nIndex)
		{
			//ENG: Gets a full body of tag
			sTagName = str.Mid(nIndex, nEndOfTag - nIndex);
			//ENG: Jump to next char after the tag
			nIndex = nEndOfTag + 1;
		} //if
		return sTagName;
	} //End of GetTagBody

	/////////////////////////////////////////////////////////////////
	// Split a tag to his name and properties
	//---------------------------------------------------------------
	// Parameters:
	//     In: sTag    - a string with tag's text
	//    Out: sTag	   - a tag's name
	// Return: A property's string 
	/////////////////////////////////////////////////////////////////
	CPPString _SplitTag(CPPString & sTag)
	{
		CPPString sParam(_T(""));
		int nIndex = 0;
		TCHAR tch = _GetIndexNextChars(sTag, nIndex, _T(" ="));
		if (tch != _T('\0'))
		{
			//ENG: The separator was found. Splits a tag's body to his name and his parameteres 
			sParam = sTag.Mid(nIndex);
			sTag = sTag.Left(nIndex);
			sParam.TrimLeft(_T(' '));
		} //if
		return sParam;
	} //End of SplitTag

	CPPString _GetNextProperty(CPPString & str, int & nIndex, CPPString & sProp)
	{
		CPPString sValue(_T(""));
		sProp.Empty();

		//Passes the spaces before a property
		if (_GetIndexNextAlphaNum(str, nIndex))
		{
			//The begin of the property was found
			int nBegin = nIndex;
			//Searching end of the property
			_GetIndexNextChars(str, nIndex, _T(" ="));
			//Gets a property's string
			sProp = str.Mid(nBegin, nIndex - nBegin);
			TCHAR chFound = _GetIndexNextNoChars(str, nIndex, _T(" "));
			if (_T('=') == chFound)
			{
				chFound = _GetIndexNextNoChars(str, nIndex, _T(" ="));
				if ((_T('\'') == chFound) || (_T('\"') == chFound))
				{
					nIndex++;
				}
				else
				{
					chFound = _T(' ');
				} //if
				sValue += chFound;
				nBegin = nIndex;
				_GetIndexNextChars(str, nIndex, sValue);
				sValue = str.Mid(nBegin, nIndex - nBegin);
				nIndex ++;
			} //if
		} //if
		return sValue;
	} //End of GetNextProperty

	/////////////////////////////////////////////////////////////////
	// Searching the next property of the tag
	//---------------------------------------------------------------
	// Parameters:
	//     In: str    - a string with html text
	//         nIndex - an index of the first char to the searching in the string
	//    Out: nIndex - an index of the char in the string after found tag's text
	// Return: A property's string 
	/////////////////////////////////////////////////////////////////
	CPPString _SearchPropertyOfTag(CPPString & str, int & nIndex)
	{
		CPPString sText = _T("");

		//Passes the spaces before a property
		if (_GetIndexNextAlphaNum(str, nIndex))
		{
			//The begin of the property was found
			int nBegin = nIndex;
			//Searching end of the property
			TCHAR chFound = _GetIndexNextChars(str, nIndex, _T(" ="));
			//Gets a property's string
			sText = str.Mid(nBegin, nIndex - nBegin);
		} //if
		return sText;
	} //End SearchPropertyOfTag

	/////////////////////////////////////////////////////////////////
	// Search a tag
	//---------------------------------------------------------------
	// Parameters:
	//     In: str    - a string with html text
	//         nIndex - an index of the first char to the searching in the string
	//    Out: nIndex - an index of the first char of the tag
	// Return: TRUE if specified tag was found 
	//---------------------------------------------------------------
	// Example: (strTag = "table") or (strTag = "/table")
	/////////////////////////////////////////////////////////////////
	BOOL _SearchTag(CPPString & str, int & nIndex, CPPString strTag)
	{
		strTag = _T("<") + strTag;
		while (nIndex < str.GetLength())
		{
			nIndex = str.Find(strTag, nIndex);
			if (nIndex < 0)
				nIndex = str.GetLength();
			else
			{
				if (nIndex > 0)
				{
					if (str.GetAt(nIndex - 1) != _T('<'))
						return TRUE;
					nIndex += 2;
				}
				else return TRUE;
			} //if
		}
		return FALSE;
	} //End SearchTag

	/////////////////////////////////////////////////////////////////
	// Search a first alpha_num chars or first arithmetic char
	//---------------------------------------------------------------
	// Parameters:
	//     In: str    - a string with html text
	//         nIndex - an index of the first char to the searching in the string
	//    Out: nIndex - an index of the first found char
	// Return: TRUE if specified char was found 
	/////////////////////////////////////////////////////////////////
	BOOL _GetIndexNextAlphaNum(CPPString & str, int & nIndex, BOOL bArithmetic = FALSE)
	{
		TCHAR ch;
		for (; nIndex < str.GetLength(); nIndex++)
		{
			ch = str.GetAt(nIndex);
			if ((ch >= _T('0')) && (ch <= _T('9')))
				return TRUE;
			if ((ch >= _T('A')) && (ch <= _T('Z')))
				return TRUE;
			if ((ch >= _T('a')) && (ch <= _T('z')))
				return TRUE;
			if (ch == _T('.'))
				return TRUE;
			if (bArithmetic)
			{
				if ((_T('+') == ch) || (_T('-') == ch) || 
					(_T('*') == ch) || (_T('/') == ch))
					return TRUE;
			} //if
		} //for
		return FALSE;
	} //End GetIndexNextAlphaNum

	/////////////////////////////////////////////////////////////////
	// Search a first char of the chars set
	//---------------------------------------------------------------
	// Parameters:
	//     In: str      - a string with html text
	//         nIndex   - an index of the first char to the searching in the string
	//		   strChars - the set of the chars
	//    Out: nIndex   - an index of the first found char
	// Return: A found char or zero if chars was not found  
	/////////////////////////////////////////////////////////////////
	TCHAR _GetIndexNextChars(CPPString & str, int & nIndex, CPPString strChars)
	{
		int i;
		for (; nIndex < str.GetLength(); nIndex++)
		{
			for (i = 0; i < strChars.GetLength(); i++)
			{
				if (str.GetAt(nIndex) == strChars.GetAt(i))
					return str.GetAt(nIndex);
			} //for
		} //for
		return 0;
	} //End GetIndexNextChars

	/////////////////////////////////////////////////////////////////
	// Search a first char isn't specified in chars set
	//---------------------------------------------------------------
	// Parameters:
	//     In: str      - a string with html text
	//         nIndex   - an index of the first char to the searching in the string
	//		   strChars - the set of the chars
	//    Out: nIndex   - an index of the first char isn't from chars set
	// Return: A found char or zero if all chars was specified in the chars set  
	/////////////////////////////////////////////////////////////////
	TCHAR _GetIndexNextNoChars(CPPString & str, int & nIndex, CPPString strChars)
	{
		int i;
		BOOL bFound;
		for (; nIndex < str.GetLength(); nIndex++)
		{
			bFound = FALSE;
			for (i = 0; (i < strChars.GetLength()) && !bFound; i++)
			{
				if (str.GetAt(nIndex) == strChars.GetAt(i))
					bFound = TRUE;
			} //for
			if (!bFound)
				return str.GetAt(nIndex);
		} //for
		return 0;
	} //End GetIndexNextNoChars

	/////////////////////////////////////////////////////////////////
	// Is exist a property's parameter?
	//---------------------------------------------------------------
	// Parameters:
	//     In: str         - a string with html text
	//         nIndex      - an index of the first char to the searching in the string
	//		   chSeparator - the char is a begin of the parameter
	//    Out: nIndex   - an index of the begin parameter (if it exist) or the begin of the next property
	// Return: TRUE if parameter was found  
	/////////////////////////////////////////////////////////////////
	BOOL _GetBeginParameter(CPPString & str, int & nIndex, TCHAR chSeparator = _T(':'))
	{
		TCHAR ch;
		for (; nIndex < str.GetLength(); nIndex++) 
		{
			//Gets a current char
			ch = str.GetAt(nIndex);
			if (_T(' ') != ch)
			{
				//if it is not space char
				if (chSeparator == ch)
				{
					//if begin of the property's parameter was found
					nIndex ++; //jump to the next char after a begin parameter
					return TRUE;
				}
				else
				{
					return FALSE;
				}//if
			} //if
		} //for
		return FALSE;
	} //End GetBeginParameter

	/////////////////////////////////////////////////////////////////
	// Gets a parameter for the currrent property
	//---------------------------------------------------------------
	// Parameters:
	//     In: str         - a string with html text
	//         nIndex      - an index of the first char to the searching in the string
	//		   chSeparator - the char is a begin of the parameter
	//    Out: nIndex   - an index of the first char after the parameter
	// Return: String of the property's parameter (empty if it is not exist)  
	/////////////////////////////////////////////////////////////////
	CPPString _GetParameterString(CPPString & str, int & nIndex, TCHAR chBeginParam = _T(':'), CPPString strSeparators = _T(";"))
	{
		if (_GetBeginParameter(str, nIndex, chBeginParam))
		{
			//Parameter for the current property was found
			TCHAR ch = _GetIndexNextNoChars(str, nIndex, strSeparators + _T(" "));
			if (0 != ch)
			{
				int nBegin = nIndex;
				if (_T('"') == str.GetAt(nIndex))
				{
					nIndex++;
					TCHAR ch = _GetIndexNextChars(str, nIndex, _T("\""));
					if (_T('"') == ch)
					{
						nIndex ++;
						return str.Mid(nBegin + 1, nIndex - nBegin - 2);
					} //if
				}
				else
				{
					_GetIndexNextChars(str, nIndex, strSeparators);
					return str.Mid(nBegin, nIndex - nBegin);
				} //if
			} //if
		} //if
		return _T("");
	} //End GetParameterString

	/////////////////////////////////////////////////////////////////
	// Gets a name of the tag
	//---------------------------------------------------------------
	// Parameters:
	//     In: str         - a tag's string
	//         nIndex      - an index of the first char to the searching in the string
	//    Out: nIndex   - an index of the first char after the parameter
	// Return: Name of the tag (empty if it is not exist)  
	/////////////////////////////////////////////////////////////////
	CPPString _GetNameOfTag(CPPString & str, int & nIndex)
	{
		CPPString strName = _T("");
		_GetIndexNextNoChars(str, nIndex, _T(" "));
		int nBegin = nIndex;
		_GetIndexNextChars(str, nIndex, _T(" ="));
		if (nIndex > nBegin)
			strName = str.Mid(nBegin, nIndex - nBegin);

		return strName;
	} //End GetNameOfTag

	/////////////////////////////////////////////////////
	// Gets dimensions of the table
	//---------------------------------------------------
	//  In: sTable - the string contains a HTML table
	// Return: cx - number of the columns
	//         cy - number of the row
	/////////////////////////////////////////////////////
	SIZE _GetTableDimensions(CPPString & sTable)
	{
		//ENG: A table dimensions by default
		SIZE szTable = {0, 0};
		int nIndex = 0;
		int nCol = 0;
		while (nIndex < sTable.GetLength())
		{
			//ENG: Search a begin of the row
			if (_SearchTag(sTable, nIndex, _T("tr")))
			{
				//ENG: Increment count of the rows
				szTable.cy++;

				//ENG: Count of the columns in current row
				nCol = 0;
				int nEndRow;
				int nNewCell;
				do 
				{
					nEndRow = nNewCell = nIndex;
					//ENG: Search an end of the row or a begin of the cell
					_SearchTag(sTable, nEndRow, _T("/tr"));
					_SearchTag(sTable, nNewCell, _T("td"));
					if (nNewCell < nEndRow)
					{
						nIndex = nNewCell;

						//ENG: Passes a tag body and get a properties of the tag
						CPPString sTag;
						_SearchNextTag(sTable, sTag, nNewCell);
						CPPString sProperties = _SplitTag(sTag);

						//ENG: Analyses a properties of the tag
						STRUCT_CHANGESTYLE style;
						SIZE szSpan = _AnalyseCellParam(sProperties, style, TRUE);

						//ENG: Increment count of the cells
						nCol += szSpan.cx;

						//ENG: Jump to end of the cell
						_SearchEndOfCell(sTable, nIndex);
					} //if
				} while (nNewCell < nEndRow);
				nIndex = nEndRow;
				if (nCol > szTable.cx)
					szTable.cx = nCol;
			} //if
		} //while
		return szTable;
	} //End GetTableDimensions

	/////////////////////////////////////////////////////
	// _SearchEndOfTable
	//	Searching the end of the table
	//---------------------------------------------------
	//  Parameter:    
	//		str - the string contains a HTML table
	//		nIndex - index of the first char after the <table> tag
	//	Return values:
	//		nIndex - index of the begin char of a </table> tag
	/////////////////////////////////////////////////////
	void _SearchEndOfTable(CPPString & str, int & nIndex)
	{
		int nBeginTable = nIndex + 7;
		int nEndTable = nIndex + 7;
		int nTable = 1;
		do
		{
			_SearchTag(str, nBeginTable, _T("table"));
			_SearchTag(str, nEndTable, _T("/table"));
			if (nBeginTable < nEndTable)
			{
				nTable++;
				nBeginTable += 7;
			}
			else if (nEndTable < nBeginTable)
			{
				nTable --;
				nEndTable += 8;
			} //if
		}
		while ((nBeginTable != nEndTable) && nTable); //while
		nIndex = nEndTable - 8;
	} //End SearchEndOfTable

	/////////////////////////////////////////////////////
	// _SearchEndOfRow
	//	Searching the end of the row
	//---------------------------------------------------
	//  Parameter:    
	//		str - the string contains a HTML table
	//		nIndex - index of the first char after the <tr> tag
	//	Return values:
	//		nIndex - index of the begin char of a </tr> tag
	/////////////////////////////////////////////////////
	void _SearchEndOfRow(CPPString & str, int & nIndex)
	{
		nIndex += 4;
		int nBeginRow, nEndRow, nStartTable;
		int nRow = 1;

		do
		{
			nBeginRow = nEndRow = nStartTable = nIndex;

			_SearchTag(str, nBeginRow, _T("tr"));
			_SearchTag(str, nEndRow, _T("/tr"));
			_SearchTag(str, nStartTable, _T("table"));

			if ((nStartTable < nBeginRow) && (nStartTable < nEndRow))
			{
				_SearchEndOfTable(str, nStartTable);
				nIndex = nStartTable + 6;
			}
			else if (nBeginRow < nEndRow)
			{
				nRow++;
				nIndex = nBeginRow + 4;
			}
			else if (nEndRow < nBeginRow)
			{
				nRow --;
				nIndex = nEndRow + 5;
			} //if
		}
		while ((nIndex < str.GetLength()) && nRow); //while
		nIndex -= 5;
	} //End SearchEndOfRow

	/////////////////////////////////////////////////////
	// _SearchEndOfCell
	//	Searching the end of the cell
	//---------------------------------------------------
	//  Parameter:    
	//		str - the string contains a HTML table
	//		nIndex - index of the first char after the <td> tag
	//	Return values:
	//		nIndex - index of the begin char of a </td> tag
	/////////////////////////////////////////////////////
	void _SearchEndOfCell(CPPString & str, int & nIndex)
	{
		nIndex += 4;
		int nEndCell, nStartTable;
		do
		{
			nEndCell = nStartTable = nIndex;

			_SearchTag(str, nEndCell, _T("/td"));
			_SearchTag(str, nStartTable, _T("table"));

			if (nStartTable < nEndCell)
			{
				_SearchEndOfTable(str, nStartTable);
				nEndCell = nIndex = nStartTable + 6;
			}
			else
			{
				nIndex = nEndCell + 5;
			} //if
		}
		while (nStartTable < nEndCell); //while
		nIndex -= 5;
	} //End SearchEndOfCell

	///////////////////////////////////////////////////////////////////////
	// Analysing the cell parameters
	//---------------------------------------------------------------------
	// Parameters:
	//   In: strTag - str string contains parameters of the <table>, <td> or <tr> tags
	//           cs - the structures contains the current styles
	//		 bTable - 
	//  Out:     cs - the structures contains the new styles
	///////////////////////////////////////////////////////////////////////
	SIZE _AnalyseCellParam(CPPString & sProperties, _STRUCT_CHANGESTYLE & cs, BOOL bTable)
	{
		SIZE szSpan = {1, 1};
		if (sProperties.IsEmpty())
			return szSpan;

		int i = 0;
		CPPString sParameter;
		CPPString sValue;

		while (i < sProperties.GetLength())
		{
			//ENG: Searching a parameters of a tag
			sValue = _GetNextProperty(sProperties, i, sParameter);

			//ENG: Processes the specific parameters for <table> tag.
			if(bTable)
			{
				if (sParameter == _T("cellpadding"))
				{
					cs.nMargin = _GetLengthUnit(sValue, cs.nMargin);
				}
				else if (sParameter == _T("cellspacing"))
				{
					cs.nPadding = _GetLengthUnit(sValue, cs.nPadding);
				} 
				else if (sParameter == _T("background"))
				{
					cs.strNameResBk = sValue;
				} //if
			} //if

			if (sParameter == _T("rowspan"))
			{
				szSpan.cy = _GetLengthUnit(sValue, szSpan.cy);
			}
			else if (sParameter == _T("colspan"))
			{
				szSpan.cx = _GetLengthUnit(sValue, szSpan.cx);
			}
			else if (sParameter == _T("border"))
			{
				cs.nBorderWidth = _GetLengthUnit(sValue, cs.nBorderWidth);
				if (!cs.nBorderWidth)
					cs.nBorderStyle = CPPDrawManager::PEN_NULL;
				else if (CPPDrawManager::PEN_NULL == cs.nBorderStyle)
					cs.nBorderStyle = CPPDrawManager::PEN_SOLID;
			}
			else if (sParameter == _T("borderstyle"))
			{
				cs.nBorderStyle = _StyleBorder(sValue, cs.nBorderStyle);
				if ((CPPDrawManager::PEN_NULL != cs.nBorderStyle) && !cs.nBorderWidth)
					cs.nBorderWidth = 1;
			}
			else if (sParameter == _T("bordercolor"))
			{
				if (m_bIsEnable)
					cs.crBorderLight = _GetStyleColor(sValue, cs.crBorderLight);
				else
					cs.crBorderLight = _GetColorByName(_T(""));
				cs.crBorderDark = cs.crBorderLight;
			}
			else if (sParameter == _T("bordercolorlight"))
			{
				if (m_bIsEnable)
					cs.crBorderLight = _GetStyleColor(sValue, cs.crBorderLight);
				else
					cs.crBorderLight = _GetColorByName(_T(""));
			}
			else if (sParameter == _T("bordercolordark"))
			{
				if (m_bIsEnable)
					cs.crBorderDark = _GetStyleColor(sValue, cs.crBorderDark);
				else
					cs.crBorderDark = _GetColorByName(_T(""));
			}
			else if (sParameter == _T("bgcolor"))
			{
				if (m_bIsEnable)
				{
					cs.crBkgnd = _GetStyleColor(sValue, cs.crBkgnd);
					if (cs.nFillBkgnd < 0)
						cs.nFillBkgnd = CPPDrawManager::EFFECT_SOLID;
				} //if
			}
			else if (sParameter == _T("bgmidcolor"))
			{
				if (m_bIsEnable)
					cs.crMidBkgnd = _GetStyleColor(sValue, cs.crMidBkgnd);
			}
			else if (sParameter == _T("bgendcolor"))
			{
				if (m_bIsEnable)
					cs.crEndBkgnd = _GetStyleColor(sValue, cs.crEndBkgnd);
			}
			else if (sParameter == _T("bgeffect"))
			{
				if (m_bIsEnable)
					cs.nFillBkgnd = _GetStyleBkgndEffect(sValue, cs.nFillBkgnd);
			}
			else if (sParameter == _T("align"))
			{
				cs.nHorzAlign = _GetStyleHorzAlign(sValue, cs.nHorzAlign);
			}
			else if (sParameter == _T("valign"))
			{
				cs.nVertAlign = _GetStyleVertAlign(sValue, cs.nVertAlign);
			}
			else if (sParameter == _T("width"))
			{
				cs.nCellWidth = _GetLengthUnit(sValue, cs.nCellWidth);
			}
			else if (sParameter == _T("height"))
			{
				cs.nCellHeight = _GetLengthUnit(sValue, cs.nCellHeight);
			} //if
		} //for

		//ENG:
		if ((CPPDrawManager::PEN_NULL == cs.nBorderStyle) || !cs.nBorderWidth)
		{
			cs.nBorderStyle = CPPDrawManager::PEN_NULL;
			cs.nBorderWidth = 0;
		}
		else if (CPPDrawManager::PEN_SOLID != cs.nBorderStyle)
		{
			cs.nBorderWidth = 1;
		}	//if

		//ENG: 
		if (!bTable && cs.nBorderWidth)
			cs.nBorderWidth = 1;

		return szSpan;
	} //End AnalyseCellParam

	///////////////////////////////////////////////////////////////////////
	// Analysing the image parameters
	//---------------------------------------------------------------------
	// Parameters:
	//   In: sProperties - the sing contains
	//           si - the structures contains the image parameters
	//  Out:     si - the structures contains the image parameters
	///////////////////////////////////////////////////////////////////////
	void _AnalyseImageParam(CPPString & sProperties, _STRUCT_IMAGE & si)
	{
		if (sProperties.IsEmpty())
			return;

		int i = 0;
		CPPString sParameter;
		CPPString sValue;

		while (i < sProperties.GetLength())
		{
			//ENG: Searching a parameters of a tag
			sValue = _GetNextProperty(sProperties, i, sParameter);

			//		sParameter = SearchPropertyOfTag(sProperties, i);
			//		sValue = GetParameterString(sProperties, i, _T('='), _T(" "));

			if (sParameter == _T("index"))
			{
				si.nIndexImageList = _GetLengthUnit(sValue, si.nIndexImageList);
			}
			else if (sParameter == _T("idres"))
			{
				si.nIdRes = _GetLengthUnit(sValue, si.nIdRes);
			}
			else if (sParameter == _T("iddll"))
			{
				si.nIdDll = _GetLengthUnit(sValue, si.nIdDll);
			}
			else if (sParameter == _T("handle"))
			{
				si.nHandle = _GetLengthUnit(sValue, si.nHandle);
			}
			else if (sParameter == _T("skinres"))						//ksgui中资源名称
			{
				si.strSkinRes = _GetStyleString(sValue, si.strSkinRes);
			}
			else if (sParameter == _T("file"))
			{
				si.strSrcFile = _GetStyleString(sValue, si.strSrcFile);
			}
			else if (sParameter == _T("srcdll"))
			{
				si.strPathDll = _GetStyleString(sValue, si.strPathDll);
			}
			else if (sParameter == _T("mask"))
			{
				si.crMask = _GetStyleColor(sValue, si.crMask);
				si.bUseMask = TRUE;
			}
			else if (sParameter == _T("style"))
			{
				si.nStyles = _GetStyleImageShortForm(sValue);
				si.nHotStyles = si.nStyles;
			}
			else if (sParameter == _T("hotstyle"))
			{
				si.nHotStyles = _GetStyleImageShortForm(sValue);
			}
			else if (sParameter == _T("cx"))
			{
				si.cx = _GetLengthUnit(sValue, si.cx);
			}
			else if (sParameter == _T("cy"))
			{
				si.cy = _GetLengthUnit(sValue, si.cy);
			}
			else if (sParameter == _T("width"))
			{
				si.bPercentWidth = _IsPercentableValue(sValue);
				si.nWidth = _GetLengthUnit(sValue, si.nWidth);
			}
			else if (sParameter == _T("height"))
			{
				si.bPercentHeight = _IsPercentableValue(sValue);
				si.nHeight = _GetLengthUnit(sValue, si.nHeight);
			}
			else if (sParameter == _T("speed"))
			{
				si.nSpeed = _GetLengthUnit(sValue, si.nSpeed);
			} //if
		} //for
	} //End AnalyseImageParam

	CPPString _GetStyleString(CPPString str, CPPString strDefault)
	{
		if (!str.IsEmpty())
			strDefault = str;
		return str;
	}

	///////////////////////////////////////////////////////////////////////
	// Analysing the short form of the font style
	//---------------------------------------------------------------------
	// Parameters:
	//   In: str - string contains parameters of the font in the short form
	// Short form styles
	//       [+] - positive style
	//       [-] - inverse style
	//       [b] - bold
	//       [i] - italic
	//       [u] - underlined
	//       [s] - strikeout
	//       [o] - overline
	///////////////////////////////////////////////////////////////////////
	void _GetStyleFontShortForm(CPPString & str)
	{
		if (!str.IsEmpty())
		{
			BOOL bSetValue = TRUE;
			for (int i = 0; i < str.GetLength(); i++)
			{
				switch (str.GetAt(i))
				{
				case _T('-'):
					bSetValue = FALSE;
					break;
				case _T('+'):
					bSetValue = TRUE;
					break;
				case _T('b'):
					m_defStyle.nWeightFont = (bSetValue) ? FW_BOLD : FW_NORMAL;
					bSetValue = TRUE;
					break;
				case _T('i'):
					m_defStyle.bItalicFont = bSetValue;
					bSetValue = TRUE;
					break;
				case _T('u'):
					m_defStyle.bUnderlineFont = bSetValue;
					bSetValue = TRUE;
					break;
				case _T('s'):
					m_defStyle.bStrikeOutFont = bSetValue;
					bSetValue = TRUE;
					break;
				case _T('o'):
					m_defStyle.bOverlineFont = bSetValue;
					bSetValue = TRUE;
					break;
				} //switch
			} //for
		} //if
	} //End GetStyleFontShortForm

	//Get font style value
	UINT _GetStyleImageShortForm(CPPString & str)
	{
		UINT uStyle = 0; //Original image

		if (!str.IsEmpty())
		{
			for (int i = 0; i < str.GetLength(); i++)
			{
				switch (str.GetAt(i))
				{
				case _T('d'):
					uStyle |= IMAGE_EFFECT_DARKEN;
					break;
				case _T('g'):
					uStyle |= IMAGE_EFFECT_GRAYEN;
					break;
				case _T('s'):
					if (m_szOffsetShadow.cx || m_szOffsetShadow.cy)
					{
						if (m_bGradientShadow)
							uStyle |= IMAGE_EFFECT_GRADIENT_SHADOW;
						else uStyle |= IMAGE_EFFECT_MONO_SHADOW;
					} //if
					break;
				case _T('l'):
					uStyle |= IMAGE_EFFECT_LIGHTEN;
					break;
				} //switch
			} //for
		} //if

		return uStyle;
	} //End GetStyleImageShortForm

	BOOL _IsPercentableValue(CPPString & str)
	{
		if (!str.IsEmpty())
		{
			if (str.GetAt(str.GetLength() - 1) == _T('%'))
				return TRUE;
		}
		return FALSE;
	}

	int _GetStyleBkgndEffect(CPPString & str, int nDefault)
	{
		if (!str.IsEmpty())
		{
			if (str == _T("transparent"))
				nDefault = -1;
			else if (str == _T("solid"))
				nDefault = CPPDrawManager::EFFECT_SOLID;
			else if (str == _T("hgradient"))
				nDefault = CPPDrawManager::EFFECT_HGRADIENT;
			else if (str == _T("vgradient"))
				nDefault = CPPDrawManager::EFFECT_VGRADIENT;
			else if (str == _T("hcgradient"))
				nDefault = CPPDrawManager::EFFECT_HCGRADIENT;
			else if (str == _T("vcgradient"))
				nDefault = CPPDrawManager::EFFECT_VCGRADIENT;
			else if (str == _T("3hgradient"))
				nDefault = CPPDrawManager::EFFECT_3HGRADIENT;
			else if (str == _T("3vgradient"))
				nDefault = CPPDrawManager::EFFECT_3VGRADIENT;
#ifdef USE_SHADE
			else if (str == _T("noise"))
				nDefault = CPPDrawManager::EFFECT_NOISE;
			else if (str == _T("diagshade"))
				nDefault = CPPDrawManager::EFFECT_DIAGSHADE;
			else if (str == _T("hshade"))
				nDefault = CPPDrawManager::EFFECT_HSHADE;
			else if (str == _T("vshade"))
				nDefault = CPPDrawManager::EFFECT_VSHADE;
			else if (str == _T("hbump"))
				nDefault = CPPDrawManager::EFFECT_HBUMP;
			else if (str == _T("vbump"))
				nDefault = CPPDrawManager::EFFECT_VBUMP;
			else if (str == _T("softbump"))
				nDefault = CPPDrawManager::EFFECT_SOFTBUMP;
			else if (str == _T("hardbump"))
				nDefault = CPPDrawManager::EFFECT_HARDBUMP;
			else if (str == _T("metal"))
				nDefault = CPPDrawManager::EFFECT_METAL;
#endif
			else nDefault = _GetLengthUnit(str, nDefault);
		} //if

		return nDefault;
	} //End GetStyleBkgndEffect

	int _GetTableWidth(CPPString & str, int nClientWidth, int nMinWidth, BOOL bSet = FALSE)
	{
		if (!str.IsEmpty())
		{
			int i = 0;
			CPPString strProperty;
			CPPString strParameter;

			while (i < str.GetLength())
			{
				strProperty = _SearchPropertyOfTag(str, i);
				strParameter = _GetParameterString(str, i, _T('='), _T(" "));
				strProperty.MakeLower();

				if (strProperty == _T("width"))
				{
					if (_IsPercentableValue(strParameter))
					{
						int nWidth = _GetLengthUnit(strParameter, 100);
						if (bSet)
						{
							if (nWidth <= 100)
								nClientWidth = ::MulDiv(nMinWidth, 100, nWidth);
							else
								nClientWidth = ::MulDiv(nMinWidth, nWidth, 100);
						}
						else
						{
							if (nWidth < 100)
								nClientWidth = ::MulDiv(nClientWidth, nWidth, 100);
						} //if
					}
					else
					{
						nClientWidth = _GetLengthUnit(strParameter, nMinWidth);
					} //if
					break;
				} //if
			} //while
		} //if

		if (nClientWidth < nMinWidth)
			nClientWidth = nMinWidth;

		return nClientWidth;
	} //End GetTableWidth

	void _DrawBackgroundImage(HDC hDC, int nDestX, int nDestY, int nWidth, int nHeight, CPPString strNameImage)
	{
		if (!m_bIsEnable)
			return;
		if (strNameImage.IsEmpty())
			return;
		if (strNameImage.GetLength() < 6)
			return;

		HBITMAP hBitmap = NULL;

		int nIndex = 0;
		if (_GetIndexNextAlphaNum(strNameImage, nIndex))
		{
			int nBegin = nIndex;
			//Searching end of the style name
			TCHAR chSymbol = _GetIndexNextChars(strNameImage, nIndex, _T(" :"));
			if (0 != chSymbol)
			{
				//Gets a property's name
				CPPString strName = strNameImage.Mid(nBegin, nIndex - nBegin);
				//Gets a property's value
				CPPString strParameter = _GetParameterString(strNameImage, nIndex, _T(':'));

				if (strName == _T("idres"))
				{
					UINT nID = (UINT)_GetLengthUnit(strParameter, 0);
					hBitmap = _GetBitmapFromResources(nID);
				}
				else if (strName == _T("iddll"))
				{
					UINT nID = (UINT)_GetLengthUnit(strParameter, 0);
					hBitmap = _GetBitmapFromDll(nID);
				}
				else if (strName == _T("file"))
				{
					hBitmap = _GetBitmapFromFile(strParameter);
				} //if
			} //if
		} //if

		if (NULL == hBitmap)
			return;

		SIZE sz;
		m_drawmanager.GetSizeOfBitmap(hBitmap, &sz);
		HDC hSrcDC = ::CreateCompatibleDC(hDC);
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hSrcDC, hBitmap);
		m_drawmanager.MultipleCopy(hDC, nDestX, nDestY, nWidth, nHeight, hSrcDC, 0, 0, sz.cx, sz.cy);
		::SelectObject(hSrcDC, hOldBitmap);
		::DeleteDC(hSrcDC);

		::DeleteObject(hBitmap);
		hBitmap = NULL;
	} //End of DrawBackgroundImage


	CPPString _GetWordWrap(CPPString & str, int nMaxSize, int & nRealSize)
	{
		int nCurIndex = 0;
		int nLastIndex = 0;
		SIZE sz = {0, 0};
		TCHAR tch = _T(' ');
		CPPString sResult = _T("");
		while ((sz.cx <= nRealSize) && (0 != tch))
		{
			nLastIndex = nCurIndex;
			nCurIndex ++;
			tch = _GetIndexNextChars(str, nCurIndex, PPHTMLDRAWER_BREAK_CHARS);
			::GetTextExtentPoint32(m_hDC, str, nCurIndex, &sz);
		} //while

		if (0 == nLastIndex)
		{
			if (nMaxSize == nRealSize)
			{
				sz.cx = 0;
				int i;
				for (i = 1; i < str.GetLength(); i++)
				{
					::GetTextExtentPoint32(m_hDC, str, i + 1, &sz);
					if (sz.cx > nRealSize)
					{
						sResult = str.Left(i);
						str = str.Mid(i);
						::GetTextExtentPoint32(m_hDC, sResult, i, &sz);
						nRealSize = sz.cx;
						return sResult;
					} //if
				} //for
				::GetTextExtentPoint32(m_hDC, str, i, &sz);
				sResult = str;
				str.Empty();
			}
			else
			{
				sz.cx = 0;
			} //if
		}
		else 
		{
			sResult = str.Left(nLastIndex + 1);
			str = str.Mid(nLastIndex + 1);
			sResult.TrimRight();
			::GetTextExtentPoint32(m_hDC, sResult, sResult.GetLength(), &sz);
			//		str.TrimRight();
			str.TrimLeft();
		} //if
		nRealSize = sz.cx;
		return sResult;
	} //End of GetWordWrap

	int _GetCountOfChars(CPPString str, TCHAR tchar = _T(' '))
	{
		int nCount = 0;
		//ENG:
		//RUS:
		for (int i = 0; i < str.GetLength(); i++)
		{
			if (tchar == str.GetAt(i))
				nCount++;
		} //if
		return nCount;
	}

// 	void _SetListOfTags();								//Fill a map of tags
// 	void _AddTagToList(LPCTSTR lpszName, DWORD dwTagIndex, LPCTSTR lpszFullName); //Add tag to the list of tags
// 	DWORD _GetTagFromList(CPPString sTagName, CPPString & strFullName, BOOL & bCloseTag); //Get tag from the list
// 
// 	void _SetListSpecChars();
// 	void _AddSpecChar(LPCTSTR lpszAlias, TCHAR tch);
// 	void _AddSpecChar(LPCTSTR lpszAlias, LPCTSTR lpszValue);
// 	void _ReplaceSpecChars();
// 
// 	//The resource's methods
// 	HICON _GetIconFromResources(DWORD dwID, int nWidth = 0, int nHeight = 0) const; //Load an icon from the app resources
// 	HICON _GetIconFromFile(LPCTSTR lpszPath, int nWidth = 0, int nHeight = 0) const; //Load an icon from the file
// 	HICON _GetIconFromDll(DWORD dwID, int nWidth = 0, int nHeight = 0, LPCTSTR lpszPathDll = NULL) const; //Load an icon from the dll resources
// 	HBITMAP _GetBitmapFromResources(DWORD dwID) const; //Load a bitmap from the app resources
// 	HBITMAP _GetBitmapFromFile(LPCTSTR lpszPath) const; //Load a bitmap from the file
// 	HBITMAP _GetBitmapFromDll(DWORD dwID, LPCTSTR lpszPathDll = NULL) const; //Load a bitmap from the dll resources
// 	CPPString _GetStringFromResource(DWORD dwID) const; //Load a string from the app resources
// 	CPPString _GetStringFromDll(DWORD dwID, LPCTSTR lpszPathDll = NULL) const; //Load a string from the dll resources
// 
// 	//The drawing methods
// 	void _DrawHtml(LPSIZE lpSize, LPCRECT lpRect); //Draws the HTML text on device context or gets the size of the output area.
// 	SIZE _DrawHtmlTable(CPPString & sTable, LPCRECT lpRect); //Draws the HTML table on device context or gets the size of the output area.
// //	SIZE DrawHtmlTableRow(CPPString & sRow, LPCRECT lpRect, vecCol & row); //Draws the HTML row of the table
// 	void _DrawHtmlTableRow(CPPString & sRow, LPCRECT lpRect, STRUCT_TABLE & st, int nRow);
// 	void _DrawHtmlTableCell(CPPString & sCell, LPCRECT lpRect, STRUCT_CELL & sc); //Draws the HTML cell of the table
// 	SIZE _DrawHtmlString(CPPString & sHtml, LPCRECT lpRect); //Draws the HTML string on device context or gets the size of the output area.
// 
// 	//The methods
// 	void _SetDefaultCssStyles();
// 	void _SetDefaultCursor();
// 	LPLOGFONT _GetSystemToolTipFont() const; //Gets the system logfont
// 
// 	CPPString _SearchNextTag(CPPString & str, CPPString & strTag, int & nIndex); //Search next tag
// 	BOOL _SearchTag(CPPString & str, int & nIndex, CPPString strTag); //Search begin of the specified tag
// 
// 	CPPString _GetTagBody(CPPString & str, int & nIndex); //Gets a name of tag and the parameters of tag
// 	CPPString _SplitTag(CPPString & sTag); //Split a tag to the tag's name and parameters
// 	CPPString _GetNextProperty(CPPString & str, int & nIndex, CPPString & sProp); //Gets next property
// 
// 
// 	CPPString _SearchPropertyOfTag(CPPString & str, int & nIndex); //Search a name or a property of a tag
// 	SIZE  _AnalyseCellParam(CPPString & sTag, _STRUCT_CHANGESTYLE & cs, BOOL bTable);
// 	void  _AnalyseImageParam(CPPString & strTag, _STRUCT_IMAGE & si);
// 	BOOL  _IsImageWithShadow(_STRUCT_IMAGE & si);
// 
// 	//Functions for hyperlink
// 	int _PtInHyperlink(LPPOINT lpPoint);
// 	void _JumpToHyperlink(int nLink);
// 	void _StoreHyperlinkArea(int left, int top, int right, int bottom);
// 	HINSTANCE _GotoURL(LPCTSTR url, int showcmd = SW_SHOW);
// 	LONG _GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata);
// 
// 	//Functions for notify
// 	void _CallbackOnClickHyperlink(LPCTSTR sLink);
// 	void _CallbackOnRepaint(int nIndexLink);
// 
// 	//Running tag
// 	int  _VerticalAlignText(int y, int nHeight);
// 	int  _VerticalAlignImage(int y, int nHeight);
// 	void _UpdateContext();
// 	BOOL _StoreRestoreStyle(BOOL bRestore);
// 	void _Tag_NewLine(LPPOINT lpPoint, int nNum, LPSIZE lpSize);
// 	void _Tag_Tabulation(LPPOINT lpPoint, int nNum);
// 	int  _InitNewLine(int x);
// 
// 	void _SelectNewHtmlStyle(LPCTSTR lpszNameStyle, STRUCT_CHANGESTYLE & cs);
// 
// 	SIZE  _GetTableDimensions(CPPString & sTable); //Gets dimensions of the table
// 	void  _SearchEndOfTable(CPPString & str, int & nIndex); //Searching end of the table
// 	void  _SearchEndOfRow(CPPString & str, int & nIndex); //Searching end of the row
// 	void  _SearchEndOfCell(CPPString & str, int & nIndex); //Searching end of the cell
// 
// 	//Functions for the map of the styles
// 	void _SetTableOfColors();
// 	void _SetColorName(LPCTSTR lpszColorName, COLORREF color);
// 	COLORREF _GetColorByName(LPCTSTR lpszColorName, COLORREF crDefColor = RGB(0, 0, 0));
// 
// 	BOOL _GetIndexNextAlphaNum(CPPString & str, int & nIndex, BOOL bArithmetic = FALSE);
// 	BOOL _GetBeginParameter(CPPString & str, int & nIndex, TCHAR chSeparator = _T(':'));
// 	TCHAR _GetIndexNextChars(CPPString & str, int & nIndex, CPPString strChars);
// 	TCHAR _GetIndexNextNoChars(CPPString & str, int & nIndex, CPPString strChars);
// 	CPPString _GetParameterString(CPPString & str, int & nIndex, TCHAR chBeginParam = _T(':'), CPPString strSeparators = _T(";"));
// 	CPPString _GetNameOfTag(CPPString & str, int & nIndex);
// 	CPPString _GetWordWrap(CPPString & str, int nMaxSize, int & nRealSize);
// 
// 	//Functions for the map of the styles
// 	LPCTSTR _GetTextStyle(LPCTSTR lpszStyleName);
// 	void _RemoveTextStyle(LPCTSTR lpszStyleName);
// 	void _AddToTextStyle(LPCTSTR lpszStyleName, LPCTSTR lpszAddStyle);
// 	void _UnpackTextStyle(CPPString strStyle, _STRUCT_CHANGESTYLE & cs);
// 
// 	//Functions for analyzing parameters
// 	void _SetDefaultStyles(_STRUCT_CHANGESTYLE & cs);
// 	BOOL _GetStyleFontStyle(CPPString & str, BOOL bDefault);
// 	int  _GetStyleFontWeight(CPPString & str, int nDefault);
// 	int  _GetStyleHorzAlign(CPPString & str, int nDefault);
// 	int  _GetStyleVertAlign(CPPString & str, int nDefault);
// 	COLORREF _GetStyleColor(CPPString & str, COLORREF crDefault);
// 	int  _GetStyleTextTransform(CPPString & str, int nDefault);
// 	CPPString _GetStyleString(CPPString str, CPPString strDefault);
// 	void _GetStyleFontShortForm(CPPString & str);
// 	UINT _GetStyleImageShortForm(CPPString & str);
// 	int _GetStyleBkgndEffect(CPPString & str, int nDefault);
// 	
// 	void _StyleTextDecoration(CPPString & str, _STRUCT_CHANGESTYLE & cs);
// 	int _StyleBorderWidth(CPPString & str, int Default);
// 	int _StyleBorder(CPPString & str, int nDefault);
// 
// 	//Get
// 	int _GetLengthUnit(CPPString & str, int nDefault, BOOL bFont = FALSE);
// 	BOOL _IsPercentableValue(CPPString & str);
// 	int _GetTableWidth(CPPString & str, int nClientWidth, int nMinWidth, BOOL bSet = FALSE);
// 
// 	//Drawing
// 	void _DrawBackgroundImage(HDC hDC, int nDestX, int nDestY, int nWidth, int nHeight, CPPString strNameImage);
// 
// 	int _GetCountOfChars(CPPString str, TCHAR tchar = _T(' ')); //Gets counts of chars
};

#endif //_PPHTMLDRAWER_H_
