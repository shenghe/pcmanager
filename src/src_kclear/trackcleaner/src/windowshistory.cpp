#include "stdafx.h"
#include "windowshistory.h"
#include "kscbase/kscsys.h"
#include <string>
using namespace std;

CWindowsHistory::CWindowsHistory()
{	
	//获得登陆用户名
	DWORD dwLoginSize=MAX_PATH;
	memset(m_szLoginName,0,sizeof(TCHAR)*MAX_PATH);
//	GetUserName(m_szLoginName,&dwLoginSize);

	m_bScan = TRUE; 
}

CWindowsHistory::~CWindowsHistory()
{

}


BOOL CWindowsHistory::myEnumerateFile(LPCTSTR lpFileOrPath, void* _pUserData,LARGE_INTEGER filesize)
{
	USERDATA* pUserData = (USERDATA*) _pUserData;
	CWindowsHistory* pObject = (CWindowsHistory*) pUserData->pObject;
	
	//过滤所有desk.ini文件
	CString strPath = lpFileOrPath;
	strPath.MakeLower();

	if (-1!=strPath.Find(_T("desktop.ini")))
	{
		return TRUE;
	}
	if (-1!=strPath.Find(_T("info2")))
	{
		return TRUE;
	}
	if (-1!=strPath.Find(_T("index.dat")))
	{
		return TRUE;
	}

	switch (pUserData->iType)
	{
	case WINSHISTORY_RECENTDOCFILE:
		{	
			CString strPath = lpFileOrPath;
			strPath.MakeUpper();
			if (-1!=strPath.Find(_T("DESKTOP.INI.LNK")))
			{
				return TRUE;
			}
			return pObject->RecentDocFile( lpFileOrPath, filesize);
		}
		break;

	case FILEGARBAGE_RECYCLED:
		return pObject->CommonFileFun(FILEGARBAGE_RECYCLED, lpFileOrPath, filesize);
		break;
	case WINSHISTORY_WINTEMP:
		return pObject->CommonFileFun(WINSHISTORY_WINTEMP, lpFileOrPath, filesize);
		break;
	case WINSHISTORY_WINLOG:
		return pObject->CommonFileFun(WINSHISTORY_WINLOG, lpFileOrPath, filesize);
		break;
	/*case FILEGARBAGE_RECYCLED:
		return pObject->CommonFileFun(FILEGARBAGE_RECYCLED, lpFileOrPath, filesize);*/
	
	case FILEGARBAGE_DESKTOPLINK:
		return pObject->InvalidLink(FILEGARBAGE_DESKTOPLINK, lpFileOrPath, filesize);
		break;
	case FILEGARBAGE_STARTMENULINK:
		return pObject->InvalidLink(FILEGARBAGE_STARTMENULINK, lpFileOrPath, filesize);
		break;
	}
	
	return TRUE;
}

BOOL CWindowsHistory::myEnumerateReg(
							  HKEY rootKey,LPCTSTR lpItemPath,
							  LPCTSTR lpName,DWORD dwValueNameSize,
							  LPBYTE lpValue ,DWORD dwValueSize,DWORD dwType,
							  void* _pUserData
							  )
{	

	USERDATA* pUserData = (USERDATA*) _pUserData;
	CWindowsHistory* pObject = (CWindowsHistory*) pUserData->pObject;

	LPTSTR lpValueTmp = (LPTSTR) lpValue;
	LPTSTR lpNameTmp = (LPTSTR) lpName;

	switch (dwType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
		{	
			switch (pUserData->iType)
			{
			case WINSHISTORY_RUNDLG:
				return pObject->RunDlg( rootKey, lpItemPath, lpNameTmp,lpValueTmp);

			case WINSHISTORY_RUNHISTORY:
				return pObject->RunHistory(rootKey, lpItemPath, lpNameTmp,lpValueTmp);

			case WINSHISTORY_OPENSAVE:
				return pObject->OpenOrSave(rootKey, lpItemPath, lpNameTmp,lpValueTmp);
			
			//四项通用处理,由于不知道这四项值为sz或bin所以各写一处
			case WINSHISTORY_NETERIVE:
				return pObject->CommonRegFun(WINSHISTORY_NETERIVE,rootKey, lpItemPath, lpNameTmp,lpValueTmp);
			case WINSHISTORY_FINDCMP:
				return pObject->CommonRegFun(WINSHISTORY_FINDCMP,rootKey, lpItemPath, lpNameTmp,lpValueTmp);
			case WINSHISTORY_FINDDOC:
				return pObject->CommonRegFun(WINSHISTORY_FINDDOC,rootKey, lpItemPath, lpNameTmp,lpValueTmp);
			case WINSHISTORY_PRTPORT:
				return pObject->CommonRegFun(WINSHISTORY_PRTPORT,rootKey, lpItemPath, lpNameTmp,lpValueTmp);
			case BAIDU_TOOLBAR:
				return pObject->CommonRegFun(BAIDU_TOOLBAR,rootKey, lpItemPath, lpNameTmp,lpValueTmp);
			}
			
		}	
	case REG_BINARY:			//除REG_SZ和REG_EXPAND_SZ其他值均忽略
		switch (pUserData->iType)
		{
		case WINSHISTORY_WINSIZE:  
			return pObject->WinSize(rootKey, lpItemPath, lpNameTmp,_T(""));

		case WINSHISTORY_RECENTDOCREG:
			return pObject->RecentDocReg(rootKey, lpItemPath, lpNameTmp,_T(""));

		case WINSHISTORY_TRAYNOTIFY:
			return pObject->TrayNotify(rootKey, lpItemPath, lpNameTmp,_T(""));

		case WINSHISTORY_USERASSIST:
			return pObject->UserAssist(rootKey, lpItemPath, lpNameTmp,_T(""));
		
		case WINSHISTORY_RUNHISTORY:
			return pObject->RunHistory(rootKey, lpItemPath, lpNameTmp,_T(""));

			//四项通用处理,由于不知道这四项值为sz或bin所以各写一处
		case WINSHISTORY_NETERIVE:
			return pObject->CommonRegFun(WINSHISTORY_NETERIVE,rootKey, lpItemPath, lpNameTmp,_T(""));
		case WINSHISTORY_FINDCMP:
			return pObject->CommonRegFun(WINSHISTORY_FINDCMP,rootKey, lpItemPath, lpNameTmp,_T(""));
		case WINSHISTORY_FINDDOC:
			return pObject->CommonRegFun(WINSHISTORY_FINDDOC,rootKey, lpItemPath, lpNameTmp,_T(""));
		case WINSHISTORY_PRTPORT:
			return pObject->CommonRegFun(WINSHISTORY_PRTPORT,rootKey, lpItemPath, lpNameTmp,_T(""));
		case WINSHISTORY_OPENSAVE:
			return pObject->OpenOrSave(rootKey, lpItemPath, lpNameTmp,_T(""));
		}
		break;
	case REG_DWORD_LITTLE_ENDIAN:
	case REG_DWORD_BIG_ENDIAN:							
	case REG_LINK:									
	case REG_MULTI_SZ:
	case REG_NONE:
	case REG_QWORD_LITTLE_ENDIAN:
		switch(pUserData->iType)
		{
		case BAIDU_TOOLBAR:
			return pObject->CommonRegFun(BAIDU_TOOLBAR,rootKey, lpItemPath, lpNameTmp,_T(""));
		}
	break;
	}

	return TRUE;
}

/************************************************************************/
/* windows最近运行对话框
/************************************************************************/

BOOL CWindowsHistory::ScanRunDlg()
{	
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_RUNDLG),0,0,0);
	CString strRunDlg = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU");
	ScanRunDlg(strRunDlg.GetBuffer());
	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_RUNDLG),0,0,0);


	return TRUE;
}

BOOL CWindowsHistory::ScanRunDlg(LPCTSTR lpszRunDlgPath)
{	
	if (m_bScan ==FALSE)
	{
		return TRUE;
	}


	CString strRunDlg = lpszRunDlgPath;
	try
	{	
		USERDATA userData;
		userData.pObject= this;
		userData.iType = WINSHISTORY_RUNDLG;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRunDlg,hRootKey,strSubKey);

		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),myEnumerateReg,&userData);

	}
	catch (...)
	{

	}

	return TRUE;
}

BOOL CWindowsHistory::RunDlg(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{
	if (m_bScan==FALSE)
		return TRUE;
	
	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);
	
	return g_fnScanReg(g_pMain,WINSHISTORY_RUNDLG,
		strRegFullPath.GetBuffer(),
		lpValueName,
		lpValueData
		);
}

/************************************************************************/
/* 最近运行程序记录
/************************************************************************/

BOOL CWindowsHistory::ScanRunHistory()
{	
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_RUNHISTORY),0,0,0);
	CString strRunDlg = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\LastVisitedMRU");
	ScanRunHistory(strRunDlg.GetBuffer());
	
	//WIN7
	strRunDlg =  _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\LastVisitedPidlMRU");
	ScanRunHistory(strRunDlg.GetBuffer());
	strRunDlg =  _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\LastVisitedPidlMRULegacy");
	ScanRunHistory(strRunDlg.GetBuffer());
	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_RUNHISTORY),0,0,0);

	return TRUE;
}

BOOL CWindowsHistory::ScanRunHistory(LPCTSTR lpszRegPath)
{	
	if (m_bScan ==FALSE)
	{
		return TRUE;
	}


	CString strRunDlg = lpszRegPath;
	try
	{	
		USERDATA userData;
		userData.pObject= this;
		userData.iType = WINSHISTORY_RUNHISTORY;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRunDlg,hRootKey,strSubKey);

		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),myEnumerateReg,&userData);

	}
	catch (...)
	{	
		//OutputDebugString(_T("崩溃:ScanRunHistory"));
		return FALSE;
	}

	return TRUE;
}


BOOL CWindowsHistory::RunHistory(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{
	if (m_bScan==FALSE)
		return TRUE;
	
	//if ( -1!=wcscmp(lpValueName,_T("MRUList")))
	//	return TRUE;
	
	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

	return g_fnScanReg(g_pMain,WINSHISTORY_RUNHISTORY,
		strRegFullPath.GetBuffer(),
		lpValueName,
		lpValueData
		);
}
/************************************************************************/
/* 最近打开与保存
/************************************************************************/

BOOL CWindowsHistory::ScanOpenOrSave()
{	
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_OPENSAVE),0,0,0);

	CString	strRunDlg = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\OpenSaveMRU");
	ScanOpenOrSave(strRunDlg.GetBuffer());
	
	//WIN7
	strRunDlg = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\OpenSavePidlMRU");
	ScanOpenOrSave(strRunDlg.GetBuffer());
	
	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_OPENSAVE),0,0,0);
	return TRUE;

}

BOOL CWindowsHistory::ScanOpenOrSave(LPCTSTR lpszRegPath)
{	
	if (m_bScan ==FALSE)
	{
		return TRUE;
	}


	CString strRunDlg = lpszRegPath;
	try
	{	
		USERDATA userData;
		userData.pObject= this;
		userData.iType = WINSHISTORY_OPENSAVE;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRunDlg,hRootKey,strSubKey);

		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),myEnumerateReg,&userData);

	}
	catch (...)
	{	
		//OutputDebugString(_T("崩溃:ScanOpenOrSave"));
		return FALSE;
	}

	return TRUE;
}

BOOL CWindowsHistory::OpenOrSave(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{
	if (m_bScan==FALSE)
		return TRUE;

	//if (0 ==wcscmp(lpValueName,_T("MRUList")))
	//	return TRUE;
	
	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

	return g_fnScanReg(g_pMain,WINSHISTORY_OPENSAVE,
		strRegFullPath.GetBuffer(),
		lpValueName,
		lpValueData
		);
}
/************************************************************************/
/* 窗口大小记录
/************************************************************************/



BOOL CWindowsHistory::ScanWinSize()
{		
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_WINSIZE),0,0,0);

	CString strRunDlg = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StreamMRU");
	ScanWinSize(strRunDlg.GetBuffer());

	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_WINSIZE),0,0,0);

	return TRUE;
}

BOOL CWindowsHistory::ScanWinSize(LPCTSTR lpszWinSizePath)
{	
	if (m_bScan ==FALSE)
	{
		return TRUE;
	}


	CString strRunDlg = lpszWinSizePath;
	try
	{	
		USERDATA userData;
		userData.pObject= this;
		userData.iType = WINSHISTORY_WINSIZE;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRunDlg,hRootKey,strSubKey);

		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),myEnumerateReg,&userData);

	}
	catch (...)
	{
		//OutputDebugString(_T("崩溃:ScanWinSize"));
		return FALSE;
	}

	return TRUE;
}

BOOL CWindowsHistory::WinSize(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{
	if (m_bScan==FALSE)
		return TRUE;

	//if (0 == wcscmp(lpValueName,_T("MRUListEx")))
	//	return TRUE;
	
	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

	return g_fnScanReg(g_pMain,WINSHISTORY_WINSIZE,
		strRegFullPath.GetBuffer(),
		lpValueName,
		_T("")
		);
}

/************************************************************************/
/* 最近打开的文档记录 注册表
/************************************************************************/

BOOL CWindowsHistory::ScanRecentDocReg()
{	
	
	CString strRunDlg = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RecentDocs");
	ScanRecentDocReg(strRunDlg.GetBuffer(),WINSHISTORY_RECENTDOCREG);

	strRunDlg = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\TypedPaths");
	ScanRecentDocReg(strRunDlg.GetBuffer(),WINSHISTORY_RECENTDOCREG);

	return TRUE;
}

BOOL CWindowsHistory::ScanRecentDocReg(LPCTSTR lpszRegPath,int iType)
{	
	if (m_bScan ==FALSE)
	{
		return TRUE;
	}


	CString strRunDlg = lpszRegPath;
	try
	{	
		USERDATA userData;
		userData.pObject= this;
		userData.iType = iType;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRunDlg,hRootKey,strSubKey);

		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),myEnumerateReg,&userData);

	}
	catch (...)
	{
		//OutputDebugString(_T("崩溃:ScanRecentDocReg"));
		return FALSE;
	}

	return TRUE;
}

BOOL CWindowsHistory::RecentDocReg(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{
	if (m_bScan==FALSE)
		return TRUE;

	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

	return g_fnScanReg(g_pMain,WINSHISTORY_RECENTDOCREG,
		strRegFullPath.GetBuffer(),
		lpValueName,
		lpValueData
		);
}

/************************************************************************/
/* 最近打开的文档记录 文件
/************************************************************************/

//??
BOOL CWindowsHistory::ScanRecentDocFile()
{
	g_fnScanFile(g_pMain,BEGINPROC(WINSHISTORY_RECENTDOCREG),0,0,0);
	
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_RECENTDOCREG),0,0,0);
	ScanRecentDocReg();
	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_RECENTDOCFILE),0,0,0);

	TCHAR szDesktopPath[MAX_PATH]={0};
	//WCHAR* pEnv = _wgetenv(_T("USERPROFILE"));
	SHGetSpecialFolderPath(NULL, szDesktopPath, CSIDL_RECENT, FALSE);
	//if (pEnv != NULL)
	//{	
//		wsprintf(szDesktopPath,_T("%s\\Recent"),pEnv);
//	OutputDebugString(szDesktopPath);
		ScanRecentDocFile(szDesktopPath,WINSHISTORY_RECENTDOCFILE);
	//}


	g_fnScanFile(g_pMain,ENDPROC(WINSHISTORY_RECENTDOCFILE),0,0,0);

	return TRUE;


}

BOOL CWindowsHistory::ScanRecentDocFile(LPCTSTR lpszFileOrFolderPath,int iType)
{	
	if (m_bScan ==FALSE)
	{
		return TRUE;
	}


	try
	{	
		USERDATA userData;
		userData.pObject= this;
		userData.iType = iType;

		m_fileOpt.DoFileEnumeration(lpszFileOrFolderPath,TRUE,TRUE,myEnumerateFile,&userData);
	}
	catch(...)
	{	
		//OutputDebugString(_T("崩溃:ScanRecentDocFile"));
		return FALSE;
	}

	return FALSE;
}

BOOL CWindowsHistory::RecentDocFile(LPCTSTR lpFileOrPath,LARGE_INTEGER filesize)
{
	if (m_bScan==FALSE)
		return TRUE;

	return g_fnScanFile(g_pMain,WINSHISTORY_RECENTDOCFILE,lpFileOrPath,filesize.LowPart,filesize.LowPart);;
}

/************************************************************************/
/* 扩展名记录
/************************************************************************/
//1.先在HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes存储所有的扩展名列表,存入map
//2.在HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts 中扩展名不在map列表中
//	且OpenWithList下无内容者或是没有任何子键的文件件

BOOL CWindowsHistory::ScanFileExtsHistory()
{	
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_FILEEXTS),0,0,0);
	CString ClassstrExts =_T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes");
	ScanFileExtsHistory(ClassstrExts.GetBuffer());
	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_FILEEXTS),0,0,0);
	return TRUE;
}

BOOL CWindowsHistory::ScanFileExtsHistory(LPCTSTR lpcszFileExtsHis)
{	
	if (m_bScan ==FALSE)
	{
		return TRUE;
	}

	CString ClassstrExts =lpcszFileExtsHis;
	
	CAtlMap<CString,char> map_SubKey;
	try
	{	
		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(ClassstrExts,hRootKey,strSubKey);
		m_regOpt.DoEnumCurrnetSubKey(hRootKey,strSubKey.GetBuffer(),map_SubKey);
		
		//特殊值
		map_SubKey.SetAt(_T("OpenWithList"),'1');
		map_SubKey.SetAt(_T(".udl"),'1');
		map_SubKey.SetAt(_T(".RES"),'1');
	}
	catch (...)
	{
		//OutputDebugString(_T("崩溃:ScanFileExtsHistory"));
	}

	CString strRunDlg = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts");
	try
	{	
		USERDATA userData;
		userData.pObject= this;
		userData.iType = WINSHISTORY_FILEEXTS;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRunDlg,hRootKey,strSubKey);
		
		//第一步获得当前键下的所有子键	
		CSimpleArray<CString> vec_SubKey;
		m_regOpt.DoEnumCurrnetSubKey(hRootKey,strSubKey.GetBuffer(),vec_SubKey);
		
		for (int i=0 ;i<vec_SubKey.GetSize();i++)
		{

			if (map_SubKey.Lookup(vec_SubKey[i]) == NULL)
			{	
				CString strSubKeyTmp;
				strSubKeyTmp = strSubKey+_T("\\") + vec_SubKey[i];

				CString strFindKeyTmp;
				strFindKeyTmp = strSubKeyTmp;
				strFindKeyTmp.Append(_T("\\OpenWithList"));

				HKEY hKey = NULL;
				if (RegOpenKeyEx(hRootKey,strFindKeyTmp,0,KEY_ALL_ACCESS,&hKey)==ERROR_SUCCESS)
				{	
					CSimpleArray<REGVALUE> vec_Value;
					m_regOpt.DoEnumCurrnetValue(hRootKey,strFindKeyTmp.GetBuffer(),vec_Value);

					if (NULL == vec_Value.GetSize())
					{
						FileExtsHistory(hRootKey,strSubKeyTmp.GetBuffer(),_T(""),_T(""));
					}
				}
				else
				{	
					CSimpleArray<CString> vec_subKey;
					m_regOpt.DoEnumCurrnetSubKey(hRootKey,strSubKeyTmp,vec_subKey);
					
					if (NULL==vec_subKey.GetSize())
					{
						FileExtsHistory(hRootKey,strSubKeyTmp.GetBuffer(),_T(""),_T(""));
					}

				}
				if (hKey)
				{
					RegCloseKey(hKey);
				}

			}
		
		}
		
		

		//string_t strRegFullPath;
		//m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);



		//m_regOpt.DoRegEnumeration(hRootKey,strSubKey.c_str(),myEnumerateReg,&userData);

	}
	catch (...)
	{

	}

	return TRUE;
}



BOOL CWindowsHistory::FileExtsHistory(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{
	if (m_bScan==FALSE)
		return TRUE;

	//if (0 == wcscmp(lpValueName,_T("MRUListEx")))
	//	return TRUE;

	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

	return g_fnScanReg(g_pMain,WINSHISTORY_FILEEXTS,
		strRegFullPath.GetBuffer(),
		lpValueName,
		lpValueData
		);
}

/************************************************************************/
/* 最近打开程序历史记录
/************************************************************************/
//HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist\\{5E6AB780-7743-11CF-A12B-00AA004AE837}\\Count
//下所有值
BOOL CWindowsHistory::ScanUserAssist()
{	
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_USERASSIST),0,0,0);

	CString strUserAssist= _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist");
	ScanUserAssist(strUserAssist.GetBuffer());

	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_USERASSIST),0,0,0);

	return TRUE;
}

BOOL CWindowsHistory::ScanUserAssist(LPCTSTR lpcszUserAssistPath)
{	
	if (m_bScan ==FALSE)
	{
		return TRUE;
	}


	CString strUserAssist= lpcszUserAssistPath;
	
	try
	{	
		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strUserAssist,hRootKey,strSubKey);
		
		USERDATA userData;
		userData.pObject= this;
		userData.iType = WINSHISTORY_USERASSIST;

		CSimpleArray<REGVALUE> vec_Value;
		m_regOpt.DoRegEnumeration(hRootKey,strSubKey,myEnumerateReg,&userData);


	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CWindowsHistory::UserAssist(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{	
	if (m_bScan==FALSE)
		return TRUE;
	
	//OutputDebugString(_T("\n扫描到项目\n"));

	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

	return g_fnScanReg(g_pMain,WINSHISTORY_USERASSIST,
		strRegFullPath.GetBuffer(),
		lpValueName,
		lpValueData
		);
}

/************************************************************************/
/* 通知区域图历史事记录
/************************************************************************/
BOOL CWindowsHistory::ScanTrayNotify()
{	
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_TRAYNOTIFY),0,0,0);

	CString strRunDlg;
	if(KGetWinVersion() < 4)
	{
		strRunDlg = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\TrayNotify");
	}
	else
	{
		strRunDlg = _T("HKEY_CURRENT_USER\\Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\TrayNotify");
	}

	ScanTrayNotify(strRunDlg.GetBuffer());

	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_TRAYNOTIFY),0,0,0);

	return TRUE;
}


BOOL CWindowsHistory::ScanTrayNotify(LPCTSTR lpcszTrayNotifyPath)
{	
	if (m_bScan ==FALSE)
	{
		return TRUE;
	}


	CString strRunDlg = lpcszTrayNotifyPath;
	try
	{	
		USERDATA userData;
		userData.pObject= this;
		userData.iType = WINSHISTORY_TRAYNOTIFY;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRunDlg,hRootKey,strSubKey);

		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),myEnumerateReg,&userData);

	}
	catch (...)
	{

	}

	return TRUE;
}
BOOL CWindowsHistory::TrayNotify(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{	
	if (m_bScan==FALSE)
		return TRUE;

	if (0 == wcscmp(lpValueName,_T("BalloonTip")))
		return TRUE;

	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

	return g_fnScanReg(g_pMain,WINSHISTORY_TRAYNOTIFY,
		strRegFullPath.GetBuffer(),
		lpValueName,
		lpValueData
		);
	
	return TRUE;
}
/************************************************************************/
/* 以下思想行为相同,统一用CommonRegFun 函数进行处理
/************************************************************************/

//网络驱动器映射历史
BOOL	CWindowsHistory::ScanNetDerive()
{	
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_NETERIVE),0,0,0);
	CString strRegPath = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Map Network Drive MRU");
	ScanRecentDocReg(strRegPath.GetBuffer(), WINSHISTORY_NETERIVE);
	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_NETERIVE),0,0,0);

	return TRUE;

}
//查找计算机历史
BOOL	CWindowsHistory::ScanFindCmp()
{	
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_FINDCMP),0,0,0);
	CString strRegPath = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Search Assistant\\ACMru\\5647");
	ScanRecentDocReg(strRegPath.GetBuffer(), WINSHISTORY_FINDCMP);

	strRegPath = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FindComputerMRU");
	ScanRecentDocReg(strRegPath.GetBuffer(), WINSHISTORY_FINDCMP);

	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_FINDCMP),0,0,0);

	return TRUE;
}
//文档查找历史
BOOL	CWindowsHistory::ScanFindDoc()
{	
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_FINDDOC),0,0,0);
	CString strRegPath = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Doc Find Spec MRU");
	ScanRecentDocReg(strRegPath.GetBuffer(), WINSHISTORY_FINDDOC);
	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_FINDDOC),0,0,0);

	return TRUE;
}
//打印端口历史
BOOL CWindowsHistory::ScanPrtPort()
{	
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_PRTPORT),0,0,0);
	CString strRegPath = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\PrnPortsMRU");
	ScanRecentDocReg(strRegPath.GetBuffer(), WINSHISTORY_PRTPORT);
	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_PRTPORT),0,0,0);

	return TRUE;
}
//通用注册表枚举
BOOL CWindowsHistory::CommonRegFun(int iType,HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{
	if (m_bScan==FALSE)
		return TRUE;

	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);


	return g_fnScanReg(g_pMain,iType,
		strRegFullPath.GetBuffer(),
		lpValueName,
		lpValueData
		);

}

/************************************************************************/
/* windows临时文件夹
/************************************************************************/
//?
BOOL CWindowsHistory::ScanTempFolder()
{	
	g_fnScanFile(g_pMain,BEGINPROC(WINSHISTORY_WINTEMP),0,0,0);
	
	if (m_bScan ==FALSE)
	{
		goto Exit;
	}
	BEGIN
	//路径1
	TCHAR szDesktopPath[MAX_PATH]={0};
	WCHAR* pEnv;
	pEnv = _wgetenv(_T("temp"));
	if (pEnv ==NULL)
	{
		goto Exit;
	}
	
	wsprintf(szDesktopPath,_T("%s"),pEnv);
	ScanRecentDocFile(szDesktopPath,WINSHISTORY_WINTEMP);
	
	//路径2
	pEnv = _wgetenv(_T("systemroot"));
	if (pEnv ==NULL)
	{
		goto Exit;
	}

	wsprintf(szDesktopPath,_T("%s\\Temp"),pEnv);
	ScanRecentDocFile(szDesktopPath,WINSHISTORY_WINTEMP);
	END

Exit:
	g_fnScanFile(g_pMain,ENDPROC(WINSHISTORY_WINTEMP),0,0,0);

	return TRUE;
}


BOOL CWindowsHistory::ScanRecycled()
{	
	g_fnScanFile(g_pMain,BEGINPROC(FILEGARBAGE_RECYCLED),0,0,0);
	
	//OutputDebugString(L"回收站\n");
	/*CString strRecycledPath = _T("c:\\RECYCLER");
	ScanRecentDocFile(strRecycledPath.GetBuffer(),FILEGARBAGE_RECYCLED);

	strRecycledPath = _T("d:\\RECYCLER");
	ScanRecentDocFile(strRecycledPath.GetBuffer(),FILEGARBAGE_RECYCLED);

	strRecycledPath = _T("e:\\RECYCLER");
	ScanRecentDocFile(strRecycledPath.GetBuffer(),FILEGARBAGE_RECYCLED);

	strRecycledPath = _T("f:\\RECYCLER");
	ScanRecentDocFile(strRecycledPath.GetBuffer(),FILEGARBAGE_RECYCLED);

	strRecycledPath = _T("g:\\RECYCLER");
	ScanRecentDocFile(strRecycledPath.GetBuffer(),FILEGARBAGE_RECYCLED);*/
	
	SHQUERYRBINFO shq;
	wstring strPath;
	wstring strPath1;
	wchar_t* drive_buff = new wchar_t[1024];
	wchar_t* pdrive_buff = drive_buff;
	
	DWORDLONG  ItemsFound=0;
	DWORDLONG  BytesFound=0;

	ZeroMemory(&shq,sizeof(shq));
	shq.cbSize = sizeof(shq);

	GetLogicalDriveStrings(256,drive_buff);
	while (*drive_buff)
	{
		if (GetDriveType(drive_buff) == DRIVE_FIXED)
		{
			SHQueryRecycleBin(drive_buff,&shq);
			ItemsFound += shq.i64NumItems;
			BytesFound += shq.i64Size;
		}

		LARGE_INTEGER large;
		large.QuadPart = BytesFound;

		strPath = drive_buff;
		strPath += L"RECYCLER";
		CSimpleArray<CString>  vec_value;
		m_appHistory.CommfunFile(FILEGARBAGE_RECYCLED,strPath.c_str(),vec_value);

		strPath1 = drive_buff;
		strPath1 += L"$Recycle.Bin";
		m_appHistory.CommfunFile(FILEGARBAGE_RECYCLED,strPath1.c_str(),vec_value);

		OutputDebugString(L"回收站2\n");
		drive_buff += wcslen(drive_buff) + 1;
		strPath = L"";
		strPath1 = L"";
	}

	delete[] pdrive_buff;
	
	
	LARGE_INTEGER large;
	large.QuadPart = BytesFound;
	
	TCHAR buf[256];
	_i64tow(ItemsFound,buf,10);


	/*g_fnScanFile(g_pMain,BEGINPROC(WINDOWS_RECYCLE),buf,large.LowPart,large.HighPart);
	
	//OutputDebugString(L"回收站1\n");*/
	g_fnScanFile(g_pMain,ENDPROC(FILEGARBAGE_RECYCLED),0,0,0);
	//OutputDebugString(L"回收站2\n");

	return TRUE;
}


BOOL CWindowsHistory::ScanLogFolder()
{	

	g_fnScanFile(g_pMain,BEGINPROC(WINSHISTORY_WINLOG),0,0,0);

	TCHAR szDesktopPath[MAX_PATH]={0};

	WCHAR* pEnv;
	pEnv = _wgetenv(_T("systemroot"));
	if (pEnv!=NULL)
	{
		CString strPath;
		CSimpleArray<CString> vec_file;
		//

		strPath.Format(_T("%s\\system32\\wbem\\Logs"),pEnv);
		m_appHistory.CommfunFile(WINSHISTORY_WINLOG,strPath,vec_file);

		//
		strPath.Format(_T("%s"),pEnv);
		vec_file.Add(_T("*.log"));
		vec_file.Add(_T("setuplog.txt"));
		vec_file.Add(_T("OEWABLog.txt"));
		m_appHistory.CommfunFile(WINSHISTORY_WINLOG,strPath,vec_file,FALSE);
		vec_file.RemoveAll();

		//
		vec_file.Add(_T("*.log"));
		strPath.Format(_T("%s\\Debug"),pEnv);
		m_appHistory.CommfunFile(WINSHISTORY_WINLOG,strPath,vec_file,FALSE);
		vec_file.RemoveAll();

		//
		vec_file.Add(_T("*.log"));
		vec_file.Add(_T("*.old"));
		strPath.Format(_T("%s\\Debug\\UserMode"),pEnv);
		m_appHistory.CommfunFile(WINSHISTORY_WINLOG,strPath,vec_file,FALSE);
		vec_file.RemoveAll();

		//
		vec_file.Add(_T("*.log"));
		strPath.Format(_T("%s\\security\\logs"),pEnv);
		m_appHistory.CommfunFile(WINSHISTORY_WINLOG,strPath,vec_file,FALSE);
		vec_file.RemoveAll();

		//
		vec_file.Add(_T("dberr.txt"));
		strPath.Format(_T("%s\\system32\\CatRoot2"),pEnv);
		m_appHistory.CommfunFile(WINSHISTORY_WINLOG,strPath,vec_file,FALSE);
		vec_file.RemoveAll();
	}
	

	g_fnScanFile(g_pMain,ENDPROC(WINSHISTORY_WINLOG),0,0,0);

	return TRUE;
}


BOOL CWindowsHistory::ScanInvaidDesktopLink()
{	
	g_fnScanFile(g_pMain,BEGINPROC(FILEGARBAGE_DESKTOPLINK),0,0,0);

	TCHAR szDesktopPath[MAX_PATH]={0};

	WCHAR* pEnv;
	pEnv = _wgetenv(_T("USERPROFILE"));
	if (pEnv !=NULL)
	{
		wsprintf(szDesktopPath,_T("%s\\桌面"),pEnv);

		CSimpleArray<CString> vec_fileExts;
		vec_fileExts.Add(_T("*.lnk"));
		ScanInvaidDesktopLink(FILEGARBAGE_DESKTOPLINK,szDesktopPath,vec_fileExts);
	}

	g_fnScanFile(g_pMain,ENDPROC(FILEGARBAGE_DESKTOPLINK),0,0,0);
	return TRUE;

}

BOOL CWindowsHistory::ScanInvaidStartLink()
{	
	g_fnScanFile(g_pMain,BEGINPROC(FILEGARBAGE_STARTMENULINK),0,0,0);

	TCHAR szDesktopPath[MAX_PATH]={0};

	WCHAR* pEnv;
	pEnv = _wgetenv(_T("USERPROFILE"));
	if (pEnv!=NULL)
	{
		wsprintf(szDesktopPath,_T("%s\\「开始」菜单"),pEnv);

		CSimpleArray<CString> vec_fileExts;
		vec_fileExts.Add(_T("*.lnk"));
		ScanInvaidDesktopLink(FILEGARBAGE_STARTMENULINK,szDesktopPath ,vec_fileExts);
	}


	g_fnScanFile(g_pMain,ENDPROC(FILEGARBAGE_STARTMENULINK),0,0,0);

	return TRUE;

}


BOOL CWindowsHistory::ScanInvaidDesktopLink(int iType,LPCTSTR lpFileOrPath,CSimpleArray<CString>& vec_fileExts)
{	
	if (m_bScan ==FALSE)
	{
		return TRUE;
	}


	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = iType;


		m_fileOpt.DoFileEnumeration(lpFileOrPath,vec_fileExts,TRUE,TRUE,myEnumerateFile,&userData);

	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CWindowsHistory::InvalidLink(int iType,LPCTSTR lpFileOrPath,LARGE_INTEGER filesize)
{	
	if (m_bScan==FALSE)
		return TRUE;

	TCHAR szFilePath[MAX_PATH]={0};

	if (TRUE==_InvalidLink((LPWSTR)lpFileOrPath,(LPWSTR)szFilePath,MAX_PATH))
	{	
		CString strPath = szFilePath;
		if (TRUE==g_regClean.ValidDervice(strPath))
		{
			if (!PathFileExists(szFilePath))
			{	
				return g_fnScanFile(g_pMain,iType,lpFileOrPath,filesize.LowPart,filesize.LowPart);;
			}
		}

	}

	return TRUE;	
}

BOOL CWindowsHistory::_InvalidLink(LPWSTR lpLinkrPath,LPWSTR lpFilePath,int iLen)
{
	//获得快捷方式指向路径
	CoInitialize ( NULL );
	HRESULT           hres;   
	IShellLink        *psl;   
	IPersistFile      *ppf;   
	WIN32_FIND_DATA   fd;
	
	hres = CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,
		IID_IShellLink,(void**)&psl);   
	
	if(!SUCCEEDED(hres))   
		return   FALSE;   
	
	hres = psl->QueryInterface(IID_IPersistFile,(void**)&ppf);   
	if(SUCCEEDED(hres))   
	{
		hres =  ppf->Load(lpLinkrPath,STGM_READ);
		if(SUCCEEDED(hres))   
			hres = psl->GetPath(lpFilePath,iLen,&fd,0);   
		ppf->Release();   
	}
	
	psl->Release();   

	return TRUE;
}

BOOL CWindowsHistory::CommonFileFun(int iType,LPCTSTR lpFileOrPath,LARGE_INTEGER filesize)
{
	if (m_bScan==FALSE)
		return TRUE;

	return g_fnScanFile(g_pMain,iType,lpFileOrPath,filesize.LowPart,filesize.LowPart);;
}

//注册表最后访问位置
BOOL CWindowsHistory::ScanRegeditHistory()
{		
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_REGEDIT),0,0,0);
	if (m_bScan==TRUE)
	{

		CString strPath = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit");	

		CSimpleArray<CString> vec_value;
		vec_value.Add(L"LastKey");
		m_appHistory.CommfunReg(WINSHISTORY_REGEDIT,strPath,vec_value);
	}
	
	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_REGEDIT),0,0,0);


	return TRUE;
}

//注册表流
BOOL CWindowsHistory::ScanRegeditStream()
{	
	g_fnScanReg(g_pMain,BEGINPROC(WINSHISTORY_STREAM),0,0,0);
	
	if (m_bScan==TRUE)
	{
		CString strPath = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Streams");	

		CSimpleArray<CString>  vec_value;
		m_appHistory.CommfunReg(WINSHISTORY_STREAM,strPath,vec_value);
	}

	g_fnScanReg(g_pMain,ENDPROC(WINSHISTORY_STREAM),0,0,0);


	return TRUE;
}

BOOL CWindowsHistory::ScanWindowsSearch()
{	
	g_fnScanReg(g_pMain,BEGINPROC(WINDOWS_SEARCH),0,0,0);

	if (m_bScan==TRUE)
	{
		CString strPath = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Search Assistant\\ACMru");	

		CSimpleArray<CString>  vec_value;
		m_appHistory.CommfunReg(WINDOWS_SEARCH,strPath,vec_value);


		strPath = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\WordWheelQuery");	
		m_appHistory.CommfunReg(WINDOWS_SEARCH,strPath,vec_value);
	}

	g_fnScanReg(g_pMain,ENDPROC(WINDOWS_SEARCH),0,0,0);


	return TRUE;
}


//google工具栏
BOOL CWindowsHistory::ScanGoogleToolBar()
{	
	g_fnScanFile(g_pMain,BEGINPROC(GOOGL_TOOLBAR),0,0,0);
	if (m_bScan==TRUE)
	{

		CString strPath = _T("%APPDATA%\\Google\\Local Search History\\");	

		CSimpleArray<CString>  vec_value;
		m_appHistory.CommfunFile(GOOGL_TOOLBAR,strPath,vec_value);
	}

	g_fnScanFile(g_pMain,ENDPROC(GOOGL_TOOLBAR),0,0,0);
	


	return TRUE;
}
//baidu工具栏
BOOL CWindowsHistory::ScanBaiduToolBar()
{
	g_fnScanReg(g_pMain,BEGINPROC(BAIDU_TOOLBAR),0,0,0);
	
	ScanBaiduToolBar(_T("HKEY_CURRENT_USER\\Software\\baidu\\BaiduToolbar\\History"));
	ScanBaiduToolBar(_T("HKEY_CURRENT_USER\\Software\\baidu\\BaiduToolbar\\navhistory"));

	ScanBaiduToolBar(_T("HKEY_CURRENT_USER\\Software\\AppDataLow\\Software\\baidu\\BaiduToolbar\\History"));
	ScanBaiduToolBar(_T("HKEY_CURRENT_USER\\Software\\AppDataLow\\Software\\baidu\\BaiduToolbar\\navhistory"));

	WCHAR szPath[MAX_PATH] = {0};
	std::wstring strPath;
	std::wstring strTemp;
	SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, FALSE);

	strTemp = szPath;
	if (strTemp.rfind(L'\\') != strTemp.size())
	{
		strTemp += L"\\";
	}
	strPath = strTemp;
	strPath += L"Baidu\\Toolbar";
	CSimpleArray<CString> vec_file;
	vec_file.Add(L"iexp.dat");
	vec_file.Add(L"logex.dat");
	g_fnScanFile(g_pMain, BEGINPROC(BAIDU_TOOLBAR), 0, 0, 0);
	m_appHistory.CommfunFile(BAIDU_TOOLBAR, strPath.c_str(), vec_file);
	g_fnScanFile(g_pMain, ENDPROC(BAIDU_TOOLBAR), 0, 0, 0);

	g_fnScanReg(g_pMain,ENDPROC(BAIDU_TOOLBAR),0,0,0);

	return TRUE;
}


BOOL CWindowsHistory::ScanBaiduToolBar(LPCTSTR lpszRegPath)
{
	CString strRegPath = lpszRegPath;
	
	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = BAIDU_TOOLBAR;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRegPath,hRootKey,strSubKey);

		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),myEnumerateReg,&userData);

	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;

}



BOOL CWindowsHistory::ScanWindowsPrgHistory()
{	
	g_fnScanFile(g_pMain,BEGINPROC(WINSHISTORY_PRGHISTORY),0,0,0);

	if (m_bScan==TRUE)
	{	
		CString  strPath =_T("");

		WCHAR* pEnv;
		pEnv = _wgetenv(_T("systemroot"));
		if (pEnv ==NULL)
		{	
			g_fnScanFile(g_pMain,ENDPROC(WINSHISTORY_PRGHISTORY),0,0,0);
			return FALSE;
		}
		strPath.Format(_T("%s\\Prefetch"),pEnv);

		CSimpleArray<CString> vec_file;
		//vec_file.Add(_T("*exe-*.pf"));
		m_appHistory.CommfunFile(WINSHISTORY_PRGHISTORY,strPath,vec_file);

	}
	
	g_fnScanFile(g_pMain,ENDPROC(WINSHISTORY_PRGHISTORY),0,0,0);

	return TRUE;
}

BOOL CWindowsHistory::ScanWindowsMemoryDMP()
{	

	g_fnScanFile(g_pMain,BEGINPROC(MEMORY_DMP),0,0,0);

	if (m_bScan==TRUE)
	{	
		CString  strPath =_T("");

		WCHAR* pEnv = NULL;
		pEnv = _wgetenv(_T("SystemRoot"));
		if (pEnv ==NULL)
		{	
			goto clean0;
		}

		strPath.Format(_T("%s"),pEnv);
		CSimpleArray<CString> vec_file;
		vec_file.Add(_T("MEMORY.DMP"));
		m_appHistory.CommfunFile(MEMORY_DMP,strPath,vec_file,FALSE,TRUE);
		vec_file.RemoveAll();


		strPath =_T("");
		strPath.Format(_T("%s\\Minidump"),pEnv);
		vec_file.Add(_T("*.dmp"));
		m_appHistory.CommfunFile(MEMORY_DMP,strPath,vec_file);

	}

clean0:
	g_fnScanFile(g_pMain,ENDPROC(MEMORY_DMP),0,0,0);

	return TRUE;
}

BOOL CWindowsHistory::CleanClipBoard()
{
	g_fnOutScanFile(g_pMain,BEGINPROC(WINSHISTORY_CLIPBOARD),0,0,0);
	
	int iCount = CountClipboardFormats();
	CString str;
	str = L"";

	if (0!=iCount)
	{
		str = L"可以清理";
		goto clean0;
	}

clean0:
	g_fnOutScanFile(g_pMain,ENDPROC(WINSHISTORY_CLIPBOARD),str,0,0);
	
	return TRUE;
}

BOOL CWindowsHistory::ScanNethood()
{
	TCHAR szFolderPath[MAX_PATH] = {0};
	wstring strPath;

	g_fnOutScanFile(g_pMain,BEGINPROC(WINDOWS_NETHOOD),0,0,0);

	::SHGetSpecialFolderPath( NULL, szFolderPath, CSIDL_NETHOOD, FALSE );

	strPath = szFolderPath;

	CSimpleArray<CString>  vec_value;
	m_appHistory.CommfunReg(WINDOWS_NETHOOD, strPath.c_str(),vec_value);

	g_fnOutScanFile(g_pMain,ENDPROC(WINDOWS_NETHOOD),0,0,0);

	return TRUE;
}

BOOL CWindowsHistory::ScanRecycle()
{
	g_fnOutScanFile(g_pMain,BEGINPROC(WINDOWS_RECYCLE),0,0,0);
	SHQUERYRBINFO syscleInfo = {0};
	syscleInfo.cbSize = sizeof(SHQUERYRBINFO);
	CString str;
	str = L"";

	int iCount = SHQueryRecycleBin(NULL, &syscleInfo);

	if (syscleInfo.i64NumItems != 0)
	{
		str = L"可以清理";
		goto clean0;
	}
clean0:
	g_fnOutScanFile(g_pMain,ENDPROC(WINDOWS_RECYCLE),str,0,0);
	return TRUE;
}

BOOL CWindowsHistory::ScanWindows7JumpList()
{
	WCHAR szPath[MAX_PATH] = {0};
	CString strPath;
	CString strTempPath;
	CSimpleArray<CString> vec_file;
	g_fnOutScanFile(g_pMain, BEGINPROC(WINDOWS_JUMPLIST), 0, 0, 0);

	SHGetSpecialFolderPath(NULL, szPath, CSIDL_RECENT, FALSE);

	strPath = szPath;
	if (strPath.ReverseFind(L'\\') != strPath.GetLength() - 1)
	{
		strPath += L"\\";
	}
	strTempPath = strPath;
	strPath += L"CustomDestinations";
	m_appHistory.CommfunFile(WINDOWS_JUMPLIST, strPath, vec_file);

	strPath = strTempPath;
	strTempPath += L"AutomaticDestinations";
	m_appHistory.CommfunFile(WINDOWS_JUMPLIST, strTempPath, vec_file);

	g_fnOutScanFile(g_pMain, ENDPROC(WINDOWS_JUMPLIST), 0, 0, 0);
	return TRUE;
}

BOOL CWindowsHistory::ScanQQToolBar()
{
	g_fnScanReg(g_pMain,BEGINPROC(QQ_TOOLBAR),0,0,0);
	g_fnScanFile(g_pMain,BEGINPROC(QQ_TOOLBAR),0,0,0);
	if (m_bScan==TRUE)
	{
		WCHAR szPathBuffer[MAX_PATH] = {0};
		SHGetSpecialFolderPath(NULL, szPathBuffer, CSIDL_APPDATA, FALSE);
		CSimpleArray<CString>  vec_value;
		std::wstring strFullPath;
		std::wstring strPathTemp;
		strPathTemp = szPathBuffer;
		if (strPathTemp.rfind(L'\\') != strPathTemp.size())
		{
			strPathTemp += L"\\";
		}
		strFullPath = strPathTemp;
		strFullPath += L"TENCENT\\QQToolbar\\visitHistory";
		m_appHistory.CommfunFile(QQ_TOOLBAR, strFullPath.c_str(), vec_value);

		strFullPath = strPathTemp;
		strFullPath += L"TENCENT\\QQToolbar\\PaneImg";
		m_appHistory.CommfunFile(QQ_TOOLBAR, strFullPath.c_str(), vec_value);

		CString strPath = _T("HKEY_CURRENT_USER\\Software\\AppDataLow\\Tencent\\QQToolbar\\History");	
		
		m_appHistory.CommfunReg(QQ_TOOLBAR,strPath,vec_value);

		strPath = _T("HKEY_CURRENT_USER\\Software\\Tencent\\QQToolbar\\History");	
		m_appHistory.CommfunReg(QQ_TOOLBAR,strPath,vec_value);
	}

	g_fnScanFile(g_pMain,ENDPROC(QQ_TOOLBAR),0,0,0);
	g_fnScanReg(g_pMain,ENDPROC(QQ_TOOLBAR),0,0,0);
	return TRUE;
}

BOOL CWindowsHistory::ScanWindowLiveToolBar()
{
	g_fnScanFile(g_pMain,BEGINPROC(WINLIVE_TOOLBAR),0,0,0);
	if (m_bScan==TRUE)
	{

		CString strPath = _T("%userprofile%\\AppData\\LocalLow\\Microsoft\\Search Enhancement Pack\\Search Box Extension");	

		CSimpleArray<CString>  vec_value;
		vec_value.Add(L"searchhs.dat");
		m_appHistory.CommfunFile(WINLIVE_TOOLBAR,strPath,vec_value);
	}

	g_fnScanFile(g_pMain,ENDPROC(WINLIVE_TOOLBAR),0,0,0);
	return TRUE;
}

BOOL CWindowsHistory::ScanWinSuol()
{
	g_fnScanFile(g_pMain,BEGINPROC(WINDOWS_SUOLIETU),0,0,0);
	WCHAR szBuffer[MAX_PATH] = {0};
	SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_LOCAL_APPDATA, NULL);
	PathAppend(szBuffer, L"Microsoft\\Windows\\Explorer");
	if (m_bScan==TRUE)
	{

		CString strPath = szBuffer;	

		CSimpleArray<CString>  vec_value;
		vec_value.Add(L"thumbcache_*.db");
		m_appHistory.CommfunFile(WINDOWS_SUOLIETU,strPath,vec_value);
	}

	g_fnScanFile(g_pMain,ENDPROC(WINDOWS_SUOLIETU),0,0,0);
	return TRUE;
}
//WINSHISTORY_CLIPBOARD