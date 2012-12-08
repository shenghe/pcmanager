#include "StdAfx.h"
#include "RichEditCtrlEx.h"

#define COLOR_LIST_LINK				RGB(40,100,165)	


HWND CRichEditCtrlEx::FirstInitialize( HWND hWndParent, UINT nID )
{
	Create(hWndParent, NULL, NULL,
		WS_VISIBLE|WS_CHILD|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY|ES_WANTRETURN|WS_VSCROLL,
		0, nID, NULL);
	SetEventMask(ENM_LINK);
// 	CHARFORMAT cf = {0};
// 	cf.cbSize = sizeof(CHARFORMAT);
// 	cf.dwMask|=CFM_SIZE;
// 	cf.yHeight =-200; //…Ë÷√∏ﬂ∂»
// 	cf.dwMask|=CFM_FACE;
// 	SetWordCharFormat(cf);

	PARAFORMAT2 pf2;
	pf2.cbSize = sizeof(PARAFORMAT2);
	pf2.dwMask = PFM_LINESPACING;
	pf2.dyLineSpacing = 255;
	pf2.bLineSpacingRule  = 4;
	SetParaFormat(pf2);


    SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
	
	SetWordWrap(TRUE);
	return m_hWnd;
}

void CRichEditCtrlEx::SetWordWrap( BOOL bWrap )
{
#if 0 
	RECT r; 
	GetWindowRect(&r); 
	HDC hDC = GetDC();
	long lLineWidth = 9999999; // This is the non-wrap width 
	if (bWrap) 
	{ 
		lLineWidth = ::MulDiv( GetDeviceCaps(hDC,PHYSICALWIDTH), 
			1440, GetDeviceCaps(hDC, LOGPIXELSX)); 
	} 
	SetTargetDevice(GetDC(), lLineWidth); 
#else
	SetTargetDevice(NULL, !bWrap);
#endif
}

void CRichEditCtrlEx::AddText( LPCTSTR szText, BOOL bBold/*=FALSE*/, COLORREF *lpColor/*=NULL*/ )
{
	CHARFORMAT cf = {0};
	cf.cbSize = sizeof(cf);
	cf.dwMask |= CFM_BOLD;
	cf.dwEffects |= bBold ? CFE_BOLD : 0;	
	cf.dwMask |= CFM_COLOR;
	cf.crTextColor = lpColor ? *lpColor : CFE_AUTOCOLOR;
	SetWordCharFormat( cf );	
	AppendText( szText );
}

void CRichEditCtrlEx::AddNewLine()
{
	CHARFORMAT cf = {0};
	cf.cbSize = sizeof(cf);
	SetWordCharFormat( cf );	
	AppendText( _T("\r\n") );
}


void CRichEditCtrlEx::AddLink( LPCTSTR szText )
{
	/*
	CHARFORMAT2 cf2;
    ZeroMemory(&cf2, sizeof(cf2));
	cf2.cbSize = sizeof(cf2);
	cf2.dwMask |= CFM_LINK|CFM_COLOR;
	cf2.dwEffects|= CFE_LINK;   
	cf2.crTextColor = COLOR_LIST_LINK;
	SetWordCharFormat( cf2 );
	*/

	CHARFORMAT cf = {0};
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = COLOR_LIST_LINK;
	SetWordCharFormat( cf );

	CHARFORMAT2	cf2;
	ZeroMemory(&cf2, sizeof(cf2));
	cf2.dwMask = CFM_LINK;   
	cf2.dwEffects |= CFE_LINK;   	
	SetWordCharFormat(cf2);

	AppendText( szText );
}

void CRichEditCtrlEx::ClearText()
{
	SetReadOnly(FALSE);
	SetSelAll();
	Clear();
	SetReadOnly(TRUE);
}

void CRichEditCtrlEx::EndLink()
{
	CHARFORMAT2	cf2;
	ZeroMemory(&cf2, sizeof(cf2));
	cf2.dwMask = CFM_LINK;   
	cf2.dwEffects = 0;
	SetWordCharFormat(cf2);
}