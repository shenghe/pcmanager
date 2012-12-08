////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : multthread.h
//      Version   : 1.0
//      Comment   : 多线程工具
//      
//      Create at : 2008-7-29
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////
#include <vector>
#include "framework/kis_lock.h"
#include "loki2/Functor.h"

#pragma once 

class KWaitObject
{
public:
	KWaitObject()	{}
	virtual ~KWaitObject() {}
	virtual HANDLE	GetWaitHandle() = 0;
	virtual HRESULT	OnWaitEvent() = 0;
	virtual HRESULT	OnWaitError() = 0;
};

typedef Loki::Functor<HRESULT> EventHander;

//////////////////////////////////////////////////////////////////////////
// event处理对象
class KEventWaitObject: public KWaitObject
{
private:
	HANDLE		m_hEvent;
	EventHander	m_EventHander;

public:
	KEventWaitObject(): m_hEvent(NULL) {}

	virtual ~KEventWaitObject() { Clear(); }

	BOOL	Create(LPSECURITY_ATTRIBUTES pAttr, BOOL bManualSet, BOOL bInitState, LPCTSTR strName)
	{
		ASSERT(!m_hEvent);
		m_hEvent = ::CreateEvent(pAttr, bManualSet, bInitState, strName);
		return m_hEvent != NULL;
	}

	BOOL	Set() {
		return ::SetEvent(m_hEvent);
	}

	BOOL	Reset() {
		return ::ResetEvent(m_hEvent);
	}

	template <class PtrObj, typename MemFn>
	HRESULT	BindEvent(const PtrObj& p, MemFn memFn)
	{
		EventHander	hander(p, memFn);
		m_EventHander = hander;
		return S_OK;
	}

	virtual HANDLE	GetWaitHandle() {
		return m_hEvent;
	}

	virtual HRESULT	OnWaitEvent() {
		if (m_EventHander)
			return m_EventHander();
		return S_OK;
	}

	virtual HRESULT	OnWaitError() {
		return S_OK;
	}

private:
	BOOL	Clear()
	{
		if (m_hEvent)
		{
			::CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}
		return TRUE;
	}

};

//////////////////////////////////////////////////////////////////////////
// 信号量处理对象
class KSemaphoreWaitObject: public KWaitObject
{
private:
	HANDLE		m_hSemphore;
	EventHander	m_EventHander;

public:
	KSemaphoreWaitObject(): m_hSemphore(NULL) {}

	virtual ~KSemaphoreWaitObject() { Clear(); }

	BOOL	Create(LPSECURITY_ATTRIBUTES pAttr, LONG lInitCnt, LONG lMaxCnt, LPCTSTR strName)
	{
		ASSERT(!m_hSemphore);
		m_hSemphore = ::CreateSemaphore(pAttr, lInitCnt, lMaxCnt, strName);
		return m_hSemphore != NULL;
	}

	BOOL	Release(LONG nCnt, LPLONG lpPrevCnt)
	{
		if (m_hSemphore)
			return ::ReleaseSemaphore(m_hSemphore, nCnt, lpPrevCnt);
		return FALSE;
	}

	template <class PtrObj, typename MemFn>
	HRESULT	BindEvent(const PtrObj& p, MemFn memFn)
	{
		EventHander	hander(p, memFn);
		m_EventHander = hander;
		return S_OK;
	}

	virtual HANDLE	GetWaitHandle() {
		return m_hSemphore;
	}

	virtual HRESULT	OnWaitEvent() {
		m_EventHander();
		return S_OK;
	}

	virtual HRESULT	OnWaitError() {
		return S_OK;
	}

private:
	BOOL	Clear()
	{
		if (m_hSemphore)
		{
			::CloseHandle(m_hSemphore);
			m_hSemphore = NULL;
		}
		return TRUE;
	}
};
//////////////////////////////////////////////////////////////////////////
// 工作分发器, 使用等待特定条件的方式分发工作
class KWorkMgr
{
private:
	vector<KWaitObject*>	m_WorkList;
	HANDLE*					m_hHandleList;
	
	HANDLE					m_hModifyStart;
	HANDLE					m_hModifyEnd;

	DWORD					m_nTimeOut;
	EventHander				m_TimeOutEvent;

	kis::KLock				m_lock;

public:
	KWorkMgr():m_hHandleList(NULL), m_nTimeOut(INFINITE)
	{
		m_hModifyStart = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		m_hModifyEnd = ::CreateEvent(NULL, TRUE, TRUE, NULL);
	}

	~KWorkMgr() 
	{
		ASSERT(!m_hHandleList && "should call OnEndWork before delete me!");
		if (m_hHandleList)
			OnEndWork();

		if (m_hModifyStart)
		{
			::CloseHandle(m_hModifyStart);
			m_hModifyStart = NULL;
		}
		if (m_hModifyEnd)
		{
			::CloseHandle(m_hModifyEnd);
			m_hModifyEnd = NULL;
		}

		m_WorkList.clear();
		if (m_hHandleList)
		{
			delete [] m_hHandleList;
			m_hHandleList = NULL;
		}
	}

	HRESULT	SetTimeOutHander(DWORD nTimeOut, EventHander hander)
	{
		HRESULT hr = E_FAIL;
		::SetEvent(m_hModifyStart);
		m_lock.Lock();

		m_nTimeOut = nTimeOut;
		m_TimeOutEvent = hander;

		::ResetEvent(m_hModifyStart);
		m_lock.Unlock();
		::SetEvent(m_hModifyEnd);
		return hr;
	}

	HRESULT AddWaitObject(KWaitObject* pWaitObject)
	{
		HRESULT hr = E_FAIL;
		::SetEvent(m_hModifyStart);
		m_lock.Lock();

		hr = _AddWaitObject(pWaitObject);

		::ResetEvent(m_hModifyStart);
		m_lock.Unlock();
		::SetEvent(m_hModifyEnd);
		return hr;
	}

	HRESULT	RemoveObject(KWaitObject* pWaitObject)
	{
		HRESULT hr = E_FAIL;
		::SetEvent(m_hModifyStart);
		m_lock.Lock();
		
		hr = _DeleteObject(pWaitObject);

		::ResetEvent(m_hModifyStart);
		m_lock.Unlock();
		::SetEvent(m_hModifyEnd);
		return hr;
	}

	HRESULT	OnWork()
	{
		while (true)
		{
			::WaitForSingleObject(m_hModifyEnd, INFINITE);
			m_lock.Lock();

			if (!m_hHandleList)
			{
				m_lock.Unlock();
				break;
			}

			DWORD nWaitRes = ::WaitForMultipleObjects((DWORD)m_WorkList.size()+1, m_hHandleList, FALSE, m_nTimeOut);
			if (nWaitRes >= WAIT_OBJECT_0 && nWaitRes < (WAIT_OBJECT_0 + m_WorkList.size() + 1))
			{
				if (nWaitRes == WAIT_OBJECT_0)
				{
					::ResetEvent(m_hModifyEnd);
					m_lock.Unlock();
				}
				else
				{
					int nIndex = nWaitRes - WAIT_OBJECT_0 - 1;
					m_WorkList[nIndex]->OnWaitEvent();
					m_lock.Unlock();
				}
			}
			else if (nWaitRes >= WAIT_ABANDONED_0 && nWaitRes < (WAIT_ABANDONED_0 + m_WorkList.size() + 1))
			{
				if (nWaitRes == WAIT_ABANDONED_0)
				{
					ASSERT(!"should not be here!");
					m_lock.Unlock();
					break;
				}
				else
				{
					int nIndex = nWaitRes - WAIT_ABANDONED_0 - 1;
					KWaitObject* pObject = m_WorkList[nIndex];
					pObject->OnWaitError();
					_DeleteObject(pObject);
					m_lock.Unlock();
				}
			}
			else if (nWaitRes == WAIT_TIMEOUT)
			{
				if (m_TimeOutEvent)
					m_TimeOutEvent();
				m_lock.Unlock();
			}
			else	// 发生了其他错误
			{
				ASSERT(!"should not be here!");
				m_lock.Unlock();
				break;
			}
		}
		return S_OK;
	}

	HRESULT	OnEndWork()
	{
		::SetEvent(m_hModifyStart);
		m_lock.Lock();

		m_WorkList.clear();
		if (m_hHandleList)
		{
			delete [] m_hHandleList;
			m_hHandleList = NULL;
		}

		::ResetEvent(m_hModifyStart);
		m_lock.Unlock();
		::SetEvent(m_hModifyEnd);
		return S_OK;
	}

private:
	HRESULT	_AddWaitObject(KWaitObject* pWaitObject)
	{
		if (m_WorkList.size() < (MAXIMUM_WAIT_OBJECTS - 1))
		{
			m_WorkList.push_back(pWaitObject);
			CreateHandle();
			return S_OK;
		}
		return E_FAIL;
	}

	HRESULT	_DeleteObject(KWaitObject* pWaitObject)
	{
		BOOL bFind = FALSE;
		for (int i = 0; i < (int)m_WorkList.size(); i++)
		{
			if (pWaitObject == m_WorkList[i])
			{
				m_WorkList.erase(m_WorkList.begin()+i);
				bFind = TRUE;
				break;
			}
		}
		if (bFind)
		{
			CreateHandle();
			return S_OK;
		}
		return E_FAIL;
	}

	void	CreateHandle()
	{
		if (m_hHandleList)
		{
			delete [] m_hHandleList;
			m_hHandleList = NULL;
		}
		if (m_WorkList.size())
		{
			m_hHandleList = new HANDLE[m_WorkList.size() + 1];
			m_hHandleList[0] = m_hModifyStart;

			for (int i = 0; i < (int)m_WorkList.size(); i++)
			{
				m_hHandleList[i+1] = m_WorkList[i]->GetWaitHandle();
				ASSERT(m_hHandleList[i+1]);
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// 线程
template<class TBase>
class KThread: public KWaitObject
{
private:
	HANDLE			m_hThread;
	HANDLE			m_hExit;
	TBase*			m_pWorkMgr;

public:
	KThread(): m_hThread(NULL), m_pWorkMgr(NULL) 
	{
		m_hExit = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	virtual ~KThread()
	{
		ASSERT(!m_hThread);
		if (m_hExit)
		{
			::CloseHandle(m_hExit);
			m_hExit = NULL;
		}
	}

	DWORD	StartThread(TBase*	pWorkMgr)
	{
		ASSERT(!m_hThread);
		ASSERT(m_hExit);
		ASSERT(pWorkMgr);

		if (!pWorkMgr || !m_hExit)
			return -1;

		m_pWorkMgr = pWorkMgr;
		if (!m_hThread)
		{
			m_pWorkMgr->AddWaitObject(static_cast<KWaitObject*>(this));
			m_hThread = ::CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
		}
		return 0;
	}

	DWORD	EndThread()
	{
		if (m_hExit)
		{
			::SetEvent(m_hExit);
			if (m_hThread)
			{
				if (::WaitForSingleObject(m_hThread, 2000) == WAIT_TIMEOUT)
				{
					ASSERT(!"wait for thread end tiem out");
					::TerminateThread(m_hThread, 0);
				}
				::CloseHandle(m_hThread);
				m_hThread = NULL;
			}
		}
		return 0;
	}

	HANDLE	GetWaitHandle() {
		return m_hExit;
	}

	HRESULT	OnWaitEvent() {
		m_pWorkMgr->OnEndWork();
		return S_OK;
	}

	HRESULT	OnWaitError() {
		m_pWorkMgr->OnEndWork();
		return S_OK;
	}

private:
	DWORD Run()
	{
		m_pWorkMgr->OnWork();
		return 0;
	}

	static DWORD WINAPI ThreadProc(	LPVOID lpParameter)
	{
		try
		{
			KThread<TBase>* pThis = (KThread<TBase>*)lpParameter;
			return pThis->Run();
		}
		catch (...)
		{
			ASSERT(!"线程异常");
		}
		return 0;
	}
};

//////////////////////////////////////////////////////////////////////////
// Timer
class KTimer
{
private:
	KWorkMgr					m_WorkMgr;
	KThread<KWorkMgr>			m_Thread;
	DWORD						m_nTimeOut;

public:
	KTimer(DWORD nTimeOut): m_nTimeOut(nTimeOut)
	{
	}

	void		SetTimeOut(DWORD nTimeOut)
	{
		m_nTimeOut = nTimeOut;
	}

	void		SetHander(EventHander hander)
	{
		m_WorkMgr.SetTimeOutHander(m_nTimeOut, hander);
	}
	void		Start()
	{
		m_Thread.StartThread(&m_WorkMgr);
	}

	void		Stop()
	{
		EventHander hander;
		m_WorkMgr.SetTimeOutHander(0, hander);
		m_Thread.EndThread();
	}
};