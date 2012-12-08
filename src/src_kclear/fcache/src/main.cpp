#include "stdafx.h"
#include "fcacheimp.h"

//////////////////////////////////////////////////////////////////////////

HINSTANCE g_hInstance;

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	g_hInstance = hInstance;
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////

BOOL __cdecl GetFCache(REFIID riid, void** ppvObject, char cVol)
{
    BOOL retval = FALSE;
    CFCacheImpl* pInstance = NULL;
    HRESULT hRetCode;

    pInstance = new CFCacheImpl(cVol);
    if (!pInstance)
        goto clean0;

    hRetCode = pInstance->QueryInterface(riid, ppvObject);
    if (FAILED(hRetCode))
        goto clean0;

    retval = TRUE;

clean0:
    if (!retval)
    {
        if (pInstance)
        {
            delete pInstance;
            pInstance = NULL;
        }
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////
