#include "chromeclean.h"
#include <vector>
#include <string>
#include <Shlwapi.h>
#include <algorithm>
#include "kcheckappexist.h"
extern std::vector<std::wstring> g_listProcessName;
extern CSimpleArray<int> g_vsNoinstallapp;

CChromClean::CChromClean()
{

}

CChromClean::~CChromClean()
{

}

BOOL CChromClean::ScanChrome()
{	
	//文件部分
	if (!_CheckChromeExist())
	{
		g_vsNoinstallapp.Add(BROWSERSCLEAN_CHROME);
		return TRUE;
	}
	CSimpleArray<CString> vec_file;
	CString strPath; 
	std::wstring str;
	g_fnScanFile(g_pMain,BEGINPROC(BROWSERSCLEAN_CHROME),0,0,0);
	
	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"chrome.exe")
		{
			str = L"正在运行，跳过";
			goto _exit_;
		}
	}
	str = L"";
	if (m_bScan ==TRUE)
	{
        TCHAR szBuffer[MAX_PATH] = {0};
		
        ::SHGetSpecialFolderPath( NULL, szBuffer, CSIDL_LOCAL_APPDATA, FALSE);

        strPath = szBuffer;
        strPath += _T("\\Google\\Chrome\\User Data\\Default\\Cache");
		m_appHistory.CommfunFile(BROWSERSCLEAN_CHROME,strPath,vec_file);
		
        strPath = szBuffer;
		strPath += _T("\\Google\\Chrome\\User Data\\Default\\Media Cache");
		m_appHistory.CommfunFile(BROWSERSCLEAN_CHROME,strPath,vec_file);

		//vec_file.Add(_T("Cookies"));
		//vec_file.Add(_T("Extension Cookies"));
		vec_file.Add(_T("Archived History"));
		//vec_file.Add(_T("History"));
		//vec_file.Add(_T("History-journal"));
		//vec_file.Add(_T("Thumbnails"));
		//vec_file.Add(_T("Thumbnails-journal"));
		vec_file.Add(_T("Current Tabs"));
		vec_file.Add(_T("Current Session"));
		vec_file.Add(_T("Last Tabs"));
		vec_file.Add(_T("Last Session"));
		vec_file.Add(_T("Safe Browsing Bloom"));
		vec_file.Add(_T("History Index*"));
		vec_file.Add(_T("Visited Links"));

		//vec_file.Add(_T("Extension Cookies"));
        strPath = szBuffer;
		strPath += _T("\\Google\\Chrome\\User Data\\Default");
		m_appHistory.CommfunFile(BROWSERSCLEAN_CHROME,strPath,vec_file);
		vec_file.RemoveAll();

        sqlite3* pDB = NULL;
        sqlite3_stmt* sspStart = NULL;

        CString strDbPath = L"";
        strPath += L"\\";
        FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("History"));
		strPath.ReleaseBuffer();

        if (strDbPath.GetLength()>=0)
        {
            KW2UTF8  szDataPath(strDbPath.GetBuffer());
			strDbPath.ReleaseBuffer();
            int nResult = sqlite3_open(szDataPath, &pDB);
            if (nResult != SQLITE_OK)
            {
                goto _exit_1;
            }
            nResult = sqlite3_prepare(pDB, "select * from segments", -1, &sspStart, 0);
            if (nResult != SQLITE_OK)
            {
                goto _exit_1;
            }

            nResult = sqlite3_step(sspStart);

            if(nResult == SQLITE_ROW)
            {
                CString strOutPut = strDbPath;
                strOutPut += L"|segments";
                g_fnScanFile(g_pMain, BROWSERSCLEAN_CHROME, strOutPut, 0, 0);
            }
        _exit_1:
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

			nResult = sqlite3_open(szDataPath, &pDB);
			if (nResult != SQLITE_OK)
			{
				goto _exit_2;
			}

            nResult = sqlite3_prepare(pDB, "select * from visits", -1, &sspStart, 0);
            if (nResult != SQLITE_OK)
            {
                goto _exit_2;
            }

            nResult = sqlite3_step(sspStart);

            if(nResult == SQLITE_ROW)
            {
                CString strOutPut = strDbPath;
                strOutPut += L"|visits";
                g_fnScanFile(g_pMain, BROWSERSCLEAN_CHROME, strOutPut, 0, 0);

            }

        _exit_2:
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

        }   

	}
	
_exit_:
	g_fnScanFile(g_pMain,ENDPROC(BROWSERSCLEAN_CHROME),str.c_str(),0,0);
	
	return TRUE;
}


BOOL CChromClean::ScanChromeFrom()
{	
	//文件部分
	if (!_CheckChromeExist())
	{
		g_vsNoinstallapp.Add(CHROME_SAVEFROM);
		return TRUE;
	}
	CSimpleArray<CString> vec_file;
	CString strPath; 

	g_fnScanFile(g_pMain,BEGINPROC(CHROME_SAVEFROM),0,0,0);
	std::wstring str;
	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"chrome.exe")
		{
			str = L"正在运行，跳过";
			goto _exit_;
		}
	}
	str = L"";
	if (m_bScan ==TRUE)
	{

        TCHAR szBuffer[MAX_PATH] = {0};

        ::SHGetSpecialFolderPath( NULL, szBuffer, CSIDL_LOCAL_APPDATA, FALSE);

        strPath = szBuffer;
		strPath += _T("\\Google\\Chrome\\User Data\\Default");
        sqlite3* pDB = NULL;
        sqlite3_stmt* sspStart = NULL;

        CString strDbPath = L"";
        strPath += L"\\";
        FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("Web Data"));
		strPath.ReleaseBuffer();

        if (strDbPath.GetLength()>=0)
        {
            KW2UTF8  szDataPath(strDbPath.GetBuffer());
			strDbPath.ReleaseBuffer();
            int nResult = sqlite3_open(szDataPath, &pDB);
            if (nResult != SQLITE_OK)
            {
                goto _exit_1;
            }
            nResult = sqlite3_prepare(pDB, "select * from autofill", -1, &sspStart, 0);
            if (nResult != SQLITE_OK)
            {
                goto _exit_1;
            }

            nResult = sqlite3_step(sspStart);

            if(nResult == SQLITE_ROW)
            {
                CString strOutPut = strDbPath;
                strOutPut += L"|autofill";
                g_fnScanFile(g_pMain, CHROME_SAVEFROM, strOutPut, 0, 0);

            }
	_exit_1:
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

			nResult = sqlite3_open(szDataPath, &pDB);
			if (nResult != SQLITE_OK)
			{
				goto _exit_2;
			}

            nResult = sqlite3_prepare(pDB, "select * from autofill_dates", -1, &sspStart, 0);
            if (nResult != SQLITE_OK)
            {
                goto _exit_2;
            }

            nResult = sqlite3_step(sspStart);

            if(nResult == SQLITE_ROW)
            {
                CString strOutPut = strDbPath;
                strOutPut += L"|autofill_dates";
                g_fnScanFile(g_pMain, CHROME_SAVEFROM, strOutPut, 0, 0);

            }

_exit_2:
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

        }   
	
	}

_exit_:
	g_fnScanFile(g_pMain,ENDPROC(CHROME_SAVEFROM),str.c_str(),0,0);
	
	return TRUE;
}

BOOL CChromClean::ScanChromePass()
{	
	if (!_CheckChromeExist())
	{
		g_vsNoinstallapp.Add(CHROME_PASSWORD);
		return TRUE;
	}
	CSimpleArray<CString> vec_file;
	CString strPath; 

	g_fnScanFile(g_pMain,BEGINPROC(CHROME_PASSWORD),0,0,0);

	std::wstring str;
	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"chrome.exe")
		{
			str = L"正在运行，跳过";
			goto _exit_;
		}
	}
	str = L"";
	if (m_bScan ==TRUE)
	{
        TCHAR szBuffer[MAX_PATH] = {0};

        ::SHGetSpecialFolderPath( NULL, szBuffer, CSIDL_LOCAL_APPDATA, FALSE);

        strPath = szBuffer;
        strPath += _T("\\Google\\Chrome\\User Data\\Default");
        sqlite3* pDB = NULL;
        sqlite3_stmt* sspStart = NULL;

        CString strDbPath = L"";
        strPath += L"\\";
        FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("Web Data"));
		strPath.ReleaseBuffer();

        if (strDbPath.GetLength()>=0)
        {
            KW2UTF8  szDataPath(strDbPath.GetBuffer());
            int nResult = sqlite3_open(szDataPath, &pDB);
            if (nResult != SQLITE_OK)
            {
                goto _exit_1;
            }
            nResult = sqlite3_prepare(pDB, "select * from logins", -1, &sspStart, 0);
            if (nResult != SQLITE_OK)
            {
                goto _exit_1;
            }

            nResult = sqlite3_step(sspStart);

            if(nResult == SQLITE_ROW)
            {
                CString strOutPut = strDbPath;
                strOutPut += L"|logins";
                g_fnScanFile(g_pMain, CHROME_PASSWORD, strOutPut, 0, 0);

            }
_exit_1:
            
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

        }   

		strDbPath = L"";
		FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("Login Data"));
		strPath.ReleaseBuffer();

		if (strDbPath.GetLength()>=0)
		{
			KW2UTF8  szDataPath(strDbPath.GetBuffer());
			strDbPath.ReleaseBuffer();
			int nResult = sqlite3_open(szDataPath, &pDB);
			if (nResult != SQLITE_OK)
			{
				goto _exit_2;
			}
			nResult = sqlite3_prepare(pDB, "select * from logins", -1, &sspStart, 0);
			if (nResult != SQLITE_OK)
			{
				goto _exit_2;
			}

			nResult = sqlite3_step(sspStart);

			if(nResult == SQLITE_ROW)
			{
				CString strOutPut = strDbPath;
				strOutPut += L"|logins";
				g_fnScanFile(g_pMain, CHROME_PASSWORD, strOutPut, 0, 0);

			}
_exit_2:

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

		}   
	}
_exit_:
	g_fnScanFile(g_pMain,ENDPROC(CHROME_PASSWORD),str.c_str(),0,0);

	return TRUE;		
}

BOOL CChromClean::CleanChrome(CString& strInPath)
{	
    //文件部分

    sqlite3* pDB = NULL;

    BOOL bRet = TRUE;

	if (strInPath.Find(L'|')==-1)
	{
		return FALSE;
	}

    TCHAR szBuffer[MAX_PATH] = {0};

    ::SHGetSpecialFolderPath( NULL, szBuffer, CSIDL_LOCAL_APPDATA, FALSE);

    CString strPath = szBuffer;
	strPath += _T("\\Google\\Chrome\\User Data\\Default");
    CString strDbPath = L"";

    strPath += L"\\";
    FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("History"));
	strPath.ReleaseBuffer();

    if (strDbPath.GetLength()>=0)
    {
        KW2UTF8  szDataPath(strDbPath.GetBuffer());
		strDbPath.ReleaseBuffer();
        char* szError = NULL;
        char szSql[MAX_PATH] = {0};

        int nResult = sqlite3_open(szDataPath, &pDB);
        if (nResult != SQLITE_OK)
        {
            bRet = FALSE;
            goto _exit_1;
        }


        sprintf_s(szSql, MAX_PATH - 1, "delete from segments");

        nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

        if(nResult != SQLITE_OK)
        {
            bRet = FALSE;
        }

        memset(szSql,0,MAX_PATH);

        sprintf_s(szSql, MAX_PATH - 1, "delete from visits");

        nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

        if(nResult != SQLITE_OK)
        {
            bRet = FALSE;
        }

     _exit_1:

        if (pDB)
        {
            sqlite3_close(pDB);
            pDB = NULL;
        }

    }   


    return TRUE;

}	


BOOL CChromClean::CleanChromeForm(CString& strInPath)
{	
    //文件部分

    sqlite3* pDB = NULL;

    BOOL bRet = TRUE;

	if (strInPath.Find(L'|')==-1)
	{
		return FALSE;
	}

    TCHAR szBuffer[MAX_PATH] = {0};

    ::SHGetSpecialFolderPath( NULL, szBuffer, CSIDL_LOCAL_APPDATA, FALSE);

    CString strPath = szBuffer;

	strPath += _T("\\Google\\Chrome\\User Data\\Default");

    CString strDbPath = L"";

    strPath += L"\\";
    FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("Web Data"));
	strPath.ReleaseBuffer();

    if (strDbPath.GetLength()>=0)
    {
        KW2UTF8  szDataPath(strDbPath.GetBuffer());
		strDbPath.ReleaseBuffer();
        char* szError = NULL;
        char szSql[MAX_PATH] = {0};

        int nResult = sqlite3_open(szDataPath, &pDB);
        if (nResult != SQLITE_OK)
        {
            bRet = FALSE;
            goto _exit_1;
        }


        sprintf_s(szSql, MAX_PATH - 1, "delete from autofill");

        nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

        if(nResult != SQLITE_OK)
        {
            bRet = FALSE;
        }

        memset(szSql,0,MAX_PATH);

        sprintf_s(szSql, MAX_PATH - 1, "delete from autofill_dates");

        nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

        if(nResult != SQLITE_OK)
        {
            bRet = FALSE;
        }

_exit_1:

        if (pDB)
        {
            sqlite3_close(pDB);
            pDB = NULL;
        }

    }   


    return TRUE;

}	

BOOL CChromClean::CleanChromePass(CString& strInPath)
{	
    sqlite3* pDB = NULL;

    BOOL bRet = TRUE;

	if (strInPath.Find(L'|')==-1)
	{
		bRet = FALSE;
		return bRet;
	}

    TCHAR szBuffer[MAX_PATH] = {0};

    ::SHGetSpecialFolderPath( NULL, szBuffer, CSIDL_LOCAL_APPDATA, FALSE);

    CString strPath = szBuffer;

	strPath += _T("\\Google\\Chrome\\User Data\\Default");

    CString strDbPath = L"";

    strPath += L"\\";
    FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("Web Data"));
	strPath.ReleaseBuffer();

    if (strDbPath.GetLength()>=0)
    {
        KW2UTF8  szDataPath(strDbPath.GetBuffer());
		strDbPath.ReleaseBuffer();
        char* szError = NULL;
        char szSql[MAX_PATH] = {0};

        int nResult = sqlite3_open(szDataPath, &pDB);
        if (nResult != SQLITE_OK)
        {
            goto _exit_1;
        }


        sprintf_s(szSql, MAX_PATH - 1, "delete from logins");

        nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

        if(nResult != SQLITE_OK)
        {
           
        }
_exit_1:

        if (pDB)
        {
            sqlite3_close(pDB);
            pDB = NULL;
        }

    }   

	strDbPath = L"";
	FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("Login Data"));
	strPath.ReleaseBuffer();

	if (strDbPath.GetLength()>=0)
	{
		KW2UTF8  szDataPath(strDbPath.GetBuffer());
		strDbPath.ReleaseBuffer();
		char* szError = NULL;
		char szSql[MAX_PATH] = {0};

		int nResult = sqlite3_open(szDataPath, &pDB);
		if (nResult != SQLITE_OK)
		{
			bRet = FALSE;
			goto _exit_2;
		}


		sprintf_s(szSql, MAX_PATH - 1, "delete from logins");

		nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);

		if(nResult != SQLITE_OK)
		{
			
		}
_exit_2:

		if (pDB)
		{
			sqlite3_close(pDB);
			pDB = NULL;
		}

	}   

	bRet = TRUE;


    return TRUE;

}	

BOOL CChromClean::ScanChormCookies()
{
	BOOL bRet = FALSE;
	WCHAR szPath[MAX_PATH] = {0};
	CString strPath;
	sqlite3* pDb = NULL;
	sqlite3_stmt* sspStart = NULL;
	string strFullPath;
	char* szError = NULL;
	char szSql[MAX_PATH] = {0};
	int nResult = -1;
	if (!_CheckChromeExist())
	{
		g_vsNoinstallapp.Add(CHROME_COOKIES);
		return TRUE;
	}
	g_fnScanFile(g_pMain, BEGINPROC(CHROME_COOKIES), 0, 0, 0);

	std::wstring str;
	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"chrome.exe")
		{
			str = L"正在运行，跳过";
			goto clean0;
		}
	}
	str = L"";
	if(m_bScan)
	{

		SHGetSpecialFolderPath(NULL, szPath, CSIDL_LOCAL_APPDATA, FALSE);
		strPath = szPath;
		if (strPath.ReverseFind(L'\\') != strPath.GetLength() - 1)
		{
			strPath += L"\\";
		}
		strPath += L"Google\\Chrome\\User Data\\Default\\Cookies";
		strFullPath = UnicodeToUtf8(strPath.GetBuffer());
		nResult = sqlite3_open(strFullPath.c_str(), &pDb);
		if (nResult != SQLITE_OK)
		{
			bRet = FALSE;
			goto clean0;
		}
		nResult = sqlite3_prepare(pDb, "select * from cookies", -1, &sspStart, 0);
		if (nResult != SQLITE_OK)
		{
			bRet = FALSE;
			goto clean0;
		}

		nResult = sqlite3_step(sspStart);
		if (nResult == SQLITE_ROW)
		{
			CString strOutput;
			strOutput = Utf8ToUnicode(strFullPath).c_str();
			strOutput += L"|";
			strOutput += L"cookies";
			g_fnScanFile(g_pMain, CHROME_COOKIES, strOutput, 0, 0);
		}

	}
clean0:

	g_fnScanFile(g_pMain, ENDPROC(CHROME_COOKIES), str.c_str(), 0, 0);
	if(sspStart != NULL)
	{
		sqlite3_finalize(sspStart);
		sspStart = NULL;
	}
	if(pDb != NULL)
	{
		sqlite3_close(pDb);
		pDb = NULL;
	}
	return bRet;
}

BOOL CChromClean::CleanChromCookies(CString& strInPath)
{
	std::wstring strFile;
	std::wstring strSection;
	std::string szFile;
	std::string szSection;
	int nPos = -1;
	sqlite3* pDb = NULL;
	int nResult = -1;
	char* szError = NULL;

	char szSql[MAX_PATH] = {0};
	nPos = strInPath.Find(L"|");
	if (nPos == -1)
	{
		return FALSE;
	}
	strFile = strInPath.Left(nPos).GetBuffer();
	strSection = strInPath.Right(strInPath.GetLength() - nPos - 1).GetBuffer();
	szFile = UnicodeToUtf8(strFile);
	szSection = UnicodeToUtf8(strSection);
	nResult = sqlite3_open(szFile.c_str(), &pDb);
	if (nResult != SQLITE_OK)
	{
		goto clean0;
	}
	if(szSection.empty())
		goto clean0;

	sprintf_s(szSql, "delete from %s", szSection.c_str());
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
	return TRUE;
}