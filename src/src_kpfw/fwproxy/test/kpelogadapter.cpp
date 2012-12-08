///////////////////////////////////////////////////////////////	
//	
// FileName  :  kpelogadapter.cpp
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-12-28  15:31
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#include "StdAfx.h"
#include "kpelogadapter.h"

extern ULONG GetStringHashWZ( WCHAR *wsSrc );

KPeLogAdapter::KPeLogAdapter(
	ULONG uModuleId,
	WCHAR *wsPePath,
	char *szUrl
)
{
	ZeroMemory( (char*)&m_peLog, sizeof(m_peLog) );
	m_peLog.m_uAppModuleId = uModuleId;
	wcscpy_s( m_peLog.m_wsModulePath, MAX_PATH, wsPePath );
	strcpy_s( m_peLog.m_szUrl, MAX_URL_LENGTH, szUrl );
}

KPeLogAdapter::~KPeLogAdapter(void)
{
}

BOOL KPeLogAdapter::Do( ILogMgr *logMgr )
{
	BOOL bResult = FALSE;
	ULONG uPeFileId = 0;

	do 
	{
		if( logMgr == NULL )
			break;

		logMgr->AddRef();
		uPeFileId = GetStringHashWZ( m_peLog.m_wsModulePath );
		logMgr->AddKeyLog( LOG_TYPE_PE_DOWNLOAD, uPeFileId, (BYTE*)&m_peLog, sizeof(m_peLog) );

	} while (FALSE);
	
	if( logMgr != NULL )
		logMgr->Release();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////

KPeCreateAdapter::KPeCreateAdapter(
	ULONG uCreatorModuleId,
	WCHAR *wsPePath
	)
{
	ZeroMemory( (char*)&m_peLog, sizeof(m_peLog) );
	m_peLog.m_uAppModuleId = uCreatorModuleId;
	wcscpy_s( m_peLog.m_wsModulePath, MAX_PATH, wsPePath );
}

KPeCreateAdapter::~KPeCreateAdapter(void)
{
}

BOOL KPeCreateAdapter::Do( ILogMgr *logMgr )
{
	BOOL bResult = FALSE;
	ILogIterator* pIter = NULL;
	ULONG uPeFileId = 0;
	HRESULT hRet = S_OK;
	DWORD dwTime = 0;
	DWORD uBufferLen = 0;
	PE_LOG peLog;
	ULONG uLogType;

	do 
	{
		if( logMgr == NULL )
			break;

		logMgr->AddRef();
		
		printf( __FUNCTION__" %ld\n", m_peLog.m_uAppModuleId );
		if( !logMgr->CreateKeyIterator( LOG_TYPE_PE_DOWNLOAD, m_peLog.m_uAppModuleId, &pIter ) )
			break;

		uBufferLen = sizeof(PE_LOG);
		hRet = pIter->Get( &uLogType, &dwTime, (BYTE*)&peLog, &uBufferLen );
		if( hRet != S_OK || uBufferLen != sizeof(PE_LOG) )
			break;

		uPeFileId = GetStringHashWZ( m_peLog.m_wsModulePath );
		strcpy_s( m_peLog.m_szUrl, MAX_URL_LENGTH, peLog.m_szUrl );
		logMgr->AddKeyLog( LOG_TYPE_PE_DOWNLOAD, uPeFileId, (BYTE*)&m_peLog, sizeof(m_peLog) );

	} while (FALSE);

	if( pIter != NULL )
		logMgr->DeleteIterator( pIter );

	if( logMgr != NULL )
		logMgr->Release();

	return TRUE;
}

