#pragma once

#include <bksafe/bksafenavigatestring.h>

typedef CWinTraits<WS_POPUP, 0> CBkSafeDDEWndTraits;

class CBkSafeDDEWindow
    : public CWindowImpl<CBkSafeDDEWindow, ATL::CWindow, CBkSafeDDEWndTraits>
{
public:
    DECLARE_WND_CLASS_EX(BKSAFE_DDE_WND_CLASS, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_WINDOW)

public:
    CBkSafeDDEWindow()
    {
    }

    ~CBkSafeDDEWindow()
    {
        if (IsWindow())
            DestroyWindow();
    }

    HWND Create()
    {
        return __super::Create(NULL);
    }

    static BOOL PostDDEMessage(LPCTSTR lpszMessage, int nSize)
    {
        if (nSize > 255)
            return FALSE;

        HWND hWndDDE = ::FindWindow(BKSAFE_DDE_WND_CLASS, NULL);
        if (NULL == hWndDDE)
            return FALSE;

        ATOM atomDDE = ::GlobalAddAtom(lpszMessage);
        if (0 == atomDDE)
            return FALSE;

        return ::PostMessage(hWndDDE, WM_APP, (WPARAM)atomDDE, (LPARAM)nSize);
    }

protected:

    LRESULT OnApp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
    {
        ATOM atomDDE = (ATOM)wParam;
        int  nSize = (int)lParam;

        CString strNavigateString;

        UINT uRet = ::GlobalGetAtomName(atomDDE, strNavigateString.GetBuffer(nSize + 1), nSize);
        if (nSize != uRet)
        {
            strNavigateString.ReleaseBuffer(0);
            return 0;
        }

        ::GlobalDeleteAtom(atomDDE);

        strNavigateString.ReleaseBuffer(nSize);

        _Module.Navigate(strNavigateString);

        return 0;
    }

public:
    BEGIN_MSG_MAP_EX(CBkSafeDDEWindow)
        MESSAGE_HANDLER_EX(WM_APP, OnApp)
    END_MSG_MAP()
};