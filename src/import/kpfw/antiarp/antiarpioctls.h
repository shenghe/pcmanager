/********************************************************************
* CreatedOn: 2007-10-12   16:49
* FileName: antiarpioctls.h
* CreatedBy: lidengwang <lidengwang@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL:  $
* Purpose:
*********************************************************************/

#ifndef __ANTIARPIOCTLS_H__
#define __ANTIARPIOCTLS_H__


#define NDIS_DEVICE_ANTIARP        0x8000



//////////////////////////////////////////////////////////////////////////
// 获取防火墙 版本 
//////////////////////////////////////////////////////////////////////////
#define IOCTL_ANTIARP_GET_VERSION			        \
    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x01, METHOD_BUFFERED, FILE_ANY_ACCESS)

//////////////////////////////////////////////////////////////////////////
// 设置防火墙状态 
//////////////////////////////////////////////////////////////////////////
#define IOCTL_ANTIARP_SET_STATUS                    \
    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x10, METHOD_BUFFERED, FILE_ANY_ACCESS)

//////////////////////////////////////////////////////////////////////////
// 获取防火墙状态 
//////////////////////////////////////////////////////////////////////////
#define IOCTL_ANTIARP_GET_STATUS                    \
    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x11, METHOD_BUFFERED, FILE_ANY_ACCESS)




//////////////////////////////////////////////////////////////////////////
// 设置 设置结构体 防火墙状态   antiarp_config
//////////////////////////////////////////////////////////////////////////
#define IOCTL_ANTIARP_SET_CONFIG                    \
    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x12, METHOD_BUFFERED, FILE_ANY_ACCESS)

//////////////////////////////////////////////////////////////////////////
// 设置防火墙 保护的IP和MAC列表 antiarp_protect_ipmac_list
//////////////////////////////////////////////////////////////////////////
#define IOCTL_ANTIARP_SET_PROCTECT_MACIP_LIST       \
    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x21, METHOD_BUFFERED, FILE_ANY_ACCESS)


//////////////////////////////////////////////////////////////////////////
// 设置防火墙 通知的事件列表 antiarp_config_event
//////////////////////////////////////////////////////////////////////////
#define IOCTL_ANTIARP_SET_CONFIG_EVENT              \
    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x22, METHOD_BUFFERED, FILE_ANY_ACCESS)



//////////////////////////////////////////////////////////////////////////
// 获取拦截发送的ARP包
//////////////////////////////////////////////////////////////////////////
#define IOCTL_ANTIARP_GET_BLOCK_SEND_ARP_PACKET     \
    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x31, METHOD_BUFFERED, FILE_ANY_ACCESS)


//////////////////////////////////////////////////////////////////////////
// 获取拦截收到的ARP包
//////////////////////////////////////////////////////////////////////////
#define IOCTL_ANTIARP_GET_BLOCK_RECEIVE_ARP_PACKET   \
    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x32, METHOD_BUFFERED, FILE_ANY_ACCESS)


//////////////////////////////////////////////////////////////////////////
// 发送一个ARP包
//////////////////////////////////////////////////////////////////////////
#define IOCTL_ANTIARP_SEND_ARP_PACKET   \
    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x41, METHOD_NEITHER, FILE_ANY_ACCESS)



//////////////////////////////////////////////////////////////////////////
// 查询一个MAC地址的IP地址
//////////////////////////////////////////////////////////////////////////
#define IOCTL_ANTIARP_MAC_TO_IP   \
    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x51, METHOD_BUFFERED, FILE_ANY_ACCESS)


//////////////////////////////////////////////////////////////////////////
// 查询一个IP地址的MAC地址
//////////////////////////////////////////////////////////////////////////
#define IOCTL_ANTIARP_IP_TO_MAC   \
    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x52, METHOD_BUFFERED, FILE_ANY_ACCESS)

//////////////////////////////////////////////////////////////////////////
// 增加一个IP过滤规则
//////////////////////////////////////////////////////////////////////////
#define IOCTL_NDIS_ADD_IP_RULES          CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0012, METHOD_BUFFERED, FILE_ANY_ACCESS)

//////////////////////////////////////////////////////////////////////////
// 删除一个IP过滤规则
//////////////////////////////////////////////////////////////////////////
#define IOCTL_NDIS_REMOVE_IP_RULES       CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0013, METHOD_BUFFERED, FILE_ANY_ACCESS)

//////////////////////////////////////////////////////////////////////////
// 清除所有IP过滤规则
//////////////////////////////////////////////////////////////////////////
#define IOCTL_NDIS_CLEAR_IP_RULES        CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0014, METHOD_BUFFERED, FILE_ANY_ACCESS)

//////////////////////////////////////////////////////////////////////////
// 加载高级过滤模块
//////////////////////////////////////////////////////////////////////////
#define IOCTL_NDIS_AUTOLOAD_FILTER_MODULES        CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0015, METHOD_BUFFERED, FILE_ANY_ACCESS)

//////////////////////////////////////////////////////////////////////////
// 加载高级过滤模块
//////////////////////////////////////////////////////////////////////////
#define IOCTL_NDIS_CLEAR_FILTER_MODULES        CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0016, METHOD_BUFFERED, FILE_ANY_ACCESS)

//////////////////////////////////////////////////////////////////////////
// 获取IP日志
//////////////////////////////////////////////////////////////////////////
#define IOCTL_NDIS_GET_COMMON_EVENT        CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0017, METHOD_BUFFERED, FILE_ANY_ACCESS)

//枚举网卡信息
#define IOCTL_NDIS_ENUM_ADAPTER        CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0080, METHOD_BUFFERED, FILE_ANY_ACCESS)


//设置ARP拦截标志
#define IOCTL_NDIS_SET_BLOCK_FLAG   CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0083, METHOD_BUFFERED, FILE_ANY_ACCESS)

//设置NDIS日志输出标志
#define IOCTL_NDIS_SET_LOG_FLAG   CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0084, METHOD_BUFFERED, FILE_ANY_ACCESS)

//设置启用模块标志	见 ENABLE_FUNCTION
#define IOCTL_NDIS_ENABLE_MODULE_FLAG   CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0085, METHOD_BUFFERED, FILE_ANY_ACCESS)

//禁用/启用网络
#define IOCTL_NDIS_ENABLE_NETWORK   CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0086, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NDIS_DISABLE_NETWORK   CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0087, METHOD_BUFFERED, FILE_ANY_ACCESS)

//添加/删除高级包过滤规则
#define IOCTL_NDIS_ADD_ADV_IP_RULES       CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0088, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NDIS_REMOVE_ADV_IP_RULES    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0089, METHOD_BUFFERED, FILE_ANY_ACCESS)
//清除高级包过滤规则
#define IOCTL_NDIS_CLEAR_ADV_IP_RULES     CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0090, METHOD_BUFFERED, FILE_ANY_ACCESS)
 
//设置实时通知网关的时间间隔
#define IOCTL_NDIS_SET_NOTIFY_TIMER    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0091, METHOD_BUFFERED, FILE_ANY_ACCESS)

//清除保护网关列表
#define IOCTL_ANTIARP_CLEAR_PROCTECT_MACIP_LIST    CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0092, METHOD_BUFFERED, FILE_ANY_ACCESS)

//设置放行GUID，自己的ping
#define IOCTL_NDIS_SET_PASS_GUID  CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0093, METHOD_BUFFERED, FILE_ANY_ACCESS)

//枚举拨号连接
#define IOCTL_NDIS_ENUM_RAS_CONNECTION  CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0094, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_NDIS_SELECT_GATEWAY_MAC	CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0095, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_NDIS_CLEAN_GATEWAY_MAC	CTL_CODE(NDIS_DEVICE_ANTIARP, 0x0096, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

