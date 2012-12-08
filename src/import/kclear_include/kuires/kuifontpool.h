//////////////////////////////////////////////////////////////////////////
//  Class Name: KUIFontPool
// Description: Font Pool
//     Creator: ZhangXiaoxuan
//     Version: 2009.4.22 - 1.0 - Change stl::map to CAtlMap
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlcoll.h>
#include <string>

#define KUIF_BOLD        0x0004U
#define KUIF_UNDERLINE   0x0002U
#define KUIF_ITALIC      0x0001U

#define KUIF_MAKEKEY(bold, underline, italic, adding) \
    (WORD)((adding << 8) \
    | (bold ? KUIF_BOLD : 0) \
    | (underline ? KUIF_UNDERLINE : 0) \
    | (italic ? KUIF_ITALIC : 0))

#define KUIF_ISBOLD(key)         ((key & KUIF_BOLD) == KUIF_BOLD)
#define KUIF_ISUNDERLINE(key)    ((key & KUIF_UNDERLINE) == KUIF_UNDERLINE)
#define KUIF_ISITALIC(key)       ((key & KUIF_ITALIC) == KUIF_ITALIC)
#define KUIF_GETADDING(key)      (key >> 8)

#define KUIF_DEFAULTFONT         (KUIF_MAKEKEY(FALSE, FALSE, FALSE, 0))
#define KUIF_BOLDFONT            (KUIF_MAKEKEY(TRUE, FALSE, FALSE, 0))

#define KUIF_GetFont(key)        KuiFontPool::GetFont(key)

class KuiFontPool
{
protected:

    typedef CAtlMap<WORD, HFONT> _TypeFontPool;

public:
    KuiFontPool()
        : m_strFaceName(_T("Tahoma"))
        , m_lFontSize(-11)
    {
        m_mapFont[KUIF_DEFAULTFONT] = _GetDefaultGUIFont();
    }
    virtual ~KuiFontPool()
    {
        HFONT hFont = NULL;

        POSITION pos = m_mapFont.GetStartPosition();

        while (pos != NULL)
        {
            hFont = m_mapFont.GetNextValue(pos);
            if (hFont)
                ::DeleteObject(hFont);
        }

        m_mapFont.RemoveAll();
    }

    static HFONT GetFont(WORD uKey)
    {
        _TypeFontPool::CPair* pPairRet = _Instance()->m_mapFont.Lookup(uKey);
        HFONT hftRet = NULL;

        if (NULL == pPairRet)
        {
            hftRet = _Instance()->_CreateNewFont(
                KUIF_ISBOLD(uKey), KUIF_ISUNDERLINE(uKey), KUIF_ISITALIC(uKey), KUIF_GETADDING(uKey)
                );
            if (hftRet)
                _Instance()->m_mapFont[uKey] = hftRet;
        }
        else
            hftRet = pPairRet->m_value;

        return hftRet;
    }

    static HFONT GetFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, char chAdding = 0)
    {
        return GetFont(KUIF_MAKEKEY(bBold, bUnderline, bItalic, chAdding));
    }

    static void SetDefaultFont(const std::wstring& strFaceName, LONG lSize)
    {
        _Instance()->m_strFaceName = strFaceName;
        _Instance()->m_lFontSize = lSize;

        HFONT hftOld = _Instance()->m_mapFont[KUIF_DEFAULTFONT];
        _Instance()->m_mapFont[KUIF_DEFAULTFONT] = _Instance()->_GetDefaultGUIFont();
        ::DeleteObject(hftOld);
    }

    static size_t GetCount()
    {
        return _Instance()->m_mapFont.GetCount();
    }

protected:

    LOGFONTW m_lfDefault;
    _TypeFontPool m_mapFont;
    std::wstring m_strFaceName;
    LONG m_lFontSize;

    static KuiFontPool* ms_pInstance;

    static KuiFontPool* _Instance()
    {
        if (!ms_pInstance)
            ms_pInstance = new KuiFontPool;
        return ms_pInstance;
    }

//     static KuiFontPool& _Instance()
//     {
//         static KuiFontPool s_obj;
//
//         return s_obj;
//     }

    HFONT _GetDefaultGUIFont()
    {
        DWORD dwSysVer = ::GetVersion();

        ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &m_lfDefault);

        m_lfDefault.lfHeight = _GetFontAbsHeight(m_lFontSize);
        wcscpy_s(m_lfDefault.lfFaceName, 32, m_strFaceName.c_str());

        m_lfDefault.lfQuality = ANTIALIASED_QUALITY;

        return ::CreateFontIndirect(&m_lfDefault);
    }

    HFONT _CreateNewFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, char chAdding)
    {
        LOGFONT lfNew;

        memcpy(&lfNew, &m_lfDefault, sizeof(LOGFONT));

        lfNew.lfWeight      = (bBold ? FW_BOLD : FW_NORMAL);
        lfNew.lfUnderline   = (FALSE != bUnderline);
        lfNew.lfItalic      = (FALSE != bItalic);

        lfNew.lfHeight = _GetFontAbsHeight(lfNew.lfHeight - chAdding);

        return ::CreateFontIndirect(&lfNew);
    }

    inline LONG _GetFontAbsHeight(LONG lSize)
    {
        return lSize;
        //         HDC hDC = ::GetDC(NULL);
        //         LONG lHeight = chSize;
        //
        //         lHeight *= 96;
        //         lHeight /= ::GetDeviceCaps(hDC, LOGPIXELSY);
        //
        //         ::ReleaseDC(NULL, hDC);
        //
        //
        //         return lHeight;
    }
};

// __declspec(selectany) CString KuiFontPool::ms_strFaceName = _T("Tahoma");
// __declspec(selectany) LONG KuiFontPool::ms_lFontSize = -11;

__declspec(selectany) KuiFontPool* KuiFontPool::ms_pInstance = NULL;
