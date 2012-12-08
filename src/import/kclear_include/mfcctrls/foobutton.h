//  FooButton.h
//  A versatile bitmap button
//  Copyright (c) 1997-2005 Ravi Bhavnani (ravib@ravib.com)
//
//  You are allowed to include the source code in any product (commercial, shareware,
//  freeware or otherwise) when your product is released in binary form.  You are allowed
//  to modify the source code in any way you want except you cannot modify the copyright
//  details at the top of each module.  If you want to distribute source code with your
//  application, then you are only allowed to distribute versions released by the author.
//  This is to maintain a single distribution point for the source code. 

#ifndef FooButton_h
#define FooButton_h

/**
 *  A versatile bitmap button.
 */

typedef UINT (CALLBACK* MSIMG32callBack)(HDC,CONST PTRIVERTEX,DWORD,CONST PVOID,DWORD,DWORD);

class FooButton : public CButton
{
/////////////////////////////////////////////////////////////////////////////
// Construction
public:
  //! Standard constructor.
	FooButton();

  //! Standard destructor.
  virtual ~FooButton();

/////////////////////////////////////////////////////////////////////////////
// Attributes
public:
  //! Button types
  static enum Type {
    //! Static button
    staticButton = 0,
    //! Standard pushbutton
    pushButton,
    //! Hot tracked pushbutton
    hotPushButton,
    //! Standard pushbutton with dropdown
    pushButtonDropDown,
    //! Hot tracked pushbutton with dropdown
    hotPushButtonDropDown,
    //! Multi pushbutton with dropdown
    pushButtonMulti,
    //! Hot tracked multi pushbutton with dropdown
    hotPushButtonMulti,
    //! Standard check button
    checkButton,
    //! Hot tracked check button
    hotCheckButton,
    //! Standard checkbox
    checkBox,
    //! Standard radio button
    radio,
    //! Hyperlink
    hyperlink,
  };

  //! Text styles
  static enum Text {
    //! No text displayed
    none = 0,
    //! Single-line left-justified text
    singleLine,
    //! Single-line centered text
    singleLineCenter,
    //! Multi-line left-justified text
    multiLine,
  };

  //! Focus styles
  static enum Focus {
    //! No focus rectangle displayed
    noFocus = 0,
    //! Show focus rectangle
    normalFocus,
    //! Show focus rectangle and defualt button indicator
    defaultFocus,
  };

/////////////////////////////////////////////////////////////////////////////
// Operations
public:
  //! Gets the button's type.
  //! @return   The button's type.
  FooButton::Type getType()
    { return m_type; }

  //! Sets the button's type.
  //! @param  type    The button's type.
  void setType
    (FooButton::Type type);

  //! Gets the button's text style.
  //! @return   The button's text style.
  FooButton::Text getTextStyle()
    { return m_textStyle; }

  //! Sets the button's text style.
  //! @param  textStyle   The button's style.
  void setTextStyle
    (FooButton::Text textStyle);

  //! Gets the button's text color.
  //! @return   The button's text color.
  COLORREF getTextColor()
    { return m_rgbText; }

  //! Sets the button's text color.
  //! @param  rgbText   The button's text color.
  void setTextColor
    (COLORREF rgbText)
    { m_rgbText = rgbText; }

  //! Gets the button's focus style.
  //! @return   The button's focus style.
  FooButton::Focus getFocusStyle()
    { return m_focusStyle; }

  //! Sets the button's focus style.
  //! @param  focusStyle   The button's focus style.
  void setFocusStyle
    (FooButton::Focus focusStyle);

  //! Gets the button's bitmap id.
  //! @return   The button's bitmap id.
  int getBitmapId()
    { return m_nBitmapId; }

  //! Gets the button's gradient property.
  //! @return   The button's gradient property.
  bool isGradient()
    { return m_bGradient; }

  //! Sets the button's gradient property.
  //! @param  bGradient   Flag: draw gradient background.
  void setGradient
    (bool bGradient)
    { m_bGradient = bGradient; }

  //! Sets the button's bitmap id.
  //! @param  nBitmapId       Bitmap id (-1 means none).
  //! @param  rgbTransparent  Bitmap's transparency color (default = RGB(255,0,255)
  void setBitmapId
    (int nBitmapId,
     COLORREF rgbTransparent = RGB (255, 0, 255));

  //! Gets the button's checked state.
  //! @return   The button's checked state.
  bool isChecked()
    { return m_bChecked; }

  //! Sets the button's checked state if the button type is checkButton or
  //! hotCheckButton.
  //! @param    bChecked    Flag: button is checked.
  void check
    (bool bChecked);

  //! Checks if the multi pushbutton's drop-down was clicked.
  //! @return   true if the button's drop-down was clicked, false otherwise.
  bool isMultiClicked()
    { return m_bMultiClicked; }

  //! Sets a multi pushbutton to its normal unpressed state.
  void clearMultiClick();

  //! Displays a popup menu if the button type is pushButton, pushButtonDropDown,
  //! hotPushButton or hotPushButtonDropDown.
  //! @param    pMenu   The popup menu to be displayed.
  //! @return   The menu selection.
  int displayPopupMenu
    (CMenu* pMenu);

  //! Enables/disables the button.
  //! @param    bEnable   Flag: button is enabled.
  void enable
    (bool bEnable);

  //! Adds the button to a named button group.
  //! @param    strGroupName    Name of button group.
  //! @return   true if successful, false otherwise.
  bool addToGroup
    (CString strGroupName);

  //! Removes the button from its parent button group.
  //! @return   true if successful, false otherwise.
  bool removeFromGroup();

/////////////////////////////////////////////////////////////////////////////
// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FooButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
// Implementation
	// Generated message map functions
protected:
	//{{AFX_MSG(FooButton)
	afx_msg BOOL OnEraseBkgnd (CDC* pDC);
  afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp (UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave (WPARAM wParam, LPARAM lParam);
	afx_msg void OnMouseMove (UINT nFlags, CPoint point);
  afx_msg void OnKillFocus (CWnd* pNewWnd);
  afx_msg BOOL OnSetCursor (CWnd* pWnd, UINT nHitTest, UINT message);
  afx_msg void OnSetFocus (CWnd* pOldWnd);
	virtual void PreSubclassWindow();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  //! Button type.
  FooButton::Type m_type;

  //! Text style.
  FooButton::Text m_textStyle;

  //! Focus style.
  FooButton::Focus m_focusStyle;

  // Text color.
  COLORREF m_rgbText;

  //! Flag: mouse is being tracked.
  BOOL  m_bTracking;

  //! Bitmap id.
  int m_nBitmapId;

  //! Flag: draw single-line centered text.
  bool m_bCenter;

  //! Flag: draw multi-line text
  bool m_bMultiLine;

  //! Flag: button is checked.
  bool m_bChecked;

  //! Flag: display text.
  bool m_bText;

  //! Flag: display dropdown indicator.
  bool m_bDropDown;

  //! Flag: behave as multi button.
  bool m_bMulti;

  //! Flag: the button's drop-down was clicked.
  bool m_bMultiClicked;

  //! Flag: display as static control.
  bool m_bStatic;

  //! Flag: behave as hot button.
  bool m_bHot;

  //! Flag: display as hyperlink
  bool m_bHyperlink;

  //! Flag: draw gradient background for pushbuttons.
  bool m_bGradient;

  // Bitmap transparency color.
  COLORREF m_rgbTransparent;

  // Flag: is default button
  bool  m_bDefaultButton;

  // Parent button group
  CString m_strButtonGroup;

  // Number of FooButton instances.
  static int m_nRefs;

  // The collection of button groups.
  static CMapStringToPtr  m_btnGroups;

  //! Handle to msimg32.dll.
	static HINSTANCE m_hMsimg32;

  //! Pointer to gradient fill function in msimg32.dll.
  static MSIMG32callBack  m_dllGradientFillFunc;

  //! Top color of gradient button background.
  static COLORREF m_rgbTopGradient;

  //! Top color of hot gradient button background.
  static COLORREF m_rgbTopGradientHot;

  //! Bottom color of gradient button background.
  static COLORREF m_rgbBottomGradient;

  //! Bottom color of hot gradient button background.
  static COLORREF m_rgbBottomGradientHot;

/////////////////////////////////////////////////////////////////////////////
// Helper functions
protected:
  //! Draws the button's frame
  void drawFrame
    (CDC* pDC,
     LPDRAWITEMSTRUCT lpDrawItemStruct);

  //! Draws the button's frame for a radio/checkbox button
  void drawRadioCheckFrame
    (CDC* pDC,
     LPDRAWITEMSTRUCT lpDrawItemStruct,
     int& nLeftEdge);

  //! Draws the button's bitmap
  void drawBitmap
    (CDC* pDC,
     LPDRAWITEMSTRUCT lpDrawItemStruct,
     int& nBitmapRightEdge);

  //! Draws the button's caption
  void drawCaption
    (CDC* pDC,
     LPDRAWITEMSTRUCT lpDrawItemStruct,
     int nLeftEdge,
     int& nRightEdge);

  //! Draws the button's drop-down indicator
  void drawDropDown
    (CDC* pDC,
     LPDRAWITEMSTRUCT lpDrawItemStruct);

  //! Draws a multi-button's pressed drop-down region
  void drawMultiDropDownRegion
    (CDC* pDC,
     LPDRAWITEMSTRUCT lpDrawItemStruct);

  //! Draws the button's focus rectangle
  void drawFocus
    (CDC* pDC,
     LPDRAWITEMSTRUCT lpDrawItemStruct,
     int nLeftEdge,
     int nRightEdge);

  //! Draws the border for a default button
  void drawDefaultBorder
    (CDC* pDC,
     LPDRAWITEMSTRUCT lpDrawItemStruct);

  //! Draws a bitmap with transparency color.
  //! See http://www.codeguru.com/Cpp/G-M/bitmap/specialeffects/article.php/c1749/
  void TransparentBlt
    (HDC hdcDest, int nXDest, int nYDest, int nWidth,
     int nHeight, HBITMAP hBitmap, int nXSrc, int nYSrc,
     COLORREF colorTransparent, HPALETTE hPal);

  //! Draws a bitmap as disabled.
  //! See http://www.codeguru.com/Cpp/G-M/bitmap/specialeffects/article.php/c1699/
  void DisabledBlt
    (HDC hdcDest, int nXDest, int nYDest, int nWidth,
     int nHeight, HBITMAP hbm, int nXSrc, int nYSrc);

  //! Draws the button's drop-down triangle.
  void drawDropDownIndicator
    (HDC hdcDest, int nX, int nY, int nWidthDropDown, int nHeightDropDown);

  //! Draws the button's frame when hot tracking.
  void drawHotButtonFrame
    (LPDRAWITEMSTRUCT lpDrawItemStruct);

  //! Checks if the mouse was clicked on multi part of button.
  void multiHitTest
    (CPoint pt);

  //! Gets a button group by name
  //! @param  strGroupName    Group name
  //! @param  pGroup          Returned group
  static void getButtonGroup
    (CString strGroupName,
     void* & pGroup);

  //! Resets static storage used by the class.
  static void reset();

  //! Draws a gradient rectangle.
  //! See http://www.codeproject.com/staticctrl/gradient_static.asp
	static void drawGradientRect
    (CDC *pDC,
     CRect r,
     COLORREF cLeft,
     COLORREF cRight,
     BOOL bVertical);
};

#endif

// End FooButton.h