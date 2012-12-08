#pragma once


// 链接按钮，hover状态下为红色，显示underline
class CHyperLinkHoverCr : public CHyperLinkImpl<CHyperLinkHoverCr>
{
private:
	COLORREF	m_crHover;
	COLORREF	m_crNormal;

public:
	DECLARE_WND_CLASS(_T("WTL_HyperLink_Cr"))

	void SetNormalColor(const COLORREF& cr)
	{
		m_crNormal = cr;
	}

	void SetHoverColor(const COLORREF& cr)
	{
		m_crHover = cr;
	}

	void DoPaint(CDCHandle dc)
	{
		DWORD		dwExStyle = m_dwExtendedStyle;

		if ( m_bHover )
			m_clrLink = m_crHover;
		else
			m_dwExtendedStyle &= ~HLINK_UNDERLINEHOVER;

        CRect		WndRect;

        GetClientRect(WndRect);
        dc.FillSolidRect(WndRect, RGB(255,255,255));
        

		CHyperLinkImpl<CHyperLinkHoverCr>::DoPaint(dc);

		if ( m_bHover )
			m_clrLink = m_crNormal;
		else
			m_dwExtendedStyle = dwExStyle;
	}
};

#define	SET_HYPERCR_LINK(wnd, id)	\
	wnd.SetHyperLinkExtendedStyle(HLINK_COMMANDBUTTON | HLINK_UNDERLINEHOVER);\
	wnd.SubclassWindow(GetDlgItem(id));\
	wnd.m_clrLink = LINK_TEXT_COLOR;\
	wnd.SetNormalColor(LINK_TEXT_COLOR);\
	wnd.SetHoverColor(RGB(255,0,0));
