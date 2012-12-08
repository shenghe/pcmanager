//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   KTimingCheckSecurityCloudTask.cpp
//  Version     :   1.0
//  Creater     :   YangXD(yxdh)
//  Date        :   2006-08-24 14:53:54
//  Comment     :   
//
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "KTimingCheckSecurityCloudTask.h"


KTimingCheckSecurityCloudTask::KTimingCheckSecurityCloudTask()
{
	m_hWnd		= NULL;
	m_pLogic	= NULL;
	m_bSignal	= false;
	m_hSensApi	= NULL;
}

KTimingCheckSecurityCloudTask::~KTimingCheckSecurityCloudTask()
{
	m_Timer.KillAlarm();

	if (m_hSensApi)
	{
		FreeLibrary(m_hSensApi);
		m_hSensApi = NULL;
	}
}

HRESULT KTimingCheckSecurityCloudTask::Startup(HWND hWnd, KLogic* pLogic)
{
	HRESULT lResult		= E_FAIL;
	HRESULT lRetCode	= E_FAIL;


	m_hWnd = hWnd;
	m_pLogic = pLogic;


	ALARM alarm;
	alarm.Type	= enumInterval;
	memset(&alarm.Time, 0, sizeof(alarm.Time));
	alarm.Time.tm_sec = 3;
	m_Timer.SetAlarm(alarm, this);


	lResult = S_OK;

Exit0:

	return lResult;
}

bool KTimingCheckSecurityCloudTask::ReTestSignal()
{
	bool bRet = m_bSignal;
	m_bSignal = FALSE;
	return bRet;
}

bool KTimingCheckSecurityCloudTask::IsSignal()
{
	return m_bSignal;
}


typedef BOOL APIENTRY IsNetworkAlive( LPDWORD lpdwFlags );
#define NETWORK_ALIVE_LAN   0x00000001
#define NETWORK_ALIVE_WAN   0x00000002
#define NETWORK_ALIVE_AOL   0x00000004

int KTimingCheckSecurityCloudTask::OnTimer()
{
	if (!::IsWindow(m_hWnd) || !::IsWindowVisible(m_hWnd))
		goto Exit0;

	{
		
		if ( m_pLogic )  // 以前的调用方式存在问题
		{		
			m_pLogic->SpTestAndStart();
		}

		BOOL bAlive	= TRUE;


		if (NULL == m_hSensApi)
		{
			m_hSensApi = LoadLibrary(_T("sensapi.dll"));
		}

		if (m_hSensApi)
		{
			IsNetworkAlive* IsNetworkAliveFn = (IsNetworkAlive*)GetProcAddress(m_hSensApi, "IsNetworkAlive");
			if (IsNetworkAliveFn)
			{
				DWORD dwFlags = 0L;
				bAlive = IsNetworkAliveFn(&dwFlags);
				if ( ERROR_SUCCESS != GetLastError() && !bAlive )
				{
					bAlive = TRUE;	// 判定联网状态失败，都假定网络联通
					FreeLibrary(m_hSensApi);
					m_hSensApi = NULL;
				}
			}
		}

		if (!m_bSignal && bAlive)
		{
			::SendMessage(m_hWnd, MSG_SIGNAL, m_bSignal, true);
			m_bSignal = m_pLogic->IsSecurityCloudValid();

		}
		else if (m_bSignal && !bAlive)
		{
			m_bSignal = false;
		}

		::SendMessage(m_hWnd, MSG_SIGNAL, m_bSignal, false);
	}

Exit0:

	return true;
}
