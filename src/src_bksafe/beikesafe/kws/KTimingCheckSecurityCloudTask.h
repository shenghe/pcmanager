//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KTimingCheckSecurityCloudTask.h
//  Version     :   1.0
//  Creater     :   YangXD(yxdh)
//  Date        :   2006-08-24 14:53:05
//  Comment     :   
//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _K_TIMING_CHECK_SECURITY_CLOUD_TASK_
#define _K_TIMING_CHECK_SECURITY_CLOUD_TASK_
#include "common/ITimerCallBack.h"
#include "common/KTimer.h"
#include <string>
#include "scan/KLogic.h"
#define	MSG_SIGNAL			(WM_APP+ 401)


class KTimingCheckSecurityCloudTask : public ITimerCallBack
{
public:
	KTimingCheckSecurityCloudTask();
	~KTimingCheckSecurityCloudTask();

	HRESULT						Startup(HWND hWnd, KLogic* pLogic);
	bool						IsSignal();

	bool						ReTestSignal();

	//  µœ÷ITimerCallBack
	virtual int					OnTimer();

private:
	KTimer						m_Timer;
	HWND						m_hWnd;
	KLogic*						m_pLogic;
	bool						m_bSignal;
	HMODULE						m_hSensApi;
};


#endif		// #ifndef _K_TIMING_CHECK_SECURITY_CLOUD_TASK_