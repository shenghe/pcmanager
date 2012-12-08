#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "beikesafesettingdlg.h"
#include "beikesafequarantinedlg.h"
#include "beikesafevirscanlogdlg.h"
#include "_idl_gen/bksafesvc.h"
#include <bksafe/bksafeconfig.h>
#include <safemon/safetrayshell.h>
#include <comproxy/bkutility.h>
#include "vulfix/BeikeUtils.h"
#include <shellapi.h>
#include "vulfix/BeikeVulfixEngine.h"
#include "beikesafevirscanwhitelistdlg.h"
#include "kpfw/msg_logger.h"
#include "kpfw/kpfw_def.h"
#include <iefix/cinifile.h>
#include "common/kopermemfile.h"
#include "runoptimize/restorerunitem.h"  /* for CRestoreRunner */
#include "kws/urlmondef.h"
#include "kpfw/netfluxdef.h"
#include "miniutil/bkprocprivilege.h"


using namespace IniFileOperate;
//

#define		TIMER_ID_REFRESH_FLOAT_INI			0x1000

const GUID  IID_KAVRECYCLE = { 0xdd0240c9, 0xf3fb, 0x43c0, { 0xb7, 0x4e, 0x2e, 0x30, 0x70, 0x10, 0xa, 0xd6 } };// hub trojan
#define HIT_STARTUPRUN _T("禁止此项将导致您的电脑在重启后失去保护，且开机延迟启动设置也将失效。\n 您确定要禁止吗？")

CBeikeSafeSettingDlg::~CBeikeSafeSettingDlg()
{
}

UINT_PTR CBeikeSafeSettingDlg::DoModal(int nPage, HWND hWndParent, CBeikeSafeMainDlg *pDialog, int nSubPage /*= -1*/)
{
    m_nPage = nPage;
	m_dlg = pDialog;
	m_nSubPage = nSubPage;
    return __super::DoModal(hWndParent);
}

BOOL CBeikeSafeSettingDlg::BkSafeSettingChanged()
{
    return m_bBkSafeSettingChanged;
}
int CBeikeSafeSettingDlg::_LoadBkSafePorxServer()
{
	ATL::CString strFilePath;
	CAppPath::Instance().GetLeidianAppPath(strFilePath);
	strFilePath.Append(L"\\Cfg\\bksafe.ini");
	CIniFile iniFile(strFilePath);
	int nType = -1;
	iniFile.GetIntValue(L"proxy",	L"type",		nType);
	if (nType <= 0)
	{
		SetItemCheck(IDC_CHECK_PROX_SERVER, FALSE);
		EnableItem(IDC_LINKTEXT_PROX_SERVER, FALSE);//代理设置的初始值
	}
	else
	{
		SetItemCheck(IDC_CHECK_PROX_SERVER, TRUE);
		EnableItem(IDC_LINKTEXT_PROX_SERVER, TRUE);//代理设置的初始值
	}
	
	return nType;
}
BOOL CBeikeSafeSettingDlg::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
    SetTabCurSel(IDC_TAB_SETTING, m_nPage);
	if (m_nSubPage != -1)
		SetTabCurSel(IDC_TAB_SETTING_SOFTMGR, m_nSubPage);

    _LoadBkSafeSettings();
    _LoadTraySettings();
    _LoadSvcSettings();
    _LoadIEFixSettings();
	_Vulfix_Setting_Init();
	_LoadBkSysOptSettings();//加载系统优化的设置
	_LoadBkSoftMgrSettings();//加载软件管理的设置
	_LoadBkSafeKwsSettings();//加载网盾的设置
	_LoadBkSafePorxServer();//代理服务器加载	

	if (TRUE == IsNetMonDriverOk())
		SetTimer(TIMER_ID_REFRESH_FLOAT_INI, 2000, NULL);
    return TRUE;
}

void CBeikeSafeSettingDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (TIMER_ID_REFRESH_FLOAT_INI == nIDEvent)
	{
		SetItemCheck(IDC_CHECK_NETFLOW_CONTROL, GetFloatWndDisplayStatus());
		//资源监控
		SetItemCheck(IDC_CHECK_SYSPER_CONTROL, IsSysPerfFloatWndVisable());

		//总的控制显示开关
		BOOL bFlag = IsFloatWndStartWithTray();
		SetItemCheck(IDC_CHECK_FLOATWND_CONTROL, bFlag);
		EnableItem(IDC_CHECK_SYSPER_CONTROL, bFlag);
		EnableItem(IDC_CHECK_NETFLOW_CONTROL, bFlag);
	}
}

LRESULT CBeikeSafeSettingDlg::OnAppProxyChange( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	GetParent().PostMessage( MSG_APP_PROXY_CHANGE, NULL, NULL );
	return TRUE;
}

void CBeikeSafeSettingDlg::OnBkBtnOK()
{
    _SaveSettings();//除去修复漏洞外  所有的
	if( !_Vulfix_Setting_Save() )//修复漏洞
	{
		// 漏洞修复目录设置失败, 需要重新指定目录 
		return;
	}
    EndDialog(IDOK);
}

void CBeikeSafeSettingDlg::OnBkBtnClose()
{
    EndDialog(IDCANCEL);
}
//hub  
//系统优化--开机提示
void CBeikeSafeSettingDlg::_LoadBkSysOptSettings()
{
	DWORD dwEditFlags = WS_CHILD | WS_TABSTOP;
	m_editDayLimit.Create( GetViewHWND(), NULL, NULL, dwEditFlags, 0, IDC_CHECK_REMIND_DAY_RDLG, NULL);
	m_editMonthLimit.Create( GetViewHWND(), NULL, NULL, dwEditFlags, 0, IDC_CHECK_REMIND_MONTH_RDLG, NULL);

	m_editDayLimit.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
	m_editMonthLimit.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));

	m_editDayLimit.ModifyStyle(ES_LEFT, ES_RIGHT | ES_NUMBER);
	m_editMonthLimit.ModifyStyle(ES_LEFT, ES_RIGHT | ES_NUMBER);

	m_editDayLimit.LimitText(5);
	m_editMonthLimit.LimitText(5);

	m_editDayLimit.SetFocus();

	CAppPath::Instance().GetLeidianAppPath(m_strFilePath);
	m_strFilePath.Append(FILEPATH);
	ReadSettingsAndSetCheck();
	return;
}
void CBeikeSafeSettingDlg::ReadSettingsAndSetCheck()
{
	CIniFile iniFile(m_strFilePath);	
	iniFile.GetIntValue(SEC_RUNOPT,KEY_CONDITION_SHOW,m_bR1Checked);
	iniFile.GetIntValue(SEC_RUNOPT,KEY_ALLWAYS_SHOW, m_bR2Checked);
	iniFile.GetIntValue(SEC_RUNOPT,KEY_NEVER_SHOW, m_bR3Checked);
	iniFile.GetIntValue(SEC_RUNOPT,KEY_AFTER_OPT, m_bC1Checked);		
	iniFile.GetIntValue(SEC_RUNOPT,KEY_BOOT_SLOW, m_bC2Checked);

	int nWndOpt = 1;
	iniFile.GetIntValue(SEC_PERFOPT,KEY_WND_OPT_CHECK, nWndOpt);
	SetItemCheck(IDC_CHECK_WND_OPT, nWndOpt != 0 );


	EnableItem(IDC_CHECK_AFTER_OPT_START,m_bR1Checked == 1);
	EnableItem(IDC_CHECK_BOOT_TOOSLOW,m_bR1Checked == 1);

	if (m_bR1Checked != 1 && m_bR2Checked != 1 && m_bR3Checked != 1)
	{
		SetItemCheck(IDC_RADIO_CONDITIONS_SHOW, FALSE);
		SetItemCheck(IDC_RADIO_ALLWAYS_SHOW, TRUE);
		SetItemCheck(IDC_RADIO_NEVER_SHOW, FALSE);
	}
	else
	{
		SetItemCheck(IDC_RADIO_CONDITIONS_SHOW,m_bR1Checked == TRUE);
		SetItemCheck(IDC_RADIO_ALLWAYS_SHOW, m_bR2Checked == TRUE);
		SetItemCheck(IDC_RADIO_NEVER_SHOW, m_bR3Checked == TRUE);
		if (m_bR1Checked == 1)
		{
			if ((m_bC1Checked == 0) && (m_bC2Checked == 0))
			{
				SetItemCheck(IDC_CHECK_AFTER_OPT_START,TRUE);
				SetItemCheck(IDC_CHECK_BOOT_TOOSLOW,TRUE);
			}
			else
			{
				SetItemCheck(IDC_CHECK_AFTER_OPT_START, m_bC1Checked == TRUE);
				SetItemCheck(IDC_CHECK_BOOT_TOOSLOW, m_bC2Checked == TRUE);
			}
		}
	}
	/*SetItemCheck(IDC_CHECK_NETFLOW_CONTROL, GetFloatWndDisplayStatus());*///读取预设的值  流量监控的check	
	SetItemCheck(IDC_CHECK_NETFLOW_CONTROL, GetFloatWndDisplayStatus());

	//资源监控
	SetItemCheck(IDC_CHECK_SYSPER_CONTROL, IsSysPerfFloatWndVisable());


	//总的控制显示开关
	BOOL bFlag = IsFloatWndStartWithTray();
	SetItemCheck(IDC_CHECK_FLOATWND_CONTROL, bFlag);

	EnableItem(IDC_CHECK_SYSPER_CONTROL, bFlag);
	EnableItem(IDC_CHECK_NETFLOW_CONTROL, bFlag);


	//显示数据
	int nDayLimite = _GetNetMoniterValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_REMAID_DAY_NAME);
	int nMonthLimite = _GetNetMoniterValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_REMAID_MONTH_NAME);
	if (0 == nDayLimite)
		SetItemCheck(IDC_CHECK_REMIND_DAY_TEXT, FALSE);
	else
	{
		SetItemCheck(IDC_CHECK_REMIND_DAY_TEXT, TRUE);
		CString strValue;
		strValue.Format(_T("%ld"), nDayLimite);
		m_editDayLimit.SetWindowText(strValue);
	}

	if (0 == nMonthLimite)
		SetItemCheck(IDC_CHECK_REMIND_MONTH_TEXT, FALSE);
	else
	{
		SetItemCheck(IDC_CHECK_REMIND_MONTH_TEXT, TRUE);
		CString strValue;
		strValue.Format(_T("%ld"), nMonthLimite);
		m_editMonthLimit.SetWindowText(strValue);
	}
	
	OnClickCheckDay();
	OnClickCheckMonth();

	return;
}
void CBeikeSafeSettingDlg::OnProxServerCheckChange()//代理服务器设置
{
	//判断配置文件中是否设置代理服务器
	m_bProxChanged = TRUE;
	BOOL b = GetItemCheck(IDC_CHECK_PROX_SERVER);
	if (b)//勾选
	{
		EnableItem(IDC_LINKTEXT_PROX_SERVER, TRUE);
		//判断是否要弹出设置框
		if (ProxServerReadCfg())
		{
			ShowProxySetting();

			if (_LoadBkSafePorxServer() > 0)
			{
				SetItemCheck(IDC_CHECK_PROX_SERVER, TRUE);
				EnableItem(IDC_LINKTEXT_PROX_SERVER, TRUE);
			}
			else
			{
				SetItemCheck(IDC_CHECK_PROX_SERVER, FALSE);
				EnableItem(IDC_LINKTEXT_PROX_SERVER, FALSE);
			}
		}
	}
	else//非勾选
	{
		EnableItem(IDC_LINKTEXT_PROX_SERVER, FALSE);
	}
	return;
}
BOOL CBeikeSafeSettingDlg::ProxServerReadCfg()
{
	TCHAR			sUser[256] = {0};
	TCHAR			sPass[256] = {0};
	TCHAR			sHost[256] = {0};		
	int				nPort;
	int				nValidate;	
	CIniFile iniFile(m_strFilePath);;
	iniFile.GetIntValue(L"proxy",	L"validate",	nValidate);
	iniFile.GetStrValue(L"proxy",	L"host",		sHost,	sizeof(sHost));
	iniFile.GetIntValue(L"proxy",	L"port",		nPort);			
	iniFile.GetStrValue(L"proxy",	L"user",		sUser,	sizeof(sUser));
	iniFile.GetStrValue(L"proxy",	L"password",	sPass,	sizeof(sPass));	
	ATL::CString strUser = sUser;
	ATL::CString strPass = sPass;
	ATL::CString strHost = sHost;
	if (strUser == L"" && strPass == L"" && strHost == L"" && nPort == -1 && nValidate == -1)
	{
		return TRUE;
	}
	return FALSE;
}

void CBeikeSafeSettingDlg::OnBkSafeNetCheckChange()//流量控制
{
	//图片的变换
/*
	if (TRUE == _Module.Exam.IsWin64())//系统为64位的
	{		
		SetItemVisible(IDC_IMAGID_NETFLOW_CONTROL_GRAY,   TRUE);
		SetItemVisible(IDC_IMAGID_NETFLOW_CONTROL_BRIGHT, FALSE);
		CBkSafeMsgBox2::ShowMultLine( L"抱歉，流量监控功能暂不支持64位系统，我们将尽快改进！", L"金山卫士流量监控",  MB_OK|MB_ICONWARNING);
		SetItemCheck(IDC_CHECK_NETFLOW_CONTROL, FALSE);
		return;
	}*/
	KillTimer(TIMER_ID_REFRESH_FLOAT_INI);

	if (FALSE == GetItemCheck(IDC_CHECK_SYSPER_CONTROL) &&
		FALSE == GetItemCheck(IDC_CHECK_NETFLOW_CONTROL))
	{
		if (IDOK != MsgBoxHideFloatWnd())
		{
			SetItemCheck(IDC_CHECK_NETFLOW_CONTROL, TRUE);
			//SetItemCheck(IDC_CHECK_NETFLOW_CONTROL, IsSysPerfFloatWndVisable());
			goto Exit;
		}

		SetItemCheck(IDC_CHECK_FLOATWND_CONTROL, FALSE);
		EnableItem(IDC_CHECK_SYSPER_CONTROL, GetItemCheck(IDC_CHECK_FLOATWND_CONTROL));
		EnableItem(IDC_CHECK_NETFLOW_CONTROL, GetItemCheck(IDC_CHECK_FLOATWND_CONTROL));
		SetItemCheck(IDC_CHECK_NETFLOW_CONTROL, TRUE);

//		if (FALSE == GetItemCheck(IDC_CHECK_FLOATWND_CONTROL))
		CloseFloatWnd();
	}
	else
	{
		if (GetItemCheck(IDC_CHECK_NETFLOW_CONTROL))//显示
		{
			OpenNetFlowWnd();
		}
		else
		{
			CloseNetFlowWnd();
		}

	}

Exit:
	SetTimer(TIMER_ID_REFRESH_FLOAT_INI, 2000, NULL);

	return;
}

DWORD CBeikeSafeSettingDlg::MsgBoxNetMonDriverError(CString strTitle, CString strDes)
{
	DWORD dwRet = IDOK;

	CBkSafeMsgBox2 msgbox;
	msgbox.AddButton(BkString::Get(2010), IDOK);
	msgbox.AddButton(BkString::Get(2011), IDCANCEL);

	dwRet = msgbox.ShowMsg(strTitle, strDes, MB_BK_CUSTOM_BUTTON | MB_ICONINFORMATION, NULL, m_hWnd);

	return dwRet;

}

DWORD CBeikeSafeSettingDlg::MsgBoxHideFloatWnd()
{
	DWORD dwRet = IDOK;
	
	dwRet = CBkSafeMsgBox2::Show(BkString::Get(IDS_EXAM_1444), BkString::Get(IDS_EXAM_1445), MB_OKCANCEL|MB_ICONQUESTION);

	return dwRet;
}

void CBeikeSafeSettingDlg::OnClickSysPerfCtrl()
{
	KillTimer(TIMER_ID_REFRESH_FLOAT_INI);

	if (FALSE == GetItemCheck(IDC_CHECK_NETFLOW_CONTROL) &&
		FALSE == GetItemCheck(IDC_CHECK_SYSPER_CONTROL))
	{
		if (IDOK != MsgBoxHideFloatWnd())
		{
			SetItemCheck(IDC_CHECK_SYSPER_CONTROL, TRUE);
			//SetItemCheck(IDC_CHECK_SYSPER_CONTROL, IsSysPerfFloatWndVisable());
			goto Exit;
		}


		SetItemCheck(IDC_CHECK_FLOATWND_CONTROL, FALSE);
		EnableItem(IDC_CHECK_SYSPER_CONTROL, GetItemCheck(IDC_CHECK_FLOATWND_CONTROL));
		EnableItem(IDC_CHECK_NETFLOW_CONTROL, GetItemCheck(IDC_CHECK_FLOATWND_CONTROL));
		SetItemCheck(IDC_CHECK_SYSPER_CONTROL, TRUE);

	//	if (FALSE == GetItemCheck(IDC_CHECK_FLOATWND_CONTROL))
		CloseFloatWnd();
	}
	else
	{
		if (TRUE == GetItemCheck(IDC_CHECK_SYSPER_CONTROL))
		{
			OpenSysPerfFloatWnd();
		}
		else
		{
			CloseSysPerfFloatWnd();
		}
	}

Exit:
	SetTimer(TIMER_ID_REFRESH_FLOAT_INI, 2000, NULL);

	return;
}


void CBeikeSafeSettingDlg::_SetNetMoniterValue(IN const CString& strTerm, IN const CString& strName, IN const CString& strValue)
{
	ATL::CString cfgFileNamePath = L"\\cfg\\floatwnd.ini";
	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	ATL::CString trayPath;
	trayPath = buffer;
	cfgFileNamePath = trayPath + cfgFileNamePath;

	::WritePrivateProfileStringW(strTerm, strName, strValue, cfgFileNamePath);
	int nTime = ::_wtol(strValue);
	if (strName == FLOATWND_CONFIG_REMAID_DAY_NAME)
		KOperMemFile::Instance().SetDayLimited(nTime);
	else if (strName == FLOATWND_CONFIG_REMAID_MONTH_NAME)
		KOperMemFile::Instance().SetMonthLimited(nTime);
	else if (strName == FLOATWND_CONFIG_REMAID_DAY_DATA_NAME)
		KOperMemFile::Instance().SetDayLastData(nTime);
	else if (strName == FLOATWND_CONFIG_REMAID_MONTH_DATA_NAME)
		KOperMemFile::Instance().SetMonthLastData(nTime);
}

int CBeikeSafeSettingDlg::_GetNetMoniterValue(IN const CString& strTerm, IN const CString& strName)
{
	ATL::CString cfgFileNamePath = L"\\cfg\\floatwnd.ini";
	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	ATL::CString trayPath;
	trayPath = buffer;
	cfgFileNamePath = trayPath + cfgFileNamePath;

	int defaultValue = 0;
	return ::GetPrivateProfileInt(strTerm,  strName, defaultValue, cfgFileNamePath);
}

void CBeikeSafeSettingDlg::WriteSettings()
{
	KOperMemFile::Instance().Init();
	CIniFile iniFile(m_strFilePath);
	m_bR1Checked = GetItemCheck(IDC_RADIO_CONDITIONS_SHOW);
	m_bC1Checked = GetItemCheck(IDC_CHECK_AFTER_OPT_START);
	m_bC2Checked = GetItemCheck(IDC_CHECK_BOOT_TOOSLOW);
	m_bR2Checked = GetItemCheck(IDC_RADIO_ALLWAYS_SHOW);
	m_bR3Checked = GetItemCheck(IDC_RADIO_NEVER_SHOW);

	BOOL bWndOpt = GetItemCheck( IDC_CHECK_WND_OPT );

	iniFile.SetIntValue(SEC_RUNOPT,KEY_CONDITION_SHOW,m_bR1Checked);
	iniFile.SetIntValue(SEC_RUNOPT,KEY_AFTER_OPT,m_bC1Checked);
	iniFile.SetIntValue(SEC_RUNOPT,KEY_BOOT_SLOW,m_bC2Checked);
	iniFile.SetIntValue(SEC_RUNOPT,KEY_ALLWAYS_SHOW,m_bR2Checked);
	iniFile.SetIntValue(SEC_RUNOPT,KEY_NEVER_SHOW,m_bR3Checked);
	iniFile.SetIntValue(SEC_PERFOPT,KEY_WND_OPT_CHECK,bWndOpt);

	//悬浮窗
// 	if (TRUE == GetItemCheck(IDC_CHECK_FLOATWND_CONTROL))
// 	{
// 		OpenFloatWnd();
// 	}
// 	else
// 	{
// 		CloseFloatWnd();
// 	}
	//流量监控
// 	BOOL bCheck3 = GetItemCheck(IDC_CHECK_NETFLOW_CONTROL);
// 	if (bCheck3)//显示
// 	{
// 		OpenNetFlowWnd();
// 	}
// 	else
// 	{
// 		CloseNetFlowWnd();
// 	} 

	//更新资源监控的信息
// 	bCheck3 = GetItemCheck(IDC_CHECK_SYSPER_CONTROL);
// 	if (TRUE == bCheck3)
// 	{
// 		OpenSysPerfFloatWnd();
// 	}
// 	else
// 	{
// 		CloseSysPerfFloatWnd();
// 	}


	BOOL bCheckRemindDay = GetItemCheck(IDC_CHECK_REMIND_DAY_TEXT);
	BOOL bCheckRemindMonth = GetItemCheck(IDC_CHECK_REMIND_MONTH_TEXT);

	int nOldDay = _GetNetMoniterValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_REMAID_DAY_NAME);
	int nOldMonth = _GetNetMoniterValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_REMAID_MONTH_NAME);

	CString strText = _T("0");
	if (bCheckRemindDay)
		m_editDayLimit.GetWindowText(strText);
	
	_SetNetMoniterValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_REMAID_DAY_NAME, strText);
	if (nOldDay != _wtol(strText))
		_SetNetMoniterValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_REMAID_DAY_DATA_NAME, _T("0"));
	
	strText = _T("0");
	if (bCheckRemindMonth)
		m_editMonthLimit.GetWindowText(strText);
	
	_SetNetMoniterValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_REMAID_MONTH_NAME, strText);

	if (nOldMonth != _wtol(strText))
		_SetNetMoniterValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_REMAID_MONTH_DATA_NAME, _T("0"));

	KOperMemFile::Instance().Uninit();
}

void CBeikeSafeSettingDlg::OnRadioCheckChanged()
{
	BOOL bChecked = GetItemCheck(IDC_RADIO_CONDITIONS_SHOW);
	if (bChecked)
	{
		if ((m_bC1Checked == 0)&&(m_bC2Checked == 0))
		{
			SetItemCheck(IDC_CHECK_AFTER_OPT_START,TRUE);
			SetItemCheck(IDC_CHECK_BOOT_TOOSLOW,TRUE);
		}
		else
		{
			SetItemCheck(IDC_CHECK_AFTER_OPT_START,m_bC1Checked == TRUE);
			SetItemCheck(IDC_CHECK_BOOT_TOOSLOW,m_bC2Checked == TRUE);
		}
	}
	EnableItem(IDC_CHECK_AFTER_OPT_START,bChecked);
	EnableItem(IDC_CHECK_BOOT_TOOSLOW,bChecked);
}

void CBeikeSafeSettingDlg::OnCheckChanged()
{
	BOOL bCheck = GetItemCheck(IDC_CHECK_BOOT_TOOSLOW);
	BOOL bCheck2 = GetItemCheck(IDC_CHECK_AFTER_OPT_START);
	if (!bCheck && !bCheck2)
	{
		SetItemCheck(IDC_RADIO_CONDITIONS_SHOW,FALSE);
		EnableItem(IDC_CHECK_AFTER_OPT_START,FALSE);
		EnableItem(IDC_CHECK_BOOT_TOOSLOW,FALSE);
		SetItemCheck(IDC_RADIO_ALLWAYS_SHOW,TRUE);
		SetItemCheck(IDC_RADIO_NEVER_SHOW,FALSE);
	}	
	return;
}



BOOL CBeikeSafeSettingDlg::IsFloatWndStartWithTray()
{
	//CString cfgFileNamePath = _GetFloatWndConfigFilePath();

	if (FALSE == IsNetMonDriverOk())
		return FALSE;
	//////////////////////////////////////////////////////////////////////////
	ATL::CString cfgFileNamePath = L"\\cfg\\floatwnd.ini";
	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	ATL::CString trayPath;
	trayPath = buffer;
	cfgFileNamePath = trayPath + cfgFileNamePath;

	int defaultValue = 1;
	int result = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_DISPLAY_WITH_TRAY_KEY_NAME, defaultValue, cfgFileNamePath);

	return result == 1;
}

void CBeikeSafeSettingDlg::OpenFloatWnd()
{
	HANDLE hEventOpenFloatWndDlg = ::OpenEvent(
		EVENT_ALL_ACCESS,
		FALSE,
		EVENT_OPEN_FLOATWND_DLG
		);

	if (hEventOpenFloatWndDlg == NULL)
	{
		return;
	}

	SetEvent(hEventOpenFloatWndDlg);
	CloseHandle(hEventOpenFloatWndDlg);

	return;
}

void CBeikeSafeSettingDlg::CloseFloatWnd()
{
	HANDLE hEventOpenFloatWndDlg = ::OpenEvent(
		EVENT_ALL_ACCESS,
		FALSE,
		EVENT_CLOSE_FLOATWND_DLG
		);

	if (hEventOpenFloatWndDlg == NULL)
	{
		return;
	}

	SetEvent(hEventOpenFloatWndDlg);
	CloseHandle(hEventOpenFloatWndDlg);
}

void CBeikeSafeSettingDlg::OpenNetFlowWnd()//打开流量窗口
{
	HANDLE hEventOpenFloatWndDlg = ::OpenEvent(
		EVENT_ALL_ACCESS,
		FALSE,
		EVENT_OPEN_NETMON_TIP_DLG
		);

	if (hEventOpenFloatWndDlg == NULL)
	{
		netmon_log(L"_OpenFloatWnd  open EVENT_OPEN_FLOATWND_DLG failed");
		return;
	}

	if( FALSE == IsFloatWndStartWithTray() )
		OpenFloatWnd();

	SetEvent(hEventOpenFloatWndDlg);
	CloseHandle(hEventOpenFloatWndDlg);

	netmon_log(L"SetEvent:EVENT_OPEN_FLOATWND_DLG");
	return;
}

void CBeikeSafeSettingDlg::CloseNetFlowWnd()//关闭流量窗口
{
	HANDLE hEventCloseFloatWndDlg = ::OpenEvent(
		EVENT_ALL_ACCESS,
		FALSE,
		EVENT_CLOSE_NETMON_TIP_DLG
		);

	if (hEventCloseFloatWndDlg == NULL)
	{
		netmon_log(L"_CloseFloatWnd  open EVENT_CLOSE_FLOATWND_DLG failed");
		return;
	}

	SetEvent(hEventCloseFloatWndDlg);
	CloseHandle(hEventCloseFloatWndDlg);

	netmon_log(L"SetEvent:EVENT_CLOSE_FLOATWND_DLG");
	return;
}

void CBeikeSafeSettingDlg::OpenSysPerfFloatWnd()
{
	HANDLE hEventOpenFloatWndDlg = ::OpenEvent(
		EVENT_ALL_ACCESS,
		FALSE,
		EVENT_SYSTEM_PERFORMANCE_DLG_UPDATE_FLOATWND_STATUS
		);

	if (hEventOpenFloatWndDlg == NULL)
	{
		netmon_log(L"_OpenFloatWnd  open EVENT_OPEN_FLOATWND_DLG failed");
		return;
	}

	if( FALSE == IsFloatWndStartWithTray() )
		OpenFloatWnd();


	SetEvent(hEventOpenFloatWndDlg);
	CloseHandle(hEventOpenFloatWndDlg);

	netmon_log(L"SetEvent:EVENT_OPEN_FLOATWND_DLG");
	return;

}

void CBeikeSafeSettingDlg::OnOpenFloatWnd()
{
	KillTimer(TIMER_ID_REFRESH_FLOAT_INI);

	SetItemCheck(IDC_CHECK_SYSPER_CONTROL, IsSysPerfFloatWndVisable());
	SetItemCheck(IDC_CHECK_NETFLOW_CONTROL, GetFloatWndDisplayStatus());


	if (TRUE == GetItemCheck(IDC_CHECK_FLOATWND_CONTROL))
	{
		static int nCount = 0;
		if (0 == nCount &&
			FALSE == IsNetMonDriverOk())
		{
			//ReBoot();
			++nCount;//只弹出一次
			//MsgBoxNetMonDriverError(BkString::Get(IDS_EXAM_1446), L"");
			CBkSafeMsgBox2::ShowMultLine(BkString::Get(IDS_EXAM_1446), NULL, MB_OK|MB_ICONINFORMATION, NULL, GetViewHWND());
		}

		OpenFloatWnd();
		
	}
	else
	{
		if (IDOK == MsgBoxHideFloatWnd())
			CloseFloatWnd();
		else
			SetItemCheck(IDC_CHECK_FLOATWND_CONTROL, TRUE);
	}

	EnableItem(IDC_CHECK_SYSPER_CONTROL, GetItemCheck(IDC_CHECK_FLOATWND_CONTROL));
	EnableItem(IDC_CHECK_NETFLOW_CONTROL, GetItemCheck(IDC_CHECK_FLOATWND_CONTROL));

	SetTimer(TIMER_ID_REFRESH_FLOAT_INI, 2000, NULL);
}

void CBeikeSafeSettingDlg::CloseSysPerfFloatWnd()
{
	HANDLE hEventCloseFloatWndDlg = ::OpenEvent(
		EVENT_ALL_ACCESS,
		FALSE,
		EVENT_SYSTEM_PERFORMANCE_CLOSE_TIP_DLG
		);

	if (hEventCloseFloatWndDlg == NULL)
	{
		netmon_log(L"_CloseFloatWnd  open EVENT_CLOSE_FLOATWND_DLG failed");
		return;
	}

	SetEvent(hEventCloseFloatWndDlg);
	CloseHandle(hEventCloseFloatWndDlg);

	netmon_log(L"SetEvent:EVENT_CLOSE_FLOATWND_DLG");
	return;

}

BOOL CBeikeSafeSettingDlg::IsNetMonDriverOk()
{
	KUrlMonCfgReader urlmonCfgReader;

	// 配置文件初始化成功，且流量信息初始化失败，则驱动加载失败。
	BOOL ret = SUCCEEDED(KNetFluxCacheReader::Instance().Init());

	return (SUCCEEDED(urlmonCfgReader.Init()) && ret);;
}

BOOL CBeikeSafeSettingDlg::GetFloatWndDisplayStatus()
{
	//CString cfgFileNamePath = _GetFloatWndConfigFilePath();
	ATL::CString cfgFileNamePath = L"\\cfg\\floatwnd.ini";
	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	ATL::CString trayPath;
	trayPath = buffer;
	cfgFileNamePath = trayPath + cfgFileNamePath;

	int defaultValue = 1;
// 	int result = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_DISPLAY_WITH_TRAY_KEY_NAME, defaultValue, cfgFileNamePath);
// 	if (1 != result)
// 		return result;
// 
// 	defaultValue = 1;
	int result = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_OPEN_NETMON_FLOATWND, defaultValue, cfgFileNamePath);
	return result == 1;
}

BOOL CBeikeSafeSettingDlg::IsSysPerfFloatWndVisable()
{
	ATL::CString cfgFileNamePath = L"\\cfg\\floatwnd.ini";
	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	ATL::CString trayPath;
	trayPath = buffer;
	cfgFileNamePath = trayPath + cfgFileNamePath;

	int defaultValue = 1;
// 	int result = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_DISPLAY_WITH_TRAY_KEY_NAME, defaultValue, cfgFileNamePath);
// 	if (1 != result)
// 		return result;
// 
// 	defaultValue = 1;

	int result = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_OPEN_SYSPERFORMANCE_FLOATWND, defaultValue, cfgFileNamePath);
	return result == 1;

}
//系统优化  开机提示设置结束
//软件管理设置
void CBeikeSafeSettingDlg::_LoadBkSoftMgrSettings()
{
	DWORD dwEditFlags = WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY;
	m_ctlEditStorePath.Create( GetViewHWND(), NULL, NULL, dwEditFlags, WS_EX_NOPARENTNOTIFY, IDC_EDIT_STORE_DIR, NULL);
	m_ctlEditStorePath.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));

	LoadSoftMgrSettings();

	CString		strFreeDiskSpace;
	CString		strStorePath;
	m_ctlEditStorePath.GetWindowText( strStorePath );
	GetCurDiskFreeSpace( strStorePath, strFreeDiskSpace );
	SetItemText( IDC_LBL_DISK_SPACE, strFreeDiskSpace );
	return;
}
void CBeikeSafeSettingDlg::LoadSoftMgrSettings()
{
	CString	strStoreDir;
	BKSafeConfig::GetStoreDir( strStoreDir );
	m_ctlEditStorePath.SetWindowText( strStoreDir );

	int		nRet;

	nRet = BKSafeConfig::GetHintPlugin();
	if (nRet)
		SetItemCheck(IDC_RADIO_SHOW_PLUGIN_TIP, TRUE);
	else
		SetItemCheck(IDC_RADIO_IGNORE_PLUGIN_TIP, TRUE);

	nRet = BKSafeConfig::GetUpdatePointOut();
	switch(nRet)
	{
	case 0:SetItemCheck(IDC_RADIO_UPDATE_TIP_DAY, TRUE);break;
	case 1:SetItemCheck(IDC_RADIO_UPDATE_TIP_WEEK, TRUE);break;
	case 2:SetItemCheck(IDC_RADIO_UPDATE_TIP_NEVER, TRUE);break;
	}

	nRet = BKSafeConfig::GetAutoInstall();
	switch(nRet)
	{
	case 0:SetItemCheck(IDC_RADIO_RUN_SETUP_HAND, TRUE);break;
	case 1:SetItemCheck(IDC_RADIO_RUN_SETUP_AUTO, TRUE);break;
	}

	nRet = BKSafeConfig::GetPowerSweepFile();
	switch(nRet)
	{
	case 0:SetItemCheck(IDC_RADIO_DELETE_TO_RECYCLE, TRUE);break;
	case 1:SetItemCheck(IDC_RADIO_DELETE_TO_DIRECT, TRUE);break;
	}

	nRet = BKSafeConfig::GetDelFile();
	switch(nRet)
	{
	case 0:SetItemCheck(IDC_RADIO_DELETE_SETUP_NEVER, TRUE);break;
	case 1:SetItemCheck(IDC_RADIO_DELETE_SETUP_NOW, TRUE);break;
	case 2:SetItemCheck(IDC_RADIO_DELETE_SETUP_WEEK, TRUE);break;
	}

	nRet = BKSafeConfig::GetMajorUpdatePointOut();
	if (nRet)
		SetItemCheck(IDC_CHECK_MAJOR_POINT_OUT, TRUE);
	else
		SetItemCheck(IDC_CHECK_MAJOR_POINT_OUT, FALSE);

	nRet = BKSafeConfig::GetUpdateWhenRun();
	if (nRet)
		SetItemCheck(IDC_RADIO_UPDATE_WHEN_RUN, TRUE);
	else
		SetItemCheck(IDC_RADIO_UPDATE_WHEN_RUN, FALSE);

	nRet = BKSafeConfig::GetUnKnownFileOpen();
	if( nRet && IsUnknownFileOpenValid() )
		SetItemCheck( IDC_CHECK_UNKNOWN_FILE_OPEN, TRUE );
	else
		SetItemCheck( IDC_CHECK_UNKNOWN_FILE_OPEN, FALSE );

	BOOL bIcon = FALSE;
	BOOL bNum = FALSE;
	BKSafeConfig::GetSoftUpdateDesktopIcon(bIcon, bNum);
	if (!IsExistDesktopIcon())
	{
		bIcon = FALSE;
		BKSafeConfig::SetSoftUpdateDesktopIcon(FALSE, bNum);
	}
	if (bNum)
		SetItemCheck(IDC_CHECK_SHOW_ICO_NUM, TRUE);
	else
		SetItemCheck(IDC_CHECK_SHOW_ICO_NUM, FALSE);
	if (bIcon)
	{
		SetItemCheck(IDC_CHECK_NO_HINT, TRUE);
		EnableItem( IDC_CHECK_SHOW_ICO_NUM, TRUE);
	}
	else
	{
		SetItemCheck(IDC_CHECK_NO_HINT, FALSE);
		EnableItem( IDC_CHECK_SHOW_ICO_NUM, FALSE);
	}

	return;
}

BOOL CBeikeSafeSettingDlg::IsUnknownFileOpenValid()
{
	BOOL bRet = FALSE;

	CRegKey reg;
	LONG lRet = reg.Open( HKEY_CLASSES_ROOT, TEXT("Unknown\\shell\\openas\\command") );
	if( lRet == ERROR_SUCCESS )
	{
		TCHAR tszParam[MAX_PATH*2] = {0};
		ULONG nBuffer = MAX_PATH*2;
		if( ERROR_SUCCESS == reg.QueryStringValue( NULL, tszParam, &nBuffer ) )
		{
			CString strParam( tszParam );
			int nRet = strParam.MakeLower().Find( STMGR_UNKNOWN_FILENAME );

			bRet = ( nRet != -1 );
		}
	}
	
	return bRet;
}


BOOL CBeikeSafeSettingDlg::IsExistDesktopIcon()
{
	TCHAR dstPath[MAX_PATH] = {0};
	::SHGetSpecialFolderPath(NULL, dstPath, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);
	::PathAddBackslash(dstPath);
	wcscat_s(dstPath, MAX_PATH, L"金山卫士软件管理");
	wcscat_s(dstPath, MAX_PATH, L".lnk");

	if (PathFileExists(dstPath))
		return TRUE;

	return FALSE;
}

void CBeikeSafeSettingDlg::CreateDesktopIcon(BOOL bCreate, BOOL bNum)
{
	TCHAR dstPath[MAX_PATH] = {0};
	::SHGetSpecialFolderPath(NULL, dstPath, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);
	::PathAddBackslash(dstPath);
	wcscat_s(dstPath, MAX_PATH, L"金山卫士软件管理");
	wcscat_s(dstPath, MAX_PATH, L".lnk");

	//CString strTarget;
	//WinMod::CWinPathApi::ResolveLnkFile(dstPath, strTarget);
	int nNum = 0;
	nNum = BKSafeConfig::GetDesktopIconNum();

	if (!bCreate)
	{
		::DeleteFile(dstPath);
		return ;
	}

	if (!bNum)
		nNum = 0;

	CString  strPath;
	CAppPath::Instance().GetLeidianAppPath(strPath);
	strPath.Append(L"\\");

	CString strSoft;
	strSoft = strPath + L"KSafe.exe";
	CString strIcon;
	strIcon = strPath;

	if (nNum > 0)
	{
		CString strFile;
		if (nNum > 10)
			strFile = L"soft_10+.ico";
		else
			strFile.Format(L"soft_%d.ico", nNum);
		strIcon.Append(L"KSoft\\icon\\");
		strIcon.Append(strFile);

		if (!::PathFileExists(strIcon))
		{
			strIcon = strPath;
			strIcon.Append(L"KSoft\\softmgr.ico");
		}
	}
	else if (nNum == 0)
	{
		strIcon.Append(L"KSoft\\softmgr.ico");
	}

	WinMod::CWinPathApi::CreateLnkFile(strSoft, 
		L"-do:UI_SoftMgr_SoftUpdate",
		L"",
		dstPath,
		strIcon);
}

void CBeikeSafeSettingDlg::GetCurDiskFreeSpace( LPCTSTR szCurrentPath, CString & strFreeSpace )
{
	CString strDisk;
	strDisk = szCurrentPath;
	if ( ! strDisk.IsEmpty() )
	{
		strDisk = strDisk.Left( 3 );

		ULARGE_INTEGER   lpuse = {0}; 
		ULARGE_INTEGER   lptotal = {0};
		ULARGE_INTEGER   lpfree = {0};
		GetDiskFreeSpaceEx(strDisk,&lpuse,&lptotal,&lpfree);

		if ( lpfree.QuadPart > ( 1024 * 1024 * 1024 ) )
		{
			double	freeGB = lpfree.QuadPart / ( 1024 * 1024 * 1024.0 );
			strFreeSpace.Format( _T( "%.2fGB" ), freeGB );
		}
		else
		{
			ULONGLONG		freeMB = lpfree.QuadPart / ( 1024 * 1024 );
			strFreeSpace.Format( _T( "%dMB" ), freeMB );
		}
	}
	return;
}
void CBeikeSafeSettingDlg::OnRestoreDefaultDir()
{
	WCHAR szSystemPath[MAX_PATH] = {0};
	GetSystemDirectory(szSystemPath, MAX_PATH-1);
	CString strSysPath;
	strSysPath.Format(_T("%s"), szSystemPath);
	strSysPath = strSysPath.Left(3);
	strSysPath.Append( _T( "ksDownloads" ) );

	m_ctlEditStorePath.SetWindowText( strSysPath );

	m_bBkSoftMgrSettingChanged = TRUE;

	CString		strFreeDiskSpace;
	GetCurDiskFreeSpace( strSysPath, strFreeDiskSpace );
	SetItemText( IDC_LBL_DISK_SPACE, strFreeDiskSpace );
	return;
}
void CBeikeSafeSettingDlg::SaveSoftMgrSettings()
{
	int		nValue = 0;
	int		nValue2 = 0;

	nValue = GetItemCheck(IDC_RADIO_SHOW_PLUGIN_TIP) ? 1 : 0;
	BKSafeConfig::SetHintPlugin( nValue );

	if (GetItemCheck(IDC_RADIO_UPDATE_TIP_DAY))
		nValue = 0;
	else if (GetItemCheck(IDC_RADIO_UPDATE_TIP_WEEK))
		nValue = 1;
	else if (GetItemCheck(IDC_RADIO_UPDATE_TIP_NEVER))
		nValue = 2;
	BKSafeConfig::SetUpdatePointOut( nValue );

	if (GetItemCheck(IDC_RADIO_RUN_SETUP_HAND))
		nValue = 0;
	else if (GetItemCheck(IDC_RADIO_RUN_SETUP_AUTO))
		nValue = 1;
	BKSafeConfig::SetAutoInstall( nValue );

	if (GetItemCheck(IDC_RADIO_DELETE_TO_RECYCLE))
		nValue = 0;
	else if (GetItemCheck(IDC_RADIO_DELETE_TO_DIRECT))
		nValue = 1;
	BKSafeConfig::SetPowerSweepFile( nValue );

	if (GetItemCheck(IDC_RADIO_DELETE_SETUP_NEVER))
		nValue = 0;
	else if (GetItemCheck(IDC_RADIO_DELETE_SETUP_NOW))
		nValue = 1;
	else if (GetItemCheck(IDC_RADIO_DELETE_SETUP_WEEK))
		nValue = 2;
	BKSafeConfig::SetDelFile( nValue );

	CString	strStoreDir;
	m_ctlEditStorePath.GetWindowText( strStoreDir );
	BKSafeConfig::SetStortDir( strStoreDir );

	nValue = GetItemCheck(IDC_CHECK_MAJOR_POINT_OUT);
	BKSafeConfig::SetMajorUpdatePointOut(nValue);

	nValue = GetItemCheck(IDC_RADIO_UPDATE_WHEN_RUN);
	BKSafeConfig::SetUpdateWhenRun(nValue);

	nValue = GetItemCheck(IDC_CHECK_NO_HINT);
	nValue2 = GetItemCheck(IDC_CHECK_SHOW_ICO_NUM);
	BKSafeConfig::SetSoftUpdateDesktopIcon(nValue, nValue2);
	CreateDesktopIcon(nValue, nValue2);

	nValue = GetItemCheck( IDC_CHECK_UNKNOWN_FILE_OPEN );
	if( nValue )
	{
		BKSafeConfig::SetUnKnownFileOpen( 1 );
		CRegKey reg;
		TCHAR tszFilePath[MAX_PATH] = {0};
		::GetModuleFileName( NULL, tszFilePath, MAX_PATH );
		CPath pathFile( tszFilePath );
		pathFile.RemoveFileSpec();
		pathFile.Append( STMGR_UNKNOWN_FILENAME );
		CString strParam;
		strParam.Format( TEXT("\"%s\" \"%%1\""), pathFile.m_strPath );
		LONG lRes = reg.Create( HKEY_CLASSES_ROOT, TEXT("Unknown\\shell\\openas\\command") );
		if (lRes == ERROR_SUCCESS)
			lRes = reg.SetStringValue( NULL, strParam, REG_EXPAND_SZ );
		reg.Close();

		if( RunTimeHelper::IsVista() )
		{
			lRes = reg.Create( HKEY_CLASSES_ROOT, TEXT("Unknown\\shell\\opendlg\\command") );
			if (lRes == ERROR_SUCCESS)
				lRes = reg.SetStringValue( NULL, strParam, REG_EXPAND_SZ );
		}
	}
	else
	{
		BKSafeConfig::SetUnKnownFileOpen( 0 );
		CRegKey reg;

		LONG lRes = reg.Create( HKEY_CLASSES_ROOT, TEXT("Unknown\\shell\\openas\\command") );
		if (lRes == ERROR_SUCCESS)
			lRes = reg.SetStringValue( NULL, STMGR_DEFAULT_UN_FILE_P, REG_EXPAND_SZ );
		reg.Close();

		if( RunTimeHelper::IsVista() )
		{
			lRes = reg.Create( HKEY_CLASSES_ROOT, TEXT("Unknown\\shell\\opendlg\\command") );
			if (lRes == ERROR_SUCCESS)
				lRes = reg.SetStringValue( NULL, STMGR_DEFAULT_UN_FILE_P, REG_EXPAND_SZ );
		}
	}

	return;
}
void CBeikeSafeSettingDlg::OnOpenStoreDir()//打开目录
{
	CString strPath;
	m_ctlEditStorePath.GetWindowText( strPath );

	if( !PathIsDirectory(strPath) )
		CreateDirectoryNested( strPath );
	if( PathIsDirectory( strPath ) )
		ShellExecute(NULL, _T("open"), strPath, NULL, NULL, SW_SHOW);
	else
	{
		CString strMessage;
		strMessage.Format(BkString::Get(IDS_SOFTMGR_8045), strPath);
		CBkSafeMsgBox::Show(strMessage,BkString::Get(IDS_SOFTMGR_8046), MB_OK | MB_ICONWARNING);
	}
	return;
}
void CBeikeSafeSettingDlg::OnSelDir()//选择目录
{
	CString sFolderPath, sFolderNew;
	m_ctlEditStorePath.GetWindowText( sFolderPath );	
	sFolderNew = sFolderPath;
	while( SelectFolderEx(sFolderNew, m_hWnd) )
	{
		// 检查路径是否过长
		if ( sFolderNew.GetLength() > 200 )
		{
			CBkSafeMsgBox::Show( BkString::Get(IDS_SOFTMGR_8078), NULL, MB_OK | MB_ICONWARNING);
		}
		else
		{
			m_bBkSoftMgrSettingChanged = TRUE;
			m_ctlEditStorePath.SetWindowText( sFolderNew );

			CString		strFreeDiskSpace;
			GetCurDiskFreeSpace( sFolderNew, strFreeDiskSpace );
			SetItemText( IDC_LBL_DISK_SPACE, strFreeDiskSpace );

			break;
		}	
	}
	return;
}

void CBeikeSafeSettingDlg::OnBkSafeSoftMgrCreateDesktopIcoChanged()
{
	int		nValue = 0;
	nValue = GetItemCheck(IDC_CHECK_NO_HINT);
	if (nValue)
		EnableItem(IDC_CHECK_SHOW_ICO_NUM, TRUE);
	else
		EnableItem(IDC_CHECK_SHOW_ICO_NUM, FALSE);

	OnBkSafeSoftMgrSettingChanged();
}
void CBeikeSafeSettingDlg::OnBkSafeSoftMgrSettingChanged()
{
	m_bBkSoftMgrSettingChanged = TRUE;
	return;
}

//软件管理设置结束
//网盾设置
void CBeikeSafeSettingDlg::_LoadBkSafeKwsSettings()
{
	KwsSetting setting;
	BOOL bOpen = setting.IsTipSpannerEnable();
	SetItemCheck(IDC_CHECK_ADFILTER_CUE, bOpen);	

	bOpen = setting.IsAntiAdvLogEnabled();
	SetItemCheck(IDC_CHECK_ADFILTER_LOG, bOpen);
	//之下是下载保护的
	bOpen = setting.IsAutoScanEnable();
	SetItemCheck(IDC_CHECK_DOWNLOAD_LIS, bOpen);
	UpdataStat(bOpen);
	return;
}
void CBeikeSafeSettingDlg::UpdataStat(BOOL bOpen)
{
	KwsSetting setting;
	if (bOpen)
	{
		SetItemCheck(IDC_CHECK_DOWNLOAD_AUTOCLEAN,			setting.IsAutoCleanVirus());
		SetItemCheck(IDC_CHECK_DOWNLOAD_SAFE_NOCLUE,		!setting.IsShowSafePop());
		SetItemCheck(IDC_CHECK_DOWNLOAD_UNKNOWN_AUTOSUBMIT, !setting.IsShowGrayPop());
		EnableItem(IDC_CHECK_DOWNLOAD_AUTOCLEAN,			TRUE);
		EnableItem(IDC_CHECK_DOWNLOAD_SAFE_NOCLUE,			TRUE);
		EnableItem(IDC_CHECK_DOWNLOAD_UNKNOWN_AUTOSUBMIT,	TRUE);

	}
	else
	{
		SetItemCheck(IDC_CHECK_DOWNLOAD_AUTOCLEAN,			FALSE);
		SetItemCheck(IDC_CHECK_DOWNLOAD_SAFE_NOCLUE,		FALSE);
		SetItemCheck(IDC_CHECK_DOWNLOAD_UNKNOWN_AUTOSUBMIT, FALSE);
		EnableItem(IDC_CHECK_DOWNLOAD_AUTOCLEAN,			FALSE);
		EnableItem(IDC_CHECK_DOWNLOAD_SAFE_NOCLUE,			FALSE);
		EnableItem(IDC_CHECK_DOWNLOAD_UNKNOWN_AUTOSUBMIT,	FALSE);
	}
	return;
}
void CBeikeSafeSettingDlg::OnBkSafeKwsDownListen()
{
	BOOL bOpen = GetItemCheck(IDC_CHECK_DOWNLOAD_LIS);
	UpdataStat(bOpen);	
	m_bBkSafeKwsSettingChanged = TRUE;//网盾设置进行了改变
	return;
}
void CBeikeSafeSettingDlg::OnBkSafeKwsSettingChanged()
{
	m_bBkSafeKwsSettingChanged = TRUE;//网盾设置进行了改变
	return;
}
void CBeikeSafeSettingDlg::OnBkSafeKwsAdRuleSet()//自定义广告规则
{
	CSetBlackPage	dlg;
	dlg.DoModal();
	return;
}
void CBeikeSafeSettingDlg::OnBkSafeKwsAdFilterSet()//广告过滤白名单
{
	CSetAdFilterPage	dlg;//
	dlg.DoModal();
	return;
}
void CBeikeSafeSettingDlg::OnBkSafeKwsBrowsPage()//浏览器设置页面
{
	CSetBrowerPage		dlg;//
	dlg.DoModal();
	SendMessage( GetParent(), MSG_APP_DEF_BROWSER_CHANGE, NULL, NULL );
	return;
}

BOOL CBeikeSafeSettingDlg::ShowProxySetting()
{
	CSetProxServer dlg;
	UINT uId = dlg.DoModal();
	if( uId == IDOK )
	{

	}

	return TRUE;
}

//网盾设置结束
void CBeikeSafeSettingDlg::OnBkSafeProxServerPage()//代理服务器设置
{
	ShowProxySetting();
	if (_LoadBkSafePorxServer())
	{
		SetItemCheck(IDC_CHECK_PROX_SERVER, TRUE);
		EnableItem(IDC_LINKTEXT_PROX_SERVER, TRUE);
	}
	else
	{
		SetItemCheck(IDC_CHECK_PROX_SERVER, FALSE);
		EnableItem(IDC_LINKTEXT_PROX_SERVER, FALSE);
	}
	return;
}
void CBeikeSafeSettingDlg::_SaveBkSysOptSettings()//系统优化的设置 hub
{
	WriteSettings();
	return;
}
void CBeikeSafeSettingDlg::_SaveBkSoftMgrSettings()//软件管理的设置
{
	if (m_bBkSoftMgrSettingChanged)
	{
		SaveSoftMgrSettings();
	}	
	return;
}
void CBeikeSafeSettingDlg::SaveSafeKwsSettings()//网盾的设置
{
	KwsSetting aSetting;
	//之下是广告过滤的
	BOOL bCheck = GetItemCheck(IDC_CHECK_ADFILTER_CUE);
	aSetting.SetupTipSpannerEnable( bCheck );
	bCheck = GetItemCheck(IDC_CHECK_ADFILTER_LOG);
	aSetting.SetAntiAdvLogEnable( bCheck );

	//之下是下载保护的
	/*bCheck = GetItemCheck(IDC_CHECK_DOWNLOAD_LIS);
	aSetting.SetAutoScanEnable( bCheck );
	IPCClient ipcClient;
	NotifySettingChange setting = {!bCheck};
	unsigned int nSize = sizeof(setting);
	ipcClient.RunCall(IPC_PROC_OnNotifySettingChange, &setting, nSize);*/

	if (aSetting.IsAutoScanEnable())
	{
		bCheck = GetItemCheck(IDC_CHECK_DOWNLOAD_AUTOCLEAN);
		aSetting.SetAutoCleanVirus(bCheck);

		bCheck = GetItemCheck(IDC_CHECK_DOWNLOAD_SAFE_NOCLUE);
		aSetting.SetShowSafePop( !bCheck );

		bCheck = GetItemCheck(IDC_CHECK_DOWNLOAD_UNKNOWN_AUTOSUBMIT);
		aSetting.SetShowGrayPop( !bCheck );
	}
	return;
}
void CBeikeSafeSettingDlg::_SaveBkKwsSettings()//网盾的设置
{
	if (m_bBkSafeKwsSettingChanged)
	{
		SaveSafeKwsSettings();
	}
	return;
}
BOOL CBeikeSafeSettingDlg::doProxServerApply(ATL::CString strFilePath, int nType)
{
	TCHAR			sUser[256] = {0};
	TCHAR			sPass[256] = {0};
	TCHAR			sHost[256] = {0};		
	int				nPort;
	//读配置文件
	CIniFile iniFile(strFilePath);
	KXE_CLOUD_PROXY_INFO ProxInfo;
	HRESULT hr;
	if (nType == 0)//不需要使用代理
	{
		ProxInfo.emProxyType = EM_KXE_NETPROXY_TYPE_NOPROXY;
		hr = m_kwsScanner.SetCloudProxyInfo(ProxInfo);		
	}
	else
	{
		if (nType == 1)
			ProxInfo.emProxyType = EM_KXE_NETPROXY_TYPE_HTTP;
		else if (nType == 2)
			ProxInfo.emProxyType = EM_KXE_NETPROXY_TYPE_SOCKET5;
		iniFile.GetStrValue(L"proxy",	L"host",		sHost,	sizeof(sHost));
		iniFile.GetIntValue(L"proxy",	L"port",		nPort);	
		iniFile.GetStrValue(L"proxy",	L"user",		sUser,	sizeof(sUser));
		iniFile.GetStrValue(L"proxy",	L"password",	sPass,	sizeof(sPass));
		std::wstring strHost(sHost);
		std::wstring strUser(sUser);
		std::wstring strPass(sPass);
		ProxInfo.strProxyHost = strHost;
		ProxInfo.dwProxyPort = nPort;
		ProxInfo.strProxyUsername = strUser;
		ProxInfo.strProxyPassword = strPass;
		hr = m_kwsScanner.SetCloudProxyInfo(ProxInfo);
	}
	if (hr != 0)//设置失败
	{
		return FALSE;
	}
	
	return TRUE;
}
void CBeikeSafeSettingDlg::_SaveBkSafePorxServer()//代理服务器的保存
{
	//判断check是否勾选
	ATL::CString strFilePath;	
	if (m_bProxChanged)
	{
		CAppPath::Instance().GetLeidianAppPath(strFilePath);
		strFilePath.Append(L"\\Cfg\\bksafe.ini");
		CIniFile IniFile(strFilePath);
		if (!GetItemCheck(IDC_CHECK_PROX_SERVER))//没有勾选
		{
			if (doProxServerApply(strFilePath, 0))//设置成功
			{
				IniFile.SetIntValue(TEXT("proxy"),TEXT("type"),		0);

				CString strIniPath;
				CAppPath::Instance().GetLeidianCfgPath(strIniPath);
				strIniPath += _T("\\pubconfig.ini");
				
				if( ::PathFileExists(strIniPath) )
					::DeleteFileW(strIniPath);
			}			
		}
		else//勾选的情况
		{		
			int nType = _LoadBkSafePorxServer();
			if (nType <= 0)//不存在代理的情况
			{
				if (doProxServerApply(strFilePath, 1))//设置成功
				{
					IniFile.SetIntValue(TEXT("proxy"),TEXT("type"),		1);//默认的值为http代理
				}
				
			}
			else
			{
				doProxServerApply(strFilePath, nType);
			}
		}

		GetParent().PostMessage( MSG_APP_PROXY_CHANGE, NULL, NULL );
	}	
	
	return;
}
void CBeikeSafeSettingDlg::_SaveSettings()
{
    _SaveBkSafeSettings();//综合设置   电脑体检
    _SaveTraySettings();//综合设置中的升级方式  免打扰设置
    _SaveSvcSettings();//查杀木马  云安全  查杀策略  发现木马程序
    _SaveIEFixSettings();//已经作废的
	_SaveBkSysOptSettings();//系统优化设置
	_SaveBkSoftMgrSettings();//软件管理设置
	_SaveBkKwsSettings();//网盾设置
	_SaveBkSafePorxServer();//代理服务器的保存
	return;
}

void CBeikeSafeSettingDlg::_LoadBkSafeSettings()
{
	DWORD dwMethod = BKSafeConfig::GetAutoExam();
	if ( dwMethod == 0 )
	{
		SetItemCheck(IDC_CHK_SETTING_BKSAFE_AUTO_EXAM, TRUE);
	}
	else if ( dwMethod == 1 )
	{
		SetItemCheck(IDC_CHK_SETTING_BKSAFE_DAY_EXAM, TRUE);
	}
	else if ( dwMethod == 2 )
	{
		SetItemCheck(IDC_CHK_SETTING_BKSAFE_MANUAL_EXAM, TRUE);
	}
}

void CBeikeSafeSettingDlg::_LoadTraySettings()
{
    CSafeMonitorTrayShell tray;

    /* 
	BOOL bTrayAutorun = tray.GetAutorunTray();  

    SetItemCheck(IDC_CHK_SETTING_TRAY_AUTORUN, bTrayAutorun); 
	*/

	SetItemCheck(IDC_CHK_SETTING_RUNSTARTUP,      tray.GetAutorunTray());  /* 是否开机启动 */
    SetItemCheck(IDC_CHK_SETTING_TRAY_GAMEBYPASS, tray.GetGameBypass());   /* 是否免打扰 */

	DWORD dwUpdateMethod = tray.GetUpdateMethod();
	if ( dwUpdateMethod == 0 )
	{
		SetItemCheck(IDC_CHK_SETTING_TRAY_AUTO_UPDATE, TRUE);
	}
	else
	{
		SetItemCheck(IDC_CHK_SETTING_TRAY_ASK_UPDATE, TRUE);
	}

	/*
	if ( bTrayAutorun )
	{
		BOOL bProtectionTrunOnAll = tray.GetMonitorRunCount() == tray.GetMonitorCount();

		SetItemVisible(IDC_TXT_SETTING_TRAY_AUTORUN_WARNING1, FALSE);
		SetItemVisible(IDC_TXT_SETTING_TRAY_AUTORUN_WARNING2, !bProtectionTrunOnAll);
		SetItemVisible(IDC_LBL_SETTING_TRAY_PROTECTION_TRUN_ON, !bProtectionTrunOnAll);
	}
	else
	{
		SetItemVisible(IDC_TXT_SETTING_TRAY_AUTORUN_WARNING1, TRUE);
	}
	*/

	SetItemCheck(IDC_CHK_SETTING_TRAY_SAFEPASS, tray.GetSafeBypass());

    m_bTraySettingChanged = FALSE;
}

void CBeikeSafeSettingDlg::_LoadSvcSettings()
{
    SetItemCheck(IDC_CHK_SETTING_SVC_JOIN_BEIKECLOUD, BKSafeConfig::GetAutoReportUnknown());

	SetItemCheck(IDC_CHK_SETTING_AUTO_EXPANDSCAN, BKSafeConfig::GetAutoExpandScan());

    BOOL bSet = BKSafeConfig::GetAutoClean();

    SetItemCheck(bSet ? IDC_CHK_SETTING_SVC_AUTO_CLEAN : IDC_CHK_SETTING_SVC_MANUAL_CLEAN, TRUE);

    m_bSvcSettingChanged = FALSE;
}

void CBeikeSafeSettingDlg::_LoadIEFixSettings()
{
    SetItemCheck(IDC_CHK_SETTING_IEFIX_RIGHTMENU   , BKSafeConfig::GetIEFixRightMenu());
    SetItemCheck(IDC_CHK_SETTING_IEFIX_TOOLBARICON , BKSafeConfig::GetIEFixToolbarIcon());
    SetItemCheck(IDC_CHK_SETTING_IEFIX_THIRDTOOLBAR, BKSafeConfig::GetIEFixThirdToolbar());
    SetItemCheck(IDC_CHK_SETTING_IEFIX_BHO         , BKSafeConfig::GetIEFixBHO());
    SetItemCheck(IDC_CHK_SETTING_IEFIX_SAFEWEB     , BKSafeConfig::GetIEFixSafeWeb());
    SetItemCheck(IDC_CHK_SETTING_IEFIX_PROTOCOL    , BKSafeConfig::GetIEFixProtocol());
    SetItemCheck(IDC_CHK_SETTING_IEFIX_SEARCHENG   , BKSafeConfig::GetIEFixSearchEngine());

    m_bIEFixSettingChanged = FALSE;
}
//hub 免打扰对话框 
void CBeikeSafeSettingDlg::OnSafePassDlg()
{
	CBeikeSafeSafepassSettingDlg dlg;//免打扰
	dlg.DoModal();
	return;
}

void CBeikeSafeSettingDlg::SaveExamSetting()
{
	DWORD dwMethod = 0;

	if ( GetItemCheck(IDC_CHK_SETTING_BKSAFE_AUTO_EXAM) )
	{
		dwMethod = 0;
	}
	else if ( GetItemCheck(IDC_CHK_SETTING_BKSAFE_DAY_EXAM) )
	{
		dwMethod = 1;
	}
	else if ( GetItemCheck(IDC_CHK_SETTING_BKSAFE_MANUAL_EXAM) )
	{
		dwMethod = 2;
	}

	BKSafeConfig::SetAutoExam(dwMethod);
}
void CBeikeSafeSettingDlg::_SaveBkSafeSettings()
{
    if (m_bBkSafeSettingChanged)
	{
		// FixME!! 保存新的开机体检设置 3.9
		SaveExamSetting();
	}
}



void CBeikeSafeSettingDlg::_SaveTraySettings()
{
    if (m_bTraySettingChanged)
    {
        CSafeMonitorTrayShell tray;

		SaveExamSetting();

        // tray.SetAutorunTray(GetItemCheck(IDC_CHK_SETTING_TRAY_AUTORUN));

		BOOL fEnableAutorun = GetItemCheck(IDC_CHK_SETTING_RUNSTARTUP);

		if (fEnableAutorun)
		{
			CRestoreRunner* pLogRunner = new CRestoreRunner;
			pLogRunner->DeleteItemFromLog(_T("KSafeTray"), KSRUN_TYPE_STARTUP, 590);
			delete pLogRunner;
		}

		tray.SetAutorunTray(fEnableAutorun);                               /* 保存开机启动设置 */
        tray.SetGameBypass(GetItemCheck(IDC_CHK_SETTING_TRAY_GAMEBYPASS)); /* 保存游戏免打扰设置 */
		tray.SetSafeBypass(GetItemCheck(IDC_CHK_SETTING_TRAY_SAFEPASS));   /* 保存不提示注册表修改设置 */

		::PostMessage(GetParent(), MSG_SYSOPT_REFRESH_ITEMLIST, fEnableAutorun, 0); /* 刷新开机启动列表 */

		DWORD dwUpdateMethod = 0;
		if ( GetItemCheck(IDC_CHK_SETTING_TRAY_AUTO_UPDATE) )
		{
			dwUpdateMethod = 0;
		}
		else if ( GetItemCheck(IDC_CHK_SETTING_TRAY_ASK_UPDATE) )
		{
			dwUpdateMethod = 1;
		}

		tray.SetUpdateMethod(dwUpdateMethod);
    }
}

void CBeikeSafeSettingDlg::_SaveSvcSettings()
{
    if (m_bSvcSettingChanged)
    {
        BKSafeConfig::SetAutoReportUnknown(GetItemCheck(IDC_CHK_SETTING_SVC_JOIN_BEIKECLOUD));
        BKSafeConfig::SetAutoClean(GetItemCheck(IDC_CHK_SETTING_SVC_AUTO_CLEAN));
		BKSafeConfig::SetAutoExpandScan(GetItemCheck(IDC_CHK_SETTING_AUTO_EXPANDSCAN));

        CBkUtility util;

        util.Initialize();
        util.SettingChange();
        util.Uninitialize();
    }
}

void CBeikeSafeSettingDlg::_SaveIEFixSettings()
{
    if (m_bIEFixSettingChanged)
    {
        BKSafeConfig::SetIEFixRightMenu(GetItemCheck(IDC_CHK_SETTING_IEFIX_RIGHTMENU));
        BKSafeConfig::SetIEFixToolbarIcon(GetItemCheck(IDC_CHK_SETTING_IEFIX_TOOLBARICON));
        BKSafeConfig::SetIEFixThirdToolbar(GetItemCheck(IDC_CHK_SETTING_IEFIX_THIRDTOOLBAR));
        BKSafeConfig::SetIEFixBHO(GetItemCheck(IDC_CHK_SETTING_IEFIX_BHO));
        BKSafeConfig::SetIEFixSafeWeb(GetItemCheck(IDC_CHK_SETTING_IEFIX_SAFEWEB));
        BKSafeConfig::SetIEFixProtocol(GetItemCheck(IDC_CHK_SETTING_IEFIX_PROTOCOL));
        BKSafeConfig::SetIEFixSearchEngine(GetItemCheck(IDC_CHK_SETTING_IEFIX_SEARCHENG));
    }
}

// 漏洞修复 
void CBeikeSafeSettingDlg::_Vulfix_Setting_Init()
{
	DWORD dwEditFlags = WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL;
	m_ctlEdit.Create( GetViewHWND(), NULL, NULL, dwEditFlags, WS_EX_NOPARENTNOTIFY, 111, NULL);	
    m_ctlEdit.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
	
	m_ctlVulEditLocalPath.Create( GetViewHWND(), NULL, NULL, dwEditFlags, WS_EX_NOPARENTNOTIFY, 1250, NULL);
	m_ctlVulEditLocalPath.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
	
	_Vulfix_Setting_Reset();
	_Vulfix_Setting_UpdateTitle();
	m_isVulRepairing = (theEngine && theEngine->m_isRepairing);
	if( m_isVulRepairing )
	{
		// 如果正在修复
		m_ctlEdit.EnableWindow( FALSE );
		EnableItem(IDC_BTN_SETTING_VULFIX_SELFOLDER, FALSE);
		EnableItem(IDC_BTN_SETTING_VULFIX_DELETEFILES, FALSE);		

		EnableItem(IDC_DIV_SETTING_VULFIX_SAVE_DIR,FALSE);
		EnableItem(IDC_CHK_SETTING_VULFIX_SAVE_DOWN,FALSE);
		EnableItem(IDC_CHK_SETTING_VULFIX_NO_SAVE,FALSE);
		EnableItem(111,FALSE);	

		// 
		EnableItem(1200,FALSE);	
		EnableItem(1201,FALSE);	
		EnableItem(1250,FALSE);	
		EnableItem(1203,FALSE);	
		EnableItem(1204,FALSE);	
	}
}

void CBeikeSafeSettingDlg::_Vulfix_Setting_Reset()
{
	m_bVulfixSettingChanged = FALSE;

	CString strPath;
	BKSafeConfig::Get_Vulfix_DownloadPath( strPath );
	m_ctlEdit.SetWindowText( strPath );

	BOOL bSave = BKSafeConfig::Get_Vulfix_SaveDownFile();
	if (bSave)
	{
		SetItemVisible(IDC_DIV_SETTING_VULFIX_SAVE_DIR,TRUE);
		SetItemCheck(IDC_CHK_SETTING_VULFIX_SAVE_DOWN,TRUE);
		SetItemCheck(IDC_CHK_SETTING_VULFIX_NO_SAVE,FALSE);
	}
	else
	{
		SetItemVisible(IDC_DIV_SETTING_VULFIX_SAVE_DIR,FALSE);
		SetItemCheck(IDC_CHK_SETTING_VULFIX_SAVE_DOWN,FALSE);
		SetItemCheck(IDC_CHK_SETTING_VULFIX_NO_SAVE,TRUE);
	}
	
	CString strLocalPath;
	BOOL bUseLocal = BKSafeConfig::Get_Vulfix_ProfileInt(_T("download"), _T("use_local"), 0);
	BOOL bDownIfNotExists = BKSafeConfig::Get_Vulfix_ProfileInt(_T("download"), _T("down_if_not_exists"), 1);
	BKSafeConfig::Get_Vulfix_ProfileString(_T("download"), _T("local_path"), strLocalPath);
	SetItemCheck(1204, bDownIfNotExists);
	SetItemCheck( bUseLocal ? 1201 : 1200, TRUE );
	_Vulfix_Update_DownloadSetting(bUseLocal);
	m_ctlVulEditLocalPath.SetWindowText(strLocalPath);
	if(!theEngine)
	{
		theEngine = new CVulEngine;	
		theEngine->m_fixLog.Load();//把源文件也加载进来
	}
	else
	{
		theEngine->m_fixLog.Load();//把源文件也加载进来
	}
		
}

BOOL IsValidVulLocalPath(LPCTSTR szPath)
{
	// 是否网络路径, 或者网络邻居路径
	if(_tcslen(szPath)>=2)
		return (isalpha(szPath[0]) && szPath[1]==_T(':'));
	return FALSE;
}

BOOL IsValidVulPath(LPCTSTR szPath)
{
	//return PathIsUNC(szPath) ? PathIsUNCServerShare(szPath) : (!PathIsRelative(szPath) && PathIsDirectory(szPath));
	if(PathIsUNC(szPath))
		return TRUE;
	else
		return (!PathIsRelative(szPath) && PathIsDirectory(szPath));
}

BOOL CBeikeSafeSettingDlg::_Vulfix_Setting_Save()
{
	BOOL bNeedReselect = FALSE;
	CString strPath;
	
	BOOL bSave = GetItemCheck(IDC_CHK_SETTING_VULFIX_SAVE_DOWN);
	BKSafeConfig::Set_Vulfix_SaveDownFile(bSave);
	
	CString strErrmsg;
	CString strOrig;
	BKSafeConfig::Get_Vulfix_DownloadPath( strOrig );
	m_ctlEdit.GetWindowText( strPath );
	if( strPath!=strOrig )
	{
        LPCTSTR szModuleName = BkString::Get(IDS_SETTING_500);
		if(!IsValidVulLocalPath(strPath))
		{
			strErrmsg.Format(BkString::Get(IDS_SETTING_501), szModuleName);
			if( IDYES==CBkSafeMsgBox2::ShowMultLine( strErrmsg, NULL, MB_YESNO | MB_ICONQUESTION, NULL, m_hWnd) )
			{
				bNeedReselect = TRUE;
			}
		}
		else 
		{
			if(!PathIsDirectory(strPath))
			{
				strErrmsg.Format(BkString::Get(IDS_SETTING_502), szModuleName);
				if( IDYES==CBkSafeMsgBox2::ShowMultLine( strErrmsg, NULL, MB_YESNO | MB_ICONQUESTION, NULL) )
				{
					CreateDirectoryNested( strPath );
					if( !PathIsDirectory( strPath ) )
					{
						strErrmsg.Format(BkString::Get(IDS_SETTING_503), szModuleName);
						CBkSafeMsgBox2::ShowMultLine( strErrmsg, NULL, MB_OK | MB_ICONWARNING);
						bNeedReselect = TRUE;
					}
				}
			}
			if(PathIsDirectory(strPath))
			{
				if(!theEngine)
					theEngine = new CVulEngine;

				theEngine->m_fixLog.MoveFiles( strOrig, strPath, m_hWnd);
				BKSafeConfig::Set_Vulfix_DownloadPath( strPath );
			}
		}
	}
	
	// 如果需要重新选择目录的话, 直接返回!
	if(bNeedReselect)
	{
		SetTabCurSel(IDC_TAB_SETTING, 2);
		m_ctlEdit.SetFocus();
		return !bNeedReselect;
	}
	
	// 使用本地补丁 ... 
	BOOL bUseLocal = GetItemCheck(1201);
	if(bUseLocal)
	{
		CString strLocalPath;
		m_ctlVulEditLocalPath.GetWindowText(strLocalPath);

		LPCTSTR szModuleName = BkString::Get(IDS_SETTING_504);
		if(IsValidVulPath(strLocalPath)) // 网络路径, 或者本地全路径
		{
			BKSafeConfig::Set_Vulfix_ProfileInt(_T("download"), _T("use_local"), bUseLocal);
			BKSafeConfig::Set_Vulfix_ProfileInt(_T("download"), _T("down_if_not_exists"), GetItemCheck(1204));
			BKSafeConfig::Set_Vulfix_ProfileString(_T("download"), _T("local_path"), strLocalPath);
		}
		else 
		{			
			strErrmsg.Format(BkString::Get(IDS_SETTING_505), szModuleName);
			if( IDYES==CBkSafeMsgBox2::ShowMultLine( strErrmsg, NULL, MB_YESNO | MB_ICONQUESTION, NULL, m_hWnd) )
			{
				bNeedReselect = TRUE;
			}
		}
	}
	else
	{
		BKSafeConfig::Set_Vulfix_ProfileInt(_T("download"), _T("use_local"), bUseLocal);
	}
	
	if(bNeedReselect)
	{
		SetTabCurSel(IDC_TAB_SETTING, 2);
		m_ctlVulEditLocalPath.SetFocus();
	}
	return !bNeedReselect;
}

void CBeikeSafeSettingDlg::_Vulfix_Setting_UpdateTitle()
{
	// 	CString strPath;
	// 	m_ctlEdit.GetWindowText( strPath );
	// 	INT64 sizeFolder = GetFolderSize( strPath, FALSE, IsBkDownloadFile );
	if(!theEngine)
		theEngine = new CVulEngine;

	INT64 sizeFolder = theEngine->m_fixLog.GetDownloadFilesSize();

	CString strSize;
	FormatSizeString( sizeFolder, strSize );
	SetItemText(105, strSize);
	SetItemVisible(115, sizeFolder>0);
}

void CBeikeSafeSettingDlg::_Vulfix_Setting_UpdateTitle(CString strFileName)
{
	if(!theEngine)
		theEngine = new CVulEngine;

	INT64 sizeFolder = theEngine->m_fixLog.GetDownloadFilesSize(strFileName);

	CString strSize;
	FormatSizeString( sizeFolder, strSize );
	SetItemText(105, strSize);
	SetItemVisible(115, sizeFolder>0);
}

void CBeikeSafeSettingDlg::OnBtn_Vulfix_SelFolder()
{
	CString sFolderPath, sFolderNew;
	m_ctlEdit.GetWindowText( sFolderPath );
	
	if(theEngine && theEngine->m_isRepairing)
	{
		CBkSafeMsgBox2::ShowMultLine(BkString::Get(IDS_SETTING_506), NULL, MB_OK | MB_ICONWARNING);
		return;
	}
	
	sFolderNew = sFolderPath;
	if( SelectFolderEx(sFolderNew, m_hWnd) )
	{
		m_bVulfixSettingChanged = TRUE;
		m_ctlEdit.SetWindowText( sFolderNew );	
		theEngine->m_fixLog.Load(sFolderNew);//先load一次
		_Vulfix_Setting_UpdateTitle(sFolderNew);
	}
}

void CBeikeSafeSettingDlg::OnBtn_Vulfix_OpenFolder()
{
	CString strPath;
	m_ctlEdit.GetWindowText( strPath );
	if( !PathIsDirectory(strPath) )
    {
        CString strErrmsg;
        LPCTSTR szModuleName = BkString::Get(IDS_SETTING_500);
        strErrmsg.Format(BkString::Get(IDS_SETTING_501), szModuleName);
        if( IDYES==CBkSafeMsgBox2::ShowMultLine( strErrmsg, NULL, MB_YESNO | MB_ICONQUESTION, NULL) )
            CreateDirectoryNested( strPath );
    }
	if( PathIsDirectory( strPath ) )
		ShellExecute(NULL, _T("open"), strPath, NULL, NULL, SW_SHOW);
	else
	{
		CString strMessage;
		strMessage.Format(BkString::Get(IDS_SETTING_507), strPath);
		CBkSafeMsgBox2::Show(strMessage, BkString::Get(IDS_SETTING_508), MB_OK | MB_ICONWARNING);
	}
}

void CBeikeSafeSettingDlg::OnBtn_Vulfix_DeleteAllFiles()
{	
	CString strFolder;
	m_ctlEdit.GetWindowText( strFolder );
	if( PathIsDirectory(strFolder) )
	{		
		CString strTitle;
		strTitle.Format(BkString::Get(IDS_SETTING_509), strFolder);
		if( IDYES!=CBkSafeMsgBox2::Show(strTitle, BkString::Get(IDS_SETTING_510), MB_YESNO | MB_ICONQUESTION) )
			return;
		
		if(!theEngine)
			theEngine = new CVulEngine;	
		theEngine->m_fixLog.CleanFiles( TRUE, strFolder );
	}
	else
	{
		CBkSafeMsgBox2::Show(BkString::Get(IDS_SETTING_511), NULL, MB_OK | MB_ICONWARNING);
	}
	_Vulfix_Setting_UpdateTitle();
}

void CBeikeSafeSettingDlg::OnBtn_Vulfix_Changed_DownloadSetting()
{
	BOOL bUseLocal = GetItemCheck(1201);
	_Vulfix_Update_DownloadSetting(bUseLocal);
}

void CBeikeSafeSettingDlg::OnBtn_Vulfix_SelectLocalPath()
{
	CString sFolderPath, sFolderNew;
	m_ctlVulEditLocalPath.GetWindowText( sFolderPath );	
	sFolderNew = sFolderPath;
	if( SelectFolderEx(sFolderNew, m_hWnd) )
	{
		m_bVulfixSettingChanged = TRUE;
		m_ctlVulEditLocalPath.SetWindowText( sFolderNew );
	}
}

void CBeikeSafeSettingDlg::_Vulfix_Update_DownloadSetting( BOOL bEnableBtnsForLocal )
{
	m_ctlVulEditLocalPath.EnableWindow(bEnableBtnsForLocal);
	EnableItem(1250, bEnableBtnsForLocal);
	EnableItem(1203, bEnableBtnsForLocal);
	EnableItem(1204, bEnableBtnsForLocal);
}

void CBeikeSafeSettingDlg::OnBkSafeSettingChanged()
{
    m_bBkSafeSettingChanged = TRUE;
}

void CBeikeSafeSettingDlg::OnBkSafeAndSvcSettingChanged()
{
    m_bBkSafeSettingChanged = TRUE;
    m_bSvcSettingChanged = TRUE;
}

void CBeikeSafeSettingDlg::OnClickWndOpt()
{
	
}

void CBeikeSafeSettingDlg::OnTraySettingAutorun()
{
	BOOL bCheck = GetItemCheck(IDC_CHK_SETTING_TRAY_AUTORUN);

	if ( bCheck )
	{
		CSafeMonitorTrayShell tray;
		BOOL bProtectionTrunOnAll = tray.GetMonitorRunCount() == tray.GetMonitorCount();

		SetItemVisible(IDC_TXT_SETTING_TRAY_AUTORUN_WARNING1, FALSE);
		SetItemVisible(IDC_TXT_SETTING_TRAY_AUTORUN_WARNING2, !bProtectionTrunOnAll);
		SetItemVisible(IDC_LBL_SETTING_TRAY_PROTECTION_TRUN_ON, !bProtectionTrunOnAll);
	}
	else
	{
		SetItemVisible(IDC_TXT_SETTING_TRAY_AUTORUN_WARNING1, TRUE);
		SetItemVisible(IDC_TXT_SETTING_TRAY_AUTORUN_WARNING2, FALSE);
		SetItemVisible(IDC_LBL_SETTING_TRAY_PROTECTION_TRUN_ON, FALSE);
	}

    m_bTraySettingChanged = TRUE;
}

/**
*  开机启动选框事件
*/
void CBeikeSafeSettingDlg::OnTraySettingStartupRun()
{
	BOOL fCheck = GetItemCheck(IDC_CHK_SETTING_RUNSTARTUP);

	if (!fCheck) /* 如果取消开机启动，则弹窗提示 */
	{
		if (CBkSafeMsgBox2::ShowMultLine(
			HIT_STARTUPRUN, 
			L"金山卫士", 
			MB_ICONWARNING|MB_OKCANCEL) != IDOK)
		{
			SetItemCheck(IDC_CHK_SETTING_RUNSTARTUP, TRUE);
			return;
		}
	}
    m_bTraySettingChanged = TRUE;
}

void CBeikeSafeSettingDlg::OnTraySettingChanged()
{
    m_bTraySettingChanged = TRUE;
}

void CBeikeSafeSettingDlg::OnTrayProtectionTrunOn()
{
	CSafeMonitorTrayShell shell;

	shell.SetAllMonitorRun(TRUE);

	OnTraySettingAutorun(); // 刷新界面
}

void CBeikeSafeSettingDlg::OnIEFixSettingChanged()
{
    m_bIEFixSettingChanged = TRUE;
}

void CBeikeSafeSettingDlg::OnSvcSettingChanged()
{
    m_bSvcSettingChanged = TRUE;
}

void CBeikeSafeSettingDlg::OnBtnSettingShowQuarantineDlg()
{
// 	CBeikeSafeVirScanLogDlg dlg;
// 
// 	dlg.DoModal();
	if (m_dlg)
	{
		//调用查杀历史记录
		m_dlg->ShowVirusLog();
	}
	

    m_bBkSafeSettingChanged = TRUE;
}

void CBeikeSafeSettingDlg::OnChkSettingVulfixNosave()
{
	m_bVulfixSettingChanged = TRUE;
	SetItemVisible(IDC_DIV_SETTING_VULFIX_SAVE_DIR,FALSE);
}

void CBeikeSafeSettingDlg::OnChkSettingVulfixSaveDown()
{
	m_bVulfixSettingChanged = TRUE;
	SetItemVisible(IDC_DIV_SETTING_VULFIX_SAVE_DIR,TRUE);
}

CBkNavigator* CBeikeSafeSettingNavigator::OnNavigate(CString &strChildName)
{
    int nPage = SettingPageCommon;

    if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_PAGE_COMMON))
    {
        nPage = SettingPageCommon;
		DoModal(nPage);
    }
    else if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_PAGE_VIRSCAN))
    {
        nPage = SettingPageVirScan;
		DoModal(nPage);
    }
//     else if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_PAGE_VULSCAN))
//     {
//         nPage = SettingPageVulScan;
//     }
    else if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_PAGE_PROTECTION))
    {
        nPage = SettingPageProtection;
		DoModal(nPage);
    }
//     else if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_PAGE_IEFIX))
//     {
//         nPage = SettingPageIEFix;
//     }
	else if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_PAGE_SYSOPT))//hub  系统优化
	{
		nPage = SettingPageSysopt;
		DoModal(nPage);
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_PAGE_KWS))//hub  网盾页面设置
	{
		nPage = SettingPageKws;
		DoModal(nPage);
	}
	else if (strChildName.Find(BKSFNS_SETTING_PAGE_SOFTMGR) != -1)//hub 软件管理
	{
		nPage = SettingPageSoftMgr;

		int nSubPage = -1;
		CString strCmd;
		int nSpliter = strChildName.Find(L':');
		if (-1 != nSpliter)
		{ 
			strCmd = strChildName.Mid(nSpliter + 1);
			if (0 == strCmd.CompareNoCase(BKSFNS_SETTING_PAGE_SOFTMGR_UPDATE))
				nSubPage = 1;
		}
		DoModal(nPage, ::GetActiveWindow(), nSubPage);
	}

    //DoModal(nPage);

    return NULL;
}

UINT_PTR CBeikeSafeSettingNavigator::DoModal(int nPage/* = SettingPageCommon*/, HWND hWndParent/* = ::GetActiveWindow()*/, int nSubPage /*= -1*/)
{
	UINT_PTR uRet = 0;
	if (!m_bHasDoModal)
	{
		m_bHasDoModal = TRUE;
		CBeikeSafeSettingDlg dlg;
		uRet = dlg.DoModal(nPage, hWndParent, m_pDlg, nSubPage);
		m_bHasDoModal = FALSE;	
		if (dlg.BkSafeSettingChanged())
			m_pDlg->SettingChanged();
	}

    return uRet;
}

void CBeikeSafeSettingDlg::OnLblSettingWhiteList()
{
//     CBeikeSafeVirScanWhiteListDlg dlg;
// 
//     dlg.DoModal();
	if (m_dlg)
	{
		//调用查杀历史记录
		m_dlg->ShowVirusTrustList();
	}
}

void CBeikeSafeSettingDlg::OnClickNetRemindDay( void )
{
	
}

void CBeikeSafeSettingDlg::OnClickNetRemindMonth( void )
{

}

void CBeikeSafeSettingDlg::OnClickCheckDay( void )
{
	BOOL bCheckDay = GetItemCheck(IDC_CHECK_REMIND_DAY_TEXT);
	if (!bCheckDay)
	{
		SetItemAttribute(IDC_CHECK_REMIND_DAY_TEXT, "crtext", "808080");
		SetItemAttribute(IDC_CHECK_REMIND_DAY_TEXT2, "crtext", "808080");
		SetItemAttribute(IDC_CHECK_REMIND_DAY_REAMIND, "crtext", "808080");
		m_editDayLimit.EnableWindow(FALSE);
	}
	else
	{
		SetItemAttribute(IDC_CHECK_REMIND_DAY_TEXT, "crtext", "000000");
		SetItemAttribute(IDC_CHECK_REMIND_DAY_TEXT2, "crtext", "000000");
		SetItemAttribute(IDC_CHECK_REMIND_DAY_REAMIND, "crtext", "000000");
		m_editDayLimit.EnableWindow(TRUE);
	}
}

void CBeikeSafeSettingDlg::OnClickCheckMonth( void )
{
	BOOL bCheckMonth = GetItemCheck(IDC_CHECK_REMIND_MONTH_TEXT);

	if (!bCheckMonth)
	{
		SetItemAttribute(IDC_CHECK_REMIND_MONTH_TEXT, "crtext", "808080");
		SetItemAttribute(IDC_CHECK_REMIND_MONTH_TEXT2, "crtext", "808080");
		SetItemAttribute(IDC_CHECK_REMIND_MON_REAMIND, "crtext", "808080");
		m_editMonthLimit.EnableWindow(FALSE);
	}
	else
	{
		SetItemAttribute(IDC_CHECK_REMIND_MONTH_TEXT, "crtext", "000000");
		SetItemAttribute(IDC_CHECK_REMIND_MONTH_TEXT2, "crtext", "000000");
		SetItemAttribute(IDC_CHECK_REMIND_MON_REAMIND, "crtext", "000000");
		m_editMonthLimit.EnableWindow(TRUE);
	}
}

int CBeikeSafeSettingDlg::ReBoot()
{
	CBkProcPrivilege privilege;

	if (!privilege.EnableShutdown())
		return -1;

	if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, 
		SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
		SHTDN_REASON_MINOR_HOTFIX |
		SHTDN_REASON_FLAG_PLANNED)) 
		return 0;

	return -1;
}
