#include "stdafx.h"
#include "koperatefile.h"


DWORD _DoGetLineByBuf(WCHAR* const pszBuf, DWORD dwlen,
					std::vector<std::wstring>& vcStrline)
{
	if (!pszBuf)
	{
		return 0;
	}

	WCHAR* pszBeginPoint = pszBuf;
	WCHAR* pszCurrentPoint = pszBuf;
	WCHAR* pszTempBuf = NULL;
	DWORD dwlenSize = 0;
	DWORD dwLineCount = 0;

	while (*pszCurrentPoint != 0 && (unsigned)(pszCurrentPoint - pszBuf) <= dwlen)
	{
		if (*pszCurrentPoint == '\n')
		{
			dwlenSize = (DWORD)(pszCurrentPoint - pszBeginPoint);
			pszTempBuf = new WCHAR[dwlenSize + 1];
			if (!pszTempBuf)
			{
				break;
			}
			::ZeroMemory(pszTempBuf, (dwlenSize + 1)* 2);
			::memcpy(pszTempBuf, pszBeginPoint, dwlenSize * 2);

			pszBeginPoint = pszCurrentPoint + 1;

			vcStrline.push_back(pszTempBuf);
			dwLineCount++ ;

			delete []pszTempBuf;
			pszTempBuf = NULL;
		}

		pszCurrentPoint ++;
	}

	return dwLineCount;
}

DWORD _DoGetFileSizeByFileName(const WCHAR* pFileName)
{
	if (!pFileName)
	{
		return 0;
	}

	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwFileSize = 0;

	hFile = ::CreateFile(
		pFileName, 
		GENERIC_READ,
		0, 
		NULL,
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL
		);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwFileSize = ::GetFileSize(hFile, NULL);
		::CloseHandle(hFile);
	}

	return dwFileSize;
}