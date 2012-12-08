#pragma once

#include <wtlhelper/whwindow.h>
#include <uplive/updproxy.h>
#include <skylark2/bkksup_mod.h>

#define MSG_APP_INSTALL_PROGRESS        (WM_APP)

BOOL IsFileExist(LPCTSTR pszFile);

class CBeikeSafeInstallKavEngineDlg
    : public CBkDialogImpl<CBeikeSafeInstallKavEngineDlg>
    , public CWHRoundRectFrameHelper<CBeikeSafeInstallKavEngineDlg>
    , public IUpdateDownloadFileCallback
    , public Skylark::IBKEngKsgUpdateCallback
{
public:
    CBeikeSafeInstallKavEngineDlg(HWND& hWndRef)
        : CBkDialogImpl<CBeikeSafeInstallKavEngineDlg>(IDR_BK_INSTALL_KAV_ENGINE_DLG)
        , m_nUpdateTotalFileCount(0)
        , m_nUpdatedFileCount(0)
        , m_nProgress(0)
        , m_nCurrentStep(StepDownload)
        , m_nUpdatingFileItem(0)
        , m_bCancel(FALSE)
        , m_hMutex(NULL)
        , m_hWndRef(hWndRef)
    {
    }
    ~CBeikeSafeInstallKavEngineDlg();

    UINT_PTR DoModal();

protected:

    enum {
        StepDownload = 1, 
        StepInstall,
        StepCheckUpdate,
        StepUpdate,
        StepFinish
    };

    HWND& m_hWndRef;

    int m_nUpdateTotalFileCount;
    int m_nUpdatedFileCount;
    int m_nProgress;
    int m_nCurrentStep;
    int m_nUpdatingFileItem;
    BOOL m_bCancel;
    CString m_strUpdatingFile;

    CWHListViewCtrl m_wndListProgress;

    HANDLE m_hMutex;

    void BeginDownload();
    void Retry(DWORD dwTimes);
    BOOL DownloadProgress(DWORD dwTotalSize, DWORD dwReadSize);
    void FinishDownload(HRESULT hError);

    HRESULT STDMETHODCALLTYPE OnPrepareFile(
        LPCWSTR     lpszFileName,
        DWORD       dwFileSize);
    HRESULT STDMETHODCALLTYPE OnDownloadFile(
        LPCWSTR     lpszFileName,
        DWORD       dwTotalSize,
        DWORD       dwDownloadedSize);
    HRESULT STDMETHODCALLTYPE FinishDownloadFile(
        LPCWSTR     lpszFileName,
        DWORD       dwFileSize,
        HRESULT     hrDownload);
    HRESULT STDMETHODCALLTYPE OnReplaceFile(
        LPCWSTR     lpszFileName,
        LPCWSTR     lpszDstPath,
        HRESULT     hrReplace);

    void OnBkBtnCancel();
    void OnBkBtnFinish();
    void OnBkBtnMinimize();

    BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/);
    LRESULT OnAppInstallProgress(UINT uMsg, WPARAM wParam, LPARAM lParam);

    static DWORD WINAPI _InstallThreadProc(LPVOID pvParam);

    BOOL _DownloadInstallPackage(LPCTSTR lpszPath);
    BOOL _InstallPackage(LPCTSTR lpszPath);
    BOOL _UpdateSignature();
    void _PostProgress(int nStep, int nProgress);

    void OnSysCommand(UINT nID, CPoint point);

public:
    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_INSAVE_CANCEL, OnBkBtnCancel)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnCancel)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_INSAVE_FINISH, OnBkBtnFinish)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_INSAVE_MINIMIZE, OnBkBtnMinimize)
    BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBeikeSafeInstallKavEngineDlg)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CBkDialogImpl<CBeikeSafeInstallKavEngineDlg>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBeikeSafeInstallKavEngineDlg>)

        MSG_WM_SYSCOMMAND(OnSysCommand)
        MSG_WM_INITDIALOG(OnInitDialog)
        MESSAGE_HANDLER_EX(MSG_APP_INSTALL_PROGRESS, OnAppInstallProgress)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};

#define KAVENGINE_STATUS_KEY_NAME       L"SOFTWARE\\KSafe\\KEng"
#define KAVENGINE_STATUS_VALUE_NAME     L"Install"

#define KAVENGINE_INSTALLING_MUTEX      L"{DE29120A-6820-476d-BDDF-DD481796CD9A}"

namespace KavEngine
{
    static BOOL Installed()
    {//判断是否安装毒霸本地引擎 
        CRegKey reg;

        LRESULT lRet = ERROR_SUCCESS;
        DWORD dwValue = 0;

		lRet = reg.Open(HKEY_LOCAL_MACHINE, KOpKsafeReg::ReplaceRegStr(KAVENGINE_STATUS_KEY_NAME).c_str());
        if (ERROR_SUCCESS != lRet)
            return FALSE;

        reg.QueryDWORDValue(KAVENGINE_STATUS_VALUE_NAME, dwValue);

        reg.Close();

        if (0 == dwValue)
            return FALSE;

        CString strDllFileName;

        CAppPath::Instance().GetLeidianAvsPath(strDllFileName);

        strDllFileName += L"\\ksafeave.dll";

        return IsFileExist(strDllFileName);
    }

    static void Enable(BOOL bEnable)
    {//
        CRegKey reg;

        LRESULT lRet = ERROR_SUCCESS;
        lRet = reg.Create(HKEY_LOCAL_MACHINE, KOpKsafeReg::ReplaceRegStr(KAVENGINE_STATUS_KEY_NAME).c_str());
        lRet = reg.SetDWORDValue(KAVENGINE_STATUS_VALUE_NAME, bEnable);

        reg.Close();
    }

    static void Install(BOOL bWait)
    {//安装毒霸本地引擎的接口
        CBkCmdLine cmdline;

        cmdline.SetParam(CMDPARAM_NAME_INSTALL_LOCAL_ENGINE, 1);

        cmdline.Execute(_Module.GetAppFilePath(), FALSE, bWait);
    }

    static BOOL IsInstalling()
    {
        HANDLE hMutex = ::OpenMutex(SYNCHRONIZE, FALSE, KAVENGINE_INSTALLING_MUTEX);
        if (NULL == hMutex)
            return FALSE;

        ::CloseHandle(hMutex);

        return TRUE;
    }

    static BOOL RegisterInstalling(HANDLE &hMutex)
    {
        hMutex = ::CreateMutex(NULL, TRUE, KAVENGINE_INSTALLING_MUTEX);
        if (NULL == hMutex)
            return FALSE;

        if (ERROR_ALREADY_EXISTS == ::GetLastError())
        {
            ::CloseHandle(hMutex);
            hMutex = NULL;
            return FALSE;
        }

        return TRUE;
    }

    static void UnregisterInstalling(HANDLE &hMutex)
    {
        ::CloseHandle(hMutex);
        hMutex = NULL;
    }
};
