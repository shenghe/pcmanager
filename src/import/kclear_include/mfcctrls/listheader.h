
#ifndef CListHeader_H
#define CListHeader_H

// CListHeader window

#define   draw_space             4
#define   draw_checkbox          13
#define   Default_BKG_RGB        RGB(10, 16, 116)
#define   Default_TEXT_RGB       RGB(9,9,9)

class CListHeader :public CHeaderCtrl //public CHeaderCtrl
{
// Construction
public:
	CListHeader();
    virtual ~CListHeader();
// Attributes
public:
    COLORREF m_clrBkGnd ;
    COLORREF m_clrTextColor;
    CString m_strTitle;
    int m_headHight;
// Operations

// Overrides
   //{{AFX_VIRTUAL(CListHeader)
protected:
    virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
    // Generated message map functions
protected:
	//{{AFX_MSG(CListHeader)
    afx_msg LRESULT OnLayOut(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
