#include "stdafx.h"
#include "kflowstat.h"
#include <time.h>
#include "common/utility.h"

#define		DEF_SQLITE_FILE		_T("flow.db3")

KFlowStat::KFlowStat()
{
	/*HRESULT hr = BKDbCreateObject(__uuidof(ISQLiteComDatabase3), (void**)&m_spiDB);
	if (!SUCCEEDED(hr))
	return;

	int nCmdLen = MAX_PATH * 2;
	wchar_t szFilePath[MAX_PATH * 2] = {0};
	DWORD dwCode = ::GetModuleFileName(NULL, szFilePath, nCmdLen);
	if (dwCode == 0)a
	return;

	::PathRemoveFileSpecW(szFilePath);
	::PathAppend(szFilePath, DEF_SQLITE_FILE);

	hr = m_spiDB->Open(szFilePath);
	if (!SUCCEEDED(hr))
	return;
	*/

	HRESULT hr = S_OK;

	hr = CAppPath::Instance().GetLeidianAppPath( m_modpath.m_strPath );
	if ( FAILED( hr ) )
		goto Exit0;


	hr = CAppPath::Instance().GetLeidianLogPath( m_dbpath.m_strPath );
	if ( FAILED( hr ) )
		goto Exit0;

	m_dbpath.Append( DEF_SQLITE_FILE );

	m_modpath.Append( BKMOD_NAME_BKDB );
/*	{
		int nCmdLen = MAX_PATH * 2;
		wchar_t szFilePath[MAX_PATH * 2] = {0};
		DWORD dwCode = ::GetModuleFileName(NULL, szFilePath, nCmdLen);
		::PathRemoveFileSpecW(szFilePath);
		::PathAppend(szFilePath, DEF_SQLITE_FILE);
		m_dbpath.m_strPath = szFilePath;
	}
*/	


	hr = m_dbmod.LoadLib( m_modpath );
	if ( FAILED( hr ) )
		goto Exit0;

	hr = m_dbmod.BKDbCreateObject( 
		__uuidof( Skylark::ISQLiteComDatabase3 ), 
		( void** )&m_piWlDB 
		);
	if ( FAILED( hr ) )
		goto Exit0;

	hr = m_piWlDB->Open( m_dbpath );
	if ( FAILED( hr ) )
	{
		::SetFileAttributes( m_dbpath, 0 );
		::DeleteFile( m_dbpath );
		hr = m_piWlDB->Open( m_dbpath );
	}

	if ( FAILED( hr ) )
		goto Exit1;

	goto Exit0;

Exit1:
	if ( m_piWlDB )
	{
		m_piWlDB->Release();
		m_piWlDB = NULL;
	}

Exit0:
	return ;
}

KFlowStat::~KFlowStat()
{
	HRESULT hr = S_OK;

	if ( m_piWlDB )
	{
		m_piWlDB->Close();
		m_piWlDB->Release();
		m_piWlDB = NULL;
	}

	m_dbmod.FreeLib();
Exit0:
	return ;
}

BOOL KFlowStat::InsertData( IN CString strTime, IN LONGLONG lUpAllFlow, IN LONGLONG lDownAllFlow, IN LONGLONG lAllFlow, IN LONGLONG lAllTime )
{
	if (!m_piWlDB->TableExists(L"flowlist"))
	{
		HRESULT hr = m_piWlDB->ExecuteUpdate(L"create table flowlist(time char(20), upflow int, downflow int, allflow int, alltime int, fyear int, fmonth int, fweek int);");
		if (!SUCCEEDED(hr))
			return FALSE;
	}

	CComPtr<ISQLiteComResultSet3> spiRs;
	CString strSql;
	strSql.Format(_T("select * from flowlist where time='%s';"), strTime);
	HRESULT hr = m_piWlDB->ExecuteQuery(strSql, &spiRs);
	if (!SUCCEEDED(hr) || spiRs->IsEof())
	{
		//insert 的时候，将年月周加入进去
		DWORD dwTime = _time32(NULL);
		dwTime = dwTime / (60 * 60 * 24) + 4;
		int nWeek = dwTime / 7;

		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);

		strSql.Format(_T("insert into flowlist values('%s', %ld, %ld, %ld, %ld, %ld, %ld, %ld)"), strTime, (long)(lUpAllFlow/1024), (long)(lDownAllFlow/1024), (long)(lAllFlow/1024), (long)(lAllTime), (long)(sysTime.wYear), (long)(sysTime.wYear * 100 + sysTime.wMonth), (long)nWeek);
		hr = m_piWlDB->ExecuteUpdate(strSql);
		if (!SUCCEEDED(hr))
			return FALSE;

		return TRUE;
	}
	else
	{
		//update
		strSql.Format(_T("UPDATE flowlist SET upflow = %ld, downflow=%ld, allflow=%ld, alltime=%ld WHERE time = '%s';"), (long)(lUpAllFlow/1024), (long)(lDownAllFlow/1024), (long)(lAllFlow/1024), (long)(lAllTime), strTime);
		hr = m_piWlDB->ExecuteUpdate(strSql);
		if (!SUCCEEDED(hr))
			return FALSE;

		return TRUE;
	}

	return FALSE;

}

BOOL KFlowStat::QueryData( IN CString strFrom, IN CString strTo, OUT CComPtr<ISQLiteComResultSet3>& spiRet )
{
	CString strSql;
	strSql.Format(L"select * from flowlist where time<='%s' and time>='%s' order by time desc;", strTo, strFrom);
	HRESULT hr = m_piWlDB->ExecuteQuery(strSql, &spiRet);

	if (!SUCCEEDED(hr))
		return FALSE;

	return TRUE;
}

BOOL KFlowStat::QueryData( IN enumQueryType enumFrom, OUT CComPtr<ISQLiteComResultSet3>& spiRet )
{
	CString strGroupBy;
	switch(enumFrom)
	{
	case enumQueryTypeEveryDay:
		strGroupBy = _T("time");
		break;
	case enumQueryTypeEveryWeek:
		strGroupBy = _T("fweek");
		break;
	case enumQueryTypeEveryMonth:
		strGroupBy = _T("fmonth");
		break;
	default :
		break;
	}

	CString strSql;
	strSql.Format(L"select min(time) as mintime, max(time) as maxtime,sum(upflow) as sumupflow,sum(downflow) as sumdownflow,sum(allflow) as sumallflow,sum(alltime) as sumalltime from flowlist group by %s order by time desc;", strGroupBy);
	HRESULT hr = m_piWlDB->ExecuteQuery(strSql, &spiRet);

	if (!SUCCEEDED(hr))
		return FALSE;

	return TRUE;
}

BOOL KFlowStat::QueryData( IN enumQueryType enumFrom, IN CString strCurTime, OUT CComPtr<ISQLiteComResultSet3>& spiRet )
{
	CString strGroupBy;
	switch(enumFrom)
	{
	case enumQueryTypeEveryDay:
		strGroupBy = _T("time");
		break;
	case enumQueryTypeEveryWeek:
		strGroupBy = _T("fweek");
		break;
	case enumQueryTypeEveryMonth:
		strGroupBy = _T("fmonth");
		break;
	default :
		break;
	}

	CString strSql;
	strSql.Format(L"select min(time) as mintime, max(time) as maxtime,sum(upflow) as sumupflow,sum(downflow) as sumdownflow,sum(allflow) as sumallflow,sum(alltime) as sumalltime from flowlist where %s='%s';", strGroupBy, strCurTime);
	HRESULT hr = m_piWlDB->ExecuteQuery(strSql, &spiRet);

	if (!SUCCEEDED(hr))
		return FALSE;

	return TRUE;
}

BOOL KFlowStat::BeginTransaction( void )
{
	return m_piWlDB->BeginTransaction();
}

BOOL KFlowStat::EndTransaction( void )
{
	return m_piWlDB->CommitTransaction();
}