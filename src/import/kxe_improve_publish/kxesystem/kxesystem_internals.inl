//////////////////////////////////////////////////////////////////////
///		@file		kxesystem_internals.inl
///		@author		luopeng
///		@date		2009-3-3 09:07:58
///
///		@brief		针对system的一些API兼容性封装实现
///                
//////////////////////////////////////////////////////////////////////

#pragma once
#include <Windows.h>
#include <tchar.h>
#if _MSC_VER > 1200
#include <WtsApi32.h>
#pragma comment(lib, "wtsapi32.lib")
#else
#include "../common/kxe_wtsapi32.h"
#endif
#include <TlHelp32.h>
#include "netfw.h"
#include "../kxeerror/kxeerror.h"
#pragma comment(lib,"ole32")
#pragma comment(lib,"oleaut32")

/**
 * @defgroup kxesystem_compatible_interface KXEngine System Compatible Interface
 * @{
 */

inline int KxEGetActiveSession(DWORD& dwSession)
{
	HMODULE hWTSApi = LoadLibrary(_T("wtsapi32.dll"));
	if (hWTSApi == NULL)
	{
		return GetLastError();
	}

	typedef BOOL
		WINAPI
		KxEWTSEnumerateSessionsW(
		IN HANDLE hServer,
		IN DWORD Reserved,
		IN DWORD Version,
		OUT PWTS_SESSION_INFOW * ppSessionInfo,
		OUT DWORD * pCount
		);
	KxEWTSEnumerateSessionsW* pfnWTSEnumerateSessionsW = (KxEWTSEnumerateSessionsW*)GetProcAddress(
		hWTSApi,
		"WTSEnumerateSessionsW"
		);
	int nRet = 0;
	if (pfnWTSEnumerateSessionsW == NULL)
	{
		nRet = GetLastError();
		FreeLibrary(hWTSApi);
		return nRet;
	}

	typedef VOID 
		WINAPI
		KxEWTSFreeMemory(
		IN PVOID pMemory
		);
	KxEWTSFreeMemory* pfnWTSFreeMemory = (KxEWTSFreeMemory*)GetProcAddress(
		hWTSApi,
		"WTSFreeMemory"
		);
	if (pfnWTSFreeMemory == NULL)
	{
		nRet = GetLastError();
		FreeLibrary(hWTSApi);
		return nRet;
	}

	PWTS_SESSION_INFO pSessionInfoArray = NULL;
	DWORD dwCount = 0;
	BOOL bRet = pfnWTSEnumerateSessionsW(
		WTS_CURRENT_SERVER_HANDLE,
		0,
		1,
		&pSessionInfoArray,
		&dwCount
		);
	if (!bRet)
	{
		FreeLibrary(hWTSApi);
		return 0;
	}

	nRet = ERROR_NOT_FOUND;
	for (DWORD i = 0; i < dwCount; ++i)
	{
		if (pSessionInfoArray[i].State == WTSActive)
		{
			dwSession = pSessionInfoArray[i].SessionId;
			nRet = 0;
			break;
		}
	}
	pfnWTSFreeMemory(pSessionInfoArray);
	FreeLibrary(hWTSApi);

	return nRet;
}

inline int kxe_system_query_active_user_token_on_2k(HANDLE* phToken)
{
	HANDLE hToolhelp = CreateToolhelp32Snapshot(
		TH32CS_SNAPPROCESS,
		0
		);
	if (hToolhelp == INVALID_HANDLE_VALUE)
	{
		return GetLastError();
	}	

	DWORD dwProcessId = 0;
	int nRet = ERROR_NOT_FOUND;
	PROCESSENTRY32 processEntry = {0};
	processEntry.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hToolhelp, &processEntry))
	{
		do 
		{
			if (_wcsicmp(processEntry.szExeFile, L"explorer.exe") == 0)
			{
				dwProcessId = processEntry.th32ProcessID;
				nRet = 0;
				break;
			}
		} while (Process32Next(hToolhelp, &processEntry));
	}
	else
	{
		nRet = GetLastError();
	}
	CloseHandle(hToolhelp);

	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION ,
		FALSE,
		dwProcessId
		);
	if (hProcess == NULL)
	{
		return GetLastError();
	}

	BOOL bRet = OpenProcessToken(
		hProcess,
		TOKEN_ALL_ACCESS,
		phToken
		);
	if (!bRet)
	{
		nRet = GetLastError();
	}

	CloseHandle(hProcess);
	return nRet;
}

inline int kxe_system_query_active_user_token_on_xp(HANDLE* phToken)
{
	DWORD dwSessionId = 0;
	int nRet = KxEGetActiveSession(dwSessionId);
	if (nRet != 0)
	{
		return nRet;
	}

	// 此函数在2K下没有，需要判断兼容性，在2000下直接获取explorer.exe的Token
	HMODULE hWTSApi = LoadLibrary(_T("wtsapi32.dll"));
	if (hWTSApi == NULL)
	{
		return GetLastError();
	}

	typedef BOOL WINAPI
		KxEWTSQueryUserToken(
		ULONG SessionId, 
		PHANDLE phToken
		);
	KxEWTSQueryUserToken* pfnWTSQueryUserToken = (KxEWTSQueryUserToken*)GetProcAddress(
		hWTSApi,
		"WTSQueryUserToken"
		);
	if (pfnWTSQueryUserToken == NULL)
	{
		nRet = GetLastError();
	}
	else
	{
		BOOL bRet = pfnWTSQueryUserToken(dwSessionId, phToken);
		if (!bRet)
		{
			nRet = GetLastError();	
		}
	}

	FreeLibrary(hWTSApi);
	return nRet;
}

inline int kxe_system_query_active_user_token(HANDLE* phToken)
{
	if (phToken == NULL)
	{
		return E_KXEBASE_INVALID_PARAMETER;
	}

	OSVERSIONINFO version = {0};
	version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	bool bIsWIN2K = false;
	if (GetVersionEx(&version))
	{
		if (version.dwMajorVersion == 5 && version.dwMinorVersion == 0)
		{
			bIsWIN2K = true;
		}
	}

	if (!bIsWIN2K)
	{
		int nRet = kxe_system_query_active_user_token_on_xp(phToken);
		if (nRet == ERROR_PRIVILEGE_NOT_HELD)  // 没有特权，则是用户权限在运行，再次获取
		{
			return kxe_system_query_active_user_token_on_2k(phToken);
		}
		return nRet;
	}
	else
	{
		return kxe_system_query_active_user_token_on_2k(phToken);
	}
}

inline int __stdcall kxe_base_system_windows_get_firewall_apps(INetFwAuthorizedApplications** ppFwApps)
{
	CComPtr<INetFwMgr> pFwMgr;
	HRESULT hr = pFwMgr.CoCreateInstance(
		__uuidof(NetFwMgr),
		NULL,
		CLSCTX_INPROC_SERVER
		);
	if (FAILED(hr))
	{
		return hr;
	}

	INetFwPolicy* pFwPolicy = NULL;
	hr = pFwMgr->get_LocalPolicy(&pFwPolicy);
	if (FAILED(hr))
	{
		return hr;
	}
	CComPtr<INetFwPolicy> pFwPolicyPtr = pFwPolicy;
	pFwPolicy->Release();

	INetFwProfile* pProfile = NULL;
	hr = pFwPolicy->get_CurrentProfile(&pProfile);
	if (FAILED(hr))
	{
		return hr;
	}
	CComPtr<INetFwProfile> pProfilePtr = pProfile;
	pProfile->Release();

	INetFwAuthorizedApplications* pFwApps = NULL;
	hr = pProfile->get_AuthorizedApplications(ppFwApps);
	if (FAILED(hr))
	{
		return hr;
	}

	return 0;
}

inline int __stdcall kxe_base_system_add_app_windows_firewall_imp(const wchar_t* pwszImageFileName,
																  const wchar_t* pwszFriendlyName)
{
	INetFwAuthorizedApplications* pFwApps = NULL;
	int nRet = kxe_base_system_windows_get_firewall_apps(&pFwApps);
	if (nRet != 0)
	{
		return nRet;
	}
	CComPtr<INetFwAuthorizedApplications> pFwAppPtr = pFwApps;
	pFwApps->Release();

	CComPtr<INetFwAuthorizedApplication> pFwApp;
	HRESULT hr = pFwApp.CoCreateInstance(
		__uuidof(NetFwAuthorizedApplication),
		NULL,
		CLSCTX_INPROC_SERVER
		);
	if (FAILED(hr))
	{
		return hr;
	}

	CComBSTR bstrImageFileName(pwszImageFileName);
	hr = pFwApp->put_ProcessImageFileName(bstrImageFileName);
	if (FAILED(hr))
	{
		return hr;
	}

	CComBSTR bstrFriendlyName(pwszFriendlyName);
	hr = pFwApp->put_Name(bstrFriendlyName);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pFwApps->Add(pFwApp);
	if (FAILED(hr))
	{
		return hr;
	}
	return 0;
}


inline int __stdcall kxe_base_system_add_app_windows_firewall(const wchar_t* pwszImageFileName,
															  const wchar_t* pwszFriendlyName)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); //CoInitialize(NULL);
	if (FAILED(hr))
	{
		return hr;
	}

	int nRet = kxe_base_system_add_app_windows_firewall_imp(
		pwszImageFileName,
		pwszFriendlyName
		);

	CoUninitialize();

	return nRet;
}

inline int __stdcall kxe_base_system_remove_app_windows_firewall(const wchar_t* pwszImageFileName)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); //CoInitialize(NULL);
	if (FAILED(hr))
	{
		return hr;
	}

	INetFwAuthorizedApplications* pFwApps = NULL;
	int nRet = kxe_base_system_windows_get_firewall_apps(&pFwApps);
	if (nRet == 0)
	{
		CComBSTR bstrImageFileName = pwszImageFileName;
		nRet = pFwApps->Remove(bstrImageFileName);
		pFwApps->Release();
	}

	CoUninitialize();

	return nRet;
}


/**
 * @}
 */
