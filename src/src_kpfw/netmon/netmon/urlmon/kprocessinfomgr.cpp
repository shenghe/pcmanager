////////////////////////////////////////////////////////////////////////////////
//      
//      File for url moniter service
//      
//      File      : kprocessinfomgr.cpp
//      Version   : 1.0
//      Comment   : 木马网络防火墙的管理所有的进程安全，并且提供执行线程
//      
//      Create at : 2010-10-8
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "kprocessinfomgr.h"
#include "kmodulemgr.h"
#include "kpfw/msg_logger.h"
#include "kprocessinfo.h"
#include <algorithm>
#include "common/systool.h"
#include "shlobj.h"

//////////////////////////////////////////////////////////////////////////
KProcessInfoCacheMgr::KProcessInfoCacheMgr(): m_hFile(NULL),
										m_hFileMap(NULL),
										m_pData(NULL),
										m_strUrlMapName(NULL)
{

}

KProcessInfoCacheMgr::~KProcessInfoCacheMgr()
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

HRESULT	KProcessInfoCacheMgr::Init(LPCWSTR strCacheFile, LPCWSTR urlMapName)
{
	m_strCacheFile = strCacheFile;
	m_strUrlMapName = urlMapName;

	if (!OpenCacheFile())
	{
		if (!CreateCacheFile())
		{
			kws_log(TEXT("KProcessInfoCacheMgr::Init try create cache file"));
			return E_FAIL;
		}
		if (!OpenCacheFile())
		{
			kws_log(TEXT("KProcessInfoCacheMgr::Init try open cache file 2 failed"));
			return E_FAIL;
		}
	}
	kws_log(TEXT("KProcessInfoCacheMgr::Init sucess"));
	return S_OK;
}

HRESULT	KProcessInfoCacheMgr::UnInit()
{
	kws_log(TEXT("KProcessInfoCacheMgr::UnInit"));
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

DWORD KProcessInfoCacheMgr::GetDataSize()
{
	return 1024*1024;
}

BOOL KProcessInfoCacheMgr::CreateCacheFile()
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

BOOL KProcessInfoCacheMgr::ResizeFileToSize(HANDLE hFile, DWORD nSize)
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

BOOL KProcessInfoCacheMgr::InitCacheFile(BYTE* pData, DWORD nSize)
{
	ProcInfoMonCacheFileHeader* pFileHeader = (ProcInfoMonCacheFileHeader*)pData;
	memset(pFileHeader, 0, sizeof(ProcInfoMonCacheFileHeader));

	pFileHeader->m_nMagic		= PROC_INFO_MON_FILE_MAGIC;
	pFileHeader->m_nFileVersion = PROC_INFO_MON_FILE_FILE;

	DWORD nOffsetCur = sizeof(ProcInfoMonCacheFileHeader);
	pFileHeader->m_nDataOffset[PROC_INFO_MON_DATA_CACHE] = nOffsetCur;
	pFileHeader->m_nDataDirs++;
	nSize -= nOffsetCur;

	InitProcInfoListData((KPocessInfoList*)(pData + nOffsetCur), nSize, TRUE);
	return TRUE;
}

BOOL KProcessInfoCacheMgr::InitProcInfoListData(KPocessInfoList* pData, DWORD nSize, BOOL bFirstInit)
{
	memset(pData, 0, sizeof(KPocessInfoList));
	pData->m_nSize = sizeof(KPocessInfoList);
	pData->m_lock.InitLock();
	pData->m_nMaxCnt = (nSize - sizeof(KPocessInfoList)) / sizeof(KProcInfoItem);
	return TRUE;
}

BOOL KProcessInfoCacheMgr::OpenCacheFile()
{
	BOOL	bRes = FALSE;
	HANDLE hFile = NULL;

	kws_log(TEXT("KProcessInfoCacheMgr::OpenCacheFile begin"));

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

	kws_log(TEXT("KProcessInfoCacheMgr::OpenCacheFile end sucess"));
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

BOOL KProcessInfoCacheMgr::VerifyCacheFile(HANDLE hFile)
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

BOOL KProcessInfoCacheMgr::VerifyCacheFile(BYTE* pData, DWORD nFileSize)
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

	ProcInfoMonCacheFileHeader* pFileHeader = (ProcInfoMonCacheFileHeader*)pData;
	if (pFileHeader->m_nMagic != PROC_INFO_MON_FILE_MAGIC)
	{
		kws_log(TEXT("file magic is not equal"));
		return FALSE;
	}

	if (pFileHeader->m_nFileVersion != PROC_INFO_MON_FILE_FILE)
	{
		kws_log(TEXT("file version error"));
		return FALSE;
	}

	if (nFileSize < sizeof(ProcInfoMonCacheFileHeader))
		return FALSE;

	if (pFileHeader->m_nDataDirs > 20)
	{
		kws_log(TEXT("data dir too many"));
		return FALSE;
	}

	if (pFileHeader->m_nDataDirs >= 1)
	{
		if (!VerifyProcInfoListData((KPocessInfoList*)(pData + pFileHeader->m_nDataOffset[0])))
			return FALSE;
	}

	return TRUE;
}

BOOL KProcessInfoCacheMgr::VerifyProcInfoListData(KPocessInfoList* pData)
{
	if (pData->m_nSize != sizeof(KPocessInfoList))
	{
		kws_log(TEXT("flux data size is not equal"));
		return FALSE;
	}
	return TRUE;
}

KPocessInfoList* KProcessInfoCacheMgr::GetProcessInfoList()
{
	if (m_pData)
	{
		return (KPocessInfoList*)((BYTE*)m_pData + ((ProcInfoMonCacheFileHeader*)(m_pData))->m_nDataOffset[PROC_INFO_MON_DATA_CACHE]);
	}
	return NULL;
}

HRESULT	KProcessInfoCacheMgr::FlushToDisk()
{
	if (m_pData)
	{
		::FlushViewOfFile(m_pData, GetDataSize());
	}
	return S_OK;
}
// 初始化map的数据，主要是初始化锁等数据
BOOL KProcessInfoCacheMgr::InitMapData(BYTE* pData, DWORD nSize)
{
	KPocessInfoList* pList = (KPocessInfoList*)((BYTE*)pData + ((ProcInfoMonCacheFileHeader*)(pData))->m_nDataOffset[PROC_INFO_MON_DATA_CACHE]);
	InitProcInfoListData(pList, nSize - ((ProcInfoMonCacheFileHeader*)(pData))->m_nDataOffset[PROC_INFO_MON_DATA_CACHE], FALSE);
	return TRUE;
}

BOOL KProcessInfoCacheMgr::SetObjectToLowIntegrity(HANDLE hObject, SE_OBJECT_TYPE type)
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
KProcessInfoMgr::KProcessInfoMgr(): m_pModuleMgr(NULL),
									m_ScanThread(NULL),
									m_bScanThreadWorking(FALSE),
									m_hThreadExit(NULL),
									m_bExit(FALSE),
									m_pUrlMgr(NULL),
									m_pCacheMgr(NULL)
{

}

KProcessInfoMgr::~KProcessInfoMgr()
{

}

HRESULT KProcessInfoMgr::Init(KModuleMgr* pModuleMgr, KUrlCacheMgr* pUrlMgr)
{
	m_pModuleMgr = pModuleMgr;
	m_pUrlMgr = pUrlMgr;
	if (!m_pCacheMgr)
	{
		WCHAR bufPath[MAX_PATH] = {0};
		::GetModuleFileName(NULL, bufPath, MAX_PATH);
		::PathRemoveFileSpecW(bufPath);
		::PathAppend(bufPath, TEXT("procinfo.dat"));

		m_pCacheMgr = new KProcessInfoCacheMgr;
		if (FAILED(m_pCacheMgr->Init(bufPath, PROC_INFO_MON_MAPDATA_NAME)))
		{
			delete m_pCacheMgr;
			m_pCacheMgr = NULL;
			kws_log(TEXT("KProcessInfoMgr::Init init KProcessInfoCacheMgr Failed"));
			return E_FAIL;
		}
	}
	m_hThreadExit = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_bScanThreadWorking = FALSE;
	m_ScanThread = ::CreateThread(NULL, 0, ThreadScan, this, NULL, NULL);

	return S_OK;
}

HRESULT KProcessInfoMgr::UnInit()
{
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

	if (m_pCacheMgr)
	{
		m_pCacheMgr->UnInit();
		delete m_pCacheMgr;
		m_pCacheMgr = NULL;
	}

	Clear();
	return S_OK;
}

void KProcessInfoMgr::Clear()
{
	kis::KLocker		_locker(m_lock);
	map<DWORD, KProcessInfo*>::iterator it = m_RycProcessList.begin();
	while (it != m_RycProcessList.end())
	{
		delete it->second;
		it++;
	}
	m_RycProcessList.clear();

	it = m_ProcessList.begin();
	while (it != m_ProcessList.end())
	{
		delete it->second;
		it++;
	}
	m_ProcessList.clear();
}

KProcessInfo* KProcessInfoMgr::GetProcessInfo(DWORD nProcessID, ULONGLONG nCreateTime)
{
	kis::KLocker		_locker(m_lock);
	if (m_bExit) return NULL;

	if (m_ProcessList.find(nProcessID) == m_ProcessList.end())
	{
		KProcessInfo* pInfo = new KProcessInfo(nProcessID, nCreateTime, m_pUrlMgr);
		if (pInfo)
		{
			m_ProcessList[nProcessID] = pInfo;
			return pInfo;
		}
	}
	else
	{
		KProcessInfo* pInfo = m_ProcessList[nProcessID];
		if (pInfo->GetCreateTime()  == nCreateTime)
			return pInfo;
		else
		{
			// 保证新创建的进程创建时间比去掉的进程的时间晚
			if (pInfo->GetCreateTime() < nCreateTime)
			{
				AddToRecycle(pInfo);
				pInfo = new KProcessInfo(nProcessID, nCreateTime, m_pUrlMgr);
				if (pInfo)
				{
					m_ProcessList[nProcessID] = pInfo;
					return pInfo;
				}
				else
				{
					m_ProcessList.erase(m_ProcessList.find(nProcessID));
					return NULL;
				}
			}
			else
			{
				// 后创建的进程还请求了数据
				kws_log(TEXT("KProcessInfoMgr::GetProcessInfo find error process get timethis:%d time last:%d"), nCreateTime, pInfo->GetCreateTime());
				return NULL;
			}
		}
	}
	return NULL;
}

void KProcessInfoMgr::ClearProcessNotNow(ULONGLONG nUpdateCounter)
{
	kis::KLocker		_locker(m_lock);
	vector<KProcessInfo*> procToRemove;
	map<DWORD, KProcessInfo*>::iterator it = m_ProcessList.begin();
	while (it != m_ProcessList.end())
	{
		if (it->second->GetUpdateCounter() != nUpdateCounter)
			procToRemove.push_back(it->second);
		it++;
	}
	for (size_t i = 0; i < procToRemove.size(); i++)
	{
		kws_log(TEXT("KProcessInfoMgr::ClearProcessNotNow erase: %d, %s"), (DWORD)procToRemove[i]->GetPID(),
			procToRemove[i]->GetProcPath());
		m_ProcessList.erase(m_ProcessList.find((DWORD)procToRemove[i]->GetPID()));
		AddToRecycle(procToRemove[i]);
	}
}

// 添加到后备列表中，用来等待模块判定黑白
void KProcessInfoMgr::AddToRecycle(KProcessInfo* pInfo)
{
	if (pInfo->HasWaitingProcess())
	{
		if (m_RycProcessList.find(pInfo->GetPID()) != m_RycProcessList.end())
		{
			KProcessInfo* pInfoRec = m_RycProcessList[pInfo->GetPID()];
			if (pInfoRec->GetCreateTime() != pInfo->GetCreateTime())
				delete pInfoRec;
			else
			{
				if (pInfoRec == pInfo)
					return;
				else
				{
					// 出现严重错误
					kws_log(TEXT("KProcessInfoMgr::AddToRecycle error"));
					delete pInfo;
					return;
				}
			}
		}
		m_RycProcessList[pInfo->GetPID()] = pInfo;
	}
	else
	{
		delete pInfo;
	}
}

DWORD KProcessInfoMgr::ScanProcessInfo()
{
	kis::KLocker		_locker(m_lock);

	vector<KProcessInfo*>	ProcessFinishList;
	map<DWORD, KProcessInfo*>::iterator it = m_RycProcessList.begin();
	while ((it != m_RycProcessList.end()) && !m_bExit)
	{
		KProcessInfo* pProcess = it->second;
		pProcess->Process();

		// 如果没有需要处理的url了，那么它就应该释放了
		if (!pProcess->HasWaitingProcess())
			ProcessFinishList.push_back(pProcess);

		it++;
	}

	it = m_ProcessList.begin();
	while (it != m_ProcessList.end() && !m_bExit)
	{
		KProcessInfo* pProcess = it->second;
		pProcess->Process();
		it++;
	}

	if (!ProcessFinishList.empty())
	{
		for (size_t i = 0; i < ProcessFinishList.size(); i++)
		{
			KProcessInfo* pProcess = ProcessFinishList[i];
			m_RycProcessList.erase(pProcess->GetPID());
			delete pProcess;
		}
	}
	return 0;
}

DWORD KProcessInfoMgr::DoScan()
{
	m_bScanThreadWorking = TRUE;
	try
	{
		while (1)
		{
			DWORD nWaitRes = ::WaitForSingleObject(m_hThreadExit, 500);
			switch (nWaitRes)
			{
			case WAIT_TIMEOUT:
				ScanProcessInfo();
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

DWORD KProcessInfoMgr::ThreadScan(LPVOID lpParameter)
{
	KProcessInfoMgr* pThis = (KProcessInfoMgr*)lpParameter;
	if (pThis)
		pThis->DoScan();
	return 0;
}

HRESULT	KProcessInfoMgr::RefreshToCache()
{
	if (!m_pCacheMgr) return E_FAIL;
	KPocessInfoList* pList = m_pCacheMgr->GetProcessInfoList();
	if (!pList) return E_FAIL;
	kis::KLocker		_locker(m_lock);

	pList->m_lock.LockWrite();

	KProcInfoItem* pItem = &(pList->m_Items[0]);
	pList->m_nCurrentCnt = 0;

	INT nCnt = pList->m_nMaxCnt;
	INT nItemCount = 0;
	
	if (m_ProcessList.size() > 0)
	{		
		map<DWORD, KProcessInfo*>::iterator it = m_ProcessList.begin();

		for (; it != m_ProcessList.end(); it++)
		{
			if (SUCCEEDED(it->second->GetProcInfo(pItem)))
			{
				pItem++;
				nItemCount++;
				nCnt--;
				if (nCnt <= 0)
					break;
			}

	}
	}
	pList->m_nCurrentCnt = nItemCount;

	pList->m_lock.UnLockWrite();

	return S_OK;
}