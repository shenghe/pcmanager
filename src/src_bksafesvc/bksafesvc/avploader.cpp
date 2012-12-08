/**
* @file    avploader.cpp
* @brief   ...
* @author  bbcallen
* @date    2010-04-09 14:30
*/

#include "stdafx.h"
#include "avploader.h"

#include "winmod\winpath.h"

NS_SKYLARK_USING

HMODULE g_hMod_bkeng    = NULL;
HMODULE g_hMod_bkatr    = NULL;
HMODULE g_hMod_bkbak    = NULL;
HMODULE g_hMod_bkdb     = NULL;
HMODULE g_hMod_bkcache  = NULL;

EXTERN_C HRESULT WINAPI BKEngLoad(LPCWSTR lpszEngineDir)
{
    if (g_hMod_bkeng)
        return S_FALSE;


    CWinPath pathEngineDir;
    if (lpszEngineDir)
    {
        pathEngineDir = lpszEngineDir;
    }
    else
    {
        pathEngineDir.GetModuleFileName((HMODULE)&__ImageBase);
        pathEngineDir.RemoveFileSpec();
    }


    pathEngineDir.AddBackslash();
    g_hMod_bkeng = ::LoadLibrary(pathEngineDir.m_strPath + BKMOD_NAME_BKENG);
    if (!g_hMod_bkeng)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;

    
    g_hMod_bkatr    = ::LoadLibrary(pathEngineDir.m_strPath + BKMOD_NAME_BKATRUN);
    g_hMod_bkbak    = ::LoadLibrary(pathEngineDir.m_strPath + BKMOD_NAME_BKBAK);
    g_hMod_bkdb     = ::LoadLibrary(pathEngineDir.m_strPath + BKMOD_NAME_BKDB);
    g_hMod_bkcache  = ::LoadLibrary(pathEngineDir.m_strPath + BKMOD_NAME_BKCACHE);


    return S_OK;
}

EXTERN_C HRESULT WINAPI BKEngUnload()
{
    if (!g_hMod_bkeng)
        return S_FALSE;


    ::FreeLibrary(g_hMod_bkeng);
    g_hMod_bkeng = NULL;
    return S_OK;
}

EXTERN_C HRESULT WINAPI BKEngInitialize(Skylark::BKENG_PLATFORM_INIT* pInit)
{
    if (!g_hMod_bkeng)
        return E_HANDLE;

    
    PFN_BKEngInitialize pfn = (PFN_BKEngInitialize)::GetProcAddress(g_hMod_bkeng, FN_BKEngInitialize);
    if (!pfn)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


    return pfn(pInit);
}

EXTERN_C HRESULT WINAPI BKEngUninitialize()
{
    if (!g_hMod_bkeng)
        return E_HANDLE;


    PFN_BKEngUninitialize pfn = (PFN_BKEngUninitialize)::GetProcAddress(g_hMod_bkeng, FN_BKEngUninitialize);
    if (!pfn)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


    return pfn();
}

EXTERN_C HRESULT WINAPI BKEngCreateObject(REFIID riid, void** ppvObj, DWORD dwBKEngVer)
{
    if (!g_hMod_bkeng)
        return E_HANDLE;


    PFN_BKEngCreateObject pfn = (PFN_BKEngCreateObject)::GetProcAddress(g_hMod_bkeng, FN_BKEngCreateObject);
    if (!pfn)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


    return pfn(riid, ppvObj, dwBKEngVer);
}

EXTERN_C HRESULT WINAPI BKAtrunCreateObject(REFIID riid, void** ppvObj, DWORD dwBKEngVer)
{
    if (!g_hMod_bkatr)
        return E_HANDLE;


    PFN_BKAtrunCreateObject pfn = (PFN_BKAtrunCreateObject)::GetProcAddress(g_hMod_bkatr, FN_BKAtrunCreateObject);
    if (!pfn)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


    return pfn(riid, ppvObj, dwBKEngVer);
}

EXTERN_C HRESULT WINAPI BKBakCreateObject(REFIID riid, void** ppvObj, DWORD dwBKEngVer)
{
    if (!g_hMod_bkbak)
        return E_HANDLE;


    PFN_BKBakCreateObject pfn = (PFN_BKBakCreateObject)::GetProcAddress(g_hMod_bkbak, FN_BKBakCreateObject);
    if (!pfn)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


    return pfn(riid, ppvObj, dwBKEngVer);
}

EXTERN_C HRESULT WINAPI BKCacheCreateObject(REFIID riid, void** ppvObj, DWORD dwBKEngVer)
{
    if (!g_hMod_bkcache)
        return E_HANDLE;


    PFN_BKCacheCreateObject pfn = (PFN_BKCacheCreateObject)::GetProcAddress(g_hMod_bkcache, FN_BKCacheCreateObject);
    if (!pfn)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


    return pfn(riid, ppvObj, dwBKEngVer);
}

EXTERN_C HRESULT WINAPI BKDbCreateObject(REFIID riid, void** ppvObj, DWORD dwBKEngVer)
{
    if (!g_hMod_bkdb)
        return E_HANDLE;


    PFN_BKDbCreateObject pfn = (PFN_BKDbCreateObject)::GetProcAddress(g_hMod_bkdb, FN_BKDbCreateObject);
    if (!pfn)
        return GetLastError() ? AtlHresultFromLastError() : E_FAIL;


    return pfn(riid, ppvObj, dwBKEngVer);
}
