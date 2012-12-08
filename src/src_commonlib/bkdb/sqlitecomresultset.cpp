/**
* @file    sqlitecomresultset.cpp
* @brief   ...
* @author  zhangrui
* @date    2008-11-13  18:38
*/

#include "stdafx.h"
#include "sqlitecomresultset.h"

NS_SKYLARK_USING

CSQLiteComResultSet::CSQLiteComResultSet():
    m_db(NULL),
    m_stmt(NULL),
    m_eof(TRUE),
    m_first(TRUE),
    m_cols(NULL)
{

}

CSQLiteComResultSet::~CSQLiteComResultSet()
{
    Finalize();
}

void CSQLiteComResultSet::AttachResultSet(
    sqlite3*                db,
    sqlite3_stmt*           stmt,
    BOOL                    eof,
    BOOL                    first,
    ISQLiteComStatement3*   piStmt)
{
    m_db        = db;
    m_stmt      = stmt;
    m_eof       = eof;
    m_first     = first;
    m_cols      = sqlite3_column_count(m_stmt);
    m_spiStmt   = piStmt;
}


int STDMETHODCALLTYPE CSQLiteComResultSet::GetColumnCount()
{
    if (!m_db || !m_stmt)
        return 0;

    return m_cols;
}

int STDMETHODCALLTYPE CSQLiteComResultSet::FindColumnIndex(LPCWSTR columnName)
{
    if (!m_db || !m_stmt)
        return 0;

    CW2A conv(columnName, CP_UTF8);
    if (*conv != '\0')
    {
        for (int columnIndex = 0; columnIndex < m_cols; columnIndex++)
        {
            const char* temp = sqlite3_column_name(m_stmt, columnIndex);

            if (StrCmpA(conv, temp) == 0)
                return columnIndex;
        }
    }

    return 0;
}

LPCWSTR STDMETHODCALLTYPE CSQLiteComResultSet::GetColumnName(int columnIndex)
{
    if (!m_db || !m_stmt)
        return NULL;

    if (columnIndex < 0 || columnIndex > m_cols-1)
        return NULL;

    return (LPCWSTR)sqlite3_column_name16(m_stmt, columnIndex);
}

LPCWSTR STDMETHODCALLTYPE CSQLiteComResultSet::GetDeclaredColumnType(int columnIndex)
{
    if (!m_db || !m_stmt)
        return NULL;

    if (columnIndex < 0 || columnIndex > m_cols-1)
        return NULL;
 
    return (LPCWSTR)sqlite3_column_decltype16(m_stmt, columnIndex);
}

int STDMETHODCALLTYPE CSQLiteComResultSet::GetColumnType(int columnIndex)
{
    if (!m_db || !m_stmt)
        return -1;

    if (columnIndex < 0 || columnIndex > m_cols-1)
        return -1;

    return sqlite3_column_type(m_stmt, columnIndex);
}




LPCWSTR STDMETHODCALLTYPE CSQLiteComResultSet::GetAsString(int columnIndex, LPCWSTR nullValue)
{
    if (!m_db || !m_stmt)
        return nullValue;

    if (GetColumnType(columnIndex) == SQLITE_NULL)
        return nullValue;
    
    // sqlite3_column_bytes16() force the result into the desired format
    sqlite3_column_bytes16(m_stmt, columnIndex);
    LPCWSTR localValue = (LPCWSTR)sqlite3_column_text16(m_stmt, columnIndex);
    return localValue ? localValue : nullValue;
}

LPCWSTR STDMETHODCALLTYPE CSQLiteComResultSet::GetAsString(LPCWSTR columnName, LPCWSTR nullValue)
{
    int columnIndex = FindColumnIndex(columnName);
    if (-1 == columnIndex)
        return nullValue;

    return GetAsString(columnIndex, nullValue);
}

int STDMETHODCALLTYPE CSQLiteComResultSet::GetInt(int columnIndex, int nullValue)
{
    if (!m_db || !m_stmt)
        return nullValue;

    if (GetColumnType(columnIndex) == SQLITE_NULL)
        return nullValue;

    return sqlite3_column_int(m_stmt, columnIndex);
}

int STDMETHODCALLTYPE CSQLiteComResultSet::GetInt(LPCWSTR columnName, int nullValue)
{
    int columnIndex = FindColumnIndex(columnName);
    if (-1 == columnIndex)
        return nullValue;

    return GetInt(columnIndex, nullValue);
}

ULONGLONG STDMETHODCALLTYPE CSQLiteComResultSet::GetInt64(int columnIndex, ULONGLONG nullValue)
{
    if (!m_db || !m_stmt)
        return nullValue;

    if (GetColumnType(columnIndex) == SQLITE_NULL)
        return nullValue;

    return sqlite3_column_int64(m_stmt, columnIndex);
}

ULONGLONG STDMETHODCALLTYPE CSQLiteComResultSet::GetInt64(LPCWSTR columnName, ULONGLONG nullValue)
{
    int columnIndex = FindColumnIndex(columnName);
    if (-1 == columnIndex)
        return nullValue;

    return GetInt64(columnIndex, nullValue);
}


int STDMETHODCALLTYPE CSQLiteComResultSet::GetBlob(int columnIndex, char* pBlog, int nBlogBytes)
{
    if (!m_db || !m_stmt)
        return 0;

    if (GetColumnType(columnIndex) == SQLITE_NULL)
        return 0;

    // sqlite3_column_bytes() force the result into the desired format
    int nColumnBytes = sqlite3_column_bytes(m_stmt, columnIndex);
    int nCopyBytes   = min(nBlogBytes, nColumnBytes);
    memcpy(pBlog, sqlite3_column_blob(m_stmt, columnIndex), nCopyBytes);
    return nCopyBytes;
}

int STDMETHODCALLTYPE CSQLiteComResultSet::GetBlob(LPCWSTR columnName, char* pBlog, int nBlogBytes)
{
    int columnIndex = FindColumnIndex(columnName);
    if (-1 == columnIndex)
        return 0;

    return GetBlob(columnIndex, pBlog, nBlogBytes);
}

int STDMETHODCALLTYPE CSQLiteComResultSet::GetBlobBytes(int columnIndex)
{
    if (!m_db || !m_stmt)
        return 0;

    if (GetColumnType(columnIndex) == SQLITE_NULL)
        return 0;

    return sqlite3_column_bytes(m_stmt, columnIndex);
}

int STDMETHODCALLTYPE CSQLiteComResultSet::GetBlobBytes(LPCWSTR columnName)
{
    int columnIndex = FindColumnIndex(columnName);
    if (-1 == columnIndex)
        return 0;

    return GetBlobBytes(columnIndex);
}

BOOL STDMETHODCALLTYPE CSQLiteComResultSet::IsNull(int columnIndex)
{
    if (!m_db || !m_stmt)
        return TRUE;

    return (GetColumnType(columnIndex) == SQLITE_NULL);
}

BOOL STDMETHODCALLTYPE CSQLiteComResultSet::IsNull(LPCWSTR columnName)
{
    int columnIndex = FindColumnIndex(columnName);
    if (-1 == columnIndex)
        return 0;

    return (GetColumnType(columnIndex) == SQLITE_NULL);
}

BOOL STDMETHODCALLTYPE CSQLiteComResultSet::IsEof()
{
    if (!m_db || !m_stmt)
        return TRUE;

    return m_eof;
}

BOOL STDMETHODCALLTYPE CSQLiteComResultSet::NextRow()
{
    if (!m_db || !m_stmt)
        return FALSE;

    if (m_eof)
        return FALSE;

    int rc = sqlite3_step(m_stmt);
    if (rc == SQLITE_DONE) // no more rows
    {
        m_eof = true;
        return FALSE;
    }
    else if (rc == SQLITE_ROW) // more rows
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return S_OK;
}

void CSQLiteComResultSet::Finalize()
{
    m_stmt  = NULL;
    m_db    = NULL;
    m_spiStmt.Release();
}

