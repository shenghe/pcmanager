#pragma once

#include <WinInet.h>
#include <atlenc.h>

#pragma comment(lib, "wininet.lib")

#define _SZ_DUMP_REPORTER_

#define SZ_DUMP_HOST _T("stat.ijinshan.com")
#define SZ_DUMP_PORT 80
#define SZ_DUMP_FILE _T("/cinfo/repdump.php")
#define SZ_DUMP_PRARM "p=%d&c=%d&base64=1&dump="

class CSZDumpReporter
{
public:
    static void ReportDump(LPCTSTR lpszFileName, DWORD dwProductID, DWORD dwChannelID)
    {
        BOOL bRet = FALSE;
        HINTERNET hNet = NULL, hConnect = NULL, hRequest = NULL;
        DWORD dwConnectTimeout = 0, dwFileSize = 0;
        CStringA strContent, strContentEncode;
        int nEncodeLength = 0;

        HANDLE hFile = ::CreateFile(
            lpszFileName, GENERIC_READ, FILE_SHARE_READ, 
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
            );
        if (INVALID_HANDLE_VALUE == hFile)
            goto Exit0;

        dwFileSize = ::GetFileSize(hFile, NULL);

        bRet = ::ReadFile(hFile, strContent.GetBuffer(dwFileSize + 1), dwFileSize, &dwFileSize, NULL);
        strContent.ReleaseBuffer(dwFileSize);
        if (!bRet)
            goto Exit0;

        nEncodeLength = ATL::Base64EncodeGetRequiredLength(strContent.GetLength());

        bRet = ATL::Base64Encode(
            (BYTE *)(LPCSTR)strContent, strContent.GetLength(), 
            strContentEncode.GetBuffer(nEncodeLength + 1), &nEncodeLength
            );
        strContentEncode.ReleaseBuffer(nEncodeLength);
        if (!bRet)
            goto Exit0;

        strContent.Format(SZ_DUMP_PRARM, dwProductID, dwChannelID);
        strContent += strContentEncode;

        hNet = ::InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, _T(""), _T(""), 0);
        if (NULL == hNet)
            goto Exit0;

        dwConnectTimeout = 5000;
        bRet = ::InternetSetOption(hNet, INTERNET_OPTION_CONNECT_TIMEOUT, &dwConnectTimeout, sizeof(DWORD));
        if (!bRet)
            goto Exit0;

        dwConnectTimeout = 5000;
        bRet = ::InternetSetOption(hNet, INTERNET_OPTION_RECEIVE_TIMEOUT, &dwConnectTimeout, sizeof(DWORD));
        if (!bRet)
            goto Exit0;

        hConnect = ::InternetConnect(
            hNet, SZ_DUMP_HOST, SZ_DUMP_PORT, 
            L"", L"", INTERNET_SERVICE_HTTP, 0, 0
            );
        if (NULL == hConnect)
            goto Exit0;

        hRequest = ::HttpOpenRequest(
            hConnect, _T("POST"), SZ_DUMP_FILE, NULL, NULL, NULL, 
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0
            );
        if (NULL == hRequest)
            goto Exit0;

        ::HttpAddRequestHeaders(hRequest, _T("Content-Type: application/x-www-form-urlencoded"), 48, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);

        bRet = ::HttpSendRequest(hRequest, NULL, 0, (LPVOID)(LPCSTR)strContent, strContent.GetLength());
        if (!bRet)
            goto Exit0;

    Exit0:

        if (INVALID_HANDLE_VALUE != hFile)
        {
            ::CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
        }

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

        return;
    }
};

