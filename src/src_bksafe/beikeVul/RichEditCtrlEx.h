#pragma once

class CRichEditCtrlEx : public CRichEditCtrl
{
public:
	HWND FirstInitialize(HWND hWndParent, UINT nID);
	
	void SetWordWrap(BOOL bWrap);
	UINT GetWordWrapMode() const
	{ ATLASSERT(::IsWindow(m_hWnd)); return (UINT) ::SendMessage(m_hWnd, EM_GETWORDWRAPMODE, 0, 0); }

	UINT SetWordWrapMode(UINT uFlags) const
	{ ATLASSERT(::IsWindow(m_hWnd)); return (UINT) ::SendMessage(m_hWnd, EM_SETWORDWRAPMODE, (WPARAM) uFlags, 0); }


	void AddText(LPCTSTR szText, BOOL bBold=FALSE, COLORREF *lpColor=NULL);
	void AddNewLine();
	void AddLink(LPCTSTR szText);
	void ClearText();
};
