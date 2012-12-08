/********************************************************************
* CreatedOn: 2006-12-12   18:35
* FileName:  KDOMAutoArrayElem.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KDOMAutoArrayElem_H_
#define __KDOMAutoArrayElem_H_

#include "KDOMBasic.h"

_KAN_DOM_BEGIN


template<class _KDOMCursor>
class KDOMAutoArrayEx
{
public:
    KDOMAutoArrayEx(_KDOMCursor* pDOMCursor, bool bWrite):
        m_pDOMCursor(pDOMCursor),
        m_bWrite(bWrite),
        m_bEnterChild(false)
    {
        assert(pDOMCursor);
        if (m_pDOMCursor)
        {
            m_pDOMCursor->BeginArray(bWrite);
        }
    }

    ~KDOMAutoArrayEx()
    {
        if (m_pDOMCursor)
        {
            m_pDOMCursor->EndArray();
        }
    }

    long GetLastError()
    {
        return m_lLastError;
    }

private:
    KDOMAutoArrayEx(const KDOMAutoArrayEx&);
    KDOMAutoArrayEx& operator=(const KDOMAutoArrayEx&);



    long            m_lLastError;
    bool            m_bWrite;
    _KDOMCursor*    m_pDOMCursor;
    bool            m_bEnterChild;
};




template<class _KDOMCursor>
class KDOMAutoArrayElemEx
{
public:
    KDOMAutoArrayElemEx(_KDOMCursor* pDOMCursor, bool bWrite):
        m_pDOMCursor(pDOMCursor),
        m_bWrite(bWrite),
        m_bEnterChild(false)
    {
        assert(pDOMCursor);
        if (m_pDOMCursor)
        {
            m_pDOMCursor->BeginFirstArrayElem(bWrite);
        }
    }

    ~KDOMAutoArrayElemEx()
    {
        if (m_pDOMCursor)
        {
            m_pDOMCursor->EndArrayElem();
        }
    }

    void MoveToNext()
    {
        if (m_pDOMCursor)
        {
            m_pDOMCursor->ToNextArrayElem();
        }
    }

    bool End()
    {
        if (m_pDOMCursor)
        {
            return m_pDOMCursor->ArrayElemIsEnd();
        }

        return true;
    }

    long GetLastError()
    {
        return m_lLastError;
    }

private:
    KDOMAutoArrayElemEx(const KDOMAutoArrayElemEx&);
    KDOMAutoArrayElemEx& operator=(const KDOMAutoArrayElemEx&);



    long            m_lLastError;
    bool            m_bWrite;
    _KDOMCursor*    m_pDOMCursor;
    bool            m_bEnterChild;
};

_KAN_DOM_END

#endif//__KDOMAutoArrayElem_H_
