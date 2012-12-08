#pragma once

#include "beikesafemsgbox.h"
#include "vulfix/BeikeVulfixHandler.h"
#include "bksafeprotectionnotifywnd.h"
#include "beikesafesettingdlg.h"
#include "beikesafeupliveuihandler.h"
#include "bksafeuihandlerbase.h"
#include "kupdateversiondlg.h"
#include "communits/Locker.h"
#include <softmgr\URLEncode.h>
#include "kwebshielduihandler.h"
#include "ktrojanuihandler.h"//hub trojan

class CBeikeSafeExamUIHandler;
class CBeikeSafeVirusScanUIHandler;
class CBeikeSafePluginUIHandler;
//class CBeiKeSafeIEFixUIHandler;
class CBeikeSafeSysOptUIHandler;
class CBeikeSafeSoftmgrUIHandler;
class CKClearUIHandler;
class CKTrojanUIHandler;//hub trojan
class CBeikeSafeFreeVirusUIHandler;

class CBeikeSafeMainDlg
    : public CBkDialogImpl<CBeikeSafeMainDlg>
    , public CWHRoundRectFrameHelper<CBeikeSafeMainDlg>
    , public CBkNavigator
	, public CBkSafeUIHandlerMgr
	, public IScanToMain
{
public:
    CBeikeSafeMainDlg();
    ~CBeikeSafeMainDlg();

    void SetDefaultNavigate(LPCWSTR lpszNavigate);

    void EndDialog(UINT uRetCode);

    void RemoveFromTodoList(int nItem);
	void RemoveFromTodoList(REMOVE_EXAM_RESULT_ITEM RemoveExamItemInfo);

    void SetTodoText(int nOrder, CString& strNavigatorString, UINT uDivID, UINT uNoticeID, UINT uLinkID, UINT uImgID = 0);

    void SettingChanged();

    void InstallAVEngine();
    void AVEInstallFininshed();

	void InitIECtrl();

    void RefreshLastScanInfo(BOOL bScanned, DWORD dwScanMode, SYSTEMTIME stStartLast, SYSTEMTIME stStartFast, SYSTEMTIME stStartFull, SYSTEMTIME stStartCustom, DWORD dwVirusCount, DWORD dwRemainVirusCount);
    void RefreshReportedUnknownInfo(int nCount);

    void ShowVirusLog();
	void ShowVirusTrustList();
    void ShowReportedUnknownLog();

    void RescanVirusInLastScan();

	void ModifyMonitorState(int nType);
	void ModifyBwsMonitorState();
	void Echo();
	void TabMainSelChange( int nTabOldItem, int nTabNewItem );
	void UpdateDubaState();
	void InitProxy( BOOL bForce = FALSE );
	CBeikeSafePluginUIHandler*	GetPluginUIHandler();
	CURLEncode					url_encode;

	// IScanToMain
	virtual void	TrojanToMainChange(LPCWSTR strCommand);//木马查杀变更主界面
	virtual void	VirusInstallFinish();
	virtual int		SysCnvertExamind( int nId );
	virtual void    ExamRemove1( WPARAM wParam );
	virtual void	ExamRemove2( WPARAM wParam );
	virtual void	OpenDuba( WPARAM wParam );
	virtual void    TryOpenShell( WPARAM wParam );
	virtual void	ShowLocalTrojanVer( WPARAM wParam, LPARAM lParam );

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:

    CBeikeSafeExamUIHandler*         m_ExamHandler;
    CBeikeSafeUpliveUIHandler       m_UpliveHandler;
	CBeikeSafeSysOptUIHandler*       m_sysoptHandler;
	CBeikeSafeSoftmgrUIHandler*		m_SoftmgrHandle;
    CKClearUIHandler*                m_KClearHandle;
	CKTrojanUIHandler*				 m_KTrojanHandle;//hub trojan
	CBeikeSafeFreeVirusUIHandler*	m_pFreeVirusHandle;
	CKWebShieldUIHandler*           m_pWebShieldHandle;

    CWHHtmlContainerWindow m_wndHtmlOnlineInfo;
//     CWHHtmlContainerWindow m_wndHtmlVirScan;

    CBeikeSafeSettingNavigator m_SettingCaller;

    BOOL m_bFirstPageChange;
    BOOL m_bPage0NeverShowed;
    BOOL m_bPage1NeverShowed;
    BOOL m_bPage3NeverShowed;
	BOOL m_bPage5NeverShowed;

    CString m_strDefaultNavigate;

    void OnBkBtnMax();
    void OnBkBtnMin();
	void OnBkLblTitleSetting();
    void OnBkBtnSet();

    BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/);
	LRESULT OnDestroy();
	LRESULT OnTimer( UINT_PTR uIdEvent );
    LRESULT OnAppDefaultNavigate(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);

    LRESULT OnAppEchoFinish(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
	LRESULT OnAppShowPlugClean(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
    BOOL OnBkTabMainSelChange(int nTabItemIDOld, int nTabItemIDNew);
    void OnSysCommand(UINT nID, CPoint point);
	
	LRESULT OnAppDelayExam(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
	LRESULT OnAppDelayNavigateIE(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
	LRESULT OnAppProxyChange( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam );
	void InitVulFix();
	void TryToInitIEFix();
	void TryToInitKClear();
	void TryToInitKTrojan();//hub trojan

	LRESULT OnSettingMenuSetting(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSettingMenuUpdate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSettingMenuBBS(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	void _HideOemElement(void);//显示或隐藏OEM的界面元素
	LRESULT OnShowUserDefineAdRule(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void CheckReportDesktopInfo();
	static DWORD WINAPI ReportDeskInfoProc(LPVOID pvParam);
	void DoReportDeskInfo();

	//hub trojan
	LRESULT OnVirsLblDuba( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam );
	LRESULT OnKwsSettingChange( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam );
	LRESULT OnKwsFullScan( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam );
	LRESULT OnKwsFastScan( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam );

protected:
	// Vulfix 
	BOOL			m_bVulfixInited;
	BOOL			m_bVulfixRescanRequired;
	CEmbeddedView	m_viewVulfix;
	BOOL			m_bIeFixInit;
	BOOL			m_bKClearInit;
	BOOL			m_bKTrojanInit;//hub trojan
	BOOL			m_KWebShieldInit;
	CMenu			m_SettingMenu;
	CLocker			m_LockRemoveTodoList1;
	CLocker			m_LockRemoveTodoList2;

public:
	void OnBkBtnClose();
    CBkNavigator* OnNavigate(CString &strChildName);

    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_MAX,   OnBkBtnMax)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_MIN,   OnBkBtnMin)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_SET,   OnBkBtnSet)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_TITLE_SETTING, OnBkLblTitleSetting)
        BK_NOTIFY_TAB_SELCHANGE(IDC_TAB_MAIN,   OnBkTabMainSelChange)
    BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX_DECLARE(CBeikeSafeMainDlg)
};
