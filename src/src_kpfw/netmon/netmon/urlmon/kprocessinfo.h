////////////////////////////////////////////////////////////////////////////////
//      
//      File for url moniter service
//      
//      File      : kprocessinfo.h
//      Version   : 1.0
//      Comment   : 木马网络防火墙的管理一个进程内的模块和其他安全信息
//      
//      Create at : 2010-10-8
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "common/kis_lock.h"
#include "runoptimize/processmon.h"
#include <vector>
#include <string>
#include <set>
#include <atlstr.h>
using namespace std;

//////////////////////////////////////////////////////////////////////////
class KModuleInfo;
class KUrlCacheMgr;

class KProcImage
{
public:
	KProcImage(KModuleInfo* pModule, ULONGLONG nLoadTime);
	~KProcImage();
	BOOL				IsSameModule(KModuleInfo* pInfo);
	void				SetCreateTime(ULONGLONG nLoadTime) {m_nLoadTime = nLoadTime;}
	BOOL				IsLaterThan(KProcImage* pImage);
	BOOL				IsSafe();
	BOOL				IsUnkown();
	BOOL				IsDanger();
	BOOL				IsUnScan();
	BOOL				GetModuleStastic(DWORD& nUnkownCnt, DWORD& nDangerCnt, DWORD& NnUnScanCnt);
	ULONGLONG			GetLoadTime() {return m_nLoadTime;}
	LPCWSTR				GetPath();
	DWORD				GetSecurityState();
	BOOL				IsSkipSoftware();
	KModuleInfo*		GetModule() {return m_pModule;}

private:
	KModuleInfo*		m_pModule;
	ULONGLONG			m_nLoadTime;
};

class KProcessInfo
{
public:
	KProcessInfo(ULONGLONG nPID, ULONGLONG nCreateTime, KUrlCacheMgr* pUrlMgr);
	~KProcessInfo();

	HRESULT			Process();
	HRESULT			ProcessStrategy_SendOnlyUnkownModules();
	HRESULT			ProcessStrategy_SendAllUrl();
	BOOL			HasWaitingProcess();

	BOOL			HasExeImage();
	BOOL			SetExeImage(KModuleInfo* pModule);
	KModuleInfo*	GetExeImage();

	BOOL			OnLoadImage(ULONGLONG nLoadTime, KModuleInfo* pModule);

	BOOL			OnUrlSend(ULONGLONG nSendTime, char* pHost, char* pUrl, ULONG nOp, DWORD& nResult);

	ULONGLONG		GetPID() {return m_nPID;}
	ULONGLONG		GetCreateTime() {return m_nCreateTime;}
	BOOL			IsSkipSoftware();

	LPCWSTR			GetProcPath();
	DWORD			GetProcSecurity();
    BOOL            IsSpecialProc();

	void			SetUpdateCounter(ULONGLONG nCounter) {m_nUpdateCounter = nCounter;}
	ULONGLONG		GetUpdateCounter() {return m_nUpdateCounter;}

	HRESULT			GetProcInfo(KProcInfoItem* pItem);
private:
	void			AddImageToList(KProcImage* pImage, ULONGLONG nLoadTime);
	void			ClearModuleList();
	void			GetModuleStastic(DWORD& nUnkownCnt, DWORD& nDangerCnt, DWORD& nUnScanCnt);

private:
	kis::KLock			m_lock;
	ULONGLONG			m_nPID;
	ULONGLONG			m_nCreateTime;
	ULONGLONG			m_nLastImageLoadTime;
	ULONGLONG			m_nLastUrlSendTime;

	KProcImage*			m_pExeImage;
	vector<KProcImage*> m_ModuleList;
	set<string>			m_ShowedList;

	KUrlCacheMgr*		m_pUrlCacheMgr;
    BOOL                m_bFirstCheck;
    BOOL                m_bSpecialProc;

	ULONGLONG			m_nUpdateCounter;
};