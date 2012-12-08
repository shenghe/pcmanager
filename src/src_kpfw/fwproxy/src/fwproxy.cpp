#include "stdafx.h"
#include <winIoctl.h>
#include "ktdifilt_public.h"
#include "fwproxyimp.h"
#include "fwinstaller.h"

HMODULE g_szModule = NULL;

REGISTER_TO_FACTORY(CFwProxy,"IFwProxy.1");
REGISTER_TO_FACTORY(CFwInstaller,"IFwInstaller.1");

//////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(
    __in HANDLE     hInstance, 
    __in DWORD      dwReason, 
    __in_opt LPVOID lpReserved
    )
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
		{
			g_szModule = (HMODULE)hInstance;
			DisableThreadLibraryCalls((HMODULE)hInstance);
		}
        break;
    case DLL_PROCESS_DETACH:
        break;
    default:
        break;
    }

    return TRUE;
}

DECLARE_DLLEXPORTS();


