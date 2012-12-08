#include "stdafx.h"
#include "FileDownloader.h"
#include "Utils.h"

// 分析出下载的 hostname, query path, 以及下载的文件名
BOOL SplitDownloadUrl( LPCTSTR szUrl, CString &strHostname, CString &strPath, CString &strFilename )
{
	static LPCTSTR strProtocolHttp = _T("http://");
	static LPCTSTR strProtocolHttps = _T("https://");

	LPCTSTR p = NULL;
	if( _tcsnicmp(szUrl, strProtocolHttp, _tcslen(strProtocolHttp))==0 )
		p = szUrl + _tcslen( strProtocolHttp );
	else if( _tcsnicmp(szUrl, strProtocolHttps, _tcslen(strProtocolHttps))==0 )
		p = szUrl + _tcslen( strProtocolHttps );
	
	if(p)
	{
		LPCTSTR pslash = _tcschr(p, _T('/'));
		if( pslash )
		{
			LPCTSTR pslashRight = _tcsrchr(pslash, _T('/') );
			if(pslashRight)
			{
				strHostname.SetString( p, (int)(pslash-p) );
				strPath = pslash;
				strFilename = pslashRight+1;

				return TRUE;
			}
		}
	}
	return FALSE;
}

CFileDownloader::CFileDownloader()
{
	m_hConnection = NULL;
	m_hSession = NULL;
	m_hRequest = NULL;
	m_hFile = NULL;

	m_pObserver = NULL;
	m_pUserData = 0;

	m_dwTimeout = HTTP_TIMEOUT;
	m_pBuffer = NULL;
}

CFileDownloader::~CFileDownloader()
{
	_CloseFileHandler();
	_Cleanup();
	if(m_pBuffer)
		SAFE_DELETE_ARRAY(m_pBuffer);
}

void CFileDownloader::_Cleanup()
{
	if (m_hRequest)
	{
		InternetCloseHandle(m_hRequest);
		m_hRequest = NULL;
	}
	if (m_hConnection)
	{
		InternetCloseHandle(m_hConnection);
		m_hConnection = NULL;
	}
	if (m_hSession)
	{
		InternetCloseHandle(m_hSession);
		m_hSession = NULL;
	}
}

void CFileDownloader::SetHttpTimeout( DWORD dwTimeout )
{
	ATLASSERT(dwTimeout>0);
	m_dwTimeout = dwTimeout;
}

void CFileDownloader::SetObserver( IFileDownloaderObserver *pObserver )
{
	m_pObserver = pObserver;
}

DWORD CFileDownloader::Download( LPCTSTR lpszURL, LPCTSTR lpszFilename, LPCTSTR lpszRefererUrl, LPVOID pUserData, BOOL bUseCache, BOOL bUseProxyConfig )
{
	m_Stopped = FALSE;
	if(!SplitDownloadUrl( lpszURL, m_strHostname, m_strHostPath, m_strHostFilename ))
		return FALSE;
	
	m_pUserData = pUserData;
	m_strDownloadURL = lpszURL;
	m_strDownloadFile = lpszFilename;

	// Check the tmp file 
	m_strDownloadFileTemp = m_strDownloadFile + _T(".tmp");
	m_strDownloadFileInfo = m_strDownloadFile + _T(".nfo");		
	
	// Check if file exists 
	if( bUseCache && IsFileExist(m_strDownloadFile) )
	{
		return TRUE;
	}
	else if(!bUseCache)
	{
		DeleteFile(m_strDownloadFileTemp);
		DeleteFile(m_strDownloadFileInfo);
		DeleteFile(m_strDownloadFile);
	}
	
	CPubConfig::T_ProxySetting proxyconfig;
	CPubConfig pubconfig;
	pubconfig.LoadProxyConfig(proxyconfig);	
	SetProxySetting(proxyconfig);


	INT64 lContentSize = 0;
	INT64 lFileSize = 0;
	if( IsFileExist(m_strDownloadFileTemp) && IsFileExist(m_strDownloadFileInfo) )
	{
		CString strLastModified;
		BOOL notModified = FALSE;
		if( file_get_contents(m_strDownloadFileInfo, strLastModified) )		
		{
			LPCTSTR lpszLengthTag = _T("; length=");
			LPCTSTR pszLen = _tcsstr(strLastModified, lpszLengthTag);
			if(pszLen)
			{
				pszLen+= _tcslen(lpszLengthTag);
				lContentSize = _ttoi64(pszLen);
			}

			// 检查文件是否完全下载完成了 
			lFileSize = file_get_size( m_strDownloadFileTemp );
			if(lFileSize>0 && lFileSize==lContentSize)
			{
				MoveFile(m_strDownloadFileTemp, m_strDownloadFile);
				DeleteFile(m_strDownloadFileInfo);
				return TRUE;
			}

			if( _CheckIfModifiedSince(lpszURL, strLastModified, bUseProxyConfig, notModified) && notModified )
			{
			}
			else
			{
				lFileSize = 0;
			}
		}
	}
	// 如果文件已经过期, 或者文件的信息文件不存在, 都删除旧下载临时文件 
	if(lFileSize==0)
	{
		DeleteFile(m_strDownloadFileTemp);
		DeleteFile(m_strDownloadFileInfo);
	}
	
	// Prepare file 
	_CloseFileHandler();
	m_hFile = CreateFile(m_strDownloadFileTemp, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if( m_hFile==INVALID_HANDLE_VALUE )
		return FALSE;
	SetFilePointer( m_hFile, lFileSize, 0, FILE_BEGIN );
	
	if(m_pBuffer==NULL)
		m_pBuffer = new BYTE[HTTP_BUFFER_SIZE];	
	INT nRetry = HTTP_DEFAULT_RETYR;
	INT nRetried = 0;
	do 
	{
		INT iRet = _DownloadFileFrom(lFileSize, lContentSize, bUseProxyConfig);
		DEBUG_TRACE(_T("FD:_DownloadFileFrom %I64d/%I64d LastError:%d Ret:%d"), lFileSize, lContentSize, GetLastError(), iRet);
		_Cleanup();

		if(iRet>0)
		{
			// 考虑socket 被主动关闭 
			if(lFileSize>=lContentSize)
				break;
			nRetry = HTTP_DEFAULT_RETYR;
		}
		else if(iRet==0)
		{
			--nRetry;
		}
		else if(iRet==-1)
		{
			// 文件长度不匹配! 需要删了文件, 然后重新下载 
			_CloseFileHandler();
			DeleteFile(m_strDownloadFileTemp);
			DeleteFile(m_strDownloadFileInfo);
			lFileSize = lContentSize = 0;
			
			m_hFile = CreateFile(m_strDownloadFileTemp, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if( m_hFile==INVALID_HANDLE_VALUE )
				break;
			--nRetry;
		}
		else if(iRet==-2)
		{
			// 写入文件失败, 直接返回
			break;
		}
		else
		{
			ATLASSERT(FALSE);
			break;
		}
		++ nRetried;
	} while (!m_Stopped && nRetry>0 && nRetried<HTTP_DEFAULT_MAXRETYR);
	
	_CloseFileHandler();
	BOOL bFileDone = FALSE;
	if(lContentSize==0)
	{
		// 163.com 等页面
		if(lFileSize==0)
		{
			DeleteFile(m_strDownloadFileTemp);
		}
		else if(!m_Stopped)
		{
			bFileDone = TRUE;
		}
	}
	else
	{
		bFileDone = lFileSize>=lContentSize;
	}
	if(bFileDone)
	{
		MoveFile(m_strDownloadFileTemp, m_strDownloadFile);
		DeleteFile(m_strDownloadFileInfo);
	}
	BOOL bRet = PathFileExists(m_strDownloadFile);
	_FireFileDownloaderEvent(bRet ? IFileDownloaderObserver::DOWNLOAD_COMPLETE : IFileDownloaderObserver::DOWNLOAD_ERROR);
	return bRet;
}

INT CFileDownloader::_DownloadFileFrom( INT64 &lFileSize, INT64 &lContentSize, BOOL bUseProxyConfig )
{	
	INT64 lTotalReaded = 0;
	DWORD dwTickCount = 0;
	LPCTSTR lpszRangeHeadr = NULL;
	CString strHeaderRange;
	
	// 断点续传的必须有文件大小
	if( lFileSize>0 && lContentSize==0 )
		return 0;
	
	if( lFileSize>0 )
	{
		strHeaderRange.Format(_T("Range: bytes=%d-\r\n"), lFileSize);
		lpszRangeHeadr = strHeaderRange;
	}
	if( !_IssueRequest(m_strHostname, m_strHostPath, lpszRangeHeadr, bUseProxyConfig) )
		return 0;
	
	DWORD dwStatusRequired = lFileSize==0 ? HTTP_STATUS_OK:HTTP_STATUS_PARTIAL_CONTENT;
	DWORD dwStatus = 0;
	DWORD dwSize = sizeof(DWORD);
	BOOL bStatusOK = HttpQueryInfo(m_hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatus, &dwSize, NULL);
	if( !bStatusOK || dwStatus!=dwStatusRequired)
		return 0;
	
	lTotalReaded = lFileSize;											// 已下载文件大小
	INT lContentSizeNew = _GetHttpInfoFileSize(m_hRequest) + lFileSize;	// 文件大小 
	if(lFileSize==0)
		lContentSize = lContentSizeNew;
	else
	{
		if(lContentSize!=lContentSizeNew)
			return -1;
	}
	
	// 对新文件保存下载的信息文件 nfo 文件 
	CString strLastModified;
	if( lFileSize==0 && lContentSize>0 && _GetHttpInfo(m_hRequest, HTTP_QUERY_LAST_MODIFIED, strLastModified) )
	{
		strLastModified.AppendFormat(_T("; length=%I64d"), lContentSize);
		file_put_contents(m_strDownloadFileInfo, strLastModified);
	}
	
#ifdef _DEBUG
	if( lFileSize!=0 )
	{
		CString strContentRange;
		ATLVERIFY( _GetHttpInfo(m_hRequest, HTTP_QUERY_CONTENT_RANGE, strContentRange) );
		{
			LPCTSTR psz = strContentRange;
			while( *psz && !isdigit(*psz) )
				++psz;
			DEBUG_TRACE(_T("ContentRange(%I64d) %s, %s\n"), lFileSize, strContentRange, psz);
		}
	}
#endif
	
	int err = 0;
	while(!m_Stopped)
	{
		DWORD dwReaded = 0;
		if( !::InternetReadFile(m_hRequest, (LPVOID)m_pBuffer, HTTP_BUFFER_SIZE-1, &dwReaded) )
		{
			DEBUG_TRACE(_T("InternetReadFile Failed %d \n"), GetLastError());
			// 如果有读取到内容的话, 直接从默认的最大RETRY 开始倒数 
			// err = 0;
			break;
		}
		else
		{
			DWORD dwLen = 0;
			if( !WriteFile(m_hFile, m_pBuffer, dwReaded, &dwLen, NULL) || dwReaded!=dwLen )	// Write File error  
			{
				err = -2;
				break;
			}

			lTotalReaded += dwReaded;
			DWORD dwNow = GetTickCount();
			if( (dwNow-dwTickCount)>200 )
			{
				// Send file download progress in 0.2s 
				dwTickCount = dwNow;
				_FireFileDownloaderEvent(IFileDownloaderObserver::DOWNLOAD_STATUS, lTotalReaded, lContentSize);
			}
		}
		err = 1;
		if(dwReaded==0)
			break;
	}
	DEBUG_TRACE(_T("InternetReadFile Done:%d (LastError :%d) TotalReaded/FileSize/ContentLength: %I64d/%I64d/%I64d \n"), err, GetLastError(), lTotalReaded, lFileSize, lContentSize);
	lFileSize = lTotalReaded;
	return err;
}

void CFileDownloader::Stop()
{
	_CloseFileHandler();
	//_Cleanup();
	m_Stopped = TRUE;
}

BOOL CFileDownloader::SetProxySetting( CPubConfig::T_ProxySetting &proxysetting )
{
	m_currentProxySetting = proxysetting;
	return TRUE;
}

void CFileDownloader::_FireFileDownloaderEvent( IFileDownloaderObserver::TFileDownloaderEventType evt, DWORD dwParam1, DWORD dwParam2 )
{
	if(m_pObserver)
		m_pObserver->OnFileDownloaderEvent( evt, dwParam1, dwParam2, m_pUserData );
}

BOOL CFileDownloader::_CheckIfModifiedSince( LPCTSTR lpszURL, LPCTSTR lpszLastModifed, BOOL bUseProxyConfig, BOOL& modified )
{
	if(!lpszLastModifed)
		return FALSE;
	
	BOOL bRet = FALSE;
	do
	{
		CString strHeaderIfModifiedSince;
		strHeaderIfModifiedSince.Format(_T("If-Modified-Since:%s\r\n"), lpszLastModifed);
		
		if(!_IssueRequest(m_strHostname, m_strHostPath, strHeaderIfModifiedSince, bUseProxyConfig))
			break;
		
		DWORD dwStatus = 0;
		DWORD dwSize = sizeof(DWORD);
		BOOL bStatusOK = HttpQueryInfo(m_hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatus, &dwSize, NULL);
		if( !bStatusOK)
			break;
		
		modified = dwStatus == HTTP_STATUS_NOT_MODIFIED;
		return TRUE;

	}while(FALSE);
	
	return bRet;
}

BOOL CFileDownloader::_IssueRequest( CString strHostname, CString strPath, LPCTSTR szHeader, BOOL bUseProxyConfig )
{
	for(int nRetry=0; nRetry<3 && !_IfInterruptted(); ++nRetry)
	{
		_Cleanup();
		DEBUG_TRACE(_T("_IssueRequest Try %d \n"), nRetry );
		do 
		{
			DWORD dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG;
			CString strProxyServer;			

			if(m_currentProxySetting.nProxyMode==PROXY_MODE_NONE)
			{
				dwAccessType = INTERNET_OPEN_TYPE_DIRECT;
			}
			else if(m_currentProxySetting.nProxyMode==PROXY_MODE_USER)
			{
				dwAccessType = INTERNET_OPEN_TYPE_PROXY;
				strProxyServer = m_currentProxySetting.strHost;
				if(m_currentProxySetting.nPort>0)
					strProxyServer.AppendFormat(_T(":%d"), m_currentProxySetting.nPort);
			}
			m_hSession = ::InternetOpen(BK_USER_AGENT, dwAccessType, strProxyServer, NULL, 0);
			if(m_hSession==NULL)
				break;
			
			::InternetSetOption(m_hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &m_dwTimeout, sizeof(DWORD)/sizeof(BYTE));
			::InternetSetOption(m_hSession, INTERNET_OPTION_SEND_TIMEOUT, &m_dwTimeout, sizeof(DWORD)/sizeof(BYTE));
			::InternetSetOption(m_hSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &m_dwTimeout, sizeof(DWORD)/sizeof(BYTE));
			m_hConnection = InternetConnect(m_hSession, strHostname, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
			if(m_hConnection==NULL)
				break; 
			
			DWORD dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS;
			//DEBUG_TRACE(_T("InternetConnect DONE \n"));
			static LPCTSTR ppszAcceptTypes[2] =
			{
				_T("*/*"),
				NULL
			};
			m_hRequest = HttpOpenRequest(m_hConnection, _T("GET"), strPath, _T("HTTP/1.1"), NULL, ppszAcceptTypes, dwFlags, 0);
			if(m_hRequest==NULL)
				break; 
			if(szHeader)
				HttpAddRequestHeaders(m_hRequest, szHeader, (DWORD)_tcslen(szHeader), HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);

			if(m_currentProxySetting.nProxyMode!=PROXY_MODE_NONE && m_currentProxySetting.bNeedAuthorization)
			{
				// 不管是否autodetect 都设置密码 
				if ( InternetSetOption(m_hRequest, INTERNET_OPTION_PROXY_USERNAME, (LPVOID) m_currentProxySetting.strLogin.operator LPCTSTR(), (m_currentProxySetting.strLogin.GetLength()+1) * sizeof(TCHAR))
					&& InternetSetOption(m_hRequest, INTERNET_OPTION_PROXY_PASSWORD, (LPVOID) m_currentProxySetting.strPassword.operator LPCTSTR(), (m_currentProxySetting.strPassword.GetLength()+1) * sizeof(TCHAR))
					)
				{
					;
				}
			}

			//DEBUG_TRACE(_T("HttpOpenRequest DONE \n"));
			BOOL bRet = HttpSendRequest(m_hRequest, NULL, 0, NULL, 0);
			DEBUG_TRACE(_T("HttpSendRequest DONE \n"));
			if(bRet)
				return TRUE;
		} while (FALSE);
		Sleep( 1*1000 );
	}	
	return FALSE;
}

BOOL CFileDownloader::_GetHttpInfo( HINTERNET hRequest, DWORD dwInfoLevel, CString &str )
{
	DWORD dwBufLen = MAX_PATH;
	DWORD dwIndex = 0;
	BOOL bRet = HttpQueryInfo(hRequest, dwInfoLevel, str.GetBuffer(MAX_PATH), &dwBufLen, &dwIndex);
	str.ReleaseBuffer();
	return bRet;
}

INT64 CFileDownloader::_GetHttpInfoFileSize(HINTERNET hRequest)
{
	CString str;
	if(_GetHttpInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH, str))
		return _ttoi64(str);
	return 0;
}

void CFileDownloader::_CloseFileHandler()
{
	autolock<CThreadGuard> _lk(m_thLocker);
	if(m_hFile)
	{
		if(m_hFile!=INVALID_HANDLE_VALUE)
			CloseHandle(m_hFile);
		m_hFile = NULL;
	}
}

bool CFileDownloader::_IfInterruptted()
{
	return m_Stopped ;
}
