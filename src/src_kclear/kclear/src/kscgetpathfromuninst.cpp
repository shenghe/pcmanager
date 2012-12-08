#include "stdafx.h"
#include "kscgetpathfromuninst.h"
//#include "AnalyzeCmdLine.h"

#define MAX_KEY_LENGTH   256



BOOL KSearchSoftwarePathFromUninstallList::SearchSoftwarePathFromUninstallList( 
	LPCTSTR  pszDisplayName,
	LPCTSTR  pszFileName,
	LPTSTR   pszMainDir,
	int      nOutSize
	)
{
	if( pszDisplayName    == NULL || 
		pszDisplayName[0] == 0    ||
		pszFileName       == NULL || 
		pszFileName[0]    == 0    || 
		pszMainDir        == NULL ||
		nOutSize          <= 0 
		)
	{
		return FALSE;
	}


	m_pszDisplayName = pszDisplayName;
	m_pszFileName    = pszFileName;
	m_pszMainDir     = pszMainDir;
	m_nOutSize       = nOutSize;
	m_pszMainDir[0]  = 0;

	BOOL bRet = QueryUninstallData();

	m_pszDisplayName = NULL;
	m_pszFileName    = NULL;
	m_pszMainDir     = NULL;
	m_nOutSize       = 0;

	return pszMainDir[0] != 0;
}



BOOL KSearchSoftwarePathFromUninstallList::QueryUninstallData()
{
	BOOL     bRet        = FALSE;
	DWORD    dwRet       = ERROR_SUCCESS;
	HKEY     hRootKey    = HKEY_LOCAL_MACHINE;
	HKEY     hKey        = NULL; 
	HKEY     hSubKey     = NULL;
	DWORD    dwSize      = 0;
	FILETIME ftLastRec                = {0};
	CString  strRegKeyUninstall       = TEXT( "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall" );
	TCHAR    szSubKey[MAX_KEY_LENGTH] = {0};  
	TCHAR    szBuffer[MAX_KEY_LENGTH] = {0};
	

	try
	{
		dwRet = ::RegOpenKeyEx( hRootKey, strRegKeyUninstall, 0, KEY_READ, &hKey );
		if( dwRet != ERROR_SUCCESS )
			throw 0;


		for( int i = 0; true; i++ )
		{
			dwSize = MAX_KEY_LENGTH;
			dwRet  = ::RegEnumKeyEx( hKey, i, szSubKey, &dwSize, NULL, NULL, NULL, &ftLastRec );
			if( dwRet != ERROR_SUCCESS )
				break;


			if( ::RegOpenKeyEx( hKey, szSubKey, 0, KEY_READ, &hSubKey ) == ERROR_SUCCESS )
			{
				UNINSTALL_DATA  uiData;


				dwSize = MAX_KEY_LENGTH * sizeof(TCHAR);
				dwRet = ::RegQueryValueEx( hSubKey, TEXT( "DisplayName" ), 0, NULL, (LPBYTE)szBuffer, &dwSize );	
				if( dwRet == ERROR_SUCCESS )
					uiData.strDisplayName = szBuffer;


				dwSize = MAX_KEY_LENGTH * sizeof(TCHAR);
				dwRet = ::RegQueryValueEx( hSubKey, TEXT( "DisplayIcon" ), 0, NULL, (LPBYTE)szBuffer, &dwSize );	
				if( dwRet == ERROR_SUCCESS )
					uiData.strDisplayIcon = szBuffer;


				dwSize = MAX_KEY_LENGTH * sizeof(TCHAR);
				dwRet = ::RegQueryValueEx( hSubKey, TEXT( "UninstallString" ), 0, NULL, (LPBYTE)szBuffer, &dwSize );	
				if( dwRet == ERROR_SUCCESS )
					uiData.strUninstallString = szBuffer;


				::RegCloseKey( hSubKey );


				if( !OnFoundUninstallItem( &uiData ) )
					break;
			}
		}


		bRet = TRUE;
	}
	catch( int )
	{
		bRet = FALSE;
	}


	if( hKey != NULL )
	{
		::RegCloseKey( hKey );
		hKey = NULL;
	}

	return bRet;
}




BOOL KSearchSoftwarePathFromUninstallList::OnFoundUninstallItem( const UNINSTALL_DATA* pUninstallData )
{
	// return TRUE if need to continue.

	if( pUninstallData->strDisplayName.Find( m_pszDisplayName ) == -1 )
		return TRUE;

	BOOL bRet   = FALSE;
	BOOL bFound = FALSE;
	CString strStartPath;
	ATL::CPath strRetPath;



	bRet = GetPathFromDisplayIcon( pUninstallData->strDisplayIcon, strStartPath );
	if( bRet )
	{
		bFound = SearchFileInSpecPath( m_pszFileName, strStartPath, strRetPath );
		if( bFound )
			goto Exit0;
	}


	bRet = GetPathFromUninstallString( pUninstallData->strUninstallString, strStartPath );
	if( bRet )
	{
		bFound = SearchFileInSpecPath( m_pszFileName, strStartPath, strRetPath );
		if( bFound )
			goto Exit0;
	}



Exit0:
	if( bFound )
	{
		_tcsncat_s( m_pszMainDir, m_nOutSize, strRetPath, m_nOutSize - 1 );
		return FALSE; // Needn't to continue.
	}

	return TRUE;
}



BOOL KSearchSoftwarePathFromUninstallList::SearchFileInSpecPath( 
	LPCTSTR pszFileName, 
	LPCTSTR pszStartPath, 
	ATL::CPath& strPath
	)
{
	if( pszFileName == NULL || pszFileName[0] == 0 ||
		pszStartPath == NULL || pszStartPath[0] == 0
		)
	{
		return FALSE;
	}


	BOOL bFound = FALSE;
	strPath = pszStartPath;

    if (!strPath.IsDirectory())
        return FALSE;

    strPath += m_pszFileName;
	if (strPath.FileExists())
	{
		bFound = TRUE;
	}

	if (!bFound && !strPath.IsRoot())
    {
        strPath.RemoveFileSpec();
    }

	return bFound;
}



BOOL KSearchSoftwarePathFromUninstallList::GetPathFromDisplayIcon( 
	LPCTSTR pszDisplayIcon, 
	CString& strPath 
	)
{
	if (NULL == pszDisplayIcon)
		return FALSE;

    if (!_tcslen(pszDisplayIcon))
        return FALSE;

	TCHAR szBuffer[MAX_PATH] = {0};
	if (!GetFirstPath( pszDisplayIcon, szBuffer, MAX_PATH ))
		return FALSE;

	if (!::PathRemoveFileSpec( szBuffer ))
		return FALSE;

	strPath = szBuffer;
	return TRUE;
}


BOOL KSearchSoftwarePathFromUninstallList::GetPathFromUninstallString( 
	LPCTSTR pszUninstallString, 
	CString& strPath 
	)
{
	if( pszUninstallString == NULL )
		return FALSE;

	TCHAR szBuffer[MAX_PATH] = {0};
	if( !GetFirstPath( pszUninstallString, szBuffer, MAX_PATH ) )
		return FALSE;

	if( !::PathRemoveFileSpec( szBuffer ) )
		return FALSE;

	strPath = szBuffer;
	return TRUE;
}


BOOL KSearchSoftwarePathFromUninstallList::SearchFirstPath( LPCTSTR pszCmdLine, LPCTSTR* ppszStart, LPCTSTR* ppszEnd )
{
	BOOL bRet = FALSE;

	try
	{
		LPCTSTR pszFilePathReg = TEXT( "([a-zA-Z]:(\\\\[^\\\\/:*?\"<>|]+)+)" );


		CAtlRegExp<> re;
		REParseError status = re.Parse( pszFilePathReg );

		if( REPARSE_ERROR_OK != status )
			throw 0;


		CAtlREMatchContext<> mc;
		if( !re.Match( pszCmdLine, &mc ) )
			throw 0;


		ptrdiff_t nLength = mc.m_Match.szEnd - mc.m_Match.szStart;
		if( nLength <= 0 )
			throw 0;

		*ppszStart = mc.m_Match.szStart;
		*ppszEnd   = mc.m_Match.szEnd;

		bRet = TRUE;
	}

	catch( int )
	{
		bRet = FALSE;
	}

	return bRet;
}

LPCTSTR KSearchSoftwarePathFromUninstallList::HasRundll( LPCTSTR pszCmdLine )
{
	if (pszCmdLine == NULL || _tcslen(pszCmdLine) == 0)
		return NULL;

	TCHAR* pszBuffer = new TCHAR[::_tcslen( pszCmdLine ) + 1];
	if( pszBuffer == NULL )
		return NULL;

	::_tcscpy( pszBuffer, pszCmdLine );
	::_tcslwr( pszBuffer );

	LPCTSTR pszRunDllText = _T( "rundll" );
	LPCTSTR pszRunDll = ::_tcsstr( pszBuffer, pszRunDllText );

	if( pszRunDll != NULL )
	{
		pszRunDll = pszCmdLine + ( pszRunDll - pszBuffer ) + ::_tcslen( pszRunDllText );
	}

	delete [] pszBuffer;
	return pszRunDll; // 返回"rundll"字符后的位置
}



inline
BOOL KSearchSoftwarePathFromUninstallList::IsNumber( TCHAR c )
{
	return ( c >= '0' ) && ( c <= '9' );
}

inline 
BOOL KSearchSoftwarePathFromUninstallList::IsAlpha( TCHAR c )
{
	return ( ( c >= 'a' ) && ( c <= 'z' ) ) || 
		( ( c >= 'A' ) && ( c <= 'Z' ) ) ;
}

inline
BOOL KSearchSoftwarePathFromUninstallList::IsPossibleSeperator( TCHAR c )
{
	//return ( c == ' ' ) || ( c == ',' ) || ( c == ';' ) || ( c == '-' ) ;
	return ( c < 128 ) && !IsNumber( c ) && !IsAlpha( c );
}


BOOL KSearchSoftwarePathFromUninstallList::IsFileExists( LPCTSTR pszFile )
{
	return ::PathFileExists( pszFile ) && !PathIsDirectory( pszFile );
}


BOOL KSearchSoftwarePathFromUninstallList::_KillParamEx( LPCTSTR pszCmdLine, LPTSTR pszFileName, int nSize )
{
	BOOL   bRet       = FALSE;
	TCHAR* pszTemp    = NULL;

	try
	{
		if( pszCmdLine == NULL || pszFileName == NULL || nSize <= 0 )
			throw 0;

		int nTempSize = nSize + 128;

		pszTemp = new TCHAR[nTempSize];
		if( pszTemp == NULL )
			throw 0;

		//-----------------------------------------
		// 替换环境变量
		DWORD dwRet = ::ExpandEnvironmentStrings( pszCmdLine, pszTemp, nTempSize);
		if( (int)dwRet > nTempSize )
			throw 0;

		//-----------------------------------------
		// 找出命令行中的第一个路径字符串
		LPCTSTR pszStart = NULL;
		LPCTSTR pszEnd   = NULL;
		bRet = SearchFirstPath( pszTemp, &pszStart, &pszEnd );

		if( !bRet )
			throw 0;

		ptrdiff_t nLength = pszEnd - pszStart;
		if( nLength <= 0 || nLength > nSize - 5 )
			throw 0;

		::_tcsncpy( pszFileName, pszStart, nLength );
		pszFileName[nLength] = 0;


		//-----------------------------------------
		// 查找文件
		// <1> 首先查找该文件名, 若不存在则附加上可能的扩展名进行查找
		// <2> 如果还不存在, 则把文件截断至上一个空格, 并以<1>查找
		// <3> 直到文件存在或已经截断至'\\'.
		LPTSTR pszIter      = pszFileName + nLength;
		LPTSTR pszLastSlash = ::_tcsrchr( pszFileName, '\\' );

		if( pszLastSlash == NULL )
			throw 0;

		while( pszIter > pszLastSlash )
		{
			pszIter[0] = 0;
			if( IsFileExists( pszFileName ) )
				break;

			::_tcsncpy( pszIter, _T( ".exe" ), 5 );
			if( IsFileExists( pszFileName ) )
				break;

			::_tcsncpy( pszIter, _T( ".bat" ), 5 );
			if( IsFileExists( pszFileName ) )
				break;

			::_tcsncpy( pszIter, _T( ".com" ), 5 );
			if( IsFileExists( pszFileName ) )
				break;

			//pszIter = ::_tcsrchr( pszFileName, ' ' );

			do 
			{
				pszIter--;
			} 
			while( pszIter > pszLastSlash && !IsPossibleSeperator( *pszIter ) );
		}


		if( pszIter <= pszLastSlash )
			throw 0;


		bRet = TRUE;
	}

	catch( int )
	{
		if( pszFileName != NULL && nSize > 0 )
		{
			pszFileName[0] = 0;
		}

		bRet = FALSE;
	}


	delete [] pszTemp;
	return bRet;
}




BOOL KSearchSoftwarePathFromUninstallList::GetFirstPath( LPCTSTR pszCmdLine, LPTSTR pszFileName, int nSize )
{
	BOOL bRet = FALSE;

	LPCTSTR pszAfterRundll = HasRundll( pszCmdLine );

	if( pszAfterRundll != NULL && pszAfterRundll[0] != 0 )
	{
		bRet = _KillParamEx( pszAfterRundll, pszFileName, nSize );
	}

	if( !bRet )
	{
		bRet = _KillParamEx( pszCmdLine, pszFileName, nSize );
	}

	return bRet;
}