//////////////////////////////////////////////////////////////////////
///		@file		kxebase_export_fun_def.h
///		@author		luopeng
///		@date		2009-5-25 09:07:58
///	
///		@brief		动态加载kxebase.dll，定义导出的函数类型
//////////////////////////////////////////////////////////////////////
#pragma once

typedef int (__stdcall* type_kxe_base_init)();

typedef int (__stdcall* type_kxe_base_init_client)();

typedef int (__stdcall* type_kxe_base_start)();

typedef int (__stdcall* type_kxe_base_stop)();

typedef int (__stdcall* type_kxe_base_fini)();

typedef int (__stdcall* type_kxe_base_get_system_method)(
	const char* pszSystem,
	const char* pszMethod,
	void** ppMethodFunction
	);

typedef int (__stdcall* type_kxe_base_register_unhandled_exception_filter)(
	LPTOP_LEVEL_EXCEPTION_FILTER lpExceptionFilter
	);

typedef int (__stdcall* type_kxe_base_unregister_unhandled_exception_filter)(
	LPTOP_LEVEL_EXCEPTION_FILTER lpExceptionFilter
	);

typedef int (__stdcall* type_kxe_base_set_crash_post_option)(
	const char* pszPostAddr,
	bool bShowGUI
	);

typedef int (__stdcall* type_kxe_base_notify_activity)(
	unsigned int uProductId,
	const wchar_t* pwszProductVersion
	);

typedef int (__stdcall* type_kxe_base_create_config_key)(
	kxe_config_key_t key, 
	const wchar_t* pwszPath, 
	kxe_config_key_t* pKey
	);

typedef int (__stdcall* type_kxe_base_open_config_key)(
	kxe_config_key_t key, 
	const wchar_t* pwszPath, 
	kxe_config_key_t* pKey
	);

typedef int (__stdcall* type_kxe_base_close_config_key)(
	kxe_config_key_t key
	);

typedef int (__stdcall* type_kxe_base_get_config)(
	kxe_config_key_t key, 
	const wchar_t* pwszPath, 
	const wchar_t* pwszValueName, 
	wchar_t* pwszValue,
	int nValueLength,
	int* pnReturnValueLength
	);

typedef int (__stdcall* type_kxe_base_set_config)(
	kxe_config_key_t key,
	const wchar_t* pwszPath, 
	const wchar_t* pwszValueName, 
	const wchar_t* pwszValue,
	int nValueLength
	);

typedef int (__stdcall* type_kxe_base_delete_config_key)(
	kxe_config_key_t key,
	const wchar_t* pwszPath
	);

typedef int (__stdcall* type_kxe_base_enum_config_key)(
	kxe_config_key_t key,
	unsigned int uIndex,
	wchar_t* pwszBuffer,
	int nBufferLength,
	int* pnReturnBuffer
	);

typedef int (__stdcall* type_kxe_base_post_event)(
	KXE_SUB_SYSTEM_ID subSystemId,
	short sEventId,
	const char* pszBuffer, 
	int nBufferSize
	);

typedef int (__stdcall* type_kxe_base_register_event_notify)(
	KXE_SUB_SYSTEM_ID subSystemId,
	short sEventId, 
	IKxEEventNotify* pNotify
	);

typedef int (__stdcall* type_kxe_base_unregister_event_notify)(
	KXE_SUB_SYSTEM_ID subSystemId,
	short sEventId,
	IKxEEventNotify* pNotify
	);

typedef int (__stdcall* type_kxe_base_register_uri_dispatch)(
	const char* pszUri, 
	IKxERequestDispatch* pDispatch
	);

typedef int (__stdcall* type_kxe_base_unregister_uri_dispatch)(
	const char* pszUri, 
	IKxERequestDispatch* pDispatch
	);

typedef int (__stdcall* type_kxe_base_stop_http)();

typedef int (__stdcall* type_kxe_base_resume_http)();

typedef int (__stdcall* type_kxe_base_register_web_path)(
	const char* pszName,
	const char* pszDirectory
	);

typedef int (__stdcall* type_kxe_base_unregister_web_path)(
	const char* pszName
	);

typedef int (__stdcall* type_kxe_base_register_sp_dispatch)(
	const wchar_t* pwszSPName,
	IKxERequestDispatch* pDispatch
	);

typedef int (__stdcall* type_kxe_base_unregister_sp_dispatch)(
	const wchar_t* pwszSPName, 
	IKxERequestDispatch* pDispatch
	);

typedef int (__stdcall* type_kxe_base_stop_ipc)();

typedef int (__stdcall* type_kxe_base_resume_ipc)();

typedef int (__stdcall* type_kxe_base_create_job)(
	kxe_job_type type, 
	IKxETask* pTask, 
	void* pParam, 
	kxe_job_t* pJobHandle
	);

typedef int (__stdcall* type_kxe_base_create_job_as_thread_token)(
	kxe_job_type type, 
	IKxETask* pTask, 
	void* pParam, 
	kxe_job_t* pJobHandle
	);

typedef int (__stdcall* type_kxe_base_set_and_run_control_cpu_job)(
	kxe_job_t job,
	float fCpuPercent
	);

typedef int (__stdcall* type_kxe_base_set_and_run_user_away_job)(
	kxe_job_t job,
	unsigned int uSeconds
	);

typedef int (__stdcall* type_kxe_base_run_job)(
	kxe_job_t job
	);

typedef int (__stdcall* type_kxe_base_stop_job)(
	kxe_job_t job
	);

typedef int (__stdcall* type_kxe_base_pause_job)(
	kxe_job_t job
	);

typedef int (__stdcall* type_kxe_base_resume_job)(
	kxe_job_t job
	);

typedef int (__stdcall* type_kxe_base_wait_job)(
	kxe_job_t job,
	unsigned int uMilliseconds
	);

typedef int (__stdcall* type_kxe_base_wait_multi_job)(
	kxe_job_t* arrayJob,
	unsigned int uJobCount,
	bool bWaitAll,
	unsigned int uMilliseconds
	);

typedef int (__stdcall* type_kxe_base_get_job_exit_code)(
	kxe_job_t job,
	int* pnExitCode
	);

typedef int (__stdcall* type_kxe_base_get_job_status)(
	kxe_job_t job,
	kxe_job_status* pStatus
	);

typedef int (__stdcall* type_kxe_base_close_job)(
	kxe_job_t job
	);

typedef int (__stdcall* type_kxe_base_create_debug_log)(
	const char* pszLogObject, 
	kxe_log_t* pLog
	);

typedef int (__stdcall* type_kxe_base_debug_log)(
	kxe_log_t log, 
	KXE_LOG_LEVEL nLevel, 
	const char* pszMessage
	);

typedef int (__stdcall* type_kxe_base_close_debug_log)(
	kxe_log_t log
	);

typedef int (__stdcall* type_kxe_base_load_plugin_path)(
	const wchar_t* pwszPath
	);

typedef int (__stdcall* type_kxe_base_unload_plugin_path)(
	const wchar_t* pwszPath
	);

typedef int (__stdcall* type_kxe_base_system_query_active_user_token)(
	HANDLE* phToken
	);

typedef int (__stdcall* type_kxe_base_uncompress_file)(
	const wchar_t* pwszSourceFileName,
	const wchar_t* pwszTargetDir
	);

typedef int (__stdcall* type_kxe_base_load_plugin)(
	const wchar_t* pwszPluginDllPath
	);

typedef int (__stdcall* type_kxe_base_unload_plugin)(
	const wchar_t* pwszPluginDllPath
	);



