////////////////////////////////////////////////////////////////////////////////
//      
//      File for url moniter service
//      
//      File      : kmodulemgr.h
//      Version   : 1.0
//      Comment   : 木马网络防火墙的管理所有系统内模块
//      
//      Create at : 2010-10-8
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "common/kis_lock.h"
#include "atlstr.h"
#include <map>
#include <vector>
#include "scan/klitescanservicewrapper.h"
using namespace std;

enum FILE_SCAN_RES
{
	enumKSBW_FSR_GRAY_FILE = 1,		// 是灰文件
	enumKSBW_FSR_WHITE_FILE = 2,
	enumKSBW_FSR_BLACK_FILE = 3,
	enumKSBW_FSR_IGNORE = 4,
	enumKSBW_FSR_NET_ERROR = 5,
	enumKSBW_FSR_MSI_GRAY = 6,
};

// class KSEScanWrapper2;
class KUrlMonCfg;

enum SoftwareClass
{
	SC_Unkown		= 0,
	SC_IE			= 1,
	SC_FLASHGET	= 2,
	SC_XUNLEI		= 3,
	SC_QQDOWN		= 4,
	SC_FIREFOX		= 5,
	SC_TT			= 6,
	SC_MAXTHON		= 7,		// 傲游，马桶
	SC_TheWorld	= 8,		// 世界之窗
	SC_SogouExplorer	= 9,	// 搜狗浏览器
	SC_chrome		 = 10,		// google浏览器
	SC_FlashGetMini = 11,		// 迷你快车
	SC_WebThunder = 12,		// 网页迅雷
	SC_IM_QQ = 13,				// 使用qq传输的文件
	SC_IM_MSN = 14,			// 使用MSN传输的文件
	SC_MINIXUNLEI = 15,		// 迷你迅雷
	SC_IM_TM = 16,
	SC_IM_AliIM = 17,
	SC_Thunder5 = 18,
	SC_XBrowser = 19,
	SC_Thunder7 = 20,
	SC_RayResource = 21,
	SC_Maxthon2 = 22,
	SC_NamiRobot = 23,
	SC_Emule = 24,
	SC_Orbit = 25
};

//////////////////////////////////////////////////////////////////////////
class KModuleInfo
{
public:
	KModuleInfo(LPCWSTR strPath, ULONGLONG nID);
	~KModuleInfo();

	BOOL	SetMd5(BYTE md5[16]);
	ULONGLONG GetID	() {return m_nID;}
	DWORD	GetSecurityState() {return m_nSecurityState;}
	BOOL	IsSafe();
	BOOL	IsUnkown();
	BOOL	IsDanger();
	BOOL	IsUnScan();

	VOID	SetSecurityState(DWORD nSecurity) {m_nSecurityState = nSecurity;}
	LPCWSTR GetPath() {return m_strPath;}
	VOID	SetScanTime(__int64 nTime) {m_timeLastScan = nTime;}
	__int64 GetLastScanTime()		{return m_timeLastScan;}
	BOOL	IsSame(LPCWSTR strPath);
	VOID	AddLoadCnt()		{m_nLoadCnt++;}
	DWORD	GetLoadCnt() {return m_nLoadCnt;}
	VOID	AddScanCnt() {m_nScanCnt++;}
	BOOL	NeedReScan();
	BOOL	CanScanNow(__int64 nTimeNow);
	BOOL	GetModuleStastic(DWORD& nUnkownCnt, DWORD& nDangerCnt, DWORD& NnUnScanCnt);
	BOOL	IsSkipSoftware();
	BOOL	IsChanged();
	BOOL	ResetScan();

	BOOL	GetAutoFlux() {return m_bAutoFlux;}
	VOID	SetAutoFlux(BOOL bFlux) {m_bAutoFlux = bFlux;}
	BOOL	GetFluxLimit(DWORD& nSendLimit, DWORD& nRecvLimit, DWORD& bDisable);
	BOOL	SetFluxLimit(DWORD nSendLimit, DWORD nRecvLimit, DWORD bDisable);
	VOID	SetTestChange(BOOL b) {m_bNeedTestChange = b;}

private:
	DWORD	GetSoftwareClass();

private:
	ULONGLONG			m_nID;
	ATL::CString		m_strPath;
	DWORD				m_nSoftwareClass;
	DWORD				m_nSecurityState;
	DWORD				m_nLoadCnt;
	__int64				m_timeLastScan;
	DWORD				m_nScanCnt;
	LPBYTE				m_pMd5;
	DWORD				m_nFileSize;
	FILETIME			m_FileLastModifyTime;
	BOOL				m_bNeedTestChange;

	DWORD				m_bAutoFlux;			// 是否自动记住限速
	DWORD				m_nSendLimit;			// 发送限速
	DWORD				m_nRecvLimit;			// 接收限速
	DWORD				m_bDisable;
};

//////////////////////////////////////////////////////////////////////////
class KModuleMgr
{
public:
	KModuleMgr();
	~KModuleMgr();

	HRESULT Init();
	HRESULT UnInit();

	HRESULT	AddModule(LPCWSTR strModulePath, ULONGLONG& nModuleID, KModuleInfo*& pInfo);
	KModuleInfo* GetModuleInfo(ULONGLONG& nModuleID);
	BOOL	OnSetAutoFlux();

private:
	void	Clear();
	DWORD	GetModuleHashCode(LPCWSTR strModulePath);
	void	AddModuleToScan(KModuleInfo* pModule);
	HRESULT	ScanModule(KModuleInfo* pModule);
	DWORD	DoScan();
	DWORD	ScanModuleItem();
// 	KSEScanWrapper2* GetScanEngine();
	HRESULT GetScanSession(S_KSE_LITESCAN_SESSION& session);

	BOOL	TestFileChange();
	BOOL	LoadModuleFromCache();
	BOOL	FlushCache();

	static DWORD WINAPI ThreadScan(LPVOID lpParameter);

private:
	kis::KLock						m_lock;
	BOOL							m_bExit;

	map<ULONGLONG, KModuleInfo*>	m_ModuleList;

	HANDLE							m_ScanThread;
	BOOL							m_bScanThreadWorking;
	HANDLE							m_hThreadExit;
	
	// KUrlMonCfg*						m_pCfg;
	// KAutoFluxCacheMgr*				m_pAuotFlux;
	// BOOL							m_bAutoFluxChanged;

	S_KSE_LITESCAN_SESSION			m_hScanSession;
	KLiteScanServiceWrapper			m_scanServiceWrapper;

	// KSEScanWrapper2*				m_pScanEngine;
	vector<KModuleInfo*>			m_ModulesToScan;
};