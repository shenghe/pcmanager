#ifndef _OPERACLEAN_H_
#define _OPERACLEAN_H_
#include <vector>
#include "CleanProc.h"
#include "i_clean.h"
#include "winosver.h"
#include "fileopt.h"
#include "regopt.h"
#include "apphistory.h"

class CKOperaClean
{
public:
	CKOperaClean();
	~CKOperaClean();

	BOOL ScanOperaHistory();
	BOOL ScanOperaCookies();
	BOOL ScanOperaForm();

	BOOL CleanOperaHistory(std::wstring strInFile);
public:
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
	BOOL ScanTxtSection(int nType);
private:
	CAppHistory m_appHistory;
	BOOL		m_bScan;
};
#endif