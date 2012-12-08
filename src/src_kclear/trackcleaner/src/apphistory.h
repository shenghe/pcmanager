#ifndef _APPHISTORY_H_
#define _APPHISTORY_H_

#include "CleanProc.h"
#include "i_clean.h"

#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"

/************************************************************************/
//功能:应用软件清理
//
/************************************************************************/

extern ScanFileCallBack g_fnScanFile;
extern ScanRegCallBack  g_fnScanReg;
extern void*			g_pMain;

class CAppHistory
{
public:
	CAppHistory();
	~CAppHistory();
public:
	void	Start()
	{
		m_bScan=TRUE;
		m_fileOpt.UserBreadFlag(FALSE);
		m_regOpt.UserBreadFlag(FALSE);

	};
	void	StopScan()
	{ 
		m_bScan=FALSE;
		m_fileOpt.UserBreadFlag(TRUE);
		m_regOpt.UserBreadFlag(TRUE);
	};
public:
	BOOL CommfunReg(int iType,LPCTSTR lpcszRegPath,CSimpleArray<CString>& vec_valueName);
	BOOL CommfunFile(int iType,LPCTSTR lpcszFilePath,CSimpleArray<CString>& vec_fileName,BOOL bRecursion=TRUE,BOOL bEnumFiles=TRUE);
	
	BOOL CommfunRegProc(int iType,HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData );
	BOOL CommfunFileProc(int iType,LPCTSTR lpFileOrPath,LARGE_INTEGER filesize);
	BOOL ConvetPath(CString& strSourcPath);
	CString GetFolderPath(LPCTSTR pszName);
private:
	CFileOpt				m_fileOpt;
	CRegOpt					m_regOpt;
	BOOL					m_bScan;
	TCHAR					m_szLoginName[MAX_PATH];
public:
	static BOOL WINAPI myEnumerateFile(LPCTSTR lpFileOrPath, void* pUserData,LARGE_INTEGER filesize);
	static BOOL WINAPI myEnumerateReg(HKEY rootKey,LPCTSTR lpItemPath,LPCTSTR lpName,DWORD dwValueNameSize,LPBYTE lpValue ,DWORD dwValueSize,DWORD dwType,void* pUserData);
};

#endif