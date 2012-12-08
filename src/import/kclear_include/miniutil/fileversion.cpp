#include "stdafx.h"
#include "fileversion.h"
#include "szstr.h"

#pragma comment(lib, "version.lib")

BOOL GetPEFileVersion(LPCTSTR lpszFileName, ULARGE_INTEGER *pullVersion)
{
    BOOL bResult = FALSE, bRet = FALSE;
    DWORD dwHandle          = 0;
    DWORD dwFileInfoSize    = 0;
    VS_FIXEDFILEINFO *pFixFileInfo = NULL;
    BYTE *pbyInfo           = NULL;
    UINT uLen               = 0;

    pullVersion->QuadPart = 0;

    dwFileInfoSize = ::GetFileVersionInfoSize(lpszFileName, &dwHandle);
    if (0 == dwFileInfoSize)
        goto Exit0;

    pbyInfo = new BYTE[dwFileInfoSize];
    if (!pbyInfo)
        goto Exit0;

    bRet = ::GetFileVersionInfo(lpszFileName, dwHandle, dwFileInfoSize, pbyInfo);
    if (!bRet)
        goto Exit0;

    bRet = ::VerQueryValue(pbyInfo, _T("\\"), (LPVOID *)&pFixFileInfo, &uLen);
    if (!bRet)
        goto Exit0;

    if (uLen > 0)
    {
        pullVersion->HighPart = pFixFileInfo->dwFileVersionMS;
        pullVersion->LowPart = pFixFileInfo->dwFileVersionLS;
    }    

    bResult = TRUE;

Exit0:

    if (pbyInfo)
    {
        delete[] pbyInfo;
        pbyInfo = NULL;
    }

    return bResult;
}

BOOL GetNormalFileVersion(LPCTSTR lpszFileName, ULARGE_INTEGER *pullVersion)
{
    BOOL bResult = FALSE, bRet = FALSE;
    FILETIME ftWrite;
    SYSTEMTIME stUTC;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    pullVersion->QuadPart = 0;

    hFile = ::CreateFile(
        lpszFileName, GENERIC_READ, FILE_SHARE_READ, 
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
        );
    if (INVALID_HANDLE_VALUE == hFile)
        goto Exit0;

    bRet = ::GetFileTime(hFile, NULL, NULL, &ftWrite);
    if (!bRet)
        goto Exit0;

    bRet = ::FileTimeToSystemTime(&ftWrite, &stUTC);
    if (!bRet)
        goto Exit0;

    pullVersion->HighPart = (DWORD)MAKELONG(stUTC.wMonth, stUTC.wYear);
    pullVersion->LowPart = (DWORD)MAKELONG(stUTC.wHour * 100 + stUTC.wMinute, stUTC.wDay);

    bResult = TRUE;

Exit0:

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    return bResult;
}

BOOL GetFileVersion(LPCTSTR lpszFileName, ULARGE_INTEGER *pullVersion)
{
    if (!lpszFileName || !pullVersion)
        return FALSE;

    BOOL bRet = GetPEFileVersion(lpszFileName, pullVersion);

/*
    if (!bRet)
        bRet = GetNormalFileVersion(lpszFileName, pullVersion);
*/

    return bRet;
}

BOOL GetFileVersion( LPCTSTR lpszFileName, CString& strVersion )
{
	ULARGE_INTEGER		llver;

	if ( GetFileVersion(lpszFileName,&llver) )
	{
		GetFileVersionString(llver,strVersion);
		return TRUE;
	}
	return FALSE;
}

BOOL GetFileVersionString(ULARGE_INTEGER ullVersion, CString &strVersion)
{
    strVersion.Format(
        _T("%d.%d.%d.%d"), 
        HIWORD(ullVersion.HighPart), LOWORD(ullVersion.HighPart), 
        HIWORD(ullVersion.LowPart), LOWORD(ullVersion.LowPart)
        );

    return TRUE;
}

ULARGE_INTEGER GetDigitalVersionByString(LPCTSTR lpszVersion)
{
    ULARGE_INTEGER ullRet = {0};
    CString strVersion = lpszVersion;
    int nPos[4] = {0, 0, 0, 0};

    for (int i = 1; i < 4; i ++)
    {
        nPos[i] = strVersion.Find(_T('.'), nPos[i - 1]) + 1;
    }

    ullRet.HighPart = (DWORD)MAKELONG(_szttoi(strVersion.Mid(nPos[1])), _szttoi(strVersion));
    ullRet.LowPart = (DWORD)MAKELONG(_szttoi(strVersion.Mid(nPos[3])), _szttoi(strVersion.Mid(nPos[2])));

    return ullRet;
}

BOOL VersionLess(LPCTSTR lpszVersionLeft, LPCTSTR lpszVersionRight)
{
    ULARGE_INTEGER ullVerLeft  = ::GetDigitalVersionByString(lpszVersionLeft);
    ULARGE_INTEGER ullVerRight = ::GetDigitalVersionByString(lpszVersionRight);

    return (ullVerLeft.QuadPart < ullVerRight.QuadPart);
}