#include "stdafx.h"
#include "beikesafemonitordlg.h"
#include <safemon/safemonitorctrlquerier.h>
#include "bksafe/bksafeconfig.h"
#include "beikesafemsgbox.h"
#include "beikesafemaindlg.h"
#include <safemon/safemonitor.h>
#include "kws\kactivate.h"
#include "communits/ThreadLangLocale.h"
#include "kws/urlmondef.h"
#include "kws/kwssettingold.h"
#include "kws/KwsCmptComm.h"
#include "beikesafearpdlg.h"
#include "downloadtoolsdlg.h"
#include "zip/zip.h"
#include "zip/unzip.h"
#include "kpfw/arpsetting_public.h"
#include "arpinstallcheck.h"
#include "svc/kbasicspwrapperclient.h"

#define KARP_INSTALLER_URL TEXT("http://dl.ijinshan.com/safe/Karp.zip")
#define KSAFE_REG_KNDISFLT TEXT("SYSTEM\\CurrentControlSet\\Services\\Kndisflt")

#define SAFE_CALL_FUN(x,y) if (NULL != (x)){(x)->y;}

#define STR_ALERT_OPEN_SAFEMONITOR BkString::Get(IDS_ALERT_OPEN_SAFEMONITOR_X64)

CBeiKeSafeMonitorDlg::CBeiKeSafeMonitorDlg(CBeikeSafeMainDlg* pNotifyWnd)
: m_bWarningTrunOff(FALSE)
, m_bShowBwspSetting(FALSE)
, m_hIconRegedit(NULL)
, m_wndProtectionNotify(TRUE)

{
	m_nNowTab = 0;
	m_pNotifyWnd = NULL;
	m_pNotifyWnd = pNotifyWnd;

	m_bRefreshExit = FALSE;
	m_hRefreshEvent = NULL;
	m_hRefreshThread = NULL;
	m_hIconRegedit = ::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_REGEDIT));
    m_pDownInstaller = NULL;
    m_hInstallArpFw = NULL;
    m_dlgWarning =  NULL;

}

CBeiKeSafeMonitorDlg::~CBeiKeSafeMonitorDlg(void)
{
	size_t nCount = m_arrIconTemp.GetCount();
    if (m_dlgWarning)
    {
        delete m_dlgWarning;
        m_dlgWarning = NULL;
    }

	for (size_t i = 0; i < nCount; i ++)
	{
		if (m_arrIconTemp[i])
			::DestroyIcon(m_arrIconTemp[i]);
	}

	::DestroyIcon(m_hIconRegedit);
}


void CBeiKeSafeMonitorDlg::Init()
{
	if (NULL == m_pNotifyWnd)
		return;

    if (IskArpInstalled() && (NULL == m_piArpSetting))
    {
        m_piArpSetting.LoadInterface(_T("arpsetting.dll"));
        BOOL bArpRun = QueryArpRegStatus(L"ArprunFlag");
        if (bArpRun && m_piArpSetting)
        {
            if (!QueryArpSpStatu())
            {
                StartArpfwSp();
            }

            //m_piArpSetting->EnableArpFw(TRUE);
            StartArpFirewallAsync();    // 异步启动ARP防火墙           
        }
        else
        {
            PostMessage(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_ARP, 0);
        }
    }

    else if (IskArpInstalled() && !CheckLocalFileExist(L"arpproxy.dll"))
    {
        PostMessage(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_ARP, 0);
    }

    if (_Module.Exam.IsWin64())
    {
        PostMessage(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_ARP, 0);
    }

	m_wndProtectionNotify.Create(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE);
	m_bWarningTrunOff = BKSafeConfig::Get_Protection_Trunoff_Warning();

	m_wndListMonitorCtrl.Create(m_pNotifyWnd->GetViewHWND(), IDC_LST_PROTECTION_CTRL);
	m_wndListMonitorCtrl.Load(IDR_BK_PROTECTION_CTRL_LIST_TEMPLATE);

	m_wndListMonitorLog.Create(m_pNotifyWnd->GetViewHWND(), IDC_LST_PROTECTION_CTRL_LOG);
	m_wndListMonitorLog.Load(IDR_BK_PROTECTION_LOG_LIST_TEMPLATE);

	if (IsKwsStdExist())
	{
		::PostMessage(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_KWS, 0);
		::PostMessage(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_KWS_SAFE_DOWN, 0);
	}


	m_hRefreshEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	m_hRefreshThread = (HANDLE)_beginthreadex(NULL, 0, RefreshThreadProc, this, NULL, 0);
}

int CBeiKeSafeMonitorDlg::CloseCheck()
{
	m_bRefreshExit = TRUE;

	if ( m_hRefreshEvent != NULL )
	{
		SetEvent(m_hRefreshEvent);
	}

	if ( m_hRefreshThread != NULL )
	{
		WaitForSingleObject(m_hRefreshThread, -1);

		CloseHandle(m_hRefreshThread);
		m_hRefreshThread = NULL;
	}

	if ( m_hRefreshEvent != NULL )
	{
		CloseHandle(m_hRefreshEvent);
		m_hRefreshEvent = NULL;
	}

	return 0;
}

LRESULT CBeiKeSafeMonitorDlg::OnAppProtectionSwitchChange(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	BOOL bOn = (BOOL)lParam;
	UINT uIDImgStatus = 0, uIDLblTurn = 0, uIDTxtTurn = 0;
	UINT uIDTxt = 0;

	// 	if (NULL == m_pNotifyWnd)
	// 		return 0;

	switch ( m_wndProtectionNotify.GetMonitorStatus() )
	{
	case 0:
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_OK, TRUE));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_WARNING1, FALSE));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_WARNING2, FALSE));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_WARNING3, FALSE));
		break;

	case 1:
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_OK, FALSE));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_WARNING1, TRUE));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_WARNING2, FALSE));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_WARNING3, FALSE));
		break;

	case 2:
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_OK, FALSE));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_WARNING1, FALSE));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_WARNING2, TRUE));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_WARNING3, FALSE));
		break;

	case 3:
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_OK, FALSE));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_WARNING1, FALSE));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_WARNING2, FALSE));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_WARNING3, TRUE));
		break;
	}

    switch (m_wndProtectionNotify._GetMonitorStatus())
    {
    case 0:
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_OK, TRUE));
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_WARNING1, FALSE));
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_WARNING2, FALSE));
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_WARNING3, FALSE));
        break;

    case 1:
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_OK, FALSE));
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_WARNING1, TRUE));
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_WARNING2, FALSE));
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_WARNING3, FALSE));
        break;

    case 2:
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_OK, FALSE));
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_WARNING1, FALSE));
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_WARNING2, TRUE));
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_WARNING3, FALSE));
        break;

    case 3:
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_OK, FALSE));
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_WARNING1, FALSE));
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_WARNING2, FALSE));
        SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_NETPROT_WARNING3, TRUE));
        break;

    }

	switch (wParam)
	{
	case SM_ID_INVAILD:
		SAFE_CALL_FUN(m_pNotifyWnd, RemoveFromTodoList(BkSafeExamItem::ConfigSystemMonitor));
		return 0;
		break;

	case SM_ID_RISK:
		uIDImgStatus    = IDC_IMG_PROTECTION_STAT_REGISTER;
		uIDLblTurn      = IDC_LBL_PROTECTION_TURN_REGISTER;
		uIDTxtTurn      = IDC_TXT_PROTECTION_TURN_REGISTER;
		uIDTxt			= IDC_TXT_PROTECTION_RISK1;
		break;

	case SM_ID_PROCESS:
		uIDImgStatus    = IDC_IMG_PROTECTION_STAT_PROCESS;
		uIDLblTurn    = IDC_LBL_PROTECTION_TURN_PROCESS;
		uIDTxtTurn    = IDC_TXT_PROTECTION_TURN_PROCESS;
		uIDTxt			= IDC_TXT_PROTECTION_PROCESS1;
		break;

	case SM_ID_UDISK:
		uIDImgStatus    = IDC_IMG_PROTECTION_STAT_UDISK;
		uIDLblTurn    = IDC_LBL_PROTECTION_TURN_UDISK;
		uIDTxtTurn   = IDC_TXT_PROTECTION_TURN_UDISK;
		uIDTxt			= IDC_TXT_PROTECTION_UDISK1;
		break;

	case SM_ID_LEAK:
		uIDImgStatus    = IDC_IMG_PROTECTION_STAT_LEAK;
		uIDLblTurn		= IDC_LBL_PROTECTION_TURN_LEAK;
		uIDTxtTurn		= IDC_TXT_PROTECTION_TURN_LEAK;
		uIDTxt			= IDC_TXT_PROTECTION_LEAK1;
		break;

	case SM_ID_KWS:
		{
			uIDImgStatus	= IDC_IMG_PROTECTION_STAT_KWS;
			uIDLblTurn		= IDC_LBL_PROTECTION_TURN_KWS;
			uIDTxtTurn		= IDC_TXT_PROTECTION_TURN_KWS;
			uIDTxt			= IDC_TXT_PROTECTION_KWS1;

			if (IsKwsStdExist())
			{
				kws_old_ipc::KwsSetting setting;
				bOn = setting.IsKwsEnable();
			}
		}
		break;

	case SM_ID_KWS_SAFE_DOWN:
		{
			uIDImgStatus	= IDC_IMG_PROTECTION_STAT_KWS_SAFEDOWN;
			uIDLblTurn		= IDC_LBL_PROTECTION_TURN_KWS_SAFEDOWN;
			uIDTxtTurn		= IDC_TXT_PROTECTION_TURN_KWS_SAFEDOWN;
			uIDTxt			= IDC_TXT_PROTECTION_KWS_SAFEDOWN1;

			if (IsKwsStdExist())
			{
				kws_old_ipc::KwsSetting setting;
				bOn = setting.IsAutoScanEnable();
			}
		}
		break;

    case SM_ID_ARP:
        {          
            if (IskArpInstalled() && \
                CheckLocalFileExist(L"arpproxy.dll") && \
                !CheckLocalFileExist(L"ArpUninstall.log"))
            {            
                    uIDImgStatus = IDC_IMG_PROTECTION_STAT_ARP;
                    uIDLblTurn   = IDC_LBL_PROTECTION_TURN_ARP;
                    uIDTxtTurn   = IDC_TXT_PROTECTION_TURN_ARP;
                    uIDTxt       = IDC_TXT_PROTECTION_ARP1;

                    SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_INSTALL, TRUE));
                    SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_UNINSTALL, FALSE));
                    SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_INSTALL_RESTART, FALSE));
                    SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_INSTALL_KWSEXIST, FALSE));
                
            }

            else  if (IskArpInstalled())
            {
                if ( CheckArpSysReg() && \
                    (!CheckLocalFileExist(L"arpproxy.dll") || !QueryArpRegStatus(L"ArprunFlag"))  && \
                    !CheckLocalFileExist(L"ArpUninstall.log")) /*网盾防火墙存在*/
                {
                    SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_INSTALL, FALSE));
                    SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_UNINSTALL, FALSE));
                    SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_INSTALL_RESTART, FALSE));
                    SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_INSTALL_KWSEXIST, TRUE));
                    return 0;
                }
                else if (!CheckArpSysReg() || \
                    CheckLocalFileExist(L"ArpUninstall.log"))/*ARP防火墙已经卸载但未重启*/
                {          
                    SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_INSTALL, FALSE));
                    SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_UNINSTALL, FALSE));
                    SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_INSTALL_RESTART, TRUE));
                    SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_INSTALL_KWSEXIST, FALSE));
                    return 0;
                }         
            }

            else  
            {
                if (_Module.Exam.IsWin64())
                {
                   SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_UNINSTALL, FALSE));
                }
                else
                   SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_UNINSTALL, TRUE));

                SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_INSTALL, FALSE));
                SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_INSTALL_RESTART, FALSE));
                SAFE_CALL_FUN(m_pNotifyWnd, SetItemVisible(IDC_DIV_PROTECTION_ARP_INSTALL_KWSEXIST, FALSE));
                return 0;

            }

        
        }
        break;
	}

	if (bOn)
	{
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemIntAttribute(uIDImgStatus, "sub", 0));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemColorAttribute(uIDTxt, "crtext", RGB(0, 0, 0)));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemColorAttribute(uIDTxt + 1, "crtext", RGB(0, 0, 0)));

		SAFE_CALL_FUN(m_pNotifyWnd, SetItemText(uIDLblTurn, _T("关闭")));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemText(uIDTxtTurn, _T("已开启")));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemColorAttribute(uIDTxtTurn, "crtext", RGB(0x00, 0x99, 0x00)));
	}
	else
	{
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemIntAttribute(uIDImgStatus, "sub", 1));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemColorAttribute(uIDTxt, "crtext", RGB(128, 128, 128)));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemColorAttribute(uIDTxt + 1, "crtext", RGB(128, 128, 128)));

		SAFE_CALL_FUN(m_pNotifyWnd, SetItemText(uIDLblTurn, _T("开启")));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemText(uIDTxtTurn, _T("已关闭")));
		SAFE_CALL_FUN(m_pNotifyWnd, SetItemColorAttribute(uIDTxtTurn, "crtext", RGB(0xff, 0x00, 0x00)));
	}

	SAFE_CALL_FUN(m_pNotifyWnd, ModifyMonitorState( -1 ));

	return 0;
}

void CBeiKeSafeMonitorDlg::OnLblProtectionTurnLeak()
{
	TurnSwitch(SM_ID_LEAK);
}

void CBeiKeSafeMonitorDlg::OnLblProtectionTurnRegister()
{
	TurnSwitch(SM_ID_RISK);
}

void CBeiKeSafeMonitorDlg::OnLblProtectionTurnProcess()
{
	TurnSwitch(SM_ID_PROCESS);
}

void CBeiKeSafeMonitorDlg::OnLblProtectionTurnUDisk()
{
	TurnSwitch(SM_ID_UDISK);
}

void CBeiKeSafeMonitorDlg::OnLblProtectionTurnKws()
{
	TurnSwitch(SM_ID_KWS);
}

void CBeiKeSafeMonitorDlg::OnLblProtectionTurnKwsSafeDown()
{
	TurnSwitch(SM_ID_KWS_SAFE_DOWN);
}


void CBeiKeSafeMonitorDlg::OnLblProtectionArpSetting()
{
    HRESULT hRetCode;

    BOOL bArpsp = QueryArpSpStatu();
    if (!bArpsp) 
        StartArpfwSp();

    if (NULL == m_piArpSetting)
        return;

    hRetCode = m_piArpSetting->ShowSetting(m_pNotifyWnd->m_hWnd);
    return;
}

void CBeiKeSafeMonitorDlg::OnLblProtectionArpStart()
{

    if (IskArpInstalled() && CheckArpSysReg()) /*如果安装了网盾防火墙*/
    {
        PostMessage(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_ARP, 0);
        return;     
    }

    UINT _nId = CBkSafeMsgBox2::ShowMultLine( BkString::Get(IDS_ARPSETTING_TURNON), 
                                                NULL, 
                                                MB_OKCANCEL | MB_ICONWARNING, 
                                                NULL);
    if (IDOK == _nId)
    {
        UINT uId = IDCANCEL;
        HRESULT hr = S_FALSE;

        WCHAR szModuleFile[MAX_PATH] = { 0 };
        GetModuleFileName(NULL, szModuleFile, sizeof(szModuleFile));
        PathRemoveFileSpec(szModuleFile);
        PathAppend(szModuleFile, L"karp.zip");

        SAFE_CALL_FUN(m_pNotifyWnd, EnableItem(IDC_LBL_PROTECTION_TURN_ARP_START, FALSE));

        if( !m_pDownInstaller )
        {
            m_pDownInstaller = new CToolsDownloadDlg( KARP_INSTALLER_URL, szModuleFile, NULL, TEXT("金山卫士ARP防火墙") );
        }

        if( m_pDownInstaller )
        {
            uId = m_pDownInstaller->DoModal();

            delete m_pDownInstaller;
            m_pDownInstaller = NULL;

            if (uId != IDOK)
                return;
        }

        UnZipArpPackage();

        m_hInstallArpFw = (HANDLE)_beginthreadex(NULL, 0, InstallArpFwThreadProc, (LPVOID)this, 0, NULL);

        m_dlgWarning = new CARPInstallWarning();
        if (m_dlgWarning)
        {
            m_dlgWarning->DoModal();
        }
         return;       
    }

}

void CBeiKeSafeMonitorDlg::UnZipArpPackage()
{
    WCHAR szModuleFile[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szModuleFile, sizeof(szModuleFile));
    PathRemoveFileSpec(szModuleFile);
    PathAppend(szModuleFile, L"karp.zip");

    if (PathFileExists(szModuleFile))
    {
        CString strTmp = szModuleFile;
        CString strCurdir = strTmp.Left(strTmp.ReverseFind(L'\\') + 1);

        HZIP hz;
        ZIPENTRY ze;
        hz = OpenZip(szModuleFile, 0 ); 
        GetZipItem(hz, -1, &ze);
        int nItems = ze.index;

        for (int i = 0; i < nItems; ++i)
        {
            GetZipItem(hz, i, &ze);        
            CString strOldCruDir = strCurdir;
            strOldCruDir+= ze.name;       
            UnzipItem(hz, i, strOldCruDir);
        }

        CloseZip(hz);
    }

}


unsigned __stdcall CBeiKeSafeMonitorDlg::InstallArpFwThreadProc(LPVOID pvParam)
{
    HRESULT hr = S_FALSE;
    CBeiKeSafeMonitorDlg* pThis = (CBeiKeSafeMonitorDlg*)pvParam;
    hr = pThis->InstallArpFw();

    if (S_FALSE == hr)
    {
        SAFE_CALL_FUN(pThis->m_pNotifyWnd, EnableItem(IDC_LBL_PROTECTION_TURN_ARP_START, TRUE));
    }

    if (pThis->m_hInstallArpFw)
    {
        CloseHandle(pThis->m_hInstallArpFw);
        pThis->m_hInstallArpFw = NULL;
    }

    if (pThis->m_dlgWarning && pThis->m_dlgWarning->IsWindowVisible())
    {
        pThis->m_dlgWarning->EndDialog(0);
    }
 
    return 0;
}


HRESULT CBeiKeSafeMonitorDlg::InstallArpFw()
{
    HRESULT hr = S_FALSE;

    if (NULL == m_piArpSetting)
    {
         m_piArpSetting.LoadInterface(_T("arpsetting.dll"));
    }

    if (NULL !=  m_piArpSetting)
    {                    
        hr = m_piArpSetting->InstallArpFw();            
        if (S_OK == hr)   /*成功安装完成之后启动SP*/
        {
           StartArpfwSp(); 
           PostMessage(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_ARP, 1);
           m_piArpSetting->EnableArpFw(TRUE);
           WriteArprunFlag(L"ArprunFlag", 1);
        }         
    }   

    return hr;
}

BOOL CBeiKeSafeMonitorDlg::StartArpfwSp()
{
    BOOL bRet = FALSE;

	KBasicSPWrapperClient spClient;
    KXE_NULL_PARAEMETER null;
    EM_SP_ID spId = enum_ARP_SP;
    EM_SP_STATUS Status = em_SP_Unknown;
    int nStep = 0 ;

    do 
    {
        spClient.QuerySPStatus( spId, Status );

        if( Status != em_SP_Ready )
        {
            spClient.StartSP(enum_ARP_SP, null);
            Sleep( 50 );
        }

        nStep ++;
        spClient.QuerySPStatus( spId, Status );

    } while( Status != em_SP_Ready && nStep < 30 );

    if (em_SP_Ready == Status)
    {
        bRet = TRUE;
    }

    return bRet;
}

BOOL CBeiKeSafeMonitorDlg::QueryArpSpStatu()
{
    KBasicSPWrapperClient spClient;
    KXE_NULL_PARAEMETER null;
    EM_SP_ID spId = enum_ARP_SP;
    EM_SP_STATUS Status = em_SP_Unknown;

    spClient.QuerySPStatus(spId, Status);

    return (em_SP_Ready == Status);

}

void CBeiKeSafeMonitorDlg::OnLblProtectionTurnArp()
{
    BOOL bEnable = FALSE;
    BOOL bArpsp = QueryArpSpStatu();
    if (!bArpsp)
    {
        UINT nId = CBkSafeMsgBox2::ShowMultLine( BkString::Get(IDS_ARPSETTING_TURNON), 
                                                NULL, 
                                                MB_OKCANCEL | MB_ICONWARNING, 
                                                NULL);
        if (IDOK == nId)
        {
            StartArpfwSp();
                  
            if (m_piArpSetting && QueryArpSpStatu())
            {
                m_piArpSetting->EnableArpFw(TRUE);
                PostMessage(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_ARP, 1);
                WriteArprunFlag(L"ArprunFlag", 1);
            }                
           
            else
            {
                PostMessage(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_ARP, 0);
                WriteArprunFlag(L"ArprunFlag", 0);
            }          
        }
    }

    else 
    {
        if (m_piArpSetting)
        {
            m_piArpSetting->IsArpFwEnable(&bEnable);

            if (!bEnable)
            {
                UINT nId = CBkSafeMsgBox2::ShowMultLine( BkString::Get(IDS_ARPSETTING_TURNON), 
                                                        NULL, 
                                                        MB_OKCANCEL | MB_ICONWARNING, 
                                                        NULL);
                if (IDOK == nId)
                    TurnSwitch(SM_ID_ARP);  
            }

            else
            {
                CBkSafeMsgBox2 dlg;
                dlg.AddButton(TEXT("暂停 "), IDOK);
                dlg.AddButton(TEXT("彻底关闭"), IDCANCEL);

                UINT _nId = dlg.ShowMutlLineMsg( BkString::Get(IDS_ARP_TURNOFF), 
                                                    NULL, 
                                                    MB_BK_CUSTOM_BUTTON|MB_ICONINFORMATION, 
                                                    NULL);
                if (IDCANCEL == _nId)
                {
                    m_piArpSetting->UnInstallArpFw();
                    CreateArpUninstllog(L"ArpUninstall.log"); 
                    WriteArprunFlag(L"ArprunFlag", 0);
                    PostMessage(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_ARP, 0);
                    CBkSafeMsgBox2::ShowMultLine(L"ARP防火墙已停止，将在您下次重新启动电脑后彻底关闭",
                                                  NULL,
                                                  MB_OK,
                                                  NULL);
                    return;
                }

                TurnSwitch(SM_ID_ARP); 
            }
        }
                     
    }

}

void CBeiKeSafeMonitorDlg::OnLblProtectionArpHistory()
{   
    WCHAR szModule[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szModule, sizeof(szModule));
    PathRemoveFileSpec(szModule);
    PathAppend(szModule, L"AppData\\antiarp.log");
    ::ShellExecute(NULL, _T("open"), szModule, NULL, NULL, SW_SHOWNORMAL);
}

void CBeiKeSafeMonitorDlg::OnLblUrlMonTurn()
{
	KUrlMonCfgReader cfgUrlMon;

	cfgUrlMon.Init();

	if (cfgUrlMon.GetEnable())
	{
		LPCWSTR lpName = BkString::Get(IDS_MONITER_NAME_URL_MON);

		ATL::CString strText;
		strText.Format(BkString::Get(IDS_MONITOR_TURN_OFF_NOTICE_FORMAT), lpName);
		UINT nID = CBkSafeMsgBox2::ShowMultLine(strText, NULL, MB_OKCANCEL | MB_ICONWARNING, NULL);
		if ( nID == IDOK )
		{
			cfgUrlMon.SetEnable(!cfgUrlMon.GetEnable());
			_LoadUrlMonCfg(cfgUrlMon);
		}
	}
	else
	{
		cfgUrlMon.SetEnable(!cfgUrlMon.GetEnable());
		_LoadUrlMonCfg(cfgUrlMon);
	}

	cfgUrlMon.Uninit();
}

void CBeiKeSafeMonitorDlg::OnLblUrlMonLog()
{

}

void CBeiKeSafeMonitorDlg::OnLblProtectionLogShow()
{
	CString strFileName, strLog;

	CAppPath::Instance().GetLeidianLogPath(strFileName);

	strFileName += L"\\ksfmon.log";

	::ShellExecute(NULL, L"open", L"notepad.exe", strFileName, NULL, SW_SHOWNORMAL);
}

void CBeiKeSafeMonitorDlg::OnLblProtectionScan()
{
	//SAFE_CALL_FUN(m_pNotifyWnd, PostMessage(MSG_APP_FULL_SCAN_TROJAN, 0, 0))

	SAFE_CALL_FUN(m_pNotifyWnd, Navigate(BKSFNS_MAKE(BKSFNS_PAGE_VIRSCAN, BKSFNS_VIRSCAN_FAST)));
//		EndDialog(0);
}

void CBeiKeSafeMonitorDlg::OnLblNetProtectionScan()
{
    SAFE_CALL_FUN(m_pNotifyWnd, Navigate(BKSFNS_MAKE(BKSFNS_PAGE_VIRSCAN, BKSFNS_VIRSCAN_FAST)));
}

void CBeiKeSafeMonitorDlg::OnLblProtectionTrunOnAll()
{
	m_wndProtectionNotify.TurnOnAll();
}

void CBeiKeSafeMonitorDlg::OnLblProtectionSetting()
{
	m_wndProtectionNotify.OnStartupRun();
}

void CBeiKeSafeMonitorDlg::OnLblNetProtectionTrunOnAll()
{
    m_wndProtectionNotify._TurnOnAll();
}

void CBeiKeSafeMonitorDlg::OnLblNetProtectionSetting()
{
    m_wndProtectionNotify.OnStartupRun();
}

void CBeiKeSafeMonitorDlg::TurnSwitch(DWORD dwMonitorId)
{
	BOOL bOn;
	BOOL bKws = IsKwsStdExist();
	kws_old_ipc::KwsSetting setting;

	if (dwMonitorId == SM_ID_KWS && bKws)
	{
		/* 如果存在独立版网盾则获取网盾总开关设置 */
		bOn = setting.IsKwsEnable();
	}
	else if (dwMonitorId == SM_ID_KWS_SAFE_DOWN && bKws)
	{
		bOn = setting.IsAutoScanEnable();
	}
    else if(SM_ID_ARP == dwMonitorId)
    {
        if (m_piArpSetting)
        {
             m_piArpSetting->IsArpFwEnable(&bOn);
        }
       
    }
	else
	{
		bOn = m_wndProtectionNotify.GetTurnStatus(dwMonitorId);
	}

	if ( bOn )
	{
		if ( m_bWarningTrunOff )
		{
			BOOL bRember = FALSE;
			CString strText;
			LPCTSTR lpName = NULL;

			switch ( dwMonitorId )
			{
			case SM_ID_LEAK:
				lpName = BkString::Get(IDS_MONITOR_NAME_LEAK);
				break;

			case SM_ID_RISK:
				lpName = BkString::Get(IDS_MONITOR_NAME_RISK);
				break;

			case SM_ID_PROCESS:
				lpName = BkString::Get(IDS_MONITOR_NAME_PROCESS);
				break;

			case SM_ID_UDISK:
				lpName = BkString::Get(IDS_MONITOR_NAME_UDISK);
				break;

			case SM_ID_KWS:
				strText = _T("关闭“网页病毒木马过滤”，金山卫士将无法拦截挂马网页，\n影响您的上网安全。\n\r\n您确定要关闭吗？");
				break;

			case SM_ID_KWS_SAFE_DOWN:
				strText = _T("关闭“下载保护”，将无法防止在网上下载文件时感染木马病毒，\n下载文件将不安全。\n\r\n您确定要关闭吗？");
				break;
            case  SM_ID_ARP:
                strText = _T("关闭ARP防火墙，将无法有效拦截局域网中的ARP攻击，\n\r\n你确实要关闭吗？");
                break;

			default:
				assert(FALSE);
			}

			if ( strText.GetLength() == 0 )
			{
				strText.Format(BkString::Get(IDS_MONITOR_TURN_OFF_NOTICE_FORMAT), lpName);
			}

			UINT nID = CBkSafeMsgBox2::ShowMultLine(strText, NULL, MB_OKCANCEL | MB_ICONWARNING, &bRember);
			if ( nID == IDOK )
			{
				if ( bRember )
				{
					m_bWarningTrunOff = FALSE;
					BKSafeConfig::Set_Protection_Trunoff_Warning(m_bWarningTrunOff);
				}
			}
			else
				return;
		}

		if (dwMonitorId == SM_ID_KWS && bKws)
		{
			setting.SetKwsEnable(!bOn);
			OnAppProtectionSwitchChange(MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_KWS, 0);
			return;
		}
		else if (dwMonitorId == SM_ID_KWS_SAFE_DOWN && bKws)
		{
			setting.SetAutoScanEnable(!bOn);
			OnAppProtectionSwitchChange(MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_KWS_SAFE_DOWN, 0);
			return;
		}
        else if(dwMonitorId == SM_ID_ARP)
        {
            if (m_piArpSetting && IskArpInstalled())
            {
                m_piArpSetting->IsArpFwEnable(&bOn);
                m_piArpSetting->EnableArpFw(!bOn);    
                WriteArprunFlag(L"ArprunFlag", !bOn);
            }
             return;
        }

		m_wndProtectionNotify.TurnSwitch(dwMonitorId);
	}
	else
	{
		if (dwMonitorId == SM_ID_KWS && bKws)
		{
			setting.SetKwsEnable(!bOn);
			OnAppProtectionSwitchChange(MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_KWS, 0);
			return;
		}
		else if (dwMonitorId == SM_ID_KWS_SAFE_DOWN && bKws)
		{
			setting.SetAutoScanEnable(!bOn);
			OnAppProtectionSwitchChange(MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_KWS_SAFE_DOWN, 0);
			return;
		}

        else if(dwMonitorId == SM_ID_ARP)
        {
            if (m_piArpSetting && IskArpInstalled())
            {
                m_piArpSetting->IsArpFwEnable(&bOn);
                m_piArpSetting->EnableArpFw(!bOn);
                WriteArprunFlag(L"ArprunFlag", !bOn);
            }
            return;
        }

		m_wndProtectionNotify.TurnSwitch(dwMonitorId);
	}
}
UINT CBeiKeSafeMonitorDlg::ShowMsg(LPCTSTR pszCaption)
{
	CBkSafeMsgBox dlg;
	return dlg.ShowMsg(pszCaption, NULL, MB_OK|MB_ICONINFORMATION, NULL, m_pNotifyWnd->GetViewHWND());
}

BOOL CBeiKeSafeMonitorDlg::OnBkTabMainSelChange(int nTabItemIDOld, int nTabItemIDNew)
{
	m_nNowTab = nTabItemIDNew;
	switch (nTabItemIDNew)
	{
	case enum_SafeMonitorTab_Protection:
    case enum_safeMonitorTab_NetProt:
	case enum_SafeMonitorTab_Trust:
	case enum_SafeMonitorTab_Log:
	// case enum_SafeMonitorTab_UrlMon:
		if ( m_hRefreshEvent != NULL )
		{
			SetEvent(m_hRefreshEvent);
		}
		break;
	}

	return TRUE;
}

LRESULT CBeiKeSafeMonitorDlg::OnBkLvmGetDispInfoProtectionCtrl(LPNMHDR pnmh)
{
	LPBKLVMGETDISPINFO pnms = (LPBKLVMGETDISPINFO)pnmh;

	pnms->nListItemID;

	ATLTRACE(L"GetDispInfo(%d, %d, %d)\r\n", pnms->nListItemID, pnms->bHover, pnms->bSelect);

	if (m_arrProtectionCtrl[pnms->nListItemID].hIcon)
	{
		m_wndListMonitorCtrl.SetItemIconHandle(IDC_ICO_PROTCTRL_LISTITEM_ICON, m_arrProtectionCtrl[pnms->nListItemID].hIcon);
	}
	else
	{
		m_wndListMonitorCtrl.SetItemIconHandle(IDC_ICO_PROTCTRL_LISTITEM_ICON, m_hIconRegedit);
	}

	m_wndListMonitorCtrl.SetItemCheck(IDC_CHK_PROTCTRL_LISTITEM, m_arrProtectionCtrl[pnms->nListItemID].bCheck);
	m_wndListMonitorCtrl.SetItemColorAttribute(IDC_DIV_PROTCTRL_LISTITEM_BG, "crbg", pnms->bSelect ? RGB(0xCC, 0xCC, 0xFF) : (pnms->bHover ? RGB(0xCC, 0xFF, 0xCC) : RGB(0xFF, 0xFF, 0xFF)));

	m_wndListMonitorCtrl.SetItemVisible(IDC_DIV_PROTCTRL_LISTITEM_TRUST, !m_arrProtectionCtrl[pnms->nListItemID].bBlock);
	m_wndListMonitorCtrl.SetItemVisible(IDC_DIV_PROTCTRL_LISTITEM_DENY, m_arrProtectionCtrl[pnms->nListItemID].bBlock);
	m_wndListMonitorCtrl.SetItemText(IDC_LBL_PROTCTRL_LISTITEM_FILENAME, m_arrProtectionCtrl[pnms->nListItemID].strPath);
	m_wndListMonitorCtrl.SetItemStringAttribute(IDC_LBL_PROTCTRL_LISTITEM_FILENAME, "tip", m_arrProtectionCtrl[pnms->nListItemID].strPath);
	m_wndListMonitorCtrl.SetItemText(IDC_LBL_PROTCTRL_LISTITEM_DETAIL, m_arrProtectionCtrl[pnms->nListItemID].strDetail);
	m_wndListMonitorCtrl.SetItemStringAttribute(IDC_LBL_PROTCTRL_LISTITEM_DETAIL, "tip", m_arrProtectionCtrl[pnms->nListItemID].strDetail);

	return 0;
}

class _DeleteProtCtrlListParam
{
public:
	CBeiKeSafeMonitorDlg* _This;
	CAtlList<int> lstDelete;
};

LRESULT CBeiKeSafeMonitorDlg::OnBkLvmItemClickProtectionCtrl(LPNMHDR pnmh)
{
	LPBKLVMITEMCLICK pnms = (LPBKLVMITEMCLICK)pnmh;

	pnms->nListItemID;

	CString strMsg;

	switch (pnms->uCmdID)
	{
	case IDC_BTN_PROTCTRL_LISTITEM_CANCEL_TRUST:
	case IDC_BTN_PROTCTRL_LISTITEM_CANCEL_DENY:
		{
			_DeleteProtCtrlListParam *pParam = new _DeleteProtCtrlListParam;
			pParam->_This = this;
			pParam->lstDelete.AddTail(pnms->nListItemID);

			HANDLE hThread = ::CreateThread(NULL, 0, _DeleteProtectionCtrlListThreadProc, pParam, 0, NULL);
			::CloseHandle(hThread);
		}
		break;
	case IDC_CHK_PROTCTRL_LISTITEM:
		m_arrProtectionCtrl[pnms->nListItemID].bCheck = m_wndListMonitorCtrl.GetItemCheck(IDC_CHK_PROTCTRL_LISTITEM);
		break;
	default:
		return 0;
	}

	return 0;
}



HICON _GetFileIconHandle2(LPCTSTR lpFilePath)
{
	SHFILEINFO sfi = { 0 };

	DWORD_PTR dwRet = SHGetFileInfo(lpFilePath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);

	return SUCCEEDED(dwRet) ? sfi.hIcon: NULL;
}

void CBeiKeSafeMonitorDlg::_LoadProtectionCtrl()
{
	CSafeMonitorCtrlQuerierLoader ctrlLoader;
	ISafeMonitorCtrlQuerier *pMonitorCtrlQuerier = NULL;

	CAtlArray<_ProtectionCtrlArrayItem> arrData;

	pMonitorCtrlQuerier = ctrlLoader.NewSafeMonitorCtrlQuerier();
	if (pMonitorCtrlQuerier)
	{
		DWORD dwIndex = 0;
		SM_CTRL_ITEM itemret = {sizeof(SM_CTRL_ITEM)};
		_ProtectionCtrlArrayItem item;

		pMonitorCtrlQuerier->Init();

		while (pMonitorCtrlQuerier->Enum(dwIndex, &itemret))
		{
			item.bCheck = FALSE;
			item.bBlock = IS_SM_CTRL_BLOCK(itemret.dwCtrl);
			item.strPath = itemret.lpFilePath ? itemret.lpFilePath : itemret.lpPath;
			item.strDetail = itemret.lpVirusName ? itemret.lpVirusName : L"";
			item.hIcon = itemret.lpFilePath ? _GetFileIconHandle2(itemret.lpFilePath) : NULL;

			int i = 0;

			while (TRUE)
			{
				WCHAR ch = item.strPath[i];
				if (L'\\' == ch || L'\0' == ch)
					break;

				if (ch >= L'a' && ch <= L'z')
					item.strPath.SetAt(i, ch - L'a' + L'A');

				i ++;
			}

			arrData.Add(item);

			item.hIcon = NULL;

			dwIndex ++;
		}

		pMonitorCtrlQuerier->Uninit();
		ctrlLoader.FreeSafeMonitorCtrlQuerier(&pMonitorCtrlQuerier);
	}

	if ( m_pNotifyWnd != NULL && ::IsWindow(m_pNotifyWnd->m_hWnd))
		::SendMessage(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_CTRL_LOAD_FINISH, (WPARAM)&arrData, 0);
}

void CBeiKeSafeMonitorDlg::_LoadProtectionLog()
{
	USE_CHINESE_LANG_LOCAL;

	CAtlFile hMonitorLog;
	CString strFileName, strLog;
	CAtlArray<_ProtectionCtrlLogArrayItem> arrLog;

	CSafeMonitorTrayShell tray;

	DWORD dwCount = 0;

	CAppPath::Instance().GetLeidianLogPath(strFileName);

	strFileName += L"\\ksfmon.log";

	HRESULT hRet = hMonitorLog.Create(strFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING);
	if (SUCCEEDED(hRet))
	{
		ULONGLONG ullRemainSize = 0;
		hMonitorLog.GetSize(ullRemainSize);

		CStringA strRead, strLine, strLastLineRemain;

		int nLineNum = 0;

		while (0 != ullRemainSize)
		{
			DWORD dwRead = min(100, (DWORD)ullRemainSize);

			hRet = hMonitorLog.Read(strRead.GetBuffer(dwRead + 1), dwRead);
			if (FAILED(hRet))
			{
				strRead.ReleaseBuffer(0);
				break;
			}

			strRead.ReleaseBuffer(dwRead);

			int nPosNextLine = -1, nPosThisLine = 0;
			DWORD dwCount = 0;

			while (TRUE)
			{
				nPosNextLine = strRead.Find('\n', nPosThisLine);

				if (-1 == nPosNextLine)
				{
					strLastLineRemain += strRead.Mid(nPosThisLine);
					break;
				}

				strLine = strLastLineRemain + strRead.Mid(nPosThisLine, nPosNextLine - nPosThisLine);
				strLastLineRemain.Empty();

				nLineNum ++;

				if (1 == nLineNum)
				{
					sscanf(strLine, "block_count=%d", &dwCount);
					goto _TagNextLine;
				}
				else if (2 == nLineNum)
					goto _TagNextLine;

				_AppendLogToArray(strLine, arrLog);


_TagNextLine:

				nPosThisLine = nPosNextLine + 1;
			}

			ullRemainSize -= dwRead;
		}

		if (0 == ullRemainSize)
		{
			_AppendLogToArray(strLastLineRemain, arrLog);
		}
	}

	if ( m_pNotifyWnd != NULL && ::IsWindow(m_pNotifyWnd->m_hWnd))
		::SendMessage(m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_LOG_LOAD_FINISH, (WPARAM)&arrLog, 0);
}

#define TEXT_SAFE_COLOR						RGB(0,128,0)//listview中安全文字的颜色
#define TEXT_DANGER_COLOR					RGB(200,0,0)//listview中危险文字的颜色

void CBeiKeSafeMonitorDlg::_LoadUrlMonCfg(KUrlMonCfgReader& cfgUrlMon)
{
	BOOL bEnable = cfgUrlMon.GetEnable();
	/*DWORD dwBlockCount = cfgUrlMon.GetBlockUrlCnt();
	DWORD dwScanCount = cfgUrlMon.GetModuleScanCnt();
	ATL::CTime time(cfgUrlMon.GetLastBlockTime());
	ATL::CString strTime;
	strTime.Format(TEXT("%d-%d-%d %d:%d:%d"), time.GetYear(), time.GetMonth(), time.GetDay(),
		time.GetHour(), time.GetMinute(), time.GetSecond());

	__int64 nTime = cfgUrlMon.GetLastBlockTime();
	

	TCHAR szValue[100];

	_itot(dwBlockCount, szValue, 10);
	m_pNotifyWnd->SetItemText(IDC_TXT_URLMON_BLOCK_COUNT, szValue);

	_itot(dwScanCount, szValue, 10);
	m_pNotifyWnd->SetItemText(IDC_TXT_URLMON_SCAN_COUNT, szValue);
	m_pNotifyWnd->SetItemText(IDC_TXT_URLMON_LAST_TIME, strTime);*/

	if ( bEnable )
	{
		m_pNotifyWnd->SetItemText(IDC_TXT_URLMON_TURN, _T("已开启"));
		m_pNotifyWnd->SetItemColorAttribute(IDC_TXT_URLMON_TURN, "crtext", RGB(0x00, 0x99, 0x00));

		m_pNotifyWnd->SetItemText(IDC_LBL_URLMON_TURN, _T("关闭"));

		m_pNotifyWnd->SetItemVisible(IDC_DIV_URLMON_OK, TRUE);
		m_pNotifyWnd->SetItemVisible(IDC_DIV_URLMON_OFF, FALSE);
	}
	else
	{
		m_pNotifyWnd->SetItemText(IDC_TXT_URLMON_TURN, _T("已关闭"));
		m_pNotifyWnd->SetItemColorAttribute(IDC_TXT_URLMON_TURN, "crtext", RGB(0xff, 0x00, 0x00));
		
		m_pNotifyWnd->SetItemText(IDC_LBL_URLMON_TURN, _T("开启"));

		m_pNotifyWnd->SetItemVisible(IDC_DIV_URLMON_OK, FALSE);
		m_pNotifyWnd->SetItemVisible(IDC_DIV_URLMON_OFF, TRUE);
	}
}

void CBeiKeSafeMonitorDlg::_RefreshThreadProc()
{
	HWND hWndNotify = m_pNotifyWnd->m_hWnd;

	CString strCtrlPath, strLogPath;
	WIN32_FIND_DATA wfdCtrlFile, wfdLogFile;

	memset(&wfdCtrlFile, -1, sizeof (wfdCtrlFile));
	memset(&wfdLogFile, -1, sizeof (wfdLogFile));

	CAppPath::Instance().GetLeidianLogPath(strLogPath);
	strLogPath += _T("\\ksfmon.log");

	CAppPath::Instance().GetLeidianLogPath(strCtrlPath);
	strCtrlPath += _T("\\bkmc.dat");

	KUrlMonCfgReader cfgUrlMon;
	cfgUrlMon.Init();

	while ( !m_bRefreshExit )
	{
		WaitForSingleObject(m_hRefreshEvent, 2000);
		if ( m_bRefreshExit )
		{
			break;
		}

		if ( m_nNowTab == enum_SafeMonitorTab_Protection )
		{
			if (IsKwsStdExist())
			{
				PostMessage(hWndNotify, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_KWS, 0);
				PostMessage(hWndNotify, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_KWS_SAFE_DOWN, 0);
			}			
		}
		else if ( m_nNowTab == enum_SafeMonitorTab_Trust )
		{
			WIN32_FIND_DATA wfdCtrlFileNew = { 0 };
			GetFileAttributesEx(strCtrlPath, GetFileExInfoStandard, &wfdCtrlFileNew);
			if ( wfdCtrlFileNew.ftLastWriteTime.dwHighDateTime != wfdCtrlFile.ftLastWriteTime.dwHighDateTime ||
				wfdCtrlFileNew.ftLastWriteTime.dwLowDateTime != wfdCtrlFile.ftLastWriteTime.dwLowDateTime
				)
			{
				memcpy(&wfdCtrlFile, &wfdCtrlFileNew, sizeof (wfdCtrlFileNew));
				_LoadProtectionCtrl();
			}
		}
		else if ( m_nNowTab == enum_SafeMonitorTab_Log )
		{
			WIN32_FIND_DATA wfdLogFileNew = { 0 };
			GetFileAttributesEx(strLogPath, GetFileExInfoStandard, &wfdLogFileNew);
			if ( wfdLogFileNew.ftLastWriteTime.dwHighDateTime != wfdLogFile.ftLastWriteTime.dwHighDateTime ||
				wfdLogFileNew.ftLastWriteTime.dwLowDateTime != wfdLogFile.ftLastWriteTime.dwLowDateTime
				)
			{
				memcpy(&wfdLogFile, &wfdLogFileNew, sizeof (wfdLogFileNew));
				_LoadProtectionLog();
			}
		}
		/*
		else if ( m_nNowTab == enum_SafeMonitorTab_UrlMon )
		{
			_LoadUrlMonCfg(cfgUrlMon);
		}*/
	}
	cfgUrlMon.Uninit();
}

unsigned __stdcall CBeiKeSafeMonitorDlg::RefreshThreadProc(LPVOID pvParam)
{
	CBeiKeSafeMonitorDlg *pThis = (CBeiKeSafeMonitorDlg *)pvParam;

	pThis->_RefreshThreadProc();

	_endthreadex(0);
	return 0;
}

LRESULT CBeiKeSafeMonitorDlg::OnAppProtectionCtrlLoadFinish(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	CAtlArray<_ProtectionCtrlArrayItem> *parrData = (CAtlArray<_ProtectionCtrlArrayItem> *)wParam;

	m_arrProtectionCtrl.Copy(*parrData);

	size_t nCount = m_arrIconTemp.GetCount();

	for (size_t i = 0; i < nCount; i ++)
	{
		if (m_arrIconTemp[i])
			::DestroyIcon(m_arrIconTemp[i]);
	}

	nCount = m_arrProtectionCtrl.GetCount();

	m_arrIconTemp.SetCount(nCount);

	for (size_t i = 0; i < nCount; i ++)
	{
		m_arrIconTemp[i] = m_arrProtectionCtrl[i].hIcon;
	}

	m_wndListMonitorCtrl.SetItemCount(nCount);

	m_pNotifyWnd->SetItemVisible(IDC_LST_PROTECTION_CTRL, TRUE);
	m_pNotifyWnd->FormatItemText(IDC_LBL_PROTECTION_CTRL_COUNT, BkString::Get(IDS_PROTECTION_CTRL_COUNT_FORMAT), m_arrProtectionCtrl.GetCount());

	m_pNotifyWnd->EnableItem(IDC_TXT_PROTECTION_CTRL_CHECK, nCount != 0);
	m_pNotifyWnd->EnableItem(IDC_BTN_PROTECTION_CTRL_REMOVE_CHECK, nCount != 0);

	return 0;
}

DWORD WINAPI CBeiKeSafeMonitorDlg::_DeleteProtectionCtrlListThreadProc(LPVOID pvParam)
{
	_DeleteProtCtrlListParam *pParam = (_DeleteProtCtrlListParam *)pvParam;

	POSITION pos = pParam->lstDelete.GetHeadPosition();

	CSafeMonitorCtrlQuerierLoader ctrlLoader;
	ISafeMonitorCtrlQuerier *pMonitorCtrlQuerier = NULL;

	pMonitorCtrlQuerier = ctrlLoader.NewSafeMonitorCtrlQuerier();
	if (pMonitorCtrlQuerier)
	{
		pMonitorCtrlQuerier->Init();

		while (pos)
		{
			int nItemID = pParam->lstDelete.GetNext(pos);

			pMonitorCtrlQuerier->Delete((DWORD)nItemID);
		}

		pMonitorCtrlQuerier->Save();

		pMonitorCtrlQuerier->Uninit();
		ctrlLoader.FreeSafeMonitorCtrlQuerier(&pMonitorCtrlQuerier);

		CSafeMonitorTrayShell::NotifySafeMonitorCtrlUpdated();
	}

	pParam->_This->_LoadProtectionCtrl();

	delete pParam;

	return 0;
}

void CBeiKeSafeMonitorDlg::OnLblProtectionCtrlCheckAll()
{
	for (int i = m_arrProtectionCtrl.GetCount() - 1; i >= 0 ; i --)
	{
		if (!m_arrProtectionCtrl[i].bCheck)
		{
			m_arrProtectionCtrl[i].bCheck = TRUE;
			m_wndListMonitorCtrl.RedrawItem(i);
		}
	}
}

void CBeiKeSafeMonitorDlg::OnLblProtectionCtrlUncheckAll()
{
	for (int i = m_arrProtectionCtrl.GetCount() - 1; i >= 0 ; i --)
	{
		if (m_arrProtectionCtrl[i].bCheck)
		{
			m_arrProtectionCtrl[i].bCheck = FALSE;
			m_wndListMonitorCtrl.RedrawItem(i);
		}
	}
}

void CBeiKeSafeMonitorDlg::OnLblProtectionCtrlRemoveCheck()
{
	_DeleteProtCtrlListParam *pParam = new _DeleteProtCtrlListParam;
	pParam->_This = this;

	for (int i = m_arrProtectionCtrl.GetCount() - 1; i >= 0 ; i --)
	{
		if (m_arrProtectionCtrl[i].bCheck)
			pParam->lstDelete.AddTail(i);
	}

	if ( pParam->lstDelete.GetCount() < 1)
	{
		CBkSafeMsgBox2::ShowMultLine(BkString::Get(IDS_SP_ASK_CLEAN_LOG_94), NULL, MB_OK | MB_ICONINFORMATION, NULL, m_pNotifyWnd->m_hWnd);
		return;
	}


	HANDLE hThread = ::CreateThread(NULL, 0, _DeleteProtectionCtrlListThreadProc, pParam, 0, NULL);
	::CloseHandle(hThread);
}

void CBeiKeSafeMonitorDlg::_AppendLogToArray(
	LPCSTR szLine,
	CAtlArray<_ProtectionCtrlLogArrayItem>& arrLog
	)
{
	CString strLineW = CA2W(szLine);
	_ProtectionCtrlLogArrayItem item;

	int nTypeSize = strLineW.Find(L']', 1) - 1;
	if (nTypeSize < 0)
		return;

	int nOpSize = strLineW.Find(L']', nTypeSize + 24) - nTypeSize - 24;
	if (nOpSize < 0)
		return;

	item.strType = strLineW.Mid(1, nTypeSize);
	item.strTime = strLineW.Mid(nTypeSize + 3, 16);
	item.strOp = strLineW.Mid(nTypeSize + 24, nOpSize);

	int nFileNameSize = strLineW.Find(L']', nTypeSize + nOpSize + 26) - nTypeSize - nOpSize - 26;
	if (nFileNameSize < 0)
		return;

	item.strFileName = strLineW.Mid(nTypeSize + nOpSize + 26, nFileNameSize);

	int nDetailSize = strLineW.Find(L']', nTypeSize + nOpSize + nFileNameSize + 28) - nTypeSize - nOpSize - nFileNameSize - 28;
	if (nDetailSize < 0)
		return;

	item.strDetail = strLineW.Mid(nTypeSize + nOpSize + nFileNameSize + 28, nDetailSize);

	//arrLog.Add(item);
	arrLog.InsertAt(0, item);
}


LRESULT CBeiKeSafeMonitorDlg::OnAppProtectionLogLoadFinish(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	CAtlArray<_ProtectionCtrlLogArrayItem> *parrData = (CAtlArray<_ProtectionCtrlLogArrayItem> *)wParam;

	m_arrProtectionCtrlLog.Copy(*parrData);

	m_wndListMonitorLog.SetItemCount(m_arrProtectionCtrlLog.GetCount());

	m_pNotifyWnd->SetItemVisible(IDC_DIV_PROTECTION_CTRL_LOG, TRUE);

	m_pNotifyWnd->EnableItem(IDC_BTN_PROTECTION_CTRL_LOG_CLEAR, FALSE);
	if (m_arrProtectionCtrlLog.GetCount() > 0)
		m_pNotifyWnd->EnableItem(IDC_BTN_PROTECTION_CTRL_LOG_CLEAR, TRUE);

	return 0;
}

LRESULT CBeiKeSafeMonitorDlg::OnBkLvmGetDispInfoProtectionLog(LPNMHDR pnmh)
{
	LPBKLVMGETDISPINFO pnms = (LPBKLVMGETDISPINFO)pnmh;

	pnms->nListItemID;

	m_wndListMonitorLog.SetItemText(IDC_LBL_PROTLOG_LISTITEM_TIME, m_arrProtectionCtrlLog[pnms->nListItemID].strTime);
	m_wndListMonitorLog.SetItemText(IDC_LBL_PROTLOG_LISTITEM_TYPE, m_arrProtectionCtrlLog[pnms->nListItemID].strType);
	m_wndListMonitorLog.SetItemText(IDC_LBL_PROTLOG_LISTITEM_OP, m_arrProtectionCtrlLog[pnms->nListItemID].strOp);
	m_wndListMonitorLog.SetItemText(IDC_LBL_PROTLOG_LISTITEM_FILENAME, m_arrProtectionCtrlLog[pnms->nListItemID].strFileName);
	m_wndListMonitorLog.SetItemStringAttribute(IDC_LBL_PROTLOG_LISTITEM_FILENAME, "tip", m_arrProtectionCtrlLog[pnms->nListItemID].strFileName);
	m_wndListMonitorLog.SetItemText(IDC_LBL_PROTLOG_LISTITEM_DETAIL, m_arrProtectionCtrlLog[pnms->nListItemID].strDetail);
	m_wndListMonitorLog.SetItemStringAttribute(IDC_LBL_PROTLOG_LISTITEM_DETAIL, "tip", m_arrProtectionCtrlLog[pnms->nListItemID].strDetail);

	return 0;
}

LRESULT CBeiKeSafeMonitorDlg::OnBkLvmItemClickProtectionLog(LPNMHDR pnmh)
{
	LPBKLVMITEMCLICK pnms = (LPBKLVMITEMCLICK)pnmh;

	return 0;
}

void CBeiKeSafeMonitorDlg::OnLblProtectionCtrlLogClear()
{
	
	if (IDOK != CBkSafeMsgBox2::ShowMultLine(BkString::Get(IDS_SP_ASK_CLEAN_LOG_92), NULL, MB_OKCANCEL | MB_ICONINFORMATION, NULL, m_pNotifyWnd->m_hWnd))
		return;

	HANDLE hThread = ::CreateThread(NULL, 0, _ClearProtectionLogThreadProc, this, 0, NULL);
	::CloseHandle(hThread);
}

DWORD WINAPI CBeiKeSafeMonitorDlg::_ClearProtectionLogThreadProc(LPVOID pvParam)
{
	CBeiKeSafeMonitorDlg* _This = (CBeiKeSafeMonitorDlg*)pvParam;
	CString strFileName;

	CAppPath::Instance().GetLeidianLogPath(strFileName);

	strFileName += L"\\ksfmon.log";

	for (int i = 0; i < 3; i ++)
	{
		if (::DeleteFile(strFileName))
			break;

		::Sleep(100);
	}

	_This->_LoadProtectionLog();

	return 0;
}

void CBeiKeSafeMonitorDlg::OnLblProtectionBwspMoreSetting()
{
	KActivate Activate;
	Activate.ActivateKWS(_T("/enter ksafe"), TRUE, 3);

	//m_bShowBwspSetting = TRUE;
	//EndDialog(IDOK);
}


CBkNavigator* CBeiKeSafeMonitorDlg::OnNavigate( CString &strChildName )
{
	if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_PROTECTION_COMMON))
	{
		m_pNotifyWnd->SetTabCurSel(IDC_SAFEMONITOR_TAB_2001, 0);
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_PROTECTION_BANDT))
	{
		m_pNotifyWnd->SetTabCurSel(IDC_SAFEMONITOR_TAB_2001, 2);
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_PROTECTION_HISTORY))
	{
		m_pNotifyWnd->SetTabCurSel(IDC_SAFEMONITOR_TAB_2001, 3);
	}


	return NULL;
}

UINT CBeiKeSafeMonitorDlg::ShowPanelMsg(LPCTSTR pszCaption)
{
	CBkSafeMsgBox2 dlg;
/*
	CString strXml;
	int nWidth = 0;
	nWidth = _tcslen(pszCaption) * 12;
	strXml.Format(L"<text pos=\"0,10\">%s</text>", pszCaption);
	CRect rc(0, 0, nWidth, 0);
	return dlg.ShowPanelMsg(strXml, &rc, NULL, MB_OK | MB_ICONINFORMATION, NULL, m_pNotifyWnd->m_hWnd);
*/
	return dlg.ShowMutlLineMsg(pszCaption, NULL, MB_OK | MB_ICONINFORMATION, NULL, m_pNotifyWnd->m_hWnd);
}

BOOL CBeiKeSafeMonitorDlg::QueryArpRegStatus(LPCWSTR lpValuename)
{
    HKEY hKey = NULL;
    DWORD dwType = REG_SZ;
    DWORD dwsize = sizeof(dwsize);
    LONG lRet = 0;
    DWORD dwValue;

    lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                            L"SOFTWARE\\KSafe\\ARP", 
                            0, 
                            KEY_QUERY_VALUE, 
                            &hKey );

    if (ERROR_SUCCESS != lRet)
    {
        goto Exit0;
    }

    lRet = ::RegQueryValueEx( hKey, 
                           lpValuename, 
                            0, 
                            &dwType, 
                            (BYTE*)&dwValue, 
                            &dwsize);

    ::RegCloseKey(hKey);
    if (ERROR_SUCCESS != lRet)
    {           
        goto Exit0;
    }

    return (1 == dwValue) ? TRUE : FALSE;

Exit0:
    return FALSE;

}

BOOL   CBeiKeSafeMonitorDlg:: WriteArprunFlag(LPCWSTR lpvaluename,DWORD dwRegvalue)
{
    HKEY hKey = NULL;
    HKEY hTempkey = NULL;
    LONG lRet = 0;

    if (NULL == lpvaluename)
        return FALSE;

    lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                            L"SOFTWARE\\KSafe", 
                            0, 
                            KEY_QUERY_VALUE, 
                            &hKey);

    if (ERROR_SUCCESS == lRet)
    {
        lRet = ::RegCreateKey(hKey,  L"ARP", &hTempkey);
        if (ERROR_SUCCESS == lRet)
        {
            lRet = ::RegSetValueEx( hTempkey, 
                                   lpvaluename, 
                                    0, 
                                    REG_DWORD, 
                                    (BYTE*)&dwRegvalue, 
                                    sizeof(dwRegvalue));                                  
        }
    }

    if (hKey)
    {
        ::RegCloseKey(hKey);
    }
    if (hTempkey)
    {
        RegCloseKey(hTempkey);
    }

    return lRet == 0; 
}

BOOL CBeiKeSafeMonitorDlg:: CheckLocalFileExist(LPCWSTR lpFilename)
{
    BOOL bExist  = FALSE;
    WCHAR szModule[MAX_PATH] = { 0 };
    if (NULL == lpFilename)
        return FALSE;

    ::GetModuleFileName(NULL, szModule, MAX_PATH);
    ::PathRemoveFileSpec(szModule);
    PathAppend(szModule, lpFilename);

    bExist = ::PathFileExists(szModule);

    return bExist;
}

BOOL CBeiKeSafeMonitorDlg::CheckArpSysReg()
{
    CRegKey regkey;
    DWORD   dwRet;
    BOOL     bExist = FALSE;
    dwRet = regkey.Open(HKEY_LOCAL_MACHINE, KSAFE_REG_KNDISFLT, KEY_READ);

    bExist = (dwRet == ERROR_SUCCESS);

    regkey.Close();

    return bExist;

}


BOOL CBeiKeSafeMonitorDlg::CreateArpUninstllog(LPCWSTR lpFilename)
{
    HANDLE hFile = NULL;
    WCHAR szModule[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szModule, MAX_PATH);
    ::PathRemoveFileSpec(szModule);
    PathAppend(szModule, lpFilename);

    hFile = CreateFile(szModule, 
        GENERIC_READ | GENERIC_WRITE, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, 
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (INVALID_HANDLE_VALUE == hFile)
        return FALSE;

    CloseHandle(hFile);
    MoveFileEx(szModule, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

    return TRUE;  
}

BOOL CBeiKeSafeMonitorDlg::StartArpFirewallAsync()
{
    HANDLE hThread = (HANDLE)_beginthreadex(
        NULL, 0, StartArpFirewallThreadProc, this, 0, NULL);
    if (hThread)
    {
        CloseHandle(hThread);
        hThread = NULL;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

UINT CBeiKeSafeMonitorDlg::StartArpFirewallThreadProc(void* pParam)
{
    CBeiKeSafeMonitorDlg* pThis = (CBeiKeSafeMonitorDlg*)pParam;
    if (pThis)
    {
        if (pThis->m_piArpSetting)
        {
            pThis->m_piArpSetting->EnableArpFw(TRUE);
            PostMessage(pThis->m_pNotifyWnd->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE, SM_ID_ARP, 1);
        }
    }

    _endthreadex(0);
    return 0;
}
