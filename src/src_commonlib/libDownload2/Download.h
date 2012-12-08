#pragma once
#include "Defines.h"
#include "Segment.h"
#include "UrlPool.h"
#include "pubconfig/pubconfig.h"


class HttpAsyncRequest;
class CDownload;
class CSegmentPool;
class CRequestPool;
class CFileStream;


typedef CHttpAsyncIn* CHttpAsyncInPtr;
typedef std::vector<CHttpAsyncInPtr> CHttpAsyncInPtrs;

struct TDownStat : public IDownStat
{
public:
	TDownStat() 
	{
		Reset();
	}
	virtual int64 Downloaded() 
	{
		return m_iDownloaded;
	}

	virtual int64 Speed()
	{
		return m_nSpeed;
	}

	virtual int64 AverageSpeed()
	{
		if(!m_bBegined)
			return 0;
		
		if(m_bEnded)
			return (m_iDownloaded*1000)/(m_dwTmEnd-m_dwTmBegin+1);
		DWORD dwTmNow = GetTickCount();
		ATLASSERT(dwTmNow>=m_dwTmBegin);
		int64 nAvgSpeed = (m_iDownloaded*1000)/(dwTmNow-m_dwTmBegin+1);

		return nAvgSpeed;
	}

	virtual DWORD TimeUsed()
	{
		return m_dwTmEnd - m_dwTmBegin;
	}

	void Reset()
	{
		m_nSpeed = 0;
		m_iDownloaded = 0;
		m_bBegined = m_bEnded = FALSE;
		m_dwTmBegin = m_dwTmEnd = m_dwTmLastSlot = m_dwTmLastDownloadProgess = 0;
	}

	void OnDownBegin()
	{
		m_bBegined = TRUE;
		m_iDownloaded = 0;
		m_dwTmBegin = m_dwTmEnd = m_dwTmLastSlot = m_dwTmLastDownloadProgess = GetTickCount();
		m_nSpeed = m_nSpeedSlot = 0;
	}

	void OnDownEnd()
	{
		ATLASSERT(m_bBegined);
		m_bEnded = TRUE;
		m_dwTmEnd = GetTickCount();
	}

	void OnDownData(DWORD dwTimeNow, int64 iDownloaded)
	{
		if((dwTimeNow-m_dwTmLastSlot)>DOWNLOAD_SPEED_TEST_INTERVAL)
		{
			m_nSpeed = m_nSpeedSlot*(dwTimeNow-m_dwTmLastSlot)/(DOWNLOAD_SPEED_TEST_INTERVAL);
			m_nSpeedSlot = 0;
			m_dwTmLastSlot = dwTimeNow;
		}
		m_nSpeedSlot += iDownloaded;
	}

	void OnDownData(int64 iDownloaded)
	{
		//OnDownData(GetTickCount(), iDownloaded);
		m_iDownloaded += iDownloaded;
		m_nSpeedSlot += iDownloaded;
	}
	
protected:
	BOOL m_bBegined, m_bEnded;
	int64 m_iDownloaded;

	// speed 
	int64 m_nSpeed, m_nSpeedSlot;
	DWORD m_dwTmBegin, m_dwTmEnd, m_dwTmLastSlot;

	// 
	DWORD m_dwTmLastDownloadProgess;
};


class CDownload : public IDownload
{
public:
	CDownload(int nCocurrent=DOWNLOAD_DEFAULT_COCURRENT);
	~CDownload();
	
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
	static unsigned int __stdcall _RunThreadFun( void* lpThis);
	unsigned int _RunThreadFun_();
	ProcessState _StartSegments( CRequestPool &request_pool, CHttpAsyncInPtr http );
	int _ProcessSegment( CRequestPool &request_pool, CSegmentPool &pool, DWORD dwWaitRet, CFileStream &fs, BOOL &gotData );

	void _RollbackSegment( Segment &s );
	BOOL _Notify(ProcessState state, LPARAM lParam=0);
	void _Reset();
	void _Cleanup();
	
protected:
	int m_nCocurrent;
	CUrlPool m_locationPool;
	CString m_strFilePath;
	LPVOID m_lpUserData;
	HANDLE m_hThread;
	HANDLE m_hStopEvent;
	ProcessState m_state;
	
	IHttpAsyncObserver *m_pObserver;
	int64 m_iInitialDownloaded;
	DLERRCode m_errCode;
	
	// 
	RemoteFileInfo m_FileInfo;
	TDownStat m_downStat;	
	// 
	DWORD m_dwTmLastDownloadProgess;
};
