/***************************************************************
 * @date:   2007-10-19
 * @author: BrucePeng
 * @brief:  Define the download thread object
 */

#include "KFileDownloadThread.h"
#include "KDebugInfoRecord.h"
#include "KFileDownloadCommon.h"
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

//Define the size of range string buffer
const int   cnRangeBufSize      = 0x20;

//Define the low speed limit, in bytes per second
const long  clLowSpeedLimit     = 0x01;

//Define the low speed time,  in seconds
const long  clLowSpeedTime      = 0x20;

CFileDownloadThread::CFileDownloadThread(
    int             nUserDefID, 
    IDTCallback*    m_pCallback
):m_nUserDefID(nUserDefID), 
  m_pCallback(m_pCallback),
  m_ulSleepTime(0),
  m_pCurl(NULL)
{
}

CFileDownloadThread::~CFileDownloadThread(void)
{
    Uninitialize();
}

bool CFileDownloadThread::Initialize(void)
{
    CURLcode nCurlCode  = CURLE_OK;

    Uninitialize();

    nCurlCode = curl_global_init(CURL_GLOBAL_WIN32);
    if(CURLE_OK != nCurlCode)
    {
        return false;
    }

    m_pCurl = curl_easy_init();
    if(NULL == m_pCurl)
    {
        return false;
    }
    return true;
}

bool CFileDownloadThread::Uninitialize(void)
{
    if(NULL == m_pCurl)
    {
        return true;
    }
    curl_easy_cleanup(m_pCurl);
    m_pCurl = NULL;
    curl_global_cleanup();

    return true;
}

void CFileDownloadThread::Run(void)
{
    Sleep(m_ulSleepTime);

    //We will never Reuse connection, ^_^.
    int                     nRetCode                        = -1;
    CURLcode                nCurlCode                       = CURLE_OK;
    CURL*                   pCurl                           = NULL;
    ST_FILE_BLOCK_INFO*     pBlockInfo                      = NULL;
    struct curl_slist*      pCurlSlist                      = NULL;
    char                    szURL[defURL_MAX_LEN]           = {0};
    char                    szRange[cnRangeBufSize]         = {0};
    char                    szUsrPwd[cnProxyStrMaxLen * 2]  = {0};
    ST_PROXY_INFO           stInfo;
    
    if(NULL == m_pCallback)
        return;

    if(NULL == m_pCurl && !Initialize())
        goto Exit0;

    pCurl = m_pCurl;

    //Set option "URL"
    if(!m_pCallback->GetDownloadUrl(szURL, defURL_MAX_LEN))
        goto Exit0;

    nCurlCode = curl_easy_setopt(pCurl, CURLOPT_URL, szURL);
    if(CURLE_OK != nCurlCode)
        goto Exit0;

    //Set option "HTTP Header"
    if(!SetOurHttpHeader(szURL, &pCurlSlist))
        goto Exit0;

    nCurlCode = curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pCurlSlist);
    if(CURLE_OK != nCurlCode)
        goto Exit0;

    //Set option "write function"
    nCurlCode = curl_easy_setopt(pCurl, 
        CURLOPT_WRITEFUNCTION, 
        CFileDownloader::MyWriteFunction
    );
    if(CURLE_OK != nCurlCode)
        goto Exit0;

    //Set option "header function"
    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_HEADERFUNCTION,
        MyHeaderFunction
    );
    if(CURLE_OK != nCurlCode)
        goto Exit0;

    //Set option "write data"
    pBlockInfo = m_pCallback->GetFileBlockInfoByID(m_nUserDefID);
    if(NULL == pBlockInfo)
        goto Exit0;

    nCurlCode = curl_easy_setopt(pCurl, 
        CURLOPT_WRITEDATA, 
        pBlockInfo
    );
    if(CURLE_OK != nCurlCode)
        goto Exit0;

    //Set progress function
    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_NOPROGRESS,
        FALSE
    );
    if(CURLE_OK != nCurlCode)
        goto Exit0;

    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_PROGRESSFUNCTION,
        CFileDownloader::MyProgressFunction
    );
    if(CURLE_OK != nCurlCode)
        goto Exit0;

    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_PROGRESSDATA,
        pBlockInfo->pDownloader
    );
    if(CURLE_OK != nCurlCode)
        goto Exit0;
    
    if(pBlockInfo->bRangeNotSupport)
    {
        nCurlCode = curl_easy_setopt(pCurl,
            CURLOPT_RANGE,
            NULL
        );
        if(CURLE_OK != nCurlCode)
            goto Exit0;
    }
    else
    {
        //compute range
        if(pBlockInfo->ulSeek + pBlockInfo->ulCompleted 
            == pBlockInfo->ulEnd)
        {
            //it means this block is 100% completed
            goto Exit0;
        }

        nRetCode = _snprintf(szRange, 
            cnRangeBufSize,
            "%d-%d", 
            pBlockInfo->ulSeek + pBlockInfo->ulCompleted, 
            pBlockInfo->ulEnd - 1
            );
        if(nRetCode <= 0 || cnRangeBufSize == nRetCode)
            goto Exit0;

        nCurlCode = curl_easy_setopt(pCurl, 
            CURLOPT_RANGE, 
            szRange
        );
        if(CURLE_OK != nCurlCode)
            goto Exit0;
    }

    //Set proxy information
    if(!m_pCallback->GetProxyInfo(stInfo))
        goto Exit0;

    if(IsProxyOptionsNeeded(stInfo))
    {
        int nProxyType = CURLPROXY_HTTP;
        if(enumPROXY_TYPE_SOCK5 == stInfo.nProxyType)
        {
            nProxyType = CURLPROXY_SOCKS5;
        }
        else if(enumPROXY_TYPE_HTTP == stInfo.nProxyType)
        {
            nProxyType = CURLPROXY_HTTP;
        }
        else if(enumPROXY_TYPE_SOCK4 == stInfo.nProxyType)
        {
            nProxyType = CURLPROXY_SOCKS4;
        }
        else
        {
            RecordWhatHappened("error ", "Unknown proxy type", m_nUserDefID);
            goto Exit0;
        }
        
        //Set option "proxy type"
        nCurlCode = curl_easy_setopt(pCurl,
            CURLOPT_PROXYTYPE, 
            nProxyType
        );
        if(CURLE_OK != nCurlCode)
            goto Exit0;

        //Set option "proxy address"
        nCurlCode = curl_easy_setopt(pCurl, 
            CURLOPT_PROXY, 
            stInfo.szProxyAddr
        );
        if(CURLE_OK != nCurlCode)
            goto Exit0;

        //Set option "proxy port"
        nCurlCode = curl_easy_setopt(pCurl, 
            CURLOPT_PROXYPORT, 
            stInfo.usProxyPort
        );
        if(CURLE_OK != nCurlCode)
            goto Exit0;

        if(strlen(stInfo.szUsername) > 0)
        {
            nCurlCode = curl_easy_setopt(pCurl, 
                CURLOPT_PROXYAUTH, 
                1
            );
            if(CURLE_OK != nCurlCode)
                goto Exit0;

            _snprintf(szUsrPwd,
                cnProxyStrMaxLen * 2,
                "%s:%s",
                stInfo.szUsername,
                stInfo.szPassword
            );

            nCurlCode = curl_easy_setopt(pCurl, 
                CURLOPT_PROXYUSERPWD, 
                szUsrPwd
            );
            if(CURLE_OK != nCurlCode)
                goto Exit0;

        }//if(strlen(stInfo.szUsrPwd) > 0)
    }//if(stInfo.bProxyNeeded && strlen(stInfo.szProxyAddr) > 0)

    //Set option "Connection timeout"
    nCurlCode = curl_easy_setopt(pCurl, 
        CURLOPT_CONNECTTIMEOUT, 
        m_pCallback->GetConnectTimeout()
    );
    if(CURLE_OK != nCurlCode)
        goto Exit0;

    //Set speed limit
    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_LOW_SPEED_LIMIT,
        clLowSpeedLimit
    );
    if(CURLE_OK != nCurlCode)
        goto Exit0;

    //Set speed limit time
    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_LOW_SPEED_TIME,
        m_pCallback->GetRecvTimeout()
    );
    if(CURLE_OK != nCurlCode)
        goto Exit0;

    //Enable auto redirect
    if(!EnableAutoRedirect(pCurl, 256, true))
        goto Exit0;

    //Perform Perform Perform Perform Perform Perforem Perform

#ifndef USE_ASYNCHRONISM_CURL
    nCurlCode = curl_easy_perform(pCurl);
    if(CURLE_OK != nCurlCode)
    {
        goto Exit0;
    }

#else

    //为什么要使用异步的?
    //主要是为了在客户端长时间等待数据库返回数据时，用户能够
    //快速使下载退出(其实我不想用的，容易出问题)
    PerformCurlRequest(pCurl, m_pCallback);

#endif//USE_ASYNCHRONISM_CURL

Exit0:
    if(S_OK != nCurlCode)
    {
        //Record error message
        //Log error here please
        char szError[MAX_PATH] = {0};
        _snprintf(szError, 
            MAX_PATH, 
            "Return Code=%.2d, description: %s",
            nCurlCode,
            curl_easy_strerror(nCurlCode)
        );
        RecordWhatHappened("error", szError, m_nUserDefID);
    }

    //Record the time when thread exit
    pBlockInfo->ulLastStopTime = GetTickCount();

    //Clear list
    if(NULL != pCurlSlist)
    {
        curl_slist_free_all(pCurlSlist);
    }
}

size_t CFileDownloadThread::MyHeaderFunction(
    void*                   ptr, 
    size_t                  size, 
    size_t                  nmemb, 
    void*                   stream 
)
{
    if(NULL == ptr)//////////////////////////////
    {
        return size * nmemb;
    }

    char* pos = strstr((char* )ptr, "HTTP/1.");
    if(NULL != pos)
    {
        char* pos1 = strstr((char* )ptr, csz200OK);
        char* pos2 = strstr((char* )ptr, cszPartialContent);

        if(NULL == pos1 && NULL == pos2)
        {
            char szInfo[MAX_PATH] = {0};
            _snprintf(szInfo, MAX_PATH, 
                "not 200 or 206, so exit, return from server: %s",
                pos
            );
            RecordWhatHappened("error", szInfo, -1);
            return -1;
        }
    }
    return size * nmemb;
}

bool CFileDownloadThread::IsProxyOptionsNeeded(const ST_PROXY_INFO& stInfo)
{
    if(enumPROXY_METHOD_DIRECT == stInfo.nProxyMethod)
        return false;

    if(enumPROXY_METHOD_USE_IE == stInfo.nProxyMethod
        && false == stInfo.bProxyEnable)
    {
        return false;
    }

    if(strlen(stInfo.szProxyAddr) <= 0)
        return false;

    return true;
}