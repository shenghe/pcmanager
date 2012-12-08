#include "BeikeUtils.h"
#include <expevaluate/expevaluate.h>
#include "regularexpress/RegularExpress.h"
#include <common/registrywow.h>

//#define TRACE_FUNCTION_EVAL _tprintf
#define TRACE_FUNCTION_EVAL	


BOOL EvaluateCondition(LPCTSTR lpszExpression)
{
	return EvaluateCondition(lpszExpression, DefaultEvaluateFunction);
}

BOOL EvaluateCondition(LPCTSTR lpszExpression, Function_EvaluateCallback pfnCallback)
{
	if( _tcslen(lpszExpression)>0 )
	{
		USES_CONVERSION;
		std::string strA = CT2CA(lpszExpression);
		CExpEvaluate p(strA);
		return abs(p.Evaluate( pfnCallback )) > 0.0001;
	}
	return FALSE;
}


enum T_WOW_NODIRECTION_FLAGS
{
	WOW_NDF_FILE = 1, 
	WOW_NDF_REG  = 2
};

typedef BOOL (*FunctionDo)(const std::vector<std::string> &params, double &evalResult, DWORD dwWowFlags);
#define DO_FUNCTION_ENTRY(arg) OneArgumentFunctions [#arg] = arg

static char g_is64 = -1;

BOOL _CheckFileVersion( CString strPath, CString strOp, CString strVersion )
{
	LARGE_INTEGER lfile;
	BOOL bResult = FALSE;
	if( GetFileVersion( strPath, lfile ) &&  CompareVersion( lfile, ParseVersion(strVersion), strOp, bResult ) )
	{
		;
	}	
	return bResult;
}

// fileexist("windows\microsoft.net\framework\v3.5","microsoft.build.tasks.v3.5.dll")
// path, filename 
BOOL Dofileexists(const std::vector<std::string> &params, double &evalResult, DWORD dwWowFlags)
{
	USES_CONVERSION;

	CString strPath;	
	if(params.size()==1)
		strPath = CA2CT( params[0].c_str() );
	else if (params.size()==2)
		strPath.Format(_T("%s\\%s"), A2T(params[0].c_str()), A2T(params[1].c_str()));
	else
		return FALSE;
	
	CWowFsRedirectDisableHelper _no_redirect((dwWowFlags & WOW_NDF_FILE) && g_is64);
	ExpandFilePath(strPath);
	evalResult = IsFileExist( strPath );
	TRACE_FUNCTION_EVAL(_T("fe%d('%s') %d\n"), dwWowFlags, strPath, (BOOL)evalResult);
	return TRUE;
}

// fileversion("system","urlmon.dll","<","7.0.6000.16945")
// fileversion("windows\apppatch","aclayers.dll","<","5.1.2600.3647")
BOOL Dofileversion(const std::vector<std::string> &params, double &evalResult, DWORD dwWowFlags)
{
	if(params.size()!=3)
		return FALSE;
	
	USES_CONVERSION;	
	CString strPath = CA2CT( params[0].c_str() );
	CString strOp = CA2CT( params[1].c_str() );
	CString strVersion = CA2CT( params[2].c_str() );
	
	CWowFsRedirectDisableHelper _no_redirect((dwWowFlags & WOW_NDF_FILE) && g_is64);
	ExpandFilePath(strPath);	
	evalResult = _CheckFileVersion(strPath, strOp, strVersion);
	TRACE_FUNCTION_EVAL(_T("fv%d('%s', '%s', '%s') %d\n"), dwWowFlags, strPath, strOp, strVersion, (BOOL)evalResult);
	return TRUE;
}

void GetRegSubKeyArray( CString & strKey, CSimpleArray<CString> & strSubKeyArray, HKEY hKey, BOOL bNoDirect)
{	
	BOOL bOpened = FALSE;
	CRegKey	cKey;
	if(bNoDirect)
	{
		HKEY hSubKey = NULL;
		CRegistryWow<REGISTRY_WOW> reg;
		bOpened = ERROR_SUCCESS==reg.OpenKeyEx(hKey, strKey, 0, KEY_READ, &hSubKey);
		if(bOpened)
			cKey.Attach(hSubKey);
	}
	else
	{
		bOpened = cKey.Open(hKey, strKey, KEY_READ)==ERROR_SUCCESS;
	}
	if ( bOpened )
	{
		DWORD	iIndex = 0;
		do 
		{
			TCHAR	szName[MAX_PATH] = {0};
			DWORD	nLen = MAX_PATH;

			LONG nRet = cKey.EnumKey(iIndex, szName,&nLen);
			if ( nRet == ERROR_SUCCESS )
			{
				strSubKeyArray.Add( CString(szName) );
			}
			else if ( nRet == ERROR_MORE_DATA )
			{
				CString	strSubX;
				nLen = 1024;
				nRet = cKey.EnumKey(iIndex, strSubX.GetBuffer(nLen), &nLen);
				strSubX.ReleaseBuffer();

				if ( !strSubX.IsEmpty() )
					strSubKeyArray.Add(strSubX);
			}
			else
				break;

			iIndex++;
		} while (TRUE);
	}
	return;
}

// regkeyexist("hklm","SOFTWARE\Microsoft\RemovalTools\MRT")
// 
BOOL Doregkeyexist(const std::vector<std::string> &params, double &evalResult, DWORD dwWowFlags)
{
	USES_CONVERSION;
	CString strKey = CA2CT( params[0].c_str() );
	CString strSubkey;

	if(params.size()==2)
		strSubkey = CA2CT( params[1].c_str() );
	else if (params.size()==3)
		strSubkey.Format(_T("%s\\%s"),A2T(params[1].c_str()), A2T(params[2].c_str()));
	else
		return FALSE;
	
	HKEY hKey;
	if( !TransHKey(strKey, hKey) )
		return FALSE;

	TRACE_FUNCTION_EVAL(_T(" re :%s \\ %s \n"), strKey, strSubkey);

	if ( strSubkey.Find( _T("[*]")) == -1 )
	{
		evalResult = RegKeyExistsEx( hKey, strSubkey, (dwWowFlags & WOW_NDF_REG) && g_is64);
	}
	else
	{
		CString	strParKey;
		CString	strMatchKey;
		int iPos = strSubkey.ReverseFind(_T('\\'));

		if ( iPos != -1 )
		{
			strParKey = strSubkey.Left(iPos);
			strMatchKey = strSubkey.Mid(iPos+1,strSubkey.GetLength()-iPos);
		}
		else
		{
			strMatchKey = strSubkey;
		}

		strMatchKey.Replace( _T("[*]"), _T(".*") );
		strMatchKey.Replace( _T("[?]"),  _T(".?") );
		strMatchKey.Replace( _T("\\"),  _T("\\\\"));
		CSimpleArray<CString>	strSubKeyS;
		GetRegSubKeyArray(strParKey,strSubKeyS,hKey,(dwWowFlags & WOW_NDF_REG) && g_is64);

		for ( int i = 0; i < strSubKeyS.GetSize(); i++)
		{
			const unsigned short* lpMatch = (const unsigned short*)(LPCTSTR)strMatchKey;
			const unsigned short* lpSubKey = (const unsigned short*)(LPCTSTR)strSubKeyS[i];
			CRegexp regExp(lpMatch, IGNORECASE);

			MatchResult result = regExp.MatchExact( lpSubKey );
			if (!result.IsMatched())
				continue;

			evalResult = TRUE;
			break;
		}
	}

	return TRUE;
}
//regkeyfileexist("HKLM","SOFTWARE\Rising\Rav","installpath","Rav.exe")

BOOL Doregfileexist(const std::vector<std::string> &params, double &evalResult, DWORD dwWowFlags)
{
	if(params.size()!=4)
		return FALSE;

	USES_CONVERSION;
	CString strKey = CA2CT( params[0].c_str() );
	CString strSubkey = CA2CT( params[1].c_str() );
	CString strName = CA2CT( params[2].c_str() );
	CString strFilename = CA2CT( params[3].c_str() );

	HKEY hKey;
	if( !TransHKey(strKey, hKey) )
		return FALSE;

	CString sPath;
	BOOL bReadReg = ReadRegStringEx(hKey, strSubkey, strName, sPath, (dwWowFlags & WOW_NDF_REG) && g_is64);

	if( !bReadReg )
	{
		evalResult = FALSE;
	}
	else
	{
		// check file path 
		if( !strFilename.IsEmpty() )
		{
			PathAppend(sPath.GetBuffer(MAX_PATH), strFilename);
			sPath.ReleaseBuffer();
		}
		
		CWowFsRedirectDisableHelper _no_redirect((dwWowFlags & WOW_NDF_FILE) && g_is64);
		evalResult  = IsFileExist( sPath );
		TRACE_FUNCTION_EVAL(_T("rfe%d('%s') %d\n"), dwWowFlags, sPath, (BOOL)evalResult);
	}	
	return TRUE;
}

// regvalueexist("hklm","SOFTWARE\Microsoft\Windows\CurrentVersion\Component Based Servicing\Packages\package_[*]_for_KB931573[*]","installtimelow")
BOOL Doregvalueexist(const std::vector<std::string> &params, double &evalResult, DWORD dwWowFlags)
{
	if(params.size()!=3)
		return FALSE;
	
	USES_CONVERSION;
	CString strKey = CA2CT( params[0].c_str() );
	CString strSubkey = CA2CT( params[1].c_str() );
	CString strName = CA2CT( params[2].c_str() );

	HKEY hKey;
	if( !TransHKey(strKey, hKey) )
		return FALSE;
	
	if ( strSubkey.Find( _T("[*]")) == -1 )
	{
		evalResult = RegValueExistsEx(hKey, strSubkey, strName, (dwWowFlags & WOW_NDF_REG) && g_is64);
	}
	else
	{
		CString	strParKey;
		CString	strMatchKey;
		int iPos = strSubkey.ReverseFind(_T('\\'));

		if ( iPos != -1 )
		{
			strParKey = strSubkey.Left(iPos);
			strMatchKey = strSubkey.Mid(iPos+1,strSubkey.GetLength()-iPos);
		}
		else
		{
			strMatchKey = strSubkey;
		}

		strMatchKey.Replace( _T("[*]"), _T(".*") );
		strMatchKey.Replace( _T("[?]"),  _T(".?") );
		strMatchKey.Replace( _T("\\"),  _T("\\\\"));
		CSimpleArray<CString>	strSubKeyS;
		GetRegSubKeyArray(strParKey,strSubKeyS,hKey,(dwWowFlags & WOW_NDF_REG) && g_is64);

		for ( int i = 0; i < strSubKeyS.GetSize(); i++)
		{
			const unsigned short* lpMatch = (const unsigned short*)(LPCTSTR)strMatchKey;
			const unsigned short* lpSubKey = (const unsigned short*)(LPCTSTR)strSubKeyS[i];
			CRegexp regExp(lpMatch, IGNORECASE);

			MatchResult result = regExp.MatchExact( lpSubKey );
			if (!result.IsMatched())
				continue;

			CString	strFullKey;
			
			if ( !strParKey.IsEmpty() )
			{
				strFullKey = strParKey;
				strFullKey += _T("\\");
				strFullKey += strSubKeyS[i];
			}
			else
			{
				strFullKey = strSubKeyS[i];
			}
			if ( RegValueExistsEx(hKey, strFullKey, strName, (dwWowFlags & WOW_NDF_REG) && g_is64) )
			{
				evalResult = TRUE;
				break;
			}
		}
	}

	return TRUE;
}

// regfileversion("HKCR","clsid\{f2175210-368c-11d0-ad81-00a0c90dc8d9}\inprocserver32","","..\snapview.exe",">=","0.0.0.0"))
BOOL Doregfileversion(const std::vector<std::string> &params, double &evalResult, DWORD dwWowFlags)
{
	if(params.size()!=6)
		return FALSE;

	USES_CONVERSION;
	CString strKey = CA2CT( params[0].c_str() );
	CString strSubkey = CA2CT( params[1].c_str() );
	CString strName = CA2CT( params[2].c_str() );
	CString strFilename = CA2CT( params[3].c_str() );
	CString strOp = CA2CT( params[4].c_str() );
	CString strTestVal = CA2CT( params[5].c_str() );

	HKEY hKey;
	if( !TransHKey(strKey, hKey) )
		return FALSE;

	CString sPath;
	BOOL bReadReg = ReadRegStringEx(hKey, strSubkey, strName, sPath, (dwWowFlags & WOW_NDF_REG) && g_is64);
	if( !bReadReg )
	{
		evalResult = FALSE;
	}
	else
	{
		// check file path 
		if( !strFilename.IsEmpty() )
		{
			PathAppend(sPath.GetBuffer(MAX_PATH), strFilename);
			sPath.ReleaseBuffer();
		}

		CWowFsRedirectDisableHelper _no_redirect((dwWowFlags & WOW_NDF_FILE) && g_is64);
		evalResult = _CheckFileVersion(sPath, strOp, strTestVal);
	}	
	return TRUE;
}

//regvalue("hklm","SYSTEM\CurrentControlSet\Control\Windows","CSDVersion",">=","256")
BOOL Doregvalue(const std::vector<std::string> &params, double &evalResult, DWORD dwWowFlags)
{
	if(params.size()!=5)
		return FALSE;
	
	USES_CONVERSION;
	CString strKey = CA2CT( params[0].c_str() );
	CString strSubkey = CA2CT( params[1].c_str() );
	CString strName = CA2CT( params[2].c_str() );
	CString strOp = CA2CT( params[3].c_str() );
	CString strTestVal = CA2CT( params[4].c_str() );
	
	HKEY hKey;
	if( !TransHKey(strKey, hKey) )
		return FALSE;
	
	HKEY hSubKey;
	if((dwWowFlags & WOW_NDF_REG) && g_is64)
	{
		CRegistryWow<REGISTRY_WOW> reg;
		reg.OpenKeyEx(hKey, strSubkey, 0, KEY_READ, &hSubKey);
	}
	else
		RegOpenKeyEx(hKey, strSubkey, 0, KEY_READ, &hSubKey);
		
	DWORD dwType;
	DWORD dwSize;
	CString strVal;

	if( hSubKey )
	{
		if( ERROR_SUCCESS == SHGetValue( hSubKey, _T(""), strName, &dwType, NULL, &dwSize ) )
		{
			LPTSTR p = new TCHAR[dwSize+4];
			memset(p, 0, sizeof(TCHAR)*(dwSize+4));
			SHGetValue( hSubKey, _T(""), strName, &dwType, (void*)p, &dwSize );
			if( dwType == REG_SZ || dwType==REG_EXPAND_SZ )
				strVal = p;
			else if(dwType==REG_DWORD)
				strVal.Format(_T("%d"), *(DWORD*)p );
			delete[] p;
		}

		RegCloseKey(hSubKey);
		hSubKey = NULL;
	}
	
	BOOL bResult;
	if( !CompareString( strVal, strTestVal, strOp, bResult ) )
		return FALSE;
	evalResult = bResult;
	return TRUE;
}

// osversion(ver, sp)
BOOL Doosversion(const std::vector<std::string> &params, double &evalResult, DWORD dwWowFlags)
{
	if(params.size()!=2)
		return FALSE;
	
	USES_CONVERSION;
	CString strOsVer = CA2CT( params[0].c_str() );
	CString strSP = CA2CT( params[1].c_str() );

	static T_WindowsInfo *wininfo = NULL;
	if(wininfo==NULL)
	{
		wininfo = new T_WindowsInfo;
		wininfo->Init();
	}
		
	CString s1, s2;
	s1.Format(_T("%d.%d"), wininfo->osvi.dwMajorVersion, wininfo->osvi.dwMinorVersion);
	s2.Format(_T("%d"), wininfo->osvi.wServicePackMajor);
	evalResult = (strOsVer.IsEmpty() || strOsVer==s1) && (strSP.IsEmpty()||strSP==s2);
	return TRUE;
}

// ossp("<","3")
// 检测 os 的xp 小于某个值 
BOOL Doosspcompare(const std::vector<std::string> &params, double &evalResult, DWORD dwWowFlags)
{
	if(params.size()!=2)
		return FALSE;
	
	USES_CONVERSION;
	CString strOp = CA2CT( params[0].c_str() );
	CString strTestVal = CA2CT( params[1].c_str() );
	INT nVal = _ttoi( strTestVal );
	
	static T_WindowsInfo *wininfo = NULL;
	if(wininfo==NULL)
	{
		wininfo = new T_WindowsInfo;
		wininfo->Init();
	}
	
	BOOL bTestresult;
	BOOL bRet = EvalCompareResult(wininfo->nSP - nVal, strOp, bTestresult );
	evalResult = bTestresult;
	return bRet;
}



bool DefaultEvaluateFunction(const char *funcname, const std::vector<std::string> &params, double &evalResult, long userdata)
{
	struct TFunctionMapStruct{
		const char *funcname;
		FunctionDo func;
	};
		
	static TFunctionMapStruct functions[] = {
		{"fe",		Dofileexists},
		{"fv",		Dofileversion},
		{"rk",		Doregkeyexist},
		{"rv",		Doregvalue},
		{"rve",		Doregvalueexist},		
		{"rfv",		Doregfileversion},
		{"rfe",		Doregfileexist},

		{"os",		Doosversion},
		{"ossp",	Doosspcompare},
	};
	
	const char *funcname_orig = funcname;

	if(g_is64==-1)
	{
		g_is64 = IsWindows64() ? 1 : 0; 
	}
	
	DWORD dwFlags = 0;
	if(*funcname=='z' || *funcname=='Z')
	{
		INT shift = 0;
		while(*funcname=='z' || *funcname=='Z')
		{
			if(*funcname=='Z')
				dwFlags |= (1<<shift);
			++funcname;
			++shift;
		}
	}

	BOOL bRet = FALSE;
	for(int i=0; i<sizeof(functions)/sizeof(TFunctionMapStruct); ++i)
	{
		if( stricmp(funcname, functions[i].funcname)==0 )
		{
			bRet = (functions[i].func( params, evalResult, dwFlags ) == TRUE);
			return bRet;
		}
	}
	return false;
}

