#pragma once

template <class T, DWORD t_dwElapse = 1000>
class CThreadNotify
{
public:
    CThreadNotify()
        : m_hEventStop(::CreateEvent(NULL, TRUE, FALSE, NULL))
        , m_hWndNotify(NULL)
        , m_uMsgNotify(WM_NULL)
    {
    }

    virtual ~CThreadNotify()
    {
        if (m_hEventStop)
        {
            ::CloseHandle(m_hEventStop);
            m_hEventStop = NULL;
        }
    }

    void Start(HWND hWnd, UINT uMsg)
    {
        m_hWndNotify = hWnd;
        m_uMsgNotify = uMsg;

        HANDLE hThread = ::CreateThread(NULL, 0, _WorkThreadProc, this, 0, NULL);
        ::CloseHandle(hThread);
    }

    void Stop()
    {
        ::SetEvent(m_hEventStop);
    }

    // Need Overide
    // Return NeedNotify
    void Work(BOOL &bNeedNotify, WPARAM& wParam, LPARAM& lParam)
    {
        ATLASSERT(FALSE);
    }

private:

    HANDLE m_hEventStop;
    HWND m_hWndNotify;
    UINT m_uMsgNotify;

protected:

    static DWORD WINAPI _WorkThreadProc(LPVOID pvParam)
    {
        BOOL bNeedNotify = FALSE;
        WPARAM wParam = 0;
        LPARAM lParam = 0;

        T *pThis = (reinterpret_cast<T *>(pvParam));

        while (TRUE)
        {
            pThis->Work(bNeedNotify, wParam, lParam);

            if (bNeedNotify && pThis->m_hWndNotify && ::IsWindow(pThis->m_hWndNotify))
            {
                ::SendMessage(pThis->m_hWndNotify, pThis->m_uMsgNotify, wParam, lParam);
            }

            DWORD dwRet = ::WaitForSingleObject(pThis->m_hEventStop, t_dwElapse);
            if (WAIT_TIMEOUT != dwRet)
                break;
        }

        return 0;
    }

};
