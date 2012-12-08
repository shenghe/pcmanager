#include "ttclean.h"
#include <algorithm>
#include <vector>
#include <string>
#include "kcheckappexist.h"
#include "kOperateFile.h"
extern std::vector<std::wstring> g_listProcessName;
extern CSimpleArray<int> g_vsNoinstallapp;

CTTClean::CTTClean()
{

}

CTTClean::~CTTClean()
{

}

BOOL CTTClean::ScanTT()
{
	if (!_CheckTTExist())
	{
		g_vsNoinstallapp.Add(BROWSERSCLEAN_TT);
		return TRUE;
	}
	CSimpleArray<CString> vec_file;
	CString strPath; 
    sqlite3* pDB = NULL;
    sqlite3_stmt* sspStart = NULL;

	g_fnScanFile(g_pMain,BEGINPROC(BROWSERSCLEAN_TT),0,0,0);
	std::wstring str;

	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"ttraveler.exe")
		{
			str = L"正在运行，跳过";
			goto _exit_;
		}
	}
	str = L"";
	
//	vec_file.Add(_T("navinfo.db"));
	vec_file.Add(_T("TtConf.dat.bak"));
	vec_file.Add(_T("NewMostVisitInfo.ini"));
//	vec_file.Add(_T("navinfo.db-journal"));

	if (m_bScan ==TRUE)
	{	
		strPath = _T("%APPDATA%\\Tencent\\TencentTraveler\\100");
        m_fileOpt.ConvetPath(strPath);

		m_appHistory.CommfunFile(BROWSERSCLEAN_TT,strPath,vec_file);

        
        CString strDbPath = L"";
        strPath += L"\\";

        FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("navinfo.db"));

        if (strDbPath.GetLength()>=0)
        {
			ModifyDataBase(strDbPath, L"histable");
//			ModifyDataBase(strDbPath, L"urltable");
        }
		FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("TtConf.dat"));
		if (strDbPath.GetLength()>=0)
		{
			ModifyTxtData(strDbPath, L"CUROPENURL");
			ModifyTxtData(strDbPath, L"MAXVISITHOSTS");
			ModifyTxtData(strDbPath, L"LASTOPENURL");
			ModifyTxtData(strDbPath, L"SEARCHHISTORY");
			ModifyTxtData(strDbPath, L"strNameSpace_Mbrowser");
		}
	}
_exit_:
	g_fnScanFile(g_pMain,ENDPROC(BROWSERSCLEAN_TT),str.c_str(),0,0);

	return TRUE;
	
}

BOOL CTTClean::ModifyDataBase(const CString strFileName, const CString strTable)
{
	BOOL bRet = FALSE;
	CString strFileNameEx;
	CString strTableEx;
	strFileNameEx = strFileName;
	strTableEx = strTable;
	string szFileName;
	string szTable;
	szTable =UnicodeToUtf8(strTableEx.GetBuffer());
	strTableEx.ReleaseBuffer();
	szFileName = UnicodeToUtf8(strFileNameEx.GetBuffer());
	strFileNameEx.ReleaseBuffer();
	sqlite3* pDB = NULL;
	sqlite3_stmt* sspStart = NULL;
	char szSql[MAX_PATH] = {0};

	int nResult = sqlite3_open(szFileName.c_str(), &pDB);
	if (nResult != SQLITE_OK)
	{
		goto clean0;
	}

	if (szTable.empty())
		goto clean0;

	sprintf_s(szSql, "select * from %s", szTable.c_str());
	nResult = sqlite3_prepare(pDB, szSql, -1, &sspStart, 0);
	if (nResult != SQLITE_OK)
	{
		goto clean0;
	}

	nResult = sqlite3_step(sspStart);

	if(nResult == SQLITE_ROW)
	{
		CString strOutPut = strFileName;
		strOutPut += L"|";
		strOutPut += strTable;
		g_fnScanFile(g_pMain, BROWSERSCLEAN_TT,strOutPut, 0, 0);
		bRet = TRUE;
	}
clean0:
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

BOOL CTTClean::ModifyTxtData(const CString strFileName, const CString strSection)
{
	BOOL bRet = FALSE;
	CString strFileNameEx = strFileName;
	CString strSectionEx = strSection;
	BYTE* pBuf = NULL;
	HRESULT hrRet = E_FAIL;
	FILE* pFile = NULL;
	DWORD dwRealReadSize = 0;
	std::vector<std::wstring> vLogCache;
	std::vector<std::wstring>::iterator itor;
	CString szTarget;
	int nPos = -1;

	DWORD dwFileSize  = _DoGetFileSizeByFileName(strFileNameEx);
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

	pFile = ::_wfopen(strFileNameEx, L"rt,ccs=UTF-8");
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
		nPos = szTarget.Find(strSectionEx.GetBuffer());
		if (nPos != -1)
		{
			CString strOutPut = strFileName;
			strOutPut += L"|";
			strOutPut += strSection;
			g_fnScanFile(g_pMain, BROWSERSCLEAN_TT,strOutPut, 0, 0);
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

BOOL CTTClean::CleanTT(CString& strInPath)
{	
    sqlite3* pDB = NULL;
    BOOL bRet = TRUE;
	int nPos = -1;
	CString strFileName;
	CString strSection;
	CString strTemp;
	string szFileName;
	string szSection;

	BYTE* pBuf = NULL;
	FILE* pFile = NULL;		
	FILE* pfile = NULL;

	nPos = strInPath.Find(L'|');

	if (nPos == -1)
	{
// 		bRet = FALSE;
// 		goto clean0;
		return FALSE;
	}
	strFileName = strInPath.Left(nPos);
	strSection = strInPath.Right(strInPath.GetLength() - nPos - 1);

	if (strSection == L"histable" || strSection == L"urltable")
	{
		szFileName = UnicodeToUtf8(strFileName.GetBuffer());
		strFileName.ReleaseBuffer();
		szSection = UnicodeToUtf8(strSection.GetBuffer());
		strSection.ReleaseBuffer();

		char* szError = NULL;
		char szSql[MAX_PATH] = {0};
	
		int nResult = sqlite3_open(szFileName.c_str(), &pDB);
		if (nResult != SQLITE_OK)
		{
			bRet = FALSE;
			goto clean0;
		}

		if (szSection.empty())
			goto clean0;

		sprintf_s(szSql, MAX_PATH - 1, "delete from %s", szSection.c_str());

		nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

		if(nResult != SQLITE_OK)
		{
			bRet = FALSE;
		}
	}
	else
	{
		HRESULT hrRet = E_FAIL;
		DWORD dwRealReadSize = 0;
		std::vector<std::wstring> vLogCache;
		std::vector<std::wstring> vLogCacheNew;
		std::vector<std::wstring>::iterator itor;
		CString szTarget;
		int nPos = -1;

		DWORD dwFileSize  = _DoGetFileSizeByFileName(strFileName);
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

		pFile = ::_wfopen(strFileName, L"rt,ccs=UTF-8");
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
		BOOL bStart = FALSE;

		DWORD dwLineCount = _DoGetLineByBuf(pszInfo, (DWORD)::wcslen(pszInfo), vLogCache);

		for (itor = vLogCache.begin(); itor != vLogCache.end(); itor++)
		{
			szTarget = (*itor).c_str();
			if (!bStart)
			{
				nPos = szTarget.Find(strSection.GetBuffer());
				strSection.ReleaseBuffer();
				if (nPos != -1)
				{
					bStart = TRUE;
					continue;
				}	
				vLogCacheNew.push_back(szTarget.GetBuffer());
				szTarget.ReleaseBuffer();
			}
			if (bStart)
			{
				nPos = szTarget.Find(_T("["));
				if(nPos != -1)
				{
					bStart = FALSE;
					vLogCacheNew.push_back(szTarget.GetBuffer());
					szTarget.ReleaseBuffer();
				}
			}
		}


		{
			pfile = ::_wfopen(strFileName, L"wt,ccs=UTF-8");
		}

		if (!pfile)
		{
			hrRet = S_OK;
			goto clean0;
		}


		for (itor = vLogCacheNew.begin(); itor != vLogCacheNew.end(); itor++)
		{
			wstring szTargetEx; 
			szTargetEx = (*itor);
			DWORD dwSize = (DWORD)::fwrite(szTargetEx.c_str(), sizeof(WCHAR), ::wcslen(szTargetEx.c_str()), pfile);
			::fwrite(L"\n", sizeof(WCHAR), ::wcslen(L"\n"), pfile);
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
	if (pfile)
	{
		fclose(pfile);
		pFile = NULL;
	}
    if (pDB)
	{
       sqlite3_close(pDB);
	   pDB = NULL;
	}


	HRESULT hr = S_OK;	
	IUrlHistoryStg2* pUrlHistoryStg2 = NULL;

	CoInitialize(NULL); 
	hr = CoCreateInstance(CLSID_CUrlHistory, NULL, 
		CLSCTX_INPROC_SERVER, IID_IUrlHistoryStg2, 
		(void**)&pUrlHistoryStg2);

	hr = pUrlHistoryStg2->ClearHistory();

	pUrlHistoryStg2->Release(); 

    return TRUE;
}	

