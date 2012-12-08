/********************************************************************
//	FileName	:	KAVCOM.h
//	Version		:	1.0
//	Author		:	Chen Rui
//	Date		:	2004-1-30	22:43	--	Created.
//	Comment		:	1.0 :   Mini-COM implement for KAV7.
*********************************************************************/

#ifndef __KAV_COM_H_
#define __KAV_COM_H_

//  C++ COM support
#ifdef  __cplusplus
//

#include <Unknwn.h>

/*
    Macro definations and template functions for KAVCOM
*/

#define KAV_DECLARE_IUNKNOWN()                                      \
virtual HRESULT STDMETHODCALLTYPE QueryInterface(                   \
    /* [in] */ REFIID riid,                                         \
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);      \
virtual ULONG STDMETHODCALLTYPE AddRef( void);                      \
virtual ULONG STDMETHODCALLTYPE Release( void);                 

#define __KAV_IMPELEMENT_ADDREF_RELEASE(CoClass, m_uRefCount)       \
ULONG STDMETHODCALLTYPE CoClass::AddRef( void)                      \
{                                                                   \
    return ++m_uRefCount;                                           \
}                                                                   \
ULONG STDMETHODCALLTYPE CoClass::Release( void)                     \
{                                                                   \
    --m_uRefCount;                                                  \
    if (0 == m_uRefCount)                                           \
    {                                                               \
        delete this;                                                \
        return 0;                                                   \
    }                                                               \
    return m_uRefCount;                                             \
}

#define KAV_IMPLEMENT_IUNKNOWN(CoInterface, CoClass, m_uRefCount)   \
HRESULT STDMETHODCALLTYPE CoClass::QueryInterface(                  \
    /* [in] */ REFIID riid,                                         \
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)       \
{                                                                   \
    if (!ppvObject)                                                 \
        return E_INVALIDARG;                                        \
    *ppvObject = NULL;                                              \
    if (                                                            \
        (IID_IUnknown           == riid) ||                         \
        (__uuidof(CoInterface)  == riid)                            \
       )                                                            \
    {                                                               \
        *ppvObject = static_cast<void *>(this);                     \
    }                                                               \
    else                                                            \
    {                                                               \
        return E_NOINTERFACE;                                       \
    }                                                               \
    (static_cast<IUnknown *>(*ppvObject))->AddRef();                \
    return S_OK;                                                    \
}                                                                   \
    __KAV_IMPELEMENT_ADDREF_RELEASE(CoClass, m_uRefCount)

#define KAV_IMPLEMENT_IUNKNOWN_2(CoInterface1, CoInterface2, CoClass, m_uRefCount) \
HRESULT STDMETHODCALLTYPE CoClass::QueryInterface(                  \
    /* [in] */ REFIID riid,                                         \
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)       \
{                                                                   \
    if (!ppvObject)                                                 \
        return E_INVALIDARG;                                        \
    *ppvObject = NULL;                                              \
    if (                                                            \
        (IID_IUnknown           == riid) ||                         \
        (__uuidof(CoInterface1) == riid)                            \
       )                                                            \
    {                                                               \
        *ppvObject = static_cast<CoInterface1 *>(this);             \
    }                                                               \
    else if (__uuidof(CoInterface2) == riid)                        \
    {                                                               \
        *ppvObject = static_cast<CoInterface2 *>(this);             \
    }                                                               \
    else                                                            \
    {                                                               \
        return E_NOINTERFACE;                                       \
    }                                                               \
    (static_cast<IUnknown *>(*ppvObject))->AddRef();                \
    return S_OK;                                                    \
}                                                                   \
    __KAV_IMPELEMENT_ADDREF_RELEASE(CoClass, m_uRefCount)

#define KAV_IMPLEMENT_IUNKNOWN_3(CoInterface1, CoInterface2, CoInterface3, CoClass, m_uRefCount) \
HRESULT STDMETHODCALLTYPE CoClass::QueryInterface(                  \
    /* [in] */ REFIID riid,                                         \
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)       \
{                                                                   \
    if (!ppvObject)                                                 \
        return E_INVALIDARG;                                        \
    *ppvObject = NULL;                                              \
    if (                                                            \
        (IID_IUnknown           == riid) ||                         \
        (__uuidof(CoInterface1) == riid)                            \
       )                                                            \
    {                                                               \
        *ppvObject = static_cast<CoInterface1 *>(this);             \
    }                                                               \
    else if (__uuidof(CoInterface2) == riid)                        \
    {                                                               \
        *ppvObject = static_cast<CoInterface2 *>(this);             \
    }                                                               \
    else if (__uuidof(CoInterface3) == riid)                        \
    {                                                               \
        *ppvObject = static_cast<CoInterface3 *>(this);             \
    }                                                               \
    else                                                            \
    {                                                               \
        return E_NOINTERFACE;                                       \
    }                                                               \
    (static_cast<IUnknown *>(*ppvObject))->AddRef();                \
    return S_OK;                                                    \
}                                                                   \
    __KAV_IMPELEMENT_ADDREF_RELEASE(CoClass, m_uRefCount)

inline
HRESULT _CoCanUnloadNow(LPVOID pv)
{
    if (!pv)
        return E_INVALIDARG;
    IUnknown *pi = static_cast<IUnknown *>(pv);
    return ((0 == pi->Release()) ? S_OK : S_FALSE);
}

template <class CoInterface, class CoClass>
HRESULT _CoGetClassObject(REFIID riid, LPVOID* ppv)
{
    if (!ppv)
        return E_INVALIDARG;

    if (__uuidof(CoInterface) == riid)
    {
        CoInterface *pi = new CoClass;
        if (!pi)
            return E_FAIL;
        
        *ppv = static_cast<LPVOID>(pi);
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}
/*
    End of definations for KAVCOM
*/
//
#endif//__cplusplus


/************************************************************************/
/*  Export functions and macros for KAVCOM supports                     */
/************************************************************************/
/*
    1. KAVCoGetClassObject
*/
STDAPI KAVCoGetClassObject(
    /* [in]  */ HMODULE     hModule, 
    /* [in]  */ REFIID      riid, 
    /* [out] */ void* FAR*  ppv
);
/*
    2. KAVCoRelease
*/
STDAPI KAVCoRelease(
    /* [in ] */ LPVOID pv
);
/*
    3. KAVDllCoGetClassObject
*/
STDAPI KAVDllCoGetClassObject(
    /* [in]  */ const TCHAR cszDllName[], 
    /* [in]  */ REFIID      riid, 
    /* [out] */ HMODULE*    phModule,
    /* [out] */ void* FAR*  ppv
);
/*
    4. KAVDllCoUnload
*/
STDAPI KAVDllCoUnload(
    /* [in]  */ HMODULE     hModule,
    /* [in]  */  void FAR*  pv
);

#endif//__KAV_COM_H_