//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

#ifndef _KCHECKAPPEXIST_H_
#define _KCHECKAPPEXIST_H_

//////////////////////////////////////////////////////////////////////////

#include <string>

//////////////////////////////////////////////////////////////////////////

class KCheckApp
{
public:
	std::wstring strSubKey;
	std::wstring strKeyName;
	HKEY hKey;
	BOOL bIsFile;
	std::wstring strFileName;
};

class KCkeckByUnistall
{
public:
	std::wstring strDisplayName;
	std::wstring strUninstallString;
	std::wstring strInstallLocation;
	std::wstring strDisplayIcon;
};
//////////////////////////////////////////////////////////////////////////
// ÓÐ¿Õ¼Ó¼Ó×¢ÊÍ
BOOL QueryUninstallData(const std::wstring pszDisplayName, 
						const std::wstring pszFileName);
BOOL UninstallInfoCheck(const KCkeckByUnistall appUninatall, 
						const std::wstring pszDisplayName, 
						const std::wstring pszFileName);
BOOL IsFileExists( LPCTSTR pszFile );
BOOL IsAlpha( TCHAR c );
BOOL IsNumber( TCHAR c );
BOOL IsPossibleSeperator( TCHAR c );
BOOL GetPathFromDisplayIcon(LPCTSTR pszDisplayIcon, std::wstring& strPath);
BOOL GetPathFromUninstallString(LPCTSTR pszUninstallString, std::wstring& strPath);
LPCTSTR HasRundll( LPCTSTR pszCmdLine );
BOOL _KillParamEx( LPCTSTR pszCmdLine, LPTSTR pszFileName, int nSize );
BOOL SearchFirstPath( LPCTSTR pszCmdLine, LPCTSTR* ppszStart, LPCTSTR* ppszEnd );
BOOL GetFirstPath( LPCTSTR pszCmdLine, LPTSTR pszFileName, int nSize );
BOOL CheckFileIsExist(const std::wstring strFilePath);
BOOL GetAppPathByReg(const KCheckApp theAppInfo);
BOOL CheckPathFiles(LPCTSTR szFilePath);
BOOL GetComputerDrives(std::vector<CString>& vecDrive);

BOOL _Check360Exist();
BOOL _CheckTheworldExist();
BOOL _CheckFireFoxExist();
BOOL _CheckMaxthonExist();
BOOL _CheckTTExist();
BOOL _CheckChromeExist();
BOOL _CheckSogouExist();

BOOL _CheckOfficeExist();

BOOL _CheckTudouVideo();
BOOL _CheckXunleiVideo();
BOOL _CheckYoukuVideo();
BOOL _CheckKu6Video();
BOOL _CheckPPTVVideo();
BOOL _CheckFengxing();
BOOL _CheckQQlive();
BOOL _CheckOperaExist();

BOOL _CheckSafariExist();
BOOL _CheckQvodVideo();
BOOL _CheckPicasaExist();
BOOL _CheckStormVideo();
BOOL _CheckPPSreamVideo();
BOOL _CheckQQMusic();
BOOL _CheckPIPIVideo();

#endif
//////////////////////////////////////////////////////////////////////////
extern HRESULT CheckProcessInstaller(UINT nSoftID);
extern HRESULT CheckSoftInstalled(UINT nSoftID);
extern HRESULT CheckSoftPathExist(std::vector<CString>& vPaths);
//////////////////////////////////////////////////////////////////////////

#define CHK_BEGIN_METHOD_MAP() \
	HRESULT CheckProcessInstaller(UINT nSoftID) { \
	BOOL retval = FALSE; \
	if (FALSE) {}

#define CHK_METHOD_ENTRY(ID, FUNC) \
		else if (ID == nSoftID) { retval = FUNC(); }

#define CHK_END_METHOD_MAP() \
		else {} \
		return retval; \
}

////////////////////////////////////////////////////////////////////////
//  Example:
// 
//    CHK_BEGIN_METHOD_MAP()
//        CHK_METHOD_ENTRY(100, _CheckChromeExist)
//    CHK_END_METHOD_MAP()

////////////////////////////////////////////////////////////////////////