#pragma once

#include <atlstr.h>

BOOL GetPEFileVersion(LPCTSTR lpszFileName, ULARGE_INTEGER *pullVersion);
BOOL GetFileVersion(LPCTSTR lpszFileName, ULARGE_INTEGER *pullVersion);
BOOL GetFileVersionString(ULARGE_INTEGER ullVersion, CString &strVersion);
BOOL GetFileVersion(LPCTSTR lpszFileName, CString& strVersion);
ULARGE_INTEGER GetDigitalVersionByString(LPCTSTR lpszVersion);
BOOL VersionLess(LPCTSTR lpszVersionLeft, LPCTSTR lpszVersionRight);