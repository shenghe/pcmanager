#include "StdAfx.h"
#include "ProgramTrafficMgr.h"
#include "fwproxy_public.h"

extern IFwProxy *g_pFwProxy;
extern VOID OutputProgramTrafficItem( PROGRAM_TRAFFIC *pProgramTraffic );

CProgramTrafficMgr::CProgramTrafficMgr(void):m_hThread(NULL)
{
}

CProgramTrafficMgr::~CProgramTrafficMgr(void)
{
}

BOOL CProgramTrafficMgr::Init()
{
	if( m_hThread != NULL )
		return FALSE;
	
	m_hExit = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hThread = CreateThread( 0, 0, ThreadGetProgramTraffic, (PVOID)this, 0, 0 );
	return TRUE;
}

BOOL CProgramTrafficMgr::Uninit()
{
	if( m_hThread != NULL )
	{
		SetEvent( m_hExit );
		WaitForSingleObject( m_hThread, INFINITE );
	}
	return TRUE;
}

DWORD WINAPI CProgramTrafficMgr::ThreadGetProgramTraffic( PVOID pParam )
{
	PROGRAM_TRAFFIC pProgramTraffic[200];
	ULONG dwRetCount;

	CProgramTrafficMgr *pThis = (CProgramTrafficMgr *)pParam;
	while( WaitForSingleObject( pThis->m_hExit, 1000 * 31 ) == WAIT_TIMEOUT )
	{
		g_pFwProxy->EnumProgramTrafficFromCache( pProgramTraffic, 200, &dwRetCount );

		for( DWORD i=0; i<dwRetCount; i++ )
		{
			printf( "\n#%d---------------------------------------\n", i );
			OutputProgramTrafficItem( &pProgramTraffic[i] );
		}
	}

	return 0;
}