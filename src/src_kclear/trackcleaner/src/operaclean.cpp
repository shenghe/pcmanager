#include "operaclean.h"
#include "kcheckappexist.h"
#include "kscbase/kscconv.h"
#include "kscbase/kscbase.h"
#include "sqlite3.h"
#include <algorithm>
#include "koperatefile.h"
extern std::vector<std::wstring> g_listProcessName;


CKOperaClean::CKOperaClean()
{

}
CKOperaClean::~CKOperaClean()
{

}

BOOL CKOperaClean::ScanOperaHistory()
{
	BOOL bRet = FALSE;
	WCHAR szOaperaApp[MAX_PATH] = {0};
	WCHAR szOperaLocal[MAX_PATH] = {0};
	SHGetSpecialFolderPath(NULL, szOaperaApp, CSIDL_APPDATA, FALSE);
	SHGetSpecialFolderPath(NULL, szOperaLocal, CSIDL_LOCAL_APPDATA, FALSE);
	CSimpleArray<CString> vec_file;
	std::wstring str;
	std::wstring strTemp;
	std::wstring strTempEx;
	std::wstring strFilePath;
	PathAppend(szOaperaApp, L"Opera\\Opera");
	PathAppend(szOperaLocal, L"Opera\\Opera");
	strTemp = szOperaLocal;
	strTempEx = szOaperaApp;

	g_fnScanFile(g_pMain, BEGINPROC(BROWSERSCLEAN_OPERA), 0, 0, 0);
	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"opera.exe")
		{
			str = L"正在运行，跳过";
			goto clean0;
		}
	}
	str = L"";
	if (m_bScan)
	{
		vec_file.Add(L"global_history.dat");
		vec_file.Add(L"vlink4.dat");
		vec_file.Add(L"typed_history.xml");
		vec_file.Add(L"search_field_history.dat");
		strFilePath = strTempEx;
		m_appHistory.CommfunFile(BROWSERSCLEAN_OPERA, strFilePath.c_str(), vec_file, FALSE, TRUE);
		vec_file.RemoveAll();
		strFilePath = strTemp;
		strFilePath += L"\\opcache";
		m_appHistory.CommfunFile(BROWSERSCLEAN_OPERA, strFilePath.c_str(), vec_file);
		strFilePath = strTemp;
		strFilePath += L"\\cache";
		m_appHistory.CommfunFile(BROWSERSCLEAN_OPERA, strFilePath.c_str(), vec_file);
		ScanTxtSection(BROWSERSCLEAN_OPERA);
	}

clean0:
	g_fnScanFile(g_pMain, ENDPROC(BROWSERSCLEAN_OPERA), str.c_str(), 0, 0);
	return bRet;
}

BOOL CKOperaClean::ScanOperaCookies()
{
	BOOL bRet = FALSE;
	WCHAR szOaperaApp[MAX_PATH] = {0};
	SHGetSpecialFolderPath(NULL, szOaperaApp, CSIDL_APPDATA, FALSE);
	CSimpleArray<CString> vec_file;
	std::wstring str;
	std::wstring strFilePath;
	PathAppend(szOaperaApp, L"Opera\\Opera");
	strFilePath = szOaperaApp;

	g_fnScanFile(g_pMain, BEGINPROC(OPERA_COOKIES), 0, 0, 0);
	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"opera.exe")
		{
			str = L"正在运行，跳过";
			goto clean0;
		}
	}
	str = L"";
	
	vec_file.Add(L"cookies4.dat");
	m_appHistory.CommfunFile(OPERA_COOKIES, strFilePath.c_str(), vec_file);

clean0:
	g_fnScanFile(g_pMain, ENDPROC(OPERA_COOKIES), str.c_str(), 0, 0);
	return bRet;
}

BOOL CKOperaClean::ScanOperaForm()
{
	BOOL bRet = FALSE;
	WCHAR szOaperaApp[MAX_PATH] = {0};
	SHGetSpecialFolderPath(NULL, szOaperaApp, CSIDL_APPDATA, FALSE);
	CSimpleArray<CString> vec_file;
	std::wstring str;
	std::wstring strFilePath;
	PathAppend(szOaperaApp, L"Opera\\Opera");
	strFilePath = szOaperaApp;

	g_fnScanFile(g_pMain, BEGINPROC(OPERA_FORM), 0, 0, 0);
	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"opera.exe")
		{
			str = L"正在运行，跳过";
			goto clean0;
		}
	}
	str = L"";

	vec_file.Add(L"wand.dat");
	m_appHistory.CommfunFile(OPERA_FORM, strFilePath.c_str(), vec_file);

clean0:
	g_fnScanFile(g_pMain, ENDPROC(OPERA_FORM), str.c_str(), 0, 0);
	return bRet;
}

BOOL CKOperaClean::ScanTxtSection(int nType)
{
	BOOL bRet = FALSE;
	WCHAR szFile[MAX_PATH] = {0};
	BYTE* pBuf = NULL;
	HRESULT hrRet = E_FAIL;
	FILE* pFile = NULL;
	DWORD dwRealReadSize = 0;
	std::vector<std::wstring> vLogCache;
	std::vector<std::wstring>::iterator itor;
	CString szTarget;
	int nPos = -1;
	SHGetSpecialFolderPath(NULL, szFile, CSIDL_APPDATA, FALSE);
	PathAppend(szFile, L"Opera\\Opera\\sessions\\autosave.win");

	DWORD dwFileSize  = _DoGetFileSizeByFileName(szFile);
	if (dwFileSize <= 0)
	{
		hrRet = E_FAIL;
		goto clean0;
	}

	pBuf = new BYTE[dwFileSize * 2 + 2];
	if (!pBuf)
	{
		hrRet = E_OUTOFMEMORY;
		goto clean0;
	}

	::ZeroMemory(pBuf, dwFileSize * 2 + 2);

	pFile = ::_wfopen(szFile, L"rt,ccs=UTF-8");
	if (!pFile)
	{
		hrRet = E_FAIL;
		goto clean0;
	}

	dwRealReadSize = (DWORD)::fread(pBuf, sizeof(WCHAR), dwFileSize, pFile);
	if (dwRealReadSize == 0)
	{
		hrRet = E_FAIL;
		goto clean0;
	}

	WCHAR* pszInfo = (WCHAR*)pBuf;

	DWORD dwLineCount = _DoGetLineByBuf(pszInfo, (DWORD)::wcslen(pszInfo), vLogCache);


	for (itor = vLogCache.begin(); itor != vLogCache.end(); itor++)
	{
		szTarget = (*itor).c_str();
		nPos = szTarget.Find(L"[2]");
		if (nPos != -1)
		{
			CString strOutPut = szFile;
			strOutPut += L"|history";
			g_fnScanFile(g_pMain, nType,strOutPut, 0, 0);
			bRet = TRUE;
			break;
		}
	}
clean0:
	if (pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	if (pFile)
	{
		fclose(pFile);
		pFile = NULL;
	}
	return bRet;
}

BOOL CKOperaClean::CleanOperaHistory(std::wstring strInFile)
{
	BOOL bRet = FALSE;
	std::vector<std::wstring> vec_file;
	BYTE* pBuf = NULL;
	FILE* pFile = NULL;		
	FILE* pfile = NULL;
	std::wstring strFileName;
	std::wstring szTargetEx; 
	std::wstring szTarget;

	std::vector<std::wstring> vLogCache;
	std::vector<std::wstring> vLogCacheNew;
	std::vector<std::wstring>::iterator itor;
	int nPos = -1;

	HRESULT hrRet = E_FAIL;
	DWORD dwRealReadSize = 0;
	if (SplitCString(strInFile, vec_file, L'|') < 2)
	{
		bRet = FALSE;
		goto clean0;
	}
	bRet = TRUE;
	strFileName = vec_file[0];

	DWORD dwFileSize  = _DoGetFileSizeByFileName(strFileName.c_str());
	if (dwFileSize <= 0)
	{
		hrRet = E_FAIL;
		goto clean0;
	}

	pBuf = new BYTE[dwFileSize * 2 + 2];
	if (!pBuf)
	{
		hrRet = E_OUTOFMEMORY;
		goto clean0;
	}

	::ZeroMemory(pBuf, dwFileSize * 2 + 2);

	pFile = ::_wfopen(strFileName.c_str(), L"rt,ccs=UTF-8");
	if (!pFile)
	{
		hrRet = E_FAIL;
		goto clean0;
	}

	dwRealReadSize = (DWORD)::fread(pBuf, sizeof(WCHAR), dwFileSize, pFile);
	if (dwRealReadSize == 0)
	{
		hrRet = E_FAIL;
		goto clean0;
	}

	WCHAR* pszInfo = (WCHAR*)pBuf;

	DWORD dwLineCount = _DoGetLineByBuf(pszInfo, (DWORD)::wcslen(pszInfo), vLogCache);

	for (itor = vLogCache.begin(); itor != vLogCache.end(); itor++)
	{
		szTarget = (*itor).c_str();

		nPos = (int)szTarget.find(L"[2]");
		if (nPos != -1)
		{
			break;
		}
		if (szTarget.find(L"window count") != -1)
		{
			szTarget = L"window count=1";
		}
		vLogCacheNew.push_back(szTarget);
	}


	{
		pfile = ::_wfopen(strFileName.c_str(), L"wt,ccs=UTF-8");
	}

	if (!pfile)
	{
		hrRet = S_OK;
		goto clean0;
	}


	for (itor = vLogCacheNew.begin(); itor != vLogCacheNew.end(); itor++)
	{
		szTargetEx = (*itor);
		DWORD dwSize = (DWORD)::fwrite(szTargetEx.c_str(), sizeof(WCHAR), ::wcslen(szTargetEx.c_str()), pfile);
		::fwrite(L"\n", sizeof(WCHAR), ::wcslen(L"\n"), pfile);
	}
clean0:
	if (pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	if (pFile)
	{
		fclose(pFile);
		pFile = NULL;
	}
	if (pfile)
	{
		fclose(pfile);
		pFile = NULL;
	}
	return bRet;
}
