//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KTimer.h
//  Creator     :   YangXD(yxdh)
//  Date        :   2008-8-25 17:30:50
//  Comment     :   Interface for the KTimer class.
//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _K_TIMER_H_
#define _K_TIMER_H_
#define _WIN32_WINNT	0x0501
#include <windows.h>
#include <time.h>
#include "ITimerCallBack.h"
 

typedef enum _ALARM_TYPE
{
	enumOnce = 0,
	enumInterval,
	enumDaily,
	enumWeekly,
	enumMonthly
} ALARM_TYPE;


typedef struct _ALARM_
{
	ALARM_TYPE		Type;
	tm				Time;
} ALARM;


class KTimer
{
public:
	KTimer();
	~KTimer();

	HRESULT								SetAlarm(const ALARM& Alarm, ITimerCallBack* pCallBack);
	HRESULT								KillAlarm();

protected:
	HRESULT								InitData();

	HRESULT								Activate();
	HRESULT								DeActivate();

	static unsigned __stdcall 			Timing(void* lParam);
	static VOID CALLBACK				IntervalTimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired);

	HRESULT								AlarmTrigger();
	HRESULT								CalcAlarmTime();

private:
	HANDLE								m_pThread;
	unsigned							m_uThreadID;
	HANDLE								m_hStopEvent;

	ITimerCallBack*						m_pCallBack;
	
	__time64_t							m_AlarmTime;
	ALARM								m_Alarm;
};


#endif	// #ifndef _K_TIMER_H_