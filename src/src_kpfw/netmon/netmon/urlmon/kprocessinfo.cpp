////////////////////////////////////////////////////////////////////////////////
//      
//      File for url moniter service
//      
//      File      : kprocessinfo.cpp
//      Version   : 1.0
//      Comment   : 木马网络防火墙的管理一个进程内的模块和其他安全信息
//      
//      Create at : 2010-10-8
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "kprocessinfo.h"
#include "kpfw/msg_logger.h"
#include "kmodulemgr.h"
#include "kurlcachemgr.h"
#include "kpfw/fwproxy_public.h"
#include "kpfw/ktdifilt_public.h"
#include "simpleipc/KwsIPCWarpper.h"

//////////////////////////////////////////////////////////////////////////
KProcImage::KProcImage(KModuleInfo* pModule, ULONGLONG nLoadTime):
							m_pModule(pModule), 
							m_nLoadTime(nLoadTime)
{

}

KProcImage::~KProcImage()
{

}

BOOL KProcImage::IsSameModule(KModuleInfo* pInfo)
{
	if (m_pModule)
	{
		if (m_pModule == pInfo)
			return TRUE;
		if (m_pModule->GetID() == pInfo->GetID())
			return TRUE;
	}
	return FALSE;
}
BOOL KProcImage::IsLaterThan(KProcImage* pImage)
{
	return m_nLoadTime >= pImage->m_nLoadTime;
}
DWORD KProcImage::GetSecurityState()
{
	return m_pModule->GetSecurityState();
}
BOOL KProcImage::IsSafe()
{
	return m_pModule->IsSafe();
}
BOOL KProcImage::IsUnkown()
{
	return m_pModule->IsUnkown();
}
BOOL KProcImage::IsDanger()
{
	return m_pModule->IsDanger();
}
BOOL KProcImage::IsUnScan()
{
	return m_pModule->IsUnScan();
}
BOOL KProcImage::GetModuleStastic(DWORD& nUnkownCnt, DWORD& nDangerCnt, DWORD& nUnScanCnt)
{
	return m_pModule->GetModuleStastic(nUnkownCnt, nDangerCnt, nUnScanCnt);
}
LPCWSTR	KProcImage::GetPath()
{
	return m_pModule->GetPath();
}
BOOL KProcImage::IsSkipSoftware()
{
	return m_pModule->IsSkipSoftware();
}

//////////////////////////////////////////////////////////////////////////
KProcessInfo::KProcessInfo(ULONGLONG nPID, ULONGLONG nCreateTime, KUrlCacheMgr* pUrlMgr): 
						m_nPID(nPID),
						m_nCreateTime(nCreateTime),
						m_pExeImage(NULL),
						m_nLastImageLoadTime(0),
						m_nLastUrlSendTime(0),
						m_pUrlCacheMgr(pUrlMgr),
                        m_bFirstCheck(TRUE),
                        m_bSpecialProc(FALSE),
						m_nUpdateCounter(0)
{

}

KProcessInfo::~KProcessInfo()
{
	ClearModuleList();
}

BOOL KProcessInfo::HasWaitingProcess()
{
	kis::KLocker	_locker(m_lock);

	return FALSE;
}

HRESULT	KProcessInfo::Process()
{
	return S_OK;
}

void KProcessInfo::ClearModuleList()
{
	kis::KLocker	_locker(m_lock);
	for (size_t i = 0; i < m_ModuleList.size(); i++)
		delete m_ModuleList[i];
	m_ModuleList.clear();
	if (m_pExeImage)
	{
		delete m_pExeImage;
		m_pExeImage = NULL;
	}
	m_nLastImageLoadTime = 0;
}

BOOL KProcessInfo::HasExeImage()
{
	kis::KLocker	_locker(m_lock);
	return (m_pExeImage != NULL);
}

BOOL KProcessInfo::SetExeImage(KModuleInfo* pModule)
{
	kis::KLocker	_locker(m_lock);
	if (!m_pExeImage)
	{
		m_pExeImage = new KProcImage(pModule, m_nCreateTime);
		pModule->SetTestChange(TRUE);
		return TRUE;
	}
	return FALSE;
}

KModuleInfo* KProcessInfo::GetExeImage()
{
	kis::KLocker	_locker(m_lock);
	if (m_pExeImage)
		return m_pExeImage->GetModule();
	return NULL;
}

BOOL KProcessInfo::OnLoadImage(ULONGLONG nLoadTime, KModuleInfo* pModule)
{
	kis::KLocker	_locker(m_lock);
	KProcImage* pImage = new KProcImage(pModule, nLoadTime);
	AddImageToList(pImage, nLoadTime);
	return TRUE;
}

void KProcessInfo::AddImageToList(KProcImage* pImage, ULONGLONG nLoadTime)
{
	// 按照时间顺序插入列表中
	if (nLoadTime >= m_nLastImageLoadTime)
	{
		m_ModuleList.push_back(pImage);
		m_nLastImageLoadTime = nLoadTime;
	}
	else
	{
		BOOL bFind = FALSE;
		vector<KProcImage*>::iterator it = m_ModuleList.begin();
		while (it != m_ModuleList.end())
		{
			KProcImage* pImage1 = *it;
			if (pImage1->IsLaterThan(pImage))
			{
				m_ModuleList.insert(it, pImage);
				bFind = TRUE;
				break;
			}
			it++;
		}

		if (!bFind)
		{
			kws_log(TEXT("KProcessInfo::AddImageToList find a image load time error"));
			m_ModuleList.push_back(pImage);
		}
	}
}

void KProcessInfo::GetModuleStastic(DWORD& nUnkownCnt, DWORD& nDangerCnt, DWORD& nUnScanCnt)
{
	kis::KLocker	_locker(m_lock);

	if  (!m_ModuleList.empty())
	{
		vector<KProcImage*>	ImageLeft;
		size_t i = 0;
		while (i < m_ModuleList.size())
		{
			KProcImage* pImage = m_ModuleList[i];
			if (pImage->IsSafe())
			{
				delete pImage;
			}
			else
			{
				pImage->GetModuleStastic(nUnkownCnt, nDangerCnt, nUnScanCnt);
				ImageLeft.push_back(pImage);
			}
			i++;
		}
		m_ModuleList = ImageLeft;
	}
}

BOOL KProcessInfo::IsSkipSoftware()
{
	kis::KLocker	_locker(m_lock);
	if (m_pExeImage)
		return m_pExeImage->IsSkipSoftware();
	return FALSE;
}

LPCWSTR	KProcessInfo::GetProcPath()
{
	kis::KLocker	_locker(m_lock);
	if (m_pExeImage)
		return m_pExeImage->GetPath();
	return NULL;
}

DWORD KProcessInfo::GetProcSecurity()
{
	kis::KLocker	_locker(m_lock);
	if (m_nPID == 4 || m_nPID == 8 || m_nPID == 0)
		return enumKSBW_FSR_WHITE_FILE;
	if (m_pExeImage)
		return m_pExeImage->GetSecurityState();
	return 0;
}

BOOL KProcessInfo::IsSpecialProc()
{
    LPCWSTR pwcszPath = NULL;
    LPWSTR  pwszFileName = NULL;

    if (!m_bFirstCheck)
    {
        return m_bSpecialProc;
    }

    m_bFirstCheck = FALSE;

    pwcszPath = GetProcPath();
    if (NULL == pwcszPath)
    {
        return FALSE;
    }
    
    pwszFileName = PathFindFileNameW(pwcszPath);
    if (NULL == pwszFileName)
    {
        return FALSE;
    }

    if (wcsicmp(pwszFileName, L"wscript.exe") == 0 ||
        wcsicmp(pwszFileName, L"rundll32.exe") == 0)
    {
        m_bSpecialProc = TRUE;
        return TRUE;
    }

    return FALSE;
}

HRESULT	KProcessInfo::GetProcInfo(KProcInfoItem* pItem)
{
	LPCWSTR strPath = GetProcPath();
	if (strPath)
	{
		memset(pItem, 0, sizeof(KProcInfoItem));
		pItem->m_nCbSize = sizeof(KProcInfoItem);
		pItem->m_nCreateID = m_nCreateTime;
		pItem->m_nProcessID = (DWORD)m_nPID;
		if (m_pExeImage)
			pItem->m_nTrustMode = m_pExeImage->GetSecurityState();
		if (wcslen(strPath) <= MAX_PATH)
			wcscpy_s(pItem->m_strProcPath, MAX_PATH, strPath);
		return S_OK;
	}
	return E_FAIL;
}