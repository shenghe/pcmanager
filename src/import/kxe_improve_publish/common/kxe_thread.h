//////////////////////////////////////////////////////////////////////
///		@file		kxe_thread.h
///		@author		luopeng
///		@date		2008-8-22 10:38:34
///
///		@brief		线程相关的定义
//////////////////////////////////////////////////////////////////////

#pragma once

/**
 * @defgroup kxebase_src_thread_group KXEngine Architecture Thread Assistant Interface
 * @{
 */

/// 定义自动获取及释放同步对像的Guard模板
template
<
	typename MutexType
>
class KxEMutexGuard
{
public:
	/**
	 * @brief 通过调用模板参数的Acquire自动获取所有权
	 */
	KxEMutexGuard(MutexType& mutex) : m_mutex(mutex)
	{
		m_mutex.Acquire();
	}

	/**
	* @brief 通过调用模板参数的Release自动在退出函数时释放所有权
	*/
	~KxEMutexGuard()
	{
		m_mutex.Release();
	}

private:
	MutexType& m_mutex;
};

/**
 * @brief 定义线程同步对像,为CRITICAL_SECTION的封装
 */
class KxEThreadMutex
{
public:
	KxEThreadMutex()
	{
		InitializeCriticalSection(&m_cs);
	}
	~KxEThreadMutex()
	{
		DeleteCriticalSection(&m_cs);
	}

	/**
	 * @brief 获取线程同步对像所有权,若没获取到,将阻塞
	 */
	void Acquire()
	{
		EnterCriticalSection(&m_cs);
	}

	/**
	* @brief 释放线程同步对像所有权
	*/
	void Release()
	{
		LeaveCriticalSection(&m_cs);
	}

	/**
	 * @brief 尝试获取线程同步对像所有权,若没获取到,将返回false
	 * @return TRUE 成功获取
	 * @return FALSE 获取失败
	 */
#if _WIN32_WINNT >= 0x400
	bool TryAcquire()
	{
		return TryEnterCriticalSection(&m_cs) == TRUE;
	}
#endif

private:
	CRITICAL_SECTION m_cs;
};

/**
 * @brief 定义用于自动获取及释放线程同步对像KxEThreadMutex的类型
 */
typedef KxEMutexGuard<KxEThreadMutex> KxEThreadMutexGuard;

/// 定义多线程安全的队列
template
<
	typename T
>
class KxEThreadQueue
{
public:
	KxEThreadQueue()
	{
		m_uMaxThread = 0;
		m_uWaitThreadCount = 0;
		m_uQueueLength = 0;
		m_hCompletionPort = NULL;
	}

	int Open(unsigned int uMaxThread)
	{
		KxEThreadMutexGuard guard(m_mutex);
		if (m_hCompletionPort != NULL)
		{
			return -1;
		}

		m_uMaxThread = uMaxThread;
		m_hCompletionPort = CreateIoCompletionPort(
			INVALID_HANDLE_VALUE,
			NULL,
			0,
			uMaxThread
			);
		if (m_hCompletionPort == NULL)
		{
			return GetLastError();
		}

		return 0;
	}

	int Close()
	{
		KxEThreadMutexGuard guard(m_mutex);
		if (m_hCompletionPort == NULL)
		{
			return -1;
		}

		// 发给所有
		for (unsigned int i = 0; i < m_uWaitThreadCount; ++i)
		{
			PostQueuedCompletionStatus(
				m_hCompletionPort,
				0,
				0xffffffff,
				NULL
				);
		}

		CloseHandle(m_hCompletionPort);
		m_hCompletionPort = NULL;

		return 0;
	}

	int Enqueue(T* pValue)
	{
		KxEThreadMutexGuard guard(m_mutex);
		if (m_hCompletionPort == NULL)
		{
			return -1;
		}

		BOOL bRet = PostQueuedCompletionStatus(
			m_hCompletionPort,
			0,
			0,
			reinterpret_cast<LPOVERLAPPED>(pValue)
			);
		if (!bRet)
		{
			return GetLastError();
		}

		return 0;
	}

	int Dequeue(
		T** ppValue, 
		unsigned int uMillisecond
		)
	{
		{
			KxEThreadMutexGuard guard(m_mutex);
			if (m_hCompletionPort == NULL)
			{
				return -1;
			}

			++m_uWaitThreadCount;
		}

		DWORD dwSize = 0;
		DWORD dwKey = 0;
		BOOL bRet = GetQueuedCompletionStatus(
			m_hCompletionPort,
			&dwSize,
			&dwKey,
			reinterpret_cast<LPOVERLAPPED*>(ppValue),
			uMillisecond
			);

		{
			KxEThreadMutexGuard guard(m_mutex);
			if (m_hCompletionPort == NULL)
			{
				return -1;
			}

			--m_uWaitThreadCount;
		}

		if (!bRet)
		{
			return GetLastError();
		}

		if (dwKey == 0xFFFFFFFF) // 标志结束
		{
			return -1;
		}
		else  // 取出了正确数据
		{
			return 0;
		}
	}

private:
	KxEThreadMutex m_mutex;
	unsigned int m_uWaitThreadCount;
	unsigned int m_uMaxThread;
	unsigned int m_uQueueLength;
	HANDLE m_hCompletionPort;
};


/**
 * @}
 */
