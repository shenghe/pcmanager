/********************************************************************
* CreatedOn: 2007-10-17   11:01
* FileName: AntiArpInterface.h
* CreatedBy: lidengwang <lidengwang@kingsoft.net> & lvweili
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL:  $
* Purpose:
*********************************************************************/


#pragma once

#include "arpcommondef.h"
#include "pkthdr.h"


typedef BOOL(__stdcall * pfnIsDriverInstalled)();
typedef BOOL(__stdcall * pfnInstallDriver)(BOOL);
typedef BOOL(__stdcall * pfnUninstallDriver)();

#define FUNCNAME_ISDRIVERINSTALLED  _T("IsDriverInstalled")
#define FUNCNAME_INSTALLDRIVER      _T("InstallDriver")
#define FUNCNAME_UNINSTALLDRIVER    _T("UninstallDriver")


class IAntiArpCallback
{
public:
    virtual void __stdcall BlockSendArpPacket(p_block_arp_packet)   = 0;
    virtual void __stdcall BlockSendIpArpPacket(p_block_arp_packet) = 0;

    virtual void __stdcall BlockReceiveArpPacket(p_block_arp_packet)   = 0;
    virtual void __stdcall BlockReceiveIpArpPacket(p_block_arp_packet) = 0;


    virtual void __stdcall StatusChanged(int nStatus ) = 0;
};


extern const GUID CLSID_IANTIARPDEVC;

struct __declspec( uuid( "B6119EFE-365B-4a23-8D83-07776FFA069D" ) )
IAntiArpDevC : public IUnknown
{
    virtual BOOL __stdcall InitDevC() = 0;
    virtual void __stdcall UninitDevC() = 0;

    virtual unsigned int __stdcall GetDriverVersion() = 0;
    virtual unsigned int __stdcall GetDllVersion() = 0;

    virtual int  __stdcall GetStatus() = 0;
    virtual BOOL __stdcall SetStatus( int nStatus) = 0;

    
    virtual BOOL __stdcall SetAntiArpConfig(p_antiarp_config p_config) = 0;
    virtual BOOL __stdcall SetAntiArpProtectList(p_protect_addr plist, int ncount) = 0;
    
    virtual BOOL __stdcall SendArpPacket( PARPPACKET p_arp_packet ) = 0;



    virtual BOOL __stdcall StartSendPacket(DWORD dwSecCount, DWORD dwSec, PARPPACKET p_packet, /* max = 10*/int nCount) = 0;
    virtual void __stdcall StopSendPacket() = 0;


    virtual BOOL __stdcall GetMacInfo( p_mac_ip pmacip, char* szhostname, int nsize) = 0;

    virtual void __stdcall SetCallback(IAntiArpCallback* piCallback) = 0;

    virtual BOOL __stdcall IpToMac(p_mac_ip pmacip, BOOL bTrueIp) = 0;

    virtual BOOL __stdcall IsDriverInstalled() = 0;
};


typedef struct differentMAC 
{
	unsigned int ip;
	unsigned char OldMac[6];
	unsigned char NewMac[6];
}DifferentMac, *PDifferentMac;

class IDifferentMacCallback
{
public:

// -------------------------------------------------------------------------
// 函数		: __stdcall differentMac
// 功能		: 监控虚拟表的变化
// 返回值	: virtual void 
// 参数		: PDifferentMac
// 附注		: 
// -------------------------------------------------------------------------
	virtual void __stdcall differentMac( PDifferentMac ) = 0;

// -------------------------------------------------------------------------
// 函数		: __stdcall DifferentNetCard
// 功能		: 监控网卡，网关的变化
// 返回值	: virtual void 
// 参数		: /*[in，out]*/mac_ip*
// 参数		: /*[out]*/ size_t
// 参数		: /*[in，out]*/unsigned int
// 附注		: 
// -------------------------------------------------------------------------
	virtual void __stdcall DifferentNetCard(/*[in, out]*/mac_ip*, 
											/*[out]*/ size_t, 
											/*[in, out]*/mac_ip*,
                                            /*[out]*/ size_t) = 0;

};


extern const GUID CLSID_IARPTABLESAFEGUARD;

struct __declspec(uuid("C3E6C94F-A3ED-4ea6-80AD-1EABF140330D"))
IArpTableSafeguard : public IUnknown
{
	virtual int __stdcall UnInit() = 0;

	virtual void __stdcall InitTable(IDifferentMacCallback* piDiffMacFunc) = 0;

	virtual int __stdcall InitTableFromSys(IDifferentMacCallback* piDiffMacFunc) = 0;

// -------------------------------------------------------------------------
// 函数		: __stdcall Get_IPMAC_Info
// 功能		: 
// 返回值	: virtual int 
// 参数		: /*[in, out]*/_mac_ip* LocalArray 本地网卡信息
// 参数		: /*[in，out]*/int* nArrSize 返回网卡数目
// 参数		: /*[in，out]*/_mac_ip* Gateway 网关
// 附注		: 函数里面清空了缓存，调了SendARP()
// -------------------------------------------------------------------------
	virtual int __stdcall Get_IPMAC_Info(/*[in, out]*/_mac_ip* pLocalArray, 
										 /*[in, out]*/ int* nLocalArraySize, 
										 /*[in, out]*/_mac_ip* pGateway,
                                         /*[in, out]*/ int* nGatewaySize) = 0;

// -------------------------------------------------------------------------
// 函数		: __stdcall ModifyState
// 功能		: 静态绑定
// 返回值	: virtual int 
// 参数		: /*[in]*/_mac_ip* IpMacItem 
// 参数		: DWORD dwType 
// 附注		: // dwType: Static == 4; Dynamic == 3; Invalid == 2; Other == 1
// -------------------------------------------------------------------------
	virtual int __stdcall ModifyState(/*[in]*/_mac_ip* IpMacItem, DWORD dwType) = 0;
	
	virtual int __stdcall AddItem(/*[in]*/_mac_ip* IpMacItem) = 0;

	virtual int __stdcall JudgeIp(/*[in]*/_mac_ip* IpMacItem) = 0;

// -------------------------------------------------------------------------
// 函数		: __stdcall DeleteARPBuff
// 功能		: 清空缓存
// 返回值	: virtual int 
// 附注		: 用FlushIpNetTable()，没用DeleteIpNetEntry()
// -------------------------------------------------------------------------
	virtual int __stdcall DeleteARPBuff() = 0;

	virtual int __stdcall StartLookTableThread(size_t nFrequence) = 0;

	virtual int __stdcall EndLookTableTread() = 0;

// -------------------------------------------------------------------------
// 函数		: __stdcall StartLookNetCardThread
// 功能		: 　开线程，监控网卡，网关的变化
// 返回值	: virtual int 
// 参数		: size_t nFrequence
// 附注		: 如果有变化，调用回调DifferentNetCard()
// -------------------------------------------------------------------------
	virtual int __stdcall StartLookNetCardThread(size_t nFrequence) = 0;

	virtual int __stdcall EndLookNetCardTread() = 0;

};



