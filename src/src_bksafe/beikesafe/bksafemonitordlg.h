#pragma once

#include <wtlhelper/whwindow.h>
#include "beikesafe.h"
#include "bkmsgdefine.h"
#include <common/utility.h>
#include "bksafeprotectionnotifywnd.h"
#include <safemon/safemonitorctrlquerier.h>

class CBeikeSafeMainDlg;

enum {
	MonitorSettingPageCommon = 0,
	MonitorSettingPageBlockAndTrust,
	MonitorSettingPageHistory,
};

class CBKSafeMonitorDlg
	: public CBkDialogImpl<CBKSafeMonitorDlg>
	, public CWHRoundRectFrameHelper<CBKSafeMonitorDlg>

{
public:
	CBKSafeMonitorDlg(CBeikeSafeMainDlg *pDialog)
		: CBkDialogImpl<CBKSafeMonitorDlg>(IDR_BK_SAFEMONITOR_DLG)
		, m_pNotifyWnd(NULL)
		, m_bWarningTrunOff(FALSE)
        , m_bShowBwspSetting(FALSE)
        , m_hIconRegedit(NULL)
        , m_wndProtectionNotify(TRUE)
	{
        for (int i = 0; i < 3; i ++)
            m_bPageNeverShow[i] = TRUE;

		if (NULL != pDialog)
			m_pNotifyWnd = pDialog;

        m_hIconRegedit = ::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_REGEDIT));
	}
	~CBKSafeMonitorDlg(void);

	BOOL OnInitDialog(HWND wParam, LPARAM lParam);
	void Init();

	UINT_PTR DoModal(int nPage, HWND hWndParent);

	void OnBkClose()
	{
		EndDialog(0);
	}

    BOOL NeedShowBwspSetting()
    {
        return m_bShowBwspSetting;
    }
public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_OFF_LEAK, OnLblProtectionTurnOffLeak)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_OFF_REGISTER, OnLblProtectionTurnOffRegister)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_OFF_PROCESS, OnLblProtectionTurnOffProcess)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_OFF_UDISK, OnLblProtectionTurnOffUDisk)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_OFF_BROWSERTPROTECT, OnLblProtectionTurnOffBrowserProtect)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_OFF_BWSP_PHISHING, OnLblProtectionTurnOffBwspPhishing)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ON_LEAK, OnLblProtectionTurnOnLeak)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ON_REGISTER, OnLblProtectionTurnOnRegister)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ON_PROCESS, OnLblProtectionTurnOnProcess)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ON_UDISK, OnLblProtectionTurnOnUDisk)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ON_BROWSERTPROTECT, OnLblProtectionTurnOnBrowserProtect)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ON_BWSP_PHISHING, OnLblProtectionTurnOnBwspPhishing)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_LOG_SHOW, OnLblProtectionLogShow)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_SCAN, OnLblProtectionScan)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TRUN_ON_ALL, OnLblProtectionTrunOnAll)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_SETTING, OnLblProtectionSetting)

		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkClose)
        BK_NOTIFY_TAB_SELCHANGE(IDC_TAB_MAIN, OnBkTabMainSelChange)

        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_CTRL_CHECK_ALL, OnLblProtectionCtrlCheckAll)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_CTRL_UNCHECK_ALL, OnLblProtectionCtrlUncheckAll)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_PROTECTION_CTRL_REMOVE_CHECK, OnLblProtectionCtrlRemoveCheck)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_PROTECTION_CTRL_LOG_CLEAR, OnLblProtectionCtrlLogClear)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_BWSP_MORE_SETTING1, OnLblProtectionBwspMoreSetting)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_BWSP_MORE_SETTING2, OnLblProtectionBwspMoreSetting)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBKSafeMonitorDlg)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CBkDialogImpl<CBKSafeMonitorDlg>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBKSafeMonitorDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        MESSAGE_HANDLER_EX(MSG_APP_PROTECTION_SWITCH_CHANGE, OnAppProtectionSwitchChange)
        MESSAGE_HANDLER_EX(MSG_APP_PROTECTION_CTRL_LOAD_FINISH, OnAppProtectionCtrlLoadFinish)
        MESSAGE_HANDLER_EX(MSG_APP_PROTECTION_LOG_LOAD_FINISH, OnAppProtectionLogLoadFinish)
        NOTIFY_HANDLER_EX(IDC_LST_PROTECTION_CTRL, BKLVM_GET_DISPINFO, OnBkLvmGetDispInfoProtectionCtrl)
        NOTIFY_HANDLER_EX(IDC_LST_PROTECTION_CTRL, BKLVM_ITEMCLICK, OnBkLvmItemClickProtectionCtrl)
        NOTIFY_HANDLER_EX(IDC_LST_PROTECTION_CTRL_LOG, BKLVM_GET_DISPINFO, OnBkLvmGetDispInfoProtectionLog)
        NOTIFY_HANDLER_EX(IDC_LST_PROTECTION_CTRL_LOG, BKLVM_ITEMCLICK, OnBkLvmItemClickProtectionLog)
	END_MSG_MAP()

protected:
	LRESULT OnAppProtectionSwitchChange(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
	void OnLblProtectionTurnOffLeak();
	void OnLblProtectionTurnOffRegister();
	void OnLblProtectionTurnOffProcess();
	void OnLblProtectionTurnOffUDisk();
	void OnLblProtectionTurnOffBrowserProtect();
    void OnLblProtectionTurnOffBwspPhishing();
	void OnLblProtectionTurnOnLeak();
	void OnLblProtectionTurnOnRegister();
	void OnLblProtectionTurnOnProcess();
	void OnLblProtectionTurnOnUDisk();
	void OnLblProtectionTurnOnBrowserProtect();
    void OnLblProtectionTurnOnBwspPhishing();
	void OnLblProtectionLogShow();
	void OnLblProtectionScan();
	void OnLblProtectionTrunOnAll();
	void OnLblProtectionSetting();
	void TrunOn(DWORD dwMonitorId, BOOL bOn);

    BOOL OnBkTabMainSelChange(int nTabItemIDOld, int nTabItemIDNew);
    LRESULT OnBkLvmGetDispInfoProtectionCtrl(LPNMHDR pnmh);
    LRESULT OnBkLvmItemClickProtectionCtrl(LPNMHDR pnmh);
    LRESULT OnBkLvmGetDispInfoProtectionLog(LPNMHDR pnmh);
    LRESULT OnBkLvmItemClickProtectionLog(LPNMHDR pnmh);
	LRESULT OnAppProtectionCtrlLoadFinish(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
	LRESULT OnAppProtectionLogLoadFinish(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
    void OnLblProtectionCtrlCheckAll();
    void OnLblProtectionCtrlUncheckAll();
    void OnLblProtectionCtrlRemoveCheck();
    void OnLblProtectionCtrlLogClear();
    void OnLblProtectionBwspMoreSetting();

protected:
	int m_nPage;

private:
	CBeikeSafeMainDlg *m_pNotifyWnd;
	BOOL m_bWarningTrunOff;
	CBkSafeProtectionNotifyWindow m_wndProtectionNotify;

    BOOL m_bPageNeverShow[3];

	UINT ShowPanelMsg(LPCTSTR pszCaption);

    CBkListView m_wndListMonitorCtrl;
    CBkListView m_wndListMonitorLog;
    BOOL m_bShowBwspSetting;

    class _ProtectionCtrlArrayItem 
    {
    public:
        _ProtectionCtrlArrayItem()
        {
            bCheck = FALSE;
            bBlock = FALSE;
            hIcon  = NULL;
        }

//         ~_ProtectionCtrlArrayItem()
//         {
//             if (hIcon)
//                 ::DestroyIcon(hIcon);
//         }

        BOOL bCheck;
        BOOL bBlock;
        CString strPath;
        CString strDetail;
        HICON hIcon;
    };

    CAtlArray<_ProtectionCtrlArrayItem> m_arrProtectionCtrl;
    CAtlArray<HICON> m_arrIconTemp;
    HICON m_hIconRegedit;

    class _ProtectionCtrlLogArrayItem 
    {
    public:
        CString strType;
        CString strTime;
        CString strOp;
        CString strFileName;
        CString strDetail;
    };

    CAtlArray<_ProtectionCtrlLogArrayItem> m_arrProtectionCtrlLog;

    static DWORD WINAPI _LoadProtectionCtrlListThreadProc(LPVOID pvParam);
    static DWORD WINAPI _DeleteProtectionCtrlListThreadProc(LPVOID pvParam);
    static DWORD WINAPI _LoadProtectionLogThreadProc(LPVOID pvParam);
    static DWORD WINAPI _ClearProtectionLogThreadProc(LPVOID pvParam);

    static void _AppendLogToArray(LPCSTR szLine, CAtlArray<_ProtectionCtrlLogArrayItem>& arrLog);
    static void _LoadLog(HWND hWndNotify);
};

class CBKSafeMonitorNavigator
	: public CBkNavigator
{
public:
	CBKSafeMonitorNavigator(CBeikeSafeMainDlg *pDialog)
		: m_pDlg(pDialog)
	{

	}

	CBkNavigator* OnNavigate(CString &strChildName);
	UINT_PTR DoModal(int nPage = MonitorSettingPageCommon, HWND hWndParent = ::GetActiveWindow());

protected:

	CBeikeSafeMainDlg *m_pDlg;
};
