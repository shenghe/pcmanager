#pragma once

BOOL IsVistaOrLater();
namespace Win
{
	HRESULT IsServiceExists(LPCTSTR pszSrvName, BOOL &bExists);
	HRESULT IsServiceEnabled(LPCTSTR szSvcName, BOOL &bEnabled);
	HRESULT GetServiceStatus(LPCTSTR pszSrvName, SERVICE_STATUS &status);
	BOOL DumpServiceInfo(LPCTSTR szSvcName);
	
	HRESULT StartSvc(LPCTSTR lpServiceName);	
	HRESULT StopSvc(LPCTSTR lpServiceName);
	HRESULT EnableService(LPCTSTR szSvcName, BOOL fDisable);
	HRESULT UpdateSvcDesc(LPCTSTR lpServiceName, LPCTSTR szDesc);
	HRESULT InstallSvc(LPCTSTR lpServiceName, LPCTSTR lpDisplayName, DWORD dwServiceType, LPCTSTR lpBinaryPathName, LPCTSTR lpServiceStartName, LPCTSTR lpDependencies);
	HRESULT RemoveSvc(LPCTSTR lpServiceName);
};
