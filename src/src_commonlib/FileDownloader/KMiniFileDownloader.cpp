/***************************************************************
* @date:   2007-10-19
* @author: BrucePeng
* @brief:  Define the file CMiniFileDownloader object
*/

#include "KMiniFileDownloader.h"
#include "KDownloadFile.h"
#include <AtlConv.h>

#pragma comment(lib, "Wininet.lib")

const unsigned long ulDefaultTimeout = 0x10;

CMiniFileDownloader::CMiniFileDownloader(void)
:m_pCallback(NULL),
 m_bExitFlag(false),
 m_ulConnTimeout(ulDefaultTimeout),
 m_ulSendTimeout(ulDefaultTimeout),
 m_ulRecvTimeout(ulDefaultTimeout),
 m_ulStartTickCount(0),
 m_ulFileLength(0)

{
    ZeroMemory(m_szDownloadURL, defURL_MAX_LEN);
    ZeroMemory(m_szLocalFileName, defPFN_MAX_LEN);
    ZeroMemory(&m_stProxy, sizeof(ST_PROXY_INFO));
    m_stProxy.nProxyMethod  = enumPROXY_METHOD_DIRECT;
}

CMiniFileDownloader::~CMiniFileDownloader(void)
{
}

HRESULT CMiniFileDownloader::SetProxyMethod(int nProxyMethod)
{
    m_stProxy.nProxyMethod  = nProxyMethod;
    return S_OK;
}

HRESULT CMiniFileDownloader::SetProxyInfo(
    IN int              nProxyType, 
    IN const char*      pszProxyAddr, 
    IN unsigned short   usProxyPort, 
    IN const char*      pszUsername /* = NULL */, 
    IN const char*      pszPassword /* = NULL  */
)
{
    HRESULT hrRetCode  = E_FAIL;

    if(NULL == pszProxyAddr)
        goto Exit0;

    m_stProxy.nProxyType    = nProxyType;
    m_stProxy.usProxyPort   = usProxyPort;

    if(strlen(pszProxyAddr) >= cnProxyStrMaxLen)
        goto Exit0;

    strncpy(m_stProxy.szProxyAddr, 
        pszProxyAddr, 
        cnProxyStrMaxLen
    );

    if(!pszUsername && !pszPassword)
    {
        if(strlen(pszUsername) >= cnProxyStrMaxLen
            ||strlen(pszPassword) >= cnProxyStrMaxLen)
        {
            goto Exit0;
        }
        strncpy(m_stProxy.szUsername, 
            pszUsername, 
            cnProxyStrMaxLen
        );
        strncpy(m_stProxy.szPassword, 
            pszPassword, 
            cnProxyStrMaxLen
        );
    }

    hrRetCode = S_OK;
Exit0:
    if(E_FAIL == hrRetCode)
        ZeroMemory(&m_stProxy, sizeof(ST_PROXY_INFO));

    return hrRetCode;
}

HRESULT CMiniFileDownloader::SetCallback(IN IDownloadCallback* pCallback)
{
    m_pCallback = pCallback;
    return S_OK;
}

HRESULT CMiniFileDownloader::SetConnectTimeout(IN unsigned long ulTimeout)
{
    m_ulConnTimeout = ulTimeout;
    return S_OK;
}

HRESULT CMiniFileDownloader::SetSendTimeout(IN unsigned long ulTimeout)
{
    m_ulSendTimeout = ulTimeout;
    return S_OK;
}

HRESULT CMiniFileDownloader::SetRecvTimeout(IN unsigned long ulTimeout)
{
    m_ulRecvTimeout = ulTimeout;
    return S_OK;
}

HRESULT CMiniFileDownloader::Download(//////////////////////////
    IN  const char*         pszDownloadURL, 
    IN  const char*         pszLocalFileName, 
    IN  bool                bFailIfExists, 
    OUT unsigned long&      ulErrorCode 
)
{
    BOOL        bRetCode                    = FALSE;
    HRESULT     hrResult                    = E_FAIL;
    HINTERNET	hInetOpen	                = NULL; 
    HINTERNET	hInetConnect                = NULL;
    HINTERNET	hInetRequest                = NULL;	
    char        szServer[MAX_PATH]          = {0};
    char        szObject[defURL_MAX_LEN]    = {0};
    USHORT      usPort                      = 80;
    CDownloadFile   LocalFile;

    if(!pszDownloadURL || !pszLocalFileName)
        goto Exit0;

    //Parse URL
    bRetCode = ParseURL(pszDownloadURL,
        szServer,
        MAX_PATH,
        szObject,
        defURL_MAX_LEN,
        usPort
    );
    if(TRUE != bRetCode)
        goto Exit0;

    //Connect
    if(!DoInternetConnect(szServer, usPort, hInetOpen, hInetConnect))
        goto Exit0;

    //Open a request
    hInetRequest = HttpOpenRequestA(hInetConnect,
        "GET",
        szObject,
        "HTTP/1.1",
        NULL,
        NULL,
        INTERNET_FLAG_DONT_CACHE,
        0
    );
    if(NULL == hInetRequest)
        goto Exit0;

    //Query file information
    DWORD dwSize = sizeof(unsigned long);
    bRetCode = HttpQueryInfoA(hInetRequest,
        HTTP_QUERY_CONTENT_LENGTH,
        &m_ulFileLength,
        &dwSize,
        NULL
    );
    if(TRUE != bRetCode)
        goto Exit0;

    hrResult = S_OK;
Exit0:
    if (NULL != hInetOpen)
    {
        InternetCloseHandle(hInetOpen);
    }
    if (NULL != hInetConnect)
    {
        InternetCloseHandle(hInetConnect);
    }
    if (NULL != hInetRequest)
    {
        InternetCloseHandle(hInetRequest);
    }

    LocalFile.Close();
    return hrResult;
}

HRESULT CMiniFileDownloader::NotifyExit(void)
{
    m_bExitFlag = true;
    return S_OK;
}

bool CMiniFileDownloader::DoInternetConnect(
    IN  const char*     pszServer, 
    IN  unsigned short  usPort, 
    OUT HINTERNET&      hInetOpen, 
    OUT HINTERNET&      hInetConnect 
)
{
    BOOL bRetCode = FALSE;

    if(NULL == pszServer)
        return false;

    //Initializes an application's use of the WinINet functions
    hInetOpen = InternetOpenA("@@@^_^@@@",
        INTERNET_OPEN_TYPE_DIRECT,
        NULL,
        NULL,
        0
    );
    if(NULL == hInetOpen)
        return false;

    //Opens an HTTP session.
    hInetConnect = InternetConnectA(hInetOpen,
        pszServer,
        usPort,
        NULL,
        NULL,
        INTERNET_SERVICE_HTTP,
        0,
        0
    );
    if (NULL == hInetConnect)
        return false;

    //Set timeout options
    bRetCode = InternetSetOptionA(hInetConnect,
        INTERNET_OPTION_CONNECT_TIMEOUT,
        &m_ulConnTimeout,
        sizeof(unsigned long)
    );
    if(TRUE != bRetCode)
        return false;

    bRetCode = InternetSetOptionA(hInetConnect,
        INTERNET_OPTION_SEND_TIMEOUT,
        &m_ulSendTimeout,
        sizeof(unsigned long)
        );
    if(TRUE != bRetCode)
        return false;

    bRetCode = InternetSetOptionA(hInetConnect,
        INTERNET_OPTION_RECEIVE_TIMEOUT,
        &m_ulRecvTimeout,
        sizeof(unsigned long)
        );
    if(TRUE != bRetCode)
        return false;

    //Set proxy options
    INTERNET_PROXY_INFO stProxy;
    char szProxy[cnProxyStrMaxLen] = {0};
    ZeroMemory(&stProxy, sizeof(INTERNET_PROXY_INFO));

    switch(m_stProxy.nProxyMethod)
    {
    case enumPROXY_METHOD_DIRECT:
        stProxy.dwAccessType    = INTERNET_OPEN_TYPE_DIRECT;
        break;
    case enumPROXY_METHOD_USE_IE:
        stProxy.dwAccessType    = INTERNET_OPEN_TYPE_PRECONFIG;
        break;
    case enumPROXY_METHOD_CUSTOM:
        stProxy.dwAccessType    = INTERNET_OPEN_TYPE_PROXY;
        break;
    }

    if(enumPROXY_METHOD_CUSTOM == m_stProxy.nProxyMethod)
    {
        if(enumPROXY_TYPE_HTTP == m_stProxy.nProxyType)
        {
            int nRet = _snprintf(szProxy, 
                cnProxyStrMaxLen,
                "http=http://%s:%d",
                m_stProxy.szProxyAddr,
                m_stProxy.usProxyPort
            );
            if(nRet <= 0 || cnProxyStrMaxLen == nRet)
                return false;
        }
        else
        {
            int nRet = _snprintf(szProxy,
                cnProxyStrMaxLen, 
                "socks=%s:%d",
                m_stProxy.szProxyAddr,
                m_stProxy.usProxyPort
            );
            if(nRet <= 0 || cnProxyStrMaxLen == nRet)
                return false;
        }
    }

    stProxy.lpszProxy       = szProxy;
    stProxy.lpszProxyBypass = NULL;

    bRetCode = InternetSetOptionA(hInetConnect,
        INTERNET_OPTION_PROXY, 
        &stProxy,
        sizeof(INTERNET_PROXY_INFO)
    );
    if (FALSE == bRetCode)
        return false;

    if(enumPROXY_METHOD_CUSTOM == m_stProxy.nProxyMethod)
    {
        bRetCode = InternetSetOptionA(hInetConnect, 
            INTERNET_OPTION_PROXY_USERNAME,
            m_stProxy.szUsername,
            (DWORD)(strlen(m_stProxy.szProxyAddr) + 1)
        );
        if (FALSE == bRetCode)
            return false;

        bRetCode = InternetSetOptionA(hInetConnect,
            INTERNET_OPTION_PROXY_PASSWORD,
            m_stProxy.szPassword,
            (DWORD)(strlen(m_stProxy.szPassword) + 1)
        );
        if (FALSE == bRetCode)
            return false;
    }

    return true;
}

bool CMiniFileDownloader::ParseURL(
    IN  const char*     pszURL, 
    OUT char*           pszServer, 
    IN  unsigned long   ulSSize, 
    OUT char*           pszObject, 
    IN  unsigned long   ulOSize, 
    OUT unsigned short& usPort 
)
{
    if(!pszURL || !pszServer || !pszObject)
        return false;

    char szURL[MAX_PATH * 4] = {0};
    strncpy(szURL, pszURL, MAX_PATH * 4);

    char* pHttp = strstr(szURL, "http://");
    if(NULL == pHttp)
        return false;

    pHttp += strlen("http://");

    char* pObj  = strstr(pHttp, "/");
    if(NULL == pObj)
    {
        strncpy(pszObject, "", 1);
    }
    else
    {
        if(ulOSize <= strlen(pObj))
            return false;
        strncpy(pszObject, pObj, ulOSize);
        *pObj = '\0';
    }

    char* pPort = strstr(pHttp, ":");
    if(NULL == pPort)
    {
        //default HTTP port
        usPort = 80;
    }
    else
    {
        *pPort = '\0';
        ++pPort;
        usPort = (unsigned short)atoi(pPort);
    }

    if(ulSSize <= strlen(pHttp))
        return false;

    strncpy(pszServer, pHttp, ulSSize);
    return TRUE;
}
