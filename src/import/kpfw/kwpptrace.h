////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : kwpptrace.h
//      Version   : 1.0
//      Comment   : 定义WPP需要的数据
//      
//      Create at : 2008-9-10
//      Create by : yangzhenhui
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once

extern ULONG  g_TraceFlag;
#define WPP_CONTROL_GUIDS \
	WPP_DEFINE_CONTROL_GUID(CtlGuid,(A6D44C8C,A8D8,4ad1,8684,97D43E2D7809),  \
	WPP_DEFINE_BIT(FLAG_MAIN)				\
	WPP_DEFINE_BIT(FLAG_TDI_HOOK)           \
	WPP_DEFINE_BIT(FLAG_TCP)                \
	WPP_DEFINE_BIT(FLAG_UDP)				\
	WPP_DEFINE_BIT(FLAG_RAWIP)				\
	WPP_DEFINE_BIT(FLAG_LAN)				\
	WPP_DEFINE_BIT(FLAG_NDIS_HOOK)			\
	WPP_DEFINE_BIT(FLAG_NDIS_FILTER)		\
	WPP_DEFINE_BIT(FLAG_FILE_MON)			\
	WPP_DEFINE_BIT(FLAG_STACK)				\
	WPP_DEFINE_BIT(FLAG_FILTER))

#if DBG == 1
#define WPP_DEBUG(_msg_) DbgPrint _msg_, DbgPrint("\n")
#define WPP_LEVEL_FLAGS_ENABLED(level, flags) ( (1 << WPP_BIT_##flags) & g_TraceFlag )
#else
#define WPP_LEVEL_FLAGS_ENABLED(level, flags) (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= level)
#endif

#define WPP_LEVEL_FLAGS_LOGGER(level,flags) WPP_LEVEL_LOGGER(flags)
#define WPP_FLAG_EXP_PRE(FLAGS, HR) {if (HR != STATUS_SUCCESS) {
#define WPP_FLAG_EXP_POST(FLAGS, HR) ;}}
#define WPP_FLAG_EXP_ENABLED(FLAGS, HR) WPP_FLAG_ENABLED(FLAGS)
#define WPP_FLAG_EXP_LOGGER(FLAGS, HR) WPP_FLAG_LOGGER(FLAGS)


typedef enum tagIpAddrMode {
	enumAM_Any = 0,									// 匹配任何地址
	enumAM_Single = 1,								// 匹配一个ip地址
	enumAM_Range = 2,								// 匹配一个范围
	enumAM_SingleList = 3,							// 匹配一个列表
	enumAM_Area = 4,								// 匹配一个区域
	enumAM_RollBack = 5,							// 匹配回环地址：127.0.0.1
} IP_ADDR_MODE;

typedef enum tagPortMode {
	enumPM_Any = 0,
	enumPM_Single = 1,
	enumPM_Range = 2,
	enumPM_SingleList = 3,
} PORT_MODE;

typedef enum tagMatchOperator
{
	enumMO_Log					= 1,				// 匹配时记录日志
	enumMO_PlaySound			= 1 << 1,			// 匹配时播放声音
	enumMO_Deny					= 1 << 2,			// 匹配时阻止
	enumMO_Warning				= 1 << 3,			// 匹配时发出警告
} MATCH_OPERATOR;

typedef enum tagIpPacketDirection
{
	enumPD_Send					= 1,				// 发送的数据包
	enumPD_Recieve				= 2,				// 接受的数据包
	enumPD_Both					= 3,				// 双向
} PACKET_DIR;

typedef enum tagProtoType {
	enumPT_TCP = 0,
	enumPT_UDP = 1,
	enumPT_ICMP = 2,
	enumPT_IP = 3,
	enumPT_IGMP = 4,
	enumPT_RAWIP = 5,
	enumPT_LANMAN = 6,
	enumPT_HTTP = 7,
	enumPT_FILE_MON = 8,
	enumPT_PROCESS_MON = 9,
	enumPT_Unknow = -1,
} PROTO_TYPE;

//////////////////////////////////////////////////////////////////////////
// 
typedef enum tagRequestType {

	enumRT_ProcessParent = 0,
	enumRT_AppRule = 1,
	enumRT_Max = 2

} REQUEST_TYPE;


typedef enum tagTcpState {

	enumTS_None,
	enumTS_SynSent,
	enumTS_SynRcvd,
	enumTS_EstablishedIn,
	enumTS_EstablishedOut,
	enumTS_FinWait1,
	enumTS_FinWait2,
	enumTS_TimeWait,
	enumTS_CloseWait,
	enumTS_LastAck,
	enumTS_Closed,
	enumTS_Max
} TCPSTATE;

typedef enum tagApplicationRequestType {
	ART_Connect = 0,
	ART_Listen = 1,
	ART_Accept = 2,
	ART_Bind = 3,
	ART_Close = 4,
	ART_Create = 5,
	ART_TrustUrl = 6,
	ART_DownloadPeFile = 7, 
	ART_FileChanged = 8,
	ART_PacketRecord = 9,
	ART_CreatePeFile = 10,
	ART_RenameFile = 11,
	ART_LoadImage = 12,
} APP_REQUEST_TYPE;

// begin_wpp config
// CUSTOM_TYPE(REQUEST_TYPE, ItemEnum(tagApplicationRequestType));
// end_wpp


typedef enum tagApplicationRequestResult {
	Pass = 0,
	Deny = 1,
	AlwaysPass = 2,
	AlwaysDeny = 3,
	NoFoundKfw = 4,
	KfwDisabled = 5,
	KfwRedirect = 6
} APP_REQUEST_RESULT;

// begin_wpp config
// CUSTOM_TYPE(REQUEST_RESULT, ItemEnum(tagApplicationRequestResult));
// end_wpp

typedef enum tagWarningType {
	NoFoundConnection = 0,
	NoFoundAddrObject = 1
} WARNING_TYPE;

// begin_wpp config
// CUSTOM_TYPE(WARNING_TYPE, ItemEnum(tagWarningType));
// end_wpp

typedef enum tagSetTdiEventType {
	_TDI_EVENT_CONNECT = 0,
	_TDI_EVENT_DISCONNECT = 1,
	_TDI_EVENT_ERROR = 2,
	_TDI_EVENT_RECEIVE = 3,
	_TDI_EVENT_RECEIVE_DATAGRAM = 4,
	_TDI_EVENT_RECEIVE_EXPEDITED = 5,
	_TDI_EVENT_SEND_POSSIBLE = 6,
	_TDI_EVENT_CHAINED_RECEIVE = 7,
	_TDI_EVENT_CHAINED_RECEIVE_DATAGRAM = 8,
	_TDI_EVENT_CHAINED_RECEIVE_EXPEDITED = 9,
	_TDI_EVENT_ERROR_EX = 10
}SET_TDI_EVENT_TYPE;

// begin_wpp config
// CUSTOM_TYPE(SetEventType, ItemEnum(tagSetTdiEventType));
// end_wpp


typedef enum tagEnableFunction {
	ENABLE_TCP				= 0x1,
	ENABLE_UDP				= 0x2,
	ENABLE_RAWIP			= 0x4,
	ENABLE_LANMAN			= 0x8,
	ENABLE_PROG_TRAFFIC		= 0x40,
	ENABLE_USER_SENDER		= 0x80,
	ENABLE_ARP				= 0x100,
	ENABLE_IP_FILTER		= 0x200,
	ENABLE_DHCP_FILTER		= 0x400,
	ENABLE_ADV_IP_FILTER	= 0x800,
	ENABLE_PROCESS_TRAFFIC  = 0x1000,
	ENABLE_STACK_CHECK	    = 0x2000,
	ENABLE_FILE_MON			= 0x4000,
	ENABLE_ANTI_SCAN		= 0x8000,
	ENABLE_PACK_RECORDER	= 0x10000,
	ENABLE_TRAFFIC_CONTRL	= 0x20000,
	ENABLE_3G_TRAFFIC_MON	= 0x40000,
	ENABLE_ALL				= 0xffffffff
}ENABLE_FUNCTION;

// begin_wpp config
// CUSTOM_TYPE(EnableFunction, ItemEnum(tagEnableFunction));
// end_wpp

typedef enum tagEnableRequest {
	REQUEST_TCP				= 0x1,
	REQUEST_UDP				= 0x2,
	REQUEST_RAWIP			= 0x4,
	REQUEST_LANMAN			= 0x8,
	REQUEST_TRUST_URL		= 0x10,
	REQUEST_DOWNLOAD_PE		= 0x20,
	REQUEST_ARP_LOG			= 0x40,
	REQUEST_IP_LOG			= 0x80,
	REQUEST_ADV_IP_LOG		= 0x100,
	REQUEST_LOOP_BACK_IP	= 0x200,
	REQUEST_ALL				= 0xffffffff
}ENABLE_REQUEST;

// begin_wpp config
// CUSTOM_TYPE(EnableRequest, ItemEnum(tagEnableRequest));
// end_wpp


typedef enum tagIpResult
{
	enumIP_Pass		= 0,
	enumIP_Block	= 1,
	enumIP_Unknow	= 2,
}IPRESULT;
// begin_wpp config
// CUSTOM_TYPE(IpResult, ItemEnum(tagIpResult));
// end_wpp


typedef enum _tagAttackType
{
	pa_none									= 0,	// 

	pa_arp_send_attack						= 1,	// 发送攻击包
	pa_arp_send_fake_gate_mac				= 2,	// 目标ＩＰ是网关,但是ＭＡＣ不是
	pa_arp_send_fake_bindmac				= 3,	// 目标ＩＰ是绑定地址,但是ＭＡＣ不是
	pa_arp_send_unexcept_arp_on_safe_mode	= 4,	// 安全模式下，发送ＡＲＰ给其它机器(除了网关和已绑定地址)

	pa_arp_recv_ip_conflict					= 5,	// 收到源ＩＰ等于自己的ＡＲＰ包(ＩＰ冲突)
	pa_arp_recv_fake_gate_mac				= 6,	// 源ＩＰ是网关,但是ＭＡＣ不是
	pa_arp_recv_fake_bindmac				= 7,	// 源ＩＰ是绑定地址,但是ＭＡＣ不是
	pa_arp_recv_unexcept_arp_on_safe_mode	= 8,	// 安全模式下，接收到ＡＲＰ从其它机器(除了网关和已绑定地址)
	pa_arp_adv_ip_filter					= 9,		// 高级包过滤
	pa_arp_anti_scan						= 10		// 反扫描
}PacketAttackType;


// begin_wpp config
// CUSTOM_TYPE(BlockReason, ItemEnum(tagBlockReason));
// end_wpp

enum CompType
{
	enumChar  = 0,
	enumShort = 1,
	enumLong = 2,
	enumInt64 = 3,

	enumCharRange  = 4,
	enumShortRange = 5,
	enumLongRange = 6,
	enumInt64Range = 7,

	enumCharMult = 8,
	enumShortMult = 9,
	enumLongMult = 10,
	enumInt64Mult = 11,

	enumMem = 12
};

//////////////////////////////////////////////////////////////////////////

enum BlockArpFlag
{
	enumSend_Attack = 0x1,
	enumRecv_Attack = 0x2,
	enumRecv_IP_Attack = 0x4, 
	enumSafe_Mode = 0x8,
	enumNotify_Gateway = 0x10,
};

//////////////////////////////////////////////////////////////////////////

enum NdisEventType
{
	enumIpLog,
	enumRasEvent
};