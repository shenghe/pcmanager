#include "stdafx.h"
#include "beikesafe.h"
#include "_idl_gen/bksafesvc.h"
#include <comproxy/bkutility.h>
#include <common/utility.h>
#include "beikesafeinstallkavenginedlg.h"

// #define KAV_ENGINE_INSTALLER_DOWNLOAD_URL   L"http://dl.ijinshan.com/kav2/kaveinstall.exe"
#define KAV_ENGINE_INSTALLER_DOWNLOAD_URL   L"http://dl.ijinshan.com/kav2/kavinstall.pack"
// #define KAV_ENGINE_INSTALLER_DOWNLOAD_URL   L"http://192.168.3.31/kaveinstall2.exe"
#define KAV_ENGINE_INSTALLER_TEMP_FILE_NAME L"\\kaveinstall.exe"

CBeikeSafeInstallKavEngineDlg::~CBeikeSafeInstallKavEngineDlg()
{

}

UINT_PTR CBeikeSafeInstallKavEngineDlg::DoModal()
{
    if (KavEngine::RegisterInstalling(m_hMutex))
        return __super::DoModal();

    return -1;
}

BOOL CBeikeSafeInstallKavEngineDlg::OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
{
    m_hWndRef = m_hWnd;

	int nBig = 0;
	int nSmall = 0;

	if (KisPublic::Instance()->Init())
	{
		BOOL bRet = KisPublic::Instance()->KQueryOEMIntA(KIS::KOemKsfe::oemc_IconBig, nBig);
		bRet = KisPublic::Instance()->KQueryOEMIntA(KIS::KOemKsfe::oemc_IconSmall, nSmall);
	}

	if (nBig > 0 && nSmall > 0)
	{
		SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(nBig)));
		SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(nSmall)), FALSE);
	}
	else
	{
		SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_BEIKESAFE)));
		SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_SMALL)), FALSE);
	}

    ModifyStyle(0, WS_MINIMIZEBOX);

    _Module.SetActiveWindow(m_hWnd);

    m_wndListProgress.Create(
        GetViewHWND(), NULL, NULL, 
        WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER,     
        NULL, IDC_LST_INSTALL_PROGRESS_DETAIL);

    m_wndListProgress.InsertColumn(0, L"", LVCFMT_LEFT, 200);
    m_wndListProgress.InsertColumn(1, L"", LVCFMT_LEFT, 160);
    m_wndListProgress.SetItemHeight(20);
    m_wndListProgress.SetImageList(BkBmpPool::GetBitmap(IDB_EXAM_STATUS_ICON), 15);
    m_wndListProgress.AddItem(0, 0, BkString::Get(IDS_VIRSCAN_2572), 4);

    HANDLE hThread = ::CreateThread(NULL, 0, _InstallThreadProc, this, 0, NULL);
    ::CloseHandle(hThread);
    hThread = NULL;

    return TRUE;
}

void CBeikeSafeInstallKavEngineDlg::OnBkBtnCancel()
{
    if (IsItemEnable(IDC_BTN_INSAVE_CANCEL) && IsItemVisible(IDC_BTN_INSAVE_CANCEL, TRUE))
    {
        m_bCancel = TRUE;

        EnableItem(IDC_BTN_INSAVE_CANCEL, FALSE);
    }
    else
    {
        EndDialog(IDOK);
    }
}

void CBeikeSafeInstallKavEngineDlg::OnBkBtnFinish()
{
    EndDialog(IDOK);
}

void CBeikeSafeInstallKavEngineDlg::OnBkBtnMinimize()
{
    SendMessage(WM_SYSCOMMAND, SC_MINIMIZE | HTCAPTION, 0);
}

void CBeikeSafeInstallKavEngineDlg::BeginDownload()
{

}

void CBeikeSafeInstallKavEngineDlg::Retry(DWORD dwTimes)
{

}

void FormatSizeString(INT64 nFileSize, CString &str);

BOOL CBeikeSafeInstallKavEngineDlg::DownloadProgress(DWORD dwTotalSize, DWORD dwReadSize)
{
    _PostProgress(StepDownload, dwReadSize * 100 / dwTotalSize);

    CString strText;

    CString strTotalSize, strReadSize;

    FormatSizeString(dwTotalSize, strTotalSize);
    FormatSizeString(dwReadSize, strReadSize);

    strText.Format(BkString::Get(IDS_VIRSCAN_2573), strReadSize, strTotalSize);
    m_wndListProgress.SetItemText(0, 1, strText);

    return !m_bCancel;
}

void CBeikeSafeInstallKavEngineDlg::FinishDownload(HRESULT hError)
{
    if (SUCCEEDED(hError))
    {
        _PostProgress(StepDownload, 100);
        m_wndListProgress.SetItemText(0, 0, BkString::Get(IDS_VIRSCAN_2574));
        m_wndListProgress.SetItemText(0, 1, L"");
        m_wndListProgress.SetItem(0, 0, LVIF_IMAGE, NULL, 2, 0, 0, 0);
    }
}

DWORD WINAPI CBeikeSafeInstallKavEngineDlg::_InstallThreadProc(LPVOID pvParam)
{
    int nRet = 0;
    BOOL bSvcRunning = FALSE;
    BOOL bRet = FALSE;
    SC_HANDLE hSCM = NULL, hService = NULL;
    SERVICE_STATUS ss;

    if (NULL == pvParam)
        return -1;

    CBeikeSafeInstallKavEngineDlg *pThis = (CBeikeSafeInstallKavEngineDlg *)pvParam;
    CString strPath;

    CAppPath::Instance().GetLeidianTempPath(strPath, TRUE);

    ::CreateDirectory(strPath, NULL);

    strPath += KAV_ENGINE_INSTALLER_TEMP_FILE_NAME;

    bRet = pThis->_DownloadInstallPackage(strPath);

    if (pThis->m_bCancel)
        goto Exit0;

    if (!bRet)
    {
        nRet = 2;
        goto Exit0;
    }

    bRet = pThis->_InstallPackage(strPath);
    if (pThis->m_bCancel)
        goto Exit0;

    if (!bRet)
    {
        nRet = 3;
        goto Exit0;
    }

    pThis->_PostProgress(StepCheckUpdate, 0);

    bRet = pThis->_UpdateSignature();
    if (!bRet)
    {
        nRet = 4;
        goto Exit0;
    }

    hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL)
    {
        nRet = 5;
        goto Exit0;
    }

    hService = ::OpenService(hSCM, L"KSafeSvc", SERVICE_QUERY_STATUS);
    if (hService == NULL)
    {
        nRet = 5;
        goto Exit0;
    }

    bRet = ::QueryServiceStatus(hService, &ss);
    if (!bRet)
    {
        nRet = 5;
        goto Exit0;
    }

    if (SERVICE_RUNNING == ss.dwCurrentState)
    {
        ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

        CBkUtility utility;

        HRESULT hRet = utility.Initialize();
        if (SUCCEEDED(hRet))
        {
            hRet = utility.EnableAvEngine();
            utility.Uninitialize();
        }

        ::CoUninitialize();

        if (FAILED(hRet))
        {
            nRet = 5;
            goto Exit0;
        }
    }

Exit0:

    if (hService)
        ::CloseServiceHandle(hService);
    if (hSCM)
        ::CloseServiceHandle(hSCM);

    pThis->_PostProgress(StepFinish, pThis->m_bCancel ? 1 : nRet);

    return 0;
}

BOOL CBeikeSafeInstallKavEngineDlg::_DownloadInstallPackage(LPCTSTR lpszPath)
{
    CUpdUtilityProxy downloader;

    HRESULT hRet = downloader.Initialize();
    if (FAILED(hRet))
        goto Exit0;

    hRet = downloader.DownloadFile(KAV_ENGINE_INSTALLER_DOWNLOAD_URL, lpszPath, TRUE, NULL, this);
    if (FAILED(hRet))
        goto Exit0;

Exit0:

    downloader.Uninitialize();

    return SUCCEEDED(hRet);
}

BOOL CBeikeSafeInstallKavEngineDlg::_InstallPackage(LPCTSTR lpszPath)
{
    STARTUPINFO si = {sizeof(STARTUPINFO)};
    PROCESS_INFORMATION pi;
    int nProgress = 1;
    DWORD dwRet = 0;

    CString strCmdLine;
	
	strCmdLine += L"\"";
	strCmdLine += lpszPath;
    strCmdLine += L"\" /S /D2=\"";
    strCmdLine += _Module.GetAppDirPath();
    strCmdLine += L'\"';
//     strCmdLine.Truncate(strCmdLine.GetLength() - 1);

    BOOL bRet = ::CreateProcess(NULL, (LPTSTR)(LPCTSTR)strCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (!bRet)
        return FALSE;
    
    ::CloseHandle(pi.hThread);

    while (WAIT_OBJECT_0 != ::WaitForSingleObject(pi.hProcess, 500 * nProgress))
    {
        _PostProgress(StepInstall, 100 * (nProgress - 1) / nProgress);

        nProgress ++;
    }

    _PostProgress(StepInstall, 100);

    ::GetExitCodeProcess(pi.hProcess, &dwRet);

    ::CloseHandle(pi.hProcess);

    {
        CString strDebug;

        strDebug.Format(L" * Installer Return 0x%08X\r\n", dwRet);
        ::OutputDebugString(strDebug);
    }

    return (0 == dwRet);
}

HRESULT STDMETHODCALLTYPE CBeikeSafeInstallKavEngineDlg::OnPrepareFile(
    LPCWSTR     lpszFileName,
    DWORD       dwFileSize)
{
    if (0 == m_nUpdateTotalFileCount)
    {
        _PostProgress(StepUpdate, 0);
        m_nUpdatingFileItem = m_wndListProgress.GetItemCount() - 1;
    }

    m_nUpdateTotalFileCount ++;

    CString strFileName = L"  ";
    
    strFileName += lpszFileName;

    int nItem = m_wndListProgress.AddItem(m_wndListProgress.GetItemCount(), 0, strFileName, 8);
    m_wndListProgress.SetItemText(nItem, 1, BkString::Get(IDS_VIRSCAN_2575));

    return m_bCancel ? E_ABORT : S_OK;
}

HRESULT STDMETHODCALLTYPE CBeikeSafeInstallKavEngineDlg::OnDownloadFile(
    LPCWSTR     lpszFileName,
    DWORD       dwTotalSize,
    DWORD       dwDownloadedSize)
{
    if (0 != m_nUpdateTotalFileCount)
    {
        if (m_strUpdatingFile != lpszFileName)
        {
            m_nUpdatingFileItem ++;

            m_wndListProgress.SetItem(m_nUpdatingFileItem, 0, LVIF_IMAGE, NULL, 4, 0, 0, 0);
            m_wndListProgress.EnsureVisible(m_nUpdatingFileItem, FALSE);

            m_strUpdatingFile = lpszFileName;
        }

        CString strText;
        CString strTotalSize, strReadSize;

        FormatSizeString(dwTotalSize, strTotalSize);
        FormatSizeString(dwDownloadedSize, strReadSize);

        strText.Format(BkString::Get(IDS_VIRSCAN_2573), strReadSize, strTotalSize);
        m_wndListProgress.SetItemText(m_nUpdatingFileItem, 1, strText);

        _PostProgress(StepUpdate, (m_nUpdatedFileCount * dwTotalSize + dwDownloadedSize) * 100 / m_nUpdateTotalFileCount / dwTotalSize);
    }

    return m_bCancel ? E_ABORT : S_OK;
}

HRESULT STDMETHODCALLTYPE CBeikeSafeInstallKavEngineDlg::FinishDownloadFile(
    LPCWSTR     lpszFileName,
    DWORD       dwFileSize,
    HRESULT     hrDownload)
{
    if (0 != m_nUpdateTotalFileCount)
    {
        if (SUCCEEDED(hrDownload))
        {
            m_wndListProgress.SetItemText(m_nUpdatingFileItem, 1, BkString::Get(IDS_VIRSCAN_2576));
            m_wndListProgress.SetItem(m_nUpdatingFileItem, 0, LVIF_IMAGE, NULL, 2, 0, 0, 0);

            m_nUpdatedFileCount ++;
        }
    }

    return m_bCancel ? E_ABORT : S_OK;
}

HRESULT STDMETHODCALLTYPE CBeikeSafeInstallKavEngineDlg::OnReplaceFile(
    LPCWSTR     lpszFileName,
    LPCWSTR     lpszDstPath,
    HRESULT     hrReplace)
{
    return m_bCancel ? E_ABORT : S_OK;
}

BOOL CBeikeSafeInstallKavEngineDlg::_UpdateSignature()
{
    BOOL bRet = FALSE;
    Skylark::IBKEngKsgUpdate *piUpdate = NULL;
    Skylark::CWinModule_bkksup upmod;

    CString strDllFileName = _Module.GetAppDirPath();

    CAppPath::Instance().GetLeidianAvsPath(strDllFileName);

    strDllFileName += L"\\ksignup.dll";

    HRESULT hRet = upmod.LoadLib(strDllFileName);
    if (FAILED(hRet))
        goto Exit0;

    hRet = upmod.BKKsupCreateObject(__uuidof(Skylark::IBKEngKsgUpdate), (LPVOID *)&piUpdate);
    if (FAILED(hRet))
        goto Exit0;

    if (NULL == piUpdate)
        goto Exit0;

    Skylark::BKKSUP_INIT init;
    Skylark::BKENG_INIT(&init);

    hRet = piUpdate->Initialize(&init);
    if (FAILED(hRet))
        goto Exit0;

    Skylark::BKKSUP_UPDATE_PARAM param;
    Skylark::BKKSUP_UPDATE_INSTRUCTION instruction;

    Skylark::BKENG_INIT(&param);
    Skylark::BKENG_INIT(&instruction);

    param.uRetryTime = 3;
    param.piCallback = this;

    hRet = piUpdate->Update(&param, &instruction);
    if (FAILED(hRet))
        goto Exit0;

    if (FALSE == instruction.bNeedUpdate || 0 == m_nUpdateTotalFileCount)
    {
        _PostProgress(StepUpdate, -1);
    }
    else
    {
        _PostProgress(StepUpdate, 100);
    }

    bRet = TRUE;

Exit0:

    if (piUpdate)
    {
        piUpdate->Uninitialize();
        piUpdate->Release();
        piUpdate = NULL;
    }

    upmod.FreeLib();

    return bRet;
}

void CBeikeSafeInstallKavEngineDlg::_PostProgress(int nStep, int nProgress)
{
    SendMessage(MSG_APP_INSTALL_PROGRESS, nStep, nProgress);
}

LRESULT CBeikeSafeInstallKavEngineDlg::OnAppInstallProgress(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int nProgress = 0;

    BOOL bStepChanged = (wParam != m_nCurrentStep);

    switch (wParam)
    {
    case StepDownload:
        nProgress = lParam * 4 / 10;
    	break;
    case StepInstall:
        nProgress = 40 + lParam / 20;
        if (bStepChanged)
        {
            SetItemText(IDC_LBL_PROGRESS, BkString::Get(IDS_VIRSCAN_2577));
        }
        break;
    case StepCheckUpdate:
        nProgress = 45;
        if (bStepChanged)
        {
            m_wndListProgress.AddItem(m_wndListProgress.GetItemCount(), 0, BkString::Get(IDS_VIRSCAN_2578), 2);
            m_wndListProgress.AddItem(m_wndListProgress.GetItemCount(), 0, BkString::Get(IDS_VIRSCAN_2579), 4);
            SetItemText(IDC_LBL_PROGRESS, BkString::Get(IDS_VIRSCAN_2580));
        }
        break;
    case StepUpdate:
        nProgress = 50 + lParam / 2;
        if (bStepChanged)
        {
            m_wndListProgress.DeleteItem(m_wndListProgress.GetItemCount() - 1);
            m_wndListProgress.AddItem(m_wndListProgress.GetItemCount(), 0, BkString::Get(IDS_VIRSCAN_2581), 4);
            SetItemText(IDC_LBL_PROGRESS, BkString::Get(IDS_VIRSCAN_2582));
        }
        if (-1 == lParam)
        {
            m_wndListProgress.DeleteItem(m_wndListProgress.GetItemCount() - 1);
            m_wndListProgress.AddItem(m_wndListProgress.GetItemCount(), 0, BkString::Get(IDS_VIRSCAN_2583), 2);
        }
        else if (100 == lParam)
        {
            m_wndListProgress.DeleteItem(2);
            m_wndListProgress.AddItem(2, 0, BkString::Get(IDS_VIRSCAN_2584), 2);
        }
        break;
    case StepFinish:
        if (bStepChanged)
        {
            int nItem = 0;
            BOOL bFailed = FALSE;

            switch (lParam)
            {
            case 1:
                if (0 == m_nUpdatingFileItem)
                {
                    if (StepDownload == m_nCurrentStep)
                    {
                        m_wndListProgress.SetItemText(0, 0, BkString::Get(IDS_VIRSCAN_2585));
                        m_wndListProgress.SetItemText(0, 1, BkString::Get(IDS_VIRSCAN_2586));
                        m_wndListProgress.SetItem(0, 0, LVIF_IMAGE, NULL, 0, 0, 0, 0);
                    }
                }
                else
                {
                    while (m_nUpdatingFileItem < m_wndListProgress.GetItemCount())
                    {
                        m_wndListProgress.SetItemText(m_nUpdatingFileItem, 1, BkString::Get(IDS_VIRSCAN_2586));
                        m_wndListProgress.SetItem(m_nUpdatingFileItem, 0, LVIF_IMAGE, NULL, 0, 0, 0, 0);
                        m_nUpdatingFileItem ++;
                    }
                }
                nItem = m_wndListProgress.AddItem(m_wndListProgress.GetItemCount(), 0, BkString::Get(IDS_VIRSCAN_2587), 0);
                SetItemText(IDC_LBL_PROGRESS, BkString::Get(IDS_VIRSCAN_2587));
                break;

            case 2: // 下载失败
                m_wndListProgress.SetItemText(0, 0, BkString::Get(IDS_VIRSCAN_2588));
                m_wndListProgress.SetItemText(0, 1, L"");
                m_wndListProgress.SetItem(0, 0, LVIF_IMAGE, NULL, 9, 0, 0, 0);
                bFailed = TRUE;
                break;
            case 3: // 安装失败
                m_wndListProgress.SetItemText(1, 0, BkString::Get(IDS_VIRSCAN_2589));
                m_wndListProgress.SetItemText(1, 1, L"");
                m_wndListProgress.SetItem(0, 0, LVIF_IMAGE, NULL, 9, 0, 0, 0);
                bFailed = TRUE;
                break;
            case 4: // 升级失败
                m_wndListProgress.SetItemText(2, 0, BkString::Get(IDS_VIRSCAN_2590));
                m_wndListProgress.SetItemText(2, 1, L"");
                m_wndListProgress.SetItem(2, 0, LVIF_IMAGE, NULL, 9, 0, 0, 0);

                if (0 != m_nUpdatingFileItem)
                {
                    for (; m_nUpdatingFileItem < m_wndListProgress.GetItemCount(); m_nUpdatingFileItem ++)
                    {
                        m_wndListProgress.SetItemText(m_nUpdatingFileItem, 1, BkString::Get(IDS_VIRSCAN_2591));
                        m_wndListProgress.SetItem(m_nUpdatingFileItem, 0, LVIF_IMAGE, NULL, 9, 0, 0, 0);
                    }
                }
            case 0: // 成功
                KavEngine::Enable(TRUE);

                nItem = m_wndListProgress.AddItem(m_wndListProgress.GetItemCount(), 0, BkString::Get(IDS_VIRSCAN_2592), 2);
                SetItemText(IDC_LBL_PROGRESS, BkString::Get(IDS_VIRSCAN_2592));
                break;
            }

            if (bFailed)
            {
                nItem = m_wndListProgress.AddItem(m_wndListProgress.GetItemCount(), 0, BkString::Get(IDS_VIRSCAN_2593), 9);
                SetItemText(IDC_LBL_PROGRESS, BkString::Get(IDS_VIRSCAN_2593));
            }

            nProgress = 100;

            {
                KavEngine::UnregisterInstalling(m_hMutex);
                BeikeSafe::Navigate(BKSFNS_MAKE(BKSFNS_UI, BKSFNS_AVE_INSTALL_FINISH));
            }

            m_wndListProgress.EnsureVisible(nItem, TRUE);

            SetItemVisible(IDC_DIV_INSAVE_WORKING, FALSE);
            SetItemVisible(IDC_DIV_INSAVE_FINISH, TRUE);

            if (NULL == ::GetActiveWindow())
                FlashWindow(TRUE);
        }
        break;
    }

    m_nCurrentStep = wParam;

    if (m_nProgress < nProgress)
    {
        m_nProgress = nProgress;
        SetItemIntAttribute(IDC_PROGRESS_INSTALL, "value", nProgress);
    }

    return 0;
}

void CBeikeSafeInstallKavEngineDlg::OnSysCommand(UINT nID, CPoint point)
{
    SetMsgHandled(FALSE);

    switch (nID & 0xFFF0)
    {
    case SC_CLOSE:
        SetMsgHandled(TRUE);
        EndDialog(IDCANCEL);
        break;
    }
}
