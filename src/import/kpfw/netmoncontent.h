////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : netmoncontent.h
//      Version   : 1.0
//      Comment   : 封装网镖组件接口
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#include "netmonapi.h"

//////////////////////////////////////////////////////////////////////////
interface IFwProxy;
interface IAntiArpDevC;
interface IArpTableSafeguard;

class KNetmonContent
{
private:
	IKNetMon*				m_pNetMon;

	IFwProxy*				m_fwProxy;
	IAntiArpDevC*			m_pAntiArpDev;
	IKNetTools*				m_pTools;
	IKNetConfig*			m_pConfig;
	IArpTableSafeguard*		m_pIpGuard;
	IKNetmonLogger*			m_pLoger;

	IKIpRuleMgr*			m_pIpRuleMgr;
	IKNetmonAttackMgr*		m_pAttackMgr;
	IKNetmonPopMgr*			m_pPopMgr;
	IKNetmonTrustMgr*		m_pTrustMgr;
	IKMsnCrypt*				m_pMsnMgr;

public:
	KNetmonContent(IKNetMon* pNetMon): m_pNetMon(pNetMon)
		, m_fwProxy(NULL)
		, m_pAntiArpDev(NULL)
		, m_pTools(NULL)
		, m_pConfig(NULL)
		, m_pLoger(NULL)
		, m_pIpGuard(NULL)
		, m_pIpRuleMgr(NULL)
		, m_pAttackMgr(NULL)
		, m_pPopMgr(NULL)
		, m_pTrustMgr(NULL)
		, m_pMsnMgr(NULL)
	{}
	~KNetmonContent()	{}

	IKNetMon*		GetNetmon()
	{
		return m_pNetMon;
	}

#define	EXPORT_COMPOBJECT(_interface, _funcname, _member)	\
	_interface*		_funcname()								\
	{														\
		if (!_member)	QueryObject(_member);				\
		ASSERT(_member);									\
		return _member;										\
	}

	EXPORT_COMPOBJECT(IFwProxy,			GetFwProxy,				m_fwProxy)
	EXPORT_COMPOBJECT(IKNetTools,		GetNetTools,			m_pTools)
	EXPORT_COMPOBJECT(IKNetConfig,		GetNetConfig,			m_pConfig)
	EXPORT_COMPOBJECT(IAntiArpDevC,		GetAntiArpDevC,			m_pAntiArpDev)
	EXPORT_COMPOBJECT(IArpTableSafeguard, GetArpTableSafegaurd, m_pIpGuard)
	EXPORT_COMPOBJECT(IKIpRuleMgr,		GetIpMgr,				m_pIpRuleMgr)
	EXPORT_COMPOBJECT(IKNetmonAttackMgr, GetAttackMgr,			m_pAttackMgr)
	EXPORT_COMPOBJECT(IKNetmonLogger,	GetLogger,				m_pLoger)
	EXPORT_COMPOBJECT(IKNetmonPopMgr,	GetPopMgr,				m_pPopMgr)
	EXPORT_COMPOBJECT(IKNetmonTrustMgr, GetTrustMgr,			m_pTrustMgr)
	EXPORT_COMPOBJECT(IKMsnCrypt,		GetMsnMgr,				m_pMsnMgr)

	template<class T>
	HRESULT	QueryObject(T*& pObject)
	{
		return m_pNetMon->QueryObject(__uuidof(T), (void**)&pObject);
	}
};