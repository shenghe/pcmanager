/********************************************************************
	created:	2010/11/05
	filename: 	beikesafeminidownloader.h
	author:		Zhang KangZong
	
	purpose:	Use WinHTTP
*********************************************************************/

#pragma once

#include "../../publish/winhttp/winhttp.h"
#include <WinInet.h>
#pragma comment(lib, "wininet.lib")

#include <atlfile.h>

#pragma warning(push)
#pragma warning(disable: 4819)
#include <atlsecurity.h>
#pragma warning(pop)

// 下载器http Agent名
#define VKSDL_HTTP_AGENT_NAME      L"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)"

// TCP连接超时
#define VKSDL_TCP_CONNECT_TIMEOUT  10000
// TCP接收超时
#define VKSDL_TCP_RECEIVE_TIMEOUT  5000

// 网络传输buffer大小
#define VKSDL_NETWORK_BUFFER_SIZE  1024

#define VKSDL_REQUEST_HEADER_CONNECTION_CLOSE          _T("Connection: Close")

typedef HINTERNET (WINAPI *tWinHttpOpen)(LPCWSTR pszAgentW, DWORD dwAccessType, __in_opt LPCWSTR pszProxyW, __in_opt LPCWSTR pszProxyBypassW, DWORD dwFlags );
typedef HINTERNET (WINAPI *tWinHttpConnect)( IN HINTERNET hSession, IN LPCWSTR pswzServerName, IN INTERNET_PORT nServerPort, IN DWORD dwReserved );
typedef HINTERNET (WINAPI *tWinHttpOpenRequest)( IN HINTERNET hConnect, IN LPCWSTR pwszVerb, IN LPCWSTR pwszObjectName, IN LPCWSTR pwszVersion, IN LPCWSTR pwszReferrer OPTIONAL, IN LPCWSTR FAR * ppwszAcceptTypes OPTIONAL, IN DWORD dwFlags );
typedef BOOL (WINAPI *tWinHttpAddRequestHeaders)( IN HINTERNET hRequest, IN LPCWSTR pwszHeaders, IN DWORD dwHeadersLength, IN DWORD dwModifiers );
typedef BOOL (WINAPI *tWinHttpSendRequest)( IN HINTERNET hRequest, IN LPCWSTR pwszHeaders OPTIONAL, IN DWORD dwHeadersLength, IN LPVOID lpOptional OPTIONAL, IN DWORD dwOptionalLength, IN DWORD dwTotalLength, IN DWORD_PTR dwContext );
typedef BOOL (WINAPI *tWinHttpReceiveResponse)( IN HINTERNET hRequest, IN LPVOID lpReserved );
typedef BOOL (WINAPI *tWinHttpQueryHeaders)( IN HINTERNET hRequest, IN DWORD dwInfoLevel, IN LPCWSTR pwszName OPTIONAL, OUT LPVOID lpBuffer OPTIONAL, IN OUT LPDWORD lpdwBufferLength, IN OUT LPDWORD lpdwIndex OPTIONAL );
typedef BOOL (WINAPI *tWinHttpReadData)( IN HINTERNET hRequest, IN LPVOID lpBuffer, IN DWORD dwNumberOfBytesToRead, OUT LPDWORD lpdwNumberOfBytesRead );
typedef BOOL (WINAPI *tWinHttpCloseHandle)( IN HINTERNET hInternet );
typedef BOOL (WINAPI *tWinHttpCrackUrl)(__in_ecount(dwUrlLength) LPCWSTR pwszUrl, DWORD dwUrlLength, DWORD dwFlags, __inout LPURL_COMPONENTS lpUrlComponents);
typedef BOOL (WINAPI *tWinHttpSetOption)(IN HINTERNET hInternet, IN DWORD dwOption, IN LPVOID lpBuffer, IN DWORD dwBufferLength);


class CBkTemporaryFile
{
public:
	CBkTemporaryFile() throw()
	{
	}

	~CBkTemporaryFile() throw()
	{
		// Ensure that the temporary file is closed and deleted,
		// if necessary.
		if (m_file.m_h != NULL)
		{
			Close();
		}
	}

	HRESULT Create(__in_opt LPCTSTR pszDir = NULL, __in DWORD dwDesiredAccess = GENERIC_WRITE) throw()
	{
		TCHAR szPath[_MAX_PATH]; 
		TCHAR tmpFileName[_MAX_PATH]; 

		ATLASSUME(m_file.m_h == NULL);

		if (pszDir == NULL)
		{
			DWORD dwRet = GetTempPath(_MAX_DIR, szPath);
			if (dwRet == 0)
			{
				// Couldn't find temporary path;
				return AtlHresultFromLastError();
			}
			else if (dwRet > _MAX_DIR)
			{
				return DISP_E_BUFFERTOOSMALL;
			}
		}
		else
		{
			Checked::tcsncpy_s(szPath, _countof(szPath), pszDir, _TRUNCATE);
		}

		if (!GetTempFileName(szPath, _T("TFR"), 0, tmpFileName))
		{
			// Couldn't create temporary filename;
			return AtlHresultFromLastError();
		}
		tmpFileName[_countof(tmpFileName)-1]='\0';

		Checked::tcsncpy_s(m_szTempFileName, _countof(m_szTempFileName), tmpFileName, _TRUNCATE);
		SECURITY_ATTRIBUTES secatt;
		secatt.nLength = sizeof(secatt);
		secatt.lpSecurityDescriptor = NULL;
		secatt.bInheritHandle = TRUE;

		m_dwAccess = dwDesiredAccess;

		return m_file.Create(
			m_szTempFileName,
			m_dwAccess,
			0,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_TEMPORARY,
			&secatt);
	}

	HRESULT Close(__in_opt LPCTSTR szNewName = NULL) throw()
	{
		ATLASSUME(m_file.m_h != NULL);

		// This routine is called when we are finished writing to the 
		// temporary file, so we now just want to close it and copy
		// it to the actual filename we want it to be called.

		// So let's close it first.
		m_file.Close();

		// no new name so delete it
		if (szNewName == NULL)
		{
			::DeleteFile(m_szTempFileName);
			return S_OK;
		}

		// delete any existing file and move our temp file into it's place
		if (!::DeleteFile(szNewName))
		{
			DWORD dwError = GetLastError();
			if (dwError != ERROR_FILE_NOT_FOUND)
				return AtlHresultFromWin32(dwError);
		}

		if (!::MoveFile(m_szTempFileName, szNewName))
			return AtlHresultFromLastError();

		return S_OK;
	}

	HRESULT HandsOff() throw()
	{
		m_file.Flush();
		m_file.Close();

		return S_OK;
	}

	HRESULT HandsOn() throw()
	{
		HRESULT hr = m_file.Create(
			m_szTempFileName,
			m_dwAccess,
			0,
			OPEN_EXISTING);
		if (FAILED(hr))
			return hr;

		return m_file.Seek(0, FILE_END);
	}

	HRESULT Read(
		__out_bcount(nBufSize) LPVOID pBuffer,
		__in DWORD nBufSize,
		__out DWORD& nBytesRead) throw()
	{
		return m_file.Read(pBuffer, nBufSize, nBytesRead);
	}

	HRESULT Write(
		__in_bcount(nBufSize) LPCVOID pBuffer,
		__in DWORD nBufSize,
		__out_opt DWORD* pnBytesWritten = NULL) throw()
	{
		return m_file.Write(pBuffer, nBufSize, pnBytesWritten);
	}

	HRESULT Seek(__in LONGLONG nOffset, __in DWORD dwFrom = FILE_CURRENT) throw()
	{
		return m_file.Seek(nOffset, dwFrom);
	}

	HRESULT GetPosition(__out ULONGLONG& nPos) const throw()
	{
		return m_file.GetPosition(nPos);
	}

	HRESULT Flush() throw()
	{
		return m_file.Flush();
	}

	HRESULT LockRange(__in ULONGLONG nPos, __in ULONGLONG nCount) throw()
	{
		return m_file.LockRange(nPos, nCount);
	}

	HRESULT UnlockRange(__in ULONGLONG nPos, __in ULONGLONG nCount) throw()
	{
		return m_file.UnlockRange(nPos, nCount);
	}

	HRESULT SetSize(__in ULONGLONG nNewLen) throw()
	{
		return m_file.SetSize(nNewLen);
	}

	HRESULT GetSize(__out ULONGLONG& nLen) const throw()
	{
		return m_file.GetSize(nLen);
	}

	operator HANDLE() throw()
	{
		return m_file;
	}

	LPCTSTR TempFileName() throw()
	{
		return m_szTempFileName;
	}

private:
	CAtlFile m_file;
	TCHAR m_szTempFileName[_MAX_FNAME+1];
	DWORD m_dwAccess;
};


interface IBkMiniDownloaderCallback
{
    virtual BOOL OnProgress(DWORD dwTotalSize, DWORD dwDownloadedSize) = 0;
};

class CBkMiniDownloader
{
public:

    class _UrlCracker
    {
    public:
        _UrlCracker()
            : m_bIsValidUrl(FALSE)
        {

        }
        ~_UrlCracker()
        {

        }

        BOOL SetUrl(LPCTSTR lpszUrl)
        {
            URL_COMPONENTS urlcomp = {sizeof(URL_COMPONENTS)};
            BOOL bRet = FALSE;

            urlcomp.lpszHostName = m_strHostName.GetBuffer(MAX_PATH + 1);
            urlcomp.dwHostNameLength = MAX_PATH;
            urlcomp.lpszUrlPath = m_strUrlPath.GetBuffer(MAX_PATH + 1);
            urlcomp.dwUrlPathLength = MAX_PATH;

            TCHAR szSysDir[MAX_PATH] = { 0 };
            ::GetSystemDirectory(szSysDir, MAX_PATH);
            _tcscat_s(szSysDir, _T("\\winhttp.dll"));

            HMODULE _hWinHttp = ::LoadLibrary(szSysDir);
            if (NULL != _hWinHttp)
            {
                tWinHttpCrackUrl _WinHttpCrackUrl = (tWinHttpCrackUrl)::GetProcAddress(_hWinHttp, "WinHttpCrackUrl");
                if (NULL != _WinHttpCrackUrl)
                {
                    bRet = _WinHttpCrackUrl(lpszUrl, 0, ICU_ESCAPE, &urlcomp);
                }
                else
                {
                    bRet = ::InternetCrackUrl(lpszUrl, 0, ICU_ESCAPE, &urlcomp);
                }

                FreeLibrary(_hWinHttp);
                _hWinHttp = NULL;
            }
            else
            {
                bRet = ::InternetCrackUrl(lpszUrl, 0, ICU_ESCAPE, &urlcomp);
            }

            m_strHostName.ReleaseBuffer();
            m_strUrlPath.ReleaseBuffer();

            m_usPort = urlcomp.nPort;

            m_bIsValidUrl = bRet;

            m_strFullUrl = lpszUrl;

            return m_bIsValidUrl;
        }

        LPCTSTR GetHostName(void)
        {
            if (!m_bIsValidUrl)
                return NULL;

            return m_strHostName;
        }

        LPCTSTR GetUrlPath(void)
        {
            if (!m_bIsValidUrl)
                return NULL;

            return m_strUrlPath;
        }

        INTERNET_PORT GetPort(void)
        {
            if (!m_bIsValidUrl)
                return NULL;

            return m_usPort;
        }

        LPCTSTR GetFullUrl(void)
        {
            if (!m_bIsValidUrl)
                return NULL;

            return m_strFullUrl;
        }

    private:
        BOOL m_bIsValidUrl;
        CString m_strFullUrl;
        CString m_strHostName;
        CString m_strUrlPath;
        INTERNET_PORT m_usPort;
    };

    CBkMiniDownloader()
    {
        m_hWinHttp = NULL;
        _WinHttpOpen = NULL;
        _WinHttpConnect = NULL;
        _WinHttpOpenRequest = NULL;
        _WinHttpAddRequestHeaders = NULL;
        _WinHttpSendRequest = NULL;
        _WinHttpReceiveResponse = NULL;
        _WinHttpQueryHeaders = NULL;
        _WinHttpReadData = NULL;
        _WinHttpCloseHandle = NULL;
        _WinHttpSetOption = NULL;

    }

    ~CBkMiniDownloader()
    {
        if (m_hWinHttp != NULL)
        {
            FreeLibrary(m_hWinHttp);
            m_hWinHttp = FALSE;
        }
    }

    BOOL LoadWinHttp()
    {
        TCHAR szSysDir[MAX_PATH] = { 0 };
        ::GetSystemDirectory(szSysDir, MAX_PATH);
        _tcscat_s(szSysDir, _T("\\winhttp.dll"));

        m_hWinHttp = ::LoadLibrary(szSysDir);
        BOOL bInited = FALSE;
        if (m_hWinHttp)
        {
            _WinHttpOpen				= (tWinHttpOpen)::GetProcAddress(m_hWinHttp,				"WinHttpOpen");
            _WinHttpConnect				= (tWinHttpConnect)::GetProcAddress(m_hWinHttp,				"WinHttpConnect");
            _WinHttpOpenRequest			= (tWinHttpOpenRequest)::GetProcAddress(m_hWinHttp,			"WinHttpOpenRequest");
            _WinHttpAddRequestHeaders	= (tWinHttpAddRequestHeaders)::GetProcAddress(m_hWinHttp,	"WinHttpAddRequestHeaders");
            _WinHttpSendRequest			= (tWinHttpSendRequest)::GetProcAddress(m_hWinHttp,			"WinHttpSendRequest");
            _WinHttpReceiveResponse		= (tWinHttpReceiveResponse)::GetProcAddress(m_hWinHttp,		"WinHttpReceiveResponse");
            _WinHttpQueryHeaders		= (tWinHttpQueryHeaders)::GetProcAddress(m_hWinHttp,		"WinHttpQueryHeaders");
            _WinHttpReadData			= (tWinHttpReadData)::GetProcAddress(m_hWinHttp,			"WinHttpReadData");
            _WinHttpCloseHandle			= (tWinHttpCloseHandle)::GetProcAddress(m_hWinHttp,			"WinHttpCloseHandle");
            _WinHttpSetOption			= (tWinHttpSetOption)::GetProcAddress(m_hWinHttp,			"WinHttpSetOption");

            //
            bInited = (_WinHttpOpen 
                && _WinHttpAddRequestHeaders
                && _WinHttpCloseHandle
                && _WinHttpConnect
                && _WinHttpOpenRequest
                && _WinHttpQueryHeaders
                && _WinHttpReadData
                && _WinHttpReceiveResponse
                && _WinHttpSendRequest
                && _WinHttpSetOption);
        }

        return bInited;
    }

    HRESULT Download(LPCTSTR lpszUrl, CBkTemporaryFile& tmpFile, IBkMiniDownloaderCallback *piCallback = NULL)
    {
        HRESULT hResult = E_FAIL;
        BOOL bRet = FALSE;
        HINTERNET hNet = NULL, hConnect = NULL, hRequest = NULL;
        DWORD dwConnectTimeout = 0, dwSize = 0;
        DWORD dwHttpRetCode = 0, dwTotalSize = 0, dwDownloadSize = 0;
        BYTE pbyBuffer[VKSDL_NETWORK_BUFFER_SIZE];
        _UrlCracker url;

        bRet = url.SetUrl(lpszUrl);
        if (!bRet)
            goto Exit0;

        BOOL bWinHttpInited = LoadWinHttp();

        if (FALSE == bWinHttpInited)
        {
            hNet = ::InternetOpen(VKSDL_HTTP_AGENT_NAME, INTERNET_OPEN_TYPE_PRECONFIG, _T(""), _T(""), 0);
            if (NULL == hNet)
                goto Exit0;
        }
        else
        {
            hNet = _WinHttpOpen(VKSDL_HTTP_AGENT_NAME, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, _T(""), _T(""), 0);
            if (NULL == hNet)
                goto Exit0;
        }

        if (FALSE == bWinHttpInited)
        {
            dwConnectTimeout = VKSDL_TCP_CONNECT_TIMEOUT;
            bRet = ::InternetSetOption(hNet, INTERNET_OPTION_CONNECT_TIMEOUT, &dwConnectTimeout, sizeof(DWORD));
            if (!bRet)
                goto Exit0;
        }
        else
        {
            dwConnectTimeout = VKSDL_TCP_CONNECT_TIMEOUT;
            bRet = _WinHttpSetOption(hNet, WINHTTP_OPTION_CONNECT_TIMEOUT, &dwConnectTimeout, sizeof(DWORD));
            if (!bRet)
                goto Exit0;
        }

        if (FALSE == bWinHttpInited)
        {
            dwConnectTimeout = VKSDL_TCP_RECEIVE_TIMEOUT;
            bRet = ::InternetSetOption(hNet, INTERNET_OPTION_RECEIVE_TIMEOUT, &dwConnectTimeout, sizeof(DWORD));
            if (!bRet)
                goto Exit0;
        }
        else
        {
            dwConnectTimeout = VKSDL_TCP_RECEIVE_TIMEOUT;
            bRet = _WinHttpSetOption(hNet, WINHTTP_OPTION_RECEIVE_TIMEOUT, &dwConnectTimeout, sizeof(DWORD));
            if (!bRet)
                goto Exit0;
        }


        if (FALSE == bWinHttpInited)
        {
            hConnect = ::InternetConnect(
                hNet, url.GetHostName(), url.GetPort(), 
                L"", L"", INTERNET_SERVICE_HTTP, 0, 0
                );
            if (NULL == hConnect)
                goto Exit0;
        }
        else
        {
            hConnect = _WinHttpConnect(
                hNet, url.GetHostName(), url.GetPort(), 0);
            if (NULL == hConnect)
                goto Exit0;
        }


        if (FALSE == bWinHttpInited)
        {
            hRequest = ::HttpOpenRequest(
                hConnect, NULL, url.GetUrlPath(), NULL, NULL, NULL, 
                INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0
                );
            if (NULL == hRequest)
                goto Exit0;
        }
        else
        {
            hRequest = _WinHttpOpenRequest(
                hConnect, _T("GET"), url.GetUrlPath(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 
                WINHTTP_FLAG_REFRESH
                );
            if (NULL == hRequest)
                goto Exit0;
        }


        if (FALSE == bWinHttpInited)
        {
            ::HttpAddRequestHeaders(
                hRequest, 
                VKSDL_REQUEST_HEADER_CONNECTION_CLOSE, 
                (DWORD)_tcslen(VKSDL_REQUEST_HEADER_CONNECTION_CLOSE), 
                HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE
                );
        }
        else
        {
            bRet = _WinHttpAddRequestHeaders(
                hRequest, 
                VKSDL_REQUEST_HEADER_CONNECTION_CLOSE, 
                (DWORD)_tcslen(VKSDL_REQUEST_HEADER_CONNECTION_CLOSE), 
                WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE
                );
        }

        if (FALSE == bWinHttpInited)
        {
            bRet = ::HttpSendRequest(hRequest, NULL, 0, NULL, 0);
            if (!bRet)
                goto Exit0;
        }
        else
        {
            bRet = _WinHttpSendRequest(hRequest, NULL, 0, NULL, 0, 0, NULL);
            if (!bRet)
                goto Exit0;
        }


        if (FALSE == bWinHttpInited)
        {
            dwSize = sizeof(DWORD);
            bRet = ::HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwHttpRetCode, &dwSize, NULL);
            if (!bRet)
                goto Exit0;
        }
        else
        {
            _WinHttpReceiveResponse(hRequest, NULL);

            dwSize = sizeof(DWORD);
            bRet = _WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE  | WINHTTP_QUERY_FLAG_NUMBER, 
                0, &dwHttpRetCode, &dwSize, NULL);
            if (!bRet)
                goto Exit0;
        }

        if (HTTP_STATUS_OK != dwHttpRetCode)
        {
            hResult = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_HTTP, dwHttpRetCode);
            goto Exit0;
        }


        if (FALSE == bWinHttpInited)
        {
            dwSize = sizeof(DWORD);
            bRet = ::HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &dwTotalSize, &dwSize, NULL);
            if (!bRet)
                goto Exit0;
        }
        else
        {
            dwSize = sizeof(DWORD);
            bRet = _WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, 
                0, &dwTotalSize, &dwSize, NULL);
            if (!bRet)
                goto Exit0;
        }

        tmpFile.SetSize(dwTotalSize);
        tmpFile.Seek(0, FILE_BEGIN);

        if (piCallback)
        {
            bRet = piCallback->OnProgress(dwTotalSize, dwDownloadSize);
            if (!bRet)
            {
                hResult = E_ABORT;
                goto Exit0;
            }
        }

        do 
        {
            if (FALSE == bWinHttpInited)
            {
                bRet = ::InternetReadFile(hRequest, pbyBuffer, VKSDL_NETWORK_BUFFER_SIZE, &dwSize);
                if (!bRet)
                    goto Exit0;
            }
            else
            {
                bRet = _WinHttpReadData(hRequest, pbyBuffer, VKSDL_NETWORK_BUFFER_SIZE, &dwSize);
                if (!bRet)
                    goto Exit0;
            }
            

            if (0 == dwSize)
                break;

            if (dwDownloadSize + dwSize > dwTotalSize)
                dwSize = dwTotalSize - dwDownloadSize;

            tmpFile.Write(pbyBuffer, dwSize);

            dwDownloadSize += dwSize;

            if (piCallback)
            {
                bRet = piCallback->OnProgress(dwTotalSize, dwDownloadSize);
                if (!bRet)
                {
                    hResult = E_ABORT;
                    goto Exit0;
                }
            }

        } while (dwDownloadSize < dwTotalSize);

        hResult = S_OK;

Exit0:

        if (E_FAIL == hResult)
            hResult = AtlHresultFromLastError();

        if (FALSE == bWinHttpInited)
        {
            if (hRequest)
            {
                ::InternetCloseHandle(hRequest);
                hRequest = NULL;
            }

            if (hConnect)
            {
                ::InternetCloseHandle(hConnect);
                hConnect = NULL;
            }

            if (hNet)
            {
                ::InternetCloseHandle(hNet);
                hNet = NULL;
            }
        }
        else
        {
            if (hRequest)
            {
                _WinHttpCloseHandle(hRequest);
                hRequest = NULL;
            }

            if (hConnect)
            {
                _WinHttpCloseHandle(hConnect);
                hConnect = NULL;
            }

            if (hNet)
            {
                _WinHttpCloseHandle(hNet);
                hNet = NULL;
            }
        }


        return hResult;
    }

private:
    HMODULE  m_hWinHttp;

    tWinHttpOpen                _WinHttpOpen;
    tWinHttpConnect             _WinHttpConnect;
    tWinHttpOpenRequest         _WinHttpOpenRequest;
    tWinHttpAddRequestHeaders   _WinHttpAddRequestHeaders;
    tWinHttpSendRequest         _WinHttpSendRequest;
    tWinHttpReceiveResponse     _WinHttpReceiveResponse;
    tWinHttpQueryHeaders        _WinHttpQueryHeaders;
    tWinHttpReadData            _WinHttpReadData;
    tWinHttpCloseHandle         _WinHttpCloseHandle;
    tWinHttpSetOption           _WinHttpSetOption;

};