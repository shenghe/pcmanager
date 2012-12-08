/************************************************************************
* @file      : ksclog.cpp
* @author    : gaoyi <gaoyi@kingsoft.com>
* @date      : 2011/1/10 11:36:04
* @brief     : Implement a  mini log class
*
* $Id: $
/************************************************************************/
#include "ksclog.h"
#include <atlconv.h>
#include <atlstr.h>
#include <iostream>
#include <time.h>
#include <atltime.h>
#include <stdarg.h>
#include <shlwapi.h>
using namespace std;

#pragma  comment(lib, "shlwapi.lib")

////////////////////////////////////////////////////////////////
bool _GetTimeString(char* pszTime, size_t uSize)
{   
    size_t uRet = 0;

    if (NULL == pszTime)
        return false;
   
    //Get the local system time
    time_t lNow = 0;
    time(&lNow);
    uRet = strftime(pszTime, uSize, "%Y-%m-%d %H:%M:%S", localtime(&lNow));    
    return (0 >= uRet) ? false : true;
}

const char* GetLevelName(int nLevel)
{
    switch (nLevel)
    {
    case LOG_LEVEL_ONEKEYCLEAN:
        return "OneKeyClean";
    case LOG_LEVEL_TRASHCLEAN:
        return "TrashClean";
    case LOG_LEVEL_REGCLEAN:
        return "RegClean";
    case LOG_LEVEL_TRACKCLEAN:
        return "TrackClean";
    }

    return "UNKNOWN";
}

//////////////////////////////////////////////////////////////////////////
CFileLogWriter::CFileLogWriter(void)
{
    
}

CFileLogWriter::CFileLogWriter(LPCTSTR lpcleantype)
{
    CString strFormat;
    WCHAR sztimevalue[64] = { 0 };
    WCHAR szLogFileName[MAX_PATH] = { 0 };

    time_t lNow = 0;
    time(&lNow);
    wcsftime(sztimevalue, 64, L"%Y-%m-%d", localtime(&lNow));
    strFormat.Format(CLEAR_LOG_FILE, lpcleantype ? lpcleantype : L"", sztimevalue);

    GetModuleFileName(NULL, szLogFileName, MAX_PATH);
    PathRemoveFileSpec(szLogFileName);
    PathAppend(szLogFileName, strFormat);

    InitializeCriticalSection(&m_LogLock);

    m_stream.clear();
    m_stream.open(szLogFileName, std::ios::out  | std::ios::app );
//    m_stream.imbue(std::locale(""));

    //Delete the log before 2 days
    ClearSomeLogfile();

}

CFileLogWriter::~CFileLogWriter(void)
{
    if (m_stream)
    {
        m_stream.close();
    }

    DeleteCriticalSection(&m_LogLock);
}

CFileLogWriter& CFileLogWriter::Log(int nLevel)
{ 
    if (m_stream)
    {
        PreFix();
        const char* pszLevelname = GetLevelName(nLevel);
        m_stream << "<" <<  pszLevelname << ">" << ":";     
    }

    return *this;
}

CFileLogWriter& CFileLogWriter::operator <<(int nVal)
{
    m_stream << nVal;
    return *this;
}

CFileLogWriter& CFileLogWriter::operator <<(unsigned int nVal)
{
    m_stream << nVal;
    return *this;

}

CFileLogWriter& CFileLogWriter::operator <<(long lVal)
{
    m_stream << lVal;
    return *this;
}

CFileLogWriter& CFileLogWriter::operator <<(unsigned long ulval)
{
    m_stream << ulval;
    return *this;
}

CFileLogWriter& CFileLogWriter::operator <<(TCHAR* pchar)
{
    m_stream << pchar;
    return *this;
}

CFileLogWriter& CFileLogWriter::operator <<(const TCHAR* pchar)
{
    m_stream<<pchar;
    return *this;
}

CFileLogWriter& CFileLogWriter::operator <<(TCHAR chr)
{
    m_stream << chr;
    return *this;
}


#if defined(UNICODE) || defined(_UNICODE)
CFileLogWriter& CFileLogWriter:: operator<<(std::wstring& strval)
{
    m_stream << strval;
    return *this;
}

CFileLogWriter& CFileLogWriter::operator <<(const std::wstring& strVal)
{
    m_stream << strVal;
    return *this;
}

#else
CFileLogWriter& CFileLogWriter::operator <<(std::string& strval)
{
    m_stream << strval;
    return *this;
}

CFileLogWriter& CFileLogWriter::operator <<(const std::string& strval)
{
    m_stream << strval;
    return *this;
}
#endif

CFileLogWriter& CFileLogWriter::operator<<(ENDL_FLAG enumflag)
{
    switch (enumflag)
    {
    case enum_endl:
        m_stream << std::endl;
        m_stream.flush();
        break;

    default:
        break;
    }

    return *this;
}

void CFileLogWriter::PreFix()
{
    if (!m_stream)
        return;
 
    char szTimeValue[64] = { 0 };
    _GetTimeString(szTimeValue, 64);
    m_stream << "[" << szTimeValue << "]";
    return;
}

void CFileLogWriter::_LockWork()
{
    EnterCriticalSection(&m_LogLock);
}

void CFileLogWriter::_UnLockwork()
{
    LeaveCriticalSection(&m_LogLock);
}

BOOL CFileLogWriter::ClearLog(LPCTSTR lpLogfileName)
{
    if (!lpLogfileName)
        return FALSE;

    if (::PathFileExists(lpLogfileName))
    {
        ::DeleteFile(lpLogfileName);
    }
    return TRUE;
}

/*Delete the logfiles  two days ago */
BOOL CFileLogWriter::ClearSomeLogfile( )
{
     WCHAR szLogFileName[MAX_PATH] = { 0 };

     GetModuleFileName(NULL, szLogFileName, MAX_PATH);
     PathRemoveFileSpec(szLogFileName);
     PathAppend(szLogFileName, L"log\\*.*");
     WIN32_FIND_DATA fd = { 0 };

     HANDLE hFile  = FindFirstFile(szLogFileName, &fd);
     if (INVALID_HANDLE_VALUE == hFile)
     {
         return FALSE;
     }

     do 
     {
         if (0 == _wcsicmp(L".", fd.cFileName) || 0 == _wcsicmp(L"..", fd.cFileName))
             continue;
        
         time_t lNow = 0;
         time(&lNow);  

         FILETIME localfiletime;    
         FileTimeToLocalFileTime(&(fd.ftLastWriteTime), &localfiletime);

         CTime stSys(lNow);
         CTime stFile(localfiletime);
         CTimeSpan stSPan;

         stSPan =  stSys - stFile;

         if (stSPan.GetDays() > 6)
         {           
             CString strtemp = szLogFileName ;
             CString strFileName = strtemp.Left(strtemp.ReverseFind(L'\\') + 1);
             strFileName += fd.cFileName;                 
             ClearLog(strFileName);
         }
              
     } while (FindNextFile(hFile, &fd));

     FindClose(hFile);

     return TRUE; 
}

// -------------------------------------------------------------------------
// $Log: $
