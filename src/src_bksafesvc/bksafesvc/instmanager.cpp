/**
* @file    instmanager.cpp
* @brief   ...
* @author  bbcallen
* @date    2010-03-24 16:05
*/

#include "stdafx.h"
#include "instmanager.h"

#include "_idl_gen\bksafesvc.h"
#include "comproxy\bkutility.h"

HRESULT CInstManager::LoadAvEngine()
{
    CoInitialize(NULL);

    CBkUtility utility;
    HRESULT hr = utility.Initialize();
    if (SUCCEEDED(hr))
    {
        utility.EnableAvEngine();
    }

    CoUninitialize();

    return hr;
}


HRESULT CInstManager::UnloadAvEngine()
{
    CoInitialize(NULL);

    CBkUtility utility;
    HRESULT hr = utility.Initialize();
    if (SUCCEEDED(hr))
    {
        utility.DisableAvEngine();
    }

    CoUninitialize();

    return hr;
}


HRESULT CInstManager::ReloadAvSign()
{
    CoInitialize(NULL);

    CBkUtility utility;
    HRESULT hr = utility.Initialize();
    if (SUCCEEDED(hr))
    {
        utility.ReloadAvSign();
    }

    CoUninitialize();

    return hr;
}

HRESULT CInstManager::UpdateWhiteList()
{
    CoInitialize(NULL);

    CBkUtility utility;
    HRESULT hr = utility.Initialize();
    if (SUCCEEDED(hr))
    {
        utility.WhiteListUpdate();
    }

    CoUninitialize();

    return hr;
}