#pragma once

#include <kuiwin/kuiimage.h>

class CWH3StateCheckImageList
    : public CImageList
{
public:

    BOOL Create(int cx = 16, int cy = 16)
    {
        BOOL bRet = CImageList::Create(cx, cy, ILC_COLOR24 | ILC_MASK, 0, 4);

        CKuiImage img;
        CDC dc, dcMem;
        CKuiCheckBoxTheme theme;
        CRect rcCheckBox;

        int nThemeState[] = {
            0,
            CBS_UNCHECKEDNORMAL,
            CBS_CHECKEDNORMAL,
            CBS_MIXEDNORMAL
        };
        int nNoThemeState[] = {
            0,
            DFCS_BUTTONCHECK | DFCS_TRANSPARENT,
            DFCS_BUTTONCHECK | DFCS_TRANSPARENT | DFCS_CHECKED,
            DFCS_BUTTONCHECK | DFCS_TRANSPARENT | DFCS_CHECKED | DFCS_BUTTON3STATE
        };

        HWND hWndDesktop = ::GetDesktopWindow();
        HDC hDCDesktop = ::GetDC(hWndDesktop);

        dcMem.CreateCompatibleDC(hDCDesktop);
        img.CreateBitmap(cx * 4, cy, RGB(0xFF, 0, 0xFF));

        HBITMAP hBmpOld = dcMem.SelectBitmap(img);

        ::ReleaseDC(hWndDesktop, hDCDesktop);

        theme.OpenTheme(hWndDesktop);

        for (int i = 1; i < 4; i ++)
        {
            rcCheckBox.SetRect(i * cx + (cx - 13) / 2, (cy - 13) / 2, i * cx + (cx + 13) / 2, (cy + 13) / 2);

            if (theme.IsValid())
            {
                theme.DrawBackground(dcMem, nThemeState[i], rcCheckBox);
            }
            else
            {
                dcMem.DrawFrameControl(rcCheckBox, DFC_BUTTON, nNoThemeState[i]);
            }
        }

        dcMem.SelectBitmap(hBmpOld);

        Add(img, RGB(0xFF, 0, 0xFF));

        img.DeleteObject();

        return bRet;
    }
};
