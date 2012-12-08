#ifndef	 _WORDWINCLEAN_H_
#define  _WORDWINCLEAN_H_

#include "CleanProc.h"
#include "i_clean.h"

#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"
#include "apphistory.h"
#include <vector>

/************************************************************************/
//功能:世界之窗清理
//
/************************************************************************/

extern ScanFileCallBack g_fnScanFile;
extern ScanRegCallBack  g_fnScanReg;
extern void*			g_pMain;


class CWordWinClean
{
public:
	CWordWinClean();
	~CWordWinClean();
	void	Start()
	{
		m_appHistory.Start();
		m_regOpt.UserBreadFlag(FALSE);
		m_bScan =TRUE;
	};
	void	StopScan()
	{
		m_appHistory.StopScan();
		m_regOpt.UserBreadFlag(TRUE);
		m_bScan =FALSE;
	};
public:
	BOOL	ScanWordWinClean();
	BOOL	ScanWordWinUser();
	BOOL    ScanWorldIni(CString strPath, CString strSection);
	BOOL    CleanWorldIni(CString strFullInfo);
	BOOL    CleanWordWinUser(CString str);
private:
	CAppHistory m_appHistory;
	CRegOpt		m_regOpt;
	BOOL		m_bScan;
};


#endif