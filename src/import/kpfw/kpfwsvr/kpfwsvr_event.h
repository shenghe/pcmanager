#ifndef _KPFWSVR_H_1
#define _KPFWSVR_H_1

//网镖服务和托盘的事件通知

//监控开启
const TCHAR* const g_pcszPfwStartEvent = _T("Global\\Kingsoft_Personal_Firewall_2010_Start_Event_Object_Name");
//监控关闭
const TCHAR* const g_pcszPfwEndEvent = _T("Global\\Kingsoft_Personal_Firewall_2010_End_Event_Object_Name");
//监控正在运行
const TCHAR* const g_pcszPfwRuningEvent = _T("Global\\Kingsoft_Personal_Firewall_2010_Runing_Event_Object_Name");
//卸载
const TCHAR* const g_pcszPfwUnInstallEvent = _T("Global\\Kingsoft_Personal_Firewall_2010_UnInstall_Event_Object_Name");

#endif