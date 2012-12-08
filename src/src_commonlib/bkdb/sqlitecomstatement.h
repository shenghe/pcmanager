/**
* @file    sqlitecomstatement.h
* @brief   ...
* @author  zhangrui
* @date    2008-11-13  16:54
*/

#ifndef SQLITECOMSTATEMENT_H
#define SQLITECOMSTATEMENT_H

#include "skylark2\bkdb.h"

NS_SKYLARK_BEGIN

class CSQLiteComStatement: public ISQLiteComStatement3
{
public:
    virtual HRESULT     STDMETHODCALLTYPE ExecuteUpdate(int* pnRows);

    virtual HRESULT     STDMETHODCALLTYPE ExecuteQuery(ISQLiteComResultSet3** ppiResultSet);

    virtual int         STDMETHODCALLTYPE ExecuteScalar();



    virtual HRESULT     STDMETHODCALLTYPE Bind(int paramIndex, LPCWSTR szValue);

    virtual HRESULT     STDMETHODCALLTYPE Bind(int paramIndex, int intValue);

    virtual HRESULT     STDMETHODCALLTYPE Bind(int paramIndex, ULONGLONG int64Value);

    /// bind utf-8 value
    virtual HRESULT     STDMETHODCALLTYPE Bind(int paramIndex, LPCSTR szValue);

    virtual HRESULT     STDMETHODCALLTYPE BindBlob(int paramIndex, const char* blobValue, int blobLen);

    virtual HRESULT     STDMETHODCALLTYPE BindNull(int paramIndex);

    virtual HRESULT     STDMETHODCALLTYPE BindZeroBlob(int paramIndex, int blobSize);

    virtual int         STDMETHODCALLTYPE GetParamCount();

    virtual int         STDMETHODCALLTYPE GetParamIndex(LPCWSTR szParam);


    virtual void        STDMETHODCALLTYPE Reset();




public:
    CSQLiteComStatement();

    virtual ~CSQLiteComStatement();

    void AttachStmt(sqlite3* db, sqlite3_stmt* stmt);

    void Finalize();


public:
    SCOM_BEGIN_COM_MAP(CSQLiteComStatement)
        SCOM_INTERFACE_ENTRY(ISQLiteComStatement3)
    SCOM_END_COM_MAP()

private:

    sqlite3*        m_db;   ///< associated SQLite3 database
    sqlite3_stmt*   m_stmt; ///< associated SQLite3 statement
};

NS_SKYLARK_END

#endif//SQLITECOMSTATEMENT_H