/***************************************************************
* @date:   2007-10-19
* @author: BrucePeng
* @brief:  Define the file CMiniFileDownloader object
*/
#ifndef K_MINI_FILE_DOWNLOADER_H_
#define K_MINI_FILE_DOWNLOADER_H_

#include "IFileDownloader.h"
#include "KFileDownloadDefine.h"

#include <WinInet.h>

class CMiniFileDownloader : public IMiniFileDownloader
{
public:
    CMiniFileDownloader(void);
    virtual ~CMiniFileDownloader(void);

public:
    virtual HRESULT __stdcall SetProxyMethod(
        int                 nProxyMethod
    );

    virtual HRESULT __stdcall SetProxyInfo(
        IN  int             nProxyType,
        IN  const char*     pszProxyAddr,
        IN  unsigned short  usProxyPort,
        IN  const char*     pszUsername     = NULL,
        IN  const char*     pszPassword     = NULL
    );

    virtual HRESULT __stdcall SetCallback(
        IN  IDownloadCallback*  pCallback
    );

    virtual HRESULT __stdcall SetConnectTimeout(
        IN unsigned long    ulTimeout
    );

    virtual HRESULT __stdcall SetSendTimeout(
        IN unsigned long    ulTimeout
    );

    virtual HRESULT __stdcall SetRecvTimeout(
        IN unsigned long    ulTimeout
    );

    virtual HRESULT __stdcall Download(
        IN  const char*     pszDownloadURL,
        IN  const char*     pszLocalFileName,
        IN  bool            bFailIfExists,
        OUT unsigned long&  ulErrorCode
    );

    virtual HRESULT __stdcall NotifyExit(void);

private:
    bool DoInternetConnect(
        IN  const char*     pszServer,
        IN  unsigned short  usPort,
        OUT HINTERNET&      hInetOpen,
        OUT HINTERNET&      hInetConnect
    );

    bool ParseURL(
        IN  const char*     pszURL, 
        OUT char*           pszServer,
        IN  unsigned long   ulSSize,
        OUT char*           pszObject,
        IN  unsigned long   ulOSize,
        OUT unsigned short& usPort 
    );

private:
    //The proxy information
    ST_PROXY_INFO       m_stProxy;

    //The callback interface
    IDownloadCallback*  m_pCallback;

    //The timeout for connecting
    unsigned long       m_ulConnTimeout;

    //The timeout for sending data
    unsigned long       m_ulSendTimeout;
    
    //The timeout for receiving data
    unsigned long       m_ulRecvTimeout;

    //The Flag: If exit needed
    bool                m_bExitFlag;
    
    //The URL of file being downloaded
    char                m_szDownloadURL[defURL_MAX_LEN];

    //The file path name of local file
    char                m_szLocalFileName[defPFN_MAX_LEN];

    //The tick count when start
    unsigned long       m_ulStartTickCount;

    //The length of file being downloaded
    unsigned long       m_ulFileLength;
};

#endif//K_MINI_FILE_DOWNLOADER_H_
