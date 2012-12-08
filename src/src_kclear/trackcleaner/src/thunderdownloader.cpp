#include "ThunderDownloader.h"
#include "getinstallpath.h"
#include <shlwapi.h>
#include <map>
#include <algorithm>
#include <vector>
#include <string>
extern std::vector<std::wstring> g_listProcessName;

std::map<std::string, std::string>g_ms;

#pragma comment(lib, "shlwapi.lib")
CThunderDownloader::CThunderDownloader(void)
{
}

CThunderDownloader::~CThunderDownloader(void)
{
}
BOOL CThunderDownloader::ScanThunder()
{
	KSearchSoftwareStruct sss;
	wstring strPath;
	wstring strPath2;
	WCHAR szPath[MAX_PATH] = {0};
	CSimpleArray<CString> vec_file;
	CSimpleArray<CString> vec_file2;
	BOOL bRet = FALSE;
	sss.pszMainFileName      = TEXT( "Thunder.exe" );
	sss.nDefPathFolderCSIDL  = CSIDL_PROGRAM_FILES;
	sss.pszDefPath           = TEXT( "Thunder Network\\Thunder" );
	sss.hRegRootKey          = HKEY_LOCAL_MACHINE;
	sss.pszRegSubKey         = TEXT( "SOFTWARE\\Thunder Network\\ThunderOem\\thunder_backwnd" );
	sss.pszPathValue      	 = TEXT( "Path" );
	sss.bFolder              = FALSE;
	sss.pszUninstallListName = TEXT( "迅雷5" );
	sss.pszFileDesc          = TEXT( "Thunder" );

	bRet = SearchSoftwarePath( &sss, strPath2 );
	wcscpy_s(szPath, MAX_PATH - 1, strPath2.c_str());
	PathRemoveFileSpec(szPath);
	PathRemoveFileSpec(szPath);
	strPath2 = szPath;

	g_fnScanFile(g_pMain,BEGINPROC(THUNDERDOWNLOADER),0,0,0);

	std::wstring str;

	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"thunder.exe")
		{
			str = L"正在运行，跳过";
			goto Exit;
		}
	}
	str = L"";

	vec_file.Add(_T("SearchHistory.xml"));
	vec_file2.Add(_T("ThunderStorage.dat"));
	if (m_bScan)
	{
		WCHAR szPathEx[MAX_PATH] = {0};
		//BOOL bRet = SHGetSpecialFolderPath(NULL, szPath, CSIDL_COMMON_APPDATA, FALSE);
		UINT uResult = GetSystemDirectory(szPathEx, MAX_PATH);

		if (uResult > 3 && szPathEx[1] == TEXT(':') && szPathEx[2] == TEXT('\\'))
		{
			szPathEx[2] = TEXT('\0');       
		}
		if (szPathEx != NULL)
		{
			strPath = szPathEx;
			strPath += L"\\Users\\Public\\Thunder Network";
			WIN32_FIND_DATA wfd = {0};
			HANDLE hFile = INVALID_HANDLE_VALUE;
			wstring strTemp;
			strTemp = strPath;
			strTemp += L"\\thunder_*.*";

			hFile = FindFirstFile(strTemp.c_str(),&wfd);
			OutputDebugString(strTemp.c_str());
			if (hFile != INVALID_HANDLE_VALUE)
			{
				do 
				{
					if ((wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
						&& _wcsicmp(L".", wfd.cFileName) != 0 && _wcsicmp(L"..", wfd.cFileName) != 0)
					{
						wstring strTemp2;
						wstring strTemp3;
						strTemp2 = strPath;
						strTemp2 += L"\\";
						strTemp2 += wfd.cFileName;
						strTemp3 = strTemp2;
						strTemp2 += L"\\Profiles\\Program\\ThunderStorage.dat";
						if (PathFileExists(strTemp2.c_str()))
						{
							strTemp3 += L"\\Profiles\\Program";
							m_appHistory.CommfunFile(THUNDERDOWNLOADER, strTemp3.c_str(), vec_file2);
							//ModifySqlite(strTemp3, L"ThunderStorage.dat");
						}				
					}
				} while(FindNextFile(hFile, &wfd));
				::FindClose(hFile);
			}
		}
		//m_appHistory.CommfunFile(THUNDERDOWNLOADER, strPath.c_str(), vec_file);
		wstring strFile;
		strFile = L"ThunderStorage.dat";
		strPath2 += _T("\\UserData\\Profiles\\Program");
		//ModifySqlite(strPath2, strFile);

		m_appHistory.CommfunFile(THUNDERDOWNLOADER, strPath2.c_str(), vec_file2);
	}
Exit:
	g_fnScanFile(g_pMain,ENDPROC(THUNDERDOWNLOADER),str.c_str(),0,0);
	return TRUE;
}

BOOL CThunderDownloader::ModifySqlite(std::wstring strPath, std::wstring strFile)
{
	sqlite3* pDB = NULL;
	sqlite3_stmt* sspStart = NULL;
	BOOL bRet = FALSE;

	g_ms.clear();
	if(m_bScan)
	{
	//	g_fnScanFile(g_pMain,BEGINPROC(PPTV_PLAYER),0,0,0);
		wstring strFullPath;
		strFullPath = strPath;
		strFullPath += L"\\";
		strFullPath += strFile;

		int nResult = -1;

		BOOL bRet = FALSE;
		WCHAR szPath[MAX_PATH] = {0};
		char* szError = NULL;

		string  szDataPath;
		szDataPath = UnicodeToUtf8(strFullPath);
		nResult = sqlite3_open(szDataPath.c_str(), &pDB);
		if (nResult != SQLITE_OK)
		{
			goto _exit_;
		}

		nResult = sqlite3_prepare(pDB, "select * from BaseTaskInfo where status = 11", -1, &sspStart, 0);
		if (nResult != SQLITE_OK)
		{
			goto _exit_;
		}

		nResult = sqlite3_step(sspStart);

		while(nResult == SQLITE_ROW)
		{
			int nType = sqlite3_column_int(sspStart, 2);
			if (nType == 11)
			{
				const char* szName = (char*)sqlite3_column_text(sspStart, 21);
				const char* szTaskID = (char*)sqlite3_column_text(sspStart, 0);
				g_ms.insert(pair<string, string>(szName, szTaskID));
				wstring strName;
				strName = Utf8ToUnicode(szName);
				wstring strOutPut;
				strOutPut = strFullPath;
				strOutPut += L"|";
				strOutPut += L"BaseTaskInfo";
				strOutPut += L"|";
				strOutPut += strName;
				g_fnScanFile(g_pMain, THUNDERDOWNLOADER, strOutPut.c_str(), 0, 0);
			}
			
			nResult = sqlite3_step(sspStart);
		}
	}

	bRet = TRUE;
_exit_:
//	g_fnScanFile(g_pMain,ENDPROC(PPTV_PLAYER),0,0,0);

	if (sspStart)
	{
		sqlite3_finalize(sspStart);
		sspStart = NULL;
	}
	if (pDB)
	{
		sqlite3_close(pDB);
		pDB = NULL;
	}
	return bRet;
}

BOOL CThunderDownloader::CleanThunder(LPCTSTR lpszFile)
{
	int nResult = -1;

	wstring strPathFile;
	wstring strFileName;
	wstring strDbTable;
	sqlite3* pDB;
	sqlite3_stmt* sspStart = NULL;
	BOOL bRet = FALSE;
	CSimpleArray<CString>vec_String;

	int nSize = SplitCString1(lpszFile, vec_String, '|');
	strPathFile = vec_String[0];
	strFileName = vec_String[2];
	strDbTable = vec_String[1];

	string szPathFile;
	string szFileName;
	string szDbTable;

	szPathFile = UnicodeToUtf8(strPathFile);
	szFileName = UnicodeToUtf8(strFileName);
	szDbTable = UnicodeToUtf8(strDbTable);

	nResult = sqlite3_open(szPathFile.c_str(), &pDB);

	char* szError = NULL;
	char szSql[MAX_PATH] = {0};

	if(szDbTable.empty()||szFileName.empty())
		goto _exit_;

	sprintf_s(szSql, MAX_PATH - 1, "delete from %s where FileName = '%s'", szDbTable.c_str(), szFileName.c_str());

	nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

	if(nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto _exit_;
	}

//	nResult = sqlite3_step(sspStart);
	if (g_ms[szFileName].empty())
		goto _exit_;

	sprintf_s(szSql, MAX_PATH - 1, "delete from GenericTaskInfo where TaskID='%s'", g_ms[szFileName].c_str());

	nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

	if(nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto _exit_;
	}

	if (g_ms[szFileName].empty())
		goto _exit_;

	sprintf_s(szSql, MAX_PATH - 1, "delete from BlockRec where TaskID='%s'", g_ms[szFileName].c_str());

	nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

	if(nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto _exit_;
	}

	sprintf_s(szSql, MAX_PATH - 1, "delete from TaskExtraInfo where TaskID='%s'", g_ms[szFileName].c_str());

	nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

	if(nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto _exit_;
	}
//	nResult = sqlite3_step(sspStart);

	sprintf_s(szSql, MAX_PATH - 1, "update Category set CategoryDetailInfo='' where CategoryDescription='已下载'");

	nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

	if(nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto _exit_;
	}

	sprintf_s(szSql, MAX_PATH - 1, "update Category set IDList='' where CategoryDescription='已下载'");

	nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

	if(nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto _exit_;
	}

_exit_:

	if(sspStart)
	{
		sqlite3_finalize(sspStart);
		sspStart = NULL;
	}

	if(pDB)
	{
		sqlite3_close(pDB);
		pDB = NULL;
	}

	return TRUE;
}

int CThunderDownloader::SplitCString1(CString strIn, CSimpleArray<CString>& vec_String, TCHAR division) 
{ 
	vec_String.RemoveAll(); 
	if (!strIn.IsEmpty()) 
	{ 
		int nCount = 0; 
		int nPos = -1; 
		nPos = strIn.Find(division); 
		CString strTemp = strIn; 
		while (nPos != -1) 
		{ 
			CString strSubString = strTemp.Left(nPos);  
			strTemp = strTemp.Right(strTemp.GetLength() - nPos-1);  
			nPos = strTemp.Find(division); 
			nCount++; 
			vec_String.Add(strSubString); 
		} 

		if (nCount == vec_String.GetSize()) 
		{ 
			CString str; 
			int nSize = strIn.ReverseFind(division); 
			str = strIn.Right(strIn.GetLength()-nSize-1); 
			vec_String.Add(str); 
		} 
	} 

	return vec_String.GetSize(); 
} 