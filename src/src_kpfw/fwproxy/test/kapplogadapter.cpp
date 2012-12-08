///////////////////////////////////////////////////////////////	
//	
// FileName  :  kapplogadapter.cpp
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-12-21  14:17
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#include "StdAfx.h"
#include "kapplogadapter.h"

KAppLogAdapter::KAppLogAdapter(	
	ULONG uAppModuleId, 
	ULONG nRequestProtocol, // PROTO_TYPE
	ULONG nRequestType,		// APP_REQUEST_TYPE
	ULONG uLocalIP,
	USHORT uLocalPort,
	ULONG uRemoteIP,
	USHORT uRemotePort,
	GUID  guidRule,
	ULONG uTrustResult, 
	ULONG uResult,
	ULONG uParentModuleId,
	ULONG uStackModuleId )
{
	m_appLog.m_uAppModuleId = uAppModuleId;
	m_appLog.m_uTrustResult = uTrustResult;		//可信认证结果
	m_appLog.m_nRequestProtocol = nRequestProtocol; // PROTO_TYPE
	m_appLog.m_nRequestType = nRequestType;		// APP_REQUEST_TYPE
	m_appLog.m_uLocalIP = uLocalIP;			//本地IP
	m_appLog.m_uLocalPort = uLocalPort;		//本地端口	
	m_appLog.m_uRemoteIP = uRemoteIP;		//目标IP
	m_appLog.m_uRemotePort = uRemotePort;		//目标端口	
	m_appLog.m_guidRule = guidRule;			//匹配规则GUID
	m_appLog.m_uResult = uResult;			//处理结果
	m_appLog.m_uParentModuleId = uParentModuleId;	//父进程路径
	m_appLog.m_uStackModuleId = uStackModuleId;	//堆栈模块路径
}

//////////////////////////////////////////////////////////////////////////

KAppLogAdapter::~KAppLogAdapter(void)
{
}

//////////////////////////////////////////////////////////////////////////

BOOL KAppLogAdapter::Do( ILogMgr *logMgr )
{
	BOOL bResult = FALSE;

	do 
	{
		if( logMgr == NULL )
			break;

		logMgr->AddRef();
		logMgr->AddLog( LOG_TYPE_APP, (BYTE*)&m_appLog, sizeof(m_appLog) );

	} while (FALSE);

	if( logMgr != NULL )
		logMgr->Release();

	return TRUE;
}
