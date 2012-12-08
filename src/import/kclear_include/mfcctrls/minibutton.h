#if !defined(AFX_MINIBUTTON_H__29F06BA2_B3C0_4467_958F_72028126E913__INCLUDED_)
#define AFX_MINIBUTTON_H__29F06BA2_B3C0_4467_958F_72028126E913__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MiniButton.h : header file
//

#include "xskinbutton.h"

/////////////////////////////////////////////////////////////////////////////
// CMiniButton window

class CMiniButton : public CxSkinButton
{
// Construction
public:
	CMiniButton();
	virtual ~CMiniButton();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMiniButton)
	public:
	virtual BOOL Create(LPCTSTR lpszWindowName, const RECT& rect, CWnd* pParentWnd, UINT nID);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:

    void _CreateMiniSkin();

    static HBITMAP ms_hbmpNormal;
    static HBITMAP ms_hbmpHover;
    static HBITMAP ms_hbmpDown;
    static HBITMAP ms_hbmpDisable;
    static CSize ms_sizeMiniBtn;

	//{{AFX_MSG(CMiniButton)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MINIBUTTON_H__29F06BA2_B3C0_4467_958F_72028126E913__INCLUDED_)
