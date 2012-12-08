////////////////////////////////////////////////////////////////////////////////
//      
//      File for kisnetproxy
//      
//      File      : net_proxy.h
//      Version   : 1.0
//      Comment   : 驱动代理和驱动代理控制器之间的rpc通讯接口定义
//      
//      Create at : 2008-9-27
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 

#include "kisrpc/rfcdef.h"
#include "SCOM/SCOM/SCOMBase.h"
#include "SCOM/SCOM/SCOMDef.h"
#include "unknwn.h"

#include "netwatch.h"
#include "antiarp/arpcommondef.h"
#include "antiarp/pkthdr.h"
#include "netdef.h"

#define PROXY_DRIVER_CONTROLER_IPC				TEXT("\\\\.\\pipe\\KIS-A2323F3F-B5A2-466c-905B-AC45F2C394CC")

//////////////////////////////////////////////////////////////////////////
// 定义网络驱动接口
RCF_CLASS_BEGIN(ProxyNetDriver)
	RCF_METHOD_R_0( DWORD, GetControlProcessID)
	RCF_METHOD_R_2( DWORD, SetTdiEvents, DWORD, DWORD)

	RCF_METHOD_R_1( DWORD, GetAppRequests, vector<APP_REQUEST_INFO>*)
	RCF_METHOD_R_2( DWORD, GetAppResponse, APP_REQUEST_INFO, ULONG)

	RCF_METHOD_R_1( DWORD, GetIpRuleLogs, vector<LOG_IP_RULE>*)
	
	RCF_METHOD_R_2( DWORD, GetFlux, PULONGLONG, PULONGLONG)
	RCF_METHOD_R_1( DWORD, GetConnectInfos, vector<TCP_CONN_INFO>*)

	RCF_METHOD_R_1( DWORD, SetEnableFw, BOOL)
	RCF_METHOD_R_1( DWORD, SetEnableNetwork, BOOL)

	RCF_METHOD_R_2( DWORD, AddIpRules, vector<KIP_FILTER_INFO>, BOOL)

	RCF_METHOD_R_1( DWORD, RemoveIpRules, GUID)
	RCF_METHOD_R_0( DWORD, RemoveAllIpRules)
RCF_CLASS_END

//////////////////////////////////////////////////////////////////////////
// 定义arp防火墙驱动接口
RCF_CLASS_BEGIN(ProxyArpDriver)
	RCF_METHOD_R_0( DWORD, GetControlProcessID)
	// 发送arp阻止包信息, 和发送ip地址改变信息event
	RCF_METHOD_R_1( DWORD, SetArpBlockEvent, DWORD)
	RCF_METHOD_R_1( DWORD, SetCardEvent, DWORD)

	// 获取被拦截的arp包和被拦截的类型
	RCF_METHOD_R_2( DWORD, GetBlockPackets, vector<block_arp_packet>*, vector<int>*)

	// 获取当前系统网卡和网关信息
	RCF_METHOD_R_2( DWORD, Get_IPMAC_Info, vector<_mac_ip>*, vector<_mac_ip>*)

	RCF_METHOD_R_0( DWORD, GetStatus)
	RCF_METHOD_R_1( DWORD, SetStatus, DWORD)

	RCF_METHOD_R_1( DWORD, SetAntiArpConfig, antiarp_config)
	RCF_METHOD_R_1( DWORD, SetAntiArpProtectList, vector<protect_addr>)
	
RCF_CLASS_END

//////////////////////////////////////////////////////////////////////////
// 定义工具接口
RCF_CLASS_BEGIN(ProxyNetTools)
	RCF_METHOD_R_3( DWORD, GetProcInfo, APP_REQUEST_INFO, ATL::CString*, vector<ATL::CString>*)
	RCF_METHOD_R_1( BOOL,  IsCoperate, ATL::CString)
	RCF_METHOD_R_1( BOOL,  IsDigSign, ATL::CString)
	RCF_METHOD_R_2( DWORD,  CalcCheckSum, ATL::CString, vector<BYTE>*)
RCF_CLASS_END

//////////////////////////////////////////////////////////////////////////
// 定义可信认证接口
RCF_CLASS_BEGIN(ProxyRepTrust)
	RCF_METHOD_R_4( DWORD, GetRepResult, ATL::CString, DWORD*, vector<BYTE>*, ATL::CString*)
	RCF_METHOD_R_1( DWORD, UploadFile, ATL::CString)
RCF_CLASS_END
