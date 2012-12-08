#pragma once

#include <bksafesvc/comproxy/bkutility.h>
#include <skylark2/bkengdef.h>

class CBkSafeEchoCall
{
public:
    CBkSafeEchoCall()
        : m_hEventStop(NULL)
        , m_hThread(NULL)
    {
    }

    ~CBkSafeEchoCall()
    {
    }

    void Echo(HWND hWndNotify, UINT uMsgNotify)
    {
        if (NULL == m_hEventStop)
        {
            m_hEventStop = ::CreateEvent(NULL, TRUE, FALSE, NULL);

            m_hThread = ::CreateThread(NULL, 0, _EchoThread, new _EchoThreadParam(hWndNotify, uMsgNotify, m_hEventStop), 0, NULL);
        }
//         ::CloseHandle(m_hThread);
    }

    void Stop(BOOL bWait)
    {
        if (m_hEventStop)
        {
            SetEvent(m_hEventStop);

            if (bWait)
                ::WaitForSingleObject(m_hThread, 20000);

            ::CloseHandle(m_hThread);
            m_hThread = NULL;
        }
    }

protected:

    HANDLE m_hEventStop;
    HANDLE m_hThread;

    class _EchoThreadParam
    {
    public:
        _EchoThreadParam(HWND hWnd, UINT uMsg, HANDLE &hStop)
            : hWndNotify(hWnd)
            , uMsgNotify(uMsg)
            , hEventStop(hStop)
        {
        }

        HWND hWndNotify;
        UINT uMsgNotify;
        HANDLE& hEventStop;
    };

    static DWORD WINAPI _EchoThread(LPVOID pvParam)
    {
        _EchoThreadParam *pParam = (_EchoThreadParam *)pvParam;

        if (NULL == pParam)
            return -1;

        ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

        CBkEcho echo;
        CAtlArray<CString> arrEchoFiles;
        BK_ECHO_RESULT result;

        result.dwEchoStatus = BKENG_ECHO_LOCAL_COMM_ERROR;
        result.hEchoCode = E_FAIL;

        HRESULT hRet = echo.Initialize();
        if (FAILED(hRet))
            goto Exit0;

        arrEchoFiles.SetCount(2);
        arrEchoFiles[0] = _Module.GetAppFilePath();
        arrEchoFiles[1] = _Module.GetAppDirPath();
        arrEchoFiles[1] += L"bksafesvc.exe";

        hRet = echo.Echo(arrEchoFiles);
        if (FAILED(hRet))
            goto Exit0;

        do
        {
            hRet = echo.GetEchoResult(result);
            if (FAILED(hRet))
                goto Exit0;

            DWORD dwRet = ::WaitForSingleObject(pParam->hEventStop, 200);
            if (WAIT_TIMEOUT != dwRet)
                goto Exit0;
        }
        while (S_OK != hRet);

    Exit0:

        if (FAILED(hRet))
            result.hEchoCode = hRet;

        echo.Uninitialize();

        if (pParam->hWndNotify && ::IsWindow(pParam->hWndNotify))
            ::PostMessage(pParam->hWndNotify, pParam->uMsgNotify, result.dwEchoStatus, result.hEchoCode);

        ::CloseHandle(pParam->hEventStop);

        pParam->hEventStop = NULL;

        delete pParam;

        ::CoUninitialize();

        return 0;
    }
};