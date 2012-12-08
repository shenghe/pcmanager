#ifndef _IECLEAN_H_
#define _IECLEAN_H_

#include "CleanProc.h"
#include "i_clean.h"

#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"
#include "FileMapWrite.h"
#include <string>
#include "apphistory.h"

/************************************************************************/
//功能:清理IE使用痕迹 
//
/************************************************************************/

extern ScanFileCallBack g_fnScanFile;
extern ScanRegCallBack  g_fnScanReg;
extern void*			g_pMain;

class CIEClean :IClean
{
public:
	CIEClean();
	virtual ~CIEClean();
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
	//临时文件夹 1
	BOOL	ScanIETempFolder();
	BOOL	ScanIETempFolder(LPCTSTR lpFileOrFolderPath);
	BOOL	TempFoldFolder(LPCTSTR lpFileOrPath,LARGE_INTEGER filesize);
	//访问过的网址 2
	BOOL	ScanVisitHost();
	BOOL	ScanVisitHost(LPCTSTR lpFileOrFolderPath);
	BOOL	VisitHost(LPCTSTR lpFileOrPath,LARGE_INTEGER filesize);
	//IE Cookie 3
	BOOL	ScanIECookie(int nType);
	BOOL	ScanIECookie(LPCTSTR lpszIECookiePath);
	BOOL	IECookie(LPCTSTR lpFileOrPath,LARGE_INTEGER filesize);
	
	BOOL	ScanMaxthon3Cookies();
	//地址栏缓冲 4
	BOOL	ScanIEAddrBar();
	BOOL	ScanIEAddrBar(LPCTSTR lpszIEAddrBarPath);
	BOOL	IEAddrBar(LPCTSTR lpValueName,LPCTSTR lpValueData);
	//ie密码
	//废弃
	BOOL	ScanIEPass(int nType);
	BOOL	ScanIEPassword(LPCTSTR lpszIEAddrBarPath);
	BOOL	IEPassword(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData);
	//ie表单
	//废弃
	BOOL	ScanIEFrom(int nType);
	BOOL	ScanIEFrom(LPCTSTR lpszIEAddrBarPath);
	BOOL	IEFrom(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData);
	BOOL	IEFrom_Clean();
	//Ie密码
	BOOL	CleanIEPassword();
	//Ie表单
	BOOL	CleanFrom();
	//擦除index文件
	BOOL	_CleanIndexDat(); //不做任何事情
	BOOL	CleanIndexDat();
	BOOL	CleanIndex(LPCTSTR lpFileOrPath,LARGE_INTEGER filesize);
	BOOL	ClearLen(LPCTSTR lpFile);
	BOOL	WipeTime(LPCTSTR lpFile);

	//清除ie历史记录
	BOOL    ScanIEHistory();
	BOOL    CleanIEHistory(CString pszUrl);
	std::string deescapeURL(const std::string &URL);
	short int hexChar2dec(char c) ;

	//辅助函数
	BOOL	IsWindowsNT();//判断系统
	BOOL	IsWindows2k();
	BOOL	GetUserSid(PSID* ppSid);
    void	GetSidString(PSID pSid, CString& strSid);
	BOOL	GetOldSD(HKEY hKey, LPCTSTR pszSubKey, BYTE** pSD);
	BOOL	CreateNewSD(PSID pSid, SECURITY_DESCRIPTOR* pSD, PACL* ppDacl);
	BOOL	RegSetPrivilege(HKEY hKey, LPCTSTR pszSubKey, SECURITY_DESCRIPTOR* pSD, BOOL bRecursive);

private:
	CFileOpt				m_fileOpt;
	CRegOpt					m_regOpt;
	TCHAR					m_szLoginName[MAX_PATH];
	BOOL					m_bScan;
	int		                m_nFlags;
	CAppHistory             m_appHistory;
	BOOL                    m_bFlagPass[10000];
	BOOL                    m_bFlagForm[10000];
public:
	static BOOL WINAPI myEnumerateFile(LPCTSTR lpFileOrPath, void* pUserData,LARGE_INTEGER filesize);
	static BOOL WINAPI myEnumerateReg(HKEY rootKey,LPCTSTR lpItemPath,LPCTSTR lpName,DWORD dwValueNameSize,LPBYTE lpValue ,DWORD dwValueSize,DWORD dwType,void* pUserData);

};
	

#endif
