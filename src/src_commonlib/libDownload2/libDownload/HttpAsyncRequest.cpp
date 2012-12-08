#include "StdAfx.h"
#include "HttpAsyncRequest.h"
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <atldef.h>

CHttpAsyncIn::CHttpAsyncIn()
	: m_hWaitEvent(NULL)
	, m_hWaitableTimer(NULL)
	, m_hSession(NULL), m_hConnect(NULL), m_hRequest(NULL)
	, m_pBuffer(NULL)
	, m_nMaxRetry(3)
{
	_Reset();
}

CHttpAsyncIn::~CHttpAsyncIn()
{
	Close();
	SAFE_DELETE_ARRAY(m_pBuffer);
}

BOOL CHttpAsyncIn::Open( UrlInfo *location, int64 startPosition/*=0*/, int64 endPosition/*=0*/, int64 filesize/*=0*/ )
{
	Close();
	_Reset();
	ATLASSERT(location);
	if(!location)
		return FALSE;
	
	ATLASSERT(!m_hWaitEvent);
	m_hWaitEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	if( !m_hWaitEvent )
		return FALSE;
	
	if(m_pBuffer==NULL)
		m_pBuffer = new CHAR[HTTP_RECEIVE_BUFFER_LENGTH];
	if(!m_pBuffer)
		return FALSE;
	
	m_ResourceLocation = *location;
	m_state = ProcessState_Idle;
	ATLASSERT(endPosition<=0 || endPosition>startPosition);
	m_iPositionBegin = m_iPositionCurrent = startPosition;
	m_iPositionEnd = endPosition;
	m_iFilesizeExpect = filesize;
	return Process()!=EAsync_Failed;
}

BOOL CHttpAsyncIn::Continue( int64 startPosition, int64 endPosition )
{
	ATLASSERT(startPosition == m_iPositionCurrent);
	if(startPosition!=m_iPositionCurrent)
	{
		m_state = ProcessState_Failed;
		return FALSE;
	}
	m_iPositionEnd = endPosition;
	m_state = ProcessState_ReceiveData;
	return TRUE;
}

BOOL CHttpAsyncIn::OpenRange( int64 startPosition, int64 endPosition )
{
	m_iPositionBegin = m_iPositionCurrent = startPosition;
	m_iPositionEnd = endPosition;
	m_nRetry = 0;
	return _TryRetry(FALSE);
}

BOOL CHttpAsyncIn::ModifyRange( int64 /*startPosition*/, int64 endPosition )
{
	m_iPositionEnd = endPosition;
	return TRUE;
}

BOOL CHttpAsyncIn::Close()
{
	CObjGuard __guard__(m_objLock, TRUE);
	if(m_hRequest)
		InternetSetStatusCallback(m_hRequest, NULL);
	if(m_hConnect)
		InternetSetStatusCallback(m_hConnect, NULL);
	if(m_hSession)
		InternetSetStatusCallback(m_hSession, NULL);
	
	SAFE_CLOSE_HANDLE(m_hWaitEvent);
	SAFE_CLOSE_HANDLE(m_hWaitableTimer);
	SAFE_CLOSE_INTERNETHANDLE(m_hRequest);
	SAFE_CLOSE_INTERNETHANDLE(m_hConnect);
	SAFE_CLOSE_INTERNETHANDLE(m_hSession);
	return TRUE;
}

EAsyncInState CHttpAsyncIn::Process()
{
	EAsyncInState ret = _Process();
	if (EAsync_Failed==ret)
	{
		m_state = ProcessState_Failed;
		Close();
	}
	return ret;
}

BOOL CHttpAsyncIn::Release()
{
	delete this;
	return TRUE;
}

BOOL CHttpAsyncIn::SetProxyConfig( CPubConfig::T_ProxySetting &proxyconfig )
{
	m_currentProxySetting = proxyconfig;
	return TRUE;
}

HANDLE CHttpAsyncIn::GetWaitHandle()
{
	return m_hWaitableTimer ? m_hWaitableTimer : m_hWaitEvent;
}

RemoteFileInfo CHttpAsyncIn::GetRemoteFileInfo()
{
	return m_fileInfo;
}

PVOID CHttpAsyncIn::GetLastBuffer()
{
	return &InetBuff;
}

DWORD CHttpAsyncIn::GetLastAsyncError()
{
	return m_dwAsyncError;
}

const UrlInfo * CHttpAsyncIn::GetLocationInfo() const
{
	return &m_ResourceLocation;
}

VOID CHttpAsyncIn::SetRetry( INT nMaxRetry )
{
	ATLASSERT(nMaxRetry>=0);
	if(nMaxRetry>0)
		m_nMaxRetry = nMaxRetry;
	else
		m_nMaxRetry = 0;
}

VOID CHttpAsyncIn::SetFileSizeExpect( int64 filesize )
{
	m_iFilesizeExpect = filesize;
}

BOOL CHttpAsyncIn::IsLastValidBufferZeroBit() const
{
	return m_dwLastValidBufferSize==1 && m_dwLastValidBufferFirstBit==0;
}

void CHttpAsyncIn::_Reset()
{
	m_iFilesizeExpect = 0;
	m_bHandleValid = FALSE;
	m_state = ProcessState_Idle;
	m_nRetry = 0;
	m_iDownloaded = 0;
}

EAsyncInState CHttpAsyncIn::_Process()
{
	switch( m_state )
	{
	case ProcessState_Idle:
	case ProcessState_StartConnectToServer:
		m_state = ProcessState_OpenRequest;
		return _ConnectServer() ? EAsync_Idle : EAsync_Failed;

	case ProcessState_OpenRequest:
		// Open the request
		SAFE_CLOSE_HANDLE(m_hWaitableTimer);
		m_state = ProcessState_EndRequest;
		{
			DWORD dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS;
			if ( INTERNET_SCHEME_HTTPS == m_ResourceLocation.nScheme )
				dwFlags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
			
			ResetEvent(m_hWaitEvent);
			CString strQuery = m_ResourceLocation.szUrlPath;
			strQuery += m_ResourceLocation.szExtraInfo;
			m_hRequest = HttpOpenRequest(m_hConnect, _T("GET"), strQuery
				, _T( "HTTP/1.1" ), NULL, NULL, dwFlags, (DWORD)this);  // Request handle's context 
			ATLASSERT(m_hRequest);
			if (m_hRequest)
			{
				INTERNET_BUFFERS ibsIn  = {sizeof( INTERNET_BUFFERS )};
				TCHAR szHeader[ INTERNET_MAX_URL_LENGTH * 2 ] = _T( "" );
				_CreateAdditionalHeaders( szHeader, SIZE_OF_ARRAY( szHeader ) );
				ibsIn.lpcszHeader = szHeader;
				ibsIn.dwHeadersLength = _tcslen( szHeader );

				ULONG uSendTimeout = HTTP_SEND_TIME_OUT;
				InternetSetOption( m_hRequest, INTERNET_OPTION_SEND_TIMEOUT, &uSendTimeout, sizeof( uSendTimeout ) );
				
				// 
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

				if (HttpSendRequestEx(m_hRequest, &ibsIn, NULL, HSR_ASYNC, (DWORD)this))
				{
					SetEvent(m_hWaitEvent);
					return EAsync_Idle;
				}
				else if(ERROR_IO_PENDING==GetLastError())
				{
					//MYTRACE( _T("[%08x]ProcessState_OpenRequest IO_PENDING\r\n"), this );
					return EAsync_Idle;
				}
			}
		}
		MYTRACE( _T("[%08x]ProcessState_OpenRequest error %d\r\n"), this, GetLastError() );
		return EAsync_Failed;
	
	case ProcessState_EndRequest:
		m_state = ProcessState_EndRequestDone;
		ResetEvent(m_hWaitEvent);
		if (HttpEndRequest(m_hRequest, NULL, 0, 0 ))
		{
			SetEvent(m_hWaitEvent);
			return EAsync_Idle;
		}
		else if (GetLastError() == ERROR_IO_PENDING)
		{
			//MYTRACE( _T("[%08x]ProcessState_EndRequest IO_PENDING\r\n"), this );
			return EAsync_Idle;
		}
		MYTRACE( _T("[%08x]ProcessState_EndRequest error %d\r\n"), this, GetLastError() );
		return EAsync_Failed;
	
	case ProcessState_EndRequestDone:
		// Read Host information 
		{
			m_dwLastValidBufferSize			= 0;
			m_dwLastValidBufferFirstBit		= 0;
				
			_ReadRemoteFileInfo();
			DWORD dwHttpState = m_fileInfo.nStatusCode;
			MYTRACE( _T("[%08x]HttpSendRequest called successfully (HTTP Code:%d - Len:%d)\r\n"), this, m_fileInfo.nStatusCode, m_fileInfo.fileSize );
			
			if(m_iFilesizeExpect>0)
			{				
				if(m_iPositionCurrent>0)
				{
					// Need to match the content size and range !
					int64 iBegin=0, iEnd=0, iEntityLength=0;			
					_QueryHttpRangeInfo(m_hRequest, iBegin, iEnd, iEntityLength);
					ATLASSERT(iEntityLength==0 || iEntityLength==m_iFilesizeExpect);
					if(iBegin!=m_iPositionCurrent || iEntityLength!=m_iFilesizeExpect)
					{
						MYTRACE(_T("[%08x]!Cotent-Range Error: Want :%I64d Got: %I64d  Range:%I64d - %I64d\r\n"), this, m_iFilesizeExpect, iEntityLength, iBegin, iEnd);
						return EAsync_Failed;
					}
				}
				else if(m_iFilesizeExpect!=m_fileInfo.fileSize) // 如果有Offset 则大小不一致的!!
				{
					MYTRACE(_T("[%08x]!Cotent-Length Error: Want :%I64d Got: %I64d\r\n"), this, m_iFilesizeExpect, m_fileInfo.fileSize);
					return EAsync_Failed;
				}
			}
			
			if ( ERROR_SUCCESS == m_dwAsyncError 
				&& (HTTP_STATUS_PARTIAL_CONTENT == dwHttpState || HTTP_STATUS_OK == dwHttpState)  )
			{
				m_state = ProcessState_BeginReceiveData;
				ULONG uReceiveTimeout = HTTP_RECEIVE_TIME_OUT;
				InternetSetOption( m_hRequest, INTERNET_OPTION_RECEIVE_TIMEOUT, &uReceiveTimeout, sizeof( uReceiveTimeout ) );
			}
			else if (HTTP_STATUS_SERVER_ERROR == dwHttpState || 0 == dwHttpState)
			{
				MYTRACE(_T("[%08x]!ProcessState_EndRequestDone Server Error(%d) , to Retry %d\r\n"), this, dwHttpState, m_nRetry);
				return _TryRetry() ? EAsync_Idle : EAsync_Failed;
			}
			else
			{
				MYTRACE(_T("[%08x]!ProcessState_EndRequestDone Server Error(%d), and failed\r\n"), this, dwHttpState);
				return EAsync_Failed;
			}
			//ResetEvent(m_hWaitEvent);
		}
		return EAsync_Opened;
	
	case ProcessState_BeginReceiveData:
	case ProcessState_ReceiveData:
		{
			ResetEvent( m_hWaitEvent );
			m_state = ProcessState_ReceiveData;

#ifdef _DEBUG
			CHAR _lastBuffer[32] = {0};
			DWORD _lastLen = InetBuff.dwBufferLength;
			DWORD _lastBufferLen = 32;
			if(_lastLen>=HTTP_RECEIVE_BUFFER_LENGTH)
				_lastLen = 0;
			if(_lastLen<32)
				_lastBufferLen = _lastLen;

			if(_lastBufferLen>0)
				memcpy(_lastBuffer, m_pBuffer+_lastLen-_lastBufferLen, _lastBufferLen);
#endif 
			ZeroMemory(&InetBuff, sizeof(InetBuff));
			InetBuff.dwStructSize = sizeof(InetBuff);
			InetBuff.lpvBuffer = m_pBuffer;
			InetBuff.dwBufferLength = HTTP_RECEIVE_BUFFER_LENGTH - 1;

			// 继续的的时候有问题
			if(m_iPositionEnd>0)
			{
				ATLASSERT(m_iPositionCurrent<=m_iPositionEnd);
				InetBuff.dwBufferLength = m_iPositionCurrent>m_iPositionEnd ? 0 : min( m_iPositionEnd-m_iPositionCurrent, InetBuff.dwBufferLength );
			}			
			//MYTRACE( _T("Calling InternetReadFileEx\r\n") );
			if (InetBuff.dwBufferLength==0 || InternetReadFileExA(m_hRequest, &InetBuff, IRF_NO_WAIT, (DWORD)this))
			{
				m_state = ProcessState_ReceivedData;
				m_pBuffer[InetBuff.dwBufferLength] = 0;
				SetEvent(m_hWaitEvent);
				if(InetBuff.dwBufferLength>0)
				{
					m_dwLastValidBufferSize		= InetBuff.dwBufferLength;
					m_dwLastValidBufferFirstBit	= m_pBuffer[0];
					return EAsync_Data;
				}
				else //if(InetBuff.dwBufferLength==0)
				{
					MYTRACE(_T("[%08x]!ReceiveData==0 %I64d-%I64d-%I64d AsyncError:%d(Last:%d) LastError:%d\r\n"), this, m_iPositionBegin, m_iPositionCurrent, m_iPositionEnd, m_dwAsyncError, m_dwLastAsyncError, GetLastError());
#ifdef _DEBUG
					CString strLastBuffer;
					for(int i=0; i<_lastBufferLen; ++i)
						strLastBuffer.AppendFormat(_T("%02x "), (BYTE)_lastBuffer[i]);
					MYTRACE(_T("[%08x]!ReceiveData==0 LastBuffer:%d [%s]\r\n"), this, _lastBufferLen, strLastBuffer);
#endif
					return (m_iPositionCurrent==m_iPositionEnd) ? EAsync_Finished : EAsync_Failed;
				}
			}
			else if ( ERROR_IO_PENDING == GetLastError() )
			{
				//MYTRACE( _T("ProcessState_ReceiveData Waiting for InternetReadFileEx to complete\r\n") );
				return EAsync_Idle;
			}
			
			MYTRACE( _T("[%08x]ProcessState_ReceiveData InternetReadFileEx failed, error %d\r\n"), this, GetLastError() );
			if(InetBuff.dwBufferLength==0)
			{
				return _TryRetry() ? EAsync_Idle : EAsync_Failed;
			}
			return EAsync_Failed;
		}
		break;

	case ProcessState_ReceivedData:
		//cout << "-- GOT " << m_iPositionBegin << " - " << m_iDownloaded+InetBuff.dwBufferLength << " / " << m_iPositionEnd - m_iPositionBegin << "  " << InetBuff.dwBufferLength << std::endl;
		// SAVE BUFFER 
		//cout << m_pBuffer << std::endl;
		m_iDownloaded += InetBuff.dwBufferLength;
		m_iPositionCurrent += InetBuff.dwBufferLength;
		m_state = InetBuff.dwBufferLength == 0 ? ProcessState_Finished : ProcessState_ReceiveData;
		return EAsync_Idle;
		break;
	
	case ProcessState_Finished:
		Close();
		return EAsync_Idle;
	case ProcessState_UserCanceled:
	case ProcessState_Failed:
	default:
		MYTRACE( _T("[%08x]Unknown Stat %d\r\n"), this, m_state);
		return EAsync_Failed;
	}
	return EAsync_Failed;
}

BOOL CHttpAsyncIn::_TryRetry( BOOL bSetRetry/*=TRUE*/ )
{
	ATLASSERT(!m_hWaitableTimer);
	if(bSetRetry)
		++m_nRetry;
	if (!bSetRetry || m_nRetry<m_nMaxRetry)
	{
		// Retry  
		m_hWaitableTimer = CreateWaitableTimer(NULL, TRUE, NULL);
		if(m_hWaitableTimer)
		{
			SAFE_CLOSE_INTERNETHANDLE(m_hRequest);
			m_hRequest = NULL;

			LARGE_INTEGER liDueTime;
			liDueTime.QuadPart = -50000000LL;	//5,000,000,0 nano secs = 5s 
			SetWaitableTimer( m_hWaitableTimer, &liDueTime, 0, NULL, NULL, FALSE );

			m_state = ProcessState_OpenRequest;
			return TRUE;
		}
	}
	m_state = ProcessState_Failed;
	return FALSE;
}

VOID CALLBACK CHttpAsyncIn::RequestCallback( HINTERNET hInternet, DWORD dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength )
{
	//MYTRACE( _T("Callback dwInternetStatus:  %d\t Context: %x\r\n"), dwInternetStatus, dwContext );
	CHttpAsyncIn *pThis = (CHttpAsyncIn*)dwContext;
	if(pThis)
		pThis->RequestCallback_(hInternet, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
}

VOID CHttpAsyncIn::RequestCallback_( HINTERNET hInternet, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength )
{
	CObjGuard __guard__(m_objLock, TRUE);
	switch(dwInternetStatus)
	{
		//case INTERNET_STATUS_HANDLE_CREATED:
		//	{		
		//		LPINTERNET_ASYNC_RESULT pResult = (LPINTERNET_ASYNC_RESULT)lpvStatusInformation;
		//		MYTRACE(_T("Inet: Connect handle created: %u, Error: %u\r\n"), pResult->dwResult, pResult->dwError);
		//		m_dwAsyncError = pResult->dwError;
		//	}
		//	break;
	case INTERNET_STATUS_CONNECTING_TO_SERVER:
		MYTRACE(_T("[%08x]ConnectTo %s \r\n"), this, (TCHAR*)lpvStatusInformation);
		break;
	case INTERNET_STATUS_REDIRECT:
		{
			USES_CONVERSION;
			LPTSTR lpszRedirectUrlSrc = OLE2T( ( LPWSTR )lpvStatusInformation );
			MYTRACE(_T("[%08x]!RedirectTo : %s\r\n"), this, lpszRedirectUrlSrc);
		}
		break;
	case INTERNET_STATUS_REQUEST_COMPLETE:
		{			
			LPINTERNET_ASYNC_RESULT pResult = (LPINTERNET_ASYNC_RESULT)lpvStatusInformation;
			//MYTRACE( _T("INTERNET_STATUS_REQUEST_COMPLETE: %u, Error: %u\r\n"), pResult->dwResult, pResult->dwError);
			m_dwLastAsyncError = m_dwAsyncError;
			m_dwAsyncError = pResult->dwError;
			SetEvent(m_hWaitEvent);
		}
		break;
		//case INTERNET_STATUS_HANDLE_CLOSING:
		//	{
		//		//MYTRACE( _T("Inet: Handle closing [req: %s]"), _T("Y") );
		//		m_bHandleValid = FALSE;
		//	}
		//	break;
	//case INTERNET_STATUS_REQUEST_SENT:
	//	{
	//		DWORD *lpBytesSent = (DWORD*)lpvStatusInformation;
	//		MYTRACE( _T("Inet: Bytes Sent:  %d\r\n"), *lpBytesSent );
	//	}
	//	break;
		//case INTERNET_STATUS_RECEIVING_RESPONSE:
		//	// MYTRACE( _T("Inet: Receiving Response\r\n") );
		//	break;
	case INTERNET_STATUS_RESPONSE_RECEIVED:
		{
			DWORD *dwBytesReceived = (DWORD*)lpvStatusInformation;
			ATLASSERT( dwBytesReceived );
			if(dwBytesReceived)
				m_dwLastReceivedByte = *dwBytesReceived;
			//MYTRACE( _T("Inet: Bytes Received:  %d\r\n"), *dwBytesReceived );
			//if (*dwBytesReceived == 0)
			//	m_state = ProcessState_Finished;
		}
		break;
	//case INTERNET_STATUS_RECEIVING_RESPONSE:
	//case INTERNET_STATUS_RESPONSE_RECEIVED:
	//	break;
	default:
		//MYTRACE(_T("Inet: InternetStatus callback: %u\r\n"), dwInternetStatus);
		break;
	}
}

BOOL CHttpAsyncIn::_ConnectServer()
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

	m_hSession = InternetOpen(LIBDOWNLOAD_USER_AGENT, dwAccessType, strProxyServer, NULL, INTERNET_FLAG_ASYNC); // ASYNC Flag
	if (m_hSession == NULL)
	{
		MYTRACE( _T("[%08x]InternetOpen failed, error %d\n"), this, GetLastError() );
		return FALSE;
	}

	// Setup callback function
	if (InternetSetStatusCallback(m_hSession,(INTERNET_STATUS_CALLBACK)&RequestCallback) == INTERNET_INVALID_STATUS_CALLBACK)
	{
		MYTRACE( _T("[%08x]InternetSetStatusCallback failed, error %d\n"), this, GetLastError() );
		return FALSE;
	}

	ULONG uConnectTimeout = HTTP_CONNECT_TIME_OUT;
	InternetSetOption( m_hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &uConnectTimeout, sizeof( uConnectTimeout ) );
	m_bHandleValid = TRUE;
	// First call that will actually complete asynchronously even
	// though there is no network traffic
	m_hConnect = InternetConnect(m_hSession, m_ResourceLocation.szHostName, m_ResourceLocation.nPort
		, NULL,	NULL, INTERNET_SERVICE_HTTP, 0, (DWORD)this);
	if(m_hConnect==NULL)
	{
		MYTRACE( _T("[%08x]InternetConnect failed, error %d\n"), this, GetLastError() );
		return FALSE;
	}
	return TRUE;
}

void CHttpAsyncIn::_CreateAdditionalHeaders( LPTSTR lpszHeaders, DWORD dwLen )
{
	USES_CONVERSION;
	TCHAR szBuffer[ INTERNET_MAX_URL_LENGTH ] = _T( "" );
	TCHAR szHeaders[ INTERNET_MAX_URL_LENGTH ] = _T( "" );

	if ( m_iPositionCurrent>0 )
	{
		_sntprintf( szHeaders, SIZE_OF_ARRAY( szHeaders ) - 1, _T( "Range: bytes=%I64d-\r\n" ), m_iPositionCurrent );
		_tcsncat( lpszHeaders, szHeaders, min( dwLen - _tcslen( lpszHeaders ), _tcslen( szHeaders ) ) );
	}

#if 0 
	int nLen = _tcslen(  );
	if ( nLen > 0 )
	{
		memset( szHeaders, 0, sizeof( szHeaders ) );
		_sntprintf( szHeaders, SIZEOF( szHeaders ) - 1, _T( "Referer: %s\r\n" ), m_szReferer );
		_tcsncat( lpszHeaders, szHeaders, min( dwLen - _tcslen( lpszHeaders ), _tcslen( szHeaders ) ) );
	}

	nLen = _tcslen( m_szUserAgent );
	if ( nLen > 0 )
	{
		memset( szHeaders, 0, sizeof( szHeaders ) );
		_sntprintf( szHeaders, SIZEOF( szHeaders ) - 1, _T( "User-Agent: %s\r\n" ), m_szUserAgent );
		_tcsncat( lpszHeaders, szHeaders, min( dwLen - _tcslen( lpszHeaders ), _tcslen( szHeaders ) ) );
	}
#endif 

	LPCTSTR lpAddHeaders = _T( "Pragma: no-cache\r\nCache-Control: no-cache\r\nConnection: close\r\n" );
	if ( lpAddHeaders )
		_tcsncat( lpszHeaders, lpAddHeaders, min( dwLen - _tcslen( lpszHeaders ), _tcslen( lpAddHeaders ) ) );
}

BOOL CHttpAsyncIn::_ReadRemoteFileInfo()
{
	m_fileInfo.Reset();

	if(!m_hRequest)	return FALSE;
	//_QueryHttpHeaderInfo(TRUE);
	//_QueryHttpHeaderInfo(FALSE);

	m_fileInfo.nStatusCode = _QueryHttpStatus(m_hRequest);
	_QueryContentLength(m_hRequest, m_fileInfo.fileSize);
	m_fileInfo.bRandomAccess = _QueryRandomAccess(m_hRequest, m_fileInfo.fileSize); 
	return TRUE;
}

DWORD CHttpAsyncIn::_QueryHttpStatus( HINTERNET hRequest ) const
{
	if ( hRequest )
	{
		DWORD dwState = 0;
		DWORD dwIndex = 0;
		DWORD dwLen = sizeof( dwState );

		if ( HttpQueryInfo( hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwState, &dwLen, &dwIndex ) )
			return dwState;
	}
	return 0;
}

BOOL CHttpAsyncIn::_QueryContentLength( HINTERNET hRequest, int64 &nContentLength ) const
{
	if ( hRequest )
	{
		TCHAR szBuffer[ MAX_PATH ] = _T( "" );
		DWORD dwLen = sizeof( szBuffer );
		DWORD dwIndex = 0;

		if ( HttpQueryInfo( hRequest, HTTP_QUERY_CONTENT_LENGTH, szBuffer, &dwLen, &dwIndex ) )
		{
			nContentLength = _ttoi64( szBuffer );
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CHttpAsyncIn::_QueryRandomAccess( HINTERNET hRequest, int64 iContentLength ) const
{
	if ( hRequest && -1 != iContentLength )
	{
		TCHAR szBuffer[ MAX_PATH ] = _T( "" );
		DWORD dwLen = sizeof( szBuffer );
		DWORD dwIndex = 0;
		
		if ( HttpQueryInfo( hRequest, HTTP_QUERY_VERSION, szBuffer, &dwLen, &dwIndex ) )
		{
			//MYTRACE(_T("[%08x]QueryRandomAccess : %s\r\n"), this, szBuffer);
#if 0 
			if( 0 == _tcscmp( szBuffer, _T( "HTTP/1.1" ) ) )
			{
				memset( szBuffer, 0, sizeof( szBuffer ) );
				dwLen = sizeof( szBuffer );
				dwIndex = 0;

				if ( HttpQueryInfo( hRequest, HTTP_QUERY_ACCEPT_RANGES, szBuffer, &dwLen, &dwIndex ) )
				{						
					return TRUE;
				}
			}
#else
			return TRUE;
#endif 
		}
	}
	return FALSE;
}

BOOL CHttpAsyncIn::_QueryHttpHeaderInfo(BOOL bReqeust)
{
	LPVOID lpOutBuffer=NULL;
	DWORD dwSize = 0;

retry:
	// This call will fail on the first pass, because 
	// no buffer is allocated.
	DWORD dwFlags = bReqeust ? HTTP_QUERY_FLAG_REQUEST_HEADERS : 0;
	if(!HttpQueryInfo(m_hRequest,HTTP_QUERY_RAW_HEADERS_CRLF|dwFlags, (LPVOID)lpOutBuffer,&dwSize,NULL))
	{ 
		if (GetLastError()==ERROR_HTTP_HEADER_NOT_FOUND)
		{
			// Code to handle the case where the header isn't available.
			return TRUE;
		}		
		else
		{
			// Check for an insufficient buffer.
			if (GetLastError()==ERROR_INSUFFICIENT_BUFFER)
			{
				// Allocate the necessary buffer.
				lpOutBuffer = new char[dwSize];

				// Retry the call.
				goto retry;				
			}		
			else
			{
				// Error handling code.
				if (lpOutBuffer)
				{
					delete [] lpOutBuffer;
				}
				return FALSE;
			}		
		}		
	}	
	
	//printf("%s\r\n", lpOutBuffer);

	if (lpOutBuffer)
	{
		delete [] lpOutBuffer;
	}

	return TRUE;
}

BOOL CHttpAsyncIn::_QueryHttpRangeInfo( HINTERNET hRequest, int64 &iBegin, int64 &iEnd, int64 &iEntityLength )
{
	TCHAR szBuffer[ MAX_PATH ] = _T( "" );
	DWORD dwLen = sizeof( szBuffer );
	DWORD dwIndex = 0;
	if ( HttpQueryInfo( hRequest, HTTP_QUERY_CONTENT_RANGE, szBuffer, &dwLen, &dwIndex ) )
	{
		MYTRACE(_T("[%08x]Content-Range:%s\r\n"), this, szBuffer);
		TCHAR *p1 = _tcschr(szBuffer, _T(' '));
		if(p1)
		{
			++p1;
			iBegin = _ttoi64(p1);
			p1 = _tcschr(p1, _T('-'));
			if(p1)
			{
				++p1;
				iEnd = _ttoi64(p1);
				p1 = _tcschr(p1, _T('/'));
				if(p1)
				{
					++p1;
					iEntityLength = _ttoi64(p1);
					return	TRUE;
				}			
			}
		}
	}
	return FALSE;
}
