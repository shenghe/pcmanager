#pragma once
#include <wininet.h>
#include <vector>
#include <libdownload\libDownload.h>
#include "Macro.h"

typedef CComAutoCriticalSection                     CObjLock;
typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;

const int HTTP_CONNECT_TIME_OUT = 30000;			// 30s 
const int HTTP_SEND_TIME_OUT = 30000;				// 30s 
const int HTTP_RECEIVE_TIME_OUT = 30000;			// 30s 

const int HTTP_RECEIVE_BUFFER_LENGTH = 1024*8;	// 8k
const int64 MIN_BLOCK = 32 * 1024;
const int64 MIN_BLOCK_TOSPLIT = MIN_BLOCK * 2;		// 128k 
const int DOWNLOAD_DEFAULT_COCURRENT = 4;			// 默认并发数
const int DOWNLOAD_SPEED_TEST_INTERVAL = 1000;		// 1s 测速 
const int DOWNLOAD_PROGRESS_REPORT_INTERVAL = 300; // 0.3s 
const int DOWNLOAD_MAINURL_MAXRETYR = 3;			// Main url 's max retry 
const int DOWNLOAD_FAIL_RETRY_INTERVAL = 3000;		// 3s 


#define LIBDOWNLOAD_USER_AGENT _T("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 2.0.50727; .NET CLR 3.0.04506.648; .NET CLR 3.5.8888.8888;)")


struct UrlInfo
{
	TCHAR szUrl[ MAX_PATH ];

	INTERNET_PORT	nPort;
	INTERNET_SCHEME nScheme;
	TCHAR szHostName[ MAX_PATH ];
	TCHAR szUrlPath[ MAX_PATH ];
	TCHAR szUserName[ MAX_PATH ];
	TCHAR szPassword[ MAX_PATH ];
	TCHAR szExtraInfo[ MAX_PATH ];
};


