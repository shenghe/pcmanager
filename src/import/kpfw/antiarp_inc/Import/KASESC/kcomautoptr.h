/********************************************************************
* CreatedOn: 2007-8-24   17:59
* FileName:  KComAutoPtr.h
* CreatedBy: lidengwang <lidengwang@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL:  $
* Purpose:
*********************************************************************/

#ifndef __KAS_KCOMAUTOPTR_H__
#define __KAS_KCOMAUTOPTR_H__
#include <assert.h>

namespace KAS{

    template <class T>
    class _NoAddRefReleaseOnCComPtr : public T
    {
    private:
        STDMETHOD_(ULONG, AddRef)()=0;
        STDMETHOD_(ULONG, Release)()=0;
    };
};

template<class T>
class KComAutoPtr
{
    typedef T _PtrClass;

public:

    KComAutoPtr() throw()
    {
        m_pT = NULL;
    }

    KComAutoPtr(int nNull) throw()
    {
        m_pT = NULL;
    }

    KComAutoPtr(T* lp) throw()
    {
        m_pT = lp;

        if (m_pT != NULL)
            m_pT->AddRef();
    }

public:


    ~KComAutoPtr() throw()
    {
        if (m_pT)
            m_pT->Release();
    }

    operator T*() const throw()
    {
        return m_pT;
    }

    T& operator*() const
    {
        assert(m_pT!=NULL);
        return *m_pT;
    }

    T** operator&() throw()
    {
        assert(m_pT==NULL);

        return &m_pT;
    }

    KAS::_NoAddRefReleaseOnCComPtr<T>* operator->() const throw()
    {
        assert(m_pT != NULL);
        return (KAS::_NoAddRefReleaseOnCComPtr<T>*)m_pT;
    }

    bool operator!() const throw()
    {
        return (m_pT == NULL);
    }
    bool operator<(T* pT) const throw()
    {
        return m_pT < pT;
    }
    bool operator!=(T* pT) const
    {
        return !operator==(pT);
    }
    bool operator==(T* pT) const throw()
    {
        return m_pT == pT;
    }

    void Release() throw()
    {
        T* pTemp = m_pT;
        if (pTemp)
        {
            m_pT = NULL;
            pTemp->Release();
        }
    }

    bool IsEqualObject(IUnknown* pOther) throw()
    {
        if (m_pT == NULL && pOther == NULL)
            return true;	

        if (m_pT == NULL || pOther == NULL)
            return false;	

        CComPtr<IUnknown> punk1;
        CComPtr<IUnknown> punk2;

        m_pT->QueryInterface(__uuidof(IUnknown), (void**)&punk1);
        pOther->QueryInterface(__uuidof(IUnknown), (void**)&punk2);

        return punk1 == punk2;
    }

    void Attach( T* p2) throw()
    {
        if (m_pT)
            m_pT->Release();

        m_pT = p2;
    }

    T* Detach() throw()
    {
        T* pt = m_pT;
        m_pT = NULL;
        return pt;
    }

    HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) throw()
    {
        assert(m_pT == NULL);

        return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&m_pT);
    }

    HRESULT CoCreateInstance(LPCOLESTR szProgID, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) throw()
    {
        CLSID clsid;

        HRESULT hr = CLSIDFromProgID(szProgID, &clsid);
        assert(m_pT == NULL);


        if (SUCCEEDED(hr))
            hr = ::CoCreateInstance(clsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&m_pT);

        return hr;
    }

    template <class Q>
    HRESULT QueryInterface(Q** pp) const throw()
    {
        assert(pp != NULL);
        assert(m_pT != NULL);

        if (pp == NULL || m_pT == NULL)
            return E_FAIL;

        return m_pT->QueryInterface(__uuidof(Q), (void**)pp);
    }

public:

    T* m_pT;
};




#endif