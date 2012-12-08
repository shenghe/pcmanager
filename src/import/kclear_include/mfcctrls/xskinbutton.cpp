// CxSkinButton.cpp : implementation file
/** 15/03/2001 v1.00
 * ing.davide.pizzolato@libero.it
 ** 29/03/2001 v1.10
 * Milan.Gardian@LEIBINGER.com
 * - mouse tracking
 ** 02/04/2001 v1.20
 * ing.davide.pizzolato@libero.it
 * - new CreateRgnFromBitmap
 ** 14/04/2001 v1.21
 * - OnMouseLeave cast fixed
 * - Over bitmap consistency check
 ** 25/04/2001 v1.30
 * Fable@aramszu.net
 * - ExtCreateRegion replacement
*
** 04/07/2007 V1.31
* HuangShengSheng@kingsoft.net
* - new SetTextFont
*/

#include "stdafx.h"
#include "xskinbutton.h"
#include "kdraw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CxSkinButton
CxSkinButton::CxSkinButton()
{
	m_DrawMode=1;			// normal drawing mode
	m_FocusRectMargin=0;	// disable focus dotted rect
	hClipRgn=NULL;			// no clipping region
	m_TextColor=GetSysColor(COLOR_BTNTEXT); // default button text color
	m_button_down = m_tracking = false;
	m_hCursor = NULL;


}
/////////////////////////////////////////////////////////////////////////////
CxSkinButton::~CxSkinButton()
{
	if (hClipRgn) DeleteObject(hClipRgn);	// free clip region

	if (m_hCursor != NULL)
		::DestroyCursor(m_hCursor);

	m_bNormal.Detach();
	m_bDown.Detach();
	m_bOver.Detach();
	m_bDisabled.Detach();
	m_bMask.Detach();
}



void CxSkinButton::_Init()
{
	SetTextFont(CFont::FromHandle( KDraw::GetDefaultFont() ));
}

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CxSkinButton, CButton)
	//{{AFX_MSG_MAP(CxSkinButton)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KILLFOCUS()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_SETCURSOR()
	ON_WM_CREATE()
	//ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CxSkinButton message handlers
/////////////////////////////////////////////////////////////////////////////

void CxSkinButton::PreSubclassWindow()
{
	_Init();
}

int CxSkinButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int nRet = CButton::OnCreate( lpCreateStruct );
	_Init();
	return nRet;
}



BOOL CxSkinButton::OnEraseBkgnd(CDC* pDC) 
{
	return 1;	// doesn't erase the button background
}
/////////////////////////////////////////////////////////////////////////////
void CxSkinButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
    ASSERT (lpDrawItemStruct);
    //TRACE("* Captured: %08X\n", ::GetCapture());

    //Check if the button state in not in inconsistent mode...
    POINT mouse_position;
    if ((m_button_down) && (::GetCapture() == m_hWnd) && (::GetCursorPos(&mouse_position))){
		if (::WindowFromPoint(mouse_position) == m_hWnd){
			if ((GetState() & BST_PUSHED) != BST_PUSHED) {
				//TRACE("* Inconsistency up detected! Fixing.\n");
				SetState(TRUE);
				return;
			}
		} else {
			if ((GetState() & BST_PUSHED) == BST_PUSHED) {
				//TRACE("* Inconsistency up detected! Fixing.\n");
				SetState(FALSE);
				return;
			}
		}
	}
	
    //TRACE("* Drawing: %08x\n", lpDrawItemStruct->itemState);
	CString sCaption;

	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);	// get device context
	RECT r=lpDrawItemStruct->rcItem;					// context rectangle
	int cx = r.right  - r.left ;						// get width
	int cy = r.bottom - r.top  ;						// get height
    CRect rcText;
    rcText = r;
	CFont *pOldFont = NULL;

	GetWindowText(sCaption);							// get button text
	pDC->SetBkMode(TRANSPARENT);
	pOldFont = pDC->SelectObject(&m_Font); //get text font

	// Select the correct skin 
	if (lpDrawItemStruct->itemState & ODS_DISABLED)
    {	// DISABLED BUTTON
		if(m_bDisabled.m_hObject==NULL)
			 // no skin selected for disabled state -> standard button
			pDC->FillSolidRect(&r,GetSysColor(COLOR_BTNFACE));
		else // paint the skin
			DrawBitmap(pDC,(HBITMAP)m_bDisabled,r,m_DrawMode);

		// if needed, draw the standard 3D rectangular border
		if (m_Border)
            pDC->DrawEdge(&r,EDGE_RAISED,BF_RECT);
		
        // paint the etched button text
		pDC->SetTextColor(GetSysColor(COLOR_3DHILIGHT));
        rcText.OffsetRect(1, 1);
        pDC->DrawText(sCaption, rcText, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
        rcText.OffsetRect(-1, -1);
        pDC->DrawText(sCaption, rcText, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
    else
    {										// SELECTED (DOWN) BUTTON
		if (lpDrawItemStruct->itemState & ODS_SELECTED)
        {
			if(m_bDown.m_hObject==NULL)
				 // no skin selected for selected state -> standard button
				pDC->FillSolidRect(&r,GetSysColor(COLOR_BTNFACE));
            else { // paint the skin
				DrawBitmap(pDC,(HBITMAP)m_bDown,r,m_DrawMode);
            }
			// if needed, draw the standard 3D rectangular border
			if (m_Border)
                pDC->DrawEdge(&r,EDGE_SUNKEN,BF_RECT);
		}
        else
        {											// DEFAULT BUTTON
			if(m_bNormal.m_hObject == NULL)
				 // no skin selected for normal state -> standard button
				pDC->FillSolidRect(&r, GetSysColor(COLOR_BTNFACE));
			else // paint the skin
                if (m_tracking && (m_bOver.m_hObject != NULL))
                {
					DrawBitmap(pDC,(HBITMAP)m_bOver,r,m_DrawMode);
				}
                else
                {
					DrawBitmap(pDC,(HBITMAP)m_bNormal,r,m_DrawMode);
				}

			// if needed, draw the standard 3D rectangular border
			if (m_Border)
                pDC->DrawEdge(&r, EDGE_RAISED, BF_RECT);
		}
		
//         // paint the focus rect
// 		if ((lpDrawItemStruct->itemState & ODS_FOCUS)&&(m_FocusRectMargin>0))
//         {
// 			r.left   += m_FocusRectMargin ;
// 			r.top    += m_FocusRectMargin ;
// 			r.right  -= m_FocusRectMargin ;
// 			r.bottom -= m_FocusRectMargin ;
// 			DrawFocusRect (lpDrawItemStruct->hDC, &r) ;
// 		}

		// paint the enabled button text
		pDC->SetTextColor(m_TextColor);
        pDC->DrawText(sCaption, rcText, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
	pDC->SelectObject(pOldFont); //restore text font
}
/////////////////////////////////////////////////////////////////////////////
int CxSkinButton::GetBitmapWidth (HBITMAP hBitmap)
{ BITMAP bm; GetObject(hBitmap,sizeof(BITMAP),(PSTR)&bm); return bm.bmWidth;}
/////////////////////////////////////////////////////////////////////////////
int CxSkinButton::GetBitmapHeight (HBITMAP hBitmap)
{ BITMAP bm; GetObject(hBitmap,sizeof(BITMAP),(PSTR)&bm); return bm.bmHeight;}
/////////////////////////////////////////////////////////////////////////////
void CxSkinButton::DrawBitmap(CDC* dc, HBITMAP hbmp, RECT r, int DrawMode)
{
//	DrawMode: 0=Normal; 1=stretch; 2=tiled fill; 3=TRANSPARENT
	if(DrawMode==2){
		FillWithBitmap(dc,hbmp,r);
		return;
	}
	if(!hbmp) return;	//safe check

	if (m_bNormal.GetSafeHandle())
	{
		BITMAP   Bmp;   
		m_bNormal.GetBitmap(&Bmp);
		if (r.bottom - r.top != Bmp.bmHeight)
		{
			r.top += (r.bottom - r.top - Bmp.bmHeight) / 2;
			r.bottom = r.top + Bmp.bmHeight;
			r.left += 3;
		}
	}

	int cx=r.right  - r.left;
	int cy=r.bottom - r.top;
	CDC dcBmp,dcMask;
// 	if (DrawMode == 3)
// 	{
// 		KDraw::DrawTransparentBitmap(dc->GetSafeHdc(), hbmp, r.left, r.top);
// 		return;
// 	}
	if (m_bMask.m_hObject!=NULL){
		dcMask.CreateCompatibleDC(dc);
		dcMask.SelectObject(m_bMask);

		CDC hdcMem;
		hdcMem.CreateCompatibleDC(dc);
	    CBitmap hBitmap;
		hBitmap.CreateCompatibleBitmap(dc,cx,cy);
	    hdcMem.SelectObject(hBitmap);
		
        hdcMem.BitBlt(r.left,r.top,cx,cy,dc,0,0,SRCCOPY);
		int bx=GetBitmapWidth(hbmp);
		int by=GetBitmapHeight(hbmp);
		switch (DrawMode)
		{
		case 0:
	dcBmp.CreateCompatibleDC(dc);
	dcBmp.SelectObject(hbmp);
			hdcMem.BitBlt(r.left,r.top,cx,cy,&dcBmp,0,0,SRCINVERT);
			hdcMem.BitBlt(r.left,r.top,cx,cy,&dcMask,0,0,SRCAND);
			hdcMem.BitBlt(r.left,r.top,cx,cy,&dcBmp,0,0,SRCINVERT);
			break;
		case 1:
		case 2:
	dcBmp.CreateCompatibleDC(dc);
	dcBmp.SelectObject(hbmp);
			hdcMem.StretchBlt(r.left,r.top,cx,cy,&dcBmp,0,0,bx,by,SRCINVERT);
			hdcMem.StretchBlt(r.left,r.top,cx,cy,&dcMask,0,0,bx,by,SRCAND);
			hdcMem.StretchBlt(r.left,r.top,cx,cy,&dcBmp,0,0,bx,by,SRCINVERT);
			break;
		case 3:
			KDraw::DrawTransparentBitmap(hdcMem.GetSafeHdc(), hbmp, r.left, r.top);
			break;
		}
        dc->BitBlt(r.left,r.top,cx,cy,&hdcMem,0,0,SRCCOPY);

		hdcMem.DeleteDC();
		hBitmap.DeleteObject();

		//DeleteDC(dcMask);
		dcMask.DeleteDC();
	}
	else {
		int bx=GetBitmapWidth(hbmp);
		int by=GetBitmapHeight(hbmp);
		switch (DrawMode)
		{
		case 0:
	dcBmp.CreateCompatibleDC(dc);
	dcBmp.SelectObject(hbmp);
			dc->BitBlt(r.left,r.top,cx,cy,&dcBmp,0,0,SRCCOPY);
			break;
		case 1:
		case 2:
	dcBmp.CreateCompatibleDC(dc);
	dcBmp.SelectObject(hbmp);
			dc->StretchBlt(r.left,r.top,cx,cy,&dcBmp,0,0,bx,by,SRCCOPY);
			break;
		case 3:
			KDraw::DrawTransparentBitmap(dc->GetSafeHdc(), hbmp, r.left, r.top);
			break;
		}
	}
	//DeleteDC(dcBmp);
	dcBmp.DeleteDC();
}
/////////////////////////////////////////////////////////////////////////////
void CxSkinButton::FillWithBitmap(CDC* dc, HBITMAP hbmp, RECT r)
{
	if(!hbmp) return;
	CDC memdc;
	memdc.CreateCompatibleDC(dc);
	memdc.SelectObject(hbmp);
	int w = r.right - r.left;
	int	h = r.bottom - r.top;
	int x,y,z;
	int	bx=GetBitmapWidth(hbmp);
	int	by=GetBitmapHeight(hbmp);
	for (y = r.top ; y < h ; y += by){
		if ((y+by)>h) by=h-y;
		z=bx;
		for (x = r.left ; x < w ; x += z){
			if ((x+z)>w) z=w-x;
			dc->BitBlt(x, y, z, by, &memdc, 0, 0, SRCCOPY);
		}
	}
	DeleteDC(memdc);
}
/////////////////////////////////////////////////////////////////////////////
void CxSkinButton::SetSkin(
    UINT normal,UINT down,UINT over,UINT disabled,UINT mask,
    short drawmode, short border, short margin, BOOL bFitBmpSize
    )
{
    SetSkin(
        KDraw::LoadBitmap(normal), 
        KDraw::LoadBitmap(down), 
        KDraw::LoadBitmap(over), 
        KDraw::LoadBitmap(disabled), 
        KDraw::LoadBitmap(mask), 
        drawmode, border, margin, bFitBmpSize
        );
}

void CxSkinButton::SetSkin(
    HBITMAP hNormal, HBITMAP hDown, HBITMAP hOver, HBITMAP hDisabled, HBITMAP hMask,
    short drawmode,short border,short margin,BOOL bFitBmpSize
    )
{
	m_bNormal.Detach();
	m_bDown.Detach();
	m_bOver.Detach();
	m_bDisabled.Detach();
	m_bMask.Detach();

    if (hNormal > 0)
        m_bNormal.Attach(hNormal);
	if (hDown > 0)
        m_bDown.Attach(hDown);
	if (hOver > 0)
        m_bOver.Attach(hOver);

	if (hDisabled > 0)
        m_bDisabled.Attach(hDisabled);
	else if (hNormal > 0)
        m_bDisabled.Attach(hNormal);

	m_DrawMode = max(0, min(drawmode, 3));
	m_Border = border;
	m_FocusRectMargin = max(0, margin);
	
	//add by seawind
	if (bFitBmpSize && (HBITMAP)m_bNormal != NULL)
	{
		SetWindowPos(NULL, 0, 0, GetBitmapWidth(m_bNormal), GetBitmapHeight(m_bNormal), SWP_NOZORDER | SWP_NOMOVE);
	}

	if (hMask > 0)
    {
		m_bMask.Attach(hMask);
		if (hClipRgn)
            DeleteObject(hClipRgn);
		hClipRgn = CreateRgnFromBitmap(m_bMask, RGB(255, 0, 255));

		if (hClipRgn)
        {
			SetWindowRgn(hClipRgn, TRUE);
			SelectClipRgn((HDC)GetDC(),hClipRgn);
		}

		if (m_DrawMode == 0)
        {
			SetWindowPos(
                NULL, 0, 0, GetBitmapWidth(m_bMask),
				GetBitmapHeight(m_bMask), SWP_NOZORDER | SWP_NOMOVE
                );
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
HRGN CxSkinButton::CreateRgnFromBitmap(HBITMAP hBmp, COLORREF color)
{
	if (!hBmp) return NULL;

	BITMAP bm;
	GetObject( hBmp, sizeof(BITMAP), &bm );	// get bitmap attributes

	CDC dcBmp;
	dcBmp.CreateCompatibleDC(GetDC());	//Creates a memory device context for the bitmap
	dcBmp.SelectObject(hBmp);			//selects the bitmap in the device context

	const DWORD RDHDR = sizeof(RGNDATAHEADER);
	const DWORD MAXBUF = 40;		// size of one block in RECTs
									// (i.e. MAXBUF*sizeof(RECT) in bytes)
	LPRECT	pRects;								
	DWORD	cBlocks = 0;			// number of allocated blocks

	INT		i, j;					// current position in mask image
	INT		first = 0;				// left position of current scan line
									// where mask was found
	bool	wasfirst = false;		// set when if mask was found in current scan line
	bool	ismask;					// set when current color is mask color

	// allocate memory for region data
	RGNDATAHEADER* pRgnData = (RGNDATAHEADER*)new BYTE[ RDHDR + ++cBlocks * MAXBUF * sizeof(RECT) ];
	memset( pRgnData, 0, RDHDR + cBlocks * MAXBUF * sizeof(RECT) );
	// fill it by default
	pRgnData->dwSize	= RDHDR;
	pRgnData->iType		= RDH_RECTANGLES;
	pRgnData->nCount	= 0;
	for ( i = 0; i < bm.bmHeight; i++ )
	for ( j = 0; j < bm.bmWidth; j++ ){
		// get color
		ismask=(dcBmp.GetPixel(j,bm.bmHeight-i-1)!=color);
		// place part of scan line as RECT region if transparent color found after mask color or
		// mask color found at the end of mask image
		if ( wasfirst && (ismask ^ (j < bm.bmWidth - 1)) ){
			// get offset to RECT array if RGNDATA buffer
			pRects = (LPRECT)((LPBYTE)pRgnData + RDHDR);
			// save current RECT
			pRects[ pRgnData->nCount++ ] = CRect( first, bm.bmHeight - i - 1, j, bm.bmHeight - i );
			// if buffer full reallocate it
			if ( pRgnData->nCount >= cBlocks * MAXBUF ){
				LPBYTE pRgnDataNew = new BYTE[ RDHDR + ++cBlocks * MAXBUF * sizeof(RECT) ];
				memcpy( pRgnDataNew, pRgnData, RDHDR + (cBlocks - 1) * MAXBUF * sizeof(RECT) );
				delete pRgnData;
				pRgnData = (RGNDATAHEADER*)pRgnDataNew;
			}
			wasfirst = false;
		} else if ( !wasfirst && ismask ){		// set wasfirst when mask is found
			first = j;
			wasfirst = true;
		}
	}
	DeleteObject(dcBmp);	//release the bitmap
	// create region
/*  Under WinNT the ExtCreateRegion returns NULL (by Fable@aramszu.net) */
//	HRGN hRgn = ExtCreateRegion( NULL, RDHDR + pRgnData->nCount * sizeof(RECT), (LPRGNDATA)pRgnData );
/* ExtCreateRegion replacement { */
	HRGN hRgn=CreateRectRgn(0, 0, 0, 0);
	ASSERT( hRgn!=NULL );
	pRects = (LPRECT)((LPBYTE)pRgnData + RDHDR);
	for(i=0;i<(int)pRgnData->nCount;i++)
	{
		HRGN hr=CreateRectRgn(pRects[i].left, pRects[i].top, pRects[i].right, pRects[i].bottom);
		VERIFY(CombineRgn(hRgn, hRgn, hr, RGN_OR)!=ERROR);
	}
	ASSERT( hRgn!=NULL );
/* } ExtCreateRegion replacement */

	delete pRgnData;
	return hRgn;
}
/////////////////////////////////////////////////////////////////////////////
COLORREF CxSkinButton::SetTextColor(COLORREF new_color)
{
	COLORREF tmp_color=m_TextColor;
	m_TextColor=new_color;
	return tmp_color;			//returns the previous color
}
//////////////////////////////////////////////////////////////////////////
void CxSkinButton::SetTextFont(CFont *pNewFont)
{
	LOGFONT font = {0};

	ASSERT(pNewFont);
	pNewFont->GetLogFont(&font);

	m_Font.DeleteObject();

	m_Font.CreateFontIndirect(&font);
}
/////////////////////////////////////////////////////////////////////////////
void CxSkinButton::SetToolTipText(CString s)
{
	if(m_tooltip.m_hWnd==NULL){
		if(m_tooltip.Create(this))	//first assignment
			if(m_tooltip.AddTool(this, (LPCTSTR)s))
				m_tooltip.Activate(1);
	} else {
		m_tooltip.UpdateTipText((LPCTSTR)s,this);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CxSkinButton::RelayEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
// This function will create a MSG structure, fill it in a pass it to
// the ToolTip control, m_ttip.  Note that we ensure the point is in window
// coordinates (relative to the control's window).
	if(NULL != m_tooltip.m_hWnd){
		MSG msg;
		msg.hwnd = m_hWnd;
		msg.message = message;
		msg.wParam = wParam;
		msg.lParam = lParam;
		msg.time = 0;
		msg.pt.x = LOWORD(lParam);
		msg.pt.y = HIWORD(lParam);

		m_tooltip.RelayEvent(&msg);
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//Method......: OnLButtonDblClk
//Class.......: CxSkinButton
//
//Author......: Milan Gardian
//Created.....: MAR-2001
//
//Return value: NONE
//Parameters..: Used only to be forwarded as WM_LBUTTONDOWN message parameters
//Exceptions..: NONE
//------------
//Description :
//
//  > We do not care about doublelicks - handle this event
//    like an ordinary left-button-down event
//
//---------------------------------------------------------
void CxSkinButton::OnLButtonDblClk(UINT flags, CPoint point) 
{
    SendMessage(WM_LBUTTONDOWN, flags, MAKELPARAM(point.x, point.y));
}


/////////////////////////////////////////////////////////////////////////////
//
//Method......: OnLButtonDown
//Class.......: CxSkinButton
//
//Author......: Milan Gardian
//Created.....: MAR-2001
//
//Return value: NONE
//Parameters..: As follows
//    > [in] nFlags: not used
//    > [in] point: coordinates of the mouse pointer when this event was spawned
//Exceptions..: NONE
//------------
//Description :
//
//  > Handle event when left button is pressed down
//
//---------------------------------------------------------
void CxSkinButton::OnLButtonDown(UINT nFlags, CPoint point)
{
    //TRACE("* %08X: down\n", ::GetTickCount());

	//Pass this message to the ToolTip control
	RelayEvent(WM_LBUTTONDOWN,(WPARAM)nFlags,MAKELPARAM(LOWORD(point.x),LOWORD(point.y)));

    //If we are tracking this button, cancel it
    if (m_tracking) {
        TRACKMOUSEEVENT t = {
            sizeof(TRACKMOUSEEVENT),
            TME_CANCEL | TME_LEAVE,
            m_hWnd,
            0
        };
        if (::_TrackMouseEvent(&t)) {
            m_tracking = false;
        }
    }

    //Default-process the message
    m_button_down = true;
	CButton::OnLButtonDown(nFlags, point);
}


/////////////////////////////////////////////////////////////////////////////
//
//Method......: OnLButtonUp
//Class.......: CxSkinButton
//
//Author......: Milan Gardian
//Created.....: MAR-2001
//
//Return value: NONE
//Parameters..: As follows
//    > [in] nFlags: not used
//    > [in] point: coordinates of the mouse pointer when this event was spawned
//Exceptions..: NONE
//------------
//Description :
//
//  > Handle event when left button is released (goes up)
//
//---------------------------------------------------------
void CxSkinButton::OnLButtonUp(UINT nFlags, CPoint point)
{
    //TRACE("* %08X: up\n", ::GetTickCount());

	//Pass this message to the ToolTip control
	RelayEvent(WM_LBUTTONUP,(WPARAM)nFlags,MAKELPARAM(LOWORD(point.x),LOWORD(point.y)));

    //Default-process the message
    m_button_down = false;
	CButton::OnLButtonUp(nFlags, point);
}


/////////////////////////////////////////////////////////////////////////////
//
//Method......: OnMouseMove
//Class.......: CxSkinButton
//
//Author......: Milan Gardian
//Created.....: MAR-2001
//
//Return value: NONE
//Parameters..: As follows
//    > [in] nFlags: not used
//    > [in] point: coordinates of the mouse pointer when this event was spawned
//Exceptions..: NONE
//------------
//Description :
//
//  > Handle change of mouse position: see the comments in the
//    method for further info.
//
//---------------------------------------------------------
void CxSkinButton::OnMouseMove(UINT nFlags, CPoint point)
{
    //TRACE("* %08X: Mouse\n", ::GetTickCount());

	//Pass this message to the ToolTip control
	RelayEvent(WM_MOUSEMOVE,(WPARAM)nFlags,MAKELPARAM(LOWORD(point.x),LOWORD(point.y)));

    //If we are in capture mode, button has been pressed down
    //recently and not yet released - therefore check is we are
    //actually over the button or somewhere else. If the mouse
    //position changed considerably (e.g. we moved mouse pointer
    //from the button to some other place outside button area)
    //force the control to redraw
    //
    if ((m_button_down) && (::GetCapture() == m_hWnd)) {
	    POINT p2 = point;
        ::ClientToScreen(m_hWnd, &p2);
        HWND mouse_wnd = ::WindowFromPoint(p2);

        bool pressed = ((GetState() & BST_PUSHED) == BST_PUSHED);
        bool need_pressed = (mouse_wnd == m_hWnd);
        if (pressed != need_pressed) {
            //TRACE("* %08X Redraw\n", GetTickCount());
            SetState(need_pressed ? TRUE : FALSE);
            Invalidate();
        }
    } else {

	//Otherwise the button is released. That means we should
    //know when we leave its area - and so if we are not tracking
    //this mouse leave event yet, start now!
    //
        if (!m_tracking) {
            TRACKMOUSEEVENT t = {
                sizeof(TRACKMOUSEEVENT),
                TME_LEAVE,
                m_hWnd,
                0
            };
            if (::_TrackMouseEvent(&t)) {
                //TRACE("* Mouse enter\n");
                m_tracking = true;
                Invalidate();
            }
        }
    }

    //Forward this event to superclass
    CButton::OnMouseMove(nFlags, point);
}


/////////////////////////////////////////////////////////////////////////////
//
//Method......: OnMouseLeave
//Class.......: CxSkinButton
//
//Author......: Milan Gardian
//Created.....: MAR-2001
//
//Return value: NULL
//Parameters..: NOT USED
//Exceptions..: NONE
//------------
//Description :
//
//  > Handle situation when mouse cursor leaves area of this
//    window (button). This event might be generated ONLY
//    if we explicitely call 'TrackMouseEvent'. This is
//    signalled by setting the m_tracking flag (see the assert
//    precondition) - in 'OnMouseMove' method
//
//  > When a mouse pointer leaves area of this button (i.e.
//    when this method is invoked), presumably the look of
//    the button changes (e.g. when hover/non-hover images are set)
//    and therefore we force the control to redraw.
//
//---------------------------------------------------------
LRESULT CxSkinButton::OnMouseLeave(WPARAM, LPARAM)
{
    ASSERT (m_tracking);
    //TRACE("* Mouse leave\n");
    m_tracking = false;
    Invalidate();
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
//Method......: OnKillFocus
//Class.......: CxSkinButton
//
//Author......: Milan Gardian
//Created.....: MAR-2001
//
//Return value: NONE
//Parameters..: See superclass documentation
//Exceptions..: NONE
//------------
//Description :
//
//  > If focus is killed during capture, we may no longer
//    have the exclusive access to user input and therefore
//    release it.
//
//  > Such a situation might happens when the user left-clicks
//    this button, keeps the button down and simultaneously
//    presses TAB key.
//
//---------------------------------------------------------
void CxSkinButton::OnKillFocus(CWnd *new_wnd)
{
    if (::GetCapture() == m_hWnd) {
        ::ReleaseCapture();
        ASSERT (!m_tracking);
        m_button_down = false;
    }
    CButton::OnKillFocus(new_wnd);
}


/////////////////////////////////////////////////////////////////////////////
//
//Method......: OnClicked
//Class.......: CxSkinButton
//
//Author......: Milan Gardian
//Created.....: MAR-2001
//
//Return value: FALSE (do not stop in this handler - forward to parent)
//Parameters..: NONE
//Exceptions..: NONE
//------------
//Description :
//
//  > Keep consistency of attributes of this instance before
//    submitting click event to the parent.
//
//  > Currently NOT used. To use, umcomment line
//    "ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)" in message map
//    at the beginning of this file.
//
//---------------------------------------------------------
BOOL CxSkinButton::OnClicked() 
{
    if (::GetCapture() == m_hWnd) {
        ::ReleaseCapture();
        ASSERT (!m_tracking);
    }
    m_button_down = false;
    //Invalidate();
    return FALSE;
}


//EOF

void CxSkinButton::SetToolTipText(UINT nID)
{
	CString s;

	s.LoadString(nID);

	SetToolTipText(s);
}

BOOL CxSkinButton::SetBtnCursor(int nCursorId)
{
	HINSTANCE hInstResource;
	// Destroy any previous cursor
	if (m_hCursor != NULL) ::DestroyCursor(m_hCursor);
	m_hCursor = NULL;

	// If we want a cursor
	if (nCursorId != -1)
	{
		hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nCursorId),
											RT_GROUP_CURSOR);
		// Load icon resource
		m_hCursor = (HCURSOR)::LoadImage(hInstResource/*AfxGetApp()->m_hInstance*/, MAKEINTRESOURCE(nCursorId), IMAGE_CURSOR, 0, 0, 0);
		// If something wrong then return FALSE
		if (m_hCursor == NULL) return FALSE;
	}

	return TRUE;
}

BOOL CxSkinButton::SetHandCursor()
{
	m_hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND));
	return TRUE;
}

BOOL CxSkinButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// If a cursor was specified then use it!
	if (m_hCursor != NULL)
	{
		::SetCursor(m_hCursor);
		return TRUE;
	}

	return CButton::OnSetCursor(pWnd, nHitTest, message);
}