// beikesafeexam.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "beikesafeexamimpl.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

HRESULT WINAPI BkSafeCreateObject(DWORD dwNeedDllVersion, REFCLSID rclsid, LPVOID *ppiOut)
{
    if (NULL == ppiOut)
        return E_INVALIDARG;

    if (dwNeedDllVersion != BK_SAFEEXAM_DLL_VERSION)
        return ::AtlHresultFromWin32(ERROR_PRODUCT_VERSION);

    if (__uuidof(IBkSafeExaminer) == rclsid || IID_IUnknown == rclsid)
    {
        IBkSafeExaminer *piExaminer = new KComObject<CBkSafeExaminer>;
        if (NULL == piExaminer)
            return E_OUTOFMEMORY;

        piExaminer->AddRef();

        *ppiOut = piExaminer;

        return S_OK;
    }

    return E_NOINTERFACE;
}
