/************************************************************************
* @file      : 
* @author    : zhaoxinxing
* @date      : 2010/7/13
* @brief     : 
*
* $Id: $
/************************************************************************/

#include "stdafx.h"
#include "kregcleantask.h"
#include "kscbase/kscconv.h"
#include "kscbase/kscsys.h"


// -------------------------------------------------------------------------



// -------------------------------------------------------------------------
KRegCleanTask::KRegCleanTask()
{
    m_pCallBack = NULL;
    m_hScanThread = NULL;
    m_hScanThread = NULL;
    m_nRegistryCount = 0;
    regBack = NULL;
    TaskMap[L"RST_SHAREDDLL"] = false;
    TaskMap[L"RST_OPENSAVEMRU"] = false;
    TaskMap[L"RST_EXTHISTORY"] = false;
    TaskMap[L"RST_STARTMENU"] = false;
    TaskMap[L"RST_MUICACHE"] = false;
    TaskMap[L"RST_HELP"] = false;
    TaskMap[L"RST_APPPATH"] = false;
    TaskMap[L"RST_UNINSTALL"] = false;
    TaskMap[L"RST_FILEASSOC"] = false;
    TaskMap[L"RST_COM"] = false;

    TaskKey.push_back(L"RST_SHAREDDLL");
    TaskKey.push_back(L"RST_OPENSAVEMRU");
    TaskKey.push_back(L"RST_EXTHISTORY");
    TaskKey.push_back(L"RST_STARTMENU");
    TaskKey.push_back(L"RST_MUICACHE");
    TaskKey.push_back(L"RST_HELP");
    TaskKey.push_back(L"RST_APPPATH");
    TaskKey.push_back(L"RST_UNINSTALL");
    TaskKey.push_back(L"RST_FILEASSOC");
    TaskKey.push_back(L"RST_COM");

    
   


    
}

KRegCleanTask::~KRegCleanTask()
{
    Uninitialize();
}

HRESULT KRegCleanTask::Initialize()
{
   
    HRESULT hr = E_FAIL;
    static BOOL is_init = FALSE;

    if(!is_init)
    {
        is_init = TRUE;
    }
    hr = S_OK;
    return hr;
}

HRESULT KRegCleanTask::Uninitialize()
{
    HRESULT hr = S_OK;

    if (m_pCallBack)
    {
        m_pCallBack = NULL;
    }

    if ( m_hScanThread != NULL )
    {
        ::CloseHandle(m_hScanThread);
        m_hScanThread = NULL;
    }

    return hr;
}

void KRegCleanTask::SetCallback(ICleanCallback* pCallBack)
{
    m_pCallBack = pCallBack;
}

bool KRegCleanTask::StartClean(int nTask,const char* szTask)
{
    bool bResult = true;

    if (m_pCallBack != NULL)
        m_pCallBack->OnBegin();

    Initialize();

    SelectCleanItem(nTask,szTask);
    
    if (m_hScanThread != NULL)
    {
       goto Exit0;
    }
    
        
    regBack = new CKBackupReg;
    if (regBack == NULL)
    {
        bResult = false;
        goto Exit0;
    }
    
    m_scanResultArray.clear();
    m_nRegistryCount = 0;

    m_hScanThread = ::CreateThread(NULL, 0, &KRegCleanTask::ScanThreadProc, this, 0, NULL);
    if ( m_hScanThread == NULL )
    {
        bResult = false;
    }
  
Exit0:
    return bResult;
}

bool KRegCleanTask::StopClean()
{
    RegClean.Stop();
    if(m_pCallBack!=NULL)
        m_pCallBack->OnCancel();
    return true;
}

void KRegCleanTask::SelectCleanItem(int nTask,const char* szTask)
{
    
    if ( szTask == NULL)
        goto Exit0;
    {
    std::wstring strTmp(KANSI_TO_UTF16(szTask));
    std::vector<std::wstring> TaskVector;
    SplitStrTask(strTmp,TaskVector);

    TaskMap[L"RST_SHAREDDLL"] = false;
    TaskMap[L"RST_OPENSAVEMRU"] = false;
    TaskMap[L"RST_EXTHISTORY"] = false;
    TaskMap[L"RST_STARTMENU"] = false;
    TaskMap[L"RST_MUICACHE"] = false;
    TaskMap[L"RST_HELP"] = false;
    TaskMap[L"RST_APPPATH"] = false;
    TaskMap[L"RST_UNINSTALL"] = false;
    TaskMap[L"RST_FILEASSOC"] = false;
    TaskMap[L"RST_COM"] = false;
    
    for(std::vector<std::wstring>::iterator iter = TaskVector.begin();iter!=TaskVector.end();iter++)
    {
        
        std::map<std::wstring,bool>::iterator mapiter = TaskMap.find(*iter);
        if(mapiter!=TaskMap.end())
        {
            mapiter->second = true;
        }
        
    }

    
    for (int i = CRegistryData::RST_BEGIN; i < (int)CRegistryData::RST_END; ++i)
    {
        if(TaskMap[TaskKey.at(i)])
            m_CleanMap[(CRegistryData::REG_SCAN_TYPE)i] = true;
        else
            m_CleanMap[(CRegistryData::REG_SCAN_TYPE)i] = false;
    }
    }
Exit0:
    return;
}

DWORD WINAPI KRegCleanTask::ScanThreadProc(LPVOID lpParameter)
{
	

	KRegCleanTask*	lpThis = static_cast<KRegCleanTask*>(lpParameter);

	if ( lpThis == NULL )
		return 0;

	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);



	// 扫描
	 lpThis->RegClean.Scan(&KRegCleanTask::ScanTypeCallBack, lpThis, &KRegCleanTask::ScanCallback, lpThis);
	
	 lpThis->RegClean.Clean(&KRegCleanTask::ScanTypeCallBack,lpThis);

     if(lpThis->m_pCallBack!=NULL)
         lpThis->m_pCallBack->OnEnd();

    if(lpThis->regBack != NULL)
    {
        delete lpThis->regBack;
        InterlockedExchange((LONG*)&lpThis->regBack, NULL);
    }
    InterlockedExchange((LONG*)&lpThis->m_hScanThread, NULL);
	return 0;
}

// 扫描回调函数
bool KRegCleanTask::ScanCallback(CRegistryData::SCAN_RESULT* result, LPVOID lpParam)
{
    KRegCleanTask* lpThis = static_cast<KRegCleanTask*>(lpParam);
    //CRegistryData::REG_SCAN_TYPE rstType;
    tstring	strKeyPath;
   
    if ( lpThis == NULL || result == NULL )
        return false;

    {
        KAutoLock	lockTree(lpThis->m_lockTree);
        lpThis->m_scanResultArray[result->rstType].push_back(result);
        result->scanData.GetKeyFullPath(strKeyPath, true);

        if(strKeyPath.find_last_of(L'\\') == strKeyPath.length() - 1)
        {
            strKeyPath.erase(strKeyPath.length()-1);
        }
//         lpThis->regBack->Init();
//         lpThis->regBack->KscBackupReg(result->scanData.hRootKey, result->scanData.strSubKey, result->scanData.strValueName);
//         lpThis->regBack->Unit();
        
        
        // 删除节点
        result->bToClean = TRUE;
        lpThis->m_nRegistryCount++;
        if(lpThis->m_pCallBack!=NULL)
            lpThis->m_pCallBack->OnCleanEntry((int)result->strTitle.size(),result->strTitle.c_str(),(int)wcslen(strKeyPath.c_str()),strKeyPath.c_str(),lpThis->m_nRegistryCount);

// 		if ( result->cleaner != NULL )
// 		{
// 			(lpThis->RegClean.*result->cleaner)(result->scanData);
// 		}
    }

    return true;
}


bool KRegCleanTask::ScanTypeCallBack(CRegistryData::REG_SCAN_TYPE rstType, LPVOID lpParam)
{
    KRegCleanTask* lpThis = static_cast<KRegCleanTask*>(lpParam);
    CLEAN_MAP::iterator	iter;

    if ( lpThis == NULL )
        return false;

    iter = lpThis->m_CleanMap.find(rstType);
    if (iter == lpThis->m_CleanMap.end() || !iter->second)
        return false;

    // 建立此类别的数据
    {
        KAutoLock		lockTree(lpThis->m_lockTree);
        VSR				v;
        MTRP::iterator	iterArray;

        iterArray = lpThis->m_scanResultArray.find(rstType);
        if ( iterArray == lpThis->m_scanResultArray.end() )
        {
            lpThis->m_scanResultArray.insert(std::make_pair(rstType, v));
           
        }
    }
    return true;
}

bool trims( const std::wstring& str, std::vector <std::wstring>& vcResult, char c)
{
    size_t fst = str.find_first_not_of( c );
    size_t lst = str.find_last_not_of( c );

    if( fst != std::wstring::npos )
        vcResult.push_back(str.substr(fst, lst - fst + 1));

    return true;
}

bool KRegCleanTask::SplitStrTask( 
                   /*[in]*/  std::wstring str, 
                  /*[out]*/ std::vector <std::wstring>& vcResult,
                  /*[in]*/  char delim
                  )
{
    size_t nIter = 0;
    size_t nLast = 0;
    std::wstring v;

    while (true)
    {
        nIter = str.find(delim, nIter); 
        trims(str.substr(nLast, nIter - nLast), vcResult,' ');
        if( nIter == std::wstring::npos )
            break;

        nLast = ++nIter;
    }
    return true;

}

bool __cdecl GetCleaner(const GUID& riid, void** ppv)
{
    bool bRet = false;
    KRegCleanTask *IRegClean = NULL;

    if (!ppv)
        goto Exit0;

    IRegClean = new KRegCleanTask;
    if (!IRegClean)
        goto Exit0;

    *ppv = IRegClean;
Exit0:
    return bRet;
}



// -------------------------------------------------------------------------
// $Log: $
