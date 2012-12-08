/**
* @file    autorunentrytool.cpp
* @brief   ...
* @author  zhangrui
* @date    2009-04-26  17:08
*/

#include "stdafx.h"
#include "autorunentrytool.h"

#include <assert.h>
#include <shlobj.h>
#pragma  comment(lib, "shell32.lib")

// for Task Scheduler 1.0 Interfaces
#include <initguid.h>
#include <ole2.h>
#include <mstask.h>
#include <msterr.h>

// for Task Scheduler 2.0 Interfaces
#define _WIN32_DCOM
#include <comdef.h>

#if _MSV_VER >= 1400
#include <taskschd.h>
#pragma  comment(lib, "taskschd.lib")
#endif//_MSV_VER < 1400

#pragma  comment(lib, "comsupp.lib")

#include "winmod\winosver.h"
#include "winmod\winfilefind.h"
#include "winmod\winregkeyfind.h"
#include "winmod\winregvaluefind.h"
#include "winmod\winservicefind.h"

#pragma warning(disable: 4996)



SLATR_ENTRY_BEGIN(g_pEntryTable)
    //////////////////////////////////////////////////////////////////////////
    // 服务和驱动
    SLATR_ENTRY(SLATR_HKLM_Services,    CAtrEntryTool::EnumServices,        NULL,   NULL)


    //////////////////////////////////////////////////////////////////////////
    // 计划任务
    SLATR_ENTRY(SLATR_Scheduled_Tasks,  CAtrEntryTool::EnumScheduledTasks,  NULL,   NULL)                        


    //////////////////////////////////////////////////////////////////////////
    // 启动目录
    SLATR_ENTRY_SHFOLDER(SLATR_Folder_CSIDL_COMMON_STARTUP,     CAtrEntryTool::ScanShellFolder,         CSIDL_COMMON_STARTUP)
    SLATR_ENTRY_SHFOLDER(SLATR_Folder_CSIDL_STARTUP,            CAtrEntryTool::ScanShellFolder,         CSIDL_STARTUP)
    SLATR_ENTRY_SHFOLDER(SLATR_Folder_CSIDL_COMMON_ALTSTARTUP,  CAtrEntryTool::ScanShellFolder,         CSIDL_COMMON_ALTSTARTUP)
    SLATR_ENTRY_SHFOLDER(SLATR_Folder_CSIDL_ALTSTARTUP,         CAtrEntryTool::ScanShellFolder,         CSIDL_ALTSTARTUP)


    //////////////////////////////////////////////////////////////////////////
    // 注册表启动项
    //              ID                                                                      Scan Function                                       Fix Function    Root                ParentKey                                                   SubKey                                                          ValueName                           CanBeZero


    // Logon
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_TermServ_Wds_rdpwd__StartupPrograms,         CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"Terminal Server\\Wds\\rdpwd",                                 L"StartupPrograms",                 FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Winlogon__Appsetup,                            CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon",                                                    L"AppSetup",                        FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Policies_MS_Windows_System_Scripts_Startup,                  CAtrEntryTool::EnumSubSubKeyAndScanRegValue,        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__Policies_MS_Windows_System_Scripts,    L"Startup",                                                     L"Scripts",                         FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Policies_MS_Windows_System_Scripts_Logon,                    CAtrEntryTool::EnumSubSubKeyAndScanRegValue,        NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__Policies_MS_Windows_System_Scripts,    L"Logon",                                                       L"Scripts",                         FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Policies_MS_Windows_System_Scripts_Logon,                    CAtrEntryTool::EnumSubSubKeyAndScanRegValue,        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__Policies_MS_Windows_System_Scripts,    L"Logon",                                                       L"Scripts",                         FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Windows__Userinit,                             CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon",                                                    L"Userinit",                        FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_Policies_System__Shell,                           CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS_Windows,                         L"Policies\\System",                                            L"Shell",                           FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_NT_Winlogon__Shell,                               CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon",                                                    L"Shell",                           FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_Policies_System__Shell,                           CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"Policies\\System",                                            L"Shell",                           FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Winlogon__Shell,                               CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon",                                                    L"Shell",                           FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Winlogon__Taskman,                             CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon",                                                    L"Taskman",                         FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_TermServ_MS_Windows__Runonce,                  CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__Terminal_Server_MS_Windows,            L"Runonce",                                                     NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_TermServ_MS_Windows__RunonceEx,                CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__Terminal_Server_MS_Windows,            L"RunonceEx",                                                   NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_TermServ_MS_Windows__Run,                      CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__Terminal_Server_MS_Windows,            L"Run",                                                         NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_Run,                                              CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"Run",                                                         NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_RunOnceEx,                                        CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"RunOnceEx",                                                   NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_RunOnce,                                          CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"RunOnce",                                                     NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_NT_Windows__Load,                                 CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Windows",                                                     L"load",                            FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_NT_Windows__Run,                                  CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Windows",                                                     L"run",                             FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_Policies_Explorer_Run,                            CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"Policies\\Explorer\\Run",                                     NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_Policies_Explorer_Run,                            CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS_Windows,                         L"Policies\\Explorer\\Run",                                     NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_Run,                                              CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS_Windows,                         L"Run",                                                         NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_RunOnce,                                          CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS_Windows,                         L"RunOnce",                                                     NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_NT_TermServ_MS_Windows__Runonce,                  CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__Terminal_Server_MS_Windows,            L"Runonce",                                                     NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_NT_TermServ_MS_Windows__RunonceEx,                CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__Terminal_Server_MS_Windows,            L"RunonceEx",                                                   NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_NT_TermServ_MS_Windows__Run,                      CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__Terminal_Server_MS_Windows,            L"Run",                                                         NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Policies_MS_Windows_System_Scripts_Logoff,                   CAtrEntryTool::EnumSubSubKeyAndScanRegValue,        NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__Policies_MS_Windows_System_Scripts,    L"Logoff",                                                      L"Scripts",                         FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Policies_MS_Windows_System_Scripts_Shutdown,                 CAtrEntryTool::EnumSubSubKeyAndScanRegValue,        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__Policies_MS_Windows_System_Scripts,    L"Shutdown",                                                    L"Scripts",                         FALSE)

    // Explorer
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_Protocols_Filter,                                    CAtrEntryTool::EnumSubKeyAndScanRegValueForCLSID,   NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"Protocols\\Filter",                                           L"CLSID",                           FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_Protocols_Handler,                                   CAtrEntryTool::EnumSubKeyAndScanRegValueForCLSID,   NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"Protocols\\Handler",                                          L"CLSID",                           FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_IE_Desktop_Components,                                    CAtrEntryTool::EnumSubKeyAndScanRegValue,           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS_IE,                              L"Desktop\\Components",                                         L"Source",                          TRUE)   // may be About:Home
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Active_Setup_Installed_Components,                        CAtrEntryTool::EnumSubKeyAndScanRegValue,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS,                                 L"Active Setup\\Installed Components",                          L"StubPath",                        FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Active_Setup_Installed_Components,                        CAtrEntryTool::EnumSubKeyAndScanRegValue,           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS,                                 L"Active Setup\\Installed Components",                          L"StubPath",                        FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_Explorer_SharedTaskScheduler,                     CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"Explorer\\SharedTaskScheduler",                               NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_ShellServiceObjectDelayLoad,                      CAtrEntryTool::EnumValueForCLSID,                   NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"ShellServiceObjectDelayLoad",                                 L"CLSID",                           FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_ShellServiceObjectDelayLoad,                      CAtrEntryTool::EnumValueForCLSID,                   NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS_Windows,                         L"ShellServiceObjectDelayLoad",                                 L"CLSID",                           FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_Explorer_ShellExecuteHooks,                       CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"Explorer\\ShellExecuteHooks",                                 NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Classes_Asterisk_ShellEx_ContextMenuHandlers,                CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_Classes,                            L"*\\ShellEx\\ContextMenuHandlers",                             NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_Asterisk_ShellEx_ContextMenuHandlers,                CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"*\\ShellEx\\ContextMenuHandlers",                             NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Classes_AllFileSystemObjects_ShellEx_ContextMenuHandlers,    CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_Classes,                            L"AllFileSystemObjects\\ShellEx\\ContextMenuHandlers",          NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_AllFileSystemObjects_ShellEx_ContextMenuHandlers,    CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"AllFileSystemObjects\\ShellEx\\ContextMenuHandlers",          NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Classes_Directory_ShellEx_ContextMenuHandlers,               CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_Classes,                            L"Directory\\ShellEx\\ContextMenuHandlers",                     NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_Directory_ShellEx_ContextMenuHandlers,               CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"Directory\\ShellEx\\ContextMenuHandlers",                     NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Classes_Directory_ShellEx_ContextMenuHandlers,               CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_Classes,                            L"Directory\\ShellEx\\DragDropHandlers",                        NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_Directory_ShellEx_ContextMenuHandlers,               CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"Directory\\ShellEx\\DragDropHandlers",                        NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Classes_Directory_ShellEx_PropertySheetHandlers,             CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_Classes,                            L"Directory\\ShellEx\\PropertySheetHandlers",                   NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_Directory_ShellEx_PropertySheetHandlers,             CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"Directory\\ShellEx\\PropertySheetHandlers",                   NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Classes_Directory_ShellEx_CopyHookHandlers,                  CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_Classes,                            L"Directory\\ShellEx\\CopyHookHandlers",                        NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_Directory_ShellEx_CopyHookHandlers,                  CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"Directory\\ShellEx\\CopyHookHandlers",                        NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Classes_Folder_ShellEx_ColumnHandlers,                       CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_Classes,                            L"Folder\\ShellEx\\ColumnHandlers",                             NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_Folder_ShellEx_ColumnHandlers,                       CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"Folder\\ShellEx\\ColumnHandlers",                             NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Classes_Folder_ShellEx_ContextMenuHandlers,                  CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_Classes,                            L"Folder\\ShellEx\\ContextMenuHandlers",                        NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_Folder_ShellEx_ContextMenuHandlers,                  CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"Folder\\ShellEx\\ContextMenuHandlers",                        NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Classes_Directory_Background_ShellEx_ContextMenuHandlers,    CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_Classes,                            L"Directory\\Background\\ShellEx\\ContextMenuHandlers",         NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_Directory_Background_ShellEx_ContextMenuHandlers,    CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"Directory\\Background\\ShellEx\\ContextMenuHandlers",         NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_Explorer_ShellIconOverlayIdentifiers,             CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS_Windows,                         L"Explorer\\ShellIconOverlayIdentifiers",                       NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_Explorer_ShellIconOverlayIdentifiers,             CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"Explorer\\ShellIconOverlayIdentifiers",                       NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Ctf_LangBarAddin,                                         CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS,                                 L"Ctf\\LangBarAddin",                                           NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Ctf_LangBarAddin,                                         CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS,                                 L"Ctf\\LangBarAddin",                                           NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Windows_Shell_Extensions_Approved,                        CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS_Windows,                         L"Shell Extensions\\Approved",                                  NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_Shell_Extensions_Approved,                        CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"Shell Extensions\\Approved",                                  NULL,                               FALSE)


    // Internet Explorer
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_Explorer_Browser_Helper_Objects,                  CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"Explorer\\Browser Helper Objects",                            L"",                                FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_IE_UrlSearchHooks,                                        CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS,                                 L"Internet Explorer\\UrlSearchHooks",                           L"",                                FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_IE_Toolbar,                                               CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS,                                 L"Internet Explorer\\Toolbar",                                  L"",                                FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_IE_Explorer_Bars,                                         CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS,                                 L"Internet Explorer\\Explorer Bars",                            L"",                                FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_IE_Explorer_Bars,                                         CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS,                                 L"Internet Explorer\\Explorer Bars",                            L"",                                FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_IE_Extensions,                                            CAtrEntryTool::EnumSubKeyAndScanRegValue,           NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS,                                 L"Internet Explorer\\Extensions",                               L"Exec",                            FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_IE_Extensions,                                            CAtrEntryTool::EnumSubKeyAndScanRegValue,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS,                                 L"Internet Explorer\\Extensions",                               L"Exec",                            FALSE)



    // Scheduled Tasks
    // Services
    // Drivers

    // Codec
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Drivers32,                                     CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Drivers32",                                                   NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_Filter,                                              CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"Filter",                                                      NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_083863F1_70DE_11d0_BD40_00A0C911CE86_Instance,       CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"CLSID\\{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance",     NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_AC757296_3522_4E11_9862_C17BE5A1767E_Instance,       CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"CLSID\\{AC757296-3522-4E11-9862-C17BE5A1767E}\\Instance",     NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_7ED96837_96F0_4812_B211_F13C24117ED3_Instance,       CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance",     NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_ABE3B9A4_257D_4B97_BD1A_294AF496222E_Instance,       CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"CLSID\\{ABE3B9A4-257D-4B97-BD1A-294AF496222E}\\Instance",     NULL,                               FALSE)


    // Boot Execute
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_Session_Manager__BootExecute,                CAtrEntryTool::ScanRegValueAsMultiString,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"Session Manager",                                             L"BootExecute",                     FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_Session_Manager__SetupExecute,               CAtrEntryTool::ScanRegValueAsMultiString,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"Session Manager",                                             L"SetupExecute",                    FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_Session_Manager__Execute,                    CAtrEntryTool::ScanRegValueAsMultiString,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"Session Manager",                                             L"Execute",                         FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_Session_Manager__S0InitialCommand,           CAtrEntryTool::ScanRegValueAsMultiString,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"Session Manager",                                             L"S0InitialCommand",                FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl__ServiceControlManagerExtension,             CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet,                     L"Control",                                                     L"ServiceControlManagerExtension",  FALSE)


    // Image Hijacks
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Image_File_Execution_Options,                  CAtrEntryTool::EnumSubKeyAndScanRegValue,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Image File Execution Options",                                L"Debugger",                        FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Command_Processor__Autorun,                               CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS,                                 L"Command Processor",                                           L"Autorun",                         FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_MS_Command_Processor__Autorun,                               CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS,                                 L"Command Processor",                                           L"Autorun",                         FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Classes_Exefile_Shell_Open_Command,                          CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_Classes,                            L"Exefile\\Shell\\Open\\Command",                               L"",                                TRUE)   // may be arguments
    SLATR_ENTRY_REG(SLATR_HKLM_Classes_Exefile_Shell_Open_Command,                          CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_Classes,                            L"Exefile\\Shell\\Open\\Command",                               L"",                                TRUE)


    // AppInit
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Windows__AppInit_DLLs,                         CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Windows",                                                     L"AppInit_Dlls",                    FALSE)


    // KnownDlls
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_Session_Manager_KnownDlls,                   CAtrEntryTool::EnumRegValues,                       NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"Session Manager\\KnownDlls",                                  NULL,                               TRUE)



    // Winlogon
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Winlogon__ServiceControllerStart,              CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon",                                                    L"ServiceControllerStart",          FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Winlogon__LsaStart,                            CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon",                                                    L"LsaStart",                        FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Winlogon__System,                              CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon",                                                    L"System",                          FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Winlogon__UIHost,                              CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon",                                                    L"UIHost",                          FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Winlogon_Notify__DllName,                      CAtrEntryTool::EnumSubKeyAndScanRegValue,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon\\Notify",                                            L"DLLName",                         FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Winlogon__GinaDLL,                             CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon",                                                    L"GinaDLL",                         FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Winlogon__Taskman,                             CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon",                                                    L"Taskman",                         FALSE)
    SLATR_ENTRY_REG(SLATR_HKCU_Control_Panel_Desktop,                                       CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__Control_Panel,                         L"Desktop",                                                     L"ScrnSave.exe",                    FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_BootVerificationProgram__ImagePath,          CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"BootVerificationProgram",                                     L"ImagePath",                       TRUE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_NT_Winlogon__SaveDumpStart,                       CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_CURRENT_USER,  SLATR_RegKey_Parent__SW_MS_Windows_NT,                      L"Winlogon",                                                    L"SaveDumpStart",                   FALSE)


    // Winsock Providers
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Svc_WinSock2_Namespace_Catalog5,                  CAtrEntryTool::EnumSubKeyAndScanRegValue,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Services,            L"WinSock2\\Parameters\\NameSpace_Catalog5\\Catalog_Entries",   L"LibraryPath",                     FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Svc_WinSock2_Protocol_Catalog9,                   CAtrEntryTool::ScanRegValueAsBinary,                NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Services,            L"WinSock2\\Parameters\\Protocol_Catalog9\\Catalog_Entries",    L"PackedCatalogItem",               FALSE)

    // Print Monitors
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_Print_Monitors,                              CAtrEntryTool::EnumSubKeyAndScanRegValue,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"Print\\Monitors",                                             L"Driver",                          FALSE)

    // Lsa Providers
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_SecurityProviders,                           CAtrEntryTool::ScanRegValue,                        NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"SecurityProviders",                                           L"SecurityProviders",               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_Lsa__Authentication_Packages,                CAtrEntryTool::ScanRegValueAsMultiString,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"Lsa",                                                         L"Authentication Packages",         FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_Lsa__Notification_Packages,                  CAtrEntryTool::ScanRegValueAsMultiString,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"Lsa",                                                         L"Notification Packages",           FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_Lsa__Security_Packages,                      CAtrEntryTool::ScanRegValueAsMultiString,           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"Lsa",                                                         L"Security Packages",               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_Authentication_Credential_Providers,              CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"Authentication\\Credential Providers",                        NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_Authentication_Credential_Provider_Filters,       CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"Authentication\\Credential Provider Filters",                 NULL,                               FALSE)
    SLATR_ENTRY_REG(SLATR_HKLM_MS_Windows_Authentication_PLAP_Providers,                    CAtrEntryTool::EnumCLSID,                           NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__SW_MS_Windows,                         L"Authentication\\PLAP Providers",                              NULL,                               FALSE)

    // Network Providers
    SLATR_ENTRY_REG(SLATR_HKLM_CurCtrlSet_Ctrl_NetworkProvider_Order__ProviderOrder,        CAtrEntryTool::ScanNetworkProvider,                 NULL,   NULL,   HKEY_LOCAL_MACHINE, SLATR_RegKey_Parent__CurrentControlSet_Control,             L"NetworkProvider\\Order",                                      L"ProviderOrder",                   FALSE)


    // Autorun.inf
    SLATR_ENTRY(SLATR_Ini_Autorun_Inf_At_Disk_Root,     CAtrEntryTool::ScanAutorunInf,      NULL,   NULL)


SLATR_ENTRY_END()





using namespace Skylark;
using namespace WinMod;

BOOL CAtrEntryTool::IsValidEntry(const SLATR_ENTRY_DATA* pEntry)
{
    if (!pEntry)
        return FALSE;

    if (SLATR_None == pEntry->m_dwEntryID)
        return FALSE;

    return TRUE;
}

CString CAtrEntryTool::GetRegPath(const SLATR_ENTRY_DATA* pEntry)
{
    assert(pEntry);
    assert(pEntry->m_lpszRegParentKeyName && *pEntry->m_lpszRegParentKeyName);
    if (!pEntry->m_lpszRegParentKeyName || !*pEntry->m_lpszRegParentKeyName)
        return L"";


    if (!pEntry->m_lpszRegSubKeyName || !*pEntry->m_lpszRegSubKeyName)
        return pEntry->m_lpszRegParentKeyName;


    CString strKeyName = pEntry->m_lpszRegParentKeyName;
    strKeyName.AppendChar(L'\\');
    strKeyName.Append(pEntry->m_lpszRegSubKeyName);
    return strKeyName;
}


const SLATR_ENTRY_DATA* CAtrEntryTool::GetEntryTable()
{
    return g_pEntryTable;
}

DWORD CAtrEntryTool::GetEntryTableCount()
{
    DWORD dwCount = _countof(g_pEntryTable);
    assert(dwCount);
    return dwCount - 1;
}

const SLATR_ENTRY_DATA* CAtrEntryTool::FindByEntryID(
    const SLATR_ENTRY_DATA* pFirstEntry,
    DWORD                   dwEntryID)
{
    if (!pFirstEntry)
        return NULL;


    // 将不存在的id加入了表
    assert(SLATR_No_Existing != pFirstEntry->m_dwEntryID);


    // 扫描到了末尾
    if (SLATR_None == pFirstEntry->m_dwEntryID)
        return NULL;


    // 扫描所有
    if (SLATR_All == dwEntryID)
        return pFirstEntry;


    // 查找要扫描的id
    for (NULL; SLATR_None != pFirstEntry->m_dwEntryID; ++pFirstEntry)
    {
        if (pFirstEntry->m_dwEntryID == dwEntryID)
            return pFirstEntry;
    }

    return NULL;
}   


LPCWSTR CAtrEntryTool::GetHKEYRootString(HKEY hRootKey)
{
    if (HKEY_CLASSES_ROOT == hRootKey)
        return L"HKEY_CLASSES_ROOT";

    if (HKEY_CURRENT_USER == hRootKey)
        return L"HKEY_CURRENT_USER";

    if (HKEY_LOCAL_MACHINE == hRootKey)
        return L"HKEY_LOCAL_MACHINE";

    if (HKEY_USERS == hRootKey)
        return L"HKEY_USERS";

    if (NULL == hRootKey)
        return L"NULL";

    return L"UNKNOWN";
}

HRESULT CAtrEntryTool::EnumRegValues(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);


    // 获取注册表的键路径
    HKEY    hKeyRoot   = pEntry->m_hRootKey;
    CString strRegPath = CAtrEntryTool::GetRegPath(pEntry);
    assert(!strRegPath.IsEmpty());


    // 遍历键下的所有值
    CWinRegValueFind hRegValueFind;
    BOOL bFind = hRegValueFind.FindFirstValueName(hKeyRoot, strRegPath);
    for (NULL; bFind; bFind = hRegValueFind.FindNextValueName())
    {
        CString strValue;
        HRESULT hr = hRegValueFind.GetStringValue(strValue);
        if (FAILED(hr))
        {
            // todo: bugfeed
            continue;
        }

        piContext->Append_RegValue_As_Startup(
            pEntry,
            hKeyRoot,
            strRegPath,
            hRegValueFind.GetValueName(),
            strValue);
    }

    return S_OK;
}


HRESULT CAtrEntryTool::EnumSubKeyAndScanRegValue(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);


    // 获取注册表的键路径
    HKEY    hKeyRoot = pEntry->m_hRootKey;
    CString strRegPath = CAtrEntryTool::GetRegPath(pEntry);
    assert(!strRegPath.IsEmpty());


    // 查找注册表键值
    CWinRegKeyFind hRegKeyFind;
    BOOL bFind = hRegKeyFind.FindFirstSubKey(hKeyRoot, strRegPath);
    for (NULL; bFind; bFind = hRegKeyFind.FindNextSubKey())
    {
        // 打开注册表键
        CWinRegKey hRegKey;
        LONG lRet = hRegKey.Open(hKeyRoot, hRegKeyFind.GetFullRegPath(), KEY_QUERY_VALUE);
        if (ERROR_SUCCESS != lRet)
            continue;


        CString strValue;
        lRet = hRegKey.ExQueryStringValue(pEntry->m_lpszRegValueName, strValue);
        if (ERROR_SUCCESS == lRet)
        {
            piContext->Append_RegValue_As_Startup(
                pEntry,
                hKeyRoot,
                hRegKeyFind.GetFullRegPath(),
                pEntry->m_lpszRegValueName,
                strValue);
        }
    }


    // 获取指定键的值
    return S_OK;
}



// 枚举组策略脚本
HRESULT CAtrEntryTool::EnumSubSubKeyAndScanRegValue(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);
    assert(pEntry->m_lpszRegValueName);


    // 获取注册表的键路径
    HKEY    hKeyRoot = pEntry->m_hRootKey;
    CString strRegPath = CAtrEntryTool::GetRegPath(pEntry);
    assert(!strRegPath.IsEmpty());


    // 查找注册表键值
    // Path\\*\\*\\[Scripts]
    CWinRegKeyFind hRegKeyFind;
    BOOL bFind = hRegKeyFind.FindFirstSubKey(hKeyRoot, strRegPath);
    for (NULL; bFind; bFind = hRegKeyFind.FindNextSubKey())
    {
        CWinRegKeyFind hRegKeySubFind;
        BOOL bSubFind = hRegKeySubFind.FindFirstSubKey(hKeyRoot, hRegKeySubFind.GetFullRegPath());
        for (NULL; bSubFind; bSubFind = hRegKeyFind.FindNextSubKey())
        {
            CString strValue;
            LONG lRet = hRegKeySubFind.m_hKeyEnum.ExQueryStringValue(pEntry->m_lpszRegValueName, strValue);
            if (ERROR_SUCCESS == lRet)
            {

                piContext->Append_RegValue_As_Startup(
                    pEntry,
                    hKeyRoot,
                    hRegKeySubFind.GetFullRegPath(),
                    pEntry->m_lpszRegValueName,
                    strValue);
            }
        }
    }


    // 获取指定键的值
    return S_OK;
}

HRESULT CAtrEntryTool::EnumCLSID(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    EnumSubKeyAndScanRegKeyForCLSID(piContext, pEntry);
    EnumValueNameForCLSID(piContext, pEntry);

    return S_OK;
}


HRESULT CAtrEntryTool::EnumSubKeyAndScanRegKeyForCLSID(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);


    // 获取注册表的键路径
    HKEY    hKeyRoot = pEntry->m_hRootKey;
    CString strRegPath = CAtrEntryTool::GetRegPath(pEntry);
    assert(!strRegPath.IsEmpty());


    // 查找注册表键值
    CWinRegKeyFind hRegKeyFind;
    BOOL bFind = hRegKeyFind.FindFirstSubKey(hKeyRoot, strRegPath);
    for (NULL; bFind; bFind = hRegKeyFind.FindNextSubKey())
    {
        CString strCLSID;
        CString strSubKeyName = hRegKeyFind.GetSubKeyName();

        // 如果键路径不是CLSID,则取键的默认值作为CLSID
        if (strSubKeyName.IsEmpty() || strSubKeyName[0] != L'{')
        {
            CWinRegKey hRegKey;
            LONG lRet = hRegKey.Open(hKeyRoot, hRegKeyFind.GetFullRegPath(), KEY_QUERY_VALUE);
            if (ERROR_SUCCESS != lRet)
                continue;

            lRet = hRegKey.ExQueryStringValue(L"", strCLSID);
            if (ERROR_SUCCESS != lRet)
                continue;


            if (strCLSID.IsEmpty() || strCLSID[0] != L'{')
                continue;
        }
        else
        {
            strCLSID = strSubKeyName;
        }

        // 获取CLSID代表的键值
        // HKEY_CLASSES_ROOT\CLSID\{CLSID}\InprocServer32
        CString strRegInprocServer32;
        strRegInprocServer32.Append(L"CLSID");
        strRegInprocServer32.Append(L"\\");
        strRegInprocServer32.Append(strCLSID);
        strRegInprocServer32.Append(L"\\");
        strRegInprocServer32.Append(L"InprocServer32");
        CWinRegKey hRegInprocServer32;
        LONG lRet = hRegInprocServer32.Open(HKEY_CLASSES_ROOT, strRegInprocServer32, KEY_QUERY_VALUE);
        if (ERROR_SUCCESS != lRet)
            continue;


        // 获取默认键的值
        CString strValue;
        lRet = hRegInprocServer32.ExQueryStringValue(L"", strValue);
        if (ERROR_SUCCESS != lRet)
            continue;



        piContext->Append_RegValue_As_Startup(
            pEntry,
            hKeyRoot,
            hRegKeyFind.GetFullRegPath(),
            pEntry->m_lpszRegValueName,
            strValue);
    }


    // 获取指定键的值
    return S_OK;
}

HRESULT CAtrEntryTool::EnumSubKeyAndScanRegValueForCLSID(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);
    assert(pEntry->m_lpszRegValueName);


    // 获取注册表的键路径
    HKEY    hKeyRoot = pEntry->m_hRootKey;
    CString strRegPath = CAtrEntryTool::GetRegPath(pEntry);
    assert(!strRegPath.IsEmpty());


    // 查找注册表键值
    CWinRegKeyFind hRegKeyFind;
    BOOL bFind = hRegKeyFind.FindFirstSubKey(hKeyRoot, strRegPath);
    for (NULL; bFind; bFind = hRegKeyFind.FindNextSubKey())
    {
        CString strCLSID;
        CWinRegKey hRegKey;
        LONG lRet = hRegKey.Open(hKeyRoot, hRegKeyFind.GetFullRegPath(), KEY_QUERY_VALUE);
        if (ERROR_SUCCESS != lRet)
            continue;

        lRet = hRegKey.ExQueryStringValue(pEntry->m_lpszRegValueName, strCLSID);
        if (ERROR_SUCCESS != lRet)
            continue;

        // CLSID 以 { 开头
        if (strCLSID.IsEmpty() || strCLSID[0] != L'{')
            continue;

        // 获取CLSID代表的键值
        // HKEY_CLASSES_ROOT\CLSID\{CLSID}\InprocServer32
        CString strRegInprocServer32;
        strRegInprocServer32.Append(L"CLSID");
        strRegInprocServer32.Append(L"\\");
        strRegInprocServer32.Append(strCLSID);
        strRegInprocServer32.Append(L"\\");
        strRegInprocServer32.Append(L"InprocServer32");
        CWinRegKey hRegInprocServer32;
        lRet = hRegInprocServer32.Open(HKEY_CLASSES_ROOT, strRegInprocServer32, KEY_QUERY_VALUE);
        if (ERROR_SUCCESS != lRet)
            continue;


        // 获取默认键的值
        CString strValue;
        lRet = hRegInprocServer32.ExQueryStringValue(L"", strValue);
        if (ERROR_SUCCESS != lRet)
            continue;



        piContext->Append_RegValue_As_Startup(
            pEntry,
            hKeyRoot,
            hRegKeyFind.GetFullRegPath(),
            pEntry->m_lpszRegValueName,
            strValue);
    }


    // 获取指定键的值
    return S_OK;
}

HRESULT CAtrEntryTool::EnumValueNameForCLSID(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);


    // 获取注册表的键路径
    HKEY    hKeyRoot = pEntry->m_hRootKey;
    CString strRegPath = CAtrEntryTool::GetRegPath(pEntry);
    assert(!strRegPath.IsEmpty());


    // 查找注册表键值
    CWinRegValueFind hRegValueFind;
    BOOL bFind = hRegValueFind.FindFirstValueName(hKeyRoot, strRegPath);
    for (NULL; bFind; bFind = hRegValueFind.FindNextValueName())
    {
        CString strValueName = hRegValueFind.GetValueName();

        // CLSID 以 { 开头
        if (strValueName.IsEmpty() || strValueName[0] != L'{')
            continue;


        // 获取CLSID代表的键值
        // HKEY_CLASSES_ROOT\CLSID\{CLSID}\InprocServer32
        CString strRegInprocServer32;
        strRegInprocServer32.Append(L"CLSID");
        strRegInprocServer32.Append(L"\\");
        strRegInprocServer32.Append(strValueName);
        strRegInprocServer32.Append(L"\\");
        strRegInprocServer32.Append(L"InprocServer32");
        CWinRegKey hRegInprocServer32;
        LONG lRet = hRegInprocServer32.Open(HKEY_CLASSES_ROOT, strRegInprocServer32, KEY_QUERY_VALUE);
        if (ERROR_SUCCESS != lRet)
            continue;


        // 获取默认键的值
        CString strValue;
        lRet = hRegInprocServer32.ExQueryStringValue(L"", strValue);
        if (ERROR_SUCCESS != lRet)
            continue;



        piContext->Append_RegValue_As_Startup(
            pEntry,
            hKeyRoot,
            strRegPath,
            strValueName,
            strValue);
    }


    // 获取指定键的值
    return S_OK;
}

HRESULT CAtrEntryTool::EnumValueForCLSID(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);


    // 获取注册表的键路径
    HKEY    hKeyRoot = pEntry->m_hRootKey;
    CString strRegPath = CAtrEntryTool::GetRegPath(pEntry);
    assert(!strRegPath.IsEmpty());


    // 查找注册表键值
    CWinRegValueFind hRegValueFind;
    BOOL bFind = hRegValueFind.FindFirstValueName(hKeyRoot, strRegPath);
    for (NULL; bFind; bFind = hRegValueFind.FindNextValueName())
    {
        CString strValueName = hRegValueFind.GetValueName();

        CString strCLSID;
        HRESULT hr = hRegValueFind.GetStringValue(strCLSID);
        if (FAILED(hr))
            continue;


        // CLSID 以 { 开头
        if (strCLSID.IsEmpty() || strCLSID[0] != L'{')
            continue;


        // 获取CLSID代表的键值
        // HKEY_CLASSES_ROOT\CLSID\{CLSID}\InprocServer32
        CString strRegInprocServer32;
        strRegInprocServer32.Append(L"CLSID");
        strRegInprocServer32.Append(L"\\");
        strRegInprocServer32.Append(strCLSID);
        strRegInprocServer32.Append(L"\\");
        strRegInprocServer32.Append(L"InprocServer32");
        CWinRegKey hRegInprocServer32;
        LONG lRet = hRegInprocServer32.Open(HKEY_CLASSES_ROOT, strRegInprocServer32, KEY_QUERY_VALUE);
        if (ERROR_SUCCESS != lRet)
            continue;


        // 获取默认键的值
        CString strValue;
        lRet = hRegInprocServer32.ExQueryStringValue(L"", strValue);
        if (ERROR_SUCCESS != lRet)
            continue;



        piContext->Append_RegValue_As_Startup(
            pEntry,
            hKeyRoot,
            strRegPath,
            strValueName,
            strValue);
    }


    // 获取指定键的值
    return S_OK;
}

// 遍历注册表键下的值
HRESULT CAtrEntryTool::ScanRegValue(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);
    assert(pEntry->m_lpszRegValueName);


    // 获取注册表的键路径
    HKEY    hKeyRoot = pEntry->m_hRootKey;
    CString strRegPath = CAtrEntryTool::GetRegPath(pEntry);
    assert(!strRegPath.IsEmpty());


    // 打开注册表键
    CWinRegKey hRegKey;
    LONG lRet = hRegKey.Open(hKeyRoot, strRegPath, KEY_QUERY_VALUE);
    if (ERROR_SUCCESS != lRet)
        return AtlHresultFromWin32(lRet);


    // 获取指定键的值
    CString strValue;
    lRet = hRegKey.ExQueryStringValue(pEntry->m_lpszRegValueName, strValue);
    if (ERROR_SUCCESS != lRet)
        return AtlHresultFromWin32(lRet);


    piContext->Append_RegValue_As_Startup(
        pEntry,
        hKeyRoot,
        strRegPath,
        pEntry->m_lpszRegValueName,
        strValue);
    return S_OK;
}

// 扫描注册表键下的指定值(REG_MULTI_SZ),并以单个文件的方式解析
HRESULT CAtrEntryTool::ScanRegValueAsMultiString(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);
    assert(pEntry->m_lpszRegValueName);


    // 获取注册表的键路径
    HKEY    hKeyRoot = pEntry->m_hRootKey;
    CString strKeyName = CAtrEntryTool::GetRegPath(pEntry);
    assert(!strKeyName.IsEmpty());


    // 打开注册表键
    CWinRegKey hRegKey;
    LONG lRet = hRegKey.Open(hKeyRoot, strKeyName, KEY_QUERY_VALUE);
    if (ERROR_SUCCESS != lRet)
        return AtlHresultFromWin32(lRet);


    // 获取指定键的值
    CAtlList<CString> valueList;
    lRet = hRegKey.ExQueryMultiStringValue(pEntry->m_lpszRegValueName, valueList);
    if (ERROR_SUCCESS != lRet)
        return AtlHresultFromWin32(lRet);


    while (!valueList.IsEmpty())
    {
        const CString& strValue = valueList.GetHead();

        piContext->Append_RegValue_As_Startup(
            pEntry,
            hKeyRoot,
            strKeyName,
            pEntry->m_lpszRegValueName,
            strValue);

        valueList.RemoveHeadNoReturn();
    }


    return S_OK;
}

HRESULT CAtrEntryTool::ScanRegValueAsBinary(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);


    // 获取注册表的键路径
    HKEY    hKeyRoot = pEntry->m_hRootKey;
    CString strRegPath = CAtrEntryTool::GetRegPath(pEntry);
    assert(!strRegPath.IsEmpty());


    // 查找注册表键值
    CWinRegKeyFind hRegKeyFind;
    BOOL bFind = hRegKeyFind.FindFirstSubKey(hKeyRoot, strRegPath);
    for (NULL; bFind; bFind = hRegKeyFind.FindNextSubKey())
    {
        // 打开注册表键
        CWinRegKey hRegKey;
        LONG lRet = hRegKey.Open(hKeyRoot, hRegKeyFind.GetFullRegPath(), KEY_QUERY_VALUE);
        if (ERROR_SUCCESS != lRet)
            continue;


        CAtlArray<BYTE> buf;
        lRet = hRegKey.ExBinaryValue(pEntry->m_lpszRegValueName, buf);
        if (ERROR_SUCCESS == lRet)
        {
            buf.Add(0);
            piContext->Append_RegValue_As_Startup(
                pEntry,
                hKeyRoot,
                hRegKeyFind.GetFullRegPath(),
                pEntry->m_lpszRegValueName,
                CA2W((LPCSTR)buf.GetData()));
            // 考虑到这里是比较关键的目录,通常不可能出现非ASCII字符,所以使用当前代码页转换
        }
    }


    // 获取指定键的值
    return S_OK;
}

HRESULT CAtrEntryTool::ScanNetworkProvider(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);
    assert(pEntry->m_lpszRegValueName);


    // 获取注册表的键路径
    HKEY    hKeyRoot = pEntry->m_hRootKey;
    CString strRegPath = CAtrEntryTool::GetRegPath(pEntry);
    assert(!strRegPath.IsEmpty());


    // 打开注册表键
    CWinRegKey hRegKey;
    LONG lRet = hRegKey.Open(hKeyRoot, strRegPath, KEY_QUERY_VALUE);
    if (ERROR_SUCCESS != lRet)
        return AtlHresultFromWin32(lRet);


    // 获取指定键的值
    CString strProviderList;
    lRet = hRegKey.ExQueryStringValue(pEntry->m_lpszRegValueName, strProviderList);
    if (ERROR_SUCCESS != lRet)
        return AtlHresultFromWin32(lRet);


    int nPos = 0;
    CString strProviderName;
    while (nPos >= 0)
    {
        strProviderName = strProviderList.Tokenize(L",", nPos);
        strProviderName.Trim();
        if (strProviderName.IsEmpty())
            continue;


        // 打开provider的注册表键
        CString strRegProvider = SLATR_RegKey_Parent__CurrentControlSet_Services;
        strRegProvider.Append(L"\\");
        strRegProvider.Append(strProviderName);
        strRegProvider.Append(L"\\");
        strRegProvider.Append(L"NetworkProvider");


        CWinRegKey hRegKey;
        LONG lRet = hRegKey.Open(hKeyRoot, strRegProvider, KEY_QUERY_VALUE);
        if (ERROR_SUCCESS != lRet)
            continue;


        // 获取provider文件
        CString strProviderFile;
        lRet = hRegKey.ExQueryStringValue(L"ProviderPath", strProviderFile);
        if (ERROR_SUCCESS != lRet)
            return AtlHresultFromWin32(lRet);


        piContext->Append_RegValue_As_Startup(
            pEntry,
            hKeyRoot,
            strRegProvider,
            L"ProviderPath",
            strProviderFile);
    }



    return S_OK;
}

// 枚举服务
HRESULT CAtrEntryTool::EnumServices(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);


    // 遍历键下的所有值
    CWinServiceFind hServiceFind;
    BOOL bFind = hServiceFind.FindFirstService();
    for (NULL; bFind; bFind = hServiceFind.FindNextService())
    {
        // 在命令行中寻找
        CWinPath imagePath;
        CWinPath serviceDll;

        piContext->Append_Service(
            pEntry,
            hServiceFind.m_hRegKeyFind.GetSubKeyName(),
            hServiceFind.GetServiceType(),
            hServiceFind.GetServiceStart(),
            hServiceFind.GetImagePath().m_strPath,
            hServiceFind.GetServiceDll().m_strPath,
            hServiceFind.GetDisplayName(),
            hServiceFind.GetDescription());
    }

    return S_OK;  
}


HRESULT CAtrEntryTool::DisableService(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    UNREFERENCED_PARAMETER(piContext);
    UNREFERENCED_PARAMETER(pEntry);
    return E_NOTIMPL;
}

HRESULT CAtrEntryTool::RemoveService(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    UNREFERENCED_PARAMETER(piContext);
    UNREFERENCED_PARAMETER(pEntry);
    return E_NOTIMPL;
}




HRESULT CAtrEntryTool::EnumScheduledTasks(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
        return hr;

    if (CWinOSVer::IsVistaOrLater())
    {
        hr = DoEnumScheduledTasksInVista(piContext, pEntry);
    }
    else
    {
        hr = DoEnumScheduledTasks(piContext, pEntry);
    }

    CoUninitialize();
    return hr;
}

HRESULT CAtrEntryTool::DoEnumScheduledTasks(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    CComPtr<ITaskScheduler> spiTaskScheduler;
    HRESULT hr = CoCreateInstance(
        CLSID_CTaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskScheduler,
        (void**)&spiTaskScheduler);
    if (FAILED(hr))
        return hr;



    CComPtr<IEnumWorkItems> spiEnum;
    hr = spiTaskScheduler->Enum(&spiEnum);
    if (FAILED(hr))
        return hr;



    LPWSTR* ppszNames      = NULL;
    DWORD   dwFetchedTasks = 0;
    while (TRUE)
    {
        hr = spiEnum->Next(1, &ppszNames, &dwFetchedTasks);
        if (FAILED(hr) || 0 == dwFetchedTasks || NULL == ppszNames)
            break;

        if (NULL == ppszNames[0])
        {
            CoTaskMemFree(ppszNames);
            continue;
        }


        CComPtr<ITask> spiTask;
        hr = spiTaskScheduler->Activate(ppszNames[0], IID_ITask, (IUnknown**)&spiTask);
        if (FAILED(hr))
        {
            CoTaskMemFree(ppszNames[0]);
            CoTaskMemFree(ppszNames);
            continue;
        }


        LPWSTR lpwszApplicationName = NULL;
        hr = spiTask->GetApplicationName(&lpwszApplicationName);
        if (FAILED(hr) || !lpwszApplicationName)
        {
            CoTaskMemFree(ppszNames[0]);
            CoTaskMemFree(ppszNames);
            continue;
        }


        piContext->Append_Scheduled_Task(pEntry, ppszNames[0], lpwszApplicationName);
    }


    return S_OK;
}

HRESULT CAtrEntryTool::DoEnumScheduledTasksInVista(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
#if _MSV_VER >= 1400
    HRESULT hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        0,
        NULL);
    if (FAILED(hr))
        return hr;


    //  Create an instance of the Task Service. 
    CComPtr<ITaskService> spiService;
    hr = CoCreateInstance(
        CLSID_TaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskService,
        (void**)&spiService);  
    if (FAILED(hr))
        return hr;



    //  Connect to the task service.
    hr = spiService->Connect(CComVariant(), CComVariant(), CComVariant(), CComVariant());
    if (FAILED(hr))
        return hr;




    //  Get the pointer to the root task folder.
    CComPtr<ITaskFolder> spiRootFolder;
    hr = spiService->GetFolder(CComBSTR( L"\\") , &spiRootFolder);
    if (FAILED(hr))
        return hr;



    CAtlList<CAdapt<CComPtr<ITaskFolder> > > taskFolderStack;
    taskFolderStack.AddTail(spiRootFolder);


    // 遍历 task 目录及子目录
    while (!taskFolderStack.IsEmpty())
    {
        CComPtr<ITaskFolder> spiFirstFolder = taskFolderStack.RemoveHead();


        // 遍历子目录
        CComPtr<ITaskFolderCollection> spiSubFolderCollection;
        hr = spiFirstFolder->GetFolders(0, &spiSubFolderCollection);
        if (SUCCEEDED(hr))
        {
            LONG numSubFolders = 0;
            hr = spiSubFolderCollection->get_Count(&numSubFolders);
            if (SUCCEEDED(hr))
            {
                for (LONG i = 0; i < numSubFolders; i++)
                {
                    CComPtr<ITaskFolder> spiSubFolder;
                    hr = spiSubFolderCollection->get_Item(CComVariant(i + 1), &spiSubFolder);
                    if (SUCCEEDED(hr))
                    {
                        taskFolderStack.AddTail(spiSubFolder);
                    }
                }
            }
        }       



        // 枚举目录下的task文件
        //  Get the registered tasks in the folder.
        CComPtr<IRegisteredTaskCollection> spiTaskCollection;
        hr = spiFirstFolder->GetTasks(TASK_ENUM_HIDDEN, &spiTaskCollection);
        if (FAILED(hr))
            continue;



        LONG numTasks = 0;
        hr = spiTaskCollection->get_Count(&numTasks);
        if (FAILED(hr))
            continue;


        // 获取 task 的 action
        for (LONG i = 0; i < numTasks; i++)
        {
            CComPtr<IRegisteredTask> spiRegisteredTask;
            hr = spiTaskCollection->get_Item(CComVariant(i + 1), &spiRegisteredTask);
            if (FAILED(hr))
                continue;


            CComBSTR bstrTaskPath;
            hr = spiRegisteredTask->get_Path(&bstrTaskPath);
            if (FAILED(hr))
                continue;


            CComPtr<ITaskDefinition> spiTaskDefinition;
            hr = spiRegisteredTask->get_Definition(&spiTaskDefinition);
            if (FAILED(hr))
                continue;


            CComPtr<IActionCollection> spiActionCollection;
            hr = spiTaskDefinition->get_Actions(&spiActionCollection);
            if (FAILED(hr))
                continue;


            LONG numAction = 0;
            hr = spiActionCollection->get_Count(&numAction);
            if (FAILED(hr))
                continue;


            // 枚举 action
            for (LONG k = 0; k < numAction; k++)
            {
                CComPtr<IAction> spiAction;
                hr = spiActionCollection->get_Item(k + 1, &spiAction);
                if (FAILED(hr))
                    continue;


                CComPtr<IExecAction> spiExecAction;
                hr = spiAction->QueryInterface(IID_IExecAction, (void**)&spiExecAction);
                if (FAILED(hr))
                    continue;


                CComBSTR bstrExecPath;
                spiExecAction->get_Path(&bstrExecPath);
                if (FAILED(hr))
                    continue;


                // 添加任务
                piContext->Append_Scheduled_Task(pEntry, bstrTaskPath, bstrExecPath);
            }
        }
    }

#endif
    return S_OK;
}

HRESULT CAtrEntryTool::ScanAutorunInf(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);

    WCHAR szPath[] = L"C:\\";
    for (int i = 0; i < 24; ++i, ++szPath[0])
    {
        UINT uDriveType = ::GetDriveType(szPath);
        if (DRIVE_CDROM         == uDriveType ||
            DRIVE_UNKNOWN       == uDriveType ||
            DRIVE_NO_ROOT_DIR   == uDriveType)
        {
            continue;
        }


        WCHAR szAutorunInf[MAX_PATH];
        wcscpy(szAutorunInf, szPath);
        wcscat(szAutorunInf, L"Autorun.inf");


        // 使用SetErrorMode 避免弹出"驱动器未就绪"的错误对话框
        BOOL bAccessible = FALSE;
        UINT uOldErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
        if (CWinPathApi::IsDeviceAccessible(szPath[0]))
        {
            DWORD dwAttr = ::GetFileAttributes(szAutorunInf);
            if (INVALID_FILE_ATTRIBUTES != dwAttr)
                bAccessible = TRUE;
        }
        // 恢复ErrorMode;
        SetErrorMode(uOldErrorMode);
        if (!bAccessible)
            continue;



        WCHAR wszStringValue[MAX_PATH + 1];
        if (::GetPrivateProfileString(L"AutoRun", L"Open", L"", wszStringValue, MAX_PATH, szAutorunInf))
        {
            wszStringValue[MAX_PATH] = L'\0';
            piContext->Append_Startup_Ini_File(pEntry, szAutorunInf, L"AutoRun", L"Open", wszStringValue);
        }

        if (::GetPrivateProfileString(L"AutoRun", L"ShellExecute", L"", wszStringValue, MAX_PATH, szAutorunInf))
        {
            wszStringValue[MAX_PATH] = L'\0';
            piContext->Append_Startup_Ini_File(pEntry, szAutorunInf, L"AutoRun", L"ShellExecute", wszStringValue);
        }
    }

    return S_OK;
}

HRESULT CAtrEntryTool::ScanShellFolder(IAutorunContext* piContext, const SLATR_ENTRY_DATA* pEntry)
{
    assert(piContext);
    assert(pEntry);
   

    WCHAR szBuf[MAX_PATH + 1];
    BOOL bRet = ::SHGetSpecialFolderPath(NULL, szBuf, pEntry->m_nFolderCSIDL, FALSE);
    if (!bRet)
        return AtlHresultFromLastError();
    
    
    szBuf[MAX_PATH] = L'\0';


    CWinPath path = szBuf;
    path.Append(L"*");

    CWinFileFind hFileFind;
    BOOL bFind = hFileFind.FindFirstFile(path.m_strPath);
    for (NULL; bFind; bFind = hFileFind.FindNextFile())
    {
        CWinPath fullPath = szBuf;
        fullPath.Append(hFileFind.GetFileName());

        CString strExt = hFileFind.GetFileNameExt();
        if (0 == strExt.CompareNoCase(L".lnk"))
        {   // 如果是快捷方式,则在这里进行解析
            CString strRealPath;
            HRESULT hr = CWinPathApi::ResolveLnkFile(fullPath, strRealPath);
            if (SUCCEEDED(hr) && CWinPathApi::IsFileExisting(strRealPath))
            {
                fullPath.m_strPath = strRealPath;
            }
        }


        piContext->Append_Startup_Folder_File(pEntry, szBuf, fullPath);
    }


    return S_OK;
}