#include "StdAfx.h"
#include "ImplVulfixed.h"
#include "LeakDB.h"
#include "ItemFilter.h"
#include "EnvUtils.h"
#include "HotfixUtils.h"
#include "LibLoader.h"

CImplVulfixed::CImplVulfixed()
{
	m_pObserver = NULL;
	m_pFilterOS = NULL;
}

CImplVulfixed::~CImplVulfixed(void)
{
	if( m_pFilterOS )
	{
		delete m_pFilterOS;
		m_pFilterOS = NULL;
	}
}

void CImplVulfixed::SetObserver( IVulfixObserver *pObserver )
{
	m_pObserver = pObserver;
}

HRESULT CImplVulfixed::Scan(DWORD dwFlags)
{
	TIME_CHECK( _T("CImplVulfixed::Scan ") );
	
	m_Canceled = FALSE;
	HRESULT hr; 
	T_ComInit __init__com__;
	Reset();	
	do 
	{
		CSysEnv &sysEnv = singleton<CSysEnv>::Instance();
		sysEnv.Init();
		if( FAILED( hr=sysEnv.IsSupported(FALSE) ) )
			break;
		
		if(!m_pFilterOS)
			m_pFilterOS = CreateOSFilter(sysEnv.m_WinVer, dwFlags);
		InitOSFilter( m_pFilterOS, sysEnv.m_WinVer, dwFlags);
		if(m_pFilterOS->WaitComplete())
		{
			m_db.SetObserver( m_pObserver );
			CString filename;
			if( !m_Canceled && !m_db.Load( GetXmlDBFileName(VTYPE_WINDOWS, filename, IsWin64()), m_pFilterOS, dwFlags ) )
			{
				hr = KERR_LOAD_FILE;
				break;
			}

			
			if( !m_Canceled )
			{
				CSimpleArray<LPTUpdateItem> arrInstalled, arrInvalid;
				CSimpleArray<TReplacedUpdate*> arrReplaced;
				m_db.GetUnfixedLeakList( arrInstalled, m_arrInstalledPatches, arrInvalid, arrReplaced );				
			}
		}
		hr = KERR_NONE;
	} while (FALSE);
	return S_OK;
}

HRESULT CImplVulfixed::Cancel()
{
	m_Canceled = TRUE;
	if(m_pFilterOS)
		m_pFilterOS->Cancel();
	m_db.Cancel();
	return S_OK;
}

int CImplVulfixed::Count()
{
	return m_arrInstalledPatches.GetSize();
}

const CSimpleArray<TItemFixed*>& CImplVulfixed::GetResults()
{
	return m_arrInstalledPatches;
}

const CSimpleArray<TReplacedUpdate*>& CImplVulfixed::GetReplacedUpdates()
{
	return m_arrReplacedUpdates;
}

void CImplVulfixed::Reset()
{
	m_arrInstalledPatches.RemoveAll();
	m_arrReplacedUpdates.RemoveAll();
}
