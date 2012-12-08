#pragma once

#include <runoptimize/interface.h>

class CRunDatQueryEng : public IRunDatQueryEng
{
public:
	CRunDatQueryEng()
	{
		m_bInit = FALSE;
		m_pLibLoader = NULL;
	}
	virtual ~CRunDatQueryEng()
	{
		if (m_pLibLoader)
		{
			delete m_pLibLoader;
			m_pLibLoader = NULL;
		}
	}

public:
	virtual BOOL GetInfoByPathValue(DWORD nType, LPCTSTR lpPath, LPCTSTR lpFullPath, KSRunInfo** ppInfo)
	{
		_Init();

		KSRunInfo*	pInfoFind = NULL;
		if ( m_pLibLoader->FindDatInfo( CFindData(nType,CString(lpPath),CString(lpFullPath)), &pInfoFind) )
		{
			if (ppInfo!=NULL && pInfoFind!= NULL )
			{
				KSRunInfo* pNewInfo = new KSRunInfo(*pInfoFind);
				*ppInfo = pNewInfo;
			}

			return TRUE;
		}
		return FALSE;
	}

	virtual VOID DeleteInfo(KSRunInfo* pInfo)
	{
		if (pInfo!=NULL)
		{
			delete pInfo;
		}
	}

protected:
	BOOL _Init()
	{
		if (m_bInit)
			return TRUE;

		m_pLibLoader = new CLibLoader;
		m_pLibLoader->LoadLib(NULL);
		m_bInit = TRUE;
		return TRUE;
	}

protected:
	CLibLoader*	m_pLibLoader;
	BOOL		m_bInit;
};