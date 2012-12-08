///////////////////////////////////////////////////////////////	
//	
// FileName  :  kmodulelogadapter.cpp
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-12-22  14:27
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#include "StdAfx.h"
#include "kmodulelogadapter.h"

KModuleLogAdapter::KModuleLogAdapter(
	ULONG uModuleId, 
	WCHAR *wsModulePath,
	UCHAR *byMd5
)
{
	m_moduleLog.m_uModuleId = uModuleId;	
	wcscpy_s( m_moduleLog.m_wsModulePath, MAX_PATH, wsModulePath );
	memcpy( m_moduleLog.m_byMd5, byMd5, MD5_LENGTH );
}

KModuleLogAdapter::~KModuleLogAdapter(void)
{
}

BOOL KModuleLogAdapter::Do( ILogMgr *logMgr )
{
	BOOL bResult = FALSE;
	ILogIterator* pIter = NULL;

	do 
	{
		if( logMgr == NULL )
			break;

		logMgr->AddRef();

		if( !logMgr->CreateKeyIterator( LOG_TYPE_MODULE, m_moduleLog.m_uModuleId, &pIter ) )
		{
			logMgr->AddKeyLog( LOG_TYPE_MODULE, m_moduleLog.m_uModuleId, (BYTE*)&m_moduleLog, sizeof(m_moduleLog) );
		}
		else
		{
			logMgr->DeleteIterator( pIter );
			pIter = NULL;
		}

	} while (FALSE);
	

	if( logMgr != NULL )
		logMgr->Release();

	return TRUE;
}
