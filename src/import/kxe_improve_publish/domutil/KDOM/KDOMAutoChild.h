/********************************************************************
* CreatedOn: 2006-12-11   17:58
* FileName:  KDOMAutoChild.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef __KDOMAutoChild_H_
#define __KDOMAutoChild_H_

#include "KDOMBasic.h"

_KAN_DOM_BEGIN

template<class _KDOMCursor>
class KDOMAutoChildEx
{
public:
    KDOMAutoChildEx(const char* pcszChildName, _KDOMCursor* pDOMCursor, bool bWrite):
        m_pDOMCursor(pDOMCursor),
        m_bWrite(bWrite)
    {
        assert(pDOMCursor);
        if (m_pDOMCursor)
        {
            m_pDOMCursor->BeginChild(pcszChildName, bWrite);
        }
    }

    ~KDOMAutoChildEx()
    {
        if (m_pDOMCursor)
        {
            m_pDOMCursor->EndChild();
        }
    }

    int GetLastError()
    {
        return m_nLastError;
    }

private:
    KDOMAutoChildEx(const KDOMAutoChildEx&);
    KDOMAutoChildEx& operator=(const KDOMAutoChildEx&);



    int         m_nLastError;
    bool        m_bWrite;
    _KDOMCursor* m_pDOMCursor;
};



template<class _KDOMCursor>
class KDOMAutoBrotherEx
{
public:
    KDOMAutoBrotherEx(_KDOMCursor* pDOMCursor, bool bWrite):
        m_pDOMCursor(pDOMCursor),
        m_bWrite(bWrite)
    {
        assert(pDOMCursor);
        if (m_pDOMCursor)
        {
            m_pDOMCursor->BeginFirstChild(bWrite);
        }
    }

    ~KDOMAutoBrotherEx()
    {
        if (m_pDOMCursor)
        {
            m_pDOMCursor->EndChild();
        }
    }

    void MoveToNextBrother()
    {
        if (m_pDOMCursor)
        {
            m_pDOMCursor->ToNextBrother();
        }
    }

    bool NoMoreBrother()
    {
        if (m_pDOMCursor)
        {
            return m_pDOMCursor->NoMoreBrother();
        }

        return true;
    }

    int GetLastError()
    {
        return m_nLastError;
    }

private:
    KDOMAutoBrotherEx(const KDOMAutoBrotherEx&);
    KDOMAutoBrotherEx& operator=(const KDOMAutoBrotherEx&);



    int         m_nLastError;
    bool        m_bWrite;
    _KDOMCursor* m_pDOMCursor;
};


_KAN_DOM_END

#endif//__KDOMAutoChild_H_
