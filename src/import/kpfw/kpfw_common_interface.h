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

#include "SCOM/SCOM/SCOMBase.h"
#include "SCOM/SCOM/SCOMDef.h"
#include "kis/kpfw/serializetools.h"
//////////////////////////////////////////////////////////////////////////
// 流量信息
struct KPFWFLUX
{
	ULONGLONG	nRec;
	ULONGLONG	nSend;
};

//////////////////////////////////////////////////////////////////////////
// 连接信息
struct KConnInfo
{
	CONN_INFO	info;
	WCHAR		exePath[MAX_PATH];
	BOOL		bCanKill;
};

class KConnectInfo: public KisEnableSerialize
{
private:
	INT				m_nConnCnt;
	KConnInfo*		m_pConnectInfo;

public:
	KConnectInfo(): m_nConnCnt(0), m_pConnectInfo(NULL)
	{

	}

	~KConnectInfo()
	{
		Clear();
	}

	INT	GetCount()
	{
		return m_nConnCnt;
	}

	KConnInfo* GetConnInfo(INT nIndex)
	{
		return m_pConnectInfo + nIndex;
	}

	KConnectInfo& operator=(const KConnectInfo& info)
	{
		Clear();
		m_nConnCnt = info.m_nConnCnt;
		if (m_nConnCnt)
		{
			m_pConnectInfo = new KConnInfo[info.m_nConnCnt];
			memcpy(m_pConnectInfo, info.m_pConnectInfo, sizeof(KConnInfo)*m_nConnCnt);
		}
		return *this;
	}

	void	SetListenInfo(INT nCnt, KConnInfo* pInfo)
	{
		Clear();

		m_nConnCnt = nCnt;
		if (m_pConnectInfo)
		{
			delete[] m_pConnectInfo;
			m_pConnectInfo = NULL;
		}
		m_pConnectInfo = pInfo;
	}


	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			Clear();
			_serializer >> m_nConnCnt;
			if (m_nConnCnt != 0)
			{
				m_pConnectInfo = new KConnInfo[m_nConnCnt];
				_serializer.read(m_pConnectInfo, sizeof(KConnInfo), m_nConnCnt );
			}
		}
		else
		{
			_serializer << m_nConnCnt;
			if (m_nConnCnt != 0)
				_serializer.write(m_pConnectInfo, sizeof(KConnInfo), m_nConnCnt);			
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		return sizeof(INT) + sizeof(KConnInfo) * m_nConnCnt;
	}

private:
	void	Clear()
	{
		if (m_pConnectInfo)
		{
			delete[] m_pConnectInfo;
			m_pConnectInfo = NULL;
		}
		m_nConnCnt = 0;
	}
};

//////////////////////////////////////////////////////////////////////////
class KProcFluxItem: public KisEnableSerialize
{
private:
	ULONGLONG				m_nCreateID;
	DWORD					m_nProcessID;
	DWORD					m_nModuleID;
	KPFWFLUX				m_flux;
	DWORD					m_nTrustMode;
	BOOL					m_bCanKill;
	LPVOID					m_pExtraData;
	ATL::CString			m_strProcessPath;
	
public:
	KProcFluxItem():m_nCreateID(0), m_nProcessID(0), m_nTrustMode(0), m_pExtraData(0), m_bCanKill(TRUE) {}
	~KProcFluxItem() {}

	void	SetCreateID(DWORD id) {m_nCreateID = id;}
	DWORD	GetCreateID()		  {return m_nCreateID;}

	void	SetModuleID(DWORD id)	{m_nModuleID = id;}
	DWORD	GetModuleID()			{return m_nModuleID;}

	void	SetProcessID(DWORD id) {m_nProcessID = id;}
	DWORD	GetProcessID()		   {return m_nProcessID;}

	void	SetFlux(KPFWFLUX flux) {m_flux = flux;}
	KPFWFLUX GetFlux()				{return m_flux;}
	
	void	SetTrustMode(DWORD mode) {m_nTrustMode = mode;}
	DWORD	GetTrustMode()			{return m_nTrustMode;}

	void	SetPath(ATL::CString& path) {m_strProcessPath = path;}
	LPCWSTR GetPath()					{return m_strProcessPath;}

	void	SetExtraData(LPVOID pdata)	{m_pExtraData = pdata;}
	LPVOID	GetExtraData()				{return m_pExtraData;}

	void	SetCanKill(BOOL b)			{m_bCanKill = b;}
	BOOL	GetCanKill()				{return m_bCanKill;}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			_serializer >> m_nCreateID;
			_serializer >> m_nProcessID;
			_serializer >> m_nModuleID;
			_serializer >> m_flux;
			_serializer >> m_nTrustMode;
			_serializer >> m_strProcessPath;
			_serializer >> m_bCanKill;
		}
		else
		{
			_serializer << m_nCreateID;
			_serializer << m_nProcessID;
			_serializer << m_nModuleID;
			_serializer << m_flux;
			_serializer << m_nTrustMode;
			_serializer << m_strProcessPath;
			_serializer << m_bCanKill;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		return sizeof(ULONGLONG) + sizeof(DWORD)*3 + sizeof(KPFWFLUX) + TypeSize::get_size(m_strProcessPath) + sizeof(BOOL);
	}
};

class KProcFluxItemColl: public KisEnableSerialize
{
private:
	vector<KProcFluxItem*>	m_FluxInfo;

public:
	KProcFluxItemColl() {}
	~KProcFluxItemColl() {Clear();}

	KProcFluxItem* CreateItem()
	{
		return new KProcFluxItem;
	}

	void		AddItem(KProcFluxItem* pItem)
	{
		m_FluxInfo.push_back(pItem);
	}

	INT			 GetCnt()
	{
		return (INT)m_FluxInfo.size();
	}

	KProcFluxItem* GetItem(INT i)
	{
		if (i >= 0 && i < (INT)m_FluxInfo.size())
			return m_FluxInfo[i];
		return NULL;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			Clear();
			INT nCnt = 0;
			_serializer >> nCnt;
			for (int i = 0; i < nCnt; i++)
			{
				KProcFluxItem* pItem = CreateItem();
				pItem->Serialize(_serializer);
				AddItem(pItem);
			}
		}
		else
		{
			INT nCnt = (INT)m_FluxInfo.size();
			_serializer << nCnt;
			for (int i = 0; i < nCnt; i++)
			{
				m_FluxInfo[i]->Serialize(_serializer);
			}
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSum = sizeof(INT);
		for (int i = 0; i < (INT)m_FluxInfo.size(); i++)
			nSum += m_FluxInfo[i]->GetSeriSize();
		return nSum;
	}

	void Clear()
	{
		for (int i = 0; i < (INT)m_FluxInfo.size(); i++)
			delete m_FluxInfo[i];
		m_FluxInfo.clear();
	}
};
//////////////////////////////////////////////////////////////////////////
MIDL_INTERFACE("FFCC0F74-AE29-4a50-B1C0-CC66EA6FD5FB")
KNetInfo : public IUnknown
{
	// 网络流量信息
	STDPROC_(HRESULT)	GetFluxInfo (vector<KPFWFLUX>*)		PURE;

	//  进程流量信息
	STDPROC_(HRESULT)	GetProcessFluxInfo (DWORD, KProcFluxItemColl*)		PURE;

	// 隐身
	STDPROC_(BOOL)		GetHide()							PURE;
	STDPROC_(BOOL)		SetHide(BOOL)						PURE;

	// 网络监控状态
	STDPROC_(BOOL)		GetEnableFireWall()					PURE;
	STDPROC_(BOOL)		SetEnableFireWall(BOOL)				PURE;

	// 自适应优化
	STDPROC_(BOOL)		GetEnableArea()						PURE;
	STDPROC_(BOOL)		SetEnableArea(BOOL)					PURE;

	// 断网
	STDPROC_(BOOL)		GetEnableNetwork()					PURE;
	STDPROC_(BOOL)		SetEnableNetwork(BOOL)				PURE;

	// 当前已连接信息
	STDPROC_(BOOL)		GetNetConnectInfo(KConnectInfo*)	PURE;

	// 受到攻击时报警
	STDPROC_(HRESULT)	GetSoundOnAttack(BOOL*)				PURE;
	STDPROC_(HRESULT)	SetSoundOnAttack(BOOL)				PURE;

	STDPROC_(HRESULT)	GetSoundPath(ATL::CString*)			PURE;
	STDPROC_(HRESULT)	SetSoundPath(ATL::CString)			PURE;

	// 动态防御
	STDPROC_(HRESULT)	GetShieldCnt(INT*)					PURE;

	STDPROC_(HRESULT)	GetAutoShieldAttacker(BOOL*)		PURE;
	STDPROC_(HRESULT)	SetAutoShieldAttacker(BOOL)			PURE;

	STDPROC_(HRESULT)	GetShieldAttackCount(INT*)			PURE;
	STDPROC_(HRESULT)	SetShieldAttackCount(INT)			PURE;

	STDPROC_(HRESULT)	GetShieldTime(INT*)					PURE;
	STDPROC_(HRESULT)	SetShieldTime(INT)					PURE;


	// 是否信任可信认证
	STDPROC_(BOOL)		GetTrustRep()							PURE;
	STDPROC_(BOOL)		SetTrustRep(BOOL)						PURE;

	// 设置用户类型
	STDPROC_(DWORD)		GetUserType()							PURE;
	STDPROC_(HRESULT)	SetUserType(DWORD)						PURE;

	STDPROC_(TrustMode) CheckTrustMode(ATL::CString, DWORD)			PURE;

	// 获取地理地址
	STDPROC_(HRESULT)	 GetGeoAddr(DWORD, ATL::CString*)		PURE;

	//终止进程
	STDPROC_(HRESULT)	StopProcess( ULONG uProcessId )			PURE;

	//日志相关
	STDPROC_(HRESULT)	ClearLog()								PURE;

	STDPROC_(HRESULT)	GetLogMaxSize(DWORD*)					PURE;
	STDPROC_(HRESULT)	SetLogMaxSize(DWORD)					PURE;

	STDPROC_(HRESULT)	GetLogRetention(DWORD*)					PURE;
	STDPROC_(HRESULT)	SetLogRetention(DWORD)					PURE;

	STDPROC_(HRESULT)	GetLogPageCnt(DWORD*)					PURE;
	STDPROC_(HRESULT)	SetLogPageCnt(DWORD)					PURE;
	STDPROC_(HRESULT)	GetLogSize(DWORD*, DWORD*)				PURE;

	STDPROC_(HRESULT)	GetInstallPath(ATL::CString*)			PURE;

};

// icp 接口
RCF_CLASS_BEGIN_WITH_COM(KNetInfo)
// 父接口信息
// RCF_METHOD_R_0(unsigned long, AddRef)
// RCF_METHOD_R_0(unsigned long, Release)

// 网络流量信息
RCF_METHOD_R_1(HRESULT, GetFluxInfo,		vector<KPFWFLUX>*)

RCF_METHOD_R_2(HRESULT,	GetProcessFluxInfo , DWORD, KProcFluxItemColl*)

// 隐身
RCF_METHOD_R_0(BOOL,	GetHide)
RCF_METHOD_R_1(BOOL,	SetHide,	BOOL)

// 网络监控状态
RCF_METHOD_R_0(BOOL,	GetEnableFireWall)
RCF_METHOD_R_1(BOOL,	SetEnableFireWall,	BOOL)

// 自适应优化
RCF_METHOD_R_0(BOOL,	GetEnableArea)
RCF_METHOD_R_1(BOOL,	SetEnableArea,	BOOL)

// 断网
RCF_METHOD_R_0(BOOL,	GetEnableNetwork)
RCF_METHOD_R_1(BOOL,	SetEnableNetwork,	BOOL)
// 当前已连接信息
RCF_METHOD_R_1(BOOL,	GetNetConnectInfo,	KConnectInfo*)

// 受到攻击时报警
RCF_METHOD_R_1(HRESULT, GetSoundOnAttack,		BOOL*)
RCF_METHOD_R_1(HRESULT, SetSoundOnAttack,		BOOL)

RCF_METHOD_R_1(HRESULT, GetSoundPath,		ATL::CString*)
RCF_METHOD_R_1(HRESULT, SetSoundPath,		ATL::CString)

// 动态防御
RCF_METHOD_R_1(HRESULT, GetShieldCnt, INT*)

RCF_METHOD_R_1(HRESULT, GetAutoShieldAttacker,		BOOL*)
RCF_METHOD_R_1(HRESULT, SetAutoShieldAttacker,		BOOL)

RCF_METHOD_R_1(HRESULT, GetShieldAttackCount,		INT*)
RCF_METHOD_R_1(HRESULT, SetShieldAttackCount,		INT)

RCF_METHOD_R_1(HRESULT, GetShieldTime,		INT*)
RCF_METHOD_R_1(HRESULT, SetShieldTime,		INT)

// 是否信任可信认证 
RCF_METHOD_R_0(BOOL,		GetTrustRep)
RCF_METHOD_R_1(BOOL,		SetTrustRep,	BOOL)

// 设置用户类型
RCF_METHOD_R_0(DWORD,		GetUserType)
RCF_METHOD_R_1(HRESULT,		SetUserType,	DWORD)

RCF_METHOD_R_2(TrustMode,	CheckTrustMode, ATL::CString, DWORD)

// 获取地理地址
RCF_METHOD_R_2(HRESULT,		GetGeoAddr, DWORD, ATL::CString*)

// 终止进程
RCF_METHOD_R_1(HRESULT,		StopProcess,	ULONG )

//日志相关
RCF_METHOD_R_0(HRESULT,		ClearLog)

RCF_METHOD_R_1(HRESULT,		GetLogMaxSize,		DWORD*)
RCF_METHOD_R_1(HRESULT,		SetLogMaxSize,		DWORD)

RCF_METHOD_R_1(HRESULT,		GetLogRetention,	DWORD*)
RCF_METHOD_R_1(HRESULT,		SetLogRetention,	DWORD)

RCF_METHOD_R_1(HRESULT,		GetLogPageCnt,		DWORD*)
RCF_METHOD_R_1(HRESULT,		SetLogPageCnt,		DWORD)

RCF_METHOD_R_2(HRESULT,		GetLogSize,			DWORD*, DWORD*)
RCF_METHOD_R_1(HRESULT,		GetInstallPath,		ATL::CString*)

RCF_CLASS_END

// 实现类的CLSID
_declspec(selectany) extern const KSCLSID CLSID_KNetInfoImp = { 0x34b53966, 0x744b, 0x40c4, { 0xb6, 0xcd, 0xc8, 0xb7, 0x5f, 0x82, 0xcb, 0xb8 } };
