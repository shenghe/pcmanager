/**
* @file    sqlitetest.h
* @brief   ...
* @author  zhangrui
* @date    2008-11-16  15:23
*/

#ifndef SQLITETEST_H
#define SQLITETEST_H

class CSQLiteTest: public CPPUNIT_NS::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(CSQLiteTest);
        CPPUNIT_TEST(testSQLiteSingleThread);
        CPPUNIT_TEST(testSQLiteSingleThreadWithCrypt);
    CPPUNIT_TEST_SUITE_END();

private:
    void testSQLiteSingleThread();
    void testSQLiteSingleThreadWithCrypt();

};

#endif//SQLITETEST_H