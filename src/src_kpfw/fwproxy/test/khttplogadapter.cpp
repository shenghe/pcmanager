///////////////////////////////////////////////////////////////	
//	
// FileName  :  khttplogadapter.cpp
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-12-25  09:07
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#include "StdAfx.h"
#include "khttplogadapter.h"

KHttpLogAdapter::KHttpLogAdapter(
	ULONG uModuleId, 
	char *szUrl
)
{
	ZeroMemory( (char*)&m_httpLog, sizeof(m_httpLog) );
	m_httpLog.m_uAppModuleId = uModuleId;
	strcpy_s( m_httpLog.m_szUrl, MAX_URL_LENGTH+MAX_HOST_LENGTH, szUrl );
}

KHttpLogAdapter::~KHttpLogAdapter(void)
{
}

BOOL KHttpLogAdapter::Do( ILogMgr *logMgr )
{
	BOOL bResult = FALSE;
	ILogIterator* pIter = NULL;

	do 
	{
		if( logMgr == NULL )
			break;

		logMgr->AddRef();
		logMgr->AddKeyLog( LOG_TYPE_HTTP, m_httpLog.m_uAppModuleId, (BYTE*)&m_httpLog, sizeof(m_httpLog) );

	} while (FALSE);
	

	if( logMgr != NULL )
		logMgr->Release();

	return TRUE;
}
