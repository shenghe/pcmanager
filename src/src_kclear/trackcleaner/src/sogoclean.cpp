#include "sogoclean.h"
#include "kcheckappexist.h"
#include "kscbase/kscconv.h"
#include "kscbase/kscbase.h"
#include "sqlite3.h"
#include <algorithm>
extern std::vector<std::wstring> g_listProcessName;
extern CSimpleArray<int> g_vsNoinstallapp;

CKSogoClean::CKSogoClean()
{
}
CKSogoClean::~CKSogoClean()
{

}

BOOL CKSogoClean::ScanSogoHistory()
{
	if (!_CheckSogouExist())
	{
		g_vsNoinstallapp.Add(BROWSERSCLEAN_SOGO);
		return TRUE;
	}
	BOOL bRet = FALSE;
	WCHAR strPath[MAX_PATH] = {0};
	std::wstring strDbPath;
	std::wstring strDbTemp;
	std::wstring str;
	str = L"";
	SHGetSpecialFolderPath(NULL, strPath, CSIDL_APPDATA, FALSE);
	g_fnScanFile(g_pMain, BEGINPROC(BROWSERSCLEAN_SOGO), 0, 0, 0);
	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"sogouexplorer.exe")
		{
			str = L"正在运行，跳过";
			goto clean0;
		}
	}
	str = L"";
	if (m_bScan)
	{
		PathAppend(strPath, L"SogouExplorer");
		strDbTemp = strPath;
		strDbPath = strPath;
		strDbPath += L"\\uhistory.db";
		ScanDbTable(strDbPath, L"tb_urlhistory", BROWSERSCLEAN_SOGO);
		ScanDbTable(strDbPath, L"tb_urlinfo", BROWSERSCLEAN_SOGO);

		strDbPath = strDbTemp;
		strDbPath += L"\\HistoryUrl.db";
		ScanDbTable(strDbPath, L"UserRankUrl", BROWSERSCLEAN_SOGO);
		ScanDbTable(strDbPath, L"often", BROWSERSCLEAN_SOGO);
		ScanDbTable(strDbPath, L"UndoUrl", BROWSERSCLEAN_SOGO);
		EnumUserInfo(BROWSERSCLEAN_SOGO);

		strDbPath = strDbTemp;
		strDbPath += L"\\Webkit\\Cache";
		CSimpleArray<CString> vec_file;
		m_appHistory.CommfunFile(BROWSERSCLEAN_SOGO, strDbPath.c_str(), vec_file);
	}
clean0:
	g_fnScanFile(g_pMain, ENDPROC(BROWSERSCLEAN_SOGO), str.c_str(), 0, 0);
	return bRet;
}

BOOL CKSogoClean::ScanSogoCookies()
{
	if (!_CheckSogouExist())
	{
		g_vsNoinstallapp.Add(SOGO_COOKIES);
		return TRUE;
	}
	BOOL bRet = FALSE;
	WCHAR szCookiesPath[MAX_PATH] = {0};
	WCHAR szSogoAppPath[MAX_PATH] = {0};
	std::wstring strFilePath;
	CSimpleArray<CString> vec_file;
	SHGetSpecialFolderPath(NULL, szCookiesPath, CSIDL_COOKIES, FALSE);
	SHGetSpecialFolderPath(NULL, szSogoAppPath, CSIDL_APPDATA, FALSE);
	PathAppend(szSogoAppPath, L"SogouExplorer\\Webkit\\Cookies");
	strFilePath = szSogoAppPath;
	g_fnScanFile(g_pMain, BEGINPROC(SOGO_COOKIES), 0, 0, 0);
	std::wstring str;
	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"sogouexplorer.exe")
		{
			str = L"正在运行，跳过";
			goto clean0;
		}
	}
	str = L"";
	if (m_bScan)
	{
		m_appHistory.CommfunFile(SOGO_COOKIES, szCookiesPath, vec_file);
		ScanDbTable(strFilePath, L"cookies", SOGO_COOKIES);
	}
clean0:

	g_fnScanFile(g_pMain, ENDPROC(SOGO_COOKIES), str.c_str(), 0, 0);
	return bRet;
}

BOOL CKSogoClean::ScanSogoForm()
{

	if (!_CheckSogouExist())
	{
		g_vsNoinstallapp.Add(SOGO_FORM);
		return TRUE;
	}

	BOOL bRet = FALSE;
	static BOOL bFlag = FALSE;
	std::wstring str;

	g_fnScanFile(g_pMain,BEGINPROC(SOGO_FORM),0,0,0);
	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"sogouexplorer.exe")
		{
			str = L"正在运行，跳过";
			goto clean0;
		}
	}
	str = L"";

	if (!bFlag)
	{
		str = L"可以清理";
		bFlag = TRUE;
	}
clean0:
	g_fnScanFile(g_pMain,ENDPROC(SOGO_FORM),str.c_str(),0,0);
	return TRUE;
}

BOOL CKSogoClean::ScanSogoPass()
{
	if (!_CheckSogouExist())
	{
		g_vsNoinstallapp.Add(SOGO_PASS);
		return TRUE;
	}

	BOOL bRet = FALSE;
	static BOOL bFlag = FALSE;
	std::wstring str;

	g_fnScanFile(g_pMain,BEGINPROC(SOGO_PASS),0,0,0);
	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"sogouexplorer.exe")
		{
			str = L"正在运行，跳过";
			goto clean0;
		}
	}
	str = L"";

	if (!bFlag)
	{
		str = L"可以清理";
		bFlag = TRUE;
	}
clean0:
	g_fnScanFile(g_pMain,ENDPROC(SOGO_PASS),str.c_str(),0,0);

	return bRet;
}

BOOL CKSogoClean::ScanSogoAdvForm()
{
	if (!_CheckSogouExist())
	{
		g_vsNoinstallapp.Add(SOGO_ADVFORM);
		return TRUE;
	}

	BOOL bRet = FALSE;
	WCHAR szSogoAppPath[MAX_PATH] = {0};
	std::wstring strTemp;
	SHGetSpecialFolderPath(NULL, szSogoAppPath, CSIDL_APPDATA, FALSE);
	PathAppend(szSogoAppPath, L"SogouExplorer");
	strTemp = szSogoAppPath;
	PathAppend(szSogoAppPath, L"FormData.dat");
	g_fnScanFile(g_pMain,BEGINPROC(SOGO_ADVFORM),0,0,0);

	std::wstring str;

	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"sogouexplorer.exe")
		{
			str = L"正在运行，跳过";
			goto clean0;
		}
	}
	str = L"";

	if (m_bScan)
	{
		ScanDbTable(szSogoAppPath, L"IndexPrecise", SOGO_ADVFORM);
		ScanDbTable(szSogoAppPath, L"PreciseData", SOGO_ADVFORM);
		std::vector<std::wstring> vec_userInfo;
		std::vector<std::wstring>::iterator it;
		if (GetUserInfo(vec_userInfo))
		{
			for (it = vec_userInfo.begin(); it != vec_userInfo.end(); it++)
			{
				std::wstring strUserPath;
				strUserPath = strTemp;
				strUserPath += L"\\";
				strUserPath += *it;
				strUserPath += L"\\FormData.dat";
				ScanDbTable(strUserPath, L"IndexPrecise", SOGO_ADVFORM);
				ScanDbTable(strUserPath, L"PreciseData", SOGO_ADVFORM);
			}
		}
	}
clean0:
	g_fnScanFile(g_pMain,ENDPROC(SOGO_ADVFORM),str.c_str(),0,0);

	return bRet;
}

BOOL CKSogoClean::ScanDbTable(std::wstring strFileName, std::wstring strTable, int nType)
{
	BOOL bRet = FALSE;
	sqlite3* pDb = NULL;
	sqlite3_stmt* sspStart = NULL;
	int nResult = -1;
	std::string szFileName;
	std::string szTable;
	char szSql[MAX_PATH] = {0};

	szFileName = UnicodeToUtf8(strFileName);
	szTable = UnicodeToUtf8(strTable);
	nResult = sqlite3_open(szFileName.c_str(), &pDb);
	if (nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto clean0;
	}

	if (szTable.empty())
		goto clean0;

	sprintf_s(szSql, MAX_PATH - 1, "select * from %s", szTable.c_str());
	nResult = sqlite3_prepare(pDb, szSql, -1, &sspStart, 0);
	if (nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto clean0;
	}

	nResult = sqlite3_step(sspStart);
	if (nResult == SQLITE_ROW)
	{
		std::wstring strOutput;
		strOutput = strFileName;
		strOutput += L"|";
		strOutput += strTable;
		g_fnScanFile(g_pMain, nType, strOutput.c_str(), 0, 0);
	}

clean0:

	if (sspStart)
	{
		sqlite3_finalize(sspStart);
		sspStart = NULL;
	}
	if (pDb)
	{
		sqlite3_close(pDb);
		pDb = NULL;
	}
	return bRet;
}

BOOL CKSogoClean::CleanDbTable(std::wstring strInFile)
{
	BOOL bRet = FALSE;
	sqlite3* pDb = NULL;
	std::vector<std::wstring> vec_String;
	std::string szFileName;
	std::string szTable;
	int nResult = -1;
	char szSql[MAX_PATH] = {0};
	char* szError = NULL;
	if (SplitCString(strInFile, vec_String, L'|') < 2)
	{
		bRet = FALSE;
		goto clean0;
	}

	bRet = TRUE;
	szFileName = UnicodeToUtf8(vec_String[0]);
	szTable = UnicodeToUtf8(vec_String[1]);

	nResult = sqlite3_open(szFileName.c_str(), &pDb);
	if (nResult != SQLITE_OK)
	{
		goto clean0;
	}

	if(szTable.empty())
		goto clean0;

	sprintf_s(szSql, "delete from %s", szTable.c_str());
	nResult = sqlite3_exec(pDb, szSql, 0, 0, &szError);
	if (nResult != SQLITE_OK)
	{
		goto clean0;
	}

clean0:
	if (pDb)
	{
		sqlite3_close(pDb);
		pDb = NULL;
	}
	return bRet;
}

BOOL CKSogoClean::CleanSogoForm()
{
	HMODULE hMoudle = LoadLibrary(L"InetCpl.cpl");
	if (hMoudle == NULL)
	{
		goto clean0;
	}
	void* pVoid = NULL;
	pVoid = (void*)GetProcAddress(hMoudle, "ClearMyTracksByProcessW");
	if (pVoid == NULL)
	{
		goto clean0;
	}

	::ShellExecute(NULL, _T("open"), _T("rundll32.exe"),  _T("InetCpl.cpl,ClearMyTracksByProcess 16"), NULL, SW_HIDE);

clean0:
	if (hMoudle)
	{
		FreeLibrary(hMoudle);
		hMoudle = NULL;
	}

	return TRUE;
}

BOOL CKSogoClean::CleanSogoPass()
{
	HMODULE hMoudle = LoadLibrary(L"InetCpl.cpl");

	if (hMoudle == NULL)
	{
		goto clean0;
	}
	void* pVoid = NULL;
	pVoid = (void*)GetProcAddress(hMoudle, "ClearMyTracksByProcessW");
	if (pVoid == NULL)
	{
		goto clean0;
	}

	::ShellExecute(NULL, _T("open"), _T("rundll32.exe"),  _T("InetCpl.cpl,ClearMyTracksByProcess 32"), NULL, SW_HIDE);

clean0:
	if (hMoudle)
	{
		FreeLibrary(hMoudle);
		hMoudle = NULL;
	}

	return TRUE;
}

BOOL CKSogoClean::EnumUserInfo(int nType)
{
	BOOL bRet = FALSE;
	std::wstring strSogoPath;
	std::wstring strSogoTemp;
	std::vector<std::wstring>::iterator it;
	std::vector<std::wstring> vec_userInfo;

	WCHAR strKeyName[MAX_PATH] = {0};
	DWORD dwSize = MAX_PATH;
	WCHAR strFileFodler[MAX_PATH] = {0};
	SHGetSpecialFolderPath(NULL, strFileFodler, CSIDL_APPDATA, FALSE);
	PathAppend(strFileFodler, L"SogouExplorer");
	GetUserInfo(vec_userInfo);

	strSogoTemp = strFileFodler;
	strSogoPath = strSogoTemp;
	if (vec_userInfo.size() == 0)
	{
		goto clean0;
	}
	for (it = vec_userInfo.begin(); it != vec_userInfo.end(); it++)
	{
		std::wstring strTemp;
		strSogoPath = strSogoTemp;
		strSogoPath += L"\\";
		strSogoPath += *it;
		strTemp = strSogoPath;

		strSogoPath += L"\\uhistory.db";
		ScanDbTable(strSogoPath, L"tb_urlhistory", nType);
		ScanDbTable(strSogoPath, L"tb_urlinfo", nType);

		strSogoPath = strTemp;
		strSogoPath += L"HistoryUrl.db";
		ScanDbTable(strSogoPath, L"UserRankUrl", nType);
		ScanDbTable(strSogoPath, L"often", nType);
		ScanDbTable(strSogoPath, L"UndoUrl", nType);
	}

clean0:
	return bRet;
}

BOOL CKSogoClean::GetUserInfo(std::vector<std::wstring>& vec_userInfo)
{
	CRegKey reg;
	BOOL bRet = FALSE;
	HKEY hKey = HKEY_CURRENT_USER;
	std::wstring strSubKey;
	LONG lRet = -1;
	int nIndex = 0;
	WCHAR szInfo[MAX_PATH] = {0};
	DWORD dwSize = MAX_PATH;
	strSubKey = L"Software\\SogouExplorer\\Users";
	lRet = reg.Open(hKey, strSubKey.c_str(), KEY_READ);
	if (lRet != ERROR_SUCCESS)
	{
		bRet = FALSE;
		goto clean0;
	}
	while(ERROR_SUCCESS == reg.EnumKey(nIndex, szInfo, &dwSize))
	{
		vec_userInfo.push_back(szInfo);
		nIndex++;
		ZeroMemory(szInfo, MAX_PATH);
		dwSize = MAX_PATH;
	}
	if (nIndex > 0)
	{
		bRet = TRUE;
	}
clean0:
	reg.Close();
	return bRet;
}

BOOL CKSogoClean::CleanSogoHistory(std::wstring strInFile)
{
	return CleanDbTable(strInFile);
}

BOOL CKSogoClean::CleanSogoCookies(std::wstring strInFile)
{
	return CleanDbTable(strInFile);
}
BOOL CKSogoClean::CleanSogoAdvForm(std::wstring strInFile)
{
	return CleanDbTable(strInFile);
}