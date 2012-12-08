/************************************************************************
* @file      : kscbase/ksclog.h
* @author    : gaoyi <gaoyi@kingsoft.com>
* @date      : 2011/1/10 11:38:24
* @brief     : Define the implement a mini log class
*
* $Id: $
/************************************************************************/
#ifndef __KSCBASE_KSCLOG_H__
#define __KSCBASE_KSCLOG_H__

#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#ifdef WIN32
#include <windows.h>
#endif

#define LOG_SUCCESS         (0)
#define LOG_ERROR           (-1)

#define LOG_STRING_MAX_SIZE (0x0800)

#define CLEAR_LOG_FILE  L"log\\%s_%s.log"


typedef enum log_level
{
    LOG_LEVEL_ONEKEYCLEAN         = 0x01,
    LOG_LEVEL_TRASHCLEAN          = 0x02,
    LOG_LEVEL_REGCLEAN            = 0x04,
    LOG_LEVEL_TRACKCLEAN          = 0x08
}LOG_LEVEL;

typedef enum  endl_flag
{
    enum_endl = 0,
}ENDL_FLAG;


/**
* @brief Define the file log writer class
*/
class CFileLogWriter
{
public:
    CFileLogWriter(void);
    CFileLogWriter(LPCTSTR lpcleantype);
    virtual ~CFileLogWriter(void);

public:
    void PreFix();
    CFileLogWriter& Log(int nLevel);

public:
    void _LockWork();
    void _UnLockwork();
    BOOL ClearLog(LPCTSTR lpLogfileName);
    BOOL ClearSomeLogfile();

public:
    virtual CFileLogWriter& operator<<(TCHAR* pchar);
    virtual CFileLogWriter& operator <<(const TCHAR* pchar);
    virtual CFileLogWriter& operator<<(TCHAR chr);

#if defined(UNICODE) || defined(_UNICODE)  
    virtual CFileLogWriter& operator<<(std::wstring& strval);
    virtual CFileLogWriter& operator<<(const std::wstring& strval);
#else
    virtual CFileLogWriter& operator<<(std::string& strval);
    virtual CFileLogWriter& operator<<(const std::string& strval);
#endif

    virtual CFileLogWriter& operator<<(int nVal);
    virtual CFileLogWriter& operator<<(unsigned int nVal);
    virtual CFileLogWriter& operator<<(long lVal);
    virtual CFileLogWriter& operator<<(unsigned long ulVal);
    virtual CFileLogWriter& operator<<(ENDL_FLAG enumflag);
  
private:
#if defined(UNICODE) || defined(_UNICODE)
    std::wofstream    m_stream;
#else
    std::ofstream     m_stream;
#endif
     CRITICAL_SECTION        m_LogLock;  
};

//////////////////////////////////////////////////////////////////////////


#endif /* __KSCBASE_KSCLOG_H__ */
