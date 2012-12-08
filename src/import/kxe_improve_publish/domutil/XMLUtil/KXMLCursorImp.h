/********************************************************************
* CreatedOn: 2006-11-27   15:06
* FileName:  KXMLCursorImp.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KXMLCursorImp_H_
#define __KXMLCursorImp_H_


#include "KXMLCursor.h"
#include "../KMPCommon/KConv.h"

#include <string>

_KAN_DOM_BEGIN

static const char* s_pcszNS_KDOM        = "xmlns:kdom";
static const char* s_pcszNSURN_KDOM     = "urn:kdom:www.kingsoft.com/ksce";
static const char* s_pcszNS_Array       = "kdom:Array";
static const char* s_pcszNS_ArrayElem   = "kdom:e";

//////////////////////////////////////////////////////////////////////////
// public implement

inline KXMLCursor::KXMLCursor(): m_bInitFlag(false)
{
}


inline KXMLCursor::~KXMLCursor()
{
}




inline long KXMLCursor::LoadFile(const TKChar* pcszFileName)
{
    assert(pcszFileName);

    _ClearCursor();

    long lRet = m_xmlDoc.LoadFile(pcszFileName);
    if (FAILED(lRet))
    {
        return lRet;
    }

    lRet = _InitCursor();
    if (FAILED(lRet))
    {
        return lRet;
    }

    return S_OK;
}

inline long KXMLCursor::LoadString(const char* pcszInput)
{
    assert(pcszInput);

    _ClearCursor();

    long lRet = m_xmlDoc.LoadXml(pcszInput);
    if (FAILED(lRet))
    {
        return lRet;
    }

    lRet = _InitCursor();
    if (FAILED(lRet))
    {
        return lRet;
    }

    return S_OK;
}

inline long KXMLCursor::LoadDOMName(const char* pcszName)
{
    assert(pcszName);

    _ClearCursor();

    long lRet = m_xmlDoc.LoadXMLRootName(pcszName);
    if (FAILED(lRet))
    {
        return lRet;
    }

    lRet = _InitCursor();
    if (FAILED(lRet))
    {
        return lRet;
    }

    return S_OK;
}


inline long KXMLCursor::SaveFile(const TKChar* pcszFileName)
{
    assert(m_bInitFlag);
    assert(pcszFileName);
    return m_xmlDoc.SaveFile(pcszFileName);
}


inline long KXMLCursor::ToString(std::string* pstrOutput)
{
    assert(m_bInitFlag);
    assert(pstrOutput);
    return m_xmlDoc.ToXMLString(pstrOutput);
}

inline long KXMLCursor::ToCompactString(std::string* pstrRet)
{
    assert(m_bInitFlag);
    assert(pstrRet);
    return m_xmlDoc.ToCompactXMLString(pstrRet);
}

template<class _KOtherDOMCursor>
inline long KXMLCursor::ToOtherDOM(_KOtherDOMCursor& otherDOMCursor)
{
    assert(m_bInitFlag);

    // 避免自复制
    if ((void*)this == (void*)&otherDOMCursor)
        return S_OK;

    std::string strExchangeValue;
    std::string strObjName;

    long lRet = GetObjName(strObjName);
    if (FAILED(lRet))
    {
        return lRet;
    }

    lRet = otherDOMCursor.LoadDOMName(strObjName.c_str());
    if (FAILED(lRet))
    {
        return lRet;
    }

    PrepareForRead();
    otherDOMCursor.PrepareForWrite();

    return CopyNodeTo(otherDOMCursor);
}





//inline long KXMLCursor::GetLastInternalError()
//{
//    long lRet = m_lLastInternalError;
//
//    m_lLastInternalError = S_OK;
//
//    return lRet;
//}

inline void KXMLCursor::DumpPath()
{
    m_pathDump.clear();

    for (size_t i = 0; i < m_scopeStack.size(); ++i)
    {
        m_pathDump.push_back(m_scopeStack[i].m_strObjName);

        if (m_scopeStack[i].m_hObj.Node() == NULL)
        {   // 找到出错的位置,仅保留第一个遇到的null对象
            break;
        }
    }
}

inline void KXMLCursor::PopPathDump(std::vector<std::string>& pathInfo)
{
    pathInfo.swap(m_pathDump);
}










inline long KXMLCursor::PhrasePath(const char* pcszPath, std::vector<std::string>& pathItemList)
{
    assert(m_bInitFlag);
    assert(pcszPath);
    return m_xmlDoc.PhrasePath(pcszPath, pathItemList);
}





inline void KXMLCursor::PrepareForRead()
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    m_scopeStack[0].m_bWrite = false;
}

inline void KXMLCursor::PrepareForWrite()
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    m_scopeStack[0].m_bWrite = true;
}



inline void KXMLCursor::BeginChild(const char* pcszName, bool bWrite)
{
    assert(m_bInitFlag);
    assert(pcszName);
    assert(!m_scopeStack.empty());

    KXMLLocalScope hNewScope;

    hNewScope.m_bWrite     = bWrite;
    hNewScope.m_strObjName = pcszName;

    if (pcszName != NULL && isdigit(pcszName[0]))
    {
        hNewScope.m_strObjName.insert(0, "_");
    }

    if (!m_scopeStack.empty())
    {
        bool bCreateNewChild = bWrite;

        if (m_scopeStack.back().m_hObj.Node() != NULL)
        {
            hNewScope.m_hObj = m_xmlDoc.OpenChild(
                m_scopeStack.back().m_hObj,
                hNewScope.m_strObjName.c_str(),
                bCreateNewChild
            );
        }
    }

    // 如果失败，这里会将一个空的KXMLHandle推到栈顶
    m_scopeStack.push_back(hNewScope);
}


inline void KXMLCursor::BeginFirstChild(bool bWrite)
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());
    // 写操作不应该调用这个函数
    assert(!bWrite);

    KXMLLocalScope hNewScope;

    hNewScope.m_bWrite = bWrite;

    if (!m_scopeStack.empty())
    {
        if (m_scopeStack.back().m_hObj.Node() != NULL)
        {
            hNewScope.m_hObj = m_xmlDoc.OpenFirstChildForRead(m_scopeStack.back().m_hObj);
            if (hNewScope.m_hObj.Node())
            {
                m_xmlDoc.GetName(hNewScope.m_hObj, hNewScope.m_strObjName);   
            }
        }
    }

    // 如果失败，这里会将一个空的KXMLHandle推到栈顶
    m_scopeStack.push_back(hNewScope);
}


inline void KXMLCursor::ToNextBrother()
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    KXMLLocalScope hNewScope;

    if (!m_scopeStack.empty())
    {
        hNewScope.m_bWrite = m_scopeStack.back().m_bWrite;
        // 写操作不应该调用这个函数
        assert(!hNewScope.m_bWrite);

        if (m_scopeStack.back().m_hObj.Node() != NULL)
        {
            hNewScope.m_hObj = m_xmlDoc.OpenNextBrotherForRead(m_scopeStack.back().m_hObj);
            if (hNewScope.m_hObj.Node() != NULL)
            {
                m_xmlDoc.GetName(hNewScope.m_hObj, hNewScope.m_strObjName);   
            }
        }
    }

    // 如果失败，这里会将一个空的KXMLHandle推到栈顶
    m_scopeStack.pop_back();
    assert(!m_scopeStack.empty());
    m_scopeStack.push_back(hNewScope);
}

inline bool KXMLCursor::NoMoreBrother()
{
    assert(m_bInitFlag);

    if (m_scopeStack.empty())
    {
        return true;
    }

    bool bWrite = m_scopeStack.back().m_bWrite;

    if (bWrite)
    {   // 写操作总是在末尾添加,所以永远不会到达终点
        return false;
    }

    return (m_scopeStack.back().m_hObj.Node() == NULL);
}

inline void KXMLCursor::EndChild()
{
    assert(m_bInitFlag);

    m_scopeStack.pop_back();

    assert(!m_scopeStack.empty());
}

inline bool KXMLCursor::IsNull()
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    if (m_scopeStack.empty())
    {
        return false;
    }

    return (m_scopeStack.back().m_hObj.Node() == NULL);
}


inline bool KXMLCursor::IsLeaf()
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    KXMLHANDLE hChild = m_scopeStack.back().m_hObj.FirstChildElement();

    return (hChild.Node() == NULL);
}








inline void KXMLCursor::BeginFirstArrayElem(bool bWrite)
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    BeginChild(s_pcszNS_ArrayElem, bWrite);    
}

inline void KXMLCursor::ToNextArrayElem()
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    KXMLLocalScope hNewScope;

    if (!m_scopeStack.empty())
    {
        hNewScope.m_bWrite     = m_scopeStack.back().m_bWrite;
        hNewScope.m_strObjName = m_scopeStack.back().m_strObjName;

        bool bCreateNewChild = hNewScope.m_bWrite;

        if (m_scopeStack.back().m_hObj.Node() != NULL)
        {
            hNewScope.m_hObj = m_xmlDoc.OpenNext(
                m_scopeStack.back().m_hObj,
                hNewScope.m_strObjName.c_str(),
                bCreateNewChild
            );
        }
    }

    // 如果失败，这里会将一个空的KXMLHandle推到栈顶
    m_scopeStack.pop_back();
    assert(!m_scopeStack.empty());
    m_scopeStack.push_back(hNewScope);
}

inline bool KXMLCursor::ArrayElemIsEnd()
{
    assert(m_bInitFlag);

    if (m_scopeStack.empty())
    {
        return true;
    }

    bool bWrite = m_scopeStack.back().m_bWrite;

    if (bWrite)
    {   // 写操作总是在末尾添加,所以永远不会到达终点
        return false;
    }

    return (m_scopeStack.back().m_hObj.Node() == NULL);
        
}

inline void KXMLCursor::EndArrayElem()
{
    assert(m_bInitFlag);

    m_scopeStack.pop_back();

    assert(!m_scopeStack.empty());
}

inline void KXMLCursor::BeginArray(bool bWrite)
{
    // 声明一个命名空间
    KXMLHANDLE hObj = m_scopeStack.back().m_hObj.Node();

    if (NULL != hObj.Node())
    {
        if (bWrite)
        {
            m_xmlDoc.SetAttribute(
                hObj,
                s_pcszNS_KDOM,
                s_pcszNSURN_KDOM
            );
        }
    }

    BeginChild(s_pcszNS_Array, bWrite);
}

inline void KXMLCursor::EndArray()
{
    assert(m_bInitFlag);

    m_scopeStack.pop_back();

    assert(!m_scopeStack.empty());
}

inline bool KXMLCursor::IsArray()
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    if (m_scopeStack.back().m_bWrite)
    {   //写操作时,对象还未被创建,可以是任意类型
        return true;
    }

    std::string strNameSpaceURL;

    KXMLHANDLE hObj = m_scopeStack.back().m_hObj.Node();
    if (NULL == hObj.Node())
    {
        return false;
    }


    KXMLHANDLE hChild = hObj.FirstChildElement(s_pcszNS_Array);
    if (NULL == hChild.Node())
    {
        return false;
    }

    return true;
}



inline long KXMLCursor::EraseAllChildren()
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    long lRet = _VerifyCursor();
    if (FAILED(lRet))
    {
        return lRet;
    }


    return m_xmlDoc.EraseAllChildren(m_scopeStack.back().m_hObj);
}

inline long KXMLCursor::RemoveChildren(const char* pcszName)
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    long lRet = _VerifyCursor();
    if (FAILED(lRet))
    {
        return lRet;
    }

    return m_xmlDoc.RemoveChildren(m_scopeStack.back().m_hObj, pcszName);
}

inline long KXMLCursor::GetObjName(std::string& strName)
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    long lRet = _VerifyCursor();
    if (FAILED(lRet))
    {
        return lRet;
    }

    strName = m_scopeStack.back().m_strObjName;

    if (!strName.empty() && strName[0] == '_' && isdigit(strName[1]))
    {
        strName.erase(0, 1);
    }

    return S_OK;
}




template<class _KOtherDOMCursor>
inline long KXMLCursor::CopyNodeTo(_KOtherDOMCursor& otherDOMCursor)
{
    assert(m_bInitFlag);
    assert(!IsNull());

    long lRet = -1;

    if (IsNull())
    {
        return E_KXML_CURRENT_NODE_IS_NULL;
    }
    else if (IsLeaf())
    {
        std::string strValue;

        // 获取值失败,可能因为结点值为空,这里不做为错误处理
        lRet = GetObjValue(strValue);
        if (SUCCEEDED(lRet))
        {
            lRet = otherDOMCursor.SetObjValue(strValue);
            if (FAILED(lRet))
            {
                return lRet;
            }
        }

        return S_OK;
    }
    else if (IsArray())
    {
        // open 'this' for reading
        KDOMAutoArrayEx<KXMLCursor>     autoArray(this, false);
        KDOMAutoArrayElemEx<KXMLCursor> autoIter(this, false);

        if (!autoIter.End())
        {   // open 'other' for writing
            KDOMAutoArrayEx<_KOtherDOMCursor>     otherArray(&otherDOMCursor, true);
            KDOMAutoArrayElemEx<_KOtherDOMCursor> otherIter(&otherDOMCursor, true);


            lRet = CopyNodeTo(otherDOMCursor);
            if (FAILED(lRet))
            {
                return lRet;
            }
            

            // 读对象的MoveToNext()不会有副作用
            // 写对象的MoveToNext()可能会产生一个null object
            // 所以这里对两个MoveToNext()的顺序作了特殊处理
            autoIter.MoveToNext();
            while (!autoIter.End())
            {   // open 'other' for writing
                otherIter.MoveToNext();

                lRet = CopyNodeTo(otherDOMCursor);
                if (FAILED(lRet))
                {
                    return lRet;
                }

                autoIter.MoveToNext();                
            }
        }
    }
    else
    {
        // open 'this' for reading
        KDOMAutoBrotherEx<KXMLCursor> autoBrother(this, false);

        while (!autoBrother.NoMoreBrother())
        {
            std::string strName;

            lRet = GetObjName(strName);
            if (FAILED(lRet))
            {
                return lRet;
            }


            {
                // open 'other' for writing
                KDOMAutoChildEx<_KOtherDOMCursor>
                autoOther(strName.c_str(), &otherDOMCursor, true);

                lRet = CopyNodeTo(otherDOMCursor);
                if (FAILED(lRet))
                {
                    return lRet;
                }
            }

            autoBrother.MoveToNextBrother();
        }
    }

    return S_OK;
}








template<class T>
inline long KXMLCursor::ObjDataExchange(bool bWrite, T& value)
{
    assert(m_bInitFlag);

    return KXMLDataExchange(*this, value, bWrite);
}

template<class T>
inline long KXMLCursor::ObjDataExchange(bool bWrite, T& value, const T& defValue)
{
    assert(m_bInitFlag);

    long lRet = KXMLDataExchange(*this, value, bWrite);
    if (FAILED(lRet))
    {
        value = defValue;
    }

    return S_OK;
}




template<class T>
inline long KXMLCursor::BasicDataExchange(bool bWrite, T& value)
{
    assert(m_bInitFlag);

    if (bWrite)
    {
        return SetObjValue(value);
    }
    else
    {
        return GetObjValue(value);
    }
}



template<class T>
inline long KXMLCursor::BasicDataExchange(bool bWrite, T& value, const T& defValue)
{
    assert(m_bInitFlag);

    if (bWrite)
    {
        return SetObjValue(value);
    }
    else
    {
        return GetObjValue(value, defValue);
    }
}







template<class T>
inline long KXMLCursor::SetObjValue(const T& value)
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    long lRet = _VerifyCursor();
    if (FAILED(lRet))
    {
        return lRet;
    }

    lRet = m_xmlDoc.SetValue(m_scopeStack.back().m_hObj, value);
    if (FAILED(lRet))
    {
        return lRet;
    }

    return S_OK;
}




template<class T>
inline long KXMLCursor::GetObjValue(T& value)
{
    assert(m_bInitFlag);
    assert(!m_scopeStack.empty());

    long lRet = _VerifyCursor();
    if (FAILED(lRet))
    {
        return lRet;
    }


    lRet = m_xmlDoc.GetValue(m_scopeStack.back().m_hObj, value);
    if (FAILED(lRet))
    {
        return lRet;
    }

    return S_OK;
}

template<class T>
inline long KXMLCursor::GetObjValue(T& value, const T& defValue)
{
    assert(m_bInitFlag);

    long lRet = GetObjValue(value);

    if (FAILED(lRet))
    {
        value = defValue;
    }

    return S_OK;
}







template<class T>
inline long KXMLCursor::SetObjAttribute(const char* pcszAttName, const T& value)
{
    assert(m_bInitFlag);
    assert(pcszAttName);
    assert(!m_scopeStack.empty());

    long lRet = _VerifyCursor();
    if (FAILED(lRet))
    {
        return lRet;
    }

    lRet = m_xmlDoc.SetAttribute(m_scopeStack.back().m_hObj, pcszAttName, value);
    if (FAILED(lRet))
    {
        return lRet;
    }

    return S_OK;
}

template<class T>
inline long KXMLCursor::GetObjAttribute(const char* pcszAttName, T& value)
{
    assert(m_bInitFlag);
    assert(pcszAttName);
    assert(!m_scopeStack.empty());

    long lRet = _VerifyCursor();
    if (FAILED(lRet))
    {
        _TraceError(lRet);
        return E_KXML_INVALID_CURSOR_STATE;
    }

    lRet = m_xmlDoc.GetAttribute(m_scopeStack.back().m_hObj, pcszAttName, value);
    if (FAILED(lRet))
    {
        return lRet;
    }

    return S_OK;
}

template<class T>
inline long KXMLCursor::GetObjAttribute(
    const char* pcszAttName,
    T&          value,
    const T&    defValue
)
{
    assert(m_bInitFlag);
    assert(pcszAttName);

    long lRet = GetObjValue(pcszAttName, value);

    if (FAILED(lRet))
    {
        value = defValue;
    }

    return S_OK;
}












//////////////////////////////////////////////////////////////////////////
// private implement
inline long KXMLCursor::_InitCursor()
{
    _ClearCursor();

    KXMLNode* pRoot = NULL;

    int xmlRet = m_xmlDoc.XMLManager().GetRootNode(&pRoot);
    if (!xmlRet || !pRoot)
    {
        return E_KXML_FAIL_TO_GET_NODE_NAME;
    }

    std::string strRootName;
    xmlRet = m_xmlDoc.XMLManager().GetNodeName(pRoot, &strRootName);
    if (!xmlRet)
    {
        return E_KXML_FAIL_TO_GET_NODE_NAME;
    }

    m_scopeStack.push_back(KXMLLocalScope(true, KXMLHANDLE(pRoot), strRootName));

    m_bInitFlag = true;

    return S_OK;
}

inline void KXMLCursor::_UninitCursor()
{
    _ClearCursor();

    m_bInitFlag = false;
}

inline void KXMLCursor::_ClearCursor()
{
    m_scopeStack.clear();

    m_bInitFlag = false;
}

inline long KXMLCursor::_VerifyCursor()
{
    if (m_scopeStack.empty())
    {
        return E_KXML_SCOPE_STACK_IS_EMPTY;
    }

    if (m_scopeStack.back().m_hObj.Node() == NULL)
    {
        return E_KXML_CURRENT_NODE_IS_NULL;
    }

    return S_OK;
}

//
//inline void KXMLCursor::_TraceError(long lRet)
//{
//    m_lLastInternalError = lRet;
//}




_KAN_DOM_END




















#endif//__KXMLCursorImp_H_
