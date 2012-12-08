#ifndef _SPECIALAPP_H_
#define _SPECIALAPP_H_

#include "CleanProc.h"
#include "i_clean.h"

#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"
#include "regclean.h"
#include "apphistory.h"
#include <string>
#include "sqlite3.h"
#include "kscbase/kscconv.h"

/************************************************************************/
//功能:特殊应用软件清理
//
/************************************************************************/

extern ScanFileCallBack g_fnScanFile;
extern ScanRegCallBack  g_fnScanReg;
extern ScanFileCallBackError g_fnScanFileError;
extern ScanRegCallBackError  g_fnScanRegError;
extern void*			g_pMain;
extern CRegClean		g_regClean;


class  CSpecialApp
{
public:
	CSpecialApp(){m_bScan =TRUE;};
	~CSpecialApp(){};
public:
	void Start()
	{
		m_bScan = TRUE;
		m_regOpt.UserBreadFlag(FALSE);
		m_fileOpt.UserBreadFlag(FALSE);
	}

	void StopScan()
	{
		m_bScan = FALSE;
		m_regOpt.UserBreadFlag(TRUE);
		m_fileOpt.UserBreadFlag(TRUE);
	}


	BOOL	CleanMyIE2();
	BOOL	CleanMyIE2_Clean(int iType,LPCTSTR lpcszAppPath);
	//扫描内容
	BOOL	CleanKuGoo(int iType,LPCTSTR lpcszAppPath);
	BOOL    CleanKugooEx(CString strFileName, CString strTable);
	BOOL    CleanKuGoo_Clean(int iType,LPCTSTR lpcszAppPath);
	
	BOOL	CleanKMPlayer(int Itype,LPCTSTR lpcszAppPath,LPCTSTR lpcszValue);

	BOOL	CleanStorm(int Itype,LPCTSTR lpcszAppPath);
	BOOL	CleanStorm_Clean(int Itype,LPCTSTR lpcszAppPath);

	BOOL	CleanQQLive(int iType,LPCTSTR lpcszAppPath);

	//一键清理
	BOOL	CleanUltraEdit(int iType,LPCTSTR lpcszAppPath);
	//扫描内容
	BOOL	CleanUltraEdit_Scan(int iType,LPCTSTR lpcszAppPath);
	BOOL	ScanSection(int iType,LPCTSTR lpcszAppPath,LPCTSTR lpcszIniPath);
	//扫描清理
	BOOL	CleanUltraEdit_Clean(int iType,LPCTSTR lpcszPath);					
	int		SplitCString1(CString strIn, CSimpleArray<CString>& vec_String, TCHAR division);

	//迅雷7搜索记录
	BOOL    ScanXunleiSearch(int iType);
	BOOL    CleanXunleiSearch(LPCTSTR lpcszAppPath);

	BOOL    ModifyTxtData(int nType, const CString strFileName);
private:
	CRegOpt					m_regOpt;
	CFileOpt				m_fileOpt;
	BOOL					m_bScan;
	CAppHistory				m_appHistory;
};







#endif
