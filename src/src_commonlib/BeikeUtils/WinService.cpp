#include <windows.h>
#include "BeikeUtils.h"
#include "SmartHandle.h"
#include "WinService.h"

//#define DPRINT printf
#define DPRINT 
enum {
	em_OS_MajorVer_Vista    = 6,
	em_OS_MajorVer_Win2k3   = 5,
	em_OS_MajorVer_WinXP    = 5,

	em_OS_MinorVer_Win2k3   = 2,
	em_OS_MinorVer_WinXP    = 1,
};

/**
* @retval   >0  current OS is greater than compared version
* @retval   <0  current OS is less than compared version
* @retval   0   current OS is equal to compared version
*/
int CompareVersion(DWORD dwMajorVer, DWORD dwMinorVer)
{
	T_WindowsInfo wininfo;
	wininfo.Init();

	if (wininfo.osvi.dwMajorVersion > dwMajorVer)
	{
		return 1;
	}
	else if (wininfo.osvi.dwMajorVersion < dwMajorVer)
	{
		return -1;
	}
	return wininfo.osvi.dwMinorVersion - dwMinorVer;
}

BOOL IsVistaOrLater()
{
	return 0 <= CompareVersion(em_OS_MajorVer_Vista, 0);
}

namespace Win
{
	HRESULT IsServiceExists( LPCTSTR pszSrvName, BOOL &bExists )
	{
		CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if(m_schSCManager==NULL)
			return E_POINTER;
		
		bExists = FALSE;
		CAutoServiceHandle schService = OpenService( m_schSCManager, pszSrvName, SERVICE_ALL_ACCESS);
		if (NULL == schService)
		{
			return ERROR_SERVICE_DOES_NOT_EXIST==GetLastError() ? S_OK : E_FAIL;
		}
		bExists = TRUE;
		return S_OK;
	}
	
	HRESULT IsServiceEnabled( LPCTSTR szSvcName, BOOL &bEnabled )
	{
		CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if(m_schSCManager==NULL)
			return E_POINTER;

		LPQUERY_SERVICE_CONFIG lpqscBuf;
		DWORD dwBytesNeeded; 

		// Open a handle to the service. 
		CAutoServiceHandle schService = OpenService( m_schSCManager, szSvcName, SERVICE_QUERY_CONFIG);
		if (schService == NULL)
		{ 
			DPRINT("OpenService failed (%d)", GetLastError());
			return E_FAIL;
		}

		// Allocate a buffer for the configuration information.
		lpqscBuf = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LPTR, 4096); 
		if (lpqscBuf == NULL) 
		{
			return E_FAIL;
		}
		// Get the configuration information. 
		if (! QueryServiceConfig( schService, lpqscBuf, 4096, &dwBytesNeeded) ) 
		{
			DPRINT("QueryServiceConfig failed (%d)", GetLastError());
		}

		bEnabled = SERVICE_DISABLED!=lpqscBuf->dwStartType;
		LocalFree(lpqscBuf);
		return S_OK;
	}
	
	HRESULT GetServiceStatus( LPCTSTR pszSrvName, SERVICE_STATUS &status )
	{
		CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if(m_schSCManager==NULL)
			return E_POINTER;

		CAutoServiceHandle schService = OpenService( m_schSCManager, pszSrvName, SERVICE_ALL_ACCESS);
		if (NULL == schService)
			return E_FAIL;
		BOOL bRet = QueryServiceStatus(schService, &status);
		return bRet ? S_OK : E_FAIL;
	}
	
	BOOL DumpServiceInfo( LPCTSTR szSvcName )
	{
		CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if(m_schSCManager==NULL)
			return E_POINTER;

		CAutoServiceHandle schService=NULL;
		LPQUERY_SERVICE_CONFIG lpqscBuf; 
		LPSERVICE_DESCRIPTION lpqscBuf2;
		DWORD dwBytesNeeded; 
		BOOL bSuccess=TRUE;

		// Open a handle to the service. 

		schService = OpenService( 
			m_schSCManager,         // SCManager database 
			szSvcName,				// name of service 
			SERVICE_QUERY_CONFIG);  // need QUERY access 
		if (schService == NULL)
		{ 
			DPRINT("OpenService failed (%d)", GetLastError());
			return FALSE;
		}

		// Allocate a buffer for the configuration information.

		lpqscBuf = (LPQUERY_SERVICE_CONFIG) LocalAlloc( 
			LPTR, 4096); 
		if (lpqscBuf == NULL) 
		{
			return FALSE;
		}

		lpqscBuf2 = (LPSERVICE_DESCRIPTION) LocalAlloc( 
			LPTR, 4096); 
		if (lpqscBuf2 == NULL) 
		{
			return FALSE;
		}

		// Get the configuration information. 

		if (! QueryServiceConfig( 
			schService, 
			lpqscBuf, 
			4096, 
			&dwBytesNeeded) ) 
		{
			DPRINT("QueryServiceConfig failed (%d)", GetLastError());
			bSuccess = FALSE; 
		}

		if (! QueryServiceConfig2( 
			schService, 
			SERVICE_CONFIG_DESCRIPTION,
			(LPBYTE)lpqscBuf2, 
			4096, 
			&dwBytesNeeded) ) 
		{
			DPRINT("QueryServiceConfig2 failed (%d)", GetLastError());
			bSuccess = FALSE;
		}

		// Print the configuration information.

		DPRINT("\nSample_Srv configuration: \n");
		DPRINT(" Type: 0x%x\n", lpqscBuf->dwServiceType);
		DPRINT(" Start Type: 0x%x\n", lpqscBuf->dwStartType);
		DPRINT(" Error Control: 0x%x\n", lpqscBuf->dwErrorControl);
		DPRINT(" Binary path: %s\n", lpqscBuf->lpBinaryPathName);

		if (lpqscBuf->lpLoadOrderGroup != NULL)
			DPRINT(" Load order group: %s\n", lpqscBuf->lpLoadOrderGroup);
		if (lpqscBuf->dwTagId != 0)
			DPRINT(" Tag ID: %d\n", lpqscBuf->dwTagId);
		if (lpqscBuf->lpDependencies != NULL)
			DPRINT(" Dependencies: %s\n", lpqscBuf->lpDependencies);
		if (lpqscBuf->lpServiceStartName != NULL)
			DPRINT(" Start Name: %s\n", lpqscBuf->lpServiceStartName);
		if (lpqscBuf2->lpDescription != NULL)
			DPRINT(" Description: %s\n", lpqscBuf2->lpDescription);

		LocalFree(lpqscBuf); 
		LocalFree(lpqscBuf2); 

		return bSuccess;
	}

	HRESULT StartSvc( LPCTSTR lpServiceName )
	{
		CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if(m_schSCManager==NULL)
			return E_POINTER;

		CAutoServiceHandle schService;
		schService = OpenService(m_schSCManager, lpServiceName, SERVICE_ALL_ACCESS);
		if (schService == NULL) {
			return E_FAIL;
		}
		BOOL bRet = StartService(schService,0,NULL);
		return bRet ? S_OK : E_FAIL;
	}
	
	HRESULT StopSvc( LPCTSTR lpServiceName )
	{
		CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if(m_schSCManager==NULL)
			return E_POINTER;

		CAutoServiceHandle schService;
		schService = OpenService(m_schSCManager, lpServiceName, SERVICE_ALL_ACCESS);
		if (schService == NULL) {
			return E_FAIL;
		}

		SERVICE_STATUS ss = {0};
		BOOL bRet = ControlService(schService,SERVICE_CONTROL_STOP,&ss);
		return bRet ? S_OK : E_FAIL;
	}
	
	HRESULT EnableService( LPCTSTR szSvcName, BOOL fDisable )
	{
		CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if(m_schSCManager==NULL)
			return E_POINTER;

		CAutoServiceHandle schService=NULL;
		SC_LOCK sclLock; 
		LPQUERY_SERVICE_LOCK_STATUS lpqslsBuf; 
		DWORD dwBytesNeeded, dwStartType; 
		BOOL bSuccess=TRUE;

		// Need to acquire database lock before reconfiguring. 
		sclLock = LockServiceDatabase(m_schSCManager); 

		// If the database cannot be locked, report the details. 
		if (sclLock == NULL) 
		{ 
			// Exit if the database is not locked by another process. 

			if (GetLastError() != ERROR_SERVICE_DATABASE_LOCKED) 
			{
				DPRINT("Database lock failed (%d)\n", GetLastError()); 
				return E_FAIL;
			}

			// Allocate a buffer to get details about the lock. 
			lpqslsBuf = (LPQUERY_SERVICE_LOCK_STATUS) LocalAlloc( 
				LPTR, sizeof(QUERY_SERVICE_LOCK_STATUS)+256); 
			if (lpqslsBuf == NULL) 
			{
				DPRINT("LocalAlloc failed (%d)\n", GetLastError()); 
				return E_FAIL;
			}

			// Get and print the lock status information. 
			if (!QueryServiceLockStatus( 
				m_schSCManager, 
				lpqslsBuf, 
				sizeof(QUERY_SERVICE_LOCK_STATUS)+256, 
				&dwBytesNeeded) ) 
			{
				DPRINT("Query lock status failed (%d)", GetLastError()); 
				return E_FAIL;
			}

			if (lpqslsBuf->fIsLocked) 
				DPRINT("Locked by: %s, duration: %d seconds\n", 
				lpqslsBuf->lpLockOwner, 
				lpqslsBuf->dwLockDuration); 
			else 
				DPRINT("No longer locked\n"); 

			LocalFree(lpqslsBuf); 
		} 

		// The database is locked, so it is safe to make changes. 
		// Open a handle to the service. 
		schService = OpenService( 
			m_schSCManager,           // SCManager database 
			szSvcName,				// name of service 
			SERVICE_CHANGE_CONFIG); // need CHANGE access 
		if (schService == NULL) 
		{
			DPRINT("OpenService failed (%d)\n", GetLastError());
			return E_FAIL;
		}

		dwStartType = (fDisable) ? SERVICE_DISABLED : SERVICE_DEMAND_START; 

		// Make the changes. 

		if (! ChangeServiceConfig( 
			schService,        // handle of service 
			SERVICE_NO_CHANGE, // service type: no change 
			dwStartType,       // change service start type 
			SERVICE_NO_CHANGE, // error control: no change 
			NULL,              // binary path: no change 
			NULL,              // load order group: no change 
			NULL,              // tag ID: no change 
			NULL,              // dependencies: no change 
			NULL,              // account name: no change 
			NULL,              // password: no change 
			NULL) )            // display name: no change
		{
			DPRINT("ChangeServiceConfig failed (%d)\n", GetLastError()); 
			bSuccess = FALSE;
		}
		else 
			DPRINT("ChangeServiceConfig succeeded.\n"); 

		// Release the database lock. 
		UnlockServiceDatabase(sclLock); 
		return bSuccess ? S_OK : E_FAIL;
	}
	
	HRESULT UpdateSvcDesc( LPCTSTR lpServiceName, LPCTSTR szDesc )
	{
		CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if(m_schSCManager==NULL)
			return E_POINTER;

		CAutoServiceHandle schService;
		SERVICE_DESCRIPTION sd = {0};
		schService = OpenService( m_schSCManager, lpServiceName, SERVICE_CHANGE_CONFIG);
		if (schService == NULL)
		{ 
			DPRINT("OpenService failed (%d)\n", GetLastError()); 
			return E_FAIL;
		}

		sd.lpDescription = (LPTSTR)szDesc;
		BOOL bRet = ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &sd);

		if( bRet ) 
			DPRINT("Service description updated successfully.\n");
		else
			DPRINT("ChangeServiceConfig2 failed\n");
		return bRet ? S_OK : E_FAIL;
	}
	
	HRESULT InstallSvc( LPCTSTR lpServiceName, LPCTSTR lpDisplayName, DWORD dwServiceType, LPCTSTR lpBinaryPathName, LPCTSTR lpServiceStartName, LPCTSTR lpDependencies )
	{
		CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if(m_schSCManager==NULL)
			return E_POINTER;
		
		HRESULT hr = S_OK;
		CAutoServiceHandle hService = CreateService(m_schSCManager, lpServiceName, lpDisplayName,
			SERVICE_ALL_ACCESS,
			dwServiceType,	//SERVICE_WIN32_SHARE_PROCESS
			SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
			lpBinaryPathName, NULL, NULL, lpDependencies,
			lpServiceStartName, NULL);

		if (hService == NULL){
			hService = OpenService(m_schSCManager, lpServiceName, SERVICE_ALL_ACCESS);
			if (hService == NULL) {
				return E_FAIL;
			}
		}
		return S_OK;
	}

	HRESULT RemoveSvc( LPCTSTR lpServiceName )
	{
		CAutoServiceHandle m_schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if(m_schSCManager==NULL)
			return E_POINTER;

		CAutoServiceHandle hService = OpenService(m_schSCManager, lpServiceName, SERVICE_ALL_ACCESS);
		if (hService == NULL) {
			return E_FAIL;
		}
		BOOL bRet = DeleteService(hService);
		return bRet ? S_OK : E_FAIL;
	}
}
