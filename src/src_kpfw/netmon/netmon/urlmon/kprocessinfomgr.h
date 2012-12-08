////////////////////////////////////////////////////////////////////////////////
//      
//      File for url moniter service
//      
//      File      : kprocessinfomgr.h
//      Version   : 1.0
//      Comment   : 木马网络防火墙的管理所有的进程安全，并且提供执行线程
//      
//      Create at : 2010-10-8
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "runoptimize/processmon.h"
#include "kurlcachemgr.h"
#include "common/kis_lock.h"
#include <map>
using namespace std;

struct KPocessInfoList;

//////////////////////////////////////////////////////////////////////////
class KProcessInfoCacheMgr
{
public:
	KProcessInfoCacheMgr();
	~KProcessInfoCacheMgr();

	HRESULT		Init(LPCWSTR strCacheFile, LPCWSTR urlMapName);
	HRESULT		UnInit();
	KPocessInfoList* GetProcessInfoList();
	HRESULT		FlushToDisk();

private:
	BOOL	CreateCacheFile();
	BOOL	InitCacheFile(BYTE* pData, DWORD nSize);
	BOOL	InitProcInfoListData(KPocessInfoList* pData, DWORD nSize, BOOL bFirstInit);
	BOOL	ResizeFileToSize(HANDLE hFile, DWORD nSize);

	BOOL	OpenCacheFile();
	BOOL	VerifyCacheFile(HANDLE hFile);
	BOOL	VerifyCacheFile(BYTE* pData, DWORD nFileSize);
	BOOL	VerifyProcInfoListData(KPocessInfoList* pData);

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
class KProcessInfo;
class KModuleMgr;
class KUrlCacheMgr;

class KProcessInfoMgr
{
public:
	KProcessInfoMgr();
	~KProcessInfoMgr();

	HRESULT Init(KModuleMgr* pModuleMgr, KUrlCacheMgr* pUrlMgr);
	HRESULT UnInit();

	KProcessInfo* GetProcessInfo(DWORD nProcessID, ULONGLONG nCreateTime);
	HRESULT	OnProcessCreate(DWORD nProcessID);
	HRESULT	OnProcessDestroy(DWORD nProcessID);
	void	ClearProcessNotNow(ULONGLONG nUpdateCounter);

	// url发送线程
	void	AddToRecycle(KProcessInfo* pInfo);
	HRESULT	RefreshToCache();

private:
	DWORD	ScanProcessInfo();
	DWORD	DoScan();
	void	Clear();

	static DWORD WINAPI ThreadScan(LPVOID lpParameter);

private:
	kis::KLock					m_lock;
	KModuleMgr*					m_pModuleMgr;
	KUrlCacheMgr*				m_pUrlMgr;
	KProcessInfoCacheMgr*		m_pCacheMgr;

	map<DWORD, KProcessInfo*>	m_ProcessList;
	map<DWORD, KProcessInfo*>	m_RycProcessList;

	HANDLE						m_ScanThread;
	BOOL						m_bScanThreadWorking;
	HANDLE						m_hThreadExit;

	BOOL						m_bExit;
};
