//////////////////////////////////////////////////////////////////////
///		@file		kxe_base_loader.h
///		@author		luopeng
///		@date		2009-5-25 09:07:58
///	
///		@brief		∂ØÃ¨º”‘ÿkxebase.dll
//////////////////////////////////////////////////////////////////////
#pragma once
#include "../common/kxe_thread.h"
#include "../scom/scom/singleton.h"
#include "../kxeerror/kxeerror.h"
#include "kxeloaddllutility.h"
#include "kxebase_export_fun_def.h"
#include "kxecommon/kxecommon.h"

class KxEBaseDynamicLoader
{
public:
	KxEBaseDynamicLoader()
	{
		m_hModule = NULL;
		m_bSelfLoaded = false;
		ClearFunction();

		m_hModule = GetModuleHandle(_TEXT(KXECOMMON_CONFIG_KXEBASE_DLL_NAME));
		if (m_hModule != NULL)
		{
			int nRet = GetFunction();
			if (nRet != 0)
			{
				ClearFunction();
				m_hModule = NULL;
			}
		}
	}

	int kxe_base_init()
	{
		int nRet = E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		if (m_hModule == NULL)
		{
			nRet = Load();
			if (nRet != 0)
			{
				return nRet;
			}

			KxESearchFile searchFile(_TEXT(KXECOMMON_CONFIG_KXEBASE_DLL_NAME));
			TCHAR szFilePath[MAX_PATH] = {0};
			DWORD dwRet = GetModuleFileName(m_hModule, szFilePath, MAX_PATH);
			if (dwRet != 0 && dwRet != MAX_PATH)
			{
				searchFile.WritePathToReg(szFilePath);
			}
		}
		//////////////////////////////////////////////////////////////////////////
		if (m_pfn_kxe_base_init)
		{
			nRet = m_pfn_kxe_base_init();
			if (nRet != 0 && m_bSelfLoaded)
			{
				Unload();
			}
		}	
		return nRet;
	}

	int kxe_base_init_client()
	{
		int nRet = E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		if (m_hModule == NULL)
		{
			nRet = Load();
			if (nRet != 0)
			{
				return nRet;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		if (m_pfn_kxe_base_init_client)
		{
			nRet = m_pfn_kxe_base_init_client();
			if (nRet != 0 && m_bSelfLoaded)
			{
				Unload();
			}
		}
		return nRet;
	}

	int kxe_base_fini()
	{
		if (m_hModule == NULL)
		{
			return E_KXEBASE_NOT_INITED;
		}

		int nRet = m_pfn_kxe_base_fini();
		if (nRet == 0 && m_bSelfLoaded)
		{
			Unload();
		}

		return nRet;
	}

	int kxe_base_start()
	{
		if (m_pfn_kxe_base_start == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_start();
	}

	int kxe_base_stop()
	{
		if (m_pfn_kxe_base_stop == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_stop();
	}

	int kxe_base_get_system_method(
		const char* pszSystem,
		const char* pszMethod,
		void** ppMethodFunction
		)
	{
		if (m_pfn_get_system_method == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_get_system_method(pszSystem, pszMethod, ppMethodFunction);
	}

	int kxe_base_register_unhandled_exception_filter(
		LPTOP_LEVEL_EXCEPTION_FILTER lpExceptionFilter
		)
	{
		if (m_pfn_kxe_base_register_unhandled_exception_filter == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_register_unhandled_exception_filter(lpExceptionFilter);
	}

	int kxe_base_unregister_unhandled_exception_filter(
		LPTOP_LEVEL_EXCEPTION_FILTER lpExceptionFilter
		)
	{
		if (m_pfn_kxe_base_unregister_unhandled_exception_filter == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_unregister_unhandled_exception_filter(lpExceptionFilter);
	}

	int __stdcall kxe_base_set_crash_post_option(
		const char* pszPostAddr,
		bool bShowGUI
		)
	{
		if (m_pfn_kxe_base_set_crash_post_option == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_set_crash_post_option(pszPostAddr, bShowGUI);
	}

	int __stdcall kxe_base_notify_activity(
		unsigned int uProductId,
		const wchar_t* pwszProductVersion
		)
	{
		if (m_pfn_kxe_base_notify_activity == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_notify_activity(uProductId, pwszProductVersion);
	}


	int kxe_base_create_config_key(
		kxe_config_key_t key, 
		const wchar_t* pwszPath, 
		kxe_config_key_t* pKey
		)
	{
		if (m_pfn_kxe_base_create_config_key == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_create_config_key(
			key, 
			pwszPath, 
			pKey
			);
	}

	int __stdcall kxe_base_open_config_key(
		kxe_config_key_t key, 
		const wchar_t* pwszPath, 
		kxe_config_key_t* pKey
		)
	{
		if (m_pfn_kxe_base_open_config_key == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_open_config_key(
			key,
			pwszPath,
			pKey
			);
	}

	int __stdcall kxe_base_close_config_key(
		kxe_config_key_t key
		)
	{
		if (m_pfn_kxe_base_close_config_key == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_close_config_key(key);
	}

	int __stdcall kxe_base_get_config(
		kxe_config_key_t key, 
		const wchar_t* pwszPath, 
		const wchar_t* pwszValueName, 
		wchar_t* pwszValue,
		int nValueLength,
		int* pnReturnValueLength
		)
	{
		if (m_pfn_kxe_base_get_config == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_get_config(
			key,
			pwszPath,
			pwszValueName,
			pwszValue,
			nValueLength,
			pnReturnValueLength
			);
	}

	int __stdcall kxe_base_set_config(
		kxe_config_key_t key,
		const wchar_t* pwszPath, 
		const wchar_t* pwszValueName, 
		const wchar_t* pwszValue,
		int nValueLength
		)
	{
		if (m_pfn_kxe_base_set_config == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_set_config(
			key,
			pwszPath,
			pwszValueName,
			pwszValue,
			nValueLength
			);
	}

	int __stdcall kxe_base_delete_config_key(
		kxe_config_key_t key,
		const wchar_t* pwszPath
		)
	{
		if (m_pfn_kxe_base_delete_config_key == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_delete_config_key(
			key, 
			pwszPath
			);
	}

	int __stdcall kxe_base_enum_config_key(
		kxe_config_key_t key,
		unsigned int uIndex,
		wchar_t* pwszBuffer,
		int nBufferLength,
		int* pnReturnBuffer
		)
	{
		if (m_pfn_kxe_base_enum_config_key == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_enum_config_key(
			key,
			uIndex,
			pwszBuffer,
			nBufferLength,
			pnReturnBuffer
			);
	}

	int kxe_base_post_event(
		KXE_SUB_SYSTEM_ID subSystemId,
		short sEventId,
		const char* pszBuffer, 
		int nBufferSize
		)
	{
		if (m_pfn_kxe_base_post_event == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_post_event(
			subSystemId, 
			sEventId, 
			pszBuffer, 
			nBufferSize
			);
	}

	int kxe_base_register_event_notify(
		KXE_SUB_SYSTEM_ID subSystemId,
		short sEventId, 
		IKxEEventNotify* pNotify
		)
	{
		if (m_pfn_kxe_base_register_event_notify == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_register_event_notify(
			subSystemId,
			sEventId,
			pNotify
			);
	}

	int kxe_base_unregister_event_notify(
		KXE_SUB_SYSTEM_ID subSystemId,
		short sEventId,
		IKxEEventNotify* pNotify
		)
	{
		if (m_pfn_kxe_base_unregister_event_notify == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_unregister_event_notify(
			subSystemId,
			sEventId,
			pNotify
			);
	}

	int kxe_base_register_uri_dispatch(
		const char* pszUri, 
		IKxERequestDispatch* pDispatch
		)
	{
		if (m_pfn_kxe_base_register_uri_dispatch == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_register_uri_dispatch(
			pszUri,
			pDispatch
			);
	}

	int kxe_base_unregister_uri_dispatch(
		const char* pszUri, 
		IKxERequestDispatch* pDispatch
		)
	{
		if (m_pfn_kxe_base_unregister_uri_dispatch == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_unregister_uri_dispatch(
			pszUri,
			pDispatch
			);
	}

	int kxe_base_stop_http()
	{
		if (m_pfn_kxe_base_stop_http == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_stop_http();
	}

	int kxe_base_resume_http()
	{
		if (m_pfn_kxe_base_resume_http == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_resume_http();
	}

	int kxe_base_register_web_path(
		const char* pszName,
		const char* pszDirectory
		)
	{
		if (m_pfn_kxe_base_register_web_path == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_register_web_path(
			pszName,
			pszDirectory
			);
	}

	int kxe_base_unregister_web_path(const char* pszName)
	{
		if (m_pfn_kxe_base_unregister_web_path == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_unregister_web_path(pszName);
	}
	
	int kxe_base_register_sp_dispatch(
		const wchar_t* pwszSPName,
		IKxERequestDispatch* pDispatch
		)
	{
		if (m_pfn_kxe_base_register_sp_dispatch == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_register_sp_dispatch(
			pwszSPName, 
			pDispatch
			);
	}

	int kxe_base_unregister_sp_dispatch(
		const wchar_t* pwszSPName, 
		IKxERequestDispatch* pDispatch
		)
	{
		if (m_pfn_kxe_base_unregister_sp_dispatch == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_unregister_sp_dispatch(
			pwszSPName,
			pDispatch
			);
	}

	int kxe_base_stop_ipc()
	{
		if (m_pfn_kxe_base_stop_ipc == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_stop_ipc();
	}

	int kxe_base_resume_ipc()
	{
		if (m_pfn_kxe_base_resume_ipc == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_resume_ipc();
	}

	int kxe_base_create_job(
		kxe_job_type type, 
		IKxETask* pTask, 
		void* pParam, 
		kxe_job_t* pJobHandle
		)
	{
		if (m_pfn_kxe_base_create_job == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_create_job(
			type,
			pTask,
			pParam,
			pJobHandle
			);
	}

	int kxe_base_create_job_as_thread_token(
		kxe_job_type type, 
		IKxETask* pTask, 
		void* pParam, 
		kxe_job_t* pJobHandle
		)
	{
		if (m_pfn_kxe_base_create_job_as_thread_token == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_create_job_as_thread_token(
			type,
			pTask,
			pParam,
			pJobHandle
			);
	}

	int kxe_base_set_and_run_control_cpu_job(
		kxe_job_t job,
		float fCpuPercent
		)
	{
		if (m_pfn_kxe_base_set_and_run_control_cpu_job == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_set_and_run_control_cpu_job(
			job,
			fCpuPercent
			);
	}

	int kxe_base_set_and_run_user_away_job(
		kxe_job_t job,
		unsigned int uSeconds
		)
	{
		if (m_pfn_kxe_base_set_and_run_user_away_job == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_set_and_run_user_away_job(
			job,
			uSeconds
			);
	}

	int kxe_base_run_job(
		kxe_job_t job
		)
	{
		if (m_pfn_kxe_base_run_job == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_run_job(job);
	}

	int kxe_base_stop_job(
		kxe_job_t job
		)
	{
		if (m_pfn_kxe_base_stop_job == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_stop_job(job);
	}

	int kxe_base_pause_job(
		kxe_job_t job
		)
	{
		if (m_pfn_kxe_base_pause_job == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_pause_job(job);
	}

	int kxe_base_resume_job(
		kxe_job_t job
		)
	{
		if (m_pfn_kxe_base_resume_job == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_resume_job(job);
	}

	int kxe_base_wait_job(
		kxe_job_t job,
		unsigned int uMilliseconds
		)
	{
		if (m_pfn_kxe_base_wait_job == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_wait_job(
			job, 
			uMilliseconds
			);
	}

	int kxe_base_wait_multi_job(
		kxe_job_t* arrayJob,
		unsigned int uJobCount,
		bool bWaitAll,
		unsigned int uMilliseconds
		)
	{
		if (m_pfn_kxe_base_wait_multi_job == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_wait_multi_job(
			arrayJob,
			uJobCount,
			bWaitAll,
			uMilliseconds
			);
	}

	int kxe_base_get_job_exit_code(
		kxe_job_t job,
		int* pnExitCode
		)
	{
		if (m_pfn_kxe_base_get_job_exit_code == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_get_job_exit_code(
			job,
			pnExitCode
			);
	}

	int kxe_base_get_job_status(
		kxe_job_t job,
		kxe_job_status* pStatus
		)
	{
		if (m_pfn_kxe_base_get_job_status == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_get_job_status(
			job,
			pStatus
			);
	}

	int kxe_base_close_job(
		kxe_job_t job
		)
	{
		if (m_pfn_kxe_base_close_job == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_close_job(job);
	}

	int kxe_base_create_debug_log(
		const char* pszLogObject, 
		kxe_log_t* pLog
		)
	{
		if (m_pfn_kxe_base_create_debug_log == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_create_debug_log(
			pszLogObject,
			pLog
			);
	}

	int kxe_base_debug_log(
		kxe_log_t log, 
		KXE_LOG_LEVEL nLevel, 
		const char* pszMessage
		)
	{
		if (m_pfn_kxe_base_debug_log == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_debug_log(
			log,
			nLevel,
			pszMessage
			);
	}

	int kxe_base_close_debug_log(
		kxe_log_t log
		)
	{
		if (m_pfn_kxe_base_close_debug_log == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_close_debug_log(log);
	}

	int kxe_base_load_plugin_path(const wchar_t* pwszPath)
	{
		if (m_pfn_kxe_base_load_plugin_path == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_load_plugin_path(pwszPath);
	}

	int kxe_base_unload_plugin_path(const wchar_t* pwszPath)
	{
		if (m_pfn_kxe_base_unload_plugin_path == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_unload_plugin_path(pwszPath);
	}

	int kxe_base_system_query_active_user_token(HANDLE* phToken)
	{
		if (m_pfn_kxe_base_unload_plugin_path == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_system_query_active_user_token(phToken);
	}

	int kxe_base_uncompress_file(const wchar_t* pwszSourceFileName,
								 const wchar_t* pwszTargetDir)
	{
		if (m_pfn_kxe_base_uncompress_file == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_uncompress_file(
			pwszSourceFileName,
			pwszTargetDir
			);
	}

	int kxe_base_load_plugin(const wchar_t* pwszPluginDllPath)
	{
		if (m_pfn_kxe_base_load_plugin == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		return m_pfn_kxe_base_load_plugin(pwszPluginDllPath);
	}

	int kxe_base_unload_plugin(const wchar_t* pwszPluginDllPath)
	{
		if (m_pfn_kxe_base_unload_plugin == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}
		
		return m_pfn_kxe_base_unload_plugin(pwszPluginDllPath);
	}
	

private:
	int Load(bool bRegFirst = true)
	{
		KxEThreadMutexGuard guard(m_mutex);
		if (m_hModule != NULL)
		{
			return -1;
		}

		m_hModule = GetModuleHandle(_TEXT(KXECOMMON_CONFIG_KXEBASE_DLL_NAME));
		if (m_hModule == NULL)
		{
			m_hModule = KxELoadDllUtility::AutoSearchLoadLibrary(
				_TEXT(KXECOMMON_CONFIG_KXEBASE_DLL_NAME), 
				bRegFirst
				);
			if (m_hModule != NULL)
			{
				m_bSelfLoaded = true;
			}
			else
			{
				return GetLastError();
			}
		}
		else
		{
			m_bSelfLoaded = false;
		}

		int nRet = -1;
		if (m_hModule)
		{
			nRet = GetFunction();
			if (nRet != 0)
			{
				ClearFunction();
				if (m_bSelfLoaded)
				{
					FreeLibrary(m_hModule);
					m_hModule = NULL;
					m_bSelfLoaded = false;
				}				
			}
		}

		return nRet;
	}

	void Unload()
	{
		KxEThreadMutexGuard guard(m_mutex);

		ClearFunction();
		if (m_bSelfLoaded && m_hModule != NULL)
		{
			FreeLibrary(m_hModule);
		}

		m_bSelfLoaded = false;
		m_hModule = NULL;
	}

	int GetFunction()
	{
		m_pfn_kxe_base_init = (type_kxe_base_init)GetProcAddress(m_hModule, "kxe_base_init");
		if (m_pfn_kxe_base_init == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		m_pfn_kxe_base_init_client = (type_kxe_base_init_client)GetProcAddress(m_hModule, "kxe_base_init_client");
		if (m_pfn_kxe_base_init_client == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}


		m_pfn_kxe_base_start = (type_kxe_base_start)GetProcAddress(m_hModule, "kxe_base_start");
		if (m_pfn_kxe_base_start == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		m_pfn_kxe_base_stop = (type_kxe_base_stop)GetProcAddress(m_hModule, "kxe_base_stop");
		if (m_pfn_kxe_base_stop == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		m_pfn_kxe_base_fini = (type_kxe_base_fini)GetProcAddress(m_hModule, "kxe_base_fini");
		if (m_pfn_kxe_base_fini == NULL)
		{
			return E_KXEBASE_FUNCTION_NOT_IMPLEMENTED;
		}

		m_pfn_get_system_method = (type_kxe_base_get_system_method)GetProcAddress(m_hModule, "kxe_base_get_system_method");
		m_pfn_kxe_base_register_unhandled_exception_filter = (type_kxe_base_register_unhandled_exception_filter)GetProcAddress(m_hModule, "kxe_base_register_unhandled_exception_filter");
		m_pfn_kxe_base_unregister_unhandled_exception_filter = (type_kxe_base_unregister_unhandled_exception_filter)GetProcAddress(m_hModule, "kxe_base_unregister_unhandled_exception_filter");
		m_pfn_kxe_base_set_crash_post_option = (type_kxe_base_set_crash_post_option)GetProcAddress(m_hModule, "kxe_base_set_crash_post_option");
		m_pfn_kxe_base_notify_activity = (type_kxe_base_notify_activity)GetProcAddress(m_hModule, "kxe_base_notify_activity");
		m_pfn_kxe_base_create_config_key = (type_kxe_base_create_config_key)GetProcAddress(m_hModule, "kxe_base_create_config_key");
		m_pfn_kxe_base_open_config_key = (type_kxe_base_open_config_key)GetProcAddress(m_hModule, "kxe_base_open_config_key");
		m_pfn_kxe_base_close_config_key = (type_kxe_base_close_config_key)GetProcAddress(m_hModule, "kxe_base_close_config_key");
		m_pfn_kxe_base_get_config = (type_kxe_base_get_config)GetProcAddress(m_hModule, "kxe_base_get_config");
		m_pfn_kxe_base_set_config = (type_kxe_base_set_config)GetProcAddress(m_hModule, "kxe_base_set_config");
		m_pfn_kxe_base_delete_config_key = (type_kxe_base_delete_config_key)GetProcAddress(m_hModule, "kxe_base_delete_config_key");
		m_pfn_kxe_base_enum_config_key = (type_kxe_base_enum_config_key)GetProcAddress(m_hModule, "kxe_base_enum_config_key");
		m_pfn_kxe_base_post_event = (type_kxe_base_post_event)GetProcAddress(m_hModule, "kxe_base_post_event");
		m_pfn_kxe_base_register_event_notify = (type_kxe_base_register_event_notify)GetProcAddress(m_hModule, "kxe_base_register_event_notify");
		m_pfn_kxe_base_unregister_event_notify = (type_kxe_base_unregister_event_notify)GetProcAddress(m_hModule, "kxe_base_unregister_event_notify");
		m_pfn_kxe_base_register_uri_dispatch = (type_kxe_base_register_uri_dispatch)GetProcAddress(m_hModule, "kxe_base_register_uri_dispatch");
		m_pfn_kxe_base_unregister_uri_dispatch = (type_kxe_base_unregister_uri_dispatch)GetProcAddress(m_hModule, "kxe_base_unregister_uri_dispatch");
		m_pfn_kxe_base_stop_http = (type_kxe_base_stop_http)GetProcAddress(m_hModule, "kxe_base_stop_http");
		m_pfn_kxe_base_resume_http = (type_kxe_base_resume_http)GetProcAddress(m_hModule, "kxe_base_resume_http");
		m_pfn_kxe_base_register_web_path = (type_kxe_base_register_web_path)GetProcAddress(m_hModule, "kxe_base_register_web_path");
		m_pfn_kxe_base_unregister_web_path = (type_kxe_base_unregister_web_path)GetProcAddress(m_hModule, "kxe_base_unregister_web_path");
		m_pfn_kxe_base_register_sp_dispatch = (type_kxe_base_register_sp_dispatch)GetProcAddress(m_hModule, "kxe_base_register_sp_dispatch");
		m_pfn_kxe_base_unregister_sp_dispatch = (type_kxe_base_unregister_sp_dispatch)GetProcAddress(m_hModule, "kxe_base_unregister_sp_dispatch");
		m_pfn_kxe_base_stop_ipc = (type_kxe_base_stop_ipc)GetProcAddress(m_hModule, "kxe_base_stop_ipc");
		m_pfn_kxe_base_resume_ipc = (type_kxe_base_resume_ipc)GetProcAddress(m_hModule, "kxe_base_resume_ipc");
		m_pfn_kxe_base_create_job = (type_kxe_base_create_job)GetProcAddress(m_hModule, "kxe_base_create_job");
		m_pfn_kxe_base_create_job_as_thread_token = (type_kxe_base_create_job_as_thread_token)GetProcAddress(m_hModule, "kxe_base_create_job_as_thread_token");
		m_pfn_kxe_base_set_and_run_control_cpu_job = (type_kxe_base_set_and_run_control_cpu_job)GetProcAddress(m_hModule, "kxe_base_set_and_run_control_cpu_job");
		m_pfn_kxe_base_set_and_run_user_away_job = (type_kxe_base_set_and_run_user_away_job)GetProcAddress(m_hModule, "kxe_base_set_and_run_user_away_job");
		m_pfn_kxe_base_run_job = (type_kxe_base_run_job)GetProcAddress(m_hModule, "kxe_base_run_job");
		m_pfn_kxe_base_stop_job = (type_kxe_base_stop_job)GetProcAddress(m_hModule, "kxe_base_stop_job");
		m_pfn_kxe_base_pause_job = (type_kxe_base_pause_job)GetProcAddress(m_hModule, "kxe_base_pause_job");
		m_pfn_kxe_base_resume_job = (type_kxe_base_resume_job)GetProcAddress(m_hModule, "kxe_base_resume_job");
		m_pfn_kxe_base_wait_job = (type_kxe_base_wait_job)GetProcAddress(m_hModule, "kxe_base_wait_job");
		m_pfn_kxe_base_wait_multi_job = (type_kxe_base_wait_multi_job)GetProcAddress(m_hModule, "kxe_base_wait_multi_job");
		m_pfn_kxe_base_get_job_exit_code = (type_kxe_base_get_job_exit_code)GetProcAddress(m_hModule, "kxe_base_get_job_exit_code");
		m_pfn_kxe_base_get_job_status = (type_kxe_base_get_job_status)GetProcAddress(m_hModule, "kxe_base_get_job_status");
		m_pfn_kxe_base_close_job = (type_kxe_base_close_job)GetProcAddress(m_hModule, "kxe_base_close_job");
		m_pfn_kxe_base_create_debug_log = (type_kxe_base_create_debug_log)GetProcAddress(m_hModule, "kxe_base_create_debug_log");
		m_pfn_kxe_base_debug_log = (type_kxe_base_debug_log)GetProcAddress(m_hModule, "kxe_base_debug_log");
		m_pfn_kxe_base_close_debug_log = (type_kxe_base_close_debug_log)GetProcAddress(m_hModule, "kxe_base_close_debug_log");
		m_pfn_kxe_base_load_plugin_path = (type_kxe_base_load_plugin_path)GetProcAddress(m_hModule, "kxe_base_load_plugin_path");
		m_pfn_kxe_base_unload_plugin_path = (type_kxe_base_unload_plugin_path)GetProcAddress(m_hModule, "kxe_base_unload_plugin_path");
		m_pfn_kxe_base_system_query_active_user_token = (type_kxe_base_system_query_active_user_token)GetProcAddress(m_hModule, "kxe_base_system_query_active_user_token");
		m_pfn_kxe_base_uncompress_file = (type_kxe_base_uncompress_file)GetProcAddress(m_hModule, "kxe_base_uncompress_file");
		m_pfn_kxe_base_load_plugin = (type_kxe_base_load_plugin)GetProcAddress(m_hModule, "kxe_base_load_plugin");
		m_pfn_kxe_base_unload_plugin = (type_kxe_base_unload_plugin)GetProcAddress(m_hModule, "kxe_base_unload_plugin");

		return 0;
	}

	void ClearFunction()
	{
		m_pfn_kxe_base_init = NULL;
		m_pfn_kxe_base_init_client = NULL;
		m_pfn_kxe_base_start = NULL;
		m_pfn_kxe_base_stop = NULL;
		m_pfn_kxe_base_fini = NULL;
		m_pfn_get_system_method = NULL;
		m_pfn_kxe_base_register_unhandled_exception_filter = NULL;
		m_pfn_kxe_base_unregister_unhandled_exception_filter = NULL;
		m_pfn_kxe_base_set_crash_post_option = NULL;
		m_pfn_kxe_base_notify_activity = NULL;
		m_pfn_kxe_base_create_config_key = NULL;
		m_pfn_kxe_base_open_config_key = NULL;
		m_pfn_kxe_base_close_config_key = NULL;
		m_pfn_kxe_base_get_config = NULL;
		m_pfn_kxe_base_set_config = NULL;
		m_pfn_kxe_base_delete_config_key = NULL;
		m_pfn_kxe_base_enum_config_key = NULL;
		m_pfn_kxe_base_post_event = NULL;
		m_pfn_kxe_base_register_event_notify = NULL;
		m_pfn_kxe_base_unregister_event_notify = NULL;
		m_pfn_kxe_base_register_uri_dispatch = NULL;
		m_pfn_kxe_base_unregister_uri_dispatch = NULL;
		m_pfn_kxe_base_stop_http = NULL;
		m_pfn_kxe_base_resume_http = NULL;
		m_pfn_kxe_base_register_web_path = NULL;
		m_pfn_kxe_base_unregister_web_path = NULL;
		m_pfn_kxe_base_register_sp_dispatch = NULL;
		m_pfn_kxe_base_unregister_sp_dispatch = NULL;
		m_pfn_kxe_base_stop_ipc = NULL;
		m_pfn_kxe_base_resume_ipc = NULL;
		m_pfn_kxe_base_create_job = NULL;
		m_pfn_kxe_base_create_job_as_thread_token = NULL;
		m_pfn_kxe_base_set_and_run_control_cpu_job = NULL;
		m_pfn_kxe_base_set_and_run_user_away_job = NULL;
		m_pfn_kxe_base_run_job = NULL;
		m_pfn_kxe_base_stop_job = NULL;
		m_pfn_kxe_base_pause_job = NULL;
		m_pfn_kxe_base_resume_job = NULL;
		m_pfn_kxe_base_wait_job = NULL;
		m_pfn_kxe_base_wait_multi_job = NULL;
		m_pfn_kxe_base_get_job_exit_code = NULL;
		m_pfn_kxe_base_get_job_status = NULL;
		m_pfn_kxe_base_close_job = NULL;
		m_pfn_kxe_base_create_debug_log = NULL;
		m_pfn_kxe_base_debug_log = NULL;
		m_pfn_kxe_base_close_debug_log = NULL;
		m_pfn_kxe_base_load_plugin_path = NULL;
		m_pfn_kxe_base_unload_plugin_path = NULL;
		m_pfn_kxe_base_uncompress_file = NULL;
		m_pfn_kxe_base_load_plugin = NULL;
		m_pfn_kxe_base_unload_plugin = NULL;
	}
private:
	HMODULE m_hModule;
	bool m_bSelfLoaded;
	KxEThreadMutex m_mutex;

	type_kxe_base_init m_pfn_kxe_base_init;
	type_kxe_base_init_client m_pfn_kxe_base_init_client;
	type_kxe_base_start m_pfn_kxe_base_start;
	type_kxe_base_stop m_pfn_kxe_base_stop;
	type_kxe_base_fini m_pfn_kxe_base_fini;
	type_kxe_base_get_system_method m_pfn_get_system_method;
	type_kxe_base_register_unhandled_exception_filter m_pfn_kxe_base_register_unhandled_exception_filter;
	type_kxe_base_unregister_unhandled_exception_filter m_pfn_kxe_base_unregister_unhandled_exception_filter;
	type_kxe_base_set_crash_post_option m_pfn_kxe_base_set_crash_post_option;
	type_kxe_base_notify_activity m_pfn_kxe_base_notify_activity;
	type_kxe_base_create_config_key m_pfn_kxe_base_create_config_key;
	type_kxe_base_open_config_key m_pfn_kxe_base_open_config_key;
	type_kxe_base_close_config_key m_pfn_kxe_base_close_config_key;
	type_kxe_base_get_config m_pfn_kxe_base_get_config;
	type_kxe_base_set_config m_pfn_kxe_base_set_config;
	type_kxe_base_delete_config_key m_pfn_kxe_base_delete_config_key;
	type_kxe_base_enum_config_key m_pfn_kxe_base_enum_config_key;
	type_kxe_base_post_event m_pfn_kxe_base_post_event;
	type_kxe_base_register_event_notify m_pfn_kxe_base_register_event_notify;
	type_kxe_base_unregister_event_notify m_pfn_kxe_base_unregister_event_notify;
	type_kxe_base_register_uri_dispatch m_pfn_kxe_base_register_uri_dispatch;
	type_kxe_base_unregister_uri_dispatch m_pfn_kxe_base_unregister_uri_dispatch;
	type_kxe_base_stop_http m_pfn_kxe_base_stop_http;
	type_kxe_base_resume_http m_pfn_kxe_base_resume_http;
	type_kxe_base_register_web_path m_pfn_kxe_base_register_web_path;
	type_kxe_base_unregister_web_path m_pfn_kxe_base_unregister_web_path;
	type_kxe_base_register_sp_dispatch m_pfn_kxe_base_register_sp_dispatch;
	type_kxe_base_unregister_sp_dispatch m_pfn_kxe_base_unregister_sp_dispatch;
	type_kxe_base_stop_ipc m_pfn_kxe_base_stop_ipc;
	type_kxe_base_resume_ipc m_pfn_kxe_base_resume_ipc;
	type_kxe_base_create_job m_pfn_kxe_base_create_job;
	type_kxe_base_create_job_as_thread_token m_pfn_kxe_base_create_job_as_thread_token;
	type_kxe_base_set_and_run_control_cpu_job m_pfn_kxe_base_set_and_run_control_cpu_job;
	type_kxe_base_set_and_run_user_away_job m_pfn_kxe_base_set_and_run_user_away_job;
	type_kxe_base_run_job m_pfn_kxe_base_run_job;
	type_kxe_base_stop_job m_pfn_kxe_base_stop_job;
	type_kxe_base_pause_job m_pfn_kxe_base_pause_job;
	type_kxe_base_resume_job m_pfn_kxe_base_resume_job;	
	type_kxe_base_wait_job m_pfn_kxe_base_wait_job;
	type_kxe_base_wait_multi_job m_pfn_kxe_base_wait_multi_job;
	type_kxe_base_get_job_exit_code m_pfn_kxe_base_get_job_exit_code;
	type_kxe_base_get_job_status m_pfn_kxe_base_get_job_status;
	type_kxe_base_close_job m_pfn_kxe_base_close_job;
	type_kxe_base_create_debug_log m_pfn_kxe_base_create_debug_log;
	type_kxe_base_debug_log m_pfn_kxe_base_debug_log;
	type_kxe_base_close_debug_log m_pfn_kxe_base_close_debug_log;
	type_kxe_base_load_plugin_path m_pfn_kxe_base_load_plugin_path;
	type_kxe_base_unload_plugin_path m_pfn_kxe_base_unload_plugin_path;
	type_kxe_base_system_query_active_user_token m_pfn_kxe_base_system_query_active_user_token;
	type_kxe_base_uncompress_file m_pfn_kxe_base_uncompress_file;
	type_kxe_base_load_plugin m_pfn_kxe_base_load_plugin;
	type_kxe_base_unload_plugin m_pfn_kxe_base_unload_plugin;
};

typedef SingletonHolder<KxEBaseDynamicLoader> KxEBaseDynamicLoaderSingleton;
