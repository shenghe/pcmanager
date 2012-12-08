/********************************************************************
* CreatedOn: 2007-10-12   11:39
* FileName: pkthdr.h
* CreatedBy: lidengwang <lidengwang@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL:  $
* Purpose:
*********************************************************************/


#ifndef __PKTHDR_H__
#define __PKTHDR_H__




#define EPT_IP          0x0800            /* type: IP    */
#define EPT_ARP         0x0806            /* type: ARP */
#define EPT_RARP        0x8035            /* type: RARP */
#define ARP_HARDWARE    0x0001            /* Dummy type for 802.3 frames  */
#define ARP_REQUEST     0x0001            /* ARP request */
#define ARP_REPLY       0x0002            /* ARP reply */

#define Max_Num_Adapter 10


#pragma pack(push, 1)

typedef struct ehhdr 
{
    unsigned char    eh_dst[6];        /* destination ethernet addrress */
    unsigned char    eh_src[6];        /* source ethernet addresss */
    unsigned short   eh_type;          /* ethernet pachet type    */
}EHHDR, *PEHHDR;


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


typedef struct arpPacket
{
    EHHDR    ehhdr;
    ARPHDR   arphdr;

} ARPPACKET, *PARPPACKET;


enum em_arp_block_type {
    em_abt_receive    = 0,   // 外部
    em_abt_receive_ip = 1,   // 外部IP冲突
    em_abt_send       = 2    // 向外发送
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

#pragma pack(pop)




#endif

