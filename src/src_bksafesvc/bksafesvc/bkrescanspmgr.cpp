/** 
* @file     BKRescanSpMgr.h
* @author   jiaoyafei
* @date     2010-12-10
* @brief    管理rescansp的类
*/
#include "stdafx.h"

#include "bkrescanspmgr.h"


#define PATH_COMMSP L"kse\\ksbcommsp.dll"

typedef HRESULT STDMETHODCALLTYPE fnDllGetClassObject(REFCLSID, REFIID, LPVOID*);

HRESULT CBKRescanSpMgr::Initialize(IReScanCallback* pIRescanCallback /*= NULL*/)
{
    fnDllGetClassObject* pfnDllGetClassObject = NULL;

    HRESULT hrRC = S_OK;
    int nResult = 0;  

    if (m_bInit)
    {
        return S_OK;
    }

	wchar_t pwszPath[MAX_PATH * 4] = {0};

	if (FAILED(_GetFilePath(pwszPath, PATH_COMMSP, MAX_PATH * 4)) || (0 == pwszPath[0]))
	{
		return E_FAIL;
	}

    m_hCommSPModule = LoadLibrary(pwszPath);
    if (NULL == m_hCommSPModule)
    {
        goto Exit0;
    }

    pfnDllGetClassObject = (fnDllGetClassObject *)(::GetProcAddress(m_hCommSPModule, "KSDllGetClassObject"));
    if (NULL == pfnDllGetClassObject)
    {
        goto Exit0;
    }

    hrRC = pfnDllGetClassObject(CLSID_CKRescanSP, __uuidof(IKRescanSPManager), (LPVOID *)&m_piReScanSP);
    if (FAILED(hrRC) || (NULL == m_piReScanSP))
    {
        goto Exit0;
    }

    nResult = m_piReScanSP->InitializeService(pIRescanCallback);
    if (0 != nResult)
    {
        goto Exit0;
    }

	m_bInit = true;

    return S_OK;

Exit0:
    Uninitialize();

    return E_FAIL;
}

HRESULT CBKRescanSpMgr::Uninitialize()
{
    if (m_piReScanSP)
    {
        m_piReScanSP->StopService();

        if (0 != m_piReScanSP->UninitializeService())
        {
            return E_FAIL;
        }
        m_piReScanSP = NULL;
    }

    if (m_hCommSPModule)
    {
        if (0 == FreeLibrary(m_hCommSPModule))
        {
            return E_FAIL;
        }
        m_hCommSPModule = NULL;
    }

	m_bInit = false;

    return S_OK;
}

HRESULT CBKRescanSpMgr::StartRescanSP()
{   
    if (NULL == m_piReScanSP)
    {
        return E_FAIL;
    }

    if (0 != m_piReScanSP->StartService())
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CBKRescanSpMgr::StopRescanSP()
{
    if (NULL == m_piReScanSP)
    {
        return E_FAIL;
    }

    if (0 != m_piReScanSP->StopService())
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CBKRescanSpMgr::_GetFilePath(wchar_t* pwszFilePath, wchar_t* pwszFileName, int nLen)
{
	if (NULL == pwszFilePath || NULL == pwszFileName)
	{
		return E_INVALIDARG;
	}

	if (0 == GetModuleFileName((HMODULE)(&__ImageBase), pwszFilePath, nLen - 1 - MAX_PATH))
	{
		return E_FAIL;
	}

	if (!PathRemoveFileSpec(pwszFilePath) ||
		NULL == PathAddBackslash(pwszFilePath) ||
		!PathAppend(pwszFilePath, pwszFileName))
	{
		return E_FAIL;
	}

	return S_OK;
}