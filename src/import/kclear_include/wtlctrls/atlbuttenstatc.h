/************************************************************************
* @file      : kppskin/ATLButtenStatc.h
* @author    : ChenZhiQiang <chenzhiqiang@kingsoft.com>
* @date      : 2010/3/22 21:20:44
* @brief     : 
*
* $Id: $
/************************************************************************/
#ifndef __KPPSKIN_ATLBUTTENSTATC_H__
#define __KPPSKIN_ATLBUTTENSTATC_H__

// -------------------------------------------------------------------------

class CStaticBtn:	public CWindowImpl<CStaticBtn, CStatic>

{
    // Construction
public:
    static enum FlashType {None, Text, Background };
    static enum Type3D { Raised, Sunken};
    static enum BackFillMode { Normal, Gradient };

    BEGIN_MSG_MAP(CStaticBtn)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSysColorChange)
        MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    END_MSG_MAP()

    // =============================================================================================
    // Function name	: CStaticBtn::CStaticBtn
    // Description	    : Default contructor
    // Return type		: 
    // =============================================================================================
    CStaticBtn::CStaticBtn() :	m_bPaintLabel(true), m_hBackBrush(NULL), m_crHiColor(0), m_crLoColor(0),
    m_bTimer(FALSE), m_bState(FALSE), m_bTransparent(FALSE), m_bLink(TRUE),
    m_hCursor(NULL), m_Type(None), m_bFont3d(FALSE), m_bNotifyParent(FALSE),
    m_bToolTips(FALSE), m_bRotation(FALSE), m_fillmode(Normal), m_hFont(NULL)
    {
        m_crText = GetSysColor(COLOR_WINDOWTEXT);
        m_cr3DHiliteColor =	RGB(255,255,255);		
        m_hwndBrush = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));
        m_hBitmap = 0;
        m_nIndex = 0;
        m_nHeight = 0;
        m_nWidth = 0;
        m_nCount = 0;
    }

    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
    {
        return 1L;
    }

    // =============================================================================================
    // Function name	: ~CStaticBtn
    // Description	    : Destructor
    // Return type		: virtual 
    // =============================================================================================
    virtual ~CStaticBtn()
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
    virtual CStaticBtn& SetBkColor(COLORREF crBkgnd, COLORREF crBkgndHigh = 0, BackFillMode mode = Normal)
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
    virtual CStaticBtn& SetTextColor(COLORREF crText)
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
    virtual CStaticBtn& SetText(const ATL::CString& strText)
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
    virtual CStaticBtn& SetFontBold(BOOL bBold)
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
    virtual CStaticBtn& SetFontName(const ATL::CString& strFont, BYTE byCharSet = ANSI_CHARSET)
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
    virtual CStaticBtn& SetFontUnderline(BOOL bSet)
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
    virtual CStaticBtn& SetFontItalic(BOOL bSet)
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
    virtual CStaticBtn& SetFontSize(int nSize)
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
    virtual CStaticBtn& SetSunken(BOOL bSet)
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
    virtual CStaticBtn& SetBorder(BOOL bSet)
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
    virtual CStaticBtn& SetTransparent(BOOL bSet)
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
    virtual CStaticBtn& FlashText(BOOL bActivate)
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
    virtual CStaticBtn& FlashBackground(BOOL bActivate)
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
    virtual CStaticBtn& SetLink(BOOL bLink,BOOL bNotifyParent)
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
    virtual CStaticBtn& SetLinkCursor()
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
    virtual CStaticBtn& SetFont3D(BOOL bSet,Type3D type=Raised)
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
    virtual CStaticBtn& SetRotationAngle(UINT nAngle,BOOL bRotation)
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
    virtual CStaticBtn& SetText3DHiliteColor(COLORREF cr3DHiliteColor)
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
        BOOL bRet = CWindowImpl<CStaticBtn, CStatic>::SubclassWindow(hWnd);
        if(bRet)
            Init();
        return bRet;
    }

    void DrawBitmap(HDC hdc, int nIndex)
    {
        int nBmpPosX = 0;
        HDC hdcMem = NULL;

        if (!(HBITMAP)m_hBitmap)
        {
            return;
        }

        if (m_nCount)
        {
            nBmpPosX = nIndex * m_nWidth;
        }

        hdcMem = CreateCompatibleDC(hdc);
        SelectObject(hdcMem, m_hBitmap);

        //文字的前面
        BitBlt(hdc, 0, 0, m_nWidth, m_nHeight, hdcMem, nBmpPosX, 0, SRCCOPY);

        DeleteDC(hdcMem);
    }

    void SetBitmap(HBITMAP hBitmap)
    {
        m_hBitmap = hBitmap;
        GetStaticSize();
    }

    void SetBitmap(HBITMAP hBitmap, int nCount, int nIndex)
    {
        m_hBitmap = hBitmap;
        m_nCount = nCount;
        m_nIndex = nIndex;
        GetStaticSize();
        InvalidateRect(NULL);
    }

    void GetStaticSize()
    {
        BITMAP bm;

        if (!m_hBitmap)
            return;

        ::GetObject(m_hBitmap, sizeof(BITMAP), (PSTR)&bm); 

        if (m_nCount)
        {
            m_nWidth = bm.bmWidth / m_nCount;
        }
        m_nHeight = bm.bmHeight;
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
    HBITMAP         m_hBitmap;
    int             m_nIndex;

    int m_nWidth;
    int m_nHeight;
    int m_nCount;

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
        if (m_hBitmap)
        {
            DrawBitmap(dc, m_nIndex);
        }

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
            pDCMem->TextOut (m_nWidth, 0, strText) ;
            SetViewportOrgEx (pDCMem->m_hDC,pt.x / 2, pt.y / 2, NULL) ;
            pDCMem->SetTextAlign (nAlign);
        }
        else
        {
            rc.left += m_nWidth;    //留出前面位图        

            pDCMem->DrawText(cValue, -1, rc,dwFlags);
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
            dc.BitBlt(0,0,rc.Width(),rc.Height(), pDCMem->m_hDC,0,0,SRCCOPY);
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

// -------------------------------------------------------------------------
// $Log: $

#endif /* __KPPSKIN_ATLBUTTENSTATC_H__ */
