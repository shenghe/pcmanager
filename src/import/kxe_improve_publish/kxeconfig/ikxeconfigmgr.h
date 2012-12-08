//////////////////////////////////////////////////////////////////////
///		@file		ikxeconfigmgr.h
///		@author		luopeng
///		@date		2008-10-07 09:07:58
///	
///		@brief		kxeconfig相关定义
//////////////////////////////////////////////////////////////////////

#pragma once
#include <Unknwn.h>

/**
 * @defgroup kxeconfig_component_interface KXEngine Architecture Config Component Interface
 * @remark  1. 此组件可以在平台外独立进行使用\n
 *          2. 此组件在访问注册表时,将完全访问WOW64的注册表内容,不对32位的注册表内容进行读取,以保证64位下模块兼容性
 * @{
 */

/**
 * @brief 打开的键值类型
 */
typedef enum _KXE_CONFIG_TYPE
{
	enum_KCT_User,          ///< 当前用户键
	enum_KCT_LoalMachine    ///< 本地机器键
} KXE_CONFIG_TYPE;

/**
 * @brief 代表当前的键内容
 */
class IKxEConfigKey
{
public:
	/**
	 * @brief 释放该键的资源,该键将被关闭
	 */
	virtual void __stdcall Release() = 0;

	/**
	 * @brief 删除当前Key的SubKey
	 * @param[in] pwszSubKey 需要删除的子键相对路径
	 * @return 0 成功, 其他为失败错误骊
	 */
	virtual int __stdcall DeleteKey(
		const wchar_t* pwszSubKey
		) = 0;

	/**
	 * @brief 获取当前对象所代表的路径
	 * @return 返回当前对像代表的路径
	 */
	virtual const wchar_t* __stdcall GetKeyPath() = 0;

	/**
	 * @brief 获取当前键所代表的类型
	 * @return 返回当前键的类型
	 */
	virtual KXE_CONFIG_TYPE __stdcall GetKeyType() = 0;

	/**
	 * @brief 枚举当前键的子键
	 * @param[in] nIndex 枚举第nIndex个子键
	 * @param[out] ppKey 获取到子键后返回
	 * @return 0 成功, 其他为失败错误码
	 */
	virtual int __stdcall EnumChildKey(
		int nIndex, 
		IKxEConfigKey** ppKey
		) = 0;

	/**
	 * @brief 获举当前键的子键名字
	 * @param[in] nIndex 枚举第nIndex个子键
	 * @param[in] pwszBuffer 用于存储子键的buffer
	 * @param[in] nBufferLength 用于指定pwszBuffer的大小,单位为wchar_t
	 * @param[out] pnReturnBuffer 需要的buffer大小, 单位为wchar_t,包含字符串结束符
	 * @return 0 成功,了其他为失败错误码
	 */
	virtual int __stdcall EnumChildKey(
		int nIndex,
		wchar_t* pwszBuffer,
		int nBufferLength,
		int* pnReturnBuffer
		) = 0;

	/**
	 * @brief 设置当前键下的Value内容
	 * @param[in] pwszValueName 要指定的的ValueName
	 * @param[in] pwszValue 要存储的内容,只能是字符串
	 * @param[in] nValueLength 该字符串的长度,包含字符串结束符, 单位为wchar_t
	 */
	virtual int __stdcall SetValue(
		const wchar_t* pwszValueName, 
		const wchar_t* pwszValue,
		int nValueLength
		) = 0;

	/**
	 * @brief 查询当前键下的Value内容
	 * @param[in] pwszValueName 要指定的的ValueName
	 * @param[in] pwszValueBuffer 获取到的内容存储的buffer
	 * @param[in] nValueLength 该字符串的长度,包含字符串结束符, 单位为wchar_t
	 * @param[out] pnReturnValueLength 需要的字符串长度,包含字符串结束符,单位为wchar_t
	 * @return 0 成功, 其他为失败错误码
	 */
	virtual int __stdcall QueryValue(
		const wchar_t* pwszValueName,
		wchar_t* pwszValueBuffer,
		int nValueLength,
		int* pnReturnValueLength
		) = 0;

	/**
	 * @brief 删除指定键下的指定ValueName
	 * @param[in] pwszValueName 要删除的ValueName
	 * @return 0 成功, 其他为失败错误码
	 */
	virtual int __stdcall DeleteValue(
		const wchar_t* pwszValueName
		) = 0;
};

/**
 * @brief 配置管理器
 */
MIDL_INTERFACE("83FB006E-CFCB-45be-8E12-80EE44C06A3E")
IKxEConfigMgr : public IUnknown
{
	/**
	 * @brief 初始化配置管理器
	 */
	virtual int __stdcall Initialize() = 0;

	/**
	 * @brief 反初始化配置管理器
	 */
	virtual int __stdcall Uninitialize() = 0;

	/**
	 * @brief 启动初始化配置管理器
	 */
	virtual int __stdcall Start() = 0;

	/**
	 * @brief 停止配置管理器
	 */
	virtual int __stdcall Stop() = 0;

	/**
	 * @brief 创建配置键,如果该键存在,则打开它
	 * @param[in] type 打开的键类型
	 * @param[in] pwszPath 需要打开的路径
	 * @param[out] ppConfigKey 用于存储打开的key
	 * @return 0 成功, 其他为失败错误码
	 */
	virtual int __stdcall CreateKey(
		KXE_CONFIG_TYPE type,
		const wchar_t* pwszPath,
		IKxEConfigKey** ppConfigKey
		) = 0;

	/**
	 * @brief 打开配置键,如果该键不存在,则失败
	 * @param[in] type 打开的键类型
	 * @param[in] pwszPath 需要打开的路径
	 * @param[out] ppConfigKey 用于存储打开的key
	 * @return 0 成功, 其他为失败错误码
	 */
	virtual int __stdcall OpenKey(
		KXE_CONFIG_TYPE type,
		const wchar_t* pwszPath,
		IKxEConfigKey** ppConfigKey
		) = 0;

	/**
	 * @brief 删除配置键
	 * @param[in] type 键类型
	 * @param[in] pwszPath 需要删除的路径
	 * @return 0 成功, 其他为失败错误码
	 */
	virtual int __stdcall DeleteKey(
		KXE_CONFIG_TYPE type,
		const wchar_t* pwszPath
		) = 0;
};

/**
 * @}
 */