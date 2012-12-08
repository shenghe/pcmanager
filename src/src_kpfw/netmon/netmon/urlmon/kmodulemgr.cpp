////////////////////////////////////////////////////////////////////////////////
//      
//      File for url moniter service
//      
//      File      : kmodulemgr.cpp
//      Version   : 1.0
//      Comment   : 木马网络防火墙的管理所有系统内模块
//      
//      Create at : 2010-10-8
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "kmodulemgr.h"
#include "common/systool.h"
#include "shlobj.h"
#include "atlconv.h"
#include "atltime.h"
#include "scan/klitescanservicewrapper.h"

#include "kpfw/msg_logger.h"
#define kws_log
#include "kurlcachemgr.h"

//////////////////////////////////////////////////////////////////////////
KAutoFluxCacheMgr::KAutoFluxCacheMgr(): m_hFile(NULL),
										m_hFileMap(NULL),
										m_pData(NULL),
										m_strUrlMapName(NULL)
{

}

KAutoFluxCacheMgr::~KAutoFluxCacheMgr()
{
	if (m_hFileMap)
	{
		::CloseHandle(m_hFileMap);
		m_hFileMap = NULL;
	}
	if (m_hFile)
	{
		::CloseHandle(m_hFile);
		m_hFile = NULL;
	}
}

HRESULT	KAutoFluxCacheMgr::Init(LPCWSTR strCacheFile, LPCWSTR urlMapName)
{
	m_strCacheFile = strCacheFile;
	m_strUrlMapName = urlMapName;

	if (!OpenCacheFile())
	{
		if (!CreateCacheFile())
		{
			kws_log(TEXT("KAutoFluxCacheMgr::Init try create cache file"));
			return E_FAIL;
		}
		if (!OpenCacheFile())
		{
			kws_log(TEXT("KAutoFluxCacheMgr::Init try open cache file 2 failed"));
			return E_FAIL;
		}
	}
	kws_log(TEXT("KAutoFluxCacheMgr::Init sucess"));
	return S_OK;
}

HRESULT	KAutoFluxCacheMgr::UnInit()
{
	if (m_pData && m_hFileMap)
	{
		::UnmapViewOfFile(m_pData);
		m_pData = NULL;
	}
	if (m_hFileMap)
	{
		::CloseHandle(m_hFileMap);
		m_hFileMap = NULL;
	}
	if (m_hFile)
	{
		::CloseHandle(m_hFile);
		m_hFile = NULL;
	}
	return S_OK;
}

DWORD KAutoFluxCacheMgr::GetDataSize()
{
	return 1024*1024;
}

BOOL KAutoFluxCacheMgr::CreateCacheFile()
{
	BOOL	bRes = FALSE;
	HANDLE hFile = NULL;

	DWORD nWriteSize = 0;
	HANDLE hFileMap = NULL;
	KSecurityDesAcessByAnyone secByAny;

	hFile = ::CreateFile(m_strCacheFile,
		GENERIC_READ | GENERIC_WRITE,
		0, //FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("create file error :%d\n", ::GetLastError());
		goto exit0;
	}

	ResizeFileToSize(hFile, GetDataSize());

	hFileMap = ::CreateFileMapping(hFile, NULL, PAGE_READWRITE | SEC_COMMIT, 0, ::GetFileSize(hFile, NULL), NULL);

	if (NULL == hFileMap)
	{
		printf("CreateFileMapping failed: %d\n", ::GetLastError());
		goto exit0;
	}

	BYTE* pMapBuf = (BYTE*)::MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	bRes = InitCacheFile(pMapBuf, GetDataSize());
	UnmapViewOfFile(pMapBuf);
	pMapBuf = NULL;


	bRes = TRUE;
exit0:
	if (hFileMap)
	{
		::CloseHandle(hFileMap);
		hFileMap = NULL;
	}
	if (hFile)
	{
		::CloseHandle(hFile);
		hFile = NULL;
	}
	return bRes;
}

BOOL KAutoFluxCacheMgr::ResizeFileToSize(HANDLE hFile, DWORD nSize)
{
	BYTE	pBuf[1024] = {0};
	memset(pBuf, 0, 1024);

	DWORD nFileSize = ::GetFileSize(hFile, NULL);
	if (nFileSize > nSize)
		return  TRUE;

	DWORD nWriteSize = nSize - nFileSize;
	::SetFilePointer(hFile, 0, NULL, FILE_END);

	for (; nWriteSize > 0; )
	{
		DWORD nWrite = 0;
		if (nWriteSize > 1024)
		{
			if (!::WriteFile(hFile, pBuf, 1024, &nWrite, NULL))
			{
				printf("WriteFile failed: %d\n", ::GetLastError());
				return FALSE;
			}
			nWriteSize -= nWrite;
		}
		else
		{
			if (!::WriteFile(hFile, pBuf, nWriteSize, &nWrite, NULL))
			{
				printf("WriteFile failed: %d\n", ::GetLastError());
				return FALSE;
			}
			nWriteSize -= nWrite;
		}
	}
	return TRUE;
}

BOOL KAutoFluxCacheMgr::InitCacheFile(BYTE* pData, DWORD nSize)
{
	AutoFluxCacheFileHeader* pFileHeader = (AutoFluxCacheFileHeader*)pData;
	memset(pFileHeader, 0, sizeof(AutoFluxCacheFileHeader));

	pFileHeader->m_nMagic		= NET_AUTO_FLEX_FILE_MAGIC;
	pFileHeader->m_nFileVersion = NET_AUTO_FLEX_FILE_VERSIN;

	DWORD nOffsetCur = sizeof(AutoFluxCacheFileHeader);
	pFileHeader->m_nDataOffset[NET_AUTO_FLEX_DATA_CACHE] = nOffsetCur;
	pFileHeader->m_nDataDirs++;
	nSize -= nOffsetCur;

	InitFluxListData((KAutoFluxList*)(pData + nOffsetCur), nSize, TRUE);
	return TRUE;
}

BOOL KAutoFluxCacheMgr::InitFluxListData(KAutoFluxList* pData, DWORD nSize, BOOL bFirstInit)
{
	if (bFirstInit)
		memset(pData, 0, sizeof(KAutoFluxList));

	pData->m_nSize = sizeof(KAutoFluxList);
	pData->m_lock.InitLock();
	pData->m_nMaxCnt = (nSize - sizeof(KAutoFluxList)) / sizeof(KAutoFluxItem);
	return TRUE;
}

BOOL KAutoFluxCacheMgr::OpenCacheFile()
{
	BOOL	bRes = FALSE;
	HANDLE hFile = NULL;

	kws_log(TEXT("KAutoFluxCacheMgr::OpenCacheFile begin"));

	DWORD nWriteSize = 0;
	HANDLE hFileMap = NULL;
	BYTE* pMapBuf = NULL;
	KSecurityDesAcessByAnyone secByAny;

	hFile = ::CreateFile(m_strCacheFile,
		GENERIC_READ | GENERIC_WRITE,
		0, //FILE_SHARE_READ|FILE_SHARE_WRITE, 
		secByAny, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		kws_log(TEXT("create file error :%d: %s"), ::GetLastError(), (LPCWSTR)m_strCacheFile);
		goto exit0;
	}

	if (!VerifyCacheFile(hFile))
	{
		kws_log(TEXT("VerifyCacheFile failed:"));
		goto exit0;
	}

	hFileMap = ::CreateFileMapping(hFile, secByAny, PAGE_READWRITE | SEC_COMMIT, 
		0, ::GetFileSize(hFile, NULL), m_strUrlMapName);

	if (NULL == hFileMap)
	{
		kws_log(TEXT("CreateFileMapping failed: %d"), ::GetLastError());
		goto exit0;
	}

	pMapBuf = (BYTE*)::MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

	m_hFile = hFile;
	m_hFileMap = hFileMap;
	m_pData = pMapBuf;

	SetObjectToLowIntegrity(m_hFile);
	SetObjectToLowIntegrity(m_hFileMap);

	kws_log(TEXT("KAutoFluxCacheMgr::OpenCacheFile end sucess"));
	return TRUE;

exit0:
	if (pMapBuf)
	{
		UnmapViewOfFile(pMapBuf);
		pMapBuf = NULL;
	}
	if (hFileMap)
	{
		::CloseHandle(hFileMap);
		hFileMap = NULL;
	}
	if (hFile)
	{
		::CloseHandle(hFile);
		hFile = NULL;
	}
	return FALSE;
}

BOOL KAutoFluxCacheMgr::VerifyCacheFile(HANDLE hFile)
{
	BOOL bRes = FALSE;
	HANDLE hFileMap = NULL;
	hFileMap = ::CreateFileMapping(hFile, NULL, PAGE_READWRITE | SEC_COMMIT, 0, ::GetFileSize(hFile, NULL), NULL);

	if (NULL == hFileMap)
	{
		kws_log(TEXT("CreateFileMapping failed: %d"), ::GetLastError());
		goto exit0;
	}

	DWORD nFileSize = ::GetFileSize(hFile, NULL);

	BYTE* pMapBuf = (BYTE*)::MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if (!VerifyCacheFile(pMapBuf, nFileSize))
	{
		kws_log(TEXT("VerifyCacheFile failed"));
		goto exit0;
	}

	bRes = InitMapData(pMapBuf, nFileSize);

exit0:
	if (pMapBuf)
	{
		UnmapViewOfFile(pMapBuf);
		pMapBuf = NULL;
	}
	if (hFileMap)
	{
		::CloseHandle(hFileMap);
		hFileMap = NULL;
	}
	return bRes;
}

BOOL KAutoFluxCacheMgr::VerifyCacheFile(BYTE* pData, DWORD nFileSize)
{
	if (nFileSize < GetDataSize())
	{
		kws_log(TEXT("file size smaller than data size"));
		return FALSE;
	}

	if (IsBadReadPtr(pData, nFileSize))
	{
		kws_log(TEXT("map data can not read filesize:%d"), nFileSize);
		return FALSE;
	}

	AutoFluxCacheFileHeader* pFileHeader = (AutoFluxCacheFileHeader*)pData;
	if (pFileHeader->m_nMagic != NET_AUTO_FLEX_FILE_MAGIC)
	{
		kws_log(TEXT("file magic is not equal"));
		return FALSE;
	}

	if (pFileHeader->m_nFileVersion != NET_AUTO_FLEX_FILE_VERSIN)
	{
		kws_log(TEXT("file version error"));
		return FALSE;
	}

	if (nFileSize < sizeof(AutoFluxCacheFileHeader))
		return FALSE;

	if (pFileHeader->m_nDataDirs > 20)
	{
		kws_log(TEXT("data dir too many"));
		return FALSE;
	}

	if (pFileHeader->m_nDataDirs >= 1)
	{
		if (!VerifyFluxListData((KAutoFluxList*)(pData + pFileHeader->m_nDataOffset[0])))
			return FALSE;
	}

	return TRUE;
}

BOOL KAutoFluxCacheMgr::VerifyFluxListData(KAutoFluxList* pData)
{
	if (pData->m_nSize != sizeof(KAutoFluxList))
	{
		kws_log(TEXT("flux data size is not equal"));
		return FALSE;
	}
	return TRUE;
}

KAutoFluxList* KAutoFluxCacheMgr::GetAutoFluxList()
{
	if (m_pData)
	{
		return (KAutoFluxList*)((BYTE*)m_pData + ((AutoFluxCacheFileHeader*)(m_pData))->m_nDataOffset[NET_AUTO_FLEX_DATA_CACHE]);
	}
	return NULL;
}

// 初始化map的数据，主要是初始化锁等数据
BOOL KAutoFluxCacheMgr::InitMapData(BYTE* pData, DWORD nSize)
{
	KAutoFluxList* pList = (KAutoFluxList*)((BYTE*)pData + ((AutoFluxCacheFileHeader*)(pData))->m_nDataOffset[NET_AUTO_FLEX_DATA_CACHE]);
	InitFluxListData(pList, nSize - ((AutoFluxCacheFileHeader*)(pData))->m_nDataOffset[NET_AUTO_FLEX_DATA_CACHE], FALSE);
	return TRUE;
}

BOOL KAutoFluxCacheMgr::SetObjectToLowIntegrity(HANDLE hObject, SE_OBJECT_TYPE type)
{
	bool  bRetCode       = false;
	DWORD dwErr          = ERROR_SUCCESS;
	PACL  pSacl          = NULL;
	BOOL  fSaclPresent   = FALSE;
	BOOL  fSaclDefaulted = FALSE;
	PSECURITY_DESCRIPTOR pSD = NULL;

#define LOW_INTEGRITY_SDDL_SACL     TEXT("S:(ML;;NW;;;LW)")
#ifndef LABEL_SECURITY_INFORMATION
#define LABEL_SECURITY_INFORMATION   (0x00000010L)
#endif

	if (
		ConvertStringSecurityDescriptorToSecurityDescriptor(
		LOW_INTEGRITY_SDDL_SACL, SDDL_REVISION_1, &pSD, NULL)
		)
	{
		if (GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl, &fSaclDefaulted))
		{
			dwErr = SetSecurityInfo( 
				hObject, type, LABEL_SECURITY_INFORMATION,
				NULL, NULL, NULL, pSacl
				);

			bRetCode = (ERROR_SUCCESS == dwErr);
		}

		LocalFree (pSD);
	}

	return bRetCode;
}


//////////////////////////////////////////////////////////////////////////
KModuleInfo::KModuleInfo(LPCWSTR strPath, ULONGLONG nID): m_nID(nID),														  
														  m_nSecurityState(-1),
														  m_timeLastScan(0),
														  m_nLoadCnt(0),
														  m_nScanCnt(0),
														  m_nSoftwareClass(-1),
														  m_bNeedTestChange(FALSE)
{
	m_strPath = strPath;
	m_nFileSize = 0;
	m_FileLastModifyTime.dwHighDateTime = m_FileLastModifyTime.dwLowDateTime = 0;

	m_bAutoFlux = 0;
	m_nSendLimit = 0;
	m_nRecvLimit = 0;
	m_bDisable = 0;

	IsChanged();
}

KModuleInfo::KModuleInfo(KAutoFluxItem* pFlux, ULONGLONG nID):m_nID(nID),															
															m_nSecurityState(0),
															m_timeLastScan(0),
															m_nLoadCnt(0),
															m_nScanCnt(0),
															m_nSoftwareClass(-1)
{
	m_strPath = pFlux->m_strProcPath;
	m_nFileSize = 0;
	m_FileLastModifyTime.dwHighDateTime = m_FileLastModifyTime.dwLowDateTime = 0;

	m_bAutoFlux = 0;
	m_nSendLimit = 0;
	m_nRecvLimit = 0;
	m_bDisable = 0;
	m_bUnPopOnBlock = 0;

	IsChanged();
}

KModuleInfo::~KModuleInfo()
{
	
}

BOOL KModuleInfo::IsSame(LPCWSTR strPath)
{
	return m_strPath.CompareNoCase(strPath) == 0;
}

BOOL KModuleInfo::NeedReScan()
{
	switch (m_nSecurityState)
	{
	case enumKSBW_FSR_BLACK_FILE:
	case enumKSBW_FSR_WHITE_FILE:
	case enumKSBW_FSR_IGNORE:
		break;
	case enumKSBW_FSR_GRAY_FILE:
	case enumKSBW_FSR_NET_ERROR:
		{
			// 如果没有结果，那么最多扫描3次
			if (m_nScanCnt < 3)
				return TRUE;
		}
		break;
	case 0:
		return TRUE;
	}
	return FALSE;
}

BOOL KModuleInfo::CanScanNow(__int64 nTimeNow)
{
	// 看是否到了需要扫描的时候，基本间隔2分钟，每两分钟扫描一次，一共3次
	static CTimeSpan s_spanbase(0, 0, 2, 0);
	if ((m_timeLastScan == 0) || (m_nScanCnt == 0))
		return TRUE;

	if ((nTimeNow - m_timeLastScan) > s_spanbase.GetTotalSeconds() * m_nScanCnt)
		return TRUE;

	return FALSE;
}

BOOL KModuleInfo::IsSafe()
{
	return (m_nSecurityState == enumKSBW_FSR_WHITE_FILE || 
		m_nSecurityState == enumKSBW_FSR_IGNORE);
}

BOOL KModuleInfo::IsUnkown()
{
	return (m_nSecurityState == enumKSBW_FSR_GRAY_FILE || 
		m_nSecurityState == enumKSBW_FSR_NET_ERROR);
}
BOOL KModuleInfo::IsDanger()
{
	return m_nSecurityState == enumKSBW_FSR_BLACK_FILE;
}
BOOL KModuleInfo::IsUnScan()
{
	return m_nSecurityState == 0;
}

BOOL KModuleInfo::GetModuleStastic(DWORD& nUnkownCnt, DWORD& nDangerCnt, DWORD& nUnScanCnt)
{
	switch (m_nSecurityState)
	{
	case enumKSBW_FSR_GRAY_FILE:
	case enumKSBW_FSR_NET_ERROR:
		nUnkownCnt++;
		break;
	case enumKSBW_FSR_BLACK_FILE:
		nDangerCnt++;
		break;
	case 0:
		nUnScanCnt++;
		break;
	}
	return TRUE;
}

BOOL KModuleInfo::IsSkipSoftware()
{
	if (m_nSoftwareClass == -1)
		m_nSoftwareClass = GetSoftwareClass();
	return m_nSoftwareClass != 0;
}

DWORD KModuleInfo::GetSoftwareClass()
{
	LPWSTR strFileName = ::PathFindFileNameW(m_strPath);
	if (!strFileName || (strFileName == (LPCWSTR)m_strPath))	return SC_Unkown;

	if (!_wcsicmp(strFileName, L"iexplore.exe") || 
		!_wcsicmp(strFileName, L"ieuser.exe"))
		return SC_IE;
	if (!_wcsicmp(strFileName, L"Flashget3.exe"))
		return SC_FLASHGET;
	if	(!_wcsicmp(strFileName, L"QQDownload.exe"))
		return SC_QQDOWN;
	if	(!_wcsicmp(strFileName, L"ThunderService.exe"))
		return SC_XUNLEI;
	if	(!_wcsicmp(strFileName, L"firefox.exe"))
		return SC_FIREFOX;
	if	(!_wcsicmp(strFileName, L"TTraveler.exe"))
		return SC_TT;
	if	(!_wcsicmp(strFileName, L"Maxthon.exe") ||
		!_wcsicmp(strFileName, L"MxDownloader.exe"))
		return SC_MAXTHON;
	if	(!_wcsicmp(strFileName, L"TheWorld.exe"))
		return SC_TheWorld;
	if	(!_wcsicmp(strFileName, L"SogouExplorer.exe"))
		return SC_SogouExplorer;
	if	(!_wcsicmp(strFileName, L"chrome.exe"))
		return SC_chrome;
	if	(!_wcsicmp(strFileName, L"FlashGetMini.exe"))
		return SC_FlashGetMini;
	if	(!_wcsicmp(strFileName, L"WebThunder.exe"))
		return SC_WebThunder;
	if	(!_wcsicmp(strFileName, L"QQ.exe"))
		return SC_IM_QQ;
	if	(!_wcsicmp(strFileName, L"msnmsgr.exe"))
		return SC_IM_MSN;
	if	(!_wcsicmp(strFileName, L"TM.exe"))
		return SC_IM_TM;
	if	(!_wcsicmp(strFileName, L"AliIM.exe"))
		return SC_IM_AliIM;
	if	(!_wcsicmp(strFileName, L"XBrowser.exe"))
		return SC_XBrowser;
	if	(!_wcsicmp(strFileName, L"ThunderPlatform.exe"))
		return SC_Thunder7;
	if	(!_wcsicmp(strFileName, L"peer.exe"))
		return SC_RayResource;
	if	(!_wcsicmp(strFileName, L"MxDownloadServer.exe"))
		return SC_Maxthon2;
	if	(!_wcsicmp(strFileName, L"DUTool.exe"))
		return SC_NamiRobot;
	if	(!_wcsicmp(strFileName, L"emule.exe"))
		return SC_Emule;
	if	(!_wcsicmp(strFileName, L"orbitdm.exe"))
		return SC_Orbit;
	if	(!_wcsicmp(strFileName, L"Thunder5.exe") ||
		!_wcsicmp(strFileName, L"Thunder.exe"))
		return SC_Thunder5;

	return SC_Unkown;
}

BOOL KModuleInfo::IsChanged()
{
	if (!m_bNeedTestChange) return FALSE;

	BOOL bResult = FALSE;
	DWORD nFileSize = 0;
	FILETIME fileLastModify = {0};

	HANDLE hFile = CreateFile(m_strPath, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile && (hFile != INVALID_HANDLE_VALUE))
	{
		nFileSize = ::GetFileSize(hFile, NULL);
		::GetFileTime(hFile, NULL, NULL, &fileLastModify);

		if (m_nFileSize == 0 || m_FileLastModifyTime.dwLowDateTime == 0)
		{
			m_nFileSize = nFileSize;
			m_FileLastModifyTime = fileLastModify;
		}
		else
		{
			if ((m_nFileSize != nFileSize) || 
				(m_FileLastModifyTime.dwLowDateTime != fileLastModify.dwLowDateTime) || 
				(m_FileLastModifyTime.dwHighDateTime != fileLastModify.dwHighDateTime))
			{
				m_nFileSize = nFileSize;
				m_FileLastModifyTime = fileLastModify;

				bResult = TRUE;
			}

			// 每次进行test，最多测试一次
			m_bNeedTestChange = FALSE;
		}
		::CloseHandle(hFile);
	}
	return bResult;
}

BOOL KModuleInfo::ResetScan()
{
	m_nSecurityState = 0;
	m_nScanCnt = 0;
	
	return TRUE;
}

BOOL KModuleInfo::GetFluxLimit(DWORD& nSendLimit, DWORD& nRecvLimit, DWORD& bDisable, DWORD& bUnPop)
{
	nSendLimit = m_nSendLimit;
	nRecvLimit = m_nRecvLimit;
	bDisable = m_bDisable;
	bUnPop = m_bUnPopOnBlock;
	return TRUE;
}

BOOL KModuleInfo::SetFluxLimit(DWORD nSendLimit, DWORD nRecvLimit, DWORD bDisable, DWORD bUnPop)
{
	m_nSendLimit = nSendLimit;
	m_nRecvLimit = nRecvLimit;
	m_bDisable = bDisable;
	m_bUnPopOnBlock = bUnPop;
	return TRUE;
}

BOOL KModuleInfo::UpdateAutoFlux(KAutoFluxItem* pItem)
{
	if (m_bAutoFlux)
	{
		pItem->m_nCbSize = sizeof(KAutoFluxItem);
		pItem->m_bAutoFlux = m_bAutoFlux;
		pItem->m_nRecvLimit = m_nRecvLimit;
		pItem->m_nSendLimit = m_nSendLimit;
		pItem->m_bDisable = m_bDisable;
		pItem->m_bUnPopOnBlock = m_bUnPopOnBlock;

		if (m_strPath.GetLength() < MAX_PATH)
			wcscpy_s(pItem->m_strProcPath, MAX_PATH, (LPCWSTR)m_strPath);
		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
KModuleMgr::KModuleMgr(): m_ScanThread(NULL),
							m_bScanThreadWorking(FALSE),
							m_hThreadExit(NULL),							
							m_bExit(FALSE),
							m_pCfg(NULL),
							m_pAuotFlux(NULL),
							m_bAutoFluxChanged(FALSE)
{

}
KModuleMgr::~KModuleMgr()
{

}
KLiteScanServiceWrapper* KModuleMgr::GetScanEngine()
{
	if (!m_liteScanServiceWrapper.IsInitialized())
	{
		int nRet = m_liteScanServiceWrapper.Initialize();
		if (nRet != 0)
		{
			return NULL;
		}

		nRet = m_liteScanServiceWrapper.CreateScanSession(
			KXE_NULL_PARAEMETER(),
			m_scanSession
			);
		if (nRet != 0)
		{
			return NULL;
		}
	}

	return &m_liteScanServiceWrapper;
}

HRESULT KModuleMgr::Init(KUrlMonCfg* pCfg)
{
	m_pCfg = pCfg;
	if (!m_pAuotFlux)
	{
		WCHAR bufPath[MAX_PATH] = {0};
		::GetModuleFileName(NULL, bufPath, MAX_PATH);
		::PathRemoveFileSpecW(bufPath);
		::PathAppend(bufPath, TEXT("autoflux.dat"));

		m_pAuotFlux = new KAutoFluxCacheMgr;
		if (FAILED(m_pAuotFlux->Init(bufPath, NET_AUTO_FLEX_MAPDATA_NAME)))
		{
			delete m_pAuotFlux;
			m_pAuotFlux = NULL;
			kws_log(TEXT("KModuleMgr::Init init KAutoFluxCacheMgr Failed"));
			return E_FAIL;
		}
	}
	LoadModuleFromCache();

	m_hThreadExit = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_bScanThreadWorking = FALSE;
	m_ScanThread = ::CreateThread(NULL, 0, ThreadScan, this, NULL, NULL);
	return S_OK;
}

HRESULT KModuleMgr::UnInit()
{
	// 先停止扫描线程，再清除模块信息
	m_bExit = TRUE;

	if (m_hThreadExit)
		::SetEvent(m_hThreadExit);

	if (m_ScanThread)
	{
		if (::WaitForSingleObject(m_ScanThread, 5000) == WAIT_TIMEOUT)
		{
			if (m_bScanThreadWorking)
			{
				//::DebugBreak();
				::TerminateThread(m_ScanThread, 0);
			}
		}
		::CloseHandle(m_ScanThread);
		m_ScanThread = NULL;
	}
	if (m_hThreadExit)
	{
		::CloseHandle(m_hThreadExit);
		m_hThreadExit = NULL;
	}

	if (m_liteScanServiceWrapper.IsInitialized())
	{
		m_liteScanServiceWrapper.Uninitialize();
	}

	if (m_pAuotFlux)
	{
		m_pAuotFlux->UnInit();
		delete m_pAuotFlux;
		m_pAuotFlux = NULL;
	}

	Clear();
	return S_OK;
}

void KModuleMgr::Clear()
{
	kis::KLocker _locker(m_lock);
	map<ULONGLONG, KModuleInfo*>::iterator it = m_ModuleList.begin();
	for(; it != m_ModuleList.end(); it++)
		delete it->second;
	m_ModuleList.clear();
}

BOOL KModuleMgr::LoadModuleFromCache()
{
	if (!m_pAuotFlux || !m_pAuotFlux->GetAutoFluxList())
		return FALSE;

	KAutoFluxList* pList = m_pAuotFlux->GetAutoFluxList();
	pList->m_lock.LockRead();

	for (DWORD i = 0; i < pList->m_nCurrentCnt; i++)
	{
		KModuleInfo* pModuleInfo = NULL;
		ULONGLONG nID = 0;
		AddModule(pList->m_Items[i].m_strProcPath, nID, pModuleInfo);
		if (pModuleInfo)
		{
			pModuleInfo->SetAutoFlux(pList->m_Items[i].m_bAutoFlux);
			pModuleInfo->SetFluxLimit(pList->m_Items[i].m_nSendLimit,
				pList->m_Items[i].m_nRecvLimit,
				pList->m_Items[i].m_bDisable,
				pList->m_Items[i].m_bUnPopOnBlock);
		}
	}

	pList->m_lock.UnLockRead();
	return TRUE;
}

BOOL KModuleMgr::OnSetAutoFlux()
{
	m_bAutoFluxChanged = TRUE;
	return TRUE;
}

BOOL KModuleMgr::FlushCache()
{
	if (m_bAutoFluxChanged)
	{
		kis::KLocker _locker(m_lock);

		m_bAutoFluxChanged = FALSE;

		if (!m_pAuotFlux) return E_FAIL;
		KAutoFluxList* pList = m_pAuotFlux->GetAutoFluxList();
		if (!pList) return E_FAIL;

		pList->m_lock.LockWrite();
		
		KAutoFluxItem* pItem = &(pList->m_Items[0]);
		INT nCnt = pList->m_nMaxCnt;
		size_t i = 0;
		map<ULONGLONG, KModuleInfo*>::iterator it = m_ModuleList.begin();
		INT nItemCount = 0;
		for (i = 0 ; it != m_ModuleList.end(); i++, it++)
		{
			KModuleInfo* pInfo = it->second;
			if (pInfo->UpdateAutoFlux(pItem))
			{
				nItemCount++;
				pItem++;
				nCnt--;
				if (nCnt <= 0)
					break;
			}
		}

		pList->m_nCurrentCnt = nItemCount;
		pList->m_lock.UnLockWrite();
	}
	return TRUE;
}

void KModuleMgr::AddModuleToScan(KModuleInfo* pModule)
{
	m_ModulesToScan.push_back(pModule);
}

DWORD KModuleMgr::ScanModuleItem()
{
	// 推迟到系统开机5分钟以后才开始扫描
	if (m_pCfg && !m_pCfg->GetDisableDelayScan())
	{
		if (GetTickCount() < 5 * 60 * 1000)
			return 0;
	}

	vector<KModuleInfo*> ModulesToScan;
	vector<KModuleInfo*> ModulesToScanNext;
	if (!m_ModulesToScan.empty())
	{
		kis::KLocker _locker(m_lock);
		ModulesToScan = m_ModulesToScan;
		m_ModulesToScan.clear();
	}
	__int64 nTimeNow = ATL::CTime::GetCurrentTime().GetTime();

	for (size_t i = 0; i < ModulesToScan.size(); i++)
	{
		if (m_bExit) break;
		KModuleInfo* pInfo = ModulesToScan[i];
		if (pInfo->CanScanNow(nTimeNow))
		{
			ScanModule(pInfo);
			if (pInfo->NeedReScan())
				ModulesToScanNext.push_back(pInfo);
		}
		else
		{
			ModulesToScanNext.push_back(pInfo);
		}
	}
	
	// 将需要下次扫描的文件放入扫描列表中
	if (!ModulesToScanNext.empty())
	{
		kis::KLocker _locker(m_lock);
		for (size_t i = 0; i < ModulesToScanNext.size(); i++)
			AddModuleToScan(ModulesToScanNext[i]);
	}
	return 0;
}

HRESULT KModuleMgr::ScanModule(KModuleInfo* pModule)
{
	HRESULT hr = S_OK;

	kws_log(TEXT("KModuleMgr::ScanModule file: %s"), pModule->GetPath());	

	unsigned int nStatus = 0;
	unsigned int nVirusId = 0;

	kws_log(TEXT("KModuleMgr::ScanModule ScanFile start "));
	pModule->SetScanTime(ATL::CTime::GetCurrentTime().GetTime());
	pModule->AddScanCnt();

	KLiteScanServiceWrapper* pScanService = GetScanEngine();
	if (pScanService != NULL)
	{
		S_KSE_LITESCAN_TARGET_INFO info;
		info.vecScanFiles.push_back(pModule->GetPath());
		std::vector<S_KSE_LITESCAN_RESULT> vecResult;
		int nRet = pScanService->ScanFiles(
			m_scanSession,
			info,
			vecResult
			);
		if (nRet == 0 && !vecResult.empty()) // 扫描成功，并且结果不为空
		{
			if (m_pCfg)
				m_pCfg->AddModuleScanCnt();

			switch (vecResult[0].emleval)
			{
			case EM_KSE_SECURITY_LEVAL_VIRUS:
				{
					pModule->SetSecurityState(enumKSBW_FSR_BLACK_FILE);
					if (vecResult[0].strVirusName.size() > 0)
					{
						kws_log(TEXT("KModuleMgr::ScanModule find virus: %s"), vecResult[0].strVirusName.c_str());
					}
				}
				break;
			case EM_KSE_SECURITY_LEVAL_SAFE:
				{
					pModule->SetSecurityState(enumKSBW_FSR_WHITE_FILE);
				}
				break;
			case EM_KSE_SECURITY_LEVAL_NONE:
				{
					pModule->SetSecurityState(enumKSBW_FSR_IGNORE);
				}
				break;
			case EM_KSE_SECURITY_LEVAL_GRAY:
			case EM_KSE_SECURITY_LEVAL_GRAYPE:
				{
					pModule->SetSecurityState(enumKSBW_FSR_GRAY_FILE);
				}
				break;		
			}
		}
		else
		{
			pModule->SetSecurityState(enumKSBW_FSR_NET_ERROR);
			kws_log(TEXT("KModuleMgr::ScanModule Sacn failed: %d"), nRet);
		}
	}
	else
	{
		pModule->SetSecurityState(enumKSBW_FSR_NET_ERROR);
		kws_log(TEXT("KModuleMgr::ScanModule GetScanEngine failed"));
		return E_FAIL;
	}	

	kws_log(TEXT("KModuleMgr::ScanModule ScanFile end \n"));
	return S_OK;
}

BOOL KModuleMgr::TestFileChange()
{
	map<ULONGLONG, KModuleInfo*> curModuleList;
	if (!m_ModuleList.empty())
	{
		kis::KLocker _locker(m_lock);
		curModuleList = m_ModuleList;
	}
	map<ULONGLONG, KModuleInfo*>::iterator it = curModuleList.begin();
	for (; it !=  curModuleList.end(); it++)
	{
		if (it->second->IsChanged())
		{
			it->second->SetTestChange(FALSE);
			kis::KLocker _locker(m_lock);
			it->second->ResetScan();
			AddModuleToScan(it->second);
		}
	}
	return TRUE;
}

DWORD KModuleMgr::DoScan()
{
	m_bScanThreadWorking = TRUE;
	INT nCount = 0;
	try
	{
		while (1)
		{
			DWORD nWaitRes = ::WaitForSingleObject(m_hThreadExit, 500);
			switch (nWaitRes)
			{
			case WAIT_TIMEOUT:
				{
					// 每5秒扫描一次，看看文件是否变化
					if (((++nCount) % 10) == 0)
						TestFileChange();
					ScanModuleItem();
					if (m_bAutoFluxChanged)
						FlushCache();
				}
				break;
			case WAIT_OBJECT_0:
				goto exit0;
				break;
			default:
				goto exit0;
				break;
			}
		}
	}
	catch (...)
	{

	}

exit0:
	m_bScanThreadWorking = FALSE;
	return 0;
}

DWORD KModuleMgr::ThreadScan(LPVOID lpParameter)
{
	KModuleMgr* pThis = (KModuleMgr*)lpParameter;
	if (pThis)
		pThis->DoScan();
	return 0;
}

HRESULT	KModuleMgr::AddModule(LPCWSTR strModulePath, ULONGLONG& nModuleID, KModuleInfo*& pInfoRes)
{
	if (m_bExit) return E_FAIL;
	kis::KLocker _locker(m_lock);

	ULARGE_INTEGER id = {0};
	id.LowPart = GetModuleHashCode(strModulePath);
	id.HighPart = 0;

	while (m_ModuleList.find(id.QuadPart) != m_ModuleList.end())
	{
		KModuleInfo* pInfoItem = m_ModuleList[id.QuadPart];
		if (pInfoItem && pInfoItem->IsSame(strModulePath))
		{
			pInfoItem->AddLoadCnt();
			nModuleID = pInfoItem->GetID();
			pInfoRes = pInfoItem;
			return S_OK;
		}
		id.HighPart++;
	}
	
	KModuleInfo* pInfoItem = new KModuleInfo(strModulePath, id.QuadPart);
	if (!pInfoItem)
		return E_FAIL;

	m_ModuleList[id.QuadPart] = pInfoItem;
	nModuleID = pInfoItem->GetID();
	pInfoRes = pInfoItem;

	// 新发现的模块放入文件列表中
	AddModuleToScan(pInfoRes);
	return S_OK;
}

KModuleInfo* KModuleMgr::GetModuleInfo(ULONGLONG& nModuleID)
{
	if (m_ModuleList.find(nModuleID) == m_ModuleList.end())
		return NULL;
	return m_ModuleList[nModuleID];
}

DWORD KModuleMgr::GetModuleHashCode(LPCWSTR strModulePath)
{
	static UCHAR gsToUpperTable[256 + 4] =
	{
		0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
		64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
		80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
		96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
		80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,123,124,125,126,127,
		128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
		144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
		160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
		176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
		192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
		208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
		224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
		240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
		0, 0, 0, 0
	};
	
	DWORD ulLoopCount = 0;
	DWORD ulResult = 0;

	if ( strModulePath == NULL)
		return 0;

	while( strModulePath[ulLoopCount] )
	{
		ulResult = (ulResult * 31) +  gsToUpperTable[strModulePath[ulLoopCount] >> 8];
		ulResult = (ulResult * 31) +  gsToUpperTable[strModulePath[ulLoopCount] & 0x00ff];
		ulLoopCount++;
	}

	return ulResult;
}