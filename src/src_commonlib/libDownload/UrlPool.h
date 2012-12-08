#pragma once
#include <vector>
typedef std::vector<CString> CStrings; 

class CUrlPool
{
public:
	CUrlPool(INT threadFromMain=3);
	CUrlPool(LPCTSTR szMainUrl, const CStrings &mirrors, INT threadFromMain=3);

	VOID SetLocation(LPCTSTR szMainUrl, const CStrings &mirrors);
	LPCTSTR GetMainUrl();

	BOOL Get(CString &strMirrorUrl);
	BOOL Put(LPCTSTR szMirrorUrl, BOOL bFailed);
	VOID SetMainSiteLimit(INT threadFromMain);

public:
	CString m_strMainUrl;

protected:
	int m_nThreadFromOrigLimit;
	CStrings m_arrMirrors;
	CStrings m_arrRunning;
	CStrings m_arrFailed;
	int m_nThreadFromOrig;
	INT m_nMainError;	// err num of main url 
};