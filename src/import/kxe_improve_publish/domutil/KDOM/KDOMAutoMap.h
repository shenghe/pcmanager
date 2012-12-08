/********************************************************************
* CreatedOn: 2007-6-8   11:53
* FileName:  KDOMAutoMap.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef KDOMAutoMap_H_
#define KDOMAutoMap_H_

#include "KDOMBasic.h"
#include "KDOMAutoChild.h"

_KAN_DOM_BEGIN

static const char* s_pcszNS_TmpForMap = "kdom_TmpForMap";

template<class _KDOMCursor>
class KDOMAutoMapEx
{
public:
    KDOMAutoMapEx(_KDOMCursor* pDOMCursor, bool bWrite):
        m_bWrite(bWrite),
        m_pDOMCursor(pDOMCursor)
    {
    }

    ~KDOMAutoMapEx()
    {
    }

    template <class K, class V>
    long SetMapElem(K& key, V& value)
    {
        assert(m_pDOMCursor);
        long lRet;
        if (m_pDOMCursor)
        {
            std::string name;

            {
                KDOMAutoChildEx<_KDOMCursor> autoChild(s_pcszNS_TmpForMap, m_pDOMCursor, true);
                lRet = m_pDOMCursor->SetObjValue(key);
                if (FAILED(lRet))
                {
                    return lRet;
                }

                lRet = m_pDOMCursor->GetObjValue(name);
                if (FAILED(lRet))
                {
                    return lRet;
                }
            }

            lRet = m_pDOMCursor->RemoveChildren(s_pcszNS_TmpForMap);
            if (FAILED(lRet))
            {
                return lRet;
            }

            {
                KDOMAutoChildEx<_KDOMCursor> autoChild(name.c_str(), m_pDOMCursor, true);

                lRet = m_pDOMCursor->SetObjValue(value);
                if (FAILED(lRet))
                {
                    return lRet;
                }
            }
        }

        return S_OK;
    }

    long GetLastError()
    {
        return m_lLastError;
    }

private:
    KDOMAutoMapEx(const KDOMAutoMapEx<_KDOMCursor>&);
    KDOMAutoMapEx& operator=(const KDOMAutoMapEx<_KDOMCursor>&);


    long            m_lLastError;
    bool            m_bWrite;
    _KDOMCursor*    m_pDOMCursor;
};




template<class _KDOMCursor>
class KDOMAutoMapElemEx
{
public:
    KDOMAutoMapElemEx(_KDOMCursor* pDOMCursor, bool bWrite):
        m_pDOMCursor(pDOMCursor),
        m_bWrite(bWrite),
        m_bEnterChild(false),
        m_autoBrother(pDOMCursor, bWrite)
    {
        assert(pDOMCursor);
    }

    ~KDOMAutoMapElemEx()
    {
    }

    template <class K, class V>
    long GetMapElem(K& key, V& value)
    {
        assert(m_pDOMCursor);
        long lRet;
        if (m_pDOMCursor)
        {
            std::string name;

            lRet = m_pDOMCursor->GetObjValue(value);
            if (FAILED(lRet))
            {
                return lRet;
            }

            {
                // transform name from 'string' to 'K'
                lRet = m_pDOMCursor->GetObjName(name);
                if (FAILED(lRet))
                {
                    return lRet;
                }

                lRet = m_pDOMCursor->SetObjValue(name);
                if (FAILED(lRet))
                {
                    return lRet;
                }

                lRet = m_pDOMCursor->GetObjValue(key);
                if (FAILED(lRet))
                {
                    return lRet;
                }
            }

            lRet = m_pDOMCursor->SetObjValue(value);
            if (FAILED(lRet))
            {
                return lRet;
            }
        }

        return S_OK;
    }

    void MoveToNextElem()
    {
        m_autoBrother.MoveToNextBrother();
    }

    bool End()
    {
        return m_autoBrother.NoMoreBrother();
    }

    long GetLastError()
    {
        return m_autoBrother.GetLastError();
    }

private:
    KDOMAutoMapElemEx(const KDOMAutoMapElemEx<_KDOMCursor>&);
    KDOMAutoMapElemEx& operator=(const KDOMAutoMapElemEx<_KDOMCursor>&);



    long            m_lLastError;
    bool            m_bWrite;
    _KDOMCursor*    m_pDOMCursor;
    bool            m_bEnterChild;

    KDOMAutoBrotherEx<_KDOMCursor> m_autoBrother;
};

_KAN_DOM_END


#endif//KDOMAutoMap_H_
