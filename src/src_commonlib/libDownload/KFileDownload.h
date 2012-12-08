#pragma once
#include <libdownload\libdownload.h>

#include "Download.h"
#include "FileDownloadObjectFactory.h"

class KFileDownload :
	public IDownload, public IDownloadCallback
{
public:
	KFileDownload();
	~KFileDownload(void);
	
	// 
	//////////////////////////////////////////////////////////////////////////
	// IDownload 
	virtual void Release();
	// Create 
	VOID SetObserver(IHttpAsyncObserver *pObserver);
	void SetDownloadInfo(LPCTSTR szUrl, LPCTSTR szFilePath);
	LPVOID SetUserData(LPVOID lpUserData);

	// Control
	BOOL Fetch(INT nCorrurent=0);
	BOOL Start(INT nCorrurent=0);

	void _Reset();
	void Stop();
	void Wait();	
	// Info 
	DLERRCode LastError();
	LPVOID GetUserData();
	virtual void GetDownloadInfo(CString &strUrl, CString &strFilePath);

	virtual RemoteFileInfo *GetRemoteFileInfo();
	virtual IDownStat *GetDownloadStat();

public:
	//////////////////////////////////////////////////////////////////////////
	// IDownloadCallback 
	/**
     * @brief Notify the client the state of downloading
     *
     * @param[IN]   nState          The state of downloading
     *                              See: ENUM_DOWNLOADING_STATE
     * @return  None
     */
    virtual void __stdcall OnNotifyDownloadState(
        IN  FDSCode             nState
    );

    /**
     * @brief Notify the client the progress of downloading
     *
     * @param[IN]   ulFileSize      The total size of file
     * @param[IN]   ulDownloadSize  The size that have been downloaded
     * @param[IN]   nSpeed          The speed(bytes/s)
     * @param[IN]   nThreadNumber   The number of total threads
     * @param[IN]   nThreadRunning  The number of threads running
     * @param[IN]   pszDescription  the Description of download state
     *
     * @return  None
     */
    virtual void __stdcall OnNotifyDownloadProgress(
        IN  unsigned long       ulFileSize,
        IN  unsigned long       ulDownloadSize,
        IN  int                 nSpeed,
        IN  int                 nThreadNumber,
        IN  int                 nThreadRunning,
        IN  const char*         pszDescription
    );

    /**
     * @brief Notify the client the error while downloading
     *
     * @param[IN]   nError          The error code
     *
     * @return None
     */
    virtual void __stdcall OnNotifyDownloadError(
        IN  FDECode             nError
    );

    /**
     * @brief Ask client whether exiting is needed
     * 
     * @note
     * Not use any more
     *
     * !!!!!!!!!!!!!!!!Pay Attention!!!!!!!!!!!!!!!!!!!!
     * You have to implement this function, but do nothing
     *
     * @return If exiting needed
     * - true   yes
     *   false  no
     */
    virtual bool __stdcall OnQueryExitNeeded(void);


protected:
	BOOL _Notify( ProcessState state, LPARAM lParam=0 );

	static unsigned int __stdcall _RunThreadFun( void* lpThis );

protected:
	HMODULE	m_hModuleKFileDownload;
	IFileDownloader *m_pIFileDownloader;

	CString m_strUrl;
	CString m_strFilePath;
	LPVOID m_lpUserData;

	IHttpAsyncObserver *m_pObserver;
	DLERRCode m_errCode;

	RemoteFileInfo m_FileInfo;
	TDownStat m_DownStat;

	INT m_nCorrurent;
	HANDLE m_hThread;
	INT m_nCurrentStat;

	CFileDownloadObjectFactory m_fdFactory;
	CObjLock m_lckFileDownloaderObject; 
};
