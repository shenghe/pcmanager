/**
* @file    bkdbexportfunc.cpp
* @brief   ...
* @author  bbcallen
* @date    2009-12-14 19:17
*/

#include "stdafx.h"

#include "skylark2\bkdb.h"
#include "sqlitecomdatabase.h"

NS_SKYLARK_USING

EXTERN_C HRESULT WINAPI BKDbInitialize(Skylark::BKDB_PLATFORM_INIT* pInit)
{
    BKInitDebugOutput();
    return S_OK;
}

EXTERN_C HRESULT WINAPI BKDbUninitialize()
{
    return S_OK;
}

EXTERN_C HRESULT WINAPI BKDbCreateObject(REFIID riid, void** ppvObj, DWORD dwBKEngVer)
{
    if (!ppvObj)
        return E_POINTER;

    assert(!*ppvObj && "*ppiObj is supposed to be NULL!");

    IUnknown* piUnknown = NULL;
    HRESULT hr = E_FAIL;
    if (riid == __uuidof(ISQLiteComDatabase3))
    {
        piUnknown = new KComObject<CSQLiteComDatabase>;
    }
    else
    {
        return E_NOTIMPL;
    }



    if (NULL == piUnknown)
        return E_OUTOFMEMORY;


    hr = piUnknown->QueryInterface(riid, ppvObj);
    if (FAILED(hr))
    {
        delete piUnknown;
        return hr;
    }


    return S_OK;
}