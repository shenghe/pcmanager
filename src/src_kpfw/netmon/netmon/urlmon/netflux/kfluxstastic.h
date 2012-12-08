////////////////////////////////////////////////////////////////////////////////
//      
//      File for net flux
//      
//      File      : kfluxstastic.h
//      Version   : 1.0
//      Comment   : 流量统计
//      
//      Create at : 2010-12-30
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "kpfw/netstastic.h"
#include "../kurlcachemgr.h"
#include <map>
#include <vector>
using namespace std;

struct KStasticFluxProcessList;

//////////////////////////////////////////////////////////////////////////
class KStasticFluxCacheMgr
{
public:
	KStasticFluxCacheMgr();
	~KStasticFluxCacheMgr();

	HRESULT		Init(LPCWSTR strCacheFile, LPCWSTR urlMapName);
	HRESULT		UnInit();
	KStasticFluxProcessList* GetStasticFluxList();
	HRESULT		FlushToDisk();

private:
	BOOL	CreateCacheFile();
	BOOL	InitCacheFile(BYTE* pData, DWORD nSize);
	BOOL	InitFluxListData(KStasticFluxProcessList* pData, DWORD nSize, BOOL bFirstInit);
	BOOL	ResizeFileToSize(HANDLE hFile, DWORD nSize);

	BOOL	OpenCacheFile();
	BOOL	VerifyCacheFile(HANDLE hFile);
	BOOL	VerifyCacheFile(BYTE* pData, DWORD nFileSize);
	BOOL	VerifyFluxListData(KStasticFluxProcessList* pData);

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
class KStasticProgramFlux
{
public:
	KStasticProgramFlux();
	~KStasticProgramFlux();

	HRESULT Init(KProcFluxItem* pItem);
	HRESULT Init(KFluxStasticProcItem* pItem);
	BOOL	IsMatch(KProcFluxItem* pItem);
	BOOL	IsMatch(KStasticProgramFlux* pItem);

	HRESULT	UpdateData(KProcFluxItem* pItem);
	HRESULT	GetFlux(KFluxStasticProcItem* pItem);
	LPCWSTR GetProgramePath() {return m_strProgramPath;}

	ATL::CString			m_strProgramPath;
	ULONGLONG				m_nTotalRecv;			// 总计接受的流量
	ULONGLONG				m_nTotalSend;			// 总计发送的流量
};

//////////////////////////////////////////////////////////////////////////
// 流量统计要考虑3种特殊情况：
// 1、系统启动后跨域天的情况
//		对每秒的流量判定它是否属于同一天，如果不属于，那么清除当前的数据，重新开始统计
// 2、系统重启后，于当天再次开启的情况
//		在当前统计信息中保留记录的时间
// 3、服务被重启，卫士退出后又打开的情况
//		观察每秒的流量增量，所以可以在服务启动时，清除一次流量
//		保证每次服务启动后从0开始计算

class KFluxStastic
{
public:
	KFluxStastic();
	~KFluxStastic();

	HRESULT		Init();
	HRESULT		UnInit();
	HRESULT		UpdateFlux(KProcessFluxList* pFluxList, __int64 nDisableTime);

private:
	HRESULT		RefreshToCache(__int64 nTimeNow, __int64 nDisableTime);
	HRESULT		ResetToday(__int64 nTimeNow);
	KStasticProgramFlux* GetStasticFlux(KProcFluxItem* pItem);
	HRESULT		Clear();
	__int64		GetDayLastTime(__int64 nTime);
	__int64		GetDayStartTime(__int64 nTime);

private:
	KStasticFluxCacheMgr*				m_pCacheMgr;

	__int64								m_nTimeLastWatch;
	__int64								m_nTimeStartWatch;
	__int64								m_nTimeTodayStartTime;
	__int64								m_nTimeTodayLastTime;

	vector<KStasticProgramFlux*>		m_todayPrograms;
	map<ULONGLONG, size_t>				m_procToProgramMap;

	ULONGLONG							m_nFluxRecv;
	ULONGLONG							m_nFluxSend;

	// 为了防止在重启的时候，服务不能得到通知就直接退出，使用一个计数器
	// 刷新
	DWORD								m_nFlushCounter; 
};