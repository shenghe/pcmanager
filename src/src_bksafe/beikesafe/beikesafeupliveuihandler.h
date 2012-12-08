#pragma once

class CBeikeSafeMainDlg;

class CBeikeSafeUpliveUIHandler
{
public:
    CBeikeSafeUpliveUIHandler(CBeikeSafeMainDlg *pDialog)
        : m_pDlg(pDialog)
    {

    }

    void Init();
    void Update();

protected:

    CBeikeSafeMainDlg *m_pDlg;

    void _CheckVersion();

    static DWORD WINAPI _CheckVersionThread(LPVOID pvParam);
    static DWORD WINAPI _UpdateCallThread(LPVOID pvParam);
    static DWORD WINAPI _UpdateReceiveThread(LPVOID pvParam);

    LRESULT OnAppUpdateCheckFinish(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
    LRESULT OnAppUpdateProgress(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);

public:

    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        BK_NOTIFY_ID_COMMAND(IDC_DIV_UPLIVE_UPDATE, Update)
    BK_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(CBeikeSafeUpliveUIHandler)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
        MESSAGE_HANDLER_EX(MSG_APP_UPDATE_CHECK_FINISH, OnAppUpdateCheckFinish)
        MESSAGE_HANDLER_EX(MSG_APP_UPDATE_PROGRESS, OnAppUpdateProgress)
    END_MSG_MAP()
};