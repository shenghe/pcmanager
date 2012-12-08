/***************************************************************
 * @date:   2007-12-04
 * @author: BrucePeng
 * @brief:  
 */
#include "KDebugInfoRecord.h"

#include <fstream>
#include <time.h>
using namespace std;

#include <shlwapi.h>
#include <shlobj.h>
#pragma  comment(lib, "shlwapi.lib")
#pragma  comment(lib, "shell32.lib")

//Define the max path of time value
#define defTIME_MAX_PATH            0x20

//The Flag: If invoke first time
static bool g_bFistInvoke                 = true;

//The Flag: If initialized
static bool g_bInited                     = false;

//The path file name of Log file
static char g_szLogPathFileName[MAX_PATH] = {0};

//Define the name of log file
const char* const cszLogFileName    = "file_downoader_log.txt";

//Define the max size of log file(4M)
const unsigned long culLogFileMaxSize   = 0x0400 * 0x0400 * 0x04;

/**
 * @brief Get the local time
 *
 * @param[OUT]  pTimeValue  The buffer to save time value
 * @param[IN]   uSize       The size of buffer
 *
 * @return If successfully
 * -true    YES
 *  false   NO
 */ 
bool GetTime(OUT char* pTimeValue, IN size_t uSize)
{
    time_t  lNowLocalTime   = 0;
    tm*	    pstTime         = NULL;

    if(NULL == pTimeValue)
        return false;

    //Get the system time
    time(&lNowLocalTime);

    //Convert a time value to a structure.
    pstTime = gmtime(&lNowLocalTime);

    int nCurrentYear    = pstTime->tm_year + 1900;
    int nCurrentMonth   = pstTime->tm_mon + 1;
    int nCurrentDay     = pstTime->tm_mday;
    int nCurrentHour    = pstTime->tm_hour + 8;
    int nCurrentMin     = pstTime->tm_min;
    int nCurrentSec     = pstTime->tm_sec;

    //Create string
    size_t uRetCode = _snprintf(pTimeValue, 
        uSize, 
        "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
        nCurrentYear, 
        nCurrentMonth, 
        nCurrentDay,
        nCurrentHour, 
        nCurrentMin, 
        nCurrentSec
    );
    if(0 >= uRetCode || uSize == uRetCode)
        return false;

    return true;
}

/**
 * @brief Get the size of a file
 *
 * @param[IN]   pszFileName The path file name
 * @param[OUT]  ulSize      The size of file
 *
 * @return If successfully
 * -true    YES
 *  false   NO
 */
bool GetFileSize(IN const char* pszFileName, OUT unsigned long& ulSize)
{
    long lRet = -1;

    FILE* fp = fopen(pszFileName, "r");
    if(NULL == fp)
    {
        return false;
    }

    fseek(fp, 0, SEEK_END);
    lRet = ftell(fp);
    fclose(fp);

    if(lRet < 0)
    {
        return false;
    }

    ulSize = (unsigned long)lRet;
    return true;

}

//Record what happened simply
bool RecordWhatHappened(
    IN  const char* pszTitle, 
    IN  const char* pszText,
    IN  int         nThreadID
)
{
    if(!pszTitle || !pszText)
    {
        return false;
    }

    if(true == g_bFistInvoke)
    {
        //Initialize the path file name of log file
        char szModule[MAX_PATH] = {0};
        char szFolder[MAX_PATH] = {0};
        if(GetModuleFileNameA(NULL, szModule, MAX_PATH))
        {
            PathRemoveFileSpecA(szModule);

            _snprintf(szFolder, MAX_PATH, "%s\\LOG", szModule);
            if(!PathFileExistsA(szFolder))
            {
                SHCreateDirectoryExA(NULL, szFolder, NULL);
            }

            _snprintf(g_szLogPathFileName, MAX_PATH,
                "%s\\%s",
                szFolder,
                cszLogFileName
            );
            g_bInited = true;
        }

        //Get file size and check
        unsigned long ulFileSize = 0;
        if(true == g_bInited &&
           GetFileSize(g_szLogPathFileName, ulFileSize) &&
           culLogFileMaxSize < ulFileSize)
        {
            remove(g_szLogPathFileName);
        }

        g_bFistInvoke = false;
    }

    if(false == g_bInited)
        return false;

    char szTime[defTIME_MAX_PATH] = {0};

    //Get time first
    if(!GetTime(szTime, defTIME_MAX_PATH))
        return false;

    //Open file
    ofstream fout(g_szLogPathFileName, ios_base::out | ios_base::app);
    if(!(fout.is_open()))
    {
        return false;
    }
    fout << "[" << szTime << "]" << "[" 
         << nThreadID << "] [" << pszTitle << "]  " << pszText << endl;
    fout.close();
    return true;
}
