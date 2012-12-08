////////////////////////////////////////////////////////////////////////////////
//      
//      File for url moniter service
//      
//      File      : kurlcachemgr.h
//      Version   : 1.0
//      Comment   : 管理木马网络防火墙的cache
//      
//      Create at : 2010-10-8
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#pragma once
//#include "../newblock/urlcachemgr.h"
#include <atlstr.h>
#include <sddl.h>
#include <aclapi.h>
#include "urlmondef.h"
#include <set>
#include <string>
using namespace std;

//////////////////////////////////////////////////////////////////////////
class KUrlMonCfg
{
public:
	KUrlMonCfg();
	~KUrlMonCfg();

	HRESULT		Init(LPCWSTR strCacheFile, LPCWSTR urlMapName);
	HRESULT		UnInit();
	BOOL		AddModuleScanCnt();
	BOOL		GetEnable();
	BOOL		SetEnable(BOOL bEnable);
	BOOL		GetEnableFluxStastic();
	BOOL		SetEnableFluxStastic(BOOL bEnable);
	BOOL		GetDisableDelayScan();
	BOOL		SetDisableDelayScan(BOOL bEnable);

private:
	BOOL	CreateCacheFile();
	BOOL	InitCacheFile(BYTE* pData, DWORD nSize);
	BOOL	InitCfgData(UrlMonCfg* pData);
	BOOL	ResizeFileToSize(HANDLE hFile, DWORD nSize);

	BOOL	OpenCacheFile();
	BOOL	VerifyCacheFile(HANDLE hFile);
	BOOL	VerifyCacheFile(BYTE* pData, DWORD nFileSize);
	BOOL	VerifyCfgData(UrlMonCfg* pData);

	BOOL	InitMapData(BYTE* pData);
	BOOL	SetObjectToLowIntegrity(HANDLE hObject, SE_OBJECT_TYPE type = SE_KERNEL_OBJECT);
	DWORD	GetDataSize();
	UrlMonCfg* GetCfg();

private:
	HANDLE				m_hFile;
	HANDLE				m_hFileMap;

	LPVOID				m_pData;
	ATL::CString		m_strCacheFile;
	LPCWSTR				m_strUrlMapName;
};

//////////////////////////////////////////////////////////////////////////
class KUrlCacheMgr
{
public:
	KUrlCacheMgr();
	~KUrlCacheMgr();

	HRESULT	Init();
	HRESULT UnInit();

	KUrlMonCfg* GetCfg() {return m_pCfg;}

private:
	KUrlMonCfg*			m_pCfg;
};