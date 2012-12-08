/************************************************************************
* @file      : ktrashcleantask.h
* @author    : zhaoxinxing <xinxing.zh@gmail.com>
* @date      : 2010/7/13
* @brief     : 
*
* $Id: $
/************************************************************************/
#ifndef __KTRASHSCANTASK_H__
#define __KTRASHSCANTASK_H__

#define DEF_LIMIT_REPORT_SIZE	(160*1024*1024)

#include "configdata.h"
#include "filelistdata.h"
#include "trashdefine.h"
#include "trashcleaner.h"

// -------------------------------------------------------------------------

HWND g_hMainHwnd;

class KTrashScanTask
{
public:
    KTrashScanTask();
    ~KTrashScanTask();

  

public:

    HRESULT __stdcall Initialize();

    HRESULT __stdcall Uninitialize();

    bool __stdcall StopScan();

    bool __stdcall StartScan();

    DWORD GetFileSize()
    {
        return m_ulTotalJunkFileSize;
    }


private:

 
    BOOL IsScaning() const;
    void RunScanFile();
    static DWORD WINAPI ScanFileThread(LPVOID lpVoid);

    CConfigData m_config;
    CFileListData  m_FileListData;
    HANDLE m_hThread;
    DWORD  m_dwThreadID;
   
    CConfigData::DirWorkArray m_DirWorks;
    DWORD m_ulTotalJunkFileSize;
    int m_nCurrentIndex;
    std::map<std::wstring,bool> TaskMap;
    std::vector<std::wstring> TaskVector;
};

bool __stdcall HasTrash()
{
    bool bRet = false;
    KTrashScanTask ITrashScan;
    
    ITrashScan.StartScan();

    WCHAR Test[MAX_PATH];
    _itow_s(ITrashScan.GetFileSize(),Test,MAX_PATH,10);
    ::OutputDebugString(Test);
    if(ITrashScan.GetFileSize()> DEF_LIMIT_REPORT_SIZE)
        bRet = true;
   
    return bRet;
}


bool __stdcall SetTrashNotifyHwnd(HWND hNotifyWnd)
{
	g_hMainHwnd = hNotifyWnd;
	return true;
}

// -------------------------------------------------------------------------
// $Log: $

#endif /* __CKSSCANPLUG_H__ */
