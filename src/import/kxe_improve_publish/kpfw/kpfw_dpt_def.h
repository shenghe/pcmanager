////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : kpfw_dpt_def.h
//      Version   : 1.0
//      Comment   : 定义网镖服务和界面之间的转HTTP_RPC接口定义文件
//      
//      Create at : 2009-07-09
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 

#include <string>
//#include "kpfw_interface_ex.h"
#include "../../src/import/kpfw/netwatch.h"
#include "include/idl.h"
#include "include/kxe_serialize.h"
#include "kxeproduct/kxeproduct_id.h"

#include "kpfw_common_dpt_def.h"

KXE_JSON_DDX_BEGIN(FILETIME)
KXE_JSON_DDX_MEMBER("dwLowDateTime", dwLowDateTime);
KXE_JSON_DDX_MEMBER("dwHighDateTime", dwHighDateTime);
KXE_JSON_DDX_END()

typedef struct  tagGUIDParam
{
	unsigned long	Data1;
	unsigned short	Data2;
	unsigned short	Data3;
	unsigned long	Data4;
	unsigned long	Data5;
} Kxe_GUID_Param;

KXE_JSON_DDX_BEGIN(Kxe_GUID_Param)
KXE_JSON_DDX_MEMBER("Data1", Data1);
KXE_JSON_DDX_MEMBER("Data2", Data2);
KXE_JSON_DDX_MEMBER("Data3", Data3);
KXE_JSON_DDX_MEMBER("Data4", Data4);
KXE_JSON_DDX_MEMBER("Data5", Data5);
KXE_JSON_DDX_END()


KXE_JSON_DDX_BEGIN(KIPRange)
KXE_JSON_DDX_MEMBER("ipStart",	ipStart);
KXE_JSON_DDX_MEMBER("ipEnd",	ipEnd);
KXE_JSON_DDX_END()


KXE_JSON_DDX_BEGIN(KPortRange)
KXE_JSON_DDX_MEMBER("nStart",	nStart);
KXE_JSON_DDX_MEMBER("nEnd",		nEnd);
KXE_JSON_DDX_END()


//////////////////////////////////////////////////////////////////////////
// ip 规则信息

struct IpAddrUnion
{
	KIPAddr						ipAddr;
	KIPRange					ipRange;
};

KXE_JSON_DDX_BEGIN(IpAddrUnion)
KXE_JSON_DDX_MEMBER("ipAddr", ipAddr);
KXE_JSON_DDX_MEMBER("ipRange", ipRange);
KXE_JSON_DDX_END()


typedef struct tagFilterIpAddParam
{
	int					eMode;
	IpAddrUnion			IpAddr;
}Kxe_FilterIPAddr_Param;

KXE_JSON_DDX_BEGIN(Kxe_FilterIPAddr_Param)
KXE_JSON_DDX_MEMBER("eMode", eMode);
KXE_JSON_DDX_MEMBER("IpAddr", IpAddr);
KXE_JSON_DDX_END()


struct PortUnion
{
	KPort			port;
	KPortRange		portRange;
};

KXE_JSON_DDX_BEGIN(PortUnion)
KXE_JSON_DDX_MEMBER("port", port);
KXE_JSON_DDX_MEMBER("portRange", portRange);
KXE_JSON_DDX_END()


typedef struct tagFilterPortParam
{
	int					eMode;
	PortUnion			port;	
}Kxe_FilterPort_Param;

KXE_JSON_DDX_BEGIN(Kxe_FilterPort_Param)
KXE_JSON_DDX_MEMBER("eMode", eMode);
KXE_JSON_DDX_MEMBER("port", port);
KXE_JSON_DDX_END()


typedef struct tagTCPDataParam 
{
	Kxe_FilterPort_Param			LocalPort;					// 本地端口
	Kxe_FilterPort_Param			RemotePort;					// 远程端口
	unsigned char					byTcpFlags;	
} Kxe_TCPData_Param;

KXE_JSON_DDX_BEGIN(Kxe_TCPData_Param)
KXE_JSON_DDX_MEMBER("LocalPort", LocalPort);
KXE_JSON_DDX_MEMBER("RemotePort", RemotePort);
KXE_JSON_DDX_MEMBER("byTcpFlags", byTcpFlags);
KXE_JSON_DDX_END()


typedef struct tagUDPDataParam 
{
	Kxe_FilterPort_Param			LocalPort;					// 本地端口
	Kxe_FilterPort_Param			RemotePort;	
} Kxe_UDPData_Param;

KXE_JSON_DDX_BEGIN(Kxe_UDPData_Param)
KXE_JSON_DDX_MEMBER("LocalPort", LocalPort);
KXE_JSON_DDX_MEMBER("RemotePort", RemotePort);
KXE_JSON_DDX_END()


typedef struct tagICMPDataParam 
{
	unsigned char		byType;						//255 means any
	unsigned char		byCode;	
} Kxe_ICMPData_Param;

KXE_JSON_DDX_BEGIN(Kxe_ICMPData_Param)
KXE_JSON_DDX_MEMBER("byType", byType);
KXE_JSON_DDX_MEMBER("byCode", byCode);
KXE_JSON_DDX_END()


typedef struct tagIGMPDataParam 
{
	unsigned char		byType;						//255 means any
	unsigned char		byTypeOfGroup;				//255 means any
	ULONG				uMulticastAddr;		
} Kxe_IGMPData_Param;

KXE_JSON_DDX_BEGIN(Kxe_IGMPData_Param)
KXE_JSON_DDX_MEMBER("byType", byType);
KXE_JSON_DDX_MEMBER("byTypeOfGroup", byTypeOfGroup);
KXE_JSON_DDX_MEMBER("uMulticastAddr", uMulticastAddr);
KXE_JSON_DDX_END()

struct DataUnion
{
	Kxe_TCPData_Param		TCP_DATA;					// TCP 设置
	Kxe_UDPData_Param		UDP_DATA;					// udp 设置
	Kxe_ICMPData_Param		ICMP_DATA;					// icmp 设置
	Kxe_IGMPData_Param		IGMP_DATA;					// igmp 设置数据	
};

KXE_JSON_DDX_BEGIN(DataUnion)
KXE_JSON_DDX_MEMBER("TCP_DATA",		TCP_DATA);
KXE_JSON_DDX_MEMBER("UDP_DATA",		UDP_DATA);
KXE_JSON_DDX_MEMBER("ICMP_DATA",	ICMP_DATA);
KXE_JSON_DDX_MEMBER("IGMP_DATA",	IGMP_DATA);
KXE_JSON_DDX_END()


typedef struct tagFilterInfoParam
{
	unsigned long		cbSize;			
	unsigned long		nVer;
	
	int			        ProtocolType;				// 协议
	int					Direction;	
	INT					Operation;

	Kxe_FilterIPAddr_Param		LocalAddr;
	Kxe_FilterIPAddr_Param		RemoteAddr;
	DataUnion					Data;
} Kxe_FilterInfo_Param;

KXE_JSON_DDX_BEGIN(Kxe_FilterInfo_Param)
KXE_JSON_DDX_MEMBER("cbSize",		cbSize);
KXE_JSON_DDX_MEMBER("nVer",			nVer);
KXE_JSON_DDX_MEMBER("Operation",	Operation);
KXE_JSON_DDX_MEMBER("ProtocolType", ProtocolType);
KXE_JSON_DDX_MEMBER("Direction",	Direction);
KXE_JSON_DDX_MEMBER("LocalAddr",	LocalAddr);
KXE_JSON_DDX_MEMBER("RemoteAddr",	RemoteAddr);
KXE_JSON_DDX_MEMBER("Data",			Data);
KXE_JSON_DDX_END()


typedef struct tagIpFilterInfo 
{
	Kxe_GUID_Param			guid;
	Kxe_FilterInfo_Param	filterInfo;
}Kxe_IPFilterInfo_Param;

KXE_JSON_DDX_BEGIN(Kxe_IPFilterInfo_Param)
KXE_JSON_DDX_MEMBER("guid", guid);
KXE_JSON_DDX_MEMBER("filterInfo", filterInfo);
KXE_JSON_DDX_END()


typedef struct tagIpRuleParam
{
	std::wstring	strName;
	std::wstring	strDesc;
	std::wstring	strCreator;
	std::wstring	strReason;
	std::wstring	strModifyer;

	BOOL			bValid;
	DWORD			pExternData;
	BOOL			bModify;

	FILETIME		timeCreate;
	FILETIME		timeModifyer;
	Kxe_IPFilterInfo_Param	IpInfo;
} Kxe_IpRule_Param;

KXE_JSON_DDX_BEGIN(Kxe_IpRule_Param)
KXE_JSON_DDX_MEMBER("strName",		strName);
KXE_JSON_DDX_MEMBER("strDesc",		strDesc);
KXE_JSON_DDX_MEMBER("strCreator",	strCreator);
KXE_JSON_DDX_MEMBER("strReason",	strReason);
KXE_JSON_DDX_MEMBER("strModifyer",	strModifyer);

KXE_JSON_DDX_MEMBER("bValid",		bValid);
KXE_JSON_DDX_MEMBER("pExternData",	pExternData);
KXE_JSON_DDX_MEMBER("bModify",		bModify);

KXE_JSON_DDX_MEMBER("timeCreate",	timeCreate);
KXE_JSON_DDX_MEMBER("timeModifyer", timeModifyer);
KXE_JSON_DDX_MEMBER("IpInfo",		IpInfo);
KXE_JSON_DDX_END()


typedef struct tagIpRuleVecParam
{
	vector< Kxe_IpRule_Param >	nRec;
	int							m_nTheSameRuleCnt;
} Kxe_IpRuleVec_Param;

KXE_JSON_DDX_BEGIN(Kxe_IpRuleVec_Param)
KXE_JSON_DDX_MEMBER("m_nTheSameRuleCnt", m_nTheSameRuleCnt);
KXE_JSON_DDX_MEMBER("nRec", nRec);
KXE_JSON_DDX_END()

typedef struct tagIpRuleRet
{
	vector< Kxe_IpRule_Param >	nRec;
	int							m_nTheSameRuleCnt;
	HRESULT						hr;
} Kxe_IpRuleVec_Ret;

KXE_JSON_DDX_BEGIN(Kxe_IpRuleVec_Ret)
KXE_JSON_DDX_MEMBER("m_nTheSameRuleCnt", m_nTheSameRuleCnt);
KXE_JSON_DDX_MEMBER("nRec", nRec);
KXE_JSON_DDX_MEMBER("hr", hr);
KXE_JSON_DDX_END()

typedef struct tagExportIpRuleParam
{
	Kxe_IpRuleVec_Param			nRec;
	wstring						strFileName;
}Kxe_ExportIpRuleVec_Param;

KXE_JSON_DDX_BEGIN(Kxe_ExportIpRuleVec_Param)
KXE_JSON_DDX_MEMBER("nRec", nRec);
KXE_JSON_DDX_MEMBER("FileName", strFileName);
KXE_JSON_DDX_END()

typedef struct tagExportIpRuleRet
{
	Kxe_IpRuleVec_Param			nRec;
	wstring						strFileName;
	HRESULT						hr;
}Kxe_ExportIpRuleVec_Ret;

KXE_JSON_DDX_BEGIN(Kxe_ExportIpRuleVec_Ret)
KXE_JSON_DDX_MEMBER("nRec", nRec);
KXE_JSON_DDX_MEMBER("FileName", strFileName);
KXE_JSON_DDX_MEMBER("hr", hr);
KXE_JSON_DDX_END()


typedef struct tagImportIpRuleParam
{
	wstring						strFileName;
}Kxe_ImportIpRuleVec_Param;

KXE_JSON_DDX_BEGIN(Kxe_ImportIpRuleVec_Param)
KXE_JSON_DDX_MEMBER("FileName", strFileName);
KXE_JSON_DDX_END()

typedef struct tagImportIpRuleRet
{
	Kxe_IpRuleVec_Param			nRec;
	HRESULT						hr;
}Kxe_ImportIpRuleVec_Ret;

KXE_JSON_DDX_BEGIN(Kxe_ImportIpRuleVec_Ret)
KXE_JSON_DDX_MEMBER("nRec", nRec);
KXE_JSON_DDX_MEMBER("hr", hr);
KXE_JSON_DDX_END()


interface IIpRuleInfoDpt
{
	IIpRuleInfoDpt(){};

	virtual ~IIpRuleInfoDpt(){};

	STDPROC_(int)		GetUserIpRules		( IN const Kxe_VOID_Param&,			OUT Kxe_IpRuleVec_Ret& )		PURE;
	STDPROC_(int)		UpdateUserIpRules	( IN const Kxe_IpRuleVec_Param&,	OUT Kxe_HRESULT_Ret& )			PURE;
	STDPROC_(int)		ExportIpRules		(IN const Kxe_ExportIpRuleVec_Param&, OUT Kxe_HRESULT_Ret& )		PURE;
	STDPROC_(int)		ImportIpRules		(IN const Kxe_ImportIpRuleVec_Param& inParam, OUT Kxe_ImportIpRuleVec_Ret& ) PURE;
	STDPROC_(int)		GetEnableAntiScan	(IN const Kxe_VOID_Param& inParam, OUT Kxe_BOOL_Ret& ) PURE;
	STDPROC_(int)		SetEnableAntiScan	(IN const Kxe_BOOL_Param& inParam, OUT Kxe_BOOL_Ret& ) PURE;
};

KXE_RCF_BEGIN_EX(IIpRuleInfoDpt, "KpfwSvrIpRuleInfo", "", "0x00000000", "kxespdt", true)
KXE_RCF_METHOD(GetUserIpRules,				Kxe_VOID_Param,			Kxe_IpRuleVec_Ret);
KXE_RCF_METHOD(UpdateUserIpRules,			Kxe_IpRuleVec_Param,	Kxe_HRESULT_Ret);

KXE_RCF_METHOD(ExportIpRules,				Kxe_ExportIpRuleVec_Param, Kxe_HRESULT_Ret);
KXE_RCF_METHOD(ImportIpRules,				Kxe_ImportIpRuleVec_Param, Kxe_ImportIpRuleVec_Ret);

KXE_RCF_METHOD(GetEnableAntiScan,				Kxe_VOID_Param, Kxe_BOOL_Ret);
KXE_RCF_METHOD(SetEnableAntiScan,				Kxe_BOOL_Param, Kxe_BOOL_Ret);
KXE_RCF_END()

// ip 规则信息
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// 区域规则

typedef struct tagMACParam 
{
	BYTE		 val0;
	BYTE		 val1;
	BYTE		 val2;
	BYTE		 val3;
	BYTE		 val4;
	BYTE		 val5;
} Kxe_MAC_Param;

KXE_JSON_DDX_BEGIN(Kxe_MAC_Param)
KXE_JSON_DDX_MEMBER("val0",			val0);
KXE_JSON_DDX_MEMBER("val1",			val1);
KXE_JSON_DDX_MEMBER("val2",			val2);
KXE_JSON_DDX_MEMBER("val3",			val3);
KXE_JSON_DDX_MEMBER("val4",			val4);
KXE_JSON_DDX_MEMBER("val5",			val5);
KXE_JSON_DDX_END()

typedef struct tagAreaParm
{
	std::wstring	strName;
	INT				nAreaType;
	Kxe_MAC_Param	vecGateMac;
	FILETIME		ftCreate;
	UINT			uIp;
	BOOL			bWireless;
	DWORD			nLastChangeTime;
} Kxe_Area_Param;

KXE_JSON_DDX_BEGIN(Kxe_Area_Param)
KXE_JSON_DDX_MEMBER("strName",			strName);
KXE_JSON_DDX_MEMBER("nAreaType",		nAreaType);
KXE_JSON_DDX_MEMBER("vecGateMac",		vecGateMac);
KXE_JSON_DDX_MEMBER("ftCreate",			ftCreate);
KXE_JSON_DDX_MEMBER("uIp",				uIp);
KXE_JSON_DDX_MEMBER("bWireless",		bWireless);
KXE_JSON_DDX_MEMBER("nLastChangeTime", nLastChangeTime);
KXE_JSON_DDX_END()


typedef struct tagAreaRet 
{
	Kxe_Area_Param area;
	HRESULT			hr;
} Kxe_Area_Ret;

KXE_JSON_DDX_BEGIN(Kxe_Area_Ret)
KXE_JSON_DDX_MEMBER("area",			area);
KXE_JSON_DDX_MEMBER("hr",			hr);
KXE_JSON_DDX_END()


typedef struct tagAreaVecRet 
{
	vector< Kxe_Area_Param >			vecArea;
	BOOL								bOwn;
	HRESULT								hr;
} Kxe_AreaVec_Ret;

KXE_JSON_DDX_BEGIN(Kxe_AreaVec_Ret)
KXE_JSON_DDX_MEMBER("vecArea",			vecArea);
KXE_JSON_DDX_MEMBER("bOwn",				bOwn);
KXE_JSON_DDX_MEMBER("hr",				hr);
KXE_JSON_DDX_END()


interface IAreaInfoDpt
{
	IAreaInfoDpt(){};
	virtual ~IAreaInfoDpt(){};

	STDPROC_(int) GetAreas			( IN const Kxe_VOID_Param&,		OUT Kxe_AreaVec_Ret&)		PURE;
	STDPROC_(int) GetCurArea		( IN const Kxe_VOID_Param&,		OUT Kxe_AreaVec_Ret&)		PURE;
	STDPROC_(int) DeleteArea		( IN const Kxe_Area_Param&,		OUT Kxe_HRESULT_Ret& )		PURE;
	STDPROC_(int) UpdateArea		( IN const Kxe_Area_Param&,		OUT Kxe_HRESULT_Ret& )		PURE;
	STDPROC_(int) GetTrustIpRule	( IN const Kxe_VOID_Param&,		OUT Kxe_IpRuleVec_Ret& )	PURE;
	STDPROC_(int) GetUnTrustIpRule	( IN const Kxe_VOID_Param&,		OUT Kxe_IpRuleVec_Ret& )	PURE;	
	STDPROC_(int) GetUnKnownIpRule	( IN const Kxe_VOID_Param&,		OUT Kxe_IpRuleVec_Ret& )	PURE;
	STDPROC_(int) GetArea			( IN const Kxe_MAC_Param&,		OUT Kxe_Area_Ret&)			PURE;
};

KXE_RCF_BEGIN_EX(IAreaInfoDpt, "KpfwSvrAreaInfo", "", "0x00000000", "kxespdt", true)
KXE_RCF_METHOD(GetAreas,			Kxe_VOID_Param,		Kxe_AreaVec_Ret);
KXE_RCF_METHOD(GetCurArea,			Kxe_VOID_Param,		Kxe_AreaVec_Ret);
KXE_RCF_METHOD(DeleteArea,			Kxe_Area_Param,		Kxe_HRESULT_Ret);
KXE_RCF_METHOD(UpdateArea,			Kxe_Area_Param,		Kxe_HRESULT_Ret);

KXE_RCF_METHOD(GetTrustIpRule,		Kxe_VOID_Param,		Kxe_IpRuleVec_Ret);
KXE_RCF_METHOD(GetUnTrustIpRule,	Kxe_VOID_Param,		Kxe_IpRuleVec_Ret);
KXE_RCF_METHOD(GetUnKnownIpRule,	Kxe_VOID_Param,		Kxe_IpRuleVec_Ret);
KXE_RCF_METHOD(GetArea,				Kxe_MAC_Param,		Kxe_Area_Ret);
KXE_RCF_END()

// 区域规则
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// 动态规则
typedef struct tagAttackInfoParam 
{
	KIPAddr				attackAddr;
	Kxe_GUID_Param		nReason;
	DWORD				nAttackTimes;
	DWORD				nTime;
} Kxe_AttackInfo_Param;

KXE_JSON_DDX_BEGIN(Kxe_AttackInfo_Param)
KXE_JSON_DDX_MEMBER("attackAddr",	attackAddr);
KXE_JSON_DDX_MEMBER("nReason",		nReason);
KXE_JSON_DDX_MEMBER("nAttackTimes", nAttackTimes);
KXE_JSON_DDX_MEMBER("nTime",		nTime);
KXE_JSON_DDX_END()


typedef struct tagAttckListRet
{
	vector<Kxe_AttackInfo_Param>		attackList;
	HRESULT								hr;
} Kxe_AttackList_Ret;

KXE_JSON_DDX_BEGIN(Kxe_AttackList_Ret)
KXE_JSON_DDX_MEMBER("attackList",	attackList);
KXE_JSON_DDX_MEMBER("hr",			hr);
KXE_JSON_DDX_END()


interface IAttackInfoDpt
{
	IAttackInfoDpt(){};
	virtual ~IAttackInfoDpt(){};

	STDPROC_(int) GetAttackList		( IN const Kxe_VOID_Param&,				OUT Kxe_AttackList_Ret& )		PURE;
	STDPROC_(int) DeleteAttackItem	( IN const Kxe_AttackInfo_Param&,		OUT Kxe_HRESULT_Ret& )			PURE;
};

KXE_RCF_BEGIN_EX(IAttackInfoDpt, "KpfwSvrAttackInfo", "", "0x00000000", "kxespdt", true)
KXE_RCF_METHOD(GetAttackList,			Kxe_VOID_Param,			Kxe_AttackList_Ret);
KXE_RCF_METHOD(DeleteAttackItem,		Kxe_AttackInfo_Param,	Kxe_HRESULT_Ret);
KXE_RCF_END()

// 动态规则
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// log 信息
typedef struct tagKpfwLogParam 
{
	int						nID;			// 本身ID
	int						nType;			// 日志类型
	int						nSize;			// sizeof(int) * 3 + strData.length 我们关心数据长度
	vector< unsigned char>	vecData;
} Kxe_KpfwLog_Param;

KXE_JSON_DDX_BEGIN(Kxe_KpfwLog_Param)
KXE_JSON_DDX_MEMBER("nID", nID);
KXE_JSON_DDX_MEMBER("nType", nType);
KXE_JSON_DDX_MEMBER("nSize", nSize);
KXE_JSON_DDX_MEMBER("vecData", vecData);
KXE_JSON_DDX_END()


typedef struct tagEventInfoRet
{
	INT							nMax;
	vector< Kxe_KpfwLog_Param >	Logs;
	Kxe_Init64_Param			uLogId;
	HRESULT						hr;
} Kxe_EventInfo_Ret;

KXE_JSON_DDX_BEGIN(Kxe_EventInfo_Ret)
KXE_JSON_DDX_MEMBER("nMax",		nMax);
KXE_JSON_DDX_MEMBER("Logs",		Logs);
KXE_JSON_DDX_MEMBER("uLogId",	uLogId);
KXE_JSON_DDX_MEMBER("hr",		hr);
KXE_JSON_DDX_END()


typedef struct tagARP_LOG_STASTIC
{
	ULONG					m_nAttackCnt;
	ULONG					m_lastAttackerIp;
}Kxe_ArpLog_Stastic_Info;

KXE_JSON_DDX_BEGIN(Kxe_ArpLog_Stastic_Info)
KXE_JSON_DDX_MEMBER("count",			m_nAttackCnt);
KXE_JSON_DDX_MEMBER("last_attack_ip",	m_lastAttackerIp);
KXE_JSON_DDX_END()

typedef struct tagADVFILTER_LOG_STASTIC
{
	ULONG								m_nAttackCnt;
	Kxe_GUID_Param						m_lastAttackType;
}Kxe_Advflt_Stastic_Info;

KXE_JSON_DDX_BEGIN(Kxe_Advflt_Stastic_Info)
KXE_JSON_DDX_MEMBER("count",			m_nAttackCnt);
KXE_JSON_DDX_MEMBER("last_attack_type",	m_lastAttackType);
KXE_JSON_DDX_END()

typedef struct tagDYN_ATTACK_LOG_STASTIC
{
	ULONG					m_nAttackCnt;
	ULONG					m_lastAttackerIp;
}Kxe_DnyAttack_Stastic_Info;

KXE_JSON_DDX_BEGIN(Kxe_DnyAttack_Stastic_Info)
KXE_JSON_DDX_MEMBER("count",			m_nAttackCnt);
KXE_JSON_DDX_MEMBER("last_attack_ip",	m_lastAttackerIp);
KXE_JSON_DDX_END()

typedef struct tagAPPRULE_LOG_STASTIC
{
	ULONG					m_nModifyCnt;
	wstring					m_lastRulePath;
}Kxe_ArpRule_Stastic_Info;
KXE_JSON_DDX_BEGIN(Kxe_ArpRule_Stastic_Info)
KXE_JSON_DDX_MEMBER("count",		m_nModifyCnt);
KXE_JSON_DDX_MEMBER("last_path",	m_lastRulePath);
KXE_JSON_DDX_END()

typedef struct tagAPPBLOCK_LOG_STASTIC
{
	ULONG					m_nBlockCnt;
	wstring					m_lastBlockExePath;
	ULONG					m_lastBlockReason;
}Kxe_AppBlock_Stastic_Info;
KXE_JSON_DDX_BEGIN(Kxe_AppBlock_Stastic_Info)
KXE_JSON_DDX_MEMBER("count",		m_nBlockCnt);
KXE_JSON_DDX_MEMBER("last_path",	m_lastBlockExePath);
KXE_JSON_DDX_MEMBER("last_reason",	m_lastBlockReason);
KXE_JSON_DDX_END()

typedef struct tagNetLogStstic
{
	Kxe_ArpLog_Stastic_Info		arpStastic;	
	Kxe_Advflt_Stastic_Info		advFltAttackStastic;
	Kxe_DnyAttack_Stastic_Info	dynAttackStastic;
	Kxe_ArpRule_Stastic_Info	appRuleStastic;
	Kxe_AppBlock_Stastic_Info	appBlockStastic;
	HRESULT						hr;
}Kxe_NetLogStastic_Ret;
KXE_JSON_DDX_BEGIN(Kxe_NetLogStastic_Ret)
KXE_JSON_DDX_MEMBER("arp_info",		arpStastic);
KXE_JSON_DDX_MEMBER("advflt_info",	advFltAttackStastic);
KXE_JSON_DDX_MEMBER("dyn_attack_info",	dynAttackStastic);
KXE_JSON_DDX_MEMBER("app_rule_info",	appRuleStastic);
KXE_JSON_DDX_MEMBER("app_block_info",	appBlockStastic);
KXE_JSON_DDX_MEMBER("hr",	hr);
KXE_JSON_DDX_END()


interface IKpfwLogInfoDpt
{
	STDPROC_(int) GetEventInfo ( IN const Kxe_Init64_Param&, OUT Kxe_EventInfo_Ret&)		PURE;
	STDPROC_(int) GetStasticInfo (IN const Kxe_VOID_Param&, OUT Kxe_NetLogStastic_Ret&)		PURE;
};

KXE_RCF_BEGIN_EX(IKpfwLogInfoDpt, "KpfwSvrLogInfo", "", "0x00000000", "kxespdt", true)
KXE_RCF_METHOD(GetEventInfo,			Kxe_Init64_Param,		Kxe_EventInfo_Ret);
KXE_RCF_METHOD(GetStasticInfo,			Kxe_VOID_Param,			Kxe_NetLogStastic_Ret);
KXE_RCF_END()

// log 信息
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// 应用程序规则信息

typedef struct tagMD5Param 
{
	long		 val0;
	long		 val1;
	long		 val2;
	long		 val3;
} Kxe_MD5_Param;

KXE_JSON_DDX_BEGIN(Kxe_MD5_Param)
KXE_JSON_DDX_MEMBER("val0",			val0);
KXE_JSON_DDX_MEMBER("val1",			val1);
KXE_JSON_DDX_MEMBER("val2",			val2);
KXE_JSON_DDX_MEMBER("val3",			val3);
KXE_JSON_DDX_END()

typedef struct tagAppRuleParam
{
	int						nRuleType;
	Kxe_GUID_Param			id;
	std::wstring			strExePath;
	Kxe_MD5_Param			checkSum;
	FILETIME				ftLast;
	FILETIME				ftLastTrustRep;

	std::wstring			strCreator;
	FILETIME				ftCreate;
	std::wstring			strReason;
	FILETIME				ftLastModify;
	std::wstring			m_strModifyer;
	vector<KPortRange>		tcpRemoteDenyRange;
	vector<KPortRange>		tcpLocalDenyRange;

	vector<KPortRange>		udpRemoteDenyRange;
	vector<KPortRange>		udpLocalDenyRange;

	INT						nUserMode;
	INT						nTrustMode;
	ULONG					lPathhashCode;
	DWORD					pExternData;
} Kxe_AppRule_Param;


KXE_JSON_DDX_BEGIN(Kxe_AppRule_Param)
KXE_JSON_DDX_MEMBER("nRuleType",		nRuleType);
KXE_JSON_DDX_MEMBER("id",				id);
KXE_JSON_DDX_MEMBER("strExePath",		strExePath);
KXE_JSON_DDX_MEMBER("checkSum",			checkSum);
KXE_JSON_DDX_MEMBER("ftLast",			ftLast);
KXE_JSON_DDX_MEMBER("ftLastTrustRep",	ftLastTrustRep);

KXE_JSON_DDX_MEMBER("strCreator",		strCreator);
KXE_JSON_DDX_MEMBER("ftCreate",			ftCreate);
KXE_JSON_DDX_MEMBER("strReason",		strReason);
KXE_JSON_DDX_MEMBER("ftLastModify",		ftLastModify);
KXE_JSON_DDX_MEMBER("m_strModifyer",	m_strModifyer);
KXE_JSON_DDX_MEMBER("tcpRemoteDenyRange",	tcpRemoteDenyRange);
KXE_JSON_DDX_MEMBER("tcpLocalDenyRange",	tcpLocalDenyRange);

KXE_JSON_DDX_MEMBER("udpRemoteDenyRange",	udpRemoteDenyRange);
KXE_JSON_DDX_MEMBER("udpLocalDenyRange",	udpLocalDenyRange);
KXE_JSON_DDX_MEMBER("nUserMode",			nUserMode);
KXE_JSON_DDX_MEMBER("nTrustMode",			nTrustMode);
KXE_JSON_DDX_MEMBER("lPathhashCode",		lPathhashCode);
KXE_JSON_DDX_MEMBER("pExternData",			pExternData);
KXE_JSON_DDX_END()


typedef struct tagAppRuleVecParam
{
	vector< Kxe_AppRule_Param >				vecAppRules;
} Kxe_AppRuleVec_Param;
	
KXE_JSON_DDX_BEGIN(Kxe_AppRuleVec_Param)
KXE_JSON_DDX_MEMBER("vecAppRules", vecAppRules);
KXE_JSON_DDX_END()


typedef struct tagAppRuleVecRet 
{
	vector< Kxe_AppRule_Param >				vecAppRules;
	ULONG									nVersion;
	HRESULT									hr;
} Kxe_AppRuleVec_Ret;

KXE_JSON_DDX_BEGIN(Kxe_AppRuleVec_Ret)
KXE_JSON_DDX_MEMBER("vecAppRules",	vecAppRules);
KXE_JSON_DDX_MEMBER("nVersion",	nVersion);
KXE_JSON_DDX_MEMBER("hr",			hr);
KXE_JSON_DDX_END()


interface IAppRuleInfoDpt
{
	STDPROC_(int) GetAppRules ( IN const Kxe_VOID_Param&,			OUT Kxe_AppRuleVec_Ret& )	PURE;
	STDPROC_(int) UpdateAppRules ( IN const Kxe_AppRuleVec_Param&,	OUT Kxe_HRESULT_Ret& )		PURE;
	STDPROC_(int) DeleteRule ( IN const Kxe_AppRule_Param&,			OUT	Kxe_HRESULT_Ret& )		PURE;
	STDPROC_(int) UpdateRule (IN const Kxe_AppRule_Param&,			OUT Kxe_HRESULT_Ret& )		PURE;
	STDPROC_(int) AddRule ( IN const Kxe_String_Param&,				OUT Kxe_HRESULT_Ret& )		PURE;
	STDPROC_(int) GetAppRulesVersion ( IN const Kxe_VOID_Param&,	OUT Kxe_DWORD_Ret& )		PURE;

	STDPROC_(int) GetModuleRules ( IN const Kxe_VOID_Param&,			OUT Kxe_AppRuleVec_Ret& )	PURE;
	STDPROC_(int) UpdateModuleRules ( IN const Kxe_AppRuleVec_Param&,	OUT Kxe_HRESULT_Ret& )		PURE;
	STDPROC_(int) DeleteModuleRule ( IN const Kxe_AppRule_Param&,		OUT	Kxe_HRESULT_Ret& )		PURE;
	STDPROC_(int) UpdateModuleRule (IN const Kxe_AppRule_Param&,		OUT Kxe_HRESULT_Ret& )		PURE;
	STDPROC_(int) AddModuleRule ( IN const Kxe_String_Param&,			OUT Kxe_HRESULT_Ret& )		PURE;
	STDPROC_(int) GetModuleRulesVersion ( IN const Kxe_VOID_Param&,		OUT Kxe_DWORD_Ret& )		PURE;
};


KXE_RCF_BEGIN_EX(IAppRuleInfoDpt, "KpfwSvrAppRuleInfo", "", "0x00000000", "kxespdt", true)
KXE_RCF_METHOD(GetAppRules,			Kxe_VOID_Param,			Kxe_AppRuleVec_Ret);
KXE_RCF_METHOD(UpdateAppRules,		Kxe_AppRuleVec_Param,	Kxe_HRESULT_Ret);
KXE_RCF_METHOD(DeleteRule,			Kxe_AppRule_Param,		Kxe_HRESULT_Ret);
KXE_RCF_METHOD(UpdateRule,			Kxe_AppRule_Param,		Kxe_HRESULT_Ret);
KXE_RCF_METHOD(AddRule,				Kxe_String_Param,		Kxe_HRESULT_Ret);
KXE_RCF_METHOD(GetAppRulesVersion,	Kxe_VOID_Param,			Kxe_DWORD_Ret);

KXE_RCF_METHOD(GetModuleRules,		Kxe_VOID_Param,			Kxe_AppRuleVec_Ret);
KXE_RCF_METHOD(UpdateModuleRules,	Kxe_AppRuleVec_Param,	Kxe_HRESULT_Ret);
KXE_RCF_METHOD(DeleteModuleRule,	Kxe_AppRule_Param,		Kxe_HRESULT_Ret);
KXE_RCF_METHOD(UpdateModuleRule,	Kxe_AppRule_Param,		Kxe_HRESULT_Ret);
KXE_RCF_METHOD(AddModuleRule,		Kxe_String_Param,		Kxe_HRESULT_Ret);
KXE_RCF_METHOD(GetModuleRulesVersion,	Kxe_VOID_Param,			Kxe_DWORD_Ret);
KXE_RCF_END()

// 应用程序规则信息
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// arp 防火墙关联

typedef struct tagMacIpParam
{
	Kxe_MAC_Param		mac;
	UINT				ip;
} Kxe_MacIp_Param;


KXE_JSON_DDX_BEGIN(Kxe_MacIp_Param)
KXE_JSON_DDX_MEMBER("mac",	mac);
KXE_JSON_DDX_MEMBER("ip",	ip);
KXE_JSON_DDX_END()


typedef struct tagArpHostInfo 
{
	INT							nHostIpCnt;
	vector< Kxe_MacIp_Param >	vecHostMacIp;
	INT							nGateWayCnt;
	vector< Kxe_MacIp_Param >	vecGetWay;
} Kxe_ArpHostInfo_Param;

KXE_JSON_DDX_BEGIN(Kxe_ArpHostInfo_Param)
KXE_JSON_DDX_MEMBER("nHostIpCnt",			nHostIpCnt);
KXE_JSON_DDX_MEMBER("vecHostMacIp",			vecHostMacIp);
KXE_JSON_DDX_MEMBER("nGateWayCnt",			nGateWayCnt);
KXE_JSON_DDX_MEMBER("vecGetWay",			vecGetWay);
KXE_JSON_DDX_END()


typedef struct tagArpHostinfoRet 
{
	Kxe_ArpHostInfo_Param		hostInfo;
	HRESULT						hr;
}Kxe_ArpHostInfo_Ret;

KXE_JSON_DDX_BEGIN(Kxe_ArpHostInfo_Ret)
KXE_JSON_DDX_MEMBER("hostInfo",			hostInfo);
KXE_JSON_DDX_MEMBER("hr",				hr);
KXE_JSON_DDX_END()


typedef struct tagMacIpRet
{
	Kxe_MacIp_Param		macIp;
	HRESULT				hr;
} Kxe_MacIp_Ret;

KXE_JSON_DDX_BEGIN(Kxe_MacIp_Ret)
KXE_JSON_DDX_MEMBER("macIp",	macIp);
KXE_JSON_DDX_MEMBER("hr",		hr);
KXE_JSON_DDX_END()



typedef struct tagKAdapterInfoParam
{
	std::wstring		ConnGuidId;
	std::wstring		ConnName;
	int					ConnStatus;
	int					ConnMedia;
	std::wstring		DeviceName;

	ULONG				uHostIp;
	Kxe_MAC_Param		vecHostMac;
	ULONG				uGatewayIp;
	Kxe_MAC_Param		vecGatewayMac;

	std::wstring		strHostName;
	std::wstring		strGateName;
	BOOL				bWireless;				// 是否是无线网
	BOOL				bShared;				// 是否被共享了
	BOOL				bAttackGate;				// 是否受到攻击
	BOOL				bAttackInter;				// 是否受到攻击
	Kxe_Init64_Param	adapterFlux;				// 该网卡上面的流量

}Kxe_KAdapterInfo_Param;

KXE_JSON_DDX_BEGIN(Kxe_KAdapterInfo_Param)
KXE_JSON_DDX_MEMBER("ConnGuidId",		ConnGuidId);
KXE_JSON_DDX_MEMBER("ConnName",			ConnName);
KXE_JSON_DDX_MEMBER("ConnStatus",		ConnStatus);
KXE_JSON_DDX_MEMBER("ConnMedia",		ConnMedia);
KXE_JSON_DDX_MEMBER("DeviceName",		DeviceName);

KXE_JSON_DDX_MEMBER("uHostIp",			uHostIp);
KXE_JSON_DDX_MEMBER("HostMac",			vecHostMac);
KXE_JSON_DDX_MEMBER("uGatewayIp",		uGatewayIp);
KXE_JSON_DDX_MEMBER("GatewayMac",		vecGatewayMac);

KXE_JSON_DDX_MEMBER("strHostName",		strHostName);
KXE_JSON_DDX_MEMBER("strGateName",		strGateName);
KXE_JSON_DDX_MEMBER("bWireless",		bWireless);
KXE_JSON_DDX_MEMBER("bShared",			bShared);
KXE_JSON_DDX_MEMBER("bAttackGate",		bAttackGate);
KXE_JSON_DDX_MEMBER("bAttackInter",		bAttackInter);
KXE_JSON_DDX_MEMBER("adapterFlux",		adapterFlux);
KXE_JSON_DDX_END()


typedef struct tagKHostAdapterInfoRet
{
	vector<Kxe_KAdapterInfo_Param>		info;
	HRESULT								hr;
}Kxe_KHostAdapterInfo_Ret;

KXE_JSON_DDX_BEGIN(Kxe_KHostAdapterInfo_Ret)
KXE_JSON_DDX_MEMBER("info",				info);
KXE_JSON_DDX_MEMBER("hr",				hr);
KXE_JSON_DDX_END()


typedef struct tagGetWayParam 
{
	BOOL					bAuto;
	vector<Kxe_MacIp_Param>	getway;
}Kxe_Getway_Param;

KXE_JSON_DDX_BEGIN(Kxe_Getway_Param)
KXE_JSON_DDX_MEMBER("bAuto",				bAuto);
KXE_JSON_DDX_MEMBER("getway",				getway);
KXE_JSON_DDX_END()


typedef struct  tagGetWayRet 
{
	BOOL					bAuto;
	vector<Kxe_MacIp_Param>	getway;
	HRESULT					hr;
} Kxe_Getway_Ret;

KXE_JSON_DDX_BEGIN(Kxe_Getway_Ret)
KXE_JSON_DDX_MEMBER("bAuto",				bAuto);
KXE_JSON_DDX_MEMBER("getway",				getway);
KXE_JSON_DDX_MEMBER("hr",					hr);
KXE_JSON_DDX_END()


typedef struct NotifyGetwayParam 
{
	BOOL			bNotify;
	INT				nTimes;
} Kxe_NotifyGetway_Param;

KXE_JSON_DDX_BEGIN(Kxe_NotifyGetway_Param)
KXE_JSON_DDX_MEMBER("bNotify",				bNotify);
KXE_JSON_DDX_MEMBER("nTimes",				nTimes);
KXE_JSON_DDX_END()


typedef struct NotifyGetwayRet 
{
	BOOL			bNotify;
	INT				nTimes;
	HRESULT			hr;
} Kxe_NotifyGetway_Ret;

KXE_JSON_DDX_BEGIN(Kxe_NotifyGetway_Ret)
KXE_JSON_DDX_MEMBER("bNotify",				bNotify);
KXE_JSON_DDX_MEMBER("nTimes",				nTimes);
KXE_JSON_DDX_MEMBER("hr",					hr);
KXE_JSON_DDX_END()


typedef struct tagArpAdvParam 
{
	BOOL	bSafeMode;
	BOOL	bBlockRecvAttack;
	BOOL	bBlockRecvIPAttack;
	BOOL	bBlockSendAttack;
} Kxe_ArpAdv_Param;

KXE_JSON_DDX_BEGIN(Kxe_ArpAdv_Param)
KXE_JSON_DDX_MEMBER("safe_mode",		    bSafeMode);
KXE_JSON_DDX_MEMBER("block_recv_attack",    bBlockRecvAttack);
KXE_JSON_DDX_MEMBER("block_recv_ip_attack", bBlockRecvIPAttack);
KXE_JSON_DDX_MEMBER("block_send_attack",    bBlockSendAttack);
KXE_JSON_DDX_END()


typedef struct  tagArpAdvRet 
{
	Kxe_ArpAdv_Param	arpdav;
	HRESULT				hr;
} Kxe_ArpAdv_Ret;

KXE_JSON_DDX_BEGIN(Kxe_ArpAdv_Ret)
KXE_JSON_DDX_MEMBER("arpdav",				arpdav);
KXE_JSON_DDX_MEMBER("hr",					hr);
KXE_JSON_DDX_END()


interface IArpInfoDpt
{
	STDPROC_(int) GetHostInfo			(IN const Kxe_VOID_Param&,			OUT Kxe_ArpHostInfo_Ret& )	PURE;
	STDPROC_(int) GetHostAdapterInfo	(IN const Kxe_VOID_Param&,			OUT Kxe_KHostAdapterInfo_Ret&) PURE;
	STDPROC_(int) Ping					(IN const Kxe_String_Param&,		OUT Kxe_HRESULT_Ret& )		PURE;

	// 启用arp防火墙
	STDPROC_(int) GetEnable				(IN const Kxe_VOID_Param&,			OUT Kxe_INTHResult_Ret& )	PURE;
	STDPROC_(int) SetEnable				(IN const Kxe_BOOL_Param&,			OUT Kxe_HRESULT_Ret&)		PURE;

	// 使用手动设置的网关或者自动获取网关
	STDPROC_(int) GetAutoGateWay		(IN const Kxe_VOID_Param&,			OUT Kxe_Getway_Ret&)		PURE;
	STDPROC_(int) SetAutoGateWay		(IN const Kxe_Getway_Param&,		OUT Kxe_HRESULT_Ret&)		PURE;
	STDPROC_(int) GetMacFromIp			(IN const Kxe_MacIp_Param&,			OUT Kxe_MacIp_Ret&)			PURE;

	//arp 高级设置
	STDPROC_(int)	GetNotifyGateway	(IN const Kxe_VOID_Param&,			OUT Kxe_NotifyGetway_Ret&)	PURE;
	STDPROC_(int)	SetNotifyGateway	(IN const Kxe_NotifyGetway_Param&,	OUT Kxe_HRESULT_Ret&)		PURE;

	STDPROC_(int)	GetAdvSetting		(IN const Kxe_VOID_Param&,			OUT Kxe_ArpAdv_Ret& outParam)	PURE;
	STDPROC_(int)	SetAdvSetting		(IN const Kxe_ArpAdv_Param& ,		OUT Kxe_HRESULT_Ret& outParam)	PURE;

	STDPROC_(int)	Refresh				(IN const Kxe_VOID_Param&,			OUT Kxe_HRESULT_Ret&)		PURE;
	STDPROC_(int)	GetLastLog		    (IN const Kxe_VOID_Param&,			OUT Kxe_StringHResult_Ret&)	PURE;
	STDPROC_(int)	ApplySetting		(IN const Kxe_VOID_Param&,			OUT Kxe_HRESULT_Ret&)		PURE;
    STDPROC_(int)	ClearProtectList    (IN const Kxe_VOID_Param&,			OUT Kxe_HRESULT_Ret&)		PURE;
};

KXE_RCF_BEGIN_EX(IArpInfoDpt, "KpfwSvrArpInfo", "", "0x00000000", "kxespdt", true)
KXE_RCF_METHOD(GetHostInfo,					Kxe_VOID_Param,			Kxe_ArpHostInfo_Ret);
KXE_RCF_METHOD(GetHostAdapterInfo,			Kxe_VOID_Param,			Kxe_KHostAdapterInfo_Ret);
KXE_RCF_METHOD(Ping,						Kxe_String_Param,		Kxe_HRESULT_Ret);
KXE_RCF_METHOD(GetEnable,					Kxe_VOID_Param,			Kxe_INTHResult_Ret);
KXE_RCF_METHOD(SetEnable,					Kxe_BOOL_Param,			Kxe_HRESULT_Ret);
KXE_RCF_METHOD(GetAutoGateWay,				Kxe_VOID_Param,			Kxe_Getway_Ret);
KXE_RCF_METHOD(SetAutoGateWay,				Kxe_Getway_Param,		Kxe_HRESULT_Ret);
KXE_RCF_METHOD(GetMacFromIp,				Kxe_MacIp_Param,		Kxe_MacIp_Ret);
KXE_RCF_METHOD(GetNotifyGateway,			Kxe_VOID_Param,			Kxe_NotifyGetway_Ret);
KXE_RCF_METHOD(SetNotifyGateway,			Kxe_NotifyGetway_Param,	Kxe_HRESULT_Ret);
KXE_RCF_METHOD(GetAdvSetting,				Kxe_VOID_Param,			Kxe_ArpAdv_Ret);
KXE_RCF_METHOD(SetAdvSetting,				Kxe_ArpAdv_Param,		Kxe_HRESULT_Ret);
KXE_RCF_METHOD(Refresh,						Kxe_VOID_Param,			Kxe_HRESULT_Ret);
KXE_RCF_METHOD(GetLastLog,				    Kxe_VOID_Param,			Kxe_StringHResult_Ret);
KXE_RCF_METHOD(ApplySetting,				Kxe_VOID_Param,			Kxe_HRESULT_Ret);
KXE_RCF_METHOD(ClearProtectList,			Kxe_VOID_Param,			Kxe_HRESULT_Ret);
KXE_RCF_END()
// arp 防火墙关联
//////////////////////////////////////////////////////////////////////////

