#include "StdAfx.h"
#include "LibPkgUpk.h"
#include "Utils.h"
#include "MsiUpk.h"
#include "CabUpk.h"
#include <vulfix/BeikeUtils.h>


#pragma comment(lib, "version.lib")

BOOL GetPEFileDescription(LPCTSTR lpszFileName, CString &strFileDescription)
{
	BOOL bResult = FALSE, bRet = FALSE;
	DWORD dwHandle          = 0;
	DWORD dwFileInfoSize    = 0;
	BYTE *pbyInfo           = NULL;
	UINT uLen               = 0;

	dwFileInfoSize = ::GetFileVersionInfoSize(lpszFileName, &dwHandle);
	if (0 == dwFileInfoSize)
		goto Exit0;

	pbyInfo = new BYTE[dwFileInfoSize];
	if (!pbyInfo)
		goto Exit0;

	bRet = ::GetFileVersionInfo(lpszFileName, dwHandle, dwFileInfoSize, pbyInfo);
	if (!bRet)
		goto Exit0;

	// Structure used to store enumerated languages and code pages.
	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate=NULL;

	// Read the list of languages and code pages.
	if( ::VerQueryValue(pbyInfo, _T("\\VarFileInfo\\Translation"), (LPVOID *)&lpTranslate, &uLen) )
	{
		// Read the file description for each language and code page.
		for( int i=0; i < (uLen/sizeof(struct LANGANDCODEPAGE)); i++ )
		{
			TCHAR szBlock[MAX_PATH+1] = {0};
			_stprintf(szBlock, _T("\\StringFileInfo\\%04x%04x\\FileDescription"), lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
			// Retrieve file description for language and code page "i". 
			LPTSTR   lpBuffer=NULL;
			if( ::VerQueryValue(pbyInfo, szBlock, (LPVOID *)&lpBuffer, &uLen) )
			{
				strFileDescription = lpBuffer;
				bResult = TRUE;
				break;
			}
			++ lpTranslate;
		}
	}

Exit0:
	if (pbyInfo)
	{
		delete[] pbyInfo;
		pbyInfo = NULL;
	}

	return bResult;
}


BOOL ExtractExe(LPCTSTR szExeFilename, LPCTSTR szDestPath, Files &files);
BOOL ExtractCab(LPCTSTR szCabFilename, LPCTSTR szDestPath, Files &files);

#define UPK_PATH_SUFFIX _T("_")

ENUM_ExtractErrorCode ExtractOfficePackage( LPCTSTR szPkgFilename, LPCTSTR szDestPath, Files &files )
{
	Files mspfiles, cabfiles, dstfiles;
	
	if(!CreateDirectoryNested(szDestPath))
		return EE_PATH;
	
	if(!ExtractExe(szPkgFilename, szDestPath, mspfiles))
		return EE_EXE;
	
	for(int i=0; i<mspfiles.size(); ++i)
	{
		CString strPath = mspfiles[i];
		strPath += UPK_PATH_SUFFIX;
		if(!CreateDirectoryNested(strPath))
			return EE_PATH;
		if(!ExtractMSP(mspfiles[i], strPath, cabfiles))
			return EE_MSP;
	}

	for(int i=0; i<cabfiles.size(); ++i)
	{
		CString strPath = cabfiles[i];
		strPath += UPK_PATH_SUFFIX;
		if(!CreateDirectoryNested(strPath))
			return EE_PATH;
		if(!ExtractCab(cabfiles[i], strPath, files))
			return EE_CAB;
	}
	return EE_SUCCESS;
}

BOOL ExtractExe( LPCTSTR szExeFilename, LPCTSTR szDestPath, Files &files )
{
	LPCTSTR szFolder = szDestPath;

	LPCTSTR p1=NULL, p2=NULL;
	p1 = _tcsrchr(szExeFilename, _T('/'));
	p2 = _tcsrchr(szExeFilename, _T('\\'));
	if(p1 && p2)
		p1 = max(p1, p2);
	else if(p2)
		p1 = p2;
	if(!p1)
		return FALSE;
	
	CString strParam;
	INT nState = 0;
	if(_tcsstr(p1, _T("2003")))
		nState = 1;
	else if(_tcsstr(p1, _T("2007")))
		nState = 2;
	else if(_tcsstr(p1, _T("2010")))
		nState = 3;
	else
	{
		// 根据文件的属性来获取是否Office  : office-kb981715-fullfile-x86-glb.exe
		CString strFileDescription;
		if( GetPEFileDescription(szExeFilename, strFileDescription) )
		{
			strFileDescription.MakeLower();
			if(_tcsstr(strFileDescription, _T("office"))!=NULL)
			{
				if(_tcsstr(strFileDescription, _T("2003")))
					nState = 1;
				else if(_tcsstr(strFileDescription, _T("2007")))
					nState = 2;
				else if(_tcsstr(strFileDescription, _T("2010")))
					nState = 3;
			}
		}
	}

	switch ( nState )
	{
	case 1:
		strParam.Format(_T("/Q /C /T:\"%s\""), szDestPath);
		break;
	case 2:
	case 3:
		strParam.Format(_T("/q /extract:\"%s\""), szDestPath);
		break;
	default:
		return FALSE;
		break;
	}	

	DWORD dwExitCode = 0;
	if(!ExecuteFile(szExeFilename, strParam, dwExitCode))
		return FALSE;

	RPathHelper_GetMSPFile rp(files);
	RecursePath(szFolder, rp);
	return !files.empty();
}

BOOL ExtractCab( LPCTSTR szCabFilename, LPCTSTR szDestPath, Files &files )
{
	CCabExtract ex;
	if(!ex.Extract(szCabFilename, szDestPath))
		return FALSE;
	std::copy(ex.m_files.begin(), ex.m_files.end(), std::back_inserter(files));
	return TRUE;
}
