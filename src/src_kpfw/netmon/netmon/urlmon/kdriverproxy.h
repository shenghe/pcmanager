////////////////////////////////////////////////////////////////////////////////
//      
//      File for url moniter service
//      
//      File      : kdriverproxy.h
//      Version   : 1.0
//      Comment   : 木马网络防火墙的驱动接口
//      
//      Create at : 2010-10-8
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "kpfw/fwproxy_public.h"
#include "kpfw/ktdifilt_public.h"
#include "common/kis_lock.h"
#include <map>
#include <string>
using namespace std;

//////////////////////////////////////////////////////////////////////////
#define BUFSIZE 512
#define SYSTEM_ROOT_PREFIX		L"\\SystemRoot\\"
#define WINDOWS_FOLDER_PREFIX		L"\\WINDOWS\\"
#define DEVICE_PREFIX			L"\\Device\\"

class KDeviceNameMgr
{
public:
	KDeviceNameMgr();
	~KDeviceNameMgr();
	INT			FixDosPathToNormalPath(LPCWSTR strDevicePath, WCHAR path[MAX_PATH]);

private:
	BOOL		DevicePathToDosPath(LPCWSTR strDevicepath, WCHAR path[MAX_PATH]);
	LPCWSTR		GetSystemRoot() ;
	void		Refresh();
private:
	kis::KLock				m_lock;
	map<wstring, wstring>	m_DeviceNameMap;
	wstring					m_strSystemRoot;
};

//////////////////////////////////////////////////////////////////////////
class KProcessInfoMgr;
class KModuleMgr;
class KUrlMonCfg;

struct WAIT_RESPONSE_INFO
{
	PVOID pThis;
	HANDLE hEventInit;
	RESPONSE_APP_REQUEST_INFO appResponseInfo;
};

struct APP_NOTIFY_INFO_CONTEXT
{
	PVOID pThis;
	HANDLE hEventInit;
	APP_NOTIFY_INFO appNotifyInfo;
};

#define TDI_FILTER_DRIVER       L"kmodurl"

class KTdiDriverProxy: public IFwEvent
{
public:
	KTdiDriverProxy();
	~KTdiDriverProxy();

	// interface IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef( void);

	virtual ULONG STDMETHODCALLTYPE Release( void);

	// interface IFwEvent
	virtual STDMETHODIMP OnApplicationRequest( RESPONSE_APP_REQUEST_INFO  *pAppRequestInfo );
	virtual STDMETHODIMP OnApplicationNotify( APP_NOTIFY_INFO  *pAppNotifyInfo );

	HRESULT	Init(KProcessInfoMgr* pProcessMgr, KModuleMgr* pModuleMgr, KUrlMonCfg* pUrlCfgMon);
	HRESULT	UnInit();
	IFwProxy* GetDriverProxy() {return m_pFwProxy;}
	void	ClearDumpState();

	BOOL	CheckMustUnLoadDriver();
	BOOL	DoCheckForceDisableTDI();
	BOOL	DoCheckHasUnCompatibleDriver();
	static KDeviceNameMgr			m_NameMgr;

private:
	BOOL   StartDriverSvr(LPCTSTR lpcszSvrName);
	BOOL   InstallDriverSvr(DWORD dwEnable, DWORD dwRequest);
	BOOL   GetModuleNameFromProcess(HANDLE hProcess, PVOID ImageBase, LPCWSTR strDosPath, WCHAR path[MAX_PATH]);
	INT    FixDosPathToNormalPath(LPCWSTR dosPath, LPCWSTR pathProcess, WCHAR path[MAX_PATH]);
	BOOL   IsSamePath(LPCWSTR dosPath, LPCWSTR strNormanPath);
	BOOL	IsDriverExist(LPCWSTR strDriverServiceName);
	BOOL	CheckDumpFile();
	BOOL	IsFileCreateLaterThan(ULONGLONG nTime, LPCWSTR strFile);
	BOOL	HasUnCompatibleSoftware();

	STDMETHODIMP OnApplicationRequest_( APP_REQUEST_INFO  *pAppRequestInfo, ULONGLONG pContent, PULONG pAllow );
	STDMETHODIMP OnApplicationNotify_( APP_NOTIFY_INFO  *pAppNotifyInfo);

	static BOOL   LoadInterface(const GUID& iid,  void** pInterface);

	static BOOL  __stdcall IsInit(HANDLE hEvent);
	static DWORD __stdcall WaitResponseThread( LPVOID pParam );
	static DWORD __stdcall AppNotifyThread( LPVOID pParam );


private:
	IFwProxy *				m_pFwProxy;
	KProcessInfoMgr*		m_pProcessMgr;
	KModuleMgr*				m_pModuleMgr;
	KUrlMonCfg*				m_pUrlCfgMon;
	HANDLE					m_hEventInit;
	volatile LONG			m_nWorkItemCount;
};