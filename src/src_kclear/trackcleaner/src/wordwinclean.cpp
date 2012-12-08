#include "wordwinclean.h"
#include "kscbase/kscbase.h"
#include <algorithm>
#include <vector>
#include <string>
#include "kcheckappexist.h"
extern std::vector<std::wstring> g_listProcessName;
extern CSimpleArray<int> g_vsNoinstallapp;

CWordWinClean::CWordWinClean()
{

}

CWordWinClean::~CWordWinClean()
{

}

BOOL CWordWinClean::ScanWordWinClean()
{	
	BOOL bRetCheck = _CheckTheworldExist();
	if (!bRetCheck)
	{
		g_vsNoinstallapp.Add(BROWSERSCLEAN_WORDWIN);
		return bRetCheck;
	}
	g_fnScanFile(g_pMain,BEGINPROC(BROWSERSCLEAN_WORDWIN),0,0,0);

	std::wstring str;

	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"theworld.exe")
		{
			str = L"正在运行，跳过";
			goto Exit;
		}
	}
	str = L"";
	
	if (m_bScan ==TRUE)
	{
        CString strReg;
        CSimpleArray<CString> vec_reg;
        HKEY	hRootKey;
        CString strSubKey;
        CString strValueName = _T("Path");
        CString strValueData;
        CString strRegPath = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\TheWorld.exe");
        CString strTemp;

		g_fnScanReg(g_pMain, BEGINPROC(BROWSERSCLEAN_WORDWIN), 0, 0, 0);
		
		strReg = L"HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\TypedURLs";
		m_appHistory.CommfunReg(BROWSERSCLEAN_WORDWIN, strReg, vec_reg);
		g_fnScanReg(g_pMain, ENDPROC(BROWSERSCLEAN_WORDWIN), 0, 0, 0);

		
		m_regOpt.CrackRegKey(strRegPath,hRootKey,strSubKey);
		m_regOpt.GetDefValue(hRootKey,strSubKey,strValueName,strValueData);

		int iLen = strValueData.GetLength();
		
		if (iLen <= 0)
			goto Exit;

		if (strValueData.GetAt(iLen-1) != _T('\\'))
		{
			strValueData.Append(_T("\\"));
		}


        strTemp = strValueData + _T("TheWorld.ini");
        if (GetFileAttributes(strTemp) == INVALID_FILE_ATTRIBUTES)
        {
            TCHAR szAppData[MAX_PATH] = { 0 };
            SHGetSpecialFolderPath(NULL, szAppData, CSIDL_APPDATA, FALSE);

            PathAppend(szAppData, _T("TheWorld"));
            PathAddBackslash(szAppData);
            strValueData = szAppData;

            if (GetFileAttributes(strValueData) == INVALID_FILE_ATTRIBUTES)
                strValueData = _T("");
        }

		if (strValueData != _T(""))
		{

			CString strFileFullPath;
			CString strWorldIni;
			strWorldIni = strValueData;

			strFileFullPath = strValueData;
			CSimpleArray<CString> vec_file;
			vec_file.Add(_T("history.dat"));
			vec_file.Add(_T("user.dat"));
			strFileFullPath.Append(_T("Data"));
			m_appHistory.CommfunFile(BROWSERSCLEAN_WORDWIN, strFileFullPath, vec_file);
			strWorldIni.Append(_T("TheWorld.ini"));
			ScanWorldIni(strWorldIni, L"RecentUrl");
			ScanWorldIni(strWorldIni, L"OldUrl");
			ScanWorldIni(strWorldIni, L"LastUrl");
 
		}

	}

Exit:
	g_fnScanFile(g_pMain,ENDPROC(BROWSERSCLEAN_WORDWIN),str.c_str(),0,0);

	return TRUE;
}

BOOL CWordWinClean::ScanWorldIni(CString strPath, CString strSection)
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

			g_fnScanFile(g_pMain,BROWSERSCLEAN_WORDWIN,strOut,0,0);

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
BOOL CWordWinClean::CleanWorldIni(CString strFullInfo)
{
	CString strPath = strFullInfo;

	std::vector<std::wstring> vec_String;

	if (SplitCString(strPath.GetBuffer(),vec_String,'|') < 3)
	{
		return FALSE;
	}

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


BOOL	CWordWinClean::ScanWordWinUser()
{	
	BOOL bRetCheck = _CheckTheworldExist();
	if (!bRetCheck)
	{
		g_vsNoinstallapp.Add(WORDWIN_USER);
		return bRetCheck;
	}
	g_fnScanFile(g_pMain,BEGINPROC(WORDWIN_USER),0,0,0);

	std::wstring str;

	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"theworld.exe")
		{
			str = L"正在运行，跳过";
			goto Exit;
		}
	}
	str = L"";
	static BOOL bFlag = TRUE;
	if (bFlag)
	{
		str = L"可以清理";
		bFlag = FALSE;
	}
Exit:
	g_fnScanFile(g_pMain,ENDPROC(WORDWIN_USER),str.c_str(),0,0);

	return TRUE;
}

BOOL CWordWinClean::CleanWordWinUser(CString str)
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
	::ShellExecute(NULL, _T("open"), _T("rundll32.exe"),  _T("InetCpl.cpl,ClearMyTracksByProcess 32"), NULL, SW_HIDE);
clean0:
	if (hMoudle)
	{
		FreeLibrary(hMoudle);
		hMoudle = NULL;
	}
	return TRUE;
}