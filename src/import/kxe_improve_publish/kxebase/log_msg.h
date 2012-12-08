#pragma once
#include <stdio.h>
#include <iostream>
#include <cassert>
#include <vector>
#include "kxebase_log.h"
#include "../kxelog/log_priority.h"
#include "../scom/scom/singleton.h"

/**
 * @defgroup kxengine_debug_log_interface KXEngine Architecture Debug Log Interface
 * @{
 */

class KxEDebugLogMsg
{
public:
	KxEDebugLogMsg()
	{
		m_hLog = NULL;
	}

	~KxEDebugLogMsg()
	{
		assert(m_hLog == NULL);
	}

	int Log(
		KXE_LOG_LEVEL level, 
		const char* pszFormat, 
		...
		)
	{
		va_list args;
		va_start(args, pszFormat);

		int nRet = LogV(
			level,
			pszFormat,
			args
			);

		va_end(args);
		return nRet;
	}

	int LogV(
		KXE_LOG_LEVEL level, 
		const char* pszFormat, 
		va_list args
		)
	{
		/*获取日志信息并写入日志中*/
		char szBuffer[1024] = {0};
#if _MSC_VER == 1400
		int nCount = vsnprintf_s(
			szBuffer,
			1024, 
			_TRUNCATE,
			pszFormat, 
			args
			);
#else
		int nCount = _vsnprintf(
			szBuffer,
			1024,
			pszFormat,
			args
			);
#endif

		int nRet = 0;
		if (nCount == -1 || nCount == 1024)  // buffer不够大
		{
#if _MSC_VER == 1400
			nCount = _vscprintf(
				pszFormat,
				args
				);
			std::vector<char> vecBuffer(nCount + 1);

			vsnprintf_s(
				&vecBuffer[0],
				vecBuffer.size(),
				_TRUNCATE,
				pszFormat,
				args
				);
#else
			nCount = vprintf(
				pszFormat,
				args
				);
			std::vector<char> vecBuffer(nCount + 1);

			_vsnprintf(
				&vecBuffer[0],
				vecBuffer.size(),
				pszFormat,
				args
				);
#endif
			nRet = kxe_base_debug_log(
				m_hLog, 
				level, 
				&vecBuffer[0]
			);
		}
		else
		{
			nRet = kxe_base_debug_log(
				m_hLog, 
				level, 
				szBuffer
				);
		}

		return nRet;
	}

public:
	int CreateLogInstance(const char* pszName)
	{
		return kxe_base_create_debug_log(
			pszName,
			&m_hLog
			);

	}

	int CloseLogInstance()
	{
		int nRet = kxe_base_close_debug_log(m_hLog);
		m_hLog = NULL;

		return nRet;
	}
private:
	kxe_log_t m_hLog;
};

typedef SingletonHolder<KxEDebugLogMsg> KxEDebugLogMsgSinglton;

#ifdef NO_DEBUG_LOG

#define KXE_INIT_DEBUG_LOG(name)

#define KXE_DEBUG_LOG(X)

#define KXE_DEBUG_LOG_V(X)

#define KXE_FINI_DEBUG_LOG()

#else

/**
 * @brief 初始创建以name名字的调试日志管理器
 * @param[in] name 创建调试日志的名字,方便查阅,一般使用子系统名字
 */
#define KXE_INIT_DEBUG_LOG(name) \
	do\
{\
	KxEDebugLogMsgSinglton::Instance().CreateLogInstance(name);\
	KxEDebugLogMsgSinglton::Instance().Log(LOG_INFO, "To start debugging.");\
} while (0)

/**
 * @brief 调试日志输出宏
 * @param[in] X 输出的日志内容,分别为KXE_LOG_LEVEL, format string, arg
 * @remark 调用方式为KXE_DEBUG_LOG((LOG_INFO, "just for test %s", "test"));
 */
#define KXE_DEBUG_LOG(X) \
	do\
{\
	KxEDebugLogMsgSinglton::Instance().Log X;\
} while (0)

/**
 * @brief 调试日志输出宏
 * @param[in] X 输出的日志内容,分别为KXE_LOG_LEVEL, format string, va_list
 * @remark 调用方式为KXE_DEBUG_LOG((LOG_INFO, "just for test %s", va_list));
 */
#define KXE_DEBUG_LOG_V(X) \
	do\
{\
	KxEDebugLogMsgSinglton::Instance().LogV X;\
} while (0)

/**
 * @brief 关闭日志输出
 */
#define KXE_FINI_DEBUG_LOG() \
	do\
{\
	KxEDebugLogMsgSinglton::Instance().Log(LOG_INFO, "To end debugging.");\
	KxEDebugLogMsgSinglton::Instance().CloseLogInstance();\
} while (0);


#endif

class KxEFunctionTraceDebugLog
{
public:
	KxEFunctionTraceDebugLog(
		const char* pszFunctionName,
		KXE_LOG_LEVEL logLevel,
		const char* pszFileName,
		int nLine
		) : m_pszFunctionName(pszFunctionName), m_level(logLevel)
	{
		KXE_DEBUG_LOG((
			logLevel,
			"Enter Function %s at file %s in line %d", 
			pszFunctionName,
			pszFileName,
			nLine
			));
	}

	~KxEFunctionTraceDebugLog()
	{
		KXE_DEBUG_LOG((
			m_level,
			"Leave Function %s", 
			m_pszFunctionName
			));
	}

private:
	const char* m_pszFunctionName;
	KXE_LOG_LEVEL m_level;
};

/// 函数调用追踪宏
#define KXE_FUNCTION_TRACE() \
	KxEFunctionTraceDebugLog __dummyLog__(__FUNCTION__, LOG_INFO, __FILE__, __LINE__);

/**
 * @}
 */
