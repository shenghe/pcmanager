#include "maxmonth.h"
#include <algorithm>
#include <vector>
#include <string>
#include "kscbase/kscbase.h"
#include "kcheckappexist.h"
extern std::vector<std::wstring> g_listProcessName;
extern CSimpleArray<int> g_vsNoinstallapp;

CMaxmonthClean::CMaxmonthClean()
{

}

CMaxmonthClean::~CMaxmonthClean()
{


}

BOOL CMaxmonthClean::ScanMaxMonthClean()
{
	if (!_CheckMaxthonExist())
	{
		g_vsNoinstallapp.Add(BROWSERSCLEAN_MAXTHON);
		return TRUE;
	}
	g_fnScanFile(g_pMain,BEGINPROC(BROWSERSCLEAN_MAXTHON),0,0,0);
	std::wstring str;

	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"maxthon.exe")
		{
			str = L"正在运行，跳过";
			goto _exit_;
		}
	}
	str = L"";
	
	{
		CSimpleArray<CString> vec_max;
		CSimpleArray<CString> vec_File;
		CSimpleArray<CString> vec_FileEx;
		vec_File.Add(_T("history.dat"));
		vec_File.Add(_T("lasttab.dat"));
		vec_File.Add(_T("mxundo.dat"));
		WCHAR szFile[MAX_PATH] = {0};
		SHGetSpecialFolderPath(NULL, szFile, CSIDL_APPDATA, FALSE);
		CString strFilePath;
		CString strFilePathEx;
		strFilePath = szFile;
		int nLength = -1;
		nLength = strFilePath.GetLength();
		if (strFilePath.GetAt(nLength-1) != _T('\\'))
		{
			strFilePath.Append(_T("\\"));
		}
		strFilePathEx = strFilePath;
		strFilePath.Append(_T("Maxthon3\\Users\\guest\\History"));
		m_appHistory.CommfunFile(BROWSERSCLEAN_MAXTHON, strFilePath,vec_File);

		strFilePathEx.Append(L"Maxthon3\\Temp\\Webkit\\Cache");
		m_appHistory.CommfunFile(BROWSERSCLEAN_MAXTHON, strFilePathEx, vec_FileEx);
	
		strFilePath = szFile;
		nLength = strFilePath.GetLength();
		if (strFilePath.GetAt(nLength-1) != _T('\\'))
		{
			strFilePath.Append(_T("\\"));
		}
		strFilePath.Append(_T("Maxthon3\\Users"));
		ScanMamonthAccount(strFilePath, _T("History"), vec_File);

		vec_File.RemoveAll();

		strFilePath = szFile;
		nLength = strFilePath.GetLength();
		vec_File.Add(_T("SmartUrl.dat"));
		if (strFilePath.GetAt(nLength-1) != _T('\\'))
		{
			strFilePath.Append(_T("\\"));
		}
		strFilePath.Append(_T("Maxthon3\\Users\\guest\\SmartUrl"));
		m_appHistory.CommfunFile(BROWSERSCLEAN_MAXTHON, strFilePath,vec_File);

		strFilePath = szFile;
		nLength = strFilePath.GetLength();
		if (strFilePath.GetAt(nLength-1) != _T('\\'))
		{
			strFilePath.Append(_T("\\"));
		}
		strFilePath.Append(_T("Maxthon3\\Users"));
		ScanMamonthAccount(strFilePath, _T("SmartUrl"), vec_File);

		vec_File.RemoveAll();

		vec_max.Add(_T("HKEY_CURRENT_USER\\Software\\Maxthon"));
		vec_max.Add(_T("HKEY_CURRENT_USER\\Software\\Maxthon2"));
		vec_max.Add(_T("HKEY_CURRENT_USER\\Software\\Maxthon3"));

		for (int i=0;i<vec_max.GetSize();i++)
		{
			CString strRegPath =vec_max[i];
			HKEY	hRootKey;
			CString strSubKey;
			m_regOpt.CrackRegKey(strRegPath,hRootKey,strSubKey);

			CString strValueName = _T("Folder");
			CString strValueData;
			if (FALSE==m_regOpt.GetDefValue(hRootKey,strSubKey,strValueName,strValueData))
			{
				continue ;
			}

			int iLen = strValueData.GetLength();

			if (iLen<=0)
			{
				continue;
			}

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
				if(strFileFullPath.Find(_T("Maxthon2")) != -1)
				{
					vec_File.Add(_T("Dynamic.ini"));
					vec_File.Add(_T("history2.dat"));
					vec_File.Add(_T("LastSession.ini"));
					vec_File.Add(_T("TmpSession.ini"));
					vec_File.Add(_T("Session.ini"));
					ScanMamonthAccount(strFileFullPath, _T("Data"), vec_File);
				}

				strFileFullPath.Append(_T("Config\\Dynamic.ini"));
				hFindFile = FindFirstFile(strFileFullPath.GetBuffer(), &fd);
				strFileFullPath.ReleaseBuffer();
				if(hFindFile != INVALID_HANDLE_VALUE)
				{
					g_fnScanFile(g_pMain,BROWSERSCLEAN_MAXTHON,strFileFullPath,fd.nFileSizeLow,fd.nFileSizeHigh);
					::FindClose(hFindFile); 
				}
				strFileFullPath = strValueData;
				strFileFullPath.Append(_T("SharedAccount\\Data\\Dynamic.ini"));
				hFindFile = FindFirstFile(strFileFullPath.GetBuffer(), &fd);
				strFileFullPath.ReleaseBuffer();
				if(hFindFile != INVALID_HANDLE_VALUE)
				{
					g_fnScanFile(g_pMain,BROWSERSCLEAN_MAXTHON,strFileFullPath,fd.nFileSizeLow,fd.nFileSizeHigh);
					::FindClose(hFindFile); 
				}
				strFileFullPath = strValueData;
				strFileFullPath.Append(_T("SharedAccount\\Data\\history2.dat"));
				hFindFile = FindFirstFile(strFileFullPath.GetBuffer(), &fd);
				strFileFullPath.ReleaseBuffer();
				if(hFindFile != INVALID_HANDLE_VALUE)
				{
					g_fnScanFile(g_pMain,BROWSERSCLEAN_MAXTHON,strFileFullPath,fd.nFileSizeLow,fd.nFileSizeHigh);
					::FindClose(hFindFile); 
				}
				strFileFullPath = strValueData;
				strFileFullPath.Append(_T("SharedAccount\\Data\\LastSession.ini"));
				hFindFile = FindFirstFile(strFileFullPath.GetBuffer(), &fd);
				strFileFullPath.ReleaseBuffer();
				if(hFindFile != INVALID_HANDLE_VALUE)
				{
					g_fnScanFile(g_pMain,BROWSERSCLEAN_MAXTHON,strFileFullPath,fd.nFileSizeLow,fd.nFileSizeHigh);
					::FindClose(hFindFile); 
				}
				strFileFullPath = strValueData;
				strFileFullPath.Append(_T("SharedAccount\\Data\\TmpSession.ini"));
				hFindFile = FindFirstFile(strFileFullPath.GetBuffer(), &fd);
				strFileFullPath.ReleaseBuffer();
				if(hFindFile != INVALID_HANDLE_VALUE)
				{
					g_fnScanFile(g_pMain,BROWSERSCLEAN_MAXTHON,strFileFullPath,fd.nFileSizeLow,fd.nFileSizeHigh);
					::FindClose(hFindFile); 
				}
				strFileFullPath = strValueData;
				strFileFullPath.Append(_T("SharedAccount\\Data\\Session.ini"));
				hFindFile = FindFirstFile(strFileFullPath.GetBuffer(), &fd);
				strFileFullPath.ReleaseBuffer();
				if(hFindFile != INVALID_HANDLE_VALUE)
				{
					g_fnScanFile(g_pMain,BROWSERSCLEAN_MAXTHON,strFileFullPath,fd.nFileSizeLow,fd.nFileSizeHigh);
					::FindClose(hFindFile); 
				}

				strFileFullPath = strValueData;
				strFileFullPath.Append(_T("SharedAccount\\Config\\Config.ini"));
				ScanMaxmonthIni(strFileFullPath, L"TypedUrls");
			}

		}

	}
	
_exit_:
	g_fnScanFile(g_pMain,ENDPROC(BROWSERSCLEAN_MAXTHON),str.c_str(),0,0);

	return TRUE;
}

BOOL CMaxmonthClean::ScanMamonthAccount(CString strFilePath, CString strSub, CSimpleArray<CString> vec_file)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA wfd = {0};
	BOOL bRet = FALSE;
	CString strFile;
	int nLen = -1;
	nLen = strFilePath.GetLength();
	if (strFilePath.GetAt(nLen - 1) != _T('\\'))
	{
		strFilePath.Append(_T("\\"));
	}
	strFile = strFilePath;
	strFilePath.Append(_T("*@*.com"));

	hFile = FindFirstFile(strFilePath, &wfd);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		bRet = FALSE;
		goto clean0;
	}
	do 
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			strFile.Append(wfd.cFileName);
			nLen = strFile.GetLength();
			if (strFile.GetAt(nLen - 1) != _T('\\'))
			{
				strFile.Append(_T("\\"));
			}
			strFile.Append(strSub);
			m_appHistory.CommfunFile(BROWSERSCLEAN_MAXTHON, strFile, vec_file);
		}
	} while (FindNextFile(hFile, &wfd));
clean0:
	if (hFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	return bRet;
}

BOOL CMaxmonthClean::ScanMaxmonthIni(CString strPath, CString strSection)
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
		strSection, 
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

			CString strOut;
			strOut.Format(_T("%s|%s|%s|%s"),strPath,strSection,strKeyName,strKeyValue);

			g_fnScanFile(g_pMain,BROWSERSCLEAN_MAXTHON,strOut,0,0);

			i = 0;
		}
		else
		{
			szKey[i] = szBuffer[n];

			i++;
		}
	}

clean0:
	if (szBuffer)
	{
		delete[] szBuffer;
		szBuffer = NULL;
	}

	return retval;
}
BOOL CMaxmonthClean::CleanMaxmonthIni(CString strFullInfo)
{
	CString strPath = strFullInfo;

	std::vector<std::wstring> vec_String;

	if (SplitCString(strPath.GetBuffer(),vec_String,'|') < 3)
	{
		return FALSE;
	}

	if(vec_String.size()<3)
		return FALSE;

	if (TRUE !=WritePrivateProfileStruct(vec_String[1].c_str(),vec_String[2].c_str(), NULL, 0,vec_String[0].c_str()))
	{
		DWORD dwErrCode = GetLastError();
		if (dwErrCode == ERROR_FILE_NOT_FOUND)	//系统找不到指定文件
		{
			return TRUE;
		}
	}

	return TRUE;
}