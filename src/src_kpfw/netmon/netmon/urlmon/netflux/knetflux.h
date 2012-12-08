////////////////////////////////////////////////////////////////////////////////
//      
//      File for net flux
//      
//      File      : knetflux.h
//      Version   : 1.0
//      Comment   : 流量监控
//      
//      Create at : 2010-11-4
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <map>
#include "kpfw/netfluxdef.h"
#include "../kprocessinfo.h"
#include "../kprocessinfomgr.h"
#include "../kdriverproxy.h"
#include "../kurlcachemgr.h"
#include "../kmodulemgr.h"
#include "kfluxstastic.h"

using namespace std;

class KProcessFlux
{
private:
	ULONGLONG				m_nCreateID;
	ULONGLONG				m_nProcessID;
	DWORD					m_nModuleID;
	DWORD					m_nTrustMode;
	DWORD					m_nCanNotKill;

	ULONG					m_nSendLimit;
	ULONG					m_nRecvLimit;
	ULONG					m_bDisable;
	BOOL					m_bAutoFixed;

	ATL::CString			m_strProcessPath;

	DWORD					m_nBufCnt; // 表示buff的长度，由构造函数初始化。
	KPFWFLUX*				m_FluxBuf;
	KPFWFLUX*				m_pCurrentItem;
	ULONGLONG				m_nCurCnt;

public:
	KProcessFlux(PROCESS_TRAFFIC* pProcessInfo, DWORD nBufCnt, DWORD nTrustMode, LPCWSTR path);
	~KProcessFlux();

	HRESULT	UpdateData(PROCESS_TRAFFIC* pProcessInfo, DWORD nTrustMode);
	HRESULT	GetFlux(KPFWFLUX* pFlux, DWORD nTime);
	HRESULT	GetFlux(KProcFluxItem* pItem, DWORD nTime);
	DWORD	GetTrustMode()	{return m_nTrustMode;}
	BOOL	IsSameModule(LPCWSTR strPath);
	ULONGLONG	GetPID () {return m_nProcessID;}
	LPCWSTR GetPath() {return m_strProcessPath;}
	ULONGLONG	GetCreateID() {return m_nCreateID;}
	VOID	SetAutoFixed(BOOL bAuto) {m_bAutoFixed = bAuto;}

private:
	DWORD	GetCanKill(ULONGLONG nProcessID, LPCWSTR strPath);
};

//////////////////////////////////////////////////////////////////////////
class KFluxCacheMgr
{
public:
	KFluxCacheMgr();
	~KFluxCacheMgr();

	HRESULT		Init(LPCWSTR strCacheFile, LPCWSTR urlMapName);
	HRESULT		UnInit();
	KProcessFluxList* GetFluxList();

private:
	BOOL	CreateCacheFile();
	BOOL	InitCacheFile(BYTE* pData, DWORD nSize);
	BOOL	InitFluxListData(KProcessFluxList* pData, DWORD nSize);
	BOOL	ResizeFileToSize(HANDLE hFile, DWORD nSize);

	BOOL	OpenCacheFile();
	BOOL	VerifyCacheFile(HANDLE hFile);
	BOOL	VerifyCacheFile(BYTE* pData, DWORD nFileSize);
	BOOL	VerifyFluxListData(KProcessFluxList* pData);

	BOOL	InitMapData(BYTE* pData, DWORD nSize);
	BOOL	SetObjectToLowIntegrity(HANDLE hObject, SE_OBJECT_TYPE type = SE_KERNEL_OBJECT);
	DWORD	GetDataSize();


private:
	HANDLE				m_hFile;
	HANDLE				m_hFileMap;

	LPVOID				m_pData;
	ATL::CString		m_strCacheFile;
	LPCWSTR				m_strUrlMapName;
};

//////////////////////////////////////////////////////////////////////////
class KProcessDisableNetFluxPopInfo
{
public:
	KProcessDisableNetFluxPopInfo()
	{

	}
	~KProcessDisableNetFluxPopInfo()
	{

	}

	ULONGLONG				m_nCreateID;
	ULONGLONG				m_nProcessID;
	ULONGLONG				m_nModuleID;
	DWORD					m_nTrustMode;
	ATL::CString			m_strProcessPath;
};
//////////////////////////////////////////////////////////////////////////
class KNetFluxMgr
{
public:
	KNetFluxMgr();
	~KNetFluxMgr();

	HRESULT	Init(IFwProxy* pProxy, KProcessInfoMgr* pMgr, KModuleMgr* pModuleMgr, KUrlMonCfg* pCfg);
	HRESULT	Uninit();

	INT		GetInfoCnt();
	HRESULT	UpdateProcessInfo(PROCESS_TRAFFIC* pProcessInfo, INT nCnt, KPFWFLUX& fluxSys);
	void	SetProcessSpeed(FluxSetProcessSpeed* pParam);
	BOOL	GetPopInfo(ProcessDisableNetPopInfo* pPop);
	void	SetProcUnPop(FluxSetProcUnPop* pParam);

private:
	BOOL	GetProccessPath(DWORD nPID, DWORD nModuleID, WCHAR bufPath[MAX_PATH]);
	void	Clear();
	void	RefreshToCache(KPFWFLUX& fluxSys);
	BOOL	CheckDumpFile();
	BOOL	CheckUnNormalRun();
	BOOL	CheckMiniDump();
	BOOL	IsFileCreateLaterThan(ULONGLONG nTime, LPCWSTR strFile);

	DWORD	UpdateFlux();
	DWORD	DoUpdateFlux();
	static DWORD WINAPI ThreadUpdate(LPVOID lpParameter);

private:
	typedef map<ULONGLONG, KProcessFlux*>  ProcFluxColl;
	ProcFluxColl			m_ProcFlux;
	kis::KLock				m_lock;

	KFluxStastic*			m_pFluxStastic;

	IFwProxy*				m_pProxy;
	KProcessInfoMgr*		m_pMgr;
	KFluxCacheMgr*			m_pFluxCache;
	KModuleMgr*				m_pModuleMgr;
	KUrlMonCfg*				m_pCfg;

	HANDLE					m_UpdateThread;
	BOOL					m_bUpdateThreadWorking;
	HANDLE					m_hThreadExit;

	BOOL					m_bEnableFluxStastic;
	__int64					m_nLastDiableTime;
	__int64					m_nDisableTime;

	ULONGLONG				m_nUpdateCounter;

	vector<KProcessDisableNetFluxPopInfo*>	m_PopInfo;
};