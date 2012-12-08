#include "stdafx.h"
#include "KTimer.h"
#include "kws/KAVPublic.h"
#include <process.h>


#define	EVENT_COUNT						(2)
#define	WAIT_THREAD_TIMEOUT				(5000LL)
#define	WAIT_TIMEOUT_MIN_MILLISECONDS	(200LL)
#define	WAIT_TIMEOUT_MAX_MILLISECONDS	(1000LL)

#define ONE_SECOND						(1000LL)
#define ONE_MINUTE						(ONE_SECOND	* 60)
#define ONE_HOUR						(ONE_SECOND	* 3600)
#define ONE_DAY							(ONE_HOUR	* 24)


KTimer::KTimer()
{
	m_uThreadID			= 0L;
	m_pThread			= NULL;
	m_hStopEvent		= NULL;
	m_pCallBack			= NULL;
}

KTimer::~KTimer()
{
	KillAlarm();
}

HRESULT KTimer::SetAlarm(const ALARM& Alarm, ITimerCallBack* pCallBack)
{
	HRESULT lResult		= E_FAIL;
	HRESULT lRetCode	= E_FAIL;


	lRetCode = InitData();
	KAV_COM_PROCESS_ERROR(lRetCode);	


	m_pCallBack = pCallBack;
	m_Alarm = Alarm;

	KAV_COM_PROCESS_ERROR(CalcAlarmTime());


	lRetCode = Activate();
	KAV_COM_PROCESS_ERROR(lRetCode);


	lResult = S_OK;

Exit0:

	if (FAILED(lResult))
	{
		KillAlarm();
	}

	return lResult;
}

HRESULT KTimer::KillAlarm()
{
	DeActivate();

	if (NULL != m_hStopEvent)
	{
		::CloseHandle(m_hStopEvent);
		m_hStopEvent = NULL;
	}

	return S_OK;
}

HRESULT KTimer::InitData()
{
	HRESULT lResult		= E_FAIL;
	HRESULT	lRetCode	= E_FAIL;


	KillAlarm();

	m_pThread			= NULL;
	m_pCallBack			= NULL;

	m_AlarmTime			= 0LL;

	::memset(&m_Alarm, 0, sizeof(m_Alarm));

	
	if (NULL != m_hStopEvent)
	{
		::CloseHandle(m_hStopEvent);
		m_hStopEvent = NULL;
	}

	m_hStopEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	KAV_PROCESS_ERROR(m_hStopEvent);


	lResult = S_OK;

Exit0:

	return lResult;
}

// 使定时器开始计时，计时方法是用一个新线程
HRESULT KTimer::Activate()
{
	HRESULT lResult	= E_FAIL;


	KAV_PROCESS_ERROR(NULL == m_pThread);
	m_pThread = (HANDLE)::_beginthreadex(NULL, 0, Timing, this, 0, &m_uThreadID);
	KAV_PROCESS_ERROR(m_pThread);


	lResult = S_OK;

Exit0:

	return lResult;
}

HRESULT KTimer::DeActivate()
{
	if (m_hStopEvent)
	{
		::SetEvent(m_hStopEvent);
	}


	while (m_pThread)
	{
		DWORD dwWaitCode = ::WaitForSingleObject(m_pThread, WAIT_THREAD_TIMEOUT);
		if (dwWaitCode == WAIT_TIMEOUT)
		{
			continue;
		}

		CloseHandle(m_pThread);
		m_pThread = NULL;
	}

	return S_OK;
}

unsigned __stdcall KTimer::Timing(void* lParam)
{
	HRESULT	lRetCode		= E_FAIL;
	DWORD	dwEvent			= 0L;
	HANDLE	hIntervalEvent	= NULL;
	HANDLE	hTimerQueue		= NULL;
	HANDLE	hEvents[EVENT_COUNT]; 


	hIntervalEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	KAV_PROCESS_ERROR(hIntervalEvent);


	KTimer* pThis			= (KTimer*)lParam;

	hEvents[0]				= pThis->m_hStopEvent;
	hEvents[1]				= hIntervalEvent;


	DWORD					dwStart		= GetTickCount();
	DWORD					dwEnd		= 0L; 
	DWORD					dwElapsed	= 0L; 
	__time64_t				tStart;
	__time64_t				tNow;
	tm						tmNow;
	UINT					uWaitTime	= WAIT_TIMEOUT_MAX_MILLISECONDS;

	_time64(&tStart);

	hTimerQueue = CreateTimerQueue();
	KAV_PROCESS_ERROR(hTimerQueue);

	{
		HANDLE hTimer = NULL;
		lRetCode = CreateTimerQueueTimer(
			&hTimer, 
			hTimerQueue, 
			(WAITORTIMERCALLBACK)IntervalTimerRoutine,
			hIntervalEvent, 
			uWaitTime, 
			1, 
			0);
		KAV_PROCESS_ERROR(lRetCode);
	}

	while (true)
	{

		if (pThis->m_Alarm.Type == enumInterval)
		{
			dwEnd = GetTickCount();
			if (dwEnd < dwStart)
			{
				dwElapsed = (DWORD) - 1 - dwStart + dwEnd;
			}
			else
				dwElapsed = dwEnd - dwStart;
			
			if (dwElapsed >= pThis->m_AlarmTime)
			{

				KAV_COM_PROCESS_ERROR(pThis->AlarmTrigger());
				dwStart = GetTickCount();
				continue;
			}

			if (pThis->m_AlarmTime <= dwElapsed + ONE_SECOND)
			{
				uWaitTime = WAIT_TIMEOUT_MIN_MILLISECONDS;
			}
		}
		else
		{
			_time64(&tNow);
			if (tNow < tStart)
			{
				KAV_COM_PROCESS_ERROR(pThis->CalcAlarmTime());
				tStart = tNow;
			}

			if (tNow >= pThis->m_AlarmTime)
			{
				KAV_COM_PROCESS_ERROR(pThis->AlarmTrigger());
				KAV_COM_PROCESS_ERROR(pThis->CalcAlarmTime());
				continue;
			}


			KAV_PROCESS_ERROR(_localtime64_s(&tmNow, &tNow) == 0);

			tmNow.tm_sec += 1;
			tNow = _mktime64(&tmNow);
			KAV_PROCESS_ERROR(tNow != (__time64_t) - 1);

			if (pThis->m_AlarmTime <= tNow)
			{
				uWaitTime = WAIT_TIMEOUT_MIN_MILLISECONDS;
			}
		}


		dwEvent = WaitForMultipleObjects(
			EVENT_COUNT,
			hEvents,
			FALSE,		// wait for any
			INFINITE);
		switch (dwEvent) 
		{ 
		// stop event was signaled.
		case WAIT_OBJECT_0 + 0: 
			goto Exit0;	
		default: 
			break;
		}

		ResetEvent(hIntervalEvent);
	}


Exit0:

	if (hTimerQueue)
	{
		DeleteTimerQueueEx(hTimerQueue, INVALID_HANDLE_VALUE);
		hTimerQueue = NULL;
	}

	if (NULL != hIntervalEvent)
	{
		::CloseHandle(hIntervalEvent);
		hIntervalEvent = NULL;
	}

	return true;
}

VOID CALLBACK KTimer::IntervalTimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	if (lpParam)
	{
		SetEvent(lpParam);
	}
}

HRESULT KTimer::AlarmTrigger()
{
	HRESULT lResult = E_FAIL;


	if (m_pCallBack)
	{
		KAV_PROCESS_ERROR(m_pCallBack->OnTimer());
	}


	lResult = S_OK;

Exit0:
	return lResult;
}

HRESULT KTimer::CalcAlarmTime()
{
	HRESULT	lResult = E_FAIL;


	__time64_t tNow;
	_time64(&tNow);


	switch(m_Alarm.Type)
	{
	case enumOnce:
		{
			__time64_t tAlarm = _mktime64(&m_Alarm.Time);
			KAV_PROCESS_ERROR(tAlarm != (__time64_t) - 1);
			//KAV_PROCESS_ERROR(tNow < tAlarm);

			m_AlarmTime = tAlarm;
		}
		break;

	case enumInterval:
		{
			KAV_PROCESS_ERROR(m_Alarm.Time.tm_year == 0);
			KAV_PROCESS_ERROR(m_Alarm.Time.tm_mon == 0);
			KAV_PROCESS_ERROR(m_Alarm.Time.tm_yday == 0);
			KAV_PROCESS_ERROR(m_Alarm.Time.tm_mday >= 0 && m_Alarm.Time.tm_mday <= 7);
			KAV_PROCESS_ERROR(m_Alarm.Time.tm_hour >= 0 && m_Alarm.Time.tm_hour <= 24);
			KAV_PROCESS_ERROR(m_Alarm.Time.tm_min >= 0 && m_Alarm.Time.tm_min <= 60);
			KAV_PROCESS_ERROR(m_Alarm.Time.tm_sec >= 0 && m_Alarm.Time.tm_sec <= 60);


			m_AlarmTime = ONE_DAY * m_Alarm.Time.tm_mday 
				+ ONE_HOUR * m_Alarm.Time.tm_hour
				+ ONE_MINUTE * m_Alarm.Time.tm_min
				+ ONE_SECOND * m_Alarm.Time.tm_sec;
		}
		break;

	case enumDaily:
		{
			tm tmNow;
			KAV_PROCESS_ERROR(_localtime64_s(&tmNow, &tNow) == 0);

			m_Alarm.Time.tm_wday	= tmNow.tm_wday;
			m_Alarm.Time.tm_mday	= tmNow.tm_mday;
			m_Alarm.Time.tm_mon		= tmNow.tm_mon;
			m_Alarm.Time.tm_yday	= tmNow.tm_yday;
			m_Alarm.Time.tm_year	= tmNow.tm_year;

			__time64_t tAlarm = _mktime64(&m_Alarm.Time);
			KAV_PROCESS_ERROR(tAlarm != (__time64_t) - 1);

			if (tNow >= tAlarm)
			{
				m_Alarm.Time.tm_mday += 1;
				
				tAlarm = _mktime64(&m_Alarm.Time);
				KAV_PROCESS_ERROR(tAlarm != (__time64_t) - 1);
			}

			m_AlarmTime = tAlarm;
		}
	    break;

	case enumWeekly:
		{
			tm tmNow;
			KAV_PROCESS_ERROR(_localtime64_s(&tmNow, &tNow) == 0);

			/* int tm_mday;----day of the month - [1,31] */
			/* int tm_wday;----days since Sunday - [0,6] */
			m_Alarm.Time.tm_mday	= tmNow.tm_mday - tmNow.tm_wday + m_Alarm.Time.tm_wday;
			m_Alarm.Time.tm_mon		= tmNow.tm_mon;
			m_Alarm.Time.tm_year	= tmNow.tm_year;

			__time64_t tAlarm = _mktime64(&m_Alarm.Time);
			KAV_PROCESS_ERROR(tAlarm != (__time64_t) - 1);

			if (tNow >= tAlarm)
			{
				m_Alarm.Time.tm_mday += 7;

				tAlarm = _mktime64(&m_Alarm.Time);
				KAV_PROCESS_ERROR(tAlarm != (__time64_t) - 1);
			}

			m_AlarmTime = tAlarm;
		}
	    break;

	case enumMonthly:
		{
			tm tmNow;
			KAV_PROCESS_ERROR(_localtime64_s(&tmNow, &tNow) == 0);

			m_Alarm.Time.tm_mon		= tmNow.tm_mon;
			m_Alarm.Time.tm_year	= tmNow.tm_year;

			__time64_t tAlarm = _mktime64(&m_Alarm.Time);
			KAV_PROCESS_ERROR(tAlarm != (__time64_t) - 1);

			if (tNow >= tAlarm)
			{
				m_Alarm.Time.tm_mon += 1;

				tAlarm = _mktime64(&m_Alarm.Time);
				KAV_PROCESS_ERROR(tAlarm != (__time64_t) - 1);
			}

			m_AlarmTime = tAlarm;
		}
		break;

	default:
		goto Exit0;
	}
	

	lResult = S_OK;

Exit0:

	return lResult;
}


