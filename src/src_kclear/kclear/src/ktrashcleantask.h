/************************************************************************
* @file      : ktrashcleantask.h
* @author    : zhaoxinxing <xinxing.zh@gmail.com>
* @date      : 2010/7/13
* @brief     : 
*
* $Id: $
/************************************************************************/
#ifndef __KTRASHCLEANTASK_H__
#define __KTRASHCLEANTASK_H__


#include "kclear/ionekeyclean.h"
#include "kclear/kclsiddef.h"
#include "common/kavcom.h"
#include "configdata.h"
#include "filelistdata.h"
#include "trashdefine.h"
#include "deletefiletraverse.h"
#include "trashcleaner.h"


// -------------------------------------------------------------------------



class KTrashCleanTask : public ICleanTask
{
public:
    KTrashCleanTask();
    ~KTrashCleanTask();

   /* KIS_DEFINE_GETCLSID(CLSID_CLEAN);

    DECLARE_KISCOM_OBJECT();

    KISCOM_QUERY_BEGIN()
        KISCOM_QUERY_ENTRY(ICleanTask)
    KISCOM_QUERY_END()*/

public:

    //初始化
    HRESULT __stdcall Initialize();

    //反初始化
    HRESULT __stdcall Uninitialize();

    //设置回调
    void __stdcall SetCallback(ICleanCallback* pCallBack);

   //停止清除
    bool __stdcall StopClean();

    //开始清除
    bool __stdcall StartClean(int nTask,const char* szTask);

    void __stdcall Release()
    {
        delete this;
    }

private:

    BOOL IsCleaning() const;
    void RunCleanFile();
    static DWORD WINAPI CleanFileThread(LPVOID lpVoid);

    bool SplitStrTask( 
        /*[in]*/  std::wstring str, 
        /*[out]*/ std::vector <std::wstring>& vcResult,
        /*[in]*/  char delim = ','
        );

 
    ICleanCallback* m_pCallBack;       //回调
    CConfigData m_config;
    CFileListData  m_FileListData;
    HANDLE m_hThread;
    DWORD  m_dwThreadID;
    CConfigData::DirWorkArray m_DirWorks;
    DWORD m_ulTotalJunkFileSize;
    int m_nCurrentIndex;
    CFileDelete DelFile;
    std::map<std::wstring,bool> TaskMap;
    std::vector<std::wstring> TaskVector;
    std::vector<std::wstring> TaskDesc;
    std::wstring strDesc;
  
};

bool __cdecl GetCleaner(const GUID& riid, void** ppv)
{
    bool bRet = false;
    KTrashCleanTask *ITrashClean = NULL;

    if (!ppv)
        goto Exit0;

    ITrashClean = new KTrashCleanTask;
    if (!ITrashClean)
        goto Exit0;
   
    *ppv = ITrashClean;
Exit0:
    return bRet;
}

// -------------------------------------------------------------------------
// $Log: $

#endif /* __CKSSCANPLUG_H__ */
