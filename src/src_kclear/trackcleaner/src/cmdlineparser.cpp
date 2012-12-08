// CmdLineParser.cpp: implementation of the CCmdLineParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CmdLineParser.h"
#include <assert.h>

#pragma warning( disable:4996)

//
//////////////////////////////////////////////////////////////////////////
static inline bool is_path_separator_char(TCHAR c)
{
	return (c == _T('/')) || (c == _T('\\'));
}

static inline bool is_absolute_path(LPCTSTR lpPath)
{
	return (lpPath[1] == _T(':') ||
		_tcsncmp(lpPath, _T("\\\\"), 2) == 0 ||
		_tcsnicmp(lpPath, _T("file://"), 7) == 0);
}

static inline bool is_relative_path(LPCTSTR lpPath) // 调用这个函数的 lpPath 必须fix过,没有多余的 '\'
{
	bool bResult = TRUE;
	TCHAR c;

	while ( (c = *lpPath) != 0 )
	{
		if ( c == _T('\\') )
		{
			if ( bResult )
			{
				break;
			}

			bResult = TRUE;
		}
		else if ( c != _T('.') )
		{
			bResult = FALSE;
		}

		lpPath++;
	}

	return bResult;
}	

static inline bool is_blank_char(TCHAR c)
{
	return (c == _T(' ')) || (c == _T('\t'));
}

static inline BOOL is_empty_str(LPCTSTR str)
{
	return (str == NULL) || (*str == _T('\0'));
}

static inline LPCTSTR skip_blank(LPCTSTR lpStr)
{
	TCHAR c;
	while ( (c = *lpStr) != 0 )
	{
		if ( !is_blank_char(c) )
		{
			return lpStr;
		}

		lpStr++;
	}

	return lpStr;
}

static inline LPCTSTR skip_no_blank(LPCTSTR lpStr)
{
	TCHAR c;
	while ( (c = *lpStr) != 0 )
	{
		if ( is_blank_char(c) )
		{
			return lpStr;
		}
		
		lpStr++;
	}
	
	return lpStr;
}

static inline LPCTSTR skip_quotation(LPCTSTR lpStr)
{
	TCHAR c;
	while ( (c = *lpStr) != 0 )
	{
		if ( c != _T('\"') )
		{
			return lpStr;
		}
		
		lpStr++;
	}
	
	return lpStr;
}

static size_t cmd_param_len(LPCTSTR lpParam)
{
	int nBacket = 0;
	TCHAR c;

	LPCTSTR lpStr = lpParam;
	while ( (c = *lpStr) != 0 )
	{
		if ( is_blank_char(c) && (nBacket == 0 || nBacket == 2) )
		{
			break;
		}

		lpStr++;
		if ( c == _T('\"') )
		{
			nBacket++;
		}
		else if ( c == _T('\\') )
		{
			// 转义
			if ( lpStr[1] == 0 )
			{
				break;
			}

			lpStr++;
		}
	}

	return (lpStr - lpParam);
}

static BOOL fix_path_separator(LPTSTR lpDst, size_t nDstLen, LPCTSTR lpSrc, size_t nSrcLen = -1)
{
	bool seqarator = false;
	LPCTSTR lpDst_End = lpDst + nDstLen;

	if ( nDstLen < 3 )
	{
		return false;
	}

	if ( nSrcLen == -1 )
	{
		nSrcLen = _tcslen(lpSrc);
	}

	if ( nSrcLen >= 2 && _tcsncmp(lpSrc, _T("\\\\"), 2) == 0 )
	{
		*lpDst++ = _T('\\');
		*lpDst++ = _T('\\');
		*lpDst = 0;

		lpSrc += 2;
		nSrcLen -= 2;
	}
	else if ( nSrcLen >= 7 && _tcsnicmp(lpSrc, _T("file://"), 7) == 0 )
	{
		if ( lpSrc[8] != _T(':') )
		{
			*lpDst++ = _T('\\');
			*lpDst++ = _T('\\');
			*lpDst = 0;
		}

		lpSrc += 7;
		nSrcLen -= 7;
	}

	while ( nSrcLen-- != 0 )
	{
		if ( lpDst >= lpDst_End )
		{
			return FALSE;
		}

		TCHAR c = *lpSrc;
		if ( is_path_separator_char(c) )
		{
			if ( !seqarator )
			{
				seqarator = true;
				*lpDst++ = _T('\\');
			}
		}
		else
		{
			*lpDst++ = c;
			seqarator = false;
		}
	
		lpSrc++;
	}

	*lpDst = 0;
	return true;
}

static inline LPCTSTR re_strchr(LPCTSTR lpStrBegin, LPCTSTR lpStr, TCHAR c)
{
	assert(lpStr >= lpStrBegin);

	while ( lpStr != lpStrBegin )
	{
		if ( *lpStr == c )
		{
			return lpStr;
		}

		lpStr--;
	}

	return NULL;
}

static inline BOOL is_vaild_file_name(LPCTSTR lpFileName)
{
	TCHAR c;
	while ( (c = *lpFileName) != 0 )
	{
		if ( _tcschr(_T("\\/:*?\"<>|"), c) != NULL )
		{
			return FALSE;
		}

		lpFileName++;
	}

	return TRUE;
}

static BOOL append_file_name(LPTSTR lpPath, LPCTSTR lpFileName, size_t nFileNameLen = -1)
{
	if( lpPath[0] == _T('\0') )
	{
		return FALSE;
	}

	size_t nPathLen = _tcslen(lpPath);
	LPTSTR lpDstFileName = lpPath + nPathLen;
	
	if ( lpDstFileName[-1] != _T('\\') )
	{
		*lpDstFileName++ = _T('\\');
		nPathLen++;
	}

	while ( is_path_separator_char(*lpFileName) )
	{
		lpFileName++;
	}

	return fix_path_separator(lpDstFileName, MAX_PATH - nPathLen - 1, lpFileName, nFileNameLen);
}

static LPCTSTR get_file_name(LPCTSTR lpFilePath)
{
	LPCTSTR lpFileName = _tcsrchr(lpFilePath, _T('\\'));
	if ( lpFileName == NULL )
	{
		lpFileName = _tcsrchr(lpFilePath, _T('/'));
		if ( lpFileName == NULL )
		{
			lpFileName = lpFilePath;
		}
		else
		{
			lpFileName++;
		}
	}
	else
	{
		lpFileName++;
	}

	return lpFileName;
}

static inline int get_path_type(LPCTSTR lpPath)
{
	DWORD dwErrMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	
	DWORD dwAttr = GetFileAttributes(lpPath);
	
	SetErrorMode(dwErrMode);
	
	if ( dwAttr == INVALID_FILE_ATTRIBUTES )
		return 0;
	
	return (dwAttr & FILE_ATTRIBUTE_DIRECTORY) ? 2 : 1;
}

LPCTSTR GetDefaultPathVar()
{
	static CString strPathVar;
	static BOOL bInitPathVar = FALSE;

	if ( !bInitPathVar )
	{
		DWORD dwPathVars = GetEnvironmentVariable(_T("path"), NULL, 0);
		if ( dwPathVars != 0 )
		{
			LPTSTR lpDefEnvs = strPathVar.GetBuffer(dwPathVars + 1);
			GetEnvironmentVariable(_T("path"), lpDefEnvs, dwPathVars);
			strPathVar.ReleaseBuffer(dwPathVars);
		}
	}

	return strPathVar;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCmdLineParser::CCmdLineParser(DWORD dwFlag)
{
	m_dwFlag = dwFlag;
	m_lpParam = NULL;
}

CCmdLineParser::CCmdLineParser(LPCTSTR lpCmdLine /*dwFlag = DEF_CUR_DIR | DEF_ENV_VAR | DEF_EXT_NAME | NEED_PARAM*/)
{
	m_dwFlag = DEF_CUR_DIR | DEF_ENV_VAR | DEF_EXT_NAME | NEED_PARAM;
	m_lpParam = NULL;

	DoParser(lpCmdLine);
}

CCmdLineParser::~CCmdLineParser()
{
	delete m_lpParam;
}

BOOL CCmdLineParser::ExpandExtNames(LPTSTR lpFilePath, LPCTSTR lpExtNames)
{
	BOOL bResult = FALSE;
	
	if ( get_path_type(lpFilePath) != 1 )
	{
		LPTSTR lpExtName = _tcsrchr(lpFilePath, _T('.'));
		if ( lpExtName == NULL )
		{
			lpExtName = lpFilePath + _tcslen(lpFilePath);
			*lpExtName++ = _T('.');
			*lpExtName = _T('\0');
			
			LPCTSTR lpFixExtName = lpExtNames;
			while ( !is_empty_str(lpFixExtName) )
			{
				size_t nFixExtNameLen = 0;
				
				LPTSTR lpNextFixExtName = (LPTSTR)_tcschr(lpFixExtName, _T(';'));
				if ( lpNextFixExtName != NULL )
				{
					nFixExtNameLen = lpNextFixExtName - lpFixExtName;
					lpNextFixExtName++;
				}
				else
				{
					nFixExtNameLen = _tcslen(lpFixExtName);
				}
				
				if ( lpExtName + nFixExtNameLen < lpFilePath + MAX_PATH )
				{
					_tcsncpy(lpExtName, lpFixExtName, nFixExtNameLen);
					lpExtName[nFixExtNameLen] = 0;

					if ( get_path_type(lpFilePath) == 1 )
					{
						bResult = TRUE;
						break;
					}
				}

				lpFixExtName = lpNextFixExtName;
			}
			
			if ( !bResult )
			{
				lpExtName[-1] = _T('\0');
			}
		}
	}
	else
	{
		bResult = TRUE;
	}

	return bResult;
}

BOOL CCmdLineParser::ExpandEnvVars(LPTSTR lpFilePath, LPCTSTR lpEnvVars, LPCTSTR lpExtNames)
{
	TCHAR szTempBuff[MAX_PATH + 10];

	GetSystemDirectory(szTempBuff, MAX_PATH);
	append_file_name(szTempBuff, lpFilePath);
	if ( ExpandExtNames(szTempBuff, lpExtNames) )
	{
		GetLongPathName(szTempBuff, lpFilePath, MAX_PATH);
		return TRUE;
	}

	GetWindowsDirectory(szTempBuff, MAX_PATH);
	append_file_name(szTempBuff, lpFilePath);
	if ( ExpandExtNames(szTempBuff, lpExtNames) )
	{
		GetLongPathName(szTempBuff, lpFilePath, MAX_PATH);
		return TRUE;
	}

	if ( (m_dwFlag & DEF_ENV_VAR) && lpEnvVars == NULL )
	{
		lpEnvVars = GetDefaultPathVar();
	}

	if ( lpEnvVars != NULL )
	{
		//
		// 展开环境变量
		//
		LPCTSTR lpVar = lpEnvVars;
		while ( !is_empty_str(lpVar)  )
		{
			size_t nVarLen;
			TCHAR c;

			while ( (c = *lpVar) != 0 )
			{
				if ( c != _T(';') && c != _T(' ') && c != _T('\t') )
				{
					break;
				}

				lpVar++;
			}

			if ( c == 0 )
			{
				break;
			}

			LPCTSTR lpNextVar = _tcschr(lpVar, _T(';'));
			if ( lpNextVar != NULL )
			{
				nVarLen = lpNextVar - lpVar;
				lpNextVar++;
			}
			else
			{
				nVarLen = _tcslen(lpVar);
			}

			if ( fix_path_separator(szTempBuff, MAX_PATH, lpVar, nVarLen) &&
				append_file_name(szTempBuff, lpFilePath) &&
				ExpandExtNames(szTempBuff, lpExtNames) )
			{
				GetLongPathName(szTempBuff, lpFilePath, MAX_PATH);
				return TRUE;
			}

			lpVar = lpNextVar;
		}
	}

	return FALSE;
}

void CCmdLineParser::ExpandParam(LPCTSTR lpCmdEnd)
{
	if ( m_dwFlag & NEED_PARAM )
	{
		LPCTSTR lpParamBegin;

		lpParamBegin = lpCmdEnd;
		lpParamBegin = skip_blank(lpParamBegin);

		if ( !is_empty_str(lpParamBegin) )
		{
			size_t nParamLen = _tcslen(lpParamBegin);

			m_lpParam = new TCHAR[nParamLen + 2];
			_tcscpy(m_lpParam, lpParamBegin);
		}
	}
}

LPTSTR CCmdLineParser::ExpandMacro(LPCTSTR lpCmdBegin)
{
	size_t nMacroLen, nExpandMacroLen;
	LPTSTR lpCmdBuff = NULL;

	if ( _tcsnicmp(lpCmdBegin, _T("%SystemRoot%"), nMacroLen = sizeof ("%SystemRoot%") - 1) == 0 ||
		_tcsnicmp(lpCmdBegin, _T("%windir%"), nMacroLen = sizeof ("%windir%") - 1) == 0
		)
	{
		nExpandMacroLen = GetWindowsDirectory(m_szCmd, MAX_PATH);
	}
	else if ( _tcsnicmp(lpCmdBegin, _T("%ProgramFiles%"), nMacroLen = sizeof ("%ProgramFiles%") - 1) == 0 )
	{
		if ( SHGetSpecialFolderPath(NULL, m_szCmd, CSIDL_PROGRAM_FILES, FALSE) )
		{
			nExpandMacroLen = _tcslen(m_szCmd);
		}
	}
	else if ( _tcsnicmp(lpCmdBegin, _T("%CommonProgramFiles%"), nMacroLen = sizeof ("%CommonProgramFiles%") - 1) == 0 )
	{
		if ( SHGetSpecialFolderPath(NULL, m_szCmd, CSIDL_PROGRAM_FILES_COMMON, FALSE) )
		{
			nExpandMacroLen = _tcslen(m_szCmd);
		}
	}
	else if ( _tcsnicmp(lpCmdBegin, _T("%Temp%"), nMacroLen = sizeof ("%Temp%") - 1) == 0 ||
		_tcsnicmp(lpCmdBegin, _T("%Tmp%"), nMacroLen = sizeof ("%Tmp%") - 1) == 0
		)
	{
		nExpandMacroLen = GetTempPath(MAX_PATH, m_szCmd);
	}
	else 
	{
		// 不支持的宏路径
		goto _Failed_Exit;
	}

	if ( nExpandMacroLen != 0 )
	{
		size_t dwBuffLen = nExpandMacroLen + _tcslen(lpCmdBegin + nMacroLen) + 2;

		lpCmdBuff = new TCHAR[dwBuffLen];
		if ( lpCmdBuff != NULL )
		{
			_tcscpy(lpCmdBuff, m_szCmd);
			if ( lpCmdBuff[nExpandMacroLen - 1] == _T('\\') )
			{
				nExpandMacroLen--;
			}

			_tcscpy(lpCmdBuff + nExpandMacroLen, lpCmdBegin + nMacroLen);
		}
	}

_Failed_Exit:
	return lpCmdBuff;
}

LPTSTR CCmdLineParser::ExpandMacro2(LPCTSTR lpCmdBegin)
{
	size_t nMacroLen, nExpandMacroLen;
	LPTSTR lpCmdBuff = NULL;

	if ( _tcsnicmp(lpCmdBegin, _T("SystemRoot"), nMacroLen = sizeof ("SystemRoot") - 1) == 0 )
	{
		nExpandMacroLen = GetWindowsDirectory(m_szCmd, MAX_PATH);
	}
	else if ( _tcsnicmp(lpCmdBegin, _T("system32"), nMacroLen = sizeof ("system32") - 1) == 0 )
	{
		nExpandMacroLen = GetSystemDirectory(m_szCmd, MAX_PATH);
	}
	else
	{
		goto _Failed_Exit;
	}

	if ( nExpandMacroLen != 0 )
	{
		size_t dwBuffLen = nExpandMacroLen + _tcslen(lpCmdBegin + nMacroLen) + 2;

		lpCmdBuff = new TCHAR[dwBuffLen];
		if ( lpCmdBuff != NULL )
		{
			_tcscpy(lpCmdBuff, m_szCmd);
			if ( lpCmdBuff[nExpandMacroLen - 1] == _T('\\') )
			{
				nExpandMacroLen--;
			}

			_tcscpy(lpCmdBuff + nExpandMacroLen, lpCmdBegin + nMacroLen);
		}
	}

_Failed_Exit:
	return lpCmdBuff;
}


BOOL CCmdLineParser::ExpandAbsolute(LPCTSTR lpCmdBegin, LPCTSTR lpExtNames, BOOL bQuotation)
{
	BOOL bResult = FALSE;

	if ( is_absolute_path(lpCmdBegin) )
	{
		//
		// 绝对路径
		// 支持不加引号的命令行如 C:\Program Files\ASUS\Power4 Gear\BatteryLife.exe 1
		//
		TCHAR szCmdBuff[MAX_PATH + 10];
		LPCTSTR lpCmdEnd = NULL, lpCmdEnd_Last = NULL;

		m_szCmd[0] = 0;

		if ( bQuotation )
		{
			lpCmdEnd = _tcschr(lpCmdBegin, _T('\"'));
			if ( fix_path_separator(szCmdBuff, MAX_PATH, lpCmdBegin, lpCmdEnd - lpCmdBegin) &&
				ExpandExtNames(szCmdBuff, lpExtNames)
				)
			{
				GetLongPathName(szCmdBuff, m_szCmd, MAX_PATH);

				lpCmdEnd = skip_quotation(lpCmdEnd);
				lpCmdEnd = skip_blank(lpCmdEnd);
			}
		}
		else
		{
			lpCmdEnd = skip_no_blank(lpCmdBegin);
			lpCmdEnd_Last = lpCmdEnd;

			while ( TRUE )
			{
				if ( !fix_path_separator(szCmdBuff, MAX_PATH, lpCmdBegin, lpCmdEnd - lpCmdBegin) )
				{
					break;
				}

				if ( ExpandExtNames(szCmdBuff, lpExtNames) )
				{
					lpCmdEnd_Last = lpCmdEnd;
					GetLongPathName(szCmdBuff, m_szCmd, MAX_PATH);
				}

				lpCmdEnd = skip_blank(lpCmdEnd);
				if ( *lpCmdEnd == 0 ) 
				{
					break;
				}

				lpCmdEnd = skip_no_blank(lpCmdEnd);
			}

			lpCmdEnd = lpCmdEnd_Last;
		}

		if ( m_szCmd[0] != 0 )
		{
			ExpandParam(lpCmdEnd);
			bResult = TRUE;
		}
	}

	return bResult;
}

LPTSTR CCmdLineParser::ExpandRelative(LPCTSTR lpCmdBegin, LPCTSTR lpCurDir, BOOL bQuotation)
{
	LPTSTR	lpFileNameOffset, lpFileNameOffsetEnd;
	LPTSTR	lpDirOffset;
	BOOL	bJmpDir;
	TCHAR	szTempBuff[MAX_PATH + 10];
	LPCTSTR lpCmdEnd = NULL, lpCmdEnd_Last = NULL;
	LPTSTR	lpCmdBuff = NULL;

	if ( lpCurDir == NULL )
	{
		GetCurrentDirectory(MAX_PATH, m_szCmd);
	}
	else
	{
		fix_path_separator(m_szCmd, MAX_PATH, lpCurDir);
	}

	lpFileNameOffset = m_szCmd + _tcslen(m_szCmd);
	lpFileNameOffsetEnd = m_szCmd + MAX_PATH;
	if ( lpFileNameOffset[-1] == _T('\\') )
	{
		*--lpFileNameOffset = 0;
	}

	if ( bQuotation )
	{
		lpCmdEnd = _tcschr(lpCmdBegin, _T('\"'));
		if ( lpCmdEnd == NULL )
		{
			goto _Failed_Exit;
		}
	}
	else
	{
		lpCmdEnd = skip_no_blank(lpCmdBegin);
	}

	if ( !fix_path_separator(szTempBuff, MAX_PATH, lpCmdBegin, lpCmdEnd - lpCmdBegin) )
	{
		goto _Failed_Exit;
	}

	// ../../a.exe
	// ./a.exe
	// ./../a.exe
	// a/../a.exe
	// ../a/2.exe
	bJmpDir = TRUE;
	lpDirOffset = szTempBuff;
	
	while ( *lpDirOffset != 0 )
	{
		if ( lpFileNameOffset >= lpFileNameOffsetEnd )
		{
			// 缓冲区不够
			goto _Failed_Exit;
		}

		if ( lpDirOffset[0] == _T('.') )
		{
			if ( lpDirOffset[1] == _T('.') && lpDirOffset[2] == _T('\\') )
			{
				lpFileNameOffset = (LPTSTR)re_strchr(m_szCmd, lpFileNameOffset, _T('\\'));
				if ( lpFileNameOffset == NULL )
				{
					// Bad Command Line
					goto _Failed_Exit;
				}

				*lpFileNameOffset = 0;
				lpDirOffset += 3;

				bJmpDir = TRUE;
				continue;
			}
			else if ( is_path_separator_char(lpDirOffset[1]) )
			{
				lpDirOffset += 2;

				bJmpDir = TRUE;
				continue;
			}
		}
		else if ( lpDirOffset[0] == _T('\\') )
		{
			lpDirOffset++;

			bJmpDir = TRUE;
			continue;
		}

		if ( bJmpDir )
		{
			bJmpDir = FALSE;
			*lpFileNameOffset++ = _T('\\');
		}

		*lpFileNameOffset++ = *lpDirOffset++;
		*lpFileNameOffset = 0;
	}

	if ( !bJmpDir )
	{
		size_t dwLen1 = _tcslen(m_szCmd);

		lpCmdBuff = new TCHAR[dwLen1 +  _tcslen(lpCmdEnd) + 2];
		if ( lpCmdBuff != NULL )
		{
			_tcscpy(lpCmdBuff, m_szCmd);
			if ( lpCmdBuff[dwLen1 - 1] == _T('\\') )
			{
				dwLen1--;
			}

			_tcscpy(lpCmdBuff + dwLen1, lpCmdEnd);
		}
	}

_Failed_Exit:
	return lpCmdBuff;
}

BOOL CCmdLineParser::DoParser(LPCTSTR lpCmdLine,
							  LPCTSTR lpCurDir, 
							  LPCTSTR lpEnvVars, 
							  LPCTSTR lpExtNames,
							  LPCTSTR lpParentPath
							  )
{
	BOOL	bResult = FALSE;
	LPCTSTR	lpCmdBegin = NULL;
	BOOL	bQuotation = FALSE;
	
	m_szCmd[0] = 0;

	delete m_lpParam;
	m_lpParam = NULL;

	if ( is_empty_str(lpCmdLine) ) return FALSE;
	if ( is_empty_str(lpParentPath) ) lpParentPath = NULL;
	if ( is_empty_str(lpCurDir) ) lpCurDir = NULL;
	if ( is_empty_str(lpEnvVars) ) lpEnvVars = NULL;
	if ( is_empty_str(lpExtNames) ) lpExtNames = NULL;

	/*
	1、The directory from which the application loaded. 
	2、The current directory for the parent process. 
	3、The 32-bit Windows system directory. Use the GetSystemDirectory function to get the path of this directory.
	Windows Me/98/95:  The Windows system directory. Use the GetSystemDirectory function to get the path of this directory.
	4、The 16-bit Windows system directory. There is no function that obtains the path of this directory, but it is searched. The name of this directory is System. 
	5、The Windows directory. Use the GetWindowsDirectory function to get the path of this directory. 
	6、The directories that are listed in the PATH environment variable. Note that this function does not search the per-application path specified by the App Paths registry key. To include this per-application path in the search sequence, use the ShellExecute function. 
	*/
	lpCmdBegin = skip_blank(lpCmdLine);
	if ( *lpCmdBegin == _T('\"') )
	{
		lpCmdBegin = skip_quotation(lpCmdBegin);
		bQuotation = TRUE;
	}

	if ( (m_dwFlag & DEF_EXT_NAME) && lpExtNames == NULL )
	{
		// 默认扩展名
		lpExtNames = _T("bat;cmd;exe;pif");
	}

	if ( _tcsnicmp(lpCmdBegin, _T("\\??\\"), 4) == 0 )
	{
		lpCmdBegin += 4;
	}

	if ( _tcsnicmp(lpCmdBegin, _T("file://"), 7) == 0 )
	{
		if ( lpCmdBegin[8] == _T(':') )
		{
			lpCmdBegin += 7;
		}
	}

	/*
		宏展开
	*/
	if ( *lpCmdBegin == _T('%') )
	{
		LPTSTR lpCmdBuff = ExpandMacro(lpCmdBegin);
		if ( lpCmdBuff != NULL )
		{
			bResult = ExpandAbsolute(lpCmdBuff, lpExtNames, bQuotation);

			delete lpCmdBuff;
			lpCmdBuff = NULL;
		}
		
		goto _Exit;
	}
	
	/*
		绝对路径展开
	*/
	if ( is_absolute_path(lpCmdBegin) )
	{
		bResult = ExpandAbsolute(lpCmdBegin, lpExtNames, bQuotation);
		goto _Exit;
	}

	/*
		相对路径转换
	*/
	if ( (lpCurDir != NULL) || (m_dwFlag & DEF_CUR_DIR) )
	{
		LPTSTR lpCmdBuff = ExpandRelative(lpCmdBegin, lpCurDir, bQuotation);
		if ( lpCmdBuff != NULL )
		{
			bResult = ExpandAbsolute(lpCmdBuff, lpExtNames, bQuotation);

			delete lpCmdBuff;
			lpCmdBuff = NULL;

			if ( bResult )
			{
				goto _Exit;
			}
		}
	}
	
	/*
		二次宏展开
	*/
	if ( m_dwFlag & DEF_ENV_VAR )
	{
		LPTSTR lpCmdBuff = ExpandMacro2(lpCmdBegin);
		if ( lpCmdBuff != NULL )
		{
			bResult = ExpandAbsolute(lpCmdBuff, lpExtNames, bQuotation);
			
			delete lpCmdBuff;
			lpCmdBuff = NULL;

			if ( bResult )
			{
				goto _Exit;
			}
		}
	}

	/*
		在父程序目录下查找
	*/
	if ( lpParentPath != NULL )
	{
		if ( GetLongPathName(lpParentPath, m_szCmd, MAX_PATH) != 0 )
		{
			LPTSTR lpFileName = (LPTSTR)get_file_name(m_szCmd);
			*lpFileName = 0;
			DWORD dwLen2 = (DWORD)_tcslen(m_szCmd);

			LPTSTR lpCmdBuff = new TCHAR[dwLen2 + _tcslen(lpCmdBegin) + 2];
			if ( lpCmdBuff != NULL )
			{
				_tcscpy(lpCmdBuff, m_szCmd);
				if ( lpCmdBuff[dwLen2 - 1] != _T('\\') )
				{
					lpCmdBuff[dwLen2++] = _T('\\');
				}
				_tcscpy(lpCmdBuff + dwLen2, lpCmdBegin);
				//MessageBox(0, lpCmdBuff, NULL, 0);

				bResult = ExpandAbsolute(lpCmdBuff, lpExtNames, bQuotation);
				
				delete lpCmdBuff;
				lpCmdBuff = NULL;

				if ( bResult )
				{
					goto _Exit;
				}
			}
		}
	}

	/*
	环境变量中查找
	*/
	if ( lpEnvVars != NULL || (m_dwFlag & DEF_ENV_VAR) )
	{
		LPCTSTR lpCmdEnd = NULL;

		if ( bQuotation )
		{
			lpCmdEnd = _tcschr(lpCmdBegin, _T('\"'));
			if ( lpCmdEnd == NULL )
			{
				goto _Exit;
			}
		}
		else
		{
			lpCmdEnd = skip_no_blank(lpCmdBegin);
		}

		if ( !fix_path_separator(m_szCmd, MAX_PATH, lpCmdBegin, lpCmdEnd - lpCmdBegin) )
		{
			goto _Exit;
		}

		if ( !is_relative_path(m_szCmd) )
		{
			bResult = ExpandEnvVars(m_szCmd, lpEnvVars, lpExtNames);
			if ( bResult )
			{
				ExpandParam(lpCmdEnd);
			}
		}
	}

_Exit:
	if ( !bResult )
	{
		m_szCmd[0] = 0;
	}

	return bResult;
}


DWORD GetFileListFromCmd(CCmdLineParser& cmdParser, CAtlArray<CString>& fileArray, LPCTSTR lpCurDir)
{
	LPCTSTR lpCmd, lpParam;
	if ( (lpCmd = cmdParser.GetCmd()) != NULL && (lpParam = cmdParser.GetParam()) != NULL )
	{
		LPCTSTR lpFileName = _tcsrchr(lpCmd, _T('\\'));
		if ( lpFileName != NULL )
		{
			lpFileName++;
			if ( _tcsicmp(lpFileName, _T("rundll32.exe")) == 0 )
			{
				CCmdLineParser a((DWORD)0);
				if ( a.DoParser(lpParam, lpCurDir, NULL, _T("dll")) )
				{
					fileArray.Add(a.GetCmd());
				}//
			}
			else if ( _tcsicmp(lpFileName, _T("wscript.exe")) == 0 ||
				_tcsicmp(lpFileName, _T("cscript.exe")) == 0
				)
			{
				LPCTSTR lpStr = lpParam;
				while ( *lpStr != 0 )
				{
					lpStr = skip_blank(lpStr);
					if ( *lpStr == _T('/') || *lpStr == _T('-') )
					{
						lpStr = skip_no_blank(lpStr);
					}
					else//
					{
						CCmdLineParser a((DWORD)0);
						if ( a.DoParser(lpStr, lpCurDir, NULL, _T("wsh;vbs;vbe;")) )
						{
							fileArray.Add(a.GetCmd());
						}
						break;
					}//
				}
			}
			else if ( _tcsicmp(lpFileName, _T("cmd.exe")) == 0 )
			{
				LPCTSTR lpStr = lpParam;
				while ( *lpStr != 0 )
				{
					lpStr = skip_blank(lpStr);
					if ( *lpStr == _T('/') || *lpStr == _T('-') )
					{
						lpStr = skip_no_blank(lpStr);
					}
					else
					{
						CCmdLineParser a((DWORD)0);
						if ( a.DoParser(lpStr, lpCurDir, NULL, _T("bat;cmd;exe;")) )
						{
							fileArray.Add(a.GetCmd());
							GetFileListFromCmd(a, fileArray, lpCurDir);
						}
						break;
					}
				}
			}
			else if ( _tcsicmp(lpFileName, _T("regsvr32.exe")) == 0 )
			{
				LPCTSTR lpStr = lpParam;
				while ( *lpStr != 0 )
				{
					lpStr = skip_blank(lpStr);
					if ( *lpStr == _T('/') || *lpStr == _T('-') )
					{
						lpStr = skip_no_blank(lpStr);
					}
					else
					{
						CCmdLineParser a((DWORD)0);
						if ( a.DoParser(lpStr, lpCurDir) )
						{
							fileArray.Add(a.GetCmd());
						}
						break;
					}
				}
			}
		}
	}

	return (DWORD)fileArray.GetCount();
}

