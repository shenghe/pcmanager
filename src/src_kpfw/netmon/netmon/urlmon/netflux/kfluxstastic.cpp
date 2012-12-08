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
#include "stdafx.h"
#include "kfluxstastic.h"
#include "common/systool.h"
#include "shlobj.h"
#include "atltime.h"
#include "kpfw/msg_logger.h"

//////////////////////////////////////////////////////////////////////////
KStasticFluxCacheMgr::KStasticFluxCacheMgr(): m_hFile(NULL),
								m_hFileMap(NULL),
								m_pData(NULL),
								m_strUrlMapName(NULL)
{

}

KStasticFluxCacheMgr::~KStasticFluxCacheMgr()
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

HRESULT	KStasticFluxCacheMgr::Init(LPCWSTR strCacheFile, LPCWSTR urlMapName)
{
	m_strCacheFile = strCacheFile;
	m_strUrlMapName = urlMapName;

	if (!OpenCacheFile())
	{
		if (!CreateCacheFile())
		{
			kws_log(TEXT("KStasticFluxCacheMgr::Init try create cache file"));
			return E_FAIL;
		}
		if (!OpenCacheFile())
		{
			kws_log(TEXT("KStasticFluxCacheMgr::Init try open cache file 2 failed"));
			return E_FAIL;
		}
	}
	kws_log(TEXT("KStasticFluxCacheMgr::Init sucess"));
	return S_OK;
}

HRESULT	KStasticFluxCacheMgr::UnInit()
{
	kws_log(TEXT("KStasticFluxCacheMgr::UnInit"));
	FlushToDisk();
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

DWORD KStasticFluxCacheMgr::GetDataSize()
{
	return 1024*1024;
}

BOOL KStasticFluxCacheMgr::CreateCacheFile()
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

BOOL KStasticFluxCacheMgr::ResizeFileToSize(HANDLE hFile, DWORD nSize)
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

BOOL KStasticFluxCacheMgr::InitCacheFile(BYTE* pData, DWORD nSize)
{
	NetFluxStasticCacheFileHeader* pFileHeader = (NetFluxStasticCacheFileHeader*)pData;
	memset(pFileHeader, 0, sizeof(NetFluxStasticCacheFileHeader));

	pFileHeader->m_nMagic		= NET_FLEX_STASTIC_FILE_MAGIC;
	pFileHeader->m_nFileVersion = NET_FLEX_STASTIC_FILE_FILE;

	DWORD nOffsetCur = sizeof(NetFluxStasticCacheFileHeader);
	pFileHeader->m_nDataOffset[NET_FLUX_STASTIC_DATA_CACHE] = nOffsetCur;
	pFileHeader->m_nDataDirs++;
	nSize -= nOffsetCur;

	InitFluxListData((KStasticFluxProcessList*)(pData + nOffsetCur), nSize, TRUE);
	return TRUE;
}

BOOL KStasticFluxCacheMgr::InitFluxListData(KStasticFluxProcessList* pData, DWORD nSize, BOOL bFirstInit)
{
	if (bFirstInit)
		memset(pData, 0, sizeof(KStasticFluxProcessList));

	pData->m_nSize = sizeof(KStasticFluxProcessList);
	pData->m_lock.InitLock();
	pData->m_nMaxCnt = (nSize - sizeof(KStasticFluxProcessList)) / sizeof(KFluxStasticProcItem);
	return TRUE;
}

BOOL KStasticFluxCacheMgr::OpenCacheFile()
{
	BOOL	bRes = FALSE;
	HANDLE hFile = NULL;

	kws_log(TEXT("KStasticFluxCacheMgr::OpenCacheFile begin"));

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

	kws_log(TEXT("KStasticFluxCacheMgr::OpenCacheFile end sucess"));
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

BOOL KStasticFluxCacheMgr::VerifyCacheFile(HANDLE hFile)
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

BOOL KStasticFluxCacheMgr::VerifyCacheFile(BYTE* pData, DWORD nFileSize)
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

	NetFluxStasticCacheFileHeader* pFileHeader = (NetFluxStasticCacheFileHeader*)pData;
	if (pFileHeader->m_nMagic != NET_FLEX_STASTIC_FILE_MAGIC)
	{
		kws_log(TEXT("file magic is not equal"));
		return FALSE;
	}

	if (pFileHeader->m_nFileVersion != NET_FLEX_STASTIC_FILE_FILE)
	{
		kws_log(TEXT("file version error"));
		return FALSE;
	}

	if (nFileSize < sizeof(NetFluxStasticCacheFileHeader))
		return FALSE;

	if (pFileHeader->m_nDataDirs > 20)
	{
		kws_log(TEXT("data dir too many"));
		return FALSE;
	}

	if (pFileHeader->m_nDataDirs >= 1)
	{
		if (!VerifyFluxListData((KStasticFluxProcessList*)(pData + pFileHeader->m_nDataOffset[0])))
			return FALSE;
	}

	return TRUE;
}

BOOL KStasticFluxCacheMgr::VerifyFluxListData(KStasticFluxProcessList* pData)
{
	if (pData->m_nSize != sizeof(KStasticFluxProcessList))
	{
		kws_log(TEXT("flux data size is not equal"));
		return FALSE;
	}
	return TRUE;
}

KStasticFluxProcessList* KStasticFluxCacheMgr::GetStasticFluxList()
{
	if (m_pData)
	{
		return (KStasticFluxProcessList*)((BYTE*)m_pData + ((NetFluxStasticCacheFileHeader*)(m_pData))->m_nDataOffset[NET_FLUX_STASTIC_DATA_CACHE]);
	}
	return NULL;
}

HRESULT	KStasticFluxCacheMgr::FlushToDisk()
{
	if (m_pData)
	{
		::FlushViewOfFile(m_pData, GetDataSize());
	}
	return S_OK;
}
// 初始化map的数据，主要是初始化锁等数据
BOOL KStasticFluxCacheMgr::InitMapData(BYTE* pData, DWORD nSize)
{
	KStasticFluxProcessList* pList = (KStasticFluxProcessList*)((BYTE*)pData + ((NetFluxStasticCacheFileHeader*)(pData))->m_nDataOffset[NET_FLUX_STASTIC_DATA_CACHE]);
	InitFluxListData(pList, nSize - ((NetFluxStasticCacheFileHeader*)(pData))->m_nDataOffset[NET_FLUX_STASTIC_DATA_CACHE], FALSE);
	return TRUE;
}

BOOL KStasticFluxCacheMgr::SetObjectToLowIntegrity(HANDLE hObject, SE_OBJECT_TYPE type)
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
KStasticProgramFlux::KStasticProgramFlux(): m_nTotalRecv(0), 
											m_nTotalSend(0)
{

}
KStasticProgramFlux::~KStasticProgramFlux()
{

}

HRESULT KStasticProgramFlux::Init(KProcFluxItem* pItem)
{
	m_strProgramPath = pItem->m_strProcPath;
	if (m_strProgramPath.GetLength() == 0)
		return E_FAIL;

	if (pItem->m_nFlux.nRecvSpeed > pItem->m_nFlux.nRecvSpeedLocal)
		m_nTotalRecv = pItem->m_nFlux.nRecvSpeed - pItem->m_nFlux.nRecvSpeedLocal;
	if (pItem->m_nFlux.nSendSpeed > pItem->m_nFlux.nSendSpeedLocal)
		m_nTotalSend = pItem->m_nFlux.nSendSpeed - pItem->m_nFlux.nSendSpeedLocal;
	return S_OK;
}

HRESULT KStasticProgramFlux::Init(KFluxStasticProcItem* pItem)
{
	m_strProgramPath = pItem->m_strProcPath;
	if (m_strProgramPath.GetLength() == 0)
		return E_FAIL;

	m_nTotalRecv = pItem->m_nTotalRecv;
	m_nTotalSend = pItem->m_nTotalSend;
	kws_log(TEXT("KStasticProgramFlux init: recv: %*I64d, send:%*I64d, %s"), 20, 
		m_nTotalRecv, 20, m_nTotalSend, (LPCWSTR)m_strProgramPath);
	return S_OK;
}

HRESULT	KStasticProgramFlux::UpdateData(KProcFluxItem* pItem)
{
	if (pItem->m_nFlux.nRecvSpeed > pItem->m_nFlux.nRecvSpeedLocal)
		m_nTotalRecv += pItem->m_nFlux.nRecvSpeed - pItem->m_nFlux.nRecvSpeedLocal;
	if (pItem->m_nFlux.nSendSpeed > pItem->m_nFlux.nSendSpeedLocal)
		m_nTotalSend += pItem->m_nFlux.nSendSpeed - pItem->m_nFlux.nSendSpeedLocal;
	return S_OK;
}

HRESULT	KStasticProgramFlux::GetFlux(KFluxStasticProcItem* pItem)
{
	pItem->m_nCbSize = sizeof(KFluxStasticProcItem);
	pItem->m_nTotalRecv = m_nTotalRecv;
	pItem->m_nTotalSend = m_nTotalSend;

	if (m_strProgramPath.GetLength() < MAX_PATH && m_strProgramPath.GetLength() > 0)
	{
		wcscpy_s(pItem->m_strProcPath, MAX_PATH, (LPCWSTR)m_strProgramPath);
		return S_OK;
	}
	return E_FAIL;
}

BOOL KStasticProgramFlux::IsMatch(KProcFluxItem* pItem)
{
	if (m_strProgramPath.CompareNoCase(pItem->m_strProcPath) == 0)
		return TRUE;
	return FALSE;
}

BOOL KStasticProgramFlux::IsMatch(KStasticProgramFlux* pItem)
{
	if (m_strProgramPath.CompareNoCase(pItem->m_strProgramPath) == 0)
		return TRUE;
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
KFluxStastic::KFluxStastic(): m_pCacheMgr(NULL),
							  m_nTimeLastWatch(0),
							  m_nTimeStartWatch(0),
							  m_nTimeTodayLastTime(0),
							  m_nTimeTodayStartTime(0),
							  m_nFluxRecv(0),
							  m_nFluxSend(0),
							  m_nFlushCounter(0)
{

}

KFluxStastic::~KFluxStastic()
{
	UnInit();
}

HRESULT	KFluxStastic::Init()
{
	if (!m_pCacheMgr)
	{
		WCHAR bufPath[MAX_PATH] = {0};
		::GetModuleFileName(NULL, bufPath, MAX_PATH);
		::PathRemoveFileSpecW(bufPath);
		::PathAppend(bufPath, TEXT("fluxstastic.dat"));

		m_pCacheMgr = new KStasticFluxCacheMgr;
		if (FAILED(m_pCacheMgr->Init(bufPath, NET_FLUX_STASTIC_MAPDATA_NAME)))
		{
			delete m_pCacheMgr;
			m_pCacheMgr = NULL;
			kws_log(TEXT("KFluxStastic::Init init KStasticFluxCacheMgr Failed"));
			return E_FAIL;
		}
	}

	__int64 nTimeNow = ATL::CTime::GetCurrentTime().GetTime();
	KStasticFluxProcessList* pList = m_pCacheMgr->GetStasticFluxList();
	if (!pList) return E_FAIL;

	if (nTimeNow >= pList->m_nTimeTodayLastTime || nTimeNow < pList->m_nTimeTodayStart)
		ResetToday(nTimeNow);
	else
	{
		// 从当天的缓存中读取
		m_nTimeStartWatch = nTimeNow;
		m_nTimeLastWatch = pList->m_nTimeWatch;
		if (m_nTimeLastWatch < 0)
			m_nTimeLastWatch = 0;

		m_nTimeTodayStartTime = pList->m_nTimeTodayStart;
		m_nTimeTodayLastTime = pList->m_nTimeTodayLastTime;

		ULONGLONG nTotalTodayRecv = 0;
		ULONGLONG nTotalTodaySend = 0;

		for (DWORD i = 0; i < pList->m_nCurrentCnt; i++)
		{
			KStasticProgramFlux* pFlux = new KStasticProgramFlux;
			if (SUCCEEDED(pFlux->Init(&(pList->m_Items[i]))))
			{
				BOOL bFind = FALSE;
				for (size_t nIndex = 0 ; nIndex < m_todayPrograms.size(); nIndex++)
				{
					if (m_todayPrograms[nIndex]->IsMatch(pFlux))
					{
						bFind = TRUE;
						break;
					}
				}
				if (!bFind)
				{
					kws_log(TEXT("KFluxStastic init add program: %s"), pFlux->GetProgramePath());
					nTotalTodayRecv += pFlux->m_nTotalRecv;
					nTotalTodaySend += pFlux->m_nTotalSend;
					m_todayPrograms.push_back(pFlux);
				}
			}
			else
			{
				delete pFlux;
				pFlux = NULL;
			}
		}

		m_nFluxRecv = nTotalTodayRecv;
		m_nFluxSend = nTotalTodaySend;
		/*
		m_nFluxRecv = pList->m_nTotalRecv;
		if (m_nFluxRecv > ((ULONGLONG)1 << 40))
		{
			LARGE_INTEGER i;
			i.QuadPart = m_nFluxRecv;
			kws_log(TEXT("find data not ok at KFluxStastic::init: %d:%d"), i.HighPart, i.LowPart);
			m_nFluxRecv = nTotalTodayRecv;
		}
		if (m_nFluxRecv < nTotalTodayRecv)
			m_nFluxRecv = nTotalTodayRecv;

		m_nFluxSend = pList->m_nTotalSend;
		if (m_nFluxSend > ((ULONGLONG)1 << 40))
		{
			LARGE_INTEGER i;
			i.QuadPart = m_nFluxSend;
			kws_log(TEXT("find data not ok at KFluxStastic::init: %d:%d"), i.HighPart, i.LowPart);
			m_nFluxSend = nTotalTodaySend;
		}
		if (m_nFluxSend < nTotalTodaySend)
			m_nFluxSend = nTotalTodaySend;*/
	}
	return S_OK;
}

HRESULT	KFluxStastic::UnInit()
{
	if (m_pCacheMgr)
	{
		m_pCacheMgr->UnInit();
		delete m_pCacheMgr;
		m_pCacheMgr = NULL;
	}
	Clear();
	return S_OK;
}

HRESULT	KFluxStastic::UpdateFlux(KProcessFluxList* pFluxList, __int64 nDisableTime)
{
	ATL::CTime nTimeNow;
	nTimeNow = ATL::CTime::GetCurrentTime();
	if (nTimeNow.GetTime() >= m_nTimeTodayLastTime || 
		nTimeNow.GetTime() < m_nTimeTodayStartTime )
		ResetToday(nTimeNow.GetTime());
	
	pFluxList->m_lock.LockRead();
	if (pFluxList->m_SysFlux.nRecvSpeed > pFluxList->m_SysFlux.nRecvSpeedLocal)
	{
		m_nFluxRecv += pFluxList->m_SysFlux.nRecvSpeed - pFluxList->m_SysFlux.nRecvSpeedLocal;
		if (m_nFluxRecv > ((ULONGLONG)1 << 40))
		{
			LARGE_INTEGER i;
			i.QuadPart = m_nFluxRecv;
			kws_log(TEXT("find data not ok at KFluxStastic::UpdateFlux: %u:%u"), i.HighPart, i.LowPart);
		}
	}
	if (pFluxList->m_SysFlux.nSendSpeed > pFluxList->m_SysFlux.nSendSpeedLocal)
	{
		m_nFluxSend += pFluxList->m_SysFlux.nSendSpeed - pFluxList->m_SysFlux.nSendSpeedLocal;
		if (m_nFluxSend > ((ULONGLONG)1 << 40))
		{
			LARGE_INTEGER i;
			i.QuadPart = m_nFluxSend;
			kws_log(TEXT("find data not ok at KFluxStastic::UpdateFlux: %u:%u"), i.HighPart, i.LowPart);
		}
	}

	for (DWORD i = 0; i < pFluxList->m_nCurrentCnt; i++)
	{
		KStasticProgramFlux* pFlux = GetStasticFlux(&(pFluxList->m_Items[i]));
		if (pFlux) pFlux->UpdateData(&(pFluxList->m_Items[i]));
	}
	pFluxList->m_lock.UnLockRead();

	RefreshToCache(nTimeNow.GetTime(), nDisableTime);
	return S_OK;
}

HRESULT	KFluxStastic::RefreshToCache(__int64 nTimeNow, __int64 nDisableTime)
{
	if (!m_pCacheMgr) return E_FAIL;
	KStasticFluxProcessList* pList = m_pCacheMgr->GetStasticFluxList();
	if (!pList) return E_FAIL;

	pList->m_lock.LockWrite();
	pList->m_nTimeTodayLastTime = m_nTimeTodayLastTime;
	pList->m_nTimeTodayStart = m_nTimeTodayStartTime;
	if (nTimeNow > m_nTimeStartWatch)
		pList->m_nTimeWatch = m_nTimeLastWatch + (nTimeNow - m_nTimeStartWatch) - nDisableTime;
	else
		pList->m_nTimeWatch = m_nTimeLastWatch;

	pList->m_nTotalRecv = m_nFluxRecv;
	pList->m_nTotalSend = m_nFluxSend;

	KFluxStasticProcItem* pItem = &(pList->m_Items[0]);

	INT nCnt = pList->m_nMaxCnt;
	INT nItemCount = 0;
	
	for (size_t i = 0 ; i < m_todayPrograms.size(); i++)
	{
		if (SUCCEEDED(m_todayPrograms[i]->GetFlux(pItem)))
		{
			pItem++;
			nItemCount++;
			nCnt--;
			if (nCnt <= 0)
				break;
		}
	}
	pList->m_nCurrentCnt = nItemCount;

	pList->m_lock.UnLockWrite();

	// 30s刷新一次磁盘，防止数据的丢失
	m_nFlushCounter += 1;
	m_nFlushCounter %= 30;
	if (m_nFlushCounter == 0)
		m_pCacheMgr->FlushToDisk();
	return S_OK;
}

HRESULT	KFluxStastic::ResetToday(__int64 nTimeNow)
{
	Clear();
	m_nTimeLastWatch = 0;
	m_nTimeStartWatch = nTimeNow;
	m_nTimeTodayLastTime = GetDayLastTime(nTimeNow);
	m_nTimeTodayStartTime = GetDayStartTime(nTimeNow);
	m_nFluxRecv = 0;
	m_nFluxSend = 0;
	return S_OK;
}

__int64	KFluxStastic::GetDayLastTime(__int64 nTime)
{
	ATL::CTime nTimeNow(nTime);
	ATL::CTimeSpan span(1, 0, 0, 0);
	nTimeNow += span;
	ATL::CTime nTimeNextDay(nTimeNow.GetYear(), nTimeNow.GetMonth(), nTimeNow.GetDay(),
		0, 0, 0);
	return nTimeNextDay.GetTime();
}

__int64	KFluxStastic::GetDayStartTime(__int64 nTime)
{
	ATL::CTime nTimeNow(nTime);
	ATL::CTime nTimeDayStart(nTimeNow.GetYear(), nTimeNow.GetMonth(), nTimeNow.GetDay(),
		0, 0, 0);
	return nTimeDayStart.GetTime();
}

KStasticProgramFlux* KFluxStastic::GetStasticFlux(KProcFluxItem* pItem)
{
	if (m_procToProgramMap.find(pItem->m_nCreateID) != m_procToProgramMap.end())
	{
		size_t i = m_procToProgramMap[pItem->m_nCreateID];
		if (i < m_todayPrograms.size())
			return m_todayPrograms[i];
	}

	for (size_t i = 0 ; i < m_todayPrograms.size(); i++)
	{
		if (m_todayPrograms[i]->IsMatch(pItem))
		{
			m_procToProgramMap[pItem->m_nCreateID] = i;
			return m_todayPrograms[i];
		}
	}
	
	KStasticProgramFlux* pFlux = new KStasticProgramFlux;
	if (FAILED(pFlux->Init(pItem)))
	{
		delete pFlux;
		return NULL;
	}
	size_t index = m_todayPrograms.size();
	m_todayPrograms.push_back(pFlux);
	m_procToProgramMap[pItem->m_nCreateID] = index;
	return pFlux;
}

HRESULT	KFluxStastic::Clear()
{
	for (size_t i = 0; i < m_todayPrograms.size(); i++)
		delete m_todayPrograms[i];

	m_todayPrograms.clear();
	m_procToProgramMap.clear();
	return S_OK;
}