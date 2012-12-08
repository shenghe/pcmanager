#include <stdafx.h>
#include "examimp.h"
#include "filtertool.h"
#include "trashpublicfunc.h"
#include "stubbornfiles.h"
#include "dlgsetting.h"
#include "kvideocache.h"

//////////////////////////////////////////////////////////////////////////

const unsigned long DEF_LIMIT_REPORT_SIZE_MB = (1024*1024);

//////////////////////////////////////////////////////////////////////////
// 体检是否有垃圾
bool __stdcall HasTrash()
{
    bool retval = false;
    ULONGLONG uLimit = 0;
    KExamScanner trashscanner;
    CString strDebugMsg;

    SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

    trashscanner.StartScan();
    
    strDebugMsg.Format(_T("%I64d"), trashscanner.GetFileSize());
    ::OutputDebugString(strDebugMsg);
    
    uLimit = trashscanner.GetLimit() * DEF_LIMIT_REPORT_SIZE_MB;

    if (trashscanner.GetFileSize() > uLimit)
        retval = true;

    return retval;
}

//////////////////////////////////////////////////////////////////////////
// 设置体检通知窗体
bool __stdcall SetTrashNotifyHwnd(HWND hNotifyWnd)
{
    bool retval = false;
    BOOL bAlreadyExisted = FALSE;
    HRESULT hr;
    CAtlFileMapping<HWND> *pNotifyWndMapping = NULL;

    SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

    pNotifyWndMapping = new CAtlFileMapping<HWND>();
    if (!pNotifyWndMapping)
        goto clean0;

    hr = pNotifyWndMapping->MapSharedMem(
        sizeof(HWND), _T("{B0ED42F4-56C7-4fc6-9793-C5F863459F24}"), &bAlreadyExisted);
    if (FAILED(hr))
        goto clean0;

    *(HWND*)pNotifyWndMapping->GetData() = hNotifyWnd;

    retval = true;

clean0:
    if (bAlreadyExisted)
    {
        delete pNotifyWndMapping;
        pNotifyWndMapping = NULL;
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////
// 获得体检通知窗体
HWND __stdcall GetTrashNotifyHwnd()
{
    HWND retval = NULL;
    BOOL bAlreadyExisted = FALSE;
    HRESULT hr;
    CAtlFileMapping<HWND> *pNotifyWndMapping = NULL;

    pNotifyWndMapping = new CAtlFileMapping<HWND>();
    if (!pNotifyWndMapping)
        goto clean0;

    hr = pNotifyWndMapping->MapSharedMem(
        sizeof(HWND), _T("{B0ED42F4-56C7-4fc6-9793-C5F863459F24}"), &bAlreadyExisted);
    if (FAILED(hr))
        goto clean0;

    if (!bAlreadyExisted)
        goto clean0;

    retval = *(HWND*)pNotifyWndMapping->GetData();

clean0:
    if (pNotifyWndMapping)
    {
        delete pNotifyWndMapping;
        pNotifyWndMapping = NULL;
    }

    return retval;
}

//////////////////////////////////////////////////////////////////////////


// -------------------------------------------------------------------------
KExamScanner::KExamScanner()
{
    m_hThread = NULL;
    m_dwThreadID = 0;
    m_nLimitSize = 0;

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

KExamScanner::~KExamScanner()
{
}

HRESULT KExamScanner::Initialize()
{

    HRESULT hr = E_FAIL;

    if(m_config.GetConfigData().size() == 0)
    {     
        m_config.ReadConfig();
        m_config.ConvToLocatePath();
        //		m_FileListData.m_itemArray.resize(m_config.m_ConfigData.size());
        int len = ENUM_ID_END;  // 暂时先给定的大小设为最多的那项
        m_FileListData.m_itemArray.resize(len + 1);
    }

    hr = S_OK;
    return hr;
}

HRESULT KExamScanner::Uninitialize()
{
    HRESULT hr = E_FAIL;

    return hr;
}



bool KExamScanner::StartScan()
{
    CConfigData::DirWorkArray::iterator iter ;
    KClearSettingDlg setting;
    bool bResult = TRUE;

    Initialize();

    if (IsScaning())
    {
        bResult = FALSE;
        goto Exit0;
    }
    
    setting.GetReportSize(m_nLimitSize);

    if (setting.CheckCustomSelect())
    {
        size_t iCount, jCount;
        setting.GetCustomSelected(m_vCustomSelected);
        
        for (iCount = 0; iCount < m_config.m_ConfigData.size(); ++iCount)
        {
            m_config.m_ConfigData[iCount].bDefaultSelect = FALSE;
        }

        for (iCount = 0; iCount < m_vCustomSelected.size(); ++iCount)
        {
            // 过滤掉深度扫描
            if (m_vCustomSelected[iCount] == SYS_TRASH)
                continue;

            for (jCount = 0; jCount < m_config.m_ConfigData.size(); ++jCount)
            {
                if (m_config.m_ConfigData[jCount].id == m_vCustomSelected[iCount])
                {
                    m_config.m_ConfigData[jCount].bDefaultSelect = TRUE;
                    break;
                }
            }
        }
    }
    

    m_ulTotalJunkFileSize = 0; 

    for (size_t nIndex = 0; nIndex < m_FileListData.m_itemArray.size(); nIndex++)
    {
        m_FileListData.m_itemArray.at(nIndex).ulAllFileSize = 0;
        m_FileListData.m_itemArray.at(nIndex).itemArray.clear();
    }

    g_bStop[2] = FALSE;

    ScanFileThread(this);


    if (m_hThread == NULL)   
        bResult = FALSE;

Exit0:
    return bResult;
}



bool KExamScanner::StopScan()
{
    g_bStop[2] = TRUE;
    return true;
}

BOOL KExamScanner::IsScaning() const
{
    return m_hThread != NULL;
}


DWORD WINAPI KExamScanner::ScanFileThread(LPVOID lpVoid)
{

    KExamScanner* pThis = (KExamScanner*)lpVoid;
    pThis->RunScanFile();
    ::CloseHandle(pThis->m_hThread);
    InterlockedExchange((LONG*)&pThis->m_hThread, NULL);
    InterlockedExchange((LONG*)&pThis->m_dwThreadID, 0);
    return 0;
}

void KExamScanner::RunScanFile()
{
    CString strDirectory = _T("");

    for (size_t i = 0; i < m_config.m_ConfigData.size() && !g_bStop[2]; i++)
    {
        DIRECTORYWORK& dirwork = m_config.m_ConfigData[i];
        
        m_nCurrentIndex = dirwork.id;

        if (dirwork.bDefaultSelect)
        { 

//           if (dirwork.id >= VIDEO_TUDOU && dirwork.id <= VIDEO_QQLIVE)
            {
                if (RunVideoCacheFile(dirwork.id))
                    continue;
            } 

            std::vector<CString>::iterator iter = dirwork.strDirectory.begin();
            for(; iter != dirwork.strDirectory.end(); iter++)
            {
                BOOL bFilter = FALSE;

                strDirectory = *iter;
                if (strDirectory.GetLength() <= 0 && dirwork.id != WIN_HUISHOUZHAN)
                    continue;

                for (size_t ivIndex = 0; ivIndex < dirwork.strFilt.size(); ivIndex++)
                {
                    if (CFilterTool::Instance()._DoFindFuncMap(dirwork.strFilt.at(ivIndex),strDirectory))
                    {
                        bFilter = TRUE;
                        break;
                    }
                }

                if (bFilter)
                {
                    continue;
                }
                if (dirwork.id != WIN_HUISHOUZHAN)
                {
                    if (strDirectory[strDirectory.GetLength() - 1] != _T('\\'))
                        strDirectory += _T('\\');
                }
                FindFileInDirectory(
                    strDirectory, 
                    dirwork.strExtArray, 
                    dirwork.bRecursive ? 15 : 0,
                    2,
                    NULL,
                    &m_FileListData,
                    dirwork.id,
                    dirwork.strFilt,
                    dirwork.strFriendName
                    ); 
            }
        }
    }

    m_ulTotalJunkFileSize = 0; 

    CStubbornFiles StuFile;

    StuFile.Init();

    for (size_t nIndex = 0; nIndex < m_FileListData.m_itemArray.size(); nIndex++)
    {
        for (size_t test=0; test < m_FileListData.m_itemArray.at(nIndex).itemArray.size(); test++)
        {
            if (!StuFile.IsStubbornFile(m_FileListData.m_itemArray.at(nIndex).itemArray.at(test).strFileName))
            {
                m_ulTotalJunkFileSize += m_FileListData.m_itemArray.at(nIndex).itemArray.at(test).ulFileSize;
            }
        }
    }

    StuFile.UnInit();
}

//////////////////////////////////////////////////////////////////////////
void KExamScanner::TraverseFile(LPFINDFILEDATA pFileData)
{
    m_FileListData.AddTrashFile(m_nCurrentIndex, pFileData);
}

void KExamScanner::TraverseProcess(unsigned long uCurPos)
{

}

void KExamScanner::TraverseProcessEnd(unsigned long uCurPos,CString videoPath)
{

}

void KExamScanner::TraverseFinished()
{

}

BOOL KExamScanner::RunVideoCacheFile(int id)
{
    BOOL bRet = FALSE;
    CString strPath;

    switch(id)
    {
    case VIDEO_TUDOU:
        bRet = _ScanTudouCache(this, strPath, 2);
        break;
    case VIDEO_XUNLEI:
        bRet = _ScanKankanCache(this, strPath, 2);
        break;
    case VIDEO_YOUKU:
        bRet = _ScanYoukuCache(this, strPath, 2);
        break;
    case VIDEO_KU6:
        bRet = _ScanKu6Cache(this, strPath, 2);
        break;
    case VIDEO_PPTV:
        bRet = _ScanPPTVCache(this, strPath, 2);
        break;
    case VIDEO_FENGXING:
        bRet = _ScanFengxingCache(this, strPath, 2);
        break;
    case VIDEO_QQLIVE:
        bRet = _ScanQQLiveCache(this, strPath, 2);
        break;
    case VIDEO_STORM:
        bRet = _ScanStormCache(this, strPath, 2);
        break;
    case VIDEO_PIPI:
        bRet = _ScanPIPICache(this, strPath, 2);
        break;
    case VIDEO_QVOD:
        bRet = _ScanQvodCache(this, strPath, 2);
        break;
    case VIDEO_KUGOO:
        bRet = _ScanKuGooCache(this, strPath, 2);
        break;
    case VIDEO_KUWO:
        bRet = _ScanKuwoCache(this, strPath, 2);
        break;
    default:
        {
            bRet = FALSE;
            break;
        }
    }

    return bRet;
}
