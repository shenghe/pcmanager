#pragma once
#include <Msi.h>
#include "Utils.h"
BOOL ExtractMSP(LPCTSTR szDatabasePath, LPCTSTR pszDir, Files &files);
UINT GetString(MSIHANDLE hRecord, UINT iField, LPTSTR* ppszProperty, DWORD* pcchProperty);
UINT SaveStream(MSIHANDLE hRecord, LPCTSTR pszDir, CString &strFilename);
LPTSTR MakePathForData(LPCVOID pBuffer, size_t cbBuffer);
LPTSTR MakePath(LPTSTR pszDest, size_t cchDest, LPCTSTR pszDir, LPCTSTR pszName, LPCTSTR pszExt);

