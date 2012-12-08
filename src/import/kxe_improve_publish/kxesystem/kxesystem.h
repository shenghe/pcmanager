//////////////////////////////////////////////////////////////////////
///		@file		kxesystem.h
///		@author		luopeng
///		@date		2009-3-3 09:07:58
///
///		@brief		针对system的一些API兼容性封装
///                
//////////////////////////////////////////////////////////////////////

#pragma once
#include <Windows.h>
#include <tchar.h>

/**
 * @defgroup kxesystem_compatible_interface KXEngine System Compatible Interface
 * @{
 */

/**
 * @brief 查询活动用户的Token
 * @param[in] phToken 存储查询到的活动用户Token
 * @return 0 成功， 其他为失败错误码
 * @remark phTokean需要使用CloseHandle关闭
 */
int __stdcall kxe_base_system_query_active_user_token(
	HANDLE* phToken
	);

/**
 * @brief 将应用程序添加至Windows的防火墙
 * @param[in] pwszImageFileName 需要添加的程序的合路径
 * @param[in] pwszFriendlyName 名字
 * @return 0 成功， 其他为失败错误码
 */
int __stdcall kxe_base_system_add_app_windows_firewall(
	const wchar_t* pwszImageFileName,
	const wchar_t* pwszFriendlyName
	);

/**
* @brief 将应用程序从Windows的防火墙移出
* @param[in] pwszImageFileName 需要移出的程序的合路径
* @return 0 成功， 其他为失败错误码
*/
inline int __stdcall kxe_base_system_remove_app_windows_firewall(
	const wchar_t* pwszImageFileName
	);

/**
 * @}
 */
