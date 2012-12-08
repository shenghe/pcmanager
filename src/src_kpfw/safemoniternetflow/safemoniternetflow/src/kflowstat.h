#pragma once

#include "stdafx.h"
#include "skylark2\bkdb.h"
#include "com_s\com\scom.h"
#include "skylark2\bkdb_mod.h"
#include "winmod\winpath.h"

using namespace Skylark;

enum enumQueryType
{
	enumQueryTypeEveryDay	= 1,
	enumQueryTypeEveryWeek	= 2,
	enumQueryTypeEveryMonth = 3,
	enumQueryTypeDefineSelf	= 4
};

class KFlowStat
{
public:
	KFlowStat() ;
	~KFlowStat() ;

public:
	BOOL BeginTransaction(void);
	BOOL EndTransaction(void);
	BOOL InsertData(IN CString strTime, IN LONGLONG lUpAllFlow, IN LONGLONG lDownAllFlow, IN LONGLONG lAllFlow, IN LONGLONG lAllTime);
	BOOL QueryData(IN CString strFrom, IN CString strTo, OUT CComPtr<ISQLiteComResultSet3>& spiRet);
	BOOL QueryData(IN enumQueryType enumFrom, OUT CComPtr<ISQLiteComResultSet3>& spiRet);
	BOOL QueryData( IN enumQueryType enumFrom, IN CString strCurTime, OUT CComPtr<ISQLiteComResultSet3>& spiRet );

private:
	//CComPtr<ISQLiteComDatabase3> m_spiDB;

	WinMod::CWinPath    m_modpath;
	WinMod::CWinPath    m_dbpath;
	Skylark::CWinModule_bkdb        m_dbmod;
	Skylark::ISQLiteComDatabase3*   m_piWlDB;
};