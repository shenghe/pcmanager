//////////////////////////////////////////////////////////////////////
///		@file		kxeconnect.inl
///		@author		luopeng
///		@date		2008-11-12 9:07:58
///
///		@brief		
//////////////////////////////////////////////////////////////////////

#pragma once
#include "kxeconnect.h"
#include "../kxeerror/kxeerror.h"
#include <tchar.h>

inline int GetTcpTableOnXp(MIB_TCPEXTABLE** ppTcpTable)
{
	typedef DWORD WINAPI AllocateAndGetTcpExTableFromStackFunction(
		PVOID* ppTcpTable,
		BOOL bOrder,
		HANDLE hHeap,
		DWORD dwFlags,
		DWORD dwFamily
		);

	HMODULE hDll = LoadLibrary(_T("Iphlpapi.dll"));
	if (hDll == NULL)
	{
		return GetLastError();
	}

	AllocateAndGetTcpExTableFromStackFunction* pFunction = (AllocateAndGetTcpExTableFromStackFunction*)GetProcAddress(
		hDll,
		"AllocateAndGetTcpExTableFromStack"
		);
	if (pFunction == NULL)
	{
		int nRet = GetLastError();
		FreeLibrary(hDll);
		return nRet;
	}

	int nRet = pFunction(
		(PVOID*)ppTcpTable,
		TRUE,
		GetProcessHeap(),
		0,
		AF_INET
		);
	FreeLibrary(hDll);
	return nRet;
}

inline int FreeTcpTableOnXp(MIB_TCPEXTABLE* pTcpTable)
{
	HeapFree(
		GetProcessHeap(),
		0,
		pTcpTable
		);

	return 0;
}

inline int GetTcpTableOnVista(MIB_TCPEXTABLE** ppTcpTable)
{
	typedef DWORD WINAPI GetExtendedTcpTableFunction(
		PVOID pTcpTable,
		PDWORD pdwSize,
		BOOL bOrder,
		ULONG ulAf,
		int,
		ULONG Reserved
		);

	HMODULE hDll = LoadLibrary(_T("Iphlpapi.dll"));
	if (hDll == NULL)
	{
		return GetLastError();
	}

	GetExtendedTcpTableFunction* pFunction = (GetExtendedTcpTableFunction*)GetProcAddress(
		hDll,
		"GetExtendedTcpTable"
		);
	if (pFunction == NULL)
	{
		int nRet = GetLastError();
		FreeLibrary(hDll);
		return nRet;
	}

	DWORD dwSize = 0;
	int nRet = pFunction(
		NULL,
		&dwSize,
		TRUE,
		AF_INET,
		5,
		0
		);
	if (nRet == ERROR_INSUFFICIENT_BUFFER)
	{
		*ppTcpTable = (MIB_TCPEXTABLE*)new char[dwSize];
	}

	nRet = pFunction(
		*ppTcpTable,
		&dwSize,
		TRUE,
		AF_INET,
		5,
		0
		);
	FreeLibrary(hDll);
	return nRet;
}

inline int FreeTcpTableOnVista(MIB_TCPEXTABLE* pTcpTable)
{
	delete (char*)pTcpTable;
	return 0;
}

inline int KxEGetExtendTcpTable(MIB_TCPEXTABLE** ppTcpTable)
{
	OSVERSIONINFOEX version = {0};
	version.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((LPOSVERSIONINFO)&version);
	if (version.dwMajorVersion == 5)
	{
		if (version.dwMinorVersion == 0)  // 2000
		{
			return E_KXEBASE_SYSTEM_NOT_SUPPORT;
		}
		else if (version.dwMinorVersion == 1 || version.dwMinorVersion == 2) // XP, 2003, XP64
		{
			return GetTcpTableOnXp(ppTcpTable);
		}
	}
	else if (version.dwMajorVersion == 6)  // Vista, WIN7
	{
		return GetTcpTableOnVista(ppTcpTable);
	}

	return E_KXEBASE_SYSTEM_NOT_SUPPORT;  // 未知系统版本
}

inline int KxEFreeExtendTcpTable(MIB_TCPEXTABLE* pTcpTable)
{
	OSVERSIONINFOEX version = {0};
	version.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((LPOSVERSIONINFO)&version);
	if (version.dwMajorVersion == 5)
	{
		if (version.dwMinorVersion == 0)  // 2000
		{
			return E_KXEBASE_SYSTEM_NOT_SUPPORT;
		}
		else if (version.dwMinorVersion == 1 || version.dwMinorVersion == 2) // XP, 2003, XP64
		{
			return FreeTcpTableOnXp(pTcpTable);
		}
	}
	else if (version.dwMajorVersion == 6)
	{
		if (version.dwMinorVersion == 0) // Vista
		{
			return FreeTcpTableOnVista(pTcpTable);
		}
	}

	return E_KXEBASE_SYSTEM_NOT_SUPPORT;  // 未知系统版本
}
