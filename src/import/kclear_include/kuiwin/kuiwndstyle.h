//////////////////////////////////////////////////////////////////////////
//   File Name: KuiWndStyle.h
// Description: KuiWindow Styles Definition
//     Creator: Zhang Xiaoxuan
//     Version: 2009.04.28 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "kuiskin.h"

class KuiStyle : public CKuiObject
{
    KUIOBJ_DECLARE_CLASS_NAME(KuiStyle, "style")

public:
    KuiStyle()
        : m_uAlign(0)
        , m_nTextAlign(DT_BOTTOM | DT_SINGLELINE)
        , m_crBg(CLR_INVALID)
        , m_crBgHover(CLR_INVALID)
        , m_crText(CLR_INVALID)
        , m_crHoverText(CLR_INVALID)
        , m_crDisabledText(RGB(0xCC, 0xCC, 0xCC))
        , m_ftText(NULL)
        , m_ftHover(NULL)
        , m_nMarginX(0)
        , m_nMarginY(0)
        , m_nSpacing(0)
        , m_nLineSpacing(20)
        , m_lpCursorName(IDC_ARROW)
    {
    }

    enum {
        // Specify by "cursor" attribute
        Cursor_Mask     = 0xF00UL,
        Cursor_Arrow    = 0x000UL,   // cursor = "arrow"
        Cursor_Hand     = 0x100UL,   // cursor = "hand"
    };

    COLORREF m_crBg;
    COLORREF m_crBgHover;
    COLORREF m_crText;
    COLORREF m_crHoverText;
    COLORREF m_crDisabledText;
    UINT m_uAlign;
    int m_nTextAlign;
    HFONT m_ftText;
    HFONT m_ftHover;
    int m_nMarginX;
    int m_nMarginY;
    int m_nSpacing;
    int m_nLineSpacing;
    LPCTSTR m_lpCursorName;
    CStringA m_strSkinName;

    // Get class name
    LPCSTR GetName()
    {
        return m_strClassName;
    }

    // Get style object from pool by class name
    static const KuiStyle& GetStyle(LPCSTR lpszName)
    {
        CStringA strName = lpszName;
        const __StylePoolClass::CPair *pFindRet = _GetStylePool()->Lookup(strName);

        if (pFindRet)
            return pFindRet->m_value;
        else
            return (*_GetStylePool())[""];
    }

	static BOOL LoadStyles(const std::string& strXml)
	{
		return LoadStyles(strXml.c_str());
	}

    static BOOL LoadStyles(LPCSTR lpszXml)
    {
        TiXmlDocument xmlDoc;

        _GetStylePool()->RemoveAll();

        (*_GetStylePool())[""];

        xmlDoc.Parse(lpszXml, NULL, TIXML_ENCODING_UTF8);

        if (xmlDoc.Error())
            return FALSE;

        _LoadStylePool(xmlDoc.RootElement());

        return TRUE;
    }

    static size_t GetCount()
    {
        return _GetStylePool()->GetCount();
    }

protected:

    typedef CAtlMap<CStringA, KuiStyle> __StylePoolClass;

    CStringA m_strClassName;

    static __StylePoolClass* ms_pStylePool;

    static __StylePoolClass* _GetStylePool()
    {
        if (!ms_pStylePool)
            ms_pStylePool = new __StylePoolClass;
        return ms_pStylePool;
    }

//     static __StylePoolClass& _GetStylePool()
//     {
//         static __StylePoolClass s_mapStylePool;
//
//         return s_mapStylePool;
//     }

    // Load style-pool from xml tree
    static void _LoadStylePool(TiXmlElement *pXmlStyleRootElem)
    {
        LPCSTR lpszClassName = NULL;

        if (!pXmlStyleRootElem)
            return;

        if (strcmp(pXmlStyleRootElem->Value(), "style") != 0)
            return;

        for (TiXmlElement* pXmlChild = pXmlStyleRootElem->FirstChildElement("class"); NULL != pXmlChild; pXmlChild = pXmlChild->NextSiblingElement("class"))
        {
            lpszClassName = pXmlChild->Attribute("name");
            if (!lpszClassName)
                continue;

            (*_GetStylePool())[lpszClassName].Load(pXmlChild);
        }
    }

    KUIWIN_DECLARE_ATTRIBUTES_BEGIN()
        KUIWIN_STRING_ATTRIBUTE("name", m_strClassName, TRUE)
        KUIWIN_STRING_ATTRIBUTE("skin", m_strSkinName, TRUE)
        KUIWIN_HEX_ATTRIBUTE("textmode", m_nTextAlign, TRUE)
        KUIWIN_COLOR_ATTRIBUTE("crbg", m_crBg, FALSE)
        KUIWIN_COLOR_ATTRIBUTE("crbghover", m_crBgHover, FALSE)
        KUIWIN_COLOR_ATTRIBUTE("crtext", m_crText, FALSE)
        KUIWIN_COLOR_ATTRIBUTE("crhover", m_crHoverText, FALSE)
        KUIWIN_COLOR_ATTRIBUTE("crdisabled", m_crDisabledText, FALSE)
        KUIWIN_FONT_ATTRIBUTE("font", m_ftText, TRUE)
        KUIWIN_FONT_ATTRIBUTE("hoverfont", m_ftHover, TRUE)
        KUIWIN_INT_ATTRIBUTE("x-margin", m_nMarginX, TRUE)
        KUIWIN_INT_ATTRIBUTE("y-margin", m_nMarginY, TRUE)
        KUIWIN_INT_ATTRIBUTE("margin", m_nMarginX = m_nMarginY, TRUE) // 这样比较bt，不过.....凑合用吧
        KUIWIN_INT_ATTRIBUTE("spacing", m_nSpacing, TRUE)
        KUIWIN_INT_ATTRIBUTE("linespacing", m_nLineSpacing, TRUE)
        KUIWIN_ENUM_ATTRIBUTE("cursor", LPCTSTR, FALSE)
            KUIWIN_ENUM_VALUE("hand", IDC_HAND)
            KUIWIN_ENUM_VALUE("arrow", IDC_ARROW)
        KUIWIN_ENUM_END(m_lpCursorName)
    KUIWIN_DECLARE_ATTRIBUTES_END()
};

__declspec(selectany) KuiStyle::__StylePoolClass* KuiStyle::ms_pStylePool = NULL;
