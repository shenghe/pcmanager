/**
* @file    sqlitecomstatement.cpp
* @brief   ...
* @author  zhangrui
* @date    2008-11-13  17:53
*/

#include "stdafx.h"
#include "sqlitecomstatement.h"

#include "skylark2\bkdb.h"
#include "sqlitecomresultset.h"

NS_SKYLARK_USING

CSQLiteComStatement::CSQLiteComStatement():
    m_db(NULL),
    m_stmt(NULL)
{
}

CSQLiteComStatement::~CSQLiteComStatement()
{
    Finalize();
}

void CSQLiteComStatement::AttachStmt(sqlite3* db, sqlite3_stmt* stmt)
{
    Finalize();

    m_db    = db;
    m_stmt  = stmt;
}

HRESULT STDMETHODCALLTYPE CSQLiteComStatement::ExecuteUpdate(int* pnRows)
{
    if (!m_db || !m_stmt)
        return E_POINTER;


    int rc = sqlite3_step(m_stmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_reset(m_stmt);
        return MAKE_SKYLARK_SQLITE_ERROR(rc);
    }

    int rowsChanged = sqlite3_changes(m_db);
    rc = sqlite3_reset(m_stmt);
    if (rc != SQLITE_OK)
    {
        return MAKE_SKYLARK_SQLITE_ERROR(rc);
    }

    if (pnRows)
        *pnRows = rowsChanged;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CSQLiteComStatement::ExecuteQuery(ISQLiteComResultSet3** ppiResultSet)
{
    assert(ppiResultSet && !*ppiResultSet);
    if (!ppiResultSet || *ppiResultSet)
        return E_POINTER;


    if (!m_db || !m_stmt)
        return E_HANDLE;


    BOOL bEof = TRUE;
    int  rc   = sqlite3_step(m_stmt);
    if (rc == SQLITE_DONE) // no rows
    {
        bEof = TRUE;
    }
    else if (rc == SQLITE_ROW) // one or more rows
    {
        bEof = FALSE;
    }
    else
    {
        return MAKE_SKYLARK_SQLITE_ERROR(rc);
    }


    CSQLiteComResultSet* pResultSet = new KComObject<CSQLiteComResultSet>;
    if(!pResultSet)
        return E_OUTOFMEMORY;


    BOOL bFirst   = TRUE;

    pResultSet->AttachResultSet(m_db, m_stmt, bEof, bFirst, this);
    pResultSet->AddRef();
    *ppiResultSet = pResultSet;
    return S_OK;
}


int STDMETHODCALLTYPE CSQLiteComStatement::ExecuteScalar()
{
    CComPtr<ISQLiteComResultSet3> spiResultSet;
    HRESULT hr = ExecuteQuery(&spiResultSet);
    if (FAILED(hr))
        return hr;


    if (spiResultSet->IsEof() || spiResultSet->GetColumnCount() < 1)
        return 0;


    return spiResultSet->GetInt(0);
}

HRESULT STDMETHODCALLTYPE CSQLiteComStatement::Bind(int paramIndex, LPCWSTR szValue)
{
    if (!m_db || !m_stmt)
        return E_HANDLE;

    if (!szValue)
        return E_POINTER;

    int rc = sqlite3_bind_text16(m_stmt, paramIndex, szValue, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK)
        return MAKE_SKYLARK_SQLITE_ERROR(rc);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSQLiteComStatement::Bind(int paramIndex, int intValue)
{
    if (!m_db || !m_stmt)
        return E_HANDLE;

    int rc = sqlite3_bind_int(m_stmt, paramIndex, intValue);
    if (rc != SQLITE_OK)
        return MAKE_SKYLARK_SQLITE_ERROR(rc);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSQLiteComStatement::Bind(int paramIndex, ULONGLONG int64Value)
{
    if (!m_db || !m_stmt)
        return E_HANDLE;

    int rc = sqlite3_bind_int64(m_stmt, paramIndex, int64Value);
    if (rc != SQLITE_OK)
        return MAKE_SKYLARK_SQLITE_ERROR(rc);

    return S_OK;
}

/// bind utf-8 value
HRESULT STDMETHODCALLTYPE CSQLiteComStatement::Bind(int paramIndex, LPCSTR szValue)
{
    if (!m_db || !m_stmt)
        return E_HANDLE;

    if (!szValue)
        return E_POINTER;

    int rc = sqlite3_bind_text(m_stmt, paramIndex, szValue, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK)
        return MAKE_SKYLARK_SQLITE_ERROR(rc);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSQLiteComStatement::BindBlob(int paramIndex, const char* blobValue, int blobLen)
{
    if (!m_db || !m_stmt)
        return E_HANDLE;

    if (!blobValue)
        return E_POINTER;

    int rc = sqlite3_bind_blob(m_stmt, paramIndex, blobValue, blobLen, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK)
        return MAKE_SKYLARK_SQLITE_ERROR(rc);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSQLiteComStatement::BindNull(int paramIndex)
{
    if (!m_db || !m_stmt)
        return E_HANDLE;

    int rc = sqlite3_bind_null(m_stmt, paramIndex);
    if (rc != SQLITE_OK)
        return MAKE_SKYLARK_SQLITE_ERROR(rc);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSQLiteComStatement::BindZeroBlob(int paramIndex, int blobSize)
{
    if (!m_db || !m_stmt)
        return E_HANDLE;

    int rc = sqlite3_bind_zeroblob(m_stmt, paramIndex, blobSize);
    if (rc != SQLITE_OK)
        return MAKE_SKYLARK_SQLITE_ERROR(rc);

    return S_OK;
}

int STDMETHODCALLTYPE CSQLiteComStatement::GetParamCount()
{
    if (!m_db || !m_stmt)
        return 0;

    return sqlite3_bind_parameter_count(m_stmt);
}

int STDMETHODCALLTYPE CSQLiteComStatement::GetParamIndex(LPCWSTR szParam)
{
    if (!m_db || !m_stmt)
        return -1;

    return sqlite3_bind_parameter_index(m_stmt, CW2A(szParam, CP_UTF8));
}


void STDMETHODCALLTYPE CSQLiteComStatement::Reset()
{
    if (!m_db || !m_stmt)
        return;

    sqlite3_reset(m_stmt);
}





void CSQLiteComStatement::Finalize()
{
    if (m_stmt)
    {
        sqlite3_finalize(m_stmt);
        m_stmt  = NULL;
    }

    m_db = NULL;
}

