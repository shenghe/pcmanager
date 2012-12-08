#ifndef _KOPERATEFILE_H_
#define _KOPERATEFILE_H_
#include <vector>
#include <algorithm>
#include <string>

DWORD _DoGetLineByBuf(WCHAR* const pszBuf, DWORD dwlen, std::vector<std::wstring>& vcStrline);
DWORD _DoGetFileSizeByFileName(const WCHAR* pFileName);
#endif