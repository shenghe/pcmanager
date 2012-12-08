#include "KCallInfocSend.h"
#include <shlwapi.h>

#define defKIS_CFG_VAL_ANTIVIRUS					L"SOFTWARE\\Kingsoft\\KISCommon\\KXEngine"
#define defKLIST_PROGRAMPATH						L"path"
#define defMAXWAITPROCESSTIME						1000
#define defUPDATANAME								L"report\\kupdata.exe"

#ifndef ENOENT
#define ENOENT 2
#endif

KCallInfocSend::KCallInfocSend(void)
{
}

KCallInfocSend::~KCallInfocSend(void)
{
}

void KCallInfocSend::SendData( IN int nSendMode /*= enumSendByGet*/, IN const wchar_t * lpStrFilePath )
{
	std::wstring strFilePath;
	if (lpStrFilePath == NULL)
	{
		BOOL bRet = _GetKInfocPath(strFilePath);
		if (!bRet)
			return ;
	}
	else
		strFilePath = lpStrFilePath;

	OutputDebugString(L"SendData");
	_LaunchAppEx(strFilePath.c_str(), L"-send 0", TRUE, 0, FALSE);

}

BOOL KCallInfocSend::_GetKInfocPath( OUT std::wstring& strInfocPath )
{
	HMODULE hModule = ::GetModuleHandle(NULL); 
	if (hModule == NULL)
		return FALSE;

	WCHAR szPath[MAX_PATH];
	DWORD dwReturn = ::GetModuleFileNameW(hModule, szPath, MAX_PATH);
	if (dwReturn == 0)
		return FALSE;

	int nLen = (int)wcslen(szPath);
	for (int i = nLen - 1; i >= 0; i--)
	{
		if (szPath[i] == '\\')
		{
			szPath[i + 1] = '\0';
			break;
		}
	}

	strInfocPath = szPath;
	strInfocPath += defUPDATANAME;

	if (_waccess_s(strInfocPath.c_str(), 0) != ENOENT)
		return TRUE;
	else
		return _CheckFileFromReg(strInfocPath);

	return FALSE;
}

BOOL KCallInfocSend::_CheckFileFromReg( OUT std::wstring& strFilePath )
{
	WCHAR szInstallPath[MAX_PATH] = {0};
	if (!_ReadOneRegKey(szInstallPath, MAX_PATH))
		return FALSE;

	wcscat_s(szInstallPath, MAX_PATH, L"\\operation\\cas\\");
	wcscat_s(szInstallPath, MAX_PATH, defUPDATANAME);

	if (_waccess_s(szInstallPath, 0) == ENOENT)
		return FALSE;

	strFilePath = szInstallPath;
	return TRUE;
}

BOOL KCallInfocSend::_ReadOneRegKey( OUT wchar_t * lpszResult, IN long dwOutMaxSize )
{
	BOOL bResult = FALSE;
	LONG lRet = ERROR_SUCCESS;
	HKEY hKey = NULL;
	DWORD dwCount = MAX_PATH;
	DWORD dwType = REG_BINARY;
	LPBYTE pBuf = NULL;
	DWORD dwSize = 0;

	do 
	{
		lRet = ::RegOpenKeyExW(
			HKEY_LOCAL_MACHINE,
			defKIS_CFG_VAL_ANTIVIRUS,
			0,
			KEY_QUERY_VALUE,
			&hKey
			);

		if (lRet != ERROR_SUCCESS)
		{
			lRet = ::RegOpenKeyExW(
				HKEY_CURRENT_USER,
				defKIS_CFG_VAL_ANTIVIRUS,
				0,
				KEY_QUERY_VALUE,
				&hKey
				);
		}

		if (lRet != ERROR_SUCCESS || hKey == NULL)
			break;

		lRet = ::RegQueryValueExW(hKey, defKLIST_PROGRAMPATH, 0, &dwType, NULL, &dwSize);

		if (ERROR_SUCCESS == lRet)
		{
			pBuf = new BYTE[max(dwSize, 4)];
			lRet = ::RegQueryValueExW(hKey, defKLIST_PROGRAMPATH, 0, &dwType, pBuf, &dwSize);

			// 			ASSERT(dwType == REG_SZ || dwType == REG_MULTI_SZ || dwType == REG_DWORD);
			// 			ASSERT(!((dwType == REG_SZ || dwType == REG_MULTI_SZ) && (dwSize % 2 != 0)));
			// 			ASSERT(!(dwType == REG_DWORD && dwSize != 4));

			wcscpy_s(lpszResult, dwOutMaxSize, (WCHAR *)pBuf);
			bResult = TRUE;
		}


	} while(0);

	if (pBuf != NULL)
	{
		delete[] pBuf;
		pBuf = NULL;
	}

	if (hKey)
	{
		::RegCloseKey(hKey);
		hKey = NULL;
	}

	return bResult;
}


DWORD KCallInfocSend::_LaunchAppEx(LPCWSTR lpczApp, LPWSTR lpszCmdLine, BOOL bWait, PDWORD pdwExitCode/*=NULL*/, BOOL bShowWnd/*=TRUE*/)
{
	DWORD dwResult = -1;	

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	si.dwFlags = STARTF_USESHOWWINDOW;
	if (bShowWnd == FALSE)
		si.wShowWindow = SW_HIDE;
	else
		si.wShowWindow = SW_SHOW;

	// Start the child process. 
	if	( !CreateProcessW( 
						lpczApp, 
						lpszCmdLine, 
						NULL,              
						NULL,             
						TRUE,           
						0,               
						NULL,            
						NULL,            
						&si,             
						&pi)
		)          
	{
		dwResult = GetLastError();
		OutputDebugString(L"_LaunchAppEx:error");
		goto Exit0;
	}

	if (bWait)
	{
		if (WaitForSingleObject(pi.hProcess, defMAXWAITPROCESSTIME) == WAIT_OBJECT_0)
			if (pdwExitCode)
				GetExitCodeProcess(pi.hProcess, pdwExitCode);	
	}

	dwResult = 0;
Exit0:
	if (pi.hProcess != NULL)
	{
		CloseHandle(pi.hProcess);
		pi.hProcess = NULL;
	}
	if (pi.hThread != NULL)
	{
		CloseHandle(pi.hThread);
		pi.hThread = NULL;
	}

	return dwResult;
}