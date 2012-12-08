#include "stdafx.h"
#include "antiarp.h"
#include <strsafe.h>

//////////////////////////////////////////////////////////////////////////

CAntiArpHelper::CAntiArpHelper()
    : m_hModule(NULL)
    , m_pDllGetClassObject(NULL)
{

}

CAntiArpHelper::~CAntiArpHelper()
{

}

CAntiArpHelper& CAntiArpHelper::Instance()
{
    static CAntiArpHelper _singleton;
    return _singleton;
}

//////////////////////////////////////////////////////////////////////////

const WCHAR g_cwszAntiArpModule[] = L"kantiarpdevc.dll";

//////////////////////////////////////////////////////////////////////////

int CAntiArpHelper::Initialize()
{
    int retval = false;
    DWORD dwRetCode;
    HRESULT hr;
    WCHAR wszModulePath[MAX_PATH + 1] = { 0 };

    if (m_hModule && m_pDllGetClassObject)
    {
        retval = true;
        goto clean0;
    }

    dwRetCode = GetModuleFileName(NULL, wszModulePath, MAX_PATH);
    if (!dwRetCode)
        goto clean0;

    *(wcsrchr(wszModulePath, L'\\')) = 0L;
    StringCchCat(wszModulePath, MAX_PATH, L"\\");
    hr = StringCchCat(wszModulePath, MAX_PATH, g_cwszAntiArpModule);
    if (FAILED(hr))
        goto clean0;

    m_hModule = LoadLibraryW(wszModulePath);
    if (!m_hModule)
        goto clean0;

    m_pDllGetClassObject = (TDllGetClassObject)
        GetProcAddress(m_hModule, "DllGetClassObject");
    if (!m_pDllGetClassObject)
        goto clean0;

    retval = true;

clean0:
    return retval;
}

void CAntiArpHelper::UnInitialize()
{
    m_pDllGetClassObject = NULL;

    if (m_hModule)
    {
        FreeLibrary(m_hModule);
        m_hModule = NULL;
    }
}

HRESULT CAntiArpHelper::GetInstance(const IID& riid, void** ppvObject)
{
    HRESULT hr = E_INVALIDARG;
    
    if (!ppvObject)
        goto clean0;

    if (!Initialize())
    {
        hr = E_FAIL;
        goto clean0;
    }

    if (__uuidof(IAntiArpDevC) == riid)
    {
        hr = GetInstance((IAntiArpDevC**)ppvObject);
    }
    else if (__uuidof(IArpTableSafeguard) == riid)
    {
        hr = GetInstance((IArpTableSafeguard**)ppvObject);
    }
    else
    {
        hr = E_NOINTERFACE; 
    }

clean0:
    return hr;
}

HRESULT CAntiArpHelper::GetInstance(IAntiArpDevC** ppiAntiArpDevC)
{
    HRESULT hr = E_INVALIDARG;
    CLSID nilClsid = CLSID_NULL;

    if (!ppiAntiArpDevC)
        goto clean0;

    if (!m_pDllGetClassObject)
        goto clean0;

    hr = m_pDllGetClassObject(
        nilClsid, 
        __uuidof(IAntiArpDevC), 
        (void**)ppiAntiArpDevC
        );
    if (FAILED(hr))
        goto clean0;

clean0:
    return hr;
}

HRESULT CAntiArpHelper::GetInstance(IArpTableSafeguard** ppiArpTableSafeguard)
{
    HRESULT hr = E_INVALIDARG;
    CLSID nilClsid;

    if (!ppiArpTableSafeguard)
        goto clean0;

    if (!m_pDllGetClassObject)
        goto clean0;

    hr = m_pDllGetClassObject(
        nilClsid, 
        __uuidof(IArpTableSafeguard), 
        (void**)ppiArpTableSafeguard
        );
    if (FAILED(hr))
        goto clean0;

clean0:
    return hr;
}

//////////////////////////////////////////////////////////////////////////

