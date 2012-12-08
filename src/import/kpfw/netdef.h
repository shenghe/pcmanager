////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : netdef.h
//      Version   : 1.0
//      Comment   : 定义网镖常用的类型
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 

//////////////////////////////////////////////////////////////////////////
// 应用程序规则类型
typedef enum emAppRuleType
{
	enumART_Unknown		= 0,							// 未知类型
	enumART_AllowAways	= 1,							// 一直允许
	enumART_AllowOnce	= 2,							// 允许一次
	enumART_AllowThisProcess = 3,						// 本次进程允许

	enumART_DenyAways	= 4,							// 一直禁止
	enumART_DenyOnce	= 5,							// 禁止本次
	enumART_DenyNextProc = 6,							// 禁止道下次重启进程

	enumART_Prompt		= 7,							// 总是询问
}AppRuleType;
//------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// 规则用户选择类型
typedef enum emUserSelectMode
{
	enumUSM_Mask			= 0xffff0000,				// 掩码

	enumUSM_Auto		= 1	<< 16,						// 使用可信认证结果判定
	
		enumUSM_Auto_Auto = 0,							// 未知程序
		enumUSM_Allow_Pop	= 2,						// 泡泡记住放行
		enumUSM_Deny_Pop	= 4,						// 泡泡记住禁止
		enumUSM_Allow_SafeDign	= 5,					// 安全程序

	enumUSM_Allow		= 2 << 16,						// 除非被修改，否则放行
	enumUSM_Deny		= 3	<< 16,						// 总是禁止
	enumUSM_Prompt		= 4 << 16,						// 总是弹出提示
} UserSelectMode;
//------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// 可信认证类型
typedef enum emTrustMode
{
	enumTM_Mask			= 0xffff0000,					// 掩码

	enumTM_Unknown		= 1 << 16,						// 当前未知程序
	enumTM_Safe			= 2 << 16,						// 可信认证自动放行
	enumTM_Danger		= 3 << 16,						// 泡泡记住放行
} TrustMode;

// 可信认证子类型
typedef enum emTrustSubMode
{
	enumTSM_DigtSign				= 1,				// 数字签名
	enumTSM_Cooperate				= 2,				// 合作伙伴

	enumTSM_TrustRep				= 3,				// 可信认证

	enumTSM_ServerBusy				= 4,				// 服务器忙
	enumTSM_TrustUnkonwn			= 5,				// 可信认证未知

	enumTSM_Virus					= 6,				// 病毒
	enumTSM_Hack					= 7,				// 黑客软件
	enumTSM_Worm					= 8,				// 蠕虫
	enumTSM_Infected				= 9,				// 被感染
	enumTSM_RiskWare				= 10,				// 危险软件
	enumTSM_Other_Virus				= 11,				// 其他未知病毒

	enumTSM_TrustFailed				= 12,				// 可信认证失败
} TrustSubMode;
//------------------------------------------------------------------------
