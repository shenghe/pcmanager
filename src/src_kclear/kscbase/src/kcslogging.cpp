//////////////////////////////////////////////////////////////////////////
#include <atlconv.h>
#include <atlstr.h>
#include <iostream>
#include <time.h>
#include <atltime.h>
#include <stdarg.h>
#include <shlwapi.h>
#include <shlobj.h>
#include "kcslogging.h"

#pragma  comment(lib, "shlwapi.lib")

//////////////////////////////////////////////////////////////////////////
const char* _GetLevelName(int nLevel)
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

bool _GetTimeStringA(char* pszTime, size_t uSize)
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
//////////////////////////////////////////////////////////////////////////
CKcsLogging::CKcsLogging()
{

}
CKcsLogging::CKcsLogging(LOG_LEVEL level) : m_eLevel(level)
{
//    Init();
    Log(m_eLevel);
}

CKcsLogging::~CKcsLogging()
{
    CStringA strFormat;
    CStringA strPath;
    char szPath[MAX_PATH * 2] = { 0 };   
    char szTime[64] = { 0 };
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    time_t tNow = 0;
    time(&tNow);
    
    strftime(szTime, 64, "%Y-%m-%d", localtime(&tNow));
    strFormat.Format(CLEAR_LOG_FILE, _GetLevelName(m_eLevel), szTime);

    m_stream << std::endl;
    std::string str_newLine(m_stream.str());

    if (str_newLine.empty() || str_newLine.length() <= 10)
        goto Clear0;



    OutputDebugStringA(str_newLine.c_str());

    SHGetSpecialFolderPathA(NULL, szPath, CSIDL_LOCAL_APPDATA, FALSE);

    PathAppendA(szPath, "KSafe\\KClear\\Logs");

    if (!_CreateDirectoryNested(szPath))
        goto Clear0;

    PathAppendA(szPath, strFormat);

    fileHandle = CreateFileA(szPath,
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_ALWAYS,// | CREATE_NEW,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (fileHandle == INVALID_HANDLE_VALUE || fileHandle == NULL)
        goto Clear0;

    SetFilePointer(fileHandle, 0, 0, FILE_END);

    DWORD num_written;
    WriteFile(fileHandle,
        static_cast<const void*>(str_newLine.c_str()),
        static_cast<DWORD>(str_newLine.length()),
        &num_written,
        NULL);
    
Clear0:
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(fileHandle);
        fileHandle = INVALID_HANDLE_VALUE;
    }
    return;
}
CKcsLogging& CKcsLogging::Log(LOG_LEVEL nLevel)
{
    m_stream << _GetFix() << ":" << _GetLevelName(nLevel) << ":";

    return *this;
}
//////////////////////////////////////////////////////////////////////////
CKcsLogging& CKcsLogging::operator << (int nVal)
{
    m_stream << nVal;
    return *this;
}
CKcsLogging& CKcsLogging::operator << (unsigned int nVal)
{
    m_stream << nVal;
    return *this;
}
CKcsLogging& CKcsLogging::operator << (std::string strVal)
{
    m_stream << strVal;
    return *this;
}
CKcsLogging& CKcsLogging::operator << (long nVal)
{
    m_stream << nVal;
    return *this;
}
CKcsLogging& CKcsLogging::operator << (ULONGLONG uVal)
{
    m_stream << uVal;
    return *this;
}
//////////////////////////////////////////////////////////////////////////
void CKcsLogging::ClearLogFileByDays(int nDay /* = 7 */)
{
    CHAR szLogFileName[MAX_PATH * 2] = { 0 };
    WIN32_FIND_DATAA fd = { 0 };
    HANDLE hFile = INVALID_HANDLE_VALUE;

    SHGetSpecialFolderPathA(NULL, szLogFileName, CSIDL_LOCAL_APPDATA, FALSE);

    PathAppendA(szLogFileName, "KSafe\\KClear\\Logs");
    PathAppendA(szLogFileName, "*.*");


    hFile = FindFirstFileA(szLogFileName, &fd);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        goto Clear0;
    }

    do 
    {
        if (0 == stricmp(".", fd.cFileName) || 0 == stricmp("..", fd.cFileName))
            continue;

        time_t lNow = 0;
        time(&lNow);  

        FILETIME localfiletime;    
        FileTimeToLocalFileTime(&(fd.ftLastWriteTime), &localfiletime);

        CTime stSys(lNow);
        CTime stFile(localfiletime);
        CTimeSpan stSPan;

        stSPan =  stSys - stFile;

        if (stSPan.GetDays() >= nDay)
        {           
            CStringA strtemp = szLogFileName ;
            CStringA strFileName = strtemp.Left(strtemp.ReverseFind(L'\\') + 1);
            strFileName += fd.cFileName;                 
            if (::PathFileExistsA(strFileName))
            {
                ::DeleteFileA(strFileName);
            }
        }

    } while (FindNextFileA(hFile, &fd));

Clear0:
    if (hFile != INVALID_HANDLE_VALUE)
    {
        FindClose(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }   
}
//////////////////////////////////////////////////////////////////////////
std::string CKcsLogging::_GetFix()
{
    std::string strRet;
    char szTimeValue[64] = { 0 };

    if (!m_stream)
        goto Clear0;
    
    _GetTimeStringA(szTimeValue, 64);

    strRet += "[";
    strRet += szTimeValue;
    strRet += "]";

Clear0:
    return strRet;
}
inline BOOL CKcsLogging::_CreateDirectoryNested(LPSTR  lpszDir)
{
    BOOL bRet = FALSE;
    CHAR szPreDir[MAX_PATH];
    if (::PathIsDirectoryA( lpszDir ))
    {
        bRet = TRUE;
        goto Clear0;
    }

    strcpy(szPreDir, lpszDir);

    //获取上级目录
    BOOL bGetPreDir = ::PathRemoveFileSpecA(szPreDir);
    if (!bGetPreDir) 
        goto Clear0;
    //如果上级目录不存在,则递归创建上级目录
    if (!::PathIsDirectoryA(szPreDir))
    {
        _CreateDirectoryNested(szPreDir);
    }

    bRet = ::CreateDirectoryA(lpszDir, NULL);
Clear0:
    return bRet;
}

void CKcsLogging::_Lock()
{
    EnterCriticalSection(&m_LogLock);
}

void CKcsLogging::_UnLock()
{
    LeaveCriticalSection(&m_LogLock);
}
//////////////////////////////////////////////////////////////////////////

