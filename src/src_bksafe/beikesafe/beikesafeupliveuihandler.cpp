#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "beikesafeupliveuihandler.h"

struct _UPDATE_NOTIFY_PARAM
{
    HWND hWndNotify;
    UINT uMsgNotify;
};

void CBeikeSafeUpliveUIHandler::Init()
{
    _UPDATE_NOTIFY_PARAM *pParam = new _UPDATE_NOTIFY_PARAM;

    pParam->hWndNotify = m_pDlg->m_hWnd;
    pParam->uMsgNotify = MSG_APP_UPDATE_PROGRESS;

    HANDLE hThread = ::CreateThread(NULL, 0, _UpdateReceiveThread, pParam, 0, NULL);

    ::CloseHandle(hThread);

    _CheckVersion();
}

void CBeikeSafeUpliveUIHandler::_CheckVersion()
{
    _UPDATE_NOTIFY_PARAM *pParam = new _UPDATE_NOTIFY_PARAM;

    pParam->hWndNotify = m_pDlg->m_hWnd;
    pParam->uMsgNotify = MSG_APP_UPDATE_CHECK_FINISH;

    HANDLE hThread = ::CreateThread(NULL, 0, _CheckVersionThread, pParam, 0, NULL);

    ::CloseHandle(hThread);
}

void CBeikeSafeUpliveUIHandler::Update()
{
    m_pDlg->SetItemVisible(IDC_DIV_UPLIVE_CHECK_NEED_UPDATE, FALSE);
    m_pDlg->SetItemVisible(IDC_DIV_UPLIVE_UPDATING, TRUE);

    HANDLE hThread = ::CreateThread(NULL, 0, _UpdateCallThread, NULL, 0, NULL);
    ::CloseHandle(hThread);
}

DWORD WINAPI CBeikeSafeUpliveUIHandler::_CheckVersionThread(LPVOID pvParam)
{
    _UPDATE_NOTIFY_PARAM *pParam = (_UPDATE_NOTIFY_PARAM *)pvParam;
    
    if (NULL == pParam)
        return -1;

    CSafeMonitorTrayShell tray;
    CString strNewVersion;
    
    BOOL bNeedUpdate = tray.CheckNewVersion(strNewVersion);

    if (pParam->hWndNotify && ::IsWindow(pParam->hWndNotify))
        ::SendMessage(pParam->hWndNotify, pParam->uMsgNotify, bNeedUpdate, 0);

    delete pParam;

    return 0;
}

DWORD WINAPI CBeikeSafeUpliveUIHandler::_UpdateCallThread(LPVOID pvParam)
{
    CSafeMonitorTrayShell tray;
    tray.RunUpdater();

    return 0;
}

LRESULT CBeikeSafeUpliveUIHandler::OnAppUpdateCheckFinish(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
    BOOL bNeedUpdate = (BOOL)(wParam);

    m_pDlg->SetItemVisible(IDC_DIV_UPLIVE_CHECKING, FALSE);
    m_pDlg->SetItemVisible(IDC_DIV_UPLIVE_CHECK_NEED_UPDATE, bNeedUpdate);
    m_pDlg->SetItemVisible(IDC_DIV_UPLIVE_CHECK_ALREADY_NEW, !bNeedUpdate);

    return 0;
}

LRESULT CBeikeSafeUpliveUIHandler::OnAppUpdateProgress(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
//     BOOL bRet = TRUE;
    if (-1 == wParam)
    {
        m_pDlg->SetItemVisible(IDC_DIV_UPLIVE_UPDATING, FALSE);
        m_pDlg->SetItemVisible(IDC_DIV_UPLIVE_CHECK_ALREADY_NEW, TRUE);
    }
    else if (0x10 == wParam)
    {
        _CheckVersion();
    }
    else
    {
        if (-1 == lParam)
        {
            m_pDlg->SetItemVisible(IDC_DIV_UPLIVE_UPDATING, FALSE);
            m_pDlg->SetItemVisible(IDC_DIV_UPLIVE_CHECK_NEED_UPDATE, TRUE);
        }
        else
        {
            DEBUG_TRACE(L"Update %d%%\r\n", lParam);

            m_pDlg->SetItemIntAttribute(IDC_PROGRESS_UPLIVE_UPDATING, "value", (int)lParam);

//             bRet = FALSE;
        }
    }

    return 0;
}

DWORD WINAPI CBeikeSafeUpliveUIHandler::_UpdateReceiveThread(LPVOID pvParam)
{
    _UPDATE_NOTIFY_PARAM *pParam = (_UPDATE_NOTIFY_PARAM *)pvParam;

    if (NULL == pParam)
        return -1;

    CSafeMonitorTrayShell tray;
    CString strNewVersion;
    MSG msg;
    LRESULT lRet = 0;

    tray.SetUpdateObserver(::GetCurrentThreadId(), WM_APP);

    while (TRUE)
    {
        BOOL bRet = ::GetMessage(&msg, NULL, 0, 0);

        if (bRet == -1)
        {
            continue;
        }
        else if (!bRet)
        {
            break;
        }

        if (WM_APP == msg.message && NULL == msg.hwnd)
        {
            if (pParam->hWndNotify && ::IsWindow(pParam->hWndNotify))
            {
                lRet = ::SendMessage(pParam->hWndNotify, pParam->uMsgNotify, msg.wParam, msg.lParam);
//                 if (lRet)
//                     break;
            }
//             else
//                 break;
        }
    }

    delete pParam;

    return 0;
}
