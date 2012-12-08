#pragma once
#include "LibPkgUpk.h"

#ifdef _DEBUG
#define DBG_TRACE _tcprintf
#define DBG_TRACEA printf
#else
#define DBG_TRACE 
#define DBG_TRACEA 
#endif

BOOL ExecuteFile(LPCTSTR lpszFilename, LPCTSTR lpszInstallParam, DWORD &dwExitCode);
INT DeletePath(LPCTSTR lpszPath);
void DumpFiles(const Files& files, BOOL bOnlyFileName=FALSE);

struct RPathHelper_FolderSize
{
public:
	INT64 m_iFolderSize;
	RPathHelper_FolderSize() : m_iFolderSize(0) 
	{
	}

	BOOL operator()(LPCTSTR szPath, WIN32_FIND_DATA &FindFileData)
	{
		if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{
			ULARGE_INTEGER nFileSize;
			nFileSize.LowPart = FindFileData.nFileSizeLow;
			nFileSize.HighPart = FindFileData.nFileSizeHigh;
			m_iFolderSize += nFileSize.QuadPart;
		}
		return TRUE;
	}
};

struct RPathHelper_GetMSPFile
{
public:
	Files &m_MSPfiles;
	RPathHelper_GetMSPFile(Files &files) : m_MSPfiles(files)
	{ }

	BOOL operator()(LPCTSTR szPath, WIN32_FIND_DATA &FindFileData)
	{
		if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{
			LPCTSTR szExt = _tcsrchr(FindFileData.cFileName, _T('.'));
			if(szExt && _tcsicmp(szExt, _T(".msp"))==0)
			{
				TCHAR szFile[MAX_PATH] = {0};
				_tcscpy(szFile, szPath);
				PathAppend(szFile, FindFileData.cFileName);
				m_MSPfiles.push_back(szFile);
			}
		}
		return TRUE;
	}
};
template<typename Func>
BOOL RecursePath( LPCTSTR szFolder, Func &fn )
{
	BOOL bContinue = TRUE;
	CString strFolder(szFolder);
	strFolder.Append( _T("\\*") );
	WIN32_FIND_DATA FindFileData;	
	HANDLE hFind = FindFirstFile(strFolder, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		//DBG_TRACE(_T("First file name is %s\n"), FindFileData.cFileName);
		do 
		{
			//DBG_TRACE(_T("Next file name is %s\n"), FindFileData.cFileName);
			if( _tcsicmp(_T("."), FindFileData.cFileName)==0 
				|| _tcsicmp(_T(".."), FindFileData.cFileName)==0 )
			{
				continue;
			}

			bContinue = fn(szFolder, FindFileData);
			if(!bContinue)	break;

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

				bContinue = RecursePath(strResult, fn);
				if(!bContinue)	break;
			}

		} while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}
	return bContinue;
}