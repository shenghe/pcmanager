//////////////////////////////////////////////////////////////////////
///		@file		kxeprocess.h
///		@author		luopeng
///		@date		2008-10-27 09:07:58
///
///		@brief		文件针对64位操作系统的封装,以保证所有的操作均能
///                 操作到原生的文件目录结构
//////////////////////////////////////////////////////////////////////

#pragma once
#include <Windows.h>
#include <tchar.h>

/**
 * @defgroup kxeprocess_wow64_interface KXEngine Architecture Process WOW64 Interface
 * @{
 */

inline BOOL KxEIsWow64Process(HANDLE hProcess, PBOOL Wow64Process)
{
	typedef BOOL
		WINAPI
		IsWow64Process(
		__in  HANDLE hProcess,
		__out PBOOL Wow64Process
		);

	IsWow64Process* pfnIsWow64 = (IsWow64Process*)GetProcAddress(
		GetModuleHandle(_T("kernel32.dll")),
		"IsWow64Process"
		);
	if (pfnIsWow64 == NULL)
	{
		return FALSE;
	}

	return pfnIsWow64(hProcess, Wow64Process);
}


/**
 * @}
 */
