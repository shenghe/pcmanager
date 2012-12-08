//////////////////////////////////////////////////////////////////////
///		@file		kxebase.h
///		@author		luopeng
///		@date		2008-8-29 09:07:58
///	
///		@brief		kxe_base相关函数定义
//////////////////////////////////////////////////////////////////////
#pragma once
#include "../kxebase/kxebase_base.h"
#include "../kxebase/kxebase_http.h"
#include "../kxebase/kxebase_event.h"
#include "../kxebase/kxebase_log.h"
#include "../kxebase/kxebase_config.h"
#include "../kxebase/kxebase_job.h"
#include "../kxebase/kxebase_ipc.h"
#include "../kxesystem/kxesystem.h"
#include "../kxebase/log_msg.h"
#include "../kxeconfig/kxeconfig.h"

#ifndef KXE_BASE_FUNCTION_DEFINE

#if (!defined KXE_BASE_LINK_DLL && !defined _USRDLL) || defined KXE_BASE_NOT_LINK_DLL
#include "../kxebase/kxebase_import.inl"
#else
#pragma comment(lib, "kxebase.lib")
#endif

#endif



/**
* @page kxebase KXEngine Architecture Base SDK Interface
* @section kxebase_initialize SDK初始化与启动函数
* - @ref kxebase_initialize_group

* @section kxebase_ipc IPC相关函数
* - @ref kxeipc_sdk_interface_group

* @section kxebase_config_function 配置相关接口
* - @ref kxebase_config_group
* - @ref kxeconfig_group

* @section kxebase_event_function 事件管理相关接口
* - @ref kxebase_event_group
* - @ref kxeevent_group

* @section kxebase_http_function HTTP相关接口
* - @ref kxehttp_sdk_interface_group
* - @ref kxehttp_idl_interface_group

* @section kxebase_log_function 调试日志相关函数
* - @ref kxebase_log_group
* - @ref kxengine_debug_log_interface

* @section kxebase_job_function job管理相关函数
* - @ref kxebase_thread_group

* @section kxebase_serialize 传输数据序列化相关宏
* - @ref kxe_json_serialize_marco_group

* @section kxebase_system 系统相关兼容函数
* - @ref kxesystem_compatible_interface

* @section kxebase_example_list 使用示例列表
* - @ref kxeconfig_example "配置函数使用示例"
* - @ref kxeevent_example "事件管理使用示例"
* - @ref kxehttp_example "HTTP-RPC使用示例"
*/
