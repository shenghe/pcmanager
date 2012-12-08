/********************************************************************
* CreatedOn: 2007-8-27   11:06
* FileName:  KISCOM_AdvKISComObject.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef KISCOM_AdvKISComObject_H_
#define KISCOM_AdvKISComObject_H_


template < class TInterface > 
class CKISComObject :   public TInterface
{
public:
    // implement abstract methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in ] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR* __RPC_FAR* ppvObject)
    {
        if (!ppvObject)
            return E_POINTER;
        if ( (IID_IUnknown         == riid) ||
            (__uuidof(TInterface) == riid) )
        {
            *ppvObject = static_cast< IUnknown* >(this);
        }
        else                                     
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
        (static_cast< IUnknown* >(*ppvObject))->AddRef();
        return S_OK;
    }
    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return InterlockedIncrement((LONG volatile *)&m_cRef);
    }
    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
        if (0L == InterlockedDecrement((LONG volatile *)&m_cRef))
        {
            delete this;
            return 0L;
        }
        return m_cRef;
    }
    // constructor
    CKISComObject() : m_cRef(1L)
    {
    }
private:
    LONG m_cRef;
};

template < class TInterface1, class TInterface2 > 
class CKISComObject2    :   public TInterface1, public TInterface2
{
public:
    // implement abstract methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in ] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR* __RPC_FAR* ppvObject)
    {
        if (!ppvObject)
            return E_POINTER;
        if ( (IID_IUnknown           == riid)  ||
            (__uuidof(TInterface1)  == riid) 
            (__uuidof(TInterface2)  == riid) )
        {
            *ppvObject = static_cast< IUnknown* >(this);
        }
        else                                     
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
        (static_cast< IUnknown* >(*ppvObject))->AddRef();
        return S_OK;
    }
    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return InterlockedIncrement((LONG volatile *)&m_cRef);
    }
    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
        if (0L == InterlockedDecrement((LONG volatile *)&m_cRef))
        {
            delete this;
            return 0L;
        }
        return m_cRef;
    }
    // constructor
    CKISComObject2() : m_cRef(1L)
    {
    }
private:
    LONG m_cRef;
};


#endif//KISCOM_AdvKISComObject_H_
