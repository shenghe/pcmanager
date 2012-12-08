//////////////////////////////////////////////////////////////////////
///		@file		process.h
///		@author		luopeng
///		@date		2008-12-30 17:04:42
///
///		@brief		针对进程相关的封装
//////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include "../kxeerror/kxeerror.h"

typedef
BOOL (WINAPI* type_CreateEnvironmentBlock)(
	OUT LPVOID *lpEnvironment,
	IN HANDLE  hToken,
	IN BOOL    bInherit
	);


typedef
BOOL (WINAPI* type_DestroyEnvironmentBlock)(
    IN LPVOID  lpEnvironment
	);

/**
* @brief 使用指定的Token创建进程，此创建将会把环境变量相关进行初始化
* @param[in] lpszCommandLine 启动的进程，包含参数
* @param[in] lpszDirectory 需要设定的当前目录
* @param[in] hToken 需要模拟的Token
* @param[in] dwCreateFlags 创建的进程选项
* @param[out] ps 目标进程相关信息
*/
inline int KxECreateProcessAsUser(LPCTSTR lpszCommandLine,
								  LPCTSTR lpszDirectory,
								  HANDLE hToken,
								  DWORD dwCreateFlags,
								  PROCESS_INFORMATION& ps)
{
	if (lpszCommandLine == NULL)
	{
		return E_KXEBASE_INVALID_PARAMETER;
	}

	LPVOID lpEnviroment = NULL;
	HMODULE hDll = LoadLibrary(_T("userenv.dll"));
	type_CreateEnvironmentBlock pfnCreateEnvironmentBlock = NULL;
	type_DestroyEnvironmentBlock pfnDestroyEnvironmentBlock = NULL;
	if (hDll != NULL)
	{
		pfnCreateEnvironmentBlock = (type_CreateEnvironmentBlock)GetProcAddress(
			hDll, 
			"CreateEnvironmentBlock"
			);
		pfnDestroyEnvironmentBlock = (type_DestroyEnvironmentBlock)GetProcAddress(
			hDll, 
			"DestroyEnvironmentBlock"
			);
		if (pfnCreateEnvironmentBlock == NULL ||
			pfnDestroyEnvironmentBlock == NULL)
		{
			pfnCreateEnvironmentBlock = NULL;
			pfnDestroyEnvironmentBlock = NULL;
			FreeLibrary(hDll);
			hDll = NULL;
		}
	}

	if (pfnCreateEnvironmentBlock != NULL)
	{
		BOOL bRet = pfnCreateEnvironmentBlock(
			&lpEnviroment,
			hToken,
			FALSE
			);
		if (bRet)
		{
			dwCreateFlags |= CREATE_UNICODE_ENVIRONMENT;
		}
	}


	memset(&ps, 0, sizeof(PROCESS_INFORMATION));
	STARTUPINFO st = {0};
	st.cb = sizeof(STARTUPINFO);
	BOOL bRet = CreateProcessAsUser(
		hToken,
		NULL,
		const_cast<LPWSTR>(lpszCommandLine),
		NULL,
		NULL,
		FALSE,
		dwCreateFlags,
		lpEnviroment,
		lpszDirectory,
		&st,
		&ps
		);
	int nErr = 0;
	if (!bRet)
	{
		nErr = GetLastError();
	}

	if (lpEnviroment && pfnDestroyEnvironmentBlock)
	{
		pfnDestroyEnvironmentBlock(lpEnviroment);
	}
	return nErr;
}

/**
* @brief 使用指定的Token创建进程，此创建将会把环境变量相关进行初始化
* @param[in] lpszCommandLine 启动的进程，包含参数
* @param[in] lpszDirectory 需要设定的当前目录
* @param[in] hToken 需要模拟的Token
* @param[out] ps 目标进程相关信息
*/
inline int KxECreateProcessAsUser(LPCTSTR lpszCommandLine,
								  LPCTSTR lpszDirectory,
								  HANDLE hToken,
								  PROCESS_INFORMATION& ps)
{
	return KxECreateProcessAsUser(
		lpszCommandLine,
		lpszDirectory,
		hToken,
		0,
		ps
		);
}

/**
* @brief 使用指定的Token创建进程，此创建将会把环境变量相关进行初始化
* @param[in] lpszCommandLine 启动的进程，包含参数
* @param[in] lpszDirectory 需要设定的当前目录
* @param[in] hToken 需要模拟的Token
* @param[out] dwProcessId 目标进程ID
*/
inline int KxECreateProcessAsUser(LPCTSTR lpszCommandLine,
								  LPCTSTR lpszDirectory,
								  HANDLE hToken,
								  DWORD& dwProcessId)
{
	PROCESS_INFORMATION ps = {0};
	int nRet = KxECreateProcessAsUser(
		lpszCommandLine,
		lpszDirectory,
		hToken,
		ps
		);
	if (nRet == 0)
	{
		dwProcessId = ps.dwProcessId;
		CloseHandle(ps.hProcess);
		CloseHandle(ps.hThread);
	}

	return nRet;
}


/**
 * @brief 使用指定的Token创建进程，此创建将会把环境变量相关进行初始化
 * @param[in] lpszCommandLine 启动的进程，包含参数
 * @param[in] lpszDirectory 需要设定的当前目录
 * @param[in] hToken 需要模拟的Token
 */
inline int KxECreateProcessAsUser(LPCTSTR lpszCommandLine,
								  LPCTSTR lpszDirectory,
								  HANDLE hToken)
{
	DWORD dwProcessId = 0;
	return KxECreateProcessAsUser(
		lpszCommandLine,
		lpszDirectory,
		hToken,
		dwProcessId
		);
}