////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : area.h
//      Version   : 1.0
//      Comment   : 网镖区域信息
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
///////////////////////////////////////////////////////////////////////////////

#pragma once 

#include "kisrpc/kisrpcmanager.h"
#include "serializetools.h"
#include "kis/kpfw/netwatch.h"

#include "areafile.h"

typedef enum enumAreaType
{
	enumAT_Unset		= 0,						// 没有设置，在找到区域后用户在弹出泡泡上面选择之前
	enumAT_Trust		= 1 << 16,
	enumAT_UnTrust		= 2 << 16,
	enumAT_Unknown		= 3 << 16,
}AreaType;

typedef enum enumAreaSubType
{
	enumAST_Maxk			= 0x0000ffff,

	enumAST_Trust_Home		= 1,
	enumAST_Trust_Office		= 2,
	enumAST_Trust_Default	= 3,
}AreaSubType;


class KArea: public KisEnableSerialize
{
	ATL::CString		m_strName;					// 区域名
	INT					m_AreaType;					// 区域类型
	BYTE				m_GateMac[6];				// 网关mac地址
	FILETIME			m_createDate;				// 创建时间
	UINT				m_uIp;
	BOOL				m_bWireless;				// 是否是无线网络
	DWORD				m_nLastChangeTime;

public:
	KArea(): m_AreaType(enumAT_Unset), m_bWireless(FALSE), m_nLastChangeTime(0)
	{
		ZeroMemory(m_GateMac, sizeof(m_GateMac));
		ZeroStruct(m_createDate);
		m_uIp = 0;
	}

	void				SetName(LPCWSTR str)		{ m_strName = str; }
	LPCWSTR				GetName()				{ return m_strName; }

	void				SetType(INT t)			{ m_AreaType = t; }
	INT					GetType()				{ return m_AreaType; }

	void				SetGateIp( UINT ip )	{ m_uIp = ip; }
	UINT				GetGateIp()				{ return m_uIp; }

	void				SetGateMac(BYTE* addr)  { memcpy(m_GateMac, addr, sizeof(m_GateMac)); }
	void				GetGateMac(BYTE* addr)	{ memcpy(addr, m_GateMac, sizeof(m_GateMac)); }

	void				SetCreateTime(FILETIME* pTime) {m_createDate = *pTime;}
	FILETIME*			GetCreateTime()			{return &m_createDate;}

	void				SetWireless(BOOL b)		{m_bWireless = b;}
	BOOL				GetWireless()			{return m_bWireless;}

	BOOL				IsSame(BYTE* mac)		{ return memcmp(mac, m_GateMac, sizeof(m_GateMac)) == 0; }
	BOOL				IsSame(const KArea* area)	{ return memcmp(area->m_GateMac, m_GateMac, sizeof(m_GateMac)) == 0;  }

	void				SetLastChangeTime(DWORD nTime)	{m_nLastChangeTime = nTime;}
	DWORD				GetLastChangeTime()		{return m_nLastChangeTime;}

	KArea&	operator= (const KArea& rule)
	{
		m_strName = rule.m_strName;
		m_AreaType = rule.m_AreaType;
		m_uIp = rule.m_uIp;
		memcpy(m_GateMac, rule.m_GateMac, sizeof(m_GateMac));
		m_createDate = rule.m_createDate;
		m_bWireless = rule.m_bWireless;
		return *this;
	}
	bool operator== (const KArea& rule)
	{
		if (m_strName != rule.m_strName)
			return false;
		if (m_AreaType != rule.m_AreaType)
			return false;
		if ( m_uIp != rule.m_uIp )
			return false;
		if (m_bWireless != rule.m_bWireless)
			return false;
		if (memcmp(m_GateMac, rule.m_GateMac, sizeof(m_GateMac)))
			return false;
		return true;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			_serializer >> m_strName;
			int Type = 0;
			_serializer >> Type;
			m_AreaType = Type;
			UINT uIp = 0;
			_serializer >> uIp;
			m_uIp = uIp;
			_serializer >> m_bWireless;

			_serializer.read(m_GateMac, sizeof(BYTE), 6);
			_serializer.read(&m_createDate, sizeof(FILETIME), 1);
		}
		else
		{
			_serializer << m_strName;
			int Type = m_AreaType;
			_serializer << Type;
			int nIp = m_uIp;
			_serializer << nIp;
			_serializer << m_bWireless;

			_serializer.write(m_GateMac, sizeof(BYTE), 6);
			_serializer.write(&m_createDate, sizeof(FILETIME), 1);
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t nSize = 0;
		nSize += TypeSize::get_size(m_strName);
		nSize += sizeof(INT);
		nSize += sizeof(m_GateMac);
		nSize += sizeof(m_createDate);
		nSize += sizeof(m_uIp);
		nSize += sizeof(BOOL);
		return nSize;
	}
};

//////////////////////////////////////////////////////////////////////////
class KAreaVec : public KisEnableSerialize
{
private:
	vector< KArea* >				m_Areas;
	BOOL							m_Own;
public:
	KAreaVec(BOOL bOwn = TRUE):m_Own(bOwn)		{}

	~KAreaVec()
	{
		Clear();
	}

	void SetOwn( BOOL bOwn )
	{
		m_Own = bOwn;
	}

	BOOL GetOwn()
	{
		return m_Own;
	}

	KAreaVec&	operator= (const KAreaVec& rules)
	{
		Clear();
		for (int i = 0; i < (INT)rules.m_Areas.size(); i++)
		{
			if (m_Own)
			{
				KArea* pRule = new KArea;
				*pRule = *rules.m_Areas[i];
				m_Areas.push_back(pRule);
			}
			else
			{
				m_Areas.push_back(rules.m_Areas[i]);
			}
		}
		return *this;
	}

	bool IsEqual(KAreaVec& rules) 
	{
		for (int i = 0; i < (INT)m_Areas.size(); i++)
		{
			if (!rules.GetSameArea(m_Areas[i]))
				return false;
		}
		return true;
	}

	bool operator== (KAreaVec& rules)
	{
		if (m_Areas.size() != (INT)rules.m_Areas.size())
			return false;
		if (!IsEqual(rules))
			return false;
		if (!rules.IsEqual(*this))
			return false;
		return true;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			ASSERT(m_Own);
			if (m_Own)
			{
				Clear();
				INT nCnt = 0;
				_serializer >> nCnt;
				for (int i = 0; i < nCnt; i++)
				{
					KArea* pRule = new KArea;
					_serializer >> pRule;
					m_Areas.push_back(pRule);
				}
			}
		}
		else
		{
			int nSize = (INT)m_Areas.size();
			_serializer << nSize;
			for (int i = 0; i < (INT)m_Areas.size(); i++)
			{
				KArea* pRule = m_Areas[i];
				_serializer << pRule;
			}
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t size = sizeof(int);
		for (int i  = 0 ; i < (INT)m_Areas.size(); i++)
			size += m_Areas[i]->GetSeriSize();
		return size;
	}

	KArea* CreateArea()
	{
		ASSERT(m_Own);
		if (m_Own)
			return new KArea;
		return NULL;
	}

	void DestroyArea(KArea* pArea)
	{
		ASSERT(m_Own);
		delete pArea;
	}

	HRESULT	AddArea(KArea* pRule)
	{
		m_Areas.push_back(pRule);
		return S_OK;
	}

	KArea* RemoveArea(KArea* pRule)
	{
		vector< KArea* >::iterator it = m_Areas.begin();
		for(; it != m_Areas.end(); it++)
		{
			if ((*it)->IsSame(pRule))
			{
				KArea* pArea = *it;
				m_Areas.erase(it);
				return pArea;
			}
		}
		return NULL;
	}

	HRESULT UpdateArea(KArea* pRule)
	{
		vector< KArea* >::iterator it = m_Areas.begin();
		for(; it != m_Areas.end(); it++)
		{
			KArea* pOldRule = *it;
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

	KArea* GetSameAreaMac( BYTE* pGateMac )
	{
		vector< KArea* >::iterator it = m_Areas.begin();
		for(; it != m_Areas.end(); it++)
		{
			KArea* pOldRule = *it;
			if (pOldRule->IsSame(pGateMac))
			{
				return pOldRule;
			}
		}
		return NULL;
	}

	KArea* GetSameAreaMac( KArea* pRule )
	{
		vector< KArea* >::iterator it = m_Areas.begin();
		for(; it != m_Areas.end(); it++)
		{
			KArea* pOldRule = *it;
			if (pOldRule->IsSame(pRule))
			{
				return pOldRule;
			}
		}
		return NULL;
	}


	KArea* GetSameArea( KArea* pRule )
	{
		vector< KArea* >::iterator it = m_Areas.begin();
		for(; it != m_Areas.end(); it++)
		{
			KArea* pOldRule = *it;
			if (pOldRule->IsSame(pRule))
			{
				if (*pOldRule == *pRule)
				{
					return pOldRule;
				}
			}
		}
		return NULL;
	}

	INT	GetAreaCnt()
	{
		return (INT)m_Areas.size();
	}

	KArea* GetArea(INT nIndex)
	{
		ASSERT(nIndex >= 0 && nIndex < (int)m_Areas.size());
		return m_Areas[nIndex];
	}

	HRESULT	Load(LPCWSTR strFile)
	{
		ASSERT(m_Own);
		if (m_Own)
		{
			KAreaFile file(this);
			return file.Load(strFile);
		}
		return E_FAIL;
	}

	HRESULT	Save(LPCWSTR strFile)
	{
		ASSERT(m_Own);
		if (m_Own)
		{
			KAreaFile file(this);
			return file.Save(strFile);
		}
		return E_FAIL;
	}


	void Clear()
	{
		if (m_Own)
		{
			for (int i = 0; i < (INT)m_Areas.size(); i++)
				delete m_Areas[i];
		}
		m_Areas.clear();
	}
};

#include "areafile.hpp"