/********************************************************************
* CreatedOn: 2006-11-27   14:27
* FileName:  KXMLDocImp.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KXMLDocImp_H_
#define __KXMLDocImp_H_

#include "KXMLDoc.h"
#include "../KMPCommon/KConv.h"

#include <sstream>
#include <string>

_KAN_DOM_BEGIN

//////////////////////////////////////////////////////////////////////////
// public implement

static const char* s_pcszNormalXMLDeclaration =
    "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";

inline KXMLDoc::KXMLDoc(): m_bInitFlag(false)
{
}




inline KXMLDoc::~KXMLDoc()
{
}




inline long KXMLDoc::LoadFile(const TKChar* pcszFileName)
{
    assert(pcszFileName);
    int xmlRet = m_xmlManager.LoadFile(pcszFileName);

    if (!xmlRet)
    {
        return E_KXML_FAIL_TO_LOAD_FILE;
    }

    m_bInitFlag = true;

    return S_OK;
}


inline long KXMLDoc::LoadXml(const char* pcszXML)
{
    assert(pcszXML);
    int xmlRet = m_xmlManager.LoadXml(pcszXML);

    if (!xmlRet)
    {
        return E_KXML_FAIL_TO_LOAD_TEXT;
    }

    m_bInitFlag = true;

    return S_OK;
}

inline long KXMLDoc::LoadXMLRootName(const char* pcszName)
{
    assert(pcszName);

    std::ostringstream os;

    os << s_pcszNormalXMLDeclaration
       << "<"
       << pcszName
       << "/>";

    int xmlRet = m_xmlManager.LoadXml(os.str().c_str());

    if (!xmlRet)
    {
        return E_KXML_FAIL_TO_LOAD_ROOT;
    }

    m_bInitFlag = true;

    return S_OK;
}

inline long KXMLDoc::SaveFile(const TKChar* pcszFileName)
{
    assert(m_bInitFlag);
    assert(pcszFileName);
    int xmlRet = m_xmlManager.SaveFile(pcszFileName);

    if (!xmlRet)
    {
        return E_KXML_FAIL_TO_SAVE_FILE;
    }

    return S_OK;
}

inline long KXMLDoc::ToXMLString(std::string* pstrRet)
{
    assert(m_bInitFlag);
    assert(pstrRet);
    int xmlRet = m_xmlManager.ToXMLString(pstrRet);

    if (!xmlRet)
    {
        return E_KXML_FAIL_TO_GEN_TEXT;
    }

    return S_OK;
}

inline long KXMLDoc::ToCompactXMLString(std::string* pstrRet)
{
    assert(m_bInitFlag);
    assert(pstrRet);
    int xmlRet = m_xmlManager.ToXMLString(pstrRet);

    if (!xmlRet)
    {
        return E_KXML_FAIL_TO_GEN_TEXT;
    }

    return S_OK;
}














//////////////////////////////////////////////////////////////////////////
// 将形如"abcd/efg/hij"的路径字符串解析为包含"abcd","efg","hij"三个元素的列表
inline long KXMLDoc::PhrasePath(const char* pcszPath, std::vector<std::string>& pathItemList)
{
    assert(m_bInitFlag);
    assert(pcszPath);

    return KDOMPhrasePath(pcszPath, pathItemList);
}


inline KXMLHANDLE KXMLDoc::OpenItemPath(const char* pcszPath, bool bCreateIfNotExist)
{
    assert(m_bInitFlag);
    assert(pcszPath);

    std::vector<std::string> pathItemList;

    int nRet = PhrasePath(pcszPath, pathItemList);
    if (nRet < 0)
    {
        return NULL;
    }

    int xmlRet = 0;
    KXMLNode* pNode = NULL;
    xmlRet = m_xmlManager.GetRootNode(&pNode);
    if (!xmlRet || !pNode)
    {
        return NULL;
    }

    KXMLHANDLE hXMLCursor(pNode);
    for (size_t i = 0; i < pathItemList.size(); ++i)
    {
        KXMLHANDLE hXMLChild(NULL);

        hXMLChild = OpenChild(hXMLCursor, pathItemList[i].c_str(), bCreateIfNotExist);
        if (!hXMLChild.Node())
        {
            return NULL;
        }

        hXMLCursor = hXMLChild;
    }

    return hXMLCursor;
}


inline KXMLHANDLE KXMLDoc::OpenChild(
    const KXMLHANDLE&   hParent,
    const char*         pcszName,
    bool                bCreateIfNotExist
)
{
    assert(m_bInitFlag);
    assert(pcszName);

    if (!hParent.Node())
    {
        return NULL;
    }

    int xmlRet = 0;
    KXMLHANDLE hChild = hParent.FirstChildElement(pcszName);
    if (!hChild.Node())
    {
        if (!bCreateIfNotExist)
        {
            return NULL;
        }
        else
        {
            KXMLNode* pNewNode = NULL;
            xmlRet = m_xmlManager.AppendNode(pcszName, &pNewNode, hParent.Node());
            if (!xmlRet || !pNewNode)
            {
                return NULL;
            }

            hChild = pNewNode;
        }
    }

    return hChild;
}


inline KXMLHANDLE KXMLDoc::OpenNext(
    const KXMLHANDLE&   hCurrent,
    const char*         pcszName,
    bool                bCreateIfNotExist
)
{
    assert(m_bInitFlag);
    assert(pcszName);

    if (!hCurrent.Node())
    {
        return NULL;
    }


    int xmlRet;
    std::string strNodeName;

    KXMLHANDLE hNext = hCurrent.NextElement();
    while (hNext.Node() != NULL)
    {
        xmlRet = m_xmlManager.GetNodeName(hNext.Node(), &strNodeName);
        if (!xmlRet)
        {
            return NULL;
        }

        if (0 == strNodeName.compare(pcszName))
        {   // 找到符合要求的结点
            break;
        }
    }
    
    if (hNext.Node() == NULL && bCreateIfNotExist)
    {   // 如果没有找到兄弟结点，则需要新建一个
        KXMLHANDLE hParent = hCurrent.ParentElement();
        if (!hParent.Node())
        {
            return NULL;
        }

        KXMLNode* pNewNode = NULL;
        xmlRet = m_xmlManager.AppendNode(pcszName, &pNewNode, hParent.Node());
        if (!xmlRet || !pNewNode)
        {
            return NULL;
        }

        hNext = pNewNode;
    }

    return hNext;
}

inline KXMLHANDLE KXMLDoc::OpenFirstChildForRead(const KXMLHANDLE& hParent)
{
    assert(m_bInitFlag);

    if (!hParent.Node())
    {
        return NULL;
    }

    return hParent.FirstChildElement();
}

inline KXMLHANDLE KXMLDoc::OpenNextBrotherForRead(const KXMLHANDLE& hCurrent)
{
    assert(m_bInitFlag);

    if (!hCurrent.Node())
    {
        return NULL;
    }

    return hCurrent.NextElement();
}




inline long KXMLDoc::EraseAllChildren(const KXMLHANDLE& hParent)
{
    assert(m_bInitFlag);
    assert(hParent.Node());

    if (!hParent.Node())
    {
        return E_KXML_PARENT_NODE_IS_NULL;
    }

    KXMLHANDLE hChild = hParent.FirstChildElement();

    while (hChild.Node())
    {
        int xmlRet = m_xmlManager.RemoveNode(hChild.Node());
        if (!xmlRet)
        {
            return E_KXML_FAIL_TO_REMOVE_NODE;
        }

        hChild = hParent.FirstChildElement();
    }

    return S_OK;
}

inline long KXMLDoc::RemoveChildren(const KXMLHANDLE& hParent, const char* pcszName)
{
    assert(m_bInitFlag);
    assert(hParent.Node());

    if (!hParent.Node())
    {
        return E_KXML_PARENT_NODE_IS_NULL;
    }

    KXMLHANDLE hChild = hParent.FirstChildElement(pcszName);

    while (hChild.Node())
    {
        int xmlRet = m_xmlManager.RemoveNode(hChild.Node());
        if (!xmlRet)
        {
            return E_KXML_FAIL_TO_REMOVE_NODE;
        }

        hChild = hParent.FirstChildElement(pcszName);
    }

    return S_OK;
}

inline KXMLManager& KXMLDoc::XMLManager()
{
    assert(m_bInitFlag);

    return m_xmlManager;
}


inline long KXMLDoc::GetName(const KXMLHANDLE& hXML, std::string& strName)
{
    assert(m_bInitFlag);

    int xmlRet = m_xmlManager.GetNodeName(hXML.Node(), &strName);
    if (!xmlRet)
    {
        return E_KXML_FAIL_TO_GET_NODE_NAME;
    }

    return S_OK;
}






template<class T>
inline long KXMLDoc::SetValue(KXMLHANDLE hXMLCursor, const T& value)
{
    assert(m_bInitFlag);

    return _SetXMLItem(KXMLTextWriter(&m_xmlManager, hXMLCursor), value);
}

template<class T>
inline long KXMLDoc::GetValue(KXMLHANDLE hXMLCursor, T& value)
{
    assert(m_bInitFlag);

    return _GetXMLItem(KXMLTextReader(&m_xmlManager, hXMLCursor), value);
}

template<class T>
inline long KXMLDoc::SetAttribute(
    KXMLHANDLE  hXMLCursor,
    const char* pcszAttName,
    const T&    value
)
{
    assert(m_bInitFlag);
    assert(pcszAttName);

    return _SetXMLItem(
        KXMLAttributeWriter(&m_xmlManager, hXMLCursor, pcszAttName),
        value
    );
}

template<class T>
inline long KXMLDoc::GetAttribute(
    KXMLHANDLE  hXMLCursor,
    const char* pcszAttName,
    T&          value
)
{
    assert(m_bInitFlag);
    assert(pcszAttName);

    return _GetXMLItem(
        KXMLAttributeReader(&m_xmlManager, hXMLCursor, pcszAttName),
        value
    );
}





















template<class ItemWriter, class T>
inline long KXMLDoc::_SetXMLItem(const ItemWriter& writer, const T& value)
{
    assert(m_bInitFlag);

    std::ostringstream os;
    os << value;
    if (os.fail())
    {
        return E_KXML_VALUE_STREAM_ERROR;
    }

    return writer.Write(os.str());
}




template<class ItemReader, class T>
inline long KXMLDoc::_GetXMLItem(const ItemReader& reader, T& value)
{
    assert(m_bInitFlag);

    std::string strValue;

    long lRet = reader.Read(strValue);
    if (FAILED(lRet))
    {
        return E_KXML_VALUE_STREAM_ERROR;
    }


    std::istringstream is;
    is.str(strValue);
    is >> value;
    if (is.fail())
    {
        return E_KXML_VALUE_STREAM_ERROR;
    }



    return S_OK;
}




template<class ItemWriter>
inline long KXMLDoc::_SetXMLItem(const ItemWriter& writer, const wchar_t* pwszValue)
{
    assert(m_bInitFlag);

    return writer.Write(std::string(KSCE::KU16ToU8(pwszValue)));
}







////////////////////////////////////////////////////////////////////////
// std::string
template <class ItemWriter>
inline long KXMLDoc::_SetXMLItem(const ItemWriter& writer, const std::string& strValue)
{
    assert(m_bInitFlag);

    return writer.Write(strValue);
}

template <class ItemReader>
inline long KXMLDoc::_GetXMLItem(const ItemReader& reader, std::string& strValue)
{
    assert(m_bInitFlag);

    return reader.Read(strValue);
}

template <class ItemWriter, class _Traits, class _Alloc>
inline long KXMLDoc::_SetXMLItem(
    const ItemWriter& writer,
    const std::basic_string<char, _Traits, _Alloc>& strValue
)
{
    assert(m_bInitFlag);

    return writer.Write(strValue.c_str());
}

template <class ItemReader, class _Traits, class _Alloc>
inline long KXMLDoc::_GetXMLItem(
    const ItemReader& reader,
    std::basic_string<char, _Traits, _Alloc>& strValue
)
{
    assert(m_bInitFlag);

    std::string tmp;

    int nRet = reader.Read(tmp);

    strValue = tmp.c_str();

    return nRet;
}



//////////////////////////////////////////////////////////////////////////
// std::wstring
template <class ItemWriter, class _Traits, class _Alloc>
inline long KXMLDoc::_SetXMLItem(
    const ItemWriter& writer,
    const std::basic_string<wchar_t, _Traits, _Alloc>& wstrValue
)
{
    assert(m_bInitFlag);

    return writer.Write(std::string(KSCE::KU16ToU8(wstrValue.c_str())));
}

template <class ItemReader, class _Traits, class _Alloc>
inline long KXMLDoc::_GetXMLItem(
    const ItemReader& reader,
    std::basic_string<wchar_t, _Traits, _Alloc>& wstrValue
)
{
    assert(m_bInitFlag);

    std::string strUTF8;
    long lRet = reader.Read(strUTF8);
    if (FAILED(lRet))
    {
        return lRet;
    }

    wstrValue = KSCE::KU8ToU16(strUTF8.c_str());

    return S_OK;
}




//////////////////////////////////////////////////////////////////////////
// wchar_t
template <class ItemWriter>
inline long KXMLDoc::_SetXMLItem(const ItemWriter& writer, wchar_t wcValue)
{
    assert(m_bInitFlag);

    wchar_t pcszValue[] = {wcValue, L'\0'};

    return writer.Write(std::string(KSCE::KU16ToU8(pcszValue)));
}

template <class ItemReader>
inline long KXMLDoc::_GetXMLItem(const ItemReader& reader, wchar_t& wcValue)
{
    assert(m_bInitFlag);

    std::string strUTF8;
    long lRet = reader.Read(strUTF8);
    if (FAILED(lRet))
    {
        return lRet;
    }

    KSCE::KU8ToU16 conv(strUTF8.c_str());

    if (conv.CharCount() < 2)
    {
        return E_KXML_VALUE_STREAM_ERROR;
    }

    wcValue = conv[0];

    return S_OK;
}



_KAN_DOM_END












#endif//__KXMLDocImp_H_
