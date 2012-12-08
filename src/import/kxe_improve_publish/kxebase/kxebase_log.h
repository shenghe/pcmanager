//////////////////////////////////////////////////////////////////////
///		@file		kxebase_log.h
///		@author		luopeng
///		@date		2008-8-29 09:07:58
///	
///		@brief		kxebase的log相关函数定义
//////////////////////////////////////////////////////////////////////
#pragma once
#include "../kxelog/log_priority.h"

/**
 * @defgroup kxebase_log_group KXEngine Architecture Base SDK Log Interface
 * @{
 */

/// 定义日志对像类型
typedef void* kxe_log_t;

/**
* @brief 根据指定的名字,创建或打开指定的日志对像
* @param[in] pszLogObject 为指定的日志对像名字
* @param[out] pLog 成功创建出的日志对像句柄
* @remark 使用完毕需要使用kxe_base_close_debug_log关闭
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_create_debug_log(
	const char* pszLogObject, 
	kxe_log_t* pLog
	);

/**
* @brief 向指定的日志对像输出日志
* @param[in] log 为kxe_base_create_debug_log创建的日志对像
* @param[in] nLevel 此条日志的级别
* @param[in] pszMessage 实际的日志内容
* @remark 
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_debug_log(
	kxe_log_t log, 
	KXE_LOG_LEVEL nLevel, 
	const char* pszMessage
	);

/**
* @brief 关闭指定的日志对像
* @param[in] log 为kxe_base_create_debug_log创建的日志对像
* @remark 
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_close_debug_log(
	kxe_log_t log
	);

/**
 * @}
 */
