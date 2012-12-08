///////////////////////////////////////////////////////////////	
//	
// FileName  :  klanmanlogadapter.cpp
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-12-21  17:35
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#include "StdAfx.h"
#include "klanmanlogadapter.h"

KLanmanLogAdapter::KLanmanLogAdapter(
	ULONG uAppModuleId, 
	WCHAR *wsLanmanPath,
	ULONG uTrustResult, 
	ULONG uResult,
	ULONG uParentModuleId,
	ULONG uStackModuleId
)
{
	m_lanmanLog.m_uAppModuleId = uAppModuleId;
	m_lanmanLog.m_uTrustResult = uTrustResult;
	m_lanmanLog.m_uResult = uResult;
	m_lanmanLog.m_uParentModuleId = uParentModuleId;
	m_lanmanLog.m_uStackModuleId = uStackModuleId;
	wcscpy_s( m_lanmanLog.m_wsLanmanPath, MAX_PATH, wsLanmanPath );
}

KLanmanLogAdapter::~KLanmanLogAdapter(void)
{
}

BOOL KLanmanLogAdapter::Do( ILogMgr *logMgr )
{
	BOOL bResult = FALSE;

	do 
	{
		if( logMgr == NULL )
			break;

		logMgr->AddRef();
		logMgr->AddLog( LOG_TYPE_LANMAN, (BYTE*)&m_lanmanLog, sizeof(m_lanmanLog) );

	} while (FALSE);

	if( logMgr != NULL )
		logMgr->Release();

	return TRUE;
}
