//////////////////////////////////////////////////////////////////////////
//  Class Name: KuiTheme
// Description: Windows Theme(XP later)
//     Creator: ZhangXiaoxuan
//     Version: 2009.5.12 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlcoll.h>
#include <Uxtheme.h>
#include <tmschema.h>

class KuiWinThemeFunc
{
    typedef HTHEME (WINAPI *FnOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
    typedef HRESULT (WINAPI *FnCloseThemeData)(HTHEME hTheme);
    typedef HRESULT (WINAPI *FnDrawThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);
    typedef HRESULT (WINAPI *FnSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);

public:
    KuiWinThemeFunc()
        : m_bThemeValid(FALSE)
        , m_pfnOpenThemeData(NULL)
        , m_pfnCloseThemeData(NULL)
        , m_pfnDrawThemeBackground(NULL)
        , m_pfnSetWindowTheme(NULL)
    {
        HMODULE hModTheme = ::GetModuleHandle(_T("uxtheme.dll"));

        if (!hModTheme)
            return;

        m_bThemeValid = TRUE;
        m_pfnOpenThemeData          = (FnOpenThemeData)::GetProcAddress(hModTheme, "OpenThemeData");
        m_pfnCloseThemeData         = (FnCloseThemeData)::GetProcAddress(hModTheme, "CloseThemeData");
        m_pfnDrawThemeBackground    = (FnDrawThemeBackground)::GetProcAddress(hModTheme, "DrawThemeBackground");
        m_pfnSetWindowTheme         = (FnSetWindowTheme)::GetProcAddress(hModTheme, "SetWindowTheme");
    }

    static BOOL IsValid()
    {
        return _Instance()->m_bThemeValid;
    }

    static HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
    {
        if (_Instance()->m_pfnOpenThemeData)
        {
            return _Instance()->m_pfnOpenThemeData(hwnd, pszClassList);
        }
        else
            return NULL;
    }

    static HRESULT CloseThemeData(HTHEME hTheme)
    {
        if (_Instance()->m_pfnCloseThemeData)
        {
            return _Instance()->m_pfnCloseThemeData(hTheme);
        }
        else
            return E_NOTIMPL;
    }

    static HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect)
    {
        if (_Instance()->m_pfnDrawThemeBackground)
        {
            return _Instance()->m_pfnDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
        }
        else
            return E_NOTIMPL;
    }

    static HRESULT SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
    {
        if (_Instance()->m_pfnSetWindowTheme)
        {
            return _Instance()->m_pfnSetWindowTheme(hwnd, pszSubAppName, pszSubIdList);
        }
        else
            return E_NOTIMPL;
    }

    enum
    {
        KuiThemeNameButton = 0,
        KuiThemeNameEnd,
    };

    static LPCWSTR ThemeName(int nID)
    {
        static const LPCWSTR Names[] =
        {
            L"BUTTON",
            L"",
        };

        static int NamesCount = sizeof(Names) / sizeof(LPCWSTR) - 1;

        if (nID >= 0 && nID < NamesCount)
            return Names[KuiThemeNameButton];
        else
            return NULL;
    }

private:

    BOOL m_bThemeValid;

    FnOpenThemeData         m_pfnOpenThemeData;
    FnCloseThemeData        m_pfnCloseThemeData;
    FnDrawThemeBackground   m_pfnDrawThemeBackground;
    FnSetWindowTheme        m_pfnSetWindowTheme;

    static KuiWinThemeFunc* ms_pInstance;

    static KuiWinThemeFunc* _Instance()
    {
        if (!ms_pInstance)
            ms_pInstance = new KuiWinThemeFunc;
        return ms_pInstance;
    }

//     static KuiWinThemeFunc& _Instance()
//     {
//         static KuiWinThemeFunc s_obj;
//
//         return s_obj;
//     }
};

__declspec(selectany) KuiWinThemeFunc* KuiWinThemeFunc::ms_pInstance = NULL;

template<int t_nThemeId, int t_partid>
class CKuiWinTheme
{
public:
    CKuiWinTheme(HWND hWnd = NULL)
        : m_hTheme(NULL)
    {
        if (hWnd)
            OpenTheme(hWnd);
    }

    ~CKuiWinTheme()
    {
        KuiWinThemeFunc::CloseThemeData(m_hTheme);
    }

    BOOL IsValid()
    {
        return (NULL != m_hTheme);
    }

    BOOL OpenTheme(HWND hWnd)
    {
        if (m_hTheme)
            return FALSE;

        m_hTheme = KuiWinThemeFunc::OpenThemeData(NULL, KuiWinThemeFunc::ThemeName(t_nThemeId));

        if (m_hTheme)
            return TRUE;

        return FALSE;
    }

    void DrawBackground(HDC hdc, int iStateId, const RECT *pRect)
    {
        KuiWinThemeFunc::DrawThemeBackground(m_hTheme, hdc, t_partid, iStateId, pRect, NULL);
    }

protected:

    HTHEME m_hTheme;
};

typedef CKuiWinTheme<KuiWinThemeFunc::KuiThemeNameButton, BP_CHECKBOX> CKuiCheckBoxTheme;
typedef CKuiWinTheme<KuiWinThemeFunc::KuiThemeNameButton, BP_RADIOBUTTON> CKuiRadioBoxTheme;
