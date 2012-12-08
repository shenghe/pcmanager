#include "StdAfx.h"
#include "kprocesslib/interface.h"
#include "processlibqueryimp.h"
#include "kprocesslibloader.h"

CProcessLibQueryImp::CProcessLibQueryImp(void)
{
	m_bInit = FALSE;
	m_pLibLoader = NULL;
}

CProcessLibQueryImp::~CProcessLibQueryImp(void)
{
	if (m_pLibLoader)
	{
		delete m_pLibLoader;
		m_pLibLoader = NULL;
	}
}

BOOL CProcessLibQueryImp::GetInfoByPath( LPCTSTR szFullPath, KSProcessInfo** ppInfo )
{
	if (!Init(NULL))
	{
		return FALSE;
	}

	KSProcessInfo	processInfoFind;
	if ( m_pLibLoader->FindInfo( szFullPath, processInfoFind) )
	{
		if (ppInfo!=NULL)
		{
			KSProcessInfo* pNewInfo = new KSProcessInfo(processInfoFind);
			*ppInfo = pNewInfo;
		}

		return TRUE;
	}

	return FALSE;
}

VOID CProcessLibQueryImp::DeleteInfo( KSProcessInfo* pInfo )
{
	if (pInfo!=NULL)
	{
		delete pInfo;
	}
}

BOOL CProcessLibQueryImp::Init( LPCTSTR szLibPath )
{
	if (m_bInit)
		return TRUE;

	m_pLibLoader = new KProcessLibLoader;
	m_bInit = m_pLibLoader->LoadLib(szLibPath);

	return m_bInit;
}