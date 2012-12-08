#include "stdafx.h"
#include "beikesafemaindlg.h"




#include <uplive/uplive.h>
#include <Skylark2/bkrenewproxy.h>
#include "kws\kactivate.h"
#include "Vulfix\BeikeVulfixEngine.h"
#include "scan\exectrl\kexectrlloader.h"
#include "_idl_gen/bksafesvc.h"
#include "beikesafeexamuihandler.h"
//#include "beikesafevirusscanuihandler.h"
//#include "beikesafepluginuihandler.h"
//#include "beikesafeiefixuihandler.h"
#include "beikesafesysoptuihandler.h"
#include "beikesafesoftmgrHeader.h"
#include "kclearuihandler.h"
#include "bksafetabctrl.h"
#include "beikesafefreevirusuihandler.h"
#include <Psapi.h>
#pragma comment(lib, "psapi.lib")
#include "communits/KSysInfo.h"
#include "desktopinfo.h"

extern CVulEngine *theEngine;
BOOL	g_bkMsgBox = FALSE;


#define STARTUP_RUN_HIT  L"由于您禁止了金山卫士开机启动，它将无法为您提供完整保护。\n 是否允许开机启动？"


//////////////////////////////////////////////////////////////////////////
/// 2010-10-9 修改消息宏的定义到CPP，以免编译的时候太慢
BEGIN_MSG_MAP_EX_IMP(CBeikeSafeMainDlg)
	MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
	CHAIN_MSG_MAP(CBkDialogImpl<CBeikeSafeMainDlg>)
	CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBeikeSafeMainDlg>)

	MESSAGE_HANDLER_EX(MSG_APP_ECHO_FINISH, OnAppEchoFinish)
	MSG_WM_TIMER(OnTimer)

	CHAIN_MSG_MAP_MEMBER((*m_SoftmgrHandle))
	CHAIN_MSG_MAP_MEMBER((*m_ExamHandler))
	//CHAIN_MSG_MAP_MEMBER((*m_VirusScanHandler))
	//CHAIN_MSG_MAP_MEMBER((*m_PlugScanHandler))		
	CHAIN_MSG_MAP_MEMBER((*m_pFreeVirusHandle))		
	//CHAIN_MSG_MAP_MEMBER((*m_IEFixScanHandler))		
	CHAIN_MSG_MAP_MEMBER(m_viewVulfix.m_viewSoftVul)
	//CHAIN_MSG_MAP_MEMBER(m_ProtectionHandler)
	CHAIN_MSG_MAP_MEMBER(m_UpliveHandler)
	CHAIN_MSG_MAP_MEMBER((*m_sysoptHandler))
	CHAIN_MSG_MAP_MEMBER((*m_KClearHandle))
	CHAIN_MSG_MAP_MEMBER((*m_KTrojanHandle))//hub trojan
	CHAIN_MSG_MAP_MEMBER((*m_pWebShieldHandle))

	MSG_WM_INITDIALOG(OnInitDialog)
	MSG_WM_SYSCOMMAND(OnSysCommand)
	MSG_WM_DESTROY(OnDestroy)
	MESSAGE_HANDLER_EX(MSG_APP_PROXY_CHANGE, OnAppProxyChange)
	MESSAGE_HANDLER_EX(MSG_APP_DELAY_EXAM, OnAppDelayExam)
	MESSAGE_HANDLER_EX(MSG_APP_DELAY_NAVIGATE_IE, OnAppDelayNavigateIE)
	MESSAGE_HANDLER_EX(MSG_APP_DEFAULT_NAVIGATE, OnAppDefaultNavigate)
	MESSAGE_HANDLER_EX(MSG_APP_SHOW_PLUG_CLEAN, OnAppShowPlugClean)
	MESSAGE_HANDLER_EX(WM_USER_DEFINE_AD_RULE, OnShowUserDefineAdRule)

	/* 设置菜单事件 (如需修改敬请通知) */
	COMMAND_ID_HANDLER(ID_SETTING_SETTING, OnSettingMenuSetting)
	COMMAND_ID_HANDLER(ID_SETTING_UPDATE,  OnSettingMenuUpdate)
	COMMAND_ID_HANDLER(ID_SETTING_BBS,	   OnSettingMenuBBS)

	MESSAGE_HANDLER_EX(WM_DRAWITEM,    OnDrawItem)
	MESSAGE_HANDLER_EX(WM_MEASUREITEM, OnMeasureItem)
	MESSAGE_HANDLER_EX(MSG_VIRS_LBL_DUBA, OnVirsLblDuba)//调用毒霸
	MESSAGE_HANDLER_EX(MSG_KWS_SETTING_CHANGE, OnKwsSettingChange)	
	MESSAGE_HANDLER_EX(MSG_KSWEB_FULL_SCAN, OnKwsFullScan)
	MESSAGE_HANDLER_EX(MSG_KSWEB_FAST_SCAN, OnKwsFastScan)
	REFLECT_NOTIFICATIONS_EX()
END_MSG_MAP_IMP()
//////////////////////////////////////////////////////////////////////////

CBeikeSafeMainDlg::CBeikeSafeMainDlg()
	: CBkDialogImpl<CBeikeSafeMainDlg>(IDR_BK_MAIN_DIALOG)
	, m_bPage0NeverShowed(TRUE)
	, m_bPage1NeverShowed(TRUE)
	, m_bPage5NeverShowed(TRUE)
	, m_UpliveHandler(this)
	, m_SettingCaller(this)
	// Vulfix  
	, m_bVulfixInited(FALSE)
	, m_bVulfixRescanRequired(FALSE)
	, m_bFirstPageChange(TRUE)
	, m_bIeFixInit(FALSE)
	, m_bKClearInit(FALSE)
	, m_bKTrojanInit(FALSE)
	,m_KWebShieldInit(FALSE)
{
	m_ExamHandler			= new CBeikeSafeExamUIHandler(this);
	//m_VirusScanHandler		= new CBeikeSafeVirusScanUIHandler(this);
	//m_PlugScanHandler		= new CBeikeSafePluginUIHandler(this);
	//m_IEFixScanHandler		= new CBeiKeSafeIEFixUIHandler(this);
	m_sysoptHandler			= new CBeikeSafeSysOptUIHandler(this);
	m_SoftmgrHandle			= new CBeikeSafeSoftmgrUIHandler(this);
	m_KClearHandle			= new CKClearUIHandler(this);
	m_KTrojanHandle			= new CKTrojanUIHandler(this);//hub trojan
	m_pFreeVirusHandle		= new CBeikeSafeFreeVirusUIHandler(this);
	m_pWebShieldHandle      = new CKWebShieldUIHandler(this);

// 	AddUIHandler(m_VirusScanHandler);
// 	AddUIHandler( (*m_VirusScanHandler).GetSysFixUIHandler());
	//AddUIHandler(m_PlugScanHandler);
	AddUIHandler(m_KTrojanHandle);//hub trojan
	AddUIHandler(m_SoftmgrHandle);
	AddUIHandler(&m_viewVulfix.m_viewSoftVul);
	AddUIHandler(m_KClearHandle);
	AddUIHandler(m_ExamHandler);
	AddUIHandler(m_pWebShieldHandle);
}

CBeikeSafeMainDlg::~CBeikeSafeMainDlg()
{
	delete m_ExamHandler;
	//delete m_VirusScanHandler;
	//delete m_PlugScanHandler;
	//delete m_IEFixScanHandler;
	delete m_sysoptHandler;
	delete m_SoftmgrHandle;
	delete m_KClearHandle;
	delete m_KTrojanHandle;
	delete m_pFreeVirusHandle;
	delete m_pWebShieldHandle;
}

LRESULT CBeikeSafeMainDlg::OnTimer( UINT_PTR uIdEvent )
{
	if( uIdEvent == TIMER_ID_MAIN_DLG_ONCE )
	{
		KillTimer( TIMER_ID_MAIN_DLG_ONCE );

		CString	str;
		BKSafeConfig::Get_MainDlg_LastCloseTab(str);
		//在这里延迟
		m_UpliveHandler.Init();

		CheckReportDesktopInfo();

		HWND hForegdWnd = ::GetForegroundWindow();
		DWORD dwCurID = ::GetCurrentThreadId();
		DWORD dwForeID = ::GetWindowThreadProcessId(hForegdWnd, NULL);
		::AttachThreadInput(dwCurID, dwForeID, TRUE);
		::BringWindowToTop(m_hWnd);
		::SetForegroundWindow(m_hWnd);  
		::AttachThreadInput(dwCurID, dwForeID, FALSE);
	}
	else if (uIdEvent == TIMER_ID_AUTORUN_HIT)
	{
		CSafeMonitorTrayShell tray;

		if (tray.KSafeServerIsOK())  /* 检查KSafeSvc服务是否启动 */
		{
			KillTimer(TIMER_ID_AUTORUN_HIT);

			if (!tray.GetAutorunTray()) /* 检查"开机启动"未是否设置 */
			{
				KwsSetting setting;

				if (setting.IsNeedAutorunHit())   /* 是否需要提示"开机启动" */
				{
					BOOL fDontHit;

					UINT uRet = CBkSafeMsgBox2::ShowMultLine(
						STARTUP_RUN_HIT, 
						L"金山卫士", 
						MB_OKCANCEL|MB_ICONWARNING, 
						&fDontHit);

					if (uRet == IDOK)
					{
						CRestoreRunner* pLogRunner = new CRestoreRunner;
						pLogRunner->DeleteItemFromLog(_T("KSafeTray"), KSRUN_TYPE_STARTUP, 590);
						delete pLogRunner;

						tray.SetAutorunTray(TRUE);     /* 保存开机启动设置 */
					}

					setting.SetNeedAutorunHit(!fDontHit);
				}
			}
		}
	}

	SetMsgHandled( FALSE );
	return TRUE;
}

void CBeikeSafeMainDlg::CheckReportDesktopInfo()
{
	int nReport = BKSafeConfig::GetReportDeskInfo();
	if( nReport == 0 )
	{
		BKSafeConfig::SetReportDeskInfo( 1 );

		HANDLE hThread = ::CreateThread(NULL, 0, ReportDeskInfoProc, this, 0, NULL);
		if( hThread )
			::CloseHandle(hThread);
	}
}

DWORD WINAPI CBeikeSafeMainDlg::ReportDeskInfoProc(LPVOID pvParam)
{
	CBeikeSafeMainDlg* pThis = (CBeikeSafeMainDlg*)pvParam;
	if( pThis )
		pThis->DoReportDeskInfo();

	return 0;
}

void CBeikeSafeMainDlg::DoReportDeskInfo()
{
	CDesktopInfo dskTop;
	DeskTypeCnt info;
	dskTop.CollectTypeInfo( info );



	DeskTypeCnt recentInfo;
	dskTop.CollectRecentInfo( recentInfo );
	
	DeskNoUseCnt noUseInfo;
	dskTop.CollectNoUseInfo( noUseInfo );
}

void CBeikeSafeMainDlg::OnBkLblTitleSetting()
{
	m_SettingCaller.DoModal();
}

void CBeikeSafeMainDlg::InitProxy( BOOL bForce )
{
	int nType = BKSafeConfig::GetProxyType();
	CStringA strHttpVer = "HTTP/1.1";
	switch( nType )
	{
	case 0:
		if( bForce )
		{
			INTERNET_PROXY_INFO infoProxy;
			infoProxy.dwAccessType = INTERNET_OPEN_TYPE_DIRECT;
			infoProxy.lpszProxy = (LPCTSTR)"";
			infoProxy.lpszProxyBypass = (LPCTSTR)"";
			InternetSetOptionA( NULL, INTERNET_OPTION_PROXY, (LPVOID)&infoProxy, sizeof(INTERNET_PROXY_INFO) );
// 			InternetSetOptionA(NULL,  INTERNET_OPTION_HTTP_VERSION, (LPVOID)strHttpVer.GetBuffer(-1), strHttpVer.GetLength());
// 			strHttpVer.ReleaseBuffer(-1);
		}
		break;
	case 2:
		break;
	case 1:
		{
			int nPort = BKSafeConfig::GetProxyPort();
			CString strHost = BKSafeConfig::GetProxyHost();
			if( nPort == 0 )
				nPort = 80;

			if( strHost.GetLength() == 0 )
				return;

			CStringA strPort;
			CW2A w2zHost(strHost);
			strPort.Format( "%d", nPort );
			CStringA strProxy( w2zHost );
			strProxy.Append( ":" );
			strProxy.Append( strPort );

			INTERNET_PROXY_INFO infoProxy;
			infoProxy.dwAccessType = INTERNET_OPEN_TYPE_PROXY;
			infoProxy.lpszProxy = (LPCTSTR)(LPCSTR)strProxy;
			infoProxy.lpszProxyBypass = (LPCTSTR)"local";
			BOOL bRet = InternetSetOptionA( NULL, INTERNET_OPTION_PROXY, (LPVOID)&infoProxy, sizeof(INTERNET_PROXY_INFO) );

// 			InternetSetOptionA(NULL,  INTERNET_OPTION_HTTP_VERSION, (LPVOID)strHttpVer.GetBuffer(-1), strHttpVer.GetLength());
// 			strHttpVer.ReleaseBuffer(-1);

			bRet = InternetSetOptionA( NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0 );
			ASSERT( bRet );			

			/* 此功能无效，目前ie控件不支持代理设置密码
			int nValidate = BKSafeConfig::GetProxyValidate();
			if( nValidate == 1 )
			{
				CString strUser = BKSafeConfig::GetProxyUser();
				CString strPassWord = BKSafeConfig::GetProxyPassWord();

				InternetSetOption( NULL, INTERNET_OPTION_PROXY_PASSWORD, 
									(LPVOID)(LPCTSTR)strUser, strUser.GetLength()*sizeof(TCHAR) );
				InternetSetOption( NULL, INTERNET_OPTION_PROXY_USERNAME, 
									(LPVOID)(LPCTSTR)strPassWord, strPassWord.GetLength()*sizeof(TCHAR) );
			}
			else
			{
				if( bForce )
				{
					InternetSetOption( NULL, INTERNET_OPTION_PROXY_PASSWORD, (LPVOID)TEXT(""), 0 );
					InternetSetOption( NULL, INTERNET_OPTION_PROXY_USERNAME, (LPVOID)TEXT(""), 0 );
				}
			}
			*/
		}
		break;
	default:
		break;
	}
}

void CBeikeSafeMainDlg::SetDefaultNavigate(LPCWSTR lpszNavigate)
{
	m_strDefaultNavigate = lpszNavigate;
}

LRESULT CBeikeSafeMainDlg::OnAppDefaultNavigate(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	_Module.Navigate(m_strDefaultNavigate);

	m_strDefaultNavigate.Empty();

	return 0;
}



LRESULT CBeikeSafeMainDlg::OnAppEchoFinish(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	SetMsgHandled(FALSE);

	BOOL bEchoOK = FALSE;
	switch (wParam)
	{
	case BkSafeExamItemLevel::Safe:
	case BkSafeExamItemLevel::Critical:
		bEchoOK = TRUE;
		break;
	}

	//(*m_VirusScanHandler).EchoFinished(bEchoOK, (BOOL)lParam);
	if (m_KTrojanHandle)
	{
		HWND hWnd = m_KTrojanHandle->GetTrojanHwnd();
		if (hWnd)
		{
			::PostMessage(hWnd, MSG_TROHAN_VIRS_ECHOFINISH, bEchoOK, lParam);
		}
	}

	return 0;
}

void CBeikeSafeMainDlg::InitVulFix()
{
	m_viewVulfix.Create(GetViewHWND(), NULL, WS_CHILD|WS_CLIPCHILDREN, 0, 3000);
	ATLVERIFY( m_viewVulfix.Load( IDR_BK_VULDLG_MAIN ) );
	m_viewVulfix.m_viewSoftVul.SetMainDlg( this );
	m_viewVulfix.Init(m_hWnd);
}



LRESULT CBeikeSafeMainDlg::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nIDCtl = wParam;
	LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;

    if ((lpmis == NULL) || (lpmis->CtlType != ODT_MENU))
        return FALSE;

    lpmis->itemWidth = 16;
    lpmis->itemHeight = 16;

	Gdiplus::Image *pImg = NULL;

	//if (lpmis->itemID == ID_SETTING_SETTING)
	//	pImg = BkPngPool::Get(IDP_PNG_MENU_SETTING);
	//else 
	if (lpmis->itemID == ID_SETTING_UPDATE)
		pImg = BkPngPool::Get(IDP_PNG_MENU_UPDATE);
	else if( lpmis->itemID == ID_SETTING_BBS )
		pImg = BkPngPool::Get(IDP_PNG_MENU_BBS);


    if (pImg)
    {
		lpmis->itemWidth = pImg->GetWidth();
		lpmis->itemHeight = pImg->GetHeight();
    }
	return TRUE;
}


LRESULT CBeikeSafeMainDlg::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nIDCtl = wParam;
	LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

    if ((lpdis == NULL) || (lpdis->CtlType != ODT_MENU))
        return FALSE; // not for a menu
   
    if (lpdis->rcItem.left != 2)
    {
        lpdis->rcItem.left -= (lpdis->rcItem.left - 4);
        lpdis->rcItem.right -= 60;

        if (lpdis->itemState & ODS_SELECTED)
        {
            lpdis->rcItem.left++;
            lpdis->rcItem.right++;
        }
    }

	Gdiplus::Image *pImg = NULL;

	//if (lpdis->itemID == ID_SETTING_SETTING)
	//	pImg = BkPngPool::Get(IDP_PNG_MENU_SETTING);
	//else 
	if (lpdis->itemID == ID_SETTING_UPDATE)
		pImg = BkPngPool::Get(IDP_PNG_MENU_UPDATE);
	else if( lpdis->itemID == ID_SETTING_BBS )
		pImg = BkPngPool::Get(IDP_PNG_MENU_BBS);

	if (pImg)
	{
		Gdiplus::Graphics graphics(lpdis->hDC);
		graphics.DrawImage(pImg, 
			lpdis->rcItem.left, lpdis->rcItem.top, 
			pImg->GetWidth(), pImg->GetHeight());
	}
	return TRUE;
}


BOOL CBeikeSafeMainDlg::OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
{
	CBkSafeProtectionNotifyWindow prot_shell(TRUE);
	prot_shell.ShellTray();

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

	_Module.SetActiveWindow(m_hWnd);

	InitProxy();

	InitVulFix();
    m_KClearHandle->PreLoad(); // 预加载

	Skylark::BkRenewProxySettingsFromReg();

	//(*m_VirusScanHandler).Init();
	TryToInitKTrojan();//hub trojan  加载其dll
	//(*m_PlugScanHandler).Init(); 
	(*m_pFreeVirusHandle).Init(); 
	// m_ProtectionHandler.Init();
	//m_UpliveHandler.Init(); //在timer 延迟加载
	(*m_ExamHandler).InitCtrl();
	(*m_SoftmgrHandle).Init(); 

	//iefixeng 初始化
	//(*m_IEFixScanHandler).InitCtrl();
    (*m_pWebShieldHandle).InitCtrl();
	(*m_sysoptHandler).Init();

    // KClear 初始化
    //(*m_KClearHandle).Init(); 切换到清理页面才加载

	if (m_SettingMenu.IsNull())  /* 设置菜单初始化 */
	{
		m_SettingMenu.LoadMenu(IDR_SETTING_MENU);

		MENUITEMINFO item = {0};
		item.cbSize = sizeof(MENUITEMINFO);
		item.fMask = MIIM_BITMAP|MIIM_STRING;
		item.hbmpItem = HBMMENU_CALLBACK;

 		//item.dwTypeData = _T("设 置");
 		//m_SettingMenu.SetMenuItemInfo(ID_SETTING_SETTING, FALSE, &item);

		item.dwTypeData = _T("升 级");
		m_SettingMenu.SetMenuItemInfo(ID_SETTING_UPDATE, FALSE, &item);

		item.dwTypeData = _T("论 坛");
		m_SettingMenu.SetMenuItemInfo(ID_SETTING_BBS, FALSE, &item);
	}

	if (m_strDefaultNavigate.IsEmpty())
		SetTabCurSel(IDC_TAB_MAIN, 0);
	else
		PostMessage(MSG_APP_DEFAULT_NAVIGATE);

	m_wndHtmlOnlineInfo.Create(GetViewHWND(), IDC_IE_ONLINE_INFO, FALSE, RGB(0xFB, 0xFC, 0xFD));
	// 	m_wndHtmlVirScan.Create(GetViewHWND(), IDC_IE_VIRSCAN, FALSE);

	// 	PostMessage(MSG_APP_DELAY_EXAM); // 不在同一个消息内进行创建ie和体检操作

	SetItemText(IDC_LBL_PRODUCT_VERSION, _Module.GetProductVersion());

	//  [10/27/2010 zhangbaoliang]
#if 0
	SetTabPageVisible(IDC_TAB_MAIN, 6, 0);
	SetTabPageVisible(IDC_TAB_MAIN, 7, 0);
	SetTabPageVisible(IDC_TAB_MAIN, 8, 0);
#endif
	_HideOemElement();

	//延迟加载
	SetTimer( TIMER_ID_MAIN_DLG_ONCE, 10, NULL );

	if (SW_HIDE == _Module.m_nCmdShow)
	{
		DontShowWindow();
	}

	/* 显示开机启动提示 */
	SetTimer(TIMER_ID_AUTORUN_HIT, 500, NULL);

	// 启动之后强制窗口置顶 [1/8/2011 zhangbaoliang]
	::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);

	::SetForegroundWindow(m_hWnd);
	::SetFocus(m_hWnd);

	CenterWindow();
	return TRUE;
}

void CBeikeSafeMainDlg::OnBkBtnClose()
{
	EndDialog(IDCANCEL);
}


LRESULT CBeikeSafeMainDlg::OnDestroy()
{
	KExeCtrlLoader	loaderMulti;
	loaderMulti.Init(KSM_SERVICE_MULTI_EXE_LOADER);
	loaderMulti.NotifyExecItemToServer(6);
	loaderMulti.NotifyExecItemToServer(4);
	loaderMulti.NotifyExecItemToServer(3);
	
	SetMsgHandled(FALSE);

	if( m_KClearHandle )
		m_KClearHandle->UnInit();
	if (m_KTrojanHandle)//hub trojan
		m_KTrojanHandle->UnInit();//hub trojan

	return 0;
}

void CBeikeSafeMainDlg::OnBkBtnMax()
{
	if (WS_MAXIMIZE == (GetStyle() & WS_MAXIMIZE))
	{
		SendMessage(WM_SYSCOMMAND, SC_RESTORE | HTCAPTION, 0);
		//         SetItemAttribute(IDC_BTN_SYS_MAX, "skin", "maxbtn");
	}
	else
	{
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE | HTCAPTION, 0);
		//         SetItemAttribute(IDC_BTN_SYS_MAX, "skin", "restorebtn");
	}
}

void CBeikeSafeMainDlg::OnBkBtnMin()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE | HTCAPTION, 0);
}

CBkNavigator* CBeikeSafeMainDlg::OnNavigate(CString &strChildName)
{
	CBkNavigator *pChild = NULL;

	/*if (!IsWindowEnabled())
		return NULL; */

	if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_EXAM))
	{//体检->0
		pChild = &(*m_ExamHandler);

		SetTabCurSel(IDC_TAB_MAIN, 0);
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_VIRSCAN))
	{//木马扫描->1
		//pChild = &(*m_VirusScanHandler);//hub trojan
		pChild = &(*m_KTrojanHandle);
		SetTabCurSel(IDC_TAB_MAIN, 1);
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_LEAKSCAN) || 0 == strChildName.CompareNoCase(BKSFNS_PAGE_LEAKSCAN2))
	{//漏洞扫描->2
		// add by zbl [11/4/2010]
		pChild = &m_viewVulfix.m_viewSoftVul;
		m_bVulfixRescanRequired = 0 == strChildName.CompareNoCase(BKSFNS_PAGE_LEAKSCAN2);
		SetTabCurSel(IDC_TAB_MAIN, 3);
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_FREEVIRUS))
	{
		SetTabCurSel(IDC_TAB_MAIN, 2);
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_PLUGINSCAN))
	{//插件
		pChild = NULL;
		_Module.Navigate(BKSFNS_MAKE_3(BKSFNS_UI, BKSFNS_PAGE_VIRSCAN, BKSFNS_VIRSCAN_PLUG));
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_IEFIX))
	{//系统修复-》6
		//系统修复-》6
		TryToInitIEFix();
		//pChild = &(*m_IEFixScanHandler);
		pChild = &(*m_pWebShieldHandle);
		SetTabCurSel(IDC_TAB_MAIN, 6);

		/*pChild = &(*m_KTrojanHandle);
		SetTabCurSel(IDC_TAB_MAIN, 1);
		m_KTrojanHandle->OnNavigate((const CString)L"SysFix");*/
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_IEFIX1))
	{
		pChild = &(*m_KTrojanHandle);
		SetTabCurSel(IDC_TAB_MAIN, 1);
		m_KTrojanHandle->OnNavigate((const CString)L"SysFix");
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_SYSTEM_OPTIMIZE))
	{//系统优化->4
		pChild = &(*m_sysoptHandler);

		SetTabCurSel(IDC_TAB_MAIN, 4);
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_SETTING))
	{//设置
		pChild = &m_SettingCaller;

		SetTabCurSel(IDC_TAB_MAIN, GetTabCurSel(IDC_TAB_MAIN));
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_CHANGE))
	{//
		SettingChanged();
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_AVE_INSTALL_FINISH))
	{
		AVEInstallFininshed();
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_ECHO_FAILED))
	{
		CWHRoundRectDialog<CBkSimpleDialog> dlg;
		dlg.Load(IDR_BK_ECHO_FAIL_DLG);
		dlg.DoModal();
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_SYSTEM_CLEAR) )
	{//清理
		TryToInitKClear();
		pChild = &(*m_KClearHandle);
		SetTabCurSel(IDC_TAB_MAIN, 5);	
	}
	else if ( 0 == strChildName.CompareNoCase(BKSFNS_SOFTMGR) )
	{
		pChild = &(*m_SoftmgrHandle);
		SetTabCurSel(IDC_TAB_MAIN, 7);
	}
	else if ( 0 == strChildName.CompareNoCase(BKSFNS_KWS_SETTING) )
	{
		
		TryToInitIEFix();
		//pChild = &(*m_IEFixScanHandler);
		pChild = &(*m_pWebShieldHandle);
		SetTabCurSel(IDC_TAB_MAIN, 6);
		//(*m_IEFixScanHandler).OnKwsShowSetting();
		(*m_pWebShieldHandle).OnKwsShowSetting();

	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_KWS_DLSUI) )
	{//网盾
		
		TryToInitIEFix();
		//pChild = &(*m_IEFixScanHandler);
		pChild = &(*m_pWebShieldHandle);
		SetTabCurSel(IDC_TAB_MAIN, 6);
		//(*m_IEFixScanHandler).OnKwsOpenDLSPage();
		(*m_pWebShieldHandle).OnKwsOpenDLSPage();
	}
	else
	{
		SetTabCurSel(IDC_TAB_MAIN, 0);
	}

	return pChild;
}

void CBeikeSafeMainDlg::EndDialog(UINT uRetCode)
{
	if ( !GetCloseCheckRet() )
	{
		return;
	}
	CloseSuccess();

	int nIndex = GetTabCurSel(IDC_TAB_MAIN);

	__super::EndDialog(uRetCode);
}

void CBeikeSafeMainDlg::TabMainSelChange( int nTabOldItem, int nTabNewItem )
{
	SetTabCurSel(IDC_TAB_MAIN, nTabNewItem);
	OnBkTabMainSelChange( nTabOldItem, nTabNewItem );
}

BOOL CBeikeSafeMainDlg::OnBkTabMainSelChange(int nTabItemIDOld, int nTabItemIDNew)
{

	DEBUG_TRACE(L"Tab Change %d, %d\r\n", nTabItemIDOld, nTabItemIDNew);

	BOOL bRet = FALSE;

	if (m_bFirstPageChange)
	{
		if (0 != nTabItemIDNew)
		{
			PostMessage(MSG_APP_DELAY_EXAM, FALSE);
		}

		m_bFirstPageChange = FALSE;
	}

	//
	// 内存调整策略
	// 当内存使用大于80MB时，将内存使用调整为40MB
	//
	PROCESS_MEMORY_COUNTERS pmc = {sizeof(pmc)};
	if(::GetProcessMemoryInfo(::GetCurrentProcess(), &pmc, sizeof(pmc)))
	{
		SIZE_T memSize = pmc.WorkingSetSize;
		if(memSize > 80*1024*1024)
		{
			SIZE_T min, max;
			if(::GetProcessWorkingSetSize(::GetCurrentProcess(), &min, &max))
			{
				static const SIZE_T adjustSize = 40*1024*1024;
				if(::SetProcessWorkingSetSize(::GetCurrentProcess(), min, adjustSize))
				{
					// 设置最大为200MB，防止内存完全被换出
					static const SIZE_T maxSize = 200*1024*1024;
					::SetProcessWorkingSetSize(::GetCurrentProcess(), min, maxSize);
				}
			}
		}
	}

	bRet = TRUE;
	switch (nTabItemIDNew)
	{
	case 0:
		if (m_bPage0NeverShowed)
		{
			PostMessage(MSG_APP_DELAY_NAVIGATE_IE);
			PostMessage(MSG_APP_DELAY_EXAM, TRUE);

			//             if (!(*m_ExamHandler).HasExamRunned() && BKSafeConfig::GetAutoExam())
			//                 (*m_ExamHandler).StartExam();
			// 			else if (!(*m_ExamHandler).HasExamRunned() && FALSE == BKSafeConfig::GetAutoExam())
			// 				(*m_ExamHandler).ShowDiyExamUI();

			//(*m_VirusScanHandler).UpdateAVEngineState();
			m_bPage0NeverShowed = FALSE;
			//(*m_VirusScanHandler).UpdateVirusInfo();	
			
		}

		break;

	case 1:
		if (m_bPage1NeverShowed)
		{
			CString	strURL = L"";
			if ( SUCCEEDED(CAppPath::Instance().GetLeidianDataPath(strURL)) )
				strURL.Append(_T("\\html\\error.html"));

			if (m_KTrojanHandle)
			{
				HWND hWnd = m_KTrojanHandle->GetTrojanHwnd();
				if (hWnd)
				{
					::PostMessage(hWnd, MSG_TROHAN_INIT, NULL, NULL);
				}
			}				
			m_bPage1NeverShowed = FALSE;
		}
		if (m_KTrojanHandle)
		{
			(*m_KTrojanHandle).Show(TRUE);//明显
		}		

		break;

	case 2:		
		{
			bRet = (*m_pFreeVirusHandle).FirstShow();
		}
		break;
	case 3:
		m_viewVulfix.ShowWindow(SW_SHOW);
		if(!m_bVulfixInited || m_bVulfixRescanRequired)
		{
			BOOL toRescan = TRUE;
			if(m_bVulfixRescanRequired && theEngine && theEngine->m_isRepairing)
			{
				toRescan = FALSE;
			}			
			m_bVulfixInited = TRUE;
			m_bVulfixRescanRequired = FALSE;
			if(toRescan)
				m_viewVulfix.m_viewSoftVul.InitEnv();
		}
		break;
	case  4://系统优化
		if (m_bPage5NeverShowed)
		{
			m_bPage5NeverShowed = FALSE;
			(*m_sysoptHandler).FirstShow();
		}
		else
		{
			(*m_sysoptHandler).SecondShow();
		}
		break;

	case 5://清理
		TryToInitKClear();
        (*m_KClearHandle).Show();
        break;

	case  6://网盾
		{
			TryToInitIEFix();
			//(*m_IEFixScanHandler).ReloadEngData();
			(*m_pWebShieldHandle).ReloadEngData();
			(*m_pWebShieldHandle).Show(TRUE);
		}
		break;
	case 7:		// 打开软件管理
		{
			DWORD nValue = CBkSafeTabChangeCtrl::Instance().Pop(TAB_SWITCH_SOFTMGR,TAB_SWITCH_SOFTMGR_FROM_CLICK);

			if (nValue == TAB_SWITCH_SOFTMGR_FROM_DESKTOP)
			{
				

			}

			if ( !(*m_SoftmgrHandle).m_bInit )
			{
				(*m_SoftmgrHandle).InitSlow();
			}
		}
		break;
	default:
		break;
	}

	if( nTabItemIDNew == 2 )
	{
		if( bRet )//打开毒霸的类型
			(*m_KTrojanHandle).Show(FALSE);//隐藏
	}
	else
	{
		if( nTabItemIDNew != 1 )
			(*m_KTrojanHandle).Show(FALSE);//隐藏
	}

	//软件管家不隐藏窗口
	if( nTabItemIDNew == 2 )
	{
		if( bRet )
			(*m_KClearHandle).Hide();
	}
	else
	{
		if( nTabItemIDNew != 5 )
			(*m_KClearHandle).Hide();
	}

	if( nTabItemIDNew == 2 )
	{
		if( bRet )//打开毒霸的类型
			(*m_pWebShieldHandle).Show(FALSE);//隐藏
	}
	else
	{
		if( nTabItemIDNew != 6 )
			(*m_pWebShieldHandle).Show(FALSE);//隐藏
	}
// 	if ( 6 != nTabItemIDNew )
// 	{
// 		(*m_pWebShieldHandle).Show(FALSE);
// 	}

	return bRet;
}

LRESULT CBeikeSafeMainDlg::OnAppShowPlugClean(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	if (m_bPage3NeverShowed)
	{
		m_bPage3NeverShowed = FALSE;
		/*(*m_PlugScanHandler).FirstShow();*/
		if (m_KTrojanHandle)
		{
			HWND hWnd = m_KTrojanHandle->GetTrojanHwnd();
			if (hWnd)
			{
				::PostMessage(hWnd, MSG_TROHAN_PLUG_SHOWCLEAN, NULL, NULL);
			}
		}
	}

	return TRUE;
}


void CBeikeSafeMainDlg::RemoveFromTodoList(int nItem)
{

//	m_LockRemoveTodoList1.Lock();

	if (_Module.Exam.RemoveItem(nItem))
		(*m_ExamHandler).RefreshTodoList();

//	m_LockRemoveTodoList1.Unlock();
}
void CBeikeSafeMainDlg::RemoveFromTodoList(REMOVE_EXAM_RESULT_ITEM RemoveExamItemInfo)
{

//	m_LockRemoveTodoList2.Lock();

	if (_Module.Exam.RemoveItem2(RemoveExamItemInfo))
		(*m_ExamHandler).RefreshTodoList();

//	m_LockRemoveTodoList2.Unlock();
}

void CBeikeSafeMainDlg::TrojanToMainChange(LPCWSTR strCommand)
{
	//_Module.Navigate(BKSFNS_MAKE_3(BKSFNS_UI, BKSFNS_SETTING, BKSFNS_SETTING_PAGE_VIRSCAN));//木马查杀设置页面调用
	_Module.Navigate(strCommand);
	return;
}

void CBeikeSafeMainDlg::VirusInstallFinish()
{
	(*m_ExamHandler).Echo();
}

int	CBeikeSafeMainDlg::SysCnvertExamind( int nId )
{
	int nIndex = nId;
	return _Module.Exam.ConvertKWSIDToExamID(nIndex);
}

void CBeikeSafeMainDlg::ExamRemove1( WPARAM wParam )
{
	int nItem = (int)wParam;
	RemoveFromTodoList(nItem);
	return;
}

void CBeikeSafeMainDlg::ExamRemove2( WPARAM wParam )
{
	REMOVE_EXAM_RESULT_ITEM* Item = (REMOVE_EXAM_RESULT_ITEM *)wParam;
	RemoveFromTodoList(*Item);
	return;
}

void CBeikeSafeMainDlg::OpenDuba(WPARAM wParam)
{
	SetTabCurSel(IDC_TAB_MAIN, 2);
}
void CBeikeSafeMainDlg::TryOpenShell(WPARAM wParam)
{
	CBkSafeProtectionNotifyWindow prot_shell(TRUE);
	prot_shell.TryOpenShell();
}
void CBeikeSafeMainDlg::ShowLocalTrojanVer(WPARAM wParam, LPARAM lParam)
{
	Skylark::BKAVE_SIGN_VERSION *signver =  (Skylark::BKAVE_SIGN_VERSION *)wParam;
	BOOL bShowAVESignVersion = (BOOL)lParam;
	if (signver == NULL)
	{
		return;
	}
	if ( 0 != (*signver).uYear &&
		0 != (*signver).uMonth)
		FormatItemText(IDC_LBL_AVE_SIGN_VERSION, BkString::Get(IDS_VIRSCAN_2568), (*signver).uYear, (*signver).uMonth, (*signver).uDay, (*signver).uBuild);

	SetItemVisible(IDC_LBL_AVE_SIGN_VERSION, bShowAVESignVersion);
	SetItemAttribute(IDC_DIV_APP_VERSION_INFO, "pos", bShowAVESignVersion ? "350,0,500,-0" : "170,0,320,-0");
	return;
}

BOOL CBeikeSafeMainDlg::PreTranslateMessage(MSG* pMsg)
{
	if (WM_KEYDOWN == pMsg->message && VK_RETURN == pMsg->wParam)
	{
		if (m_SoftmgrHandle)
			m_SoftmgrHandle->OnKeyDownReturn(pMsg);
	}

	return FALSE;
}

/*
*  弹出设置菜单 
*/
void CBeikeSafeMainDlg::OnBkBtnSet()
{
	if (m_SettingMenu)
	{
		RECT rectwin, rectbtn;
		GetWindowRect(&rectwin);

		if (GetItemRect(IDC_BTN_SYS_SET, rectbtn))
		{
			CMenuHandle  menu = m_SettingMenu.GetSubMenu(0);
			menu.TrackPopupMenu(TPM_TOPALIGN, 
				rectwin.left + rectbtn.left, 
				rectwin.top + rectbtn.bottom, m_hWnd);
		}
	}
}

void CBeikeSafeMainDlg::SetTodoText(int nOrder, CString& strNavigatorString, UINT uDivID, UINT uNoticeID, UINT uLinkID, UINT uImgID/* = 0*/)
{
	BOOL bRet = FALSE;
	BKSAFE_EXAM_RESULT_ITEM item;
	//     LPCSTR lpszTextClass = NULL;

	bRet = _Module.Exam.GetResultItem(nOrder, item);

	if (bRet)
	{
		int nStat = 2; // Safe

		switch (item.nLevel)
		{
		case BkSafeExamItemLevel::Critical:
			nStat = 1;
			//             lpszTextClass = "dangertext";
			break;
		case BkSafeExamItemLevel::Risk_0:
			nStat = 0;
			//             lpszTextClass = "risktext";
			break;
		case BkSafeExamItemLevel::RiskInfo:
			nStat = 0;
			//             lpszTextClass = "risktext";
			break;
		case BkSafeExamItemLevel::Info:
			nStat = 3;
			//             lpszTextClass = "infotext";
			break;
		}

		if (uImgID)
			SetItemIntAttribute(uImgID, "sub", nStat);
		SetItemText(uNoticeID, item.strItemNoticeText);
		SetItemText(uLinkID, item.strOperationText);
		//         SetItemAttribute(uDivID, "class", lpszTextClass, FALSE);

		SetItemVisible(uDivID, TRUE);

		strNavigatorString = item.strItemNavigateString;
	}
	else
	{
		SetItemVisible(uDivID, FALSE);

		strNavigatorString.Empty();
	}
}

void CBeikeSafeMainDlg::SettingChanged()
{
	/*(*m_VirusScanHandler).SettingChanged();*///向子窗口发送相关消息
	if (m_KTrojanHandle)
	{
		HWND hWnd = m_KTrojanHandle->GetTrojanHwnd();
		if (hWnd)
		{
			::PostMessage(hWnd, MSG_TROHAN_VIRS_SETTINGCHANGED, NULL, NULL);
		}
	}
	//更新体检界面
	(*m_ExamHandler).UpdateExamSetting();

	if (BKSafeConfig::GetAutoReportUnknown())
		RemoveFromTodoList(BkSafeExamItem::ConfigAutoReport);
}

void CBeikeSafeMainDlg::InstallAVEngine()
{
	//(*m_VirusScanHandler).InstallAVEngine();
//	(*m_ExamHandler).OnInstallAVEngine();
}

void CBeikeSafeMainDlg::AVEInstallFininshed()
{
	//(*m_VirusScanHandler).AVEInstallFininshed();
//	(*m_ExamHandler).AVEInstallFininshed();
}

void CBeikeSafeMainDlg::RescanVirusInLastScan()
{
	/*(*m_VirusScanHandler).RescanVirusInLastScan();*/
	if (m_KTrojanHandle)//改成发消息 trojan hub
	{
		HWND hWnd = m_KTrojanHandle->GetTrojanHwnd();
		if (hWnd)
		{
			::PostMessage(hWnd, MSG_TROHAN_VIRS_LASTSCAN, NULL, NULL);
		}
	}
}

void CBeikeSafeMainDlg::ModifyMonitorState(int nType)
{
	(*m_ExamHandler).ModifyMonitorState(nType);
	if( m_bIeFixInit )
	{
		//(*m_IEFixScanHandler).ModifyMonitorState(nType);
		(*m_pWebShieldHandle).ModifyMonitorState(nType);
	}
}

void CBeikeSafeMainDlg::ModifyBwsMonitorState()
{
	//(*m_ExamHandler).ModifyBwsMonitorState();
}

void CBeikeSafeMainDlg::UpdateDubaState()
{
	(*m_ExamHandler).UpdateDubaState();
}
void CBeikeSafeMainDlg::Echo()
{
	/*(*m_ExamHandler).Echo();*/
}


void CBeikeSafeMainDlg::OnSysCommand(UINT nID, CPoint point)
{
	SetMsgHandled(FALSE);

	switch (nID & 0xFFF0)
	{
	case SC_CLOSE:
		SetMsgHandled(TRUE);
		if( !g_bkMsgBox )		//已经弹出msgbox，不关闭窗口
			EndDialog(IDCANCEL);
		break;
	case SC_RESTORE:
		{
			DWORD dwStyle = GetStyle();
			//dwStyle = dwStyle & (WS_MAXIMIZE | WS_MINIMIZE);
			if (WS_MINIMIZE == (dwStyle & WS_MINIMIZE))
				break;

			if (WS_MAXIMIZE == (dwStyle & WS_MAXIMIZE))
			{
				SetItemAttribute(IDC_BTN_SYS_MAX, "skin", "maxbtn");
				break;
			}
		}
	case SC_MAXIMIZE:
		SetItemAttribute(IDC_BTN_SYS_MAX, "skin", "restorebtn");
		break;
	}
}

LRESULT CBeikeSafeMainDlg::OnAppDelayExam(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	(*m_ExamHandler).Init((BOOL)wParam);

	return 0;
}

LRESULT CBeikeSafeMainDlg::OnAppDelayNavigateIE(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	CString	strURL;
	if ( SUCCEEDED(CAppPath::Instance().GetLeidianDataPath(strURL)) )
		strURL.Append(_T("\\html\\error.html"));


	CString strUrlValue = L"http://www.ijinshan.com/safe/embed2.htm";
	if (KisPublic::Instance()->Init())
	{
		std::wstring strUrlCur;
		if (KisPublic::Instance()->KQueryOEMLPCWSTR(KIS::KOemKsfe::oemc_SafeLinkURL, strUrlCur))
			strUrlValue = strUrlCur.c_str();

	}

	m_wndHtmlOnlineInfo.Show2(strUrlValue, strURL);


	return 0;
}

void CBeikeSafeMainDlg::InitIECtrl()
{

}

void CBeikeSafeMainDlg::RefreshLastScanInfo(BOOL bScanned, DWORD dwScanMode, SYSTEMTIME stStartLast, SYSTEMTIME stStartFast, SYSTEMTIME stStartFull, SYSTEMTIME stStartCustom, DWORD dwVirusCount, DWORD dwRemainVirusCount)
{
//	(*m_ExamHandler).RefreshLastScanInfo(bScanned, dwScanMode, stStartLast, stStartFast, stStartFull, stStartCustom, dwVirusCount, dwRemainVirusCount);
}

void CBeikeSafeMainDlg::RefreshReportedUnknownInfo(int nCount)
{
//	(*m_ExamHandler).RefreshReportedUnknownInfo(nCount);
}

void CBeikeSafeMainDlg::ShowVirusLog()
{
	/*(*m_VirusScanHandler).ShowVirusLog();*/// trojan hub
	if (m_KTrojanHandle)
	{
		HWND hWnd = m_KTrojanHandle->GetTrojanHwnd();
		if (hWnd)
		{
			::PostMessage(hWnd, MSG_TROHAN_VIRS_SHOWLOG, NULL, NULL);
		}
	}
}

void CBeikeSafeMainDlg::ShowVirusTrustList()
{
	if (m_KTrojanHandle)
	{
		HWND hWnd = m_KTrojanHandle->GetTrojanHwnd();
		if (hWnd)
		{
			::PostMessage(hWnd, MSG_TROHAN_VIRS_SHOWTRUST_LIST, NULL, NULL);
		}
	}
}

void CBeikeSafeMainDlg::ShowReportedUnknownLog()
{
	/*(*m_VirusScanHandler).ShowReportedUnknownLog();*///trojan hub
	if (m_KTrojanHandle)
	{
		HWND hWnd = m_KTrojanHandle->GetTrojanHwnd();
		if (hWnd)
		{
			::PostMessage(hWnd, MSG_TROHAN_VIRS_UNKNOWNLOG, NULL, NULL);
		}
	}
}


void CBeikeSafeMainDlg::TryToInitIEFix()
{
	if( !m_bIeFixInit )
	{
		//(*m_IEFixScanHandler).InitCtrlDelay();
		(*m_pWebShieldHandle).InitCtrlDelay();
		m_bIeFixInit = TRUE;
	}
}

void CBeikeSafeMainDlg::TryToInitKClear()
{
	if( !m_bKClearInit )
	{
		(*m_KClearHandle).Init();
		m_bKClearInit = TRUE;
	}
}
void CBeikeSafeMainDlg::TryToInitKTrojan()//hub trojan
{
	if ( !m_bKTrojanInit )
	{
		(*m_KTrojanHandle).Init();
		m_bKTrojanInit = TRUE;
	}
	return;
}

CBeikeSafePluginUIHandler* CBeikeSafeMainDlg::GetPluginUIHandler()
{
	//return m_PlugScanHandler;  hub trojan
	return NULL;
}

LRESULT CBeikeSafeMainDlg::OnSettingMenuSetting(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
 	m_SettingCaller.DoModal();
 	return TRUE;
}

LRESULT CBeikeSafeMainDlg::OnSettingMenuUpdate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	/* 显示手动升级对话框 */
	
	CUpdateVersionDlg ManulUpdaterDlg;
	ManulUpdaterDlg.DoModal();
	return TRUE;
}

LRESULT CBeikeSafeMainDlg::OnSettingMenuBBS(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::ShellExecute(NULL, L"open", TEXT("http://bbs.ijinshan.com/?fr=client"), NULL, NULL, SW_SHOWNORMAL);
	return TRUE;
}

LRESULT CBeikeSafeMainDlg::OnAppProxyChange( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam )
{
	InitProxy( TRUE );
	return TRUE;

}

LRESULT CBeikeSafeMainDlg::OnShowUserDefineAdRule(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/* 显示用户定义广告拦载规则对话框 */

	CSetBlackPage dlg;
	dlg.DoModal();
	return TRUE;
}

void CBeikeSafeMainDlg::_HideOemElement( void )
{
	int nShowSoftManger = TRUE;			//是否显示软件管理
	int nShowMoreOptimize = TRUE;			//是否显示更多优化
	int nShowVirusFree = TRUE;				//是否显示免费杀毒tab
	int nShowNetMonter = TRUE;				//是否显示流量监控
	int nShowNetM = TRUE;				//是否显示网络测速

	if (KisPublic::Instance()->Init())
	{
		KisPublic::Instance()->KQueryOEMIntA(KIS::KOemKsfe::oemc_VirusFreeShow, nShowVirusFree);
		KisPublic::Instance()->KQueryOEMIntA(KIS::KOemKsfe::oemc_SoftwareMangerShow, nShowSoftManger);
		KisPublic::Instance()->KQueryOEMIntA(KIS::KOemKsfe::oemc_MoreOptimizeShow, nShowMoreOptimize);
		KisPublic::Instance()->KQueryOEMIntA(KIS::KOemKsfe::oemc_netmonterShow, nShowNetMonter);
		KisPublic::Instance()->KQueryOEMIntA(KIS::KOemKsfe::oemc_IsOpenNetM, nShowNetM);
	}

	if (!nShowSoftManger)
	{
		SetTabPageVisible(IDC_TAB_MAIN, 8, FALSE);
		SetItemVisible(IDC_HIDE_INSTALL_MACHINE, FALSE);
	}

	if (!nShowMoreOptimize)
		SetTabPageVisible(IDC_TAB_SYSOPT_LEFT, 3, FALSE);

	if (!nShowVirusFree)
	{
		SetTabPageVisible(IDC_TROJAN_TAB_CTRL, 1, FALSE);
		SetItemVisible(573, FALSE);
	}

	if (!nShowNetMonter)
		SetTabPageVisible(IDC_TAB_SYSOPT_LEFT, 1, FALSE);

	if (!nShowNetM)
		SetTabPageVisible(IDC_TAB_SYSOPT_LEFT, 2, FALSE); 

	//	SetItemVisible(102, FALSE);
	// 	SetTabPageVisible(IDC_TAB_MAIN, 0, FALSE);
	// 	SetTabPageVisible(IDC_TAB_MAIN, 1, FALSE);
	// 	SetTabPageVisible(IDC_TAB_MAIN, 2, FALSE);
	// 	SetTabPageVisible(IDC_TAB_MAIN, 3, FALSE);
}
//hub trojan
// LRESULT CBeikeSafeMainDlg::OnVirsSettingChange( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam )
// {
// 	_Module.Navigate(BKSFNS_MAKE_3(BKSFNS_UI, BKSFNS_SETTING, BKSFNS_SETTING_PAGE_VIRSCAN));//木马查杀设置页面调用
// 	return TRUE;
//}
LRESULT CBeikeSafeMainDlg::OnVirsLblDuba(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	SetTabCurSel(IDC_TAB_MAIN, 2);
	return TRUE;
}

LRESULT CBeikeSafeMainDlg::OnKwsSettingChange( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam )
{
	_Module.Navigate(BKSFNS_MAKE_3(BKSFNS_UI, BKSFNS_SETTING, BKSFNS_SETTING_PAGE_KWS));//网盾设置页面调用
	return TRUE;
}


LRESULT CBeikeSafeMainDlg::OnKwsFullScan( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam )
{
	SetTabCurSel(IDC_TAB_MAIN, 1);
	m_KTrojanHandle->OnNavigate((const CString)L"Full");
	return TRUE;
}


LRESULT CBeikeSafeMainDlg::OnKwsFastScan( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam )
{
	SetTabCurSel(IDC_TAB_MAIN, 1);
	m_KTrojanHandle->OnNavigate((const CString)L"Fast");
	return TRUE;
}
