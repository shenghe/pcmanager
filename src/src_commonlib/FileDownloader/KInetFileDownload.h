/***************************************************************
 * @date:   2007-10-19
 * @author: BrucePeng
 * @brief:  Define the file downloader object
 */

#ifndef K_INET_FILE_DOWNLOAD_H_
#define K_INET_FILE_DOWNLOAD_H_

#include "KFileDownloadDefine.h"
#include "IFileDownloader.h"
#include "IDownloadThreadCallback.h"

class CFileDownloadThread;
class CDownloadConfigFile;

class CFileDownloader : public IFileDownloader, 
    public IDownloadThreadCallback
{
public:
    CFileDownloader(void);
    virtual ~CFileDownloader(void);

public:
    //inherit from IUnknown
    virtual HRESULT __stdcall QueryInterface( 
        REFIID                      riid,
        void __RPC_FAR *__RPC_FAR*  ppvObject
    );

    virtual ULONG   __stdcall AddRef(void);

    virtual ULONG   __stdcall Release(void);

public:
    //inherit from IFileDownloader
    virtual HRESULT __stdcall SetThreadNumber(
        IN  int             nNumber
    );

    virtual HRESULT __stdcall SetMinBlockSize(
        IN  unsigned long   ulSize
    );

    virtual HRESULT __stdcall SetProxyMethod(
        IN  int             nProxyMethod
    );
    
    virtual HRESULT __stdcall SetProxyInfo(
        IN  int             nProxyType,
        IN  const char*     pszProxyAddr,
        IN  unsigned short  usProxyPort,
        IN  const char*     pszUsername     = NULL,
        IN  const char*     pszPassword     = NULL
    );

    virtual HRESULT __stdcall SetCallback(
        IN  IDownloadCallback* pCallback
    );

    virtual HRESULT __stdcall SetConnectTimeout(
        IN  unsigned long   ulTimeout
    );

    virtual HRESULT __stdcall SetSendTimeout(
        IN  unsigned long   ulTimeout
    );

    virtual HRESULT __stdcall SetRecvTimeout(
        IN  unsigned long   ulTimeout
    );

    virtual HRESULT __stdcall SetTryTime(
        IN  int             nTime 
    );

    virtual HRESULT __stdcall Download(
        IN  const char*     pszDownloadURL,
        IN  const char*     pszLocalFileName,
        IN  bool            bFailIfExists,
        OUT unsigned long&  ulErrorCode
    );

    virtual HRESULT __stdcall NotifyExit(void);

public:
    //inherit from IDownloadThreadCallback
    virtual bool __stdcall IsExitNeeded(void);

    virtual bool __stdcall GetProxyInfo(
        OUT ST_PROXY_INFO&  stProxyInfo
    );

    virtual bool __stdcall GetDownloadUrl(
        OUT char*           pszURL, 
        IN  size_t          size
    );
 
    virtual unsigned long __stdcall GetConnectTimeout(void);
    
    virtual unsigned long __stdcall GetSendTimeout(void);

    virtual unsigned long __stdcall GetRecvTimeout(void);

    virtual ST_FILE_BLOCK_INFO* __stdcall GetFileBlockInfoByID(
        IN  int             nID
    );

public:
    //For receive data
    static size_t MyWriteFunction(
        void*           ptr, 
        size_t          size, 
        size_t          nmemb, 
        void*           stream
    );

    //For Query If range is allowed
    static size_t MyHeaderFunction1(
        void*           ptr, 
        size_t          size, 
        size_t          nmemb, 
        void*           stream
    );
    
    //For Query file size
    static size_t MyHeaderFunction2(
        void*           ptr, 
        size_t          size, 
        size_t          nmemb, 
        void*           stream
    );

    //For progress
    static int MyProgressFunction(
        void*           clientp,
        double          dltotal,
        double          dlnow,
        double          ultotal,
        double          ulnow
    );

private:
    /**
     * @struct st_http_info
     * For storing information of HTTP header 
     */
    typedef struct st_http_info
    {
        bool                bRangeAllowed;
        unsigned long       ulFileLength;
        bool                bQueryFailed;
        CFileDownloader*    pFileDownloader;
    } ST_HTTP_INFO;

    /**
     * @struct st_progress info
     * For storing information of progress
     */
    typedef struct st_progress_info
    {
        T_UNIT8     reserved;    
        T_UNIT32    uStart;
        T_UNIT32    uEnd;
        T_UNIT32    uCompleted;
    }ST_PROGRESS_INFO;

    typedef list<ST_FILE_BLOCK_INFO>    BlockDownloadList;
    
    typedef list<ST_PROGRESS_INFO>      BlockProgressList;

private:
    //DO job, download file
    HRESULT DoDownload(
        IN  const char*     pszDownloadURL, 
        IN  const char*     pszLocalFileName, 
        IN  bool            bFailIfExists, 
        OUT unsigned long&  ulErrorCode
    );
    
    //Save download URL and local path file name
    bool SaveURLAndLocalPathFileName(
        IN  const char* pszDownloadURL,
        IN  const char* pszLocalFileName
    );

    //Query information of the file being downloaded 
    bool QueryInfo(void);

    //Query the length of file
    bool QueryFileLength(IN void* pvCurl);

    //Query If range is supported
    bool QueryIsRangeSupported(IN void* pvCurl);

    //Set curl common options
    bool SetCurlCommonOption(IN void* pvCurl);

    //Perform CURL request asynchronism
    bool PerformCurlRequest(IN void* pvCurl);

    //Compute how many thread will be created
    int  ComputeNumberOfThreadNeeded(void);

    //Check If download succeeded
    bool IsDownloadSucceed(void);

    //Check If download failed
    bool IsDownloadFailed(void);

    //Check If downloading
    bool IsDownloading(void);

    //Get number of threads running
    int  GetThreadsRunning(void);

    //Get block number that not finished
    int  GetBlockNumNotFinished(void);

    //Invoke callback, notify download state
    void NotifyDownloadState(IN FDSCode nState);

    //Invoke callback. notify download progress
    void NotifyDownloadProgress(
        IN  unsigned long   ulFileSize,
        IN  unsigned long   ulDownloadSize,
        IN  int             nSpeed,
        IN  int             nThreadNumber,
        IN  int             nThreadRunning,
        IN  const char*     pszDescription
    );

    //Invoke callback. notify download error
    void NotifyDownloadError(IN FDECode nError);

    //Write progress information to config-file
	void WriteProgressInfoForResume(CDownloadConfigFile& dcFile);

    //Import IE proxy information if needed
    bool ImportIEProxy(void);

    //Read IE proxy information from register
    bool ReadIEProxyFromReg(HKEY& hKeyReg);

    //Query if exit needed
    bool CallbackToSeeIfExitNeeded(void);

private:
    //The number of threads used
    int                 m_nThreadNumber;

    //The min size of a block
    unsigned long       m_ulMinBlockSize;

    //The list records block download information
    BlockDownloadList   m_BlockDownloadList;
    
    //The URL of file downloading
    char                m_szDownloadURL[defURL_MAX_LEN];

    //The file path name of local file
    char                m_szLocalFileName[defPFN_MAX_LEN];
    
    //The proxy information
    ST_PROXY_INFO       m_stProxyInfo;

    //The username && password
    char                m_szUsrPwd[cnProxyStrMaxLen * 2];

    //The length of file being downloaded
    unsigned long       m_ulFileLength;

    //Flag: If range is allowed
    bool                m_bRangeAllowed;

    //Flag: If exit is needed
    bool                m_bExitFlag;

    //The timeout for connecting(seconds)
    unsigned long       m_ulConnectTimeout;

    //The timeout for sending(seconds)
    unsigned long       m_ulSendTimeout;

    //The timeout for receiving(seconds)
    unsigned long       m_ulRecvTimeout;

    //The pointer to callback interface
    IDownloadCallback*  m_pCallback;

    //The try time when download failed
    int                 m_nTryTime;

    //The start time(reserved)
    unsigned long       m_ulStartTickCount;

    //The count of reference(FOR COM)
    unsigned long       m_ulReference;
};

#endif //K_DOWNLOAD_THREAD_H_
