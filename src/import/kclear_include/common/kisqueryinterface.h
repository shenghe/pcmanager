/********************************************************************
* CreatedOn: 2007-8-8   17:31
* FileName:  KISCOM_AdvQueryInterface.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef KISCOM_AdvQueryInterface_H_
#define KISCOM_AdvQueryInterface_H_

#include <assert.h>

class CKISComRefCnt
{
public:
    CKISComRefCnt(): m_lValue(1) {}

    long operator++() 
    {
        return InterlockedIncrement(&m_lValue);
    }

    long operator--() 
    {
        return InterlockedDecrement(&m_lValue);
    }

    long operator=(long lValue)
    {
        return (m_lValue = lValue);
    }

    operator long() const
    {
        return m_lValue;
    }

private:
    long operator++(int);
    long operator--(int);
    volatile long m_lValue;
};


#define IMPLEMENT_KISCOM_REF_OBJECT()                       \
private:                                                    \
    CKISComRefCnt m_cRef;                                   \

#define IMPLEMENT_KISCOM_ADDREF()                           \
public:                                                     \
    virtual ULONG STDMETHODCALLTYPE AddRef(void)            \
    {                                                       \
        return ++m_cRef;                                    \
    }

#define IMPLEMENT_KISCOM_RELEASE()                          \
    virtual ULONG STDMETHODCALLTYPE Release(void)           \
    {                                                       \
        assert(0 != m_cRef);                                \
        if (0 == --m_cRef)                                  \
        {                                                   \
            delete this;                                    \
            return 0;                                       \
        }                                                   \
        return m_cRef;                                      \
    }

#define DECLARE_KISCOM_OBJECT()                             \
public:                                                     \
    IMPLEMENT_KISCOM_REF_OBJECT();                          \
    IMPLEMENT_KISCOM_ADDREF();                              \
    IMPLEMENT_KISCOM_RELEASE();







#define IMPLEMENT_NON_KISCOM_REF_OBJECT()

#define IMPLEMENT_NON_KISCOM_ADDREF()                       \
public:                                                     \
    virtual ULONG STDMETHODCALLTYPE AddRef(void)            \
    {                                                       \
        return 1;                                           \
    }

#define IMPLEMENT_NON_KISCOM_RELEASE()                      \
    virtual ULONG STDMETHODCALLTYPE Release(void)           \
    {                                                       \
        return 1;                                           \
    }

#define DECLARE_NON_KISCOM_OBJECT()                         \
public:                                                     \
    IMPLEMENT_NON_KISCOM_REF_OBJECT();                      \
    IMPLEMENT_NON_KISCOM_ADDREF();                          \
    IMPLEMENT_NON_KISCOM_RELEASE();











#define KISCOM_QUERY_BEGIN()                                    \
public:                                                         \
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(           \
        REFIID riid,                                            \
        void __RPC_FAR* __RPC_FAR* ppv                          \
    )                                                           \
    {                                                           \
        try                                                     \
        {                                                       \
	        if (NULL == ppv)                                    \
	        {                                                   \
	            return E_POINTER;
	
	
#define KISCOM_QUERY_ENTRY(_interface)                          \
	        }                                                   \
	        else if(riid == __uuidof(_interface))               \
	        {                                                   \
	            *ppv = static_cast<_interface*>(this);
	
	
#define KISCOM_QUERY_ENTRY2(_interface, _derived)               \
	        }                                                   \
	        else if(riid == __uuidof(_interface))               \
	        {                                                   \
	            *ppv = static_cast<_interface*>(                \
	                static_cast<_derived*>(this)                \
	            );
	
	
#define KISCOM_QUERY_ENTRY3(_interface, _derived, _child)       \
	        }                                                   \
	        else if(riid == __uuidof(_interface))               \
	        {                                                   \
	            *ppv = static_cast<_interface*>(                \
	                static_cast<_derived*>(                     \
	                    static_cast<_child*>(this)              \
	                )                                           \
	            );
	
	
#define KISCOM_QUERY_ENTRY_REDIRECT_TO(_base_class)             \
	        }                                                   \
	        else if(SUCCEEDED(_base_class::QueryInterface(riid, ppv)))  \
	        {                                                   \
	            return S_OK;
	
	
#define KISCOM_QUERY_END()                                      \
	        }                                                   \
	        else                                                \
	        {                                                   \
	            *ppv = NULL;                                    \
	            return E_NOINTERFACE;                           \
	        }                                                   \
	        this->AddRef();                                     \
	        return S_OK;                                        \
        }                                                       \
        catch (ATL::CAtlException& e)                           \
        {                                                       \
        	return e;                                           \
        }                                                       \
    }



#define KIS_DEFINE_GETCLSID(_clsid) public:                      \
    static const GUID& GetCLSID() {static const GUID clsid = _clsid; return clsid;}

#endif//KISCOM_AdvQueryInterface_H_
