////////////////////////////////////////////////////////////////////////////////
//      
//      File for net flux
//      
//      File      : knetflux.cpp
//      Version   : 1.0
//      Comment   : 流量监控
//      
//      Create at : 2010-11-4
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "knetflux.h"
#include "common/systool.h"
#include "shlobj.h"
#include "psapi.h"
#include "sysmprocess.h"
#include "common/kis_directory.h"
#include "atltime.h"
#include "Psapi.h"
#include "kpfw/msg_logger.h"


//////////////////////////////////////////////////////////////////////////
KProcessFlux::KProcessFlux(PROCESS_TRAFFIC* pProcessInfo, DWORD nBufCnt, DWORD nTrustMode, LPCWSTR path)
{
	m_nCreateID = pProcessInfo->ulCreateTime.QuadPart;
	m_nProcessID = pProcessInfo->uProcessID;
	m_strProcessPath = path;
	m_nModuleID = pProcessInfo->dwModuleID;
	m_nTrustMode = nTrustMode;
	m_nCanNotKill = !GetCanKill(pProcessInfo->uProcessID, path);
	kws_log(TEXT("find process can kill: %d procid: %d, porc path: %s"), !m_nCanNotKill, pProcessInfo->uProcessID, path);

	m_nBufCnt = nBufCnt;
	m_FluxBuf = new KPFWFLUX[m_nBufCnt];
	memset(m_FluxBuf, 0, sizeof(KPFWFLUX)*m_nBufCnt);

	m_pCurrentItem = m_FluxBuf;
	m_nCurCnt = 1;

	m_bDisable = 0;
	m_nSendLimit = 0;
	m_nRecvLimit = 0;
	m_bAutoFixed = FALSE;

	m_pCurrentItem->nSendSpeed = pProcessInfo->ulTrafficIn.QuadPart;
	m_pCurrentItem->nRecvSpeed = pProcessInfo->ulTrafficOut.QuadPart;
	m_pCurrentItem->nSendSpeedLocal = pProcessInfo->ulTrafficInLocal.QuadPart;
	m_pCurrentItem->nSendSpeedLocal = pProcessInfo->ulTrafficOutLocal.QuadPart;
}

KProcessFlux::~KProcessFlux()
{
	if (m_FluxBuf)
	{
		delete [] m_FluxBuf;
		m_FluxBuf = NULL;
	}
	m_nBufCnt = 0;

	m_pCurrentItem = NULL;
	m_nCurCnt = 0;
}

HRESULT	KProcessFlux::UpdateData(PROCESS_TRAFFIC* pProcessInfo, DWORD nTrustMode)
{
//	ASSERT(m_nCreateID == pProcessInfo->ulCreateTime.QuadPart);
//	ASSERT(m_nProcessID == pProcessInfo->uProcessID);
//	ASSERT(m_pCurrentItem->nTotalRecv <= pProcessInfo->ulTrafficIn.QuadPart);
//	ASSERT(m_pCurrentItem->nTotalSend <= pProcessInfo->ulTrafficOut.QuadPart);

	if ((m_pCurrentItem - m_FluxBuf) >= static_cast<int>((m_nBufCnt - 1)))
		m_pCurrentItem = m_FluxBuf;
	else
		m_pCurrentItem++;

	if (m_nCurCnt < m_nBufCnt)
		m_nCurCnt++;

	m_pCurrentItem->nTotalRecv = pProcessInfo->ulTrafficIn.QuadPart;
	m_pCurrentItem->nTotalSend = pProcessInfo->ulTrafficOut.QuadPart;
	m_pCurrentItem->nTotalRecvLocal = pProcessInfo->ulTrafficInLocal.QuadPart;
	m_pCurrentItem->nTotalSendLocal = pProcessInfo->ulTrafficOutLocal.QuadPart;

	m_nTrustMode = nTrustMode;

	m_bDisable = pProcessInfo->nDisable;
	m_nSendLimit = pProcessInfo->ulSendLimit;
	m_nRecvLimit = pProcessInfo->ulRecvLimit;

	return S_OK;
}

HRESULT	KProcessFlux::GetFlux(KPFWFLUX* pFlux, DWORD nTime)
{
	KPFWFLUX* pItem = m_pCurrentItem + 1 - ( m_nCurCnt > nTime ? (nTime + 1): m_nCurCnt );


	if ( pItem < m_FluxBuf )  
		pItem += m_nBufCnt;

//	ASSERT(m_pCurrentItem->nTotalRecv >= pItem->nTotalRecv);
//	ASSERT(m_pCurrentItem->nTotalSend >= pItem->nTotalSend);

	pFlux->nRecvSpeed = m_pCurrentItem->nTotalRecv - pItem->nTotalRecv;
	pFlux->nSendSpeed = m_pCurrentItem->nTotalSend - pItem->nTotalSend;
	pFlux->nRecvSpeedLocal = m_pCurrentItem->nTotalRecvLocal - pItem->nTotalRecvLocal;
	pFlux->nSendSpeedLocal = m_pCurrentItem->nTotalSendLocal - pItem->nTotalSendLocal;

	pFlux->nTotalRecv = m_pCurrentItem->nTotalRecv;
	pFlux->nTotalSend = m_pCurrentItem->nTotalSend;
	pFlux->nTotalRecvLocal = m_pCurrentItem->nTotalRecvLocal;
	pFlux->nTotalSendLocal = m_pCurrentItem->nTotalSendLocal;

	return S_OK;
}

HRESULT	KProcessFlux::GetFlux(KProcFluxItem* pItem, DWORD nTime)
{
	pItem->m_nCbSize = sizeof(KProcFluxItem);
	pItem->m_nCreateID = m_nCreateID;
	pItem->m_nProcessID = m_nProcessID;
	GetFlux(&pItem->m_nFlux, nTime);
	pItem->m_nTrustMode = m_nTrustMode;
	pItem->m_nCanNotKill = m_nCanNotKill;
	pItem->m_nDisable = m_bDisable;
	pItem->m_nSendLimit = m_nSendLimit;
	pItem->m_nRecvLimit = m_nRecvLimit;
	pItem->m_bAutoFixed = m_bAutoFixed;

	if (m_strProcessPath.GetLength() < MAX_PATH)
		wcscpy_s(pItem->m_strProcPath, MAX_PATH, (LPCWSTR)m_strProcessPath);
	return S_OK;
}

BOOL KProcessFlux::IsSameModule(LPCWSTR strPath)
{
	return m_strProcessPath.CompareNoCase(strPath) == 0;
}

DWORD KProcessFlux::GetCanKill(ULONGLONG nProcessID, LPCWSTR strPath)
{
	if ((nProcessID == 4) || (nProcessID == GetCurrentProcessId()))
		return FALSE;
	if (KSysApps::Instance()->Search(strPath))
		return FALSE;
	ATL::CString str(TEXT("SYSTEM"));
	if (str.CompareNoCase(strPath) == 0)
		return FALSE;
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
KFluxCacheMgr::KFluxCacheMgr(): m_hFile(NULL),
								m_hFileMap(NULL),
								m_pData(NULL),
								m_strUrlMapName(NULL)
{

}

KFluxCacheMgr::~KFluxCacheMgr()
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

HRESULT	KFluxCacheMgr::Init(LPCWSTR strCacheFile, LPCWSTR urlMapName)
{
	m_strCacheFile = strCacheFile;
	m_strUrlMapName = urlMapName;

	if (!OpenCacheFile())
	{
		if (!CreateCacheFile())
		{
			kws_log(TEXT("KFluxCacheMgr::Init try create cache file"));
			return E_FAIL;
		}
		if (!OpenCacheFile())
		{
			kws_log(TEXT("KFluxCacheMgr::Init try open cache file 2 failed"));
			return E_FAIL;
		}
	}
	kws_log(TEXT("KFluxCacheMgr::Init sucess"));
	return S_OK;
}

HRESULT	KFluxCacheMgr::UnInit()
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

DWORD KFluxCacheMgr::GetDataSize()
{
	return 1024*1024;
}

BOOL KFluxCacheMgr::CreateCacheFile()
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

BOOL KFluxCacheMgr::ResizeFileToSize(HANDLE hFile, DWORD nSize)
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

BOOL KFluxCacheMgr::InitCacheFile(BYTE* pData, DWORD nSize)
{
	NetFluxCacheFileHeader* pFileHeader = (NetFluxCacheFileHeader*)pData;
	memset(pFileHeader, 0, sizeof(NetFluxCacheFileHeader));

	pFileHeader->m_nMagic		= NET_FLEX_FILE_MAGIC;
	pFileHeader->m_nFileVersion = NET_FLEXFILE_FILE;

	DWORD nOffsetCur = sizeof(NetFluxCacheFileHeader);
	pFileHeader->m_nDataOffset[NET_FLUX_DATA_CACHE] = nOffsetCur;
	pFileHeader->m_nDataDirs++;
	nSize -= nOffsetCur;

	InitFluxListData((KProcessFluxList*)(pData + nOffsetCur), nSize);
	return TRUE;
}

BOOL KFluxCacheMgr::InitFluxListData(KProcessFluxList* pData, DWORD nSize)
{
	memset(pData, 0, sizeof(KProcessFluxList));
	pData->m_nSize = sizeof(KProcessFluxList);
	pData->m_lock.InitLock();
	pData->m_nCurrentCnt = 0;
	pData->m_nMaxCnt = (nSize - sizeof(KProcessFluxList)) / sizeof(KProcFluxItem);
	return TRUE;
}

BOOL KFluxCacheMgr::OpenCacheFile()
{
	BOOL	bRes = FALSE;
	HANDLE hFile = NULL;

	kws_log(TEXT("KFluxCacheMgr::OpenCacheFile begin"));

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

	kws_log(TEXT("KFluxCacheMgr::OpenCacheFile end sucess"));
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

BOOL KFluxCacheMgr::VerifyCacheFile(HANDLE hFile)
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

BOOL KFluxCacheMgr::VerifyCacheFile(BYTE* pData, DWORD nFileSize)
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

	NetFluxCacheFileHeader* pFileHeader = (NetFluxCacheFileHeader*)pData;
	if (pFileHeader->m_nMagic != NET_FLEX_FILE_MAGIC)
	{
		kws_log(TEXT("file magic is not equal"));
		return FALSE;
	}

	if (pFileHeader->m_nFileVersion != NET_FLEXFILE_FILE)
	{
		kws_log(TEXT("file version error"));
		return FALSE;
	}

	if (nFileSize < sizeof(NetFluxCacheFileHeader))
		return FALSE;

	if (pFileHeader->m_nDataDirs > 20)
	{
		kws_log(TEXT("data dir too many"));
		return FALSE;
	}

	if (pFileHeader->m_nDataDirs >= 1)
	{
		if (!VerifyFluxListData((KProcessFluxList*)(pData + pFileHeader->m_nDataOffset[0])))
			return FALSE;
	}

	return TRUE;
}

BOOL KFluxCacheMgr::VerifyFluxListData(KProcessFluxList* pData)
{
	if (pData->m_nSize != sizeof(KProcessFluxList))
	{
		kws_log(TEXT("flux data size is not equal"));
		return FALSE;
	}
	return TRUE;
}

KProcessFluxList* KFluxCacheMgr::GetFluxList()
{
	if (m_pData)
	{
		return (KProcessFluxList*)((BYTE*)m_pData + ((NetFluxCacheFileHeader*)(m_pData))->m_nDataOffset[NET_FLUX_DATA_CACHE]);
	}
	return NULL;
}

// 初始化map的数据，主要是初始化锁等数据
BOOL KFluxCacheMgr::InitMapData(BYTE* pData, DWORD nSize)
{
	KProcessFluxList* pList = (KProcessFluxList*)((BYTE*)pData + ((NetFluxCacheFileHeader*)(pData))->m_nDataOffset[NET_FLUX_DATA_CACHE]);
	InitFluxListData(pList, nSize - ((NetFluxCacheFileHeader*)(pData))->m_nDataOffset[NET_FLUX_DATA_CACHE]);
	return TRUE;
}

BOOL KFluxCacheMgr::SetObjectToLowIntegrity(HANDLE hObject, SE_OBJECT_TYPE type)
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
KNetFluxMgr::KNetFluxMgr(): m_pProxy(NULL),
							m_pMgr(NULL),
							m_UpdateThread(NULL),
							m_bUpdateThreadWorking(FALSE),
							m_hThreadExit(NULL),
							m_pFluxCache(NULL),
							m_pModuleMgr(NULL),
							m_pCfg(NULL),
							m_bEnableFluxStastic(TRUE),
							m_pFluxStastic(NULL),
							m_nDisableTime(0),
							m_nLastDiableTime(0),
							m_nUpdateCounter(0)
{

}

KNetFluxMgr::~KNetFluxMgr()
{

}

HRESULT	KNetFluxMgr::Init(IFwProxy* pProxy, 
						KProcessInfoMgr* pMgr, 
						KModuleMgr* pModuleMgr,
						KUrlMonCfg* pCfg)
{
	m_pProxy = pProxy;
	m_pMgr = pMgr;
	m_pModuleMgr = pModuleMgr;
	m_pCfg = pCfg;

	if (m_pCfg && m_pProxy)
	{
		m_bEnableFluxStastic = m_pCfg->GetEnableFluxStastic();
		if (!m_bEnableFluxStastic)
			m_pProxy->DisableFluxStastic();

		// 每次启动流量监控服务，自动清除一次流量
		m_pProxy->ClearFlux();
	}

	if (!m_pFluxCache)
	{
		WCHAR bufPath[MAX_PATH] = {0};
		::GetModuleFileName(NULL, bufPath, MAX_PATH);
		::PathRemoveFileSpecW(bufPath);
		::PathAppend(bufPath, TEXT("fluxcache.dat"));

		m_pFluxCache = new KFluxCacheMgr;
		if (FAILED(m_pFluxCache->Init(bufPath, NET_FLUX_MAPDATA_NAME)))
		{
			delete m_pFluxCache;
			m_pFluxCache = NULL;
			kws_log(TEXT("KNetFluxMgr::Init init FluxCache Failed"));
			return E_FAIL;
		}
	}

	if (!m_pFluxStastic)
	{
		m_pFluxStastic = new KFluxStastic;
		if (FAILED(m_pFluxStastic->Init()))
		{
			delete m_pFluxStastic;
			m_pFluxStastic = NULL;
		}
	}

	m_hThreadExit = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_bUpdateThreadWorking = FALSE;
	m_UpdateThread = ::CreateThread(NULL, 0, ThreadUpdate, this, NULL, NULL);
	return S_OK;
}

HRESULT	KNetFluxMgr::Uninit()
{
	if (m_hThreadExit)
		::SetEvent(m_hThreadExit);

	if (m_UpdateThread)
	{
		if (::WaitForSingleObject(m_UpdateThread, 5000) == WAIT_TIMEOUT)
		{
			if (m_bUpdateThreadWorking)
			{
				//::DebugBreak();
				::TerminateThread(m_UpdateThread, 0);
			}
		}
		::CloseHandle(m_UpdateThread);
		m_UpdateThread = NULL;
	}
	if (m_hThreadExit)
	{
		::CloseHandle(m_hThreadExit);
		m_hThreadExit = NULL;
	}

	if (m_pFluxCache)
	{
		m_pFluxCache->UnInit();
		delete m_pFluxCache;
		m_pFluxCache = NULL;
	}

	if (m_pFluxStastic)
	{
		m_pFluxStastic->UnInit();
		delete m_pFluxStastic;
		m_pFluxStastic = NULL;
	}
	Clear();

	vector<KProcessDisableNetFluxPopInfo*>::iterator it_pop = m_PopInfo.begin();
	while (it_pop != m_PopInfo.end())
	{
		delete (*it_pop);
		it_pop++;
	}
	m_PopInfo.clear();
	return S_OK;
}

void KNetFluxMgr::Clear()
{
	ProcFluxColl::iterator it = m_ProcFlux.begin();
	while (it != m_ProcFlux.end())
	{
		delete it->second;
		it++;
	}
	m_ProcFlux.clear();

	
}

void KNetFluxMgr::RefreshToCache(KPFWFLUX& fluxSys)
{
	// 将数据刷新到data cache中
	if (!m_pFluxCache) return;
	KProcessFluxList* pList = m_pFluxCache->GetFluxList();
	if (!pList) return;

	pList->m_lock.LockWrite();
	pList->m_SysFlux.nRecvSpeed = fluxSys.nTotalRecv - pList->m_SysFlux.nTotalRecv;
	pList->m_SysFlux.nSendSpeed = fluxSys.nTotalSend - pList->m_SysFlux.nTotalSend;
	pList->m_SysFlux.nRecvSpeedLocal = fluxSys.nTotalRecvLocal - pList->m_SysFlux.nTotalRecvLocal;
	pList->m_SysFlux.nSendSpeedLocal = fluxSys.nTotalSendLocal - pList->m_SysFlux.nTotalSendLocal;

	pList->m_SysFlux.nTotalRecv = fluxSys.nTotalRecv;
	pList->m_SysFlux.nTotalSend = fluxSys.nTotalSend;
	pList->m_SysFlux.nTotalRecvLocal = fluxSys.nTotalRecvLocal;
	pList->m_SysFlux.nTotalSendLocal = fluxSys.nTotalSendLocal;

	pList->m_nProcessPopCount = (DWORD)m_PopInfo.size();

	KProcFluxItem* pItem = &(pList->m_Items[0]);
	pList->m_nCurrentCnt = m_ProcFlux.size();
	if (pList->m_nCurrentCnt > pList->m_nMaxCnt)
		pList->m_nCurrentCnt = pList->m_nMaxCnt;

	INT nCnt = pList->m_nCurrentCnt;
	
	ProcFluxColl::iterator it = m_ProcFlux.begin();
	while (it != m_ProcFlux.end())
	{
		it->second->GetFlux(pItem, 1);
		it++, pItem++, nCnt--;
		if (nCnt <= 0)
			break;
	}

	pList->m_lock.UnLockWrite();

	if (m_pFluxStastic)
	{
		__int64 nDisableTime = m_nDisableTime;
		if (m_nLastDiableTime != 0)
		{
			if (ATL::CTime::GetCurrentTime().GetTime() > m_nLastDiableTime)
				nDisableTime += ATL::CTime::GetCurrentTime().GetTime() - m_nLastDiableTime;
		}
		m_pFluxStastic->UpdateFlux(pList, nDisableTime);
	}
}

void KNetFluxMgr::SetProcUnPop(FluxSetProcUnPop* pParam)
{
	if (m_pModuleMgr)
	{
		KModuleInfo* pInfo = m_pModuleMgr->GetModuleInfo(pParam->m_nModuleID);
		if (pInfo)
			pInfo->SetUnPop(pParam->m_bUnPop);
	}
}

void KNetFluxMgr::SetProcessSpeed(FluxSetProcessSpeed* pParam)
{
	if (m_pProxy)
	{
		if (!pParam->bRemember)
		{
			kis::KLocker  _lokcer(m_lock);
			ProcFluxColl::iterator it = m_ProcFlux.begin();
			for (; it != m_ProcFlux.end(); it++)
			{
				if (it->second->GetPID() == pParam->nProcessID)
				{
					it->second->SetAutoFixed(FALSE);

					// 如果不勾选记住，那么要去掉勾选
					KProcessInfo* pInfo = m_pMgr->GetProcessInfo(it->second->GetPID(), 
						it->second->GetCreateID());
					if (pInfo && pInfo->GetExeImage())
					{
						pInfo->GetExeImage()->SetAutoFlux(FALSE);
						pInfo->GetExeImage()->SetFluxLimit(0, 0, FALSE, FALSE);
						m_pModuleMgr->OnSetAutoFlux();
					}

					m_pProxy->SetProcessSpeed(pParam->nProcessID, 
						pParam->nSendLimit, pParam->nRecvLimit, pParam->nDisable);
					break;
				}
			}
		}
		else
		{
			// 对所有同模块名的的进程设置speed
			kis::KLocker  _lokcer(m_lock);
			ProcFluxColl::iterator it = m_ProcFlux.begin();
			for (; it != m_ProcFlux.end(); it++)
			{
				if (it->second->GetPID() == pParam->nProcessID)
				{
					LPCWSTR strPath = it->second->GetPath();

					// 保存设置
					KProcessInfo* pInfo = m_pMgr->GetProcessInfo(it->second->GetPID(), 
						it->second->GetCreateID());
					if (pInfo && pInfo->GetExeImage())
					{
						if (pParam->nSendLimit || pParam->nRecvLimit || pParam->nDisable)
						{
							pInfo->GetExeImage()->SetAutoFlux(TRUE);
							pInfo->GetExeImage()->SetFluxLimit(pParam->nSendLimit, 
								pParam->nRecvLimit,
								pParam->nDisable,
								FALSE);
						}
						else
						{
							pInfo->GetExeImage()->SetAutoFlux(FALSE);
							pInfo->GetExeImage()->SetFluxLimit(0, 0, FALSE, FALSE); 
						}

						m_pModuleMgr->OnSetAutoFlux();
					}

					for (it = m_ProcFlux.begin(); it != m_ProcFlux.end(); it++)
					{
						if (it->second->IsSameModule(strPath))
						{
							it->second->SetAutoFixed(TRUE);
							m_pProxy->SetProcessSpeed(it->second->GetPID(), 
								pParam->nSendLimit, pParam->nRecvLimit, pParam->nDisable);
						}
					}
					break;
				}
			}
		}
	}
}

BOOL KNetFluxMgr::GetPopInfo(ProcessDisableNetPopInfo* pPop)
{
	kis::KLocker  _lokcer(m_lock);
	if (m_PopInfo.size() > 0)
	{
		KProcessDisableNetFluxPopInfo* pPopInfo = m_PopInfo[0];
		m_PopInfo.erase(m_PopInfo.begin());
		pPop->m_nCreateID = pPopInfo->m_nCreateID;
		pPop->m_nModuleID = pPopInfo->m_nModuleID;
		pPop->m_nProcessID = pPopInfo->m_nProcessID;
		pPop->m_nTrustMode = pPopInfo->m_nTrustMode;
		wcscpy_s(pPop->m_strProcPath, MAX_PATH, pPopInfo->m_strProcessPath);
		delete pPopInfo;
		return TRUE;
	}
	return FALSE;
}

HRESULT	KNetFluxMgr::UpdateProcessInfo(PROCESS_TRAFFIC* pProcessInfo, INT nCnt, KPFWFLUX& fluxSys)
{
	kis::KLocker  _lokcer(m_lock);
	m_nUpdateCounter++;

	ProcFluxColl  newColl;
	for (int i = 0; i < nCnt; i++, pProcessInfo++)
	{
		KProcessFlux* pFlux = NULL;
		ProcFluxColl::iterator it = m_ProcFlux.find(pProcessInfo->ulCreateTime.QuadPart);
		if (it != m_ProcFlux.end())
		{
			pFlux = it->second;

			DWORD	nTrustMode = 0;
			KProcessInfo* pInfo = m_pMgr->GetProcessInfo(pProcessInfo->uProcessID, pProcessInfo->ulCreateTime.QuadPart);
			if (pInfo)
			{
				pInfo->SetUpdateCounter(m_nUpdateCounter);
				nTrustMode = pInfo->GetProcSecurity();
				if (pInfo->GetExeImage())
					pFlux->SetAutoFixed(pInfo->GetExeImage()->GetAutoFlux());
			}

			pFlux->UpdateData(pProcessInfo, nTrustMode);
			
			newColl.insert(make_pair(pProcessInfo->ulCreateTime.QuadPart, pFlux));
			m_ProcFlux.erase(it);
		}
		else
		{
			// 先不管有没有流量，先将所有的进程的拿出来再说
			LPCWSTR strPath = NULL;
			DWORD	nTrustMode = 0;
			KProcessInfo* pInfo = m_pMgr->GetProcessInfo(pProcessInfo->uProcessID, pProcessInfo->ulCreateTime.QuadPart);
			if (pInfo)
			{
				pInfo->SetUpdateCounter(m_nUpdateCounter);
				strPath = pInfo->GetProcPath();
				nTrustMode = pInfo->GetProcSecurity();
			}
			else
				continue;

			if (!strPath)
			{
				WCHAR path[MAX_PATH + 1] = {0};
				if (GetProccessPath(pProcessInfo->uProcessID, pProcessInfo->dwModuleID, path))
				{
					ULONGLONG nModuleID = 0;
					KModuleInfo* pModule = NULL;
					m_pModuleMgr->AddModule(path, nModuleID, pModule);
					pInfo->SetExeImage(pModule);

					strPath = pInfo->GetProcPath();
					nTrustMode = pInfo->GetProcSecurity();
				}
				else
				{
					kws_log(TEXT("find cannot get process path: %d"), pProcessInfo->uProcessID);
				}
			}

			if ((pProcessInfo->ulTrafficIn.QuadPart > 0) || 
				(pProcessInfo->ulTrafficOut.QuadPart > 0)
				|| (pProcessInfo->uTotalConnectionCnt > 0))
			{
				if (strPath)
				{
					pFlux = new KProcessFlux(pProcessInfo, 30+1, nTrustMode, strPath);
					newColl.insert(make_pair(pProcessInfo->ulCreateTime.QuadPart, pFlux));
				}
			}

			// 自动应用限速设置
			if (m_pProxy && strPath && pInfo &&
				pInfo->GetExeImage() && 
				pInfo->GetExeImage()->GetAutoFlux())
			{
				DWORD nSendLimit = 0, nRecvLimit = 0;
				DWORD bDisable = 0;
				DWORD bUnPop = 0;
				pInfo->GetExeImage()->GetFluxLimit(nSendLimit, nRecvLimit, bDisable, bUnPop);
				if (pFlux)
					pFlux->SetAutoFixed(TRUE);
				else
				{
					if (strPath)
					{
						pFlux = new KProcessFlux(pProcessInfo, 30+1, nTrustMode, strPath);
						newColl.insert(make_pair(pProcessInfo->ulCreateTime.QuadPart, pFlux));
					}
				}

				m_pProxy->SetProcessSpeed(pProcessInfo->uProcessID, 
					nSendLimit, nRecvLimit, bDisable);

				// 如果是自动禁止，那么通知服务弹出泡泡
				if (bDisable && !bUnPop && m_bEnableFluxStastic)
				{
					KProcessDisableNetFluxPopInfo* pPopInfo = new KProcessDisableNetFluxPopInfo;
					pPopInfo->m_nCreateID = pProcessInfo->ulCreateTime.QuadPart;
					pPopInfo->m_nProcessID = pProcessInfo->uProcessID;
					pPopInfo->m_nModuleID = pInfo->GetExeImage()->GetID();
					pPopInfo->m_nTrustMode = nTrustMode;
					pPopInfo->m_strProcessPath = pInfo->GetExeImage()->GetPath();
					m_PopInfo.push_back(pPopInfo);
					kws_log(TEXT("find pop info: disable net: %s"), pPopInfo->m_strProcessPath);
				}
			}


		}
	}

	Clear();

	m_ProcFlux = newColl;
	RefreshToCache(fluxSys);

	m_pMgr->ClearProcessNotNow(m_nUpdateCounter);
	m_pMgr->RefreshToCache();
	return S_OK;
}

BOOL KNetFluxMgr::GetProccessPath(DWORD nPID, DWORD nModuleID, WCHAR bufPath[MAX_PATH])
{
	if (nPID == 0)
	{
		wcscpy_s( bufPath, MAX_PATH, TEXT("System Idle"));
		return TRUE;
	}
	else if ( nPID == 4 || nPID == 8)
	{
		wcscpy_s( bufPath, MAX_PATH, TEXT("System"));
		return TRUE;
	}
	else
	{
		if (m_pProxy)
		{
			ModuleInfo info = {0};
			info.dwModuleID = nModuleID;
			m_pProxy->GetModuleInfo(&info, 1);
			if (wcslen(info.wsModulePath) > 0 && info.wsModulePath[1] == L':')
			{
				wcscpy_s( bufPath, MAX_PATH, info.wsModulePath);
				return TRUE;
			}
		}

		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, nPID);
		if (hProcess)
		{
			WCHAR buf[MAX_PATH] = {0};
			if (GetProcessImageFileNameW(hProcess, buf, MAX_PATH))
			{
				if (KTdiDriverProxy::m_NameMgr.FixDosPathToNormalPath(buf, bufPath))
				{
					CloseHandle(hProcess);
					kws_log(TEXT("get process path 4: %s"), bufPath);
					return TRUE;
				}
			}
			if (GetModuleFileNameEx(hProcess, NULL, bufPath, MAX_PATH))
			{
				CloseHandle(hProcess);
				kws_log(TEXT("get process path 5: %s"), bufPath);
				return TRUE;
			}
            ::CloseHandle(hProcess);
		}
	}
	return FALSE;
}

DWORD	KNetFluxMgr::UpdateFlux()
{
	INT nStart = 20;
	PROCESS_TRAFFIC* pProcessTrafinc = new PROCESS_TRAFFIC[nStart];
	ULONG nCnt = 0;
	KPFWFLUX fluxSys = {0};
	if (!m_pProxy) return 0;

	// 根据配置设置是否允许流量监控统计
	if (m_pCfg && m_pProxy)
	{
		BOOL bEnable = m_pCfg->GetEnableFluxStastic();
		if (bEnable != m_bEnableFluxStastic)
		{
			m_bEnableFluxStastic = bEnable;
			if (!m_bEnableFluxStastic)
			{
				m_pProxy->DisableFluxStastic();
				m_nLastDiableTime = ATL::CTime::GetCurrentTime().GetTime();
			}
			else
			{
				m_pProxy->EnableFluxStastic();
				if (m_nLastDiableTime != 0)
				{
					if (ATL::CTime::GetCurrentTime().GetTime() > m_nLastDiableTime)
						m_nDisableTime += ATL::CTime::GetCurrentTime().GetTime() - m_nLastDiableTime;
					m_nLastDiableTime = 0;
				}
			}
		}
	}

	m_pProxy->GetFlux(&fluxSys.nTotalRecv, &fluxSys.nTotalSend,
		&fluxSys.nTotalRecvLocal, &fluxSys.nTotalSendLocal);

	if (SUCCEEDED(m_pProxy->EnumProcessTraffic(pProcessTrafinc, nStart, &nCnt)))
		UpdateProcessInfo(pProcessTrafinc, nCnt, fluxSys);
	else
	{
		while (::GetLastError() == ERROR_BAD_LENGTH)
		{
			delete[] pProcessTrafinc;
			nStart *= 2;
			pProcessTrafinc = new PROCESS_TRAFFIC[nStart];
			if (SUCCEEDED(m_pProxy->EnumProcessTraffic(pProcessTrafinc, nStart, &nCnt)))
			{
				UpdateProcessInfo(pProcessTrafinc, nCnt, fluxSys);
				break;
			}
		}
	}
	delete[] pProcessTrafinc;
	
	CheckUnNormalRun();
	return 0;
}

DWORD KNetFluxMgr::DoUpdateFlux()
{
	kws_log(TEXT("KNetFluxMgr::DoUpdateFlux start"));
	m_bUpdateThreadWorking = TRUE;
	try
	{
		// CheckDumpFile();

		while (1)
		{
			DWORD nWaitRes = ::WaitForSingleObject(m_hThreadExit, 1000);
			switch (nWaitRes)
			{
			case WAIT_TIMEOUT:
				UpdateFlux();
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
	m_bUpdateThreadWorking = FALSE;
	return 0;
}

DWORD KNetFluxMgr::ThreadUpdate(LPVOID lpParameter)
{
	KNetFluxMgr* pThis = (KNetFluxMgr*)lpParameter;
	if (pThis)
		pThis->DoUpdateFlux();
	return 0;
}

BOOL KNetFluxMgr::CheckDumpFile()
{
	kws_log(TEXT("KNetFluxMgr::CheckDumpFile start"));

	if (!m_pProxy) return FALSE;
	ULONGLONG nTime = 0;
	m_pProxy->GetLastBootTime(nTime);
	if (nTime != 0)
	{
		// 枚举系统的minidump和MEMORY.DMP，找出是否上次崩溃蓝屏
		WCHAR szOSUserPath[MAX_PATH] = {0};
		::GetWindowsDirectory(szOSUserPath, MAX_PATH);

		::PathAppend(szOSUserPath, TEXT("MEMORY.DMP"));
		if (IsFileCreateLaterThan(nTime, szOSUserPath))
		{
			m_pProxy->AddDumpCount(1);
			kws_log(TEXT("clear last boot time"));
			m_pProxy->ClearLastBootTime();
			return TRUE;
		}

		::PathRemoveFileSpec(szOSUserPath);
		::PathAppend(szOSUserPath, TEXT("minidump"));

		kis::KFileIterator iter(TEXT("*.dmp"), szOSUserPath, FALSE, FALSE);
		while (iter.Next())
		{
			if (IsFileCreateLaterThan(nTime, iter.GetFileName()))
			{
				m_pProxy->AddDumpCount(1);
				kws_log(TEXT("clear last boot time"));
				m_pProxy->ClearLastBootTime();
				return TRUE;
			}
		}
		kws_log(TEXT("clear last boot time"));
		m_pProxy->ClearLastBootTime();
	}
	else
	{
		kws_log(TEXT("find last boot time is 0"));
	}
	return TRUE;
}

BOOL KNetFluxMgr::CheckUnNormalRun()
{
	static BOOL bCheck = 0;
	// 检测是否开机5分钟是否ok，如果ok，那么清除 注册表的unnormalrun
	if (!bCheck && GetTickCount() > 1000 * 60 *5  && m_pProxy)
	{
		kws_log(TEXT("KNetFluxMgr::CheckUnNormalRun clear unnormal run"));
		bCheck = TRUE;
		m_pProxy->ClearUnNormalRunCount();
	}
	return TRUE;
}

BOOL KNetFluxMgr::IsFileCreateLaterThan(ULONGLONG nTime, LPCWSTR strFile)
{
	FILETIME t = {0};
	t = *(FILETIME*)(&nTime);
	ATL::CTime nTimeNow(t);
	
	kws_log(TEXT("last boot time is :%d-%d-%d %d:%d:%d checking dump file: %s"), 
		nTimeNow.GetYear(), nTimeNow.GetMonth(), nTimeNow.GetDay(),
		nTimeNow.GetHour(),nTimeNow.GetMinute(), nTimeNow.GetSecond(),
		strFile);
	BOOL bResult = FALSE;
	HANDLE hFile = ::CreateFileW(strFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ((hFile != INVALID_HANDLE_VALUE) && (hFile != NULL))
	{
		FILETIME time = {0};
		::GetFileTime(hFile, NULL, NULL, &time);
		ULARGE_INTEGER inte = {0};
		inte.HighPart = time.dwHighDateTime;
		inte.LowPart = time.dwLowDateTime;
		if (nTime < inte.QuadPart)
		{
			kws_log(TEXT("find a dump file after last boot: %s"), strFile);
			bResult = TRUE;
		}
		else
		{
			ATL::CTime nTimeNow(time);
			kws_log(TEXT("dump file is %s, create time is :%d-%d-%d %d:%d:%d checking dump file: %s"), strFile, 
				nTimeNow.GetYear(), nTimeNow.GetMonth(), nTimeNow.GetDay(),
				nTimeNow.GetHour(),nTimeNow.GetMinute(), nTimeNow.GetSecond());
		}
		::CloseHandle(hFile);
	}
	return bResult;
}
