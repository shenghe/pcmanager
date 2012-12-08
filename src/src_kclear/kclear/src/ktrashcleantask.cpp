/************************************************************************
* @file      : 
* @author    : zhaoxinxing
* @date      : 2010/7/13
* @brief     : 
*
* $Id: $
/************************************************************************/

#include "stdafx.h"
#include "ktrashcleantask.h"
#include "trashpublicfunc.h"
#include "filtertool.h"
#include "safeexam/beikesafeexam.h"
#include "beikesafe/bkmsgdefine.h"
#include "beikesafe/bksafeexamcall.h"
#include "iefix/iefixeng.h"
extern HWND g_hMainHwnd;
// -------------------------------------------------------------------------



// -------------------------------------------------------------------------
KTrashCleanTask::KTrashCleanTask()
{
    m_pCallBack = NULL;
    m_hThread = NULL;
    m_dwThreadID = 0;
    
    
	TaskMap[L"IE_TEMP"] = false;
	TaskMap[L"WIN_TEMP"] = false;
	TaskMap[L"WIN_SUOLIETU"] = false;
    TaskMap[L"WIN_HUISHOUZHAN"] = false;
    TaskMap[L"WIN_UPDATE"] = false;
    TaskMap[L"OFFICE_CACHE"] = false;
    TaskMap[L"WIN_PRE"] = false;
    TaskMap[L"WIN_DOWNLOAD"] = false;
    TaskMap[L"U_TEMP"] = false;
   // TaskMap[L"SYS_DRIVE"] = false;
	TaskMap[L"WIN_ERRPORT"] = false;
	TaskMap[L"WIN_LOG"] = false;

    

	TaskVector.push_back(L"IE_TEMP");
	TaskVector.push_back(L"WIN_TEMP");
	TaskVector.push_back(L"WIN_SUOLIETU");
    TaskVector.push_back(L"WIN_HUISHOUZHAN");
    TaskVector.push_back(L"WIN_UPDATE");
    TaskVector.push_back(L"OFFICE_CACHE");
    TaskVector.push_back(L"WIN_PRE");
    TaskVector.push_back(L"WIN_DOWNLOAD");
    TaskVector.push_back(L"U_TEMP");
   // TaskVector.push_back(L"SYS_DRIVE");
	TaskVector.push_back(L"WIN_ERRPORT");
	TaskVector.push_back(L"WIN_LOG");

	TaskDesc.push_back(L"网页临时文件夹");
	TaskDesc.push_back(L"Windows临时目录");
	TaskDesc.push_back(L"缩略图缓存");
    TaskDesc.push_back(L"回收站");
    TaskDesc.push_back(L"Windows更新");
    TaskDesc.push_back(L"OFFICE缓存");
    TaskDesc.push_back(L"Windows预读文件");
    TaskDesc.push_back(L"Windows下载目录");
    TaskDesc.push_back(L"用户临时目录");
    //TaskDesc.push_back(L"系统盘深度搜索");
	TaskDesc.push_back(L"Windows错误报告");
	TaskDesc.push_back(L"系统日志文件");

    strDesc = L"";

    
}

KTrashCleanTask::~KTrashCleanTask()
{
}

HRESULT KTrashCleanTask::Initialize()
{
   
    HRESULT hr = E_FAIL;
    if(m_config.GetConfigData().size() == 0)
    {
        m_config.ReadOneKeyConfig();
        m_config.ConvToLocatePath();
        m_FileListData.m_itemArray.resize(m_config.m_ConfigData.size() );
    }
   
    hr = S_OK;
    return hr;
}

HRESULT KTrashCleanTask::Uninitialize()
{
    HRESULT hr = E_FAIL;

    if (m_pCallBack)
    {
        m_pCallBack = NULL;
    }
    return hr;
}

void KTrashCleanTask::SetCallback(ICleanCallback* pCallBack)
{
    m_pCallBack = pCallBack;
}

bool KTrashCleanTask::StartClean(int nTask,const char* szTask)
{

    if(m_pCallBack != NULL)
         m_pCallBack->OnBegin();
    CConfigData::DirWorkArray::iterator iter ;
    bool bResult = TRUE;


    Initialize();

    TaskMap[L"IE_TEMP"] = false;
    TaskMap[L"WIN_TEMP"] = false;
	TaskMap[L"WIN_HUISHOUZHAN"] = false;
    TaskMap[L"WIN_LOG"] = false;
    TaskMap[L"WIN_UPDATE"] = false;
    TaskMap[L"OFFICE_CACHE"] = false;
    TaskMap[L"WIN_PRE"] = false;
    TaskMap[L"WIN_DOWNLOAD"] = false;
    TaskMap[L"U_TEMP"] = false;
    TaskMap[L"SYS_DRIVE"] = false;
	TaskMap[L"WIN_ERRPORT"] = false;
	TaskMap[L"WIN_LOG"] = false;
	TaskMap[L"WIN_SUOLIETU"] = false;

    if (szTask == NULL)
        goto Exit0;
    {
        std::wstring strTmp(KANSI_TO_UTF16(szTask));
        std::vector<std::wstring> TaskVector;
        SplitStrTask(strTmp,TaskVector);

        for(std::vector<std::wstring>::iterator iter_str = TaskVector.begin();iter_str!=TaskVector.end();iter_str++)
        {
            std::map<std::wstring,bool>::iterator mapiter = TaskMap.find(*iter_str);
            if(mapiter!=TaskMap.end())
            {
                mapiter->second = true;
            }
         
        }
    }
    
     

    if (IsCleaning())
    {
        bResult = FALSE;
        goto Exit0;
    }

    m_ulTotalJunkFileSize = 0; 
    for (int nIndex = 0; nIndex < m_FileListData.m_itemArray.size(); nIndex++)
    {
        m_FileListData.m_itemArray.at(nIndex).ulAllFileSize = 0;
        m_FileListData.m_itemArray.at(nIndex).itemArray.clear();
    }

    m_DirWorks.resize(m_config.GetConfigData().size());
    m_DirWorks.clear();
    std::copy(m_config.GetConfigData().begin(),m_config.GetConfigData().end(),back_inserter(m_DirWorks));
    int i =0;
    iter = m_DirWorks.begin();
    while(iter != m_DirWorks.end() && i < m_DirWorks.size())
    {
       
        if(TaskMap[TaskVector.at(i)])
            iter->bDefaultSelect = TRUE;
        else iter->bDefaultSelect = FALSE;
       
        iter++;
        i++;
    }
    
    g_bStop[1] = FALSE;

    m_hThread = CreateThread(NULL, 
            0, 
            CleanFileThread, 
            (LPVOID)this,
            NULL,
            &m_dwThreadID);

    if (m_hThread == NULL)   
        bResult = FALSE;
  
Exit0:
    return bResult;
}



bool KTrashCleanTask::StopClean()
{
    g_bStop[1] = TRUE;
    if(m_pCallBack!=NULL)
        m_pCallBack->OnCancel();
    return true;
}

BOOL KTrashCleanTask::IsCleaning() const
{
    return m_hThread != NULL;
}



DWORD WINAPI KTrashCleanTask::CleanFileThread(LPVOID lpVoid)
{

    KTrashCleanTask* pThis = (KTrashCleanTask*)lpVoid;
    pThis->RunCleanFile();
	::CloseHandle(pThis->m_hThread);
    InterlockedExchange((LONG*)&pThis->m_hThread, NULL);
    InterlockedExchange((LONG*)&pThis->m_dwThreadID, 0);
    return 0;
}

void KTrashCleanTask::RunCleanFile()
{
    CString strDirectory = _T("");
	int nCleanCount = 0;
   

    for (int i = 0; i < m_DirWorks.size() && !g_bStop[1]; i++)
    {
		if (TaskDesc.size() > i)
		{
			strDesc = TaskDesc.at(i);
		}
        m_nCurrentIndex = i;
        DIRECTORYWORK& dirwork = m_DirWorks[i];
        

       
        if(dirwork.bDefaultSelect)
        { 
			nCleanCount++;
            std::vector<CString>::iterator iter = dirwork.strDirectory.begin();
			if (dirwork.strFriendName == L"回收站")
			{
				SHQUERYRBINFO syscleInfo = {0};
				syscleInfo.cbSize = sizeof(SHQUERYRBINFO);
				int iCount = SHQueryRecycleBin(NULL, &syscleInfo);
				if (syscleInfo.i64NumItems != 0)
				{
					SHEmptyRecycleBin(NULL, NULL, SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND);
					m_pCallBack->OnCleanEntry((int)strDesc.size(),strDesc.c_str(),(int)strDesc.size(),strDesc.c_str(),
						(double)syscleInfo.i64Size / ((1024) * (1024)));
				}
			}
            for(;iter!=dirwork.strDirectory.end();iter++)
            {
                strDirectory = *iter;

                if ( strDirectory.GetLength() <= 0 )
                    continue;

                
                BOOL bFilter = FALSE;

                for(int ivIndex=0;ivIndex<dirwork.strFilt.size();ivIndex++)
                {

                    if(CFilterTool::Instance()._DoFindFuncMap(dirwork.strFilt.at(ivIndex),strDirectory))
                    {
                        bFilter = TRUE;
                        break;
                    }
                }

                if(bFilter)
                {
                    continue;
                }

                if (strDirectory[ strDirectory.GetLength() - 1 ] != _T('\\'))
                    strDirectory += _T('\\');
                m_pCallBack->OnCleanEntry((int)strDesc.size(),strDesc.c_str(),
                    0,
                    NULL,
                    (double)m_ulTotalJunkFileSize / ((1024) * (1024)));
                DeleteFileInDirectory(strDirectory, 
                    dirwork.strExtArray, 
                    dirwork.bRecursive ? 5 : 0,
                    1,
                    &m_FileListData,
                    m_nCurrentIndex,
                    m_pCallBack,
                    &DelFile,
                    strDesc,
                    dirwork.strFilt);  

            }
         
        }
    }

    m_pCallBack->OnEnd();
	if (nCleanCount > 0)
	{
		REMOVE_EXAM_RESULT_ITEM RemoveItemInfo;
		RemoveItemInfo.uRemoveItem.RemoveItemInfo.Reset();
		RemoveItemInfo.uRemoveItem.RemoveItemInfo.dwItemID = BkSafeExamItem::EXAM_TRASH;
		::SendMessage(g_hMainHwnd, MSG_APP_EXAM_SCORE_CHANGE, BkSafeExamItemExOperate::RemoveItem, (LPARAM)&RemoveItemInfo );
	}
	nCleanCount = 0;
}

bool trims( const std::wstring& str, std::vector <std::wstring>& vcResult, char c)
{
    size_t fst = str.find_first_not_of( c );
    size_t lst = str.find_last_not_of( c );

    if( fst != std::wstring::npos )
        vcResult.push_back(str.substr(fst, lst - fst + 1));

    return true;
}

bool KTrashCleanTask::SplitStrTask( 
                                 /*[in]*/  std::wstring str, 
                                 /*[out]*/ std::vector <std::wstring>& vcResult,
                                 /*[in]*/  char delim
                                 )
{
    size_t nIter = 0;
    size_t nLast = 0;
    std::wstring v;

    while( true )
    {
        nIter = str.find(delim, nIter); 
        trims(str.substr(nLast, nIter - nLast), vcResult,' ');
        if( nIter == std::wstring::npos )
            break;

        nLast = ++nIter;
    }
    return true;

}



// -------------------------------------------------------------------------
// $Log: $
