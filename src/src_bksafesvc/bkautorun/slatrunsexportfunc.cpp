/**
* @file    slatrunsexportfunc.cpp
* @brief   ...
* @author  zhangrui
* @date    2009-04-23  21:41
*/

#include "stdafx.h"

#include <assert.h>
#include "com_s\com\scom.h"
#include "skylark2\slatruns.h"
#include "autorunscanner.h"
#include "autorunentrytool.h"

using namespace Skylark;




EXTERN_C DWORD WINAPI SkylarkAtrGetEntryType(DWORD dwEntryID)
{
    return CAutorunItem::GetEntryTypeByID(dwEntryID);
}


EXTERN_C HRESULT WINAPI SkylarkAtrCreateScanner(SLHANDLE* phScanner)
{
    if (NULL == phScanner)
        return E_POINTER;

    assert(!*phScanner && "*phScanner is supposed to be NULL!");

    CAutorunScanner* pScanner = new CAutorunScanner();
    if (!pScanner)
        return E_OUTOFMEMORY;

    HRESULT hr = pScanner->Initialize();
    if (FAILED(hr))
    {
        delete pScanner;
        return hr;
    }

    *phScanner = (SLHANDLE*)pScanner;
    return S_OK;
}

EXTERN_C void WINAPI SkylarkAtrCloseScanner(SLHANDLE hScanner)
{
    if (NULL == hScanner)
        return;

    CAutorunScanner* pScanner = (CAutorunScanner*) hScanner;

    pScanner->Uninitialize();
    delete pScanner;
}

EXTERN_C void WINAPI SkylarkAtr_EnableFeedDebug(SLHANDLE hScanner, BOOL bEnableFeedDebug)
{
    if (NULL == hScanner)
        return;

    CAutorunScanner* pScanner = (CAutorunScanner*) hScanner;

    return pScanner->EnableFeedDebug(bEnableFeedDebug);
}

EXTERN_C BOOL WINAPI SkylarkAtr_FindFirstEntryItem(SLHANDLE hScanner, DWORD dwEntryID)
{
    if (NULL == hScanner)
        return FALSE;

    CAutorunScanner* pScanner = (CAutorunScanner*) hScanner;

    return pScanner->FindFirstEntryItem(dwEntryID);
}

EXTERN_C BOOL WINAPI SkylarkAtr_FindNextEntryItem(SLHANDLE hScanner)
{
    if (NULL == hScanner)
        return FALSE;

    CAutorunScanner* pScanner = (CAutorunScanner*) hScanner;

    return pScanner->FindNextEntryItem();
}



EXTERN_C HRESULT WINAPI SkylarkAtrEntryItem_Export(SLHANDLE hSrcScanner, SLHANDLE* phDstScanner)
{
    if (NULL == hSrcScanner || NULL == phDstScanner)
        return E_POINTER;

    assert(!*phDstScanner && "*phScanner must be NULL or memory may leak");

    CAutorunScanner* pSrcScanner = (CAutorunScanner*) hSrcScanner;
    CAutorunScanner* pDstScanner = new CAutorunScanner();
    if (!pDstScanner)
        return E_OUTOFMEMORY;

    HRESULT hr = pSrcScanner->ExportTo(*pDstScanner);
    if (FAILED(hr))
    {
        delete pDstScanner;
        return hr;
    }

    *phDstScanner = (SLHANDLE*)pDstScanner;
    return S_OK;
}

EXTERN_C DWORD WINAPI SkylarkAtrEntryItem_GetEntryID(SLHANDLE hScanner)
{
    if (NULL == hScanner)
        return SLATR_None;

    CAutorunScanner* pScanner = (CAutorunScanner*) hScanner;

    return pScanner->GetCurrentEntryID();
}

EXTERN_C HRESULT WINAPI SkylarkAtrEntryItem_CleanEntry(SLHANDLE hScanner)
{
	if (NULL == hScanner)
		return SLATR_None;

	CAutorunScanner* pScanner = (CAutorunScanner*) hScanner;

	return pScanner->CleanCurrentEntry();
}

EXTERN_C DWORD WINAPI SkylarkAtrEntryItem_GetItemType(SLHANDLE hScanner)
{
    if (NULL == hScanner)
        return SLATR_None;

    CAutorunScanner* pScanner = (CAutorunScanner*) hScanner;

    return pScanner->GetCurrentEntryType();
}

EXTERN_C LPCWSTR WINAPI SkylarkAtrEntryItem_GetInformation(SLHANDLE hScanner)
{
    if (NULL == hScanner)
        return L"";

    CAutorunScanner* pScanner = (CAutorunScanner*) hScanner;

    return pScanner->GetInformation();
}




EXTERN_C LPCWSTR WINAPI SkylarkAtrEntryItem_GetFilePathAt(SLHANDLE hScanner, DWORD dwIndex)
{
    if (NULL == hScanner)
        return L"";

    CAutorunScanner* pScanner = (CAutorunScanner*) hScanner;

    return pScanner->GetFilePathAt(dwIndex);
}


EXTERN_C DWORD WINAPI SkylarkAtrEntryItem_GetFileCount(SLHANDLE hScanner)
{
    if (NULL == hScanner)
        return 0;

    CAutorunScanner* pScanner = (CAutorunScanner*) hScanner;

    return pScanner->GetFileCount();
}


