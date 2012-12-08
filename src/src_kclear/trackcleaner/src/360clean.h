#ifndef	 _360CLEAN_H_
#define  _360CLEAN_H_

#include "CleanProc.h"
#include "i_clean.h"

#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"
#include "apphistory.h"

/************************************************************************/
//¹¦ÄÜ:360
//
/************************************************************************/

extern ScanFileCallBack g_fnScanFile;
extern ScanRegCallBack  g_fnScanReg;
extern void*			g_pMain;


class C360Clean
{
public:
	C360Clean();
	~C360Clean();
	void Start()
	{
		m_appHistory.Start();
		m_bScan =TRUE;
	}
	void	StopScan(){m_appHistory.StopScan(); m_bScan =FALSE;};
public:
	BOOL	Scan360Clean();
	BOOL	Scan360User(int nType);
	BOOL    Clean360User(CString str);
private:
	CAppHistory m_appHistory;
	BOOL		m_bScan;
	BOOL        m_bFlag[10000];
};

#endif