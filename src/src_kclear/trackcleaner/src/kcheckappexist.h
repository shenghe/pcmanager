#ifndef _KCHECKAPPEXIST_H_
#define _KCHECKAPPEXIST_H_

#include <string>
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
BOOL _Check360Exist();
BOOL _CheckTheworldExist();
BOOL _CheckFireFoxExist();
BOOL _CheckMaxthonExist();
BOOL _CheckTTExist();
BOOL _CheckChromeExist();
BOOL _CheckSogouExist();


#endif