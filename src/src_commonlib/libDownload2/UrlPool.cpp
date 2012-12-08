#include "StdAfx.h"
#include "UrlPool.h"
#include <algorithm>

CUrlPool::CUrlPool( INT threadFromMain/*=3*/ )
{
	SetMainSiteLimit(threadFromMain);
}

CUrlPool::CUrlPool( LPCTSTR szMainUrl, const std::vector<CString> &mirrors, INT threadFromMain/*=3*/ )
{
	SetMainSiteLimit(threadFromMain);
	SetLocation(szMainUrl, mirrors);
}

VOID CUrlPool::SetLocation( LPCTSTR szMainUrl, const std::vector<CString> &mirrors )
{
	m_strMainUrl = szMainUrl;
	m_arrMirrors.clear();
	std::copy(mirrors.begin(), mirrors.end(), back_inserter(m_arrMirrors));
	m_nThreadFromOrig = 0;
	m_nMainError = 0;
}

LPCTSTR CUrlPool::GetMainUrl()
{
	++m_nThreadFromOrig;
	return m_strMainUrl;
}

BOOL CUrlPool::Get( CString &strMirrorUrl )
{
#if 1

	strMirrorUrl = m_strMainUrl;
	++m_nThreadFromOrig;
	return TRUE;

#else

	if(m_nThreadFromOrig<m_nThreadFromOrigLimit && m_nMainError<DOWNLOAD_MAINURL_MAXRETYR)
	{
		++m_nThreadFromOrig;
		strMirrorUrl = m_strMainUrl;
		MYTRACE(_T("CLocationPool :Get [Main %d/%d]%s\r\n"), m_nThreadFromOrig, m_nThreadFromOrigLimit, strMirrorUrl);
		return TRUE;
	}

	if(m_arrMirrors.empty())
	{
		MYTRACE(_T("CLocationPool :Get None(MainRetry:%d)\r\n"), m_nMainError);
		return FALSE;
	}
	strMirrorUrl = m_arrMirrors[m_arrMirrors.size()-1];
	m_arrMirrors.pop_back();
	m_arrRunning.push_back(strMirrorUrl);
	MYTRACE(_T("CLocationPool :Get [Pool %d/%d]%s\r\n"), m_arrMirrors.size(), m_arrRunning.size(), strMirrorUrl);
	return TRUE;
#endif 
}

BOOL CUrlPool::Put( LPCTSTR szMirrorUrl, BOOL bFailed )
{
	if(szMirrorUrl==m_strMainUrl)
	{
		--m_nThreadFromOrig;
		if(bFailed) ++m_nMainError;
	}
	else
	{
		CStrings::iterator it = std::find(m_arrRunning.begin(), m_arrRunning.end(), szMirrorUrl);
		ATLASSERT(it!=m_arrRunning.end());
		if(it!=m_arrRunning.end())
			m_arrRunning.erase(it);

		if(bFailed)
		{
			m_arrFailed.push_back(szMirrorUrl);
		}
		else
		{
			m_arrMirrors.push_back(szMirrorUrl);
		}
	}
	MYTRACE(_T("CLocationPool :Put %s [%d] [Main:%d/%d Mirror:%d/%d/%d]\r\n"), szMirrorUrl, !bFailed, m_nThreadFromOrig, m_nThreadFromOrigLimit, m_arrMirrors.size(), m_arrRunning.size(), m_arrFailed.size());
	return TRUE;
}

VOID CUrlPool::SetMainSiteLimit( INT threadFromMain )
{
	m_nThreadFromOrigLimit = threadFromMain;
	ATLASSERT(m_nThreadFromOrigLimit>0);
	if(m_nThreadFromOrigLimit<=0)
		m_nThreadFromOrigLimit = 1;
}
