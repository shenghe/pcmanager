/**
* @file    sqlitecomresultset.h
* @brief   ...
* @author  zhangrui
* @date    2008-11-13  16:54
*/

#ifndef SQLITECOMRESULTSET_H
#define SQLITECOMRESULTSET_H

#include "skylark2\bkdb.h"

NS_SKYLARK_BEGIN

class CSQLiteComResultSet: public ISQLiteComResultSet3
{
public:
    virtual int         STDMETHODCALLTYPE GetColumnCount();

    virtual int         STDMETHODCALLTYPE FindColumnIndex(LPCWSTR columnName);

    virtual LPCWSTR     STDMETHODCALLTYPE GetColumnName(int columnIndex);

    virtual LPCWSTR     STDMETHODCALLTYPE GetDeclaredColumnType(int columnIndex);

    virtual int         STDMETHODCALLTYPE GetColumnType(int columnIndex);




    virtual LPCWSTR     STDMETHODCALLTYPE GetAsString(int       columnIndex,    LPCWSTR nullValue = L"");
    virtual LPCWSTR     STDMETHODCALLTYPE GetAsString(LPCWSTR   columnName,     LPCWSTR nullValue = L"");

    virtual int         STDMETHODCALLTYPE GetInt(int            columnIndex,    int nullValue = 0);
    virtual int         STDMETHODCALLTYPE GetInt(LPCWSTR        columnName,     int nullValue = 0);

    virtual ULONGLONG   STDMETHODCALLTYPE GetInt64(int          columnIndex,    ULONGLONG nullValue = 0);
    virtual ULONGLONG   STDMETHODCALLTYPE GetInt64(LPCWSTR      columnName,     ULONGLONG nullValue = 0);

    virtual int         STDMETHODCALLTYPE GetBlob(int           columnIndex, char* pBlog, int nBlogBytes);
    virtual int         STDMETHODCALLTYPE GetBlob(LPCWSTR       columnName,  char* pBlog, int nBlogBytes);

    virtual int         STDMETHODCALLTYPE GetBlobBytes(int      columnIndex);
    virtual int         STDMETHODCALLTYPE GetBlobBytes(LPCWSTR  columnName);

    virtual BOOL        STDMETHODCALLTYPE IsNull(int            columnIndex);
    virtual BOOL        STDMETHODCALLTYPE IsNull(LPCWSTR        columnName);

    virtual BOOL        STDMETHODCALLTYPE IsEof();

    virtual BOOL        STDMETHODCALLTYPE NextRow();

    virtual void        STDMETHODCALLTYPE Finalize() throw();


public:
    CSQLiteComResultSet();

    virtual ~CSQLiteComResultSet();

    void AttachResultSet(
        sqlite3*                db,
        sqlite3_stmt*           stmt,
        BOOL                    eof,
        BOOL                    first,
        ISQLiteComStatement3*   piStmt);

public:
    SCOM_BEGIN_COM_MAP(CSQLiteComResultSet)
        SCOM_INTERFACE_ENTRY(ISQLiteComResultSet3)
    SCOM_END_COM_MAP()



private:

    void CheckDatabase();
    void CheckStmt();


    sqlite3*        m_db;       ///< associated SQLite3 database
    sqlite3_stmt*   m_stmt;     ///< associated statement
    BOOL            m_eof;      ///< Flag for end of result set
    BOOL            m_first;    ///< Flag for first row of the result set
    int             m_cols;     ///< Number of columns in row set

    CComPtr<ISQLiteComStatement3> m_spiStmt;
};

NS_SKYLARK_END

#endif//SQLITECOMRESULTSET_H