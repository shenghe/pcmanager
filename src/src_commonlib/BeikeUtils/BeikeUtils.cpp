#include "BeikeUtils.h"
#include <atlfile.h>
#include <common/registrywow.h>
#include "miniutil/bkprocprivilege.h"
#pragma comment(lib, "Version.lib")


DWORD TransSysCode(LPCTSTR lpszSysName)
{
	static struct{
		LPCTSTR szName;
		E_WinVersion ver;
	} keys[] = {
		{ _T("win2k"), WIN2K },	
		{ _T("winxp"), WINXP },	
		{ _T("win2k3"), WIN2003 },	
		{ _T("vista"), WINVISTA },	
		{ _T("win7"), WIN7 },
	};

	for(int i=0; i<sizeof(keys)/sizeof(keys[0]); ++i)
	{
		if( _tcsicmp(keys[i].szName, lpszSysName)==0 )
			return keys[i].ver;
	}
	return WINUNKNOWN;
}

BOOL TransHKey( LPCTSTR lpszKey, HKEY& hKey )
{
	static struct{
		LPCTSTR szName;
		HKEY hKey;
	} keys[] = {
		{ _T("HKCR"), HKEY_CLASSES_ROOT },	
		{ _T("HKCU"), HKEY_CURRENT_USER },	
		{ _T("HKLM"), HKEY_LOCAL_MACHINE },	
		{ _T("HKU"), HKEY_USERS },	
		{ _T("HKCC"), HKEY_CURRENT_CONFIG },
	};

	for(int i=0; i<sizeof(keys)/sizeof(keys[0]); ++i)
	{
		if( _tcsicmp(keys[i].szName, lpszKey)==0 )
		{
			hKey = keys[i].hKey;
			return TRUE;
		}
	}
	hKey = NULL;
	return FALSE;
}

BOOL IsFileExist(LPCTSTR pszFile)
{
	BOOL bRet = FALSE;
	if( pszFile == NULL )
		return bRet;
	if( pszFile[0] == 0 )
		return bRet;

	WIN32_FIND_DATA fd = {0};
	HANDLE hFile = FindFirstFile(pszFile, &fd);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFile);
		if( !(fd.dwFileAttributes &  FILE_ATTRIBUTE_DIRECTORY) )
			bRet = TRUE;
	}
	return bRet;
}

BOOL GetFileVersion( LPCTSTR szPath, LARGE_INTEGER &lgVersion )
{   
	if( szPath == NULL)
		return FALSE;
	
	DWORD dwHandle;
	UINT  cb;
	cb = GetFileVersionInfoSize( szPath, &dwHandle );
	if (cb > 0)
	{
		BYTE* pFileVersionBuffer = new BYTE[cb];
		if( pFileVersionBuffer == NULL )
			return FALSE;
		
		if (GetFileVersionInfo( szPath, 0, cb, pFileVersionBuffer))
		{
			VS_FIXEDFILEINFO* pVersion = NULL;
			if (VerQueryValue(pFileVersionBuffer, TEXT("\\"), (VOID**)&pVersion, &cb) && 
				pVersion != NULL) 
			{
				lgVersion.HighPart = pVersion->dwFileVersionMS;
				lgVersion.LowPart  = pVersion->dwFileVersionLS;
				delete[] pFileVersionBuffer;
				return TRUE;
			}
		}

		delete[] pFileVersionBuffer;
	}
	return FALSE;
}


LARGE_INTEGER ParseVersion( LPCTSTR szVer )
{
	DWORD d1 = 0, d2 = 0, d3 = 0, d4 = 0;
	_stscanf( szVer, _T("%u.%u.%u.%u"), &d1, &d2, &d3, &d4 );
	
	LARGE_INTEGER ll;	
	ll.HighPart = MAKELONG( d2, d1 );
	ll.LowPart = MAKELONG( d4, d3 );
	return ll;
}

void GenVersionStr( LARGE_INTEGER ll, CString &str )
{
	str.Format(_T("%u.%u.%u.%u"), HIWORD(ll.HighPart), LOWORD(ll.HighPart), HIWORD(ll.LowPart), LOWORD(ll.LowPart) );
}

BOOL VersionInRange(LARGE_INTEGER lVersion, LPCTSTR lpszLowerVersion, LPCTSTR lpszUpperVersion )
{
	BOOL bResult = FALSE;
	if( _tcslen(lpszLowerVersion)>0 
		&& !(CompareVersion(lVersion, ParseVersion(lpszLowerVersion), _T(">="), bResult ) && bResult) )
		return FALSE;

	if( _tcslen(lpszUpperVersion)>0 
		&& !(CompareVersion( lVersion, ParseVersion(lpszUpperVersion), _T("<"), bResult ) && bResult) )
		return FALSE;
	return TRUE;
}


BOOL IsVersionString( LPCTSTR szVer )
{
	unsigned int d1 = 0, d2 = 0, d3 = 0, d4 = 0;
	return 4==_stscanf( szVer, _T("%u.%u.%u.%u"), &d1, &d2, &d3, &d4 );
}

BOOL EvalCompareResult( INT nRet, LPCTSTR sOpt, BOOL &bResult )
{
	if( _tcsicmp(sOpt, _T("<"))==0 )
		bResult = nRet < 0;
	else if( _tcsicmp(sOpt, _T("<="))==0 )
		bResult = nRet <= 0;
	else if( _tcsicmp(sOpt, _T(">"))==0)
		bResult = nRet > 0;
	else if( _tcsicmp(sOpt, _T(">="))==0 )
		bResult = nRet >= 0;
	else if( _tcsicmp(sOpt, _T("="))==0 || _tcsicmp(sOpt, _T("=="))==0 )
		bResult = nRet == 0;
	else if( _tcsicmp(sOpt, _T("!="))==0 )
		bResult = nRet != 0;
	else
		return FALSE;
	
	return TRUE;
}

BOOL CompareString( LPCTSTR lpLeft, LPCTSTR lpRight, LPCTSTR sOpt, BOOL& bResult ) 
{
	INT nRet = 0;
	if(IsVersionString(lpLeft) && IsVersionString(lpRight))
	{
		LARGE_INTEGER llLeft, llRight;
		llLeft = ParseVersion(lpLeft);
		llRight = ParseVersion(lpRight);
		
		nRet = CompareLargeInts(llLeft, llRight);
	}
	else
	{
		nRet = _tcsicmp(lpLeft, lpRight);
	}
	
	return EvalCompareResult(nRet, sOpt, bResult);
}

BOOL CompareVersion( const LARGE_INTEGER &v1, const LARGE_INTEGER &v2, LPCTSTR sOpt, BOOL &bResult )
{
	INT nRet = CompareLargeInts(v1, v2);
	return EvalCompareResult(nRet, sOpt, bResult);
}

void GetDateString( CString &strDate )
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	strDate.Format(_T("%04d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);
}

BOOL ParseDateString( LPCTSTR szDate, INT &year, INT &month, INT &day )
{
	return _stscanf( szDate, _T("%u-%u-%u"), &year, &month, &day )==3;
}

BOOL MyMoveFileA(LPCSTR lpszFileFrom, LPCSTR lpszFileto)
{
	if( !PathFileExistsA(lpszFileFrom) )
		return FALSE;
	if( PathFileExistsA(lpszFileto) )
		SetFileAttributesA(lpszFileto, FILE_ATTRIBUTE_NORMAL);
	
	DWORD dwFlags = GetFileAttributesA( lpszFileFrom );
	SetFileAttributesA(lpszFileFrom, FILE_ATTRIBUTE_NORMAL);
	BOOL bRet = MoveFileExA(lpszFileFrom, lpszFileto, MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED);
	if(bRet)
		SetFileAttributesA(lpszFileto, dwFlags);
	return bRet;
}

BOOL IsFileUsing(LPCTSTR szFilename)
{
	DWORD dwFlags = GetFileAttributes( szFilename );
	SetFileAttributes(szFilename, FILE_ATTRIBUTE_NORMAL);

	HANDLE hfile = CreateFile(szFilename, GENERIC_ALL, 0, 0, OPEN_EXISTING, 0, 0); 
	BOOL bRet = TRUE;
	if(hfile!=INVALID_HANDLE_VALUE)
	{
		CloseHandle( hfile );
		bRet = FALSE;
	}	
	SetFileAttributes(szFilename, dwFlags);
	return bRet;
}

CString &CompletePathWithModulePath(CString &strPath, LPCTSTR lpszFilename)
{
	TCHAR szPath[MAX_PATH] = {0};
	::GetModuleFileName( (HMODULE)&__ImageBase, szPath, MAX_PATH);
	PathAppend(szPath, _T("..\\"));
	PathAppend(szPath, lpszFilename);
	strPath = szPath;
	return strPath;
}


template<size_t nSize>
void ModifyPathSpec( TCHAR (&szDst)[nSize], BOOL  bAddSpec )
{
	int nLen = _tcslen( szDst );
	ATLASSERT( nLen > 0 );
	TCHAR  ch  = szDst[ nLen - 1 ];
	if( ( ch == _T('\\') ) || ( ch == _T('/') ) )
	{
		if( !bAddSpec )
		{
			while( ch==_T('\\') || ch==_T('/') )
			{
				szDst[ nLen - 1 ] = _T('\0');
				-- nLen;
				ch = szDst[ nLen - 1];
			}
		}
	}
	else
	{
		if( bAddSpec )
		{
			szDst[ nLen ] = _T('\\');
			szDst[ nLen + 1 ] = _T('\0');
		}
	}
}

BOOL CreateDirectoryNested( LPCTSTR  lpszDir )
{
	if( ::PathIsDirectory( lpszDir ) ) 
		return TRUE;

	if(_tcslen(lpszDir)>=MAX_PATH)
		return FALSE;
	
	TCHAR   szPreDir[ MAX_PATH ];
	_tcscpy_s( szPreDir, lpszDir );
	//确保路径末尾没有反斜杠
	ModifyPathSpec( szPreDir, FALSE );

	//获取上级目录
	BOOL  bGetPreDir  = ::PathRemoveFileSpec( szPreDir );
	if( !bGetPreDir ) return FALSE;

	//如果上级目录不存在,则递归创建上级目录
	if( !::PathIsDirectory( szPreDir ) )
	{
		CreateDirectoryNested( szPreDir );
	}

	return ::CreateDirectory( lpszDir, NULL );
}

BOOL CreateDirEx (const char *lpFullPath, DWORD dwFileAttributes)
{
	USES_CONVERSION;
	return CreateDirectoryNested( CA2CT(lpFullPath) );
}

void CreateDirs (const char *lpFileName)
{
	char lpNameCopy[MAX_PATH] = {0};
	strcpy(lpNameCopy, lpFileName);

	char *pend = lpNameCopy + strlen(lpNameCopy);
	while( pend>lpNameCopy )
	{
		if(*pend=='/' || *pend=='\\')
		{
			*pend = 0;
			break;
		}
		--pend;
	}
	CreateDirEx (lpNameCopy);
}


BOOL file_get_contents( LPCTSTR lpszFilename, CStringA &strA )
{
	CAtlFile file;
	if( FAILED( file.Create(lpszFilename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING) ) )
		return FALSE;
	
	BOOL bRet = FALSE;
	do 
	{
		ULONGLONG filesize = 0;
		if( FAILED( file.GetSize(filesize) ) ) 
			break;

		strA = "";
		if(filesize>0)
		{
			file.Read( strA.GetBuffer((int)filesize), (DWORD)filesize );
			strA.ReleaseBuffer((int)filesize);
		}
		bRet = TRUE;
	} while (FALSE);
	file.Close();
	return bRet;
}

BOOL file_get_contents(LPCTSTR lpszFilename, CString &strBuffer)
{
	CStringA strA;
	BOOL bRet = file_get_contents(lpszFilename, strA);

	USES_CONVERSION;
	strBuffer = CA2CT( strA );		
	return bRet;
}

BOOL file_put_contents(LPCTSTR lpszFilename, BYTE *pBuffer, INT nLen)
{
	CAtlFile file;
	if( FAILED( file.Create(lpszFilename, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, CREATE_ALWAYS) ) )
		return FALSE;
	
	file.Write( pBuffer, nLen );
	file.Close();
	return TRUE;
}

BOOL file_put_contents(LPCTSTR lpszFilename, LPCTSTR lpszBuffer)
{
	CStringA strA;
	USES_CONVERSION;
	strA = CT2CA( lpszBuffer );
	return file_put_contents(lpszFilename, (BYTE*)strA.GetString(), strA.GetLength());
}

INT64 GetFolderSize(LPCTSTR szFolder)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	CString strFolder(szFolder);
	strFolder.Append( _T("\\*") );

	INT64 folderSize = 0;
	hFind = FindFirstFile(strFolder, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		_tprintf (_T("First file name is %s\n"), FindFileData.cFileName);
		do 
		{
			_tprintf (_T("Next file name is %s\n"), FindFileData.cFileName);

			if( _tcsicmp(_T("."), FindFileData.cFileName)==0 
				|| _tcsicmp(_T(".."), FindFileData.cFileName)==0 )
			{
				continue;
			}
			if( FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				CString strResult = szFolder;
				LPCTSTR pszResult;
				LPCTSTR pchLast;
				pszResult = strResult;
				pchLast = _tcsdec( pszResult, pszResult+strResult.GetLength() );
				ATLASSERT(pchLast!=NULL);
				if ((*pchLast != _T('\\')) && (*pchLast != _T('/')))
					strResult += _T('\\');
				strResult += FindFileData.cFileName;

				folderSize += GetFolderSize( strResult );
			}
			else
			{
				ULARGE_INTEGER nFileSize;
				nFileSize.LowPart = FindFileData.nFileSizeLow;
				nFileSize.HighPart = FindFileData.nFileSizeHigh;
				folderSize += nFileSize.QuadPart;
			}

		} while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	return folderSize;
}


INT64 file_get_size(LPCTSTR lpszFilename)
{
	CAtlFile file;
	if( FAILED( file.Create(lpszFilename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING) ) )
		return 0;

	ULONGLONG filesize = 0;
	file.GetSize(filesize);
	return filesize;
}




BOOL IsWindows64()
{
	BOOL bRet = FALSE;
	static HMODULE hKernel32 = NULL;
	if( !hKernel32 )
		hKernel32 = ::LoadLibrary(TEXT("Kernel32.DLL"));
	if( !hKernel32 )
		return FALSE;

	typedef BOOL (WINAPI *FunctionIsWow64Process)(HANDLE hProcess, PBOOL Wow64Process);
	FunctionIsWow64Process pfnIsWow64Process = NULL;
	pfnIsWow64Process = (FunctionIsWow64Process)GetProcAddress(hKernel32, "IsWow64Process");

	if (NULL == pfnIsWow64Process)
		return FALSE;

	HANDLE hCurrentProcess = GetCurrentProcess();
	pfnIsWow64Process(hCurrentProcess, &bRet);
	return bRet;
}

BOOL RegKeyExists( HKEY hRoot, LPCTSTR lpSubKey )
{
	BOOL bRet = FALSE;
	HKEY hReg;
	LONG nRet;

	nRet = RegOpenKeyEx( hRoot, lpSubKey, 0, KEY_READ, &hReg );
	if( nRet == ERROR_SUCCESS )
	{
		RegCloseKey( hReg );
		bRet = TRUE;
	}
	return bRet;
}

BOOL RegKeyExistsEx(HKEY hRoot, LPCTSTR lpSubKey, BOOL bNoDirect)
{
	if(bNoDirect)
	{
		CRegistryWow<REGISTRY_WOW> reg;
		return reg.ExistsKey(hRoot, lpSubKey);
	}
	else
	{
		return RegKeyExists(hRoot, lpSubKey);		
	}
}

BOOL RegValueExists( HKEY hRoot, LPCTSTR lpSubKey, LPCTSTR lpValueName )
{
	DWORD dwType;
	DWORD dwSize;
	return SHGetValue(hRoot, lpSubKey, lpValueName, &dwType, NULL, &dwSize)==ERROR_SUCCESS;
}

BOOL RegValueExistsEx( HKEY hRoot, LPCTSTR lpSubKey, LPCTSTR lpValueName, BOOL bNoDirect )
{
	if(bNoDirect)
	{
		CRegistryWow<REGISTRY_WOW> reg;
		return reg.ExistsValue(hRoot, lpSubKey, lpValueName);
	}
	else	
		return RegValueExists(hRoot, lpSubKey, lpValueName);
}

BOOL RegHasSubValue(HKEY hKey, LPCTSTR lpszSubKey)
{
	BOOL hasKey = FALSE;

	HKEY hSubKey;
	RegOpenKeyEx(hKey, lpszSubKey, 0, KEY_READ, &hSubKey);

	CString str;
	DWORD dwLen = MAX_PATH, dwLenValue = 0;
	DWORD dwType = 0;
	TCHAR szBuffer[MAX_PATH] = {0};

	if( ERROR_SUCCESS==SHEnumValue(hSubKey, 0, szBuffer, &dwLen, &dwType, NULL, &dwLenValue) )
	{
		hasKey = TRUE;
		str = szBuffer;
	}
	RegCloseKey(hSubKey);
	return hasKey;
}

BOOL RegHasSubKey(HKEY hKey, LPCTSTR lpszSubKey)
{
	BOOL hasKey = FALSE;

	HKEY hSubKey;
	RegOpenKeyEx(hKey, lpszSubKey, 0, KEY_READ, &hSubKey);

	CString str;
	DWORD dwLen = MAX_PATH;
	TCHAR szBuffer[MAX_PATH] = {0};

	if( ERROR_SUCCESS==SHEnumKeyEx(hSubKey, 0, szBuffer, &dwLen) )
	{
		str = szBuffer;
		hasKey = TRUE;
	}
	RegCloseKey(hSubKey);
	return hasKey;
}


BOOL ReadRegString(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, CString &str)
{
	TCHAR szBuffer[MAX_PATH] = {0};
	DWORD dwBuffer = MAX_PATH;
	DWORD dwType = REG_SZ;
	
	BOOL bRet = ERROR_SUCCESS==SHGetValue( hkey, pszSubKey, pszValue, &dwType, szBuffer, &dwBuffer );
	str = szBuffer;
	return bRet;
}

BOOL ReadRegStringEx(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, CString &str, BOOL bNoDirect)
{
	if(bNoDirect)
	{
		CRegistryWow<REGISTRY_WOW> reg;
		return reg.ReadString(hkey, pszSubKey, pszValue, str);
	}
	else
	{
		return ReadRegString(hkey, pszSubKey, pszValue, str);		
	}
}

BOOL ReadRegDWord(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD &dw)
{
	DWORD dwSize = sizeof(DWORD);
	DWORD dwType = REG_DWORD;
	return ERROR_SUCCESS == SHGetValue( hkey, pszSubKey, pszValue, &dwType, &dw, &dwSize );
}

/* ===================================================
*  CRegEntry::SetMulti(LPCTSTR lpszValue, size_t nLen, bool bInternal)
*
*	Stores an array of null-terminated string, terminated by two null characters.
*	For Example: First String\0Second\Third\0\0
*
*  Important Params:
*
*		LPCTSTR lpszValue:	The string consisting of the null-terminated string array
*		size_t  nLen:		The number of characters in the string, including null characters
*
*	Note: For inserting individual null-terminated strings into the array, 
*	use MultiAdd or MultiSetAt.
*/
BOOL ReadRegMString( HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, CSimpleArray<CString> &ms )
{
	DWORD dwBuffer = _MAX_REG_VALUE;
	DWORD dwType = REG_MULTI_SZ;

	TCHAR *pbuf = new TCHAR[_MAX_REG_VALUE];

	if( pbuf && ERROR_SUCCESS==SHGetValue(hkey, pszSubKey, pszValue, &dwType, pbuf, &dwBuffer) )
	{
		size_t nCur = 0, nPrev = 0, nShortLen = dwBuffer/sizeof(TCHAR);

		if( nShortLen>2 )
		{
			if (*(pbuf + nShortLen-1) == '\0')
				nShortLen--;	

			while( (nCur = (int)(_tcschr(pbuf+nPrev, '\0')-pbuf)) < nShortLen ) 
			{
				ms.Add( pbuf+nPrev );
				nPrev = nCur+1;
			}
		}
		return TRUE;
	}
	return FALSE;	
}


BOOL WriteRegString( HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPCTSTR strValue )
{
	size_t	nValueLen = (_tcslen(strValue) + 1) * sizeof(TCHAR);
	return ERROR_SUCCESS==SHSetValue(hkey, pszSubKey, pszValue, REG_SZ, (LPBYTE)strValue, nValueLen);
}

BOOL WriteRegExString( HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPCTSTR strValue )
{
	size_t	nValueLen = (_tcslen(strValue) + 1) * sizeof(TCHAR);
	return ERROR_SUCCESS==SHSetValue(hkey, pszSubKey, pszValue, REG_EXPAND_SZ, (LPBYTE)strValue, nValueLen);
}

BOOL WriteRegDWord( HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD dw )
{
	return ERROR_SUCCESS==SHSetValue(hkey, pszSubKey, pszValue, REG_DWORD, (LPBYTE)&dw, sizeof(dw));
}

BOOL WriteRegMString( HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, const CSimpleArray<CString> &ms )
{
	INT nLen = 0;
	for(int i=0; i<ms.GetSize(); ++i)
	{
		nLen += ms[i].GetLength() + 1;
	}
	nLen += (nLen==0 ? 2 : 1); 

	TCHAR *pbuf = new TCHAR[nLen];
	if( !pbuf )
		return FALSE;

	ZeroMemory(pbuf, sizeof(TCHAR)*nLen );

	TCHAR *p = pbuf;
	for(int i=0; i<ms.GetSize(); ++i)
	{
		const CString &str = ms[i];
		_tcscpy(p, str);

		p += str.GetLength() + 1;
	}

	BOOL bRet = ERROR_SUCCESS==SHSetValue(hkey, pszSubKey, pszValue, REG_MULTI_SZ, pbuf, nLen * sizeof(TCHAR));
	delete pbuf;

	return bRet;
}


// MAX INT is 42,9,4967295 
INT ParseKBString(LPCTSTR sz)
{
	wchar_t *szEnd = NULL;
	INT nPrefix = 0;
	INT n = _tcstol(sz, &szEnd, 10);
#if 0
	return n;
#else
	if( szEnd && _tcslen(szEnd)>0 )
	{
		if( _tcsnicmp(szEnd, _T("-v"), 2)==0 )
		{
			szEnd += 2;
			nPrefix = _ttoi(szEnd);
		}
	}
	return nPrefix*KB_VER_BASE + n;
#endif
}

void FormatKBString(INT nKB, CString &str)
{
	if(nKB<KB_VER_BASE)
		str.Format(_T("KB%d"), nKB);
	else
	{
		INT nV = nKB/KB_VER_BASE;
		INT n = nKB-nV*KB_VER_BASE;
		str.Format(_T("KB%d-v%d"), n, nV);
	}	
}

void FormatKBWebUrl( CString &strUrl, INT nKB )
{
	nKB = GetRealKBID(nKB);
	strUrl.Format(_T("http://support.microsoft.com/kb/%d"), nKB);
}

INT GetRealKBID(INT nKB)
{
	if(nKB>KB_VER_BASE)
	{
		INT nV = nKB/KB_VER_BASE;
		nKB = nKB-nV*KB_VER_BASE;
	}
	return nKB;
}


BOOL IsNumber( LPTSTR lpszName )
{
	if ( NULL == lpszName || *lpszName==0 )
		return FALSE;

	BOOL bRet = TRUE;
	BOOL gotDot = FALSE;
	while( *lpszName != 0 )
	{
		if( !_istdigit( *lpszName ) )
		{
			if( *lpszName==_T('.') && !gotDot )
				gotDot = TRUE;
			else
			{
				bRet = FALSE;
				break;
			}
		}
		++lpszName;
	}
	return bRet;
}

BOOL Split( LPCTSTR lpString, CSimpleArray<CString>& arr, TCHAR delimiter )
{
	if(lpString==NULL || _tcslen(lpString)==0)
		return FALSE;

	arr.RemoveAll();

	CString szString = lpString;
	int nStart = 0;
	int nOffset;
	while(1)
	{
		nOffset = szString.Find(delimiter, nStart);

		if(nOffset > 0)
		{
			arr.Add(szString.Mid(nStart, nOffset-nStart));
		}
		else if(nOffset==-1)
		{
			arr.Add(szString.Mid(nStart));
			break;
		}
		nStart = nOffset + 1;
	};
	return TRUE;
}

BOOL ExecuteFile( LPCTSTR lpszFilename, LPCTSTR lpszInstallParam, DWORD &dwExitCode, BOOL &bHijacked )
{
	if (!lpszFilename || !lpszInstallParam || !PathFileExists(lpszFilename))
		return FALSE;

	CString strCmdline;
	LPCTSTR pExt = _tcsrchr(lpszFilename, _T('.'));
	if( pExt && _tcsicmp( pExt, _T(".msu") ) == 0 )
	{
		TCHAR szWusaPath[MAX_PATH];
		GetSystemWindowsDirectory(szWusaPath, MAX_PATH);
		_tcscat(szWusaPath, _T("\\system32\\wusa.exe"));
		strCmdline.Format( _T("%s \"%s\" %s"), szWusaPath, lpszFilename, lpszInstallParam );
	}
	else
		strCmdline.Format( _T("\"%s\" %s"), lpszFilename, lpszInstallParam );
	
	TCHAR szCmdline[MAX_PATH] = {0};
	_tcscpy(szCmdline, strCmdline);

    STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi={0};
	
    ZeroMemory(&si, sizeof( si ));
	ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(STARTUPINFO);
	BOOL processCreated = CreateProcess(NULL, szCmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if(!processCreated)
		return FALSE;
	WaitForSingleObject(pi.hProcess, INFINITE);
	
	bHijacked = FALSE;
	// 检查是否被360 拦截
	union MFILETIME
	{
		FILETIME ft;
		__int64 i64;
	};
	MFILETIME tmCreate, tmExit, tmKernel, tmUser;
	GetProcessTimes(pi.hProcess, &tmCreate.ft, &tmExit.ft, &tmKernel.ft, &tmUser.ft);
	if( tmKernel.i64==0 || tmUser.i64==0 )
		bHijacked = TRUE;
	
	dwExitCode = 0;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);	
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
#ifdef _DEBUG
	_tcprintf(_T("%s %s -> %d(%s)\n"), lpszFilename, lpszInstallParam, dwExitCode, bHijacked ? _T("Hijacked"):_T("OK") );
#endif
	return TRUE;
}

const char * SkipXmlHeader( const char * szXml )
{
	if(!szXml)
		return szXml;
	
	const char *xml_begin = "<?xml";
	const char *xml_end = "?>";
	const char *p1 = strchr(szXml, '<');
	if(p1 && strnicmp(p1, xml_begin, strlen(xml_begin))==0)
	{
		const char *p2 = strchr(szXml+strlen(xml_begin), '>');

		if( p2 && strnicmp(p2-1, xml_end, strlen(xml_end))==0)
		{
			const char *pb = p2 + 2;
			while( *pb=='\r' || *pb=='\n' || *pb==' ')
				++pb;
			return pb;
		}
	}
	return szXml;	
}

const char *SkipXmlHeaderForWin2K(const char *szXml)
{
	static int nWinVer = -1;
	if(nWinVer==-1)
		nWinVer = GetWinVer();
	
	if(nWinVer==WIN2K)
		return SkipXmlHeader(szXml);
	else
		return szXml;
}


INT GetWinVer( )
{
	T_WindowsInfo winInfo;
	winInfo.Init();
	return winInfo.nWindowsVersion;
}

BOOL IsCurrentUserAdmin()  
{    
	HANDLE hAccessToken;    
	BYTE * InfoBuffer = new BYTE[1024];    
	PTOKEN_GROUPS ptgGroups;    
	DWORD dwInfoBufferSize;    
	PSID psidAdministrators;    
	SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;    

	if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hAccessToken))    
	{    
		delete InfoBuffer;    
		return FALSE;    
	}    

	if(!GetTokenInformation(hAccessToken,TokenGroups,InfoBuffer,1024,&dwInfoBufferSize))    
	{    
		delete InfoBuffer;    
		CloseHandle(hAccessToken);    
		return FALSE;    
	}    

	CloseHandle(hAccessToken);    

	if(!AllocateAndInitializeSid(&siaNtAuthority,    
		2,    
		SECURITY_BUILTIN_DOMAIN_RID,    
		DOMAIN_ALIAS_RID_ADMINS,    
		0,0,0,0,0,0,    
		&psidAdministrators))    
	{    
		delete InfoBuffer;    
		return FALSE;    
	}    

	ptgGroups = (PTOKEN_GROUPS)InfoBuffer;    

	for(UINT i = 0; i < ptgGroups->GroupCount; i++)    
	{    
		if(EqualSid(psidAdministrators,ptgGroups->Groups[i].Sid))    
		{    
			FreeSid(psidAdministrators);    
			delete InfoBuffer;    
			return TRUE;    
		}    
	}    
	return FALSE;    
}  

BOOL GetWinVerInfo(INT &nWindowsVersion, INT &nSP )
{
	T_WindowsInfo winInfo;
	if( !winInfo.Init() )
		return FALSE;
	nWindowsVersion = winInfo.nWindowsVersion;
	nSP = winInfo.nSP;
	return TRUE;
}

BOOL T_WindowsInfo::Init()
{
	nWindowsVersion = WINUNKNOWN;
	nSP = 0;

	ZeroMemory(&osvi, sizeof(osvi));
	if(GetWindowsVersionInfo(osvi))
	{
		FixVersionInfo(osvi);
		nWindowsVersion = _ParseWinVer( osvi );
		nSP = osvi.wServicePackMajor;
		return TRUE;
	}
	return FALSE;
}

BOOL T_WindowsInfo::GetWindowsVersionInfo( OSVERSIONINFOEX &osvi )
{
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( GetVersionEx((OSVERSIONINFO *) &osvi) )
		return TRUE;
	osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	return GetVersionEx ( (OSVERSIONINFO *) &osvi);
}

BOOL T_WindowsInfo::FixVersionInfo( OSVERSIONINFOEX &osvi )
{
	CString strKernelFile = _T("@SYSTEM");
	if( ParseSpecialFolder( strKernelFile ) )
	{
		strKernelFile += _T("\\ntoskrnl.exe");
		LARGE_INTEGER ll;
		if( PathFileExists(strKernelFile) && GetFileVersion(strKernelFile, ll) )
		{
			DWORD dwMajor = HIWORD(ll.HighPart);
			DWORD dwMinor = LOWORD(ll.HighPart);

			if( osvi.dwMajorVersion<dwMajor
				|| ( osvi.dwMajorVersion==dwMajor && osvi.dwMinorVersion<dwMinor) )
			{
				osvi.dwPlatformId = VER_PLATFORM_WIN32_NT;
				osvi.dwMajorVersion = dwMajor;
				osvi.dwMinorVersion = dwMinor;

				// OR use ntoskrnl.exe
				// MODIFY SP  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Windows@CSDVersion 
				DWORD dwCSDVersion = 0;
				ReadRegDWord(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Windows"), _T("CSDVersion"), dwCSDVersion);
				osvi.wServicePackMajor = (WORD)(dwCSDVersion >> 8);
				return TRUE;
			}
		}
	}
	return FALSE;
}

int T_WindowsInfo::_ParseWinVer( OSVERSIONINFOEX &osvi )
{
	/*
	5.0 	win2k 
	5.1 	winxp 
	5.2 	win2003 
	6.0		vista 
	6.1 	win7 
	*/

	int wVersion = WINUNKNOWN;
	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32s:
		wVersion = WIN32S;
		break;

	case VER_PLATFORM_WIN32_WINDOWS:
		if ( osvi.dwMinorVersion == 0 )
			wVersion = WIN95;	
		else if ( osvi.dwMinorVersion == 10)
			wVersion = WIN98;
		else if ( osvi.dwMinorVersion == 90)
			wVersion = WINME;
		break;

	case VER_PLATFORM_WIN32_NT:
		if ( osvi.dwMajorVersion <= 4 )
			wVersion = WINNT;
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
			wVersion = WIN2K;
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
			wVersion = WINXP;
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
			wVersion = WIN2003;
		else if ( osvi.dwMajorVersion == 6 )
		{
			if(osvi.dwMinorVersion==0)
			{
				wVersion = osvi.wProductType == VER_NT_WORKSTATION ? WINVISTA : WIN2008;
			}
			else if(osvi.dwMinorVersion==1)
			{
				wVersion = osvi.wProductType == VER_NT_WORKSTATION ? WIN7 : WIN2008R2;
			}
		}
		break;

	default:
		break;
	}
	return wVersion;
}


LANGID GetLangID()
{
	static LANGID dwLang = 0;
	if(dwLang==0)
	{
		// GetSystemDefaultLangID();
		dwLang = GetSystemDefaultUILanguage();
		if(dwLang==2057)
			dwLang = 1033;
	}
	return dwLang;
}

LCID SetLocale2GBK(LCID id)
{
	LCID old = ::GetThreadLocale();
	if(id==0)
		::SetThreadLocale( 0x0804 ); 
	return old;
}


char CWowFsRedirectDisableHelper::bIs64 = -1;
BOOL CWowFsRedirectDisableHelper::bInited = FALSE;
CWowFsRedirectDisableHelper::Wow64DisableWow64FsRedirection CWowFsRedirectDisableHelper::f_Wow64DisableWow64FsRedirection = NULL;
CWowFsRedirectDisableHelper::Wow64RevertWow64FsRedirection CWowFsRedirectDisableHelper::f_Wow64RevertWow64FsRedirection = NULL;  

CWowFsRedirectDisableHelper::CWowFsRedirectDisableHelper(BOOL bSet) : m_bSet(bSet), m_pOldValue(NULL)
{
	if(bIs64==-1)
	{
		bIs64 = IsWindows64() ? 1 : 0; 
	}
	
	if(bSet && bIs64 && Init() && f_Wow64DisableWow64FsRedirection)
	{
		f_Wow64DisableWow64FsRedirection(&m_pOldValue);
		_tprintf(_T(" -WOWFS Disabled \n"));
	}
}

CWowFsRedirectDisableHelper::~CWowFsRedirectDisableHelper()
{
	if(m_bSet && bIs64 && f_Wow64RevertWow64FsRedirection)
	{
		f_Wow64RevertWow64FsRedirection(m_pOldValue);
		_tprintf(_T(" -WOWFS Reverted \n"));
	}
}

BOOL CWowFsRedirectDisableHelper::Init()
{
	if(!bInited)
	{
		bInited = TRUE;

		HINSTANCE hlibrary = LoadLibrary(_T("Kernel32.dll"));
		if(hlibrary)
		{
			f_Wow64DisableWow64FsRedirection = (Wow64DisableWow64FsRedirection) GetProcAddress(hlibrary,"Wow64DisableWow64FsRedirection");
			f_Wow64RevertWow64FsRedirection = (Wow64RevertWow64FsRedirection) GetProcAddress(hlibrary,"Wow64RevertWow64FsRedirection");
		}
	}
	return bInited;
}

void ShutDownComputer(BOOL toReboot)
{
	BOOL go = TRUE;
#ifdef _DEBUG
	go = IDYES == ::MessageBox(NULL, toReboot ? _T("需要重新启动"):_T("需要关机"), _T("ShutDownComputer"), MB_YESNO);
#endif
	if( go )
	{
		UINT flags = toReboot ? EWX_REBOOT : EWX_SHUTDOWN;
		flags |= EWX_FORCE;

		CBkProcPrivilege privilege;
		if (!privilege.EnableShutdown())
			return;
		::ExitWindowsEx(flags, 0);
	}
}

BOOL IsWindowInSafeMode()
{
	LPCTSTR szPath = _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Option");
	LPCTSTR szKeyname = _T("OptionValue");
	CRegistryWow<REGISTRY_WOW> reg;
	DWORD dwValue = 0;
	if(reg.ReadDWord(HKEY_LOCAL_MACHINE, szPath, szKeyname, dwValue))
	{
		return dwValue != 0 ;
	}
	return FALSE;
}