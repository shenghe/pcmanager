/********************************************************************
* CreatedOn: 2007-10-12   15:45
* FileName: commondef.h
* CreatedBy: lidengwang <lidengwang@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL:  $
* Purpose:
*********************************************************************/


#ifndef __COMMONDEF_H__
#define __COMMONDEF_H__


#define  ANTIARP_DRIVER_VERSION_1      (1)
#define  ANTIARP_DRIVER_VERSION_2      (2)
#define  ANTIARP_DRIVER_VERSION_3      (3)
#define  ANTIARP_DRIVER_VERSION_4      (4)



#define KANTIARP_DEVICE_NAME_A       "KAntiArp_2007_10_12"
#define KANTIARP_NT_DEVICE_NAME_A    "\\Device\\KAntiArp_2007_10_12"
#define KANTIARP_DOS_DEVICE_NAME_A   "\\DosDevices\\KAntiArp_2007_10_12"


#define KANTIARP_DEVICE_NAME         L"KAntiArp_2007_10_12"
#define KANTIARP_NT_DEVICE_NAME      L"\\Device\\KAntiArp_2007_10_12"
#define KANTIARP_DOS_DEVICE_NAME     L"\\DosDevices\\KAntiArp_2007_10_12"


#define max_hostip_count  (10 )
#define max_filter_count  (512)


#pragma pack(push, 1)

typedef struct _mac_ip
{
    unsigned char mac[6];
    unsigned int  ip;

}mac_ip, *p_mac_ip;


//////////////////////////////////////////////////////////////////////////
//
//   设置结构体
//
//////////////////////////////////////////////////////////////////////////
typedef struct _antiarp_config
{

    unsigned char b_block_in_arp_attack;            // 拦截接受外部ARP攻击
    unsigned char b_block_in_ip_attack;             // 拦截接受外部IP冲突攻击

    unsigned char b_block_out_arp_attack;           // 拦截向外发送ARP攻击
    unsigned char b_block_out_ip_attack;            // 拦截向外发送IP冲突攻击


    unsigned char b_safe_mode;                      // 安全模式

    mac_ip        mac_ip_gateway;                   // 网关MAC IP

    struct{
        unsigned int n_mac_ip_host_count;              // 本机IP数量
        mac_ip       p_mac_ip_host[max_hostip_count];  // 本机IP列表
    };

}antiarp_config, *p_antiarp_config;

//////////////////////////////////////////////////////////////////////////
//
//   保护的IP和MAC列表
//
//////////////////////////////////////////////////////////////////////////

typedef struct _protect_addr
{
    mac_ip addr;
    unsigned char bgateway;
}protect_addr, *p_protect_addr;

typedef struct _antiarp_protect_ipmac_list
{
    unsigned int n_list_count;
    protect_addr addr_list[1];

}antiarp_protect_ipmac_list, *p_antiarp_protect_ipmac_list;


//////////////////////////////////////////////////////////////////////////
//
//   通知的事件列表
//
//////////////////////////////////////////////////////////////////////////
typedef struct _antiarp_config_event
{

    HANDLE h_block_in_arp_attack_event;
    HANDLE h_block_in_arp_ip_event;

    HANDLE h_block_out_arp_attack_event;
    HANDLE h_block_out_arp_ip_event;

    HANDLE h_status_changed_event;

}antiarp_config_event, *p_antiarp_config_event;



#pragma pack(pop)


#define status_invalid      (0)
#define status_stop         (1)
#define status_running      (2)
#define status_reset        (3)



//13775624600

#endif
