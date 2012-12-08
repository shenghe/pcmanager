/********************************************************************
* CreatedOn: 2007-8-16   11:37
* FileName:  KISCOM_v2.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*   CKISUseComDll_v2 is totally backward compatible with CKISUseComDll.
*
*   CKISUseComDll do not support create COM object by clsid. it just ignore the clsid
*
*********************************************************************/

#ifndef KISCOM_v2_H_
#define KISCOM_v2_H_

#include <unknwn.h>

#include "KISCOM_AdvQueryInterface.h"
#include "KISCOM_AdvRegistery.h"
#include "KISCOM_AdvKISComObject.h"




class CKISUseComDll_v2
{
public:
    typedef HRESULT (STDAPICALLTYPE* PFN_DLL_GET_CLASS_OBJECT)(
        /* [in ] */ REFCLSID rclsid, 
        /* [in ] */ REFIID riid, 
        /* [out, iid_is(riid)] */ LPVOID* ppv);

    typedef HRESULT (STDAPICALLTYPE* PFN_DLL_GET_CLSID)(
        /* [in ] */ LPCWSTR lpProgID, 
        /* [out] */ LPCLSID lpClsid);

    //提供统一的内存分配
    typedef void* (__stdcall *pfnKSCoTaskMemAllocType)(size_t uSize);

    //提供统一的内存回收 
    typedef void (__stdcall *pfnKSCoTaskMemFreeType)(void *pv,  size_t uSize);



    HRESULT STDMETHODCALLTYPE LoadA(
        /* [in, string, max_is(MAX_PATH)] */ const char szFileNameA[])
    {
        if (m_hModule)
            return E_FAIL;
        m_hModule = (szFileNameA ? ::LoadLibraryA(szFileNameA) : NULL);
        return (m_hModule ? S_OK : E_FAIL);
    }

    HRESULT STDMETHODCALLTYPE LoadW(
        /* [in, string, max_is(MAX_PATH)] */ const WCHAR szFileNameW[])
    {
        if (m_hModule)
            return E_FAIL;
        m_hModule = (szFileNameW ? ::LoadLibraryW(szFileNameW) : NULL);
        return (m_hModule ? S_OK : E_FAIL);
    }

    HRESULT STDMETHODCALLTYPE Load(
        /* [in, string, max_is(MAX_PATH)] */ const TCHAR szFileName[])
    {
        if (m_hModule)
            return E_FAIL;
        m_hModule = (szFileName ? ::LoadLibrary(szFileName) : NULL);
        return (m_hModule ? S_OK : E_FAIL);
    }

    HRESULT STDMETHODCALLTYPE Free(void)
    {
        HRESULT hr;
        if (!m_hModule)
            return E_INVALIDARG;
        hr = (::FreeLibrary(m_hModule) ? S_OK : E_FAIL);
        if (SUCCEEDED(hr))
            m_hModule = NULL;
        return hr;
    }

    HMODULE STDMETHODCALLTYPE Attach(HMODULE hModule)
    {
        if (m_hModule == hModule)
            return S_OK;

        Free();

        m_hModule = hModule;

        return m_hModule;
    }

    HMODULE STDMETHODCALLTYPE Detach()
    {
        HMODULE h = m_hModule;

        m_hModule = NULL;

        return h;
    }

    HRESULT STDMETHODCALLTYPE GetClassObject(
        /* [in ] */ REFIID riid, 
        /* [out, iid_is(riid)] */ LPVOID* ppv)
    {
        CLSID clsid = {0};

        return GetClassObject(clsid, riid, ppv);
    }

    HRESULT STDMETHODCALLTYPE GetClassObject(
        /* [in ] */ REFCLSID clsid, 
        /* [in ] */ REFIID riid, 
        /* [out, iid_is(riid)] */ LPVOID* ppv)
    {
        PFN_DLL_GET_CLASS_OBJECT pfnDllGetClassObject;

        if (!ppv)
            return E_POINTER;
        if (!m_hModule)
            return E_INVALIDARG;

        pfnDllGetClassObject 
            = (PFN_DLL_GET_CLASS_OBJECT)GetProcAddress(m_hModule, "DllGetClassObject");

        return (pfnDllGetClassObject ? pfnDllGetClassObject(clsid, riid, ppv) : CO_E_ERRORINDLL);
    }

    HRESULT STDMETHODCALLTYPE GetClassObject(
        /* [in ] */ LPCWSTR lpProgID,
        /* [in ] */ REFIID riid,
        /* [out, iid_is(riid)] */ LPVOID* ppv)
    {
        CLSID clsid;
        HRESULT hr = GetCLSID(m_hModule, lpProgID, &clsid);
        if (FAILED(hr))
        {
            return hr;
        }

        return GetClassObject(clsid, riid, ppv);
    }


    HRESULT STDMETHODCALLTYPE  GetCLSID(
        /* [in ] */ HMODULE hModule,
        /* [in ] */ LPCWSTR lpProgID, 
        /* [out] */ LPCLSID lpClsid)
    {
        PFN_DLL_GET_CLSID pfnDllGetCLSID;

        if (!lpProgID || !lpClsid)
            return E_POINTER;
        if (!hModule)
            return E_INVALIDARG;

        pfnDllGetCLSID 
            = (PFN_DLL_GET_CLSID)::GetProcAddress(hModule, "DllGetCLSID");

        return (pfnDllGetCLSID ? pfnDllGetCLSID(lpProgID, lpClsid) : CO_E_ERRORINDLL);
    }

    HMODULE STDMETHODCALLTYPE GetModule()
    {
        return m_hModule;
    }


    //提供统一的内存分配
    void* __stdcall KSCoTaskMemAlloc(size_t uSize)
    {
        
        pfnKSCoTaskMemAllocType pfnKSCoTaskMemAlloc = NULL;

        pfnKSCoTaskMemAlloc 
            = (pfnKSCoTaskMemAllocType)::GetProcAddress(m_hModule, "KDCoTaskMemAlloc");

        return (m_hModule && pfnKSCoTaskMemAlloc) ? pfnKSCoTaskMemAlloc(uSize) : NULL;
    }

    //提供统一的内存回收 
    void __stdcall KSCoTaskMemFree(void *pv, size_t uSize)
    {

        pfnKSCoTaskMemFreeType pfnKSCoTaskMemFree = NULL;
        pfnKSCoTaskMemFree 
            = (pfnKSCoTaskMemFreeType)::GetProcAddress(m_hModule, "KDCoTaskMemFree");

        if (m_hModule && pfnKSCoTaskMemFree)
            pfnKSCoTaskMemFree(pv, uSize);
    }

    // constructor
    CKISUseComDll_v2() : m_hModule(NULL)
    {
    }

    CKISUseComDll_v2(
        /* [in, string, max_is(MAX_PATH)] */ const TCHAR szFileName[]) : m_hModule(NULL)
    {
        Load(szFileName);
    }

    virtual ~CKISUseComDll_v2()
    {
        //if (m_hModule)
        //    Free();
    }

private:

    HMODULE m_hModule;
};

















#endif//KISCOM_v2_H_
