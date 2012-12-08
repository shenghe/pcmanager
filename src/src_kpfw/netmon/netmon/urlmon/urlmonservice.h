////////////////////////////////////////////////////////////////////////////////
//      
//      File for url moniter service
//      
//      File      : urlmonservice.h
//      Version   : 1.0
//      Comment   : 木马网络防火墙的的服务
//      
//      Create at : 2010-10-8
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "kws/ipc.h"
#include "kws/urlmondef.h"

class KTdiDriverProxy;
class KProcessInfoMgr;
class KModuleMgr;
class KUrlCacheMgr;
class KNetFluxMgr;
struct FluxSetProcessSpeed;
struct FluxSetProcUnPop;

class KUrlMonService
{
public:
	KUrlMonService();
	~KUrlMonService();

	HRESULT Initialize();
	HRESULT Uninitialize();

private:
	static DWORD WINAPI OnUrlMonIpc(LPVOID pv);
	void	OnUrlMonIpc(UrlMonIpcParam* pParam, IPC_CALL_STUB* pStub);
	void	ClearDumpStateAndStartUp();
	void	SetProcessSpeed(FluxSetProcessSpeed* pParam);
	void	SetProcUnPop(FluxSetProcUnPop* pParam);
	HRESULT _Initialize();
	void	StopUpdate();
	void	StartUpdate();
	DWORD	DoUpdateCfg();
	DWORD	UpdateCfg();
	static DWORD WINAPI ThreadUpdate(LPVOID lpParameter);
	static DWORD WINAPI ThreadInit(LPVOID lpParameter);
private:
	KTdiDriverProxy*			m_pProxy;
	KModuleMgr*					m_pModuleMgr;
	KUrlCacheMgr*				m_pUrlMgr;
	KProcessInfoMgr*			m_pProcessMgr;
	KNetFluxMgr*				m_pFluxMgr;
	IPCServer*					m_pIpcServer;
	static KUrlMonService*		s_UrlMonService;

	HANDLE						m_UpdateThread;
	BOOL						m_bUpdateThreadWorking;
	HANDLE						m_hThreadExit;
};