////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : netpop.h
//      Version   : 1.0
//      Comment   : 定义网镖泡泡数据结构
//      
//      Create at : 2008-7-14
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 
#include "SCOM/SCOM/SCOMBase.h"
#include "SCOM/SCOM/SCOMDef.h"
#include "kisrpc/rfcdef.h"
#include "serializetools.h"
#include "netdef.h"
#include "kis/kpfw/netwatch.h"
#include "kis/kpfw/area.h"
#include <vector>
#include "strsafe.h"
using namespace std;

#pragma pack(push, 1)

typedef enum KPFW_POP_INFO_TYPE
{
	emPop_None				= 0,		//一般不传这种类型
	emPop_App_Info			= 1,		//发现新的程序，模块访问网络(强泡泡) *
	emPop_Arp_Info			= 2,		//arp攻击(弱泡泡)
	emPop_Newnet_Info		= 3,		//发现新区域(强泡泡) *
	emPop_Virus				= 4,		//发现病毒，恶意程序(弱泡泡)
	emPop_PacketFilter		= 5,		//包过滤(弱泡泡)
	emPop_NetAreaChange		= 6,		//区域更改(弱泡泡)
	emPop_ActiveDefend		= 7,		//动态防御(弱泡泡)
	emPop_MSNDecrypt		= 8,		//msn加密(弱泡泡)
	emPop_SvrError			= 9,		//服务错误(弱泡泡)
	emPop_AutoTrust			= 10,		//可信认证放行程序(弱泡泡)
	emPop_UpdateComplete	= 11,		//升级完成
	emPop_enable_MSNDecrypt	= 12,		// 是否启用msn加密(强泡泡)
	emPop_App_Deny			= 13,		// 应用程序访问网络被禁止
}KpfwPopType;


typedef enum KPFW_APP_POP_RES
{
	enumPR_None			= 0,
	enumPR_Allow		= 1,
	enumPR_Deny			= 2,
}AppPopRes;



typedef enum KPFW_POP_PROTOCOL
{
	enumKPP_TCP			= 0,
	enumKPP_UDP			= 1,
	enumKPP_ICMP		= 2,
	enumKPP_IGMP		= 3,
}PopProtocol;

typedef enum APP_POP_REASON
{
	enumPop_Reason_None			= 0,
	enumPop_Reason_Virus		= 1,
	enumPop_Reason_Change		= 1 << 1,
	enumPop_Reason_Unknown_TrustRep = 1 << 2,
	enumPop_Reason_Prompt		= 1 << 3,
	enumPop_Reason_UnTrust_TrustRep = 1 << 4,
}AppPopReason;

typedef enum APP_DENY_REASON
{
	enum_Allow		= 0,						// 允许
	enum_Deny_User	= 1,						// 规则中用户选择禁止
	enum_Deny_Pop	= 2,						// 泡泡选择禁止
	enum_Deny_Trust	= 3,						// 可信认证自动禁止
	enum_Deny_Pop_Rem = 4,						// 记住了上次的泡泡阻止操作，本次自动阻止
	enum_Deny_Port = 5,							// 存在被阻止的端口
}AppDenyReason;

struct PopInfoHeader
{
	DWORD			nSize;
	DWORD			nPopType;
};

//////////////////////////////////////////////////////////////////////////
interface IKPopDataSerializer
{
	// save 接口
	virtual bool IsLoading()								= 0;
	virtual	int	BeginTag(LPCSTR childName)					= 0;
	virtual	int	EndTag(LPCSTR childName)					= 0;
	virtual	int	AddPop(LPCSTR popName, LPCSTR val)			= 0;
	virtual	int	AddPop(LPCSTR popName, INT val)				= 0;
	virtual	int	AddPop(LPCSTR popName, LONG val)			= 0;
	virtual	int	AddPop(LPCSTR popName, LPCWSTR val)			= 0;
	virtual INT	GetProp(LPCSTR propName, INT& val)			= 0;
	virtual INT	GetProp(LPCSTR propName, ATL::CString& val)			= 0;
};

interface IKPopData
{
	virtual bin_archive& Serialize(bin_archive &_serializer) = 0;
	virtual int Serialize(IKPopDataSerializer* _serializer) = 0;
	virtual size_t	GetSeriSize() = 0;
};

inline void SerializeMac(IKPopDataSerializer* _serializer, LPCSTR propName, BYTE mac[6])
{
	char buf[32] = {0};
	StringCchPrintfA(buf, 32, "%02x:02x:02x:02x:02x:02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	// sprintf(buf, "%02x:02x:02x:02x:02x:02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	_serializer->AddPop(propName, buf);
}

inline void SerializeIp(IKPopDataSerializer* _serializer, LPCSTR propName, ULONG nIp)
{
	char buf[32] = {0};
	BYTE* pBuf = (BYTE*)&nIp;
	StringCchPrintfA(buf, 32, "%d.%d.%d.%d", pBuf[0], pBuf[1], pBuf[2], pBuf[3]);
	_serializer->AddPop(propName, buf);
}

inline void SerializeGUID(IKPopDataSerializer* _serializer, LPCSTR propName, GUID& val)
{
	char buf[256] = {0};
	StringCchPrintfA(buf, 256, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", val.Data1, val.Data2, val.Data3, val.Data4[0], val.Data4[1], 
		val.Data4[2], val.Data4[3], val.Data4[4], val.Data4[5], val.Data4[6], val.Data4[7] );
	_serializer->AddPop(propName, buf);
}

inline void SerializeIpRange(IKPopDataSerializer* _serializer, LPCSTR propName, KIPRange& val)
{
	_serializer->BeginTag(propName);
	SerializeIp(_serializer, "start_ip", val.ipStart);
	SerializeIp(_serializer, "end_ip", val.ipEnd);
	_serializer->EndTag(propName);
}

inline void SerializeIPLOG_EXTRADATA(IKPopDataSerializer* _serializer, int nProtocol, LPCSTR propName, IPLOG_EXTRADATA& val)
{
	switch (nProtocol)
	{
	case enumPT_TCP:
		{
			_serializer->BeginTag("TCP_DATA");
			_serializer->AddPop("local_port", val.TCP_DATA.LocalPort);
			_serializer->AddPop("remote_port", val.TCP_DATA.RemotePort);
			_serializer->AddPop("tcp_flag", val.TCP_DATA.byTcpFlags);
			_serializer->EndTag("TCP_DATA");
		}
		break;
	case enumPT_UDP:
		{
			_serializer->BeginTag("UDP_DATA");
			_serializer->AddPop("local_port", val.UDP_DATA.LocalPort);
			_serializer->AddPop("remote_port", val.UDP_DATA.RemotePort);
			_serializer->EndTag("UDP_DATA");
		}
		break;
	case enumPT_ICMP:
		{
			_serializer->BeginTag("ICMP_DATA");
			_serializer->AddPop("code", val.ICMP_DATA.byCode);
			_serializer->AddPop("type", val.ICMP_DATA.byType);
			_serializer->EndTag("ICMP_DATA");
		}
		break;
	case enumPT_IGMP:
		{

		}
		break;
	case enumPT_IP:
		break;
	}
}

inline void SerializeAppRequest(IKPopDataSerializer* _serializer, int nProtocol, LPCSTR propName, APP_REQUEST_INFO& val)
{
	_serializer->BeginTag("APP_REQUEST_INFO");
	_serializer->AddPop("RequestProtocol", (int)val.nRequestProtocol);
	_serializer->AddPop("nRequestType", (int)val.nRequestType);
	_serializer->AddPop("ProcessId", (int)val.dwProcessId);
	_serializer->AddPop("ThreadId", (int)val.dwThreadId);
	switch (val.nRequestType)
	{
	case ART_TrustUrl:
		{
			_serializer->BeginTag("TrustUrlInfo");
			_serializer->AddPop("host", val.Parameters.TrustUrl.szHost);
			_serializer->AddPop("url", val.Parameters.TrustUrl.szUrl);
			_serializer->EndTag("TrustUrlInfo");
		}
		break;
	case ART_FileChanged:
		{

		}
		break;
	case ART_PacketRecord:
		break;
	default:
		{
			switch (val.nRequestProtocol)
			{
			case enumPT_LANMAN:
				{
					_serializer->BeginTag("LanMonInfo");
					_serializer->AddPop("path", val.Parameters.LanMan.wsPath);
					_serializer->EndTag("LanMonInfo");
				}
				break;
			default:
				{
					_serializer->BeginTag("TdiInfo");
					SerializeIp(_serializer, "RemoteAddress", val.Parameters.TDI.dwRemoteAddress);
					_serializer->AddPop("uRemoteAddress", (LONG)val.Parameters.TDI.dwRemoteAddress);
					_serializer->AddPop("RemotePort", val.Parameters.TDI.wRemotePort);
					SerializeIp(_serializer, "LocalAddress", val.Parameters.TDI.dwLocalAddress);
					_serializer->AddPop("uLocalAddress", (LONG)val.Parameters.TDI.dwLocalAddress);
					_serializer->AddPop("LocalPort", val.Parameters.TDI.wLocalPort);
					_serializer->AddPop("PacketDir", (int)val.Parameters.TDI.uDataLen);
					_serializer->EndTag("TdiInfo");
				}
			}
		}
		break;
	}
	_serializer->EndTag("APP_REQUEST_INFO");
}
//////////////////////////////////////////////////////////////////////////
// 应用程序访问网络泡泡
class KPopModuleInfo : public IKPopData
{
private:
	ATL::CString		m_strPath;
	ATL::CString		m_strDesc;
	INT					m_nPopReason;		// 当使用在应用程序询问泡泡上时类型：AppPopReason， 
											// 当使用在应用程序阻止泡泡上是类型：AppDenyReason
	DWORD				m_nTrustMode;
	DWORD				m_nUserMode;

public:
	KPopModuleInfo(): m_nPopReason(0),m_nTrustMode(0),m_nUserMode(0) {}

	KPopModuleInfo(LPCWSTR strPath, LPCWSTR strDesc, INT nPopReason, DWORD nUserMode, DWORD nTrustMode)
	{
		m_strPath = strPath;
		m_strDesc = strDesc;
		m_nPopReason = nPopReason;
		m_nUserMode = nUserMode;
		m_nTrustMode = nTrustMode;
	}

	~KPopModuleInfo() {}

	void	Init(LPCWSTR strPath ,INT nReason, LPCWSTR strDesc)
	{
		m_strPath = strPath;
		m_nPopReason = nReason;
		m_strDesc = strDesc;
	}

	void	SetMode(DWORD nTrustMode, DWORD nUserMode)
	{
		m_nTrustMode = nTrustMode;
		m_nUserMode = nUserMode;
	}
	void	SetUserMode(DWORD nUserMode)
	{
		m_nUserMode = nUserMode;
	}
	ATL::CString&	GetPath()	{ return m_strPath; }

	ATL::CString&	GetDesc()	{ return m_strDesc; }

	INT			GetPopReason() { return m_nPopReason; }

	DWORD		GetTrustMode()		{ return m_nTrustMode; }

	DWORD		GetUserMode()		{ return m_nUserMode; }

	size_t	GetSeriSize()
	{
		size_t nSize = 0;
		nSize += TypeSize::get_size(m_strPath);
		nSize += TypeSize::get_size(m_strDesc);
		nSize += sizeof(m_nPopReason) + sizeof(m_nTrustMode) + sizeof(m_nUserMode);
		return nSize;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			_serializer >> m_strPath;
			_serializer >> m_strDesc;
			_serializer >> m_nPopReason;
			_serializer >> m_nTrustMode;
			_serializer >> m_nUserMode;
		}
		else
		{
			_serializer << m_strPath;
			_serializer << m_strDesc;
			_serializer << m_nPopReason;
			_serializer << m_nTrustMode;
			_serializer << m_nUserMode;
		}
		return _serializer;
	}

	virtual int Serialize(IKPopDataSerializer* _serializer)
	{
		if (!_serializer->IsLoading())
		{
			_serializer->BeginTag("KPopModuleInfo");
			_serializer->AddPop("Path", m_strPath);
			_serializer->AddPop("Desc", m_strDesc);
			_serializer->AddPop("PopReason", m_nPopReason);
			_serializer->AddPop("TrustMode", (int)m_nTrustMode);
			_serializer->AddPop("UserMode", (int)m_nUserMode);
			_serializer->EndTag("KPopModuleInfo");
		}
		return 0;
	}

};

class KPopAppInfo: public IKPopData
{
public:
	KPopAppInfo(): m_pExeInfo(NULL),m_ePopRes(enumPR_None),	m_bRemember(FALSE) {
		memset(&m_RequestInfo, 0, sizeof(m_RequestInfo));
	}

	~KPopAppInfo()	{ 
		Clear(); 
	}

	ATL::CString&	GetExePath()			
	{
		if (m_pExeInfo)
			return m_pExeInfo->GetPath();
		static ATL::CString tmp(TEXT(""));
		return tmp;
	}

	INT				GetDllCnt()				
	{
		return (INT)m_dllInfos.size();
	}

	KPopModuleInfo* GetExeInfo()		{ return m_pExeInfo; }
	KPopModuleInfo* GetDllInfo(INT i)	
	{
		return m_dllInfos[i];
	}

	ATL::CString&	GetDllPath(INT i)		
	{
		return m_dllInfos[i]->GetPath();
	}

	KPopModuleInfo* AddExePath(LPCWSTR str, INT nReason, LPCWSTR strDesc)		
	{
		ASSERT(!m_pExeInfo);
		if (!m_pExeInfo)
		{
			m_pExeInfo = new KPopModuleInfo();
			m_pExeInfo->Init(str, nReason, strDesc);
			return m_pExeInfo;
		}
		return NULL;
	}

	KPopModuleInfo* AddDllPath(LPCWSTR str, INT nReason, LPCWSTR strDesc)		
	{
		KPopModuleInfo* pDllInfo = new KPopModuleInfo();
		pDllInfo->Init(str, nReason, strDesc);
		m_dllInfos.push_back(pDllInfo);
		return pDllInfo;
	}

	DWORD	GetProcessID()
	{
		return m_RequestInfo.dwProcessId;
	}

	VOID	SetRequestInfo(PAPP_REQUEST_INFO pInfo)
	{
		if (pInfo)
			m_RequestInfo = *pInfo;
	}
	PAPP_REQUEST_INFO GetRequestInfo()
	{
		return &m_RequestInfo;
	}

	AppPopRes GetPopRes()
	{
		return (AppPopRes)m_ePopRes;
	}
	void SetPopRes(AppPopRes res)
	{
		m_ePopRes = (DWORD)res;
	}

	BOOL GetRemember()
	{
		return m_bRemember;
	}
	VOID SetRemember(BOOL b)
	{
		m_bRemember = b;
	}

	virtual int Serialize(IKPopDataSerializer* _serializer)
	{
		if (!_serializer->IsLoading())
		{
			_serializer->BeginTag("KPopAppInfo");

			SerializeAppRequest(_serializer, 0, "", m_RequestInfo);

			_serializer->BeginTag("ExeInfo");
			if (m_pExeInfo)
				m_pExeInfo->Serialize(_serializer);
			_serializer->EndTag("ExeInfo");

			_serializer->BeginTag("DllInfo");
			for (size_t i = 0; i < m_dllInfos.size(); i++)
			{
				m_dllInfos[i]->Serialize(_serializer);
			}
			_serializer->EndTag("DllInfo");

			_serializer->EndTag("KPopAppInfo");
		}
		else
		{
			INT nVal = 0;
			_serializer->GetProp("PopRes", nVal);
			m_ePopRes = nVal;
			nVal = 0;
			_serializer->GetProp("Remember", nVal);
			m_bRemember = nVal;
		}
		return 0;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			Clear();
			_serializer >> m_ePopRes;
			_serializer >> m_bRemember;
			_serializer.read(&m_RequestInfo, sizeof(m_RequestInfo), 1);

			m_pExeInfo = new KPopModuleInfo();
			m_pExeInfo->Serialize(_serializer);
			INT nCnt = 0;
			_serializer >> nCnt;
			for (int i = 0; i < nCnt; i++)
			{
				KPopModuleInfo* pDllInfo = new KPopModuleInfo;
				pDllInfo->Serialize(_serializer);
				m_dllInfos.push_back(pDllInfo);
			}
		}
		else
		{
			ASSERT(m_pExeInfo);
			// 先序列化常规参数
			_serializer << m_ePopRes;
			_serializer << m_bRemember;
			_serializer.write(&m_RequestInfo, sizeof(m_RequestInfo), 1);

			m_pExeInfo->Serialize(_serializer);
			INT nCnt = (int)m_dllInfos.size();
			_serializer << nCnt;
			for (int i = 0; i < nCnt; i++)
				m_dllInfos[i]->Serialize(_serializer);
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSize = 0;
		nSize += sizeof(m_ePopRes) + sizeof(m_bRemember) + sizeof(m_RequestInfo);

		nSize += m_pExeInfo->GetSeriSize();

		INT nCnt = (int)m_dllInfos.size();
		nSize += sizeof(nCnt);
		for (int i = 0; i < nCnt; i++)
			nSize += m_dllInfos[i]->GetSeriSize();
		return nSize;
	}

private:
	void	Clear()
	{
		if (m_pExeInfo)
		{
			delete m_pExeInfo;
			m_pExeInfo = NULL;
		}
		for (int i = 0; i < (int)m_dllInfos.size(); i++)
		{
			delete m_dllInfos[i];
		}
		m_dllInfos.clear();
	}

private:
	DWORD						m_ePopRes;
	BOOL						m_bRemember;
	APP_REQUEST_INFO			m_RequestInfo;
	
	KPopModuleInfo*				m_pExeInfo;
	vector<KPopModuleInfo*>		m_dllInfos;
};

//////////////////////////////////////////////////////////////////////////
// 自动放行规则

class KAutoTrustInfo : public IKPopData
{
public:

	KAutoTrustInfo()
	{
	}

	~KAutoTrustInfo()
	{}

	KAutoTrustInfo( ATL::CString& strFileName )
	{
		m_strFileName = strFileName;
	}

	KAutoTrustInfo( LPCTSTR pszFileName )
	{
		m_strFileName = pszFileName;
	}

	void GetFileName( ATL::CString& strFileName )
	{
		strFileName = m_strFileName;
	}

	virtual int Serialize(IKPopDataSerializer* _serializer)
	{
		if (!_serializer->IsLoading())
		{
			_serializer->BeginTag("KAutoTrustInfo");
			_serializer->AddPop("FileName", m_strFileName);
			_serializer->EndTag("KAutoTrustInfo");
		}
		return 0;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			_serializer >> m_strFileName;
		}
		else
		{
			_serializer << m_strFileName;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSize = 0;
		nSize += TypeSize::get_size( m_strFileName );
		return nSize;
	}

private:

	ATL::CString m_strFileName;
};

//////////////////////////////////////////////////////////////////////////
// 包过滤泡泡

class KPacketFilterInfo : public IKPopData
{
private:
	LOG_IP_RULE				m_logInfo;

public:

	KPacketFilterInfo()
	{}

	KPacketFilterInfo( PLOG_IP_RULE rule)
	{
		m_logInfo = *rule;
	}

	~KPacketFilterInfo()
	{}

	PLOG_IP_RULE	GetLogInfo()
	{
		return &m_logInfo;
	}

	virtual int Serialize(IKPopDataSerializer* _serializer)
	{
		if (!_serializer->IsLoading())
		{
			_serializer->BeginTag("KPacketFilterInfo");

				_serializer->BeginTag("LOG_IP_RULE");

				SerializeGUID(_serializer, "id", m_logInfo.id);
				_serializer->AddPop("count", (int)m_logInfo.nCount);
				_serializer->AddPop("protocol", m_logInfo.byProtocolType);
				SerializeIp(_serializer, "LocalAddr", ntohl( (unsigned int) m_logInfo.LocalAddr ) );
				SerializeIp(_serializer, "RemoteAddr", ntohl( (unsigned int) m_logInfo.RemoteAddr ) );
				_serializer->AddPop("Direction", (int)m_logInfo.byDirection);
				_serializer->AddPop("Operation", (int)m_logInfo.nOperation);
				_serializer->AddPop("AttackType", (int)m_logInfo.nAttackType);
				SerializeIPLOG_EXTRADATA(_serializer, m_logInfo.byProtocolType, "", m_logInfo.ExtraInfo);

				_serializer->EndTag("LOG_IP_RULE");

			_serializer->EndTag("KPacketFilterInfo");
		}
		return 0;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			_serializer >> m_logInfo;
		}
		else
		{
			_serializer << m_logInfo;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSize = 0;
		nSize += sizeof( m_logInfo );
		return nSize;
	}

private:
};

//////////////////////////////////////////////////////////////////////////
// 发现病毒泡泡
class KFindVirusInfo : public IKPopData
{
public:

	KFindVirusInfo()
	{}

	KFindVirusInfo ( ATL::CString& strFileName, ATL::CString& strVirusName )
	{
		m_strFileName = strFileName;
		m_strVirusName = strVirusName;
	}

	KFindVirusInfo ( LPCTSTR strFileName, LPCTSTR strVirusName )
	{
		m_strFileName = strFileName;
		m_strVirusName = strVirusName;
	}

	bool operator == (const KFindVirusInfo& info)
	{
		if (info.m_strFileName != m_strFileName)
			return false;
		if (info.m_strVirusName != m_strVirusName)
			return false;
		return true;
	}

	~KFindVirusInfo ()
	{}

	void GetFileName( ATL::CString& strFileName )
	{
		strFileName = m_strFileName;
	}

	void GetVirusName( ATL::CString& strVirusName )
	{
		strVirusName = m_strVirusName;
	}

	virtual int Serialize(IKPopDataSerializer* _serializer)
	{
		if (!_serializer->IsLoading())
		{
			_serializer->BeginTag("KFindVirusInfo");
			_serializer->AddPop("FileName", m_strFileName);
			_serializer->AddPop("VirusName", m_strVirusName);
			_serializer->EndTag("KFindVirusInfo");
		}
		return 0;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			_serializer >> m_strFileName;
			_serializer >> m_strVirusName;
		}
		else
		{
			_serializer << m_strFileName;
			_serializer << m_strVirusName;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSize = 0;
		nSize += TypeSize::get_size( m_strFileName ) ;
		nSize += TypeSize::get_size( m_strVirusName ) ;
		return nSize;
	}

private:

	ATL::CString m_strFileName;
	ATL::CString m_strVirusName;
};

class KFindVirusInfoVec: public IKPopData
{
private:
	vector<KFindVirusInfo*>		m_VirusInfo;
	ATL::CString				m_ProcessName;

public:
	KFindVirusInfoVec()	{}
	~KFindVirusInfoVec()	{Clear();}

	void  SetProcessName(LPCWSTR name)
	{
		m_ProcessName = name;
	}

	ATL::CString& GetProcessName()
	{
		return m_ProcessName;
	}

	void  AddInfo(LPCWSTR file ,LPCWSTR desc)
	{
		KFindVirusInfo* pInfo = new KFindVirusInfo(file, desc);
		m_VirusInfo.push_back(pInfo);
	}

	vector<KFindVirusInfo*>& GetVirusInfo()
	{
		return m_VirusInfo;
	}

	bool operator==(const KFindVirusInfoVec& info)
	{
		if (info.m_ProcessName != m_ProcessName)
			return false;
		if (info.m_VirusInfo.size() != m_VirusInfo.size())
			return false;
		for (int i = 0; i < (int)m_VirusInfo.size(); i++)
			if (!(*m_VirusInfo[i] == *info.m_VirusInfo[i]))
				return false;
		return true;
	}

	int	GetCnt()
	{
		return (int)m_VirusInfo.size();
	}

	virtual int Serialize(IKPopDataSerializer* _serializer)
	{
		if (!_serializer->IsLoading())
		{
			_serializer->BeginTag("KFindVirusInfoVec");
			_serializer->AddPop("ProcessName", m_ProcessName);

			_serializer->BeginTag("VirusInfos");
			for (size_t i = 0; i < m_VirusInfo.size(); i++)
			{
				m_VirusInfo[i]->Serialize(_serializer);
			}
			_serializer->EndTag("VirusInfos");

			_serializer->EndTag("KFindVirusInfoVec");
		}
		return 0;
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
				KFindVirusInfo* pInfo = new KFindVirusInfo;
				pInfo->Serialize(_serializer);
				m_VirusInfo.push_back(pInfo);
			}
			_serializer >> m_ProcessName;
		}
		else
		{
			int nCnt = (int)m_VirusInfo.size();
			_serializer << nCnt;
			for (int i = 0; i < nCnt; i++)
			{
				m_VirusInfo[i]->Serialize(_serializer);
			}
			_serializer << m_ProcessName;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSize = 0;
		nSize = sizeof(int);
		for (int i = 0; i < (int)m_VirusInfo.size(); i++)
			nSize += m_VirusInfo[i]->GetSeriSize();
		nSize += TypeSize::get_size( m_ProcessName ) ;
		return nSize;
	}
private:
	void Clear()
	{
		for (int i = 0; i < (int)m_VirusInfo.size(); i++)
			delete m_VirusInfo[i];
		m_VirusInfo.clear();
	}
};

//////////////////////////////////////////////////////////////////////////

struct PopAttackInfo
{
	PopInfoHeader	header;
};

//////////////////////////////////////////////////////////////////////////
// arp阻止泡泡
class KPopArpInfo: public IKPopData
{
private:
	ARP_EXTRA_DATA*		m_pArpInfo;
	INT					m_nType;
public:
	KPopArpInfo(): m_pArpInfo(NULL)  {}
	~KPopArpInfo()  { Clear(); }

	const ARP_EXTRA_DATA* GetArpInfo()
	{
		return m_pArpInfo;
	}

	INT	GetType()
	{
		return m_nType;
	}

	void SetArpInfo(ARP_EXTRA_DATA* pInfo, int nType)
	{
		Clear();
		if (pInfo)
		{
			m_pArpInfo = new ARP_EXTRA_DATA;
			*m_pArpInfo = *pInfo;
			m_nType = nType;
		}
	}


	virtual int Serialize(IKPopDataSerializer* _serializer)
	{
		if (!_serializer->IsLoading())
		{
			_serializer->BeginTag("KPopArpInfo");
			_serializer->AddPop("ArpType", m_nType);

				_serializer->BeginTag("ARP_EXTRA_DATA");
				SerializeMac(_serializer, "eh_dst", m_pArpInfo->eh_dst);
				SerializeMac(_serializer, "eh_src", m_pArpInfo->eh_src);
				SerializeMac(_serializer, "arp_sha", m_pArpInfo->arp_sha);
				SerializeIp(_serializer, "arp_spa", m_pArpInfo->arp_spa);
				SerializeMac(_serializer, "arp_tha", m_pArpInfo->arp_tha);
				SerializeIp(_serializer, "arp_tpa", m_pArpInfo->arp_tpa);
				_serializer->EndTag("ARP_EXTRA_DATA");
			
			_serializer->EndTag("KPopArpInfo");
		}
		return 0;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			Clear();
			m_pArpInfo = new ARP_EXTRA_DATA;
			_serializer.read(m_pArpInfo, sizeof(ARP_EXTRA_DATA), 1);
			_serializer >> m_nType;
		}
		else
		{
			if (m_pArpInfo)
				_serializer.write(m_pArpInfo, sizeof(ARP_EXTRA_DATA), 1);
			_serializer << m_nType;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSize = 0;

		if (m_pArpInfo)
			nSize += sizeof(ARP_EXTRA_DATA);

		nSize += sizeof(m_nType);
		return nSize;
	}

	void Clear()
	{
		if (m_pArpInfo)
		{
			delete m_pArpInfo;
			m_pArpInfo = NULL;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
//发现新区域泡泡
class KPopAreaInfo: public IKPopData
{
private:
	BYTE			m_GateMac[6];
	long			m_GateIp;
	int				m_eRes;
	ATL::CString	m_strAreaName;
	
	long			m_nNextHome;
	long			m_nNextOffice;
	long			m_nNextUnknown;

public:
	KPopAreaInfo(): m_eRes(enumAT_Unknown), m_GateIp(0)
	{
		
	}

	~KPopAreaInfo()  { Clear(); }

	int GetPopRes()
	{
		return m_eRes;
	}
	void	SetPopRes(int res)
	{
		m_eRes = res;
	}

	void	SetID(long nNextHome, long nNextOffice, long nNextUnkonwn)
	{
		m_nNextHome = nNextHome;
		m_nNextOffice = nNextOffice;
		m_nNextUnknown = nNextUnkonwn;
	}

	BYTE*		GetGateMac()
	{
		return m_GateMac;
	}

	long	GetGateIp()
	{
		return m_GateIp;
	}

	void SetAreaName( ATL::CString& strAreaName )
	{
		m_strAreaName = strAreaName;
	}

	void GetAreaName( ATL::CString& strAreaName )
	{
		strAreaName = m_strAreaName;
	}

	void SetGateInfo(BYTE gateMac[6], ULONG ip)
	{
		Clear();
		memcpy(m_GateMac, gateMac, sizeof(m_GateMac));
		m_GateIp = ip;
	}

	virtual int Serialize(IKPopDataSerializer* _serializer)
	{
		if (!_serializer->IsLoading())
		{
			_serializer->BeginTag("KPopAreaInfo");
			SerializeIp(_serializer, "GateIp", m_GateIp);
			_serializer->AddPop("AreaName", m_strAreaName);
			SerializeMac(_serializer, "GateMac", m_GateMac);

			_serializer->AddPop("NextHomeID", m_nNextHome);
			_serializer->AddPop("NextOfficeID", m_nNextOffice);
			_serializer->AddPop("NextUnkonwnID", m_nNextUnknown);

			_serializer->EndTag("KPopAreaInfo");
		}
		else
		{
			int nType = 0;
			_serializer->GetProp("AreaType", nType);
			m_eRes = nType;
			_serializer->GetProp("AreaName", m_strAreaName);
		}
		return 0;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			Clear();
			_serializer.read(m_GateMac, sizeof(m_GateMac), 1);
			_serializer >> m_GateIp;
			int nRes = 0;
			_serializer >> nRes;
			_serializer >> m_strAreaName;
			m_eRes = (int)nRes;
		}
		else
		{
			_serializer.write(m_GateMac, sizeof(m_GateMac), 1);
			_serializer << m_GateIp;
			int nRes = m_eRes;
			_serializer << nRes;
			_serializer << m_strAreaName;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSize = 0;

		nSize += sizeof(m_GateMac) + sizeof(m_GateIp) + sizeof(int) ;
		nSize += TypeSize::get_size( m_strAreaName );

		return nSize;
	}

	void Clear()
	{
		memset(m_GateMac, 0, sizeof(m_GateIp));
		m_GateIp = 0;
	}
};
//////////////////////////////////////////////////////////////////////////
// 动态防御泡泡
class KActiveDefendInfo : public IKPopData
{
public:

	KActiveDefendInfo(){}

	KActiveDefendInfo( UINT uIp, UINT uDefendTime )
	{
		m_uIp			= uIp;
		m_uDefendTime	= uDefendTime;
	}
	~KActiveDefendInfo()
	{}

	UINT GetIp()
	{
		return m_uIp;
	}

	UINT GetDefendTime()
	{
		return m_uDefendTime;
	}

	virtual int Serialize(IKPopDataSerializer* _serializer)
	{
		if (!_serializer->IsLoading())
		{
			_serializer->BeginTag("KActiveDefendInfo");
			SerializeIp(_serializer, "Ip", ntohl( (unsigned int)m_uIp) );
			_serializer->AddPop("DefendTime", (int)m_uDefendTime);
			_serializer->EndTag("KActiveDefendInfo");
		}
		return 0;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			_serializer >> m_uIp;
			_serializer >> m_uDefendTime;
		}
		else
		{
			_serializer << m_uIp;
			_serializer << m_uDefendTime;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSize = 0;
		nSize += sizeof( m_uDefendTime );
		nSize += sizeof( m_uIp );
		return nSize;
	}

private:

	UINT	m_uIp;
	UINT	m_uDefendTime;
};

//////////////////////////////////////////////////////////////////////////
//区域改变泡泡
class KChangeAreaInfo : public IKPopData
{
public:

	KChangeAreaInfo( )
	{}

	KChangeAreaInfo( ATL::CString& strAreaName, int areaType )
	{
		m_strAreaName	= strAreaName;
		m_areaType		= areaType;
	}

	KChangeAreaInfo( LPCTSTR strAreaName, int areaType )
	{
		m_strAreaName	= strAreaName;
		m_areaType		= areaType;
	}

	~KChangeAreaInfo()
	{}

	void GetAreaName( ATL::CString& strAreaName )
	{
		strAreaName = m_strAreaName;
	}

	int GetAreaType()
	{
		return m_areaType;
	}

	virtual int Serialize(IKPopDataSerializer* _serializer)
	{
		if (!_serializer->IsLoading())
		{
			_serializer->BeginTag("KChangeAreaInfo");
			_serializer->AddPop("AreaName", m_strAreaName);
			_serializer->AddPop("AreaType", (int)m_areaType);
			_serializer->EndTag("KChangeAreaInfo");
		}
		return 0;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			_serializer >> m_strAreaName;
			_serializer >> m_areaType;
		}
		else
		{
			_serializer << m_strAreaName;
			_serializer << m_areaType;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSize = 0;
		nSize += TypeSize::get_size( m_strAreaName );
		nSize += sizeof( m_areaType );
		return nSize;
	}

private:

	ATL::CString	m_strAreaName;
	int				m_areaType;
};

//////////////////////////////////////////////////////////////////////////
class KPopUpdateInfo: public IKPopData
{
public:
	KPopUpdateInfo(DWORD nStatus ): m_nUpdateStatus(nStatus)
	{}

	~KPopUpdateInfo()
	{}

	virtual int Serialize(IKPopDataSerializer* _serializer)
	{
		if (!_serializer->IsLoading())
		{
			_serializer->BeginTag("KPopUpdateInfo");
			_serializer->AddPop("UpdateStatus", (int)m_nUpdateStatus);
			_serializer->EndTag("KPopUpdateInfo");
		}
		return 0;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			_serializer >> m_nUpdateStatus;
		}
		else
		{
			_serializer << m_nUpdateStatus;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		return sizeof(m_nUpdateStatus);
	}

private:
	DWORD			m_nUpdateStatus;
};
//////////////////////////////////////////////////////////////////////////
// 
#pragma pack(pop)