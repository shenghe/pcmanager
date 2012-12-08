/**
* @file    bkini.h
* @brief   ...
* @author  bbcallen
* @date    2010-01-19 18:01
*/

#ifndef BKINI_H
#define BKINI_H

#include <atlstr.h>
#include <atlcoll.h>

#ifndef NS_BKINI_BEGIN
#define NS_BKINI_BEGIN      namespace BKIni {   ///< declare namespace BkIni
#define NS_BKINI_END        };                  ///< declare namespace BkIni
#endif//NS_BKINI_BEGIN

NS_BKINI_BEGIN

class CNoCaseCompareTraits: public CElementTraitsBase<CString>
{
public:
    typedef const CString& INARGTYPE;
    typedef CString& OUTARGTYPE;

    static bool CompareElements( INARGTYPE element1, INARGTYPE element2 )
    {
        return 0 == element1.CompareNoCase(element2);
    }

    static int CompareElementsOrdered( INARGTYPE element1, INARGTYPE element2 )
    {
        return element1.CompareNoCase(element2);
    }
};




class KeyValue;
class Section;
class Document;



class KeyValue
{
public:
    KeyValue();
    KeyValue(Document* pDoc, POSITION posSection, POSITION posKeyValue);

    LPCWSTR     GetKey();
    LPCWSTR     GetValueString();
    DWORD       GetValueDWORD();
    ULONGLONG   GetValueULONGLONG();

    BOOL        IsNull();

    KeyValue    FindNextKeyValue();

protected:

    Document* m_pDoc;
    POSITION  m_posSection;
    POSITION  m_posKeyValue;
};



class Section
{
public:
    Section();
    Section(Document* pDoc, POSITION posSection);

    BOOL        HasKey(LPCWSTR lpszKey);
    size_t      GetKeyCount();

    KeyValue    operator[](LPCWSTR lpszKey);
    KeyValue    SetAt(LPCWSTR lpszKey, LPCWSTR lpszValue);

    BOOL        IsNull();
    Section     FindNextSection();

    LPCWSTR     GetName();

protected:

    Document* m_pDoc;
    POSITION  m_posSection;
};



class Document
{
public:
    Document();
    virtual ~Document();

    HRESULT     LoadFile(LPCWSTR lpszFilePath);
    HRESULT     LoadBuffer(const BYTE* pBuffer, DWORD dwBytes);

    BOOL        HasSection(LPCWSTR lpszKey);
    size_t      GetSectionCount();

    Section     operator[](LPCWSTR lpszKey);

    Section     FindFirstSection();

protected:
    Document(Document&);
    Document& operator=(Document&);

    friend class KeyValue;
    friend class Section;

    typedef CRBMap<CString, CString, CNoCaseCompareTraits>        SectionNode;
    typedef CRBMap<CString, SectionNode*, CNoCaseCompareTraits>   DocRoot;

    DocRoot m_docMap;
};


NS_BKINI_END

#endif//BKINI_H