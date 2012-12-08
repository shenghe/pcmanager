#pragma once
#include "Defines.h"

BOOL CrackUrl( LPCTSTR lpszUrl, UrlInfo& sUrlInfo );
BOOL GetFileNameFromUrl( LPCTSTR szUrl, CString &strFilename );
void GetUrlsFromFile( LPCTSTR szFilename, std::vector<std::string> &urls );
void FormatSizeString(INT64 nFileSize, CString &str);
