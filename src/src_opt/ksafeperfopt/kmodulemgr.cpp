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

#include "shlobj.h"
#include "atlconv.h"
#include "atltime.h"

// #include "msg_logger.h"
#define kws_log
// #include "kurlcachemgr.h"


//////////////////////////////////////////////////////////////////////////
KModuleInfo::KModuleInfo(LPCWSTR strPath, ULONGLONG nID): m_nID(nID),
														  m_pMd5(NULL),
														  m_nSecurityState(0),
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

KModuleInfo::~KModuleInfo()
{
	if (m_pMd5)
	{
		delete m_pMd5;
		m_pMd5 = NULL;
	}
}
BOOL	KModuleInfo::SetMd5(BYTE md5[16])
{
	if (!m_pMd5)
	{
		m_pMd5 = new BYTE[16];
		memcpy(m_pMd5, md5, 16);
		return TRUE;
	}
	return FALSE;
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
		return FALSE;
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
	if (m_pMd5)
	{
		delete [] m_pMd5;
		m_pMd5 = NULL;
	}
	return TRUE;
}

BOOL KModuleInfo::GetFluxLimit(DWORD& nSendLimit, DWORD& nRecvLimit, DWORD& bDisable)
{
	nSendLimit = m_nSendLimit;
	nRecvLimit = m_nRecvLimit;
	bDisable = m_bDisable;
	return TRUE;
}

BOOL KModuleInfo::SetFluxLimit(DWORD nSendLimit, DWORD nRecvLimit, DWORD bDisable)
{
	m_nSendLimit = nSendLimit;
	m_nRecvLimit = nRecvLimit;
	m_bDisable = bDisable;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
KModuleMgr::KModuleMgr(): m_ScanThread(NULL),
							m_bScanThreadWorking(FALSE),
							m_hThreadExit(NULL),
							// m_pScanEngine(NULL),
							m_bExit(FALSE)
{
	memset(&m_hScanSession, 0, sizeof(m_hScanSession));
}

KModuleMgr::~KModuleMgr()
{

}
/*
KSEScanWrapper2* KModuleMgr::GetScanEngine()
{
	if (!m_pScanEngine)
	{
		S = new KSEScanWrapper2;
		if (FAILED(m_pScanEngine->Init()))
		{
			kws_log(TEXT("KModuleMgr::Init INIT m_pScanEngine FAILED"));
			delete m_pScanEngine;
			m_pScanEngine = NULL;
		}
	}
	return m_pScanEngine;
}*/

HRESULT KModuleMgr::GetScanSession(S_KSE_LITESCAN_SESSION& session)
{
	S_KSE_LITESCAN_SESSION nullSession = {0};
	if (memcmp(&m_hScanSession, &nullSession, sizeof(S_KSE_LITESCAN_SESSION)) == 0)
	{
		if (!m_scanServiceWrapper.IsInitialized())
		{
			int nRet = m_scanServiceWrapper.Initialize();
			if (nRet != 0)
			{
				return nRet;
			}		
		}	

		int nRet = m_scanServiceWrapper.CreateScanSession(
			KXE_NULL_PARAEMETER(),
			m_hScanSession
			);
		if (nRet != 0)
		{
			return nRet;
		}
	}
	session = m_hScanSession;
	return S_OK;
}

HRESULT KModuleMgr::Init()
{
	// DebugPrintf(_T("KsScanClient.CreateScanSession hr=0x%X"), hr);

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
/*
	if (m_pScanEngine)
	{
		m_pScanEngine->Uninit();
		delete m_pScanEngine;
		m_pScanEngine = NULL;
	}*/

	m_scanServiceWrapper.CloseScanSession(m_hScanSession, KXE_NULL_PARAEMETER());

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

void KModuleMgr::AddModuleToScan(KModuleInfo* pModule)
{
	m_ModulesToScan.push_back(pModule);
}

DWORD KModuleMgr::ScanModuleItem()
{
	// 推迟到系统开机5分钟以后才开始扫描
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
	S_KSE_LITESCAN_SESSION session = {0};
	if (FAILED(GetScanSession(session)))
		return E_FAIL;

	kws_log(TEXT("KModuleMgr::ScanModule file: %s"), pModule->GetPath());

	S_KSE_LITESCAN_TARGET_INFO ScanTarget;
	vector<S_KSE_LITESCAN_RESULT> ScanResult;
	ScanTarget.vecScanFiles.push_back((LPCWSTR)(pModule->GetPath()));

	int nResult = m_scanServiceWrapper.ScanFiles(m_hScanSession, ScanTarget, ScanResult);	
	if ( SUCCEEDED(nResult) )
	{
		HRESULT hErrorCode = ScanResult[0].hErrorCode;
		if ( SUCCEEDED(hErrorCode) )
		{
			EM_KSE_SECURITY_LEVAL dwLeval = ScanResult[0].emleval;
			switch (dwLeval)
			{
			case EM_KSE_SECURITY_LEVAL_SAFE:
				{
					pModule->SetSecurityState(enumKSBW_FSR_WHITE_FILE);
				}
				break;
			case EM_KSE_SECURITY_LEVAL_VIRUS:
				{
					pModule->SetSecurityState(enumKSBW_FSR_BLACK_FILE);
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
		else if ( hErrorCode == 0xE00D0003 || hErrorCode == 0xE0E90001 )
		{
			pModule->SetSecurityState(enumKSBW_FSR_NET_ERROR);
		}
		else
		{
			pModule->SetSecurityState(enumKSBW_FSR_NET_ERROR);
		}
	}
	else
	{
		pModule->SetSecurityState(enumKSBW_FSR_NET_ERROR);
	}
/*
	if (SUCCEEDED(hr = GetScanEngine()->ScanFile(pModule->GetPath(), spCallBack, spCallBackNetDetect, m_hThreadExit, &info)))
	{
		IKSEScanCallBack* pCall = spCallBack;
		KxEScanCallback* pCallBack = static_cast<KxEScanCallback*>(pCall);
		kws_log(TEXT("KNewDownFileScan::ScanItem scan file success result: %d"), pCallBack->GetScanResult());
		FILE_SCAN_RES nStatus = pCallBack->GetScanResult();
		BOOL bArchiveFile = pCallBack->IsArchiveFile();

		switch (nStatus)
		{
		case enumKSBW_FSR_BLACK_FILE:
			{
				pModule->SetSecurityState(enumKSBW_FSR_BLACK_FILE);
				if (pCallBack->GetVirusName().length() > 0)
				{
					kws_log(TEXT("KModuleMgr::ScanModule find virus: %s"), pCallBack->GetVirusName().c_str());
				}
			}
			break;
		case enumKSBW_FSR_WHITE_FILE:
			{
				pModule->SetSecurityState(enumKSBW_FSR_WHITE_FILE);
			}
			break;
		case enumKSBW_FSR_IGNORE:
			{
				pModule->SetSecurityState(enumKSBW_FSR_IGNORE);
			}
			break;
		case enumKSBW_FSR_GRAY_FILE:
			{
				pModule->SetSecurityState(enumKSBW_FSR_GRAY_FILE);
			}
			break;
		case enumKSBW_FSR_NET_ERROR:
			{
				pModule->SetSecurityState(enumKSBW_FSR_NET_ERROR);

				kws_log(TEXT("KModuleMgr::ScanModule m_pScanEngine->ScanFile failed:%08x"), pCallBack->GetErrorCode());
			}
			break;
		}
	}
	else
	{
		pModule->SetSecurityState(enumKSBW_FSR_NET_ERROR);
		kws_log(TEXT("KModuleMgr::ScanModule m_pScanEngine->ScanFile failed:%08x"), hr);
	}
*/
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