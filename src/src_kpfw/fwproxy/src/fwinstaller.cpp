#include "stdafx.h"
#include "fwinstaller.h"
#include <shlwapi.h>
#include <string>

#pragma comment (lib, "shlwapi.lib")

//////////////////////////////////////////////////////////////////////////

CFwInstaller::CFwInstaller()
    : m_hSCManager(NULL)
{
    RtlZeroMemory(m_wszModuleDir, sizeof(m_wszModuleDir));
}

CFwInstaller::~CFwInstaller()
{
	
}

//////////////////////////////////////////////////////////////////////////
 
STDMETHODIMP CFwInstaller::Install()
{	
    HRESULT hr = E_FAIL;
    int nRetCode;
	std::wstring strBinPath;
	HANDLE hService = NULL;
	WCHAR *wsDepend = NULL;
	DWORD dwStartType = SERVICE_SYSTEM_START;
	BOOL bInstallTdi = TRUE;
	WCHAR wsDriverDir[MAX_PATH] = {0};
	/*if (IsWow64())
	{
		printf("tdi driver unsupport wow64\n");
		return E_FAIL;
	}*/

	BOOL bVista = IsVistaPlatform();

    nRetCode = OpenSCManager();
    if (!nRetCode)
        goto clean0;

	if (!GetModuleDir())
		goto clean0;
	
	
	if( bVista )
	{
		do 
		{
			strBinPath = m_wszModuleDir;
			strBinPath += TDI_ALEFILTER_MODULE;
			if( !::PathFileExists( strBinPath.c_str() ) )
				break;
			
			//copy file
			if( !GetSystemDirectory( wsDriverDir, MAX_PATH ) )
				break;
			_tcscat_s( wsDriverDir, MAX_PATH, L"\\drivers\\"TDI_ALEFILTER_MODULE );
			if( !CopyFile( strBinPath.c_str(), wsDriverDir, FALSE) )
				break;

			dwStartType = SERVICE_BOOT_START;
			strBinPath = L"\\SystemRoot\\system32\\Drivers\\"TDI_ALEFILTER_MODULE;
			bInstallTdi = FALSE;
		} while (FALSE);
	}
	
	if( bInstallTdi )
	{
		if( bVista )
			wsDepend = L"tdx\0Tcpip\0";
		else
			wsDepend = L"Tcpip\0";

		strBinPath = m_wszModuleDir;
		if (IsWow64())
			strBinPath += TDI_FILTER_MODULE64;
		else
			strBinPath += TDI_FILTER_MODULE;
	}

	nRetCode = CreateService(
		TDI_FILTER_DRIVER,
		strBinPath.c_str(),
		L"PNP_TDI",
		wsDepend,
		hService,
		dwStartType
		);
	if(!nRetCode)
	{
		goto clean0;
	}
	
	if( bInstallTdi )
		ChangeTdiStartOrder();
	else
		ChangeAleStartOrder();
	
	hr = S_OK;

clean0:
	if (hService)
	{
		CloseService(hService);
		hService = NULL;
	}
    CloseSCManager();
    return hr;
}

STDMETHODIMP CFwInstaller::UnInstall()
{
    HRESULT hr = E_FAIL;
    int nRetCode;
	HANDLE  hService = NULL;
	BOOL fExist = FALSE;
	WCHAR wsDriverDir[MAX_PATH] = {0};

    nRetCode = OpenSCManager();
    if (!nRetCode)
        goto clean0;

	nRetCode = OpenService(
		TDI_FILTER_DRIVER, 
		hService, 
		fExist);
		if(!nRetCode)
			goto clean0;

	if(DeleteService( hService ))
	{
		hr = S_OK;
	}

clean0:

	if (hService)
	{
		CloseService(hService);
		hService = NULL;
	}

    CloseSCManager();

	//delete file
	if( !GetSystemDirectory( wsDriverDir, MAX_PATH ) )
	{
		_tcscat_s( wsDriverDir, MAX_PATH, L"\\drivers\\"TDI_ALEFILTER_MODULE );
		DeleteFile( wsDriverDir );
	}

    return hr;
}

//////////////////////////////////////////////////////////////////////////

int CFwInstaller::OpenSCManager()
{
    int retval = false;

    if (m_hSCManager)
    {
        retval = true;
        goto clean0;
    }

    m_hSCManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == m_hSCManager)
        goto clean0;

    retval = true;

clean0:
    return retval;
}

void CFwInstaller::CloseSCManager()
{
    if (m_hSCManager)
    {
        CloseServiceHandle(m_hSCManager);
        m_hSCManager = NULL;
    }
}

int CFwInstaller::OpenService(
    const WCHAR cwszServiceName[], 
    HANDLE& hService, 
    BOOL& fExist
    )
{
    int retval = false;
    SC_HANDLE hSvc = NULL;

    if (!m_hSCManager)
        goto clean0;

    ::SetLastError(0);
    hSvc = ::OpenService(m_hSCManager, cwszServiceName, SERVICE_ALL_ACCESS);
    if (!hSvc)
    {
        if (::GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
            fExist = FALSE;

        goto clean0;
    }

    fExist = TRUE;
    hService = (HANDLE)hSvc;

    retval = true;

clean0:
    return retval;
}

int CFwInstaller::CreateService(
    const WCHAR cwszServiceName[],
    const WCHAR cwszBinPath[],
    const WCHAR cwszLoadOrderGroup[],
    const WCHAR cwszDependencies[],
    HANDLE& hService,
	DWORD dwStartType
    )
{
    int retval = false;
    SC_HANDLE hSvc = NULL;
    DWORD dwTagId;

    if (!m_hSCManager)
        goto clean0;

    ::SetLastError(0);
    hSvc = ::CreateService(
        m_hSCManager,
        cwszServiceName,
        cwszServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        dwStartType,
        SERVICE_ERROR_NORMAL,
        cwszBinPath,
        cwszLoadOrderGroup,
        &dwTagId,
        cwszDependencies,
        NULL,
        NULL);
    if (!hSvc)
    {
        if (::GetLastError() == ERROR_SERVICE_EXISTS)
        {
            hService = (HANDLE)hSvc;
            retval = true;
        }

        goto clean0;
    }

    hService = (HANDLE)hSvc;

    retval = true;

clean0:
    return retval;
}

void CFwInstaller::CloseService(HANDLE hService)
{
    if (hService)
    {
        ::CloseServiceHandle((SC_HANDLE)hService);
    }
}

int CFwInstaller::DeleteService(HANDLE hService)
{
    int retval = false;
    BOOL fRetCode;

    if (!hService)
        goto clean0;

    fRetCode = ::DeleteService((SC_HANDLE)hService);
    if (!fRetCode)
        goto clean0;

    retval = true;

clean0:
    return retval;
}

//////////////////////////////////////////////////////////////////////////

int CFwInstaller::GetModuleDir()
{
    int retval = false;
    DWORD dwRetCode;

    if (wcslen(m_wszModuleDir))
    {
        retval = true;
        goto clean0;
    }

    dwRetCode = GetModuleFileName(g_szModule, m_wszModuleDir, MAX_PATH);
    if (!dwRetCode)
        goto clean0;

    *(wcsrchr(m_wszModuleDir, L'\\') + 1) = 0L;

    retval = true;

clean0:
    return retval;
}

int CFwInstaller::IsVistaPlatform()
{
	OSVERSIONINFOW _versionInfo = {0};
	_versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
	if(GetVersionExW( &_versionInfo ))
	{
		return _versionInfo.dwMajorVersion >= 6;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////

void CFwInstaller::ChangeTdiStartOrder()
{
	DWORD dwKfwTag = 0;
	DWORD dwType = 0;
	DWORD dwNetBtTag = 0;
	DWORD dwSize = sizeof(DWORD);
	if( ERROR_SUCCESS != SHGetValueW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\NetBT", 
		L"Tag", &dwType, &dwNetBtTag, &dwSize  ) )
		return ;

	if( ERROR_SUCCESS != SHGetValueW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		L"Tag", &dwType, &dwKfwTag, &dwSize  ) )
		return ;

	if( dwNetBtTag > dwKfwTag)
		return ;

	if( ERROR_SUCCESS != SHSetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\NetBT", 
		L"Tag", REG_DWORD, (PVOID)&dwKfwTag, sizeof(DWORD) ) )
		return ;

	if( ERROR_SUCCESS != SHSetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		L"Tag", REG_DWORD, (PVOID)&dwNetBtTag, sizeof(DWORD) ) )
		return ;
}

//////////////////////////////////////////////////////////////////////////

void CFwInstaller::ChangeAleStartOrder()
{
	DWORD dwKfwTag = 1;
	SHSetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		L"Tag", REG_DWORD, (PVOID)&dwKfwTag, sizeof(DWORD) );
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFwInstaller::SetConfig( DWORD dwEnableFlag, DWORD dwRequestFlag )
{
	HRESULT hResult = ERROR_SUCCESS;
	
	hResult = SHSetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		L"Enable", REG_DWORD, (PVOID)&dwEnableFlag, sizeof(DWORD) );
	if( hResult != ERROR_SUCCESS )
		return hResult;

	hResult = SHSetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\"TDI_FILTER_DRIVER, 
		L"Request", REG_DWORD, (PVOID)&dwRequestFlag, sizeof(DWORD) );
	if( hResult != ERROR_SUCCESS )
		return hResult;

	return hResult;

}

BOOL CFwInstaller::IsWow64()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE hProcess,PBOOL Wow64Process);
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandleA("kernel32"),"IsWow64Process");

	BOOL bIsWow64 = FALSE;

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
		{
			// handle error
		}
	}
	return bIsWow64;
}