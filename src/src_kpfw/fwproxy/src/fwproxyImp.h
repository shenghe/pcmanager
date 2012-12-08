#ifndef FWPROXYIMP_INC_
#define FWPROXYIMP_INC_

#include "fwproxy_public.h"
#include "ktdifilt_public.h"
#include "regiterator.h"

//////////////////////////////////////////////////////////////////////////

class CFwProxy :
	public IFwProxy,
	public KSComRoot<CFwProxy> 
{
public:

    CFwProxy();
    virtual ~CFwProxy();

	KS_DEFINE_GETCLSID(CLSID_IFwProxyImp);
	DECLARE_SINGLETON_OBJECT();

	KSCOM_QUERY_BEGIN
	KSCOM_QUERY_ENTRY( IFwProxy )
	KSCOM_QUERY_END

    STDMETHODIMP Initialize(IFwEvent* piEvent);
    STDMETHODIMP UnInitialize();
	STDMETHODIMP GetKfwState( KFW_STATE *kfwState );  

	//TDI接口
    STDMETHODIMP GetFlux(PULONGLONG pInbound, PULONGLONG pOutbound,
		PULONGLONG pInboundLocal, PULONGLONG pOutboundLocal);
    STDMETHODIMP EnumConnect(PTCP_CONN_INFO pConnInfo, ULONG dwInfoCount, PULONG pdwRetCount);
    STDMETHODIMP EnableFirewall();
    STDMETHODIMP DisableFirewall();
    STDMETHODIMP EnableNetwork();
    STDMETHODIMP DisableNetwork();
	STDMETHODIMP EnableFluxStastic();
	STDMETHODIMP DisableFluxStastic();

	STDMETHODIMP EnumProcessTraffic(PROCESS_TRAFFIC* pProcessTraffic, ULONG dwProcessCount, PULONG pdwRetCount);
	STDMETHODIMP EnumProgramTraffic(PROGRAM_TRAFFIC* pProgramTraffic, ULONG dwProgramCount, PULONG pdwRetCount);
	STDMETHODIMP EnumProgramTrafficFromCache( PROGRAM_TRAFFIC* pProgramTraffic, ULONG dwProgramCount, PULONG pdwRetCount);
	STDMETHODIMP EnumProgramTrafficFromFile( PROGRAM_TRAFFIC_FROM_FILE* pProgramTraffic, ULONG dwProgramCount, ULONG uStartItem, PULONG pdwRetCount );
	STDMETHODIMP DeleteProgramTrafficFile();
	STDMETHODIMP SetProgramTrafficTimer( ULONG uTimer );
	STDMETHODIMP ClearResultCache();

	//向驱动发送上层响应
	STDMETHODIMP SendResponse(ULONGLONG responseContext, ULONG dwResult);
	
	STDMETHODIMP GetModuleInfo(	PModuleInfo pModuleInfo, ULONG dwCount );
	STDMETHODIMP SetModuleInfo(	PModuleInfo pModuleInfo, ULONG dwCount );
	STDMETHODIMP GetModuleList(	PModuleInfo pModuleInfo, ULONG dwModuleCount, PULONG pdwRetCount);

	//设置数据记录器
	STDMETHODIMP AddRecorder( PDATA_RECORDER pRecorder, ULONG dwCount );
	STDMETHODIMP RemoveRecorder( ULONG dwModlueID );
	STDMETHODIMP ClearRecorder();

	STDMETHODIMP_(BOOL) OpenPort( PROTO_TYPE pt, USHORT uPort );
	STDMETHODIMP_(BOOL) ClosePort( PROTO_TYPE pt, USHORT uPort );
	STDMETHODIMP_(BOOL) GetOpenPort( PVOID pBitVec, ULONG uVecLen );

	STDMETHODIMP SetRequestFlag( ULONG uRequestFlag );

	STDMETHODIMP SetProcessSpeed( ULONGLONG uPid, ULONG uSendSpeed, ULONG uRecvSpeed , ULONG nDisable);

	STDMETHODIMP ClearUnNormalRunCount();
	STDMETHODIMP ClearBootState();
	STDMETHODIMP GetLastBootTime(ULONGLONG& nTime);
	STDMETHODIMP ClearLastBootTime();
	STDMETHODIMP AddDumpCount(ULONG nAdd);
	STDMETHODIMP ClearDumpCount();
	STDMETHODIMP SetForceDisableTdi(ULONG nValue);
	STDMETHODIMP SetHasUnCompatibleDriver(ULONG nValue);
	STDMETHODIMP_(BOOL) TestCanLoad();
	STDMETHODIMP ClearFlux();

protected:

	static UINT __stdcall WorkerThreadRoutine( LPVOID pParam );

	int RegisterEvent();
    int BeginWorkerThreads();
    int EndWorkerThreads();
	int GetRequestInfo( RESPONSE_APP_REQUEST_INFO *requestInfo );
	int GetCommonRequestInfo( APP_NOTIFY_INFO *appNotifyInfo );
	int SetDirverVersionError(ULONG bError);

	ULONGLONG GetUnNormalBootCount();
	ULONG	GetDumpCount();
	ULONG	GetForceDisableTDI();
	ULONG	GetHasUnCompatibleDriver();
	BOOL	CheckVersion();

    int OpenDevice();
    void CloseDevice();
	BOOL GetCachePathFromReg( WCHAR *szPath );
	VOID GetModulesFromReg();
	VOID GetModulePath( ULONG uModuleID, WCHAR* wsPath );

	BOOL IsWow64();

private:
    HANDLE          m_hDriver;
    IFwEvent*       m_piEvent;

    HANDLE          m_hCommEvent;
	HANDLE          m_hActiveEvent;

    HANDLE          m_hWorkerThread;
    HANDLE          m_hExitEvent;

	BOOL			m_bInit;
	map<ULONG, wstring> m_mapModules;
};

//////////////////////////////////////////////////////////////////////////


#endif  // !FWPROXYIMP_INC_

