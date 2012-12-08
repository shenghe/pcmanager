#include "StdAfx.h"
#include "RequestPool.h"
#include "SegmentPool.h"

CRequestPool::~CRequestPool()
{
	Cleanup();
}

void CRequestPool::Cleanup()
{
	for(size_t i=0; i<m_items.size(); ++i)
	{
		m_items[i]->Close();
		delete m_items[i];
	}
	m_items.clear();
	m_items_pool.clear();
}

CHttpAsyncInPtr CRequestPool::Get()
{
	CHttpAsyncInPtr p = NULL;
	if(!m_items_pool.empty())
	{
		p = m_items_pool[ m_items_pool.size()-1 ];
		m_items_pool.pop_back();
		return p;
	}
	p = new CHttpAsyncIn;
	m_items.push_back(p);
	p->SetProxyConfig(m_currentProxyConfig);
	return p;
}

INT CRequestPool::Put( CHttpAsyncInPtr p )
{
	p->Close();
	m_items_pool.push_back(p);
	return m_items_pool.size();
}

BOOL CRequestPool::SetProxyConfig( CPubConfig::T_ProxySetting &proxyconfig )
{
	m_currentProxyConfig = proxyconfig;
	return TRUE;
}