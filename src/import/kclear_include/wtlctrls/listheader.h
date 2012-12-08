
#ifndef CListHeader_H
#define CListHeader_H

// CListHeader window

#define   draw_space             4
#define   draw_checkbox          13
#define   Default_BKG_RGB        RGB(10, 16, 116)
#define   Default_TEXT_RGB       RGB(9,9,9)

class CListHeader :public CWindowImpl<CListHeader,CHeaderCtrl> //public CHeaderCtrl
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
    BEGIN_MSG_MAP(CListHeader)
        MSG_WM_PAINT(OnPaint)
        MESSAGE_HANDLER(HDM_LAYOUT, OnLayOut)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEACTIVATE(OnMouseActivate)
    END_MSG_MAP()

protected:

    LRESULT OnLayOut(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnPaint(HDC hDC);
	void OnMouseMove(UINT nFlags, CPoint point);
	int OnMouseActivate(HWND pDesktopWnd, UINT nHitTest, UINT message);

};

#endif
