//////////////////////////////////////////////////////////////////////
///		@file		kxebase_config.h
///		@author		luopeng
///		@date		2008-8-29 09:07:58
///	
///		@brief		配置接口关函数定义
//////////////////////////////////////////////////////////////////////
#pragma once

/**
 * @defgroup kxebase_config_group KXEngine Architecture Base SDK Config Interface
 * @{
 */

/// 预定义配置键类型--配置为当前机器所有用户
#define KXE_LOCAL_MACHINE_CONFIG (kxe_config_key_t)0x00000001
/// 预定义配置键类型--配置为当前用户
#define KXE_CURRENT_USER_CONFIG  (kxe_config_key_t)0x00000002

/// 定义键类型
typedef void* kxe_config_key_t;

/**
* @brief 根据路径,创建或打开一个指定的配置键
* @param[in] key 传入要打开的绝对键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_base_create_config_key或kxe_base_open_config_key创建出来的键
* @param[in] pwszPath 相对路径,最后打开的路径为key的path+pwszPath
* @param[out] pKey 成功打开后传出的配置键
* @remark 如果指定的键存在,则打开它,否则创建它
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_create_config_key(
	kxe_config_key_t key, 
	const wchar_t* pwszPath, 
	kxe_config_key_t* pKey
	);

/**
* @brief 根据路径,打开一个指定的配置键
* @param[in] key 传入要打开的绝对键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_base_create_config_key或kxe_base_open_config_key创建出来的键
* @param[in] pwszPath 相对路径,最后打开的路径为key的path+pwszPath
* @param[out] pKey 成功打开后传出的配置键
* @remark 如果指定的键存在,则打开它,否则失败
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_open_config_key(
	kxe_config_key_t key, 
	const wchar_t* pwszPath, 
	kxe_config_key_t* pKey
	);

/**
* @brief 关闭指定的配置键
* @param[in] key 通过kxe_base_create_config_key或kxe_base_open_config_key获得的键值
* @remark 
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_close_config_key(
	kxe_config_key_t key
	);

/**
* @brief 通过指定的键及其相对路径,获取valuename对应的value
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_base_create_config_key或kxe_base_open_config_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in] pwszValueName 需要获取的指定ValueName
* @param[in, out] pwszValue 传入用于存储value的指针
* @param[in] nValueLength 传入的pwszBuffer的wchar_t的个数
* @param[out] pnReturnValueLength 返回实际需要的buffer的wchat_t的个数,包含末尾的字符串结束符
* @remark 
* @return 0 成功, 其他值为失败的错误码
*/
int __stdcall kxe_base_get_config(
	kxe_config_key_t key, 
	const wchar_t* pwszPath, 
	const wchar_t* pwszValueName, 
	wchar_t* pwszValue,
	int nValueLength,
	int* pnReturnValueLength
	);

/**
* @brief 通过指定的键及其相对路径,设定valuename对应的value
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_base_create_config_key或kxe_base_open_config_key创建出来的键
* @param[in] pwszPath 相对路径,最后获取的路径为key的path+pwszPath
* @param[in] pwszValueName 指定ValueName
* @param[in] pwszValue 传入的buffer内容,只能使用字符串存储
* @param[in] nValueLength 传入的pszBuffer的长度,用wchar_t的个数表示
* @remark 
* @return 0 成功, 其他值为失败的错误码
*/
int __stdcall kxe_base_set_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath, 
	const wchar_t* pwszValueName, 
	const wchar_t* pwszValue,
	int nValueLength
	);

/**
* @brief 删除指定的键值
* @param[in] key 传入要获取的指定键值,
*				 可以为KXE_LOCAL_MACHINE_CONFIG,KXE_CURRENT_USER_CONFIG或
*                kxe_base_create_config_key或kxe_base_open_config_key创建出来的键
* @param[in] pwszPath 相对路径,最后路径为key的path+pwszPath
* @remark 
* @return 0 成功, 其他值为失败的错误码
*/
int __stdcall kxe_base_delete_config_key(
	kxe_config_key_t key,
	const wchar_t* pwszPath
	);

/**
* @brief 枚举指令键的子键
* @param[in] key 传入要获取的指定键值,
*                kxe_base_create_config_key或kxe_base_open_config_key创建出来的键
* @param[in] uIndex 传入要获取的第uIndex个子键
* @param[in, out] pwszBuffer 传入用于获取子键名的buffer
* @param[in] nBufferLength 传入的pwszBuffer大小,用wchar_t表达
* @param[out] pnReturnBuffer 传出需要的Buffer大小,包含末尾字符串结束符
* @remark 
* @return 0 成功, 其他值为失败的错误码
*/
int __stdcall kxe_base_enum_config_key(
	kxe_config_key_t key,
	unsigned int uIndex,
	wchar_t* pwszBuffer,
	int nBufferLength,
	int* pnReturnBuffer
	);


/**
 * @}
 */