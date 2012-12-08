///////////////////////////////////////////////////////////////	
//	
// FileName  :  kapplogadapter.h
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-12-22  14:27
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#pragma once

#include "logmgr_public.h"

class KAppLogAdapter
{
public:
	KAppLogAdapter( 
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
		ULONG uStackModuleId
	);

	~KAppLogAdapter(void);
	BOOL Do( ILogMgr *logMgr );

private:
	APP_LOG m_appLog;
};
