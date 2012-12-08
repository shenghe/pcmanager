#include "StdAfx.h"
#include "../beikesafe.h"
#include "BeikeVulfixFileLog.h"
#include <bksafe/bksafeconfig.h>
#include <vulfix/IVulFix.h>
#include "BeikeVulfixUtils.h"
#include "../beikesafemsgbox.h"

template<typename T>
BOOL LoadIniSectionKeyValues(LPCTSTR iniFilename, LPCTSTR lpszSectionName, CSimpleArray<T> &keyValus)
{
	TCHAR* szBuf = new TCHAR[32768];
	if(!szBuf) return FALSE;

	BOOL bRet = FALSE;
	do 
	{
		if( GetPrivateProfileSection(lpszSectionName, szBuf, 32768 * sizeof(TCHAR), iniFilename)<=0 )
			break;

		LPTSTR szTemp = szBuf;
		while (szTemp[0])
		{
			size_t len = _tcslen(szTemp);
			LPTSTR pequal = _tcschr(szTemp, _T('='));
			if(!pequal) break;			
			*pequal = 0;

			++pequal;
			while( *pequal==_T(' ') || *pequal==_T('\t') )
				++ pequal;

			if(_tcslen(szTemp)>0 && _tcslen(pequal)>0)
			{
				keyValus.Add( T(szTemp, pequal) );
			}
			szTemp += len + 1;
		}
		bRet = TRUE;
	} while (FALSE);

	delete[] szBuf;
	return bRet;
}

template<typename T>
BOOL SaveIniSectionKeyValues(LPCTSTR iniFilename, LPCTSTR lpszSectionName, CSimpleArray<T> &keyValus)
{	
	WritePrivateProfileSection(lpszSectionName, NULL, iniFilename);
	for (int i = 0; i < keyValus.GetSize(); i++)
	{		
		CString strKey, strValue;		
		keyValus[i].GetKeyValue(strKey, strValue);
		WritePrivateProfileString(lpszSectionName, strKey, strValue, iniFilename);
	}
	return TRUE;
}

template<typename T>
int FindArrayIndex( const CSimpleArray<T> &arr, int nID )
{
	for ( int i = 0; i < arr.GetSize(); i++)
	{
		if ( arr[i].nID == nID )
		{
			return i;
		}
	}
	return -1;
}

BOOL CBeikeVulfixFileLog::Load()//源路径
{
	m_DownloadedFiles.RemoveAll();
	m_InstallResults.RemoveAll();	
	CString strDownPath = L"";
	BKSafeConfig::Get_Vulfix_DownloadPath(strDownPath);//从配置文件中读取下载文件信息   当前下载目录
	strDownPath += L"\\vulfix.ini";//配置文件路径	
	LoadIniSectionKeyValues( strDownPath, _T("downloadedfiles"), m_DownloadedFiles);
	LoadIniSectionKeyValues( strDownPath, _T("installedresults"), m_InstallResults);

	m_bDirty = FALSE;
	return TRUE;
}
BOOL CBeikeVulfixFileLog::Load(CString sFileName)//hub  点击选择目录时  所显示的  目的路径
{	
	m_DownloadedFiles2.RemoveAll();
	m_InstallResults2.RemoveAll();
	sFileName += L"\\vulfix.ini";//配置文件路径	
	LoadIniSectionKeyValues( sFileName, _T("downloadedfiles"), m_DownloadedFiles2);
	LoadIniSectionKeyValues( sFileName, _T("installedresults"), m_InstallResults2);
	return TRUE;
}

BOOL CBeikeVulfixFileLog::Save()
{
	CString strIniFile;
	BKSafeConfig::Get_Vulfix_IniFile( strIniFile );
	CString strDownPath = L"";
	BKSafeConfig::Get_Vulfix_DownloadPath(strDownPath);//从配置文件中读取下载文件信息   当前下载目录  hub
	strDownPath += L"\\vulfix.ini";//配置文件路径
	//SaveIniSectionKeyValues( strIniFile, _T("downloadedfiles"), m_DownloadedFiles);
	//SaveIniSectionKeyValues( strIniFile, _T("installedresults"), m_InstallResults);
	SaveIniSectionKeyValues( strDownPath, _T("downloadedfiles"), m_DownloadedFiles);//存到下载目录下  hub
	SaveIniSectionKeyValues( strDownPath, _T("installedresults"), m_InstallResults);
	m_bDirty = FALSE;
	return TRUE;
}

BOOL CBeikeVulfixFileLog::CleanFiles( BOOL bCleanAll, LPCTSTR szFolder )
{
	return bCleanAll ? _DeleteAll(szFolder) : _DeleteInstalled(szFolder);
}

INT CBeikeVulfixFileLog::MoveFiles( LPCTSTR szPathFrom, LPCTSTR szPathTo, HWND hWnd )
{	
	CString sPathFrom, sPathTo;
	sPathFrom = szPathFrom;
	sPathTo = szPathTo;
	
	if( sPathFrom.CompareNoCase(sPathTo)==0 )
		return FALSE;
	
	CString strTitle;
	if(!CreateDirectoryNested(szPathTo))
	{
		strTitle.Format(BkString::Get(IDS_VULFIX_5009), sPathTo);
		CBkSafeMsgBox::Show(strTitle, BkString::Get(IDS_VULFIX_5010), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	INT64 sizeFolder = GetFolderSize( szPathFrom, FALSE, NULL/*IsBkDownloadFile*/ );
	if(sizeFolder==0)
		return TRUE;
	
	strTitle.Format(BkString::Get(IDS_VULFIX_5011), sPathFrom, sPathTo);
	INT id = CBkSafeMsgBox::Show(strTitle, BkString::Get(IDS_VULFIX_5012), MB_YESNO | MB_ICONQUESTION);
	if(id==IDNO)
		return FALSE;
	
	// copy files 
	sPathFrom.Append( _T("\\") BK_FILE_PREFIX _T("*") );
	sPathFrom.AppendChar( _T('\0') );
	sPathFrom.AppendChar( _T('\0') );
	sPathTo.Append( _T("\\\0") );
	sPathTo.AppendChar(_T('\0'));

	SHFILEOPSTRUCT   shfileopstruct = {0};
	shfileopstruct.hwnd = hWnd;   
	shfileopstruct.wFunc = FO_MOVE;
	shfileopstruct.pFrom = sPathFrom;   
	shfileopstruct.pTo   = sPathTo;
	shfileopstruct.fFlags = FOF_FILESONLY;   
	shfileopstruct.hNameMappings = NULL;
	shfileopstruct.lpszProgressTitle = BkString::Get(IDS_VULFIX_5013);
	int nRet = SHFileOperation(&shfileopstruct); 


	BOOL bRet = TRUE;
	///> 移动已下载的文件
	for ( int i = 0 ; i < m_DownloadedFiles.GetSize(); i ++ )
	{
		TPairIdString &downFile = m_DownloadedFiles[i];//源文件的
		CString strSrcFile = szPathFrom;
		strSrcFile += _T("\\");
		strSrcFile += downFile.str;
		if ( IsFileExist( strSrcFile ) )
		{
			CString strDstFile = szPathTo;
			strDstFile += _T("\\");
			strDstFile += downFile.str;
			bRet &= MoveFile( strSrcFile, strDstFile );	
			if (bRet)//成功的话
			{
				CString strFileIniName = szPathFrom;
				strFileIniName += L"\\vulfix.ini";
				if (IsFileExist(strFileIniName))
				{
					CString strDstFile = szPathTo;
					strDstFile += L"\\vulfix.ini";//目的
					CString strKey, strValue;		
					downFile.GetKeyValue(strKey, strValue);
					WritePrivateProfileSection(_T("downloadedfiles"), NULL, strFileIniName);
					WritePrivateProfileString(_T("downloadedfiles"), strKey, strValue, strDstFile);
					//WritePrivateProfileString(_T("downloadedfiles"), strKey, L"", strFileIniName);//把源文件置为空
				}
			}
		}
	}
	

	return nRet;
}

BOOL CBeikeVulfixFileLog::_DeleteAllDownloadSaveInIni( LPCTSTR szFolder )
{
	CString strFolder(szFolder);

	for ( int i = 0 ; i < m_DownloadedFiles.GetSize(); i ++ )
	{
		TPairIdString &downFile = m_DownloadedFiles[i];
		CString strDelete = strFolder;
		strDelete += _T("\\");
		strDelete += downFile.str;
		if ( IsFileExist( strDelete ) )
		{
			SetFileAttributes( strDelete, 0 );
			DeleteFile( strDelete );
		}
	}

	m_DownloadedFiles.RemoveAll();
	return TRUE;
}

BOOL CBeikeVulfixFileLog::_DeleteAllDownloadWithPrefix( LPCTSTR szFolder )
{
	CString strFolder(szFolder);
	strFolder.Append( _T("\\") BK_FILE_PREFIX _T("*") );
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(strFolder, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			if( FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
			}
			else
			{
				TCHAR szPath[MAX_PATH] = {0};
				_tcscpy(szPath, szFolder);
				PathAppend(szPath, FindFileData.cFileName);
				SetFileAttributes( szPath, FILE_ATTRIBUTE_NORMAL);
				DeleteFile( szPath );
			}

		} while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
	}

	return TRUE;
}

BOOL CBeikeVulfixFileLog::_DeleteAll(LPCTSTR szFolder)
{
	_DeleteAllDownloadWithPrefix( szFolder );
	_DeleteAllDownloadSaveInIni( szFolder );
	m_InstallResults.RemoveAll();
	return Save();
}

BOOL CBeikeVulfixFileLog::_DeleteInstalled(LPCTSTR)
{
	CSimpleArray<TPairIdString> arrFile;
	for(int i=0; i<m_DownloadedFiles.GetSize(); ++i)
	{
		TPairIdString &downfile = m_DownloadedFiles[i];
		// if success installed 
		for(int j=0; j<m_InstallResults.GetSize(); ++j)
		{
			TPairIdString &install = m_InstallResults[j];

			if(downfile.nID==install.nID)
			{
				INT nsucceed = _ttoi( install.str );
				if(nsucceed)
				{
					// 可以删除
					arrFile.Add( downfile );
					break;
				}
			}
		}
	}

	// Delete files  
	if(arrFile.GetSize()==0)
		return TRUE;
	
	CString strDownloadPath;
	BKSafeConfig::Get_Vulfix_DownloadPath( strDownloadPath );

	TCHAR szPath[MAX_PATH] = {0};
	for(int i=0; i<arrFile.GetSize(); ++i)
	{
		TPairIdString &downfile = arrFile[i];
		CString strDeletePath = strDownloadPath;
		strDeletePath.TrimRight( _T('\\') );
		strDeletePath += _T("\\");
		strDeletePath += downfile.str;

		SetFileAttributes( strDeletePath, 0 );
		DeleteFile( strDeletePath );		
	}

	// Save files 
	for(int i=0; i<arrFile.GetSize(); ++i)
	{
		TPairIdString &downfile = arrFile[i];
		int idx1 = FindArrayIndex( m_DownloadedFiles, downfile.nID);
		int idx2 = FindArrayIndex( m_InstallResults,downfile.nID);

		m_DownloadedFiles.RemoveAt( idx1 );
		m_InstallResults.RemoveAt( idx2 );
	}
	return Save();
}

BOOL CBeikeVulfixFileLog::UpdateDownload( INT nKBID, LPCTSTR szFilename )
{
	LPCTSTR szEnd = szFilename + _tcslen(szFilename);
	while(--szEnd>szFilename)
	{
		 if( *szEnd=='\\' || *szEnd=='/' )
			 break;
	}
	if(*szEnd=='\\'||*szEnd=='/')
		++szEnd;
	
	TPairIdString down(nKBID, szEnd);
	
	int idx1 = FindArrayIndex( m_DownloadedFiles, nKBID);
	if( idx1==-1 )
		m_DownloadedFiles.Add( down );
	else
		m_DownloadedFiles[ idx1 ] = down;
	return Save();
}

BOOL CBeikeVulfixFileLog::UpdateInstalled( INT nKBID, BOOL bSuccess )
{
	CString str;
	str.Format(_T("%d"), bSuccess);
	TPairIdString p(nKBID, str);

	int idx2 = FindArrayIndex( m_InstallResults, nKBID);
	if( idx2==-1 )
		m_InstallResults.Add( p	);
	else
		m_InstallResults[ idx2 ] = p;
	return Save();
}

INT CBeikeVulfixFileLog::IncHotfixError( INT nKBID )
{
	CString str;
	CString strIniFile;
	BKSafeConfig::Get_Vulfix_IniFile( strIniFile );

	str.Format(VULFIX_HOTFIX_ERROR_KEY_FMT, nKBID);
	INT nError = GetPrivateProfileInt(VULFIX_HOTFIX_ERROR, str, 0, strIniFile);
	
	++nError;
	
	CString strValue;
	strValue.Format(_T("%d"), nError);
	WritePrivateProfileString(VULFIX_HOTFIX_ERROR, str, strValue, strIniFile);
	return nError;
}

VOID CBeikeVulfixFileLog::ClearHotfixError( INT nKBID )
{
	CString str;
	CString strIniFile;
	BKSafeConfig::Get_Vulfix_IniFile( strIniFile );
	str.Format(VULFIX_HOTFIX_ERROR_KEY_FMT, nKBID);
	WritePrivateProfileString(VULFIX_HOTFIX_ERROR, str, NULL, strIniFile);
}

INT64 CBeikeVulfixFileLog::GetDownloadFilesSize()
{
	INT64 n64Size = 0;
	CString strDownloadPath = L"";
	BKSafeConfig::Get_Vulfix_DownloadPath( strDownloadPath );

	for ( int i = 0 ; i < m_DownloadedFiles.GetSize() ; i ++ )
	{
		TPairIdString &downFile = m_DownloadedFiles[i];
		CString strSizeCount = strDownloadPath;
		strSizeCount += _T("\\");
		strSizeCount += downFile.str;

		WIN32_FIND_DATA FindFileData;
		HANDLE hFile = FindFirstFile( strSizeCount, &FindFileData );
		if (hFile != INVALID_HANDLE_VALUE)
		{
			if( !( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
			{
				ULARGE_INTEGER nFileSize;
				nFileSize.LowPart  = FindFileData.nFileSizeLow;
				nFileSize.HighPart = FindFileData.nFileSizeHigh;
				n64Size += nFileSize.QuadPart;
			}

			FindClose( hFile );
		}
	}

	return n64Size;
}
INT64 CBeikeVulfixFileLog::GetDownloadFilesSize(CString strDownloadPath)
{
	INT64 n64Size = 0;	

	for ( int i = 0 ; i < m_DownloadedFiles2.GetSize() ; i ++ )
	{
		TPairIdString &downFile = m_DownloadedFiles2[i];
		CString strSizeCount = strDownloadPath;
		strSizeCount += _T("\\");
		strSizeCount += downFile.str;

		WIN32_FIND_DATA FindFileData;
		HANDLE hFile = FindFirstFile( strSizeCount, &FindFileData );
		if (hFile != INVALID_HANDLE_VALUE)
		{
			if( !( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
			{
				ULARGE_INTEGER nFileSize;
				nFileSize.LowPart  = FindFileData.nFileSizeLow;
				nFileSize.HighPart = FindFileData.nFileSizeHigh;
				n64Size += nFileSize.QuadPart;
			}

			FindClose( hFile );
		}
	}

	return n64Size;
}