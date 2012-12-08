#include "firefoxclean.h"
#include <vector>
#include <string>
#include <algorithm>
#include "kcheckappexist.h"
extern std::vector<std::wstring> g_listProcessName;
extern CSimpleArray<int> g_vsNoinstallapp;
CFireFoxClean::CFireFoxClean()
{

}

CFireFoxClean::~CFireFoxClean()
{

}

BOOL CFireFoxClean::ScanFireFox()
{	
	//文件部分
	if (!_CheckFireFoxExist())
	{
		g_vsNoinstallapp.Add(BROWSERSCLEAN_FIREFOX);
		return TRUE;
	}
	CSimpleArray<CString> vec_file;
	CString strPath; 
	std::wstring str;
    sqlite3* pDB = NULL;
    sqlite3_stmt* sspStart = NULL;


	g_fnScanFile(g_pMain,BEGINPROC(BROWSERSCLEAN_FIREFOX),0,0,0);

	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"firefox.exe")
		{
			str = L"正在运行，跳过";
			goto _exit_;
		}
	}
	
	str = L"";
	if (m_bScan == TRUE)
	{
		//首先枚举文件夹名称
		

		TCHAR szBuffer[MAX_PATH] = {0};
		::SHGetSpecialFolderPath( NULL, szBuffer, CSIDL_LOCAL_APPDATA, FALSE);
		strPath = szBuffer;
		strPath += _T("\\Mozilla\\Firefox\\Profiles");
		
		CSimpleArray<CString> vec_folder;
		m_fileOpt.DoFileFolder(strPath,vec_folder,FALSE,TRUE);
		for (int i=0 ;i<vec_folder.GetSize();i++)
		{
			CString strPath = vec_folder[i];
			strPath.Append(_T("\\"));
			strPath.Append(_T("Cache"));
			m_appHistory.CommfunFile(BROWSERSCLEAN_FIREFOX,strPath,vec_file);
		}
        
        strPath = _T("%APPDATA%\\Mozilla\\Firefox\\Profiles\\");
        m_fileOpt.ConvetPath(strPath);
        CString strDbPath = L"";

        FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("places.sqlite"));
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
            nResult = sqlite3_prepare(pDB, "select * from moz_historyvisits", -1, &sspStart, 0);
            if (nResult != SQLITE_OK)
            {
                goto _exit_1;
            }

            nResult = sqlite3_step(sspStart);

            if(nResult == SQLITE_ROW)
            {
                CString strOutPut = strDbPath;
                strOutPut += L"|moz_historyvisits";
                g_fnScanFile(g_pMain, BROWSERSCLEAN_FIREFOX, strOutPut, 0, 0);

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
	}

_exit_:

	g_fnScanFile(g_pMain,ENDPROC(BROWSERSCLEAN_FIREFOX),str.c_str(),0,0);

	return TRUE;

}	



//扫描表单
BOOL CFireFoxClean::ScanFireFrom()
{
	if (!_CheckFireFoxExist())
	{
		g_vsNoinstallapp.Add(FIREFOX_SAVEFROM);
		return TRUE;
	}
	//文件部分
	CSimpleArray<CString> vec_file;
	CString strPath; 
    sqlite3* pDB = NULL;
    sqlite3_stmt* sspStart = NULL;

	g_fnScanFile(g_pMain,BEGINPROC(FIREFOX_SAVEFROM),0,0,0);

	std::wstring str;
	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"firefox.exe")
		{
			str = L"正在运行，跳过";
			goto _exit_;
		}
	}

	str = L"";

	if (m_bScan == TRUE)
	{

        strPath = _T("%APPDATA%\\Mozilla\\Firefox\\Profiles\\");
        m_fileOpt.ConvetPath(strPath);
        CString strDbPath = L"";

        FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("formhistory.sqlite"));
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
            nResult = sqlite3_prepare(pDB, "select * from moz_formhistory", -1, &sspStart, 0);
            if (nResult != SQLITE_OK)
            {
                goto _exit_1;
            }

            nResult = sqlite3_step(sspStart);

            if(nResult == SQLITE_ROW)
            {
                CString strOutPut = strDbPath;
                strOutPut += L"|moz_formhistory";
                g_fnScanFile(g_pMain, FIREFOX_SAVEFROM, strOutPut, 0, 0);

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
		
	}
_exit_:

	g_fnScanFile(g_pMain,ENDPROC(FIREFOX_SAVEFROM),str.c_str(),0,0);


	return TRUE;

}

 

//扫描密码
BOOL CFireFoxClean::ScanFirePass()
{
	if (!_CheckFireFoxExist())
	{
		g_vsNoinstallapp.Add(FIREFOX_PASSWORD);
		return TRUE;
	}
    CSimpleArray<CString> vec_file;
    CString strPath; 
    sqlite3* pDB = NULL;
    sqlite3_stmt* sspStart = NULL;

    g_fnScanFile(g_pMain,BEGINPROC(FIREFOX_PASSWORD),0,0,0);

    std::wstring str;
    std::vector<std::wstring>::iterator it;
    for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
    {
        str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
        if (str == L"firefox.exe")
        {
            str = L"正在运行，跳过";
            goto _exit_;
        }
    }

    str = L"";

    if (m_bScan == TRUE)
    {

        strPath = _T("%APPDATA%\\Mozilla\\Firefox\\Profiles\\");
        m_fileOpt.ConvetPath(strPath);
        CString strDbPath = L"";

        FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("signons.sqlite"));
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
            nResult = sqlite3_prepare(pDB, "select * from moz_logins", -1, &sspStart, 0);
            if (nResult != SQLITE_OK)
            {
                goto _exit_1;
            }

            nResult = sqlite3_step(sspStart);

            if(nResult == SQLITE_ROW)
            {
                CString strOutPut = strDbPath;
				strOutPut += L"|moz_logins";
                g_fnScanFile(g_pMain, FIREFOX_PASSWORD, strOutPut, 0, 0);

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

    }
_exit_:

    g_fnScanFile(g_pMain,ENDPROC(FIREFOX_PASSWORD),str.c_str(),0,0);


    return TRUE;

}


BOOL CFireFoxClean::CleanFireFox(CString &strInPath)
{	
    //文件部分
    
    sqlite3* pDB = NULL;
       
    BOOL bRet = TRUE;

	if (strInPath.Find(L'|')==-1)
	{
		bRet = FALSE;
		return bRet;
	}

    CString strPath = _T("%APPDATA%\\Mozilla\\Firefox\\Profiles\\");
    m_fileOpt.ConvetPath(strPath);
    CString strDbPath = L"";

    FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("places.sqlite"));
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
            

            sprintf_s(szSql, MAX_PATH - 1, "delete from moz_historyvisits");

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


BOOL CFireFoxClean::CleanFireFoxForm(CString &strInPath)
{	
    //文件部分

    sqlite3* pDB = NULL;

    BOOL bRet = TRUE;

	if (strInPath.Find(L'|')==-1)
	{
		bRet = FALSE;
		return bRet;
	}

    CString strPath = _T("%APPDATA%\\Mozilla\\Firefox\\Profiles\\");
    m_fileOpt.ConvetPath(strPath);
    CString strDbPath = L"";

    FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("formhistory.sqlite"));
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


        sprintf_s(szSql, MAX_PATH - 1, "delete from moz_formhistory");

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

BOOL CFireFoxClean::CleanFireFoxPass(CString& strInPath)
{	
    //文件部分

    sqlite3* pDB = NULL;

    BOOL bRet = TRUE;

	if (strInPath.Find(L'|')==-1)
	{
		bRet = FALSE;
		return bRet;
	}

    CString strPath = _T("%APPDATA%\\Mozilla\\Firefox\\Profiles\\");
    m_fileOpt.ConvetPath(strPath);
    CString strDbPath = L"";

    FindFileInDirectory(strPath.GetBuffer(),strDbPath,_T("signons.sqlite"));
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


        sprintf_s(szSql, MAX_PATH - 1, "delete from moz_logins");

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

BOOL CFireFoxClean::ScanFireFoxCookies()
{
	if (!_CheckFireFoxExist())
	{
		g_vsNoinstallapp.Add(FIREFOX_COOKIES);
		return TRUE;
	}
	BOOL bRet = FALSE;
	WCHAR szPath[MAX_PATH] = {0};
	CString strPath;
	CString strPathTemp;
	WCHAR szTempContent[MAX_PATH] = {0};
	int nPos = -1;
	DWORD dwRetErr;
	CSimpleArray<CString> vec_file;
	sqlite3* pDb = NULL;
	sqlite3_stmt* sspStart = NULL;
	string strFullPath;
	char* szError = NULL;
	char szSql[MAX_PATH] = {0};
	int nResult = -1;
	std::wstring str;

	g_fnScanFile(g_pMain, BEGINPROC(FIREFOX_COOKIES), 0, 0, 0);

	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"firefox.exe")
		{
			str = L"正在运行，跳过";
			goto clean0;
		}
	}

	str = L"";

	if(m_bScan)
	{
		bRet = SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);
		if (!bRet)
		{
			goto clean0;
		}
		strPath = szPath;
		if (strPath.ReverseFind(L'\\') != strPath.GetLength() -1)
		{
			strPath += L"\\";
		}
		strPathTemp = strPath;
		strPathTemp += L"Mozilla\\Firefox\\profiles.ini";
		dwRetErr = GetPrivateProfileString(L"Profile0", L"Path", L"", szTempContent, MAX_PATH-1, strPathTemp.GetBuffer());
		if (sizeof(szTempContent) > 0)
		{
			CString strConvertPath;
			int nPosTemp = -1;
			strConvertPath = szTempContent;
			nPosTemp = strConvertPath.Find(L'/');
			if (nPosTemp != -1)
			{
				CString strRight;
				CString strLeft;
				strLeft = strConvertPath.Left(nPosTemp);
				strRight = strConvertPath.Right(strConvertPath.GetLength() - nPosTemp - 1);
				strConvertPath = strLeft;
				strConvertPath += L"\\";
				strConvertPath += strRight;
			}
			strPath += L"Mozilla\\Firefox\\";
			strPath += strConvertPath;
			strPath += L"\\cookies.sqlite";

			strFullPath = UnicodeToUtf8(strPath.GetBuffer());
			nResult = sqlite3_open(strFullPath.c_str(), &pDb);
			if (nResult != SQLITE_OK)
			{
				bRet = FALSE;
				goto clean0;
			}
			nResult = sqlite3_prepare(pDb, "select * from moz_cookies", -1, &sspStart, 0);
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
				strOutput += L"moz_cookies";
				g_fnScanFile(g_pMain, FIREFOX_COOKIES, strOutput, 0, 0);
			}
		}
	}
	
clean0:

	g_fnScanFile(g_pMain, ENDPROC(FIREFOX_COOKIES), str.c_str(), 0, 0);

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


BOOL CFireFoxClean::CleanFireFoxCookies(CString& strInPath)
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