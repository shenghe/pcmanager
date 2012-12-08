//////////////////////////////////////////////////////////////////////
///		@file		kxebase_base.h
///		@author		luopeng
///		@date		2008-8-29 09:07:58
///	
///		@brief		kxebase的主函数定义
//////////////////////////////////////////////////////////////////////
#pragma once
#include <WinSock2.h>
#include <windows.h>

/**
 * @defgroup kxebase_initialize_group KXEngine Architecture Base SDK Initialize Interface
 * @{
 */

/**
 * @brief 初始化基础库,该函数必须也只能被调用一次
 * @remark 
 * @return 0 成功, 其他值为失败的错误码，
 */
int __stdcall kxe_base_init();

/**
 * @brief 初始化基础库,该函数必须也只能被调用一次，与kxe_base_init功能类似
 * @remark 该函数将遵循按需使用的原则加载相应组件，不需要的组件不再立即加载
 * @return 0 成功, 其他值为失败的错误码，
 */
int __stdcall kxe_base_init_client();

/**
 * @brief 启动基础库,该函数必须也只能被调用一次,其内部的核心组件将被加载启动
 * @remark 
 * @return 0 成功, 其他值为失败的错误码，
 */
int __stdcall kxe_base_start();

/**
 * @brief 停止基础库,该函数必须也只能被调用一次,其内部的核心组件将被加载停止
 * @remark 
 * @return 0 成功, 其他值为失败的错误码，
 */
int __stdcall kxe_base_stop();

/**
 * @brief 反初始化基础库,该函数必须也只能被调用一次
 * @remark 
 * @return 0 成功, 其他值为失败的错误码，
 */
int __stdcall kxe_base_fini();

/**
 * @brief 获取基础库加载的扩展SDK
 * @remark 
 * @param[in] pszSystem 为需要获取SDK的子系统的名字
 * @param[in] pszMethod 为需要获取SDK的函数名
 * @param[out] ppMethodFunction 为成功获取的函数指针,其类型调用者必须知道
 * @return 0 成功, 其他值为失败的错误码
 */
int __stdcall kxe_base_get_system_method(
	const char* pszSystem,
	const char* pszMethod,
	void** ppMethodFunction
	);

/**
 * @brief 向基础库注册异常处理勾子，用于在崩溃之前进行处理，与系统不同的是，将会忽略返回值
 * @param[in] lpExceptionFilter 进行异常处理的函数
 * @return 0 成功， 其他为失败错误码
 * @remark 当注册失败时，需要自己调用SetUnhandledExceptionFilter进行处理
 */
int __stdcall kxe_base_register_unhandled_exception_filter(
	LPTOP_LEVEL_EXCEPTION_FILTER lpExceptionFilter
	);

/**
 * @brief 向基础库取水异常处理勾子
 * @param[in] lpExceptionFilter 进行异常处理的函数
 * @return 0 成功， 其他为失败错误码
 */
int __stdcall kxe_base_unregister_unhandled_exception_filter(
	LPTOP_LEVEL_EXCEPTION_FILTER lpExceptionFilter
	);

/**
 * @brief 提供设置crash后post的路径及是否显示GUI的设置
 * @param[in] pszPostAddr HTTP服务的地址
 * @param[in] bShowGUI 是否显示崩溃界面，由用户决定上报，false则不显示直接上报
 */
int __stdcall kxe_base_set_crash_post_option(
	const char* pszPostAddr,
	bool bShowGUI
	);

/**
 * @brief 通知活跃度
 * @param[in] uProductId 报活跃的产品ID
 * @param[in] pwszProductVersion 报活的产品版本
 */
int __stdcall kxe_base_notify_activity(
    unsigned int uProductId,
	const wchar_t* pwszProductVersion
	);

/**
 * @brief 解压文件
 * @param[in] pwszSourceFileName 需要进行解压的文件
 * @param[in] pwszTargetDir 需要解压到的目标目录
 */
int __stdcall kxe_base_uncompress_file(
	const wchar_t* pwszSourceFileName,
	const wchar_t* pwszTargetDir
	);

/**
 * @breif 加载指定的Plugin SDK
 * @param[in] pwszPluginDllPath 需要加载的plugin sdk
 */
int __stdcall kxe_base_load_plugin(
	const wchar_t* pwszPluginDllPath
	);

/**
 * @breif 加载指定的Plugin SDK
 * @param[in] pwszPluginDllPath 需要加载的plugin sdk
 */
int __stdcall kxe_base_unload_plugin(
	const wchar_t* pwszPluginDllPath
	);
/**
 * @}
 */
