#include "StdAfx.h"
#include "WinHttpApi.h"


WinHttpApi::WinHttpApi(void)
	:hWinHttp_(NULL),
	 hWinINet_(NULL),
	 bUseWinHttp_(TRUE),
	 bInited_(FALSE)
{
	ZeroMemory(&WinHttpApi_, sizeof(WinHttpApi_));
	ZeroMemory(&WinINetApi_, sizeof(WinINetApi_));
}

WinHttpApi::~WinHttpApi(void)
{
}

BOOL WinHttpApi::Initialize()
{
	if (!bInited_)
	{
		bInited_ = LoadWinHttp();
		if (bInited_)
		{
			bUseWinHttp_ = TRUE;
		}
		else
		{
			bInited_ = LoadWinINet();
			bUseWinHttp_ = FALSE;
		}
	}

	return bInited_;
}

BOOL WinHttpApi::Uninitialize()
{
	bInited_ = FALSE;

	ZeroMemory(&WinHttpApi_, sizeof(WinHttpApi_));
	ZeroMemory(&WinINetApi_, sizeof(WinINetApi_));
	
	if (hWinHttp_)
		::FreeLibrary(hWinHttp_);
	hWinHttp_ = NULL;

	if (hWinINet_)
		::FreeLibrary(hWinINet_);
	hWinINet_ = NULL;

	return TRUE;
}

BOOL WinHttpApi::LoadWinHttp()
{
	TCHAR szSysDir[MAX_PATH] = { 0 };
	::GetSystemDirectory(szSysDir, MAX_PATH);

	_tcscat_s(szSysDir, _T("\\winhttp.dll"));

	hWinHttp_ = ::LoadLibrary(szSysDir);
	if (hWinHttp_)
	{
		WinHttpApi_.pWinHttpOpen				= (tWinHttpOpen)::GetProcAddress(hWinHttp_,					"WinHttpOpen");
		WinHttpApi_.pWinHttpConnect				= (tWinHttpConnect)::GetProcAddress(hWinHttp_,				"WinHttpConnect");
		WinHttpApi_.pWinHttpOpenRequest			= (tWinHttpOpenRequest)::GetProcAddress(hWinHttp_,			"WinHttpOpenRequest");
		WinHttpApi_.pWinHttpAddRequestHeaders	= (tWinHttpAddRequestHeaders)::GetProcAddress(hWinHttp_,	"WinHttpAddRequestHeaders");
		WinHttpApi_.pWinHttpSendRequest			= (tWinHttpSendRequest)::GetProcAddress(hWinHttp_,			"WinHttpSendRequest");
		WinHttpApi_.pWinHttpReceiveResponse		= (tWinHttpReceiveResponse)::GetProcAddress(hWinHttp_,		"WinHttpReceiveResponse");
		WinHttpApi_.pWinHttpQueryHeaders		= (tWinHttpQueryHeaders)::GetProcAddress(hWinHttp_,			"WinHttpQueryHeaders");
		WinHttpApi_.pWinHttpReadData			= (tWinHttpReadData)::GetProcAddress(hWinHttp_,				"WinHttpReadData");
		WinHttpApi_.pWinHttpCloseHandle			= (tWinHttpCloseHandle)::GetProcAddress(hWinHttp_,			"WinHttpCloseHandle");
		WinHttpApi_.pWinHttpCrackUrl			= (tWinHttpCrackUrl)::GetProcAddress(hWinHttp_,				"WinHttpCrackUrl");
		WinHttpApi_.pWinHttpSetOption			= (tWinHttpSetOption)::GetProcAddress(hWinHttp_,			"WinHttpSetOption");
		// WinINet没这个函数！
		WinHttpApi_.pWinHttpSetTimeouts			= (tWinHttpSetTimeouts)::GetProcAddress(hWinHttp_,			"WinHttpSetTimeouts");
		
		//
		WinHttpApi_.bInited = (WinHttpApi_.pWinHttpOpen 
							   && WinHttpApi_.pWinHttpAddRequestHeaders
							   && WinHttpApi_.pWinHttpCloseHandle
							   && WinHttpApi_.pWinHttpConnect
							   && WinHttpApi_.pWinHttpOpenRequest
							   && WinHttpApi_.pWinHttpQueryHeaders
							   && WinHttpApi_.pWinHttpReadData
							   && WinHttpApi_.pWinHttpReceiveResponse
							   && WinHttpApi_.pWinHttpSendRequest
							   && WinHttpApi_.pWinHttpCrackUrl
							   && WinHttpApi_.pWinHttpSetOption
							   && WinHttpApi_.pWinHttpSetTimeouts);
	}


	return WinHttpApi_.bInited;
}

BOOL WinHttpApi::LoadWinINet()
{
	TCHAR szSysDir[MAX_PATH] = { 0 };
	::GetSystemDirectory(szSysDir, MAX_PATH);

	_tcscat_s(szSysDir, _T("\\wininet.dll"));

	hWinINet_ = ::LoadLibrary(szSysDir);
	if (hWinINet_)
	{
		WinINetApi_.pInternetOpen				= (tInternetOpen)::GetProcAddress(hWinINet_,					"InternetOpenW");
		WinINetApi_.pInternetConnect			= (tInternetConnect)::GetProcAddress(hWinINet_,					"InternetConnectW");
		WinINetApi_.pHttpOpenRequest			= (tHttpOpenRequest)::GetProcAddress(hWinINet_,					"HttpOpenRequestW");
		WinINetApi_.pHttpAddRequestHeaders		= (tHttpAddRequestHeaders)::GetProcAddress(hWinINet_,			"HttpAddRequestHeadersW");
		WinINetApi_.pHttpSendRequest			= (tHttpSendRequest)::GetProcAddress(hWinINet_,					"HttpSendRequestW");
		//WinINetApi_.pWinHttpReceiveResponse		= (tWinHttpReceiveResponse)::GetProcAddress(hWinHttp_,		"WinHttpReceiveResponse");
		WinINetApi_.pHttpQueryInfo				= (tHttpQueryInfo)::GetProcAddress(hWinINet_,					"HttpQueryInfoW");
		WinINetApi_.pInternetReadFile			= (tInternetReadFile)::GetProcAddress(hWinINet_,				"InternetReadFile");
		WinINetApi_.pInternetCloseHandle		= (tInternetCloseHandle)::GetProcAddress(hWinINet_,				"InternetCloseHandle");
		WinINetApi_.pInternetCrackUrl			= (tInternetCrackUrl)::GetProcAddress(hWinINet_,				"InternetCrackUrlW");
		WinINetApi_.pInternetSetOption			= (tInternetSetOption)::GetProcAddress(hWinINet_,				"InternetSetOptionW");

		//
		WinINetApi_.bInited = (WinINetApi_.pInternetOpen 
			&& WinINetApi_.pInternetConnect
			&& WinINetApi_.pHttpOpenRequest
			&& WinINetApi_.pHttpAddRequestHeaders
			&& WinINetApi_.pHttpSendRequest
			&& WinINetApi_.pHttpQueryInfo
			&& WinINetApi_.pInternetReadFile
			&& WinINetApi_.pInternetCloseHandle
			&& WinINetApi_.pInternetCrackUrl
			&& WinINetApi_.pInternetSetOption);
	}


	return WinINetApi_.bInited;
}

//////////////////////////////////////////////////////////////////////////
//
HINTERNET WinHttpApi::OpenSession(LPCWSTR userAgent/* = NULL*/)
{
	HINTERNET hSession = NULL;

	if (bUseWinHttp_)
		hSession = (WinHttpApi_.pWinHttpOpen)(userAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, NULL/*WINHTTP_FLAG_ASYNC*/);
	else
		hSession = (WinINetApi_.pInternetOpen)(userAgent, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, INTERNET_FLAG_MAKE_PERSISTENT);
	
	return hSession;
}

HINTERNET WinHttpApi::Connect(HINTERNET hSession, LPCWSTR serverAddr, int portNo)
{
	HINTERNET hConnect = NULL;

	if (bUseWinHttp_)
		hConnect = (WinHttpApi_.pWinHttpConnect)(hSession, serverAddr, (INTERNET_PORT) portNo, 0);
	else
		hConnect = (WinINetApi_.pInternetConnect)(hSession, serverAddr, portNo, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

	return hConnect;
}

HINTERNET WinHttpApi::OpenRequest(HINTERNET hConnect, LPCWSTR verb, LPCWSTR objectName, int scheme)
{
	HINTERNET hRequest = NULL;
	DWORD flags = 0;

	if (bUseWinHttp_)
	{
		if (INTERNET_SCHEME_HTTPS == scheme) 
			flags |= WINHTTP_FLAG_SECURE;
		hRequest = (WinHttpApi_.pWinHttpOpenRequest)(hConnect, verb, objectName, L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
	}
	else
	{
		if (INTERNET_SCHEME_HTTPS == scheme) 
			flags |= INTERNET_FLAG_SECURE;
		hRequest = (WinINetApi_.pHttpOpenRequest)(hConnect, verb, objectName, L"HTTP/1.1", NULL, NULL, flags, 0);
	}

	return hRequest;
}

BOOL WinHttpApi::AddRequestHeaders(HINTERNET hRequest, LPCWSTR header)
{
	BOOL bSuccess = FALSE;
	SIZE_T len = lstrlenW(header);

	if (bUseWinHttp_)
		bSuccess = (WinHttpApi_.pWinHttpAddRequestHeaders)(hRequest, header, DWORD(len), WINHTTP_ADDREQ_FLAG_ADD);
	else
		bSuccess = (WinINetApi_.pHttpAddRequestHeaders)(hRequest, header, DWORD(len), HTTP_ADDREQ_FLAG_ADD);

	return bSuccess;
}

BOOL WinHttpApi::SendRequest(HINTERNET hRequest, const void* body, DWORD size)
{
	BOOL bSuccess = FALSE;

	if (bUseWinHttp_)
		bSuccess = (WinHttpApi_.pWinHttpSendRequest)(hRequest, 0, 0, const_cast<void*>(body), size, size, 0);
	else
		bSuccess = (WinINetApi_.pHttpSendRequest)(hRequest, 0, 0, const_cast<void*>(body), size);

	return bSuccess;
}

BOOL WinHttpApi::EndRequest(HINTERNET hRequest)
{
	BOOL bSuccess = FALSE;

	if (bUseWinHttp_)
		bSuccess = (WinHttpApi_.pWinHttpReceiveResponse)(hRequest, 0);
	else
		// if you use HttpSendRequestEx to send request then use HttpEndRequest in here!
		bSuccess = TRUE;

	return bSuccess;
}

BOOL WinHttpApi::QueryInfo(HINTERNET hRequest, int queryId, char* szBuf, DWORD* pdwSize)
{
	BOOL bSuccess = FALSE;

	if (bUseWinHttp_)
		bSuccess = (WinHttpApi_.pWinHttpQueryHeaders)(hRequest, (DWORD) queryId, 0, szBuf, pdwSize, 0);
	else
		bSuccess = (WinINetApi_.pHttpQueryInfo)(hRequest, queryId, szBuf, pdwSize, 0);

	return bSuccess;
}

BOOL WinHttpApi::ReadData(HINTERNET hRequest, void* buffer, DWORD length, DWORD* cbRead)
{
	BOOL bSuccess = FALSE;

	if (bUseWinHttp_)
		bSuccess = (WinHttpApi_.pWinHttpReadData)(hRequest, buffer, length, cbRead);
	else
		bSuccess = (WinINetApi_.pInternetReadFile)(hRequest, buffer, length, cbRead);

	return bSuccess;
}

void WinHttpApi::CloseInternetHandle(HINTERNET hInternet)
{
	if (hInternet)
	{
		if (bUseWinHttp_)
			(WinHttpApi_.pWinHttpCloseHandle)(hInternet);
		else
			(WinINetApi_.pInternetCloseHandle)(hInternet);
	}
}

BOOL WinHttpApi::CrackedUrl(IN LPCWSTR url, IN OUT URL_COMPONENTS* _pUrlComponent)
{
	BOOL bSuccess = FALSE;

	if (url && _pUrlComponent)
	{
		if (bUseWinHttp_)
			bSuccess = (WinHttpApi_.pWinHttpCrackUrl)(url, 0, ICU_ESCAPE, _pUrlComponent);
		else
			bSuccess = (WinINetApi_.pInternetCrackUrl)(url, 0, ICU_ESCAPE, _pUrlComponent);
	}
	
	return bSuccess;
}

BOOL WinHttpApi::SetTimeouts(IN HINTERNET hInternet, IN int nResolveTimeout, IN int nConnectTimeout, IN int nSendTimeout, IN int nReceiveTimeout)
{
	BOOL bSuccess = FALSE;

	if (bUseWinHttp_)
		bSuccess = (WinHttpApi_.pWinHttpSetTimeouts)(hInternet, nResolveTimeout, nConnectTimeout, nSendTimeout, nReceiveTimeout);
	else
	{
		//bSuccess = (WinINetApi_.pInternetReadFile)(hRequest, buffer, length, cbRead);
	}

	return bSuccess;
}