#pragma once
#include "logmgr_public.h"
#include "shlobj.h"
#include <string>
using namespace std;

#define BUFSIZE 512
#define SYSTEM_ROOT_PREFIX		L"\\SystemRoot\\"
#define WINDOWS_FOLDER_PREFIX		L"\\WINDOWS\\"
#define DEVICE_PREFIX			L"\\Device\\"

class KDeviceNameMgr
{
public:
	KDeviceNameMgr()
	{
		Refresh();
	}

	~KDeviceNameMgr()
	{

	}

	INT FixDosPathToNormalPath(LPCWSTR strDevicePath, WCHAR path[MAX_PATH])
	{
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

private:

	BOOL	DevicePathToDosPath(LPCWSTR strDevicepath, WCHAR path[MAX_PATH])
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

	LPCWSTR		GetSystemRoot() 
	{
		if (m_strSystemRoot.length())
			return m_strSystemRoot.c_str();

		WCHAR pathSystem[MAX_PATH];
		::SHGetFolderPath(NULL, CSIDL_WINDOWS, NULL, SHGFP_TYPE_CURRENT, pathSystem);
		m_strSystemRoot = pathSystem;
		return m_strSystemRoot.c_str();
	}

	void Refresh()
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
private:
	map<wstring, wstring>	m_DeviceNameMap;
	wstring					m_strSystemRoot;
};

class SimpleKfwEvent:public IFwEvent
{
public:
	SimpleKfwEvent(void);
	virtual ~SimpleKfwEvent(void);

	virtual STDMETHODIMP OnApplicationRequest( RESPONSE_APP_REQUEST_INFO  *pAppRequestInfo );
	virtual STDMETHODIMP OnApplicationNotify( APP_NOTIFY_INFO  *pAppNotifyInfo );
	
	virtual STDMETHODIMP Initialize();
	virtual STDMETHODIMP UnInitialize();
	//////////////////////////////////////////////////////////////////////////
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		return S_OK;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef( void ) 
	{
		return S_OK;
	}

	virtual ULONG STDMETHODCALLTYPE Release( void)
	{
		return S_OK;
	}

	struct WAIT_RESPONSE_INFO
	{
		PVOID pThis;
		RESPONSE_APP_REQUEST_INFO appResponseInfo;
	};

	struct APP_NOTIFY_INFO_CONTEXT
	{
		PVOID pThis;
		APP_NOTIFY_INFO appNotifyInfo;
	};
	KDeviceNameMgr&		GetDeviceNameMgr() {return m_DeviceNameMgr;}

private:
	BOOL LoadLogMgr(const GUID& iid,  void** pInterface );
	STDMETHODIMP OnApplicationRequest_( APP_REQUEST_INFO  *pAppRequestInfo, PULONG pAllow );
	STDMETHODIMP OnApplicationNotify_( APP_NOTIFY_INFO  *pAppNotifyInfo );
	BOOL GetStackModule( ULONG *uModuleID, ULONG *uRetAddr, string &strModule, wstring &strExePath );
	static DWORD __stdcall WaitResponseThread( LPVOID pParam );
	static DWORD __stdcall AppNotifyThread( LPVOID pParam );
	ILogMgr *m_logMgr;

	KDeviceNameMgr		m_DeviceNameMgr;
};


//////////////////////////////////////////////////////////////////////////

void ConvertUncode( const string &sString, wstring &wsString );
void ConvertAnsi(const wchar_t *wsUncodeString, string &sString);
VOID WriteDateToFile( TCHAR *szFileName, PVOID pBuffer, DWORD dwLength );