#include "stdafx.h"
#include "ieclean.h"
#include   <shlobj.h>
#include   <UrlHist.h>
#include "kscbase/kscconv.h"
#include <string>
#include <algorithm>
#include "kcheckappexist.h"
extern std::vector<std::wstring> g_listProcessName;
extern CSimpleArray<int> g_vsNoinstallapp;
using namespace std;

CIEClean::CIEClean()
{
	//获得登陆用户名
	DWORD dwLoginSize=MAX_PATH;
	memset(m_szLoginName,0,sizeof(TCHAR)*MAX_PATH);
	//GetUserName(m_szLoginName,&dwLoginSize);

	for (int i = 0; i < 10000; i++)
	{
		m_bFlagPass[10000] = FALSE;
		m_bFlagForm[10000] = FALSE;
	}
	m_nFlags = 0;
	m_bScan = TRUE; 
}

CIEClean::~CIEClean()
{

}


BOOL CIEClean::myEnumerateFile(LPCTSTR lpFileOrPath, void* _pUserData,LARGE_INTEGER filesize)
{	
	
	//过滤所有desk.ini文件
	//CString strPath = lpFileOrPath;
	//if (-1!=strPath.Find(_T("desktop.ini")))
	//{
	//	return TRUE;
	//}


	USERDATA* pUserData = (USERDATA*) _pUserData;
	CIEClean* pIEClean = (CIEClean*) pUserData->pObject;
	
	CString strFilePath = lpFileOrPath;

    strFilePath.MakeLower();
	
	//判断是否是清楚index.dat
	if (pUserData->iType != IECLEAN_INDEXDATA)
	{
		if ( -1!= strFilePath.Find(_T("index.dat")) ) //忽略index.data
		{
			return TRUE;
		}
	}
	


	switch (pUserData->iType)
	{
	case IECLEAN_TEMPFOLDER:
		return pIEClean->TempFoldFolder( lpFileOrPath, filesize);
	case IECLEAN_VISITHOST:
		return pIEClean->VisitHost (lpFileOrPath, filesize);
	case IECLEAN_COOKIE:
		return pIEClean->IECookie(lpFileOrPath, filesize);
	case IECLEAN_INDEXDATA:
		return pIEClean->CleanIndex(lpFileOrPath, filesize);

	}
	
	return TRUE;
}

BOOL CIEClean::myEnumerateReg(
							  HKEY rootKey,LPCTSTR lpItemPath,
							  LPCTSTR lpName,DWORD dwValueNameSize,
							  LPBYTE lpValue ,DWORD dwValueSize,DWORD dwType,
							  void* _pUserData
							  )
{	
	
	if (lpName==NULL&&lpValue==NULL)
	{
		return TRUE;
	}


	USERDATA* pUserData = (USERDATA*) _pUserData;
	CIEClean* pIEClean = (CIEClean*) pUserData->pObject;
	
	LPTSTR lpValueTmp = (LPTSTR) lpValue;
	LPTSTR lpNameTmp = (LPTSTR) lpName;

	

	switch (dwType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
		{	
			switch (pUserData->iType)
			{
			case IECLEAN_ADDRBSR:
					return pIEClean->IEAddrBar(lpNameTmp,lpValueTmp);
			}

		}	
	case REG_BINARY:			//除REG_SZ和REG_EXPAND_SZ其他值均忽略
		break;
	case REG_DWORD_LITTLE_ENDIAN:
	case REG_DWORD_BIG_ENDIAN:	
		{
			switch (pUserData->iType)
			{
			case IECLEAN_PASSWORD:
				return pIEClean->IEPassword(rootKey,lpItemPath,lpNameTmp,lpValueTmp);
			case IECLEAN_SAVEFROM:
				return pIEClean->IEFrom(rootKey,lpItemPath,lpNameTmp,lpValueTmp);
			}
		}
	case REG_LINK:									
	case REG_MULTI_SZ:
	case REG_NONE:
	case REG_QWORD_LITTLE_ENDIAN:
		break;
	}
	

	return TRUE;
}

/************************************************************************/
/* 扫描临时文件夹
/************************************************************************/

BOOL CIEClean::ScanIETempFolder()
{	
	g_fnScanFile(g_pMain,BEGINPROC(IECLEAN_TEMPFOLDER),0,0,0);
	//拼接临时文件夹路径
	WCHAR* pEnv;
	pEnv = _wgetenv(_T("USERPROFILE"));
	if (pEnv !=NULL)
	{	
		WCHAR szPath[MAX_PATH] = {0};
		CString strIETempFolder;
		SHGetSpecialFolderPath(NULL, szPath, CSIDL_HISTORY, FALSE);
		strIETempFolder = szPath;
		ScanIETempFolder(strIETempFolder.GetBuffer());

		memset(szPath,0,MAX_PATH);
		SHGetSpecialFolderPath(NULL, szPath, CSIDL_INTERNET_CACHE, FALSE);
		strIETempFolder = szPath;

		ScanIETempFolder(strIETempFolder.GetBuffer());
	
	}

	g_fnScanFile(g_pMain,ENDPROC(IECLEAN_TEMPFOLDER),0,0,0);
	
	return TRUE;
}

BOOL CIEClean::ScanIETempFolder(LPCTSTR lpFileOrFolderPath)
{	
	CString strIeTempFolder = lpFileOrFolderPath;

	try
	{	
		USERDATA userData;
		userData.pObject= this;
		userData.iType = IECLEAN_TEMPFOLDER;

		m_fileOpt.DoFileEnumeration(strIeTempFolder.GetBuffer(),TRUE,TRUE,myEnumerateFile,&userData);
	}
	catch(...)
	{	
		OutputDebugString(_T("崩溃:ScanIETempFolder\n"));
		return FALSE; 
	}

	return FALSE;
}


BOOL CIEClean::TempFoldFolder(LPCTSTR lpFileOrPath,LARGE_INTEGER filesize)
{	
	if (m_bScan==FALSE)
		return FALSE;
	
	return g_fnScanFile(g_pMain,IECLEAN_TEMPFOLDER,lpFileOrPath,filesize.LowPart,filesize.LowPart);
}

/************************************************************************/
/* 扫描访问网址
/************************************************************************/
BOOL CIEClean::ScanVisitHost()
{	
	g_fnScanFile(g_pMain,BEGINPROC(IECLEAN_VISITHOST),0,0,0);
	//拼接临时文件夹路径
	WCHAR* pEnv;
	pEnv = _wgetenv(_T("USERPROFILE"));
	if (pEnv !=NULL)
	{	
		CString strIETempFolder;
		strIETempFolder.Format(_T("%s\\Local Settings\\History"),pEnv);
		ScanVisitHost(strIETempFolder.GetBuffer());

		strIETempFolder.Format(_T("%s\\Local Settings\\Application Data\\Microsoft\\Windows\\History"),pEnv);
		ScanVisitHost(strIETempFolder.GetBuffer());
	}


	g_fnScanFile(g_pMain,ENDPROC(IECLEAN_VISITHOST),0,0,0);

	return TRUE;

}

BOOL CIEClean::ScanVisitHost(LPCTSTR lpFileOrFolderPath)
{	
	CString strFileOrFolderPath = lpFileOrFolderPath;

	try
	{	
		USERDATA userData;
		userData.pObject= this;
		userData.iType = IECLEAN_VISITHOST;

		m_fileOpt.DoFileEnumeration(strFileOrFolderPath.GetBuffer(),TRUE,TRUE,myEnumerateFile,&userData);

	}
	catch(...)
	{	
		OutputDebugString(_T("崩溃:ScanVisitHost"));
		return FALSE; 
	}

	return FALSE;
}

BOOL CIEClean::VisitHost(LPCTSTR lpFileOrPath,LARGE_INTEGER filesize)
{
	return g_fnScanFile(g_pMain,IECLEAN_VISITHOST,lpFileOrPath,filesize.LowPart,filesize.LowPart);;
}

/************************************************************************/
//IE Cookie                                                                      
/************************************************************************/

//IE Cookie
BOOL CIEClean::ScanIECookie(int nType)
{	
	//IE Cookie
	std::wstring str;
	std::vector<std::wstring>::iterator it;
	if (nType == MAXTHON_COOKIES)
	{
		if (!_CheckMaxthonExist())
		{
			g_vsNoinstallapp.Add(MAXTHON_COOKIES);
			return TRUE;
		}
	}
	else if (nType == THEWORLD_COOKIES)
	{
		if (!_CheckTheworldExist())
		{
			g_vsNoinstallapp.Add(THEWORLD_COOKIES);
			return TRUE;
		}
	}
	g_fnScanFile(g_pMain,BEGINPROC(nType),0,0,0);

	if (nType == IECLEAN_COOKIE)
	{
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"iexplore.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}
		str = L"";
	}

	if (nType == MAXTHON_COOKIES)
	{
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
	}

	if (nType == THEWORLD_COOKIES)
	{
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"theworld.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}
		str = L"";
	}

	/*WCHAR* pEnv;
	pEnv = _wgetenv(_T("USERPROFILE"));*/

	WCHAR pEnv[MAX_PATH] = {0};
	SHGetSpecialFolderPath(NULL, pEnv, CSIDL_COOKIES, FALSE);

	if (pEnv !=NULL)
	{	
		CString strPath;
		CSimpleArray<CString> vec_string;
		strPath = pEnv;
		m_appHistory.CommfunFile(nType, strPath, vec_string);
	}


	if (nType == MAXTHON_COOKIES)
	{
		ScanMaxthon3Cookies();
	}

_exit_:
	g_fnScanFile(g_pMain,ENDPROC(nType),str.c_str(),0,0);
	
	return TRUE;
}

BOOL CIEClean::ScanIECookie(LPCTSTR lpszIECookiePath)
{
	CString strIETempFolder = lpszIECookiePath;

	try
	{	
		USERDATA userData;
		userData.pObject= this;
		userData.iType = IECLEAN_COOKIE;

		m_fileOpt.DoFileEnumeration(strIETempFolder,TRUE,TRUE,myEnumerateFile,&userData);
	}
	catch(...)
	{	
			OutputDebugString(_T("崩溃:ScanIECookie"));
		return FALSE; 
	}

	return FALSE;

}

BOOL CIEClean::IECookie(LPCTSTR lpFileOrPath,LARGE_INTEGER filesize)
{	
	if (m_bScan==FALSE)
		return FALSE;
	
	return g_fnScanFile(g_pMain,IECLEAN_COOKIE,lpFileOrPath,filesize.LowPart,filesize.LowPart);;
}

BOOL CIEClean::ScanMaxthon3Cookies()
{
	BOOL bRet = FALSE;
	WCHAR pEnv[MAX_PATH] ={0};
	CString strPath = _T("");
	CSimpleArray<CString> vec_string;
	WIN32_FIND_DATA findData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	BOOL bFind = TRUE;

	SHGetSpecialFolderPath(NULL, pEnv, CSIDL_APPDATA, FALSE);

	strPath += pEnv;
	strPath += L"\\Maxthon3\\Users\\*.*";
	// 	m_appHistory.CommfunFile(MAXTHON_COOKIES, strPath, vec_string);

	hFind = FindFirstFile(strPath.GetBuffer(), &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		goto _exit_;
	}

	while (bFind)
	{
		if (wcscmp(findData.cFileName, _T(".")) == 0 ||
			wcscmp(findData.cFileName, _T("..")) == 0)
		{
			bFind = FindNextFile(hFind, &findData);
			continue;
		}
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			CString str = pEnv; 
			str += L"\\Maxthon3\\Users\\";
			str += findData.cFileName;
			str += "\\Cookie";
			str.MakeLower();
			m_appHistory.CommfunFile(MAXTHON_COOKIES, str, vec_string);
		}

		bFind = FindNextFile(hFind, &findData);
	}
	bRet = TRUE;

_exit_:
	if (hFind != INVALID_HANDLE_VALUE)
	{
		::FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
	}
	return bRet;
}

/************************************************************************/
//IE 地址栏                                                                     
/************************************************************************/

BOOL CIEClean::ScanIEAddrBar()
{	
	std::wstring str = L"";
	std::vector<std::wstring>::iterator it;
	CString strIEAddrBar = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\TypedURLs");
	g_fnScanReg(g_pMain,BEGINPROC(IECLEAN_ADDRBSR),0,0,0);
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"iexplore.exe")
		{
			str = L"正在运行，跳过";
			goto _exit_;
		}
	}
	str = L"";
	ScanIEAddrBar(strIEAddrBar.GetBuffer());

_exit_:
	g_fnScanReg(g_pMain,ENDPROC(IECLEAN_ADDRBSR),str.c_str(),0,0);
	
	return TRUE;
}

BOOL CIEClean::ScanIEAddrBar(LPCTSTR lpszIEAddrBarPath)
{
	CString strIEAddrBar = lpszIEAddrBarPath;

	try
	{	
		USERDATA userData;
		userData.pObject= this;
		userData.iType = IECLEAN_ADDRBSR;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strIEAddrBar,hRootKey,strSubKey);

		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),myEnumerateReg,&userData);


	}
	catch (...)
	{

	}

	return TRUE;
}

BOOL CIEClean::IEAddrBar(LPCTSTR lpValueName,LPCTSTR lpValueData)
{	
	if (m_bScan==FALSE)
		return FALSE;

	return g_fnScanReg(g_pMain,IECLEAN_ADDRBSR,_T("HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\TypedURLs"),
		lpValueName,
		lpValueData
		);
}

BOOL	CIEClean::ScanIEPass(int nType)
{	

	std::wstring str;
	std::vector<std::wstring>::iterator it;
	if (nType == MAXTHON_FORM)
	{
		if (!_CheckMaxthonExist())
		{
			g_vsNoinstallapp.Add(MAXTHON_FORM);
			return TRUE;
		}
	}
	if (nType == THEWORLD_FORM)
	{
		if (!_CheckTheworldExist())
		{
			g_vsNoinstallapp.Add(THEWORLD_FORM);
			return TRUE;
		}
	}
	g_fnScanFile(g_pMain,BEGINPROC(nType),0,0,0);

	if (nType == IECLEAN_PASSWORD)
	{
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"iexplore.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}
		str = L"";
	}

	if (nType == MAXTHON_FORM)
	{
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
	}

	if (nType == THEWORLD_FORM)
	{
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"theworld.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}
		str = L"";
	}

//	CString str;
	if (!m_bFlagPass[nType])
	{
		str = L"可以清理";
		m_bFlagPass[nType] = TRUE;
	}
_exit_:
	g_fnScanFile(g_pMain,ENDPROC(nType),str.c_str(),0,0);
	return TRUE;
}

BOOL	CIEClean::ScanIEPassword(LPCTSTR lpszIEAddrBarPath)
{	
	CString  strIEAddrBar = lpszIEAddrBarPath;

	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = IECLEAN_PASSWORD;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strIEAddrBar,hRootKey,strSubKey);
		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),myEnumerateReg,&userData);


	}
	catch (...)
	{
		return FALSE;
	}
	

	return TRUE;
}

BOOL	CIEClean::IEPassword(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{
	if (m_bScan==FALSE)
		return FALSE;
	
	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

	return g_fnScanReg(g_pMain,IECLEAN_PASSWORD,
		strRegFullPath.GetBuffer(),
		lpValueName,
		lpValueData
		);
}

//

BOOL	CIEClean::ScanIEFrom(int nType)
{	
	std::wstring str;
	std::vector<std::wstring>::iterator it;
	if (nType == MAXTHON_PASS)
	{
		if (!_CheckMaxthonExist())
		{
			g_vsNoinstallapp.Add(MAXTHON_PASS);
			return TRUE;
		}
	}
	if (nType == THEWORLD_PASS)
	{
		if (!_CheckTheworldExist())
		{
			g_vsNoinstallapp.Add(THEWORLD_PASS);
			return TRUE;
		}
	}
	g_fnScanFile(g_pMain,BEGINPROC(nType),0,0,0);

	if (nType == IECLEAN_SAVEFROM)
	{
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"iexplore.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}
		str = L"";
	}

	if (nType == MAXTHON_PASS)
	{
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
	}

	if (nType == THEWORLD_PASS)
	{
		for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
		{
			str = *it;
			transform(str.begin(), str.end(), str.begin(), towlower);
			if (str == L"theworld.exe")
			{
				str = L"正在运行，跳过";
				goto _exit_;
			}
		}
		str = L"";
	}
// 	CString str = L"";
	if (!m_bFlagForm[nType])
	{
		str = L"可以清理";
		m_bFlagForm[nType] = TRUE;
	}
_exit_:
	g_fnScanFile(g_pMain,ENDPROC(nType),str.c_str(),0,0);
	return TRUE;
}

BOOL	CIEClean::ScanIEFrom(LPCTSTR lpszIEAddrBarPath)
{
	CString  strIEAddrBar = lpszIEAddrBarPath;

	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = IECLEAN_SAVEFROM;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strIEAddrBar,hRootKey,strSubKey);
		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),myEnumerateReg,&userData);

	}
	catch (...)
	{
		return FALSE;
	}


	return TRUE;
}

BOOL CIEClean::IEFrom(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{
	if (m_bScan==FALSE)
		return FALSE;

	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

	return g_fnScanReg(g_pMain,IECLEAN_SAVEFROM,
		strRegFullPath.GetBuffer(),
		lpValueName,
		lpValueData
		);
}

BOOL CIEClean::IEFrom_Clean()
{	
	// 清除表单自动完成历史记录
	CString sKey;
	DWORD dwRet;

	if (IsWindows2k() || IsWindowsNT())//先判断系统
	{
		CString sBaseKey;
		SECURITY_DESCRIPTOR NewSD;
		BYTE* pOldSD;
		PACL pDacl = NULL;
		PSID pSid = NULL;
        CString strSid;
		if (GetUserSid(&pSid))
		{
			//get the hiden key name
			GetSidString(pSid, strSid);

			sKey = _T("Software\\Microsoft\\Protected Storage System Provider\\");
			sKey += strSid;

			//get old SD
			sBaseKey = sKey;
			GetOldSD(HKEY_CURRENT_USER, sBaseKey, &pOldSD);

			//set new SD and then clear
			if (CreateNewSD(pSid, &NewSD, &pDacl))
			{
				RegSetPrivilege(HKEY_CURRENT_USER, sKey, &NewSD, FALSE);

				sKey += _T("\\Data");
				RegSetPrivilege(HKEY_CURRENT_USER, sKey, &NewSD, FALSE);

				sKey += _T("\\e161255a-37c3-11d2-bcaa-00c04fd929db");
				RegSetPrivilege(HKEY_CURRENT_USER, sKey, &NewSD, TRUE);

				dwRet = SHDeleteKey(HKEY_CURRENT_USER, sKey);
			}

			if (pDacl != NULL)
				HeapFree(GetProcessHeap(), 0, pDacl);

			//restore old SD
			if (pOldSD)
			{
				RegSetPrivilege(HKEY_CURRENT_USER, sBaseKey, 
					(SECURITY_DESCRIPTOR*)pOldSD, FALSE);
				delete pOldSD;
			}
		}
		if (pSid)
			HeapFree(GetProcessHeap(), 0, pSid);
	}

	//win9x
	DWORD dwSize = MAX_PATH;
	TCHAR szUserName[MAX_PATH];
	GetUserName(szUserName, &dwSize);

	sKey = _T("Software\\Microsoft\\Protected Storage System Provider\\");
	sKey += szUserName;
	sKey += _T("\\Data\\e161255a-37c3-11d2-bcaa-00c04fd929db");
	dwRet = SHDeleteKey(HKEY_LOCAL_MACHINE, sKey);

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

BOOL CIEClean::IsWindowsNT()
{
	BOOL bRet = FALSE;
	BOOL bOsVersionInfoEx;
	OSVERSIONINFOEX osvi;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure,
	// If that fails, try using the OSVERSIONINFO structure.
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
			return bRet;
	}

	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion <= 4)
	{
		bRet = TRUE;
	}

	return bRet;
}

BOOL CIEClean::IsWindows2k()
{
	BOOL bRet = FALSE;
	BOOL bOsVersionInfoEx;
	OSVERSIONINFOEX osvi;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure,
	// If that fails, try using the OSVERSIONINFO structure.
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
			return bRet;
	}

	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 5)
	{
		bRet = TRUE;
	}

	return bRet;
}

BOOL CIEClean::GetUserSid(PSID* ppSid)
{
	HANDLE hToken;
	BOOL bRes;
	DWORD cbBuffer, cbRequired;
	PTOKEN_USER pUserInfo;

	// The User's SID can be obtained from the process token
	bRes = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
	if (FALSE == bRes)
	{
		return FALSE;
	}

	// Set buffer size to 0 for first call to determine
	// the size of buffer we need.
	cbBuffer = 0;
	bRes = GetTokenInformation(hToken, TokenUser, NULL, cbBuffer, &cbRequired);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		return FALSE;
	}

	// Allocate a buffer for our token user data
	cbBuffer = cbRequired;
	pUserInfo = (PTOKEN_USER) HeapAlloc(GetProcessHeap(), 0, cbBuffer);
	if (NULL == pUserInfo)
	{
		return FALSE;
	}

	// Make the "real" call
	bRes = GetTokenInformation(hToken, TokenUser, pUserInfo, cbBuffer, &cbRequired);
	if (FALSE == bRes) 
	{
		return FALSE;
	}

	// Make another copy of the SID for the return value
	cbBuffer = GetLengthSid(pUserInfo->User.Sid);

	*ppSid = (PSID) HeapAlloc(GetProcessHeap(), 0, cbBuffer);
	if (NULL == *ppSid)
	{
		return FALSE;
	}

	bRes = CopySid(cbBuffer, *ppSid, pUserInfo->User.Sid);
	if (FALSE == bRes)
	{
		HeapFree(GetProcessHeap(), 0, *ppSid);
		return FALSE;
	}

	bRes = HeapFree(GetProcessHeap(), 0, pUserInfo);

	return TRUE;
}


void CIEClean::GetSidString(PSID pSid, CString& strSid)
{
	//convert SID to string
    TCHAR szBuffer[256];
	SID_IDENTIFIER_AUTHORITY *psia = ::GetSidIdentifierAuthority( pSid );
	DWORD dwTopAuthority = psia->Value[5];
	_stprintf_s(szBuffer, 256, _T("S-1-%lu"), dwTopAuthority);

	TCHAR szTemp[32];
	int iSubAuthorityCount = *(GetSidSubAuthorityCount(pSid));
	for (int i = 0; i<iSubAuthorityCount; i++) 
	{
		DWORD dwSubAuthority = *(GetSidSubAuthority(pSid, i));
		_stprintf_s(szTemp, 32, _T("%lu"), dwSubAuthority);
		_tcscat(szBuffer, _T("-"));
		_tcscat(szBuffer, szTemp);
	}

    strSid = szBuffer;
}


BOOL CIEClean::GetOldSD(HKEY hKey, LPCTSTR pszSubKey, BYTE** pSD)
{
	BOOL bRet = FALSE;
	HKEY hNewKey = NULL;
	DWORD dwSize = 0;
	LONG lRetCode;
	*pSD = NULL;

	lRetCode = RegOpenKeyEx(hKey, pszSubKey, 0, READ_CONTROL, &hNewKey);
	if(lRetCode != ERROR_SUCCESS)
		goto cleanup;

	lRetCode = RegGetKeySecurity(hNewKey, 
		(SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, *pSD, &dwSize);
	if (lRetCode == ERROR_INSUFFICIENT_BUFFER)
	{
		*pSD = new BYTE[dwSize];
		lRetCode = RegGetKeySecurity(hNewKey, 
			(SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, *pSD, &dwSize);
		if(lRetCode != ERROR_SUCCESS)
		{
			delete *pSD;
			*pSD = NULL;
			goto cleanup;
		}
	}
	else if (lRetCode != ERROR_SUCCESS)
		goto cleanup;

	bRet = TRUE; // indicate success

cleanup:
	if (hNewKey)
	{
		RegCloseKey(hNewKey);
	}
	return bRet;
}


BOOL CIEClean::CreateNewSD(PSID pSid, SECURITY_DESCRIPTOR* pSD, PACL* ppDacl)
{
	BOOL bRet = FALSE;
	PSID pSystemSid = NULL;
	SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
	ACCESS_ALLOWED_ACE* pACE = NULL;
	DWORD dwAclSize;
	DWORD dwAceSize;

	// prepare a Sid representing local system account
	if(!AllocateAndInitializeSid(&sia, 1, SECURITY_LOCAL_SYSTEM_RID,
		0, 0, 0, 0, 0, 0, 0, &pSystemSid))
	{
		goto cleanup;
	}

	// compute size of new acl
	dwAclSize = sizeof(ACL) + 2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) + 
		GetLengthSid(pSid) + GetLengthSid(pSystemSid);

	// allocate storage for Acl
	*ppDacl = (PACL)HeapAlloc(GetProcessHeap(), 0, dwAclSize);
	if(*ppDacl == NULL)
		goto cleanup;

	if(!InitializeAcl(*ppDacl, dwAclSize, ACL_REVISION))
		goto cleanup;

	//    if(!AddAccessAllowedAce(pDacl, ACL_REVISION, KEY_WRITE, pSid))
	//		goto cleanup;

	// add current user
	dwAceSize = sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(pSid); 
	pACE = (ACCESS_ALLOWED_ACE *)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, dwAceSize);

	pACE->Mask = KEY_READ | KEY_WRITE | KEY_ALL_ACCESS;
	pACE->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
	pACE->Header.AceFlags = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
	pACE->Header.AceSize = (WORD)dwAceSize;

	memcpy(&pACE->SidStart, pSid, GetLengthSid(pSid));
	if (!AddAce(*ppDacl, ACL_REVISION, MAXDWORD, pACE, dwAceSize))
		goto cleanup;

	// add local system account
	HeapFree(GetProcessHeap(), 0, pACE);
	pACE = NULL;
	dwAceSize = sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(pSystemSid);
	pACE = (ACCESS_ALLOWED_ACE *)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, dwAceSize);

	pACE->Mask = KEY_READ | KEY_WRITE | KEY_ALL_ACCESS;
	pACE->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
	pACE->Header.AceFlags = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
	pACE->Header.AceSize = (WORD)dwAceSize;

	memcpy(&pACE->SidStart, pSystemSid, GetLengthSid(pSystemSid));
	if (!AddAce(*ppDacl, ACL_REVISION, MAXDWORD, pACE, dwAceSize))
		goto cleanup;

	if(!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
		goto cleanup;

	if(!SetSecurityDescriptorDacl(pSD, TRUE, *ppDacl, FALSE))
		goto cleanup;

	bRet = TRUE; // indicate success

cleanup:
	if(pACE != NULL)
		HeapFree(GetProcessHeap(), 0, pACE);
	if(pSystemSid != NULL)
		FreeSid(pSystemSid);

	return bRet;
}



BOOL CIEClean::RegSetPrivilege(HKEY hKey, LPCTSTR pszSubKey, 
									   SECURITY_DESCRIPTOR* pSD, BOOL bRecursive)
{
	BOOL bRet = FALSE;
	HKEY hSubKey = NULL;
	LONG lRetCode;
	LPTSTR pszKeyName = NULL;;
	DWORD dwSubKeyCnt;
	DWORD dwMaxSubKey;
	DWORD dwValueCnt;
	DWORD dwMaxValueName;
	DWORD dwMaxValueData;
	DWORD i;

	if (!pszSubKey)
		goto cleanup;

	// open the key for WRITE_DAC access
	lRetCode = RegOpenKeyEx(hKey, pszSubKey, 0, WRITE_DAC, &hSubKey);
	if(lRetCode != ERROR_SUCCESS)
		goto cleanup;

	// apply the security descriptor to the registry key
	lRetCode = RegSetKeySecurity(hSubKey, 
		(SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, pSD);
	if( lRetCode != ERROR_SUCCESS )
		goto cleanup;

	if (bRecursive)
	{
		// reopen the key for KEY_READ access
		RegCloseKey(hSubKey);
		hSubKey = NULL;
		lRetCode = RegOpenKeyEx(hKey, pszSubKey, 0, KEY_READ, &hSubKey);
		if(lRetCode != ERROR_SUCCESS)
			goto cleanup;

		// first get an info about this subkey ...
		lRetCode = RegQueryInfoKey(hSubKey, 0, 0, 0, &dwSubKeyCnt, &dwMaxSubKey,
			0, &dwValueCnt, &dwMaxValueName, &dwMaxValueData, 0, 0);
		if( lRetCode != ERROR_SUCCESS )
			goto cleanup;

		// enumerate the subkeys and call RegTreeWalk() recursivly
		pszKeyName = new TCHAR [MAX_PATH + 1];
		for (i=0 ; i<dwSubKeyCnt; i++)
		{
			lRetCode = RegEnumKey(hSubKey, i, pszKeyName, MAX_PATH + 1);
			if(lRetCode == ERROR_SUCCESS)
			{
				RegSetPrivilege(hSubKey, pszKeyName, pSD, TRUE);
			}
			else if(lRetCode == ERROR_NO_MORE_ITEMS)
			{
				break;
			}
		}
		delete [] pszKeyName ;
	}

	bRet = TRUE; // indicate success

cleanup:
	if (hSubKey)
	{
		RegCloseKey(hSubKey);
	}
	return bRet;
}


BOOL CIEClean::CleanIEPassword()
{	
	SHDeleteKey(HKEY_CURRENT_USER, 
		_T("Software\\Microsoft\\Internet Explorer\\IntelliForms"));
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

BOOL CIEClean::CleanFrom()
{
	return TRUE;
}

BOOL CIEClean::_CleanIndexDat()
{
	std::wstring str = L"";
	static BOOL bFlag = FALSE;
	g_fnScanFile(g_pMain,BEGINPROC(IECLEAN_INDEXDATA),0,0,0);

	std::vector<std::wstring>::iterator it;
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"iexplore.exe")
		{
			str = L"正在运行，跳过";
			goto Clear0;
		}
	}
	str = L"";

	if (!bFlag)
	{
		str = L"可以清理";
		bFlag = TRUE;
	}
	else
	{
		str = L"";
	}
Clear0:
	g_fnScanFile(g_pMain,ENDPROC(IECLEAN_INDEXDATA),str.c_str(),0,0);

	return TRUE;

}


BOOL CIEClean::CleanIndexDat()
{	
	WCHAR* pEnv=NULL;
	pEnv = _wgetenv(_T("USERPROFILE"));
	CString strPath = _T("");
	WCHAR szPath[MAX_PATH] = {0};

	

	if (pEnv !=NULL)
	{	

		LARGE_INTEGER larg;
		larg.QuadPart=0;

		SHGetSpecialFolderPath(NULL, szPath, CSIDL_HISTORY, FALSE);
		strPath = szPath;
		strPath += L"\\index.dat";
		CleanIndex(strPath,larg);
		
		memset(szPath,0,MAX_PATH);
		SHGetSpecialFolderPath(NULL, szPath, CSIDL_COOKIES, FALSE);
		strPath = szPath;
		strPath += L"\\index.dat";
		CleanIndex(strPath,larg);

		memset(szPath,0,MAX_PATH);
		SHGetSpecialFolderPath(NULL, szPath, CSIDL_INTERNET_CACHE, FALSE);
		strPath = szPath;
		strPath += L"\\Content.IE5\\index.dat";
		CleanIndex(strPath,larg);

	}

	return TRUE;
	

	//下面代码废弃
	if (m_bScan==TRUE)
	{
		CString strPath = _T("");

		WCHAR* pEnv=NULL;
		pEnv = _wgetenv(_T("USERPROFILE"));
		if (pEnv !=NULL)
		{
			strPath = pEnv;

			CSimpleArray<CString> vec_file;
			vec_file.Add(_T("index.dat"));

			USERDATA userData;
			userData.pObject= this;
			userData.iType = IECLEAN_INDEXDATA;

			m_fileOpt.DoFileEnumeration(strPath,vec_file,TRUE,TRUE,myEnumerateFile,&userData);
		}
	
	}

	return TRUE;
}

BOOL CIEClean::CleanIndex(LPCTSTR lpFileOrPath,LARGE_INTEGER filesize)
{	
	
	if (FALSE == m_bScan)
	{
		return FALSE;
	}

	HANDLE hFile = CreateFile(lpFileOrPath, GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(lpFileOrPath);
		OutputDebugString(_T("无效的路径或没有！"));
		return FALSE;
	}
	try
	{

		CString  strPath = lpFileOrPath;

		SetFileAttributes(strPath, FILE_ATTRIBUTE_NORMAL);

		CFileMapWrite MappingFile(strPath) ;
		DWORD dwFileLen ;
		LPBYTE pBuffer ;

		dwFileLen = MappingFile.GetSize() ;
		if (0 == dwFileLen)
		{
			OutputDebugString(_T("Error:获取文件长度失败!")) ;
		}
		else
		{
			if (MappingFile.GetData(&pBuffer))
			{
                if (pBuffer)
                {
                    for (DWORD dwByte=0; dwByte<dwFileLen; dwByte++)
				    {
					    pBuffer[dwByte] = 0;
				    }
                }
			}

			MappingFile.Close() ;
		}
;
	}
	catch (...)
	{
		OutputDebugString(_T("崩溃:index"));
	}

	ClearLen(lpFileOrPath) ;
	WipeTime(lpFileOrPath) ;

	CloseHandle(hFile);
	return  TRUE;
}

//文件清除长度
BOOL CIEClean::ClearLen(LPCTSTR lpFile)
{
	FILE *fp = NULL ;

	fp = _tfopen(lpFile, _T("w")) ;

	if (NULL == fp)
	{
		return FALSE ;
	}

	fclose(fp) ;
	return TRUE;
}
//文件清除时间
BOOL CIEClean::WipeTime(LPCTSTR lpFile)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	hFile = CreateFile(lpFile , GENERIC_READ | GENERIC_WRITE , FILE_SHARE_READ, NULL , 
		OPEN_EXISTING , 0 , NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE ;
	}

	FILETIME m_ftWindirCreateTime;
	TCHAR szWinDir[MAX_PATH] = {0} ;
	WIN32_FILE_ATTRIBUTE_DATA wfad ;
	memset(&wfad, 0 , sizeof(WIN32_FILE_ATTRIBUTE_DATA)) ;
	GetWindowsDirectory(szWinDir, MAX_PATH) ;
	GetFileAttributesEx(szWinDir, GetFileExInfoStandard, &wfad) ;
	m_ftWindirCreateTime = wfad.ftCreationTime ;
	SetFileTime(hFile, &m_ftWindirCreateTime, &m_ftWindirCreateTime, &m_ftWindirCreateTime) ;

	CloseHandle(hFile) ;
	return true ;
}

BOOL CIEClean::ScanIEHistory()
{
	BOOL bRet = FALSE;

	HRESULT hr = S_OK;	
	IUrlHistoryStg2* pUrlHistoryStg2 = NULL;

	STATURL suURL = {0};  
	ULONG pceltFetched = 0;  
	suURL.cbSize = sizeof(suURL);

	WCHAR szPath[MAX_PATH] = {0};
	CString strIETempFolder;
	CString strTemp;
	CSimpleArray<CString> vs_File;

	std::wstring str = L"";
	std::vector<std::wstring>::iterator it;

	CoInitialize(NULL); 
	hr = CoCreateInstance(CLSID_CUrlHistory, NULL, 
		CLSCTX_INPROC_SERVER, IID_IUrlHistoryStg2, 
		(void**)&pUrlHistoryStg2);
	if (pUrlHistoryStg2 == NULL)
	{
		goto clean0;
	}

	IEnumSTATURL*   pEnumURL = NULL;
	hr = pUrlHistoryStg2->EnumUrls(&pEnumURL);
	if (pEnumURL == NULL)
	{
		goto clean0;
	}

	hr = pEnumURL->Reset();
	
	g_fnScanFile(g_pMain,BEGINPROC(IECLEAN_HISTORY),0,0,0);

	// 判断进程是否在运行
	for (it = g_listProcessName.begin(); it != g_listProcessName.end(); it++ )
	{
		str = *it;
		transform(str.begin(), str.end(), str.begin(), towlower);
		if (str == L"iexplore.exe")
		{
			str = L"正在运行，跳过";
			goto clean0;
		}
	}
	str = L"";

	while((hr = pEnumURL->Next(1, &suURL, &pceltFetched)) == S_OK)  
	{  
		if (m_nFlags == 0)
		{
			g_fnScanFile(g_pMain,IECLEAN_HISTORY, L"IE历史记录|index.dat",0 ,0 );
		}
		break;
	}  

	SHGetSpecialFolderPath(NULL, szPath, CSIDL_INTERNET_CACHE, FALSE);
	strIETempFolder = szPath;
	if (strIETempFolder.ReverseFind(L'\\') != strIETempFolder.GetLength() - 1)
	{
		strIETempFolder += L"\\";
	}
	strTemp = strIETempFolder;

	strIETempFolder += L"Content.IE5";
	m_appHistory.CommfunFile(IECLEAN_HISTORY, strIETempFolder, vs_File, TRUE, TRUE);

	strIETempFolder = strTemp;
	strIETempFolder += L"AntiPhishing";
	m_appHistory.CommfunFile(IECLEAN_HISTORY, strIETempFolder, vs_File, TRUE, TRUE);

	strIETempFolder = strTemp;
	strIETempFolder += L"Virtualized";
	m_appHistory.CommfunFile(IECLEAN_HISTORY, strIETempFolder, vs_File, TRUE, TRUE);

	strIETempFolder = strTemp;
	strIETempFolder += L"Content.MSO";
	m_appHistory.CommfunFile(IECLEAN_HISTORY, strIETempFolder, vs_File, TRUE, TRUE);

	strIETempFolder = strTemp;
	strIETempFolder += L"Content.Outlook";
	m_appHistory.CommfunFile(IECLEAN_HISTORY, strIETempFolder, vs_File, TRUE, TRUE);

	strIETempFolder= strTemp;
	strIETempFolder += L"Content.Word";
	m_appHistory.CommfunFile(IECLEAN_HISTORY, strIETempFolder, vs_File, TRUE, TRUE);

	strIETempFolder = strTemp;
	strIETempFolder += L"Low";
	m_appHistory.CommfunFile(IECLEAN_HISTORY, strIETempFolder, vs_File, TRUE, TRUE);

clean0:
	g_fnScanFile(g_pMain,ENDPROC(IECLEAN_HISTORY),str.c_str(),0,0);
	if (pEnumURL)
	{
		pEnumURL->Release();
		pEnumURL = NULL;
	}
	if (pUrlHistoryStg2)
	{
		pUrlHistoryStg2->Release();   
		pUrlHistoryStg2 = NULL;
	}
	CoUninitialize();
	bRet = TRUE;
	return bRet;
}

BOOL CIEClean::CleanIEHistory(CString pszUrl)
{
	BOOL bRet = FALSE;

	HRESULT hr = S_OK;	
	IUrlHistoryStg2* pUrlHistoryStg2 = NULL;

	if (pszUrl.Find(L"|") == -1)
	{
		goto clean0;
	}
	CoInitialize(NULL); 
	hr = CoCreateInstance(CLSID_CUrlHistory, NULL, 
		CLSCTX_INPROC_SERVER, IID_IUrlHistoryStg2, 
		(void**)&pUrlHistoryStg2);
	if (pUrlHistoryStg2 == NULL)
	{
		bRet = TRUE;
		goto clean0;
	}

//	hr   =   pUrlHistoryStg2->DeleteUrl((LPCOLESTR)szUrl , 0);  
 
	hr = pUrlHistoryStg2->ClearHistory();
	m_nFlags++;
	bRet = TRUE;
	
clean0:
	if (pUrlHistoryStg2)
	{
		pUrlHistoryStg2->Release();
		pUrlHistoryStg2 = NULL; 
	}
	return bRet;
}

std::string CIEClean::deescapeURL(const std::string &URL) {
	string result = "";
	for ( unsigned int i=0; i<URL.size(); i++ ) {
		char c = URL[i];
		if ( c != '%' ) {
			result += c;
		} else {
			char c1 = URL[++i];
			char c0 = URL[++i];
			int num = 0;
			num += hexChar2dec(c1) * 16 + hexChar2dec(c0);
			result += char(num);
		}
	}
	return result;
}

short int CIEClean::hexChar2dec(char c) {
	if ( '0'<=c && c<='9' ) {
		return short(c-'0');
	} else if ( 'a'<=c && c<='f' ) {
		return ( short(c-'a') + 10 );
	} else if ( 'A'<=c && c<='F' ) {
		return ( short(c-'A') + 10 );
	} else {
		return -1;
	}
}


