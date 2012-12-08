////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : metmonapi.h
//      Version   : 1.0
//      Comment   : 定义网镖对外暴露的接口
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 
#include "unknwn.h"

#include "netwatch.h"
#include "antiarp/arpcommondef.h"
#include "netdef.h"
#include "netlog.h"

interface IKNetConfig;

//////////////////////////////////////////////////////////////////////////
// 网络组件定义
#define NETMON_COMPONENT_LIST		L"SOFTWARE\\Kingsoft\\KISCommon\\KXEngine\\netmon\\components\\"
#define NETMON_CFG_ROOT				L"SOFTWARE\\Kingsoft\\KISCommon\\KXEngine\\netmon\\"
#define NETMON_CFG_KEY_NAME			L"components"

//////////////////////////////////////////////////////////////////////////
// 定义网镖工具使用接口，方便进行单元测试
#if defined(_MSC_VER) && (_MSC_VER >= 1300) 
[
	uuid("0C035147-D857-47e6-A7D5-F447C3C360DC")
]
#else //_MSC_VER
interface __declspec(uuid("0C035147-D857-47e6-A7D5-F447C3C360DC")) IKNetFactory;
#endif//_MSC_VER
interface IKNetFactory: public IUnknown
{
	virtual STDMETHODIMP 		CreateObject(REFIID iid, void** ppObject)		PURE;
};
//------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// 定义网镖进程工具接口
class KModuleInfo;
interface IFwProxy;

interface IKProcInfo
{
	virtual STDMETHODIMP_(KModuleInfo*) 		SetExePath(ULONG nModuleID, LPWSTR strPath, IFwProxy*)						PURE;
	virtual STDMETHODIMP_(KModuleInfo*) 		AddDllPath(ULONG nModuleID, LPWSTR strDllPath, IFwProxy*)					PURE;
};

[
uuid("7471B5A3-6E79-4188-8CE7-FF7663A9A43E")
]
interface IKNetTools: public IUnknown
{
	// 获取模块列表
	virtual STDMETHODIMP 		GetProcInfoFromRequest(APP_REQUEST_INFO* qRequest, 
									   IKProcInfo* pProcInfo)	PURE;

	virtual STDMETHODIMP 		GetProcessPath(DWORD nProcessID, WCHAR path[MAX_PATH+1])	PURE;

	virtual STDMETHODIMP_(BOOL) IsCoperate(LPCWSTR strFile)						PURE;

	virtual STDMETHODIMP_(BOOL) IsDigSign(LPCWSTR strFile)						PURE;

	virtual STDMETHODIMP 		CalcCheckSum(LPCWSTR strFile, DWORD nThreadID, DWORD nProcessID, BYTE checkSum[16])	PURE;

	virtual STDMETHODIMP 		GetLastModifyTime(LPCWSTR strFile, DWORD nThreadID, DWORD nProcessID, FILETIME& time)	PURE;

	virtual	STDMETHODIMP		AddTrustMd5(BYTE md5[16])							PURE;

	virtual STDMETHODIMP_(BOOL) IsTrustMd5(BYTE md5[16])						PURE;

	virtual STDMETHODIMP_(BOOL) IsExist(LPCWSTR strFile, DWORD nThreadID, DWORD nProcessID)						PURE;
};

//------------------------------------------------------------------------

enum ENetPathCfg
{
	Path_cfg_app_pop_html		= 1,
	Path_cfg_common_pop_html	= 2,
	Path_cfg_newarea_pop_html	= 3,
	Path_cfg_nsn_pop_html		= 4,
};

//////////////////////////////////////////////////////////////////////////
// 网镖配置接口
[
	uuid("5676E426-B6C2-4455-AFB3-96863D78E4D7")
]
interface IKNetConfig: public IUnknown
{
	virtual STDMETHODIMP_(LPCWSTR) GetDigWhiteCfgPath()								PURE;
	virtual STDMETHODIMP 		GetAppRuleFile(LPWSTR strFile, INT& nLen)			PURE;
	virtual STDMETHODIMP 		GetModuleRuleFile(LPWSTR strFile, INT& nLen)		PURE;
	virtual STDMETHODIMP 		GetIPRuleFile(LPWSTR strFile, INT& nLen)			PURE;
	virtual STDMETHODIMP 		GetDefIPRuleFile(LPWSTR strFile, INT& nLen)			PURE;
	virtual	STDMETHODIMP 		GetAdvFltRuleFile(LPWSTR strFile, INT& nLen)			PURE;

	virtual STDMETHODIMP 		GetAreaFile(LPWSTR strFile, INT& nLen)				PURE;
	virtual STDMETHODIMP 		GetTrustAreaIpRuleFile(LPWSTR strFile, INT& nLen)	PURE;
	virtual STDMETHODIMP 		GetUntrustIpRuleFile(LPWSTR strFile, INT& nLen)		PURE;
	virtual STDMETHODIMP 		GetUnknownIpRuleFile(LPWSTR strFile, INT& nLen)		PURE;
	
	virtual STDMETHODIMP 		GetPopHtmlPath(LPWSTR strFile, INT& nLen)		PURE;

	virtual STDMETHODIMP 		GetPathCfg(ENetPathCfg, LPWSTR strFile, INT& nLen)		PURE;
	virtual STDMETHODIMP 		GetFluxCfgFile(LPWSTR strFile, INT& nLen)		PURE;

	virtual STDMETHODIMP 		GetBlackUrlPackageFile(LPWSTR strFile, INT& nLen)		PURE;
	virtual STDMETHODIMP_(BOOL) GetEnableBlackUrlBlock()								PURE;
	virtual STDMETHODIMP 		SetEnableBlackUrlBlock(BOOL bEnable)					PURE;

	virtual STDMETHODIMP_(BOOL) GetHide()											PURE;
	virtual STDMETHODIMP 		SetHide(BOOL hHide)									PURE;

	// arp 防火墙设置
	virtual STDMETHODIMP_(BOOL) GetArpEnable()										PURE;
	virtual STDMETHODIMP 		SetArpEnable(BOOL bEnable)							PURE;

	virtual STDMETHODIMP_(BOOL) GetUseAutoGateAddr()								PURE;
	virtual STDMETHODIMP 		SetUseAutoGateAddr(BOOL bEnable)					PURE;

	virtual STDMETHODIMP 		GetManualGateAddr(mac_ip*& pAddr, INT& nCnt)		PURE;
	virtual STDMETHODIMP 		SetManualGateAddr(mac_ip* pAddr, INT nCnt)			PURE;

	// 开机自动运行
	virtual STDMETHODIMP 		GetBootAutoRunNetMon (BOOL*)						PURE;
	virtual STDMETHODIMP 		SetBootAutoRunNetMon (BOOL)							PURE;

	// 受到攻击时报警
	virtual STDMETHODIMP 		GetSoundOnAttack (BOOL*)							PURE;
	virtual STDMETHODIMP 		SetSoundOnAttack (BOOL)								PURE;

	virtual STDMETHODIMP 		GetSoundPath (ATL::CString*)						PURE;
	virtual STDMETHODIMP 		SetSoundPath (ATL::CString)							PURE;

	// 动态防御
	virtual STDMETHODIMP 		GetAutoShieldAttacker (BOOL*)						PURE;
	virtual STDMETHODIMP 		SetAutoShieldAttacker (BOOL)						PURE;

	virtual STDMETHODIMP 		GetShieldAttackCount (INT*)							PURE;
	virtual STDMETHODIMP 		SetShieldAttackCount (INT)							PURE;

	virtual STDMETHODIMP 		GetShieldTime (INT*)								PURE;
	virtual STDMETHODIMP 		SetShieldTime (INT)									PURE;

	// msn加密
	virtual STDMETHODIMP 		GetAutoEnableMsnCrypt (BOOL*)						PURE;
	virtual STDMETHODIMP 		SetAutoEnableMsnCrypt (BOOL)						PURE;

	virtual STDMETHODIMP 		GetShowMsnPop(BOOL*)								PURE;
	virtual STDMETHODIMP 		SetShowMsnPop(BOOL)									PURE;

	// 是否自动信任可信认证
	virtual STDMETHODIMP_(BOOL) GetTrustRep()										PURE;
	virtual STDMETHODIMP 		SetTrustRep(BOOL hTrust)							PURE;

	// arp 高级设置
	virtual STDMETHODIMP		GetNotifyGateway(BOOL* bNotify, INT* nTimes )		PURE;
	virtual STDMETHODIMP		SetNotifyGateway(BOOL bNotify, INT nTimes)			PURE;

	virtual STDMETHODIMP		GetArpSafe(BOOL* bSafe)								PURE;
	virtual STDMETHODIMP		SetArpSafe(BOOL bSafe)								PURE;

	virtual STDMETHODIMP		GetAttachSetting(BOOL*,BOOL*,BOOL*)					PURE;
	virtual STDMETHODIMP		SetAttachSetting(BOOL, BOOL, BOOL )					PURE;

	virtual	STDMETHODIMP_(LONG)	GetUpdateTime()										PURE;

	virtual	STDMETHODIMP_(DWORD) GetUserType()										PURE;
	virtual STDMETHODIMP		SetUserType(DWORD)									PURE;

	// 区域设置
	virtual STDMETHODIMP		GetAreaIndexs(long* pHome, long* pOffice, long* pUnkonwn)	PURE;
	virtual STDMETHODIMP		SetAreaIndexs(long nHome, long nOffice, long npUnkonwn)	PURE;

	//获取安装路径
	virtual STDMETHODIMP_(LPCTSTR) GetInstallPath()									PURE;	
};
//------------------------------------------------------------------------

MIDL_INTERFACE("E4D68C99-8947-44b2-BEA2-B5504F7645BE")
IKNetMonIdle : public IUnknown
{
	// 给外部一个机会创建网镖所依赖的接口，该函数必须在Init之前调用
	virtual STDMETHODIMP 		DoIdle()	PURE;
};


//////////////////////////////////////////////////////////////////////////
// 定义网镖对外接口
interface IAntiArpCallback;
interface IFwEvent;
interface IDifferentMacCallback;

MIDL_INTERFACE("67F9D9D5-5B39-4b95-9CEB-C84A6F6F4198")
IKNetMon : public IUnknown
{
	// 给外部一个机会创建网镖所依赖的接口，该函数必须在Init之前调用
	virtual STDMETHODIMP 		RegisterObject(REFIID id, void* pObject)		PURE;
	virtual STDMETHODIMP 		UnRegisterObject(REFIID id)						PURE;
	virtual STDMETHODIMP 		QueryObject(REFIID id, void** pObject)			PURE;
	virtual STDMETHODIMP		UpdateIpGaurd()									PURE;

	virtual STDMETHODIMP 		RegisterIdle(IKNetMonIdle* pIdle)				PURE;
	virtual STDMETHODIMP 		RegisterFwEvent(IFwEvent* pEvent)				PURE;
	virtual STDMETHODIMP 		RegisterArpEvent(IAntiArpCallback* pEvent)				PURE;
	virtual STDMETHODIMP 		RegisterIpGuardEvent(IDifferentMacCallback* pEvent)				PURE;
};

//////////////////////////////////////////////////////////////////////////
// msn加密接口
MIDL_INTERFACE("702637B6-3A81-49d6-9707-4720DFA644DF")
IKMsnCrypt : public IUnknown
{
	virtual STDMETHODIMP_(BOOL)	GetEnable()					PURE;
	virtual STDMETHODIMP		SetEnable(BOOL bEnable)		PURE;
};

//////////////////////////////////////////////////////////////////////////
// 日志接口
namespace netmon{
	class LOG_APP;
	class LOG_APP;
	class LOG_SHEID;
	class LOG_ARP;
	class LOG_AREA;
}
class KAppRule;

MIDL_INTERFACE("6109DD65-55D0-48c8-AB6F-CC8A7882B26D")
IKNetmonLogger : public IUnknown
{
	virtual STDMETHODIMP	WriteIpLog(const LOG_IP_RULE* logIpRule)		PURE;
	virtual STDMETHODIMP	WriteAppLog(netmon::LOG_APP* pLog)				PURE; 
	virtual STDMETHODIMP	WriteAppDenyLog(netmon::LOG_APP* pLog, LPCWSTR strModule)PURE;
	virtual STDMETHODIMP	WriteShieldLog(netmon::LOG_SHEID* pLog)PURE;
	virtual STDMETHODIMP	WriteAppRuleLog(AppRuleLogFlag eFlag,KAppRule* pRule, BOOL bDll = TRUE)PURE;
	virtual STDMETHODIMP	WriteArpLog(netmon::LOG_ARP* pLog)PURE;
	virtual STDMETHODIMP	WriteAreaLog( netmon::LOG_AREA* pLog )PURE;
	virtual STDMETHODIMP	WriteUserSetting( LogUserOpt luo, LogOptType lot )PURE;
	virtual STDMETHODIMP	CleanLog()PURE;
	virtual STDMETHODIMP	UpdateLogMaxSize(DWORD* dwSize, BOOL bGet)PURE;
	virtual STDMETHODIMP	UpdateLogRetention(DWORD* dwSize, BOOL bGet)PURE;
	virtual STDMETHODIMP	UpdateLogPageCnt(DWORD* nSize, BOOL bGet)PURE;
	virtual STDMETHODIMP	GetLogSize( DWORD * dwSizeLow, DWORD* dwSizeHigh )PURE;

	virtual void			log_msg(const wchar_t *format, ...)		PURE;
};

//////////////////////////////////////////////////////////////////////////
// 泡泡管理接口
class KArea;
class KAppInfo;

MIDL_INTERFACE("8589E7ED-29C9-41c1-9E42-6E50B2C41923")
IKNetmonPopMgr : public IUnknown
{
	virtual STDMETHODIMP	AddAppPopSession(KAppInfo* pAppInfo)PURE;
	virtual STDMETHODIMP	AddAreaPopSession(KArea* pArea, long nNextHome, long nNextOffice, long nNextUnkonwn, DWORD nMaxReTry = 0)PURE;
	virtual STDMETHODIMP	AddArpPopSession(ARP_EXTRA_DATA* arpInfo, int nType)PURE;
	virtual STDMETHODIMP	AddFindVirusPopSession(KAppInfo* pAppInfo)PURE;
	virtual STDMETHODIMP	AddChangeAreaPopSession(LPCWSTR strAreaName, int areaType)PURE;
	virtual STDMETHODIMP	AddUpdatePopSession(DWORD nStatus)PURE;
	virtual STDMETHODIMP	AddAdvFltPopSession(PLOG_IP_RULE log)PURE;
	virtual STDMETHODIMP	AddActiveDefindSession(UINT ip, UINT nTime)PURE;
	virtual STDMETHODIMP	AddAppDenyPopSession(KAppInfo* pAppInfo)PURE;
};

//////////////////////////////////////////////////////////////////////////
// 可信认证管理接口
class KTrustSession;

MIDL_INTERFACE("DF586F59-7C61-46d4-852A-7C56925CD1D1")
IKNetmonTrustMgr : public IUnknown
{
	virtual STDMETHODIMP	AddSession(KAppInfo* pAppInfo)PURE;
	virtual STDMETHODIMP	AddSession(KTrustSession* pSession)PURE;
};


//////////////////////////////////////////////////////////////////////////
// ip规则管理器接口
class KIpRuleList;

MIDL_INTERFACE("168FAEBB-DABF-4072-A704-660DDA6012D2")
IKIpRuleMgr : public IUnknown
{
	virtual STDMETHODIMP_(BOOL)			DisableNetwork()PURE;
	virtual STDMETHODIMP_(BOOL)			EnableNetwork()PURE;
	virtual STDMETHODIMP_(BOOL)			IsDisabled()PURE;
	virtual STDMETHODIMP_(BOOL)			SetHide(BOOL bHide)PURE;
	virtual STDMETHODIMP_(BOOL)			GetHide()PURE;
	virtual STDMETHODIMP_(BOOL)			SetDynRules(KIpRuleList* pRuleList)PURE;
	virtual STDMETHODIMP_(BOOL)			SetServerRule(KIpRuleList* pRuleList)PURE;
	virtual STDMETHODIMP_(BOOL)			SetAreaRules(KIpRuleList* pRuleList)PURE;
	virtual STDMETHODIMP_(BOOL)			UpdateRules()PURE;
	virtual STDMETHODIMP				StopIp()PURE;
	virtual STDMETHODIMP				StartIp()PURE;
};

//////////////////////////////////////////////////////////////////////////
// 攻击管理器
MIDL_INTERFACE("7E70D16E-B075-4bd5-B986-50C5D4435060")
IKNetmonAttackMgr : public IUnknown
{
	virtual STDMETHODIMP			OnAttack(KIPAddr remoteAddr, GUID& id)PURE;
	virtual STDMETHODIMP			SetEnable(BOOL b)PURE;
	virtual STDMETHODIMP_(INT)		GetShieldCnt()PURE;
};

//////////////////////////////////////////////////////////////////////////
// 定义网镖组件接口

// 组件初始化顺序：公共工具-》TDI驱动-》与tdi驱动相关功能组件-》ndis驱动-》与ndis驱动相关功能
enum  NetmonCompInitLevel
{
	Init_Level_Start = 0,

	Base_Tools	= 0,					// 日志，配置，工具
	Common_Tools = 1,					// 可信认证，泡泡
	Tdi_Service = 2,					// 应用程序规则，恶意网址认证，邮件监控等
	Ndis_Service = 3,					// ip规则，arp防火墙，区域
	Stastic_Service = 4,				// 程序流量统计与连接信息统计
	Other_Service	 = 5,				// msn加密

	Max_Level = 6,
};

MIDL_INTERFACE("4E7C5921-55D3-496c-BAFB-84BFAACDDD27")
IKNetmonComponent: public IUnknown
{
	virtual STDMETHODIMP						Initalize(IKNetMon* pNetMon)					PURE;
	virtual	STDMETHODIMP_(ULONG)				GetInitLevel()									PURE;
	virtual STDMETHODIMP						UnInitalize()									PURE;
	virtual STDMETHODIMP_(ULONG)				GetFunctionFlag()								PURE;
	virtual STDMETHODIMP						OnDriverInit(BOOL bInitOrUninit)				PURE;
	virtual STDMETHODIMP						InitSp()										PURE;
	virtual STDMETHODIMP						UnInitSp()										PURE;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
interface IKNetMonSp
{
	virtual ~IKNetMonSp()	{}
	virtual STDMETHODIMP_(int) 	Start()			PURE;
	virtual STDMETHODIMP_(int) 	Stop()			PURE;
};
//------------------------------------------------------------------------
