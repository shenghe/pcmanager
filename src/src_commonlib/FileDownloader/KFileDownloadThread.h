/***************************************************************
 * @date:   2007-10-19
 * @author: BrucePeng
 * @brief:  Define the download thread object
 */

#ifndef K_FILE_DOWNLOAD_THREAD_H_
#define K_FILE_DOWNLOAD_THREAD_H_

#include "KThread.h"
#include "KInetFileDownload.h"
#include "curl/curl.h"

class CFileDownloadThread : public CThread
{
public:
    //This is the default constructor
    CFileDownloadThread(int nUserDefID, IDTCallback* m_pCallback);

    //This is the default destructor
    virtual ~CFileDownloadThread(void);

public:
    virtual void Run(void);

public:
    bool Initialize(void);

    bool Uninitialize(void);

public:
    //Set the user defined ID of this download thread
    inline void SetUserDefinedID(int nID);

    //Get the user defined ID of this download thread
    inline int  GetUserDefinedID(void) const;

    //Set the sleep time before working
    inline void SetSleepTimeBeforeStart(unsigned long ulSleep);

public:
    static size_t MyHeaderFunction(
        void*           ptr, 
        size_t          size, 
        size_t          nmemb, 
        void*           stream
    );

private:
    bool IsProxyOptionsNeeded(const ST_PROXY_INFO& stInfo);

private:
    int             m_nUserDefID;
    IDTCallback*    m_pCallback;
    unsigned long   m_ulSleepTime;
    CURL*           m_pCurl;
};

inline void CFileDownloadThread::SetUserDefinedID(int nID)
{
    m_nUserDefID = nID;
}

inline int  CFileDownloadThread::GetUserDefinedID(void) const
{
    return m_nUserDefID;
}

inline void CFileDownloadThread::SetSleepTimeBeforeStart(unsigned long ulSleep)
{
    m_ulSleepTime = ulSleep;
}

#endif//I_FILE_DOWNLOADER_INNER_H_
