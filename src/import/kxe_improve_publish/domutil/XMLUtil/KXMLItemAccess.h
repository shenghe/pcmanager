/********************************************************************
* CreatedOn: 2006-12-4   16:11
* FileName:  KXMLItemAccess.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KXMLItemAccess_H_
#define __KXMLItemAccess_H_


#include "KXMLBasic.h"

_KAN_DOM_BEGIN

class KXMLTextReader
{
public:
    KXMLTextReader(KXMLManager* pXMLManager, KXMLHANDLE& hXMLCursor):
        m_pXMLManager(pXMLManager),
        m_hXMLCursor(hXMLCursor)
    {
    }

    int Read(std::string& strText) const
    {
        assert(m_pXMLManager);

        if (!m_pXMLManager)
        {
            return E_KXML_XMLMANAGER_EXPECTED;
        }

        if (!m_hXMLCursor.Node())
        {
            return E_KXML_CURRENT_NODE_IS_NULL;
        }

        // tinyXML 在读取空字符串的时候会失败
        // 所以这里不检查返回值
        m_pXMLManager->GetNodeText(m_hXMLCursor.Node(), &strText);
        //int xmlRet = m_pXMLManager->GetNodeText(m_hXMLCursor.Node(), &strText);
        //if (!xmlRet)
        //{
        //    return E_KXML_FAIL_TO_GET_NODE_TEXT;
        //}

        return S_OK;
    }

private:
    mutable KXMLManager* m_pXMLManager;
    mutable KXMLHANDLE   m_hXMLCursor;
};




class KXMLTextWriter
{
public:
    KXMLTextWriter(KXMLManager* pXMLManager, KXMLHANDLE& hXMLCursor):
        m_pXMLManager(pXMLManager),
        m_hXMLCursor(hXMLCursor)
    {
    }

    int Write(const std::string& strText) const
    {
        assert(m_pXMLManager);

        if (!m_pXMLManager)
        {
            return E_KXML_XMLMANAGER_EXPECTED;
        }

        if (!m_hXMLCursor.Node())
        {
            return E_KXML_CURRENT_NODE_IS_NULL;
        }

        int xmlRet = m_pXMLManager->SetText(strText, m_hXMLCursor.Node());
        if (!xmlRet)
        {
            return E_KXML_FAIL_TO_SET_NODE_TEXT;
        }

        return 0;
    }

private:
    mutable KXMLManager* m_pXMLManager;
    mutable KXMLHANDLE   m_hXMLCursor;
};






class KXMLAttributeReader
{
public:
    KXMLAttributeReader(
        KXMLManager*    pXMLManager,
        KXMLHANDLE&     hXMLCursor,
        const char*     pcszAttName
    ):
        m_pXMLManager(pXMLManager),
        m_hXMLCursor(hXMLCursor),
        m_strAttName(pcszAttName)
    {
    }

    int Read(std::string& strText) const
    {
        if (!m_pXMLManager || !m_hXMLCursor.Node())
        {
            return -1;
        }

        int xmlRet = m_pXMLManager->GetAttributeValue(
            m_hXMLCursor.Node(),
            m_strAttName,
            &strText
        );
        if (!xmlRet)
        {
            return -1;
        }

        return 0;
    }

private:
    mutable KXMLManager* m_pXMLManager;
    mutable KXMLHANDLE   m_hXMLCursor;
    mutable std::string  m_strAttName;
};




class KXMLAttributeWriter
{
public:
    KXMLAttributeWriter(
        KXMLManager*    pXMLManager,
        KXMLHANDLE&     hXMLCursor,
        const char*     pcszAttName
    ):
        m_pXMLManager(pXMLManager),
        m_hXMLCursor(hXMLCursor),
        m_strAttName(pcszAttName)
    {
    }

    int Write(const std::string& strText) const
    {
        if (!m_pXMLManager || !m_hXMLCursor.Node())
        {
            return -1;
        }

        int xmlRet = m_pXMLManager->SetAttribute(
            m_strAttName,
            strText,
            m_hXMLCursor.Node()
        );
        if (!xmlRet)
        {
            return -1;
        }

        return 0;
    }

private:
    mutable KXMLManager* m_pXMLManager;
    mutable KXMLHANDLE   m_hXMLCursor;
    mutable std::string  m_strAttName;
};

_KAN_DOM_END


#endif//__KXMLItemAccess_H_
