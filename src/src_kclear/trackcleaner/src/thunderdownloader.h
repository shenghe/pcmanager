#pragma once

#include "CleanProc.h"
#include "i_clean.h"
#include "sqlite3.h"
#include "kscbase/kscconv.h"
#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"
#include "apphistory.h"
#include <string>
extern ScanFileCallBack g_fnScanFile;
extern ScanRegCallBack  g_fnScanReg;
extern void*			g_pMain;
class CThunderDownloader
{
public:
	CThunderDownloader(void);
	~CThunderDownloader(void);
	void	Start(){m_appHistory.Start(); m_bScan =TRUE;};
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;};
public:
	BOOL ScanThunder();
	BOOL ModifySqlite(std::wstring strPath, std::wstring strFile);
	BOOL CleanThunder(LPCTSTR lpszFile);

	int SplitCString1(CString strIn, CSimpleArray<CString>& vec_String, TCHAR division);
private:
	CAppHistory m_appHistory;
	BOOL		m_bScan;
};
