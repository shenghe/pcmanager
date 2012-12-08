/*

Creator: Leo Jiang <JiangFengbing@kingsoft.com>

*/

#ifndef CF_PACKET_DEFINE_INC_
#define CF_PACKET_DEFINE_INC_

//////////////////////////////////////////////////////////////////////////
//
// 本头文件内部使用的类型均为C++内部类型,或在本头文件内的基于C++
// 内部类型衍生的类型,以保证高级内容过滤模块不include任何系统的头
// 文件
//
//////////////////////////////////////////////////////////////////////////

namespace CF
{
    typedef enum tagEthProtoType {
        
        enumEPT_IP = 0x0008

    } ETH_PROTO_TYPE;

    typedef enum tagIpProtoType {

        enumIPT_TCP = 0x06,
        enumIPT_UDP = 0x11

    } IP_PROTO_TYPE;

    typedef struct tagIPHEADER {

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

    } IPHEADER, *PIPHEADER;

    typedef struct tagUDPHEADER {

        unsigned short uSourcePort;
        unsigned short uDestinationPort;
        unsigned short uTotalLength;
        unsigned short uCheckSum;

    } UDPHEADER, *PUDPHEADER;

    typedef struct tagTCPHEADER {

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
            } Details;

            struct
            {
                unsigned char byByte1;
                unsigned char byByte2;
            };
        } HeaderLen_And_Flags;

        unsigned short uWindowSize;
        unsigned short uCheckSum;
        unsigned short uUrgentPointer;

    } TCPHEADER, *PTCPHEADER;

    typedef struct tagICMPHEADER {

        unsigned char byType;
        unsigned char byCode;
        unsigned short uCheckSum;

    } ICMPHEADER, *PICMPHEADER;

    typedef struct tagIGMPHEADER {

        unsigned char byVersion : 4;
        unsigned char byType : 4;
        unsigned char byReserved;
        unsigned short uCheckSum;
        unsigned long uAddress;
    } IGMPHEADER, *PIGMPHEADER;

}

//////////////////////////////////////////////////////////////////////////

#endif  // !CF_PACKET_DEFINE_INC_

