#include "BeikeUtils.h"
#include <ShlObj.h>

struct T_SpecialFolder
{
	LPCTSTR szFolder;
	int nFolder;
};

#define SPECIAL_FOLDER(x) {_T(#x), CSIDL_##x}
static T_SpecialFolder _SpecialFolder_Clsid[] =
{
	SPECIAL_FOLDER(WINDOWS),
	SPECIAL_FOLDER(SYSTEM),
	SPECIAL_FOLDER(PROGRAM_FILES),
	SPECIAL_FOLDER(PROGRAM_FILES_COMMON),
	
	// Personal 
	SPECIAL_FOLDER(COOKIES),
	SPECIAL_FOLDER(DESKTOP),	
	SPECIAL_FOLDER(DESKTOPDIRECTORY),
	SPECIAL_FOLDER(FAVORITES),
	SPECIAL_FOLDER(FONTS),
	SPECIAL_FOLDER(HISTORY),
	SPECIAL_FOLDER(LOCAL_APPDATA),
	SPECIAL_FOLDER(PERSONAL),
	SPECIAL_FOLDER(MYDOCUMENTS),
	SPECIAL_FOLDER(MYPICTURES),	

	SPECIAL_FOLDER(PROGRAMS),
	SPECIAL_FOLDER(RECENT),
	SPECIAL_FOLDER(SENDTO),
	SPECIAL_FOLDER(STARTMENU),
	SPECIAL_FOLDER(STARTUP),
	
	// common 
	SPECIAL_FOLDER(COMMON_APPDATA),
	SPECIAL_FOLDER(COMMON_DESKTOPDIRECTORY),
	SPECIAL_FOLDER(COMMON_DOCUMENTS),
	SPECIAL_FOLDER(COMMON_PROGRAMS),
	SPECIAL_FOLDER(COMMON_STARTMENU),
	SPECIAL_FOLDER(COMMON_STARTUP),
};

struct T_OfficeVersion
{
	INT nVerPub;			// 	
	INT nVerInternal;		// 
};

static T_OfficeVersion _office_versions[] =
{
	{ 2000,	9},
	{ 2002,	10},
	{ 2003,	11},
	{ 2007,	12},
	{ 2010, 14},
};


BOOL GetCLSIDFilename( LPCTSTR lpszClsid, CString &strFilename )
{
	CString strSubkey;
	strSubkey.Format( _T("CLSID\\%s\\InprocServer32"), lpszClsid );
	return ReadRegString(HKEY_CLASSES_ROOT, strSubkey, NULL, strFilename);
}

void ExpandFilePath( CString &strPath )
{
	int nOffset = strPath.Find( '\\' );
	if( nOffset > 0 ) // 替换之前的路径
	{
		CString strSpecial;
		strSpecial.SetString(strPath, nOffset);		
		ParseSpecialFolder( strSpecial );
		strPath.Format( _T("%s\\%s"),strSpecial, strPath.Mid( nOffset + 1 ));
	}
	else // 
		ParseSpecialFolder( strPath );
}

BOOL _TransClsid( LPCTSTR lpszClsid, CString &strPath )
{
	CString strFilename;
	if( GetCLSIDFilename(lpszClsid, strFilename) )
	{
		LPTSTR lpszFileName = PathFindFileName(strFilename);
		if(lpszFileName)
		{
			strPath.SetString(strFilename, (int)(lpszFileName-(LPCTSTR)strFilename) );
			return TRUE;
		}
	}
	return FALSE;
}

BOOL _TransSpecialFolder( LPCTSTR lpszSpecialFolder, CString &strPath )
{
	// Shell Folders
	if(ReadRegString(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), lpszSpecialFolder, strPath) )
		return TRUE;
	
	for(int i=0; i<sizeof(_SpecialFolder_Clsid)/sizeof(T_SpecialFolder); ++i)
	{
		if(_tcsicmp(_SpecialFolder_Clsid[i].szFolder, lpszSpecialFolder) == 0)
		{
			BOOL bRet = SHGetSpecialFolderPath(NULL, strPath.GetBuffer(MAX_PATH), _SpecialFolder_Clsid[i].nFolder, FALSE);
			strPath.ReleaseBuffer();
			return bRet;
		}
	}
	return FALSE;
}

BOOL _TransOfficeCommonDir( LPCTSTR lpszSpecialFolder, CString &strPath )
{
	LPCTSTR szDot = _tcschr(lpszSpecialFolder, _T('.'));
	if(!szDot) return FALSE;
	
	++szDot;
	INT nOfficeVersion = _tcstol(szDot, NULL, 10);
	for(int i=0; i<sizeof(_office_versions)/sizeof(_office_versions[0]); ++i)
	{
		if(nOfficeVersion!=_office_versions[i].nVerPub)
			continue;
		
		TCHAR  szPath[MAX_PATH] = {0};
		if( SHGetSpecialFolderPath(NULL,szPath, CSIDL_PROGRAM_FILES_COMMON, FALSE ) )
		{
			strPath.Format(_T("%s\\Microsoft Shared\\OFFICE%d"), szPath, _office_versions[i].nVerInternal);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

BOOL _TransVisioDir( int nOfficeVersion, CString& strPath )
{
	if ( 11 == nOfficeVersion )
	{
		///> visio2003的installRoot放在与office common中会导致
		CString strSubKey;	
		strSubKey = _T("SOFTWARE\\Microsoft\\Office\\11.0\\Common\\InstallRoot");
		if( !ReadRegString(HKEY_LOCAL_MACHINE, strSubKey, _T("Path"), strPath) )
			return FALSE;

		strPath.TrimRight( L'\\' );
		int nTrim = strPath.ReverseFind( _T('\\') );
		if ( -1 != nTrim )
		{
			strPath = strPath.Left( nTrim + 1 );
			strPath += _T("Visio11\\");

			return TRUE;
		}

		return FALSE;
	}
	else
	{
		///> visio2003之后的版本
		///> 注册表中得出的路径 HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Office\%d.0\Visio\InstallRoot
		CString strSubKey;	
		strSubKey.Format( _T("SOFTWARE\\Microsoft\\Office\\%d.0\\Visio\\InstallRoot"), nOfficeVersion );
		return ReadRegString(HKEY_LOCAL_MACHINE, strSubKey, _T("Path"), strPath);
	}
}

BOOL _TransOfficeDir( LPCTSTR lpszSpecialFolder, CString& strPath )
{
	LPCTSTR szDot = _tcschr(lpszSpecialFolder, _T('.'));
	if(!szDot) return FALSE;
	
	++szDot;
	INT nOfficeVersion = _tcstol(szDot, NULL, 10);

	for(int i=0; i<sizeof(_office_versions)/sizeof(_office_versions[0]); ++i)
	{
		if(nOfficeVersion!=_office_versions[i].nVerPub)
			continue;
		
		int nVerInternal = _office_versions[i].nVerInternal;

		///> 检测是否针对visio的特殊路径
		CString strSpecialFolder = lpszSpecialFolder;
		strSpecialFolder.MakeLower();
		if ( -1 != strSpecialFolder.Find( _T("visio") ) )
		{
			return _TransVisioDir( nVerInternal, strPath );
		}

		CString strSubKey;
		CString strName;
		strName.SetString(lpszSpecialFolder, (int)(szDot-lpszSpecialFolder-1));
		strSubKey.Format(_T("SOFTWARE\\Microsoft\\Office\\%d.0\\Common\\InstallRoot"), nVerInternal);
		return ReadRegString(HKEY_LOCAL_MACHINE, strSubKey, _T("Path"), strPath);
	}
	return FALSE;
}

BOOL ParseSpecialFolder( CString &strSpecialFolder )
{
	CString strTemp;
	if( ParseSpecialFolder( strSpecialFolder, strTemp ) )
	{
		strSpecialFolder = strTemp;
		return !strSpecialFolder.IsEmpty();
	}
	return FALSE;
}

// 1. {00020810-0000-0000-C000-000000000046}		// clsid 
// 2. @SHGetSpecialFolderPath						// special folder 
// 3. &office common dir							// &Office.2003 
// 4. %office install path							// office install path	%Office.2003, %Word.2003 %
// 5. *other type folder							// 其他类型的folder 
BOOL ParseSpecialFolder(LPCTSTR lpszSpecialFolder, CString& strPath)
{
	if(lpszSpecialFolder == NULL)
		return FALSE;

	TCHAR magic = *lpszSpecialFolder;
	switch(magic)
	{
	case '!':
		ATLASSERT(FALSE);
		break;
	case '{':	// clsid 
		return _TransClsid(lpszSpecialFolder, strPath);
	case '@':	// SHGetSpecialFolderPath
		return _TransSpecialFolder(++lpszSpecialFolder, strPath);
	case '&':
		return _TransOfficeCommonDir(++lpszSpecialFolder, strPath);
	case '%':
		return _TransOfficeDir(++lpszSpecialFolder, strPath);
	default:
		break;
	}

	// systemRoot -> c:\\ 
	TCHAR szBuffer[MAX_PATH] = {0};
	if(_tcsicmp(lpszSpecialFolder, _T("systemroot") ) == 0 )
	{
		if( GetWindowsDirectory(szBuffer, MAX_PATH) )
		{
			BOOL bRet = PathStripToRoot(szBuffer);
			if(bRet)
				strPath = szBuffer;
			return bRet;
		}
		return FALSE;
	}
	return FALSE;
}