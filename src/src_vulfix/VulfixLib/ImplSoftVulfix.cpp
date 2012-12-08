#include "StdAfx.h"
#include "ImplSoftVulfix.h"
#include "HotfixUtils.h"
#include "EnvUtils.h"
#include "LibLoader.h"

CImplSoftVulfix::CImplSoftVulfix()
	: m_Observer( NULL )
{
	_Inited = FALSE;
}

CImplSoftVulfix::~CImplSoftVulfix(void)
{
}

void CImplSoftVulfix::SetObserver( IVulfixObserver *pObserver )
{
	m_Observer = pObserver;
}

HRESULT CImplSoftVulfix::Scan(DWORD dwFlags)
{
	TIME_CHECK( _T("CImplSoftVulfix::Scan ") );
	HRESULT hr;
	T_ComInit __init__com__;
	_Reset();
	do 
	{
		CSysEnv& sysEnv = singleton<CSysEnv>::Instance();
		sysEnv.Init();
		if( FAILED( hr=sysEnv.IsSupported(FALSE) ) )
			break;
		
		_Init();
		CString filename;
		m_db.SetObserver( m_Observer );
		if( !m_db.Load( GetXmlDBFileName(VTYPE_SOFTLEAK, filename, FALSE), NULL, dwFlags) )
		{
			hr = KERR_LOAD_FILE;
			break; 
		}
		m_db.GetUnfixedLeakList( m_arrLeaks );

		hr = KERR_NONE;
	} while (FALSE);
	return hr;
}

HRESULT CImplSoftVulfix::Cancel()
{
	m_db.Cancel();
	return S_OK;
}

int CImplSoftVulfix::Count()
{
	return m_arrLeaks.GetSize();
}

const CSimpleArray<LPTVulSoft>& CImplSoftVulfix::GetResults()
{
	return m_arrLeaks;
}

HRESULT CImplSoftVulfix::EnableVulCOM( int nId, BOOL bEnable/*=FALSE*/ )
{
	HRESULT hr = S_FALSE;
	do 
	{
		int i = _FindLeak( nId );
		if(i==-1) break;
		
		LPTVulSoft pitem = m_arrLeaks[i];
		if(!pitem->nDisableCom) break;

		for(int j=0; j<pitem->arrCLSID.GetSize(); ++j)
		{
			EnableCom( pitem->arrCLSID[j], bEnable );
		}
		hr = S_OK;
	} while (FALSE);
	
	return hr;
}

void CImplSoftVulfix::_Reset()
{
	m_arrLeaks.RemoveAll();
}

void CImplSoftVulfix::_Init()
{
	if(!_Inited)
	{
		_Inited = TRUE;
	}
}

int CImplSoftVulfix::_FindLeak( int nId )
{
	for(int i=0; i<m_arrLeaks.GetSize(); ++i)
	{
		if(m_arrLeaks[i]->nID==nId)
			return i;
	}
	return -1;
}

void CImplSoftVulfix::_Notify( TVulFixEventType evt, int nKbId, DWORD dwParam1, DWORD dwParam2 )
{
	if(m_Observer)
		m_Observer->OnVulfixEvent( evt, nKbId, dwParam1, dwParam2 );
}
