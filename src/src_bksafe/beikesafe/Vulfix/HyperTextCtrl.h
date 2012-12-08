#pragma once
#include <wtl/atlcrack.h>
#include <wtl/atlmisc.h>
#include "HyperTextParser.h"

class CHyperTextCtrl : 
	public CWindowImpl<CHyperTextCtrl, CStatic>
{
	typedef CWindowImpl<CHyperTextCtrl, CStatic> _super;
	
public:
	CHyperTextCtrl(void);
	~CHyperTextCtrl(void);
	void SetMText(LPCTSTR szText);
	void SetBackgroudColor(COLORREF rgb);
	void SetFlags(UINT uFlags);
	BOOL GetCurrentLinkID( CPoint point, INT &id );
	void ReleaseMouse();
			
public:
	BEGIN_MSG_MAP_EX(CHyperTextCtrl)   
		MSG_WM_PAINT(OnPaint)
		MSG_WM_SETCURSOR(OnSetCursor)

		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_SIZE(OnSize)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	void OnPaint(CDCHandle dc);
	BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message);
	void OnSize(UINT nType, CSize size);

	void OnMouseMove(UINT nFlags, CPoint point);

protected:
	void _RecalcItemRects();
	INT _GetTTIndex( CPoint point );
	INT _GetHoverLinkIndex();
	INT _GetHoverLinkIndex(CPoint point);
	
protected:
	typedef std::vector<RECT> RECTS;
	CFont m_font, m_fontLink, m_fontBold;
	
	CHyperTextParser m_textparser;
	TextParts m_tt;
	RECTS m_tt_rects;
	RECT m_rc;
	UINT	m_dwFlags;
	COLORREF m_clrBackground;

	//
	BOOL    m_bOverControl;
	BOOL	m_bLinkRectValid;
	INT		m_nLastLink;

	BOOL m_bShowHandle;
};
