/**
* @file    sqlitecomdatabase.cpp
* @brief   ...
* @author  zhangrui
* @date    2008-11-13  17:09
*/

#include "stdafx.h"
#include "sqlitecomdatabase.h"

#include "sqlitecomstatement.h"
#include "sqlitecomresultset.h"

NS_SKYLARK_USING

CSQLiteComDatabase::CSQLiteComDatabase():
    m_db(NULL)
{
    m_busyTimeoutMs = 60000; // 60 seconds
}

CSQLiteComDatabase::~CSQLiteComDatabase()
{
    Close();
}



HRESULT STDMETHODCALLTYPE CSQLiteComDatabase::Open(LPCWSTR szFile)
{
    if (!szFile)
        return E_POINTER;

    int rc = sqlite3_open(CW2A(szFile, CP_UTF8), &m_db);
    if (rc != SQLITE_OK)
    {
        Close();
        return MAKE_SKYLARK_SQLITE_ERROR(rc);
    }

    rc = sqlite3_extended_result_codes(m_db, 1);
    if (rc != SQLITE_OK)
    {
        Close();
        return MAKE_SKYLARK_SQLITE_ERROR(rc);
    }

    SetBusyTimeout(m_busyTimeoutMs);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSQLiteComDatabase::OpenWithKey(
    LPCWSTR pszFile,
    void*   pPassword,
    DWORD   dwPasswordBytes)
{
    if (!pszFile || !pPassword)
        return E_POINTER;

    int rc = sqlite3_open(CW2A(pszFile, CP_UTF8), &m_db);
    if (rc != SQLITE_OK)
    {
        Close();
        return MAKE_SKYLARK_SQLITE_ERROR(rc);
    }

    rc = sqlite3_key(m_db, pPassword, dwPasswordBytes);
    if (rc != SQLITE_OK)
    {
        Close();
        return MAKE_SKYLARK_SQLITE_ERROR(rc);
    }

    rc = sqlite3_extended_result_codes(m_db, 1);
    if (rc != SQLITE_OK)
    {
        Close();
        return MAKE_SKYLARK_SQLITE_ERROR(rc);
    }

    SetBusyTimeout(m_busyTimeoutMs);
    return S_OK;
}

BOOL STDMETHODCALLTYPE CSQLiteComDatabase::IsOpen()
{
    return (m_db != NULL);
}

void STDMETHODCALLTYPE CSQLiteComDatabase::Close()
{
    if (m_db)
    {
        sqlite3_close(m_db);
        m_db = NULL;
    }
}

HRESULT STDMETHODCALLTYPE CSQLiteComDatabase::BeginTransaction(BKDb::EM_TRANSLOCK transLock)
{
    switch (transLock)
    {
    case BKDb::em_Deferred:
        return ExecuteUpdate(L"begin deferred transaction;", NULL);
    case BKDb::em_Immediate:
        return ExecuteUpdate(L"begin immediate transaction;", NULL);
    case BKDb::em_Exclusive:
        return ExecuteUpdate(L"begin exclusive transaction;", NULL);
    //default:
    }

    assert(!"unknown transaction behavior");
    return ExecuteUpdate(L"begin transaction;", NULL);
}

HRESULT STDMETHODCALLTYPE CSQLiteComDatabase::CommitTransaction()
{
    return ExecuteUpdate(L"commit transaction;", NULL);
}

HRESULT STDMETHODCALLTYPE CSQLiteComDatabase::RollbackTransaction()
{
    return ExecuteUpdate(L"rollback transaction;", NULL);
}



HRESULT STDMETHODCALLTYPE CSQLiteComDatabase::CreateSavePoint(LPCWSTR lpszPointName)
{
    if (!lpszPointName || !*lpszPointName)
        return E_INVALIDARG;

    CString strSql = L"savepoint ";
    strSql.Append(lpszPointName);
    return ExecuteUpdate(strSql, NULL);
}

HRESULT STDMETHODCALLTYPE CSQLiteComDatabase::ReleaseSavePoint(LPCWSTR lpszPointName) throw()
{
    if (!lpszPointName || !*lpszPointName)
        return E_INVALIDARG;

    CString strSql = L"release savepoint ";
    strSql.Append(lpszPointName);
    return ExecuteUpdate(strSql, NULL);
}

HRESULT STDMETHODCALLTYPE CSQLiteComDatabase::RollbackToSavePoint(LPCWSTR lpszPointName)
{
    if (!lpszPointName || !*lpszPointName)
        return E_INVALIDARG;

    CString strSql = L"rollback to savepoint  ";
    strSql.Append(lpszPointName);
    return ExecuteUpdate(strSql, NULL);
}




BOOL STDMETHODCALLTYPE CSQLiteComDatabase::GetAutoCommit()
{
    if (!m_db)
        return E_HANDLE;

    return sqlite3_get_autocommit(m_db) != 0;
}



BOOL STDMETHODCALLTYPE CSQLiteComDatabase::TableExists(LPCWSTR szTable)
{
    assert(szTable);
    if (!szTable)
        return FALSE;

    CComPtr<ISQLiteComStatement3> spiStmt;
    HRESULT hr = PrepareStatement(L"select count(*) from sqlite_master where type='table' and name like ?;", &spiStmt);
    if (FAILED(hr))
        return FALSE;

    spiStmt->Bind(1, szTable);
    return (0 != spiStmt->ExecuteScalar());
}


HRESULT STDMETHODCALLTYPE CSQLiteComDatabase::PrepareStatement(
    LPCWSTR szSQL, ISQLiteComStatement3** ppiStmt)
{
    HRESULT hr = DoPrepareStatement(szSQL, ppiStmt);
    if (FAILED(hr))
    {
        BKDebugOutput(BKDBG_LVL_ERROR, L"[bkdb] PrepareStatement error: %s", GetErrorMsg());
        return hr;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSQLiteComDatabase::ExecuteUpdate(LPCWSTR szSQL, int* pnRows)
{
    CComPtr<ISQLiteComStatement3> spiStmt;
    HRESULT hr = PrepareStatement(szSQL, &spiStmt);
    if (FAILED(hr))
        return hr;


    return spiStmt->ExecuteUpdate(pnRows);
}

HRESULT STDMETHODCALLTYPE CSQLiteComDatabase::ExecuteQuery(
    LPCWSTR szSQL, ISQLiteComResultSet3** ppiResultSet)
{
    CComPtr<ISQLiteComStatement3> spiStmt;
    HRESULT hr = PrepareStatement(szSQL, &spiStmt);
    if (FAILED(hr))
        return hr;


    return spiStmt->ExecuteQuery(ppiResultSet);
}

int STDMETHODCALLTYPE CSQLiteComDatabase::ExecuteScalar(LPCWSTR szSQL)
{
    CComPtr<ISQLiteComStatement3> spiStmt;
    HRESULT hr = PrepareStatement(szSQL, &spiStmt);
    if (FAILED(hr))
        return hr;

    return spiStmt->ExecuteScalar();
}


ULONGLONG STDMETHODCALLTYPE CSQLiteComDatabase::LastRowId()
{
    if (!m_db)
        return 0;

    return sqlite3_last_insert_rowid(m_db);
}

void STDMETHODCALLTYPE CSQLiteComDatabase::Interrupt()
{
    if (!m_db)
        return;

    sqlite3_interrupt(m_db);
}

void STDMETHODCALLTYPE CSQLiteComDatabase::SetBusyTimeout(int nMillisecs)
{
    if (!m_db)
        return;

    m_busyTimeoutMs = nMillisecs;
    sqlite3_busy_timeout(m_db, m_busyTimeoutMs);
}

LPCSTR STDMETHODCALLTYPE CSQLiteComDatabase::SQLiteVersion()
{
    return SQLITE_VERSION;
}

LPCWSTR STDMETHODCALLTYPE CSQLiteComDatabase::GetErrorMsg()
{
    if (!m_db)
        return L"";

    return (LPCWSTR)sqlite3_errmsg16(m_db);
}

int STDMETHODCALLTYPE CSQLiteComDatabase::GetErrorCode()
{
    if (!m_db)
        return SQLITE_OK;

    return sqlite3_errcode(m_db);
}


HRESULT CSQLiteComDatabase::DoPrepareStatement(LPCWSTR szSql, ISQLiteComStatement3** ppiStmt)
{
    assert(szSql && ppiStmt && !*ppiStmt);
    if (!szSql || !ppiStmt || *ppiStmt)
        return E_POINTER;


    if (!m_db)
        return E_HANDLE;



    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare16_v2(m_db, szSql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return MAKE_SKYLARK_SQLITE_ERROR(rc);



    CSQLiteComStatement* pStmtObj = new KComObject<CSQLiteComStatement>;
    if(!pStmtObj)
    {
        sqlite3_finalize(stmt);
        return E_OUTOFMEMORY;
    }


    pStmtObj->AttachStmt(m_db, stmt);
    pStmtObj->AddRef();
    *ppiStmt = pStmtObj;
    return S_OK;
}
