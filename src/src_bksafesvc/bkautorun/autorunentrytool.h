/**
* @file    autorunentrytool.h
* @brief   ...
* @author  zhangrui
* @date    2009-04-26  17:08
*/

#ifndef AUTORUNENTRYTOOL_H
#define AUTORUNENTRYTOOL_H

#include "winmod\winpath.h"
#include "skylark2\skylarkbase.h"
#include "autorunentry.h"



using namespace WinMod;

// entry 的注册宏


// entry 表开头
#define SLATR_ENTRY_BEGIN(entry_name__)         const SLATR_ENTRY_DATA entry_name__[] = {
// entry 表结尾
#define SLATR_ENTRY_END()                       {SLATR_None, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0}};

// entry 注册表值
#define SLATR_ENTRY_REG(id__, scan_entry__, disable_entry__, fix_entry__, root__, parent__, key__, value__, zero_files)  \
    {id__, scan_entry__, disable_entry__, fix_entry__, root__, parent__, key__, value__, zero_files, 0},

// entry 启动目录
#define SLATR_ENTRY_SHFOLDER(id__, scan_entry__, csidl__)  \
    {id__, scan_entry__, NULL, NULL, NULL, NULL, NULL, NULL, NULL, csidl__},



#define SLATR_ENTRY_BREAK() \
    {SLATR_None, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0},

#define SLATR_ENTRY(id__, scan_entry__, disable_entry__, fix_entry__)  \
    {id__, scan_entry__, disable_entry__, fix_entry__, NULL, NULL, NULL, NULL, NULL, 0},

//#define SLATR_REGKEY_CURRENT_CONTROL_SET_SERIVCES   L"SYSTEM\\CurrentControlSet\\Services"




class CAtrEntryTool
{
public:


    static BOOL    IsValidEntry(const SLATR_ENTRY_DATA* pEntry);

    static CString GetRegPath(const SLATR_ENTRY_DATA* pEntry);




    static const SLATR_ENTRY_DATA*  GetEntryTable();

    static DWORD                    GetEntryTableCount();

    static const SLATR_ENTRY_DATA*  FindByEntryID(
        const SLATR_ENTRY_DATA*     pFirstEntry,
        DWORD                       dwEntryID);



    static LPCWSTR  GetHKEYRootString(HKEY hRootKey);

    static BOOL     IsDiskInDrive(LPCWSTR lpszRootPath);


    //////////////////////////////////////////////////////////////////////////
    // 以下为Entry的表扫描方式

    // 遍历注册表键下的值
    static HRESULT  EnumRegValues(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    // 遍历一层子键,并扫描子键的指定键值
    static HRESULT  EnumSubKeyAndScanRegValue(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    // 遍历两层子键,并扫描子键的指定键值
    static HRESULT  EnumSubSubKeyAndScanRegValue(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    // EnumSubKeyAndScanRegKeyForCLSID + EnumValueNameForCLSID
    static HRESULT  EnumCLSID(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    // 遍历一层子键,并扫描子键的键名(CLSID)
    static HRESULT  EnumSubKeyAndScanRegKeyForCLSID(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    // 遍历一层子键,并扫描子键的键值(CLSID)
    static HRESULT  EnumSubKeyAndScanRegValueForCLSID(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    // 遍历注册表键下的值
    static HRESULT  EnumValueNameForCLSID(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    // 遍历注册表键下的值
    static HRESULT  EnumValueForCLSID(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);



    // 扫描注册表键下的指定值(REG_SZ)
    static HRESULT  ScanRegValue(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    // 扫描注册表键下的指定值(REG_MULTI_SZ)
    static HRESULT  ScanRegValueAsMultiString(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    // 扫描注册表键下的指定值(REG_BINARY)
    static HRESULT  ScanRegValueAsBinary(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    // 扫描 HKLM\SYSTEM\CurrentControlSet\Control\NetworkProvider\Order
    static HRESULT  ScanNetworkProvider(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    //////////////////////////////////////////////////////////////////////////
    // 对服务的处理
    static HRESULT  EnumServices(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);
    static HRESULT  DisableService(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);
    static HRESULT  RemoveService(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    //////////////////////////////////////////////////////////////////////////
    // 对计划任务的处理
    static HRESULT  EnumScheduledTasks(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);
    static HRESULT  DoEnumScheduledTasks(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);
    static HRESULT  DoEnumScheduledTasksInVista(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);

    //////////////////////////////////////////////////////////////////////////
    // AutorunInf
    static HRESULT  ScanAutorunInf(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);


    //////////////////////////////////////////////////////////////////////////
    // 对启动目录的处理
    static HRESULT  ScanShellFolder(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry);
};



#endif//AUTORUNENTRYTOOL_H