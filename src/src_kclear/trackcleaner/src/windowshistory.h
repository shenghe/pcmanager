#ifndef _WINDOWSHISTORY_H_
#define _WINDOWSHISTORY_H_

#include "CleanProc.h"
#include "i_clean.h"

#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"
#include "apphistory.h"
#include "regclean.h"

/************************************************************************/
//功能:window历史记录清理
//
/************************************************************************/

extern CRegClean		g_regClean;
extern ScanFileCallBack g_fnScanFile;
extern ScanRegCallBack  g_fnScanReg;
extern ScanFileCallBack g_fnOutScanFile;
extern ScanRegCallBack  g_fnOutScanReg ;

extern void*			g_pMain;

class  CWindowsHistory
{
public:
	CWindowsHistory();
	~CWindowsHistory();
public:
	void	Start()
	{
		m_bScan=TRUE;
		m_fileOpt.UserBreadFlag(FALSE);
		m_regOpt.UserBreadFlag(FALSE);
		m_appHistory.Start();
	
	};
	void	StopScan()
	{ 
		m_bScan=FALSE;
		m_fileOpt.UserBreadFlag(TRUE);
		m_regOpt.UserBreadFlag(TRUE);
		m_appHistory.StopScan();
	};
	//对话框历史记录
	BOOL	ScanRunDlg();
	BOOL	ScanRunDlg(LPCTSTR lpszRunDlgPath);
	BOOL	RunDlg(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData);
	//最近运行的程序的历史记录
	BOOL	ScanRunHistory();
	BOOL	ScanRunHistory(LPCTSTR lpszRegPath);
	BOOL	RunHistory(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData);
	//最近打开与保存的历史记录
	BOOL	ScanOpenOrSave();
	BOOL	ScanOpenOrSave(LPCTSTR lpszRegPath);
	BOOL	OpenOrSave(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData);
	//窗口大小记录
	BOOL	ScanWinSize();
	BOOL	ScanWinSize(LPCTSTR lpszWinSizePath);
	BOOL	WinSize(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData);
	//最近使用文档记录 (注册表)
	BOOL	ScanRecentDocReg();
	BOOL	ScanRecentDocReg(LPCTSTR lpszRegPath,int iType);
	BOOL	RecentDocReg(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData);
	//最近使用文档记录 (文件)
	BOOL	ScanRecentDocFile();
	BOOL	ScanRecentDocFile(LPCTSTR lpszFileOrFolderPath,int iType);
	BOOL	RecentDocFile(LPCTSTR lpFileOrPath,LARGE_INTEGER filesize);
	//文件扩展名历史记录
	BOOL	ScanFileExtsHistory();
	BOOL	ScanFileExtsHistory(LPCTSTR lpcszFileExtsHis);
	BOOL	FileExtsHistory(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData);
	//最近打开程序历史记录USERASSIST
	BOOL	ScanUserAssist();
	BOOL	ScanUserAssist(LPCTSTR lpcszUserAssistPath);
	BOOL	UserAssist(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData);
	//通知区域图标历史记录
	BOOL	ScanTrayNotify();
	BOOL	ScanTrayNotify(LPCTSTR lpcszTrayNotifyPath);
	BOOL	TrayNotify(HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData);
	/************************************************************************/
	// 以下行为相同,统一用CommonRegFun 函数进行处理
	/************************************************************************/
	//网络驱动器映射历史
	BOOL	ScanNetDerive();
	//查找计算机历史
	BOOL	ScanFindCmp();
	//文档查找历史
	BOOL	ScanFindDoc();
	//打印端口历史
	BOOL	ScanPrtPort();
	//通用处理函数
	BOOL	CommonRegFun(int iType,HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData);
	/************************************************************************/
	// 以下行为相同,统一用CommonFileFun 函数进行处理
	/************************************************************************/
	BOOL	ScanTempFolder();
	BOOL	ScanLogFolder();
	BOOL	ScanRecycled();
	//扫描快捷方式
	BOOL	ScanInvaidDesktopLink();
	BOOL	ScanInvaidDesktopLink(int iType,LPCTSTR lpFileOrPath,CSimpleArray<CString>& vec_fileExts);
	BOOL	ScanInvaidStartLink();
	BOOL	InvalidLink(int iType,LPCTSTR lpFileOrPath,LARGE_INTEGER filesize);	//
	BOOL	_InvalidLink(LPWSTR lpLinkrPath,LPWSTR lpFilePath,int iLen);									//验证快捷方式指向的有效性	
	BOOL	CommonFileFun(int iType,LPCTSTR lpFileOrPath,LARGE_INTEGER filesize);
	
	//注册表最后访问位置
	BOOL	ScanRegeditHistory();
	//浏览器注册表流
	BOOL	ScanRegeditStream();
	
	//google工具栏
	BOOL	ScanGoogleToolBar();
	BOOL	ScanBaiduToolBar();
	BOOL	ScanBaiduToolBar(LPCTSTR lpszRegPath);
	BOOL    ScanQQToolBar();
	BOOL    ScanWindowLiveToolBar();
	//Windows程序访问记录
	BOOL	ScanWindowsPrgHistory();
	//内存转储
	BOOL	ScanWindowsMemoryDMP();
	
	BOOL	CleanClipBoard();

	BOOL    ScanWindowsSearch();

	BOOL    ScanNethood();

	BOOL    ScanRecycle();

	//win7跳转列表
	BOOL    ScanWindows7JumpList();
	BOOL    ScanWinSuol();

private:
	CFileOpt				m_fileOpt;
	CRegOpt					m_regOpt;
	BOOL					m_bScan;
	CAppHistory				m_appHistory;
	TCHAR					m_szLoginName[MAX_PATH];
public:
	static BOOL WINAPI myEnumerateFile(LPCTSTR lpFileOrPath, void* pUserData,LARGE_INTEGER filesize);
	static BOOL WINAPI myEnumerateReg(HKEY rootKey,LPCTSTR lpItemPath,LPCTSTR lpName,DWORD dwValueNameSize,LPBYTE lpValue ,DWORD dwValueSize,DWORD dwType,void* pUserData);

};



#endif //_WINDOWSHISTORY_H_