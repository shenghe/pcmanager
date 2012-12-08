#ifndef ACF_MODULE_INC_
#define ACF_MODULE_INC_

//////////////////////////////////////////////////////////////////////////

#include "acf_support.h"

//////////////////////////////////////////////////////////////////////////

#define KS_IMP_FILTMODULE(ClassName)    \
int __stdcall ModuleMain(int a1, int a2, int a3)    \
{   \
    (a1 = a1);  \
    (a2 = a2);  \
    (a3 = a3);  \
    return 1;   \
}   \
extern "C" __declspec(dllexport)    \
IFiltSupport*   KsSupport = 0;    \
extern "C" __declspec(dllexport)    \
long __cdecl KsGetFilter(    \
    REFIID riid,    \
    void** ppvObject)   \
{   \
    if (!ppvObject)   \
        return E_FAIL;  \
    ClassName* pInstance = new ClassName();   \
    if (!pInstance) \
        return E_OUTOFMEMORY;   \
    long hr = pInstance->QueryInterface(riid, ppvObject);    \
    if (hr != S_OK) \
    {   \
        delete pInstance;   \
        pInstance = 0;   \
    }   \
    return hr;  \
}

//////////////////////////////////////////////////////////////////////////

#endif  // !ACF_MODULE_INC_

