////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : apprule.h
//      Version   : 1.0
//      Comment   : 网镖应用程序规则项
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////

#pragma once 

#include "kisrpc/kisrpcmanager.h"
#include "serializetools.h"
#include "kis/kpfw/netwatch.h"
#include "appfile.h"

#include <vector>
#include <map>
using namespace std;

//////////////////////////////////////////////////////////////////////////
class KAppRule: public KisEnableSerialize
{
public:
	AppRuleType				m_ruleType;
	GUID					m_id;
	ATL::CString			m_strExePath;
	BYTE					m_checkSum[16];
	FILETIME				m_lastFileTime;
	FILETIME				m_lastTrustRepTime;

	ATL::CString			m_strCreator;
	FILETIME				m_timeCreate;
	ATL::CString			m_strReason;
	FILETIME				m_timeLastModify;
	ATL::CString			m_strModifyer;
	vector<KPortRange>		m_tcpRemoteDenyRange;
	vector<KPortRange>		m_tcpLocalDenyRange;

	vector<KPortRange>		m_udpRemoteDenyRange;
	vector<KPortRange>		m_udpLocalDenyRange;

	INT						m_userMode;
	INT						m_trustMode;

	ULONG					m_pathhashCode;

	PVOID					m_pExternData;

public:
	KAppRule():m_ruleType(enumART_Unknown), 
			 m_userMode(0), m_trustMode(0), m_pExternData(NULL), m_pathhashCode(0)
	{
		ZeroStruct(m_id);
		ZeroStruct(m_timeCreate);
		ZeroStruct(m_timeLastModify);
		ZeroStruct(m_lastFileTime);
		ZeroStruct(m_lastTrustRepTime);
		ZeroMemory(m_checkSum, sizeof(m_checkSum));
		
		SYSTEMTIME sysTime;
		//CTime will auto convert to localtime
		GetSystemTime(&sysTime);
		SystemTimeToFileTime( &sysTime, &m_timeCreate );
		
	}

	KAppRule&	operator= (const KAppRule& rule)
	{
		m_ruleType			= rule.m_ruleType;
		m_id				= rule.m_id;
		m_strExePath		= rule.m_strExePath;
		memcpy(m_checkSum, rule.m_checkSum, sizeof(m_checkSum));
		m_lastFileTime		= rule.m_lastFileTime;
		m_lastTrustRepTime	= rule.m_lastTrustRepTime;
		m_strCreator		= rule.m_strCreator;;
		m_timeCreate		= rule.m_timeCreate;
		m_strReason			= rule.m_strReason;;
		m_timeLastModify	= rule.m_timeLastModify;
		m_strModifyer		= rule.m_strModifyer;
		m_tcpRemoteDenyRange= rule.m_tcpRemoteDenyRange;
		m_tcpLocalDenyRange = rule.m_tcpLocalDenyRange;
		m_udpRemoteDenyRange = rule.m_udpRemoteDenyRange;
		m_udpLocalDenyRange = rule.m_udpLocalDenyRange;

		m_userMode			= rule.m_userMode;
		m_trustMode			= rule.m_trustMode;
		m_pathhashCode		= rule.m_pathhashCode;
		return *this;
	}

	bool operator== (const KAppRule& rule)
	{
		if (memcmp(m_checkSum, rule.m_checkSum, sizeof(m_checkSum)) != 0)
			return false;

		if ( (m_lastFileTime.dwHighDateTime != rule.m_lastFileTime.dwHighDateTime) ||
			 (m_lastFileTime.dwLowDateTime != rule.m_lastFileTime.dwLowDateTime) )
			return false;

		if (m_tcpRemoteDenyRange != rule.m_tcpRemoteDenyRange)
			return false;
		if (m_tcpLocalDenyRange != rule.m_tcpLocalDenyRange)
			return false;
		if (m_udpRemoteDenyRange != rule.m_udpRemoteDenyRange)
			return false;
		if (m_udpLocalDenyRange != rule.m_udpLocalDenyRange)
			return false;

		if (m_userMode != rule.m_userMode)
			return false;
		if (m_trustMode != rule.m_trustMode)
			return false;

		if (m_pathhashCode != rule.m_pathhashCode)
			return false;

		return true;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			_serializer >> m_ruleType;
			_serializer >> m_id;
			_serializer >> m_strExePath;
			_serializer.read(m_checkSum, sizeof(BYTE), sizeof(m_checkSum));
			
			_serializer >> m_lastFileTime;
			_serializer >> m_lastTrustRepTime;

			_serializer >> m_strCreator;
			_serializer >> m_timeCreate;
			_serializer >> m_strReason;
			_serializer >> m_timeLastModify;
			_serializer >> m_strModifyer;
			_serializer >> m_tcpRemoteDenyRange;
			_serializer >> m_tcpLocalDenyRange;			
			_serializer >> m_udpRemoteDenyRange;
			_serializer >> m_udpLocalDenyRange;

			_serializer >> m_userMode;
			_serializer >> m_trustMode;

			_serializer >> m_pathhashCode;
		}
		else
		{
			_serializer << m_ruleType;
			_serializer << m_id;
			_serializer << m_strExePath;
			_serializer.write(m_checkSum, sizeof(BYTE), sizeof(m_checkSum));

			_serializer << m_lastFileTime;
			_serializer << m_lastTrustRepTime;

			_serializer << m_strCreator;
			_serializer << m_timeCreate;
			_serializer << m_strReason;
			_serializer << m_timeLastModify;
			_serializer << m_strModifyer;
			_serializer << m_tcpRemoteDenyRange;
			_serializer << m_tcpLocalDenyRange;
			_serializer << m_udpRemoteDenyRange;
			_serializer << m_udpLocalDenyRange;

			_serializer << m_userMode;
			_serializer << m_trustMode;

			_serializer << m_pathhashCode;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t size = 0;
		size += sizeof(m_ruleType);
		size += sizeof(m_id);
		size += TypeSize::get_size(m_strExePath);
		size += sizeof(m_checkSum);
		size += sizeof(m_lastFileTime);
		size += sizeof(m_lastTrustRepTime);

		size += TypeSize::get_size(m_strCreator);
		size += sizeof(m_timeCreate);
		size += TypeSize::get_size(m_strReason);
		size += sizeof(m_timeLastModify);
		size += TypeSize::get_size(m_strModifyer);
		size += TypeSize::get_size(m_tcpRemoteDenyRange);
		size += TypeSize::get_size(m_tcpLocalDenyRange);
		size += TypeSize::get_size(m_udpRemoteDenyRange);
		size += TypeSize::get_size(m_udpLocalDenyRange);

		size += sizeof(m_userMode);
		size += sizeof(m_trustMode);
		size += sizeof(m_pathhashCode);
		return size;
	}

	BOOL		IsSame(const KAppRule* rule)
	{
		return m_id == rule->m_id;
	}

	void		SetID(GUID& id)
	{
		m_id = id;
	}
	GUID*		GetID()
	{
		return &m_id;
	}

	BOOL		SetExePath(LPWSTR strExePaht)
	{
		//修改bug 路径相同大小不同
		//导致写入日志过多
		if (m_strExePath.CompareNoCase( strExePaht ) != 0 )
		{
			m_strExePath = strExePaht;
			m_pathhashCode = GetStringHashWZ(strExePaht);
			return TRUE;
		}
		return FALSE;
	}
	LPCWSTR		GetExePath()
	{
		return m_strExePath;
	}

	BYTE*		GetCheckSum()
	{
		return m_checkSum;
	}

	BYTE*		GetCheckSum(INT& nLen)
	{
		nLen = 16;
		return m_checkSum;
	}
	BOOL		SetCheckSum(BYTE* pBuf, INT nLen)
	{
		ASSERT(nLen > 0 && nLen <= 16);
		if (nLen > 16) nLen = 16;
		if (memcmp(pBuf, m_checkSum, nLen) != 0)
		{
			memcpy(m_checkSum, pBuf, nLen);
			return TRUE;
		}
		return FALSE;
	}

	FILETIME*	GetLastFileTime()
	{
		return &m_lastFileTime;
	}

	void		SetLastFileTime(FILETIME& time)
	{
		m_lastFileTime = time;
	}

	FILETIME*	GetLastTrustRepTime()
	{
		return &m_lastTrustRepTime;
	}

	void		SetLastTrustRepTime(FILETIME& time)
	{
		m_lastTrustRepTime = time;
	}

	void		SetCreator(LPWSTR strCreator)
	{
		m_strCreator = strCreator;
	}
	LPCWSTR		GetCreator()
	{
		return m_strCreator;
	}

	void		SetCreateDate(FILETIME& Time)
	{
		m_timeCreate = Time;
	}
	FILETIME*	GetCreateDate()
	{
		return &m_timeCreate;
	}

	void		SetCreateReason(LPWSTR strReason)
	{
		m_strReason = strReason;
	}
	LPCWSTR		GetCreateReason()
	{
		return m_strReason;
	}

	void		SetLastModifyDate(FILETIME& Time)
	{
		m_timeLastModify = Time;
	}
	FILETIME*	GetLastModifyDate()
	{
		return &m_timeLastModify;
	}

	void		SetLastModifyer(LPWSTR strName)
	{
		m_strModifyer = strName;
	}
	LPCWSTR		GetLastModifyer()
	{
		return m_strModifyer;
	}

	AppRuleType GetType()
	{
		return m_ruleType;
	}
	void		SetType(AppRuleType emType)
	{
		m_ruleType = emType;
	}

	INT			GetUserMode()
	{
		return m_userMode;
	}
	BOOL		SetUserMode(INT nMode)
	{
		if (m_userMode != nMode)
		{
			m_userMode = nMode;
			return TRUE;
		}
		return FALSE;
	}

	INT			GetTrustMode()
	{
		return m_trustMode;
	}
	BOOL		SetTrustMode(INT nMode)
	{
		if (m_trustMode != nMode)
		{
			m_trustMode = nMode;
			return TRUE;
		}
		return FALSE;
	}

	ULONG	GetHashCode()
	{
		return m_pathhashCode;
	}

	void SetExternData(PVOID pData)
	{
		m_pExternData = pData;
	}

	PVOID GetExternData()
	{
		return m_pExternData;
	}

	BOOL IsDenyPort(vector<KPortRange>& range, KPort port)
	{
		for (int i = 0; i < (INT)range.size(); i++)
		{
			if (port >= range[i].nStart && port <= range[i].nEnd)
				return TRUE;
		}
		return FALSE;
	}

	ULONG GetStringHashWZ( WCHAR *wsSrc )
	{
		static UCHAR gsToUpperTable[256 + 4] =
		{
			0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
			16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
			32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
			48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
			64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
			80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
			96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
			80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,123,124,125,126,127,
			128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
			144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
			160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
			176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
			192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
			208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
			224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
			240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
			0, 0, 0, 0
		};


		ULONG ulLoopCount = 0;
		ULONG ulResult = 0;

		if ( wsSrc == NULL)
			return 0;

		while( wsSrc[ulLoopCount] )
		{
			ulResult = (ulResult * 31) +  gsToUpperTable[wsSrc[ulLoopCount] >> 8];
			ulResult = (ulResult * 31) +  gsToUpperTable[wsSrc[ulLoopCount] & 0x00ff];
			ulLoopCount++;
		}

		return ulResult;
	}
private:
	BOOL		IntersectRange(KPortRange& p1, KPortRange& p2)
	{
		if (p1.nEnd < p2.nStart)
			return FALSE;
		if (p1.nStart > p2.nEnd)
			return FALSE;
		if (p1.nStart > p2.nStart)
			p1.nStart = p2.nStart;
		if (p1.nEnd < p2.nEnd)
			p1.nEnd = p2.nEnd;
		return TRUE;
	}
};

//////////////////////////////////////////////////////////////////////////
class KAppRuleVec : public KisEnableSerialize
{
private:
	vector< KAppRule* >				m_AppRules;
	map<ULONG, KAppRule*>			m_MapRules;

public:
	KAppRuleVec()		{}

	~KAppRuleVec()
	{
		Clear();
	}

	KAppRuleVec& operator= (KAppRuleVec& rules)
	{
		Clear();
		for (int i = 0; i < (INT)rules.m_AppRules.size(); i++)
		{
			KAppRule* pRule = new KAppRule;
			KAppRule* pRuleOld = rules.m_AppRules[i];
			*pRule = *pRuleOld;
			m_AppRules.push_back(pRule);
			ASSERT(m_MapRules.find(pRule->GetHashCode()) == m_MapRules.end());
			m_MapRules.insert(make_pair(pRule->GetHashCode(), pRule));
		}
		return *this;
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
				KAppRule* pRule = new KAppRule;
				_serializer >> pRule;
				m_AppRules.push_back(pRule);
				ASSERT(m_MapRules.find(pRule->GetHashCode()) == m_MapRules.end());
				m_MapRules.insert(make_pair(pRule->GetHashCode(), pRule));
			}
		}
		else
		{
			int nSize = (INT)m_AppRules.size();
			_serializer << nSize;
			for (int i = 0; i < (INT)m_AppRules.size(); i++)
			{
				KAppRule* pRule = m_AppRules[i];
				_serializer << pRule;
			}
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t size = sizeof(int);
		for (int i  = 0 ; i < (INT)m_AppRules.size(); i++)
			size += m_AppRules[i]->GetSeriSize();
		return size;
	}

	KAppRule* CreateRule()
	{
		return new KAppRule;
	}
	void	DeleteRule(KAppRule* pRule)
	{
		delete pRule;
	}

	HRESULT	AddRule(KAppRule* pRule)
	{
		m_AppRules.push_back(pRule);
		ASSERT(m_MapRules.find(pRule->GetHashCode()) == m_MapRules.end());
		m_MapRules.insert(make_pair(pRule->GetHashCode(), pRule));
		return S_OK;
	}

	KAppRule* RemoveRule(KAppRule* pRule)
	{
		vector< KAppRule* >::iterator it = m_AppRules.begin();
		for(; it != m_AppRules.end(); it++)
		{
			if ((*it)->IsSame(pRule))
			{
				KAppRule* pRule = *it;
				m_MapRules.erase(pRule->GetHashCode());
				m_AppRules.erase(it);
				return pRule;
			}
		}
		return NULL;
	}

	HRESULT UpdateRule(KAppRule* pRule)
	{
		vector< KAppRule* >::iterator it = m_AppRules.begin();
		for(; it != m_AppRules.end(); it++)
		{
			KAppRule* pOldRule = *it;
			if (pOldRule->IsSame(pRule))
			{
				if (!(*pOldRule == *pRule))
				{
					*pOldRule = *pRule;
					return S_OK;
				}
			}
		}
		return E_FAIL;
	}

	INT	GetRuleCnt()
	{
		return (INT)m_AppRules.size();
	}

	KAppRule* GetRule(INT nIndex)
	{
		ASSERT(nIndex >= 0 && nIndex < (INT)m_AppRules.size());
		return m_AppRules[nIndex];
	}

	KAppRule* FindRule(LPCWSTR pszName)
	{		
		for (int i = 0; i < (INT)m_AppRules.size(); i++)
		{
			KAppRule* pRule = m_AppRules[i];
			if (_wcsicmp(pRule->GetExePath(), pszName) == 0)
				return pRule;
		}
		return NULL;
	}

	KAppRule* FindRule(ULONG nModuleID, LPCWSTR pszName)
	{
		if (m_MapRules.find(nModuleID) != m_MapRules.end())
			return m_MapRules[nModuleID];
		return NULL;
	}

	HRESULT	Load(LPCWSTR strFile)
	{
		KAppFile file(this);
		return file.Load(strFile);
	}

	HRESULT	Save(LPCWSTR strFile)
	{
		KAppFile file(this);
		return file.Save(strFile);
	}

	template<class T>
	BOOL RemoveNotExist(T& action)
	{
		BOOL bModify = FALSE;
		if (!m_AppRules.empty())
		{
			vector<KAppRule*>::iterator it = m_AppRules.end();
			do 
			{
				it--;
				BOOL bBegin = (it == m_AppRules.begin());
				KAppRule* pItem = *it;
				if (action(pItem))
				{
					vector<KAppRule*>::iterator itErase = it;
					if (!bBegin)
					{
						it--;
						m_MapRules.erase((*itErase)->GetHashCode());
						m_AppRules.erase(itErase);
						it++;
					}
					else
					{
						m_MapRules.erase((*itErase)->GetHashCode());
						m_AppRules.erase(itErase);
					}
					bModify = TRUE;
					delete pItem;
				}
				if (bBegin)
					break;
			}
			while (TRUE);
		}
		return bModify;
	}

	KAppRule* FindItem(const KAppRule* pRule)
	{
		for (int i = 0; i < (INT)m_AppRules.size(); i++)
		{
			if (m_AppRules[i]->IsSame(pRule))
				return m_AppRules[i];
		}
		return NULL;
	}

	void Clear()
	{
		for (int i = 0; i < (INT)m_AppRules.size(); i++)
			delete m_AppRules[i];
		m_AppRules.clear();
		m_MapRules.clear();
	}
};

#include "appfile.hpp"