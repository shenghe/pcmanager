#include "StdAfx.h"
#include "RichEditCtrlEx.h"


HWND CRichEditCtrlEx::FirstInitialize( HWND hWndParent, UINT nID )
{
	Create(hWndParent, NULL, NULL,
		WS_VISIBLE|WS_CHILD|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY|ES_WANTRETURN|WS_VSCROLL,
		0, nID, NULL);
	SetEventMask(ENM_LINK);
	CHARFORMAT cf = {0};
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask|=CFM_SIZE;
	cf.yHeight =-200; //设置高度
	cf.dwMask|=CFM_FACE;
	_tcscpy(cf.szFaceName ,_T("新宋体")); //设置字体
	SetWordCharFormat(cf);
	
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
	CHARFORMAT2 cf2;
	ZeroMemory(&cf2, sizeof(cf2));
	cf2.cbSize = sizeof(cf2);
	cf2.dwMask |= CFM_LINK;
	cf2.dwEffects|= CFE_LINK;
	SetWordCharFormat( cf2 );
	
	//SetEventMask( ENM_LINK );
	AppendText( szText );
}

void CRichEditCtrlEx::ClearText()
{
	SetReadOnly(FALSE);
	SetSelAll();
	Clear();
	SetReadOnly(TRUE);
}
