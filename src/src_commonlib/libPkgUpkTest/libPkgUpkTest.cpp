// unpackofficeexe.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../libPkgUpk/LibPkgUpk.h"
#include "../libPkgUpk/Utils.h"
#pragma comment(lib, "../libPkgUpk/lib/FDI.LIB")


#include "../VulPatchlib/UserPatcher.h"
//#define MODULE_PATH _T("D:\\D.Conew\\Beike\\repository\\release_1044\\bin\\debug\\")
#define MODULE_PATH _T("")
LPCTSTR szPath7z = MODULE_PATH _T("7z.dll");
LPCTSTR szPathWcx = MODULE_PATH _T("msi.wcx");

#include <vulfix/BeikeUtils.h>

bool UpkUse7z(LPCTSTR szExeFile, LPCTSTR szExtractPath, Files &files)
{
#if 0 
	static CExeUnpack7z *upk=NULL;
	if(!upk)
	{
		upk = new CExeUnpack7z;
		upk->Initialize(szPath7z, szPathWcx);
	}
	
	strings fs;
	CStringA s1 = CT2CA(szExeFile);
	CStringA s2 = CT2CA(szExtractPath);
	bool bRet = upk->Extract(s1, s2, fs);
	if(bRet)
	{
		for(size_t i=0; i<fs.size(); ++i)
		{
			files.push_back( CString(CA2CT(fs[i].c_str())) );
		}
	}
	return bRet;
#endif
	return false;
}

void usage()
{
	_tcprintf(_T("(1|2) (file|path) [outfile]  1使用7z 2使用新方法\r\n"));
}

void _TestUpk()
{
	LPCTSTR szExeFile = _T("O:\\office\\2003\\ksafe_office2003-KB951535-FullFile-CHS.exe");
	LPCTSTR szExtractPath = _T("C:\\Program Files\\masdfasdf");
	DeletePath(szExtractPath);

	Files files;
	TRUE ? ExtractOfficePackage(szExeFile, szExtractPath, files) : UpkUse7z(szExeFile, szExtractPath, files);
	DumpFiles(files);
}

VOID GetTempUpkPath( CString &strTmpPath )
{
	TCHAR szTempPath[MAX_PATH] = {0};
#if 1 
	//TCHAR szTempName[MAX_PATH] = {0};
	//GetTempPath(MAX_PATH, szTempName);
	GetTempFileName(_T("c:\\office.upk\\"), _T("KVUL.UPK"), 0, szTempPath);
	DeleteFile( szTempPath );
#else
	_tcscpy(szTempPath, _T("c:\\office.unpack\\"));
#endif
	strTmpPath = szTempPath;
}

void UpkExeFile( BOOL bNewMethod, LPCTSTR szPath )
{
	CString strTmpPath;
	GetTempUpkPath(strTmpPath);
	CreateDirectoryNested(strTmpPath);

	Files files;
	INT err = bNewMethod ? ExtractOfficePackage(szPath, strTmpPath, files) : UpkUse7z(szPath, strTmpPath, files);

	_tprintf(_T("- UPK OFFICE FILE :%s - %d -> %s\r\n"), szPath, err, strTmpPath);
	DumpFiles(files, TRUE);
	_tprintf(_T("--END \r\n"));

	//DeletePath(strTmpPath);
}

struct RPathHelper_UpkFile
{
public:
	RPathHelper_UpkFile(BOOL bNewMethod) : m_bNewMethod(bNewMethod)
	{
	}

	BOOL operator()(LPCTSTR szPath, WIN32_FIND_DATA &FindFileData)
	{
		if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{
			LPCTSTR szExt = _tcsrchr(FindFileData.cFileName, _T('.'));
			if(szExt && _tcsicmp(szExt, _T(".exe"))==0)
			{
				TCHAR szFile[MAX_PATH] = {0};
				_tcscpy(szFile, szPath);
				PathAppend(szFile, FindFileData.cFileName);

				UpkExeFile(m_bNewMethod, szFile);
			}
		}
		return TRUE;
	}
	BOOL m_bNewMethod;
};



// prints information about a file in cvs format

void PrintFindData2(LPCTSTR szFilePath, FILE *stream)
{	
	FILETIME ft;
	SYSTEMTIME st;
	DWORD   dwVerInfoSize;        
	DWORD   dwVerHnd=0;
	BOOL    bRetCode;
	LPSTR   lpBuffer;
	UINT*    dwBytes = new UINT;
	char *fileName;
	char fullName[256];
	
	dwVerInfoSize = GetFileVersionInfoSize(szFilePath, &dwVerHnd);
	if (dwVerInfoSize)
	{
		LPTSTR   lpstrVffInfo; 

		HANDLE  hMem;
		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpstrVffInfo  = (TCHAR *)GlobalLock(hMem);
		GetFileVersionInfo(szFilePath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);

		// Company Name
#define PRINT_KEY(x) \
		bRetCode = VerQueryValue((LPVOID)lpstrVffInfo, TEXT("\\StringFileInfo\\000004E4\\"##x), (LPVOID *)&lpBuffer, dwBytes);\
		if(bRetCode)\
		{\
		_ftprintf(stream, _T("%s:%s, "), ##x, lpBuffer);\
		}\
		else\
			_ftprintf(stream, _T(", "));

		PRINT_KEY(_T("CompanyName"));
		PRINT_KEY(_T("FileDescription"));
		PRINT_KEY(_T("FileVersion"));
		PRINT_KEY(_T("InternalName"));
		PRINT_KEY(_T("LegalCopyright"));
		PRINT_KEY(_T("OriginalFilename"));
		PRINT_KEY(_T("ProductName"));
		PRINT_KEY(_T("ProductVersion"));
		PRINT_KEY(_T("Comments"));
		PRINT_KEY(_T("LegalTrademarks"));
		PRINT_KEY(_T("PrivateBuild"));
		PRINT_KEY(_T("SpecialBuild"));
		_ftprintf(stream, _T("\n"));
	}
	else
		_ftprintf(stream, _T(", , , , , , , , , , ,\n"));
}

int _tmain(int argc, _TCHAR* argv[])
{
	//PrintFindData2(_T("c:\\office-kb981715-fullfile-x86-glb.exe"), stdout);
	if(argc<3)
	{
		usage();
		return 0;
	}

	BOOL bNewMethod = TRUE;

	LPCTSTR szMethod = NULL;
	LPCTSTR szPath =NULL;
	if(argc>=3)
	{
		szMethod = argv[1];
		szPath = argv[2];
		if(szMethod[0]==_T('1'))
			bNewMethod = FALSE;
	}
	
	if(PathIsDirectory(szPath))
	{
		RPathHelper_UpkFile upk(bNewMethod);
		RecursePath(szPath, upk);
	}
	else if(PathFileExists(szPath))
	{
		UpkExeFile(bNewMethod, szPath);
	}
	return 0;
}

