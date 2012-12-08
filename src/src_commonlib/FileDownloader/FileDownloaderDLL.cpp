#include "KInetFileDownload.h"

#ifdef EXPORTS_USE_DLL
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    hModule; ul_reason_for_call; lpReserved;
    return TRUE;
}

/**
 * Used to determine whether the DLL can be unloaded by OLE
 */
STDAPI DllCanUnloadNow(void)
{
    return S_OK;
}

/**
 * Returns a class factory to create an object of the requested type
 */
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    rclsid;
    if (NULL == ppv)
    {
        return E_INVALIDARG;
    }

    if(__uuidof(IFileDownloader) == riid)
    {
        IFileDownloader* pi = new CFileDownloader();
        if (NULL == pi)
        {
            return E_FAIL;
        }

        pi->AddRef();
        *ppv = static_cast<LPVOID>(pi);
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}

#endif //EXPORTS_USE_DLL

bool CreateFileDownloaderFromStaticLib(OUT IFileDownloader** ppFileDownloader)
{
    if(NULL == ppFileDownloader)
        return false;

    *ppFileDownloader = new CFileDownloader();
    if(NULL == *ppFileDownloader)
        return false;

    (*ppFileDownloader)->AddRef();
    return true;
}
