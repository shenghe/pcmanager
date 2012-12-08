#include "StdAfx.h"
#include <algorithm>
#include "Download.h"
#include "FileStream.h"
#include "HttpAsyncRequest.h"
#include "Utilfunction.h"
#include "Segment.h"
#include "MD5Checksum.h"
#include "SegmentPool.h"
#include "RequestPool.h"

CDownload::CDownload(int nCocurrent)
	: m_nCocurrent(nCocurrent), m_locationPool(nCocurrent-1)
{
	ATLASSERT(m_nCocurrent>0);
	m_lpUserData = NULL;
	m_hThread = NULL;
	m_hStopEvent = NULL;
	m_pObserver = NULL;
	m_FileInfo.Reset();
}

CDownload::~CDownload()
{
	_Cleanup();
}

void CDownload::Release()
{
	delete this;
}

VOID CDownload::SetObserver( IHttpAsyncObserver *pObserver )
{
	m_pObserver = pObserver;
}

void CDownload::SetDownloadInfo( LPCTSTR szUrl, LPCTSTR szFilePath )
{
	//m_strUrl = szUrl;
	LPCTSTR szMagic = _T("http://download.");
	LPCTSTR szReplace = _T("http://qh.dlservice.");
	
	CStrings mirrors;
	if(_tcsnicmp(szUrl, szMagic, _tcslen(szMagic)) == 0)
	{
		CString sm;
		sm = szReplace;
		sm.Append(szUrl + _tcslen(szMagic));
		mirrors.push_back( sm );
	}	
	m_locationPool.SetLocation(szUrl, mirrors);
	m_strFilePath = szFilePath;
}

LPVOID CDownload::SetUserData( LPVOID lpUserData )
{
	m_lpUserData = lpUserData;
	return m_lpUserData;
}

BOOL CDownload::Fetch( INT nCorrurent/*=0*/ )
{
	if(Start(nCorrurent))
	{
		Wait();
		return PathFileExists(m_strFilePath);
	}
	return FALSE;
}

BOOL CDownload::Start(INT nCorrurent)
{
	_Cleanup();
	_Reset();
	
	if(nCorrurent>0)
	{
		m_nCocurrent = nCorrurent;
		//m_locationPool.SetMainSiteLimit(m_nCocurrent-1);
		m_locationPool.SetMainSiteLimit(m_nCocurrent);
	}
	
	m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hStopEvent)
	{
		unsigned int nThreadID = 0;
		m_hThread = (HANDLE) _beginthreadex(NULL, 0, _RunThreadFun, this, 0, &nThreadID);
	}
	return TRUE;
}

void CDownload::Stop()
{
	if(m_hStopEvent)
		SetEvent(m_hStopEvent);
	Wait();
}

DLERRCode CDownload::LastError()
{
	return m_errCode;
}

LPVOID CDownload::GetUserData()
{
	return m_lpUserData;
}

void CDownload::GetDownloadInfo( CString &strUrl, CString &strFilePath )
{
	strUrl = m_locationPool.GetMainUrl();
	strFilePath = m_strFilePath;
}
void CDownload::_Cleanup()
{
	SAFE_CLOSE_HANDLE(m_hThread);
	SAFE_CLOSE_HANDLE(m_hStopEvent);
}

void CDownload::_Reset()
{
	m_FileInfo.Reset();
	m_state = ProcessState_Idle;
	m_errCode = DLERR_SUCCESS;
	m_downStat.Reset();
}

void CDownload::Wait()
{
	if(m_hThread)
		WaitForSingleObject(m_hThread, INFINITE);
	_Cleanup();
}

unsigned int __stdcall CDownload::_RunThreadFun( void* lpThis )
{
	ATLASSERT(lpThis);
	CDownload *pThis = (CDownload*) lpThis;
	
	ULONG uMaxConns = 2;
	ULONG uSize = sizeof( uMaxConns );
	if ( InternetQueryOption( NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &uMaxConns, &uSize ) )
	{
		uMaxConns += pThis->m_nCocurrent;
		InternetSetOption( NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &uMaxConns, sizeof( uMaxConns ) );
	}
	
	if(pThis)
		pThis->_RunThreadFun_();
		
	uSize = sizeof( uMaxConns );
	if ( InternetQueryOption( NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &uMaxConns, &uSize ) )
	{
		uMaxConns -= pThis->m_nCocurrent;
		InternetSetOption( NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &uMaxConns, sizeof( uMaxConns ) );
	}
	return 0;
}

unsigned int CDownload::_RunThreadFun_()
{
	CRequestPool request_pool;
	CPubConfig::T_ProxySetting proxyconfig;
	CPubConfig pubconfig;
	pubconfig.LoadProxyConfig(proxyconfig);
	request_pool.SetProxyConfig(proxyconfig);

	CString strUrl = m_locationPool.GetMainUrl();
	UrlInfo urlinfo;
	CrackUrl( strUrl, urlinfo );
	
	CHttpAsyncInPtr http = request_pool.Get();
	m_state = ProcessState_Idle;
	if (http->Open(&urlinfo))
	{
		while(TRUE)
		{
			HANDLE hWaits[2] = {m_hStopEvent, http->GetWaitHandle()};
			DWORD dwWaitRet = WAIT_FAILED;
			dwWaitRet = WaitForMultipleObjects(2, hWaits, FALSE, INFINITE);
			
			if (WAIT_FAILED == dwWaitRet )
			{
				m_state = ProcessState_Failed;
				break;
			}
			else if (WAIT_OBJECT_0 == dwWaitRet)
			{
				m_state = ProcessState_UserCanceled;
				break;
			}
			else if (WAIT_TIMEOUT == dwWaitRet)
			{
				// check running queue each second !!!! 
				// start failed segment !!! if has valid main url !!! 
				//TODO
				;
			}
			else
			{
				EAsyncInState st = http->Process();
				if(st==EAsync_Finished||st==EAsync_Failed)
				{
					m_state = ProcessState_Failed;
					m_errCode = DLLER_SERVER_FILENOTFOUND;
					break;
				}
				else if(st==EAsync_Opened)
				{
					m_state = ProcessState_BeginReceiveData;
					break;
				}
			}
		}
		
		// 开始分批下载 
		if (ProcessState_BeginReceiveData == m_state)
		{
			m_state = _StartSegments(request_pool, http);
		}
	}
	request_pool.Cleanup();

#ifdef _DEBUG
	CString strCheckSum;
	try
	{
		strCheckSum = CMD5Checksum::GetMD5( m_strFilePath );
	}
	catch (...)
	{
		strCheckSum = _T("ERROR");
	}
	MYTRACE(_T("DOWNLOAD %s -> %s [%s] : %d - Downloaded/Content-Length: %I64d/%I64d  AvSpeed:%I64d(CO:%d) TimeUsed:%d\r\n"), 
		m_locationPool.m_strMainUrl, m_strFilePath, strCheckSum \
		, m_state, m_FileInfo.fileDownloaded-m_iInitialDownloaded, m_FileInfo.fileSize, m_downStat.AverageSpeed(), m_nCocurrent, m_downStat.TimeUsed());
#endif 

	_Notify( m_state );
	return 0;
}

ProcessState CDownload::_StartSegments( CRequestPool &request_pool, CHttpAsyncInPtr http )
{
	ProcessState state = ProcessState_Idle;
	CString		strTmpFile = m_strFilePath + DOWNLOAD_TEMP_FILE_SUFFIX;
	CString		strTmpInfoFile = m_strFilePath + DOWNLOAD_TEMP_FILE_INFO_SUFFIX;
	
	// 防止 _kt & _kti 文件不同时存在 
	if(PathFileExists(strTmpInfoFile) && !PathFileExists(strTmpFile))
	{
		DeleteFile(strTmpFile);
		DeleteFile(strTmpInfoFile);
	}
	
	CSegmentPool segment_pool(m_strFilePath);
	m_FileInfo = http->GetRemoteFileInfo();
	m_FileInfo.fileDownloaded = segment_pool.Init(m_FileInfo, m_nCocurrent);
	MYTRACE(_T("BeginDownload :%s -> %s (%I64d Bytes / %I64d )\r\n"), m_locationPool.GetMainUrl(), m_strFilePath, m_FileInfo.fileDownloaded, m_FileInfo.fileSize);
	

	if(m_FileInfo.fileDownloaded==0 && PathFileExists(strTmpFile))
	{
		DeleteFile(strTmpFile);
		DeleteFile(strTmpInfoFile);
	}
	
	CFileStream fs(strTmpFile);	
	if (!fs.Create())
	{
		MYTRACE(_T("Create File Error : %s\r\n"), m_strFilePath);
		m_errCode = DLERR_CREATEFILE;
		state = ProcessState_Failed;
	}
	else
	{
		if (m_FileInfo.fileSize > 0)
			fs.SetLength(m_FileInfo.fileSize);

		// run all segements 
		BOOL first = TRUE;
		for(int i=0; i<m_nCocurrent; ++i)
		{
			SegmentPtr s = NULL;
			if(first)
			{
				s = segment_pool.GetNextSegment(NULL);
				if(!s) break;

				s->instream = http;
				if(s->currentposition>0)
					s->instream->OpenRange(s->currentposition, s->endposition);
				else if(s->endposition>0) // 
					s->instream->ModifyRange(0, s->endposition);
				s->instream->SetFileSizeExpect(m_FileInfo.fileSize);
				first = FALSE;
			}
			else
			{
				CString strUrl;
				if( !m_locationPool.Get(strUrl) )
					break;
				else
				{
					UrlInfo urlinfo;
					CrackUrl(strUrl, urlinfo);

					s = segment_pool.GetNextSegment(NULL);
					if(!s) break;

					CHttpAsyncInPtr h = request_pool.Get();
					h->Open(&urlinfo, s->currentposition, s->endposition, m_FileInfo.fileSize);
					s->instream = h;
				}
			}
			if( s )
				segment_pool.NotifySegmentToRunning(s); 
		}

		if(first)
		{
			state = ProcessState_Finished;
		}
		else
		{
			// read and run 
			m_downStat.OnDownBegin();
			m_dwTmLastDownloadProgess = GetTickCount();
			m_iInitialDownloaded = m_FileInfo.fileDownloaded;

			INT nTriesFromNoData = 0;
			DWORD dwLastFailStartTime = m_dwTmLastDownloadProgess;
			HANDLE *hWaits = new HANDLE[1+m_nCocurrent];			
			while(segment_pool.hasRunning() || segment_pool.hasQueuing()) 
			{
				ZeroMemory( hWaits, sizeof(HANDLE)*(1+m_nCocurrent) );
				hWaits[0] = m_hStopEvent;

				SegmentPtrs &runnings = segment_pool.GetRunningSegments();
				for(size_t i=0; i<runnings.size(); ++i)
				{
					ATLASSERT(runnings[i]->instream->GetWaitHandle());
					hWaits[i+1] = runnings[i]->instream->GetWaitHandle();
				}
				DWORD dwWaitRet = WAIT_FAILED;
				//dwWaitRet = WaitForMultipleObjects(1+runnings.size(), hWaits, FALSE, INFINITE);
				dwWaitRet = WaitForMultipleObjects(1+runnings.size(), hWaits, FALSE, 1000);	// 1s for start failed segments !! 

				DWORD dwTimeNow = GetTickCount();
				m_downStat.OnDownData(dwTimeNow, 0);


				BOOL bSegmentIsDone = FALSE;
				if (WAIT_FAILED == dwWaitRet )
				{
					m_errCode = DLERR_INTERNAL;
					state = ProcessState_Failed;
					break;
				}
				else if (WAIT_OBJECT_0 == dwWaitRet)
				{
					state = ProcessState_UserCanceled;
					break;
				}
				else if (WAIT_TIMEOUT == dwWaitRet)
				{
					;
				}
				else
				{
					BOOL bGotData = FALSE;
					int ret = _ProcessSegment(request_pool, segment_pool, dwWaitRet, fs, bGotData );
					if(ret<0)
						state = ProcessState_Failed;
					else if(ret>0)
						bSegmentIsDone = TRUE;
					if(bGotData) // Reset on received data  
						nTriesFromNoData = 0;
				}

				if( (dwTimeNow-m_dwTmLastDownloadProgess)>DOWNLOAD_PROGRESS_REPORT_INTERVAL )
				{
					_Notify(ProcessState_ReceiveData);
					m_dwTmLastDownloadProgess = dwTimeNow;
				}

				// Restart error !! 
				if((dwTimeNow-dwLastFailStartTime)>DOWNLOAD_FAIL_RETRY_INTERVAL)
				{
					if(segment_pool.GetRunningSegments().size()<m_nCocurrent && segment_pool.hasQueuing())
					{
						dwLastFailStartTime = dwTimeNow;
						CString strUrl;
						if( m_locationPool.Get(strUrl) )
						{
							UrlInfo urlinfo;
							CrackUrl(strUrl, urlinfo);
							SegmentPtr s = segment_pool.GetNextSegment(NULL);
							if(s)
							{
								CHttpAsyncInPtr h = request_pool.Get();
								h->Open(&urlinfo, s->currentposition, s->endposition, m_FileInfo.fileSize);
								s->instream = h;
								segment_pool.NotifySegmentToRunning(s);
							}
						}							
						++ nTriesFromNoData;

					}
				}

				// check if all parts is done
				if(bSegmentIsDone && !segment_pool.hasRunning())
				{
					if(segment_pool.hasQueuing())
					{
						MYTRACE(_T("- All Parts Finished, %d Segments Failed, reach max try '%d' \r\n"), segment_pool.GetQueueSegments().size(), nTriesFromNoData);						
						if(nTriesFromNoData>DOWNLOAD_MAINURL_MAXRETYR)
						{
							state = ProcessState_Failed;
							break;
						}
					}
					else
					{
						MYTRACE(_T("- All Parts Finished \r\n"));
						state = ProcessState_Finished;
						break;
					}
				}
			}
			SAFE_DELETE_ARRAY(hWaits);
		}
		fs.CloseFile();
		//
		if(state==ProcessState_Finished)
		{
			CSegmentInfoFile(m_strFilePath).Unlink();
			MoveFileEx(m_strFilePath + DOWNLOAD_TEMP_FILE_SUFFIX, m_strFilePath, MOVEFILE_REPLACE_EXISTING);
		}
		else if(m_FileInfo.fileSize>0)
			CSegmentInfoFile(m_strFilePath).Save(m_FileInfo.fileSize, m_FileInfo.fileDownloaded, segment_pool.GetSegments());	
	}
	request_pool.Cleanup();
	m_downStat.OnDownEnd();
	return state;
}

int CDownload::_ProcessSegment( CRequestPool &request_pool, CSegmentPool &pool, DWORD dwWaitRet, CFileStream &fs, BOOL &gotData )
{
	int ret = 0;
	BOOL bSegmentDone = FALSE;
	int idx = dwWaitRet - 1;
	SegmentPtrs &runnings = pool.GetRunningSegments();
	Segment &s = *runnings[idx];
	CHttpAsyncIn &http = *s.instream;
	EAsyncInState st = http.Process();
	gotData = EAsync_Data==st;
	
	if(EAsync_Finished==st && s.endposition>0 && !s.isDone())
	{
		// 块应该没有下载完成 !		
		MYTRACE(_T("Unfinished Segment(%d) %I64d - %I64d - %I64d\r\n"), s.index, s.startposition, s.currentposition, s.endposition);
		st = EAsync_Failed;
	}

	if( EAsync_Finished==st && s.instream && s.instream->IsLastValidBufferZeroBit() )
	{
		MYTRACE(_T("Finished (BUT LAST BUFFER 1.ZERO) Segment(%d) %I64d - %I64d - %I64d\r\n"), s.index, s.startposition, s.currentposition, s.endposition);
		st = EAsync_Failed;
	}
	
	if(EAsync_Failed==st)
	{
		// NOTE: 需要回滚数据!!! 
		MYTRACE(_T("Queuing Failed Segment(%d) %I64d - %I64d - %I64d\r\n"), s.index, s.startposition, s.currentposition, s.endposition);
		const UrlInfo * urlinfo = s.instream->GetLocationInfo();
		m_locationPool.Put(urlinfo->szUrl, TRUE);
		
		BOOL toRollback = s.instream->IsLastValidBufferZeroBit();

		request_pool.Put( s.instream );
		s.instream = NULL;

		// 回滚 
		if(toRollback)
		{
			_RollbackSegment(s);
		}
		
#if 0 
		if(s.endposition>0)
		{
			int64 iDownloaded = s.currentposition - s.startposition;
			if((s.currentposition-s.startposition) < HTTP_RECEIVE_BUFFER_LENGTH)
			{
				s.currentposition = s.startposition;
			}
			else
			{
				s.currentposition -= HTTP_RECEIVE_BUFFER_LENGTH;
			}
			if((s.currentposition-s.startposition) < MIN_BLOCK)
			{
				s.currentposition = s.startposition;
			}
			
			int64 iDiff = iDownloaded - (s.currentposition - s.startposition);
			m_FileInfo.fileDownloaded -= iDiff; // 回滚下载长度 
			MYTRACE(_T("Rollback Failed Segment(%d) %I64d - %I64d - %I64d  (%d) BYTE\r\n"), s.index, s.startposition, s.currentposition, s.endposition, iDiff);
		}
#endif 

		pool.Put(&s);
		//TODO :notify fail 
		return 1;
	}
	else if(EAsync_Data==st)
	{		
		INTERNET_BUFFERSA *pInetBuff = (INTERNET_BUFFERSA *)http.GetLastBuffer();
		DWORD dwToWrite = pInetBuff->dwBufferLength;
		DWORD dwWrite = 0;
		
		//MYTRACE(_T("%I64d - %I64d - %I64d  %d\r\n"), s.startposition, s.currentposition, s.endposition, dwToWrite);
		if(s.endposition>0)
		{
			ATLASSERT(s.endposition>s.startposition && s.endposition>s.currentposition);
			ATLASSERT(dwToWrite>0 && s.endposition-s.currentposition);
			ATLASSERT(dwToWrite<=(s.endposition-s.currentposition));
			dwToWrite = min(s.endposition-s.currentposition, dwToWrite);
		}
		if (!fs.Write(s.currentposition, pInetBuff->lpvBuffer, dwToWrite, &dwWrite))
		{
			MYTRACE( _T("ProcessState_ReceiveData Write Error: %d/%d\r\n"), dwWrite/dwToWrite );
			m_errCode = DLERR_WRITEFILE;
			return -1;
		}
		else
		{
			s.currentposition += dwToWrite;
			m_FileInfo.fileDownloaded += dwToWrite;
			m_downStat.OnDownData(dwToWrite);
			return 0;
		}
	}
	else if(EAsync_Finished==st)
	{
		ATLASSERT(s.isDone() || s.endposition==-1);
		s.instream->Process();	// 最后close 		

		MYTRACE(_T("Part %d(%I64d-%I64d-%I64d) Done\r\n"), s.index, s.startposition, s.currentposition, s.endposition);
		SegmentPtr pnext = pool.GetNextSegment(&s);
		if(pnext)
		{
			ATLASSERT(pnext->instream!=s.instream);
			if(pnext->instream)
			{
				request_pool.Put(pnext->instream);
				pnext->instream = NULL;
			}
			if(pnext->currentposition==s.endposition)
			{
				// Reuse the connection 
				pnext->instream = s.instream;
				s.instream = NULL;
				pnext->instream->Continue(pnext->currentposition, pnext->endposition);
			}
			else
			{
				// New Connection 
				pnext->instream = s.instream;
				s.instream = NULL;
				pnext->instream->OpenRange(pnext->currentposition, pnext->endposition);
			}
			pool.NotifySegmentToRunning(pnext);
		}
		else
		{
			const UrlInfo * urlinfo = s.instream->GetLocationInfo();
			m_locationPool.Put(urlinfo->szUrl, FALSE);

			request_pool.Put( s.instream );
			s.instream = NULL;
		}
		ATLASSERT(s.instream==NULL);
		pool.Put(&s);
		return 1;
	}
	return 0;
}

void CDownload::_RollbackSegment( Segment &s )
{
	const int ROLLBACK_SIZE = 512;
	int64 iDownloaded = s.currentposition - s.startposition;
	if((s.currentposition-s.startposition) < ROLLBACK_SIZE)
	{
		s.currentposition = s.startposition;
	}
	else
	{
		s.currentposition -= ROLLBACK_SIZE;
	}
	if( (s.currentposition-s.startposition) < ROLLBACK_SIZE )
	{
		s.currentposition = s.startposition;
	}

	int64 iDiff = iDownloaded - (s.currentposition - s.startposition);
	m_FileInfo.fileDownloaded -= iDiff; // 回滚下载长度 
	MYTRACE(_T("Rollback Failed Segment(%d) %I64d - %I64d - %I64d  (%d) BYTE\r\n"), s.index, s.startposition, s.currentposition, s.endposition, iDiff);
}

BOOL CDownload::_Notify( ProcessState state, LPARAM lParam/*=0*/ )
{
	if(m_pObserver)
		return m_pObserver->OnHttpAsyncEvent(this, state, lParam);
	return TRUE;
}

RemoteFileInfo * CDownload::GetRemoteFileInfo()
{
	return &m_FileInfo;
}

IDownStat * CDownload::GetDownloadStat()
{
	return &m_downStat;
}