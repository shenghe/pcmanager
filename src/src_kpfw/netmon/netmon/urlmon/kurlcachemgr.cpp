////////////////////////////////////////////////////////////////////////////////
//      
//      File for url moniter service
//      
//      File      : kurlcachemgr.cpp
//      Version   : 1.0
//      Comment   : 管理木马网络防火墙的cache
//      
//      Create at : 2010-10-8
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "kurlcachemgr.h"
#include "common/systool.h"
#include "json/json.h"
#include "kprocessinfo.h"
#include "atltime.h"
#include "kpfw/WhiteListMan.h"
#include "atlconv.h"
#include "wininet.h"
#include "kws/commonfun.h"
#include "kpfw/msg_logger.h"

//////////////////////////////////////////////////////////////////////////

static ATL::CTimeSpan s_span(0, 1, 0, 0);

//////////////////////////////////////////////////////////////////////////

KUrlMonCfg::KUrlMonCfg(): m_hFile(NULL),
						  m_hFileMap(NULL),
							m_pData(NULL),
							m_strUrlMapName(NULL)
{

}

KUrlMonCfg::~KUrlMonCfg()
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

HRESULT	KUrlMonCfg::Init(LPCWSTR strCacheFile, LPCWSTR urlMapName)
{
	m_strCacheFile = strCacheFile;
	m_strUrlMapName = urlMapName;

	if (!OpenCacheFile())
	{
		if (!CreateCacheFile())
		{
			kws_log(TEXT("KUrlMonCfg::Init try create cache file"));
			return E_FAIL;
		}
		if (!OpenCacheFile())
		{
			kws_log(TEXT("KUrlMonCfg::Init try open cache file 2 failed"));
			return E_FAIL;
		}
	}
	kws_log(TEXT("KUrlMonCfg::Init sucess"));

	SetDisableDelayScan(FALSE);
	return S_OK;
}

HRESULT	KUrlMonCfg::UnInit()
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

BOOL KUrlMonCfg::AddModuleScanCnt()
{
	UrlMonCfg* pCfg = GetCfg();
	if (!pCfg) return FALSE;
	pCfg->m_nModuleScanCnt++;
	return TRUE;
}

BOOL KUrlMonCfg::GetEnable()
{
	UrlMonCfg* pCfg = GetCfg();
	if (!pCfg) return FALSE;
	return pCfg->m_bEnableUrlMon;
}

BOOL KUrlMonCfg::SetEnable(BOOL bEnable)
{
	UrlMonCfg* pCfg = GetCfg();
	if (!pCfg) return FALSE;
	pCfg->m_bEnableUrlMon = bEnable;
	return TRUE;
}

BOOL KUrlMonCfg::GetEnableFluxStastic()
{
	UrlMonCfg* pCfg = GetCfg();
	if (!pCfg) return TRUE;
	return !pCfg->m_bDisableFluxStastic;
}

BOOL KUrlMonCfg::SetEnableFluxStastic(BOOL bEnable)
{
	UrlMonCfg* pCfg = GetCfg();
	if (!pCfg) return FALSE;
	pCfg->m_bDisableFluxStastic = !bEnable;
	return TRUE;
}

BOOL KUrlMonCfg::GetDisableDelayScan()
{
	UrlMonCfg* pCfg = GetCfg();
	if (!pCfg) return TRUE;
	return pCfg->m_bDisableDelayScan;
}

BOOL KUrlMonCfg::SetDisableDelayScan(BOOL bEnable)
{
	UrlMonCfg* pCfg = GetCfg();
	if (!pCfg) return FALSE;
	pCfg->m_bDisableDelayScan = bEnable;
	return TRUE;
}

DWORD KUrlMonCfg::GetDataSize()
{
	return sizeof(UrlMonCfgFileHeader) + sizeof(UrlMonCfg);
}

BOOL KUrlMonCfg::CreateCacheFile()
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

BOOL KUrlMonCfg::ResizeFileToSize(HANDLE hFile, DWORD nSize)
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

BOOL KUrlMonCfg::InitCacheFile(BYTE* pData, DWORD nSize)
{
	UrlMonCfgFileHeader* pFileHeader = (UrlMonCfgFileHeader*)pData;
	memset(pFileHeader, 0, sizeof(UrlMonCfgFileHeader));

	pFileHeader->m_nMagic				= URL_MON_CFG_FILE_MAGIC;
	pFileHeader->m_nFileVersion = URL_MON_CFG_FILE_FILE;

	DWORD nOffsetCur = sizeof(UrlMonCfgFileHeader);
	pFileHeader->m_nDataOffset[URL_MON_FILE_DATA_CFG] = nOffsetCur;
	pFileHeader->m_nDataDirs++;

	InitCfgData((UrlMonCfg*)(pData + nOffsetCur));
	return TRUE;
}

BOOL KUrlMonCfg::InitCfgData(UrlMonCfg* pData)
{
	memset(pData, 0, sizeof(UrlMonCfg));
	pData->m_nSize = sizeof(UrlMonCfg);
	pData->m_bEnableUrlMon = TRUE;
	pData->m_bDisableDelayScan = FALSE;
	return TRUE;
}

BOOL KUrlMonCfg::OpenCacheFile()
{
	BOOL	bRes = FALSE;
	HANDLE hFile = NULL;

	kws_log(TEXT("KUrlMonCfg::OpenCacheFile begin"));

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

	kws_log(TEXT("KUrlMonCfg::OpenCacheFile end sucess"));
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

BOOL KUrlMonCfg::VerifyCacheFile(HANDLE hFile)
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

	bRes = InitMapData(pMapBuf);

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

BOOL KUrlMonCfg::VerifyCacheFile(BYTE* pData, DWORD nFileSize)
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

	UrlMonCfgFileHeader* pFileHeader = (UrlMonCfgFileHeader*)pData;
	if (pFileHeader->m_nMagic != URL_MON_CFG_FILE_MAGIC)
	{
		kws_log(TEXT("file magic is not equal"));
		return FALSE;
	}

	if (pFileHeader->m_nFileVersion != URL_MON_CFG_FILE_FILE)
	{
		kws_log(TEXT("file version error"));
		return FALSE;
	}

	if (nFileSize < sizeof(UrlMonCfgFileHeader))
		return FALSE;

	if (pFileHeader->m_nDataDirs > 20)
	{
		kws_log(TEXT("data dir too many"));
		return FALSE;
	}

	if (pFileHeader->m_nDataDirs >= 1)
	{
		if (!VerifyCfgData((UrlMonCfg*)(pData + pFileHeader->m_nDataOffset[0])))
			return FALSE;
	}

	return TRUE;
}

BOOL KUrlMonCfg::VerifyCfgData(UrlMonCfg* pData)
{
	if (pData->m_nSize != sizeof(UrlMonCfg))
	{
		kws_log(TEXT("cfg data too large"));
		return FALSE;
	}
	return TRUE;
}

UrlMonCfg* KUrlMonCfg::GetCfg()
{
	if (m_pData)
	{
		return (UrlMonCfg*)((BYTE*)m_pData + ((UrlMonCfgFileHeader*)(m_pData))->m_nDataOffset[URL_MON_FILE_DATA_CFG]);
	}
	return NULL;
}

// 初始化map的数据，主要是初始化锁等数据
BOOL KUrlMonCfg::InitMapData(BYTE* pData)
{
	return TRUE;
}

BOOL KUrlMonCfg::SetObjectToLowIntegrity(HANDLE hObject, SE_OBJECT_TYPE type)
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
KUrlCacheMgr::KUrlCacheMgr(): m_pCfg(NULL)
{

}

KUrlCacheMgr::~KUrlCacheMgr()
{
	// 配置相关的，需要放在最后析构
	if (m_pCfg)
	{
		m_pCfg->UnInit();
		delete m_pCfg;
		m_pCfg = NULL;
	}
}

HRESULT	KUrlCacheMgr::Init()
{
	kws_log(TEXT("KUrlCacheMgr::Init begin"));
	if (!m_pCfg)
	{
		WCHAR bufPath[MAX_PATH] = {0};
		::GetModuleFileName(NULL, bufPath, MAX_PATH);
		::PathRemoveFileSpecW(bufPath);
		::PathAppend(bufPath, TEXT("urlmon.cfg")); 
		m_pCfg = new KUrlMonCfg;
		if(FAILED(m_pCfg->Init(bufPath, URL_MON_MAPDATA_NAME)))
		{
			kws_log(TEXT("KUrlCacheMgr::Init failed to init cfg"));
			delete m_pCfg;
			m_pCfg = NULL;
		}
	}
	
	kws_log(TEXT("KUrlCacheMgr::Init end"));
	return S_OK;
}

HRESULT KUrlCacheMgr::UnInit()
{
	return S_OK;
}
