////////////////////////////////////////////////////////////////////////////////
//      
//      File for url moniter service
//      
//      File      : kdriverproxy.cpp
//      Version   : 1.0
//      Comment   : 木马网络防火墙的驱动接口
//      
//      Create at : 2010-10-8
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "kdriverproxy.h"
#include "scom/scom/ksdll.h"
#include "atlconv.h"
#include "psapi.h"
#include "kmodulemgr.h"
#include "kprocessinfomgr.h"
#include "kprocessinfo.h"
#include "kurlcachemgr.h"
#include "shlobj.h"
#include "atltime.h"
#include "common/kis_directory.h"
#include "kpfw/msg_logger.h"

#pragma comment(lib, "Psapi.lib")

//////////////////////////////////////////////////////////////////////////
KDeviceNameMgr::KDeviceNameMgr()
{
	Refresh();
}

KDeviceNameMgr::~KDeviceNameMgr()
{

}

INT KDeviceNameMgr::FixDosPathToNormalPath(LPCWSTR strDevicePath, WCHAR path[MAX_PATH])
{
	kis::KLocker _locker(m_lock);
	// 先看看是否以特殊的头开始
	if (_wcsnicmp(strDevicePath, DEVICE_PREFIX, wcslen(DEVICE_PREFIX)) == 0)
	{
		if (DevicePathToDosPath(strDevicePath, path))
			return 1;
		else
		{
			Refresh();
			if (DevicePathToDosPath(strDevicePath, path))
				return 1;
		}
		return 0;
	}
	else if (_wcsnicmp(strDevicePath, SYSTEM_ROOT_PREFIX, wcslen(SYSTEM_ROOT_PREFIX)) == 0)
	{
		wcscpy_s(path, MAX_PATH, GetSystemRoot());
		wcscat_s(path , MAX_PATH - 2, strDevicePath + wcslen(SYSTEM_ROOT_PREFIX)-1);
		if (::PathFileExists(path))
			return 2;
	}
	return 0;
}


BOOL KDeviceNameMgr::DevicePathToDosPath(LPCWSTR strDevicepath, WCHAR path[MAX_PATH])
{
	map<wstring, wstring>::iterator it = m_DeviceNameMap.begin();
	while (it != m_DeviceNameMap.end())
	{
		const wstring& device = it->first;
		const wstring& dos = it->second;
		if (_wcsnicmp(device.c_str(), strDevicepath, device.length()) == 0)
		{
			wcscpy_s(path, MAX_PATH, dos.c_str());
			wcscat_s(path, MAX_PATH, strDevicepath+device.length());
			return TRUE;
		}
		it++;
	}
	return FALSE;
}

LPCWSTR	KDeviceNameMgr::GetSystemRoot() 
{
	if (m_strSystemRoot.length())
		return m_strSystemRoot.c_str();

	WCHAR pathSystem[MAX_PATH];
	::SHGetFolderPath(NULL, CSIDL_WINDOWS, NULL, SHGFP_TYPE_CURRENT, pathSystem);
	m_strSystemRoot = pathSystem;
	return m_strSystemRoot.c_str();
}

void KDeviceNameMgr::Refresh()
{
	m_DeviceNameMap.clear();

	TCHAR szTemp[BUFSIZE] = {0};

	if (GetLogicalDriveStrings(BUFSIZE-1, szTemp)) 
	{
		TCHAR szName[MAX_PATH];
		TCHAR szDrive[3] = TEXT(" :");
		BOOL bFound = FALSE;
		TCHAR* p = szTemp;
		do 
		{
			// Copy the drive letter to the template string
			*szDrive = *p;

			// Look up each device name
			if (QueryDosDevice(szDrive, szName, BUFSIZE))
			{
				UINT uNameLen = _tcslen(szName);
				wstring strDeviceName;
				strDeviceName = szName;
				wstring strDosName;
				strDosName = szDrive;
				m_DeviceNameMap[strDeviceName] = strDosName;
			}
			while (*p++);
		} while (!bFound && *p); // end of string
	}
}

//////////////////////////////////////////////////////////////////////////
KDeviceNameMgr KTdiDriverProxy::m_NameMgr;

KTdiDriverProxy::KTdiDriverProxy(): m_pProcessMgr(NULL),
									m_hEventInit(NULL),
									m_pFwProxy(NULL),
									m_nWorkItemCount(0),
									m_pModuleMgr(NULL),
									m_pUrlCfgMon(NULL)
{

}

KTdiDriverProxy::~KTdiDriverProxy()
{

}

HRESULT	KTdiDriverProxy::Init(KProcessInfoMgr* pProcessMgr, KModuleMgr* pModuleMgr, KUrlMonCfg* pUrlCfgMon)
{
	kws_log(TEXT("KTdiDriverProxy::Init begin"));
	m_pProcessMgr = pProcessMgr;
	m_pModuleMgr = pModuleMgr;
	m_pUrlCfgMon = pUrlCfgMon;

	// 启动驱动，如果发现没有安装，那么自动安装驱动，并且启动驱动
	if ( !LoadInterface(CLSID_IFwProxyImp, (VOID**)&m_pFwProxy) )
	{
		kws_log(TEXT("KTdiDriverProxy::Init LoadInterface failed"));
		return E_FAIL;
	}

	CheckMustUnLoadDriver();
	CheckDumpFile();

	if ( m_pFwProxy->Initialize( this ) != S_OK )
	{
		kws_log(TEXT("KTdiDriverProxy::Init Initialize driver failed 1"));
		if (!m_pFwProxy->TestCanLoad())
		{
			kws_log(TEXT("KTdiDriverProxy::Init find driver cannot load"));
			return E_FAIL;
		}

		if ( !StartDriverSvr(TDI_FILTER_DRIVER) )
		{
			kws_log(TEXT("KTdiDriverProxy::Init start driver failed prepare to install driver "));

			// 如果发现驱动不存子尝试安装驱动
			if (!InstallDriverSvr(ENABLE_TCP | ENABLE_UDP | ENABLE_PROCESS_TRAFFIC, 
				REQUEST_DOWNLOAD_PE))
				return E_FAIL;

			// 尝试再启动驱动
			CheckMustUnLoadDriver();
			if (!m_pFwProxy->TestCanLoad())
			{
				kws_log(TEXT("KTdiDriverProxy::Init find driver cannot load"));
				return E_FAIL;
			}

			if ( !StartDriverSvr(TDI_FILTER_DRIVER) )
			{
				kws_log(TEXT("KTdiDriverProxy::Init start driver failed 2"));
				return E_FAIL;
			}
		}
		CheckMustUnLoadDriver();
		if (FAILED(m_pFwProxy->Initialize(this)))	
		{
			kws_log(TEXT("KTdiDriverProxy::Init Initialize driver failed 2"));
			return E_FAIL;
		}
	}

	m_pFwProxy->EnableFirewall();
	m_pFwProxy->SetRequestFlag(REQUEST_TRUST_URL | REQUEST_DOWNLOAD_PE | REQUEST_LOOP_BACK_IP | ENABLE_TRAFFIC_CONTRL);

	m_hEventInit = ::CreateEvent(NULL, TRUE, TRUE, NULL);
	
	kws_log(TEXT("KTdiDriverProxy::Init end success"));
	return S_OK;
}

HRESULT	KTdiDriverProxy::UnInit()
{
	// 停止驱动
	if (m_pFwProxy)
	{
		m_pFwProxy->DisableFirewall();
		m_pFwProxy->SetRequestFlag(0);
	}
	
	// 等待worker线程处理完成，再反初始化
	INT nCnt = 0;
	while ((m_nWorkItemCount > 0) && (nCnt < 100))
	{
		::Sleep(50);
		nCnt++;
	}

	if (m_pFwProxy)
	{
		m_pFwProxy->UnInitialize();
		m_pFwProxy->Release();
		m_pFwProxy = NULL;
	}

	if (m_hEventInit)
	{
		::ResetEvent(m_hEventInit);
		::CloseHandle(m_hEventInit);
		m_hEventInit = NULL;
	}
	return S_OK;
}

BOOL KTdiDriverProxy::IsInit(HANDLE hEvent)
{
	if (hEvent == NULL) return FALSE;
	return (::WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0);
}

BOOL KTdiDriverProxy::InstallDriverSvr(DWORD dwEnable, DWORD dwRequest)
{
	ULONG status = E_FAIL;
	IFwInstaller *FwInstaller = NULL;
	if( LoadInterface(__uuidof(IFwInstaller), (VOID**)&FwInstaller) )
	{
		// 安装前先卸载
		status = FwInstaller->UnInstall();
		if (status != S_OK)
		{
			kws_log(TEXT("KTdiDriverProxy::InstallDriverSvr uninstall driver failed"));
		}
		status = FwInstaller->Install();
		if( status == S_OK )
		{
			FwInstaller->SetConfig( dwEnable, dwRequest );
		}
		else
		{	
			kws_log(TEXT("KTdiDriverProxy::InstallDriverSvr install failed:%08x"), status);
		}
	}
	else
	{
		kws_log(TEXT("KTdiDriverProxy::InstallDriverSvr 无法获取安装接口"));
	}

	kws_log(TEXT("%s:%08X"), status == S_OK ? "安装成功" : "安装失败", status );
	if( FwInstaller != NULL )
		FwInstaller->Release();

	return status == S_OK;
}

BOOL KTdiDriverProxy::LoadInterface(const GUID& iid,  void** pInterface )
{
	static KSDll dll;
	static BOOL b = FALSE;

	if (!b)
	{
		WCHAR wszProxy[MAX_PATH + 1] = { 0 };

		GetModuleFileName(NULL, wszProxy, MAX_PATH);
		*(wcsrchr(wszProxy, L'\\')) = 0L;
		wcscat_s(wszProxy, MAX_PATH, L"\\fwproxy.dll");

		if (S_OK == dll.Open(wszProxy))
			b = TRUE;
	}

	if (!b)
		return FALSE;

	return dll.GetClassObject(iid, iid, pInterface) == S_OK;
}

BOOL KTdiDriverProxy::StartDriverSvr(LPCTSTR lpcszSvrName)
{
	if ( !lpcszSvrName )
		return FALSE;

	BOOL bRetCode = FALSE;
	SC_HANDLE hSvrMgr = NULL;
	SC_HANDLE hSvrSpe = NULL;
	SERVICE_STATUS_PROCESS ssStatus; 
	DWORD dwOldCheckPoint; 
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;

	SetLastError(0);

	hSvrMgr = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if ( !hSvrMgr )
	{
		goto Exit0;
	}

	hSvrSpe = ::OpenService(hSvrMgr, lpcszSvrName, SERVICE_ALL_ACCESS);
	if ( !hSvrSpe )
	{
		goto Exit0;
	}

	if ( !::StartService(hSvrSpe, 0, NULL) && GetLastError() != ERROR_SERVICE_ALREADY_RUNNING )
	{
		goto Exit0;
	}

	if (!QueryServiceStatusEx( 
		hSvrSpe,             // handle to service 
		SC_STATUS_PROCESS_INFO, // info level
		(LPBYTE)&ssStatus,              // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded ) )              // if buffer too small
	{
		goto Exit0;
	}

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING) 
	{ 
		// Do not wait longer than the wait hint. A good interval is 
		// one tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if( dwWaitTime < 1000 )
			dwWaitTime = 1000;
		else if ( dwWaitTime > 10000 )
			dwWaitTime = 10000;

		Sleep( dwWaitTime );


		if (!QueryServiceStatusEx( 
			hSvrSpe,             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE)&ssStatus,              // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded ) )              // if buffer too small
			break;

		if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
		{
			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if( GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint )
			{
				break;
			}
		}
	} 

	bRetCode = (ssStatus.dwCurrentState == SERVICE_RUNNING);

Exit0:
	if ( hSvrMgr )
	{
		CloseServiceHandle(hSvrMgr);
		hSvrMgr = NULL;
	}

	if ( hSvrSpe )
	{
		CloseServiceHandle( hSvrSpe );
		hSvrSpe = NULL;
	}

	return bRetCode;
}

BOOL KTdiDriverProxy::IsDriverExist(LPCWSTR strDriverServiceName)
{
	if ( !strDriverServiceName )
		return FALSE;

	BOOL bRetCode = FALSE;
	SC_HANDLE hSvrMgr = NULL;
	SC_HANDLE hSvrSpe = NULL;
	SERVICE_STATUS_PROCESS ssStatus; 

	SetLastError(0);

	hSvrMgr = ::OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
	if ( !hSvrMgr )
	{
		goto Exit0;
	}

	hSvrSpe = ::OpenService(hSvrMgr, strDriverServiceName, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
	if ( !hSvrSpe )
	{
		goto Exit0;
	}
	bRetCode = TRUE;

Exit0:
	if ( hSvrMgr )
	{
		CloseServiceHandle(hSvrMgr);
		hSvrMgr = NULL;
	}

	if ( hSvrSpe )
	{
		CloseServiceHandle( hSvrSpe );
		hSvrSpe = NULL;
	}

	return bRetCode;
}

//////////////////////////////////////////////////////////////////////////
HRESULT KTdiDriverProxy::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (riid == __uuidof(IFwEvent))
	{
		*ppvObject = static_cast<IFwEvent*>(this);
		return S_OK;
	}
	return E_FAIL;
}

ULONG KTdiDriverProxy::AddRef( void)
{
	return 1;
}

ULONG KTdiDriverProxy::Release( void)
{
	return 1;
}

//////////////////////////////////////////////////////////////////////////
HRESULT	KTdiDriverProxy::OnApplicationRequest( RESPONSE_APP_REQUEST_INFO  *pAppRequestInfo )
{
	if (!IsInit(m_hEventInit))
	{
		if( pAppRequestInfo->pResponseContext != NULL )
			m_pFwProxy->SendResponse( pAppRequestInfo->pResponseContext, Pass);
		return S_OK;
	}

	WAIT_RESPONSE_INFO *waitResponseInfo = new WAIT_RESPONSE_INFO;
	if( waitResponseInfo == NULL )
		return E_FAIL;

	memcpy((char*)&waitResponseInfo->appResponseInfo, (char*)pAppRequestInfo, sizeof(RESPONSE_APP_REQUEST_INFO) );
	waitResponseInfo->pThis = (PVOID)this;
	waitResponseInfo->hEventInit = m_hEventInit;

	InterlockedIncrement(&m_nWorkItemCount);
	QueueUserWorkItem( WaitResponseThread, (LPVOID)waitResponseInfo, WT_EXECUTELONGFUNCTION );

	return S_OK;
}

HRESULT	KTdiDriverProxy::OnApplicationNotify( APP_NOTIFY_INFO  *pAppNotifyInfo )
{
	if (!IsInit(m_hEventInit)) return S_OK;

	APP_NOTIFY_INFO_CONTEXT *notifyInfoContext = new APP_NOTIFY_INFO_CONTEXT;
	if( notifyInfoContext == NULL )
		return E_FAIL;

	memcpy((char*)&notifyInfoContext->appNotifyInfo, (char*)pAppNotifyInfo, sizeof(APP_NOTIFY_INFO) );
	notifyInfoContext->pThis = (PVOID)this;
	notifyInfoContext->hEventInit = m_hEventInit;

	InterlockedIncrement(&m_nWorkItemCount);
	QueueUserWorkItem( AppNotifyThread, (LPVOID)notifyInfoContext, WT_EXECUTELONGFUNCTION );
	return S_OK;
}

DWORD KTdiDriverProxy::WaitResponseThread( LPVOID pParam )
{
	ULONG dwResponse = Pass;
	WAIT_RESPONSE_INFO *waitInfo = (WAIT_RESPONSE_INFO *)pParam;
	if ( pParam == NULL )
		return 0;

	KTdiDriverProxy* pThis = (KTdiDriverProxy*)waitInfo->pThis;
	RESPONSE_APP_REQUEST_INFO* pRequestInfo = (RESPONSE_APP_REQUEST_INFO*)&waitInfo->appResponseInfo;

	if (KTdiDriverProxy::IsInit(waitInfo->hEventInit))
		pThis->OnApplicationRequest_( &pRequestInfo->appRequestInfo ,pRequestInfo->pResponseContext, &dwResponse );
	else
	{
		// do other thing
	}

	delete waitInfo;
	return 0;
}

DWORD KTdiDriverProxy::AppNotifyThread( LPVOID pParam )
{
	ULONG dwResponse = Pass;
	APP_NOTIFY_INFO_CONTEXT *notifyInfo = (APP_NOTIFY_INFO_CONTEXT *)pParam;
	if ( pParam == NULL )
		return 0;

	KTdiDriverProxy* pThis = (KTdiDriverProxy*)notifyInfo->pThis;
	APP_NOTIFY_INFO* pRequestInfo = (APP_NOTIFY_INFO*)&notifyInfo->appNotifyInfo;

	if (KTdiDriverProxy::IsInit(notifyInfo->hEventInit))
		pThis->OnApplicationNotify_( &notifyInfo->appNotifyInfo );

	delete notifyInfo;
	return 0;	
}

STDMETHODIMP KTdiDriverProxy::OnApplicationRequest_( APP_REQUEST_INFO  *pAppRequestInfo, ULONGLONG pContent, PULONG pAllow )
{
	DWORD nResult = Pass;

	if (!m_pProcessMgr || !m_pModuleMgr) goto EXIT0;
	/*
	if( (pAppRequestInfo->nRequestProtocol == enumPT_HTTP) &&
		(pAppRequestInfo->nRequestType == ART_TrustUrl) &&
		(pAppRequestInfo->dwProcessId != ::GetCurrentProcessId()))
	{
		nResult = Pass;
		
		//  todo: 对ie等热门软件，先放行，再查询
		// 对其他软件，先查询安全性，再放行
		if (m_pUrlCfgMon && m_pUrlCfgMon->GetEnable())
		{
			KProcessInfo* pProcess = m_pProcessMgr->GetProcessInfo(pAppRequestInfo->dwProcessId, pAppRequestInfo->dwProcessCreateID);
			if (!pProcess) goto EXIT0;

			// 如果是中途起的服务，那么有可能process拿不到
			if (!pProcess->HasExeImage())
			{
				HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, pAppRequestInfo->dwProcessId);
				if (hProcess)
				{
					WCHAR exeModule [MAX_PATH] = {0};
					if (!pProcess->HasExeImage())
					{
						if (!GetModuleFileNameEx(hProcess, NULL, exeModule, MAX_PATH))
						{
							kws_log(TEXT("GetModuleFileNameEx GetProcess: %d failed: %d"), pAppRequestInfo->dwProcessId, ::GetLastError());
						}
						else
						{
							ULONGLONG nModuleID = 0;
							KModuleInfo* pModule = NULL;
							m_pModuleMgr->AddModule(exeModule, nModuleID, pModule);
							pProcess->SetExeImage(pModule);
						}
					}

					CloseHandle(hProcess);
					hProcess = NULL;
				}
			}
			
			
			pProcess->OnUrlSend(pAppRequestInfo->dwEventID,
				pAppRequestInfo->Parameters.TrustUrl.szHost,
				pAppRequestInfo->Parameters.TrustUrl.szUrl, 
				pAppRequestInfo->Parameters.TrustUrl.nOp, nResult);
			// m_pUrlCfgMon->AddCheckUrlCnt();
			// if (nResult == Deny)
			//	m_pUrlCfgMon->AddBlockUrlCnt();
			// 先屏蔽trust url 的拦截
		}
	}*/

EXIT0:
	if (m_pFwProxy)
		m_pFwProxy->SendResponse( pContent, nResult );

	InterlockedDecrement(&m_nWorkItemCount);
	return S_OK;
}

STDMETHODIMP KTdiDriverProxy::OnApplicationNotify_( APP_NOTIFY_INFO  *pAppNotifyInfo)
{
	if (!m_pProcessMgr || !m_pModuleMgr) goto EXIT0;

	if( (pAppNotifyInfo->nRequestType == ART_LoadImage) &&
		(pAppNotifyInfo->dwProcessId != ::GetCurrentProcessId()))
	{
		WCHAR pathModule [MAX_PATH] = {0};
		BOOL bFindModule = m_NameMgr.FixDosPathToNormalPath(pAppNotifyInfo->Parameters.imageLoadInfo.m_strImagePath,pathModule);
		
		KProcessInfo* pInfo = m_pProcessMgr->GetProcessInfo(pAppNotifyInfo->dwProcessId, pAppNotifyInfo->dwProcessCreateID);
		if (!pInfo) goto EXIT0;
		/*
		if (!pInfo->HasExeImage() || !bFindModule)
		{
			HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, pAppNotifyInfo->dwProcessId);
			if (hProcess)
			{
				WCHAR exeModule [MAX_PATH] = {0};
				if (!pInfo->HasExeImage())
				{
					if (!GetModuleFileNameEx(hProcess, NULL, exeModule, MAX_PATH))
					{
						kws_log(TEXT("GetModuleFileNameEx GetProcess: %d failed: %d"), pAppNotifyInfo->dwProcessId, ::GetLastError());
					}
					else
					{
						ULONGLONG nModuleID = 0;
						KModuleInfo* pModule = NULL;
						m_pModuleMgr->AddModule(exeModule, nModuleID, pModule);
						pInfo->SetExeImage(pModule);
					}
				}
				if (!bFindModule)
				{
					// 尝试通过ImageBase或者枚举的方法直接获取路径
					bFindModule = GetModuleNameFromProcess(hProcess,
						pAppNotifyInfo->Parameters.imageLoadInfo.m_imageInfo.ImageBase,
						pAppNotifyInfo->Parameters.imageLoadInfo.m_strImagePath,
						pathModule);

					if (!bFindModule)
					{
						INT nFindSpecail = FixDosPathToNormalPath(pAppNotifyInfo->Parameters.imageLoadInfo.m_strImagePath, 
							exeModule, pathModule);
						if (nFindSpecail)
							bFindModule = TRUE;
					}
				}
				::CloseHandle(hProcess);
				hProcess = NULL;
			}
		}
		// 向进程中添加module
		if (bFindModule)
		{
			ULONGLONG nModuleID = 0;
			KModuleInfo* pModule = NULL;
			m_pModuleMgr->AddModule(pathModule, nModuleID, pModule);
			if (!pModule->IsSafe())
				pInfo->OnLoadImage(pAppNotifyInfo->dwEventID, pModule);	
		}
		else
		{
			kws_log(TEXT("******cannot find module path with enum_process_modules or guess path"));
		}*/
	}
EXIT0:
	InterlockedDecrement(&m_nWorkItemCount);
	return S_OK;
}

BOOL KTdiDriverProxy::GetModuleNameFromProcess(HANDLE hProcess, PVOID ImageBase, LPCWSTR strDosPath, WCHAR path[MAX_PATH])
{
	BOOL bFindModule = FALSE;

	WCHAR szModName[MAX_PATH];
	if ( GetModuleFileNameEx( hProcess, (HMODULE)ImageBase, szModName,
		MAX_PATH))
	{
		if (IsSamePath(strDosPath, szModName+2))
		{
			// kws_log(TEXT("find module same path direct with imagebase: %s"), szModName);
			wcscpy_s(path, MAX_PATH, szModName);
			bFindModule = TRUE;
			return bFindModule;
		}
	}
	else
	{
		HMODULE hModules[1024] = {0};
		DWORD nSizeModule = 0;
		::EnumProcessModules(hProcess, hModules, sizeof(hModules), &nSizeModule);
		for (DWORD i = 0; i < nSizeModule / sizeof(HMODULE); i++)
		{
			WCHAR szModName[MAX_PATH];

			if ( GetModuleFileNameEx( hProcess, hModules[i], szModName,
				MAX_PATH))
			{
				if (IsSamePath(strDosPath, szModName+2))
				{
					kws_log(TEXT("find module same path: %s"), szModName);
					wcscpy_s(path, MAX_PATH, szModName);
					bFindModule = TRUE;
				}
			}

			if (bFindModule)
			{
				if (hModules[i] == ImageBase)
				{
					kws_log(TEXT("find module handle equal module base"));
				}
				else
				{
					kws_log(TEXT("find module handle not equal module base base: %08x, handle:%08x"), ImageBase, hModules[i] );
				}
				break;
			}
		}
	}
	return bFindModule;
}

BOOL KTdiDriverProxy::IsSamePath(LPCWSTR dosPath, LPCWSTR strNormanPath)
{
	size_t nLen1 = wcslen(dosPath);
	size_t nLen2 = wcslen(strNormanPath);
	while (nLen1 && nLen2)
	{
		if (tolower(dosPath[nLen1-1]) == tolower(strNormanPath[nLen2-1]))
		{
			nLen1--,nLen2--;
			continue;
		}
		else
			break;
	}
	if (nLen2 == 0)
		return TRUE;
	return FALSE;
}

INT KTdiDriverProxy::FixDosPathToNormalPath(LPCWSTR dosPath, LPCWSTR pathProcess, WCHAR path[MAX_PATH])
{
	WCHAR pathSystem[MAX_PATH];
	::SHGetFolderPath(NULL, CSIDL_WINDOWS, NULL, SHGFP_TYPE_CURRENT, pathSystem);

	// 先看看是否以特殊的头开始
	if (_wcsnicmp(dosPath, SYSTEM_ROOT_PREFIX, wcslen(SYSTEM_ROOT_PREFIX)) == 0)
	{
		wcscpy_s(path, MAX_PATH, pathSystem);
		wcscat_s(path , MAX_PATH - 2, dosPath + wcslen(SYSTEM_ROOT_PREFIX)-1);
		if (::PathFileExists(path))
			return 1;
	}
	else if (_wcsnicmp(dosPath, WINDOWS_FOLDER_PREFIX, wcslen(WINDOWS_FOLDER_PREFIX)) == 0)
	{
		wcsncpy_s(path, MAX_PATH, pathSystem, 2);
		wcscpy_s(path+2 , MAX_PATH - 2, dosPath);
		if (::PathFileExists(path))
			return 2;
	}

	// 先看看是否与本进程在同一个盘下面
	WCHAR cbProcess = tolower(pathProcess[0]);
	wcsncpy_s(path, MAX_PATH, pathProcess, 2);
	wcscpy_s(path+2 , MAX_PATH - 2, dosPath);
	if (::PathFileExists(path))
		return 3;

	// 看是否在系统盘下面
	WCHAR cbSystem = tolower(pathSystem[0]);
	path[0] = pathSystem[0];
	if (::PathFileExists(path))
		return 4;

	// 枚举盘符
	for (WCHAR cb = L'c'; cb <= L'z'; cb++)
	{
		if ((cb != cbSystem) && (cb != cbProcess))
		{
			path[0] = cb;
			if (::PathFileExists(path))
				return 5;
		}
	}
	return 0;
}

void KTdiDriverProxy::ClearDumpState()
{
	if (!m_pFwProxy)
	{
		if ( !LoadInterface(CLSID_IFwProxyImp, (VOID**)&m_pFwProxy) )
		{
			kws_log(TEXT("KTdiDriverProxy::ClearDumpState LoadInterface failed"));
			return ;
		}
	}
	if (m_pFwProxy)
		m_pFwProxy->ClearBootState();
}

BOOL KTdiDriverProxy::CheckDumpFile()
{
	kws_log(TEXT("KTdiDriverProxy::CheckDumpFile start"));

	if (!m_pFwProxy) return FALSE;
	ULONGLONG nTime = 0;
	m_pFwProxy->GetLastBootTime(nTime);
	if (nTime != 0)
	{
		// 枚举系统的minidump和MEMORY.DMP，找出是否上次崩溃蓝屏
		WCHAR szOSUserPath[MAX_PATH] = {0};
		::GetWindowsDirectory(szOSUserPath, MAX_PATH);

		::PathAppend(szOSUserPath, TEXT("MEMORY.DMP"));
		if (IsFileCreateLaterThan(nTime, szOSUserPath))
		{
			m_pFwProxy->AddDumpCount(1);
			kws_log(TEXT("clear last boot time"));
			m_pFwProxy->ClearLastBootTime();
			return TRUE;
		}

		::PathRemoveFileSpec(szOSUserPath);
		::PathAppend(szOSUserPath, TEXT("minidump"));

		kis::KFileIterator iter(TEXT("*.dmp"), szOSUserPath, FALSE, FALSE);
		while (iter.Next())
		{
			if (IsFileCreateLaterThan(nTime, iter.GetFileName()))
			{
				m_pFwProxy->AddDumpCount(1);
				kws_log(TEXT("clear last boot time"));
				m_pFwProxy->ClearLastBootTime();
				return TRUE;
			}
		}
		kws_log(TEXT("clear last boot time"));
		m_pFwProxy->ClearLastBootTime();
	}
	else
	{
		kws_log(TEXT("find last boot time is 0"));
	}
	return TRUE;
}

BOOL KTdiDriverProxy::IsFileCreateLaterThan(ULONGLONG nTime, LPCWSTR strFile)
{
	BOOL bResult = FALSE;
	HANDLE hFile = ::CreateFileW(strFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ((hFile != INVALID_HANDLE_VALUE) && (hFile != NULL))
	{
		FILETIME time = {0};
		::GetFileTime(hFile, NULL, NULL, &time);
		ULARGE_INTEGER inte = {0};
		inte.HighPart = time.dwHighDateTime;
		inte.LowPart = time.dwLowDateTime;
		if (nTime < inte.QuadPart)
		{
			kws_log(TEXT("find a dump file after last boot: %s"), strFile);
			bResult = TRUE;
		}
		::CloseHandle(hFile);
	}
	return bResult;
}
BOOL KTdiDriverProxy::CheckMustUnLoadDriver()
{
	DoCheckForceDisableTDI();
	DoCheckHasUnCompatibleDriver();
	return TRUE;
}

BOOL KTdiDriverProxy::DoCheckForceDisableTDI()
{
	WCHAR bufPath[MAX_PATH] = {0};
	::GetModuleFileName(NULL, bufPath, MAX_PATH);
	::PathRemoveFileSpecW(bufPath);
	::PathAppend(bufPath, TEXT("AppData\\msg.ini"));
	int nNotify = GetPrivateProfileIntW(TEXT("netmon"), TEXT("forcedisabletdi"), 0, bufPath);

	IFwProxy* pProxy  = NULL;
	if ( !LoadInterface(CLSID_IFwProxyImp, (VOID**)&pProxy) )
	{
		kws_log(TEXT("KTdiDriverProxy::DoCheckForceDisableTDI LoadInterface failed"));
		return FALSE;
	}
	pProxy->SetForceDisableTdi(nNotify);
	pProxy->Release();
	return TRUE;
}

#define AVG_TDI_DRIVER_NAME	L"AvgTdiX"

BOOL KTdiDriverProxy::DoCheckHasUnCompatibleDriver()
{
	// BOOL bExist FALSE; = IsDriverExist(AVG_TDI_DRIVER_NAME);
	// kws_log(L"KTdiDriverProxy::DoCheckHasUnCompatibleDriver find driver:%s,  %d", AVG_TDI_DRIVER_NAME, bExist);

	BOOL bExist = HasUnCompatibleSoftware();

	IFwProxy* pProxy  = NULL;
	if ( !LoadInterface(CLSID_IFwProxyImp, (VOID**)&pProxy) )
	{
		kws_log(TEXT("KTdiDriverProxy::DoCheckHasUnCompatibleDriver LoadInterface failed"));
		return FALSE;
	}
	pProxy->SetHasUnCompatibleDriver(bExist);
	pProxy->Release();
	return TRUE;
}

BOOL KTdiDriverProxy::HasUnCompatibleSoftware()
{
	BOOL bResult = FALSE;
	// happy dailer
	HKEY hKey = NULL;
	if (ERROR_SUCCESS == (::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\HappyDialer_is1",
		0,
		KEY_READ, &hKey
		)))
	{
		kws_log(L"KTdiDriverProxy::DoCheckHasUnCompatibleDriver uncompatible software :%s", L"happydailer");
		::CloseHandle(hKey);
		bResult = TRUE;
	}
	return bResult;
}