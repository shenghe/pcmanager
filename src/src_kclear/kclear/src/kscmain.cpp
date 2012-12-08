#include "stdafx.h"
#include "kscmain.h"
#include "kscmaindlg.h"
#include "kscbase/kscfilepath.h"
#include "kscbase/kscres.h"
#include <GdiPlus.h>

//////////////////////////////////////////////////////////////////////////

CKscModule _Module;
CAppModule* _ModulePtr = &_Module;

//////////////////////////////////////////////////////////////////////////

void CKscModule::_InitUIResource()
{
	KAppRes& appRes = KAppRes::Instance();
	std::string strXml;

	KuiFontPool::SetDefaultFont(_T("ËÎÌו"), -12);

	appRes.GetXmlData("IDR_KSC_SKIN", strXml);
	KuiSkin::LoadSkins(strXml);

	appRes.GetXmlData("IDR_KSC_STYLE", strXml);
	KuiStyle::LoadStyles(strXml);
}

HRESULT CKscModule::Init(HINSTANCE hInstance)
{
	KFilePath appPath = KFilePath::GetFilePath(NULL);
	DWORD dwRet = 1;
	HRESULT hRet;
	CRegKey reg;
	
	hRet = __super::Init(NULL, hInstance);
	if (FAILED(hRet))
		return hRet;

    GetProductVersion();
	GetFileVersion();
	
	//Get install path
	DWORD dwRet1 = ::GetModuleFileName(NULL, m_strAppFileName.GetBuffer(MAX_PATH + 1), MAX_PATH);
	if (0 == dwRet1)
	{
		m_strAppFileName.ReleaseBuffer(0);

		hRet = E_FAIL;
	}
	else
	{
		m_strAppFileName.ReleaseBuffer();

		m_strAppPath = m_strAppFileName;
		//m_strAppPath.Truncate(m_strAppPath.ReverseFind(L'\\') + 1);
		m_strAppPath = m_strAppPath.Left(m_strAppPath.ReverseFind(L'\\'));
	}

	appPath.RemoveFileSpec();

    _InitUIResource();

	return hRet;
}

VOID CKscModule::UnInit()
{
}

//////////////////////////////////////////////////////////////////////////


void CKscModule::_GetFileInfo(BYTE *pbyInfo, LPCWSTR lpszFileInfoKey, LPWSTR lpszFileInfoRet)
{
    BOOL bRet = FALSE;
    UINT uCount = 0;
    UINT uRetSize = MAX_PATH - 1;
    LPWSTR lpszValue = NULL;

    struct LANGANDCODEPAGE
    {
        WORD wLanguage;
        WORD wCodePage;
    } *lpTranslate;

    bRet = ::VerQueryValue(pbyInfo, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &uCount);
    if (!bRet || uCount == 0)
        return;

    CString strSubBlock;

    strSubBlock.Format(
        L"\\StringFileInfo\\%04x%04x\\%s",
        lpTranslate[0].wLanguage,
        lpTranslate[0].wCodePage, 
        lpszFileInfoKey
        );
    bRet = ::VerQueryValue(
        pbyInfo, (LPWSTR)(LPCWSTR)strSubBlock, 
        (LPVOID *)&lpszValue, &uRetSize
        );
    if (!bRet)
        return;

    wcsncpy(lpszFileInfoRet, lpszValue, wcslen(lpszValue) + 1);
}


BOOL CKscModule::_GetPEProductVersion(LPCWSTR lpszFileName, CString &strProductVersion)
{
    BOOL bResult = FALSE;
    BOOL bRet = FALSE;
    DWORD dwHandle          = 0;
    DWORD dwFileInfoSize    = 0;
    VS_FIXEDFILEINFO *pFixFileInfo = NULL;
    BYTE *pbyInfo           = NULL;

    dwFileInfoSize = ::GetFileVersionInfoSize(lpszFileName, &dwHandle);
    if (0 == dwFileInfoSize)
        goto Exit0;

    pbyInfo = new BYTE[dwFileInfoSize];
    if (!pbyInfo)
        goto Exit0;

    bRet = ::GetFileVersionInfo(lpszFileName, dwHandle, dwFileInfoSize, pbyInfo);
    if (!bRet)
        goto Exit0;

    _GetFileInfo(pbyInfo, L"ProductVersion", strProductVersion.GetBuffer(MAX_PATH + 1));
    strProductVersion.ReleaseBuffer();

    bResult = TRUE;

Exit0:

    if (pbyInfo)
    {
        delete[] pbyInfo;
        pbyInfo = NULL;
    }

    return bResult;
}

BOOL CKscModule::_GetPEFileVersion(LPCWSTR lpszFileName, CString &strFileVersion)
{
	BOOL bResult = FALSE;
	BOOL bRet = FALSE;
	DWORD dwHandle          = 0;
	DWORD dwFileInfoSize    = 0;
	VS_FIXEDFILEINFO *pFixFileInfo = NULL;
	BYTE *pbyInfo           = NULL;

	dwFileInfoSize = ::GetFileVersionInfoSize(lpszFileName, &dwHandle);
	if (0 == dwFileInfoSize)
		goto Exit0;

	pbyInfo = new BYTE[dwFileInfoSize];
	if (!pbyInfo)
		goto Exit0;

	bRet = ::GetFileVersionInfo(lpszFileName, dwHandle, dwFileInfoSize, pbyInfo);
	if (!bRet)
		goto Exit0;

	_GetFileInfo(pbyInfo, L"FileVersion", strFileVersion.GetBuffer(MAX_PATH + 1));
	strFileVersion.ReleaseBuffer();

	bResult = TRUE;

Exit0:

	if (pbyInfo)
	{
		delete[] pbyInfo;
		pbyInfo = NULL;
	}

	return bResult;
}


BOOL CKscModule::GetProductVersion()
{
    BOOL retval = FALSE;
    KFilePath appPath = KFilePath::GetFilePath(NULL);
    BOOL fRetCode;
    int i;

    fRetCode = _GetPEProductVersion(appPath, m_strAppVersion);
    if (!fRetCode)
        goto clean0;

    for (i = 0; i < m_strAppVersion.GetLength(); ++i)
    {
        if (m_strAppVersion[i] == _T(','))
            m_strAppVersion.SetAt(i, _T('.'));
    }

    retval = TRUE;

clean0:
    return retval;
}

BOOL CKscModule::GetFileVersion()
{
	BOOL retval = FALSE;
	KFilePath appPath = KFilePath::GetFilePath(NULL);
	BOOL fRetCode;
	int i;

	fRetCode = _GetPEFileVersion(appPath, m_strFileVersion);
	if (!fRetCode)
		goto clean0;

	for (i = 0; i < m_strFileVersion.GetLength(); ++i)
	{
		if (m_strFileVersion[i] == _T(','))
			m_strFileVersion.SetAt(i, _T('.'));
	}

	retval = TRUE;

clean0:
	return retval;
}


LPCTSTR CKscModule::GetAppVersion()
{
    return m_strAppVersion;
}

LPCTSTR CKscModule::GetAppFileVersion()
{
	return m_strFileVersion;
}

LPCTSTR CKscModule::GetAppFilePath()
{
	return m_strAppFileName;
}

LPCTSTR CKscModule::GetAppDirPath()
{
	return m_strAppPath;
}

//////////////////////////////////////////////////////////////////////////
