#pragma once

#include "stdafx.h"
#include "Draw.h"

#define MAX_MENU_ITEM_TEXT_LENGTH       100
#define IMGPADDING                      6
#define TEXTPADDING                     8

// From <winuser.h>
#define OBM_CHECK                       32760

enum
{
	enmuTop100 = 0xe000,
	enmuExt1,
	enmuExt2,
	enmuExt3,
	enmuExt4,
	enmuExt5,
	enmuExt6,
	enmuExt7,
	enmuExt8,
	enmuExt9,
	enmuExt10,
	enmuExtEnd
};

template <class T>
class CFilterMenu
{
private:
    SIZE m_szBitmap;
    SIZE m_szButton;

    CFont m_fontMenu;               // used internally, only to measure text
    int m_cxExtraSpacing;
    bool m_bFlatMenus;
    COLORREF m_clrMask;
	HWND m_hNotifyHwnd;

    CSimpleStack<HMENU> m_stackMenuHandle;

protected:
	struct MenuItemData	            // menu item data
	{
		LPTSTR lpstrText;
		UINT fType;
		UINT fState;
        int iImage;
		int nPercent;
		int nId;
		CString lpstrExt;
	};

public:
    CImageList m_ImageList;

protected:
	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
	{
		MenuItemData* pmd = (MenuItemData*)lpMeasureItemStruct->itemData;

		if (pmd->fType & MFT_SEPARATOR)   // separator - use half system height and zero width
		{
			lpMeasureItemStruct->itemHeight = ::GetSystemMetrics(SM_CYMENU) / 2;
			lpMeasureItemStruct->itemWidth  = 0;
		}
		else
		{
			// Compute size of text - use DrawText with DT_CALCRECT
			CWindowDC dc(NULL);
			CFont fontBold;
			HFONT hOldFont = NULL;
			if (pmd->fState & MFS_DEFAULT)
			{
				// Need bold version of font
				LOGFONT lf = { 0 };
				m_fontMenu.GetLogFont(lf);
				lf.lfWeight += 200;
				fontBold.CreateFontIndirect(&lf);
				ATLASSERT(fontBold.m_hFont != NULL);
				hOldFont = dc.SelectFont(fontBold);
                fontBold.DeleteObject();
			}
			else
				hOldFont = dc.SelectFont(m_fontMenu);

			RECT rcText = { 0, 0, 0, 0 };
			dc.DrawText(pmd->lpstrText, -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT);

            int cx = rcText.right - rcText.left;

			dc.SelectFont(hOldFont);

			LOGFONT lf = { 0 };
			m_fontMenu.GetLogFont(lf);
			int cy = lf.lfHeight;
			if (cy < 0)
				cy = -cy;
			const int cyMargin = 10;
			cy += cyMargin;

			lpMeasureItemStruct->itemHeight = cy;

			// Width is width of text plus some
			cx += 4;                    // L/R margin for readability
			cx += 1;                    // space between button and menu text
			cx += 2 * m_szButton.cx;    // button width
			cx += m_cxExtraSpacing;     // extra between item text and accelerator keys

			// Windows adds 1 to returned value
			cx -= ::GetSystemMetrics(SM_CXMENUCHECK) - 1;
			lpMeasureItemStruct->itemWidth = cx;   // we are done
		}
	}

    void DrawItem (LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		MenuItemData* pmd = (MenuItemData*)lpDrawItemStruct->itemData;
		CDCHandle dc = lpDrawItemStruct->hDC;
		RECT& rcItem = lpDrawItemStruct->rcItem;
        LPCRECT pRect = &rcItem;
		BOOL bDisabled = lpDrawItemStruct->itemState & ODS_GRAYED;
    	BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
		BOOL bChecked = lpDrawItemStruct->itemState & ODS_CHECKED;
        COLORREF crBackImg = CLR_NONE;
        CDCHandle* pDC = &dc; 

        if (bSelected && !bDisabled)
        {
            COLORREF crHighLight = ::GetSysColor(COLOR_HIGHLIGHT);
            CPenDC pen(*pDC, crHighLight);
            CBrushDC brush(*pDC, crBackImg = bDisabled ? HLS_TRANSFORM(::GetSysColor (COLOR_3DFACE), +73, 0) : 
                                                        HLS_TRANSFORM (crHighLight, +70, -57));

            pDC->Rectangle (pRect);
        }
        else
        { 
            // Draw the menu item background
            CRect rc(pRect);

            rc.right = m_szBitmap.cx + IMGPADDING;
            pDC->FillSolidRect(rc, crBackImg = HLS_TRANSFORM(::GetSysColor(COLOR_3DFACE), +20, 0));
            rc.left = rc.right;
            rc.right = pRect->right + 100;
            pDC->FillSolidRect(rc, HLS_TRANSFORM(::GetSysColor(COLOR_3DFACE), +75, 0));
        }

        // Menu item is a separator
        if (pmd->fType & MFT_SEPARATOR)
        {
			rcItem.top += (rcItem.bottom - rcItem.top) / 2;     // vertical center
			dc.DrawEdge(&rcItem, EDGE_ETCHED, BF_TOP);          // draw separator line
        }
		else
		{   
            // Draw the text
            CRect rc (pRect);
			CString sTemp = pmd->lpstrText;
            CString sCaption = pmd->lpstrText;
			CString sCaption2;
            int nTab = sCaption.Find('|');

            if (nTab >= 0)
            {
                sCaption = sCaption.Left (nTab);
				sCaption2 = sTemp.Right(sTemp.GetLength() - nTab - 1);
            }
            pDC->SetTextColor(bDisabled ? HLS_TRANSFORM(::GetSysColor(COLOR_3DFACE), -18, 0) : ::GetSysColor(COLOR_MENUTEXT));
            pDC->SetBkMode(TRANSPARENT);

            CBoldDC bold(*pDC, (lpDrawItemStruct->itemState & ODS_DEFAULT) != 0);

            rc.left = m_szBitmap.cx + IMGPADDING + TEXTPADDING;
            pDC->DrawText(sCaption, sCaption.GetLength(), rc, DT_SINGLELINE|DT_VCENTER|DT_LEFT);

			CRect rcTemp = rc;
			rcTemp.top += 8;
			rcTemp.bottom -= 8;
			rcTemp.left += 40; 
			rcTemp.right = rcTemp.left + 52;
			int nTemp = rc.right;
			pDC->FillSolidRect(&rcTemp, RGB(200,200,200));

			rcTemp.top += 1;
			rcTemp.bottom -= 1;
			rcTemp.left +=2;
			int nPosEx = 50 - (int)(pmd->nPercent / 2);
			rcTemp.right -= nPosEx;
			pDC->FillSolidRect(&rcTemp, RGB(64,64,140));

		//	rc.left = nTemp + 30;

			int nTab2 = sCaption2.Find('|');
			if (nTab2 > 0)
			{
				sCaption2 = sCaption2.Left(nTab2);
			}
            if (nTab >= 0)
            {    
                rc.right -= TEXTPADDING + 30;
                //pDC->DrawText(pmd->lpstrText + nTab + 1, _tcslen(pmd->lpstrText + nTab + 1), rc, DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
				pDC->DrawText(sCaption2, sCaption2.GetLength(), rc, DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
            }

			if (nTab2 > 0)
			{
				rc.right = nTemp;
				rc.right -= TEXTPADDING + 4;
				pDC->DrawText(pmd->lpstrText + nTab +nTab2 + 2, (int)_tcslen(pmd->lpstrText + nTab2 +nTab + 2), rc, DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
			}
            // Draw background and border around the check mark
            if (bChecked)
            {
                COLORREF crHighLight = ::GetSysColor(COLOR_HIGHLIGHT);
                CPenDC pen(*pDC, crHighLight);
                CBrushDC brush(*pDC, crBackImg = bDisabled ? HLS_TRANSFORM(::GetSysColor (COLOR_3DFACE), +73, 0) :
                                                              (bSelected ? HLS_TRANSFORM(crHighLight, +50, -50) : HLS_TRANSFORM(crHighLight, +70, -57)));

                pDC->Rectangle(CRect(pRect->left + 1, pRect->top + 1, pRect->left + m_szButton.cx - 2, pRect->bottom - 1));
            }

            if (m_ImageList != NULL && pmd->iImage >= 0)
            {
                bool bOver = !bDisabled && bSelected;

                if (bDisabled || (bSelected && !bChecked))
                {
                    HICON hIcon = ImageList_ExtractIcon(NULL, m_ImageList, pmd->iImage);
                    CBrush brush;

                    brush.CreateSolidBrush(bOver ? HLS_TRANSFORM(::GetSysColor(COLOR_HIGHLIGHT), +50, -66) : HLS_TRANSFORM(::GetSysColor(COLOR_3DFACE), -27, 0));
                    pDC->DrawState(CPoint(pRect->left + (bOver ? 4 : 3), rc.top + (bOver ? 5 : 4)),
                                    CSize(m_szBitmap.cx, m_szBitmap.cx), hIcon, DSS_MONO, brush);
                    DestroyIcon(hIcon);
                }
                if (!bDisabled)
                {
                    ::ImageList_Draw(m_ImageList, pmd->iImage, pDC->m_hDC,
                                      pRect->left + ((bSelected && !bChecked) ? 2 : 3), rc.top + ((bSelected && !bChecked) ? 3 : 4), ILD_TRANSPARENT);
                }
            }
            else if (bChecked)
            {
                // Draw the check mark
                rc.left  = pRect->left + 5;
                rc.right = rc.left + m_szBitmap.cx + IMGPADDING;
                pDC->SetBkColor(crBackImg);
                HBITMAP hBmp = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CHECK));
                BOOL bRet = pDC->DrawState(CPoint(rc.left,rc.top + 3), CSize(rc.Size()), hBmp, DSS_NORMAL, (HBRUSH)NULL);
                DeleteObject(hBmp);
            }
		}
    }

    LRESULT InitMenuPopupHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        // System menu, do nothing
		if ((BOOL)HIWORD(lParam))   
		{
			bHandled = FALSE;
			return 1;
		}

		CMenuHandle menuPopup = (HMENU)wParam;
		ATLASSERT(menuPopup.m_hMenu != NULL);

        TCHAR szString[MAX_MENU_ITEM_TEXT_LENGTH];
        BOOL bRet = FALSE;

        for (int i = 0; i < menuPopup.GetMenuItemCount(); i++)
        {
            CMenuItemInfo mii;
            mii.cch = MAX_MENU_ITEM_TEXT_LENGTH;
            mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
            mii.dwTypeData = szString;
            bRet = menuPopup.GetMenuItemInfo(i, TRUE, &mii);
            ATLASSERT(bRet);

            if (!(mii.fType & MFT_OWNERDRAW))   // not already an ownerdraw item
            {
                MenuItemData * pMI = new MenuItemData;
                ATLASSERT(pMI != NULL);

                if (pMI)
                {
                    // Make this menu item an owner-drawn
                    mii.fType |= MFT_OWNERDRAW;

                    pMI->fType = mii.fType;
                    pMI->fState = mii.fState;

                    // Associate an image with a menu item
                    static_cast<T*>(this)->AssociateImage(mii, pMI);

                    pMI->lpstrText = new TCHAR[lstrlen(szString) + 1];
                    ATLASSERT(pMI->lpstrText != NULL);

                    if (pMI->lpstrText != NULL)
                        lstrcpy(pMI->lpstrText, szString);
                    mii.dwItemData = (ULONG_PTR)pMI;

                    bRet = menuPopup.SetMenuItemInfo(i, TRUE, &mii);
                    ATLASSERT(bRet);
                }
            }
        }

		// Add it to the list
        m_stackMenuHandle.Push(menuPopup.m_hMenu);
        
        return 0;
    }

    LRESULT MenuSelectHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
		// Check if a menu is closing, do a cleanup
		if (HIWORD(wParam) == 0xFFFF && lParam == NULL)   // Menu closing
		{
			// Restore the menu items to the previous state for all menus that were converted
            HMENU hMenu = NULL;
            while ((hMenu = m_stackMenuHandle.Pop()) != NULL)
            {
                CMenuHandle menuPopup = hMenu;
                ATLASSERT(menuPopup.m_hMenu != NULL);
                // Restore state and delete menu item data
                BOOL bRet = FALSE;
                for (int i = 0; i < menuPopup.GetMenuItemCount(); i++)
                {
                    CMenuItemInfo mii;
                    mii.fMask = MIIM_DATA | MIIM_TYPE;
                    bRet = menuPopup.GetMenuItemInfo(i, TRUE, &mii);
                    ATLASSERT(bRet);

                    MenuItemData * pMI = (MenuItemData*)mii.dwItemData;
                    if (pMI != NULL)
                    {
                        mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
                        mii.fType = pMI->fType;
                        mii.dwTypeData = pMI->lpstrText;
                        mii.cch = lstrlen(pMI->lpstrText);
                        mii.dwItemData = NULL;

                        bRet = menuPopup.SetMenuItemInfo(i, TRUE, &mii);
                        ATLASSERT(bRet);

                        delete [] pMI->lpstrText;
                        delete pMI;
                    }
                }
			}
		}

		bHandled = FALSE;
		return 1;
    }

	LRESULT MeasureItemHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LPMEASUREITEMSTRUCT lpMeasureItemStruct = (LPMEASUREITEMSTRUCT)lParam;
		MenuItemData * pmd = (MenuItemData*)lpMeasureItemStruct->itemData;

		if (lpMeasureItemStruct->CtlType == ODT_MENU && pmd != NULL)
		    MeasureItem(lpMeasureItemStruct);
		else
			bHandled = FALSE;

		return (LRESULT)TRUE;
	}

    LRESULT DrawItemHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;

        MenuItemData * pMI = (MenuItemData*)lpDrawItemStruct->itemData;
		if (lpDrawItemStruct->CtlType == ODT_MENU && pMI != NULL)     // only owner-drawn menu item
            DrawItem(lpDrawItemStruct);
		else
			bHandled = FALSE;

        return TRUE;
    }

public:
    CFilterMenu()
    {
        m_cxExtraSpacing = 0;
        m_clrMask = RGB(192, 192, 192);

        m_szBitmap.cx = 16;
        m_szBitmap.cy = 15;

    	m_szButton.cx = m_szBitmap.cx + 6;
		m_szButton.cy = m_szBitmap.cy + 6;
    }

    ~CFilterMenu()
    {
        m_ImageList.Destroy();
        m_fontMenu.DeleteObject();
    }

	// Note: do not forget to put CHAIN_MSG_MAP in your message map.
	BEGIN_MSG_MAP(CCoolContextMenu)
        MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup)
        MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
        MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
        MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
	END_MSG_MAP()

    LRESULT OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return InitMenuPopupHandler(uMsg, wParam, lParam, bHandled);
    }

    LRESULT OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return MenuSelectHandler(uMsg, wParam, lParam, bHandled);
    }

    LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return MeasureItemHandler(uMsg, wParam, lParam, bHandled);
    }

    LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return DrawItemHandler(uMsg, wParam, lParam, bHandled);
    }

    void AssociateImage(CMenuItemInfo& mii, MenuItemData * pMI)
    {
        // Default implementation, does not do anything
	}

	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		DWORD dwCurrentItem = LOWORD(wParam);

		HMENU hMenu = NULL;
		while ((hMenu = m_stackMenuHandle.Pop()) != NULL)
		{
			CMenuHandle menuPopup = hMenu;
			ATLASSERT(menuPopup.m_hMenu != NULL);
			// Restore state and delete menu item data
			BOOL bRet = FALSE;
			for (int i = 0; i < menuPopup.GetMenuItemCount(); i++)
			{
				CMenuItemInfo mii;
				mii.fMask = MIIM_DATA | MIIM_TYPE;
				bRet = menuPopup.GetMenuItemInfo(i, TRUE, &mii);
				ATLASSERT(bRet);

				MenuItemData * pMI = (MenuItemData*)mii.dwItemData;
				if (pMI != NULL)
				{
					mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE;
					mii.fType = pMI->fType;
					mii.dwTypeData = pMI->lpstrText;
					mii.cch = lstrlen(pMI->lpstrText);
					mii.dwItemData = NULL;

					if (pMI->nId == dwCurrentItem)
					{
						//::SendMessage(m_hNotifyHwnd, MSG_USER_POP_MENU, (WPARAM)&pMI->lpstrExt, 0);
					}

					bRet = menuPopup.SetMenuItemInfo(i, TRUE, &mii);
					ATLASSERT(bRet);

					delete [] pMI->lpstrText;
					delete pMI;
				}
			}
		}
		return 0;
	}

	void SetNotifyHwnd(HWND hWnd)
	{
		m_hNotifyHwnd = hWnd;
	}
};