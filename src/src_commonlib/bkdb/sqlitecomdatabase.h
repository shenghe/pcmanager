/**
* @file    sqlitecomdatabase.h
* @brief   ...
* @author  zhangrui
* @date    2008-11-13  16:54
*/

#ifndef SQLITECOMDATABASE_H
#define SQLITECOMDATABASE_H

#include "skylark2\bkdb.h"

NS_SKYLARK_BEGIN

class CSQLiteComDatabase:
    public ISQLiteComDatabase3
{
public:
    virtual HRESULT     STDMETHODCALLTYPE Open(LPCWSTR szFile);

    virtual HRESULT     STDMETHODCALLTYPE OpenWithKey(
        LPCWSTR pszFile,
        void*   pPassword,
        DWORD   dwPasswordBytes);

    virtual BOOL        STDMETHODCALLTYPE IsOpen() throw();

    virtual void        STDMETHODCALLTYPE Close() throw();


    virtual HRESULT     STDMETHODCALLTYPE BeginTransaction(BKDb::EM_TRANSLOCK transLock);

    virtual HRESULT     STDMETHODCALLTYPE CommitTransaction();

    virtual HRESULT     STDMETHODCALLTYPE RollbackTransaction() throw();



    virtual HRESULT     STDMETHODCALLTYPE CreateSavePoint(LPCWSTR lpszPointName);

    virtual HRESULT     STDMETHODCALLTYPE ReleaseSavePoint(LPCWSTR lpszPointName) throw();

    virtual HRESULT     STDMETHODCALLTYPE RollbackToSavePoint(LPCWSTR lpszPointName);



    /// Autocommit is disabled by a BEGIN statement* and reenabled by the next COMMIT or ROLLBACK. 
    virtual BOOL        STDMETHODCALLTYPE GetAutoCommit();



    virtual BOOL        STDMETHODCALLTYPE TableExists(LPCWSTR szTable);



    virtual HRESULT     STDMETHODCALLTYPE PrepareStatement(LPCWSTR szSQL, ISQLiteComStatement3** ppiStmt);

    virtual HRESULT     STDMETHODCALLTYPE ExecuteUpdate(LPCWSTR szSQL, int* pnRows);

    virtual HRESULT     STDMETHODCALLTYPE ExecuteQuery(LPCWSTR szSQL, ISQLiteComResultSet3** ppiResultSet);

    virtual int         STDMETHODCALLTYPE ExecuteScalar(LPCWSTR szSQL);



    virtual ULONGLONG   STDMETHODCALLTYPE LastRowId();

    virtual void        STDMETHODCALLTYPE Interrupt();

    virtual void        STDMETHODCALLTYPE SetBusyTimeout(int nMillisecs);

    virtual LPCSTR      STDMETHODCALLTYPE SQLiteVersion() throw();

    virtual LPCWSTR     STDMETHODCALLTYPE GetErrorMsg();

    virtual int         STDMETHODCALLTYPE GetErrorCode();


public:
    CSQLiteComDatabase();

    virtual ~CSQLiteComDatabase();



public:
    SCOM_BEGIN_COM_MAP(CSQLiteComDatabase)
        SCOM_INTERFACE_ENTRY(ISQLiteComDatabase3)
    SCOM_END_COM_MAP()



private:


    HRESULT DoPrepareStatement(LPCWSTR szSql, ISQLiteComStatement3** ppiStmt);



    sqlite3*        m_db;               ///< associated SQLite3 database
    int             m_busyTimeoutMs;    ///< Timeout in milli seconds
};

NS_SKYLARK_END

#endif//SQLITECOMDATABASE_H