//////////////////////////////////////////////////////////////////////
///		@file		kxeserviceutility.inl
///		@author		luopeng
///		@date		2008-12-18 09:07:58
///
///		@brief		将服务相关函数提出
//////////////////////////////////////////////////////////////////////

#pragma once
#include <WinSvc.h>
#include "kxeserviceutility.h"

inline int KxEInstallService(TCHAR* pPath, 
							 TCHAR* pInternalName, 
							 TCHAR* pszServiceName,
							 TCHAR* pDescriptiveName,
							 TCHAR* pSvcGroupOrder)
{  
	SC_HANDLE schSCManager = OpenSCManager(
		NULL, 
		NULL, 
		SC_MANAGER_CREATE_SERVICE
		); 
	if (schSCManager == NULL) 
	{
		return GetLastError();
	}

	SC_HANDLE schService = CreateService( 
		schSCManager,                    /* SCManager database      */ 
		pInternalName,					 /* name of service         */ 
		pszServiceName,                  /* service name to display */ 
		SERVICE_START | SERVICE_STOP | SERVICE_CHANGE_CONFIG,    /* desired access          */ 
		SERVICE_WIN32_OWN_PROCESS ,       /* service type            */ 
		SERVICE_AUTO_START,              /* start type              */ 
		SERVICE_ERROR_NORMAL,            /* error control type      */ 
		pPath,							 /* service's binary        */ 
		pSvcGroupOrder,                  /* no load ordering group  */ 
		NULL,                            /* no tag identifier       */ 
		NULL,                            /* no dependencies         */ 
		NULL,                            /* LocalSystem account     */ 
		NULL
		);                                  /* no password             */ 
	if (schService == NULL)
	{
		if (GetLastError() != ERROR_SERVICE_EXISTS) // 如果服务存在,还原设置
		{
			int nErr = GetLastError();
			CloseServiceHandle(schSCManager);
			return nErr;
		}

		schService = OpenService(
			schSCManager,
			pInternalName,
			SERVICE_CHANGE_CONFIG
			);
		if (schService == NULL)
		{
			int nErr = GetLastError();
			CloseServiceHandle(schSCManager);
			return nErr;
		}

		BOOL bRet = ChangeServiceConfig(
			schService,
			SERVICE_WIN32_OWN_PROCESS ,
			SERVICE_AUTO_START,
			SERVICE_ERROR_NORMAL,
			pPath,
			pSvcGroupOrder,
			NULL,
			NULL,
			NULL,
			NULL,
			pDescriptiveName
			);
		if (!bRet)  // 成功还原设置
		{
			int nErr = GetLastError();
			CloseServiceHandle(schSCManager);
			return nErr;
		}
	}

	SERVICE_DESCRIPTION sd = {0};
	sd.lpDescription = pDescriptiveName;
	ChangeServiceConfig2(
		schService,
		SERVICE_CONFIG_DESCRIPTION,
		&sd
		);

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return 0;
} 
inline int KxEInstallServiceInteractive(TCHAR* pPath, 
							 TCHAR* pInternalName, 
							 TCHAR* pszServiceName,
							 TCHAR* pDescriptiveName,
							 TCHAR* pSvcGroupOrder)
{  
	SC_HANDLE schSCManager = OpenSCManager(
		NULL, 
		NULL, 
		SC_MANAGER_CREATE_SERVICE
		); 
	if (schSCManager == NULL) 
	{
		return GetLastError();
	}

	SC_HANDLE schService = CreateService( 
		schSCManager,                    /* SCManager database      */ 
		pInternalName,					 /* name of service         */ 
		pszServiceName,                  /* service name to display */ 
		SERVICE_START | SERVICE_STOP | SERVICE_CHANGE_CONFIG,    /* desired access          */ 
		SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,       /* service type            */ 
		SERVICE_AUTO_START,              /* start type              */ 
		SERVICE_ERROR_NORMAL,            /* error control type      */ 
		pPath,							 /* service's binary        */ 
		pSvcGroupOrder,                  /* no load ordering group  */ 
		NULL,                            /* no tag identifier       */ 
		NULL,                            /* no dependencies         */ 
		NULL,                            /* LocalSystem account     */ 
		NULL
		);                                  /* no password             */ 
	if (schService == NULL)
	{
		if (GetLastError() != ERROR_SERVICE_EXISTS) // 如果服务存在,还原设置
		{
			int nErr = GetLastError();
			CloseServiceHandle(schSCManager);
			return nErr;
		}

		schService = OpenService(
			schSCManager,
			pInternalName,
			SERVICE_CHANGE_CONFIG
			);
		if (schService == NULL)
		{
			int nErr = GetLastError();
			CloseServiceHandle(schSCManager);
			return nErr;
		}

		BOOL bRet = ChangeServiceConfig(
			schService,
			SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
			SERVICE_AUTO_START,
			SERVICE_ERROR_NORMAL,
			pPath,
			pSvcGroupOrder,
			NULL,
			NULL,
			NULL,
			NULL,
			pDescriptiveName
			);
		if (!bRet)  // 成功还原设置
		{
			int nErr = GetLastError();
			CloseServiceHandle(schSCManager);
			return nErr;
		}
	}

	SERVICE_DESCRIPTION sd = {0};
	sd.lpDescription = pDescriptiveName;
	ChangeServiceConfig2(
		schService,
		SERVICE_CONFIG_DESCRIPTION,
		&sd
		);

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return 0;
} 

inline int KxEUninstallService(TCHAR *pInternalName)
{
	SC_HANDLE schSCManager = OpenSCManager(
		NULL, 
		NULL, 
		SC_MANAGER_CREATE_SERVICE
		); 
	if (schSCManager == NULL)
	{
		return GetLastError();
	}

	SC_HANDLE schService = OpenService(
		schSCManager,  
		pInternalName, 
		DELETE
		);     
	if (schService == NULL) 
	{
		int nErr = GetLastError();
		CloseServiceHandle(schSCManager);
		return nErr;
	}

	if (!DeleteService(schService)) 
	{
		int nErr = GetLastError();
		CloseServiceHandle(schSCManager);
		CloseServiceHandle(schService);
		return nErr;
	}

	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);

	return 0;
}

inline int KxEStartService(TCHAR *pInternalName,
						   bool bWaitRunning)
{
	SC_HANDLE schSCManager = OpenSCManager(
		NULL, 
		NULL, 
		GENERIC_WRITE
		); 
	if (schSCManager == NULL)
	{
		return GetLastError();
	}

	SC_HANDLE schService = OpenService(
		schSCManager,  
		pInternalName, 
		SERVICE_START | SERVICE_QUERY_STATUS
		);     
	if (schService == NULL) 
	{
		int nErr = GetLastError();
		CloseServiceHandle(schSCManager);
		return nErr;
	}

	SERVICE_STATUS status = {0};
	if (QueryServiceStatus(schService, &status) &&
		status.dwCurrentState == SERVICE_RUNNING)
	{
		return 0;
	}

	if (!StartService(schService, 0, NULL))
	{
		int nErr = GetLastError();
		CloseServiceHandle(schSCManager);
		CloseServiceHandle(schService);
		return nErr;
	}	

	int nRet = 0;
	if (bWaitRunning)
	{
		while (1)
		{
			BOOL bRet = QueryServiceStatus(schService, &status);
			if (!bRet)
			{
				nRet = GetLastError();
				break;
			}
			else
			{				
				if (status.dwCurrentState == SERVICE_RUNNING)
				{
					break;
				}
				else
				{
					Sleep(100);
				}
			}
		}
	}

	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);
	return nRet;
}

inline int KxEStopService(TCHAR *pInternalName,
						  bool bWaitUntilStopped)
{
	SC_HANDLE schSCManager = OpenSCManager(
		NULL, 
		NULL, 
		GENERIC_WRITE
		); 
	if (schSCManager == NULL)
	{
		return GetLastError();
	}

	SC_HANDLE schService = OpenService(
		schSCManager,  
		pInternalName, 
		SERVICE_STOP | SERVICE_QUERY_STATUS
		);     
	if (schService == NULL) 
	{
		int nErr = GetLastError();
		CloseServiceHandle(schSCManager);
		return nErr;
	}

	SERVICE_STATUS status = {0};
	if (!ControlService(schService, SERVICE_CONTROL_STOP, &status)) 
	{
		int nErr = GetLastError();
		CloseServiceHandle(schSCManager);
		CloseServiceHandle(schService);
		return nErr;
	}

	int nRet = 0;
	if (bWaitUntilStopped)
	{
		SERVICE_STATUS status = {0};
		while (1)
		{
			BOOL bRet = QueryServiceStatus(schService, &status);
			if (!bRet)
			{
				nRet = GetLastError();
				break;
			}
			else
			{				
				if (status.dwCurrentState == SERVICE_STOPPED)
				{
					break;
				}
				else
				{
					Sleep(100);
				}
			}
		}
	}

	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);

	return nRet;
}
