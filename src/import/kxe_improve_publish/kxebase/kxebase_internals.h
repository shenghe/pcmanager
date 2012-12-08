//////////////////////////////////////////////////////////////////////
///		@file		kxebase_internals.h
///		@author		luopeng
///		@date		2008-8-29 09:07:58
///	
///		@brief		kxebase的主函数定义
//////////////////////////////////////////////////////////////////////
#pragma once
#include <WinSock2.h>
#include <windows.h>

/**
 * @defgroup kxebase_initialize_internals_group KXEngine Architecture Base Internals SDK Initialize Interface
 * @{
 */
int __stdcall kxe_base_load_plugin_path(const wchar_t* pwszPath);

int __stdcall kxe_base_unload_plugin_path(const wchar_t* pwszPath);

/**
 * @}
 */
