#pragma once

#include <wtlhelper/whwindow.h>
#include "beikesafe.h"
#include "bkmsgdefine.h"
#include <common/utility.h>
#include "bksafeprotectionnotifywnd.h"
#include "beikesafearpdlg.h"
#include "downloadtoolsdlg.h"
class CBeikeSafeMainDlg;
class KUrlMonCfgReader;
class CARPInstallWarning;

enum
{
	enum_SafeMonitorTab_Protection	= 0,
//	enum_SafeMonitorTab_UrlMon		= 1,
    enum_safeMonitorTab_NetProt,
	enum_SafeMonitorTab_Trust,
	enum_SafeMonitorTab_Log		
};

class CBeiKeSafeMonitorDlg : public CBkNavigator
{
public:
	CBeiKeSafeMonitorDlg(CBeikeSafeMainDlg* pNotifyWnd);
	~CBeiKeSafeMonitorDlg(void);

public:
	void Init();
	int CloseCheck();
	CBkNavigator* OnNavigate( CString &strChildName );

protected:
	LRESULT OnAppProtectionSwitchChange(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
	void OnLblProtectionTurnLeak();
	void OnLblProtectionTurnRegister();
	void OnLblProtectionTurnProcess();
	void OnLblProtectionTurnUDisk();
	void OnLblProtectionTurnKws();
	void OnLblProtectionTurnKwsSafeDown();
    void OnLblProtectionTurnArp();
    void OnLblProtectionArpHistory();
    void OnLblProtectionArpSetting();
    void OnLblProtectionArpStart();

	void OnLblUrlMonTurn();
	void OnLblUrlMonLog();

	void OnLblProtectionLogShow();
	void OnLblProtectionScan();
    void OnLblNetProtectionScan();
	void OnLblProtectionTrunOnAll();
    void OnLblNetProtectionTrunOnAll();
	void OnLblProtectionSetting();
    void OnLblNetProtectionSetting();
	void TurnSwitch(DWORD dwMonitorId);

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

	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)	
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_LEAK, OnLblProtectionTurnLeak)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_REGISTER, OnLblProtectionTurnRegister)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_PROCESS, OnLblProtectionTurnProcess)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_UDISK, OnLblProtectionTurnUDisk)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_KWS, OnLblProtectionTurnKws)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_KWS_SAFEDOWN, OnLblProtectionTurnKwsSafeDown)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ARP, OnLblProtectionTurnArp)

		BK_NOTIFY_ID_COMMAND(IDC_LBL_URLMON_TURN, OnLblUrlMonTurn)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_URLMON_LOG, OnLblUrlMonLog)

		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_LOG_SHOW, OnLblProtectionLogShow)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_SCAN, OnLblProtectionScan)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TRUN_ON_ALL, OnLblProtectionTrunOnAll)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_NETPROT_TRUN_ON_ALL, OnLblNetProtectionTrunOnAll)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_SETTING, OnLblProtectionSetting)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_NETPROT_SETTING, OnLblNetProtectionSetting)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_NETPROT_SCAN, OnLblNetProtectionScan)

		BK_NOTIFY_TAB_SELCHANGE(IDC_SAFEMONITOR_TAB_2001, OnBkTabMainSelChange)

		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_CTRL_CHECK_ALL, OnLblProtectionCtrlCheckAll)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_CTRL_UNCHECK_ALL, OnLblProtectionCtrlUncheckAll)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_PROTECTION_CTRL_REMOVE_CHECK, OnLblProtectionCtrlRemoveCheck)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_PROTECTION_CTRL_LOG_CLEAR, OnLblProtectionCtrlLogClear)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_BWSP_MORE_SETTING1, OnLblProtectionBwspMoreSetting)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_BWSP_MORE_SETTING2, OnLblProtectionBwspMoreSetting)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_ARPHISTORY, OnLblProtectionArpHistory)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_ARPSETTING, OnLblProtectionArpSetting)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_PROTECTION_TURN_ARP_START, OnLblProtectionArpStart)
      
		BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBeiKeSafeMonitorDlg)
//		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
// 		CHAIN_MSG_MAP(CBkDialogImpl<CBeiKeSafeMonitorDlg>)
// 		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBKSafeMonitorDlg>)
		MESSAGE_HANDLER_EX(MSG_APP_PROTECTION_SWITCH_CHANGE, OnAppProtectionSwitchChange)
		MESSAGE_HANDLER_EX(MSG_APP_PROTECTION_CTRL_LOAD_FINISH, OnAppProtectionCtrlLoadFinish)
		MESSAGE_HANDLER_EX(MSG_APP_PROTECTION_LOG_LOAD_FINISH, OnAppProtectionLogLoadFinish)
		NOTIFY_HANDLER_EX(IDC_LST_PROTECTION_CTRL, BKLVM_GET_DISPINFO, OnBkLvmGetDispInfoProtectionCtrl)
		NOTIFY_HANDLER_EX(IDC_LST_PROTECTION_CTRL, BKLVM_ITEMCLICK, OnBkLvmItemClickProtectionCtrl)
		NOTIFY_HANDLER_EX(IDC_LST_PROTECTION_CTRL_LOG, BKLVM_GET_DISPINFO, OnBkLvmGetDispInfoProtectionLog)
		NOTIFY_HANDLER_EX(IDC_LST_PROTECTION_CTRL_LOG, BKLVM_ITEMCLICK, OnBkLvmItemClickProtectionLog)
	END_MSG_MAP()

private:
	int							m_nNowTab;

private:
	CBeikeSafeMainDlg*			m_pNotifyWnd;
    KComInterfacePtr<IArpFwSetting> m_piArpSetting;
    BOOL                        m_bFwEnable;
    CToolsDownloadDlg*		    m_pDownInstaller;
    CARPInstallWarning*         m_dlgWarning;

	BOOL m_bWarningTrunOff;
	CBkSafeProtectionNotifyWindow m_wndProtectionNotify;
	UINT ShowMsg(LPCTSTR pszCaption);
	UINT ShowPanelMsg(LPCTSTR pszCaption);
    BOOL QueryArpRegStatus(LPCWSTR lpValuename);
    BOOL QueryArpSpStatu();
    BOOL WriteArprunFlag(LPCWSTR lpvaluename,DWORD dwRegvalue);
    BOOL CheckArpSysReg();
    BOOL CreateArpUninstllog(LPCWSTR lpFilename);
    BOOL StartArpFirewallAsync();

    static UINT WINAPI StartArpFirewallThreadProc(void* pParam);

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

	HANDLE m_hRefreshThread;
	HANDLE m_hRefreshEvent;
    HANDLE m_hInstallArpFw;
	volatile BOOL m_bRefreshExit;
	static unsigned __stdcall RefreshThreadProc(LPVOID pvParam);
    static unsigned __stdcall InstallArpFwThreadProc(LPVOID pvParam);
	void _RefreshThreadProc();

	void _LoadProtectionCtrl();
	void _LoadProtectionLog();
	void _LoadUrlMonCfg(KUrlMonCfgReader& cfgUrlMon);

    HRESULT InstallArpFw();
    void UnZipArpPackage();
    BOOL StartArpfwSp();
    BOOL CheckLocalFileExist(LPCWSTR lpFilename);

	static DWORD WINAPI _DeleteProtectionCtrlListThreadProc(LPVOID pvParam);
	static DWORD WINAPI _ClearProtectionLogThreadProc(LPVOID pvParam);

	static void _AppendLogToArray(LPCSTR szLine, CAtlArray<_ProtectionCtrlLogArrayItem>& arrLog);
};

//////////////////////////////////////////////////////////////////////////
class CARPInstallWarning 
    : public CBkDialogImpl<CARPInstallWarning>
    , public CWHRoundRectFrameHelper<CARPInstallWarning>
{
public:
    CARPInstallWarning() : CBkDialogImpl<CARPInstallWarning>(IDR_DLG_ARP_INSTALL)
    {

    }
    virtual ~CARPInstallWarning()
    {

    }

    void OnBkBtnClose()
    {
        EndDialog(0);
    }

    void OnOK()
    {
        EndDialog(IDOK);
    }

	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)	
        BK_NOTIFY_ID_COMMAND(IDC_BTN_ARP_INATLL, OnBkBtnClose)
        BK_NOTIFY_ID_COMMAND(IDOK, OnOK)
        
    BK_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(CARPInstallWarning)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CBkDialogImpl<CARPInstallWarning>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CARPInstallWarning>)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};
//////////////////////////////////////////////////////////////////////////

