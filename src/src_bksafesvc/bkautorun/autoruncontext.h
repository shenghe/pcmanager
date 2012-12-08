/**
* @file    autoruncontext.h
* @brief   ...
* @author  zhangrui
* @date    2009-04-23  22:05
*/

#ifndef AUTORUNCONTEXT_H
#define AUTORUNCONTEXT_H

#include <atlcoll.h>
#include "winmod\winpath.h"
#include "skylark2\skylarkbase.h"
#include "autorunentry.h"
#include "autorunitem.h"
#include "autorunitem_regkey.h"
#include "autorunitem_service.h"
#include "autorunitem_scheduled_task.h"
#include "autorunitem_startup_folder_file.h"
#include "autorunitem_startup_ini.h"



using namespace WinMod;

class CAutorunEntry;

/// 启动项扫描句柄,包含当前扫描到的项目,及文件列表
class CAutorunContext: public IAutorunContext
{
public:

    typedef CAtlList<CAutorunItem*>     CItemList;
    typedef Skylark::CCopiableArray<CString>     CPathList;


    CAutorunContext();
    ~CAutorunContext();

    HRESULT Initialize();
    HRESULT Uninitialize();
    void    Reset();

    void    EnableFeedDebug(BOOL bEnable);

    BOOL    FindFirstContextItem(const SLATR_ENTRY_DATA* pEntry);
    BOOL    FindNextContextItem();
    BOOL    IsEndOfContextItemFind() const;

    HRESULT ExportTo(CAutorunContext& target) const;

    CAutorunItem* GetCurrentItem() const;




    //////////////////////////////////////////////////////////////////////////
    // 扫描辅助函数, FindXXX 函数中发现的文件以 Append 方式添加到 pathList 末尾

    // 试图查找命令行中所有的路径
    BOOL    FindAllPathInText(LPCWSTR lpszCommandLine, CPathList& pathList, CPathList* pAddionalDirList);

    // 查找命令行中第一个路径, bFirstFind用来调试
    BOOL    FindFirstPathInText(LPCWSTR lpszCommandLine, CPathList& pathList, CPathList* pAddionalDirList);

    // 查找文件列表中的每个文件
    //BOOL    FindPathInFileList(LPCWSTR lpszFileList, CPathList& pathList, LPCWSTR lpszDelimiters);

    // 添加文件,在添加前,会对文件路径作必要的处理
    void    AddPath(CPathList& pathList, LPCWSTR lpszPath);

    // 查找下一个有可能是路径的起始位置
    LPCWSTR SeekNextPossiblePathPos(LPCWSTR lpszExecFile, LPCWSTR lpszCommandLine);
    LPCWSTR SeekAfterPattern(LPCWSTR lpszCommandLine, LPCWSTR lpszPattern);

    BOOL    TrySearchPathAndExtension(CWinPath& path, CPathList* pAddionalDirList);
    BOOL    TrySupplyExtension(CWinPath& path);



    //////////////////////////////////////////////////////////////////////////
    // 以下函数由 Entry 函数调用
    virtual void STDMETHODCALLTYPE Append_RegValue_As_Startup(
        const SLATR_ENTRY_DATA* pEntry,
        HKEY                    hKeyRoot,
        LPCWSTR                 lpszRegPath,
        LPCWSTR                 lpszValueName,
        LPCWSTR                 lpszValue);

    //virtual void STDMETHODCALLTYPE Append_RegValueName_As_Startup(
    //    const SLATR_ENTRY_DATA* pEntry,
    //    HKEY                    hKeyRoot,
    //    LPCWSTR                 lpszRegPath,
    //    LPCWSTR                 lpszValueName);

    virtual void STDMETHODCALLTYPE Append_Service(
        const SLATR_ENTRY_DATA* pEntry,
        LPCWSTR                 lpszServiceName,
        DWORD                   dwServiceType,
        DWORD                   dwServiceStart,
        LPCWSTR                 lpszImagePath,
        LPCWSTR                 lpszServiceDll,
        LPCWSTR                 lpszServiceDisplayName,
        LPCWSTR                 lpszServiceDescription);


    virtual void STDMETHODCALLTYPE Append_Scheduled_Task(
        const SLATR_ENTRY_DATA* pEntry,
        LPCWSTR                 lpszTaskName,
        LPCWSTR                 lpszApplicationName);


    virtual void STDMETHODCALLTYPE Append_Startup_Folder_File(
        const SLATR_ENTRY_DATA* pEntry,
        LPCWSTR                 lpszStartupFolder,
        LPCWSTR                 lpszFilePath);

    virtual void STDMETHODCALLTYPE Append_Startup_Ini_File(
        const SLATR_ENTRY_DATA* pEntry,
        LPCWSTR                 lpszIniFile,
        LPCWSTR                 lpszIniApp,
        LPCWSTR                 lpszIniKey,
        LPCWSTR                 lpszFilePath);


    void FeedInfo(LPCWSTR lpszInformation, LPCWSTR lpszFilePath);

    void FeedDebugValue(DWORD dwFeedCode, DWORD dwEntryID, HRESULT hErrCode, LPCWSTR lpszMessage);

private:


    CItemList   m_itemList;
    POSITION    m_itemPos;

    DWORD       m_dwCurrentEntryID;

    // 下面的部分在Reset时,不会被复位
    CPathList   m_pathExtList;
    CPathList   m_searchPathList;
    BOOL        m_bDBG__;
    BOOL        m_bEnableFeedDebug;

    // 是否允许被枚举,导出的扫描句柄不应当被用来枚举
    DWORD       m_dwEnumeratale;
};




#endif//AUTORUNCONTEXT_H