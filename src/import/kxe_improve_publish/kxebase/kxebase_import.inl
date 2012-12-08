//////////////////////////////////////////////////////////////////////
///		@file		kxebase_import.inl
///		@author		luopeng
///		@date		2009-5-25 09:07:58
///	
///		@brief		∂ØÃ¨º”‘ÿkxebase.dll
//////////////////////////////////////////////////////////////////////
#pragma once
#include "kxe_base_loader.h"

inline int __stdcall kxe_base_init()
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_init();
}

inline int __stdcall kxe_base_init_client()
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_init_client();
}

inline int __stdcall kxe_base_start()
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_start();
}

inline int __stdcall kxe_base_stop()
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_stop();
}

inline int __stdcall kxe_base_fini()
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_fini();
}

inline int __stdcall kxe_base_get_system_method(
	const char* pszSystem,
	const char* pszMethod,
	void** ppMethodFunction
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_get_system_method(
		pszSystem,
		pszMethod,
		ppMethodFunction
		);
}

inline int __stdcall kxe_base_register_unhandled_exception_filter(
	LPTOP_LEVEL_EXCEPTION_FILTER lpExceptionFilter
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_register_unhandled_exception_filter(
		lpExceptionFilter
		);
}

inline int __stdcall kxe_base_unregister_unhandled_exception_filter(
	LPTOP_LEVEL_EXCEPTION_FILTER lpExceptionFilter
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_unregister_unhandled_exception_filter(
		lpExceptionFilter
		);
}

inline int __stdcall kxe_base_set_crash_post_option(
	const char* pszPostAddr,
	bool bShowGUI
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_set_crash_post_option(
		pszPostAddr,
		bShowGUI
		);
}

inline int __stdcall kxe_base_notify_activity(
	unsigned int uProductId,
	const wchar_t* pwszProductVersion
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_notify_activity(
		uProductId,
		pwszProductVersion
		);
}

inline int __stdcall kxe_base_create_config_key(
	kxe_config_key_t key, 
	const wchar_t* pwszPath, 
	kxe_config_key_t* pKey
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_create_config_key(
		key,
		pwszPath,
		pKey
		);
}

inline int __stdcall kxe_base_open_config_key(
	kxe_config_key_t key, 
	const wchar_t* pwszPath, 
	kxe_config_key_t* pKey
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_open_config_key(
		key,
		pwszPath,
		pKey
		);
}

inline int __stdcall kxe_base_close_config_key(
	kxe_config_key_t key
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_close_config_key(
		key
		);
}

inline int __stdcall kxe_base_get_config(
	kxe_config_key_t key, 
	const wchar_t* pwszPath, 
	const wchar_t* pwszValueName, 
	wchar_t* pwszValue,
	int nValueLength,
	int* pnReturnValueLength
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_get_config(
		key,
		pwszPath,
		pwszValueName,
		pwszValue,
		nValueLength,
		pnReturnValueLength
		);
}

inline int __stdcall kxe_base_set_config(
	kxe_config_key_t key,
	const wchar_t* pwszPath, 
	const wchar_t* pwszValueName, 
	const wchar_t* pwszValue,
	int nValueLength
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_set_config(
		key,
		pwszPath,
		pwszValueName,
		pwszValue,
		nValueLength
		);
}

inline int __stdcall kxe_base_delete_config_key(
	kxe_config_key_t key,
	const wchar_t* pwszPath
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_delete_config_key(
		key,
		pwszPath
		);
}

inline int __stdcall kxe_base_enum_config_key(
	kxe_config_key_t key,
	unsigned int uIndex,
	wchar_t* pwszBuffer,
	int nBufferLength,
	int* pnReturnBuffer
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_enum_config_key(
		key,
		uIndex,
		pwszBuffer,
		nBufferLength,
		pnReturnBuffer
		);
}

inline int __stdcall kxe_base_post_event(
	KXE_SUB_SYSTEM_ID subSystemId,
	short sEventId,
	const char* pszBuffer, 
	int nBufferSize
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_post_event(
		subSystemId,
		sEventId,
		pszBuffer,
		nBufferSize
		);
}

inline int __stdcall kxe_base_register_event_notify(
	KXE_SUB_SYSTEM_ID subSystemId,
	short sEventId, 
	IKxEEventNotify* pNotify
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_register_event_notify(
		subSystemId,
		sEventId,
		pNotify
		);
}

inline int __stdcall kxe_base_unregister_event_notify(
	KXE_SUB_SYSTEM_ID subSystemId,
	short sEventId,
	IKxEEventNotify* pNotify
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_unregister_event_notify(
		subSystemId,
		sEventId,
		pNotify
		);
}

inline int __stdcall kxe_base_register_uri_dispatch(
	const char* pszUri, 
	IKxERequestDispatch* pDispatch
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_register_uri_dispatch(
		pszUri,
		pDispatch
		);
}

inline int __stdcall kxe_base_unregister_uri_dispatch(
	const char* pszUri, 
	IKxERequestDispatch* pDispatch
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_unregister_uri_dispatch(
		pszUri,
		pDispatch
		);
}

inline int __stdcall kxe_base_stop_http()
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_stop_http();
}

inline int __stdcall kxe_base_resume_http()
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_resume_http();
}

inline int __stdcall kxe_base_register_web_path(
	const char* pszName,
	const char* pszDirectory
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_register_web_path(
		pszName,
		pszDirectory
		);
}

inline int __stdcall kxe_base_unregister_web_path(const char* pszName)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_unregister_web_path(
		pszName
		);
}

inline int __stdcall kxe_base_register_sp_dispatch(
	const wchar_t* pwszSPName,
	IKxERequestDispatch* pDispatch
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_register_sp_dispatch(
		pwszSPName,
		pDispatch
		);
}

inline int __stdcall kxe_base_unregister_sp_dispatch(
	const wchar_t* pwszSPName, 
	IKxERequestDispatch* pDispatch
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_unregister_sp_dispatch(
		pwszSPName,
		pDispatch
		);
}

inline int __stdcall kxe_base_stop_ipc()
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_stop_ipc();
}

inline int __stdcall kxe_base_resume_ipc()
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_resume_ipc();
}

inline int __stdcall kxe_base_create_job(
	kxe_job_type type, 
	IKxETask* pTask, 
	void* pParam, 
	kxe_job_t* pJobHandle
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_create_job(
		type,
		pTask,
		pParam,
		pJobHandle
		);
}

inline int __stdcall kxe_base_create_job_as_thread_token(
	kxe_job_type type, 
	IKxETask* pTask, 
	void* pParam, 
	kxe_job_t* pJobHandle
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_create_job_as_thread_token(
		type,
		pTask,
		pParam,
		pJobHandle
		);
}

inline int __stdcall kxe_base_set_and_run_control_cpu_job(
	kxe_job_t job,
	float fCpuPercent
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_set_and_run_control_cpu_job(
		job,
		fCpuPercent
		);
}

inline int __stdcall kxe_base_set_and_run_user_away_job(
	kxe_job_t job,
	unsigned int uSeconds
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_set_and_run_user_away_job(
		job,
		uSeconds
		);
}

inline int __stdcall kxe_base_run_job(
	kxe_job_t job
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_run_job(
		job
		);
}

inline int __stdcall kxe_base_stop_job(
	kxe_job_t job
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_stop_job(
		job
		);
}

inline int __stdcall kxe_base_pause_job(
	kxe_job_t job
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_pause_job(
		job
		);
}

inline int __stdcall kxe_base_resume_job(
	kxe_job_t job
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_resume_job(
		job
		);
}

inline int __stdcall kxe_base_wait_job(
	kxe_job_t job,
	unsigned int uMilliseconds
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_wait_job(
		job,
		uMilliseconds
		);
}

inline int __stdcall kxe_base_wait_multi_job(
	kxe_job_t* arrayJob,
	unsigned int uJobCount,
	bool bWaitAll,
	unsigned int uMilliseconds
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_wait_multi_job(
		arrayJob,
		uJobCount,
		bWaitAll,
		uMilliseconds
		);
}

inline int __stdcall kxe_base_get_job_exit_code(
	kxe_job_t job,
	int* pnExitCode
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_get_job_exit_code(
		job,
		pnExitCode
		);
}

inline int __stdcall kxe_base_get_job_status(
	kxe_job_t job,
	kxe_job_status* pStatus
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_get_job_status(
		job,
		pStatus
		);
}

inline int __stdcall kxe_base_close_job(
	kxe_job_t job
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_close_job(
		job
		);
}

inline int __stdcall kxe_base_create_debug_log(
	const char* pszLogObject, 
	kxe_log_t* pLog
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_create_debug_log(
		pszLogObject,
		pLog
		);
}

inline int __stdcall kxe_base_debug_log(
	kxe_log_t log, 
	KXE_LOG_LEVEL nLevel, 
	const char* pszMessage
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_debug_log(
		log,
		nLevel,
		pszMessage
		);
}

inline int __stdcall kxe_base_close_debug_log(
	kxe_log_t log
	)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_close_debug_log(
		log
		);
}

inline int __stdcall kxe_base_load_plugin_path(const wchar_t* pwszPath)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_load_plugin_path(
		pwszPath
		);
}

inline int __stdcall kxe_base_unload_plugin_path(const wchar_t* pwszPath)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_unload_plugin_path(
		pwszPath
		);
}

inline int __stdcall kxe_base_system_query_active_user_token(HANDLE* phToken)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_system_query_active_user_token(
		phToken
		);
}

inline int __stdcall kxe_base_uncompress_file(const wchar_t* pwszSourceFileName,
											  const wchar_t* pwszTargetDir)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_uncompress_file(
		pwszSourceFileName,
		pwszTargetDir
		);
}

inline int __stdcall kxe_base_load_plugin(const wchar_t* pwszPluginDllPath)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_load_plugin(
		pwszPluginDllPath
		);
}

inline int __stdcall kxe_base_unload_plugin(const wchar_t* pwszPluginDllPath)
{
	return KxEBaseDynamicLoaderSingleton::Instance().kxe_base_unload_plugin(
		pwszPluginDllPath
		);
}
