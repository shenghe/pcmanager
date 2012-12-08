#pragma once

#ifndef __out_data_source
	#define __out_data_source(x) 
#endif
#include "winhttp.h"

// WinHttp.dll导出函数。
typedef HINTERNET (WINAPI *tWinHttpOpen)( __in_opt LPCWSTR pszAgentW, __in DWORD dwAccessType, __in_opt LPCWSTR pszProxyW, __in_opt LPCWSTR pszProxyBypassW, __in DWORD dwFlags );
typedef HINTERNET (WINAPI *tWinHttpConnect)( IN HINTERNET hSession, IN LPCWSTR pswzServerName, IN INTERNET_PORT nServerPort, IN DWORD dwReserved );
typedef HINTERNET (WINAPI *tWinHttpOpenRequest)( IN HINTERNET hConnect, IN LPCWSTR pwszVerb, IN LPCWSTR pwszObjectName, IN LPCWSTR pwszVersion, IN LPCWSTR pwszReferrer OPTIONAL, IN LPCWSTR FAR * ppwszAcceptTypes OPTIONAL, IN DWORD dwFlags );
typedef BOOL (WINAPI *tWinHttpAddRequestHeaders)( IN HINTERNET hRequest, IN LPCWSTR pwszHeaders, IN DWORD dwHeadersLength, IN DWORD dwModifiers );
typedef BOOL (WINAPI *tWinHttpSendRequest)( IN HINTERNET hRequest, IN LPCWSTR pwszHeaders OPTIONAL, IN DWORD dwHeadersLength, IN LPVOID lpOptional OPTIONAL, IN DWORD dwOptionalLength, IN DWORD dwTotalLength, IN DWORD_PTR dwContext );
typedef BOOL (WINAPI *tWinHttpReceiveResponse)( IN HINTERNET hRequest, IN LPVOID lpReserved );
typedef BOOL (WINAPI *tWinHttpQueryHeaders)( IN HINTERNET hRequest, IN DWORD dwInfoLevel, IN LPCWSTR pwszName OPTIONAL, OUT __out_data_source(NETWORK) LPVOID lpBuffer OPTIONAL, IN OUT LPDWORD lpdwBufferLength, IN OUT LPDWORD lpdwIndex OPTIONAL );
typedef BOOL (WINAPI *tWinHttpReadData)( IN HINTERNET hRequest, IN __out_data_source(NETWORK) LPVOID lpBuffer, IN DWORD dwNumberOfBytesToRead, OUT LPDWORD lpdwNumberOfBytesRead );
typedef BOOL (WINAPI *tWinHttpCloseHandle)( IN HINTERNET hInternet );
typedef BOOL (WINAPI *tWinHttpCrackUrl)(__in_ecount(dwUrlLength) LPCWSTR pwszUrl, __in DWORD dwUrlLength, __in DWORD dwFlags, __inout LPURL_COMPONENTS lpUrlComponents);
typedef BOOL (WINAPI *tWinHttpSetOption)(IN HINTERNET hInternet, IN DWORD dwOption, IN LPVOID lpBuffer, IN DWORD dwBufferLength);
typedef BOOL (WINAPI *tWinHttpSetTimeouts)(IN HINTERNET hInternet, IN int nResolveTimeout, IN int nConnectTimeout, IN int nSendTimeout, IN int nReceiveTimeout);

struct WinHttpApiFunc
{
	BOOL							bInited;
	tWinHttpOpen					pWinHttpOpen;
	tWinHttpConnect					pWinHttpConnect;
	tWinHttpOpenRequest				pWinHttpOpenRequest;
	tWinHttpAddRequestHeaders		pWinHttpAddRequestHeaders;
	tWinHttpSendRequest				pWinHttpSendRequest;
	tWinHttpReceiveResponse			pWinHttpReceiveResponse;
	tWinHttpQueryHeaders			pWinHttpQueryHeaders;
	tWinHttpReadData				pWinHttpReadData;
	tWinHttpCloseHandle				pWinHttpCloseHandle;
	tWinHttpCrackUrl				pWinHttpCrackUrl;
	tWinHttpSetOption				pWinHttpSetOption;
	tWinHttpSetTimeouts				pWinHttpSetTimeouts;
};

// WinINet.dll导出函数。
typedef HINTERNET (WINAPI *tInternetOpen)(__in_opt LPCWSTR lpszAgent, __in DWORD dwAccessType, __in_opt LPCWSTR lpszProxy, __in_opt LPCWSTR lpszProxyBypass, __in DWORD dwFlags);
typedef HINTERNET (WINAPI *tInternetConnect)(__in HINTERNET hInternet,__in LPCWSTR lpszServerName,__in INTERNET_PORT nServerPort,__in_opt LPCWSTR lpszUserName,__in_opt LPCWSTR lpszPassword,__in DWORD dwService,__in DWORD dwFlags,__in_opt DWORD_PTR dwContext);
typedef HINTERNET (WINAPI *tHttpOpenRequest)(__in HINTERNET hConnect, __in_opt LPCWSTR lpszVerb, __in_opt LPCWSTR lpszObjectName, __in_opt LPCWSTR lpszVersion, __in_opt LPCWSTR lpszReferrer, __in_opt LPCWSTR FAR * lplpszAcceptTypes, __in DWORD dwFlags, __in_opt DWORD_PTR dwContext);
typedef BOOL (WINAPI *tHttpAddRequestHeaders)(__in HINTERNET hRequest, __in_ecount(dwHeadersLength) LPCWSTR lpszHeaders, __in DWORD dwHeadersLength, __in DWORD dwModifiers);
typedef BOOL (WINAPI *tHttpSendRequest)(__in HINTERNET hRequest, __in_ecount_opt(dwHeadersLength) LPCWSTR lpszHeaders, __in DWORD dwHeadersLength, __in_bcount_opt(dwOptionalLength) LPVOID lpOptional, __in DWORD dwOptionalLength );
//typedef BOOL (WINAPI *tWinHttpReceiveResponse)( IN HINTERNET hRequest, IN LPVOID lpReserved );
typedef BOOL (WINAPI *tHttpQueryInfo)(__in HINTERNET hRequest, __in DWORD dwInfoLevel, __inout_bcount_opt(*lpdwBufferLength) LPVOID lpBuffer, __inout LPDWORD lpdwBufferLength, __inout_opt LPDWORD lpdwIndex);
typedef BOOL (WINAPI *tInternetReadFile)(__in HINTERNET hFile, __out_bcount(dwNumberOfBytesToRead) LPVOID lpBuffer, __in DWORD dwNumberOfBytesToRead, __out LPDWORD lpdwNumberOfBytesRead);
typedef BOOL (WINAPI *tInternetCloseHandle)( IN HINTERNET hInternet );
typedef BOOL (WINAPI *tInternetCrackUrl)(__in_ecount(dwUrlLength) LPCWSTR pwszUrl, __in DWORD dwUrlLength, __in DWORD dwFlags, __inout LPURL_COMPONENTS lpUrlComponents);
typedef BOOL (WINAPI *tInternetSetOption)(__in_opt HINTERNET hInternet, __in DWORD dwOption, __in_opt LPVOID lpBuffer, __in DWORD dwBufferLength);

struct WinINetApiFunc
{
	BOOL							bInited;
	tInternetOpen					pInternetOpen;
	tInternetConnect				pInternetConnect;
	tHttpOpenRequest				pHttpOpenRequest;
	tHttpAddRequestHeaders			pHttpAddRequestHeaders;
	tHttpSendRequest				pHttpSendRequest;
	//tWinHttpReceiveResponse pWinHttpReceiveResponse;
	tHttpQueryInfo					pHttpQueryInfo;
	tInternetReadFile				pInternetReadFile;
	tInternetCloseHandle			pInternetCloseHandle;
	tInternetCrackUrl				pInternetCrackUrl;
	tInternetSetOption				pInternetSetOption;
};

// WinHttpApi
class WinHttpApi
{
public:
	WinHttpApi(void);
	~WinHttpApi(void);
public:
	BOOL Initialize();
	BOOL Uninitialize();
public:
	HINTERNET OpenSession(LPCWSTR userAgent = NULL);
	HINTERNET Connect(HINTERNET hSession, LPCWSTR serverAddr, int portNo);
	HINTERNET OpenRequest(HINTERNET hConnect, LPCWSTR verb, LPCWSTR objectName, int scheme);
	BOOL AddRequestHeaders(HINTERNET hRequest, LPCWSTR header);
	BOOL SendRequest(HINTERNET hRequest, const void* body, DWORD size);
	BOOL EndRequest(HINTERNET hRequest);
	BOOL QueryInfo(HINTERNET hRequest, int queryId, char* szBuf, DWORD* pdwSize);
	BOOL ReadData(HINTERNET hRequest, void* buffer, DWORD length, DWORD* cbRead);
	void CloseInternetHandle(HINTERNET hInternet);
public:
	BOOL CrackedUrl(IN LPCWSTR url, IN OUT URL_COMPONENTS* _pUrlComponent);
	BOOL SetTimeouts(IN HINTERNET hInternet, IN int nResolveTimeout, IN int nConnectTimeout, IN int nSendTimeout, IN int nReceiveTimeout);
protected:
	BOOL LoadWinHttp();
	BOOL LoadWinINet();
protected:
	WinHttpApiFunc WinHttpApi_;
	WinINetApiFunc WinINetApi_;
protected:
	HMODULE hWinHttp_;
	HMODULE hWinINet_;
	BOOL	bUseWinHttp_;
	BOOL	bInited_;
};
