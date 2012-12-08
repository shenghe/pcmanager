#include "stdafx.h"
#include "regclean.h"
//#include "commfun.h"

CRegClean::CRegClean()
{
	m_bScan = TRUE;
}

CRegClean::~CRegClean()
{
	
}

BOOL CRegClean::myEnumerateFile(LPCTSTR lpFileOrPath, void* _pUserData,LARGE_INTEGER filesize)
{
	USERDATA* pUserData = (USERDATA*) _pUserData;
	CRegClean* pObject = (CRegClean*) pUserData->pObject;


	return TRUE;

}


BOOL CRegClean::myEnumerateReg(
									 HKEY rootKey,LPCTSTR lpItemPath,
									 LPCTSTR lpName,DWORD dwValueNameSize,
									 LPBYTE lpValue ,DWORD dwValueSize,DWORD dwType,
									 void* _pUserData
									 )
{	
	USERDATA* pUserData = (USERDATA*) _pUserData;
	CRegClean* pObject = (CRegClean*) pUserData->pObject;

	LPTSTR lpValueTmp = (LPTSTR) lpValue;
	LPTSTR lpNameTmp = (LPTSTR) lpName;
	

	switch (dwType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
		{	

		}	
	case REG_BINARY:			//除REG_SZ和REG_EXPAND_SZ其他值均忽略
		break;
	case REG_DWORD_LITTLE_ENDIAN:
	case REG_DWORD_BIG_ENDIAN:							
	case REG_LINK:									
	case REG_MULTI_SZ:
	case REG_NONE:
	case REG_QWORD_LITTLE_ENDIAN:
		break;
	}

	return TRUE;
}

/************************************************************************/
/* 无效dll
/************************************************************************/

BOOL CRegClean::ScanInvalidDll()
{	

	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_INVALIDDLL),0,0,0);
	CString strShareDllPath = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDlls");
	ScanInvalidDll(strShareDllPath.GetBuffer());
	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_INVALIDDLL),0,0,0);

	return TRUE;
}

BOOL CRegClean::ScanInvalidDll(LPCTSTR lpszShareDllPath)
{	
	if (m_bScan==FALSE)
		return TRUE;

	CString strShareDllPath = lpszShareDllPath;
	
	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = REGCLEAN_INVALIDDLL;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strShareDllPath,hRootKey,strSubKey);
		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),InvalidDll_myEnumerateReg,&userData);
	
		
	}
	catch (...)
	{	
		OutputDebugString(_T("崩溃:ScanInvalidDll"));
		return FALSE;
	}
	
	return TRUE;
}


BOOL CRegClean::InvalidDll(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{	
	if (m_bScan==FALSE)
		return TRUE;

	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

	if (!PathFileExists(lpValueName))
	{

		return g_fnScanReg(g_pMain,REGCLEAN_INVALIDDLL,
			strRegFullPath.GetBuffer(),
			lpValueName,
			_T("")
			);
	}


	return TRUE;
}

BOOL CRegClean::InvalidDll_myEnumerateReg(HKEY rootKey,LPCTSTR lpItemPath,LPCTSTR lpName,DWORD dwValueNameSize,LPBYTE lpValue ,DWORD dwValueSize,DWORD dwType,void* _pUserData)
{	

	USERDATA* pUserData = (USERDATA*) _pUserData;
	CRegClean* pObject = (CRegClean*) pUserData->pObject;

	LPTSTR lpValueTmp = (LPTSTR) lpValue;
	LPTSTR lpNameTmp = (LPTSTR) lpName;
	
	switch (pUserData->iType)
	{
	case REGCLEAN_INVALIDDLL:
		return pObject->InvalidDll(rootKey,lpItemPath,lpNameTmp,lpValueTmp);

	case REGCLEAN_INVALIDFIRE:
		return pObject->IvalidFire(rootKey,lpItemPath,lpNameTmp,lpValueTmp);

	case REGCLEAN_INVALIDMUI:
		return pObject->IvalidMUICache(rootKey,lpItemPath,lpNameTmp,lpValueTmp);

	case REGCLEAN_INVALIDINSTALL:
		return pObject->IvalidInstall(rootKey,lpItemPath,lpNameTmp,lpValueTmp);

	case REGCLEAN_INVALIDSTARTSRUN:
		return pObject->InvelidRun(rootKey,lpItemPath,lpNameTmp,lpValueTmp);


	}
	
	return TRUE;

}

/************************************************************************/
/* 无效无效右击菜单
/************************************************************************/

BOOL CRegClean::ScanInvalidMenu()
{	

	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_INVALIDMENU),0,0,0);

	CString strMenuHandler = _T("HKEY_CLASSES_ROOT\\*\\shellex\\ContextMenuHandlers");
	ScanInvalidMenu(strMenuHandler.GetBuffer());
	
	strMenuHandler = _T("HKEY_CLASSES_ROOT\\Directory\\shellex\\ContextMenuHandlers");
	ScanInvalidMenu(strMenuHandler.GetBuffer());

	strMenuHandler = _T("HKEY_CLASSES_ROOT\\Drive\\shellex\\ContextMenuHandlers");
	ScanInvalidMenu(strMenuHandler.GetBuffer());

	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_INVALIDMENU),0,0,0);	

	return TRUE;
}


BOOL CRegClean::ScanInvalidMenu(LPCTSTR lpcszMenuHandler)
{	
	if (m_bScan==FALSE)
		return TRUE;

	CString strMenuHandler = lpcszMenuHandler;

	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = REGCLEAN_INVALIDMENU;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strMenuHandler,hRootKey,strSubKey);
		
		CSimpleArray<CString> vec_subKey;
		m_regOpt.DoEnumCurrnetSubKey(hRootKey,strSubKey.GetBuffer(),vec_subKey);
		

		for (int i=0; i<vec_subKey.GetSize();i++)
		{
			CString _strSubKey =  strSubKey;
			_strSubKey.Append(_T("\\"));
			_strSubKey.Append(vec_subKey[i].GetBuffer());
			
			//扫描clsid
			CString strDefValue;
			CString strValueName=_T("");
			m_regOpt.GetDefValue(hRootKey,_strSubKey.GetBuffer(),strValueName,strDefValue);
			
			if (strDefValue == _T(""))
				continue;

			//HKEY_LOCAL_MACHINE
			CString strClsid = _T("SOFTWARE\\Classes\\CLSID");
			strClsid.Append(_T("\\"));
			strClsid.Append(strDefValue.GetBuffer());
			
			//验证是否存在
			if ( strDefValue.GetAt(0)!= '{')
			{
				continue;
			}

			if (!m_regOpt.FindKey(HKEY_LOCAL_MACHINE,strClsid.GetBuffer()))
			{	
				CString strRegFullPath;
				strRegFullPath = lpcszMenuHandler;
				strRegFullPath.Append(_T("\\"));
				strRegFullPath.Append(vec_subKey[i].GetBuffer());

				return g_fnScanReg(g_pMain,REGCLEAN_INVALIDMENU,
					strRegFullPath.GetBuffer(),
					_T(""),
					_T("")
					);
			}
		
		
		}
		
		
	}
	catch(...)
	{	
		OutputDebugString(_T("崩溃:ScanInvalidMenu"));
		return FALSE;
	}	

	return TRUE;
}

/************************************************************************/
/* 无效应用程序路径
/************************************************************************/
BOOL CRegClean::ScanInvalidAppPath()
{		

	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_INVALIDAPPPATH),0,0,0);
	CString strAppPath = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths");
	ScanInvalidAppPath(strAppPath.GetBuffer());
	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_INVALIDAPPPATH),0,0,0);

	return TRUE;
}

BOOL CRegClean::ScanInvalidAppPath(LPCTSTR lpcszAppPath)
{	
	if (m_bScan==FALSE)
		return TRUE;

	CString strAppPath  = lpcszAppPath;

	try
	{
		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strAppPath,hRootKey,strSubKey);
		
		//枚举当下键
		CSimpleArray<CString> vec_subKey;
		m_regOpt.DoEnumCurrnetSubKey(hRootKey,strSubKey.GetBuffer(),vec_subKey);
		
		for (int i=0;i<vec_subKey.GetSize();i++)
		{
			CString _strSubKey =  strSubKey;
			_strSubKey.Append(_T("\\"));
			_strSubKey.Append(vec_subKey[i].GetBuffer());

			//获得默认值
			CString strDefValue;
			CString strValueName=_T("");
			m_regOpt.GetDefValue(hRootKey,_strSubKey.GetBuffer(),strValueName,strDefValue);
			
			
			CString strRegFullPath = lpcszAppPath;
			strRegFullPath.Append(_T("\\"));
			strRegFullPath.Append(vec_subKey[i].GetBuffer());

			ConvetPath(strDefValue);
			if(TRUE==ValidDervice(strDefValue))
			{
				if (!PathFileExists(strDefValue.GetBuffer()))
				{
					g_fnScanReg(g_pMain,REGCLEAN_INVALIDAPPPATH,
						strRegFullPath.GetBuffer(),
						_T(""),
						_T("")
						);
				}
			}
			
		}

	}
	catch (...)
	{	

		OutputDebugString(_T("崩溃:ScanInvalidAppPath"));
		return FALSE;
	}

	return TRUE;
}

/************************************************************************/
/* 无效的防火墙设置
/************************************************************************/
BOOL CRegClean::ScanIvalidFire()
{	

	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_INVALIDFIRE),0,0,0);
	CString strFirePath = _T("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\SharedAccess\\Parameters\\FirewallPolicy\\DomainProfile\\AuthorizedApplications\\List");
	ScanIvalidFire(strFirePath.GetBuffer());
	strFirePath = _T("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\SharedAccess\\Parameters\\FirewallPolicy\\StandardProfile\\AuthorizedApplications\\List");
	ScanIvalidFire(strFirePath.GetBuffer());
	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_INVALIDFIRE),0,0,0);

	return TRUE;

}

BOOL CRegClean::ScanIvalidFire(LPCTSTR lpcszShareDllPath)
{	
	if (m_bScan==FALSE)
		return TRUE;

	CString strFirePath = lpcszShareDllPath;

	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = REGCLEAN_INVALIDFIRE;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strFirePath,hRootKey,strSubKey);
		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),InvalidDll_myEnumerateReg,&userData);


	}
	catch (...)
	{	
		OutputDebugString(_T("崩溃:ScanIvalidFire"));
		return FALSE;
	}

	return TRUE;
}


BOOL CRegClean::IvalidFire(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{	

	if (m_bScan==FALSE)
		return TRUE;


	CString strValueName = lpValueName;
	
	ConvetPath(strValueName);

	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

	if (!PathFileExists(strValueName.GetBuffer()))
	{
		return g_fnScanReg(g_pMain,REGCLEAN_INVALIDFIRE,
			strRegFullPath.GetBuffer(),
			lpValueName,
			_T("")
			);
	}

	return TRUE;

}



/************************************************************************/
/* 无效MUI缓存
/************************************************************************/


BOOL  CRegClean::ScanIvalidMUICache()
{	

	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_INVALIDMUI),0,0,0);
	CString strMUICachePath = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\ShellNoRoam\\MUICache");
	ScanIvalidMUICache(strMUICachePath.GetBuffer());
	strMUICachePath = _T("HKEY_CURRENT_USER\\Software\\Classes\\Microsoft\\windows\\shell\\MuiCache");
	ScanIvalidMUICache(strMUICachePath.GetBuffer());
	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_INVALIDMUI),0,0,0);

	return TRUE;
}

BOOL CRegClean::ScanIvalidMUICache(LPCTSTR lpcszMUICachePath)
{	
	if (m_bScan==FALSE)
		return TRUE;

	CString strFirePath = lpcszMUICachePath;

	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = REGCLEAN_INVALIDMUI;
		
		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strFirePath,hRootKey,strSubKey);
		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),InvalidDll_myEnumerateReg,&userData);


	}
	catch (...)
	{	
		OutputDebugString(_T("崩溃:ScanIvalidMUICache"));
		return FALSE;
	}

	return TRUE;
}


BOOL CRegClean::IvalidMUICache(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{	
	if (m_bScan==FALSE)
		return TRUE;

	CString strPath = lpValueName;	
	
	if (strPath != _T(""))
	{
		if (strPath.GetAt(0) == '@')
		{
			return TRUE;
		}
	}

	ConvetPath(strPath);

	if (ValidDervice(strPath)) //如果存在驱动器路径则进行验证
	{
		CString strRegFullPath;
		m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);
		if(!PathFileExists(strPath.GetBuffer()))
		{
			return g_fnScanReg(g_pMain,REGCLEAN_INVALIDMUI,
				strRegFullPath.GetBuffer(),
				lpValueName,
				_T("")
				);
		}


	}

	return TRUE;
}

/************************************************************************/
/* 无效的帮助文件
/************************************************************************/
BOOL CRegClean::ScanIvalidHelp()
{	

	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_INVALIDHELP),0,0,0);
	CString strHelpPath = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\Help");
	ScanIvalidHelp(strHelpPath.GetBuffer());
	
	strHelpPath = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\Html Help");
	ScanIvalidHelp(strHelpPath.GetBuffer());
	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_INVALIDHELP),0,0,0);

	return TRUE;
}

BOOL CRegClean::ScanIvalidHelp(LPCTSTR lpcszHelpPath)
{	
	if (m_bScan==FALSE)
		return TRUE;

	CString strHelpPath = lpcszHelpPath;
	
	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = REGCLEAN_INVALIDHELP;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strHelpPath,hRootKey,strSubKey);
		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),IvalidHelp_myEnumerateReg,&userData);


	}
	catch (...)
	{	
		OutputDebugString(_T("崩溃:ScanIvalidHelp"));
		return FALSE;
	}

	return TRUE;

}

BOOL CRegClean::IvalidHelp_myEnumerateReg(HKEY rootKey,LPCTSTR lpItemPath,LPCTSTR lpName,DWORD dwValueNameSize,LPBYTE lpValue ,DWORD dwValueSize,DWORD dwType,void* _pUserData)
{	
	
	USERDATA* pUserData = (USERDATA*) _pUserData;
	CRegClean* pObject = (CRegClean*) pUserData->pObject;

	LPTSTR lpValueTmp = (LPTSTR) lpValue;
	LPTSTR lpNameTmp = (LPTSTR) lpName;


	switch (dwType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
		{	
			switch (pUserData->iType)
			{
			case REGCLEAN_INVALIDHELP:
				return pObject->IvalidHelp(rootKey,lpItemPath,lpNameTmp,lpValueTmp);
			case REGCLEAN_INVALIDFONT:
				return pObject->IvalidFont(rootKey,lpItemPath,lpNameTmp,lpValueTmp);
			}

		}	
	case REG_BINARY:			//除REG_SZ和REG_EXPAND_SZ其他值均忽略
		break;
	case REG_DWORD_LITTLE_ENDIAN:
	case REG_DWORD_BIG_ENDIAN:							
	case REG_LINK:									
	case REG_MULTI_SZ:
	case REG_NONE:
	case REG_QWORD_LITTLE_ENDIAN:
		break;
	}

	return TRUE;
}

BOOL CRegClean::IvalidHelp(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{	


	if (m_bScan==FALSE)
		return TRUE;

	CString strPath = lpValueData;
	
	ConvetPath(strPath);

	if (ValidDervice(strPath)) //验证驱器格式..help如果格式不正确则直接报无效路径无需验证
	{	

		CString strRegFullPath;
		m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);
		
		int iLen = strPath.GetLength();
		if (strPath.GetAt(iLen-1)!= '\\')
		{
			strPath.Append(_T("\\"));
		}

		strPath.Append(lpValueName);

		if (!PathFileExists(strPath.GetBuffer()))
		{
			return g_fnScanReg(g_pMain,REGCLEAN_INVALIDHELP,
				strRegFullPath.GetBuffer(),
				lpValueName,
				lpValueData
				);
		}
		
	}
	else
	{	
		CString strRegFullPath;
		m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

		return g_fnScanReg(g_pMain,REGCLEAN_INVALIDHELP,
			strRegFullPath.GetBuffer(),
			lpValueName,
			_T("")
			);
	}
	
	OutputDebugString(_T("退出帮助函数\n"));
	return TRUE;
}

/************************************************************************/
/* 无效的字体
/************************************************************************/
BOOL CRegClean::ScanIvalidFont()
{	

	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_INVALIDFONT),0,0,0);
	CString strFontsPath =  _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");
	ScanIvalidFont(strFontsPath.GetBuffer());
	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_INVALIDFONT),0,0,0);
	
	return TRUE;
}

BOOL CRegClean::ScanIvalidFont(LPCTSTR lpcszFontsPath)
{	
	if (m_bScan==FALSE)
		return TRUE;

	CString strFontsPath = lpcszFontsPath;

	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = REGCLEAN_INVALIDFONT;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strFontsPath,hRootKey,strSubKey);
		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),IvalidHelp_myEnumerateReg,&userData);


	}
	catch (...)
	{	
		OutputDebugString(_T("崩溃:ScanIvalidFont"));
		return FALSE;
	}

	return TRUE;
}

BOOL CRegClean::IvalidFont(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{
	if (m_bScan==FALSE)
		return TRUE;

	CString strFontsPath = lpValueData;
	if(!ValidDervice(strFontsPath))
	{
		strFontsPath = _T("C:\\WINDOWS\\Fonts\\");
		strFontsPath.Append(lpValueData);
	}

	if (!PathFileExists(strFontsPath.GetBuffer()))
	{
		CString strRegFullPath;
		m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

		return g_fnScanReg(g_pMain,REGCLEAN_INVALIDFONT,
			strRegFullPath.GetBuffer(),
			lpValueName,
			_T("")
			);
	}

	
	return TRUE;
}

/************************************************************************/
/* 无效的安装程序
/************************************************************************/
BOOL CRegClean::ScanIvalidInstall()
{	
	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_INVALIDINSTALL),0,0,0);
	CString strInstallPath = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\Folders");
	ScanIvalidInstall(strInstallPath.GetBuffer());
	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_INVALIDINSTALL),0,0,0);

	return TRUE;
}

BOOL CRegClean::ScanIvalidInstall(LPCTSTR lpcszInstallPath)
{	
	if (m_bScan==FALSE)
		return TRUE;

	CString strInstallPath = lpcszInstallPath;

	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = REGCLEAN_INVALIDINSTALL;
		
		//判断无效路径
		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strInstallPath,hRootKey,strSubKey);
		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),InvalidDll_myEnumerateReg,&userData);
		
		ScanIvalidInstall2(_T("HKEY_LOCAL_MACHINE\\SOFTWARE"));
		ScanIvalidInstall2(_T("HKEY_CURRENT_USER\\SOFTWARE"));
	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CRegClean::ScanIvalidInstall2(LPCTSTR lpcszInstallPath)
{
	////空键,无值,无项
	HKEY	hRootKey;
	CString strSubKey;
	CString strSoftPath = lpcszInstallPath;
	m_regOpt.CrackRegKey(strSoftPath,hRootKey,strSubKey);

	CSimpleArray<CString> vec_Temp;
	m_regOpt.DoEnumCurrnetSubKey(hRootKey,strSubKey,vec_Temp);

	for(int i=0;i<vec_Temp.GetSize();i++)
	{	
		CString _strsubkey;			//
		_strsubkey = strSubKey;
		_strsubkey.Append(_T("\\"));
		_strsubkey.Append(vec_Temp[i]);

		CSimpleArray<REGVALUE> vec_value;
		m_regOpt.DoEnumCurrnetValue(hRootKey,_strsubkey,vec_value);

		CSimpleArray<CString> vec_key;
		m_regOpt.DoEnumCurrnetSubKey(hRootKey,_strsubkey,vec_key);

		if ( (vec_value.GetSize() == 0) &&
			(vec_key.GetSize() == 0)
			)
		{	
			CString strRegFullPath;
			m_regOpt.GetHKEYToString(hRootKey,_strsubkey,strRegFullPath);

			g_fnScanReg(g_pMain,REGCLEAN_INVALIDINSTALL,
				strRegFullPath.GetBuffer(),
				_T(""),
				_T("")
				);
		}

	}

	return TRUE;
}

BOOL CRegClean::IvalidInstall(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{
	if (m_bScan==FALSE)
		return TRUE;

	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

	if (!PathFileExists(lpValueName))
	{
		return g_fnScanReg(g_pMain,REGCLEAN_INVALIDINSTALL,
			strRegFullPath.GetBuffer(),
			lpValueName,
			_T("")
			);
	}

	return TRUE;
}

/************************************************************************/
/* 无效的反安装信息
/************************************************************************/
//键下项目为空

BOOL CRegClean::ScanInvalidUnInstall()
{	

	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_INVALIDUNINSTALL),0,0,0);
	CString strUnInstallPath = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
	ScanInvalidUnInstall(strUnInstallPath.GetBuffer());
	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_INVALIDUNINSTALL),0,0,0);


	return TRUE;
}

BOOL CRegClean::ScanInvalidUnInstall(LPCTSTR lpcszUnInstallPath)
{	
	if (m_bScan==FALSE)
		return TRUE;


	CString strUninstallPath = lpcszUnInstallPath;
	
	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = REGCLEAN_INVALIDUNINSTALL;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strUninstallPath,hRootKey,strSubKey);
		
		//子键
		CSimpleArray<CString> vec_subKey;
		m_regOpt.DoEnumCurrnetSubKey(hRootKey,strSubKey.GetBuffer(),vec_subKey);
		
		for (int i=0;i<vec_subKey.GetSize();i++)
		{	
			if (m_bScan==FALSE)
				return TRUE;
	
			//将上个键值与新键值重新拼凑
			CString strTmpSubKey;
			strTmpSubKey.Append(strSubKey.GetBuffer());
			strTmpSubKey.Append(_T("\\"));
			strTmpSubKey.Append(vec_subKey[i].GetBuffer());
			
			//string_t strValueName;
			//string_t strValueData;
			//m_regOpt.GetDefValue(hRootKey,strTmpSubKey.c_str(),strValueName,strValueData);
			
			//如果默认值为空
			//if ( 0 == strValueData.compare(_T("")))
			//{	
				CSimpleArray<REGVALUE> vec_regValue;
				m_regOpt.DoEnumCurrnetValue(hRootKey,strTmpSubKey.GetBuffer(),vec_regValue);
				
				//并且没有其他值
				if (vec_regValue.GetSize() == 0)
				{
					CString strRegFullPath;
					strRegFullPath.Append(lpcszUnInstallPath);
					strRegFullPath.Append(_T("\\"));
					strRegFullPath.Append(vec_subKey[i].GetBuffer());

					g_fnScanReg(g_pMain,REGCLEAN_INVALIDUNINSTALL,
						strRegFullPath.GetBuffer(),
						_T(""),
						_T("")
						);
				}
				else
				{	
					//UninstallString 下不为空且路径无效

					CString strValueName = _T("UninstallString");
					CString strValueData;
					m_regOpt.GetDefValue(hRootKey,strTmpSubKey,strValueName,strValueData);
					
					if (strValueData == _T(""))
					{
						continue;
					}

					
					CCmdLineParser cmdLineParser(strValueData);
					CString strPath = cmdLineParser.GetCmd();
					if (strPath == _T(""))
					{
						CString strRegFullPath;
						strRegFullPath.Append(lpcszUnInstallPath);
						strRegFullPath.Append(_T("\\"));
						strRegFullPath.Append(vec_subKey[i].GetBuffer());

						g_fnScanReg(g_pMain,REGCLEAN_INVALIDUNINSTALL,
							strRegFullPath.GetBuffer(),
							_T(""),
							_T("")
							);
					}
					
				}
				
			//}
		}


	}
	catch (...)
	{	

		OutputDebugString(_T("崩溃:ScanInvalidUnInstall\n"));
		return FALSE;	
	}
	

	return TRUE;
}

/************************************************************************/
/* 无效的开始菜单
// 开始目录下枚举所有文件夹名称.如果注册表项在这个文件夹名称里没有则为无效
/************************************************************************/
BOOL  CRegClean::ScanInvalidStartMenu()
{	

	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_INVALIDSTARTMENU),0,0,0);

	TCHAR szDesktopPath[MAX_PATH]={0};
	WCHAR* pEvn;
	pEvn = _wgetenv(_T("ALLUSERSPROFILE"));
	if (pEvn == NULL)
	{	
		OutputDebugString(_T("无效环境变量:ALLUSERPROFILE"));
		return FALSE;
	}


	wsprintf(szDesktopPath,_T("%s"),pEvn);
	
	
	CString strStartMenu = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MenuOrder\\Start Menu2\\Programs");
	ScanInvalidStartMenu(strStartMenu.GetBuffer(),szDesktopPath);

	strStartMenu = _T("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MenuOrder\\Start Menu\\Programs");
	ScanInvalidStartMenu(strStartMenu.GetBuffer(),szDesktopPath);

	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_INVALIDSTARTMENU),0,0,0);

	return TRUE;

}

BOOL CRegClean::ScanInvalidStartMenu(LPCTSTR lpcszStartMenu,LPCTSTR lpcszFolderPath)
{	
	if (m_bScan==FALSE)
		return TRUE;

	CString strRegStartMenu = lpcszStartMenu;

	try
	{
		//首先获得开始菜单根下的程序文件夹名称,方便查询将内容放入map中
		CSimpleArray<CString> vec_folder;
		m_fileOpt.DoFileFolder(lpcszFolderPath,vec_folder,TRUE,FALSE);
		
		TCHAR szDesktopPath[MAX_PATH]={0};
		
		WCHAR* pEvn;
		pEvn = _wgetenv(_T("USERPROFILE"));
		if (pEvn == NULL)
		{	
			OutputDebugString(_T("无效环境变量:USERPROFILE"));
			return FALSE;
		}

		wsprintf(szDesktopPath,_T("%s\\「开始」菜单"),pEvn);
		m_fileOpt.DoFileFolder(szDesktopPath,vec_folder,TRUE,FALSE);
		
		CAtlMap<CString,char>  map_folder;
		for (int i=0;i<vec_folder.GetSize();i++)
		{
			map_folder.SetAt(vec_folder[i],'1');
		}
		

		//将注册表中内容依次对比
		CSimpleArray<CString> vec_subKey;
		
		HKEY	hRootKey;	
		CString strSubKey;
		m_regOpt.CrackRegKey(strRegStartMenu,hRootKey,strSubKey);
		m_regOpt.DoEnumCurrnetSubKeyEx(hRootKey,strSubKey.GetBuffer(),vec_subKey,TRUE,TRUE);

		for (int i=0;i<vec_subKey.GetSize();i++)
		{
			if (m_bScan==FALSE)
				return TRUE;
			
			CString strTemp = vec_subKey[i];
			int iCount = strTemp.ReverseFind('\\');
			strTemp = strTemp.Mid(iCount+1);

			if (map_folder.Lookup(strTemp)==NULL)
			{
				CString strRegFullPath;
				strRegFullPath.Append(_T("HKEY_CURRENT_USER"));
				strRegFullPath.Append(_T("\\"));
				strRegFullPath.Append(vec_subKey[i]);

				g_fnScanReg(g_pMain,REGCLEAN_INVALIDSTARTMENU,
					strRegFullPath.GetBuffer(),
					_T(""),
					_T("")
					);
			}
			
			OutputDebugString(_T("\n"));
			OutputDebugString(vec_subKey[i]);
		}


	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}


/************************************************************************/
//无效的类信息
/************************************************************************/
BOOL CRegClean::ScanInvalidClass()
{	
	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_FAILCLASS),0,0,0);
	CString strRegClass = _T("HKEY_CLASSES_ROOT\\CLSID");
	ScanInvalidClass(strRegClass.GetBuffer());


	return TRUE;
}


BOOL CRegClean::ScanInvalidClass(LPCTSTR lpcszRegClassPath)
{	
	if (m_bScan == FALSE)
		return TRUE;

	CString strRegClass = lpcszRegClassPath;

	try
	{
		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRegClass,hRootKey,strSubKey);
		
		//获得类型库信息
		CSimpleArray<CString> vec_clisdSubKey;
		m_regOpt.DoEnumCurrnetSubKey(hRootKey,strSubKey.GetBuffer(),vec_clisdSubKey);

		
		for (int i=0;i<vec_clisdSubKey.GetSize();i++)
		{
			/************************************************************************/
			//说明:
			//	InprocServer32 下无效路径进行判别
			//	LocalServer32 下路径比较复杂故没有实现后期进行实现
			/************************************************************************/
			if (m_bScan == FALSE)
				return TRUE;	
			
			CString strInprocServer32SubKey = strSubKey;

			BEGIN 
			strInprocServer32SubKey.Append(_T("\\"));
			strInprocServer32SubKey.Append(vec_clisdSubKey[i].GetBuffer());
			strInprocServer32SubKey.Append(_T("\\InprocServer32"));
			
			CString strValueName=_T("");
			CString strValueData=_T("");
			m_regOpt.GetDefValue(hRootKey,strInprocServer32SubKey.GetBuffer(),strValueName,strValueData);
		
			if(strValueData != _T(""))
			{	
				CCmdLineParser cmdLineParer(strValueData);
				CString strPath = cmdLineParer.GetCmd();
				
				if (strPath ==_T(""))
				{	
					CString strRegFullPath;
					strRegFullPath.Append(lpcszRegClassPath);
					strRegFullPath.Append(_T("\\"));
					strRegFullPath.Append(vec_clisdSubKey[i]);

					g_fnScanReg(g_pMain,REGCLEAN_FAILCLASS,
						strRegFullPath,
						_T(""),
						strValueData
						);	
				}

				//验证路径
				//CString strFullPath;
				//ConvetPath(strValueData);
				//if (TRUE==ValidDervice(strValueData))
				//{	
					/*if (!PathFileExists(strValueData.GetBuffer()))
					{	

						CString strRegFullPath;
						strRegFullPath.Append(lpcszRegClassPath);
						strRegFullPath.Append(_T("\\"));
						strRegFullPath.Append(vec_clisdSubKey[i].GetBuffer());

						g_fnScanReg(g_pMain,REGCLEAN_FAILCLASS,
								strRegFullPath.GetBuffer(),
								_T(""),
								strValueData.GetBuffer()
								);
					}	*/
				//}
			}
			
			END
			
			//LocalServer32 部分判别

			strInprocServer32SubKey = strSubKey;
			BEGIN 
			strInprocServer32SubKey.Append(_T("\\"));
			strInprocServer32SubKey.Append(vec_clisdSubKey[i].GetBuffer());
			strInprocServer32SubKey.Append(_T("\\LocalServer32"));

			CString strValueName=_T("");
			CString strValueData=_T("");
			m_regOpt.GetDefValue(hRootKey,strInprocServer32SubKey.GetBuffer(),strValueName,strValueData);

			if(strValueData != _T(""))
			{	
				CCmdLineParser cmdLineParer(strValueData);
				CString strPath = cmdLineParer.GetCmd();

				if (strPath ==_T(""))
				{	
					CString strRegFullPath;
					strRegFullPath.Append(lpcszRegClassPath);
					strRegFullPath.Append(_T("\\"));
					strRegFullPath.Append(vec_clisdSubKey[i]);

					g_fnScanReg(g_pMain,REGCLEAN_FAILCLASS,
						strRegFullPath,
						_T(""),
						strValueData
						);	
				}

			}

			END

		}

		return FALSE;

	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CRegClean::ScanInvalidClass2()
{	

	CString strInterface = _T("HKEY_CLASSES_ROOT\\Interface");
	ScanInvalidClass2(strInterface.GetBuffer());

	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_FAILCLASS),0,0,0);

	return TRUE;
}

BOOL CRegClean::ScanInvalidClass2(LPCTSTR lpcszRegClassPath)
{	
	if (m_bScan == FALSE)
		return TRUE;	


	try
	{	
		CString strRegClass = _T("HKEY_CLASSES_ROOT\\TypeLib");

		HKEY	hRootKey;
		CString strSubKey;

		//获得Interface下的所有clsid
		strRegClass = lpcszRegClassPath;
		m_regOpt.CrackRegKey(strRegClass,hRootKey,strSubKey);
		CSimpleArray<CString> vec_interface;
		m_regOpt.DoEnumCurrnetSubKey(hRootKey,strSubKey.GetBuffer(),vec_interface);
		
		for (int i=0;i<vec_interface.GetSize();i++)
		{	
			if (m_bScan == FALSE)
				return TRUE;

			if(_T("{23576951-CC2B-11D2-BC59-006008BF0962}") == vec_interface[i])
			{
				int i=0;
			}

			CString strTypeLibSubKey = strSubKey;
			strTypeLibSubKey.Append(_T("\\"));
			strTypeLibSubKey.Append(vec_interface[i].GetBuffer());
			strTypeLibSubKey.Append(_T("\\TypeLib"));

			CString strValueName=_T("");
			CString strValueData=_T("");
			if (m_regOpt.GetDefValue(hRootKey,strTypeLibSubKey.GetBuffer(),strValueName,strValueData) ==TRUE)
			{
				if (strValueData == _T(""))
				{
					//无效
					CString strRegFullPath;
					strRegFullPath.Append(lpcszRegClassPath);
					strRegFullPath.Append(_T("\\"));
					strRegFullPath.Append(vec_interface[i].GetBuffer());

					g_fnScanReg(g_pMain,REGCLEAN_FAILCLASS,
						strRegFullPath.GetBuffer(),
						_T(""),
						strValueData.GetBuffer()
						);
				}
				else
				{	
					CString  strTemp;
					strTemp.Append(_T("TypeLib\\"));
					strTemp.Append(strValueData);

					
					BOOL bRet = m_regOpt.FindKey(HKEY_CLASSES_ROOT,strTemp.GetBuffer());
					if (bRet == FALSE)
					{
							//无效
							CString strRegFullPath;
							strRegFullPath.Append(lpcszRegClassPath);
							strRegFullPath.Append(_T("\\"));
							strRegFullPath.Append(vec_interface[i].GetBuffer());

							g_fnScanReg(g_pMain,REGCLEAN_FAILCLASS,
								strRegFullPath.GetBuffer(),
								_T(""),
								strValueData.GetBuffer()
								);
					}

				}
			}
			else
			{	
				//如果为无效默认值
				if (ERROR_NODEF == m_regOpt.GetErrorCode())
				{
					//无效
					CString strRegFullPath;
					strRegFullPath.Append(lpcszRegClassPath);
					strRegFullPath.Append(_T("\\"));
					strRegFullPath.Append(vec_interface[i].GetBuffer());

					g_fnScanReg(g_pMain,REGCLEAN_FAILCLASS,
						strRegFullPath.GetBuffer(),
						_T(""),
						_T("")
						);
				}
			}

		}
		
	

	}
	catch (...)
	{
		return FALSE;	
	}

	return TRUE;
	
}

/************************************************************************/
/* 无效的ActiveX信息
/************************************************************************/
//待完善 shell等情况

BOOL  CRegClean::ScanInvelidActiveX()
{	


	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_FAILACTIVEX),0,0,0);

	try
	{
		CSimpleArray<CString> vec_Clisd;
		m_regOpt.DoEnumCurrnetSubKey(HKEY_CLASSES_ROOT,_T(""),vec_Clisd);

		for (int i=0;i<vec_Clisd.GetSize();i++)
		{	
			if (m_bScan  == FALSE)
				goto End;

			CString strTypeLibSubKey = vec_Clisd[i];
			strTypeLibSubKey.Append(_T("\\CLSID"));

			CString strValueName=_T("");
			CString strValueData=_T("");
			if (m_regOpt.GetDefValue(HKEY_CLASSES_ROOT,strTypeLibSubKey.GetBuffer(),strValueName,strValueData) ==TRUE)
			{
				if (strValueData == _T(""))
				{
					//无效
					CString strRegFullPath = _T("HKEY_CLASSES_ROOT");
					strRegFullPath.Append(_T("\\"));
					strRegFullPath.Append(vec_Clisd[i].GetBuffer());

					g_fnScanReg(g_pMain,REGCLEAN_FAILACTIVEX,
						strRegFullPath.GetBuffer(),
						_T(""),
						strValueData.GetBuffer()
						);
				}
				else
				{	//{D27CDB6E-AE6D-11cf-96B8-444553540000}
					CString  strTemp;
					strTemp.Append(_T("CLSID\\"));
					strTemp.Append(strValueData);
					
					if (strValueData == _T("{D27CDB6E-AE6D-11cf-96B8-444553540000}"))
					{
						int i =1;
					}

					BOOL bRet = m_regOpt.FindKey(HKEY_CLASSES_ROOT,strTemp.GetBuffer());
					if (bRet == FALSE)
					{
						//无效
						CString strRegFullPath = _T("HKEY_CLASSES_ROOT");
						strRegFullPath.Append(_T("\\"));
						strRegFullPath.Append(vec_Clisd[i].GetBuffer());

						g_fnScanReg(g_pMain,REGCLEAN_FAILACTIVEX,
							strRegFullPath.GetBuffer(),
							_T(""),
							strValueData.GetBuffer()
							);
					}

				}
			}

		}

	}
	catch (...)
	{
		return FALSE;	
	}
End:
	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_FAILACTIVEX),0,0,0);
	return TRUE;
}

BOOL  CRegClean::ScanInvelidActiveX(LPCTSTR strActiveXPath)
{	
	if (m_bScan ==FALSE)
		return TRUE;


	return TRUE;
}

/************************************************************************/
/* 无效启动项
/************************************************************************/
BOOL CRegClean::ScanInvelidRun()
{	

	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_INVALIDSTARTSRUN),0,0,0);
	CString strRunPath = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	ScanInvelidRun(strRunPath.GetBuffer());
	strRunPath = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
	ScanInvelidRun(strRunPath.GetBuffer());
	strRunPath = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");
	ScanInvelidRun(strRunPath.GetBuffer());
	strRunPath = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce");
	ScanInvelidRun(strRunPath.GetBuffer());
	strRunPath = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run");
	ScanInvelidRun(strRunPath.GetBuffer());
	strRunPath = _T("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	ScanInvelidRun(strRunPath.GetBuffer());
	strRunPath = _T("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
	ScanInvelidRun(strRunPath.GetBuffer());
	strRunPath = _T("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");
	ScanInvelidRun(strRunPath.GetBuffer());
	strRunPath = _T("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce");
	ScanInvelidRun(strRunPath.GetBuffer());
	strRunPath = _T("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run");
	ScanInvelidRun(strRunPath.GetBuffer());
	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_INVALIDSTARTSRUN),0,0,0);
	return TRUE;
}

BOOL CRegClean::ScanInvelidRun(LPCTSTR lpcszRunPath)
{	
	if (m_bScan == FALSE)
		return TRUE;

	try
	{
		CString strRunPath = lpcszRunPath;

		USERDATA userData;
		userData.pObject= this;
		userData.iType = REGCLEAN_INVALIDSTARTSRUN;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRunPath,hRootKey,strSubKey);

		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),InvalidDll_myEnumerateReg,&userData);
		
	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;

}

BOOL CRegClean::InvelidRun(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{	
	if (m_bScan == FALSE)
		return TRUE;

	CCmdLineParser cmdParser(lpValueData);
	CString strPath = cmdParser.GetCmd();

	if (strPath == "" && (lpValueName!=NULL) &&(lpValueData!=NULL))
	{	
		CString _strPath= lpValueData;
		if (ValidDervice(_strPath))
		{
	
			CString strRegFullPath;
			m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

			return g_fnScanReg(g_pMain,REGCLEAN_INVALIDSTARTSRUN,
					strRegFullPath.GetBuffer(),
					lpValueName,
					lpValueData
					);
		}
		
	}

	return TRUE;

}

/************************************************************************/
/* 无效的文件关联
/************************************************************************/
BOOL CRegClean::ScanValidFileAss()
{	
	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_FAILAFILEASS),0,0,0);
	try
	{	

		if (m_bScan  == FALSE)
			goto End;

		CSimpleArray<CString> vec_Temp;
		m_regOpt.DoEnumCurrnetSubKey(HKEY_CLASSES_ROOT,_T(""),vec_Temp);
		
		//获得所有扩展名
		CSimpleArray<CString> vec_fileExts;
		for (int i=0; i<vec_Temp.GetSize();i++)
		{
			if (vec_Temp[i].GetAt(0)=='.')
			{
				vec_fileExts.Add(vec_Temp[i]);
			}
		}
		
		//取得默认值
		for(int i=0;i<vec_fileExts.GetSize();i++)
		{	
			CString strValueName;
			CString strValueData;
				
			if (m_bScan  == FALSE)
				goto End;

			if(TRUE==m_regOpt.GetDefValue(HKEY_CLASSES_ROOT,vec_fileExts[i].GetBuffer(),strValueName,strValueData))
			{
				if (strValueData!=_T(""))
				{
					BOOL bRet=m_regOpt.FindKey(HKEY_CLASSES_ROOT,strValueData.GetBuffer());

					if (bRet == FALSE)
					{
						CString strRegFullPath = _T("HKEY_CLASSES_ROOT");
						strRegFullPath.Append(_T("\\"));
						strRegFullPath.Append(vec_fileExts[i].GetBuffer());

						g_fnScanReg(g_pMain,REGCLEAN_FAILAFILEASS,
							strRegFullPath.GetBuffer(),
							_T(""),
							_T("")
							);
					}
					else
					{
						//如果找到
						/*CString strRegExePath;
						strRegExePath = strValueData;
						strRegExePath.Append(_T("\\shell\\open\\command"));
					
						
						CString strValueName;
						CString strValueData;
						if (TRUE==m_regOpt.GetDefValue(HKEY_CLASSES_ROOT,strRegExePath,strValueName,strValueData))
						{
							CCmdLineParser cmdLineParser(strValueData);
							CString strPath = cmdLineParser.GetCmd();
							
							if (strPath == _T(""))
							{		
								CString strRegFullPath = _T("HKEY_CLASSES_ROOT");
								strRegFullPath.Append(_T("\\"));
								strRegFullPath.Append(vec_fileExts[i].GetBuffer());

								g_fnScanReg(g_pMain,REGCLEAN_FAILAFILEASS,
									strRegFullPath.GetBuffer(),
									_T(""),
									_T("")
									);
							}
						}*/

					}
				}
			}
			else
			{
				DWORD dwErrCode = m_regOpt.GetErrorCode();
				if (dwErrCode == ERROR_NODEF)
				{	
					CSimpleArray<REGVALUE> vec_value;
					m_regOpt.DoEnumCurrnetValue(HKEY_CLASSES_ROOT,vec_fileExts[i],vec_value,TRUE);
					if (vec_value.GetSize() == 0)
					{
						CString strRegFullPath = _T("HKEY_CLASSES_ROOT");
						strRegFullPath.Append(_T("\\"));
						strRegFullPath.Append(vec_fileExts[i].GetBuffer());

						g_fnScanReg(g_pMain,REGCLEAN_FAILAFILEASS,
							strRegFullPath.GetBuffer(),
							_T(""),
							_T("")
							);
					}

				}

			}
		}

	}
	catch (...)
	{
		return FALSE;
	}
	
End:

	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_FAILAFILEASS),0,0,0);
	return TRUE;
}

BOOL CRegClean::ScanValidFileAss(LPCTSTR strFileAssPath)
{
	return TRUE;
}

/************************************************************************/
/* 通用处理函数
/************************************************************************/




//将存在环境变量的路径进行转化
BOOL CRegClean::ConvetPath(CString& strSourcPath)
{	
	if (strSourcPath == _T(""))
	{
		return TRUE;
	}

	try
	{
		if(strSourcPath.GetAt(0) == '%')
		{	
			int iEnd = strSourcPath.Find('%',1);
			CString strEnvPath = strSourcPath.Mid(1,iEnd-1);
			CString strPath = strSourcPath.Mid(iEnd+2);
			
			wchar_t* pEnv;
			pEnv = _wgetenv(strEnvPath.GetBuffer());
			if (pEnv!=NULL)
			{
				strEnvPath = pEnv;
				int iLen = strEnvPath.GetLength();
				if (strEnvPath.GetAt(iLen-1)!='\\')
				{
					strEnvPath.Append(_T("\\"));
				}

				strEnvPath.Append(strPath.GetBuffer());
				strSourcPath = strEnvPath;
			}

		}
	}
	catch(...)
	{
		return FALSE;
	}
	
	return TRUE;
}

//验证是否存在驱动盘符
BOOL CRegClean:: ValidDervice(CString& strPath)
{	
	if (strPath == _T(""))
	{
		return FALSE;
	}

	if (
		((strPath.GetAt(0)>='a'||strPath.GetAt(0)<='z')&&(strPath.GetAt(1)==':')&&(strPath.GetAt(2)=='\\'))||
		((strPath.GetAt(0)>='A'||strPath.GetAt(0)<='Z')&&(strPath.GetAt(1)==':')&&(strPath.GetAt(2)=='\\'))
		)
	{
		return TRUE;
	}
	
	if ((((strPath.GetAt(0)=='"')&&(strPath.GetAt(1)>='a'||strPath.GetAt(1)<='z')&&(strPath.GetAt(2)==':')&&(strPath.GetAt(3)=='\\'))||
		((strPath.GetAt(0)=='"')&&(strPath.GetAt(1)>='A'||strPath.GetAt(1)<='Z')&&(strPath.GetAt(2)==':')&&(strPath.GetAt(3)=='\\'))))
	{
		strPath.Replace(_T("\""),_T(""));

		return TRUE;
	}

	return FALSE;
}


BOOL CRegClean::ScanRegRedundancey()
{	
	g_fnScanReg(g_pMain,BEGINPROC(REGCLEAN_REDUNDANCY),0,0,0);
	CString strRegPath = _T("HKEY_CURRENT_USER\\Software");
	ScanRegRedundancey(strRegPath);

	//strRegPath = _T("HKEY_LOCAL_MACHINE\\Software");
	//ScanRegRedundancey(strRegPath);

	g_fnScanReg(g_pMain,ENDPROC(REGCLEAN_REDUNDANCY),0,0,0);

	return TRUE;
}

BOOL  CRegClean::ScanRegRedundancey(LPCTSTR lpcszRegPath)
{	
	if (m_bScan ==FALSE)
	{
		return TRUE;
	}

	CString strRegPath = lpcszRegPath;
	
	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = REGCLEAN_REDUNDANCY;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRegPath,hRootKey,strSubKey);

		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),RegRedancey_myEnumerateReg,&userData);
	}
	catch (...)
	{
		OutputDebugString(_T("崩溃:ScanRegRedundancey"));
		return  FALSE;
	}


	return TRUE;
}

//if (fnRegFun(rootKey, szRoot, NULL,NULL,NULL,NULL, -1, pUserData))
//{
//	DoRegEnumeration(rootKey, szRoot, fnRegFun, pUserData);
//}

BOOL CRegClean::RegRedancey_myEnumerateReg(HKEY rootKey,LPCTSTR lpItemPath,LPCTSTR lpName,DWORD dwValueNameSize,LPBYTE lpValue ,DWORD dwValueSize,DWORD dwType,void* _pUserData)
{	
	USERDATA* pUserData = (USERDATA*) _pUserData;
	CRegClean* pObject = (CRegClean*) pUserData->pObject;

	LPTSTR lpValueTmp = (LPTSTR) lpValue;
	LPTSTR lpNameTmp = (LPTSTR) lpName;

	//如果dwType为-1则为路径
	if(dwType == -1)
	{
		
		CString strRegFullPath;
		pObject->m_regOpt.GetHKEYToString(rootKey,lpItemPath,strRegFullPath);
		//过滤路径
		strRegFullPath.MakeUpper();

		if(strRegFullPath == _T("HKEY_CURRENT_USER\\SOFTWARE\\CLASSES")||
			strRegFullPath ==_T("HKEY_CURRENT_USER\\SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\EXPLORER")||
			strRegFullPath == _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\CLASSES")||
			strRegFullPath ==_T("HKEY_LOCAL_MACHINE\\SOFTWARE\\MICROSOFT\\WINDOWS\\CurrentVersion\\EXPLORER")||
			strRegFullPath ==_T("HKEY_CURRENT_USER\\SOFTWARE\\MICROSOFT\\VISUALSTUDIO")||
			-1!=strRegFullPath.Find(_T("DEFAULTICON"))
			)
		{
			return FALSE;
		}
	}

	pObject->RegRedundancey(rootKey,lpItemPath,lpNameTmp,_T(""));

	switch (dwType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
		{	

			pObject->RegRedundancey(rootKey,lpItemPath,lpNameTmp,lpValueTmp);
		}	
	case REG_BINARY:			//除REG_SZ和REG_EXPAND_SZ其他值均忽略
		break;
	case REG_DWORD_LITTLE_ENDIAN:
	case REG_DWORD_BIG_ENDIAN:							
	case REG_LINK:									
	case REG_MULTI_SZ:
	case REG_NONE:
	case REG_QWORD_LITTLE_ENDIAN:
		break;
	}

	return TRUE;
}
BOOL CRegClean::RegRedundancey(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData)
{	

	//键
	if (lpValueData == NULL)
	{
		CString strValueData = lpValueName;

		if (ValidDervice(strValueData)==TRUE)
		{	
			CCmdLineParser cmdLinePaser(lpValueData);
			CString strPath = cmdLinePaser.GetCmd();
			if (strPath == _T(""))
			{	
				if (!PathFileExists(lpValueName))
				{
					CString strRegFullPath;
					m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

					return g_fnScanReg(g_pMain,REGCLEAN_REDUNDANCY,strRegFullPath,lpValueName,lpValueData);
				}

			}
		}
	}
	else //值
	{
		CString strValueData = lpValueData;
		
		if(lpValueName!=NULL)
		{
			//处理图标
			//eg:D:\PROGRA~1\MICROS~1\Office12\VISIO.EXE, 1
			if (
				0==wcscmp(_T("Large Icon"),lpValueName)||
				0==wcscmp(_T("Small Icon"),lpValueName)||
				0==wcscmp(_T("DefaultIcon"),lpValueName)||
				0==wcscmp(_T("icon"),lpValueName)
				)
			{	
				int iLen = strValueData.Find(_T(","));
				if (iLen>0)
				{
					strValueData = strValueData.Mid(0,iLen);
				}
				
			}

		}
		
		//多路径情况 路径;路径;路径
		if (-1 !=strValueData.Find(_T(";")))
		{
			return TRUE;
		}


		if (ValidDervice(strValueData)==TRUE)
		{
			CCmdLineParser cmdLinePaser(strValueData);
			CString strPath = cmdLinePaser.GetCmd();
			if (strPath == _T(""))
			{	
				if (!PathFileExists(lpValueData))
				{
					CString strRegFullPath;
					m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);

					return g_fnScanReg(g_pMain,REGCLEAN_REDUNDANCY,strRegFullPath,lpValueName,lpValueData);
				}
			}
		}

	}
	

	return TRUE;
}