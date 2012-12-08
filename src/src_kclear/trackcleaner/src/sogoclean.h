#ifndef _SOGOCLEAN_H_
#define _SOGOCLEAN_H_
#include <vector>
#include "CleanProc.h"
#include "i_clean.h"
#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"
#include "apphistory.h"
class CKSogoClean
{
public:
	CKSogoClean();
	~CKSogoClean();
public:
	BOOL ScanSogoHistory();
	BOOL ScanSogoCookies();
	BOOL ScanSogoForm();
	BOOL ScanSogoPass();
	BOOL ScanSogoAdvForm();

	BOOL CleanSogoHistory(std::wstring strInFile);
	BOOL CleanSogoCookies(std::wstring strInFile);
	BOOL CleanSogoForm();
	BOOL CleanSogoPass();
	BOOL CleanSogoAdvForm(std::wstring strInFile);
	void Start()
	{
		m_appHistory.Start();
		m_bScan =TRUE;
	}
	void StopScan()
	{
		m_appHistory.StopScan(); 
		m_bScan =FALSE;
	}
protected:
	BOOL ScanDbTable(std::wstring strFileName, std::wstring strTable, int nType);
	BOOL CleanDbTable(std::wstring strInFile);
	BOOL EnumUserInfo(int nType);
	BOOL GetUserInfo(std::vector<std::wstring>& vec_userInfo);
private:
	CAppHistory m_appHistory;
	BOOL		m_bScan;
	BOOL        m_bFlag[10000];
};
#endif