#pragma once
//
#include "beikesafequarantinedlg.h"
#include "setAdFilterRule.h"
#include "setAdFilter.h"
#include "setBrowsProtect.h"
#include "kws\pedownloaddef.h"
//#include "beikesafeiefixuihandler.h"
#include "setProxServer.h"
#include "beikenetmonitorlimitedit.h"

class CBeikeSafeMainDlg;

enum {
    SettingPageCommon = 0,
    SettingPageVirScan,
    //SettingPageVulScan,
    SettingPageProtection,
    //SettingPageIEFix,
	SettingPageSysopt,//hub
	SettingPageKws,//网盾设置
	SettingPageSoftMgr,//软件管理

    SettingPageCount,
};

#define FILEPATH			L"\\Cfg\\bksafe.ini"
#define SEC_RUNOPT			L"runopt"
#define	KEY_STARTTIME		L"startuptime"
#define KEY_RUNONCE			L"runonce"
#define KEY_CONDITION_SHOW	L"condition_show"
#define KEY_AFTER_OPT		L"after_opt"
#define KEY_BOOT_SLOW		L"boot_slow"
#define KEY_ALLWAYS_SHOW	L"allways_show"
#define KEY_NEVER_SHOW		L"never_show"

#define SEC_PERFOPT			L"perfopt"
#define KEY_WND_OPT_CHECK	L"check"

#define VALUE_LENGTH		10
#define STMGR_UNKNOWN_FILENAME		TEXT("kpcfileopen.exe")
#define STMGR_DEFAULT_UN_FILE_P		TEXT("%SystemRoot%\\system32\\rundll32.exe %SystemRoot%\\system32\\shell32.dll,OpenAs_RunDLL %1")

//hub 免打扰提示框
class CBeikeSafeSafepassSettingDlg
	: public CBkDialogImpl<CBeikeSafeSafepassSettingDlg>
	, public CWHRoundRectFrameHelper<CBeikeSafeSafepassSettingDlg>
{
public:
	CBeikeSafeSafepassSettingDlg()
		: CBkDialogImpl<CBeikeSafeSafepassSettingDlg>(IDR_BK_SAFEPASS_SETTING_DLG)
	{
	}

	~CBeikeSafeSafepassSettingDlg(void)
	{
	}

	void OnBkBtnOK()
	{
		EndDialog(IDCANCEL);
	}
public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SETTINGS_OK,OnBkBtnOK);	
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnOK)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBeikeSafeSafepassSettingDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBeikeSafeSafepassSettingDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBeikeSafeSafepassSettingDlg>)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
};


class CBeikeSafeSettingDlg
    : public CBkDialogImpl<CBeikeSafeSettingDlg>
    , public CWHRoundRectFrameHelper<CBeikeSafeSettingDlg>
{
public:
    CBeikeSafeSettingDlg()
        : CBkDialogImpl<CBeikeSafeSettingDlg>(IDR_BK_SETTING_DLG)
        , m_nPage(SettingPageCommon)
        , m_bBkSafeSettingChanged(FALSE)
        , m_bTraySettingChanged(FALSE)
        , m_bSvcSettingChanged(FALSE)
        , m_bIEFixSettingChanged(FALSE)
		, m_bVulfixSettingChanged(FALSE)
		, m_bR1Checked(TRUE)//开机提示
		, m_bC1Checked(TRUE)
		, m_bC2Checked(TRUE)
		, m_bR2Checked(FALSE)
		, m_bR3Checked(FALSE)
		,m_bBkSoftMgrSettingChanged(FALSE)
		,m_bBkSafeKwsSettingChanged(FALSE)
		,m_bProxChanged(FALSE)
    {
		m_nSubPage = -1;
    }
	
	~CBeikeSafeSettingDlg();

    UINT_PTR DoModal(int nPage, HWND hWndParent,  CBeikeSafeMainDlg *pDialog, int nSubPage = -1);
    BOOL BkSafeSettingChanged();

protected:

    int m_nPage;
	CBeikeSafeMainDlg* m_dlg;
	int m_nSubPage;
    BOOL m_bBkSafeSettingChanged;
    BOOL m_bTraySettingChanged;
    BOOL m_bSvcSettingChanged;
    BOOL m_bIEFixSettingChanged;
	BOOL m_bVulfixSettingChanged;	
	//开机提示
	CString m_strFilePath;
	BOOL m_bR1Checked;
	BOOL m_bC1Checked;
	BOOL m_bC2Checked;
	BOOL m_bR2Checked;
	BOOL m_bR3Checked;

	CEdit	m_ctlEditStorePath;		// 保存安装文件的目录
	BOOL	m_bBkSoftMgrSettingChanged;	
	
	BOOL	m_bBkSafeKwsSettingChanged;
	//服务器代理设置
	KwsScanner		m_kwsScanner;
	BOOL			m_bProxChanged;

	DWORD MsgBoxHideFloatWnd();
	DWORD MsgBoxNetMonDriverError(CString strTitle, CString strDes);

    BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	LRESULT OnAppProxyChange( UINT uMsg, WPARAM wParam, LPARAM lParam );

    void OnBkBtnOK();
    void OnBkBtnClose();
    void _SaveSettings();
    void _LoadBkSafeSettings();
    void _LoadTraySettings();
    void _LoadSvcSettings();
    void _LoadIEFixSettings();
    void _SaveBkSafeSettings();
    void _SaveTraySettings();
    void _SaveSvcSettings();
    void _SaveIEFixSettings();
    void OnBkSafeSettingChanged();
    void OnBkSafeAndSvcSettingChanged();
    void OnTraySettingAutorun();
    void OnTraySettingChanged();
    void OnIEFixSettingChanged();
    void OnSvcSettingChanged();
    void OnBtnSettingShowQuarantineDlg();
	void OnTrayProtectionTrunOn();

	void OnChkSettingVulfixNosave();
	void OnChkSettingVulfixSaveDown();

    void OnLblSettingWhiteList();

	void OnTraySettingStartupRun();

	// 漏洞修复 
	CEdit m_ctlEdit, m_ctlVulEditLocalPath;
	BOOL m_isVulRepairing;
	
	void _Vulfix_Setting_Init();
	void _Vulfix_Setting_Reset();
	BOOL _Vulfix_Setting_Save();
	void _Vulfix_Setting_UpdateTitle();
	void _Vulfix_Setting_UpdateTitle(CString strFileName);
	void OnBtn_Vulfix_SelFolder();
	void OnBtn_Vulfix_OpenFolder();
	void OnBtn_Vulfix_DeleteAllFiles();
	void OnBtn_Vulfix_Changed_DownloadSetting();
	void OnBtn_Vulfix_SelectLocalPath();	
	void _Vulfix_Update_DownloadSetting( BOOL bEnableBtnsForLocal );
	void SaveExamSetting();
	void OnSafePassDlg();//免打扰对话框
	void _SaveBkSysOptSettings();//系统优化的设置 hub
	void _SaveBkSoftMgrSettings();//软件管理的设置
	void _SaveBkKwsSettings();//网盾的相关设置
	void OnRadioCheckChanged();//系统优化
	void OnCheckChanged();
	void ReadSettingsAndSetCheck();
	void _LoadBkSysOptSettings();
	void WriteSettings();

	BOOL IsFloatWndStartWithTray();

	VOID OpenFloatWnd();
	void CloseFloatWnd();
	void OnClickWndOpt();

	void OpenNetFlowWnd();//打开流量窗口
	void CloseNetFlowWnd();//关闭流量窗口

	void OpenSysPerfFloatWnd();//开启性能监控
	void CloseSysPerfFloatWnd();//关闭性能监控

	void OnOpenFloatWnd();

	BOOL IsNetMonDriverOk();

	BOOL GetFloatWndDisplayStatus();//获得流量控制的值  从配置文件中获得
	BOOL IsSysPerfFloatWndVisable();
	void _LoadBkSoftMgrSettings();//软件管理
	void LoadSoftMgrSettings();
	void OnRestoreDefaultDir();//恢复默认目录
	void GetCurDiskFreeSpace( LPCTSTR szCurrentPath, CString & strFreeSpace );//获取磁盘剩余空间
	void SaveSoftMgrSettings();//保存修改的设置
	void OnOpenStoreDir();//打开目录
	void OnSelDir();//选择目录
	void OnBkSafeSoftMgrSettingChanged();//设置是否存在变更	
	void OnBkSafeSoftMgrCreateDesktopIcoChanged();
	void _LoadBkSafeKwsSettings();//网盾设置
	void OnBkSafeKwsAdRuleSet();//   广告规则
	void OnBkSafeKwsAdFilterSet();//广告过滤白名单
	void OnBkSafeKwsDownListen();//下载保护监控
	void OnBkSafeKwsSettingChanged();//网盾设置是否变更
	void UpdataStat(BOOL bOpen);//check的状态变化
	void SaveSafeKwsSettings();//网盾设置保存
	void OnBkSafeKwsBrowsPage();//浏览器设置页面	
	//CBeiKeSafeIEFixUIHandler* m_Data;
	//代理服务器设置
	void OnBkSafeProxServerPage();
	void OnProxServerCheckChange();//check改变
	BOOL ProxServerReadCfg();//读配置文件  代理服务器
	int _LoadBkSafePorxServer();//代理服务器加载
	void _SaveBkSafePorxServer();//代理服务器的保存
	BOOL doProxServerApply(ATL::CString strFilePath, int nType);//代理设置的改变
	BOOL IsUnknownFileOpenValid();

	//流量控制
	CBKNetMonitorLimitEdit m_editDayLimit;
	CBKNetMonitorLimitEdit m_editMonthLimit;
	void _SetNetMoniterValue(IN const CString& strTerm, IN const CString& strName, IN const CString& strValue);
	int  _GetNetMoniterValue(IN const CString& strTerm, IN const CString& strName);
	void OnBkSafeNetCheckChange();//流量控制的check变化	
	void OnClickSysPerfCtrl();	//资源控制的check变化	
	BOOL ShowProxySetting();
	void OnClickNetRemindDay(void);
	void OnClickNetRemindMonth(void);
	void OnClickCheckDay();//点击check
	void OnClickCheckMonth();//点击check

	BOOL IsExistDesktopIcon();
	void CreateDesktopIcon(BOOL bCreate, BOOL bNum);

	void OnTimer(UINT_PTR nIDEvent);

	int  ReBoot();

public:

    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
       BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
       BK_NOTIFY_ID_COMMAND(IDOK, OnBkBtnOK)
       BK_NOTIFY_ID_COMMAND(IDCANCEL, OnBkBtnClose)

       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_BKSAFE_AUTO_EXAM  , OnBkSafeSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_BKSAFE_DAY_EXAM  , OnBkSafeSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_BKSAFE_MANUAL_EXAM  , OnBkSafeSettingChanged)
       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_SVC_AUTO_CLEAN    , OnBkSafeAndSvcSettingChanged)
       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_SVC_MANUAL_CLEAN  , OnBkSafeAndSvcSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_AUTO_EXPANDSCAN  , OnBkSafeAndSvcSettingChanged)

	   BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_VULFIX_NO_SAVE, OnChkSettingVulfixNosave);
	   BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_VULFIX_SAVE_DOWN, OnChkSettingVulfixSaveDown);

       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_TRAY_AUTORUN   , OnTraySettingAutorun)  /* 开机启动, 未使用 */
       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_TRAY_AUTOCHECK , OnTraySettingChanged)
       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_TRAY_GAMEBYPASS, OnTraySettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_TRAY_AUTO_UPDATE, OnTraySettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_TRAY_ASK_UPDATE, OnTraySettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_TRAY_SAFEPASS, OnTraySettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_LBL_SETTING_TRAY_PROTECTION_TRUN_ON , OnTrayProtectionTrunOn)

       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_SVC_JOIN_BEIKECLOUD , OnSvcSettingChanged)

       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_IEFIX_RIGHTMENU   , OnIEFixSettingChanged)
       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_IEFIX_TOOLBARICON , OnIEFixSettingChanged)
       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_IEFIX_THIRDTOOLBAR, OnIEFixSettingChanged)
       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_IEFIX_BHO         , OnIEFixSettingChanged)
       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_IEFIX_SAFEWEB     , OnIEFixSettingChanged)
       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_IEFIX_PROTOCOL    , OnIEFixSettingChanged)
       BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_IEFIX_SEARCHENG   , OnIEFixSettingChanged)
	   

       BK_NOTIFY_ID_COMMAND(IDC_BTN_SETTING_SHOW_QUARANTINEDLG, OnBtnSettingShowQuarantineDlg)
		
	   // 漏洞修复  
	   BK_NOTIFY_ID_COMMAND(IDC_BTN_SETTING_VULFIX_SELFOLDER, OnBtn_Vulfix_SelFolder)
	   BK_NOTIFY_ID_COMMAND(IDC_BTN_SETTING_VULFIX_OPENFOLDER, OnBtn_Vulfix_OpenFolder)
	   BK_NOTIFY_ID_COMMAND(IDC_BTN_SETTING_VULFIX_DELETEFILES, OnBtn_Vulfix_DeleteAllFiles)
	   BK_NOTIFY_ID_COMMAND(1200 , OnBtn_Vulfix_Changed_DownloadSetting)
	   BK_NOTIFY_ID_COMMAND(1201 , OnBtn_Vulfix_Changed_DownloadSetting)
	   BK_NOTIFY_ID_COMMAND(1203, OnBtn_Vulfix_SelectLocalPath)
	   
       BK_NOTIFY_ID_COMMAND(IDC_LBL_SETTING_WHITE_LIST, OnLblSettingWhiteList)
	   BK_NOTIFY_ID_COMMAND(IDC_TXT_SETTING_TRAY_SAFEPASS, OnSafePassDlg)
	   
	   //hub  系统优化 开机提示
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_CONDITIONS_SHOW,	OnRadioCheckChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_ALLWAYS_SHOW,		OnRadioCheckChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_NEVER_SHOW,		OnRadioCheckChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_CHECK_AFTER_OPT_START,	OnCheckChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_CHECK_BOOT_TOOSLOW,		OnCheckChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_CHECK_NETFLOW_CONTROL,	OnBkSafeNetCheckChange)//流量监控
	   BK_NOTIFY_ID_COMMAND(IDC_CHECK_REMIND_DAY_RDLG,	OnClickNetRemindDay)//流量监控
	   BK_NOTIFY_ID_COMMAND(IDC_CHECK_REMIND_MONTH_RDLG,OnClickNetRemindMonth)//流量监控
	   BK_NOTIFY_ID_COMMAND(IDC_CHECK_REMIND_DAY_TEXT,	OnClickCheckDay)//流量监控
	   BK_NOTIFY_ID_COMMAND(IDC_CHECK_REMIND_MONTH_TEXT,OnClickCheckMonth)//流量监控
	   BK_NOTIFY_ID_COMMAND(IDC_CHECK_SYSPER_CONTROL,  OnClickSysPerfCtrl)//资源监控
	   BK_NOTIFY_ID_COMMAND(IDC_CHECK_WND_OPT,			OnClickWndOpt)

	   BK_NOTIFY_ID_COMMAND(IDC_CHECK_FLOATWND_CONTROL, OnOpenFloatWnd)

	   //hub 软件管理  
	   BK_NOTIFY_ID_COMMAND( IDC_LINKTEXT_DEFAULT_DIR, OnRestoreDefaultDir )
	   BK_NOTIFY_ID_COMMAND(IDC_LINKTEXT_OPEN_DIR, OnOpenStoreDir)
	   BK_NOTIFY_ID_COMMAND( IDC_IMGBTN_SELECT_DIR, OnSelDir )
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_SHOW_PLUGIN_TIP,	OnBkSafeSoftMgrSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_IGNORE_PLUGIN_TIP,	OnBkSafeSoftMgrSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_DELETE_SETUP_WEEK,	OnBkSafeSoftMgrSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_DELETE_SETUP_NOW,	OnBkSafeSoftMgrSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_DELETE_SETUP_NEVER,	OnBkSafeSoftMgrSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_RUN_SETUP_AUTO,	OnBkSafeSoftMgrSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_RUN_SETUP_HAND,	OnBkSafeSoftMgrSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_UPDATE_TIP_DAY,	OnBkSafeSoftMgrSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_UPDATE_TIP_WEEK,	OnBkSafeSoftMgrSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_UPDATE_TIP_NEVER,	OnBkSafeSoftMgrSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_DELETE_TO_RECYCLE,	OnBkSafeSoftMgrSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_DELETE_TO_DIRECT,	OnBkSafeSoftMgrSettingChanged)

	   BK_NOTIFY_ID_COMMAND( IDC_CHECK_NO_HINT, OnBkSafeSoftMgrCreateDesktopIcoChanged )
	   BK_NOTIFY_ID_COMMAND( IDC_CHECK_SHOW_ICO_NUM, OnBkSafeSoftMgrSettingChanged )
	   BK_NOTIFY_ID_COMMAND( IDC_CHECK_UNKNOWN_FILE_OPEN, OnBkSafeSoftMgrSettingChanged )
	   BK_NOTIFY_ID_COMMAND(IDC_CHECK_MAJOR_POINT_OUT,	OnBkSafeSoftMgrSettingChanged)
	   BK_NOTIFY_ID_COMMAND(IDC_RADIO_UPDATE_WHEN_RUN,	OnBkSafeSoftMgrSettingChanged)

	   //hub  网盾设置 
	   BK_NOTIFY_ID_COMMAND( IDC_LINKTEXT_ADRULE_SET,   OnBkSafeKwsAdRuleSet )
	   BK_NOTIFY_ID_COMMAND( IDC_LINKTEXT_ADFILTER_SET, OnBkSafeKwsAdFilterSet )
	   BK_NOTIFY_ID_COMMAND( IDC_CHECK_ADFILTER_CUE,	OnBkSafeKwsSettingChanged )//广告过滤
	   BK_NOTIFY_ID_COMMAND( IDC_CHECK_ADFILTER_LOG,	OnBkSafeKwsSettingChanged )//
	   BK_NOTIFY_ID_COMMAND( IDC_CHECK_DOWNLOAD_LIS,	OnBkSafeKwsDownListen )//下载保护监控
	   BK_NOTIFY_ID_COMMAND( IDC_CHECK_DOWNLOAD_AUTOCLEAN,	OnBkSafeKwsSettingChanged )//
	   BK_NOTIFY_ID_COMMAND( IDC_CHECK_DOWNLOAD_SAFE_NOCLUE,	OnBkSafeKwsSettingChanged )//
	   BK_NOTIFY_ID_COMMAND( IDC_CHECK_DOWNLOAD_UNKNOWN_AUTOSUBMIT,	OnBkSafeKwsSettingChanged )//
	   
	   BK_NOTIFY_ID_COMMAND( IDC_LINKTEXT_DOWNLOAD_PROBROWS,	OnBkSafeKwsBrowsPage )//浏览器设置

	   //代理服务器设置
	   BK_NOTIFY_ID_COMMAND( IDC_LINKTEXT_PROX_SERVER,	OnBkSafeProxServerPage )//代理服务器设置
	   BK_NOTIFY_ID_COMMAND( IDC_CHECK_PROX_SERVER,		OnProxServerCheckChange)//代理服务器check

	   BK_NOTIFY_ID_COMMAND(IDC_CHK_SETTING_RUNSTARTUP, OnTraySettingStartupRun)  /* 开机启动选框 */
	   

    BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBeikeSafeSettingDlg)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CBkDialogImpl<CBeikeSafeSettingDlg>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBeikeSafeSettingDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		MESSAGE_HANDLER_EX( MSG_PROXY_CHANGE, OnAppProxyChange )
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};

class CBeikeSafeSettingNavigator
    : public CBkNavigator
{
public:
    CBeikeSafeSettingNavigator(CBeikeSafeMainDlg *pDialog)
        : m_pDlg(pDialog)
		, m_bHasDoModal(FALSE)
    {

    }

    CBkNavigator* OnNavigate(CString &strChildName);
    UINT_PTR DoModal(int nPage = SettingPageCommon, HWND hWndParent = ::GetActiveWindow(), int nSubPage = -1);

protected:

    CBeikeSafeMainDlg*		m_pDlg;
	BOOL					m_bHasDoModal;
};
