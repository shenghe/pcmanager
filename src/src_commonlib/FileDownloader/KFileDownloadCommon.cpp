/***************************************************************
 * @date:   2007-12-06
 * @author: BrucePeng
 * @brief:  Define some common functions
 */

#include "KFileDownloadCommon.h"
#include "KDebugInfoRecord.h"
#include "IDownloadThreadCallback.h"

#include <WinInet.h>
#pragma comment(lib, "Wininet.lib")

//The max length of cookie string
const int cnCookieMaxLen                = 0x1000;

const char* const pcszUserAgent
= "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)";

const char* const pcszConnection    = "Connection: close";

const char* const pcszPragma        = "Pragma: no-cache";

const char* const pcszCatchCtl      = "Cache-Control: no-cache";

bool SetOurHttpHeader(const char* pszURL, curl_slist** ppCurlSlist)
{
    if(NULL == pszURL)
    {
        return false;
    }

    //Add "User-Agent"
    *ppCurlSlist = curl_slist_append(*ppCurlSlist, pcszUserAgent);
    if(NULL == *ppCurlSlist)
    {
        return false;
    }

    //Add "Referer"
    char* pos = NULL;
    char  szReferer[defURL_MAX_LEN] = {0};

    _snprintf(szReferer, defURL_MAX_LEN, "Referer: %s", pszURL);
    pos = strrchr(szReferer, '/');
    if(NULL != pos)
    {
        *pos = '\0';
    }

    *ppCurlSlist = curl_slist_append(*ppCurlSlist, szReferer);
    if(NULL == *ppCurlSlist)
    {
        return false;
    }

    //Add Cookies
    char    szCookie[cnCookieMaxLen] = {0};
    DWORD   dwSize  = cnCookieMaxLen;
    if(InternetGetCookieA(pszURL, NULL, szCookie, &dwSize))
    {
        char szCookieString[cnCookieMaxLen] = {0};
        int nRet = _snprintf(szCookieString, 
            cnCookieMaxLen,
            "Cookie: %s",
            szCookie
        );
        if(0 >= nRet || cnCookieMaxLen == nRet)
        {
            return true;
        }

        if(0 >= strlen(szCookieString))
            return true;

        *ppCurlSlist = curl_slist_append(*ppCurlSlist, szCookieString);
        if(NULL == *ppCurlSlist)
        {
            return false;
        }
    }

    //Add Pragma
    *ppCurlSlist = curl_slist_append(*ppCurlSlist, pcszPragma);
    if(NULL == *ppCurlSlist)
    {
        return false;
    }

    //Add Catch control
    *ppCurlSlist = curl_slist_append(*ppCurlSlist, pcszCatchCtl);
    if(NULL == *ppCurlSlist)
    {
        return false;
    }

    //Add "Connection"
    *ppCurlSlist = curl_slist_append(*ppCurlSlist, pcszConnection);
    if(NULL == *ppCurlSlist)
    {
        return false;
    }

    return true;
}

bool PerformCurlRequest(IN void* pvCurl, IN IDTCallback* m_pCallback)
{
    bool        bResult         = false;
    CURLMcode   nCurlMCode      = CURLM_OK;
    CURLM*      pCurlMulti      = NULL;
    int         nStillRunning   = 0;

    CURL* pCurl = (CURL* )pvCurl;
    if(NULL == pCurl)
        goto Exit0;

    //Init a multi stack
    pCurlMulti = curl_multi_init();
    if(NULL == pCurlMulti)
        goto Exit0;

    nCurlMCode = curl_multi_add_handle(pCurlMulti, pCurl);
    if(CURLM_OK != nCurlMCode)
        goto Exit0;

    /* we start some action by calling perform right away */
    while(CURLM_CALL_MULTI_PERFORM ==
        curl_multi_perform(pCurlMulti, &nStillRunning)){}

    while(nStillRunning)
    {
        struct timeval  stTimeout;
        fd_set fdread, fdwrite, fdexcep;
        int nRet    = 0;
        int nMaxfd  = 0;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);

        /* set a suitable timeout to play around with */
        stTimeout.tv_sec    = 1;
        stTimeout.tv_usec   = 0;

        /* get file descriptors from the transfers */
        nCurlMCode = curl_multi_fdset(pCurlMulti,
            &fdread,
            &fdwrite,
            &fdexcep,
            &nMaxfd
        );
        if(CURLM_OK != nCurlMCode || nMaxfd < 0)
            goto Exit0;

        //select
        nRet = select(nMaxfd + 1, &fdread, &fdwrite, &fdexcep, &stTimeout);
        switch(nRet)
        {
        case -1://In windows, just SOCKET_ERROR
            goto Exit0;
            break;
        case 0: //Timeout
        default://Good
            while(CURLM_CALL_MULTI_PERFORM ==
                curl_multi_perform(pCurlMulti, &nStillRunning)){}
            break;
        }
    }

    //Query result
    {
        /* for picking up messages with the transfer status */
        CURLMsg* pMsg        = NULL; 
        int      nMsgsLeft   = 0;

        while((pMsg = curl_multi_info_read(pCurlMulti, &nMsgsLeft)))
        {
            if(CURLMSG_DONE == pMsg->msg)
            {
                if(CURLE_OK == (pMsg->data).result)
                    continue;

                //Log error here please
                char szError[MAX_PATH] = {0};
                _snprintf(szError, 
                    MAX_PATH, 
                    "Return Code=%.2d, description: %s",
                    (pMsg->data).result,
                    curl_easy_strerror((pMsg->data).result)
                );
                RecordWhatHappened("error", szError);
                goto Exit0;
            }
        }
    }

    //If exit by notify, the return false
    if(m_pCallback->IsExitNeeded())
    {
        goto Exit0;
    }

    bResult = true;
Exit0:
    if(CURLM_OK != nCurlMCode)
    {
        char szError[MAX_PATH] = {0};
        _snprintf(szError, 
            MAX_PATH, 
            "MCode=%.2d, description: %s",
            nCurlMCode,
            curl_multi_strerror(nCurlMCode)
        );
        RecordWhatHappened("error", szError);
    }

    if(NULL != pCurlMulti && NULL != pCurl)
    {
        curl_multi_remove_handle(pCurlMulti, pCurl);
    }
    if(NULL != pCurlMulti)
    {
        curl_multi_cleanup(pCurlMulti);
    }

    return bResult;
}

bool EnableAutoRedirect(IN void* pvCurl, long lMaxRedirs, bool bAutoRefer)
{
    CURLcode nCurlCode = CURLE_OK;

    if(NULL == pvCurl || 0 >= lMaxRedirs)
    {
        return false;
    }

    CURL* pCurl = (CURL* )pvCurl;
    if(NULL == pCurl)
    {
        return false;
    }

    nCurlCode = curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1);
    if(CURLE_OK != nCurlCode)
    {
        return false;
    }

    nCurlCode = curl_easy_setopt(pCurl, CURLOPT_MAXREDIRS, lMaxRedirs);
    if(CURLE_OK != nCurlCode)
    {
        return false;
    }

    if(!bAutoRefer)
    {
        return true;
    }

    nCurlCode = curl_easy_setopt(pCurl, CURLOPT_AUTOREFERER, 1);
    if(CURLE_OK != nCurlCode)
    {
        return false;
    }

    return true;
}

