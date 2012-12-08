/***************************************************************
 * @date:   2007-10-19
 * @author: BrucePeng
 * @brief:  Define the file downloader object
 */

#include "KInetFileDownload.h"
#include "KDownloadFile.h"
#include "KDebugInfoRecord.h"
#include "KDownloadConfigFile.h"
#include "KFileDownloadThread.h"
#include "KFileDownloadCommon.h"
#include "curl/curl.h"
#include <shlwapi.h>
#include <assert.h>

//The default timeout(seconds)
const int cnDefaultTimeOut              = 0x0A;

//The default timeout for connecting(seconds)
const int cnDefaultConnTimeOut          = cnDefaultTimeOut;

//The default timeout for sending
const int cnDefaultSendTimeout          = cnDefaultTimeOut;

//The default timeout for receiving
const int cnDefaultRecvTimeout          = cnDefaultTimeOut;

//The default number of thread used 
const int cnDefaultThreadNumber         = 0x01;

//The default min size of a file block(100K)
const int cnDefaultBlockMinSize         = 0x0400 * 100;

//The max try time while downloading failed
const int cnDownloadMaxTryTime          = 0x0A;

//The max try time while querying file info failed
const int cnQueryInfoTryTime            = 0x0A;

//The default try time
const int cnDefaultTryTime              = 0x02;

//The default interval(5000 in millisecond)
const unsigned long culDefaultInterval  = 1000 * 5;

//The location of internet settings in register
const char* const pcszRegInetSetting    
    = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";

CFileDownloader::CFileDownloader(void)
: m_nThreadNumber(cnDefaultThreadNumber),
  m_ulMinBlockSize(cnDefaultBlockMinSize),
  m_ulFileLength(0),
  m_bRangeAllowed(false),
  m_bExitFlag(false),
  m_ulConnectTimeout(cnDefaultConnTimeOut),
  m_ulSendTimeout(cnDefaultSendTimeout),
  m_ulRecvTimeout(cnDefaultRecvTimeout),
  m_pCallback(NULL),
  m_nTryTime(cnDefaultTryTime),
  m_ulStartTickCount(0),
  m_ulReference(0)
{
    ZeroMemory(m_szDownloadURL, MAX_PATH);
    ZeroMemory(m_szLocalFileName, MAX_PATH);
    ZeroMemory(&m_stProxyInfo, sizeof(ST_PROXY_INFO));
    m_stProxyInfo.nProxyMethod  = enumPROXY_METHOD_DIRECT;
}

CFileDownloader::~CFileDownloader(void){}

ULONG __stdcall CFileDownloader::AddRef()
{
    return ++m_ulReference;
}

ULONG __stdcall CFileDownloader::Release()
{
    if(--m_ulReference) return m_ulReference;
    delete this;
    return 0;
}

HRESULT __stdcall CFileDownloader::QueryInterface(
    REFIID                  riid, 
    void __RPC_FAR *__RPC_FAR   *ppvObject
)
{
    *ppvObject = NULL;

    if(IsEqualIID(riid, __uuidof(IUnknown)))
    {
        *ppvObject = (IUnknown* )(IFileDownloader* )this;
    }
    if(IsEqualIID(riid, __uuidof(IFileDownloader)))
    {
        *ppvObject = (IFileDownloader* )this;
    }

    if(!(*ppvObject))
    {
        return E_NOINTERFACE;
    }

    (*(IUnknown** )ppvObject)->AddRef();

    return S_OK;
}

HRESULT __stdcall CFileDownloader::SetThreadNumber(IN int nNumber)
{
    m_nThreadNumber     = nNumber;
    return S_OK;
}

HRESULT __stdcall CFileDownloader::SetMinBlockSize(IN unsigned long ulSize)
{
    m_ulMinBlockSize    = ulSize;
    return S_OK;
}

HRESULT __stdcall CFileDownloader::SetProxyMethod(IN int nProxyMethod)
{
    if(enumPROXY_METHOD_DIRECT != nProxyMethod
        && enumPROXY_METHOD_USE_IE != nProxyMethod
        && enumPROXY_METHOD_CUSTOM != nProxyMethod)
    {
        return E_FAIL;
    }

    m_stProxyInfo.nProxyMethod  = nProxyMethod;
    return S_OK;
}

HRESULT __stdcall CFileDownloader::SetProxyInfo(
    IN  int              nProxyType, 
    IN  const char*      pszProxyAddr, 
    IN  unsigned short   usProxyPort, 
    IN  const char*      pszUsername /* = NULL */, 
    IN  const char*      pszPassword /* = NULL  */
)
{
    HRESULT hrRetCode  = E_FAIL;

    //Check pointer
    if(NULL == pszProxyAddr)
    {
        assert(false);
        goto Exit0;
    }

    //Then, try to copy information and save.
    ZeroMemory(&m_stProxyInfo, sizeof(ST_PROXY_INFO));

    m_stProxyInfo.nProxyType    = nProxyType;
    m_stProxyInfo.usProxyPort   = usProxyPort;
    
    if(strlen(pszProxyAddr) >= cnProxyStrMaxLen)
    {
        goto Exit0;
    }

    strncpy(m_stProxyInfo.szProxyAddr, pszProxyAddr, cnProxyStrMaxLen);

    if(NULL != pszUsername && NULL != pszPassword)
    {
        if(strlen(pszUsername) >= cnProxyStrMaxLen
            ||strlen(pszPassword) >= cnProxyStrMaxLen)
        {
            goto Exit0;
        }
        strncpy(m_stProxyInfo.szUsername, pszUsername, cnProxyStrMaxLen);
        strncpy(m_stProxyInfo.szPassword, pszPassword, cnProxyStrMaxLen);
    }

    hrRetCode = S_OK;
Exit0:
    if(E_FAIL == hrRetCode)
    {
        ZeroMemory(&m_stProxyInfo, sizeof(ST_PROXY_INFO));
    }
    return hrRetCode;
}

HRESULT __stdcall CFileDownloader::SetCallback(IN IDownloadCallback* pCallback)
{
    m_pCallback = pCallback;
    return S_OK;
}

HRESULT __stdcall CFileDownloader::SetConnectTimeout(IN unsigned long ulTimeout)
{
    m_ulConnectTimeout = ulTimeout;
    return S_OK;
}

HRESULT __stdcall CFileDownloader::SetSendTimeout(IN unsigned long ulTimeout)
{
    m_ulSendTimeout = ulTimeout;
    return S_OK;
}

HRESULT __stdcall CFileDownloader::SetRecvTimeout(IN unsigned long ulTimeout)
{
    m_ulRecvTimeout = ulTimeout;
    return S_OK;
}

HRESULT __stdcall CFileDownloader::SetTryTime(IN int nTime)
{
    m_nTryTime = nTime;
    return S_OK;
}

HRESULT __stdcall CFileDownloader::Download(//12345677654321
    IN  const char*     pszDownloadURL, 
    IN  const char*     pszLocalFileName, 
    IN  bool            bFailIfExists, 
    OUT unsigned long&  ulErrorCode
)
{
    return DoDownload(pszDownloadURL, 
        pszLocalFileName, 
        bFailIfExists,
        ulErrorCode
    );
}

HRESULT __stdcall CFileDownloader::NotifyExit(void)
{
    m_bExitFlag = true;
    return S_OK;
}

bool __stdcall CFileDownloader::IsExitNeeded(void)
{
    return m_bExitFlag;
}

bool __stdcall CFileDownloader::GetProxyInfo(ST_PROXY_INFO& stProxyInfo)
{
    memcpy(&stProxyInfo, &m_stProxyInfo, sizeof(ST_PROXY_INFO));
    return true;
}

bool __stdcall CFileDownloader::GetDownloadUrl(char* pszURL, size_t size)
{
    if(NULL == pszURL)
    {
        assert(false);
        return false;
    }

    if(strlen(m_szDownloadURL) >= size)
    {
        assert(false);
        return false;
    }

    strncpy(pszURL, m_szDownloadURL, size);
    return true;
}

unsigned long __stdcall CFileDownloader::GetConnectTimeout(void)
{
    return m_ulConnectTimeout;
}

unsigned long __stdcall CFileDownloader::GetSendTimeout(void)
{
    return m_ulSendTimeout;
}

unsigned long __stdcall CFileDownloader::GetRecvTimeout(void)
{
    return m_ulRecvTimeout;
}

ST_FILE_BLOCK_INFO* __stdcall CFileDownloader::GetFileBlockInfoByID(int nID)
{
    BlockDownloadList::iterator it = m_BlockDownloadList.begin();

    for(; it != m_BlockDownloadList.end(); ++it)
    {
        if((it->pThread->GetUserDefinedID()) == nID)
            return &(*it);
    }

    return NULL;
}

size_t CFileDownloader::MyWriteFunction(
    void*           ptr, 
    size_t          size, 
    size_t          nmemb, 
    void*           stream
)
{
    if(NULL == stream || NULL == ptr)
    {
        assert(false);
        return size * nmemb;
    }

    ST_FILE_BLOCK_INFO* pBlock = (ST_FILE_BLOCK_INFO* )stream;
    if(NULL == pBlock)
    {
        assert(false);
        return size * nmemb;
    }

    if(!pBlock->pLocalFile->Write(ptr, (DWORD)(size * nmemb)))
    {
        RecordWhatHappened("error", 
            "write file failed, so exit", 
            pBlock->pThread->GetUserDefinedID()
        );
        return (size * nmemb) - 1;
    }

    pBlock->ulCompleted += (unsigned long)(size * nmemb);

    if(pBlock->pDownloader->IsExitNeeded())
    {
        return (size * nmemb) - 1;
    }

    return size * nmemb;
}

size_t CFileDownloader::MyHeaderFunction1(
    void*           ptr,
    size_t          size,
    size_t          nmemb,
    void*           stream
)
{
    if(NULL == stream || NULL == ptr)
    {
        return size * nmemb;
    }

    ST_HTTP_INFO* pHttpInfo = (ST_HTTP_INFO* )stream;
    if(NULL == pHttpInfo)
    {
        return size * nmemb;
    }

    //Check Exit
    if(NULL != pHttpInfo->pFileDownloader)
    {
        if(pHttpInfo->pFileDownloader->IsExitNeeded())
        {
            return -1;
        }
    }
    
    //Check range support
    char* pos = strstr((char* )ptr, "HTTP/");
    if(NULL != pos)
    {
        char* pos1 = strstr((char* )ptr, cszPartialContent);
        if(NULL != pos1)
        {
            pHttpInfo->bRangeAllowed = true;
        }
        else
        {
            pHttpInfo->bRangeAllowed = false;

            char szInfo[MAX_PATH] = {0};
            _snprintf(szInfo, MAX_PATH, 
                "not 206, return from server: %s",
                pos
            );
            RecordWhatHappened("error", szInfo);
        }
    }

    //Check file size if any
    {
        char* pos = strstr((char* )ptr, "Content-Range: ");
        if(NULL != pos)
        {
            char* pos1 = strstr((char* )ptr, "0-0/");
            if(NULL != pos1)
            {
                pos1 += strlen("0-0/");
                char* pos2 = strstr(pos1, "\r\n");
                if(NULL != pos2)
                {
                    *pos2 = '\0';
                    pHttpInfo->ulFileLength = atoi(pos1);
                }
            }
        }
    }

    //Check Exit
    if(NULL != pHttpInfo->pFileDownloader)
    {
        if(pHttpInfo->pFileDownloader->IsExitNeeded())
        {
            return -1;
        }
    }

    return size * nmemb;
}

size_t CFileDownloader::MyHeaderFunction2(
    void*           ptr,
    size_t          size,
    size_t          nmemb,
    void*           stream
)
{
    if(NULL == stream || NULL == ptr)
    {
        return size * nmemb;
    }

    ST_HTTP_INFO* pHttpInfo = (ST_HTTP_INFO* )stream;
    if(NULL == pHttpInfo)
    {
        return size * nmemb;
    }

    //Check Exit
    if(NULL != pHttpInfo->pFileDownloader)
    {
        if(pHttpInfo->pFileDownloader->IsExitNeeded())
        {
            return -1;
        }
    }

    if(pHttpInfo->bQueryFailed)
    {
        char* posPre1 = strstr((char* )ptr, "HTTP/");

        if(NULL != posPre1)
        {
            char* posPre2 = strstr((char* )ptr, "200 OK");
            if(NULL != posPre2)
            {
                pHttpInfo->bQueryFailed = false;
            }
            else
            {
                pHttpInfo->bQueryFailed = true;
                char szInfo[MAX_PATH] = {0};
                _snprintf(szInfo, MAX_PATH, 
                    "not 200, return from server: %s",
                    posPre1
                );
                RecordWhatHappened("error", szInfo);
            }
        }
    }

    //Check Exit
    if(NULL != pHttpInfo->pFileDownloader)
    {
        if(pHttpInfo->pFileDownloader->IsExitNeeded())
        {
            return -1;
        }
    }

    char* pos = strstr((char* )ptr, cszContentLength);
    if(NULL == pos)
    {
        return size * nmemb;
    }

    pos += strlen(cszContentLength);

    char* pos1 = strstr(pos, "\r\n");
    if(NULL == pos)
        return size * nmemb;

    *pos1 = '\0';

    pHttpInfo->ulFileLength = atoi(pos);

    return size * nmemb;
}

int  CFileDownloader::MyProgressFunction(
    void*               clientp, 
    double              dltotal, 
    double              dlnow, 
    double              ultotal, 
    double              ulnow 
)
{
    CFileDownloader* pDownloader = (CFileDownloader* )clientp;
    if(NULL == pDownloader)
        return 0;

    if(pDownloader->IsExitNeeded())
        return 1;

    return 0;
}

HRESULT CFileDownloader::DoDownload(
    IN  const char*     pszDownloadURL, 
    IN  const char*     pszLocalFileName, 
    IN  bool            bFailIfExists, 
    OUT unsigned long&  ulErrorCode 
)
{
    HRESULT hrReturn                            = E_FAIL;
    int     nRet                                = 0;
    int     nRealThreadNumber                   = 0;
    char    szTempLocalFileName[defPFN_MAX_LEN] = {0};
    char    szConfigFileName[defPFN_MAX_LEN]    = {0};
    CDownloadConfigFile                         dcFile;
    ST_DC_INFO                                  stDCInfo;
    BlockDownloadList::iterator                 it;
    ST_FILE_BLOCK_INFO                          stBlockInfo;

    ulErrorCode = enumDE_NORMAL;

    //Check pointer
    if(!pszDownloadURL || !pszLocalFileName)
    {
        assert(false);
        goto Exit0;
    }

    RecordWhatHappened("info ", "====>Start a new Download========>");
    RecordWhatHappened("URL  ", pszDownloadURL);

    m_ulStartTickCount = GetTickCount();

    //Check URL for protocol support
    if(strncmp(pszDownloadURL, cszHttp, strlen(cszHttp))
        && strncmp(pszDownloadURL, cszHTTP, strlen(cszHTTP)))
    {
        RecordWhatHappened("error", "protocol not support");
        ulErrorCode = enumDE_PROTOCOL_NOT_SUPPORT;
        goto Exit0;
    }

    //copy URL and local path file name
    if(!SaveURLAndLocalPathFileName(pszDownloadURL, pszLocalFileName))
    {
        RecordWhatHappened("error", "save URL & path file name failed");
        goto Exit0;
    }

    //Check file exists
    if(bFailIfExists && PathFileExistsA(pszLocalFileName))
    {
        RecordWhatHappened("error", "The file to be downloaded exists");
        ulErrorCode = enumDE_FILE_EXISTS;
        goto Exit0;
    }

    //Notify client before querying file information
    NotifyDownloadState(enumDS_QUERYING_INFO);

    //Import IE proxy information If needed
    if(enumPROXY_METHOD_USE_IE == m_stProxyInfo.nProxyMethod)
    {
        m_stProxyInfo.bProxyEnable  = false;

        if(!ImportIEProxy())
        {
            RecordWhatHappened("error", "Import IE's Proxy failed");
        }
    }

    //Query file information
    for(int i = 1; i <= m_nTryTime; ++i)
    {
        char szInfo[MAX_PATH] = {0};
        _snprintf(szInfo, MAX_PATH, "Query file info the %.2d time", i);

        RecordWhatHappened("info ", szInfo);

        if(QueryInfo())
            break;
        
        RecordWhatHappened("error", "Query file info failed!");

        //If exit is needed, notify, then exit
        if(m_bExitFlag)
        {
            NotifyDownloadState(enumDS_USER_STOP);
            //OutputDebugStringA("User stop");
            goto Exit0;
        }

        if(i == m_nTryTime)
        {
            ulErrorCode = enumDE_QUERY_INFO;
            goto Exit0;
        }

        Sleep(100);//Milliseconds
    }

    //Callback
    NotifyDownloadProgress(m_ulFileLength,
        0,
        0,
        m_nThreadNumber,
        0,
        ""
    );

    NotifyDownloadState(enumDS_CREATING_LOCAL_FILE);

    //Create temp file name
    nRet = _snprintf(szTempLocalFileName, 
        defPFN_MAX_LEN, 
        "%s.fd", 
        pszLocalFileName
    );
    if(0 >= nRet || defPFN_MAX_LEN == nRet)
    {
        RecordWhatHappened("error", "create temp file name failed");
        goto Exit0;
    }

    //Create local configuration file
    nRet = _snprintf(szConfigFileName, 
        defPFN_MAX_LEN, 
        "%s.fd.cfg", 
        pszLocalFileName
    );
    if(0 >= nRet || defPFN_MAX_LEN == nRet)
    {
        RecordWhatHappened("error", "create config file name failed");
        goto Exit0;
    }

    //Create/open local config-file
    if(!dcFile.Create(szConfigFileName))
    {
        RecordWhatHappened("error", "create config file failed");
        ulErrorCode = enumDE_CREATE_CONFIG_FILE;
        goto Exit0;
    }

    //If local file exists, delete it
    if(PathFileExistsA(pszLocalFileName) && 0 != remove(pszLocalFileName))
    {
        RecordWhatHappened("error", "remove file exists failed");
    }

    if(PathFileExistsA(szTempLocalFileName) && dcFile.ReadConfigInfo(stDCInfo)
        && !strcmp(pszDownloadURL, stDCInfo.szDownloadURL)
        && m_ulFileLength == stDCInfo.uFileLength)
    {
        //Resume downloading
        nRealThreadNumber = stDCInfo.uBlockNumber;

        int i = 0;
        BPInfoList::iterator it = stDCInfo.BPInfolist.begin();
        for(; it != stDCInfo.BPInfolist.end(); ++it)
        {
            ++i;

            ZeroMemory(&stBlockInfo, sizeof(ST_FILE_BLOCK_INFO));

            stBlockInfo.nBlockID    = it->uID;
            stBlockInfo.ulSeek      = it->uStart;
            stBlockInfo.ulEnd       = it->uEnd;
            stBlockInfo.ulCompleted = it->uCompleted;

            stBlockInfo.pLocalFile  = new (nothrow) CDownloadFile();
            if(NULL == stBlockInfo.pLocalFile)
                goto DeleteObjectAndGotoExit0A;

            if(!stBlockInfo.pLocalFile->Create(szTempLocalFileName))
            {
                ulErrorCode = enumDE_CREATE_LOCAL_FILE;
                goto DeleteObjectAndGotoExit0A;
            }

            if(!stBlockInfo.pLocalFile->SetPointerFromBegin(
                it->uStart + it->uCompleted))
            {
                goto DeleteObjectAndGotoExit0A;
            }

            stBlockInfo.pThread = new CFileDownloadThread(it->uID, this);
            if(NULL == stBlockInfo.pThread)
            {
                goto DeleteObjectAndGotoExit0A;
            }

            stBlockInfo.bRangeNotSupport = (!m_bRangeAllowed || 1 == nRealThreadNumber);
            stBlockInfo.pDownloader = this;
            stBlockInfo.nTryCount   = 0;
            stBlockInfo.nMaxTryNum  = m_nTryTime + i;

            m_BlockDownloadList.push_back(stBlockInfo);

            continue;
DeleteObjectAndGotoExit0A:
            //@^_^@, it's so BT here
            //Consider use auto pointer to avoid this
            if(NULL != stBlockInfo.pLocalFile)
            {
                delete stBlockInfo.pLocalFile;
            }
            if(NULL != stBlockInfo.pThread)
            {
                delete stBlockInfo.pThread;
            }
            goto Exit0;
        }
    }
    else
    {
        //New downloading
        nRealThreadNumber = ComputeNumberOfThreadNeeded();
        unsigned long ulBlockSize = (m_ulFileLength / nRealThreadNumber);

        dcFile.WriteConfigHeadInfo(pszDownloadURL,
            pszLocalFileName,
            m_ulFileLength,
            nRealThreadNumber
        );

        for(int i = 1; i <= nRealThreadNumber; ++i)
        {
            ZeroMemory(&stBlockInfo, sizeof(ST_FILE_BLOCK_INFO));

            stBlockInfo.nBlockID    = i;
            stBlockInfo.ulSeek      = (i -1) * ulBlockSize;

            if(i == nRealThreadNumber)
                stBlockInfo.ulEnd   = m_ulFileLength;
            else
                stBlockInfo.ulEnd   = i* ulBlockSize;

            stBlockInfo.ulCompleted = 0;

            stBlockInfo.pLocalFile  = new (nothrow) CDownloadFile();
            if(NULL == stBlockInfo.pLocalFile)
                goto DeleteObjectAndGotoExit0B;

            if(!stBlockInfo.pLocalFile->Create(szTempLocalFileName))
            {
                ulErrorCode = enumDE_CREATE_LOCAL_FILE;
                goto DeleteObjectAndGotoExit0B;
            }

            if(!stBlockInfo.pLocalFile->Empty())
            {
                goto DeleteObjectAndGotoExit0B;
            }

            if(!stBlockInfo.pLocalFile->SetPointerFromBegin(stBlockInfo.ulSeek))
            {
                goto DeleteObjectAndGotoExit0B;
            }

            stBlockInfo.pThread     = new CFileDownloadThread(i, this);
            if(NULL == stBlockInfo.pThread)
            {
                goto DeleteObjectAndGotoExit0B;
            }

            stBlockInfo.bRangeNotSupport = (!m_bRangeAllowed || 1 == nRealThreadNumber);
            stBlockInfo.pDownloader = this;
            stBlockInfo.nTryCount   = 0;
            stBlockInfo.nMaxTryNum  = m_nTryTime + i;

            m_BlockDownloadList.push_back(stBlockInfo);

            continue;
DeleteObjectAndGotoExit0B:
            //@^_^@, it's so BT here
            //Consider use auto pointer to avoid this
            if(NULL != stBlockInfo.pLocalFile)
            {
                delete stBlockInfo.pLocalFile;
            }
            if(NULL != stBlockInfo.pThread)
            {
                delete stBlockInfo.pThread;
            }
            goto Exit0;
        }
    }

    //终于开始下载了，不容易啊
    //Start download thread

    //call back first
    NotifyDownloadState(enumDS_DOWNLOADING);

    unsigned long ulSleep = 0;
    it = m_BlockDownloadList.begin();
    for(; it != m_BlockDownloadList.end(); ++it)
    {
        it->ulLastStopTime  = GetTickCount();
        (it->pThread)->SetSleepTimeBeforeStart(ulSleep);

        if(!(it->pThread)->Initialize())
        {
            RecordWhatHappened("error", 
                "thread init failed", 
                it->pThread->GetUserDefinedID()
            );
            goto Exit0;
        }

        (it->pThread)->Start();
        ulSleep += 100;
    }

    //Record something
    {
        char szInfo[MAX_PATH] = {0};
        _snprintf(szInfo, MAX_PATH, 
            "Start downloading, TN:%d, size:%d",
            nRealThreadNumber,
            m_ulFileLength
        );
        RecordWhatHappened("info ", szInfo);
    }

    //Callback
    NotifyDownloadProgress(m_ulFileLength,
        0,
        0,
        nRealThreadNumber,
        nRealThreadNumber,
        ""
    );

    //Wait for stop
    unsigned long   ulLastDownloadSize  = 0;
    unsigned long   ulLastTickCount     = GetTickCount();
    unsigned long   ulLastSpeed         = 0;

    while("I believe I can fly")
    {
        if(IsDownloadSucceed())
        {
            WriteProgressInfoForResume(dcFile);
            break;
        }

        //If download failed, exit
        if(IsDownloadFailed())
            goto Exit0;

        bool bDownloading = IsDownloading();

        //Do something here
        unsigned long   ulDownloadSize = 0;
        it = m_BlockDownloadList.begin();

        //Reset sleep time
        ulSleep = 0;

        for(; it != m_BlockDownloadList.end(); ++it)
        {
            //Compute total download size
            ulDownloadSize  += it->ulCompleted;

            if(!bDownloading)
            {
                //If downloading stops, do something to repair
                if(it->ulSeek + it->ulCompleted < it->ulEnd
                    && it->nTryCount < it->nMaxTryNum)
                {
                    //not finished
                    //try again with try count ++
                    (it->pThread)->SetSleepTimeBeforeStart(ulSleep);
                    it->pThread->Start();
                    ++(it->nTryCount);
                    ulSleep += 100;
                    RecordWhatHappened("info ", 
                        "thread Try again with count++",
                        it->pThread->GetUserDefinedID()
                    );
                }
            }
            else
            {   
                //If downloading does stop, do something to repair
                if(enumTHREADS_FINISHED == it->pThread->GetState())
                {
                    if(it->ulSeek + it->ulCompleted < it->ulEnd &&
                        (GetTickCount() - it->ulLastStopTime) > culDefaultInterval)
                    {
                        //Try again, but not try count ++
                        (it->pThread)->SetSleepTimeBeforeStart(0);
                        it->pThread->Start();
                        RecordWhatHappened("info ", 
                            "thread Try again without count++",
                            it->pThread->GetUserDefinedID()
                        );
                    }
                }
            }
        }

        //Write progress information for resuming
        WriteProgressInfoForResume(dcFile);

        //Compute speed here(bytes/second)
        unsigned long ulTimeUsed = GetTickCount() - ulLastTickCount;
        ulTimeUsed = (0 == ulTimeUsed) ? 1 : ulTimeUsed;
        unsigned long ulSpeed = (ulDownloadSize - ulLastDownloadSize) * 1000 / ulTimeUsed;
        ulSpeed = (ulSpeed * 3) / 4 + ulLastSpeed / 4;

        //Notify client the progress
        if(0 != ulLastDownloadSize)
        {
            ulLastSpeed = ulSpeed;

            NotifyDownloadProgress(m_ulFileLength,
                ulDownloadSize,
                ulSpeed,
                nRealThreadNumber,
                GetThreadsRunning(),
                ""
            );
        }

        //Check If exit is needed
        if(CallbackToSeeIfExitNeeded())
        {
            m_bExitFlag = true;
        }

        //If exit is needed, notify, then exit
        if(m_bExitFlag)
        {
            NotifyDownloadState(enumDS_USER_STOP);
            goto Exit0;
        }

        ulLastDownloadSize  = ulDownloadSize;
        ulLastTickCount     = GetTickCount();

        //sleep one second / 2
        Sleep(500);
    }

    //Download successfully, callback to notify client
    NotifyDownloadState(enumDS_SUCCESS);
    ulErrorCode = enumDE_SUCCEEDED;
    RecordWhatHappened("info ", "@^_^@download successful@^_^@");

    hrReturn = S_OK;
Exit0:
    if(IsExitNeeded())
    {
        RecordWhatHappened("info ", "Download exit with User Stop");
    }

    //Record time spend
    if(S_OK == hrReturn)
    {
        char szInfo[MAX_PATH] = {0};
        _snprintf(szInfo, 
            MAX_PATH, 
            "<<time spend: %d milliseconds>>", 
            (GetTickCount() - m_ulStartTickCount)
        );
        RecordWhatHappened("info ", szInfo);
    }

    it = m_BlockDownloadList.begin();
    for(; it != m_BlockDownloadList.end(); ++it)
    {
        (it->pThread)->Stop();
        (it->pThread)->Uninitialize();
        delete (it->pThread);
        (it->pLocalFile)->Close();
        delete (it->pLocalFile);
    }

    dcFile.Close();

    if(S_OK == hrReturn)
    {
        //Rename local file
        rename(szTempLocalFileName, pszLocalFileName);

        //Delete config-file
        remove(szConfigFileName);
    }

    m_BlockDownloadList.clear();
    m_bExitFlag     = false;
    m_ulFileLength  = 0;
    m_bRangeAllowed = false;

    if(S_OK != hrReturn)
    {
        RecordWhatHappened("info ", "@!_!@download failed@!_!@");
        NotifyDownloadState(enumDS_FAILED);
    }

    return hrReturn;
}

bool CFileDownloader::SaveURLAndLocalPathFileName(
    const char*     pszDownloadURL, 
    const char*     pszLocalFileName 
)
{
    if(!pszDownloadURL || !pszLocalFileName)
    {
        assert(false);
        return false;
    }

    if(strlen(pszDownloadURL) >= defURL_MAX_LEN)
        return false;

    if(strlen(pszLocalFileName) >= defPFN_MAX_LEN)
        return false;

    strncpy(m_szDownloadURL, pszDownloadURL, defURL_MAX_LEN);
    strncpy(m_szLocalFileName, pszLocalFileName, defPFN_MAX_LEN);

    return true;
}

bool CFileDownloader::QueryInfo(void)
{
    bool            bResult         = false;
    CURL*           pCurl           = NULL;
    bool            bGlobalInited   = true;
    ST_HTTP_INFO    stInfo;
    struct curl_slist*  pstHeaders  = NULL;

    if(CURLE_OK != curl_global_init(CURL_GLOBAL_WIN32))
    {
        bGlobalInited = false;
        goto Exit0;
    }

    pCurl = curl_easy_init();
    if(NULL == pCurl)
        goto Exit0;

    if(CURLE_OK != curl_easy_setopt(pCurl, CURLOPT_HEADERDATA, &stInfo))
        goto Exit0;

    //Add HTTP header
    if(SetOurHttpHeader(m_szDownloadURL, &pstHeaders))
    {
        if(CURLE_OK != curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pstHeaders))
            goto Exit0;
    }

    //Set common options
    if(!SetCurlCommonOption(pCurl))
        goto Exit0;

    //Enable auto redirect
    if(!EnableAutoRedirect(pCurl, 256, true))
        goto Exit0;

    //Initialize
    stInfo.ulFileLength     = 0;
    stInfo.bRangeAllowed    = false;
    stInfo.bQueryFailed     = true;
    stInfo.pFileDownloader  = this;

    //Query If range support
    if(1 < m_nThreadNumber && false == m_bRangeAllowed)
    {
        if(!QueryIsRangeSupported(pCurl))
        {
            RecordWhatHappened("error", "query range failed");
        }
        else
        {
            m_bRangeAllowed = stInfo.bRangeAllowed;
        }
    }

    if(IsExitNeeded())
        goto Exit0;

    //Query the file length of file
    if(0 == stInfo.ulFileLength)
    {
        if(!QueryFileLength(pCurl))
        {
            RecordWhatHappened("error", "query length failed");
            goto Exit0;
        }

        if(stInfo.bQueryFailed)
            goto Exit0;
    }

    if(0 == stInfo.ulFileLength)
        goto Exit0;

    m_ulFileLength  = stInfo.ulFileLength;

    bResult = true;
Exit0:
    if(NULL != pCurl)
    {
        curl_easy_cleanup(pCurl);
    }
    if(bGlobalInited)
    {
        curl_global_cleanup();
    }
    if(NULL != pstHeaders)
    {
        curl_slist_free_all(pstHeaders);
    }
    
    return bResult;
}

bool CFileDownloader::QueryFileLength(IN void* pvCurl)
{
    CURLcode    nCurlCode       = CURLE_OK;

    CURL* pCurl = (CURL* )pvCurl;
    if(NULL == pCurl)
        return false;

    nCurlCode = curl_easy_setopt(pCurl, 
        CURLOPT_HEADERFUNCTION, 
        MyHeaderFunction2
    );
    if(CURLE_OK != nCurlCode)
        return false;

    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_NOBODY,
        1
    );
    if(CURLE_OK != nCurlCode)
        return false;

    //Disable range
    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_RANGE,
        NULL
    );
    if(CURLE_OK != nCurlCode)
        return false;

#ifdef USE_ASYNCHRONISM_CURL
    if(!PerformCurlRequest(pCurl))
        return false;
#else
    nCurlCode = curl_easy_perform(pCurl);
    if(CURLE_OK != nCurlCode)
    {
        //Log error here please
        char szError[MAX_PATH] = {0};
        _snprintf(szError, 
            MAX_PATH, 
            "perform return %3d when query length",
            nCurlCode
        );
        RecordWhatHappened("error", szError);
        return false;
    }
#endif
        
    return true;
}

bool CFileDownloader::QueryIsRangeSupported(IN void* pvCurl)
{
    CURLcode    nCurlCode   = CURLE_OK;

    CURL* pCurl = (CURL* )pvCurl;
    if(NULL == pCurl)
        return false;

    nCurlCode = curl_easy_setopt(pCurl, 
        CURLOPT_HEADERFUNCTION, 
        MyHeaderFunction1
    );
    if(CURLE_OK != nCurlCode)
        return false;

    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_NOBODY,
        0
    );
    if(CURLE_OK != nCurlCode)
        return false;

    if(CURLE_OK != curl_easy_setopt(pCurl, CURLOPT_RANGE, "0-0"))
        return false;

#ifdef USE_ASYNCHRONISM_CURL
    if(!PerformCurlRequest(pCurl))
        return false;
#else
    nCurlCode = curl_easy_perform(pCurl);
    if(CURLE_OK != nCurlCode)
    {
        //Log error here please
        char szError[MAX_PATH] = {0};
        _snprintf(szError, 
            MAX_PATH, 
            "perform return %3d when query range",
            nCurlCode
        );
        RecordWhatHappened("error", szError);
        return false;
    }
#endif

    return true;
}

bool CFileDownloader::SetCurlCommonOption(IN void* pvCurl)
{
    CURLcode nCurlCode  = CURLE_OK;

    CURL* pCurl = (CURL* )pvCurl;
    if(NULL == pCurl)
        return false;

    //Set option "URL"
    nCurlCode = curl_easy_setopt(pCurl, 
        CURLOPT_URL, 
        m_szDownloadURL
    );
    if(CURLE_OK != nCurlCode)
        return false;

    //Set progress function
    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_NOPROGRESS,
        FALSE
    );
    if(CURLE_OK != nCurlCode)
        return false;

    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_PROGRESSFUNCTION,
        MyProgressFunction
    );
    if(CURLE_OK != nCurlCode)
        return false;

    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_PROGRESSDATA,
        this
    );
    if(CURLE_OK != nCurlCode)
        return false;

    //Set option "Connect timeout"
    nCurlCode = curl_easy_setopt(pCurl, 
        CURLOPT_CONNECTTIMEOUT, 
        m_ulConnectTimeout
    );
    if(CURLE_OK != nCurlCode)
        return false;

    //Set speed limit
    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_LOW_SPEED_LIMIT,
        1
    );
    if(CURLE_OK != nCurlCode)
        return false;

    //Set speed limit time
    nCurlCode = curl_easy_setopt(pCurl,
        CURLOPT_LOW_SPEED_TIME,
        m_ulRecvTimeout
    );
    if(CURLE_OK != nCurlCode)
        return false;

    //Set proxy
    if(enumPROXY_METHOD_DIRECT == m_stProxyInfo.nProxyMethod)
        return true;

    if(enumPROXY_METHOD_USE_IE == m_stProxyInfo.nProxyMethod
        && false == m_stProxyInfo.bProxyEnable)
    { 
        return true;
    }

    //If proxy is needed, then continue setting options
    int nProxyType = CURLPROXY_HTTP;
    if(enumPROXY_TYPE_SOCK5 == m_stProxyInfo.nProxyType)
    {
        nProxyType  = CURLPROXY_SOCKS5;
    }
    else if(enumPROXY_TYPE_HTTP == m_stProxyInfo.nProxyType)
    {
        nProxyType  = CURLPROXY_HTTP;
    }
    else if(enumPROXY_TYPE_SOCK4 == m_stProxyInfo.nProxyType)
    {
        nProxyType  = CURLPROXY_SOCKS4;
    }
    else
    {
        return false;
    }

    //Set option "proxy type"
    nCurlCode = curl_easy_setopt(pCurl, 
        CURLOPT_PROXYTYPE, 
        nProxyType
    );
    if(CURLE_OK != nCurlCode)
        return false;

    //Set option "proxy address"
    nCurlCode = curl_easy_setopt(pCurl, 
        CURLOPT_PROXY, 
        m_stProxyInfo.szProxyAddr
    );
    if(CURLE_OK != nCurlCode)
        return false;

    //Set option "proxy port"
    nCurlCode = curl_easy_setopt(pCurl, 
        CURLOPT_PROXYPORT, 
        m_stProxyInfo.usProxyPort
    );
    if(CURLE_OK != nCurlCode)
        return false;

    if(strlen(m_stProxyInfo.szUsername) > 0)
    {
        nCurlCode = curl_easy_setopt(pCurl, 
            CURLOPT_PROXYAUTH, 
            1
        );
        if(CURLE_OK != nCurlCode)
            return false;

        _snprintf(m_szUsrPwd, 
            cnProxyStrMaxLen * 2, 
            "%s:%s", 
            m_stProxyInfo.szProxyAddr, 
            m_stProxyInfo.szPassword
        );

        nCurlCode = curl_easy_setopt(pCurl, 
            CURLOPT_PROXYUSERPWD, 
            m_szUsrPwd
        );
        if(CURLE_OK != nCurlCode)
            return false;
    }
    return true;
}

bool CFileDownloader::PerformCurlRequest(IN void* pvCurl)
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

    if(CURLM_OK != curl_multi_add_handle(pCurlMulti, pCurl))
        goto Exit0;

    /* we start some action by calling perform right away */
    while(CURLM_CALL_MULTI_PERFORM ==
        curl_multi_perform(pCurlMulti, &nStillRunning))
    {
        //Query If Exit is needed
        /*if(IsExitNeeded())
        {
            ::OutputDebugStringA("go out, while curl_multi_perform before while");
            goto Exit0;
        }*/
    }

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
                curl_multi_perform(pCurlMulti, &nStillRunning))
            {
                /*if(IsExitNeeded())
                {
                    ::OutputDebugStringA("go out, while curl_multi_perform loop");
                    goto Exit0;
                }*/
            }
            break;
        }

        /*if(IsExitNeeded())
        {
            ::OutputDebugStringA("go out, normal1");
            goto Exit0;
        }*/
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
                    "curl_easy_perform return %3d "
                    "while querying file info",
                    (pMsg->data).result
                );
                RecordWhatHappened("error", szError);
                //OutputDebugStringA("go out, curl_multi_info_read");
                goto Exit0;
            }
        }
    }

    //If exit by notify, the return false
    if(IsExitNeeded())
    {
        //::OutputDebugStringA("go out, before Exit0");
        goto Exit0;
    }

    //OutputDebugStringA("Quit Successful!");
    bResult = true;
Exit0:
    if(NULL != pCurlMulti && NULL != pCurl)
    {
        curl_multi_remove_handle(pCurlMulti, pCurl);
    }
    if(NULL != pCurlMulti)
    {
        curl_multi_cleanup(pCurlMulti);
    }

    //::OutputDebugStringA("quit PerformCurlRequest!!");
    return bResult;
}

int  CFileDownloader::ComputeNumberOfThreadNeeded(void)
{
    //Check if range is allowed or not
    if(!m_bRangeAllowed)
        return 1;

    if(m_ulFileLength <= m_ulMinBlockSize)
        return 1;

    ULONG ulBlockSize = m_ulFileLength / m_nThreadNumber;
    if(ulBlockSize < m_ulMinBlockSize)
    {
        return (m_ulFileLength / m_ulMinBlockSize) + 1;
    }

    return m_nThreadNumber;
}

bool CFileDownloader::IsDownloadSucceed(void)
{
    BlockDownloadList::iterator it = m_BlockDownloadList.begin();
    for(; it != m_BlockDownloadList.end(); ++it)
    {
        if(it->ulEnd - it->ulSeek > it->ulCompleted)
            return false;
    }
    return true;
}

bool CFileDownloader::IsDownloadFailed(void)
{
    bool bSucceeded = true;

    BlockDownloadList::iterator it = m_BlockDownloadList.begin();
    for(; it != m_BlockDownloadList.end(); ++it)
    {
        if(enumTHREADS_FINISHED != it->pThread->GetState())
            return false;

        if(it->ulSeek + it->ulCompleted == it->ulEnd)
            continue;

        bSucceeded = false;

        if(it->nTryCount < it->nMaxTryNum)
            return false;
    }
    return bSucceeded ? false:true;
}

bool CFileDownloader::IsDownloading(void)
{
    BlockDownloadList::iterator it = m_BlockDownloadList.begin();
    for(; it != m_BlockDownloadList.end(); ++it)
    {
        if(enumTHREADS_FINISHED != it->pThread->GetState())
            return true;
    }
    return false;
}

int  CFileDownloader::GetThreadsRunning()
{
    int nCount = 0;
    BlockDownloadList::iterator it = m_BlockDownloadList.begin();
    for(; it != m_BlockDownloadList.end(); ++it)
    {
        if(enumTHREADS_EXECUTING == it->pThread->GetState())
            ++nCount;
    }
    return nCount;
}

int  CFileDownloader::GetBlockNumNotFinished(void)
{
    int nCount = 0;
    BlockDownloadList::iterator it = m_BlockDownloadList.begin();
    for(; it != m_BlockDownloadList.end(); ++it)
    {
        if(it->ulSeek + it->ulCompleted < it->ulEnd)
            ++nCount;
    }
    return nCount;
}

void CFileDownloader::NotifyDownloadState(IN FDSCode nState)
{
    if(NULL == m_pCallback) 
        return;
    m_pCallback->OnNotifyDownloadState(nState);
}

void CFileDownloader::NotifyDownloadProgress(
    IN  unsigned long   ulFileSize, 
    IN  unsigned long   ulDownloadSize, 
    IN  int             nSpeed, 
    IN  int             nThreadNumber, 
    IN  int             nThreadRunning,
    IN  const char*     pszDescription 
)
{
    if(NULL == m_pCallback)
        return;

    m_pCallback->OnNotifyDownloadProgress(ulFileSize,
        ulDownloadSize,
        nSpeed,
        nThreadNumber,
        nThreadRunning,
        pszDescription
    );
}

void CFileDownloader::NotifyDownloadError(IN FDECode nError)
{
    if(NULL == m_pCallback)
        return;

    m_pCallback->OnNotifyDownloadError(nError);
}

void CFileDownloader::WriteProgressInfoForResume(CDownloadConfigFile& dcFile)
{
	BPInfoList  bpList;
	BlockDownloadList::iterator it = m_BlockDownloadList.begin();
	for(; it != m_BlockDownloadList.end(); ++it)
	{
		ST_BP_INFO  stBpInfo;

		stBpInfo.uID        = it->nBlockID;
		stBpInfo.uStart     = it->ulSeek;
		stBpInfo.uEnd       = it->ulEnd;
		stBpInfo.uCompleted = it->ulCompleted;
		bpList.push_back(stBpInfo);
	}
	dcFile.WriteConfigProgressInfo(bpList);
}

bool CFileDownloader::ImportIEProxy(void)
{
    bool    bResult         = false;
    long    lRetCode        = 0;
    HKEY    hRegKey         = NULL;
    DWORD   dwProxyEnable   = 0;
    DWORD   dwSize          = 0;

    //Open register first
    lRetCode = RegOpenKeyExA(HKEY_CURRENT_USER,
        pcszRegInetSetting,
        NULL,
        KEY_QUERY_VALUE,
        &hRegKey
    );
    if(ERROR_SUCCESS != lRetCode)
        goto Exit0;

    //Query: If proxy enable
    dwSize = sizeof(DWORD);
    lRetCode = RegQueryValueExA(hRegKey,
        "ProxyEnable",
        NULL,
        NULL,
        (LPBYTE)(&dwProxyEnable),
        &dwSize
    );
    if(ERROR_SUCCESS != lRetCode)
        goto Exit0;

    //If proxy enable, query the proxy server
    if(1 == dwProxyEnable)
    {
        //Get proxy information
        if(!ReadIEProxyFromReg(hRegKey))
        {
            m_stProxyInfo.bProxyEnable  = false;
        }
        else
        {
            m_stProxyInfo.bProxyEnable  = true;
        }
    }
    else
    {
        m_stProxyInfo.bProxyEnable  = false;
    }

    bResult = true;
Exit0:
    if(NULL != hRegKey)
        RegCloseKey(hRegKey);

    return bResult;
}

bool CFileDownloader::ReadIEProxyFromReg(HKEY& hKeyReg)
{
    long    lRetCode                = 0;
    char    szProxyServer[MAX_PATH] = {0};
    DWORD   dwSize                  = 0;

    if(NULL == hKeyReg)
        return false;

    //Read proxy information
    dwSize = sizeof(szProxyServer);
    lRetCode = RegQueryValueExA(hKeyReg,
        "ProxyServer",
        NULL,
        NULL,
        (LPBYTE)szProxyServer,
        &dwSize
    );
    if(ERROR_SUCCESS != lRetCode)
        return false;

    if(strlen(szProxyServer) <= 0)
        return false;

    //Parse proxy server
    //Attention: If both HTTP and SOCKS5 are used,
    //I prefer to use SOCKS5

    RecordWhatHappened("Info IE proxy", szProxyServer);

    m_stProxyInfo.nProxyType = enumPROXY_TYPE_HTTP;
    char* pos = strstr(szProxyServer, "socks=");
    if(NULL != pos)
    {
        m_stProxyInfo.nProxyType = enumPROXY_TYPE_SOCK5;
        pos += strlen("socks=");
    }
    else
    {
        pos = strstr(szProxyServer, "http=");
        if(NULL != pos)
        {
            m_stProxyInfo.nProxyType = enumPROXY_TYPE_HTTP;
            pos += strlen("http=");
        }
    }

    if(NULL == pos)
        pos = szProxyServer;

    //Search for semicolon
    char* posSem = strstr(pos, ";");
    if(NULL != posSem)
    {
        *posSem = '\0';
    }

    //Now, the format: "addr:port"
    //Get address and port
    char* posColon = strstr(pos, ":");
    if(NULL == posColon)
        return false;

    *posColon = '\0';
    ++posColon;

    //Now, pos point to the address, and posColon point to port
    if(strlen(pos) <= 0 || strlen(posColon) <= 0 )
        return false;
    
    strncpy(m_stProxyInfo.szProxyAddr, pos, cnProxyStrMaxLen);
    m_stProxyInfo.usProxyPort = (USHORT)(atoi(posColon));
    
    return true;
}

bool CFileDownloader::CallbackToSeeIfExitNeeded(void)
{
    return false;
}
