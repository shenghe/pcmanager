#pragma once
#include <WinInet.h>
#include "Utils.h"
#include <pubconfig/pubconfig.h>

#define BK_USER_AGENT _T("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 2.0.50727; .NET CLR 3.0.04506.648; .NET CLR 3.5.8888.8888;)")

#define	HTTP_BUFFER_SIZE 1024*8
#define HTTP_TIMEOUT 30*1000
#define HTTP_DEFAULT_RETYR 2 
#define HTTP_DEFAULT_MAXRETYR 10

enum TDownloadErrorType {
	KErr_Downloader_NetworkInit,
	KErr_Downloader_FileNotExists,
	KErr_Downloader_WriteFile,
	KErr_Downloader_Canceled,
	KErr_Downloader_NetworkFailed,
};

class IFileDownloaderObserver {
public:
	enum TFileDownloaderEventType{
		DOWNLOAD_STATUS,
		DOWNLOAD_COMPLETE,
		DOWNLOAD_ERROR, 
	};
	virtual void OnFileDownloaderEvent( TFileDownloaderEventType evt, DWORD dwParam1, DWORD dwParam2, PVOID pUserData=NULL) = 0;
};

class CFileDownloader  
{
public:
	CFileDownloader();
	virtual ~CFileDownloader();
	
public:	
	void SetHttpTimeout(DWORD dwTimeout);
	void SetObserver(IFileDownloaderObserver *pObserver);
	DWORD Download( LPCTSTR lpszURL, LPCTSTR lpszFilename, LPCTSTR lpszRefererUrl, LPVOID pUserData, BOOL bUseCache=TRUE, BOOL bUseProxyConfig=TRUE );
	void Stop();
	
protected:
	BOOL SetProxySetting(CPubConfig::T_ProxySetting &proxysetting);
	INT _DownloadFileFrom( INT64 &lFileSize, INT64 &lFileSizeExpected, BOOL bUseProxyConfig );
	void _CloseFileHandler();
	void _Cleanup();
	BOOL _IssueRequest( CString strHostname, CString strPath, LPCTSTR szHeader, BOOL bUseProxyConfig );

	void _FireFileDownloaderEvent(IFileDownloaderObserver::TFileDownloaderEventType evt, DWORD dwParam1=0, DWORD dwParam2=0);
	BOOL _CheckIfModifiedSince( LPCTSTR lpszURL, LPCTSTR lpszLastModifed, BOOL bUseProxyConfig, BOOL& modified);

	BOOL _GetHttpInfo( HINTERNET hRequest, DWORD dwInfoLevel, CString &str );
	INT64 _GetHttpInfoFileSize(HINTERNET hRequest);
	bool _IfInterruptted();
	

protected:
	DWORD m_dwTimeout;	// Á¬½Ó³¬Ê±
	HINTERNET   m_hSession;
	HINTERNET	m_hConnection;
	HINTERNET	m_hRequest;
	HANDLE	m_hFile;
	
	BOOL m_Stopped;
	
	IFileDownloaderObserver *m_pObserver;
	PVOID m_pUserData;	
	CThreadGuard m_thLocker;

	PBYTE m_pBuffer;
public:
	CString	m_strDownloadURL;
	CString	m_strDownloadFile;
	
	CString m_strHostname, m_strHostPath, m_strHostFilename;
	CString m_strDownloadFileTemp, m_strDownloadFileInfo;

	CPubConfig::T_ProxySetting	m_currentProxySetting;
};
