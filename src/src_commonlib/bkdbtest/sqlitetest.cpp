/**
* @file    sqlitetest.cpp
* @brief   ...
* @author  zhangrui
* @date    2008-11-16  15:23
*/

#include "stdafx.h"
#include "sqlitetest.h"

#include <time.h>
#include "com_s\com\scom.h"
#include "skylark2\bkdb.h"

#pragma warning(disable: 4996)  // 'function': was declared deprecated

CPPUNIT_TEST_SUITE_REGISTRATION(CSQLiteTest);

#define SQLITE_TEST_FILE            L"sqlitetest.db"
#define SQLITE_TEST_FILE_WITH_CRYPT L"sqlitetestcrypt.db"
#define EXPECTED_SQLITE_VERSION     "3.6.13"

#define TEST_SQLITE_OK              0
#define TEST_SQLITE_INTERRUPT       9

#define SQLITE_TEST_PASSWORD        "{6D19D4CA-5CE4-4e67-ADCE-6274AF3EDCC0}"


using namespace Skylark;




void CSQLiteTest::testSQLiteSingleThread()
{
    ::DeleteFile(SQLITE_TEST_FILE);

    time_t tmStart, tmEnd;
    CString strSql;

    CComPtr<ISQLiteComDatabase3> spiDB;
    HRESULT hr = BKDbCreateObject(__uuidof(ISQLiteComDatabase3), (void**)&spiDB);
    CPPUNIT_ASSERT(SUCCEEDED(hr));


    CPPUNIT_ASSERT_STR_EQUAL(EXPECTED_SQLITE_VERSION, spiDB->SQLiteVersion());


    // Main thread: Opening DB
    hr = spiDB->Open(SQLITE_TEST_FILE);
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    CPPUNIT_ASSERT(!spiDB->TableExists(L"emp"));
    // Creating emp table
    hr = spiDB->ExecuteUpdate(L"create table emp(empno int, empname char(20));");
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    CPPUNIT_ASSERT(spiDB->TableExists(L"emp"));

    spiDB->Close();
    hr = spiDB->Open(SQLITE_TEST_FILE);
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    ////////////////////////////////////////////////////////////////////////////////
    // Execute some DML, and print number of rows affected by each one
    ////////////////////////////////////////////////////////////////////////////////
    // DML tests
    int nRows = 0;
    hr = spiDB->ExecuteUpdate(L"insert into emp values (7, 'David Beckham');", &nRows);
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    CPPUNIT_ASSERT_EQUAL(1, nRows);

    hr = spiDB->ExecuteUpdate(L"update emp set empname = 'Christiano Ronaldo' where empno = 7;", &nRows);
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    CPPUNIT_ASSERT_EQUAL(1, nRows);

    hr = spiDB->ExecuteUpdate(L"delete from emp where empno = 7;", &nRows);
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    CPPUNIT_ASSERT_EQUAL(1, nRows);

    spiDB->Close();
    hr = spiDB->Open(SQLITE_TEST_FILE);
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    ////////////////////////////////////////////////////////////////////////////////
    // Transaction Demo
    // The transaction could just as easily have been rolled back
    ////////////////////////////////////////////////////////////////////////////////
    int nRowsToCreate(5000);
    tmStart = time(0);
    hr = spiDB->BeginTransaction();
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    for (int i = 0; i < nRowsToCreate; i++)
    {
        strSql.Format(L"insert into emp values (%d, 'Empname%06d');", i, i);
        hr = spiDB->ExecuteUpdate(strSql);
        CPPUNIT_ASSERT(SUCCEEDED(hr));
    }

    hr = spiDB->CommitTransaction();
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    tmEnd = time(0);

    ////////////////////////////////////////////////////////////////////////////////
    // Demonstrate CppSQLiteDB::execScalar()
    ////////////////////////////////////////////////////////////////////////////////
    spiDB->Close();
    hr = spiDB->Open(SQLITE_TEST_FILE);
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    nRows = spiDB->ExecuteScalar(L"select count(*) from emp;");
    CPPUNIT_ASSERT_EQUAL(nRowsToCreate, nRows);

    ////////////////////////////////////////////////////////////////////////////////
    // Re-create emp table with auto-increment field
    ////////////////////////////////////////////////////////////////////////////////
    // Auto increment test
    hr = spiDB->ExecuteUpdate(L"drop table emp;");
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    hr = spiDB->ExecuteUpdate(L"create table emp(empno integer primary key, empname char(20));");
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    for (int i = 0; i < 5; i++)
    {
        strSql.Format(L"insert into emp (empname) values ('Empname%06d');", i+1);
        hr = spiDB->ExecuteUpdate(strSql);
        CPPUNIT_ASSERT(SUCCEEDED(hr));
        CPPUNIT_ASSERT_EQUAL((ULONGLONG)i+1, spiDB->LastRowId());
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Query data and also show results of inserts into auto-increment field
    ////////////////////////////////////////////////////////////////////////////////
    // Select statement test
    spiDB->Close();
    hr = spiDB->Open(SQLITE_TEST_FILE);
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    CComPtr<ISQLiteComResultSet3> spiRs;
    hr = spiDB->ExecuteQuery(L"select * from emp order by 1;", &spiRs);
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    CPPUNIT_ASSERT_EQUAL(2, spiRs->GetColumnCount());

    CPPUNIT_ASSERT_WSTR_EQUAL(L"empno",     spiRs->GetColumnName(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"integer",   spiRs->GetDeclaredColumnType(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"empname",   spiRs->GetColumnName(1));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"char(20)",  spiRs->GetDeclaredColumnType(1));

    CPPUNIT_ASSERT(!spiRs->IsEof());
    CPPUNIT_ASSERT_EQUAL(1, spiRs->GetInt(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"Empname000001", spiRs->GetAsString(1));
    spiRs->NextRow();
    CPPUNIT_ASSERT(!spiRs->IsEof());
    CPPUNIT_ASSERT_EQUAL(2, spiRs->GetInt(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"Empname000002", spiRs->GetAsString(1));
    spiRs->NextRow();
    CPPUNIT_ASSERT(!spiRs->IsEof());
    CPPUNIT_ASSERT_EQUAL(3, spiRs->GetInt(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"Empname000003", spiRs->GetAsString(1));
    spiRs->NextRow();
    CPPUNIT_ASSERT(!spiRs->IsEof());
    CPPUNIT_ASSERT_EQUAL(4, spiRs->GetInt(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"Empname000004", spiRs->GetAsString(1));
    spiRs->NextRow();
    CPPUNIT_ASSERT(!spiRs->IsEof());
    CPPUNIT_ASSERT_EQUAL(5, spiRs->GetInt(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"Empname000005", spiRs->GetAsString(1));
    spiRs->NextRow();
    CPPUNIT_ASSERT(spiRs->IsEof());

    spiDB->Close();
    hr = spiDB->Open(SQLITE_TEST_FILE);
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    ////////////////////////////////////////////////////////////////////////////////
    // Pre-compiled Statements Demo
    ////////////////////////////////////////////////////////////////////////////////
    // Transaction test
    hr = spiDB->ExecuteUpdate(L"drop table emp;");
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    hr = spiDB->ExecuteUpdate(L"create table emp(empno int, empname char(20));");
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    tmStart = time(0);
    hr = spiDB->BeginTransaction();
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    CComPtr<ISQLiteComStatement3> spiStmt;
    hr = spiDB->PrepareStatement(L"insert into emp values (?, ?);", &spiStmt);
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    for (int i = 0; i < nRowsToCreate; i++)
    {
        char buf[16];
        sprintf(buf, "EmpName%06d", i);
        hr = spiStmt->Bind(1, i);
        CPPUNIT_ASSERT(SUCCEEDED(hr));
        hr = spiStmt->Bind(2, buf);
        CPPUNIT_ASSERT(SUCCEEDED(hr));
        hr = spiStmt->ExecuteUpdate();
        CPPUNIT_ASSERT(SUCCEEDED(hr));
        spiStmt->Reset();
    }

    hr = spiDB->CommitTransaction();
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    tmEnd = time(0);

    nRows = spiDB->ExecuteScalar(L"select count(*) from emp;");
    CPPUNIT_ASSERT_EQUAL(nRowsToCreate, nRows);

    hr = spiDB->ExecuteUpdate(L"VACUUM");
    CPPUNIT_ASSERT(SUCCEEDED(hr));
}







void CSQLiteTest::testSQLiteSingleThreadWithCrypt()
{
    ::DeleteFile(SQLITE_TEST_FILE_WITH_CRYPT);

    time_t tmStart, tmEnd;
    CString strSql;

    CComPtr<ISQLiteComDatabase3> spiDB;
    HRESULT hr = BKDbCreateObject(__uuidof(ISQLiteComDatabase3), (void**)&spiDB);
    CPPUNIT_ASSERT(SUCCEEDED(hr));


    CPPUNIT_ASSERT_STR_EQUAL(EXPECTED_SQLITE_VERSION, spiDB->SQLiteVersion());


    // Main thread: Opening DB
    hr = spiDB->OpenWithKey(
        SQLITE_TEST_FILE_WITH_CRYPT,
        SQLITE_TEST_PASSWORD,
        (DWORD)strlen(SQLITE_TEST_PASSWORD));
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    CPPUNIT_ASSERT(!spiDB->TableExists(L"emp"));
    // Creating emp table
    hr = spiDB->ExecuteUpdate(L"create table emp(empno int, empname char(20));");
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    CPPUNIT_ASSERT(spiDB->TableExists(L"emp"));


    spiDB->Close();
    hr = spiDB->OpenWithKey(
        SQLITE_TEST_FILE_WITH_CRYPT,
        SQLITE_TEST_PASSWORD,
        (DWORD)strlen(SQLITE_TEST_PASSWORD));
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    ////////////////////////////////////////////////////////////////////////////////
    // Execute some DML, and print number of rows affected by each one
    ////////////////////////////////////////////////////////////////////////////////
    // DML tests
    int nRows = 0;
    hr = spiDB->ExecuteUpdate(L"insert into emp values (7, 'David Beckham');", &nRows);
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    CPPUNIT_ASSERT_EQUAL(1, nRows);

    hr = spiDB->ExecuteUpdate(L"update emp set empname = 'Christiano Ronaldo' where empno = 7;", &nRows);
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    CPPUNIT_ASSERT_EQUAL(1, nRows);

    hr = spiDB->ExecuteUpdate(L"delete from emp where empno = 7;", &nRows);
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    CPPUNIT_ASSERT_EQUAL(1, nRows);

    spiDB->Close();
    hr = spiDB->OpenWithKey(
        SQLITE_TEST_FILE_WITH_CRYPT,
        SQLITE_TEST_PASSWORD,
        (DWORD)strlen(SQLITE_TEST_PASSWORD));
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    ////////////////////////////////////////////////////////////////////////////////
    // Transaction Demo
    // The transaction could just as easily have been rolled back
    ////////////////////////////////////////////////////////////////////////////////
    int nRowsToCreate(5000);
    tmStart = time(0);
    hr = spiDB->BeginTransaction();
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    for (int i = 0; i < nRowsToCreate; i++)
    {
        strSql.Format(L"insert into emp values (%d, 'Empname%06d');", i, i);
        hr = spiDB->ExecuteUpdate(strSql);
        CPPUNIT_ASSERT(SUCCEEDED(hr));
    }

    hr = spiDB->CommitTransaction();
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    tmEnd = time(0);

    ////////////////////////////////////////////////////////////////////////////////
    // Demonstrate CppSQLiteDB::execScalar()
    ////////////////////////////////////////////////////////////////////////////////
    spiDB->Close();
    hr = spiDB->OpenWithKey(
        SQLITE_TEST_FILE_WITH_CRYPT,
        SQLITE_TEST_PASSWORD,
        (DWORD)strlen(SQLITE_TEST_PASSWORD));
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    nRows = spiDB->ExecuteScalar(L"select count(*) from emp;");
    CPPUNIT_ASSERT_EQUAL(nRowsToCreate, nRows);

    ////////////////////////////////////////////////////////////////////////////////
    // Re-create emp table with auto-increment field
    ////////////////////////////////////////////////////////////////////////////////
    // Auto increment test
    hr = spiDB->ExecuteUpdate(L"drop table emp;");
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    hr = spiDB->ExecuteUpdate(L"create table emp(empno integer primary key, empname char(20));");
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    for (int i = 0; i < 5; i++)
    {
        strSql.Format(L"insert into emp (empname) values ('Empname%06d');", i+1);
        hr = spiDB->ExecuteUpdate(strSql);
        CPPUNIT_ASSERT(SUCCEEDED(hr));
        CPPUNIT_ASSERT_EQUAL((ULONGLONG)i+1, spiDB->LastRowId());
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Query data and also show results of inserts into auto-increment field
    ////////////////////////////////////////////////////////////////////////////////
    // Select statement test
    spiDB->Close();
    hr = spiDB->OpenWithKey(
        SQLITE_TEST_FILE_WITH_CRYPT,
        SQLITE_TEST_PASSWORD,
        (DWORD)strlen(SQLITE_TEST_PASSWORD));
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    CComPtr<ISQLiteComResultSet3> spiRs;
    hr = spiDB->ExecuteQuery(L"select * from emp order by 1;", &spiRs);
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    CPPUNIT_ASSERT_EQUAL(2, spiRs->GetColumnCount());

    CPPUNIT_ASSERT_WSTR_EQUAL(L"empno",     spiRs->GetColumnName(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"integer",   spiRs->GetDeclaredColumnType(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"empname",   spiRs->GetColumnName(1));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"char(20)",  spiRs->GetDeclaredColumnType(1));

    CPPUNIT_ASSERT(!spiRs->IsEof());
    CPPUNIT_ASSERT_EQUAL(1, spiRs->GetInt(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"Empname000001", spiRs->GetAsString(1));
    spiRs->NextRow();
    CPPUNIT_ASSERT(!spiRs->IsEof());
    CPPUNIT_ASSERT_EQUAL(2, spiRs->GetInt(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"Empname000002", spiRs->GetAsString(1));
    spiRs->NextRow();
    CPPUNIT_ASSERT(!spiRs->IsEof());
    CPPUNIT_ASSERT_EQUAL(3, spiRs->GetInt(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"Empname000003", spiRs->GetAsString(1));
    spiRs->NextRow();
    CPPUNIT_ASSERT(!spiRs->IsEof());
    CPPUNIT_ASSERT_EQUAL(4, spiRs->GetInt(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"Empname000004", spiRs->GetAsString(1));
    spiRs->NextRow();
    CPPUNIT_ASSERT(!spiRs->IsEof());
    CPPUNIT_ASSERT_EQUAL(5, spiRs->GetInt(0));
    CPPUNIT_ASSERT_WSTR_EQUAL(L"Empname000005", spiRs->GetAsString(1));
    spiRs->NextRow();
    CPPUNIT_ASSERT(spiRs->IsEof());

    spiDB->Close();
    hr = spiDB->OpenWithKey(
        SQLITE_TEST_FILE_WITH_CRYPT,
        SQLITE_TEST_PASSWORD,
        (DWORD)strlen(SQLITE_TEST_PASSWORD));
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    ////////////////////////////////////////////////////////////////////////////////
    // Pre-compiled Statements Demo
    ////////////////////////////////////////////////////////////////////////////////
    // Transaction test
    hr = spiDB->ExecuteUpdate(L"drop table emp;");
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    hr = spiDB->ExecuteUpdate(L"create table emp(empno int, empname char(20));");
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    tmStart = time(0);
    hr = spiDB->BeginTransaction();
    CPPUNIT_ASSERT(SUCCEEDED(hr));

    CComPtr<ISQLiteComStatement3> spiStmt;
    hr = spiDB->PrepareStatement(L"insert into emp values (?, ?);", &spiStmt);
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    for (int i = 0; i < nRowsToCreate; i++)
    {
        char buf[16];
        sprintf(buf, "EmpName%06d", i);
        hr = spiStmt->Bind(1, i);
        CPPUNIT_ASSERT(SUCCEEDED(hr));
        hr = spiStmt->Bind(2, buf);
        CPPUNIT_ASSERT(SUCCEEDED(hr));
        hr = spiStmt->ExecuteUpdate();
        CPPUNIT_ASSERT(SUCCEEDED(hr));
        spiStmt->Reset();
    }

    hr = spiDB->CommitTransaction();
    CPPUNIT_ASSERT(SUCCEEDED(hr));
    tmEnd = time(0);

    nRows = spiDB->ExecuteScalar(L"select count(*) from emp;");
    CPPUNIT_ASSERT_EQUAL(nRowsToCreate, nRows);

    hr = spiDB->ExecuteUpdate(L"VACUUM");
    CPPUNIT_ASSERT(SUCCEEDED(hr));
}



