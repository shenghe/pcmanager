#include "StdAfx.h"
#include "WinHttpWrapper.h"
#include <string>

WinHttpWrapper::WinHttpWrapper(void)
{
}

WinHttpWrapper::~WinHttpWrapper(void)
{
}

BOOL WinHttpWrapper::Initialize()
{
	return WinHttp_.Initialize();
}

BOOL WinHttpWrapper::Uninitialize()
{
	return WinHttp_.Uninitialize();
}

long SendHttpRequestHelper(IN WinHttpApi http,
						   IN LPCWSTR lpszUrl, 
						   OUT HINTERNET& hSession, 
						   OUT HINTERNET& hConnect, 
						   OUT HINTERNET& hRequest)
{
	long lRetCode = -8;

	const int BUF_LEN = 256;
	WCHAR host[BUF_LEN] = { 0 };
	const WCHAR * pszRequestUrl = NULL;
	// 最大 URL 长度是在 Internet Explorer 中的 2,083 字符
	// WCHAR path[2083 + 1] = { 0 };
	// WCHAR extra[256] = { 0 };

	URL_COMPONENTS uc = { 0 };

	uc.dwStructSize = sizeof(uc);
	uc.lpszHostName = host;
	uc.dwHostNameLength = BUF_LEN;
	//uc.lpszUrlPath = path;
	//uc.dwUrlPathLength = BUF_LEN;
	//uc.lpszExtraInfo = extra;
	//uc.dwExtraInfoLength = BUF_LEN;
	
	// 注意http://www.zcnet4.com/xx/do?z1=1&z2=2
	// 会拆解为 HostName:www.zcnet4.com、UrlPath:xx/do、szExtraInfo:?z1=1&z2=2
	http.CrackedUrl(lpszUrl, &uc);

	// 不想分配内存来做UrlPath和ExtraInfo的合并。by ZC. 2010-3-13 22:20
	pszRequestUrl = _tcsstr(lpszUrl, uc.lpszHostName);
	if (pszRequestUrl)
	{
		pszRequestUrl += (uc.dwHostNameLength/* + 1*/);
		if (_T(':') == *pszRequestUrl)
		{
			TCHAR szPort[32] = { 0 };
			_stprintf_s(szPort, _T(":%d"), uc.nPort);
			pszRequestUrl += _tcslen(szPort);
		}
	}
	else
		goto Exit0;

	// Open session.
	hSession = http.OpenSession(uc.lpszHostName);
	if (!hSession)
	{
		lRetCode = -2;
		goto Exit0;
	}

	// Connect.
	hConnect = http.Connect(hSession, uc.lpszHostName, uc.nPort);
	if (!hConnect)
	{
		lRetCode = -3;
		goto Exit0;
	}

	// Open request.
	hRequest = http.OpenRequest(hConnect, L"GET", pszRequestUrl, uc.nScheme);
	if (!hRequest)
	{
		lRetCode = -4;
		goto Exit0;
	}

	// Add request header.
	if (!http.AddRequestHeaders(hRequest, L"UserAgent:Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0;)"))
	{
		lRetCode = -5;
		goto Exit0;
	}

	// Send post data.
	if (!http.SendRequest(hRequest, NULL, NULL)) 
	{
		lRetCode = ::GetLastError();
		goto Exit0;
	}

	// End request
	if (!http.EndRequest(hRequest))
	{
		lRetCode = ::GetLastError();
		goto Exit0;
	}
	
	lRetCode = 0;
Exit0:
	return lRetCode;
}

long WinHttpWrapper::SendHttpRequest(LPCWSTR lpszUrl,
									 IOutStream * _pIOutStream,
									 IStatusCallback * _pIStatusCallback/* = NULL*/)
{
	long lResult = -11;
	HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
	
	const DWORD cdwSize = 1024;
	char szBuf[cdwSize + 2] = { 0 };
	DWORD dwSize = cdwSize;

	lResult = SendHttpRequestHelper(WinHttp_, lpszUrl, hSession, hConnect, hRequest);
	if (0 != lResult)
	{
		if (_pIStatusCallback)
			_pIStatusCallback->OnStatusCallback(enumHttpStatus_ErrorCode, lResult);
		
		return lResult;
	}

	// Query header info.
#ifdef USE_WINHTTP
	int contextLengthId = WINHTTP_QUERY_CONTENT_LENGTH;
	int statusCodeId = WINHTTP_QUERY_STATUS_CODE;
	int statusTextId = WINHTTP_QUERY_STATUS_TEXT;
#else
	int contextLengthId = HTTP_QUERY_CONTENT_LENGTH;
	int statusCodeId = HTTP_QUERY_STATUS_CODE;
	int statusTextId = HTTP_QUERY_STATUS_TEXT;
#endif
	dwSize = cdwSize;
	ZeroMemory(szBuf, sizeof(szBuf));
	if (WinHttp_.QueryInfo(hRequest, statusCodeId, szBuf, &dwSize))
	{
		szBuf[dwSize] = 0;
		u_int uStatusCode = static_cast<u_int>(_wtoi64((WCHAR*)szBuf));
		
		if (200 == uStatusCode)
			lResult = 0;
		else
			lResult = uStatusCode;

		if (_pIStatusCallback)
			_pIStatusCallback->OnStatusCallback(enumHttpStatus_StatusCode, uStatusCode);
	}

	dwSize = cdwSize;
	ZeroMemory(szBuf, sizeof(szBuf));
	if (WinHttp_.QueryInfo(hRequest, contextLengthId, szBuf, &dwSize))
	{
		szBuf[dwSize] = 0;
		if (_pIStatusCallback)
		{
			WCHAR * _pNum = (WCHAR*)szBuf;
			_pIStatusCallback->OnStatusCallback(enumHttpStatus_ContentLength, static_cast<int>(_wtoi64(_pNum)));
		}
	}

#ifdef _DEBUG
	dwSize = cdwSize;
	ZeroMemory(szBuf, sizeof(szBuf));
	if (WinHttp_.QueryInfo(hRequest, statusTextId, szBuf, &dwSize))
		szBuf[dwSize] = 0;
#endif

	// read data.
	while (true)
	{
		dwSize = cdwSize;
		if (!WinHttp_.ReadData(hRequest, szBuf, dwSize, &dwSize))
		{
			if (_pIStatusCallback)
				_pIStatusCallback->OnStatusCallback(enumHttpStatus_ErrorCode, ::GetLastError());
			
			break;
		}

		if (dwSize <= 0)
			break;

		szBuf[dwSize] = 0;

		if (_pIOutStream)
			_pIOutStream->Write((const byte*)szBuf, dwSize, NULL);

		if (_pIStatusCallback)
			_pIStatusCallback->OnStatusCallback(enumHttpStatus_Progress, dwSize);
	}

	WinHttp_.CloseInternetHandle(hRequest);
	WinHttp_.CloseInternetHandle(hConnect);
	WinHttp_.CloseInternetHandle(hSession);

	return lResult;
}

//////////////////////////////////////////////////////////////////////////
// WinHttpHandle

WinHttpHandle::WinHttpHandle(IN WinHttpApi& _WinHttp)
	: WinHttp_(_WinHttp),
	  m_handle(NULL)
{

}

WinHttpHandle::~WinHttpHandle()
{
  Close();
}

BOOL WinHttpHandle::Attach(HINTERNET handle)
{
  ASSERT(0 == m_handle);
  m_handle = handle;
  return 0 != m_handle;
}

HINTERNET WinHttpHandle::Detach()
{
  HANDLE handle = m_handle;
  m_handle = 0;
  return handle;
}

void WinHttpHandle::Close()
{
  if (0 != m_handle)
  {   
	  WinHttp_.CloseInternetHandle(m_handle);
	  m_handle = 0;
  }
}

HRESULT WinHttpHandle::SetOption(DWORD option, const void* value, DWORD length)
{
	/*if (!::WinHttpSetOption(m_handle,
	  option,
	  const_cast<void*>(value),
	  length))
  {
	  return HRESULT_FROM_WIN32(::GetLastError());
  }*/

  return S_OK;
}

HRESULT WinHttpHandle::QueryOption(DWORD option, void* value, DWORD& length) const
{
  /*if (!::WinHttpQueryOption(m_handle,
	  option,
	  value,
	  &length))
  {
	  return HRESULT_FROM_WIN32(::GetLastError());
  }*/

  return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// WinHttpSession
WinHttpSession::WinHttpSession(IN WinHttpApi& _WinHttp)
	: WinHttpHandle(_WinHttp)
{

}