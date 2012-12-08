#include "StdAfx.h"
#include "ImplVulIgnore.h"
#include <bksafe/bksafeconfig.h>


CImplVulIgnore::CImplVulIgnore()
{
}

HRESULT CImplVulIgnore::LoadIgnoreDB()
{
	m_arrIgnoredId.RemoveAll();
	CString ignoredfilename;
	BKSafeConfig::Get_Vulfix_IniFile( ignoredfilename );
	LoadIniSectionKeyValues( ignoredfilename, _T("igsys"), m_arrIgnoredId );
	LoadIniSectionKeyValues( ignoredfilename, _T("igsys_auto"), m_arrAutoIgnoredId );
	return S_OK;
}

HRESULT CImplVulIgnore::SaveIgnoreDB()
{
	CString ignoredfilename;
	BKSafeConfig::Get_Vulfix_IniFile( ignoredfilename );
	SaveIniSectionKeyValues( ignoredfilename, _T("igsys"), m_arrIgnoredId );
	SaveIniSectionKeyValues( ignoredfilename, _T("igsys_auto"), m_arrAutoIgnoredId );
	return S_OK;
}

HRESULT CImplVulIgnore::Ignore( int nKBID, BOOL bIgnore )
{
	int j = FindArrayIndex(m_arrIgnoredId, nKBID);
	if(bIgnore)
	{
		if(j==-1) // 需要添加 
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			CString strDate;
			strDate.Format(_T("%04d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);
			m_arrIgnoredId.Add( TPairIdString(nKBID, strDate) );
		}
	}
	else // 取消忽略
	{
		if(j!=-1)
			m_arrIgnoredId.RemoveAt( j );
		else
			return AutoIgnore(nKBID, bIgnore);
	}
	return TRUE;
}

HRESULT CImplVulIgnore::AutoIgnore( int nKBID, BOOL bIgnore )
{
	int j = FindArrayIndex(m_arrAutoIgnoredId, nKBID);
	if(bIgnore)
	{
		if(j==-1) // 需要添加 
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			CString strDate;
			strDate.Format(_T("%04d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);
			m_arrAutoIgnoredId.Add( TPairIdString(nKBID, strDate) );
		}
	}
	else
	{
		if(j!=-1)
			m_arrAutoIgnoredId.RemoveAt( j );
	}
	return TRUE;
}

int CImplVulIgnore::IsIgnored( int nKBID )
{
	if(-1 != FindArrayIndex( m_arrIgnoredId, nKBID ))
		return 1;
	else if(-1 != FindArrayIndex( m_arrAutoIgnoredId, nKBID ))
		return 2;
	return 0;
}
