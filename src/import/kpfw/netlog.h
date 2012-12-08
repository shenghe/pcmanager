////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : netlog.h
//      Version   : 1.0
//      Comment   : 定义网镖泡泡日志相关结构
//      
//      Create at : 2008-7-21
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 

#include "netwatch.h"
#include <string>

#define KIS_LOG_CATCNT					10

#define	KPFW_LOG_BUFFER_SIZE			1024*64

//////////////////////////////////////////////////////////////////////////
// 定义各种日志类型
enum 
{
	kis_log_kpfw_ip				= 1,				// ip包过滤
	kis_log_kpfw_appvisit		= 2,				// 应用程序访问网络
	kis_log_kpfw_arp			= 3,				// arp防火墙访问信息
	kis_log_kpfw_apprule		= 4,				// 应用程序规则相关日志
	kis_log_kpfw_iprule			= 5,				// ip规则相关日志
	kis_log_kpfw_area			= 6,				// 区域规则相日志
	kis_log_kpfw_opt			= 7,				// 用户设置相关日志
	kis_log_kpfw_debug			= 8,				// debug 日志				
	kis_log_kpfw_update			= 9,				// 更新日志
	kis_log_kpfw_sheid			= 10,				// 动态防御规则
};

// 应用程序访问网络日志记录
typedef enum enAppLogFlag
{
	enumALF_Unknown				= 0,				// 未知处理

	enumALF_Allow_Appfail		= 1,				// 网镖出错,自动放行
	enumALF_Allow_Popfail		= 2,				// 泡泡弹出失败, 自动允许访问网络(除非是病毒)
	enumALF_Deny_Popfail		= 3,

	enumALF_Port_Deny			= 4,				// 应用程序端口被禁止
	enumALF_Rule_Deny			= 5,				// 是否是因为exe被禁止访问网络

	enumALF_Allow_Trust			= 6,				// 有模块需要进行可信认证，并且所有模块通过了可信认证,自动允许访问网络

	enumALF_Allow_Pop			= 7,				// 是否有自动判定为不允许，但是用户在pop中选择了允许

	enumALF_Pop_Deny			= 8,				// 是否是用户在pop中选择deny

	enumALF_Allow_Rule			= 9,				// 程序访问网络被允许

	enumALF_Allow_Auto			= 10,
}AppLogFlag;

// 应用程序规则创建日志记录
typedef enum enAppRulLogeFlag
{
	enumARLF_Unknown			= 0,				// 未知类型

	enumARLF_Rule_Add			= 1,				// 规则被添加

	enumARLF_Rule_Delete		= 2,				// 规则被删除

	enumARLF_Rule_Update		= 3,				// 规则被修改
}AppRuleLogFlag;

// 应用程序用户操作日志

typedef enum tagLogUserOptFlag
{
	enumLUO_Open				= 1,				//开启
	enumLUO_Close				= 2,				//关闭
}LogUserOpt;

typedef enum tagLogOptTypeFlag
{
	enumLOT_FireWall			= 1,				//防火墙
	enumLOT_NetWork				= 2,				//网络
}LogOptType;


//区域操作日志

typedef enum tagAreaLogOptFlag
{
	enumAL_SwitchArea			= 1,
	enumAL_ModifyInfo			= 2,
	enumAL_NewArea				= 3,
	enumAL_FindNewArea			= 3,
}AreaLogOpt;

typedef enum tagBehaviorType
{
	enumBT_AccessNet			= 1,
	enumBT_ToBeService			= 2
}BehaviorType;

//////////////////////////////////////////////////////////////////////////
// arp防火墙日志
typedef enum tagArpAttackType
{
	em_Send_Arp				= 1,			// 本机发送arp攻击
	em_Send_IpArp			= 2,			// 本机发送的ip冲突攻击
	em_Recieve_Arp			= 3,			// 接收到arp攻击
	em_Recieve_IpArp		= 4,			// 接收到的ip冲突攻击
}ArpAttackType;

struct KpfwLogHeader
{
	DWORD		nSize;
	DWORD		nType;
	ULONGLONG	nID;
};

struct KpfwLogContent 
{
	__time64_t		eventTime;
	std::wstring	strInfo;
	std::wstring	strResult;
	INT				nID;
};

//不要修改结果体成员变量顺序，否则也要修改 ToBuffer FromBuffer
struct KpfwLog
{
	int				nID;			// 本身ID
	int				nType;			// 日志类型
	int				nSize;			// sizeof(int) * 3 + strData.length 我们关心数据长度
	std::string		strData;		// 字符串长度

	void ToBuffer( BYTE* const buffer, int nLen )
	{
		if ( nLen != sizeof( int ) * 3 + strData.length() )
			return;

		if ( buffer )
		{
			memcpy( buffer, &nID, sizeof(int) );
			memcpy( buffer + sizeof(int) * 1,	&nType, sizeof(int) );
			memcpy( buffer + sizeof(int) * 2, &nSize, sizeof(int) );
			memcpy( buffer + sizeof(int) * 3, strData.c_str(), strData.length() );
		}

		return;
	}

	BOOL FromBuffer( BYTE* pBuffer )
	{
		memcpy( &nID,		pBuffer, sizeof(int) );
		memcpy( &nType,		pBuffer + sizeof(int) * 1, sizeof(int) );
		memcpy( &nSize,		pBuffer + sizeof(int) * 2, sizeof(int) );

		if ( !CheckLogType( nType ) )
			return FALSE;
		else
		{
			std::string strTemp( (char*) pBuffer + sizeof(int) * 3, nSize - sizeof(int) * 3 );
			strData = strTemp;
		}
		return TRUE;
	}

	BOOL CheckLogType( int nType )
	{
		BOOL bRetCode = FALSE;
		switch( nType )
		{
		case kis_log_kpfw_ip:
		case kis_log_kpfw_appvisit:
		case kis_log_kpfw_arp:
		case kis_log_kpfw_apprule:
		case kis_log_kpfw_iprule:
		case kis_log_kpfw_area:
		case kis_log_kpfw_opt:
		case kis_log_kpfw_debug:
		case kis_log_kpfw_update:
		case kis_log_kpfw_sheid:
			bRetCode = TRUE;
			break;
		default:
			bRetCode = FALSE;
			break;
		}
		return bRetCode;
	}

};

//////////////////////////////////////////////////////////////////////////
struct ARP_LOG_STASTIC
{
	ULONG					m_nAttackCnt;
	ULONG					m_lastAttackerIp;
};

struct ADVFILTER_LOG_STASTIC
{
	ULONG					m_nAttackCnt;
	GUID					m_lastAttackType;
};

struct DYN_ATTACK_LOG_STASTIC
{
	ULONG					m_nAttackCnt;
	ULONG					m_lastAttackerIp;
};

struct APPRULE_LOG_STASTIC
{
	ULONG					m_nModifyCnt;
	WCHAR					m_lastRulePath[MAX_PATH];
};

struct APPBLOCK_LOG_STASTIC
{
	ULONG					m_nBlockCnt;
	WCHAR					m_lastBlockExePath[MAX_PATH];
	ULONG					m_lastBlockReason;
};

struct NETMON_LOG_STASTIC
{
	ARP_LOG_STASTIC			m_arpLog;
	ADVFILTER_LOG_STASTIC	m_advFltLog;
	DYN_ATTACK_LOG_STASTIC	m_dynAttackLog;
	APPRULE_LOG_STASTIC		m_appRuleLog;
	APPBLOCK_LOG_STASTIC	m_appBlockLog;
};

//////////////////////////////////////////////////////////////////////////
// 自动创建应用程序规则
struct LOG_CREATE_APP_RULE: KpfwLogHeader
{
	
};

#define KPFW_LOGBUF_64KB	1024 * 64
