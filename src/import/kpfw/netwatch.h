////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : netwatch.h
//      Version   : 1.0
//      Comment   : 定义网镖服务和驱动之间的数据交换格式
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////
#ifndef __netwatch_h__
#define __netwatch_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "kwpptrace.h"
#include "ntddndis.h"
#define MAX_PATH	260
#define MAXPATHBUFFLEN 260
#define MAX_URL_LENGTH 512
#define MAX_PE_HEADER_LENGTH	512
#define MAX_HOST_LENGTH 64
#define MAX_STACK_CHECK_NUM 32
#pragma pack(push, 1)

//////////////////////////////////////////////////////////////////////////
//COMMON

typedef int				BOOL;
typedef unsigned long	ULONG;
typedef unsigned short	USHORT;

#define MAX_PACKET_SIZE 4
#define PACK_DESCR_LEN 64
#define MAX_ADAPTER_NAME_LENGTH	256
#define	MAX_ADAPTER_IP_NUM	2
// 端口相关: host字节顺序
typedef unsigned short  KPort;

// 预定义ＩＰ规则
static const GUID ArpFilterGuid = 
{ 0xe2aad0a2, 0x264e, 0x4720, { 0x94, 0xaf, 0xb6, 0x7c, 0xf, 0xf9, 0x44, 0x5d } };
static const GUID AntiScanGuid = 
{ 0xab272f48, 0x9013, 0x4742, { 0x84, 0xdf, 0x57, 0xb9, 0xc2, 0x73, 0x8e, 0x5f }};


#define ETHER_IP  0x0008
#define ETHER_NET 0x0100
#define ETHER_ARP 0x0608
#define ETHER_PPPOE_DISCOVERY 0x8863
#define ETHER_PPPOE_SESSION 0x6488

#define ETHER_ARP_REQ 0x0100
#define ETHER_ARP_ACK 0x0200

#define IPHEADER_TTL  0x80
#define IPHEADER_UDP  0x11
#define IPHEADER_TCP  0x06
#define IPHEADER_ICMP 0x01
#define IPHEADER_IGMP 0x02

#define FLAG_IP_ICMP 0x01

//DHCP operations and stuff
#define DHCP_BOOTREQUEST	 1	/* Boot request message. */
#define DHCP_BOOTREPLY		 2	/* Boot reply message. */
#define DHCP_HTYPE_ETH		 1	/* Ethernet hardware type. */
#define DHCP_HLEN_ETH		 6	/* Ethernet hardware address length. */
//#define DHCP_FLAGS_BCAST    0x8000U	/* Reply must be broadcast to client. */
#define DHCP_FLAGS_BCAST    0x0080U	/* Reply must be broadcast to client. */
#define DHCP_FLAGS_UNICAST   0
//"Magic" first four option bytes
#define DHCP_MAGIC	htonl(0x63825363UL)

//DHCP common tags
#define DHCP_TAG_NETMASK	 1	/* Netmask. */
#define DHCP_TAG_GATEWAY	 3	/* Gateway list. */
#define DHCP_TAG_DNS		 6	/* DNS Nameserver list. */
#define DHCP_TAG_HOSTNAME	12	/* Host name. */
#define DHCP_TAG_DOMAIN		15	/* Domain. */
#define DHCP_TAG_IPMTU		26	/* Interface MTU. */

// DHCP protocol tags
#define DHCP_TAG_REQIP		50	/* Request this IP. */
#define DHCP_TAG_LEASE		51	/* Lease time requested/offered. */
#define DHCP_TAG_OVERLOAD	52	/* Options continued in file/sname. */
#define DHCP_TAG_TYPE		53	/* DHCP message (values below). */
#define DHCP_TAG_SERVERID	54	/* Server identifier. */
#define DHCP_TAG_REQPAR		55	/* Parameters requested. */
#define DHCP_TAG_MESSAGE	56	/* Error message. */
#define DHCP_TAG_MAXDHCP	57	/* Max DHCP packet size. */
#define DHCP_TAG_RENEWAL	58	/* Time to go into renewal state. */
#define DHCP_TAG_REBINDING	59	/* Time to go into rebinding state. */
#define DHCP_TAG_CLASSID	60	/* Class identifier. */
#define DHCP_TAG_CLIENTID	61	/* Client identifier. */

//DHCP messages 
#define DHCP_DISCOVER		 1	/* Locate available servers. */
#define DHCP_OFFER		 2	/* Parameters offered to client. */
#define DHCP_REQUEST		 3	/* (Re)request offered parameters. */
#define DHCP_DECLINE		 4	/* Client declines offer. */
#define DHCP_ACK		 5	/* Server acknowlegdes request. */
#define DHCP_NAK		 6	/* Server denies request. */
#define DHCP_RELEASE		 7	/* Client relinguishes address. */
#define DHCP_INFORM		 8	/* Client requests just local config. */

//Tcp flag bit
#define TCP_FLAG_FIN						0x01
#define TCP_FLAG_SYN						0x02
#define TCP_FLAG_RST						0x04
#define TCP_FLAG_PSH						0x08
#define TCP_FLAG_ACK						0x10
#define TCP_FLAG_URG						0x20

// 端口区间定义
typedef struct _tgPortRange
{
	KPort		nStart;								// 起始端口
	KPort		nEnd;								// 结束端口，包含结束端口
} KPortRange;

// ip地址 host字节顺序
typedef ULONG   KIPAddr;

// ip地址范围
typedef struct _tgIPAddrRange
{
	KIPAddr		ipStart;							// 起始ip地址
	KIPAddr		ipEnd;								// 结束ip地址
} KIPRange;


// 以太网头
typedef struct tagEthHeader
{
	UCHAR       DstAddr[6];
	UCHAR       SrcAddr[6];
	USHORT      EthType;
} ETHHEADER, *PETHHEADER;

typedef struct ehhdr 
{
	unsigned char    eh_dst[6];        /* destination ethernet addrress */
	unsigned char    eh_src[6];        /* source ethernet addresss */
	unsigned short   eh_type;          /* ethernet pachet type    */
}EHHDR, *PEHHDR;

// PPPOE头
typedef struct tagPppoeHeader
{
	unsigned char ver_type;
	unsigned char code;
	unsigned short sesson_id;
	unsigned short length;
}HdrPppoe, *PHdrPppoe;

typedef struct tagPppHeader 
{
	unsigned short proto;
}HdrPpp, *PHdrPpp;

enum enumPppProtoType
{
	//enumPppLcp	= 0xC021,
	//enumPppPap	= 0xC023,
	//enumPppChap = 0xC223,
	//enumPppIpcp = 0x8021,
	enumPppIp	= 0x2100,
};


// ip 数据头
typedef struct tagIPHEADER
{
	unsigned char byHeaderLen : 4;			// length of the header
	unsigned char byIPVersion : 4;			// version of IP
	unsigned char byServiceType;			// type of service
	unsigned short uTotalLen;				// total length of the packet

	unsigned short uIdentifier;				// unique identifier
	unsigned short uFragAndFlags;			// flags & frag offset 
	unsigned char byTtl;					// TTL(Time To Live) 
	unsigned char byProtocolType;			// protocol(TCP, UDP etc... see above)
	unsigned short uChecksum;				// IP header checksum

	unsigned long uSourceAddress;			// source IP address
	unsigned long uDestinationAddress;		// destination IP address

}IPHEADER, *PIPHEADER;


// udp 数据头
typedef struct tagUDPHEADER
{
	unsigned short uSourcePort;
	unsigned short uDestinationPort;
	unsigned short uTotalLength;
	unsigned short uCheckSum;
}UDPHEADER, *PUDPHEADER;


// tcp 数据头
typedef struct tagTCPHEADER
{
	unsigned short uSourcePort;
	unsigned short uDestinationPort;
	unsigned int uSerialNumber;
	unsigned int uACKNumber;

	union
	{
		struct
		{
			unsigned char byReserved1 : 4;
			unsigned char byHeaderLen : 4;

			unsigned char byReserved2 : 2;
			unsigned char byURG : 1;
			unsigned char byACK : 1;
			unsigned char byPSH : 1;
			unsigned char byRST : 1;
			unsigned char bySYN : 1;
			unsigned char byFIN : 1;
		}Details;

		struct
		{
			unsigned char byByte1;
			unsigned char byByte2;
		}UnnownUse;
	}HeaderLen_And_Flags;

	unsigned short uWindowSize;
	unsigned short uCheckSum;
	unsigned short uUrgentPointer;

}TCPHEADER, *PTCPHEADER;


// icmp 数据头
typedef struct tagICMPHEADER
{
	unsigned char byType;
	unsigned char byCode;
	unsigned short uCheckSum;
	unsigned short	sID;
	unsigned short	sNumber;
}ICMPHEADER, *PICMPHEADER;


// igmp 数据头
typedef struct tagIGMPHEADER
{
	unsigned char byVersion : 4;
	unsigned char byType : 4;
	unsigned char byReserved;
	unsigned short uCheckSum;
	unsigned long uAddress;
}IGMPHEADER, *PIGMPHEADER;


// ARP 数据头
typedef struct arphdr
{
	unsigned short    arp_hrd;            /* format of hardware address */
	unsigned short    arp_pro;            /* format of protocol address */
	unsigned char     arp_hln;            /* length of hardware address */
	unsigned char     arp_pln;            /* length of protocol address */
	unsigned short    arp_op;             /* ARP/RARP operation */

	unsigned char     arp_sha[6];         /* sender hardware address */
	unsigned long     arp_spa;            /* sender protocol address */
	unsigned char     arp_tha[6];         /* target hardware address */
	unsigned long     arp_tpa;            /* target protocol address */
}ARPHDR, *PARPHDR;

// ARP 数据包
typedef struct arpPacket
{
	EHHDR    ehhdr;
	ARPHDR   arphdr;
} ARPPACKET, *PARPPACKET;

// DHCP 数据头
typedef struct tagDHCPHEADER {
	unsigned char	op;			/* Message opcode/type. */
	unsigned char	htype;		/* Hardware address type. */
	unsigned char	hlen;		/* Hardware address length. */
	unsigned char	hops;		/* Hop count when relaying. */
	unsigned int	xid;		/* Transaction ID. */
	unsigned short	secs;		/* Seconds past since client began. */
	unsigned short	flags;		/* Flags. */
	unsigned int	ciaddr;		/* Client IP address. */
	unsigned int	yiaddr;		/* "Your" IP address. */
	unsigned int	siaddr;		/* Boot server IP address. */
	unsigned int	giaddr;		/* Relay agent (gateway) IP address. */
	unsigned char	chaddr[16];	/* Client hardware address. */
	unsigned char	sname[64];	/* Server host name. */
	unsigned char	file[128];	/* Boot file. */
	unsigned int	magic;		/* Magic number. */
	//unsigned char	options[308];	/* Optional parameters. */
	unsigned char	options[48];	/* Optional parameters. */
} DHCPHEADER;

// DHCP 数据包
typedef struct tagDHCP_PACKET
{
	EHHDR		eth_h;
	IPHEADER	ip_h;
	UDPHEADER	udp_h;
	DHCPHEADER  dhcp_h;
}DHCP_PACKET, *PDHCP_PACKET;

typedef struct tag_UDP_PACKET
{
	ETHHEADER	ethHdr;
	IPHEADER	ipHdr;
	UDPHEADER	udpHdr;
}UDP_PACKET;

typedef struct tag_TCP_PACKET
{
	ETHHEADER	ethHdr;
	IPHEADER	ipHdr;
	TCPHEADER	tcpHdr;
}TCP_PACKET;


// 绑定网卡信息
typedef struct tagAdapterInfo
{
	WCHAR wsDeviceName[MAX_ADAPTER_NAME_LENGTH];
	ULONG uLocalIP[MAX_ADAPTER_IP_NUM];
	UCHAR uLocalMac[6];
	ULONG uGatewagIP;
	UCHAR uGatewagMac[6];
	ULONG uIsConnect;
	NDIS_MEDIUM enumMediumType;
	NDIS_PHYSICAL_MEDIUM physicalMedium;
	__int64 inTraffic;
	__int64 outTraffic;
}ADAPTER_INFO, *PADAPTER_INFO;


//////////////////////////////////////////////////////////////////////////
//TDI

#define MAX_URL_LENGTH			512
#define MAX_PE_HEADER_LENGTH	512
#define MAX_HOST_LENGTH			64

//status
#define TDI_STATUS_NONE						0		//无状态，上层程序不会出现
#define TDI_STATUS_TCP_CONNECT				1		//连接
#define TDI_STATUS_TCP_CONNECT_COMPLETE		2		//连接完成
#define TDI_STATUS_TCP_LISTEN				3		//监听
#define TDI_STATUS_TCP_BE_CONNECTED			4		//被连接
#define TDI_STATUS_TCP_SEND					5		//发送
#define TDI_STATUS_TCP_RECV					6		//接收
#define TDI_STATUS_UDP_LISTEN				7		//监听
#define TDI_STATUS_UDP_SEND_DATAGRAM		8		//发送
#define TDI_STATUS_UDP_RECV_DATAGRAM		9		//接收

//TDI状态
typedef struct tagKfwState
{
	USHORT usMajorVer;
	USHORT usMinorVer;
	UCHAR bEnableKfw;
	UCHAR bEnableNet;
	ULONG uEnableFlag;
	ULONG uRequestFlag;
}KFW_STATE;

//连接信息
typedef struct tagConnInfo {
	ULONGLONG		SockID;
	INT             IpProto;
	INT				State;
	ULONG           LocalAddr;
	USHORT          LocalPort;
	ULONG           RemoteAddr;
	USHORT          RemotePort;
	ULONGLONG       ProcessId;
	ULONGLONG       ThreadId;
	ULONG           dwModuleID;
	ULONGLONG       BytesIn;
	ULONGLONG       BytesOut;
	char			m_strUrl[MAX_URL_LENGTH];
	char			m_strHost[MAX_HOST_LENGTH];
} TCP_CONN_INFO, CONN_INFO, *PCONN_INFO, *PTCP_CONN_INFO;

//PE文件下载信息
typedef struct tagPeFileInfo{
	UINT	m_uTimeStamp;
	ULONG	m_peHeaderLength;
	ULONG	m_nOpType;						// url发送方式（1：get， 2：post）
	WCHAR	m_strSavePath[MAX_PATH];
	char	m_strUrl[MAX_URL_LENGTH];
	char	m_strHost[MAX_HOST_LENGTH];
	char	m_peHeader[MAX_PE_HEADER_LENGTH];
}PE_FILE_INFO, *PPE_FILE_INFO;

//文件改名通知
typedef struct tagRenameFileInfo{
	WCHAR	m_strSrcPath[MAX_PATH];
	WCHAR	m_strDesPath[MAX_PATH];
}RENAME_FILE_INFO, *PRENAME_FILE_INFO;

//dll加载通知
typedef struct tagImageLoadInfo{
	WCHAR		m_strImagePath[MAX_PATH];
	struct{
		union {
			ULONG Properties;
			struct {
				ULONG ImageAddressingMode  : 8;  // Code addressing mode
				ULONG SystemModeImage      : 1;  // System mode image
				ULONG ImageMappedToAllPids : 1;  // Image mapped into all processes
				ULONG ExtendedInfoPresent  : 1;  // IMAGE_INFO_EX available
				ULONG Reserved             : 21;
			}UnkownStruct;
		}UnkownUnion;
		PVOID       ImageBase;
		ULONG       ImageSelector;
		SIZE_T      ImageSize;
		ULONG       ImageSectionNumber;
	}			m_imageInfo;
}IMAGE_LOAD_INFO, *PIMAGE_LOAD_INFO;


//应用程序响应
struct RESPONSE_RESULT
{
	ULONGLONG pResponseContext;
	ULONG pResult;
};

typedef struct tagPROGRAM_TRAFFIC
{
	ULONG dwModuleID;
	LARGE_INTEGER  ulTime;
	LARGE_INTEGER  ulTrafficIn;
	LARGE_INTEGER  ulTrafficOut;
}PROGRAM_TRAFFIC;

typedef struct tagPROGRAM_TRAFFIC_FROM_FILE
{
	PROGRAM_TRAFFIC progTraffic;
	WCHAR wsFilePath[MAX_PATH];
}PROGRAM_TRAFFIC_FROM_FILE;

typedef struct tagPROCESS_TRAFFIC
{
	ULONGLONG uProcessID;
	ULONG dwModuleID;
	LARGE_INTEGER  ulCreateTime;
	LARGE_INTEGER  ulTrafficIn;
	LARGE_INTEGER  ulTrafficOut;
	ULONG  ulSendLimit;		//Bytes / 秒
	ULONG  ulRecvLimit;		//Bytes / 秒
	LONG	uAddrConnectionCnt;
	LONG	uTotalConnectionCnt;
	ULONG	nDisable;
	LARGE_INTEGER  ulTrafficInLocal;
	LARGE_INTEGER  ulTrafficOutLocal;
}PROCESS_TRAFFIC;

typedef struct tagSET_PROCESS_LIMIT
{
	ULONGLONG uProcessID;
	ULONG ulSendLimit;
	ULONG ulRecvLimit;
	ULONG nDisable;
}SET_PROCESS_LIMIT;

typedef struct tagSET_PORT_LIMIT
{
	USHORT uPort;
	ULONG ulSendLimit;
	ULONG ulRecvLimit;
}SET_PORT_LIMIT;



//////////////////////////////////////////////////////////////////////////

typedef struct tagModuleInfo{
	ULONG dwModuleID;
	WCHAR wsModulePath[MAX_PATH];
	UCHAR byMd5[16];
	BOOLEAN bChanged;
}ModuleInfo, *PModuleInfo;

//应用程序请求信息

typedef struct tagApplicationRequestInformation {
	ULONG					nRequestProtocol;							// PROTO_TYPE
	ULONG					nRequestType;								// APP_REQUEST_TYPE
	ULONGLONG               dwProcessId;
	ULONGLONG				dwEventID;
	ULONGLONG				dwProcessCreateID;
	ULONGLONG               dwThreadId;
	ULONG					dwStackModules[MAX_STACK_CHECK_NUM];
	ULONG					dwStackRetAddr[MAX_STACK_CHECK_NUM];
	ULONG					dwParentModules[MAX_STACK_CHECK_NUM];
	union {
		struct 
		{
			ULONG                   dwRemoteAddress;
			USHORT                  wRemotePort;
			ULONG                   dwLocalAddress;
			USHORT                  wLocalPort;
			ULONG					uPacketDir;
			ULONG					uDataLen;
			UCHAR					bData[MAX_PACKET_SIZE];
		}TDI;

		struct 
		{
			WCHAR wsPath[MAXPATHBUFFLEN];
		}LanMan;
		
		struct 
		{
			ULONG nOp;
			char szHost[MAX_HOST_LENGTH];
			char szUrl[MAX_URL_LENGTH];
		}TrustUrl;
		
	}Parameters;

} APP_REQUEST_INFO, *PAPP_REQUEST_INFO;

#define TRUST_URL_OP_HTTP_GET		1
#define TRUST_URL_OP_HTTP_POST		2

//驱动通知信息
typedef struct tagNotifyInfo {
	ULONG					nRequestProtocol;							// PROTO_TYPE
	ULONG					nRequestType;								// APP_REQUEST_TYPE
	ULONGLONG               dwProcessId;
	ULONGLONG				dwEventID;
	ULONGLONG				dwProcessCreateID;
	ULONGLONG               dwThreadId;
	ULONG					dwStackModules[MAX_STACK_CHECK_NUM];
	union {
		PE_FILE_INFO peFileInfo;
		ModuleInfo fileChangedInfo;
		RENAME_FILE_INFO renameFileInfo;
		IMAGE_LOAD_INFO  imageLoadInfo;
	}Parameters;
} APP_NOTIFY_INFO, *PAPP_NOTIFY_INFO;



//应用程序请求
struct RESPONSE_APP_REQUEST_INFO
{
	ULONGLONG pResponseContext;			//由驱动给出，防火墙主程序发送响应时，原值传回。
	APP_REQUEST_INFO appRequestInfo;
};

//////////////////////////////////////////////////////////////////////////
// 邮件监控
typedef struct tagMailMonView {

	ULONG   dwRemoteIp;
	USHORT  wRemotePort;
	USHORT  wLocalPort;
	ULONG   dwProcessId;

} MAILMON_VIEW, *PMAILMON_VIEW;


//////////////////////////////////////////////////////////////////////////
// 设置数据记录器
typedef struct taDataRecorder{
	ULONG dwModlueId;			//需要记录的应用程序路径的HASH值
	USHORT dwLocalPort;			//本地端口：等于0时，表示任意端口
	USHORT dwRemotePort;		//远程端口：等于0时，表示任意端口
}DATA_RECORDER, *PDATA_RECORDER;

//////////////////////////////////////////////////////////////////////////
//NDIS
//ARP状态
typedef struct tagArpState
{
	USHORT usMajorVer;
	USHORT usMinorVer;
	ULONG uEnableNet;
	ULONG uEnableFlag;		//ENABLE_FUNCTION
	ULONG uArpBlockFlag;
	ULONG uRequestFlag;		//ENABLE_REQUEST
	ULONGLONG uTdiInterface;
}ARP_STATE;

#define IPFILTER_LIST_MAX 10 // 最多10个ip地址和10个端口

typedef struct tagFilterIPAddr
{
	IP_ADDR_MODE		eMode;
	union
	{
		KIPAddr			ipAddr;
		KIPRange		ipRange;
	};

} FILTER_IP_ADDR, *PFILTER_IP_ADDR;

typedef struct tagFilterPort
{
	PORT_MODE			eMode;
	union
	{
		KPort			port;
		KPortRange		portRange;
	};
} FILTER_PORT, *PFILTER_PORT;


//驱动通讯 ip 规则数据结构
typedef struct tagExtrDataTcpData
{
	FILTER_PORT			LocalPort;					// 本地端口
	FILTER_PORT			RemotePort;					// 远程端口
	unsigned char		byTcpFlags;					// 最高位为1表示使用该flag进行完全匹配，否则不使用flag
} TCP_EXTRA_DATA, *PTCP_EXTRA_DATA;

typedef struct tagExtrDatauDdpData
{
	FILTER_PORT			LocalPort;					// 本地端口
	FILTER_PORT			RemotePort;					// 远程端口
} UDP_EXTRA_DATA, *PUDP_EXTRA_DATA;

typedef struct tagExtrDataIcmpData
{
	unsigned char		byType;						//255 means any
	unsigned char		byCode;						//255 means any
} ICMP_EXTRA_DATA, *PICMP_EXTRA_DATA;

typedef struct tagExtrDataIgmpData
{
	unsigned char		byType;						//255 means any
	unsigned char		byTypeOfGroup;				//255 means any
	ULONG				uMulticastAddr;				//0 means any
} IGMP_EXTRA_DATA, *PIGMP_EXTRA_DATA;

typedef struct tagExtrDataArpData
{
	unsigned char  eh_dst[6];        /* destination ethernet addrress */
	unsigned char  eh_src[6];        /* source ethernet addresss */

	unsigned char  arp_sha[6];         /* sender hardware address */
	unsigned long  arp_spa;            /* sender protocol address */
	unsigned char  arp_tha[6];         /* target hardware address */
	unsigned long  arp_tpa;            /* target protocol address */
} ARP_EXTRA_DATA, *PARP_EXTRA_DATA;


typedef union tagExtraData
{
	TCP_EXTRA_DATA		TCP_DATA;					// TCP 设置
	UDP_EXTRA_DATA		UDP_DATA;					// udp 设置
	ICMP_EXTRA_DATA		ICMP_DATA;					// icmp 设置
	IGMP_EXTRA_DATA		IGMP_DATA;					// igmp 设置数据
} FILTERINFO_EXTRADATA, *PFILTERINFO_EXTRADATA;


typedef struct tagFilterInfo
{
	unsigned long		cbSize;						// 本ipfilter的长度，一般用来控制版本
	unsigned long		nVer;						// filter结构版本

	PROTO_TYPE          ProtocolType;				// 协议
	PACKET_DIR			Direction;				    // 发送方向
	INT					Operation;				    // 匹配后进行的操作 0 放行 1 拦截
	FILTER_IP_ADDR		LocalAddr;					// 本地地址
	FILTER_IP_ADDR		RemoteAddr;					// 远程地址

	FILTERINFO_EXTRADATA FilterInfo_ExtraData;		// 相关协议的辅助信息

}FILTER_INFO, *PFILTER_INFO;

typedef struct tagKIP_FILTER_INFO
{
	GUID				id;							// filter的id
	FILTER_INFO			filter;						// ip包过滤描述器
} KIP_FILTER_INFO, *PKIP_FILTER_INFO;


//IP过滤规则比较节点
struct MemCompNode
{
	ULONG l;
	unsigned char p;
};

struct KPackNode
{
	unsigned short cType;			//数据类型, CompType
	unsigned short	 bDataCount;	//数据个数
	unsigned short	 sOffset;
	unsigned short	 sNextNode;
	union
	{
		unsigned char  c;
		unsigned short s;
		unsigned int   u;
		unsigned __int64 i64;  
		MemCompNode m;
	}Parameters[1];

};

//IP过滤规则
struct KPackTemplate
{
	unsigned short cb;
	unsigned short sNodeCount;
	PROTO_TYPE protoType;
	GUID  guidID;
	char  szDesc[32];
	unsigned short uPackSize;
	unsigned short usSrcPort;
	unsigned short usDesPort;
	PACKET_DIR  bPackDir;
	INT	nOperation;
	KPackNode packNode[1];
};

typedef struct _block_arp_packet{

	unsigned short ucount;

	unsigned char  arp_block_type;   // 拦截类型 见 em_arp_block_type

	unsigned char  eh_dst[6];        /* destination ethernet addrress */
	unsigned char  eh_src[6];        /* source ethernet addresss */

	unsigned char  arp_sha[6];         /* sender hardware address */
	unsigned long  arp_spa;            /* sender protocol address */
	unsigned char  arp_tha[6];         /* target hardware address */
	unsigned long  arp_tpa;            /* target protocol address */

}block_arp_packet, *p_block_arp_packet;


typedef union tagExtraData_v2
{
	TCP_EXTRA_DATA		TCP_DATA;					// TCP 设置
	UDP_EXTRA_DATA		UDP_DATA;					// udp 设置
	ICMP_EXTRA_DATA		ICMP_DATA;					// icmp 设置
	IGMP_EXTRA_DATA		IGMP_DATA;					// igmp 设置数据
	ARP_EXTRA_DATA		ARP_DATA;
} FILTERINFO_EXTRADATA2, *PFILTERINFO_EXTRADATA2;


//typedef struct tagIPRuleLog
//{
//	GUID				id;							// 被触发的规则id
//	ULONGLONG			time;						// 日志产生的时间(格林威治)
//	
//	// 操作信息
//	PACKET_DIR			byDirection;				// 发送方向
//	INT					nOperation;					// 匹配后进行的操作 0 放行 1 拦截
//	INT					nAttackType;				// 被攻击类型，仅仅当检测到病毒攻击时有效. ARP日志时，同BLOCK_REASON
//
//	// 协议头相关信息
//	FILTERINFO_EXTRADATA2	ExtraInfo;
//
//}LOG_IP_RULE, *PLOG_IP_RULE;

//////////////////////////////////////////////////////////////////////////
// ip 规则log
typedef union tagIpLogExtraData
{
	struct
	{
		KPort				LocalPort;					// 本地端口
		KPort				RemotePort;					// 远程端口
		unsigned char		byTcpFlags;
	}TCP_DATA;

	struct
	{
		KPort				LocalPort;					// 本地端口
		KPort				RemotePort;					// 远程端口
	}UDP_DATA;

	struct
	{
		unsigned char byType;							// 255 means any
		unsigned char byCode;							// 255 means any
	}ICMP_DATA;

	struct
	{
		unsigned int unknown;
	}IGMP_DATA;

	ARP_EXTRA_DATA	ARP_DATA;
}IPLOG_EXTRADATA;


typedef struct tagIPRuleLog
{
	ULONG				nEventType;					// NdisEventType
	GUID				id;							// 被触发的规则id
	ULONG				nCount;						// 计数

	// ip 头相关信息
	unsigned char		byProtocolType;				// 协议
	KIPAddr				LocalAddr;
	KIPAddr				RemoteAddr;

	// 操作信息
	PACKET_DIR			byDirection;				// 发送方向
	INT					nOperation;					// 匹配后进行的操作 0 放行 1 拦截
	unsigned long		nAttackType;				// 被攻击类型，仅仅当检测到病毒攻击时有效

	// 协议头相关信息
	IPLOG_EXTRADATA		ExtraInfo;
}LOG_IP_RULE, *PLOG_IP_RULE;


typedef struct tagRasConnection
{
	ULONG nEventType;			// NdisEventType
	GUID id;
	UCHAR LocalAddress[6];
	UCHAR RemoteAddress[6];
	LARGE_INTEGER uInTraffic;
	LARGE_INTEGER uOutTraffic;
	LARGE_INTEGER uTimeLineUp;
	LARGE_INTEGER uTimeLineDown;
	LARGE_INTEGER uTimeLastGet;
	LARGE_INTEGER uTimeMiss;
	ULONG uStatus;
}RAS_CONNECTION, *PRAS_CONNECTION;


//////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus

inline bool operator== (const KPortRange& pt1, const KPortRange& pt2)
{
	return (pt1.nEnd == pt2.nEnd) && (pt1.nStart == pt2.nStart);
}

#endif

#endif
