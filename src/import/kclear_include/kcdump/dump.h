////////////////////////////////////////////////////////////////
//
//	Filename: 	dump.h
//	Creator:	yangjun  <yangjun@kingsoft.com>
//	Date:		2009-12-8  23: 07
//	Comment:	由大引擎dump.h改造而来
//
///////////////////////////////////////////////////////////////

#pragma once


#include <windows.h>

typedef DWORD DUMP_CHANNEL;



/**
* @brief	打开崩溃管理器
* @remark	为兼容大引擎老版本,仅供大引擎调用,其它产品或进程请勿使用
*/
void __stdcall KxEOpenDumpMonitor();



/**
* @brief	打开崩溃管理器
* @param	[in] bShowUI　		崩溃时是否显示对话框
* @param	[in] pszUUID		用户UUID(可传NULL)
* @return	[in] dwDumpChannel	上报到服务器端后的通道号标识,通过此标识可拿到对应的Dump文件,
								目前为不同的产品分配不同的通道号标识区间,产品自己可以为不同的进程设置通道号标识区间内的值来区分不同的进程

								各产品通道号范围划分,请各产品按以下分配的通道号范围来使用

								[0, 0]		测试通道
								[1,99]		安全100
								[100,199]	大引擎
								[200,299]	急救箱
								[300,399]	密报
								[400,499]	网盾

								例如:　急救箱的产品在使用就可能是这样的，KxEOpenDumpMonitorEx(true, NULL, 201),dwDumpChannel的值在[200,299]之间即可
* @remark	
*/
void __stdcall KxEOpenDumpMonitorEx(bool bShowUI, const wchar_t*pszUUID, DUMP_CHANNEL dwDumpChannel);


/**
* @brief	打开崩溃管理器
* @param	[in] bShowUI　		崩溃时是否显示对话框
* @param	[in] pszUUID        机器UUID
* @param	[in] dwProductId	产品ID，默认为0
* @return	[in] dwDumpChannel	上报到服务器端后的通道号标识,通过此标识可拿到对应的Dump文件,
								目前为不同的产品分配不同的通道号标识区间,产品自己可以为不同的进程设置通道号标识区间内的值来区分不同的进程

								各产品通道号范围划分,请各产品按以下分配的通道号范围来使用

								[0, 0]		测试通道
								[1,99]		安全100
								[100,199]	大引擎
								[200,299]	急救箱
								[300,399]	密报
								[400,499]	网盾

								例如:　急救箱的产品在使用就可能是这样的，KxEOpenDumpMonitorEx2(true, null, null, 0x100, 201),dwDumpChannel的值在[200,299]之间即可
* @remark	
*/
void __stdcall KxEOpenDumpMonitorEx2(bool bShowUI,
                                     const wchar_t *pszUUID, 
                                     DWORD dwProductId,
                                     DUMP_CHANNEL dwDumpChannel);


int __stdcall KxESetPostServer(
	const char* pszPostServerAddress
	);

/**
* @brief	设置是否启用体验模式
* @param	[in] bTrialMode		体验模式，默认为否
* @remark	
*/
void __stdcall KxESetTrialMode(
	bool bTrialMode
	);

void __stdcall KxRegisterUnhandledExceptionFilter(
	LPTOP_LEVEL_EXCEPTION_FILTER
	);

void __stdcall KxUnregisterUnhandledExceptionFilter(
	LPTOP_LEVEL_EXCEPTION_FILTER
	);



typedef void (__stdcall *PFN_KxEOpenDumpMonitor)();
typedef void (__stdcall *PFN_KxEOpenDumpMonitorEx)(bool, const wchar_t*, DWORD);
typedef void (__stdcall *PFN_KxEOpenDumpMonitorEx2)(bool, const wchar_t*, DWORD, DWORD);
typedef void (__stdcall *PFN_KxESetPostServer)(const char*);
typedef void (__stdcall *PFN_KxESetTrialMode)(bool);
typedef void (__stdcall *PFN_KxRegisterUnhandledExceptionFilter)(LPTOP_LEVEL_EXCEPTION_FILTER);
typedef void (__stdcall *PFN_KxUnregisterUnhandledExceptionFilter)(LPTOP_LEVEL_EXCEPTION_FILTER);