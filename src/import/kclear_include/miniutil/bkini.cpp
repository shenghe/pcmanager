/**
* @file    bkini.cpp
* @brief   ...
* @author  bbcallen
* @date    2010-01-19 18:01
*/

#include "stdafx.h"
#include "bkini.h"

#include <assert.h>
#include <atlfile.h>

NS_BKINI_BEGIN



struct BOMFlagType
{
    DWORD   dwBOMSize;
    LPCSTR  pszBOM;
    BOOL    bIsMutibyte;
    UINT    uMultiByteCodePage;
};
static const BOMFlagType BOMFlags[] = {
    { 3, "\xef\xbb\xbf", TRUE,  CP_UTF8 }, 
    { 2, "\xff\xfe",     FALSE, CP_ACP }, 
    { 0, NULL,           TRUE,  CP_THREAD_ACP }, 
};




class Parser
{
public:
    typedef wchar_t*                CharPtr;
    typedef const wchar_t*          ConstCharPtr;



    static HRESULT LoadText(Document& doc, ConstCharPtr pText, DWORD dwBytes)
    {
        if (!pText)
            return E_POINTER;


        HRESULT hr = ParseDoc(doc, pText, dwBytes);
        if (FAILED(hr))
            return hr;


        return S_OK;
    }



    static HRESULT ParseDoc(Document& doc, ConstCharPtr& pText, DWORD& dwBytes)
    {
        if (IsEndOfIni(pText, dwBytes))
            return S_OK;


        while (!IsEndOfIni(pText, dwBytes))
        {
            TrimSpace(pText, dwBytes);
            if (IsEndOfIni(pText, dwBytes))
                return S_OK;


            // find section [ at line being
            if (L'[' != *pText)
            {
                FindNextLine(pText, dwBytes);
                continue;
            }


            // name begin
            ConstCharPtr pSectionNameBegin = MoveToNextChar(pText, dwBytes);
            if (!FindInLine(pText, dwBytes, L']'))
            {
                FindNextLine(pText, dwBytes);
                continue;
            }


            CString strSectionName;
            strSectionName.SetString(pSectionNameBegin, (int)(pText - pSectionNameBegin));
            strSectionName.Trim();
            if (strSectionName.IsEmpty())
            {
                FindNextLine(pText, dwBytes);
                continue;
            }



            Section newSection = doc[strSectionName];
            if (newSection.IsNull())
                return E_OUTOFMEMORY;


            // skip line of section name
            FindNextLine(pText, dwBytes);
            if (IsEndOfIni(pText, dwBytes))
                return S_OK;


            ParseSection(newSection, pText, dwBytes);
        }



        return S_OK;
    }





    static HRESULT ParseSection(Section& section, ConstCharPtr& pText, DWORD& dwBytes)
    {
        // find key-value in section
        for (NULL; !IsEndOfIni(pText, dwBytes); FindNextLine(pText, dwBytes))
        {
            TrimSpace(pText, dwBytes);
            if (IsEndOfIni(pText, dwBytes))
                return S_FALSE;


            // 结束section的解析
            if (L'[' == *pText)
                return S_OK;


            // 跳过注释行
            if (L';' == *pText)
                continue;


            // find key name
            CString strKeyName;
            if (!FindKeyName(pText, dwBytes, strKeyName))
                continue;


            // find value
            CString strValue;
            FindValue(pText, dwBytes, strValue);


            // add key-value
            section.SetAt(strKeyName, strValue);
        }


        return S_FALSE;
    }


    static BOOL FindKeyName(ConstCharPtr& pText, DWORD& dwBytes, CString& strKeyName)
    {
        ConstCharPtr pKeyNameBegin = pText;
        if (IsEndOfIni(pText, dwBytes))
            return FALSE;


        // find '=' in line
        if (!FindInLine(pText, dwBytes, L'='))
            return FALSE;


        strKeyName.SetString(pKeyNameBegin, (int)(pText - pKeyNameBegin));
        strKeyName.Trim();
        if (strKeyName.IsEmpty())
            return FALSE;


        return TRUE;
    }


    static void FindValue(ConstCharPtr& pText, DWORD& dwBytes, CString& strValue)
    {
        strValue.Empty();


        if (IsEndOfIni(pText, dwBytes))
            return;


        if (!FindInLine(pText, dwBytes, L'='))
            return;


        ConstCharPtr pValueBegin = MoveToNextChar(pText, dwBytes);
        FindEndOfLine(pText, dwBytes);
        if (pText == pValueBegin)
            return;


        strValue.SetString(pValueBegin, (int)(pText - pValueBegin));
        strValue.Trim();
    }


    static BOOL FindInLine(ConstCharPtr& pText, DWORD& dwBytes, wchar_t chFind)
    {
        // find line break
        for (NULL; !IsEndOfIni(pText, dwBytes); MoveToNextChar(pText, dwBytes))
        {
            wchar_t ch = *pText; 

            if (L'\r' == ch || L'\n' == ch)
                return FALSE;

            if (chFind == ch)
                return TRUE;
        }

        return FALSE;
    }


    static void FindEndOfLine(ConstCharPtr& pText, DWORD& dwBytes)
    {
        // find line break
        for (NULL; !IsEndOfIni(pText, dwBytes); MoveToNextChar(pText, dwBytes))
        {
            wchar_t ch = *pText; 

            if (L'\r' == ch || L'\n' == ch)
                break;
        }
    }


    static void FindNextLine(ConstCharPtr& pText, DWORD& dwBytes)
    {
        // find line break
        FindEndOfLine(pText, dwBytes);

        // find next line
        for (NULL; !IsEndOfIni(pText, dwBytes); MoveToNextChar(pText, dwBytes))
        {
            wchar_t ch = *pText; 

            if (L'\r' != ch || L'\n' != ch)
                break;
        }
    }


    static void TrimSpace(ConstCharPtr& pText, DWORD& dwBytes)
    {
        // trim line break and white space
        for (NULL; !IsEndOfIni(pText, dwBytes); MoveToNextChar(pText, dwBytes))
        {
            wchar_t ch = *pText; 

            if (!iswspace(ch))
                break;
        }
    }

    
    static ConstCharPtr MoveToNextChar(ConstCharPtr& pText, DWORD& dwBytes)
    {
        if (!IsEndOfIni(pText, dwBytes))
        {
            ++pText;
            --dwBytes;
        }

        return pText;
    }


    static BOOL IsEndOfIni(ConstCharPtr pText, DWORD& dwBytes)
    {
        assert(pText);
        return !dwBytes || !*pText;
    }
};









KeyValue::KeyValue()
    : m_pDoc(NULL)
    , m_posSection(NULL)
    , m_posKeyValue(NULL)
{
}

KeyValue::KeyValue(Document* pDoc, POSITION posSection, POSITION posKeyValue)
    : m_pDoc(pDoc)
    , m_posSection(posSection)
    , m_posKeyValue(posKeyValue)
{
}

LPCWSTR KeyValue::GetKey()
{
    if (IsNull())
        return L"";

    Document::SectionNode* pSectionNode = m_pDoc->m_docMap.GetValueAt(m_posSection);
    assert(pSectionNode);
    if (!pSectionNode)
        return L"";
    
    return pSectionNode->GetKeyAt(m_posKeyValue);
}

LPCWSTR KeyValue::GetValueString()
{
    if (IsNull())
        return L"";

    Document::SectionNode* pSectionNode = m_pDoc->m_docMap.GetValueAt(m_posSection);
    assert(pSectionNode);
    if (!pSectionNode)
        return L"";

    return pSectionNode->GetValueAt(m_posKeyValue);
}

DWORD KeyValue::GetValueDWORD()
{
    if (IsNull())
        return 0;

    LPCWSTR lpszValue = GetValueString();
    if (!lpszValue)
        return 0;

    wchar_t* lpszEnd = NULL;
    return wcstoul(lpszValue, &lpszEnd, 10);
}

ULONGLONG KeyValue::GetValueULONGLONG()
{
    if (IsNull())
        return 0;

    LPCWSTR lpszValue = GetValueString();
    if (!lpszValue)
        return 0;

    wchar_t* lpszEnd = NULL;
    return _wcstoui64(lpszValue, &lpszEnd, 10);
}

BOOL KeyValue::IsNull()
{
    return !m_pDoc || !m_posSection || !m_posKeyValue;
}

KeyValue KeyValue::FindNextKeyValue()
{
    if (IsNull())
        return KeyValue();

    Document::SectionNode* pSectionNode = m_pDoc->m_docMap.GetValueAt(m_posSection);
    assert(pSectionNode);
    if (!pSectionNode)
        return KeyValue();

    POSITION posNextKeyValue = m_posKeyValue;
    pSectionNode->GetNext(posNextKeyValue);
    if (!posNextKeyValue)
        return KeyValue();

    return KeyValue(m_pDoc, m_posSection, posNextKeyValue);
}
















Section::Section()
    : m_pDoc(NULL)
    , m_posSection(NULL)
{
}

Section::Section(Document* pDoc, POSITION posSection)
    : m_pDoc(pDoc)
    , m_posSection(posSection)
{
}

BOOL Section::HasKey(LPCWSTR lpszKey)
{
    if (IsNull())
        return FALSE;

    Document::SectionNode* pNode = m_pDoc->m_docMap.GetValueAt(m_posSection);
    if (!pNode)
        return FALSE;

    return !!pNode->Lookup(lpszKey);
}

size_t Section::GetKeyCount()
{
    Document::SectionNode* pNode = m_pDoc->m_docMap.GetValueAt(m_posSection);
    if (!pNode)
        return FALSE;

    return !!pNode->GetCount();
}

KeyValue Section::operator[](LPCWSTR lpszKey)
{
    assert(lpszKey);
    assert(*lpszKey);
    if (IsNull() || !lpszKey || !*lpszKey)
        return KeyValue();

    Document::SectionNode* pNode = m_pDoc->m_docMap.GetValueAt(m_posSection);
    if (!pNode)
        return KeyValue();

    POSITION posKeyValue = pNode->Lookup(lpszKey);
    if (!posKeyValue)
    {
        posKeyValue = pNode->SetAt(lpszKey, L"");
    }

    return KeyValue(m_pDoc, m_posSection, posKeyValue);
}

KeyValue Section::SetAt(LPCWSTR lpszKey, LPCWSTR lpszValue)
{
    assert(lpszKey);
    assert(*lpszKey);
    if (IsNull() || !lpszKey || !*lpszKey)
        return KeyValue();

    Document::SectionNode* pNode = m_pDoc->m_docMap.GetValueAt(m_posSection);
    if (!pNode)
        return KeyValue();

    POSITION posKeyValue = pNode->SetAt(lpszKey, lpszValue);

    return KeyValue(m_pDoc, m_posSection, posKeyValue);
}

BOOL Section::IsNull()
{
    return !m_pDoc || !m_posSection;
}

Section Section::FindNextSection()
{
    if (IsNull())
        return Section();

    POSITION posNextSection = m_posSection;
    m_pDoc->m_docMap.GetNext(posNextSection);
    if (!posNextSection)
        return Section();

    return Section(m_pDoc, posNextSection);
}

LPCWSTR Section::GetName()
{
    if (IsNull())
        return L"";

    return m_pDoc->m_docMap.GetKeyAt(m_posSection);
}




















Document::Document()
{

}

Document::~Document()
{
    POSITION posSection = m_docMap.GetHeadPosition();
    for (NULL; posSection; m_docMap.GetNext(posSection))
    {
        Document::SectionNode* pNode = m_docMap.GetValueAt(posSection);
        if (pNode)
            delete pNode;
    }

    m_docMap.RemoveAll();
}

BOOL Document::HasSection(LPCWSTR lpszKey)
{
    assert(lpszKey);
    assert(*lpszKey);
    if (!lpszKey || !*lpszKey)
        return FALSE;


    return !!m_docMap.Lookup(lpszKey);
}

size_t Document::GetSectionCount()
{
    return m_docMap.GetCount();
}

Section Document::operator[](LPCWSTR lpszKey)
{
    assert(lpszKey);
    assert(*lpszKey);
    if (!lpszKey || !*lpszKey)
        return Section();

    POSITION posSection = m_docMap.Lookup(lpszKey);
    if (!posSection)
    {
        SectionNode* pNewSection = new SectionNode();
        if (!pNewSection)
            return Section();

        posSection = m_docMap.SetAt(lpszKey, pNewSection);
    }

    return Section(this, posSection);
}

Section Document::FindFirstSection()
{
    POSITION posFirst = m_docMap.GetHeadPosition();
    if (!posFirst)
        return Section(NULL, NULL);

    return Section(this, posFirst);
}


HRESULT Document::LoadFile(LPCWSTR lpszFilePath)
{
    if (!lpszFilePath)
        return E_POINTER;
        

    CAtlFile hFile;
    HRESULT hr = hFile.Create(
        lpszFilePath,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_DELETE,
        OPEN_EXISTING);
    if (FAILED(hr))
        return hr;



    ULONGLONG uFileSize = 0;
    hr = hFile.GetSize(uFileSize);
    if (FAILED(hr))
        return hr;



    // 对长度作简单限制
    if (uFileSize >= ULONG_MAX)
        return hr;



    CAtlArray<BYTE> byBuf;
    byBuf.SetCount((DWORD)uFileSize);
    if (uFileSize != byBuf.GetCount())
        return E_OUTOFMEMORY;



    hr = hFile.Read(byBuf.GetData(), (DWORD)byBuf.GetCount());
    if (FAILED(hr))
        return hr;


    return LoadBuffer(byBuf.GetData(), (DWORD)byBuf.GetCount());
}

HRESULT Document::LoadBuffer(const BYTE* pBuffer, DWORD dwBytes)
{
    m_docMap.RemoveAll();

    int nBOMType;
    for (nBOMType = 0; nBOMType < (_countof(BOMFlags) - 1); nBOMType++)
    {
        if (dwBytes < BOMFlags[nBOMType].dwBOMSize)
            continue;


        if (0 == memcmp(BOMFlags[nBOMType].pszBOM, pBuffer, BOMFlags[nBOMType].dwBOMSize))
            break;
    }


    pBuffer += BOMFlags[nBOMType].dwBOMSize;
    dwBytes -= BOMFlags[nBOMType].dwBOMSize;


    if (CP_UTF8 == BOMFlags[nBOMType].uMultiByteCodePage)
    {   // utf8
        CStringA strUTF8;
        strUTF8.SetString((LPCSTR)pBuffer, dwBytes);

        CString strWide = CA2W(strUTF8, CP_UTF8);
        return Parser::LoadText(*this, (LPCWSTR)strWide, strWide.GetLength());
    }
    else if (BOMFlags[nBOMType].bIsMutibyte)
    {   // ansi
        CStringA strAnsi;
        strAnsi.SetString((LPCSTR)pBuffer, dwBytes);

        CString strWide = CA2W(strAnsi);
        return Parser::LoadText(*this, (LPCWSTR)strWide, strWide.GetLength());
    }
    else
    {   // wide char
        return Parser::LoadText(*this, (LPCWSTR)pBuffer, dwBytes / sizeof(WCHAR));
    }
}


NS_BKINI_END