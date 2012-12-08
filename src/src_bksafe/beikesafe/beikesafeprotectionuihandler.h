#pragma once

#include "bksafeprotectionnotifywnd.h"

class CBeikeSafeMainDlg;

class CBeikeSafeProtectionUIHandler
    : public CBkNavigator
{
public:
    CBeikeSafeProtectionUIHandler(CBeikeSafeMainDlg *pDialog)
        : m_pDlg(pDialog)
    {

    }

    ~CBeikeSafeProtectionUIHandler();

    void Init();

protected:

    CBeikeSafeMainDlg *m_pDlg;

	BOOL m_bWarningTrunOff;
    CBkSafeProtectionNotifyWindow m_wndProtectionNotify;

    void _LoadLog();

    LRESULT OnAppProtectionSwitchChange(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);

    void OnLblProtectionTurnOffLeak();
    void OnLblProtectionTurnOffRegister();
    void OnLblProtectionTurnOffProcess();
    void OnLblProtectionTurnOffUDisk();
	void OnLblProtectionTurnOffBrowserProtect();
    void OnLblProtectionTurnOnLeak();
    void OnLblProtectionTurnOnRegister();
    void OnLblProtectionTurnOnProcess();
    void OnLblProtectionTurnOnUDisk();
	void OnLblProtectionTurnOnBrowserProtect();
    void OnLblProtectionLogShow();
	
	void OnLblProtectionScan();
	void OnLblProtectionTrunOnAll();
	void OnLblProtectionSetting();

	void TrunOn(DWORD dwMonitorId, BOOL bOn);

public:
    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_OFF_LEAK, OnLblProtectionTurnOffLeak)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_OFF_REGISTER, OnLblProtectionTurnOffRegister)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_OFF_PROCESS, OnLblProtectionTurnOffProcess)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_OFF_UDISK, OnLblProtectionTurnOffUDisk)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_OFF_BROWSERTPROTECT, OnLblProtectionTurnOffBrowserProtect)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ON_LEAK, OnLblProtectionTurnOnLeak)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ON_REGISTER, OnLblProtectionTurnOnRegister)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ON_PROCESS, OnLblProtectionTurnOnProcess)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ON_UDISK, OnLblProtectionTurnOnUDisk)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ON_BROWSERTPROTECT, OnLblProtectionTurnOnBrowserProtect)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_LOG_SHOW, OnLblProtectionLogShow)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_SCAN, OnLblProtectionScan)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TRUN_ON_ALL, OnLblProtectionTrunOnAll)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_SETTING, OnLblProtectionSetting)
    BK_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(CBeikeSafeExamUIHandler)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
        MESSAGE_HANDLER_EX(MSG_APP_PROTECTION_SWITCH_CHANGE, OnAppProtectionSwitchChange)
    END_MSG_MAP()
};
