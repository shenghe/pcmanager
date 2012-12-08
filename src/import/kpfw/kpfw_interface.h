////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : kpfw_interface.h
//      Version   : 1.0
//      Comment   : 定义网镖服务和界面之间的rfc接口
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////
#pragma once 

#include "kis/kpfw/kpfw_common_interface.h"
#include "iprule.h"
#include "apprule.h"
#include "area.h"

#include "antiarp/arpcommondef.h"

#include "antiarp/arpcomfunc.h"
#include "kis/kpfw/netlog.h"
#include "kis/kpfw/netwatch.h"
#include "antiarp/antiarpinterface.h"
#include <netcon.h>

#define KPFW_IPC_NAME			TEXT("\\\\.\\pipe\\KAVIPC_kpfw_1345")

//////////////////////////////////////////////////////////////////////////
// 用户类型
typedef enum
{
	User_Normal		= 0,		// 普通用户
	User_Expert		= 1,		// 用户专家
	User_WantQuiet = 2,			// 防打扰用户
}emUserType;

class KNetConnectHandle;

//////////////////////////////////////////////////////////////////////////
// 流量信息

//////////////////////////////////////////////////////////////////////////
class KNetEventInfo: public KisEnableSerialize
{
private:
	vector< KpfwLog*>		m_Logs;
	INT							m_nMax;

public:
	KNetEventInfo()
	{

	}
	~KNetEventInfo()
	{
		Clear();
	}

	void SetMax(INT nMax)
	{
		m_nMax = nMax;
	}

	int GetMax()
	{
		return m_nMax;
	}

	void AddEvent( KpfwLog * pLog )
	{
		m_Logs.push_back(pLog);
		
		if ((INT)m_Logs.size() > m_nMax)
		{
			delete  ((KpfwLog*)m_Logs[0]);
			m_Logs.erase(m_Logs.begin());
		}
	}

	KNetEventInfo& operator=(const KNetEventInfo& info)
	{
		Clear();
		m_nMax = info.m_nMax;

		for (int i = 0; i < (INT)info.m_Logs.size();i++)
		{
			KpfwLog* pOldLog = (KpfwLog*)info.m_Logs[i];
			KpfwLog* pNewLog = new KpfwLog;

			pNewLog->nID = pOldLog->nID;
			pNewLog->nSize = pOldLog->nSize;
			pNewLog->nType = pOldLog->nType;
			pNewLog->strData = pOldLog->strData;
			
			m_Logs.push_back( pNewLog);
		}
		return *this;
	}


	KNetEventInfo& Filter(const KNetEventInfo& info, ULONGLONG* pID)
	{
		Clear();
		m_nMax = info.m_nMax;

		for (int i = 0; i < (INT)info.m_Logs.size();i++)
		{
			KpfwLog* pOldLog = info.m_Logs[i];
			
			if (pOldLog->nID > *pID)
			{
				KpfwLog* pNewLog = new KpfwLog;

				pNewLog->nID = pOldLog->nID;
				pNewLog->nSize = pOldLog->nSize;
				pNewLog->nType = pOldLog->nType;
				pNewLog->strData = pOldLog->strData;

				m_Logs.push_back(pNewLog);
				*pID = pOldLog->nID;
			}
		}
		return *this;
	}

	INT GetCount()
	{
		return (INT)m_Logs.size();
	}

	KpfwLog* GetLog(int nIndex)
	{
		ASSERT(nIndex >= 0 && nIndex < (INT)m_Logs.size());
		return m_Logs[nIndex];
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			INT nCnt = 0;
			_serializer >> nCnt;
			for (int i = 0; i < nCnt; i++)
			{
				KpfwLog* pKpfwLog = new KpfwLog;
				_serializer >> pKpfwLog->nID;
				_serializer >> pKpfwLog->nSize;
				_serializer >> pKpfwLog->nType;
				INT nSize = pKpfwLog->nSize - sizeof(int)*3;

				BYTE* pBuf = new BYTE[nSize];
				_serializer.read(pBuf, sizeof(BYTE), nSize);
				pKpfwLog->strData.assign((char*)pBuf, nSize);
				delete []pBuf;
				
				m_Logs.push_back( pKpfwLog );
				
			}
		}
		else
		{
			INT nCnt = (INT)m_Logs.size();
			_serializer << nCnt;
			for (int i = 0; i < nCnt; i++)
			{
				KpfwLog* pKpfwLog = (KpfwLog*) m_Logs[i];
				_serializer << pKpfwLog->nID;
				_serializer << pKpfwLog->nSize;
				_serializer << pKpfwLog->nType;
				_serializer.write(pKpfwLog->strData.c_str(), sizeof(char), pKpfwLog->nSize - sizeof(int)*3);
			}
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSum = sizeof(INT);
		for (int i = 0; i < (INT)m_Logs.size(); i++)
		{
			KpfwLog* pKpfwLog = (KpfwLog*) m_Logs[i];
			nSum += pKpfwLog->nSize;
		}
		return nSum;
	}
private:
	void Clear()
	{
		for (int i = 0; i < (INT)m_Logs.size(); i++)
		{
			delete ((KpfwLog*)m_Logs[i]);
		}
		m_Logs.clear();
	}
};

//////////////////////////////////////////////////////////////////////////
class KAdapterInfo: public KisEnableSerialize
{
public:
	ATL::CString		m_ConnGuidId;
	ATL::CString		m_ConnName;
	NETCON_STATUS		m_ConnStatus;
	NETCON_MEDIATYPE	m_ConnMedia;
	ATL::CString		m_DeviceName;

	ULONG				m_uHostIp;
	BYTE				m_HostMac[6];
	ULONG				m_uGatewayIp;
	BYTE				m_GatewayMac[6];

	ATL::CString		m_strHostName;
	ATL::CString		m_strGateName;

	BOOL				m_bWireless;				// 是否是无线网

	BOOL				m_bAttackGate;				// 是否受到攻击
	BOOL				m_bAttackInter;				// 是否受到攻击

	ULONGLONG			m_adapterFlux;				// 该网卡上面的流量

public:
	KAdapterInfo():m_ConnStatus(NCS_DISCONNECTED), m_ConnMedia(NCM_NONE)
					, m_uHostIp(0), m_uGatewayIp(0)
					, m_bAttackGate(FALSE), m_bAttackInter(FALSE), m_adapterFlux(0)
	{}
	~KAdapterInfo()
	{}
	
	KAdapterInfo& operator=(const KAdapterInfo& info)
	{
		m_ConnGuidId = info.m_ConnGuidId;
		m_ConnName = info.m_ConnName;
		m_ConnStatus = info.m_ConnStatus;
		m_ConnMedia = info.m_ConnMedia;
		m_DeviceName = info.m_DeviceName;

		m_uHostIp = info.m_uHostIp;
		memcpy(m_HostMac, info.m_HostMac, sizeof(m_HostMac));
		m_uGatewayIp = info.m_uGatewayIp;
		memcpy(m_GatewayMac, info.m_GatewayMac, sizeof(m_GatewayMac));
		m_strHostName = info.m_strHostName;
		m_strGateName = info.m_strGateName;
		m_bWireless = info.m_bWireless;
		
		m_bAttackGate = info.m_bAttackGate;
		m_bAttackInter = info.m_bAttackInter;
		m_adapterFlux = info.m_adapterFlux;
		return *this;
	}

	HRESULT		Init(IKAdapter* pAdapter)
	{
		m_ConnGuidId = pAdapter->GetConnID();

		m_uHostIp = pAdapter->GetHostIp();
		memcpy(m_HostMac, pAdapter->GetHostMac(), sizeof(m_HostMac));
		m_uGatewayIp = pAdapter->GetGatewayIp();
		memcpy(m_GatewayMac, pAdapter->GetGatewayMac(), sizeof(m_GatewayMac));

		m_strGateName = pAdapter->GetGateName();

		m_bWireless = pAdapter->IsWirelessAdapter();

		m_ConnStatus = pAdapter->IsAdapterConnected() ? NCS_CONNECTED : NCS_DISCONNECTED;

		// m_ConnMedia = (NDIS_MEDIUM)pAdapter->GetMediaType();
		return S_OK;
	}

	BOOL	NeedPing()
	{
		if (m_uHostIp == 0)
			return FALSE;
		if (m_uGatewayIp == 0)
			return FALSE;
		if (NCS_CONNECTED != m_ConnStatus)
			return FALSE;
		return TRUE;
	}
	
	BOOL	HasGateway()
	{
		if (m_ConnMedia == NCM_LAN)
			return TRUE;
		return FALSE;
	}
	
	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			_serializer >> m_ConnGuidId;
			_serializer >> m_ConnName;
			_serializer >> m_ConnStatus;
			_serializer >> m_ConnMedia;
			_serializer >> m_DeviceName;

			_serializer >> m_uHostIp;
			_serializer.read(m_HostMac, sizeof(m_HostMac), 1);

			_serializer >> m_uGatewayIp;
			_serializer.read(m_GatewayMac, sizeof(m_GatewayMac), 1);

			_serializer >> m_strHostName;
			_serializer >> m_strGateName;

			_serializer >> m_bWireless;
			_serializer >> m_bAttackGate;
			_serializer >> m_bAttackInter;

			_serializer >> m_adapterFlux;
		}
		else
		{
			_serializer << m_ConnGuidId;
			_serializer << m_ConnName;
			_serializer << m_ConnStatus;
			_serializer << m_ConnMedia;
			_serializer << m_DeviceName;

			_serializer << m_uHostIp;
			_serializer.write(m_HostMac, sizeof(m_HostMac), 1);

			_serializer << m_uGatewayIp;
			_serializer.write(m_GatewayMac, sizeof(m_GatewayMac), 1);

			_serializer << m_strHostName;
			_serializer << m_strGateName;

			_serializer << m_bWireless;
			_serializer << m_bAttackGate;
			_serializer << m_bAttackInter;

			_serializer << m_adapterFlux;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSum = 0;
		nSum += TypeSize::get_size(m_ConnGuidId) + TypeSize::get_size(m_ConnName) + sizeof(m_ConnStatus) + sizeof(m_ConnMedia);
		nSum += TypeSize::get_size(m_DeviceName);
		nSum += sizeof(m_uHostIp) + sizeof(m_HostMac) + sizeof(m_uGatewayIp) + sizeof(m_GatewayMac);
		nSum += TypeSize::get_size(m_strHostName) + TypeSize::get_size(m_strGateName);
		nSum += sizeof(m_bWireless) + sizeof(m_bAttackGate) + sizeof(m_bAttackInter);
		nSum += sizeof(m_adapterFlux);
		return nSum;
	}
};

class KHostAdapterInfo: public KisEnableSerialize
{
private:
	vector<KAdapterInfo*>	m_info;

public:
	KHostAdapterInfo() {}

	~KHostAdapterInfo() {Clear();}

	KHostAdapterInfo& operator= (const KHostAdapterInfo& infos)
	{
		Clear();
		for (int i = 0; i < (INT)infos.m_info.size(); i++)
		{
			KAdapterInfo* pInfo = new KAdapterInfo;
			*pInfo = *infos.m_info[i];
			m_info.push_back(pInfo);
		}
		return *this;
	}

	INT	GetCount()
	{
		return (INT)m_info.size();
	}

	KAdapterInfo* GetInfo(INT i)
	{
		if (i < 0 || i >= (INT)m_info.size() )
			return NULL;
		return m_info[i];
	}

	KAdapterInfo* CreateInfo()
	{
		return new KAdapterInfo;
	}

	void	AddInfo(KAdapterInfo* pInfo)
	{
		m_info.push_back(pInfo);
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			Clear();
			int nCnt = 0;
			_serializer >> nCnt;
			for (int i = 0; i < nCnt; i++)
			{
				KAdapterInfo* pInfo = new KAdapterInfo;
				pInfo->Serialize(_serializer);
				m_info.push_back(pInfo);
			}
		}
		else
		{
			int nCnt = (INT)m_info.size();
			_serializer << nCnt;
			for (int i = 0; i < nCnt; i++)
				m_info[i]->Serialize(_serializer);
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSum = sizeof(int);
		for (int i = 0; i < (INT)m_info.size(); i++)
			nSum += m_info[i]->GetSeriSize();
		return nSum;
	}

	void Clear()
	{
		for (int i = 0; i < (int)m_info.size(); i++)
			delete m_info[i];
		m_info.clear();
	}
};


//////////////////////////////////////////////////////////////////////////
// ip 规则信息
MIDL_INTERFACE("B2C6FD23-C794-46df-AE08-ECCD7D098160")
KIpRuleInfo : public IUnknown
{
	STDPROC_(HRESULT) GetUserIpRules (KIpRuleVec*)		PURE;
	STDPROC_(HRESULT) UpdateUserIpRules (KIpRuleVec*)	PURE;
	STDPROC_(HRESULT) ExportIpRules(KIpRuleVec*, LPCWSTR strFileName) PURE;
	STDPROC_(HRESULT) ImportIpRules(KIpRuleVec*, LPCWSTR strFileName) PURE;
};

RCF_CLASS_BEGIN_WITH_COM(KIpRuleInfo)
	// RCF_METHOD_R_0(unsigned long, AddRef)
	// RCF_METHOD_R_0(unsigned long, Release)

	RCF_METHOD_R_1(HRESULT, GetUserIpRules,		KIpRuleVec*)
	RCF_METHOD_R_1(HRESULT, UpdateUserIpRules,	KIpRuleVec*)

	RCF_METHOD_R_2(HRESULT, ExportIpRules,		KIpRuleVec*, LPCWSTR)
	RCF_METHOD_R_2(HRESULT, ImportIpRules,		KIpRuleVec*, LPCWSTR)
RCF_CLASS_END

_declspec(selectany) extern const KSCLSID CLSID_KIpRuleInfoImp = { 0x4b1df274, 0xfb66, 0x4c83, { 0x8d, 0xec, 0x26, 0xd8, 0x66, 0x67, 0x32, 0x82 } };

//////////////////////////////////////////////////////////////////////////
// 区域规则
MIDL_INTERFACE("9F410A6E-410D-40fa-BF60-056BB1668102")
KAreaInfo : public IUnknown
{
	STDPROC_(HRESULT) GetAreas (KAreaVec*)					PURE;
	STDPROC_(HRESULT) GetCurArea (KAreaVec*)					PURE;
	STDPROC_(HRESULT) DeleteArea (KArea*)					PURE;
	STDPROC_(HRESULT) UpdateArea (KArea*)					PURE;
	STDPROC_(HRESULT) GetTrustIpRule( KIpRuleVec* )			PURE;
	STDPROC_(HRESULT) GetUnTrustIpRule( KIpRuleVec* )		PURE;	
	STDPROC_(HRESULT) GetUnKnownIpRule( KIpRuleVec* )		PURE;
	STDPROC_(HRESULT) GetArea (vector<BYTE>, KArea*)		PURE;
};

RCF_CLASS_BEGIN_WITH_COM(KAreaInfo)
// RCF_METHOD_R_0(unsigned long, AddRef)
// RCF_METHOD_R_0(unsigned long, Release)

	RCF_METHOD_R_1(HRESULT, GetAreas,		KAreaVec*)
	RCF_METHOD_R_1(HRESULT, GetCurArea,		KAreaVec*)
	RCF_METHOD_R_1(HRESULT, DeleteArea,		KArea*)
	RCF_METHOD_R_1(HRESULT, UpdateArea,		KArea*)

	RCF_METHOD_R_1(HRESULT, GetTrustIpRule,			KIpRuleVec*)
	RCF_METHOD_R_1(HRESULT, GetUnTrustIpRule,		KIpRuleVec*)
	RCF_METHOD_R_1(HRESULT, GetUnKnownIpRule,		KIpRuleVec*)

	RCF_METHOD_R_2(HRESULT, GetArea ,vector<BYTE>, KArea*)
RCF_CLASS_END

_declspec(selectany) extern const KSCLSID CLSID_KAreaInfoImp = { 0x8b3e5a8b, 0xdf7e, 0x4e77, { 0x98, 0x13, 0x2d, 0x11, 0x62, 0x2e, 0x48, 0xa2 } };

//////////////////////////////////////////////////////////////////////////
// 动态规则
struct AttackInfo
{
	KIPAddr		attackAddr;
	GUID		nReason;
	DWORD		nAttackTimes;
	DWORD		nTime;
};

MIDL_INTERFACE("4FBC6D86-7291-4cd1-BD0C-11E397C0DDC9")
KAttackInfo : public IUnknown
{
	STDPROC_(HRESULT) GetAttackList( vector<AttackInfo>* )		PURE;
	STDPROC_(HRESULT) DeleteAttackItem( AttackInfo* )			PURE;
};

RCF_CLASS_BEGIN_WITH_COM(KAttackInfo)
// RCF_METHOD_R_0(unsigned long, AddRef)
// RCF_METHOD_R_0(unsigned long, Release)

	RCF_METHOD_R_1(HRESULT, GetAttackList,		vector<AttackInfo>*)
	RCF_METHOD_R_1(HRESULT, DeleteAttackItem,	AttackInfo*)
RCF_CLASS_END

_declspec(selectany) extern const KSCLSID CLSID_KAttackInfoImp = { 0xe4c1180f, 0x1975, 0x4e90, { 0xb9, 0xa5, 0xd, 0x82, 0xde, 0xd5, 0xba, 0x8c } };

//////////////////////////////////////////////////////////////////////////
// log 信息
MIDL_INTERFACE("BD9A3DD6-2CB8-4646-9082-65184C8B47A7")
KKpfwLogInfo : public IUnknown
{
	STDPROC_(HRESULT) GetEventInfo (KNetEventInfo*, ULONGLONG*)		PURE;
	STDPROC_(HRESULT) GetStasticInfo (NETMON_LOG_STASTIC*)			PURE;
};

RCF_CLASS_BEGIN_WITH_COM(KKpfwLogInfo)
	// RCF_METHOD_R_0(unsigned long, AddRef)
	// RCF_METHOD_R_0(unsigned long, Release)

	RCF_METHOD_R_2(HRESULT, GetEventInfo,		KNetEventInfo*, ULONGLONG*)
	RCF_METHOD_R_1(HRESULT, GetStasticInfo,		NETMON_LOG_STASTIC*)
RCF_CLASS_END

_declspec(selectany) extern const KSCLSID CLSID_KKpfwLogInfoImp = { 0x329ff4e4, 0xd669, 0x4abe, { 0x9b, 0xf5, 0xf8, 0xea, 0xd0, 0x67, 0xc2, 0x16 } };

//////////////////////////////////////////////////////////////////////////
// 应用程序规则信息
MIDL_INTERFACE("874DAFE1-4651-4f4f-9D45-FC7FA622BDFA")
KAppRuleInfo : public IUnknown
{
	STDPROC_(HRESULT) GetAppRules (KAppRuleVec*)					PURE;
	STDPROC_(HRESULT) UpdateAppRules (KAppRuleVec*)					PURE;
	STDPROC_(HRESULT) DeleteRule (KAppRule*)						PURE;
	STDPROC_(HRESULT) UpdateRule (KAppRule*)						PURE;
	STDPROC_(HRESULT) AddRule (ATL::CString)						PURE;

	STDPROC_(HRESULT) GetModuleRules (KAppRuleVec*)					PURE;
	STDPROC_(HRESULT) DeleteModuleRule (KAppRule*)					PURE;
	STDPROC_(HRESULT) UpdateModuleRule (KAppRule*)					PURE;
	STDPROC_(HRESULT) UpdateModuleRules(KAppRuleVec*)				PURE;
	STDPROC_(HRESULT) AddModuleRule (ATL::CString)					PURE;
};

RCF_CLASS_BEGIN_WITH_COM(KAppRuleInfo)
	// RCF_METHOD_R_0(unsigned long, AddRef)
	// RCF_METHOD_R_0(unsigned long, Release)

	RCF_METHOD_R_1(HRESULT, GetAppRules,	KAppRuleVec*)
	RCF_METHOD_R_1(HRESULT, UpdateAppRules,	KAppRuleVec*)
	RCF_METHOD_R_1(HRESULT, DeleteRule,		KAppRule*)
	RCF_METHOD_R_1(HRESULT, UpdateRule,		KAppRule*)
	RCF_METHOD_R_1(HRESULT, AddRule,		ATL::CString)
	RCF_METHOD_R_1(HRESULT, GetModuleRules,	KAppRuleVec*)
	RCF_METHOD_R_1(HRESULT, DeleteModuleRule,	KAppRule*)
	RCF_METHOD_R_1(HRESULT, UpdateModuleRule,	KAppRule*)
	RCF_METHOD_R_1(HRESULT, UpdateModuleRules,	KAppRuleVec*)
	RCF_METHOD_R_1(HRESULT, AddModuleRule,		ATL::CString)
RCF_CLASS_END

_declspec(selectany) extern const KSCLSID CLSID_KAppRuleInfoImp = { 0x5f1587dc, 0xdc4f, 0x4d5e, { 0xa6, 0x5, 0xc1, 0x62, 0x95, 0xc7, 0xa7, 0xdd } };

//////////////////////////////////////////////////////////////////////////
// arp 防火墙关联
struct KArpHostInfo
{
	INT				m_nHostIpCnt;
	mac_ip			m_HostIp[max_hostip_count];

	INT				m_nGateWayCnt;
	mac_ip			m_GateWay[max_hostip_count];
};

MIDL_INTERFACE("A9FDC593-B8F0-4b46-B7F8-20AB66923465")
KArpInfo : public IUnknown
{
	STDPROC_(HRESULT) GetHostInfo (KArpHostInfo*)			PURE;
	STDPROC_(HRESULT) GetHostAdapterInfo (KHostAdapterInfo*)			PURE;
	STDPROC_(HRESULT) Ping(ATL::CString)								PURE;

	// 启用arp防火墙
	STDPROC_(HRESULT) GetEnable (BOOL*)						PURE;
	STDPROC_(HRESULT) SetEnable (BOOL)						PURE;

	// 使用手动设置的网关或者自动获取网关
	STDPROC_(HRESULT) GetAutoGateWay (BOOL*, vector<mac_ip>*)			PURE;
	STDPROC_(HRESULT) SetAutoGateWay (BOOL,  vector<mac_ip>*)			PURE;
	STDPROC_(HRESULT) GetMacFromIp   (mac_ip*)							PURE;
	
	//arp 高级设置
	STDPROC_(HRESULT)	GetNotifyGateway(BOOL* bNotify, INT* nTimes );
	STDPROC_(HRESULT)	SetNotifyGateway(BOOL bNotify, INT nTimes);

	STDPROC_(HRESULT)	GetAdvSetting(BOOL* bSafe, BOOL* arp_outer, BOOL*ip_outer, BOOL*ip_inner );
	STDPROC_(HRESULT)	SetAdvSetting(BOOL bSafe, BOOL arp_outer, BOOL ip_outer, BOOL ip_inner);
	
	STDPROC_(HRESULT)	Refresh()							PURE;
	//IP过滤设置
};

RCF_CLASS_BEGIN_WITH_COM(KArpInfo)
	// RCF_METHOD_R_0(unsigned long, AddRef)
	// RCF_METHOD_R_0(unsigned long, Release)

	RCF_METHOD_R_1(HRESULT, GetHostInfo,		KArpHostInfo*)
	RCF_METHOD_R_1(HRESULT, GetHostAdapterInfo, KHostAdapterInfo*)
	RCF_METHOD_R_1(HRESULT, Ping, ATL::CString)

	// 启用arp防火墙
	RCF_METHOD_R_1(HRESULT, GetEnable,			BOOL*)
	RCF_METHOD_R_1(HRESULT, SetEnable,			BOOL)

	// 使用手动设置的网关或者自动获取网关
	RCF_METHOD_R_2(HRESULT, GetAutoGateWay,		BOOL*, vector<mac_ip>*)
	RCF_METHOD_R_2(HRESULT, SetAutoGateWay,		BOOL,  vector<mac_ip>*)
	RCF_METHOD_R_1(HRESULT, GetMacFromIp,		mac_ip*)

	// arp 高级设置
	RCF_METHOD_R_2(HRESULT,		GetNotifyGateway,	BOOL*, INT*)
	RCF_METHOD_R_2(HRESULT,		SetNotifyGateway,	BOOL, INT)

	RCF_METHOD_R_4(HRESULT,		GetAdvSetting,	BOOL*, BOOL*, BOOL*,BOOL*)
	RCF_METHOD_R_4(HRESULT,		SetAdvSetting,	BOOL, BOOL, BOOL, BOOL)

	RCF_METHOD_R_0(HRESULT,		Refresh)
RCF_CLASS_END

_declspec(selectany) extern const KSCLSID CLSID_KArpInfoImp = { 0xdb3811fa, 0xfc5b, 0x49ec, { 0xa7, 0x69, 0xef, 0x25, 0xa6, 0xb1, 0xfb, 0xdd } };

//////////////////////////////////////////////////////////////////////////
// 区域信息

//////////////////////////////////////////////////////////////////////////
// 流量控制
MIDL_INTERFACE("359EB3A2-9750-470c-9529-8089BEA2B8A4")
IKFluxInfo : public IUnknown
{
	STDPROC_(HRESULT) GetHostInfo (KArpHostInfo*)			PURE;
};

_declspec(selectany) extern const KSCLSID CLSID_KFluxInfoImp = { 0xcc07658, 0xb0ec, 0x4fca, { 0x9e, 0xf, 0x51, 0x59, 0x99, 0xbd, 0x89, 0x4f } };
//////////////////////////////////////////////////////////////////////////
