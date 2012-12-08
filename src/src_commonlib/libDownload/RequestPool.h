#pragma once
#include "Download.h"

class CRequestPool
{
public:
	~CRequestPool();
	void Cleanup();
	CHttpAsyncInPtr Get();
	INT Put(CHttpAsyncInPtr p);
	BOOL SetProxyConfig(CPubConfig::T_ProxySetting &proxyconfig);

private:
	CHttpAsyncInPtrs m_items, m_items_pool;
	CPubConfig::T_ProxySetting m_currentProxyConfig;
};
