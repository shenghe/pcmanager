#include "specialapp.h"
#include "getinstallpath.h"
#include "koperatefile.h"

extern std::vector<std::wstring> g_listProcessName;

BOOL CSpecialApp::CleanMyIE2() //
{	

	g_fnScanFile(g_pMain,BEGINPROC(BROWSERSCLEAN_MYIE2),0,0,0);

	if (m_bScan ==TRUE)
	{
		CString strRegPath = _T("HKEY_CURRENT_USER\\Software\\MYIE2");

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRegPath,hRootKey,strSubKey);

		CString strValueName = _T("Folder");
		CString strValueData;
		m_regOpt.GetDefValue(hRootKey,strSubKey,strValueName,strValueData);


		int iLen = strValueData.GetLength();

		if(iLen <= 0)
			goto Exit;

		if (strValueData.GetAt(iLen-1) != _T('\\'))
		{
			strValueData.Append(_T("\\"));
		}

		if (strValueData!=_T(""))
		{

			CString strFileFullPath;
			WIN32_FIND_DATA fd;
			HANDLE hFindFile;

			strFileFullPath = strValueData;
			
			strFileFullPath.Append(_T("Config\\myie.ini"));
			hFindFile = FindFirstFile(strFileFullPath.GetBuffer(), &fd);
			strFileFullPath.ReleaseBuffer();
			if(hFindFile != INVALID_HANDLE_VALUE)
			{	
				//找到文件
				ScanSection(BROWSERSCLEAN_MYIE2,_T("UndoUrl"),strFileFullPath);	
				::FindClose(hFindFile); 
			}
		}
	}

Exit:

	g_fnScanFile(g_pMain,ENDPROC(BROWSERSCLEAN_MYIE2),0,0,0);
	return TRUE;
}
//ini路径|节|键
BOOL CSpecialApp::CleanMyIE2_Clean(int iType,LPCTSTR lpcszAppPath)
{	
	return CleanUltraEdit_Clean(iType,lpcszAppPath);
}

BOOL CSpecialApp::CleanKuGoo(int iType,LPCTSTR lpcszAppPath)
{	

	//找到KuGoo安装路径
	CString strRegPath = _T("HKEY_CURRENT_USER\\Software\\KuGoo6");
	CString strValueName = _T("AppFileName");

	HKEY	hRootKey;
	CString strSubKey;
	m_regOpt.CrackRegKey(strRegPath,hRootKey,strSubKey);

	CString strValueData;
	m_regOpt.GetDefValue(hRootKey,strSubKey,strValueName,strValueData);
	int iEnd = strValueData.Find(_T("KuGoo.exe"));

	if (iEnd != -1)
	{
		strValueData = strValueData.Mid(0 ,iEnd-1);

		int iLen = strValueData.GetLength();

		CString strValueDataEx;
		strValueDataEx = strValueData;

		if (iLen>=0)
		{
			if (strValueData.GetAt(iLen-1)!='\\')
			{
				strValueData.Append(_T("\\"));
			}

			strValueData.Append(_T("KuGoo.xml"));

			WIN32_FIND_DATA fd;
			HANDLE hFindFile = FindFirstFile(strValueData, &fd);
			if(hFindFile == INVALID_HANDLE_VALUE)
			{
				return FALSE;
			}
			::FindClose(hFindFile); 

			if (strValueDataEx.GetAt(iLen-1)!='\\')
			{
				strValueDataEx.Append(_T("\\"));
			}

			strValueDataEx.Append(_T("KGData.db"));
			CleanKugooEx(strValueDataEx, L"MusicItems");

		}
	}


	//解析xml文档

	TiXmlDocument xmlDoc;
	FILE* lpFile = _tfopen(strValueData, _T("rb"));
	if ( lpFile != NULL )
	{	
		xmlDoc.LoadFile(lpFile);
		fclose(lpFile);
		if ( xmlDoc.Error() == 0 )
		{
			OutputDebugString(_T("打开文件成功"));

			TiXmlElement* rootNode = xmlDoc.RootElement();
			if (rootNode !=NULL)
			{
				TiXmlElement* nodeGeneral = rootNode->FirstChildElement("General");
				if ( nodeGeneral != NULL )
				{
					const char* pName =  nodeGeneral->Attribute("SearchHistory");
					if (pName != NULL)
					{
						std::wstring wstrName = Utf8ToUnicode(pName);
						if (!wstrName.empty())
						{						
							if(0!=wcscmp(wstrName.c_str(),L""))
							{
								CString strFind;
								strFind.Format(_T("%s|General|SearchHistory|%s"),strValueData,wstrName.c_str());
								g_fnScanFile(g_pMain,iType,strFind,0,0);
							}
						}
					}
				}
			}
		}
	}

	return TRUE;
}
BOOL CSpecialApp::CleanKugooEx(CString strFileName, CString strTable)
{
	BOOL bRet = FALSE;
	CString strFileNameEx;
	CString strTableEx;
	strFileNameEx = strFileName;
	strTableEx = strTable;
	std::string szFileName;
	std::string szTable;
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
		g_fnScanFile(g_pMain, 9010, strOutPut, 0, 0);
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

//传参为 路径|节点|属性|值
BOOL  CSpecialApp::CleanKuGoo_Clean(int iType,LPCTSTR lpcszAppPath)
{
	CString strPath = lpcszAppPath;
	g_regClean.ConvetPath(strPath);
	std::string szFileName;
	std::string szSection;
	sqlite3* pDB = NULL;
	BOOL bRet = TRUE;

	CSimpleArray<CString> vec_String;
	int nCount = SplitCString1(strPath,vec_String,'|');

	TiXmlDocument xmlDoc;

	if (nCount == 2)
	{
		szFileName = UnicodeToUtf8(vec_String[0].GetBuffer());
		szSection = UnicodeToUtf8(vec_String[1].GetBuffer());

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
		goto clean0;
	}
	
	USES_CONVERSION;
	FILE* lpFile = _tfopen( vec_String[0], _T("rb"));
	if ( lpFile != NULL )
	{	
		xmlDoc.LoadFile(lpFile);
		fclose(lpFile);

		if ( xmlDoc.Error() == 0 )
		{
			TiXmlElement* rootNode = xmlDoc.RootElement();
			
			if (rootNode !=NULL)
			{
				TiXmlElement* nodeGeneral = rootNode->FirstChildElement(W2A(vec_String[1]));
				if ( nodeGeneral != NULL )
				{	
					nodeGeneral->SetAttribute(W2A(vec_String[2]),"");
					
					if(false==xmlDoc.SaveFile(W2A(vec_String[0])))
					{
					}
				}
			}
		}
	}
clean0:
	if (pDB)
	{
		sqlite3_close(pDB);
		pDB = NULL;
	}


	return TRUE;
}

BOOL CSpecialApp::CleanKMPlayer(int iType,LPCTSTR lpcszAppPath,LPCTSTR lpcszValue)
{	

	
	CSimpleArray<CString> vec_reg;
	vec_reg.Add(_T("HKEY_CURRENT_USER\\Software\\KMPlayer\\KMP2.0\\OptionList\\KMPWizard"));
	vec_reg.Add(_T("HKEY_CURRENT_USER\\Software\\KMPlayer\\KMP2.0\\OptionArea"));
	BOOL bSucess = FALSE;
	
	for (int i=0;i<vec_reg.GetSize();i++)
	{
		if (bSucess)
		{
			return TRUE;
		}
		CString strRegPath = vec_reg[i];

		CString strValueName = _T("InstallPath");

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRegPath,hRootKey,strSubKey);

		CString strValueData;
		m_regOpt.GetDefValue(hRootKey,strSubKey,strValueName,strValueData);


		//strValueData = _T("C:\\Program Files\\KMPlayer\\KMPlayer.exe");
		int iEnd = strValueData.Find(_T("KMPlayer.exe"));

		if (iEnd != -1)
		{
			strValueData = strValueData.Mid(0 ,iEnd-1);

			int iLen = strValueData.GetLength();

			if (iLen>=0)
			{
				if (strValueData.GetAt(iLen-1)!='\\')
				{
					strValueData.Append(_T("\\"));
				}

				strValueData.Append(_T("PlayList\\Default.kpl"));

				WIN32_FIND_DATA fd;
				HANDLE hFindFile = FindFirstFile(strValueData, &fd);
				if(hFindFile != INVALID_HANDLE_VALUE)
				{
					g_fnScanFile(g_pMain,iType,strValueData,fd.nFileSizeLow,fd.nFileSizeHigh);
					bSucess = TRUE;
				}

				::FindClose(hFindFile); 
			}
		}

	}



	return TRUE;
}

BOOL CSpecialApp::CleanStorm(int iType,LPCTSTR lpcszAppPath)
{	
	CString strAppPath = lpcszAppPath;
	//g_regClean.ConvetPath(strAppPath);
	m_appHistory.ConvetPath(strAppPath);
	TiXmlDocument xmlDoc;

	FILE* lpFile = _tfopen(strAppPath, _T("rb"));
	if ( lpFile != NULL )
	{	
		xmlDoc.LoadFile(lpFile);

		fclose(lpFile);

		if ( xmlDoc.Error() == 0 )
		{
			OutputDebugString(_T("打开文件成功"));

			TiXmlElement* rootNode = xmlDoc.RootElement();
			if (rootNode!=NULL)
			{
				TiXmlElement* nodeGeneral = rootNode->FirstChildElement("localplayhist");
				if ( nodeGeneral != NULL && nodeGeneral->FirstChildElement("item") != NULL)
				{	
					CString strFind;
					strFind.Format(_T("%s|localplayhist"),strAppPath);
					g_fnScanFile(g_pMain,iType,strFind,0,0);
				}
				TiXmlElement* nodeGeneral1 = rootNode->FirstChildElement("netplayhist");
				if ( nodeGeneral1 != NULL && nodeGeneral1->FirstChildElement("item") != NULL)
				{	
					CString strFind;
					strFind.Format(_T("%s|netplayhist"),strAppPath);
					g_fnScanFile(g_pMain,iType,strFind,0,0);
				}
			}
		}
	}

	CString strRegPath = _T("HKEY_CURRENT_USER\\Software\\Storm");
	CString strValueName = _T("exepath");

	HKEY	hRootKey;
	CString strSubKey;
	m_regOpt.CrackRegKey(strRegPath,hRootKey,strSubKey);

	CString strValueData;
	m_regOpt.GetDefValue(hRootKey,strSubKey,strValueName,strValueData);
	int iEnd = strValueData.ReverseFind(_T('\\'));

	static BOOL bCheck = TRUE;
	if (iEnd != -1 && bCheck)
	{
		CString strPath;
		CSimpleArray<CString> vec_file;
		vec_file.Add(L"playlist.smpl");
		strPath = strValueData.Left(iEnd);

		m_appHistory.CommfunFile(9008, strPath, vec_file);
		bCheck = FALSE;
	}
	return TRUE;
}

//路径|节点|子节点|属性
//C:\Documents and Settings\All Users\Application Data\Storm\config.xml
//localplayhist
//item
//url
//C:\Documents and Settings\Administrator\桌面\0812_195706.3gp
BOOL CSpecialApp::CleanStorm_Clean(int Itype,LPCTSTR lpcszAppPath)
{
	CString strPath = lpcszAppPath;
	g_regClean.ConvetPath(strPath);

	CSimpleArray<CString> vec_String;
	int iCount = SplitCString1(strPath,vec_String,'|');

	if(iCount<2)
	{
		return FALSE;
	}

	TiXmlDocument xmlDoc;


	//获得文件夹名称改变其属性
	iCount = vec_String[0].ReverseFind('\\');
	CString strTmp;
	if (iCount>0)
	{
		strTmp = vec_String[0].Mid(0,iCount);
	}

	DWORD dwErrCode=0;
	if(FALSE == SetFileAttributes(strTmp, FILE_ATTRIBUTE_NORMAL))
	{
		dwErrCode=GetLastError();
	}

	USES_CONVERSION;
	FILE* lpFile = _tfopen(vec_String[0], _T("a+b"));
	if ( lpFile != NULL )
	{
		xmlDoc.LoadFile(lpFile);

		fclose(lpFile);

		if ( xmlDoc.Error() == 0 )
		{
			OutputDebugString(_T("\n打开文件成功\n"));

			TiXmlElement* rootNode = xmlDoc.RootElement();
			if (rootNode !=NULL)
			{
				TiXmlElement* nodeGeneral = rootNode->FirstChildElement(W2A(vec_String[1]));
				if ( nodeGeneral != NULL )
				{
//					rootNode->RemoveChild(nodeGeneral);
                    nodeGeneral->Clear();
					xmlDoc.SaveFile(W2A(vec_String[0]));
				}
			}
		}
	}
	return TRUE;

}

BOOL CSpecialApp::CleanQQLive(int iType,LPCTSTR lpcszAppPath)
{	
	
	CString strPath = lpcszAppPath;
	g_regClean.ConvetPath(strPath);
	int iIndex = strPath.Find(_T("*"));
	
	if (iIndex>0)
	{
		CString strTempPath1 = strPath.Mid(0,iIndex);
		CString strTempPath2 = strPath.Mid(iIndex+1);
		
		CSimpleArray<CString> vec_folder;
		m_fileOpt.DoFileFolder(strTempPath1,vec_folder,FALSE,TRUE);

		for (int i=0;i<vec_folder.GetSize();i++)
		{
			CString strFullPath = vec_folder[i];
			strFullPath.Append(strTempPath2);
			
			int iCount = strFullPath.ReverseFind('\\');
			if (iCount>0) //将文件名与路径分离
			{
				CString strTmp = strFullPath.Mid(0,iCount);
				CString strTmp2 =strFullPath.Mid(iCount+1);

				CSimpleArray<CString> vec_Exts;
				vec_Exts.Add(strTmp2);

				m_appHistory.CommfunFile(iType,strTmp,vec_Exts);
				
			}

		}

	}


	//TCHAR szPath[MAX_PATH*2]={0};
	//CString strAppPath = lpcszAppPath;
	//g_regClean.ConvetPath(strAppPath);

	//if (NULL !=GetPrivateProfileString(_T("Cache"),_T("Directory"),NULL,szPath,MAX_PATH*2-1,strAppPath))
	//{
	//	CSimpleArray<CString>  vec_file;
	//	m_appHistory.CommfunFile(iType,szPath,vec_file);
	//}
	
	return TRUE;
}

//扫描清理 ini文件路径|节|键|值

BOOL CSpecialApp::CleanUltraEdit_Clean(int iType,LPCTSTR lpcszPath)
{	
	CString strPath = lpcszPath;
	g_regClean.ConvetPath(strPath);

	CSimpleArray<CString> vec_String;
	if(SplitCString1(strPath,vec_String,'|')< 3)
	{
		return FALSE;
	}
	
	if (TRUE !=WritePrivateProfileStruct(vec_String[1],vec_String[2], NULL, 0,vec_String[0]))
	{
		DWORD dwErrCode = GetLastError();
		if (dwErrCode == ERROR_FILE_NOT_FOUND)	//系统找不到指定文件
		{
			return TRUE;
		}
		else if (dwErrCode == ERROR_ACCESS_DENIED)
		{	
			g_fnScanFileError(g_pMain,iType,lpcszPath,0,0);	
		}
		else
		{	
			g_fnScanFileError(g_pMain,iType,lpcszPath,0,0);
		}
	}


	return TRUE;
}

//一键清理方案
BOOL CSpecialApp::CleanUltraEdit(int iType,LPCTSTR lpcszAppPath)
{	
    BOOL retval = FALSE;
	CString strPath = lpcszAppPath;
	g_regClean.ConvetPath(strPath);
	
	WritePrivateProfileSection(_T("Replace History"),NULL,strPath);
	WritePrivateProfileSection(_T("Recent File List"),NULL,strPath);
	WritePrivateProfileSection(_T("Find Hex History"),NULL,strPath);
	WritePrivateProfileSection(_T("Find History"),NULL,strPath);
	

	TCHAR* szBuffer = NULL;
	TCHAR szKey[1024] = {0};
	CString strKey = _T("");
	CString strKeyName = _T("");
	CString strKeyValue = _T("");
    int nBufferSize;

    szBuffer = new TCHAR[65536];
    if (!szBuffer)
        goto clean0;

	nBufferSize = GetPrivateProfileSection(
		_T("Open Files"), 
		szBuffer, 
		65536,
		strPath
		);
	


	for (int n = 0, i = 0; n < nBufferSize; n++)
	{
		if (szBuffer[n] == 0)
		{
			szKey[i] = 0;
			strKey = szKey;

			strKeyName = strKey.Left(strKey.Find('='));
			strKeyValue = strKey.Mid(strKey.Find('=') + 1);
			
			int iLen = (int)wcslen(_T("Open File"));
			if ((-1!=strKeyName.Find(_T("Open File"))&&(strKeyName.GetLength()>iLen)))
			{	

				if ((strKeyName.GetAt(iLen)>='0'&&strKeyName.GetAt(iLen)<='9'))
				{
					WritePrivateProfileStruct(_T("Open Files"),strKeyName, NULL, 0,lpcszAppPath);
				}
			
			}

			i = 0;

		}
		else
		{
			szKey[i] = szBuffer[n];

			i++;
		}

	}

	ZeroMemory(szBuffer, 65536);
	nBufferSize = GetPrivateProfileSection(
		_T("Recent Projects"), 
		szBuffer, 
		65536,
		strPath
		);



	for (int n = 0, i = 0; n < nBufferSize; n++)
	{
		if (szBuffer[n] == 0)
		{
			szKey[i] = 0;
			strKey = szKey;

			strKeyName = strKey.Left(strKey.Find('='));
			strKeyValue = strKey.Mid(strKey.Find('=') + 1);

			int iLen = (int)wcslen(_T("Recent Project "));
			if ((-1!=strKeyName.Find(_T("Recent Project "))&&(strKeyName.GetLength()>iLen)))
			{	

// 				if ((strKeyName.GetAt(iLen)>='0'&&strKeyName.GetAt(iLen)<='9'))
// 				{
					WritePrivateProfileStruct(_T("Open Files"),strKeyName, NULL, 0,lpcszAppPath);
//				}

			}

			i = 0;

		}
		else
		{
			szKey[i] = szBuffer[n];

			i++;
		}

	}
	
    retval = TRUE;

clean0:
    if (szBuffer)
    {
        delete[] szBuffer;
        szBuffer = NULL;
    }

	return retval;
}

//扫描方案
BOOL CSpecialApp::CleanUltraEdit_Scan(int iType,LPCTSTR lpcszAppPath)
{	

    BOOL retval = FALSE;
	CString strPath = lpcszAppPath;
	g_regClean.ConvetPath(strPath);

	ScanSection(iType,_T("Replace History"),strPath);
	ScanSection(iType,_T("Recent File List"),strPath);
	ScanSection(iType,_T("Find History"),strPath);
	ScanSection(iType,_T("Find History"),strPath);
	
	TCHAR* szBuffer = NULL;
	TCHAR szKey[1024] = {0};
	CString strKey = _T("");
	CString strKeyName = _T("");
	CString strKeyValue = _T("");
    int nBufferSize;

    szBuffer = new TCHAR[65536];
    if (!szBuffer)
        goto clean0;

	nBufferSize = GetPrivateProfileSection(
		_T("Open Files"), 
		szBuffer, 
		65536,
		strPath
		);


	for (int n = 0, i = 0; n < nBufferSize; n++)
	{
		if (szBuffer[n] == 0)
		{
			szKey[i] = 0;
			strKey = szKey;

			strKeyName = strKey.Left(strKey.Find('='));
			strKeyValue = strKey.Mid(strKey.Find('=') + 1);
			
			int iLen = (int)wcslen(_T("Open File"));
			if ((-1!=strKeyName.Find(_T("Open File"))&&(strKeyName.GetLength()>iLen)))
			{	

				if ((strKeyName.GetAt(iLen)>='0'&&strKeyName.GetAt(iLen)<='9'))
				{	

					CString strFullPath;
					strFullPath.Format(_T("%s|%s|%s|%s"),strPath,_T("Open Files"),strKeyName,strKeyValue);

					g_fnScanFile(g_pMain,iType,strFullPath,0,0);
				}
			}

			i = 0;

		}
		else
		{
			szKey[i] = szBuffer[n];

			i++;
		}

	}

	ZeroMemory(szBuffer, 65536);

	nBufferSize = GetPrivateProfileSection(
		_T("Recent Projects"), 
		szBuffer, 
		65536,
		strPath
		);


	for (int n = 0, i = 0; n < nBufferSize; n++)
	{
		if (szBuffer[n] == 0)
		{
			szKey[i] = 0;
			strKey = szKey;

			strKeyName = strKey.Left(strKey.Find('='));
			strKeyValue = strKey.Mid(strKey.Find('=') + 1);

			int iLen = (int)wcslen(_T("Recent Project "));
			if ((-1!=strKeyName.Find(_T("Recent Project "))&&(strKeyName.GetLength()>iLen)))
			{	

// 				if ((strKeyName.GetAt(iLen)>='0'&&strKeyName.GetAt(iLen)<='9'))
// 				{	

					CString strFullPath;
					strFullPath.Format(_T("%s|%s|%s|%s"),strPath,_T("Recent Projects"),strKeyName,strKeyValue);

					g_fnScanFile(g_pMain,iType,strFullPath,0,0);
//				}
			}

			i = 0;

		}
		else
		{
			szKey[i] = szBuffer[n];

			i++;
		}

	}


    retval = TRUE;

clean0:
    if (szBuffer)
    {
        delete[] szBuffer;
        szBuffer = NULL;
    }

    return retval;

}
BOOL CSpecialApp::ScanSection(int iType,LPCTSTR lpcszAppPath,LPCTSTR lpcszIniPath)
{
    BOOL retval = FALSE;
	TCHAR* szBuffer = NULL;
	TCHAR szKey[1024] = {0};
	CString strKey = _T("");
	CString strKeyName = _T("");
	CString strKeyValue = _T("");
	int nBufferSize;

    szBuffer = new TCHAR[65536];
    if (!szBuffer)
        goto clean0;

    nBufferSize = GetPrivateProfileSection(
        lpcszAppPath, 
        szBuffer, 
        65536,
        lpcszIniPath
        );

	for (int n = 0, i = 0; n < nBufferSize; n++)
	{
		if (szBuffer[n] == 0)
		{
			szKey[i] = 0;
			strKey = szKey;

			strKeyName = strKey.Left(strKey.Find('='));
			strKeyValue = strKey.Mid(strKey.Find('=') + 1);
			
			CString strPath;
			strPath.Format(_T("%s|%s|%s|%s"),lpcszIniPath,lpcszAppPath,strKeyName,strKeyValue);

			g_fnScanFile(g_pMain,iType,strPath,0,0);

			i = 0;

		}
		else
		{
			szKey[i] = szBuffer[n];

			i++;
		}

	}

    retval = TRUE;

clean0:
    if (szBuffer)
    {
        delete[] szBuffer;
        szBuffer = NULL;
    }

    return retval;
}

int CSpecialApp::SplitCString1(CString strIn, CSimpleArray<CString>& vec_String, TCHAR division) 
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

BOOL CSpecialApp::ScanXunleiSearch(int iType)
{
	KSearchSoftwareStruct sss;
	std::vector<std::wstring>::iterator it;
	WCHAR szPath[MAX_PATH] = {0};
	sss.pszMainFileName      = TEXT( "Program\\thunder.exe" );
	sss.hRegRootKey          = HKEY_LOCAL_MACHINE;
	sss.pszRegSubKey         = TEXT( "SOFTWARE\\Thunder Network\\ThunderOem\\thunder_backwnd" );
	sss.pszPathValue      	 = TEXT( "dir" );
	sss.bFolder              = TRUE;
	BOOL bRet = FALSE;
	std::wstring str;
	std::wstring strPath;
	std::wstring strTemp;
	bRet = SearchSoftwarePath( &sss, strPath);
	wcscpy_s(szPath, MAX_PATH - 1, strPath.c_str());
	PathRemoveFileSpec(szPath);
	PathRemoveFileSpec(szPath);
	PathAppend(szPath, L"Profiles\\GougouSearch\\history.history");
	strPath  = szPath;
	if (GetFileAttributes(strPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		WCHAR* pEnv = NULL;
		WCHAR  szPath[MAX_PATH] = {0};
		pEnv = _wgetenv(_T("public"));
		if (pEnv != NULL)
		{
			wcscpy(szPath, pEnv);
			PathAppend(szPath, L"Documents\\Thunder Network\\Thunder\\Profiles\\GougouSearch\\history.history");
			strPath = szPath;
		}
		else
		{
			return TRUE;
		}
	}

	g_fnScanFile(g_pMain, BEGINPROC(XUNLEI7_DOWNLOADER), 0, 0, 0);
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"thunder.exe")
		{
			str = L"正在运行，跳过";
			goto clean0;
		}
	}
	str = L"";
	if (m_bScan)
	{
		ModifyTxtData(iType, strPath.c_str());
//		m_appHistory.CommfunFile(KUWOMUSIC_PLAYER, strPath.c_str(), vec_file);
	}
clean0:
	g_fnScanFile(g_pMain, ENDPROC(XUNLEI7_DOWNLOADER), str.c_str(), 0, 0);

	return TRUE;
}

BOOL CSpecialApp::CleanXunleiSearch(LPCTSTR lpcszAppPath)
{
	BYTE* pBuf = NULL;
	FILE* pFile = NULL;		
	FILE* pfile = NULL;
	HRESULT hrRet = E_FAIL;
	DWORD dwRealReadSize = 0;
	CString strInPath;

	CString strFileName;
	CString strSection;
	CString strTemp;
	string szFileName;
	string szSection;
	std::vector<std::wstring> vLogCache;
	std::vector<std::wstring> vLogCacheNew;
	std::vector<std::wstring>::iterator itor;
	CString szTarget;
	int nPos = -1;
	strInPath = lpcszAppPath;
	nPos = strInPath.Find(L'|');

	if (nPos == -1)
	{
		// 		bRet = FALSE;
		// 		goto clean0;
		return FALSE;
	}
	strFileName = strInPath.Left(nPos);
	strSection = strInPath.Right(strInPath.GetLength() - nPos - 1);

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

		nPos = szTarget.Find(strSection.GetBuffer());
		if (nPos != -1)
		{
			continue;
		}	
		vLogCacheNew.push_back(szTarget.GetBuffer());
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
	return TRUE;
}

BOOL CSpecialApp::ModifyTxtData(int nType, const CString strFileName)
{
	BOOL bRet = FALSE;
	CString strFileNameEx = strFileName;
	BYTE* pBuf = NULL;
	HRESULT hrRet = E_FAIL;
	FILE* pFile = NULL;
	DWORD dwRealReadSize = 0;
	std::vector<std::wstring> vLogCache;
	std::vector<std::wstring>::iterator itor;
	CString szTarget;
	int nPos = -1;

	DWORD dwFileSize  = _DoGetFileSizeByFileName(strFileNameEx);
	if (dwFileSize <= 0 || dwFileSize > 1*1024*1024)
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
		CString strOutPut = strFileName;
		strOutPut += L"|";
		strOutPut += szTarget;
		g_fnScanFile(g_pMain, nType,strOutPut, 0, 0);
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