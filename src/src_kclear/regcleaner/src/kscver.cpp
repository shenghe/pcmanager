#include "stdafx.h"
#include "kscver.h"
#include "kscbase/kscfilepath.h"
#include "kscbase/kscres.h"

//////////////////////////////////////////////////////////////////////////
#pragma comment(lib,"Version.lib")

//////////////////////////////////////////////////////////////////////////

HRESULT CKscVersion::Init(HINSTANCE hInstance)
{
    KFilePath		appPath = KFilePath::GetFilePath(NULL);
    DWORD			dwRet = 1;
    HRESULT			hRet = ERROR_SUCCESS;

    GetProductVersion();
    return hRet;
}

VOID CKscVersion::UnInit()
{
}

//////////////////////////////////////////////////////////////////////////


void CKscVersion::_GetFileInfo(BYTE *pbyInfo, LPCWSTR lpszFileInfoKey, LPWSTR lpszFileInfoRet)
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


BOOL CKscVersion::_GetPEProductVersion(LPCWSTR lpszFileName, CString &strProductVersion)
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

BOOL CKscVersion::GetProductVersion()
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

LPCTSTR CKscVersion::GetAppVersion()
{
    return m_strAppVersion;
}

