#pragma once
#include <libdownload\libDownload.h>
#include "Download.h"

class CLocalFileDownload : public IDownload
{
public:
	CLocalFileDownload(void);
	~CLocalFileDownload(void);
	
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
	void Stop();
	void Wait();	
	// Info 
	DLERRCode LastError();
	LPVOID GetUserData();
	virtual void GetDownloadInfo(CString &strUrl, CString &strFilePath);
	
	virtual RemoteFileInfo *GetRemoteFileInfo();
	virtual IDownStat *GetDownloadStat();

protected:
	BOOL _Notify( ProcessState state, LPARAM lParam=0 );

protected:
	CString m_strUrl;
	CString m_strFilePath;
	LPVOID m_lpUserData;
	IHttpAsyncObserver *m_pObserver;
	DLERRCode m_errCode;
	BOOL m_bStopped;
	DWORD m_dwTmLastDownloadProgess;

	RemoteFileInfo m_FileInfo;
	TDownStat m_DownStat;
};
