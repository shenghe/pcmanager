//////////////////////////////////////////////////////////////////////
///		@file		kxefile.h
///		@author		luopeng
///		@date		2008-10-27 09:07:58
///
///		@brief		文件针对64位操作系统的封装,以保证所有的操作均能
///                 操作到原生的文件目录结构
//////////////////////////////////////////////////////////////////////

#pragma once
#include <Windows.h>
#include <tchar.h>
#include "kxeprocess.h"

/**
 * @defgroup kxefile_wow64_interface KXEngine Architecture File WOW64 Interface
 * @{
 */

typedef 
BOOL
WINAPI
Wow64DisableWow64FsRedirectionFunction(
	__out PVOID *OldValue
	);

typedef 
BOOL
WINAPI
Wow64RevertWow64FsRedirectionFunction(
	__in PVOID OlValue
	);

/**
 * @brief 关闭Wow64下的文件重定向
 * @param[out] ppVoidValue 用于存储内部数据,在KxEWow64RevertWow64FsRedirection传回
 * @attention 在LoadLibrary存在的流程中,不允许调用KxEWow64DisableWow64FsRedirection,如果已经调
 *            用,需要调用KxEWow64RevertWow64FsRedirection还原,否则可能出现不可预知的错误.
 * @remark 此函数为Wow64DisableWow64FsRedirection的间接调用方式,避免直接依赖,导致WIN32下无法运行
 * @return true 成功, false 失败
 */
inline BOOL __stdcall KxEWow64DisableWow64FsRedirection(PVOID* ppVoidValue)
{
	Wow64DisableWow64FsRedirectionFunction* pfnWow64DisableWow64FsRedirection = 
		(Wow64DisableWow64FsRedirectionFunction*)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "Wow64DisableWow64FsRedirection");
	if (pfnWow64DisableWow64FsRedirection == NULL)
	{
		return FALSE;
	}

	return pfnWow64DisableWow64FsRedirection(ppVoidValue);
}

/**
 * @brief 还原Wow64下的文件重定向
 * @param[in] pOldValue 由于存储内部数据,在KxEWow64DisableWow64FsRedirection获取
 * @remark 此函数为Wow64DisableWow64FsRedirection的间接调用方式,避免直接依赖,导致WIN32下无法运行
 * @return true 成功, false 失败
 */
inline BOOL __stdcall KxEWow64RevertWow64FsRedirection(PVOID pOldValue)
{
	Wow64RevertWow64FsRedirectionFunction* pfnWow64RevertWow64FsRedirection = 
		(Wow64RevertWow64FsRedirectionFunction*)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "Wow64RevertWow64FsRedirection");
	if (pfnWow64RevertWow64FsRedirection == NULL)
	{
		return FALSE;
	}

	return pfnWow64RevertWow64FsRedirection(pOldValue);
}

/**
 * @brief 用于在局部区域短暂关闭Wow64下的文件重定向功能
 */
class KxEFileWow64Guard
{
public:
	KxEFileWow64Guard()
	{
		m_bWow64 = false;
		if (KxEIsWow64Process(GetCurrentProcess(), &m_bWow64) && m_bWow64)
		{
			KxEWow64DisableWow64FsRedirection(&m_pVoidValue);
		}
	}

	~KxEFileWow64Guard()
	{
		if (m_bWow64)
		{
			KxEWow64RevertWow64FsRedirection(m_pVoidValue);
		}
	}

private:
	BOOL m_bWow64;
	PVOID m_pVoidValue;
};


/**
 * @}
 */
