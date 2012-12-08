/********************************************************************
//	FileName	:	KAVCOM.cpp
//	Version		:	1.0
//	Author		:	Chen Rui
//	Date		:	2004-1-30	22:43	--	Created.
//	Comment		:	1.0 :   Mini-COM implement for KAV7.
*********************************************************************/
#include "stdafx.h"
#include "KAVPublic.h"
#include "KAVCOM.h"
#include "assert.h"


typedef HRESULT STDAPICALLTYPE FN_DLL_CAN_UNLOAD_NOW(void);
typedef HRESULT STDAPICALLTYPE FN_DLL_GET_CLASS_OBJECT(REFCLSID rclsid, REFIID riid, LPVOID* ppv);

#define FN_DLL_CAN_UNLOAD_NOW_NAME          "DllCanUnloadNow"
#define FN_DLL_GET_CLASS_OBJECT_NAME        "DllGetClassObject"

STDAPI KAVCoGetClassObject(
    /* [in]  */ HMODULE     hModule, 
    /* [in]  */ REFIID      riid, 
    /* [out] */ void* FAR*  ppv
)
{
    HRESULT krResult  = E_FAIL;
    HRESULT krRetCode = E_FAIL;

    void*   pv        = NULL;

    FN_DLL_GET_CLASS_OBJECT *pfnDllGetClassObject = NULL;

    CLSID   clsid     = { 0 };          // just for compability of MS-COM

    assert(hModule);
    pfnDllGetClassObject 
        = (FN_DLL_GET_CLASS_OBJECT *)GetProcAddress(hModule, FN_DLL_GET_CLASS_OBJECT_NAME);
    KAV_PROCESS_ERROR(pfnDllGetClassObject);

    krRetCode = pfnDllGetClassObject(clsid, riid, &pv);
    KAV_COM_PROC_ERR_RET_CONDITION(krRetCode);

    krResult = S_OK;

Exit0:
    if ((ppv))
        *ppv = pv;

    return krResult;
}

STDAPI KAVCoRelease(
    /* [in ] */ LPVOID pv
)
{
    return _CoCanUnloadNow(pv);
}


STDAPI KAVDllCoGetClassObject(
    /* [in]  */ const TCHAR cszDllName[], 
    /* [in]  */ REFIID      riid, 
    /* [out] */ HMODULE*    phModule,
    /* [out] */ void* FAR*  ppv
)
{
    HRESULT krResult  = E_FAIL;
    HRESULT krRetCode = E_FAIL;

    HMODULE hModule   = NULL;
    void*   pv        = NULL;

    FN_DLL_GET_CLASS_OBJECT *pfnDllGetClassObject = NULL;

    CLSID   clsid     = { 0 };          // just for compability of MS-COM

    assert(cszDllName);
    hModule  = LoadLibrary(cszDllName);
    KAV_PROCESS_ERROR(hModule);

    pfnDllGetClassObject 
        = (FN_DLL_GET_CLASS_OBJECT *)GetProcAddress(hModule, FN_DLL_GET_CLASS_OBJECT_NAME);
    KAV_PROCESS_ERROR(pfnDllGetClassObject);

    krRetCode = pfnDllGetClassObject(clsid, riid, &pv);
    KAV_COM_PROC_ERR_RET_CONDITION(krRetCode);

    krResult = S_OK;

Exit0:
    if (FAILED(krResult))
    {
        if (hModule)
        {
            FreeLibrary(hModule);
            hModule = NULL;
        }
    }

    if ((phModule))   *phModule   = hModule;
    if ((ppv))        *ppv        = pv;

    return krResult;
}

STDAPI KAVDllCoUnload(
    /* [in]  */ HMODULE     hModule,
    /* [in]  */ void*       pv
)
{
    HRESULT hr = E_FAIL;
    FN_DLL_CAN_UNLOAD_NOW *pfnDllCanUnloadNow = NULL;

    if (hModule)
    {
        pfnDllCanUnloadNow
            = (FN_DLL_CAN_UNLOAD_NOW *)GetProcAddress(hModule, FN_DLL_CAN_UNLOAD_NOW_NAME);    

        hr = _CoCanUnloadNow(pv);
        if (S_OK == hr)
        {
            // if DllCanUnloadNow() is exported, call it and exam its return value.
            if (pfnDllCanUnloadNow)
                hr = pfnDllCanUnloadNow();
        }
        
        if (SUCCEEDED(hr))
        {
            FreeLibrary(hModule);
            hModule = NULL;

            return S_OK;
        }
    }

    return E_FAIL;
}
