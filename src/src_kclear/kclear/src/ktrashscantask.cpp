/************************************************************************
* @file      : 
* @author    : zhaoxinxing
* @date      : 2010/7/13
* @brief     : 
*
* $Id: $
/************************************************************************/

#include "stdafx.h"
#include "ktrashscantask.h"
#include "trashpublicfunc.h"
#include "filtertool.h"
#include "stubbornfiles.h"
// -------------------------------------------------------------------------



// -------------------------------------------------------------------------
KTrashScanTask::KTrashScanTask()
{
   
    m_hThread = NULL;
    m_dwThreadID = 0;

	TaskMap[L"IE_TEMP"] = true;
	TaskMap[L"WIN_TEMP"] = true;
	TaskMap[L"WIN_SUOLIETU"] = true;
	TaskMap[L"WIN_HUISHOUZHAN"] = true;
	TaskMap[L"WIN_UPDATE"] = true;
	TaskMap[L"OFFICE_CACHE"] = false;
	TaskMap[L"WIN_PRE"] = true;
	TaskMap[L"WIN_DOWNLOAD"] = false;
	TaskMap[L"U_TEMP"] = true;
	TaskMap[L"WIN_ERRPORT"] = true;
	TaskMap[L"WIN_LOG"] = true;



	TaskVector.push_back(L"IE_TEMP");
	TaskVector.push_back(L"WIN_TEMP");
	TaskVector.push_back(L"WIN_SUOLIETU");
	TaskVector.push_back(L"WIN_HUISHOUZHAN");
	TaskVector.push_back(L"WIN_UPDATE");
	TaskVector.push_back(L"OFFICE_CACHE");
	TaskVector.push_back(L"WIN_PRE");
	TaskVector.push_back(L"WIN_DOWNLOAD");
	TaskVector.push_back(L"U_TEMP");
	TaskVector.push_back(L"WIN_ERRPORT");
	TaskVector.push_back(L"WIN_LOG");
    
}

KTrashScanTask::~KTrashScanTask()
{
}

HRESULT KTrashScanTask::Initialize()
{
   
    HRESULT hr = E_FAIL;

    if(m_config.GetConfigData().size() == 0)
    {
        m_config.ReadOneKeyConfig();
        m_config.ConvToLocatePath();
//		m_FileListData.m_itemArray.resize(m_config.m_ConfigData.size());
		int len = VIDEO_QQLIVE;  // 暂时先给定的大小设为最多的那项
		m_FileListData.m_itemArray.resize(len + 1);
    }
    
    hr = S_OK;
    return hr;
}

HRESULT KTrashScanTask::Uninitialize()
{
    HRESULT hr = E_FAIL;

    return hr;
}



bool KTrashScanTask::StartScan()
{
    CConfigData::DirWorkArray::iterator iter ;
    bool bResult = TRUE;
    Initialize();

    if (IsScaning())
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
       if ( i >= TaskVector.size() )
		   break;

        if(TaskMap[TaskVector.at(i)])
            iter->bDefaultSelect = TRUE;
        else iter->bDefaultSelect = FALSE;
       
        iter++;
        i++;
    }
    
    g_bStop[2] = FALSE;

    ScanFileThread(this);


    if (m_hThread == NULL)   
        bResult = FALSE;
  
Exit0:
    return bResult;
}



bool KTrashScanTask::StopScan()
{
    g_bStop[2] = TRUE;
    return true;
}

BOOL KTrashScanTask::IsScaning() const
{
    return m_hThread != NULL;
}


DWORD WINAPI KTrashScanTask::ScanFileThread(LPVOID lpVoid)
{

    KTrashScanTask* pThis = (KTrashScanTask*)lpVoid;
    pThis->RunScanFile();
	::CloseHandle(pThis->m_hThread);
    InterlockedExchange((LONG*)&pThis->m_hThread, NULL);
    InterlockedExchange((LONG*)&pThis->m_dwThreadID, 0);
    return 0;
}

void KTrashScanTask::RunScanFile()
{
    CString strDirectory = _T("");
   

    for (int i = 0; i < m_DirWorks.size() && !g_bStop[2]; i++)
    {
       
        m_nCurrentIndex = i;
        DIRECTORYWORK& dirwork = m_DirWorks[i];
       
        if(dirwork.bDefaultSelect)
        { 
            std::vector<CString>::iterator iter = dirwork.strDirectory.begin();
            for(;iter!=dirwork.strDirectory.end();iter++)
            {
                strDirectory = *iter;

                if ( strDirectory.GetLength() <= 0 && dirwork.id != WIN_HUISHOUZHAN)
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
                if (dirwork.id != WIN_HUISHOUZHAN)
				{
					if ( strDirectory[ strDirectory.GetLength() - 1 ] != _T('\\') )
						strDirectory += _T('\\');
				}
                FindFileInDirectory(strDirectory, 
                    dirwork.strExtArray, 
                    dirwork.bRecursive ? 15 : 0,
                    2,
                    NULL,
                    &m_FileListData,
                    dirwork.id,
                    dirwork.strFilt,
					dirwork.strFriendName);  
            }
        }
    }

    m_ulTotalJunkFileSize = 0; 

    CStubbornFiles StuFile;

    StuFile.Init();

    for (int nIndex = 0; nIndex < m_FileListData.m_itemArray.size(); nIndex++)
    {
        for(int test=0;test<m_FileListData.m_itemArray.at(nIndex).itemArray.size();test++)
        {
            
            if(!StuFile.IsStubbornFile(m_FileListData.m_itemArray.at(nIndex).itemArray.at(test).strFileName))
            {

              // ::OutputDebugString(m_FileListData.m_itemArray.at(nIndex).itemArray.at(test).strFileName);
                m_ulTotalJunkFileSize += m_FileListData.m_itemArray.at(nIndex).itemArray.at(test).ulFileSize;
            }

        }
            
        
    }

    StuFile.UnInit();


}




// -------------------------------------------------------------------------
// $Log: $
