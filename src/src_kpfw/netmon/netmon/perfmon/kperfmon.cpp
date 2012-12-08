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
#include "kperfmon.h"
#include "../newblock/systool.h"
#include "shlobj.h"
#include "atltime.h"
#include "winreg.h"
#include "msg_logger.h"


//////////////////////////////////////////////////////////////////////////
KProcessPerfCacheMgr::KProcessPerfCacheMgr(): m_hFile(NULL),
												m_hFileMap(NULL),
												m_pData(NULL),
												m_strUrlMapName(NULL)
{

}

KProcessPerfCacheMgr::~KProcessPerfCacheMgr()
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

HRESULT	KProcessPerfCacheMgr::Init(LPCWSTR strCacheFile, LPCWSTR urlMapName)
{
	m_strCacheFile = strCacheFile;
	m_strUrlMapName = urlMapName;

	if (!OpenCacheFile())
	{
		if (!CreateCacheFile())
		{
			kws_log(TEXT("KProcessPerfCacheMgr::Init try create cache file"));
			return E_FAIL;
		}
		if (!OpenCacheFile())
		{
			kws_log(TEXT("KProcessPerfCacheMgr::Init try open cache file 2 failed"));
			return E_FAIL;
		}
	}
	kws_log(TEXT("KProcessPerfCacheMgr::Init sucess"));
	return S_OK;
}

HRESULT	KProcessPerfCacheMgr::UnInit()
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

DWORD KProcessPerfCacheMgr::GetDataSize()
{
	return 1024*1024;
}

BOOL KProcessPerfCacheMgr::CreateCacheFile()
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

BOOL KProcessPerfCacheMgr::ResizeFileToSize(HANDLE hFile, DWORD nSize)
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

BOOL KProcessPerfCacheMgr::InitCacheFile(BYTE* pData, DWORD nSize)
{
	ProcPerfMonCacheFileHeader* pFileHeader = (ProcPerfMonCacheFileHeader*)pData;
	memset(pFileHeader, 0, sizeof(ProcPerfMonCacheFileHeader));

	pFileHeader->m_nMagic		= PROC_PERF_MON_FILE_MAGIC;
	pFileHeader->m_nFileVersion = PROC_PERF_MON_FILE_FILE;

	DWORD nOffsetCur = sizeof(ProcPerfMonCacheFileHeader);
	pFileHeader->m_nDataOffset[PROC_PERF_MON_DATA_CACHE] = nOffsetCur;
	pFileHeader->m_nDataDirs++;
	nSize -= nOffsetCur;

	InitPerfListData((KPocessPerfList*)(pData + nOffsetCur), nSize, TRUE);
	return TRUE;
}

BOOL KProcessPerfCacheMgr::InitPerfListData(KPocessPerfList* pData, DWORD nSize, BOOL bFirstInit)
{
	if (bFirstInit)
		memset(pData, 0, sizeof(KPocessPerfList));

	pData->m_nSize = sizeof(KPocessPerfList);
	pData->m_lock.InitLock();
	pData->m_nMaxCnt = (nSize - sizeof(KPocessPerfList)) / sizeof(KProcPerfItem);
	return TRUE;
}

BOOL KProcessPerfCacheMgr::OpenCacheFile()
{
	BOOL	bRes = FALSE;
	HANDLE hFile = NULL;

	kws_log(TEXT("KProcessPerfCacheMgr::OpenCacheFile begin"));

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

	kws_log(TEXT("KProcessPerfCacheMgr::OpenCacheFile end sucess"));
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

BOOL KProcessPerfCacheMgr::VerifyCacheFile(HANDLE hFile)
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

BOOL KProcessPerfCacheMgr::VerifyCacheFile(BYTE* pData, DWORD nFileSize)
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

	ProcPerfMonCacheFileHeader* pFileHeader = (ProcPerfMonCacheFileHeader*)pData;
	if (pFileHeader->m_nMagic != PROC_PERF_MON_FILE_MAGIC)
	{
		kws_log(TEXT("file magic is not equal"));
		return FALSE;
	}

	if (pFileHeader->m_nFileVersion != PROC_PERF_MON_FILE_FILE)
	{
		kws_log(TEXT("file version error"));
		return FALSE;
	}

	if (nFileSize < sizeof(ProcPerfMonCacheFileHeader))
		return FALSE;

	if (pFileHeader->m_nDataDirs > 20)
	{
		kws_log(TEXT("data dir too many"));
		return FALSE;
	}

	if (pFileHeader->m_nDataDirs >= 1)
	{
		if (!VerifyPerfListData((KPocessPerfList*)(pData + pFileHeader->m_nDataOffset[0])))
			return FALSE;
	}

	return TRUE;
}

BOOL KProcessPerfCacheMgr::VerifyPerfListData(KPocessPerfList* pData)
{
	if (pData->m_nSize != sizeof(KPocessPerfList))
	{
		kws_log(TEXT("flux data size is not equal"));
		return FALSE;
	}
	return TRUE;
}

KPocessPerfList* KProcessPerfCacheMgr::GetProcessPerfList()
{
	if (m_pData)
	{
		return (KPocessPerfList*)((BYTE*)m_pData + ((ProcPerfMonCacheFileHeader*)(m_pData))->m_nDataOffset[PROC_PERF_MON_DATA_CACHE]);
	}
	return NULL;
}

// 初始化map的数据，主要是初始化锁等数据
BOOL KProcessPerfCacheMgr::InitMapData(BYTE* pData, DWORD nSize)
{
	KPocessPerfList* pList = (KPocessPerfList*)((BYTE*)pData + ((ProcPerfMonCacheFileHeader*)(pData))->m_nDataOffset[PROC_PERF_MON_DATA_CACHE]);
	InitPerfListData(pList, nSize - ((ProcPerfMonCacheFileHeader*)(pData))->m_nDataOffset[PROC_PERF_MON_DATA_CACHE], FALSE);
	return TRUE;
}

BOOL KProcessPerfCacheMgr::SetObjectToLowIntegrity(HANDLE hObject, SE_OBJECT_TYPE type)
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
KPerfNameMgr::KPerfNameMgr():m_NameBuf(NULL),m_pNameArray(NULL), m_nMaxNameID(0)
{
	BuildNameMap();
}

KPerfNameMgr::~KPerfNameMgr()
{
	if (m_pNameArray)
	{
		m_nMaxNameID = 0;
		delete[] m_pNameArray;
		m_pNameArray = NULL;
	}

	if (m_NameBuf)
	{
		delete[] m_NameBuf;
		m_NameBuf = NULL;
	}
}

LPCSTR	KPerfNameMgr::GetName(INT nID)
{
	if (nID >= 0 && nID < m_nMaxNameID)
		return m_pNameArray[nID];
	return NULL;
}

INT	KPerfNameMgr::GetDataIDByCounterID(INT nCounterNameID)
{
	LPCSTR strName = GetName(nCounterNameID);
	if (strName)
	{
		for (int i = 0; i < Proc_Perf_Data_Max; i++)
		{
			if (_strnicmp(strName, m_ProcessDataNames[i], strlen(m_ProcessDataNames[i])) == 0)
				return i;
		}
	}
	return -1;
}

BOOL KPerfNameMgr::BuildNameMap()
{
	HKEY hKeyPerflib;      // handle to registry key
	HKEY hKeyPerflib009;   // handle to registry key
	DWORD dwMaxValueLen;   // maximum size of key values
	DWORD dwBuffer;        // bytes to allocate for buffers
	DWORD dwBufferSize = sizeof(DWORD);  // size of dwBuffer
	LPSTR lpCurrentString; // pointer for enumerating data strings
	DWORD dwCounter;       // current counter index
	LONG lResult;          // return value

	// Get the number of Counter items.

	if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib",
		0,
		KEY_READ,
		&hKeyPerflib) != ERROR_SUCCESS
		)
		return FALSE;

	lResult = RegQueryValueEx( hKeyPerflib,
		L"Last Counter",
		NULL,
		NULL,
		(LPBYTE) &m_nMaxNameID,
		&dwBufferSize );

	RegCloseKey( hKeyPerflib );

	if( lResult != ERROR_SUCCESS )
		return FALSE;

	// Allocate memory for the names array.

	m_pNameArray = new LPSTR[m_nMaxNameID];

	if( m_pNameArray == NULL )
		return FALSE;
	memset(m_pNameArray, 0, m_nMaxNameID*sizeof(LPSTR));
	// Open the key containing the counter and object names.

	if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\009",
		0,
		KEY_READ,
		&hKeyPerflib009) != ERROR_SUCCESS
		)
		return FALSE;

	// Get the size of the largest value in the key (Counter or Help).

	if( RegQueryInfoKey( hKeyPerflib009,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&dwMaxValueLen,
		NULL,
		NULL) != ERROR_SUCCESS
		)
		return FALSE;

	// Allocate memory for the counter and object names.

	dwBuffer = dwMaxValueLen + 1;

	m_NameBuf = new CHAR[dwBuffer];//(CHAR*)malloc( dwBuffer * sizeof(CHAR) );

	if (m_NameBuf == NULL)
	{
		delete [] m_pNameArray;
		return FALSE;
	}

	// Read the counter value.

	lResult = RegQueryValueEx( hKeyPerflib009,
		L"Counters",
		NULL,
		NULL,
		(LPBYTE)m_NameBuf, &dwBuffer );

	RegCloseKey( hKeyPerflib009 ); 

	if( lResult != ERROR_SUCCESS )
		return FALSE;

	// Load names into an array, by index.

	for( lpCurrentString = m_NameBuf; *lpCurrentString;
		lpCurrentString += (strlen(lpCurrentString)+1) )
	{
		dwCounter = atol( lpCurrentString );

		lpCurrentString += (strlen(lpCurrentString)+1);

		m_pNameArray[dwCounter] = (LPSTR) lpCurrentString;
	}

	return TRUE;
}

LPCSTR KPerfNameMgr::m_ProcessDataNames[Proc_Perf_Data_Max] = 
{
	"% Processor Time",
	"% User Time",
	"% Privileged Time",
	"Virtual Bytes Peak",
	"Virtual Bytes",
	"Page Faults/sec",
	"Working Set Peak",
	"Working Set",
	"Page File Bytes Peak",
	"Page File Bytes",
	"Private Bytes",
	"Thread Count",
	"Priority Base",
	"Elapsed Time",
	"ID Process",
	"Creating Process ID",
	"Pool Paged Bytes",
	"Pool Nonpaged Bytes",
	"Handle Count",
	"IO Read Operations/sec",
	"IO Write Operations/sec",
	"IO Data Operations/sec",
	"IO Other Operations/sec",
	"IO Read Bytes/sec",
	"IO Write Bytes/sec",
	"IO Data Bytes/sec",
	"IO Other Bytes/sec",
	"Working Set",
};

//////////////////////////////////////////////////////////////////////////
KProcessPerfData::KProcessPerfData():m_nProcessID(-1),
									m_cpuCounterLast(0),
									m_cpuCounterLastTime(0),
									m_memUsage(0),
									m_ioSpeed(0),
									m_ioCounterLast(0),
									m_ioCounterLastTime(0),
									m_timeLast(0)
{
	m_cpuUsage = 0;
}

KProcessPerfData::~KProcessPerfData()
{

}

BOOL KProcessPerfData::UpdateData(PPERF_INSTANCE_DEFINITION  PerfInst,
				PPERF_COUNTER_DEFINITION   ProcessCounters[Proc_Perf_Data_Max],
				PPERF_OBJECT_TYPE PerfObj)
{
	PPERF_COUNTER_DEFINITION pCounter = ProcessCounters[Proc_Perf_Data_Process_ID];
	if (!pCounter) return FALSE;
	m_nProcessID = _GetProcessID(PerfInst, pCounter);
	m_strProcName = (wchar_t*)((PBYTE)PerfInst + PerfInst->NameOffset);

	pCounter = ProcessCounters[Proc_Perf_Data_Elapsed_Time];
	if (!pCounter) return FALSE;
	ULONGLONG nTimePast = 0;
	ULONGLONG nTimeNow = _GetUlonglong(PerfInst, pCounter);
	if (nTimeNow != 0)
	{
		nTimeNow = PerfObj->PerfTime.QuadPart - nTimeNow;
		if (m_timeLast != 0)
			nTimePast = nTimeNow - m_timeLast;
		m_timeLast = nTimeNow;
	}

	// 计算cpu使用率
	pCounter = ProcessCounters[Proc_Perf_Data_Processor_Time];
	if (!pCounter) return FALSE;
	ULONGLONG nCpuCounter = _GetCpuCounter(PerfInst, pCounter);
	if (m_cpuCounterLast == 0)
	{
		m_cpuUsage = 0;
		m_cpuCounterLast = nCpuCounter;
		m_cpuCounterLastTime = PerfObj->PerfTime.QuadPart;
	}
	else
	{
		ULONGLONG cpuUse = nCpuCounter - m_cpuCounterLast;
		ULONGLONG nTimeSpan = PerfObj->PerfTime.QuadPart - m_cpuCounterLastTime;
		if (nTimeSpan != 0)
		{
			m_cpuUsage = (double)cpuUse * 100.0 / (double)nTimeSpan;
			m_cpuCounterLast = nCpuCounter;
			m_cpuCounterLastTime = PerfObj->PerfTime.QuadPart;
		}
	}

	// 内存使用
	pCounter = ProcessCounters[Proc_Perf_Data_Working_Set];
	if (!pCounter) return FALSE;
	ULONGLONG nWorkingSet = _GetUlonglong(PerfInst, pCounter);
	m_memUsage = nWorkingSet;

	// io读写率
	pCounter = ProcessCounters[Proc_Perf_Data_IO_Data_Per_Sec];
	if (!pCounter) return FALSE;
	ULONGLONG nDataCount = _GetUlonglong(PerfInst, pCounter);
	if (m_ioCounterLast == 0)
	{
		m_ioSpeed = 0;
		m_ioCounterLast = nDataCount;
		m_ioCounterLastTime = PerfObj->PerfTime.QuadPart;
	}
	else
	{
		ULONGLONG nDataInc = nDataCount - m_ioCounterLast;
		ULONGLONG nTimeInc = PerfObj->PerfTime.QuadPart - m_ioCounterLastTime;
		if (nTimeInc != 0)
		{
			m_ioSpeed = nDataInc * PerfObj->PerfFreq.QuadPart / nTimeInc;
			m_ioCounterLast = nDataInc;
			m_ioCounterLastTime = PerfObj->PerfTime.QuadPart;
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
KProcessPerfMgr::KProcessPerfMgr():m_PerfData(NULL),
									m_pNameMgr(NULL),
									m_nPerfDataSize(0)
{

}

KProcessPerfMgr::~KProcessPerfMgr()
{
	
}

HRESULT KProcessPerfMgr::Init()
{
	if (!m_pNameMgr)
	{
		m_pNameMgr = new KPerfNameMgr;
		if (!m_pNameMgr) return E_FAIL;
	}
	if (!m_PerfData)
	{
		m_nPerfDataSize = TOTALBYTES;
		m_PerfData = (PPERF_DATA_BLOCK) new BYTE[m_nPerfDataSize];
		if (!m_PerfData) return E_FAIL;
	}
	if (!m_pCacheMgr)
	{
		WCHAR bufPath[MAX_PATH] = {0};
		::GetModuleFileName(NULL, bufPath, MAX_PATH);
		::PathRemoveFileSpecW(bufPath);
		::PathAppend(bufPath, TEXT("procperf.dat"));

		m_pCacheMgr = new KProcessPerfCacheMgr;
		if (FAILED(m_pCacheMgr->Init(bufPath, PROC_PERF_MON_MAPDATA_NAME)))
		{
			delete m_pCacheMgr;
			m_pCacheMgr = NULL;
			kws_log(TEXT("KProcessPerfMgr::Init init KProcessPerfCacheMgr Failed"));
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT KProcessPerfMgr::UnInit()
{
	if (m_PerfData)
	{
		delete[] m_PerfData;
		m_PerfData = NULL;
	}
	_ClearProcessInfo();

	if (m_pNameMgr)
	{
		delete m_pNameMgr;
		m_pNameMgr = NULL;
	}
	return S_OK;
}

HRESULT KProcessPerfMgr::Update()
{
	DWORD nBufSize = m_nPerfDataSize;

	while( RegQueryValueEx( HKEY_PERFORMANCE_DATA,
		L"232",
		NULL,
		NULL,
		(LPBYTE) m_PerfData,
		&nBufSize ) == ERROR_MORE_DATA )
	{
		// Get a buffer that is big enough.

		m_nPerfDataSize += BYTEINCREMENT;
		if (m_PerfData)
		{
			delete [] m_PerfData;
			m_PerfData = 0;
		}
		m_PerfData = (PPERF_DATA_BLOCK) new BYTE[ m_nPerfDataSize ];
		if (!m_PerfData) break;
	}
	if (m_PerfData)
	{
		// Get the first object type.
		PPERF_OBJECT_TYPE PerfObj = FirstObject( m_PerfData );
		UpdatePerfData(PerfObj);
	}
	return S_OK;
}

void KProcessPerfMgr::UpdatePerfData(PPERF_OBJECT_TYPE PerfObj)
{
	if (!_UpdateCounterInfo(PerfObj))
		return;

	PPERF_INSTANCE_DEFINITION PerfInst = NULL;
	map<DWORD, KProcessPerfData*> newProcessInfo;
	if( PerfObj->NumInstances > 0 )
	{
		PerfInst = FirstInstance( PerfObj );
		double fSumExceptIdle = 0;
		KProcessPerfData* pIdleProcess = NULL;

		for( int k=0; k < PerfObj->NumInstances; k++ )
		{ 
			wchar_t* pInstName = (wchar_t*)((PBYTE)PerfInst + PerfInst->NameOffset);
			if ((wcsicmp(pInstName, L"_total") != 0))
			{
				KProcessPerfData* pProcessData = _UpdateProcessData(PerfInst, PerfObj);
				if (pProcessData)
				{
					newProcessInfo[pProcessData->GetProcessID()] = pProcessData;
					if (pProcessData->GetProcessID() != 0)
						fSumExceptIdle += pProcessData->GetCpuUsage();
					else
						pIdleProcess = pProcessData;
				}
			}
			else
			{
				_UpdateProcessTotal(PerfInst, PerfObj);
			}
			PerfInst = NextInstance( PerfInst );
		}
		if (pIdleProcess)
		{
			double idleUsage = (double)100.0 - fSumExceptIdle;
			pIdleProcess->SetCpuUsage(idleUsage);
		}
	}

	_ClearProcessInfo();
	m_ProcessInfo = newProcessInfo;
	newProcessInfo.clear();

}

BOOL KProcessPerfMgr::_UpdateCounterInfo(PPERF_OBJECT_TYPE PerfObj)
{
	PPERF_COUNTER_DEFINITION PerfCntr = NULL, CurCntr = NULL;
	PPERF_COUNTER_BLOCK PtrToCntr = NULL;
	BOOL bShouldBuildMap = FALSE;
	if (m_mapProcessData2CounterID.empty())
		bShouldBuildMap = TRUE;

	memset(&m_ProcessCounters, 0, sizeof(m_ProcessCounters));

	PerfCntr = FirstCounter( PerfObj );
	for (DWORD i = 0; i < PerfObj->NumCounters; i++)
	{
		if (m_mapProcessData2CounterID.find(PerfCntr->CounterNameTitleIndex) != 
			m_mapProcessData2CounterID.end())
		{
			m_ProcessCounters[m_mapProcessData2CounterID[PerfCntr->CounterNameTitleIndex]]
			= PerfCntr;
		}
		else
		{
			if (bShouldBuildMap)
			{
				int nDataID = m_pNameMgr->GetDataIDByCounterID(PerfCntr->CounterNameTitleIndex);
				m_mapProcessData2CounterID[PerfCntr->CounterNameTitleIndex] = nDataID;
				m_ProcessCounters[nDataID] = PerfCntr;
			}
		}
		PerfCntr = NextCounter(PerfCntr);
	}
	return TRUE;
}

KProcessPerfData* KProcessPerfMgr::_UpdateProcessData(PPERF_INSTANCE_DEFINITION PerfInst, PPERF_OBJECT_TYPE PerfObj)
{
	DWORD nProcessID = _GetProcessID(PerfInst);
	if (nProcessID == -1) return NULL;

	KProcessPerfData* pData = _GetProcessData(nProcessID);
	if (!pData)
		pData = new KProcessPerfData;
	if (pData )
	{
		if (pData->UpdateData(PerfInst, m_ProcessCounters, PerfObj))
			return pData;
		else
			delete pData;
	}
	return NULL;
}

KProcessPerfData* KProcessPerfMgr::_GetProcessData(DWORD nProcessID)
{
	map<DWORD, KProcessPerfData*>::iterator it = m_ProcessInfo.find(nProcessID);
	if (it == m_ProcessInfo.end())
		return NULL;
	else
	{
		KProcessPerfData* pData = it->second;
		m_ProcessInfo.erase(it);
		return pData;
	}

	return NULL;
}

DWORD KProcessPerfMgr::_GetProcessID(PPERF_INSTANCE_DEFINITION PerfInst)
{
	PPERF_COUNTER_DEFINITION pCounter = m_ProcessCounters[Proc_Perf_Data_Process_ID];
	if (!pCounter) return -1;
	LPBYTE pData = GetCounterData(PerfInst, pCounter);
	if (pData) return *((DWORD*)pData);
	return -1;
}

void KProcessPerfMgr::_ClearProcessInfo()
{
	map<DWORD, KProcessPerfData*>::iterator it = m_ProcessInfo.begin();
	while (it != m_ProcessInfo.end())
	{
		delete it->second;
		it++;
	}
	m_ProcessInfo.clear();
}