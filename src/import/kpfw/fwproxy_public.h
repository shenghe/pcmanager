#ifndef FWPROXY_PUBLIC_INC_
#define FWPROXY_PUBLIC_INC_

#include <unknwn.h>
#include "netwatch.h"
#include "netlog.h"


//////////////////////////////////////////////////////////////////////////

[
    uuid(78b4fb21-ecab-4086-8892-afb5b46daa4b)
]
interface IFwEvent : IUnknown
{
	virtual STDMETHODIMP OnApplicationRequest( RESPONSE_APP_REQUEST_INFO  *pAppRequestInfo ) = 0;
	virtual STDMETHODIMP OnApplicationNotify( APP_NOTIFY_INFO  *pAppNotifyInfo ) = 0;
};

//////////////////////////////////////////////////////////////////////////

[
	uuid(441A0135-5E09-4cf2-B536-098AF58F0489)
]
interface IRedirect : IUnknown
{
	virtual STDMETHODIMP_(BOOL) Init() = 0;
	virtual STDMETHODIMP_(BOOL) UnInit() = 0;

	//ret: 1-65535 需要重定向的端口，0：不需要重定向
	virtual STDMETHODIMP_(USHORT) NeedRedirect( 
		const WCHAR *szModlueName, 
		ULONG uLocalIP, USHORT uLocalPort, 
		ULONG uRemoteIP, USHORT uRemotePort ) = 0;
};


//////////////////////////////////////////////////////////////////////////


extern const __declspec(selectany) GUID CLSID_IFwProxyImp =
{ 0xad3e3a83, 0x2353, 0x46b1, { 0xb4, 0x6a, 0xbf, 0x4d, 0x52, 0xd8, 0x4d, 0x95 } };

[
    uuid(AD3E3A83-2353-46b1-B46A-BF4D52D84D95)
]
interface IFwProxy : IUnknown
{
    virtual STDMETHODIMP Initialize(
        IFwEvent* piEvent
        ) = 0;

    virtual STDMETHODIMP UnInitialize(
        VOID
        ) = 0;
	
	virtual STDMETHODIMP GetKfwState(
		KFW_STATE *kfwState
		) = 0;  

    virtual STDMETHODIMP GetFlux(
        PULONGLONG pInbound, 
        PULONGLONG pOutbound,
		PULONGLONG pInboundLocal, 
		PULONGLONG pOutboundLocal
        ) = 0;    

    virtual STDMETHODIMP EnumConnect(
        PTCP_CONN_INFO pConnInfo, 
        ULONG dwInfoCount, 
        PULONG pdwRetCount
        ) = 0;

	virtual STDMETHODIMP EnumProcessTraffic(
		PROCESS_TRAFFIC* pProcessTraffic, 
		ULONG dwProcessCount, 
		PULONG pdwRetCount
		) = 0;

	virtual STDMETHODIMP EnumProgramTraffic(
		PROGRAM_TRAFFIC* pProgramTraffic, 
		ULONG dwProgramCount, 
		PULONG pdwRetCount
		) = 0;

	virtual STDMETHODIMP EnumProgramTrafficFromCache(
		PROGRAM_TRAFFIC* pProgramTraffic, 
		ULONG dwProgramCount, 
		PULONG pdwRetCount
		) = 0;
	
	virtual STDMETHODIMP EnumProgramTrafficFromFile( 
		PROGRAM_TRAFFIC_FROM_FILE* pProgramTraffic, 
		ULONG dwProgramCount, 
		ULONG uStartItem, 
		PULONG pdwRetCount
		) = 0;
	
	virtual STDMETHODIMP DeleteProgramTrafficFile() = 0;

	virtual STDMETHODIMP SetProgramTrafficTimer( 
		ULONG uTimer 
		) = 0;

    virtual STDMETHODIMP EnableFirewall(
        VOID
        ) = 0;

    virtual STDMETHODIMP DisableFirewall(
        VOID
        ) = 0;

    virtual STDMETHODIMP EnableNetwork(
        VOID
        ) = 0;

    virtual STDMETHODIMP DisableNetwork(
        VOID
        ) = 0;
	
	virtual STDMETHODIMP ClearResultCache(
		VOID 
		) = 0;

	virtual STDMETHODIMP SendResponse(
		ULONGLONG responseContext,
		ULONG dwResult) = 0;

	virtual STDMETHODIMP GetModuleInfo(
		PModuleInfo pModuleInfo,
		ULONG dwCount) = 0;

	virtual STDMETHODIMP SetModuleInfo(
		PModuleInfo pModuleInfo,
		ULONG dwCount) = 0;

	virtual STDMETHODIMP GetModuleList(		
		PModuleInfo pModuleInfo, 
		ULONG dwModuleCount, 
		PULONG pdwRetCount) = 0;

	virtual STDMETHODIMP AddRecorder( 
		PDATA_RECORDER pRecorder,
		ULONG dwCount
		) = 0;

	virtual STDMETHODIMP RemoveRecorder( 
		ULONG dwModlueId
		) = 0;

	virtual STDMETHODIMP ClearRecorder( 
		) = 0;

	virtual STDMETHODIMP_(BOOL) OpenPort( PROTO_TYPE pt, USHORT uPort ) = 0;
	virtual STDMETHODIMP_(BOOL) ClosePort( PROTO_TYPE pt, USHORT uPort ) = 0;
	virtual STDMETHODIMP_(BOOL) GetOpenPort( PVOID pBitVec, ULONG uVecLen ) = 0;

	virtual STDMETHODIMP SetRequestFlag( ULONG uRequestFlag ) = 0;

	virtual STDMETHODIMP SetProcessSpeed( ULONGLONG uPid, ULONG uSendSpeed, ULONG uRecvSpeed , ULONG nDisable) = 0;

	virtual STDMETHODIMP EnableFluxStastic() = 0;
	virtual STDMETHODIMP DisableFluxStastic() = 0;

	virtual STDMETHODIMP ClearUnNormalRunCount() = 0;
	virtual STDMETHODIMP ClearBootState() = 0;
	virtual STDMETHODIMP GetLastBootTime(ULONGLONG& nTime) = 0;
	virtual STDMETHODIMP ClearLastBootTime() = 0;
	virtual STDMETHODIMP AddDumpCount(ULONG nAdd) = 0;
	virtual STDMETHODIMP ClearDumpCount() = 0;
	virtual STDMETHODIMP SetForceDisableTdi(ULONG nValue) = 0;
	virtual STDMETHODIMP SetHasUnCompatibleDriver(ULONG nValue) = 0;
	virtual STDMETHODIMP_(BOOL) TestCanLoad() = 0;
	virtual STDMETHODIMP ClearFlux() = 0;
};


//////////////////////////////////////////////////////////////////////////

extern const __declspec(selectany) GUID CLSID_IFwInstallerImpl = 
{ 0xe2ae96f9, 0x8e3f, 0x43d6, { 0xaf, 0x98, 0x4b, 0xc3, 0x55, 0x9e, 0x65, 0x3 } };

[
    uuid({E2AE96F9-8E3F-43d6-AF98-4BC3559E6503})
]
interface IFwInstaller : IUnknown
{

    virtual STDMETHODIMP Install(
        VOID
        ) = 0;

    virtual STDMETHODIMP UnInstall(
        VOID
        ) = 0;

	virtual STDMETHODIMP SetConfig( 
		DWORD dwEnableFlag, 
		DWORD dwRequestFlag 
		) = 0;

};

//////////////////////////////////////////////////////////////////////////

#endif  // !FWPROXY_PUBLIC_INC_

