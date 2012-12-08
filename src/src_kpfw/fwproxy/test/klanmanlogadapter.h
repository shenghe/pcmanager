///////////////////////////////////////////////////////////////	
//	
// FileName  :  klanmanlogadapter.h
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-12-21  17:35
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#pragma once

#include "logmgr_public.h"

class KLanmanLogAdapter
{
public:
	KLanmanLogAdapter( 
		ULONG uAppModuleId, 
		WCHAR *wsLanmanPath,
		ULONG uTrustResult, 
		ULONG uResult,
		ULONG uParentModuleId,
		ULONG uStackModuleId
		);
	~KLanmanLogAdapter(void);

	BOOL Do( ILogMgr *logMgr );

private:
	LANMAN_LOG m_lanmanLog;
};
