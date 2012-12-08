////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : iprule.h
//      Version   : 1.0
//      Comment   : ÍøïÚip¹æÔòÏî
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      Modify by : cheguicheng 2008-10-09
///////////////////////////////////////////////////////////////////////////////

#pragma once 
#include "kisrpc/kisrpcmanager.h"
#include "serializetools.h"
#include "kis/kpfw/netwatch.h"
#include "ipfile.h"

class KIpRule: public KisEnableSerialize
{
private:
	KIP_FILTER_INFO		m_ipInfo;
	ATL::CString		m_strName;
	ATL::CString		m_strDesc;
	ATL::CString		m_strCreator;
	FILETIME			m_timeCreate;
	ATL::CString		m_strReason;
	FILETIME			m_timeModify;
	ATL::CString		m_strModifyer;
	BOOL				m_bValid;
	PVOID				m_pExternData;
	BOOL				m_bModify;

public:
	KIpRule(): m_bValid(TRUE), 
				m_pExternData(NULL),
				m_bModify(FALSE)
	{
		ZeroStruct(m_ipInfo);
		ZeroStruct(m_timeCreate);
		ZeroStruct(m_timeModify);
		m_ipInfo.filter.cbSize = sizeof(FILTER_INFO);
		m_ipInfo.filter.nVer = 0;
		m_ipInfo.filter.Direction = enumPD_Both;
		::CoCreateGuid( &(m_ipInfo.id) );
	}

	KIpRule&	operator= (const KIpRule& rule)
	{
		m_ipInfo = rule.m_ipInfo;
		m_strName = rule.m_strName;
		m_strDesc = rule.m_strDesc;
		m_strCreator = rule.m_strCreator;
		m_timeCreate = rule.m_timeCreate;
		m_strReason = rule.m_strReason;
		m_timeModify = rule.m_timeModify;
		m_strModifyer = rule.m_strModifyer;
		m_bValid = rule.m_bValid;
		return *this;
	}

	bool EqualMisc(const KIpRule& rule)
	{
		if ( m_bValid != rule.m_bValid )
			return false;

		if ( m_strModifyer != rule.m_strModifyer )
			return false;

		if ( ::CompareFileTime( &m_timeModify ,&( rule.m_timeModify) ) != 0 )
			return false;

		if ( m_strReason != rule.m_strReason)
			return false;

		if( ::CompareFileTime( &m_timeCreate , &(rule.m_timeCreate) ) != 0 )
			return false;

		if ( m_strCreator != rule.m_strCreator )
			return false;

		if ( m_strDesc != rule.m_strDesc)
			return false;

		if ( m_strName != rule.m_strName )
			return false;

		return true;
	}

	bool EqualFilter(const KIpRule& rule)
	{
		if ( ::memcmp( &(m_ipInfo.filter), &(rule.m_ipInfo.filter), sizeof(FILTER_INFO) ) != 0 ) 
			return false;
		return true;
	}

	bool EqualId(const KIpRule& rule)
	{
		if (!IsEqualGUID( m_ipInfo.id, rule.m_ipInfo.id ))
			return false;
		return true;
	}

	bool operator== (const KIpRule& rule )
	{
		if (!EqualId(rule))
			return false;

		if (!EqualFilter(rule))
			return false;

		if (!EqualMisc(rule))
			return false;

		return true;
	}

	void SetModify( BOOL bModify )
	{
		m_bModify = bModify;
	}

	BOOL GetModify( )
	{
		return m_bModify;
	}

	void		SetID(GUID& id)
	{ 
		m_ipInfo.id = id; 
	}

	GUID*		GetID()
	{
		return &(m_ipInfo.id);
	}

	void		SetName(LPCWSTR strName)	{
		m_strName = strName;
	}
	void		SetName(ATL::CString& strName)	{
		m_strName = strName;
	}
	LPCWSTR		GetName()			{
		return m_strName;
	}

	void		SetDesc(LPCWSTR strDesc)	{
		m_strDesc = strDesc;
	}
	void		SetDesc(ATL::CString& strDesc)	{
		m_strDesc = strDesc;
	}
	LPCWSTR		GetDesc()			{
		return m_strDesc;
	}

	void		SetCreator(LPCWSTR strCreator)	{
		m_strCreator = strCreator;
	}
	void		SetCreator(ATL::CString& strCreator)	{
		m_strCreator = strCreator;
	}
	LPCWSTR		GetCreator()		{
		return m_strCreator;
	}

	void		SetCreateDate(FILETIME& Time)	{
		m_timeCreate = Time;
	}
	FILETIME*	GetCreateDate()		{
		return &m_timeCreate;
	}

	void		SetCreateReason(LPCWSTR strReason) {
		m_strReason = strReason;
	}
	void		SetCreateReason(ATL::CString& strReason) {
		m_strReason = strReason;
	}
	LPCWSTR		GetCreateReason() {
		return m_strReason;
	}

	void		SetLastModifyDate(FILETIME& Time) {
		m_timeModify = Time;
	}
	FILETIME*	GetLastModifyDate() {
		return &m_timeModify;
	}

	void		SetLastModifyer(LPCWSTR strName) {
		m_strModifyer = strName;
	}
	void		SetLastModifyer(ATL::CString& strName) {
		m_strModifyer = strName;
	}
	LPCWSTR		GetLastModifyer() {
		return m_strModifyer;
	}

	void		SetFilterInfo(PKIP_FILTER_INFO pInfo)
	{
		m_ipInfo = *pInfo;
	}

	PKIP_FILTER_INFO GetFilterInfo()
	{
		return &m_ipInfo;
	}

	BOOL		IsSame(KIpRule* pInfo)
	{
		if ( m_ipInfo.id == GUID_NULL && pInfo->m_ipInfo.id == GUID_NULL )
			return false;

		return IsEqualGUID(m_ipInfo.id, pInfo->m_ipInfo.id);
	}

	BOOL		GetValid()
	{
		return m_bValid;
	}

	BOOL		SetValid(BOOL b)
	{
		BOOL blast = m_bValid;
		m_bValid = b;
		return blast;
	}

	PVOID		GetExternData()
	{
		return m_pExternData;
	}

	BOOL		SetExternData(PVOID pData)
	{
		m_pExternData = pData;
		return TRUE;
	}

	bin_archive& Serialize(bin_archive &_serializer)
	{
		if(_serializer.is_loading())
		{
			_serializer >> m_ipInfo;
			_serializer >> m_strName;
			_serializer >> m_strDesc;
			_serializer >> m_strCreator;
			_serializer >> m_timeCreate;
			_serializer >> m_strReason;
			_serializer >> m_timeModify;
			_serializer >> m_strModifyer;
			_serializer >> m_bValid;
		}
		else
		{
			_serializer << m_ipInfo;
			_serializer << m_strName;
			_serializer << m_strDesc;
			_serializer << m_strCreator;
			_serializer << m_timeCreate;
			_serializer << m_strReason;
			_serializer << m_timeModify;
			_serializer << m_strModifyer;
			_serializer << m_bValid;
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t size = 0;
		size += sizeof(m_ipInfo);
		size += TypeSize::get_size(m_strName);
		size += TypeSize::get_size(m_strDesc);
		size += TypeSize::get_size(m_strCreator);
		size += sizeof(m_timeCreate);
		size += TypeSize::get_size(m_strReason);
		size += sizeof(m_timeModify);
		size += TypeSize::get_size(m_strModifyer);
		size += sizeof(m_bValid);
		return size;
	}
};

class KIpRuleVec: public KisEnableSerialize
{
private:
	vector< KIpRule * >			m_Rules;
	int							m_nTheSameRuleCnt ;

public:

	

	KIpRuleVec()
	{
		
	}

	~KIpRuleVec()
	{
		Clear();
	}

	KIpRuleVec&	operator= (const KIpRuleVec& rules)
	{
		Clear();
		for (int i = 0; i < (INT)rules.m_Rules.size(); i++)
		{
			KIpRule* pRule = new KIpRule;
			*pRule = *rules.m_Rules[i];
			m_Rules.push_back(pRule);
		}
		return *this;
	}

	KIpRuleVec&	operator= (const vector< KIpRule * >& rules)
	{
		Clear();
		for (int i = 0; i < (INT)rules.size(); i++)
		{
			KIpRule* pRule = new KIpRule;
			*pRule = *rules[i];
			m_Rules.push_back(pRule);
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
				KIpRule* pIpRule = new KIpRule;
				_serializer >> pIpRule;
				m_Rules.push_back(pIpRule);
			}
		}
		else
		{
			int nSize = (INT)m_Rules.size();
			_serializer << nSize;
			for (int i = 0; i < (INT)m_Rules.size(); i++)
				_serializer << m_Rules[i];
		}
		return _serializer;
	}

	size_t GetSeriSize()
	{
		size_t size = sizeof(int);
		for (int i  = 0 ; i < (INT)m_Rules.size(); i++)
			size += m_Rules[i]->GetSeriSize();
		return size;
	}

	BOOL	HasSameFilter(KIpRule* pRule)
	{
		for (int i = 0; i < (INT)m_Rules.size(); i++)
		{
			if (pRule->EqualFilter(*m_Rules[i]))
				return TRUE;
		}
		return FALSE;
	}

	BOOL	HasSameId(KIpRule* pRule)
	{
		for (int i = 0; i < (INT)m_Rules.size(); i++)
		{
			if (pRule->EqualId(*m_Rules[i]))
				return TRUE;
		}
		return FALSE;
	}

	INT				GetRuleCnt()
	{
		return (INT)m_Rules.size();
	}

	KIpRule*		GetRule(INT nIndex)
	{
		ASSERT(nIndex >= 0 && nIndex < (INT)m_Rules.size());
		return m_Rules[nIndex];
	}

	KIpRule*		CreateRule()
	{
		return new KIpRule;
	}

	HRESULT			AddRule(KIpRule* pRule)
	{
		int nSize = (INT)m_Rules.size();
		int i = 0;
		for ( i = 0; i < nSize; i ++ )
		{
			if ( IsEqualGUID( *(pRule->GetID()), *(m_Rules[i]->GetID()) ))
				break;
		}

		if ( i >= nSize )
			m_Rules.push_back(pRule);
		else
			m_nTheSameRuleCnt ++ ;
		
		return S_OK;
	}

	void	RemoveRule(KIpRule* pRule)
	{
		vector< KIpRule * >::iterator it = m_Rules.begin();
		for (; it != m_Rules.end(); it++)
		{
			if ((*it)->IsSame(pRule))
			{
				KIpRule* pRule = *it;
				delete pRule;
				m_Rules.erase(it);
				break;
			}
		}
	}

	void	RemoveRule(INT nIndex)
	{
		ASSERT(nIndex >= 0 && nIndex < (int)m_Rules.size());
		KIpRule* pRule = m_Rules[nIndex];
		delete pRule;
		m_Rules.erase(m_Rules.begin() + nIndex);
	}

	KIpRule* MoveDown(KIpRule* pRule)
	{
		for (int i = 0; i < (INT)m_Rules.size()-1; i++)
		{
			if (m_Rules[i]->IsSame(pRule))
				return MoveDown(i);
		}
		return NULL;
	}

	KIpRule* MoveDown(INT i)
	{
		ASSERT(i >= 0 && i < (int)m_Rules.size());
		if ((m_Rules.size() == 1) || (i == m_Rules.size() - 1))
			return NULL;
		KIpRule* pTmp = m_Rules[i + 1];
		m_Rules[i+1] = m_Rules[i];
		m_Rules[i] = pTmp;
		return pTmp;
	}

	KIpRule* MoveUp(KIpRule* pRule)
	{
		for (int i = 1; i < (INT)m_Rules.size(); i++)
		{
			if (m_Rules[i]->IsSame(pRule))
				return MoveUp(i);
		}
		return NULL;
	}

	KIpRule* MoveUp(INT i)
	{
		ASSERT(i >= 0 && i < (int)m_Rules.size());
		if ((m_Rules.size() == 1) || (i == 0))
			return NULL;
		KIpRule* pTmp = m_Rules[i-1];
		m_Rules[i-1] = m_Rules[i];
		m_Rules[i] = pTmp;
		return pTmp;
	}

	HRESULT			MoveTop(INT i)
	{
		ASSERT(i >= 0 && i < (int)m_Rules.size());
		if ((m_Rules.size() == 1) || (i == 0))
			return S_OK;
		KIpRule* pTmp = m_Rules[0];
		m_Rules[0] = m_Rules[i];
		m_Rules[i] = pTmp;
		return S_OK;
	}

	HRESULT			MoveBottom(INT i)
	{
		ASSERT(i >= 0 && i < (int)m_Rules.size());
		if ((m_Rules.size() == 1) || (i == m_Rules.size() - 1))
			return S_OK;
		KIpRule* pTmp = m_Rules[m_Rules.size() - 1];
		m_Rules[m_Rules.size() - 1] = m_Rules[i];
		m_Rules[i] = pTmp;
		return S_OK;
	}

	void Clear()
	{
		for (int i = 0; i < (INT)m_Rules.size(); i++)
			delete m_Rules[i];
		m_Rules.clear();
	}

	HRESULT			Load(LPWSTR strFile)
	{
		KIpFile file(this);
		if (FAILED(file.Load(strFile)))
			return E_FAIL;
		return S_OK;
	}

	HRESULT			Save(LPWSTR strFile)
	{
		KIpFile file(this);
		if (FAILED(file.Save(strFile)))
			return E_FAIL;
		return S_OK;
	}

	void SetTheSameCnt( int nCnt )
	{
		m_nTheSameRuleCnt = nCnt;
	}

	int GetTheSameCnt()
	{
		return m_nTheSameRuleCnt;
	}
};


#include "ipfile.hpp"