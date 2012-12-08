/***************************************************************
 * @date:   2007-10-26
 * @author: BrucePeng
 * @brief:  Some public definition
 */

#ifndef K_FILE_DOWNLOAD_DEFINE_H_
#define K_FILE_DOWNLOAD_DEFINE_H_

#include <list>
#include <Windows.h>

using namespace std;

typedef unsigned char	            T_UNIT8;

typedef unsigned short	            T_UNIT16;

typedef unsigned int	            T_UNIT32;

typedef unsigned long long          T_UNIT64;

typedef HANDLE                      FILE_HANDLE;

//Define the max length of URL string
#define defURL_MAX_LEN              0x0400

//Define the max length of path file name
#define defPFN_MAX_LEN              0x0400

//Define the max length of proxy string
const int cnProxyStrMaxLen          = 0x0100;

//The string: Partial Content
const char* const cszPartialContent     = "206 Partial";

//The string: 200 OK
const char* const csz200OK              = "200 OK";

//The string: content length
const char* const cszContentLength      = "Content-Length: ";

//string: "http://"
const char* const cszHttp               = "http://";

//string: "HTTP://"
const char* const cszHTTP               = "HTTP://";

/**
* @struct st_proxy_info
* Define the structure to store proxy information
*/
typedef struct st_proxy_info
{
    //The method of using proxy
    int             nProxyMethod;

    //If proxy enable
    //Only be used when enumPROXY_METHOD_USE_IE is set
    bool            bProxyEnable;

    //The type of proxy: SOCK4 or SOCK5 or HTTP
    int             nProxyType;

    //The address of proxy server
    char            szProxyAddr[cnProxyStrMaxLen];

    //The port of proxy server open for service
    unsigned short  usProxyPort;

    //The user name for validating
    char            szUsername[cnProxyStrMaxLen];

    //The password for validating
    char            szPassword[cnProxyStrMaxLen];

}ST_PROXY_INFO, *PST_PROXY_INFO;


/**
* @struct ST_FILE_BLOCK_INFO
* @brief Define the structure to store file block information
*/
class CDownloadFile;
class CFileDownloader;
class CFileDownloadThread;

typedef struct st_file_block_info
{
    //The user defined ID of this block
    int                     nBlockID;

    bool                    bRangeNotSupport;

    //The beginning of the range
    //The block >= ulSeek
    unsigned long           ulSeek;

    //The end of range
    //The block < ulEnd
    unsigned long           ulEnd;

    //the size of data completed
    unsigned long           ulCompleted;

    //File handle, Have set file point to position <ulSeek>
    CDownloadFile*          pLocalFile;

    //The thread who takes full responsibility 
    //for downloading this block
    CFileDownloadThread*    pThread;

    //The downloader thread belongs to
    CFileDownloader*        pDownloader;

    //The number of try
    int                     nTryCount;

    //The max try
    int                     nMaxTryNum;

    //The last time in "tick count" that downloading stopped
    unsigned long           ulLastStopTime;

}ST_FILE_BLOCK_INFO, *PST_FILE_BLOCK_INFO;

#endif//K_FILE_DOWNLOAD_DEFINE_H_
