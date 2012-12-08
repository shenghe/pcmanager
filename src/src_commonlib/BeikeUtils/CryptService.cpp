#include "BeikeUtils.h"
#include "WinService.h"
#include <shlobj.h>

#define CRYPT_SERVICE_NAME _T("CryptSvc")
#define UPDATE_SERVIE_NAME _T("wuauserv")

HRESULT WriteCryptSvcPathInfo()
{
	// %SystemRoot%\system32\cryptsvc.dll
	// "ServiceDll"="%SystemRoot%\\System32\\cryptsvc.dll"	REG_EXPAND_SZ 
	// "ServiceMain"="CryptServiceMain"
	// "ServiceDllUnloadOnStop"=dword:00000001
	CString strSubPath = _T("SYSTEM\\CurrentControlSet\\Services\\CryptSvc\\Parameters");
	WriteRegExString(HKEY_LOCAL_MACHINE, strSubPath, _T("ServiceDll"), _T("%SystemRoot%\\System32\\cryptsvc.dll"));
	WriteRegString(HKEY_LOCAL_MACHINE, strSubPath, _T("ServiceMain"), _T("CryptServiceMain"));

	if(IsVistaOrLater())
		WriteRegDWord(HKEY_LOCAL_MACHINE, strSubPath, _T("ServiceDllUnloadOnStop"), 1);
	return S_OK;
}

HRESULT InstallSvc_2000_XP()
{
	LPCTSTR lpServiceName = CRYPT_SERVICE_NAME;
	LPCTSTR lpDisplayName = _T("Cryptographic Services");
	DWORD dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
	LPCTSTR lpBinaryPathName = _T("%SystemRoot%\\system32\\svchost.exe -k netsvcs");
	LPCTSTR lpServiceStartName = _T("LocalSystem");
	LPCTSTR lpServiceDesc = _T("提供三种管理服务: 编录数据库服务，它确定 Windows 文件的签字; 受保护的根服务，它从此计算机添加和删除受信根证书机构的证书;和密钥(Key)服务，它帮助注册此计算机获取证书。如果此服务被终止，这些管理服务将无法正常运行。如果此服务被禁用，任何依赖它的服务将无法启动。");
	LPCTSTR lpDependencies = _T("RpcSs\0\0");

	HRESULT hr = Win::InstallSvc(lpServiceName, lpDisplayName, dwServiceType, lpBinaryPathName, lpServiceStartName, lpDependencies);
	if(SUCCEEDED(hr))
	{
		Win::UpdateSvcDesc(lpServiceName, lpServiceDesc);
		WriteCryptSvcPathInfo();
	}	
	return hr;
}

HRESULT InstallSvc_Vista_Win7()
{
	LPCTSTR lpServiceName = CRYPT_SERVICE_NAME;
	LPCTSTR lpDisplayName = _T("@%SystemRoot%\\system32\\cryptsvc.dll,-1001");
	DWORD dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
	LPCTSTR lpBinaryPathName = _T("%SystemRoot%\\system32\\svchost.exe -k NetworkService");
	LPCTSTR lpServiceStartName = _T("NT Authority\\NetworkService");
	LPCTSTR lpServiceDesc = _T("@%SystemRoot%\\system32\\cryptsvc.dll,-1002");
	LPCTSTR lpDependencies = _T("RpcSs\0\0");
	/*
	SeChangeNotifyPrivilege
	SeCreateGlobalPrivilege
	SeImpersonatePrivilege*/

	HRESULT hr = Win::InstallSvc(lpServiceName, lpDisplayName, dwServiceType, lpBinaryPathName, lpServiceStartName, lpDependencies);
	if(SUCCEEDED(hr))
	{
		Win::UpdateSvcDesc(lpServiceName, lpServiceDesc);
		WriteCryptSvcPathInfo();
	}
	return hr;
}


BOOL IsCryptServiceExists(BOOL &bExists)
{
	return SUCCEEDED( Win::IsServiceExists(CRYPT_SERVICE_NAME, bExists) );
}

BOOL IsCryptServiceCorrupt() 
{
	// Service 是错误的 
	return !RegKeyExists(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\CryptSvc\\Parameters"));
}

BOOL InstallCryptService()
{
	HRESULT hr = IsVistaOrLater() ? InstallSvc_Vista_Win7() : InstallSvc_2000_XP();
	return SUCCEEDED(hr);
}

HRESULT GetWindowsTempFilename(CString &strFilename, LPCTSTR szPrefix)
{
	TCHAR szPath[_MAX_PATH] = {0}; 
	TCHAR tmpFileName[_MAX_PATH] = {0}; 

	DWORD dwRet = GetTempPath(_MAX_DIR, szPath);
	if (dwRet == 0)
	{
		return AtlHresultFromLastError();
	}
	else if (dwRet > _MAX_DIR)
	{
		return DISP_E_BUFFERTOOSMALL;
	}

	if (!GetTempFileName(szPath, szPrefix, 0, tmpFileName))
	{
		// Couldn't create temporary filename;
		return AtlHresultFromLastError();
	}
	strFilename = tmpFileName;
	return S_OK;
}

BOOL ExecWaitProc( LPCTSTR lpAppPath, LPCTSTR lpParam, INT nShow)
{
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = {0};

	si.cb= sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = nShow;

	BOOL bRet = CreateProcess( lpAppPath, (LPTSTR)lpParam, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi );
	if( bRet )
	{
		WaitForSingleObject(pi.hProcess,INFINITE);

		CloseHandle( pi.hThread );
		CloseHandle( pi.hProcess );
	}

	return bRet;
}

BOOL FixRegByResource(LPCTSTR lpName, LPCTSTR lpType)
{
	HRSRC	hrRes = NULL;
	DWORD	dwResSize = 0;
	LPVOID	lpRes = NULL;
	HGLOBAL hResGlobal = NULL;

	if(hrRes = FindResource(NULL, lpName, lpType))
	{
		if(hResGlobal = LoadResource(NULL, hrRes))
		{
			dwResSize = SizeofResource(NULL, hrRes);
			lpRes = LockResource(hResGlobal);

			//USES_CONVERSION;
			//CStringA str = CT2CA((TCHAR*)lpRes);
			if(lpRes)
			{
				CString strfilename;
				GetWindowsTempFilename(strfilename, _T("crypt"));
				if( file_put_contents(strfilename, (BYTE*)lpRes, dwResSize) )
				{
					TCHAR reg_path[MAX_PATH] = {0};
					SHGetSpecialFolderPath(NULL, reg_path, CSIDL_WINDOWS, FALSE );

					CString strCommand;
					strCommand.Format(_T("%s\\regedit.exe /s \"%s\""), reg_path, strfilename);

					ExecWaitProc(NULL, strCommand, SW_HIDE);
#ifdef _DEBUG
					DeleteFile(strfilename);
#endif
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOL EnableUpdateRelatedService()
{
	BOOL cryptEnabled = TRUE, updateEnabled=TRUE;
	SERVICE_STATUS st = {0};
	if(SUCCEEDED(Win::GetServiceStatus(CRYPT_SERVICE_NAME, st)))
	{
		if(st.dwCurrentState!=4)
		{
			BOOL bEnabled = FALSE;
			if( SUCCEEDED(Win::IsServiceEnabled(CRYPT_SERVICE_NAME, bEnabled)) && !bEnabled)
			{
				cryptEnabled = SUCCEEDED(Win::EnableService(CRYPT_SERVICE_NAME, FALSE));
			}
		}
	}
	
	if(SUCCEEDED(Win::GetServiceStatus(UPDATE_SERVIE_NAME, st)))
	{
		if(st.dwCurrentState!=4)
		{
			BOOL bEnabled = FALSE;
			if( SUCCEEDED(Win::IsServiceEnabled(UPDATE_SERVIE_NAME, bEnabled)) && !bEnabled)
			{
				updateEnabled = SUCCEEDED(Win::EnableService(UPDATE_SERVIE_NAME, FALSE));
			}
		}
	}
	return cryptEnabled && updateEnabled;
}