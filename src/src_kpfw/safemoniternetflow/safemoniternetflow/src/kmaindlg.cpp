#include "stdafx.h"
//#include "netflowmonlist.h"
#include "kmaindlg.h"
#include "kflowtray.h"
#include "kpfw/kpfw_def.h"
#include "kpfw/msg_logger.h"
#include "common/KCheckInstance.h"
#include "kpfw/netflowformat.h"
#include "kws/ipc.h"
#include "kws/urlmondef.h"

//#include "netflowmonlistitemdata.h"
#include "resource.h"
#include "beikesafenetmonitorlimitdlg.h"
#include <algorithm>
#include "kcompare.h"
#include "src/beikesafemsgbox.h"
#include "communits/VerifyFileFunc.h"
#include "skylark2/slbase64util.h"
#include "softmgr/URLEncode.h"
#include "KFlowStatDefine.h"
#include "kflowstatlist.h"
#include "kstatlimitdlg.h"
#include "common/kopermemfile.h"
#include "ksafenetmonforbiddlg.h"

#define UPDATE_NETFlOW_MON_TIMER_ID		2
#define NETFLOW_ICON_PADDING			6

#define LoadUI		0

#define		LISTBOX_COLUM_NAME			0
#define		LISTBOX_COLUM_LEVEL			1
#define		LISTBOX_COLUM_DOWNLOAD		2
#define		LISTBOX_COLUM_UP			3
#define		LISTBOX_COLUM_LIMIT			4
#define		LISTBOX_COLUM_DOWN_TOTAL	5
#define		LISTBOX_COLUM_UP_TOTAL		6
#define		LISTBOX_COLUM_PID			7
#define		LISTBOX_COLUM_UP_DOWN_TOTAL 8
#define MAIN_APP_NAME		_T("KSafe.exe")

// ELAPSE TIME 表示多久获取一次数据
#define ELAPSE_TIME_BY_SECOND		    1*1000
#define HIGHT_ITEM_MAX					100
#define HIGHT_ITEM_MIN					50


KMainDlg::~KMainDlg()
{
	if (NULL != m_pNetMonitorListBox)
		delete m_pNetMonitorListBox;
	m_pNetMonitorListBox = NULL;

	_ResetArray();

	if (NULL != m_pNetStatListBox)
	{
		delete m_pNetStatListBox;
		m_pNetStatListBox = NULL;
	}

	if (m_pShowStatLock != NULL)
	{
		delete m_pShowStatLock;
		m_pShowStatLock = NULL;
	}

	if (KOperMemFile::Instance().IsInitHad())
	{
		KOperMemFile::Instance().SetWaitMoniterOpen(0);
		KOperMemFile::Instance().Uninit();	
	}

//	m_fluxStatRead.Uninit();
	//::DeleteCriticalSection(&m_Lock);
}


void KMainDlg::_ReleaseThread()
{
	if (m_hEventRptThreadExit)
	{
		::SetEvent(m_hEventRptThreadExit);
	}
	if (m_hThreadReport)
	{
		if (::WaitForSingleObject(m_hEventRptThreadExit, 5000) == WAIT_TIMEOUT)
		{
			if (m_bRptThreadWorking)
			{
				::TerminateThread(m_hThreadReport, 0);
			}
		}
		::CloseHandle(m_hThreadReport);
		m_hThreadReport = NULL;
	}
	if (m_hEventRptThreadExit)
	{
		CloseHandle(m_hEventRptThreadExit);
		m_hEventRptThreadExit = NULL;
	}
}

void KMainDlg::BtnClose( void )
{
	_ReleaseThread();
	EndDialog(0);
}

void KMainDlg::_SetAccessNetCount(IN int nCount)
{
	if (!m_bNetMonEnabled)
	{
		FormatRichText(DEFACCESSNETCOUNT, BkString::Get(Str_NetMonDisabledHint), nCount);
	}
	else
	{
		FormatRichText(DEFACCESSNETCOUNT, BkString::Get(DefString1), nCount);
	}
}

CString KMainDlg::_GetNeedShowData( IN ULONGLONG uData )
{
	CString strRet	= _T("0KB");

	NetFlowToString(uData, strRet);

	return strRet;
}

void KMainDlg::_SetDownAndUpdateSum( IN ULONGLONG uDownData, IN ULONGLONG uUpdata )
{
	CString strShowDownSum = _GetNeedShowData(uDownData);
	CString strShowUpdataSum = _GetNeedShowData(uUpdata);

	FormatItemText(DEF_DOWNLOAD_SUM, BkString::Get(Str_Download_Sum), strShowDownSum);
	FormatItemText(DEF_UPLOAD_SUM, BkString::Get(Str_Upload_Sum), strShowUpdataSum);
}

void KMainDlg::_SetDownSpeed( IN ULONGLONG uDownSpeed )
{
	if (!GetNetMonIsEnabled())
	{
		FormatItemText(DEFDOWNSPEED,  BkString::Get(DefString3), BkString::Get(Str_NetMonDisabled));
		return;
	}
	CString strShowDownSpeed = _GetNeedShowData(uDownSpeed);
#if 0
	strShowDownSpeed = L"999.99KB";
#endif
	strShowDownSpeed.Append(L"/S");

	FormatItemText(DEFDOWNSPEED, BkString::Get(DefString3), strShowDownSpeed);
}

void KMainDlg::_SetUpSpeed( IN ULONGLONG uUpSpeed )
{
	if (!GetNetMonIsEnabled())
	{
		FormatItemText(DEFUPSPEED, BkString::Get(DefString4), BkString::Get(Str_NetMonDisabled));
		return;
	}

	CString strShowUpSpeed = _GetNeedShowData(uUpSpeed);
#if 0
	strShowUpSpeed = L"999.99KB";
#endif
	strShowUpSpeed.Append(L"/S");

	FormatItemText(DEFUPSPEED, BkString::Get(DefString4), strShowUpSpeed);
}


BOOL KMainDlg::OnInitDialog( CWindow /*wndFocus*/, LPARAM /*lInitParam*/ )
{
	if (KOperMemFile::Instance().Init() == S_OK)
	{
		KOperMemFile::Instance().SetWaitMoniterOpen(1);	
		_InitFlowRemindInfo();
	}

	SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_BEIKESAFE)));
	SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_SMALL)), FALSE);

	m_enumProcessMode = enumProcessModeHasNetFlow;

	KCheckInstance::Instance()->CfgFirstInstance(NULL, this->m_hWnd, FALSE);

	KNetFluxCacheReader reader;
	if (SUCCEEDED(reader.Init()))
		reader.GetProcessesFluxInfo(m_FluxSys, &m_processInfoList, m_enumProcessMode);
//	KNetFluxCacheReader::Instance().GetSystemNetFlow(m_FluxSys);
	_DisableDelayScan();

	// 	//加入任务栏右键最大化置灰
	// 	LONG lSys = GetWindowLong(GWL_STYLE);
	// 	lSys &= ~(WS_MAXIMIZEBOX);
	// 	SetWindowLong(GWL_STYLE, lSys);

	KFlowTray prot_shell(TRUE);
	prot_shell.ShellTray();

	//	CListBoxData::GetDataPtr()->SetProviderFunc(NET_MONITOR_219, &KMainDlg::_UpdateNetFlowSummaryWnd);

	_ShowNetMintorRemindDlg(TRUE);

	_InitNetMonitorListBox();
	_InitStatList();


	m_uTimer = SetTimer(ID_TIMER_UPDATE_NETFlOW_MON, UPDATE_NETFLOW_MON_INTERVAL, NULL);

	SetTimer(ID_TIMER_REFRESH_FLOATWND_STATUS, 500, NULL);

	//_SetAccessNetCount(0);
	_SetDownAndUpdateSum(0.0, 0.0);
	_SetDownSpeed(0.0);
	_SetUpSpeed(0.0);

	_InitFloatWndSwitch();

	m_hEventExit = ::CreateEvent(
		NULL,
		FALSE,
		FALSE,
		EVENT_NETMON_DLG_EXIT
		);

	m_hEventChangeFlowatWndDisplayStatusText =::CreateEvent(
		NULL,
		FALSE,
		FALSE,
		EVENT_NETMON_DLG_FLOATWND_DISPLAY_STATUS_TEXT
		);


	SetTimer(ID_TIMER_CHECK_EXIT, CHECK_EXIT_INTERVAL);

	_InitNetMonSwitch();

	m_bRptThreadWorking = FALSE;
	m_hEventRptThreadExit = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (m_nCurShowType == enumQueryTypeEveryMonth)
		OnClickShowStatMonths();

	return TRUE;
}

BOOL KMainDlg::_CheckForceDisableDriver()
{
	BOOL bResult = FALSE;
	WCHAR bufPath[MAX_PATH] = {0};
	::GetModuleFileName(NULL, bufPath, MAX_PATH);
	::PathRemoveFileSpecW(bufPath);
	::PathAppend(bufPath, TEXT("AppData\\msg.ini"));
	int nNotify = GetPrivateProfileIntW(TEXT("netmon"), TEXT("forcedisabletdi"), 0, bufPath);
	if (nNotify)
	{
		CBkSafeMsgBoxForceDisableTDI dlg;
		int result = dlg.ShowMsg(NULL, BkString::Get(STR_DRIVER_FAILED_MSGBOX_CAPTION),NULL, MB_OK|MB_ICONEXCLAMATION, NULL, m_hWnd);
		if (ID_BUTTON_FEEDBACK == result)
		{
			::ShellExecute(m_hWnd, L"open", L"http://bbs.ijinshan.com/forum-18-1.html", NULL, NULL, SW_SHOW);
		}

		return TRUE;
	}
	return FALSE;
}

BOOL KMainDlg::CheckDriver()
{
	if (_CheckDriverIsFailed())
	{
		if (_CheckTdxDriver())
			return FALSE;

		if (_CheckHasUnCompatibleDriver())
			return FALSE;

		if (_CheckHasUnCompatibleSofware())
			return FALSE;

		if (_CheckForceDisableDriver())
			return FALSE;

		if (_CheckDriverVersion())
			return FALSE;

		CBkSafeMsgBoxDriverFailed driverFailedDlg;
		int result = driverFailedDlg.ShowMsg(NULL, BkString::Get(STR_DRIVER_FAILED_MSGBOX_CAPTION),NULL, MB_OK|MB_ICONEXCLAMATION, NULL, m_hWnd);
		if (ID_BUTTON_OEPNNOW == result)
		{
			// ::ShellExecute(m_hWnd, L"open", L"http://bbs.ijinshan.com", NULL, NULL, SW_SHOW);
			INT nCount = 0;
			CWaitCursor _cursor;
			do 
			{
				_OpenFlux();
				::Sleep(100);
				nCount++;

//				KNetFluxCacheReader netflowCacheReader;
				KNetFluxCacheReader reader;
				if (SUCCEEDED(reader.Init()))
					break;

				// 启动完成，那么需要再次检查一下
				if (_CheckTdxDriver())
					return FALSE;

				if (_CheckHasUnCompatibleDriver())
					return FALSE;

				if (_CheckHasUnCompatibleSofware())
					return FALSE;

				if (_CheckForceDisableDriver())
					return FALSE;

				if (_CheckDriverVersion())
					return FALSE;

				// 如果5秒还没有准备好，说明启动失败
				if (nCount > 50)
				{
					CBkSafeMsgBox msg;
					msg.ShowMsg(BkString::Get(46), BkString::Get(44), NULL, MB_OK, NULL, m_hWnd);
					return FALSE;
					break;
				}
			} while (1);
		}
		else
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL KMainDlg::_CheckDriverVersion()
{
	ULONG nError = 0;
	DWORD nType = 0;
	DWORD nSize = sizeof(nError);
	HRESULT hResult = SHGetValueW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\kmodurl", 
		L"TdiDriverVersionFailed", &nType, (PVOID)&nError, &nSize);

	if( hResult != ERROR_SUCCESS )
		return 0;

	if ((nSize == sizeof(ULONG)) && (nType == REG_BINARY))
	{
		if (nError)
		{
			// 发现是版本不对引起的
			CBkSafeMsgBoxFindDriverVersionError dlg;
			int result = dlg.ShowMsg(NULL, BkString::Get(STR_DRIVER_FAILED_MSGBOX_CAPTION),NULL, MB_OK|MB_ICONEXCLAMATION, NULL, m_hWnd);
			if (ID_BUTTON_REBOOTNOW == result)
			{
				HANDLE hToken; 
				TOKEN_PRIVILEGES tkp; 

				if (!OpenProcessToken(GetCurrentProcess(), 
					TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
					return( TRUE ); 

				LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
					&tkp.Privileges[0].Luid); 
				tkp.PrivilegeCount = 1;  
				tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

				AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
					(PTOKEN_PRIVILEGES)NULL, 0); 

				if (GetLastError() != ERROR_SUCCESS) 
					return TRUE; 

				if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, 
					SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
					SHTDN_REASON_MINOR_UPGRADE |
					SHTDN_REASON_FLAG_PLANNED)) 
					return TRUE; 
			}
			_ReleaseThread();
			EndDialog(0);
			return TRUE;
		}
	}
	return FALSE;
}

void KMainDlg::_OpenFlux()
{
	UrlMonIpcParam param = {0};
	param.m_nCallType = Ipc_NetFlux_ClearDumpStateAndStartUp;
	IPCClient client;
	unsigned int nsize = sizeof(param);
	client.RunCall(IPC_PROC_URLMON, &param, nsize);
}

void KMainDlg::OnSize(UINT nType, CSize size)
{
	CBkDialogImpl<KMainDlg>::OnSize(nType, size);
}

LRESULT KMainDlg::OnTimer( UINT timerId )
{
	if(!::IsWindowVisible(m_hWnd))
	{
		return TRUE;
	}

	if(timerId == ID_TIMER_UPDATE_NETFlOW_MON)
	{
		CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"NetDataEvent = 0x%x", m_hEventGetNetData);
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hEventGetNetData, 300))
		{
			// 不论监控是否关闭，实时刷新界面。避免出现结束进程后，界面列表中进程不消失
			// 以及关闭监控，开启监控等刷新不及时的bug。
			m_nNetFlowListRefreshCountAfterDisable = 0;
			KNetFluxCacheReader reader;
			if (SUCCEEDED(reader.Init()))
			{
				reader.GetProcessesFluxInfo(m_FluxSys, &m_processInfoList, m_enumProcessMode);
				//_UpdateNetMonitorListBox();
				_RefreshListBoxData(m_processInfoList);

				_UpdateNetFlowSummaryWnd();
			}
			else
			{
				netmon_log(L"GetProcessesFluxInfo failed.");
			}

		}
	}

	if (timerId == ID_TIMER_CHECK_EXIT	)
	{
		DWORD dwReCoder = WaitForSingleObject(m_hEventExit, 0);

		if (WAIT_OBJECT_0 == dwReCoder  )
		{
			ResetEvent(m_hEventExit);
			_ReleaseThread();
			EndDialog(0);
		}

		// 托盘显示的悬浮窗菜单有“关闭悬浮窗”功能，netmon同步要改状态
		dwReCoder = WaitForSingleObject(m_hEventChangeFlowatWndDisplayStatusText, 0);

		if (WAIT_OBJECT_0 == dwReCoder  )
		{
			_ShowFloatWnd(FALSE);
		}

	}

	if (timerId == ID_TIMER_UPDATE_STAT_INFO)
	{
		_ShowRemindInfo();
		_GetAndShowProcessInfo();
		_GetCurLogInfo((enumQueryType)m_nCurShowType, m_strQueryBegin, m_strQueryEnd);

		m_pNetStatListBox->ForceRefresh();
		m_pNetStatListBox->ResetMouseMoveMsg();
	}

	if (timerId == ID_TIMER_REFRESH_FLOATWND_STATUS)
	{
		m_bFloatWndIsOpen = _GetFloatWndDisplayStatus();
		_HideOrShowTip(m_bFloatWndIsOpen, FALSE);
	}


	return TRUE;
}

void KMainDlg::_AddNewData()
{

}

LRESULT KMainDlg::OnDestroy()
{
	netmon_log(L"Destory maindlg.");
	KillTimer(ID_TIMER_UPDATE_NETFlOW_MON);
	KillTimer(ID_TIMER_CHECK_EXIT);


	return TRUE;
}

void KMainDlg::_UpdateNetFlowSummaryWnd()
{   
	_SetNetMonSwitchHintText(m_bNetMonEnabled);
	_SetDownAndUpdateSum(m_FluxSys.nTotalRecv , m_FluxSys.nTotalSend);
	_SetDownSpeed(m_FluxSys.nRecvSpeed);
	_SetUpSpeed(m_FluxSys.nSendSpeed);
}

void KMainDlg::_OpenFloatWnd()
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

	if (FALSE == IsFloatWndStartWithTray())
		OpenFloatWnd();

	SetEvent(hEventOpenFloatWndDlg);
	CloseHandle(hEventOpenFloatWndDlg);

	netmon_log(L"SetEvent:EVENT_OPEN_FLOATWND_DLG");
}

BOOL KMainDlg::_GetFloatWndDisplayStatus()
{
	CString cfgFileNamePath = _GetFloatWndConfigFilePath();

	int defaultValue = 1;

	int result = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_DISPLAY_WITH_TRAY_KEY_NAME, defaultValue, cfgFileNamePath);
	if (0 == result)
		return result;

	defaultValue = 1;
	result = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_OPEN_NETMON_FLOATWND, defaultValue, cfgFileNamePath);

	return result == 1;
}


void KMainDlg::_CloseFloatWnd()
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
}


void KMainDlg::OnSysCommand( UINT nID, CPoint pt )
{
	if ( nID == SC_CLOSE )
	{
		_ReleaseThread();
		EndDialog(0);
	}
	else if (nID == SC_MAXIMIZE)
		return;
	else if (nID == SC_MINIMIZE)
		DefWindowProc();

	DefWindowProc();
}

LRESULT KMainDlg::ShowUI( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	//_ShowNetMintorRemindDlg();

	if (m_statListdlg.IsShowDlg())
		return S_OK;
	
	if (this->IsIconic())
		this->ShowWindow(SW_RESTORE);

	/*
	只用下面代码有bug 50202
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
	::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
	*/

	HWND	hForeWnd	=	NULL;
	DWORD	dwForeID;         
	DWORD	dwCurID;     

	hForeWnd	=	::GetForegroundWindow();
	dwCurID		=   ::GetCurrentThreadId();         
	dwForeID	=	::GetWindowThreadProcessId(hForeWnd, NULL);         
	::AttachThreadInput(dwCurID,  dwForeID, TRUE);         
	::ShowWindow(m_hWnd, SW_SHOWNORMAL);  
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
	::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW|SWP_NOACTIVATE );

	::SetForegroundWindow(m_hWnd);         
	::AttachThreadInput(dwCurID, dwForeID, FALSE);
	return 0;
}

void KMainDlg::_SetNetMonSwitchBtn( BOOL isEnabled )
{
	if (isEnabled)
	{
		SetItemAttribute(DEF_NETFLOW_SWITCH, "skin", "netmonenable");
		SetItemAttribute(DEF_NETFLOW_SWITCH2, "skin", "netmonenable");
	}
	else
	{
		SetItemAttribute(DEF_NETFLOW_SWITCH, "skin", "netmondisable");
		SetItemAttribute(DEF_NETFLOW_SWITCH2, "skin", "netmondisable");
	}
}

BOOL KMainDlg::GetNetMonIsEnabled()
{
	BOOL result = TRUE;
	KUrlMonCfgReader reader;

	if (SUCCEEDED(reader.Init()))
	{
		result = reader.GetEnableFluxStastic();
	}

	return result;
}

BOOL KMainDlg::_EnableNetMon( BOOL isEnable )
{
	BOOL result = FALSE;
	KUrlMonCfgReader reader;

	if (SUCCEEDED(reader.Init()))
	{
		result = reader.SetEnableFluxStastic(isEnable);
	}

	return result;
}

void KMainDlg::BtnNetMonSwitch( void )
{
	// 如果m_bNetMonEnabled为true，表示按下的按钮为关闭按钮
	// 如果m_bNetMonEnabled为false，表示按下的按钮为开启按钮

	//流量统计的修改
	

	if (m_bNetMonEnabled)
	{
		_SaveIsAutoCloseFloatwndWhenDisableNetmon(m_bFloatWndIsOpen);

		// 如果悬浮窗打开的话，直接关闭，并记录保证下次启动时打开
		if (m_bFloatWndIsOpen)
		{
			_ShowFloatWnd(FALSE);
		}
	}
	else
	{
		if (_GetIsAutoCloseFloatWndWhenDisableNetmon())
		{
			_ShowFloatWnd(TRUE);
		}
	}

	if (_EnableNetMon(!m_bNetMonEnabled))
	{
		_SetNetMonSwitchBtn(m_bNetMonEnabled);
		_SetNetMonSwitchHintText(!m_bNetMonEnabled);

		m_bNetMonEnabled = !m_bNetMonEnabled;
		OnTimer(ID_TIMER_UPDATE_NETFlOW_MON);
		//		m_NetFlowListCtrl.Refresh();
	}
	else
	{
		netmon_log(L"enable(%d) netmon failed! ", !m_bNetMonEnabled);
	}

	_SetStatFlowShow();
}

void KMainDlg::BtnFloatWndSwitch( void )
{
	//关闭或者打开
	netmon_log(L"CurrentFloatWnd Status:%d", m_bFloatWndIsOpen);

	_HideOrShowTip(m_bFloatWndIsOpen);
//	SetItemIntAttribute(NET_MONITOR_241, "sub", m_bFloatWndIsOpen);

//	_ShowFloatWnd(!m_bFloatWndIsOpen);
}
void KMainDlg::_SetNetMonSwitchHintText( BOOL isEnabled )
{
	if (isEnabled)
	{
		if (m_nCurShowStr == 0)
		{
			FormatRichText(DEFACCESSNETCOUNT, BkString::Get(Str_NetMonEnabledHint), this->m_processInfoList.size());
	//		SetItemAttribute(DEV_VIEW_DETAIL, "pos", "600,16");
		}
		else
		{
			FormatRichText(DEFACCESSNETCOUNT, BkString::Get(m_nCurShowStr), this->m_processInfoList.size());
		}
	}
	else
	{
		FormatRichText(DEFACCESSNETCOUNT, BkString::Get(Str_NetMonDisabledHint));
//		SetItemAttribute(DEV_VIEW_DETAIL, "pos", "575,16");
	}
}

void KMainDlg::_InitNetMonSwitch()
{
	m_bNetMonEnabled = GetNetMonIsEnabled();

	_SetNetMonSwitchBtn(!m_bNetMonEnabled);
	_SetNetMonSwitchHintText(m_bNetMonEnabled);

	m_nNetFlowListRefreshCountAfterDisable = 0;
}

void KMainDlg::_InitFloatWndSwitch()
{
	m_bFloatWndIsOpen = _GetFloatWndDisplayStatus();

	_HideOrShowTip(m_bFloatWndIsOpen, FALSE);
//	SetItemCheck(DEF_FLOATWND_SWITCH, m_bFloatWndIsOpen);
//	SetItemIntAttribute(NET_MONITOR_241, "sub", !m_bFloatWndIsOpen);
}

void KMainDlg::_ShowFloatWnd( BOOL isShow )
{
	if(isShow)
	{
		_OpenFloatWnd();
	}
	else
	{
		_CloseFloatWnd();
	}

//	SetItemCheck(DEF_FLOATWND_SWITCH, isShow);
	m_bFloatWndIsOpen = isShow;
	_HideOrShowTip(m_bFloatWndIsOpen, FALSE);

//	SetItemIntAttribute(NET_MONITOR_241, "sub", !m_bFloatWndIsOpen);
}

BOOL KMainDlg::_GetIsAutoCloseFloatWndWhenDisableNetmon()
{
	CString cfgFileNamePath = _GetFloatWndConfigFilePath();

	int defaultValue = 0;
	int result = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  
		FLOATWND_CONFIG_AUTO_CLOSE_FLOATWND_WHEN_DISABLE_NETMON, 
		defaultValue, 
		cfgFileNamePath);

	return 1 == result;
}

CString KMainDlg::_GetFloatWndConfigFilePath()
{
	CString cfgFileNamePath = FLOATWND_CONFIG_FILEPATH;

	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	ATL::CString trayPath;
	trayPath = buffer;
	cfgFileNamePath = trayPath + cfgFileNamePath;

	return cfgFileNamePath;
}

BOOL KMainDlg::_SaveIsAutoCloseFloatwndWhenDisableNetmon( BOOL isClose )
{
	CString cfgFileNamePath = _GetFloatWndConfigFilePath();

	TCHAR szValue[MAX_PATH];
	_stprintf_s(szValue, _T("%d"), isClose ? 1 : 0);
	return  ::WritePrivateProfileString(FLOATWND_CONFIG_SECTION_NAME,
		FLOATWND_CONFIG_AUTO_CLOSE_FLOATWND_WHEN_DISABLE_NETMON, 
		szValue, 
		cfgFileNamePath);
}

BOOL KMainDlg::_GetIsRememberAutoCloseFloatWndWhenDisableNetmon()
{
	CString cfgFileNamePath = _GetFloatWndConfigFilePath();

	int defaultValue = 0;
	int result = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  
		FLOATWND_CONFIG_REMEMBER_AUTO_CLOSE_FLOATWND_WHEN_DISABLE_NETMON, 
		defaultValue, 
		cfgFileNamePath);

	return 1 == result;
}

BOOL KMainDlg::_SaveIsRememberAutoCloseFloatWndWhenDisableNetmon( BOOL isRemember )
{
	CString cfgFileNamePath = _GetFloatWndConfigFilePath();

	TCHAR szValue[MAX_PATH];
	_stprintf_s(szValue, _T("%d"), isRemember ? 1 : 0);
	return  ::WritePrivateProfileString(FLOATWND_CONFIG_SECTION_NAME,
		FLOATWND_CONFIG_REMEMBER_AUTO_CLOSE_FLOATWND_WHEN_DISABLE_NETMON, 
		szValue, 
		cfgFileNamePath);
}

void KMainDlg::BtnMin( void )
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE | HTCAPTION, 0);
}

BOOL KMainDlg::_CheckDriverIsFailed()
{
	KUrlMonCfgReader urlmonCfgReader;
//	KNetFluxCacheReader netflowCacheReader;

	// 配置文件初始化成功，且流量信息初始化失败，则驱动加载失败。
	KNetFluxCacheReader reader;
	BOOL ret =  FAILED(reader.Init());
	if (ret)
	{
		netmon_log(L"_CheckDriverIsFailed  netflowCacheReader.init failed!");
	}
	return (SUCCEEDED(urlmonCfgReader.Init()) && ret);
}

BOOL KMainDlg::_IsDriverExist(LPCWSTR strDriverServiceName)
{
	if ( !strDriverServiceName )
		return FALSE;

	BOOL bRetCode = FALSE;
	SC_HANDLE hSvrMgr = NULL;
	SC_HANDLE hSvrSpe = NULL;
	SERVICE_STATUS_PROCESS ssStatus; 
	DWORD dwOldCheckPoint; 
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;

	SetLastError(0);

	hSvrMgr = ::OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
	if ( !hSvrMgr )
	{
		goto Exit0;
	}

	hSvrSpe = ::OpenService(hSvrMgr, strDriverServiceName, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
	if ( !hSvrSpe )
	{
		goto Exit0;
	}
	bRetCode = TRUE;

Exit0:
	if ( hSvrMgr )
	{
		CloseServiceHandle(hSvrMgr);
		hSvrMgr = NULL;
	}

	if ( hSvrSpe )
	{
		CloseServiceHandle( hSvrSpe );
		hSvrSpe = NULL;
	}

	return bRetCode;
}

#define AVG_TDI_DRIVER_NAME	L"AvgTdiX"

BOOL KMainDlg::_CheckHasUnCompatibleDriver()
{
	return FALSE;
	if (_IsDriverExist(AVG_TDI_DRIVER_NAME))
	{
		CBkSafeMsgBoxFindUnCompatibleDriver dlg;
		int result = dlg.ShowMsg(NULL, BkString::Get(STR_DRIVER_FAILED_MSGBOX_CAPTION),NULL, MB_OK|MB_ICONEXCLAMATION, NULL, m_hWnd);
		if (ID_BUTTON_FEEDBACK == result)
		{
			::ShellExecute(m_hWnd, L"open", L"http://bbs.ijinshan.com", NULL, NULL, SW_SHOW);
		}
		_ReleaseThread();
		EndDialog(0);
		return TRUE;	
	}
	return FALSE;
}


BOOL KMainDlg::_DisableDelayScan()
{
	BOOL result = TRUE;
	KUrlMonCfgReader reader;

	if (SUCCEEDED(reader.Init()))
	{
		result = reader.SetDisableDelayScan(TRUE);
	}

	return TRUE;
}

// 2.0 UI [11/29/2010 zhangbaoliang]
void KMainDlg::_InitNetMonitorListBox()
{
	m_pNetMonitorListBox = new CBkNetMonitorListBox;
	if (NULL == m_pNetMonitorListBox)
		return;

	//listbox
	m_pNetMonitorListBox->Create( GetViewHWND(), NET_MONITOR_201);
	m_pNetMonitorListBox->Load(IDR_BK_LISTBOX_NETMONITOR);
	m_pNetMonitorListBox->SetCanGetFocus(FALSE);
	_RefreshListBoxData(m_processInfoList);

	//menu
	m_hNetMonitorMenu = GetSubMenu(AtlLoadMenu(MAKEINTRESOURCE(IDR_MENU_NET_MONITOR)), 0);

}


LRESULT KMainDlg::OnListBoxGetDispInfo(LPNMHDR pnmh)
{
	if (NULL == m_pNetMonitorListBox)
		return E_POINTER;

	BKLBMGETDISPINFO* pdi = (BKLBMGETDISPINFO*)pnmh;

	if (CListBoxData::GetDataPtr()->GetMaxHeightSel() == pdi->nListItemID)
	{
		pdi->nHeight = HIGHT_ITEM_MAX;
		m_pNetMonitorListBox->SetItemVisible(NET_MONITOR_240, TRUE);
	}
	else
	{
		m_pNetMonitorListBox->SetItemVisible(NET_MONITOR_240, FALSE);
		pdi->nHeight = HIGHT_ITEM_MIN;
	}

	if (pdi->nListItemID >= m_mapProcNetData.GetCount())
		return E_FAIL;

	CString strValue = L"";
	KSProcessInfo ProcInfo ;

	DWORD dwStyle = m_pNetMonitorListBox->GetStyle();			
	if (FALSE == ::IsWindowVisible(m_pNetMonitorListBox->m_hWnd))
	{
		m_pNetMonitorListBox->ModifyStyle(0, WS_VISIBLE);
		CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"ListBox visable = false");
	}


	//calc
	CRect rcSize;
	CDC dcx = ::GetDC(GetViewHWND());
	HFONT			hFntTmp = NULL;
	int nLeft = 50, nOffset = 0;
	KProcFluxItem fluxItem = m_mapProcNetData[pdi->nListItemID].GetProcNetData();
	KNetFlowMonListItemData ItemData(fluxItem);
	CStringA strPosA;
	CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"ListCtrl VISABLE：%d ", IsItemVisible(NET_MONITOR_201));
	//进程名字
	_RefreshProcName(nLeft+nOffset, m_arrColumWidth[0], ItemData);
	nLeft = m_arrColumWidth[0];//210
	//-进程描述，如果在进程库里面没有查找到的话就是用程序本身的
	_RefreshProcDes(nLeft, nLeft, pdi->nHeight, m_mapProcNetData[pdi->nListItemID]);



	//危险等级
	_RefreshProcLevel(nLeft+nOffset, m_arrColumWidth[1]+nLeft, ItemData);
	nLeft += m_arrColumWidth[1];//70
	//限制下载速度
	
	_RefreshProcDownSpeed(nLeft+nOffset, m_arrColumWidth[2]+nLeft, ItemData);
	nLeft += m_arrColumWidth[2];

	//上传速度
	
	_RefreshProcUpSpeed(nLeft+nOffset, m_arrColumWidth[3]+nLeft, ItemData);
	nLeft += m_arrColumWidth[3];

	//限速
	
	_RefreshProcLimitSpeed(nLeft+10, nLeft + m_arrColumWidth[4], ItemData);
	nLeft += m_arrColumWidth[4];


	//总共下载流量
	_RefreshProcTotalDown(nLeft, m_arrColumWidth[5]+nLeft, ItemData);
	nLeft += m_arrColumWidth[5];

	//总共上传
	_RefreshProcTotalUp(nLeft, m_arrColumWidth[6] + nLeft, ItemData);
	nLeft += m_arrColumWidth[6];


	//进程PID
 	_RefreshProcID(nLeft, nLeft + m_arrColumWidth[7], ItemData);
// 	nLeft += m_arrColumWidth[7];

	//最后一列操作对应的图片 16*16
	_RefreshProcSetting(nLeft+10, nOffset+nLeft+26, ItemData);
	nLeft += m_arrColumWidth[7];

	//更新图标
	_RefreshProcICO(0, 0, ItemData);


	//显示路径
	_RefreshProcPath(nLeft, nLeft, pdi->nListItemID, ItemData);

	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_212,"crbg",pdi->bSelect ? "ECF9FF":"FFFFFF");//EBF5FF


	//显示进程ID
//	_RefreshProcID(nLeft, nLeft, ItemData);
	//-end calc


	return S_OK;
}



LRESULT KMainDlg::OnListBoxGetmaxHeight( LPNMHDR pnmh )
{
	BKLBITEMCALCMAXITEM *pdi = (BKLBITEMCALCMAXITEM*)pnmh;
	pdi->nMaxHeight = HIGHT_ITEM_MAX;
	return 0;
}

LRESULT KMainDlg::OnListBoxGetItemHeight( LPNMHDR pnmh )
{

	BKLBITEMCACLHEIGHT *pdi	= (BKLBITEMCACLHEIGHT*)pnmh;
	if (CListBoxData::GetDataPtr()->GetMaxHeightSel() == pdi->nListItemId)
	{
		pdi->nHeight = HIGHT_ITEM_MAX;
	}else
	{
		pdi->nHeight = HIGHT_ITEM_MIN;
	}

	return 0;
}

LRESULT KMainDlg::OnBkListBoxRClickCtrl(LPNMHDR pnmh)
{
	LPBKRBMITEMCLICK pnms = (LPBKRBMITEMCLICK)pnmh;
	if (pnms->nListItemID >= m_pNetMonitorListBox->GetCount())
		return E_FAIL;

	CListBoxData::GetDataPtr()->SetCurSelData(m_mapProcNetData[pnms->nListItemID]);
	OnClickNetSetting();

	return 0;
}
LRESULT KMainDlg::OnBkListBoxClickCtrl(LPNMHDR pnmh)
{
	LPBKLBMITEMCLICK pnms = (LPBKLBMITEMCLICK)pnmh;
	if (pnms->nListItemID >= m_pNetMonitorListBox->GetCount())
		return E_FAIL;

	m_pNetMonitorListBox->SetItemCount(m_mapProcNetData.GetCount());
	CListBoxData::GetDataPtr()->SetCurSelData(m_mapProcNetData[pnms->nListItemID]);

	if (pnms->uCmdID == NET_MONITOR_211)
	{
		OnClickNetSetting();
		goto Exit0;
	}else if (pnms->uCmdID == NET_MONITOR_229)
	{
		OnClickNetLockOpen();
		goto Exit0;
	}else if (pnms->uCmdID == NET_MONITOR_230)
	{
		OnClickNetLockClose();
		goto Exit0;
	}else if (pnms->uCmdID == NET_MONITOR_207 && 
		0 == CListBoxData::GetDataPtr()->GetCurSelData().GetProcNetData().m_nDisable)
	{
		OnClickNetLimitText();
		goto Exit0;
	}else if (pnms->uCmdID == NET_MONITOR_216)
	{
		OnClickOpenDir();
		goto Exit0;
	}

	if (CListBoxData::GetDataPtr()->GetMaxHeightSel() != pnms->nListItemID)
	{//并且是不能选择元素的情况下才展开
		CListBoxData::GetDataPtr()->SetMaxHeightSel(pnms->nListItemID);
		m_pNetMonitorListBox->SetItemHeight(CListBoxData::GetDataPtr()->GetMaxHeightSel(), HIGHT_ITEM_MAX);
	}
	else
	{
		CListBoxData::GetDataPtr()->SetMaxHeightSel(-1);
		m_pNetMonitorListBox->SetItemHeight(pnms->nListItemID, HIGHT_ITEM_MIN);
		m_pNetMonitorListBox->ForceRefresh();
		m_pNetMonitorListBox->ResetMouseMoveMsg();
	}

Exit0:

	return S_OK;
}

void KMainDlg::_ResetArray()
{
	m_mapProcNetData.RemoveAll();
	m_mapSpeedLimitProcNetData.RemoveAll();
}

bool KMainDlg::SortNetInfo(KProcFluxItem& ItemData1, KProcFluxItem& ItemData2, 
						   int nColum, int nState)
{

	if (-1 == nColum ||
		-1 == nState)
		return false;

	//CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"SortNetInfo:Colum = %d, State = %d", nColum, nState);
	int nRet = Compare(ItemData1, ItemData2, nColum);
	if (0 == nState)
	{//升序
		if (nRet > 0)
			return true;
	}else if (1 == nState)
	{//降序
		if (nRet < 0)
			return true;
	}
	return false;
}


bool KMainDlg::stl_SortNetInfo(KProcFluxItem& ItemData1, KProcFluxItem& ItemData2)
{

	int nColum = -1, nState = -1;
	CListBoxData::GetDataPtr()->IsNeedSort(nColum, nState);

	if (-1 == nColum ||
		-1 == nState)
	{
		//return false;
		//默认按上传下载量之和排序
		nColum = LISTBOX_COLUM_UP_DOWN_TOTAL;
		nState = 1;
	}

	int nRet = Compare(ItemData1, ItemData2, nColum);
	if (0 == nState)
	{//升序
		if (nRet < 0)
			return true;
	}else if (1 == nState)
	{//降序
		if (nRet > 0)
			return true;
	}
	return false;
}

void KMainDlg::_CreateCacheData(vector<KProcFluxItem>& vetNetData, CAtlMap<DWORD, CNetMonitorCacheData>& mapCacheData)
{
	int nColum = -1, nState = -1;
	/*
	//开始的时候自己写的排序
	if (TRUE == CListBoxData::GetDataPtr()->IsNeedSort(nColum, nState))
	{//检测一下是否需要排序,如果需要排序那就先对数据进行排序
	for (int i = 0; i < vetNetData.size(); i++)
	{
	for (int j = i+1; j < vetNetData.size(); j++)
	{
	if (true == SortNetInfo(vetNetData[i], vetNetData[j], nColum, nState))
	{
	KProcFluxItem Item;
	Item = vetNetData[i];
	vetNetData[i] = vetNetData[j];
	vetNetData[j] = Item;
	}
	}
	}
	}
	*/
	//直接使用STL的排序
	std::sort(vetNetData.begin(), vetNetData.end(), stl_SortNetInfo);

	mapCacheData.RemoveAll();
	//复制新数据到缓存
	for (int i = 0; i < vetNetData.size(); i++)
	{
		mapCacheData[i] = CNetMonitorCacheData(vetNetData[i], i, m_bNetMonEnabled);
		//		mapCacheData[i]._nIndexLine = i;
	}
}

BOOL KMainDlg::CheckServericeIsOK()
{
	DWORD serviceStatus = _GetServiceStatus(L"KSafeSvc", 1000);

	return serviceStatus == SERVICE_RUNNING; 
}

DWORD KMainDlg::_GetServiceStatus( LPCWSTR strServiceName, int nMaxQueryMilliseconds )
{
	SC_HANDLE hSvrMgr = NULL;
	SC_HANDLE hSvrFW = NULL;
	SERVICE_STATUS svrstatus = { 0 };

	DWORD dwRetStatus = -1;
	int nTimes = 0;

	do
	{
		hSvrMgr = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_QUERY_LOCK_STATUS);
		if ( !hSvrMgr )
		{
			dwRetStatus = GetLastError();
			break;
		}

		hSvrFW = ::OpenService(hSvrMgr, strServiceName, SERVICE_QUERY_STATUS);
		if ( !hSvrFW )
		{
			dwRetStatus = 0;
			break;
		}

		if ( !::QueryServiceStatus(hSvrFW, &svrstatus) )
		{
			dwRetStatus = GetLastError();
			break;
		}
		const int kSleepTimeSpan = 100;

		while( svrstatus.dwCurrentState != SERVICE_RUNNING && nTimes++ < (nMaxQueryMilliseconds/kSleepTimeSpan) )
		{
			netmon_log(L"wait for service to running, %d00ms", nTimes);
			::Sleep(kSleepTimeSpan);
			::QueryServiceStatus(hSvrFW, &svrstatus);
		}

		dwRetStatus = svrstatus.dwCurrentState;

	} while(FALSE);


	if ( hSvrFW )
	{
		::CloseServiceHandle(hSvrFW);
		hSvrFW = NULL;
	}

	if ( hSvrMgr )
	{
		::CloseServiceHandle(hSvrMgr);
		hSvrMgr = NULL;
	}

	return dwRetStatus;
}


#define WIN7_TDI_TDX	L"tdx"

BOOL KMainDlg::_CheckTdxDriver()
{
	if (!_IsVistaPlatform())
		return FALSE;

	if (!_IsDriverExist(WIN7_TDI_TDX))
	{
		CBkSafeMsgBoxFindNoTdxDriver dlg;
		int result = dlg.ShowMsg(NULL, BkString::Get(STR_DRIVER_FAILED_MSGBOX_CAPTION),NULL, MB_OK|MB_ICONEXCLAMATION, NULL, m_hWnd);
		if (ID_BUTTON_FEEDBACK == result)
		{
			::ShellExecute(m_hWnd, L"open", L"http://bbs.ijinshan.com", NULL, NULL, SW_SHOW);
		}
		_ReleaseThread();
		EndDialog(0);
		return TRUE;	
	}
	return FALSE;
}

int KMainDlg::_IsVistaPlatform()
{
	OSVERSIONINFOW _versionInfo = {0};
	_versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
	if(GetVersionExW( &_versionInfo ))
	{
		return _versionInfo.dwMajorVersion >= 6;
	}
	return false;
}

BOOL KMainDlg::_CheckHasUnCompatibleSofware()
{
	BOOL bResult = FALSE;
	// happy dailer
	HKEY hKey = NULL;
	if (ERROR_SUCCESS == (::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\HappyDialer_is1",
		0,
		KEY_READ, &hKey
		)))
	{
		::CloseHandle(hKey);
		bResult = TRUE;
	}

	if (bResult)
	{
		CBkSafeMsgBoxFindUnCompatibleSoftware dlg;
		int result = dlg.ShowMsg(NULL, BkString::Get(STR_DRIVER_FAILED_MSGBOX_CAPTION),NULL, MB_OK|MB_ICONEXCLAMATION, NULL, m_hWnd);
		if (ID_BUTTON_FEEDBACK == result)
		{
			::ShellExecute(m_hWnd, L"open", L"http://bbs.ijinshan.com", NULL, NULL, SW_SHOW);
		}
		_ReleaseThread();
		EndDialog(0);
		return TRUE;	
	}
	return bResult;
}

void KMainDlg::_RefreshListBoxData(vector<KProcFluxItem>& vetNetData)
{

	CAutoLocker locker(m_locker);
	//::EnterCriticalSection(&m_Lock);

	BOOL bIsVisible = IsItemVisible(TAB_SHOW_FLOW_MINTER);
	if (!bIsVisible)
		goto Exit0;

	_CreateCacheData(vetNetData, m_mapProcNetData);
 

// 	if (0 == m_mapProcNetData.GetCount())
// 	{
// 		m_pNetMonitorListBox->ModifyStyle(0, WS_VISIBLE);
// 		//goto Exit0;
// 	}
	
	//更新列表
	if (m_pNetMonitorListBox->GetCount() != m_mapProcNetData.GetCount())
	{
		static int a = 1;
		//if (1 == a)
			m_pNetMonitorListBox->SetItemCount(m_mapProcNetData.GetCount());
			m_pNetMonitorListBox->RefereshVScroll();
			a = 0;
	}
	else
	{
		m_pNetMonitorListBox->ForceRefresh();
		m_pNetMonitorListBox->ResetMouseMoveMsg();
	}

Exit0:
	//::LeaveCriticalSection(&m_Lock);

	SetEvent(m_hEventGetNetData);
}

BOOL KMainDlg::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	CRect rcWin;
	if ( m_pNetMonitorListBox && m_pNetMonitorListBox->IsWindow() && m_pNetMonitorListBox->IsWindowVisible() )
	{
		m_pNetMonitorListBox->GetWindowRect(&rcWin);

		if (rcWin.PtInRect(pt))
			SendMessage(m_pNetMonitorListBox->m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
	}


	SetMsgHandled(FALSE);
	return TRUE;		 
}

void KMainDlg::OnClickColumTitle()
{
	_SetColumState(0);
}

void KMainDlg::OnClickColumLevel()
{
	_SetColumState(1);
}
void KMainDlg::OnClickColumDownLoadSpeed()
{
	_SetColumState(2);
}
void KMainDlg::OnClickColumUpLoadSpeed()
{
	_SetColumState(3);
}
void KMainDlg::OnClickColumNetLimit()
{
	_SetColumState(4);
}
void KMainDlg::OnClickColumTotalDownload()
{
	_SetColumState(5);
}
void KMainDlg::OnClickColumTotalUp()
{
	_SetColumState(6);
}
void KMainDlg::OnClickColumPID()
{
	_SetColumState(7);
}

void KMainDlg::_SetColumState(int nColum)
{
	if (nColum < 0 || nColum >= CListBoxData::GetDataPtr()->GetStateArray().GetCount())
		return;

	//set colum state
	CListBoxData::GetDataPtr()->GetStateArray()[nColum] += 1;
	if (CListBoxData::GetDataPtr()->GetStateArray()[nColum] > 1)
		CListBoxData::GetDataPtr()->GetStateArray()[nColum] = 0;

	//get img pos
	int nLeft = 0;
	for (int i = 0; i <= nColum; i++)
	{
		nLeft += m_arrColumWidth[i];
	}
	//reset colum state
	for (int i = 0; i < CListBoxData::GetDataPtr()->GetStateArray().GetCount(); i++)
	{
		if (nColum != i)
			CListBoxData::GetDataPtr()->GetStateArray()[i] = -1;
	}
	//
	SetItemVisible(NET_MONITOR_228, TRUE);
	CStringA strPos = "";
	strPos.Format("%d,5,%d,-0", nLeft-12, nLeft);
	SetItemAttribute(NET_MONITOR_228, "pos", strPos);
	SetItemIntAttribute(NET_MONITOR_228, "sub", CListBoxData::GetDataPtr()->GetStateArray()[nColum]);

}

void KMainDlg::OnClickNetSetting()
{
	POINT pt;
	GetCursorPos(&pt);


	::EnableMenuItem(m_hNetMonitorMenu, ID_NETMONITOR_ENABLE, MF_DISABLED | MF_GRAYED);
	::EnableMenuItem(m_hNetMonitorMenu, ID_NETMONITOR_DISABLE, MF_DISABLED | MF_GRAYED);
	::EnableMenuItem(m_hNetMonitorMenu, ID_NETMONITOR_LIMIT, MF_DISABLED | MF_GRAYED);
#if 0
	if (TRUE)
#else
	if (0 == CListBoxData::GetDataPtr()->GetCurSelData().GetProcNetData().m_nDisable)
#endif
	{//没有禁用网络
		::EnableMenuItem(m_hNetMonitorMenu, ID_NETMONITOR_DISABLE, MF_ENABLED);
		::EnableMenuItem(m_hNetMonitorMenu, ID_NETMONITOR_LIMIT, MF_ENABLED);
	}
	else
	{//已经禁用网络
		::EnableMenuItem(m_hNetMonitorMenu, ID_NETMONITOR_ENABLE, MF_ENABLED);
	}

	//没有路径的话，禁止查看属性和打开路径
	KProcFluxItem fluxItem = CListBoxData::GetDataPtr()->GetCurSelData().GetProcNetData();
	KNetFlowMonListItemData ItemData(fluxItem);

	::EnableMenuItem(m_hNetMonitorMenu, ID_NETMONITOR_FILEATTRI, MF_DISABLED | MF_GRAYED);
	::EnableMenuItem(m_hNetMonitorMenu, ID_NETMONITOR_OPENDIR, MF_DISABLED | MF_GRAYED);
	if (TRUE == PathFileExists(ItemData.GetProcessFullPath()))
	{
		::EnableMenuItem(m_hNetMonitorMenu, ID_NETMONITOR_FILEATTRI, MF_ENABLED);
		::EnableMenuItem(m_hNetMonitorMenu, ID_NETMONITOR_OPENDIR, MF_ENABLED);
	}


	//根据是否做了限速，设置取消限速
	::EnableMenuItem(m_hNetMonitorMenu, ID_NETMONITOR_CANCEL_LIMIT, MF_DISABLED | MF_GRAYED);
	if (0 == CListBoxData::GetDataPtr()->GetCurSelData().GetProcNetData().m_nDisable &&
		0 != CListBoxData::GetDataPtr()->GetCurSelData().GetProcNetData().m_nSendLimit)
	{//没有禁用并且没有做过限速
		::EnableMenuItem(m_hNetMonitorMenu, ID_NETMONITOR_CANCEL_LIMIT, MF_ENABLED);
	}


	SetForegroundWindow(m_hWnd);
	TrackPopupMenu(m_hNetMonitorMenu,
		TPM_RIGHTBUTTON,
		pt.x,
		pt.y,
		0,
		m_hWnd,
		NULL
		);
	//	PostMessage(WM_NULL, 0, 0);
}

//
void KMainDlg::OnClickNetLimitText()
{
	_CallNetLimitDialog(CListBoxData::GetDataPtr()->GetCurSelData());
}


//
void KMainDlg::OnClickNetLockOpen()
{
	_CallNetLimitDialog(CListBoxData::GetDataPtr()->GetCurSelData());
}

//
void KMainDlg::OnClickNetLockClose()
{
	CNetMonitorCacheData Data = CListBoxData::GetDataPtr()->GetCurSelData();
	Data.GetProcNetData().m_nSendLimit = 0;
	if (TRUE == Data.GetProcNetData().m_bAutoFixed)
	{//只有曾经对同名进程做过限速才有这个设置
		Data.SetRemember(TRUE);
	}
	//	_UpdateCacheData(Data);
	CListBoxData::GetDataPtr()->SetCurSelData(Data);
	_NetSpeedLimit(&Data);
}

int KMainDlg::Compare( KProcFluxItem& ItemData1, KProcFluxItem& ItemData2, int nsubIndex/* = -1*/ )
{
	int nCmpResult = KPFW_COMPARE_RESULT_ERROR;

	if (nsubIndex < 0)
	{
		return nCmpResult;
	}

	//KNetFlowMonListItemData* pNetFlowMonItem = (KNetFlowMonListItemData*)piTreeItem;
	KNetFlowMonListItemData Data1(ItemData1);
	KNetFlowMonListItemData Data2(ItemData2);

	CString str1 = L"", str2 = L"";
	LONGLONG num1 = 0, num2 = 0;

	switch(nsubIndex)
	{
	case LISTBOX_COLUM_NAME:
		{
			str1 = Data1.GetProcessName();
			str2 = Data2.GetProcessName();
			nCmpResult = KCompare::StringNoCase(str1, str2);
		}
		break;	
	case LISTBOX_COLUM_LEVEL:
		{
			str1 = Data1.GetTrust();
			str2 = Data2.GetTrust();
			nCmpResult = KCompare::StringNoCase(str1, str2);
		}
		break;
	case LISTBOX_COLUM_DOWNLOAD:
		{
			num1 = Data1.GetDownloadSpeed();
			num2 = Data2.GetDownloadSpeed();
			nCmpResult = KCompare::Compare<ULONGLONG>(num1,num2);
		}

		break;
	case LISTBOX_COLUM_UP:
		{
			num1 = Data1.GetUploadSpeed();
			num2 = Data2.GetUploadSpeed();
			nCmpResult =KCompare::Compare<ULONGLONG>(num1,num2);
		}

		break;
	case LISTBOX_COLUM_LIMIT:
		{
			num1 = Data1.GetSendLimitVaule();
			num2 = Data2.GetSendLimitVaule();
			nCmpResult =KCompare::Compare<ULONGLONG>(num1,num2);
		}

		break;
	case LISTBOX_COLUM_DOWN_TOTAL:
		{
			num1 = Data1.GetTotalDownload();
			num2 = Data2.GetTotalDownload();
			nCmpResult =  KCompare::Compare<ULONGLONG>(num1,num2);
		}

		break;
	case LISTBOX_COLUM_UP_TOTAL:
		{
			num1 = Data1.GetTotalUpload();
			num2 = Data2.GetTotalUpload();
			nCmpResult =  KCompare::Compare<ULONGLONG>(num1,num2);
		}

		break;
	case LISTBOX_COLUM_PID:
		{
			DWORD dw1 = Data1.GetProcessId();
			DWORD dw2 = Data2.GetProcessId();
			nCmpResult = KCompare::Compare<DWORD>(dw1, dw2);
		}
		break;
	case LISTBOX_COLUM_UP_DOWN_TOTAL:
		{
			num1 = Data1.GetTotalUpload() + Data1.GetTotalDownload();
			num2 = Data2.GetTotalUpload() + Data2.GetTotalDownload();
			nCmpResult =  KCompare::Compare<ULONGLONG>(num1,num2);

		}
		break;
	default:
		break;
	}

	return nCmpResult;
}


LRESULT KMainDlg::OnNetMonitorCommand(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CNetMonitorCacheData NetData = CListBoxData::GetDataPtr()->GetCurSelData();
	KNetFlowMonListItemData	ItemData(NetData.GetProcNetData());
	switch(wParam)
	{
	case ID_NETMONITOR_DISABLE:
		{//网络禁用
			if (ItemData._GetProcessType(NetData.GetProcNetData()) == enumProcessTypeOS)
			{
				CBkSafeMsgBox2 dlg;
				CString strTipInfo;
				strTipInfo.Format(BkString::Get(DefString61), ItemData.GetProcessName());
				dlg.ShowMutlLineMsg(strTipInfo, BkString::Get(7), MB_OK | MB_ICONEXCLAMATION);
				break;
			}
			else
			{
				NetData.GetProcNetData().m_nSendLimit = 0;
				CBKSafeNetMonForbidDlg dlg(m_hWnd, MSG_NETMONITOR_LIMIT, NetData);
				dlg.DoModal();
			}
		}
		break;
	case ID_NETMONITOR_ENABLE:
		{//允许访问网络
			NetData.GetProcNetData().m_nDisable = 0;
			NetData.SetRemember(TRUE);
			_NetSpeedLimit(&NetData);
		}
		break;
	case ID_NETMONITOR_LIMIT:
		{//网络限速
			_CallNetLimitDialog(CListBoxData::GetDataPtr()->GetCurSelData());
		}
		break;
	case ID_ONEKEY_1KB:
		{//1KB限速
			NetData.GetProcNetData().m_nSendLimit = 1*1024;
			_NetSpeedLimit(&NetData);
		}
		break;
	case ID_ONEKEY_5KB:		
		{//5KB限速
			NetData.GetProcNetData().m_nSendLimit = 5 * 1024;
			_NetSpeedLimit(&NetData);
		}
		break;
	case ID_ONEKEY_10KB:		
		{//10KB限速
			NetData.GetProcNetData().m_nSendLimit = 10 * 1024;
			_NetSpeedLimit(&NetData);
		}
		break;
	case ID_ONEKEY_20KB:	
		{//20KB限速
			NetData.GetProcNetData().m_nSendLimit = 20 * 1024;
			_NetSpeedLimit(&NetData);
		}
		break;
	case ID_NETMONITOR_OPENDIR:
		{//打开文件路径
			ItemData.OpenDirector();
		}
		break;
	case ID_NETMONITOR_FILEATTRI:
		{//查看文件属性
			ItemData.OpenFileAttribute();
		}
		break;
	case ID_NETMONITOR_KILL:
		{
			CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"CanNotKill: = %d", NetData.GetProcNetData().m_nCanNotKill);
			if (TRUE == ItemData.KillCurrentSelProcess())
			{
				OnTimer(ID_TIMER_UPDATE_NETFlOW_MON);
			}
		}
		break;
	case ID_NETMONITOR_CANCEL_LIMIT:
		{
			OnClickNetLockClose();
		}
		break;
	default:
		break;
	}
	SetMsgHandled(FALSE);
	return 0;
}

void KMainDlg::OnClickOpenDir()
{
	KNetFlowMonListItemData	ItemData(CListBoxData::GetDataPtr()->GetCurSelData().GetProcNetData());
	ItemData.OpenDirector();
}



LRESULT KMainDlg::OnNetMonitorLimitMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (MSG_NETMONITOR_LIMIT == uMsg)
	{
		CNetMonitorCacheData* pNetMonitorData = (CNetMonitorCacheData*)(wParam);
		if (NULL != pNetMonitorData)
		{
			CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"NetLimit: sendLimit = %d, recvlimit = %d", pNetMonitorData->GetProcNetData().m_nSendLimit,
				pNetMonitorData->GetProcNetData().m_nRecvLimit);
			//调用进程间通信，告诉驱动我要限速
			_NetSpeedLimit(pNetMonitorData);
		}
	}
	return 0;
}

void KMainDlg::_NetSpeedLimit(CNetMonitorCacheData* pNetData)
{
	UrlMonIpcParam param = {0};
	param.m_nCallType = Ipc_NetFlux_SetProcessSpeed;
	param.m_SetSpeed.nProcessID = pNetData->GetProcNetData().m_nProcessID;
	param.m_SetSpeed.nSendLimit = pNetData->GetProcNetData().m_nSendLimit ;
	param.m_SetSpeed.nRecvLimit = pNetData->GetProcNetData().m_nRecvLimit ;
	param.m_SetSpeed.nDisable = pNetData->GetProcNetData().m_nDisable;
	param.m_SetSpeed.bRemember = pNetData->IsRemember();
	CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"_NetSpeedLimit: %s nSendLimit = %d, nReccvLimit = %d, nDisable = %d", pNetData->GetProcNetData().m_strProcPath, pNetData->GetProcNetData().m_nSendLimit, 
		pNetData->GetProcNetData().m_nRecvLimit, pNetData->GetProcNetData().m_nDisable);

	IPCClient client;
	unsigned int nsize = sizeof(param);
	client.RunCall(IPC_PROC_URLMON, &param, nsize);

	OnTimer(ID_TIMER_UPDATE_NETFlOW_MON);

}

int KMainDlg::_CallNetLimitDialog(CNetMonitorCacheData& NetData)
{
	CBKSafeNetMonitorLimit dlg(m_hWnd, MSG_NETMONITOR_LIMIT, NetData);
	return dlg.DoModal();
}

void KMainDlg::OnStartNetSpeed()
{
	CString strPath;
	CAppPath::Instance().GetLeidianAppPath(strPath);
	strPath.Append(L"\\kmspeed.exe");
	if (FALSE == PathFileExists(strPath))
	{//不存在切换到主程序系统工具集
		TCHAR szCmd[1024] = {0};
		CString strAppPath = _GetAppPath();
		_sntprintf_s(szCmd, sizeof(szCmd), _T("-do:%s"), L"ui_SysOpt_KSafeTools");
		ShellExecute(NULL, _T("open"), strAppPath, szCmd, NULL, SW_SHOW);
	}
	else if (0 == CVerifyFileFunc::GetPtr()->CheckKingSoftFileSigner(strPath))
	{//程序存在并且签名合法直接调用
		ShellExecute(NULL, _T("open"), strPath, NULL, NULL, SW_SHOW);
	}

}

void KMainDlg::_UpdateCacheData(CNetMonitorCacheData& NetData)
{
	POSITION pos = m_mapProcNetData.GetStartPosition();

	while(NULL != pos)
	{
		CNetMonitorCacheData Data = m_mapProcNetData.GetValueAt(pos);
		if (Data.GetProcNetData().m_nProcessID == NetData.GetProcNetData().m_nProcessID)
		{
			m_mapProcNetData[m_mapProcNetData.GetKeyAt(pos)] = NetData;
			// 			m_pNetMonitorListBox->ForceRefresh();
			// 			m_pNetMonitorListBox->ResetMouseMoveMsg();
			break;
		}
		m_mapProcNetData.GetNext(pos);
	}
}

int KMainDlg::_RefreshProcName(int nLeft, int nRight, KNetFlowMonListItemData ItemData)
{
	CStringA strPosA	= "";

	// 使用库里面的displayname [1/10/2011 zhangbaoliang]
	CString strProcName = ItemData.GetProcessName();
	KSProcessInfo	ProcInfo;
	if (0 == CListBoxData::GetDataPtr()->QueryProcInfo(ItemData.GetProcessFullPath(), ProcInfo) && 
		0 != ProcInfo.strDisplayName.length())
	{//查找进程库,这里和描述分别都做了查找操作，不过不会影响到性能，因为本地有缓存
		strProcName = ProcInfo.strDisplayName.c_str();
		CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"_RefreshProcName: find %s", strProcName);
	}
	else
	{
		CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"_RefreshProcName: not find %s", strProcName);
	}

	strPosA.Format("%d,10,%d,25", nLeft, nRight);
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_203, "pos", strPosA);
	m_pNetMonitorListBox->SetItemText(NET_MONITOR_203, strProcName);
	m_pNetMonitorListBox->SetItemStringAttribute(NET_MONITOR_203, "tip", strProcName);

	return 0;
}

int KMainDlg::_RefreshProcDes(int nLeft, int nRight, int nHeight, CNetMonitorCacheData NetData)
{
	CDC dcx					= ::GetDC(GetViewHWND());
	HFONT hFntTmp			= NULL;
	CStringA strPosA		= "";
	CString	 strValue		= L"";
	CRect rcSize(0,0,0,0);
	KSProcessInfo ProcInfo ;
	KNetFlowMonListItemData ItemData(NetData.GetProcNetData());

	hFntTmp = dcx.SelectFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
	if (0 == CListBoxData::GetDataPtr()->QueryProcInfo(ItemData.GetProcessFullPath(), ProcInfo) && 
		0 != ProcInfo.strDesc.length())
	{//查找进程库
		strValue = ProcInfo.strDesc.c_str();
	}
	else
	{//使用程序自身的
		strValue = ItemData.GetProcessDescription();
		CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"信息缺失：%s ", strValue);
		//进程库中没有查到关于他的信息，那么就把这个进程的信息发送到服务端，帮助入库
	}	
	dcx.DrawText(strValue, -1, &rcSize, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_213, "class", "filename");
#if 0
	if (TRUE)
#else
	if (HIGHT_ITEM_MAX == nHeight)
#endif
	{//展开情况下显示完整描述信息
		rcSize.right += 50;
		if (rcSize.right >= 750)
		{
			rcSize.right = 750;
		}
		strPosA.Format("%d,30,%d,42", 50, rcSize.right);
	}
	else
	{//防止文字路径太长超出范围
		strPosA.Format("%d,30,%d,45", 50, nLeft);
	}
	m_pNetMonitorListBox->SetItemStringAttribute(NET_MONITOR_213, "tip", strValue);
	//m_pNetMonitorListBox->SetItemColorAttribute(NET_MONITOR_213, "crtext", RGB(109,109,109));
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_213, "pos", strPosA);
	m_pNetMonitorListBox->SetItemText(NET_MONITOR_213, strValue);

	dcx.SelectFont(hFntTmp);
	::ReleaseDC(GetViewHWND(), dcx);

	return 0;
}

int KMainDlg::_RefreshProcLevel(int nLeft, int nRight, KNetFlowMonListItemData ItemData)
{

	CStringA strPosA		= "";
	strPosA.Format("%d,10,%d,25", nLeft, nRight);
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_204, "pos", strPosA);
	m_pNetMonitorListBox->SetItemText(NET_MONITOR_204, ItemData.GetTrust());
	m_pNetMonitorListBox->SetItemColorAttribute(NET_MONITOR_204, "crtext", ItemData.GetLevelColor());

	return 0;
}

int KMainDlg::_RefreshProcUpSpeed(int nLeft, int nRight, KNetFlowMonListItemData ItemData)
{
	CStringA strPosA		= "";

	strPosA.Format("%d,10,%d,25", nLeft, nRight);
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_206, "pos", strPosA);

	CString strValue = BkString::Get(NET_MONITOR_STR_509);
	if (TRUE == m_bNetMonEnabled)
	{//监控开启状态
		if (0 == ItemData.AccessNetIsDisable())
			NetFlowToString(ItemData.GetUploadSpeed(), strValue);
		else
			strValue = BkString::Get(NET_MONITOR_STR_506);
	}

	m_pNetMonitorListBox->SetItemText(NET_MONITOR_206, strValue);


	return 0;
}

int KMainDlg::_RefreshProcDownSpeed(int nLeft, int nRight, KNetFlowMonListItemData ItemData)
{
	CStringA strPosA		= "";

	strPosA.Format("%d,10,%d,25", nLeft, nRight);
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_205, "pos", strPosA);

	CString strValue = BkString::Get(NET_MONITOR_STR_509);
	if (TRUE == m_bNetMonEnabled)
	{//监控打开状况
		if (0 == ItemData.AccessNetIsDisable())
			NetFlowToString(ItemData.GetDownloadSpeed(), strValue);
		else
			strValue = BkString::Get(NET_MONITOR_STR_506);
	}

	m_pNetMonitorListBox->SetItemText(NET_MONITOR_205, strValue);

	return 0;
}

int KMainDlg::_RefreshProcLimitSpeed(int nLeft, int nRight, KNetFlowMonListItemData ItemData)
{
	CStringA strPosA		= "";
	CString  strValue		= L"";

	m_pNetMonitorListBox->SetItemVisible(NET_MONITOR_229, FALSE);
	m_pNetMonitorListBox->SetItemVisible(NET_MONITOR_230, FALSE);
	DWORD dwNetID = NET_MONITOR_229;
	if (0 == ItemData.GetSendLimitVaule())
	{//之前没有做过限速操作
		strValue = BkString::Get(NET_MONITOR_STR_504);
	}
	else
	{//之前做过限速了,就要显示上次的上传限速
		dwNetID = NET_MONITOR_230;
		strValue.Format(BkString::Get(NET_MONITOR_STR_505), ItemData.GetSendLimitVaule());
		CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"GetDispInfo: %s 之前做过限速了 nSend = %d, nRecv = %d, nDisable = %d", 
			ItemData.GetProcessName(), ItemData.GetSendLimitVaule(), ItemData.GetRecvLimitValue(), ItemData.AccessNetIsDisable());
	}
	m_pNetMonitorListBox->EnableItem(dwNetID, TRUE);
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_207, "class", "linkcentertext");
	if (1 == ItemData.AccessNetIsDisable())
	{//是禁用网络,，显示已禁用
		m_pNetMonitorListBox->EnableItem(dwNetID, FALSE);
		strValue = BkString::Get(NET_MONITOR_STR_506);
		m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_207, "class", "righttext");
	}


	//	rcSize.SetRectEmpty();
	//dcx.DrawText(strValue, -1, &rcSize, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
	//int nRight = nLeft + m_arrColumWidth[4];
	strPosA.Format("%d,10,%d,25", nLeft, nRight - 35);
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_207, "pos", strPosA);
	m_pNetMonitorListBox->SetItemText(NET_MONITOR_207, strValue);

	//限速功能锁图片
	//nLeft += m_arrColumWidth[4];
	m_pNetMonitorListBox->SetItemVisible(dwNetID, TRUE);
	int nLockLeft = nRight - 30;//nLeft+rcSize.Width()+10;
	strPosA.Format("%d,7,%d,16", nLockLeft, nLockLeft+21);
	m_pNetMonitorListBox->SetItemAttribute(dwNetID, "pos", strPosA);

	return 0;
}

int KMainDlg::_RefreshProcTotalDown(int nLeft, int nRight, KNetFlowMonListItemData ItemData)
{
	CStringA strPosA		= "";
	CString  strValue		= L"";

	strPosA.Format("%d,10,%d,25", nLeft, nRight);
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_208, "pos", strPosA);
	NetFlowToString(ItemData.GetTotalDownload(), strValue);
	m_pNetMonitorListBox->SetItemText(NET_MONITOR_208, strValue);

	return 0;
}

int KMainDlg::_RefreshProcTotalUp(int nLeft, int nRight, KNetFlowMonListItemData ItemData)
{
	CStringA strPosA		= "";
	CString  strValue		= L"";

	strPosA.Format("%d,10,%d,25", nLeft, nRight);
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_209, "pos", strPosA);
	NetFlowToString(ItemData.GetTotalUpload(), strValue);
	m_pNetMonitorListBox->SetItemText(NET_MONITOR_209, strValue);

	return 0;
}

int KMainDlg::_RefreshProcID(int nLeft, int nRight, KNetFlowMonListItemData ItemData)
{
	CStringA strPosA		= "";
	CString  strValue		= L"";

// 	strPosA.Format("%d,10,%d,25", nLeft, nRight);
// 	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_210, "pos", strPosA);
// 	strValue.Format(L"%d", ItemData.GetProcessId());
// 	m_pNetMonitorListBox->SetItemText(NET_MONITOR_210, strValue);

	if (0 != ItemData.GetProcessId())
	{
		strValue.Format(BkString::Get(NET_MONITOR_STR_511) , ItemData.GetProcessId());
		m_pNetMonitorListBox->SetItemText(NET_MONITOR_251, strValue);
		if (TRUE == PathFileExists(ItemData.GetProcessFullPath()))
		{
			m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_251, "pos", "50,80");
		}
		else
		{
			m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_251, "pos", "50,56");
		}
		
		//m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_251, "crtext","6d6d6d");
	}
	else
	{
		strValue.Format(BkString::Get(NET_MONITOR_STR_511) , 0);
		m_pNetMonitorListBox->SetItemText(NET_MONITOR_251, strValue);
		if (TRUE == PathFileExists(ItemData.GetProcessFullPath()))
		{
			m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_251, "pos", "50,80");
		}
		else
		{
			m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_251, "pos", "50,56");
		}
	}

	return 0;
}

int KMainDlg::_RefreshProcICO(int nLeft, int nRight, KNetFlowMonListItemData ItemData)
{
	char pszValue[MAX_PATH] = {0};
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_217, "iconhandle", "0");
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_217, "srcfile", "0");
	if (TRUE == PathFileExists(ItemData.GetProcessFullPath()))
	{
		CStringA strPathIconA = ItemData.GetProcessFullPath();
		if (FALSE == PathFileExistsA(strPathIconA))
		{
			_snprintf_s(pszValue, sizeof(pszValue), "%d", CListBoxData::GetDataPtr()->GetIcon(ItemData.GetProcessFullPath()));
			m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_217, "iconhandle", pszValue);

		}
		else
		{
			m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_217, "srcfile", strPathIconA);
		}


	}else
	{
		_snprintf_s(pszValue, sizeof(pszValue), "%d", CListBoxData::GetDataPtr()->GetDefaultIcon());
		m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_217, "iconhandle", pszValue);
	}

	return 0;
}

int KMainDlg::_RefreshProcPath(int nLeft, int nRight, int nCurSel, KNetFlowMonListItemData ItemData)
{
	CDC dcx					= ::GetDC(GetViewHWND());
	HFONT hFntTmp			= NULL;
	CStringA strPosA		= "";
	CString  strValue		= L"";
	CRect	 rcSize(0,0,0,0);

	//显示路径
	m_pNetMonitorListBox->SetItemVisible(NET_MONITOR_214, FALSE);
	CListBoxData::GetDataPtr()->GetLogModule().WriteLog(L"RefreshProcPath, Name = %s, Disable = %d",ItemData.GetProcessName(), ItemData.AccessNetIsDisable());
	if (1 == ItemData.AccessNetIsDisable())
	{//被禁用显示标记
		m_pNetMonitorListBox->SetItemVisible(NET_MONITOR_218, TRUE);
	}
	else
	{
		m_pNetMonitorListBox->SetItemVisible(NET_MONITOR_218, FALSE);
	}
	if (CListBoxData::GetDataPtr()->GetMaxHeightSel() == nCurSel && 
		TRUE == PathFileExists(ItemData.GetProcessFullPath()))
	{//行展开式显示的文件路径		
		m_pNetMonitorListBox->SetItemVisible(NET_MONITOR_214, TRUE);
		hFntTmp = dcx.SelectFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
		rcSize.SetRectEmpty();
		strValue.Format(BkString::Get(NET_MONITOR_STR_508), ItemData.GetProcessFullPath());
		dcx.DrawText( strValue, -1, &rcSize, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
		rcSize.OffsetRect(50, 0);
		int nOffsetLeft = nRight - 122;
		m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_215, "class", "filename");
		if (rcSize.right >= nOffsetLeft)
		{
			rcSize.right = nOffsetLeft;
		}

		strPosA.Format("50,8,%d,23", rcSize.right);
		m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_215, "pos", strPosA);
		m_pNetMonitorListBox->SetItemText(NET_MONITOR_215, strValue);
		m_pNetMonitorListBox->SetItemStringAttribute(NET_MONITOR_215, "tip", strValue);
		//m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_215, "crtext", "6d6d6d");
		//查看路径
		strPosA.Format("%d,6,%d,20", rcSize.right+20, rcSize.right + 92);
		m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_216, "pos", strPosA);
		m_pNetMonitorListBox->SetItemText(NET_MONITOR_216, BkString::Get(NET_MONITOR_STR_501));

	}

	dcx.SelectFont(hFntTmp);
	::ReleaseDC(GetViewHWND(), dcx);

	return 0;
}

int KMainDlg::_RefreshProcSetting(int nLeft, int nRight,KNetFlowMonListItemData ItemData)
{
	CStringA strPosA		= "";

	strPosA.Format("%d,10,%d,26", nLeft-5, nRight-5);
	m_pNetMonitorListBox->SetItemAttribute(NET_MONITOR_211, "pos", strPosA);

	return 0;
}



void KMainDlg::OnClickShowList1( void )
{
	if (m_bNetMonEnabled)
	{
		m_nCurShowStr = 49;
		FormatRichText(DEFACCESSNETCOUNT, BkString::Get(m_nCurShowStr), this->m_processInfoList.size());
	}

	m_enumProcessMode = enumProcessModeHasNetFlow;
	_ShowListClick(1);
}

void KMainDlg::OnClickShowList2( void )
{
	if (m_bNetMonEnabled)
	{
		m_nCurShowStr = 50;
		FormatRichText(DEFACCESSNETCOUNT, BkString::Get(m_nCurShowStr), this->m_processInfoList.size());
	}
		
	m_enumProcessMode = enumProcessModeHasLocationFlow;
	_ShowListClick(3);
}

void KMainDlg::OnClickShowList3( void )
{
	if (m_bNetMonEnabled)
	{
		m_nCurShowStr = 51;
		FormatRichText(DEFACCESSNETCOUNT, BkString::Get(m_nCurShowStr), this->m_processInfoList.size());
	}

	m_enumProcessMode = enumProcessModeHasAll;
	_ShowListClick(5);
}

void KMainDlg::OnClickShowList4( void )
{
	if (m_bNetMonEnabled)
	{
		m_nCurShowStr = 62;
		FormatRichText(DEFACCESSNETCOUNT, BkString::Get(m_nCurShowStr), this->m_processInfoList.size());
	}

	m_enumProcessMode = enumProcessModeDisabled;
	_ShowListClick(6);
}

void KMainDlg::_ShowListClick( IN int nCurPage )
{
	SetItemVisible(NET_MONITOR_242, TRUE);
	SetItemVisible(NET_MONITOR_244, TRUE);
	SetItemVisible(NET_MONITOR_246, TRUE);
	SetItemVisible(NET_MONITOR_307, TRUE);
	SetItemVisible(NET_MONITOR_243, FALSE);
	SetItemVisible(NET_MONITOR_245, FALSE);
	SetItemVisible(NET_MONITOR_247, FALSE);
	SetItemVisible(NET_MONITOR_308, FALSE);

	if (nCurPage == 6)
	{
		SetItemVisible(NET_MONITOR_307, FALSE);
		SetItemVisible(NET_MONITOR_308, TRUE);
	}
	else
	{
		SetItemVisible(NET_MONITOR_242 + nCurPage - 1, FALSE);
		SetItemVisible(NET_MONITOR_242 + nCurPage, TRUE);
		SetItemVisible(NET_MONITOR_308, FALSE);
	}

	_ShowListInfo();

}

void KMainDlg::_ShowListInfo( void )
{
	KNetFluxCacheReader reader;
	if (SUCCEEDED(reader.Init()))
	{
		reader.GetProcessesFluxInfo(m_FluxSys, &m_processInfoList, m_enumProcessMode);
		//_UpdateNetMonitorListBox();
		_RefreshListBoxData(m_processInfoList);

		_UpdateNetFlowSummaryWnd();
	}
	else
	{
		netmon_log(L"GetProcessesFluxInfo failed.");
	}
}

void KMainDlg::_HideOrShowTip(IN BOOL bIsShowTip, IN BOOL bIsNeedAct)
{
	
	if (!bIsShowTip)
	{
		FormatItemText(NET_MONITOR_248, BkString::Get(DefString9), BkString::Get(DefString11));
		SetItemText(NET_MONITOR_249, BkString::Get(DefStringShow));
	}
	else
	{
		FormatItemText(NET_MONITOR_248, BkString::Get(DefString9), BkString::Get(DefString10));
		SetItemText(NET_MONITOR_249, BkString::Get(DefStringHide));
	}

	if (bIsNeedAct)
		_ShowFloatWnd(!bIsShowTip);
}

void KMainDlg::OnClickIsShowTip( void )
{
	//关闭或者打开
	netmon_log(L"CurrentFloatWnd Status:%d", m_bFloatWndIsOpen);

	_HideOrShowTip(m_bFloatWndIsOpen);

}

CString KMainDlg::_GetAppPath()
{
	CString strAppPath;
	CAppPath::Instance().GetLeidianAppPath(strAppPath);
	strAppPath.Append(L"\\");
	strAppPath.Append(MAIN_APP_NAME);

	return strAppPath;
}
// 
// int KMainDlg::_RefreshProcID(int nLeft, int nRight,KNetFlowMonListItemData ItemData)
// {
// 	CString strValue;
// 	strValue.Format(BkString::Get(NET_MONITOR_STR_511)) ItemData.GetProcessId();
// 	SetItemText(NET_MONITOR_251, strValue);
// 
// 	return 0;
// }
	
//}

BOOL KMainDlg::OnBkTabMainSelChange( int nTabItemIDOld, int nTabItemIDNew )
{
	if (nTabItemIDOld == 0)
		SetItemVisible(TAB_SHOW_FLOW_MINTER, FALSE);
	else if (nTabItemIDOld == 1)
		SetItemVisible(TAB_SHOW_FLOW_STAT, FALSE);

	if (nTabItemIDNew == 0)		
		SetItemVisible(TAB_SHOW_FLOW_MINTER, TRUE);
	else if (nTabItemIDNew == 1)	
		SetItemVisible(TAB_SHOW_FLOW_STAT, TRUE);
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//流量统计

void KMainDlg::_ShowStatList( IN int nCode )
{
	for (int i = TAB_SHOW_STAT_DAYS + 1; i <= TAB_SHOW_STAT_DEFINESELF + 1; i+=2)
	{
		if (IsItemVisible(i))
		{
			SetItemVisible(i - 1, TRUE);
			SetItemVisible(i, FALSE);
			break;
		}
	}

	if (nCode != 7)
	{
		SetItemVisible(TAB_SHOW_STAT_DAYS + nCode - 1, FALSE);
		SetItemVisible(TAB_SHOW_STAT_DAYS + nCode, TRUE);
	}
	
	m_pNetStatListBox->SetItemCount(m_vLogListInfo.size());
	_ShowPageForList();
}


void KMainDlg::OnClickShowStatDays( void )
{
	m_nCurShowType = enumQueryTypeEveryDay;
	_GetCurLogInfo(enumQueryTypeEveryDay);
	_ShowStatList(1);
}

void KMainDlg::OnClickShowStatWeeks( void )
{
	m_nCurShowType = enumQueryTypeEveryWeek;
	_GetCurLogInfo(enumQueryTypeEveryWeek);
	_ShowStatList(3);
}

void KMainDlg::OnClickShowStatMonths( void )
{
	m_nCurShowType = enumQueryTypeEveryMonth;
	_GetCurLogInfo(enumQueryTypeEveryMonth);
	_ShowStatList(5);
}

void KMainDlg::OnClickShowStatDefineSelf( void )
{
	KFlowStatDefine flowStat;
	if (flowStat.DoModal() == IDOK)
	{
		CString strFrom, strTo;
		flowStat.GetRangleQuery(strFrom, strTo);
		m_strQueryBegin = strFrom;
		m_strQueryEnd   = strTo;
		m_nCurShowType = enumQueryTypeDefineSelf;

		_GetCurLogInfo(enumQueryTypeDefineSelf, strFrom, strTo);
		_ShowStatList(7);
	}
	
}

BOOL KMainDlg::_GetCurLogInfo(IN enumQueryType enumType)
{
	CComPtr<ISQLiteComResultSet3> spiRet = NULL;
	if (m_FlowStatLog.QueryData(enumType, spiRet))
	{
		m_vLogListInfo.clear();

		while (!spiRet->IsEof())
		{
			stLogListInfo listInfo;
			listInfo.Init();
			listInfo.strMinTime = spiRet->GetAsString(0);
			listInfo.strMaxTime = spiRet->GetAsString(1);
			listInfo.lUpFlow = _wtoi64(spiRet->GetAsString(2));
			listInfo.lDownFlow = _wtoi64(spiRet->GetAsString(3));
			listInfo.lAllFlow = _wtoi64(spiRet->GetAsString(4));
			listInfo.lAllTime = spiRet->GetInt(5);

			m_vLogListInfo.push_back(listInfo);

			spiRet->NextRow();
		}

	/*	if (m_vLogListInfo.size() > 0 && enumType == enumQueryTypeEveryDay)
		{
			m_nTotalSendBegin = m_vLogListInfo[0].lUpFlow;
			m_nTotalRecvBegin = m_vLogListInfo[0].lDownFlow;
			m_nTotalAllBegin = m_vLogListInfo[0].lAllFlow;
			m_nTimeWatchBegin = m_vLogListInfo[0].lAllTime;
			m_nTotalAllBeginForMonth = m_nTotalAllBegin;
		}
		*/
	}

	return TRUE;
}

BOOL KMainDlg::_GetCurLogInfo(OUT LONGLONG& uUpData, OUT LONGLONG& uDownData, OUT LONGLONG& uAllData)
{
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	CString strDayQuery;
	strDayQuery.Format(_T("%4d-%02d-%02d"), systm.wYear, systm.wMonth, systm.wDay);

	CComPtr<ISQLiteComResultSet3> spiRet = NULL;
	if (m_FlowStatLog.QueryData(enumQueryTypeEveryDay, strDayQuery, spiRet))
	{
		if (!spiRet->IsEof())
		{
			uUpData = _wtoi64(spiRet->GetAsString(2));
			uDownData = _wtoi64(spiRet->GetAsString(3));
			uAllData = _wtoi64(spiRet->GetAsString(4));
		}
	}

	return TRUE;
}


BOOL KMainDlg::_GetCurLogInfo(IN enumQueryType enumType, IN CString strFrwo, IN CString strTo)
{
	if (enumType != enumQueryTypeDefineSelf)
		return _GetCurLogInfo(enumType);

	CComPtr<ISQLiteComResultSet3> spiRet = NULL;
	if (m_FlowStatLog.QueryData(strFrwo, strTo, spiRet))
	{
		m_vLogListInfo.clear();

		while (!spiRet->IsEof())
		{
			stLogListInfo listInfo;
			listInfo.Init();
			listInfo.strMinTime = spiRet->GetAsString(0);
			listInfo.strMaxTime = listInfo.strMinTime;
			listInfo.lUpFlow = _wtoi64(spiRet->GetAsString(1));
			listInfo.lDownFlow = _wtoi64(spiRet->GetAsString(2));
			listInfo.lAllFlow = _wtoi64(spiRet->GetAsString(3));
			listInfo.lAllTime = spiRet->GetInt(4);

			m_vLogListInfo.push_back(listInfo);

			spiRet->NextRow();
		}
/*
		if (m_vLogListInfo.size() > 0)
		{
			m_nTotalSendBegin = m_vLogListInfo[0].lUpFlow;
			m_nTotalRecvBegin = m_vLogListInfo[0].lDownFlow;
			m_nTotalAllBegin = m_vLogListInfo[0].lAllFlow;
			m_nTimeWatchBegin = m_vLogListInfo[0].lAllTime;
		}*/
	}

	return TRUE;
}

BOOL KMainDlg::_InitStatList( void )
{
	m_pNetStatListBox = new CBkNetMonitorListBox;
	if (NULL == m_pNetStatListBox)
		return FALSE;

	_GetCurLogInfo(enumQueryTypeEveryDay);
	
	//listbox
	m_pNetStatListBox->Create( GetViewHWND(), TAB_SHOW_STAT_WINDOW);
	m_pNetStatListBox->Load(IDR_BK_LISTBOX_STATINFO);
	m_pNetStatListBox->SetCanGetFocus(FALSE);

	_ShowPageForList();

//	m_fluxStatRead.Init();

	_GetAndShowProcessInfo();
	
	_ShowRemindInfo();

	SetTimer(ID_TIMER_UPDATE_STAT_INFO, 30000, NULL);

	PostMessage(WM_TIMER, ID_TIMER_UPDATE_STAT_INFO, 0);

	return TRUE;
}

LRESULT KMainDlg::OnStatListGetDispInfo( LPNMHDR pnmh )
{
	if (NULL == m_pNetStatListBox)
		return E_POINTER;

	BKLBMGETDISPINFO* pdi = (BKLBMGETDISPINFO*)pnmh;
	int nListItemID = pdi->nListItemID + (m_nCurPage[m_nCurShowType] - 1) * DEF_PAGE_COUNT;

	if (nListItemID >= m_vLogListInfo.size())
		return S_FALSE;

	m_pNetStatListBox->SetItemText(DEF_BEGIN_STAT_LIST_403, _GetStatListInfoTime(m_vLogListInfo[nListItemID].strMinTime, m_vLogListInfo[nListItemID].strMaxTime, m_nCurShowType));
	m_pNetStatListBox->SetItemText(DEF_BEGIN_STAT_LIST_404, _GetStatListInfoUpAllFlow(m_vLogListInfo[nListItemID].lUpFlow));
	m_pNetStatListBox->SetItemText(DEF_BEGIN_STAT_LIST_405, _GetStatListInfoDownAllFlow(m_vLogListInfo[nListItemID].lDownFlow));
	m_pNetStatListBox->SetItemText(DEF_BEGIN_STAT_LIST_406, _GetStatListInfoAllFlow(m_vLogListInfo[nListItemID].lAllFlow));
	m_pNetStatListBox->SetItemText(DEF_BEGIN_STAT_LIST_407, _GetStatListMoniterTime(m_vLogListInfo[nListItemID].lAllTime));

	return S_OK;
}

LRESULT KMainDlg::OnStatListGetmaxHeight( LPNMHDR pnmh )
{
	BKLBITEMCALCMAXITEM *pdi = (BKLBITEMCALCMAXITEM*)pnmh;
	pdi->nMaxHeight = 32;
	return 0;
}

LRESULT KMainDlg::OnStatListGetItemHeight( LPNMHDR pnmh )
{
	BKLBITEMCACLHEIGHT *pdi	= (BKLBITEMCACLHEIGHT*)pnmh;
	if (CListBoxData::GetDataPtr()->GetMaxHeightSel() == pdi->nListItemId)
		pdi->nHeight = 32;
	else
		pdi->nHeight = 32;
	
	return 0;
}

CString KMainDlg::_GetStatListInfoTime( IN CString strTimeMin, IN CString strTimeMax, IN int nMode)
{
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	CString strCurTime;
	strCurTime.Format(_T("%d-%02d-%02d"), sysTime.wYear, sysTime.wMonth, sysTime.wDay);

	if (nMode == enumQueryTypeEveryDay || nMode == enumQueryTypeDefineSelf)
	{
		if (strCurTime.CompareNoCase(strTimeMin) == 0)
			return _T("今日");
		else
			return strTimeMax;
	}
	else if (nMode == enumQueryTypeEveryWeek)
	{
		if (strCurTime <= strTimeMax && strCurTime >= strTimeMin)
			return _T("本周");
		else
		{
			CString strToDates = strTimeMax.Mid(5);
			CString strReturn;
			strReturn.Format(_T("%s~%s"), strTimeMin, strToDates);
			return strReturn;
		}
	}
	else if (nMode == enumQueryTypeEveryMonth)
	{
		WORD dwYear = _wtol(strTimeMin.Left(4));
		WORD dwMonth = _wtol(strTimeMin.Mid(5, 2));
		if (dwYear == sysTime.wYear && dwMonth == sysTime.wMonth)
			return _T("本月");
		else
		{
			CString strReturn;
			strReturn = strTimeMin.Left(7);
			return strReturn;
		}
	}
/*	else if (nMode == enumQueryTypeDefineSelf)
	{
		CString strDates;
		strDates.Format(_T("%s"), strTimeMin);
		return strDates;
	}
	*/

	return strTimeMin;
}

CString KMainDlg::_GetStatListInfoUpAllFlow( IN LONGLONG lUpAllFlow )
{
	return _GetNeedShowData(lUpAllFlow * 1024);
}

CString KMainDlg::_GetStatListInfoDownAllFlow( IN LONGLONG lDownAllFlow )
{
	return _GetNeedShowData(lDownAllFlow * 1024);
}

CString KMainDlg::_GetStatListInfoAllFlow( IN LONGLONG lAllFlow )
{
	return _GetNeedShowData(lAllFlow * 1024);
}

CString KMainDlg::_GetEveryTime(IN long& lMoniterTime, IN long lBeCalcResidue,IN CString strUnit)
{
	int nValue = 0;
	nValue = lMoniterTime / lBeCalcResidue;
	lMoniterTime -= nValue * lBeCalcResidue;

	CString strRet = _T("");
	if (nValue > 0)
		strRet.Format(_T("%ld%s"), nValue, strUnit);

	return strRet;
}

CString KMainDlg::_GetStatListMoniterTime( IN long lMoniterTime )
{

	CString strRet = _T("");
	strRet.Append(_GetEveryTime(lMoniterTime, (60 * 60 * 24), L"天"));
	strRet.Append(_GetEveryTime(lMoniterTime, (60 * 60), L"小时"));
	strRet.Append(_GetEveryTime(lMoniterTime, 60, L"分钟"));
	strRet.Append(_GetEveryTime(lMoniterTime, 1, L"秒"));

	return strRet;
}

void KMainDlg::_SetFlowStatInfo( IN CString strAllFlow, IN CString strUpFlow, IN CString strDownFlow )
{
	CString strAllFlowT = _T("总流量    ：");
	CString strUpFlowT = _T("上传总流量：");
	CString strDownFlowT = _T("下载总流量：");

	SetItemText(DEF_RIGHT_ALLFLOW, strAllFlowT + strAllFlow);
	SetItemText(DEF_UP_ALLFLOW, strUpFlowT + strUpFlow);
	SetItemText(DEF_DOWN_ALLFLOW, strDownFlowT + strDownFlow);
}

void KMainDlg::_ShowPageForList()
{
	KLocker locker(m_pShowStatLock);

	CString strShowPage;
	strShowPage.Format(_T("%ld"), m_nCurPage[m_nCurShowType]);
	SetItemText(DEF_SHOW_CUR_PAGE, strShowPage);
	
	int nCurType = m_nCurShowType;
	if (m_vLogListInfo.size() <= DEF_PAGE_COUNT)
	{
		EnableItem(DEF_SHOW_FIRST, FALSE);
		EnableItem(DEF_SHOW_BEFORE, FALSE);
		EnableItem(DEF_SHOW_NEXT, FALSE);
		EnableItem(DEF_SHOW_ENDP_PAGE, FALSE);
		m_pNetStatListBox->SetItemCount(m_vLogListInfo.size());
		return;
	}
	else if (m_vLogListInfo.size() > DEF_PAGE_COUNT)
	{
		if (m_vLogListInfo.size() < m_nCurPage[nCurType] * DEF_PAGE_COUNT)
		{
			EnableItem(DEF_SHOW_FIRST, TRUE);
			EnableItem(DEF_SHOW_BEFORE, TRUE);
			EnableItem(DEF_SHOW_NEXT, FALSE);
			EnableItem(DEF_SHOW_ENDP_PAGE, FALSE);
			m_pNetStatListBox->SetItemCount((m_vLogListInfo.size() - ((m_nCurPage[nCurType] - 1) * DEF_PAGE_COUNT)));
			return;
		}
		else if (m_nCurPage[nCurType] == 1)
		{
			EnableItem(DEF_SHOW_FIRST, FALSE);
			EnableItem(DEF_SHOW_BEFORE, FALSE);
			EnableItem(DEF_SHOW_NEXT, TRUE);
			EnableItem(DEF_SHOW_ENDP_PAGE, TRUE);
		}
		else
		{
			EnableItem(DEF_SHOW_FIRST, TRUE);
			EnableItem(DEF_SHOW_BEFORE, TRUE);
			EnableItem(DEF_SHOW_NEXT, TRUE);
			EnableItem(DEF_SHOW_ENDP_PAGE, TRUE);
		}
	}

	m_pNetStatListBox->SetItemCount(DEF_PAGE_COUNT);
}

void KMainDlg::OnClickShowFirstPage( void )
{
	KLocker locker(m_pShowStatLock);
	m_nCurPage[m_nCurShowType] = 1;
	m_pNetStatListBox->SetItemCount(DEF_PAGE_COUNT);
	_ShowPageForList();
}

void KMainDlg::OnClickShowBeforePage( void )
{
	KLocker locker(m_pShowStatLock);
	m_nCurPage[m_nCurShowType]--;
	m_pNetStatListBox->SetItemCount(DEF_PAGE_COUNT);
	_ShowPageForList();
}

void KMainDlg::OnClickShowNextPage( void )
{
	KLocker locker(m_pShowStatLock);
	m_nCurPage[m_nCurShowType]++;
	m_pNetStatListBox->SetItemCount(DEF_PAGE_COUNT);
	_ShowPageForList();
}

void KMainDlg::OnClickShowEndPage( void )
{
	KLocker locker(m_pShowStatLock);
	m_nCurPage[m_nCurShowType] = 1 + (m_vLogListInfo.size() - 1) / DEF_PAGE_COUNT ;
	m_pNetStatListBox->SetItemCount(DEF_PAGE_COUNT);
	_ShowPageForList();
}


void KMainDlg::OnClickWindowSetting( void )
{
	int nCmdLen = MAX_PATH * 2;
	wchar_t szFilePath[MAX_PATH * 2] = {0};
	DWORD dwCode = ::GetModuleFileName(NULL, szFilePath, nCmdLen);
	if (dwCode == 0)
		return;

	::PathRemoveFileSpecW(szFilePath);
	::PathAppend(szFilePath, TEXT("ksafe.exe"));

	if (::PathFileExists(szFilePath))
	{
		CString strCmd = _T("-do:ui_setting_SysOpt");
		ShellExecute(NULL, _T("open"), szFilePath, strCmd, NULL, SW_SHOW);
	}
}

BOOL KMainDlg::_SetFlowList( IN int nPos, IN const CString& strSrcFilePath, IN const CString& strNeedShowStr, IN const CString& strShowSize )
{
	int nId = (nPos - 1)  *  3 + DEF_FLOW_LIST_BEGIN;

	CString strNeedTerm = strNeedShowStr;
	strNeedTerm.Trim();
	if (strNeedTerm.IsEmpty())
		return FALSE;
	
	SetItemAttribute(nId, "srcfile", 0);

	CString strTemp = strNeedShowStr;

	//GetTextExternPoint32();
	HDC hdc = GetDC(); // 获得相关设备句柄

	SIZE sz = {0};
	GetTextExtentPoint32(hdc, strNeedShowStr, strNeedShowStr.GetLength(), &sz);
	if (sz.cx > 120)
	{
		
		for (int i = 4; i < strNeedShowStr.GetLength(); i++)
		{
			strTemp = strNeedShowStr.Left(i);
			SIZE sz2 = {0};
			GetTextExtentPoint32(hdc, strTemp, i, &sz2);
			if (sz2.cx > 110)
			{
				if (sz2.cx < 115)
					strTemp.Append(_T("..."));
				else 
				{
					strTemp = strTemp.Left(strTemp.GetLength() -1);
					strTemp.Append(_T("..."));
				}
				
				break;
			}
			
		}
		
	}
	ReleaseDC(hdc);

	if (::PathFileExistsW(strSrcFilePath))
		SetItemAttribute(nId, "srcfile", CW2A(strSrcFilePath));
	else
	{
		char pszValue[260] = {0};
		_snprintf_s(pszValue, sizeof(pszValue), "%d", CListBoxData::GetDataPtr()->GetDefaultIcon());
		SetItemAttribute(nId, "iconhandle", pszValue);
	}


	CStringA strPosA;
	SetItemStringAttribute(nId + 1, "tip", strNeedShowStr);

	SetItemText(nId + 1, strTemp);
	SetItemText(nId + 2, strShowSize);
		
	return TRUE;
}

void KMainDlg::OnClickShowMore( void )
{
	char pszValue[260] = {0};
	_snprintf_s(pszValue, sizeof(pszValue), "%d", CListBoxData::GetDataPtr()->GetDefaultIcon());

	KFlowStatList listDlg;
	listDlg.SetInfoPointer(&m_vFlowStatPList, pszValue, &m_FlowStatLog);
	listDlg.DoModal();
}

bool KMainDlg::stl_SortPorcessList( KFluxStasticProcItemEx& ItemData1, KFluxStasticProcItemEx& ItemData2 )
{
	if ((ItemData1.itemElem.m_nTotalRecv + ItemData1.itemElem.m_nTotalSend) >= 
		(ItemData2.itemElem.m_nTotalRecv + ItemData2.itemElem.m_nTotalSend))
		return true;
	else 
		return false;
}

void KMainDlg::_GetAndShowProcessInfo( void )
{
	KNetFluxStasticCacheReader	fluxStatRead;
	if (E_FAIL == fluxStatRead.Init())
		return;

	KStasticFluxProcessList* pFluxStatRead = fluxStatRead.GetStasticFluxList();
	if (pFluxStatRead == NULL)
		return ;

	pFluxStatRead->m_lock.LockRead();

//	m_processListSave = *m_pFluxStatRead;

	m_vFlowStatPList.clear();
	m_statMap.clear();
	for (int i = 0; i < pFluxStatRead->m_nCurrentCnt; i++)
	{
		KFluxStasticProcItemEx item;
		item.Init();
		CopyMemory((void *)&(item.itemElem), (void *)&pFluxStatRead->m_Items[i], sizeof(KFluxStasticProcItem));
		std::map<CString, KFluxStasticProcItemEx>::iterator iter = m_statMap.find(item.itemElem.m_strProcPath);
		if (iter != m_statMap.end())
		{
			iter->second.itemElem.m_nTotalRecv += item.itemElem.m_nTotalRecv;
			iter->second.itemElem.m_nTotalSend += item.itemElem.m_nTotalSend;
		}
		else
		{
			m_statMap.insert(StatMapPair(item.itemElem.m_strProcPath, item));	
		}
	}
	
	//显示使用统计
	LONGLONG uUpData = 0, uDownData = 0, uAllData = 0;
	_GetCurLogInfo(uUpData, uDownData, uAllData);
	_SetFlowStatInfo(_GetNeedShowData(uAllData * 1024),
					_GetNeedShowData(uUpData * 1024),
					_GetNeedShowData(uDownData * 1024));

	CString strTimerData;
	CTime time(pFluxStatRead->m_nTimeTodayStart);

	SYSTEMTIME sys;
	time.GetAsSystemTime(sys);
	strTimerData.Format(_T("%d-%02d-%02d"), sys.wYear, sys.wMonth, sys.wDay);
	
	pFluxStatRead->m_lock.UnLockRead();
	fluxStatRead.Uninit();


	std::map<CString, KFluxStasticProcItemEx>::iterator iter = m_statMap.begin();
	for (; iter != m_statMap.end(); iter++)
	{
		//获取显示的名字
		KSProcessInfo	ProcInfo;
		if (0 == CListBoxData::GetDataPtr()->QueryProcInfo(iter->second.itemElem.m_strProcPath, ProcInfo))
			iter->second.m_strProcessName = ProcInfo.strDisplayName.c_str();
		else
		{
			CString strName = iter->second.itemElem.m_strProcPath;
			int nPos = strName.ReverseFind('\\');
			strName = strName.Mid(nPos + 1);
			iter->second.m_strProcessName = strName;
		}

		if (wcslen(iter->second.itemElem.m_strProcPath) > 0)
			m_vFlowStatPList.push_back(iter->second);
	}
	
	//排序
	std::sort(m_vFlowStatPList.begin(), m_vFlowStatPList.end(), stl_SortPorcessList);

	_SetStatFlowShow();
	

	//显示今日网络流量占用排行榜
	int nCurPos = 1;
	for (int i = 0; ; i++)
	{
		if (i >= m_vFlowStatPList.size())
			break;

		if (_SetFlowList(nCurPos, m_vFlowStatPList[i].itemElem.m_strProcPath, m_vFlowStatPList[i].m_strProcessName, _GetNeedShowData(m_vFlowStatPList[i].itemElem.m_nTotalRecv + m_vFlowStatPList[i].itemElem.m_nTotalSend)))
			nCurPos++;

		if (nCurPos == 6)
			break;
	}
		
}

void KMainDlg::_GetRemindInfo( OUT int& nDay, OUT int& nMonth )
{
	ATL::CString cfgFileNamePath = L"\\cfg\\floatwnd.ini";
	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	ATL::CString trayPath;
	trayPath = buffer;
	cfgFileNamePath = trayPath + cfgFileNamePath;

	int defaultValue = 0;
	nDay = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_REMAID_DAY_NAME, defaultValue, cfgFileNamePath);
	nMonth = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_REMAID_MONTH_NAME, defaultValue, cfgFileNamePath);
}

void KMainDlg::_GetRemindLastData( OUT int& nDayData, OUT int& nMonthData )
{
	ATL::CString cfgFileNamePath = L"\\cfg\\floatwnd.ini";
	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	ATL::CString trayPath;
	trayPath = buffer;
	cfgFileNamePath = trayPath + cfgFileNamePath;

	int defaultValue = 0;
	nDayData = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_REMAID_DAY_DATA_NAME, defaultValue, cfgFileNamePath);
	nMonthData = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_REMAID_MONTH_DATA_NAME, defaultValue, cfgFileNamePath);

}


void KMainDlg::_InitFlowRemindInfo(void)
{
	int nDay = 0;
	int nMonth = 0;
	int nDayData = 0;
	int nMonthData = 0;
	int nDayMark = 0;
	int nMonthMark = 0;
	int nWaitTime = 0;

	_GetRemindInfo(nDay, nMonth);
	_GetRemindLastData(nDayData, nMonthData);
	_GetRemindMarkInfo(nDayMark, nMonthMark);
	nWaitTime = _GetHadWaitTenMuniters();

	KOperMemFile::Instance().SetDayLimited(nDay);
	KOperMemFile::Instance().SetMonthLimited(nMonth);
	KOperMemFile::Instance().SetDayMark(nDayMark);
	KOperMemFile::Instance().SetMonthMark(nMonthMark);
	KOperMemFile::Instance().SetDayLastData(nDayData);
	KOperMemFile::Instance().SetMonthLastData(nMonthData);
	KOperMemFile::Instance().SetMonthWait(nWaitTime);
}

int KMainDlg::_GetHadWaitTenMuniters(void)
{
	ATL::CString cfgFileNamePath = L"\\cfg\\floatwnd.ini";
	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	ATL::CString trayPath;
	trayPath = buffer;
	cfgFileNamePath = trayPath + cfgFileNamePath;

	int defaultValue = 0;
	int nDelayTime = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_REMAID_WAITTIME, defaultValue, cfgFileNamePath);
	return nDelayTime;
}

void KMainDlg::_GetRemindMarkInfo( OUT int& nDayMark, OUT int& nMonthMark )
{
	ATL::CString cfgFileNamePath = L"\\cfg\\floatwnd.ini";
	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	ATL::CString trayPath;
	trayPath = buffer;
	cfgFileNamePath = trayPath + cfgFileNamePath;

	int defaultValue = 0;
	nDayMark = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_REMAID_DAY_MARK_NAME, defaultValue, cfgFileNamePath);
	nMonthMark = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_REMAID_MONTH_MARK_NAME, defaultValue, cfgFileNamePath);
}

void KMainDlg::_ShowRemindInfo( void )
{
	int nDay = KOperMemFile::Instance().GetDayLimited();
	int nMonth = KOperMemFile::Instance().GetMonthLimited();

	//_GetRemindInfo(nDay, nMonth);
	CString strShowInfo;
	strShowInfo.Format(_T("当前设置：当日流量超过%dMB，当月流量超过%dMB时提醒。"), nDay, nMonth);
	SetItemText(NET_MONITOR_263, strShowInfo);
}

BOOL KMainDlg::_GetAllDataFromDB(IN enumQueryType enumType, IN CString strCurTime, OUT LONGLONG& nRet)
{
	CComPtr<ISQLiteComResultSet3> spiRet = NULL;
	if (m_FlowStatLog.QueryData(enumType, strCurTime, spiRet))
	{
	//	m_vLogListInfo.clear();

		if (!spiRet->IsEof())
		{
			nRet = _wtoi64(spiRet->GetAsString(4));
			return TRUE;
		}
	}

	return FALSE;
}

BOOL KMainDlg::_SetDelayTenMuniter(void)
{
	ATL::CString cfgFileNamePath = L"\\cfg\\floatwnd.ini";
	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	ATL::CString trayPath;
	trayPath = buffer;
	cfgFileNamePath = trayPath + cfgFileNamePath;

	int nTime = _time32(NULL);
	CString strCurTime;
	strCurTime.Format(_T("%ld"), nTime);
	::WritePrivateProfileStringW(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_REMAID_WAITTIME, strCurTime, cfgFileNamePath);
	KOperMemFile::Instance().SetMonthWait(nTime);
	return TRUE;
}

BOOL KMainDlg::_HadWaitTenMuniters(void)
{

	int nDelayTime = KOperMemFile::Instance().GetMonthWait();

	int nCurTime = _time32(NULL);
	if (nCurTime - nDelayTime >= 10 * 60)
		return TRUE;

	return FALSE;
}

long KMainDlg::_GetFormatTime(int nYear, int nMonth, int nDay)
{
	return nYear * (365) + nMonth * (30) + nDay ;
}

void KMainDlg::_ShowNetMintorRemindDlg( IN int nIsInit )
{

	int nDayMark = KOperMemFile::Instance().GetDayMark();
	int nMonthMark = KOperMemFile::Instance().GetMonthMark();
	int nNeedShow = 0;
	int nDay = KOperMemFile::Instance().GetDayLimited();
	int nMonth = KOperMemFile::Instance().GetMonthLimited();
	int nDayData = KOperMemFile::Instance().GetDayLastData();
	int nMonthData = KOperMemFile::Instance().GetMonthLastData();

//	_GetRemindInfo(nDay, nMonth);
//	_GetRemindLastData(nDayData, nMonthData);
//	_GetRemindMarkInfo(nDayMark, nMonthMark);

	CTime tm(nDayMark);
	CTime tm2(nDayData);
	CTime tm3(nMonthData);
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	long nCurTime = _time32(NULL);
	CString strDayQuery;
	CString strMonthQuery;
	strDayQuery.Format(_T("%4d-%02d-%02d"), systm.wYear, systm.wMonth, systm.wDay);
	strMonthQuery.Format(_T("%ld"), systm.wYear * 100 + systm.wMonth);

	if ((nDay > 0) 
		&&(nCurTime - nDayData >= 14400)
		&&(_GetFormatTime(systm.wYear, systm.wMonth, systm.wDay) -  _GetFormatTime(tm.GetYear(), tm.GetMonth(), tm.GetDay()) > 0))
		nNeedShow = 1;

	int nCurMonth = systm.wYear * 100 + systm.wMonth;
	if ((nMonth > 0) &&  (nCurMonth > nMonthMark) && (nCurTime - nMonthData >= 14400))
		nNeedShow += 2;

	BOOL bIsRunDay = FALSE;

	if ((nNeedShow & 1))
	{
		LONGLONG nAllFlow = 0;
		if (_GetAllDataFromDB(enumQueryTypeEveryDay, strDayQuery, nAllFlow))
		{
			if (nAllFlow >= LONGLONG(nDay * 1024))
			{//显示日超标
				bIsRunDay = TRUE;
				_ShowDialogRemind(DEFMODEISDAY, nDay, nIsInit);
			}
		}
	}

	if ((nNeedShow & 2))
	{
		LONGLONG nAllFlow = 0;
		if (_GetAllDataFromDB(enumQueryTypeEveryMonth, strMonthQuery, nAllFlow))
		{
			if (nAllFlow >= LONGLONG(nMonth * 1024))
			{//显示月超标

				if (bIsRunDay)
					_SetDelayTenMuniter();
				else
				{
					if (_HadWaitTenMuniters())
						_ShowDialogRemind(DEFMODEISMONTH, nMonth, nIsInit);
				}

			}
		}
	}

	/*
	if ((nNeedShow & 1))
	{
		if (m_nRecordEveryDays == 0)
			_GetAllDataFromDB(enumQueryTypeEveryDay, m_nRecordEveryDays);

		if (m_nRecordEveryDays >= nDay * 1024)
		{//显示日超标
			bIsRunDay = TRUE;
			_ShowDialogRemind(DEFMODEISDAY, nDay, nIsInit);
		}
	}

	if ((nNeedShow & 2))
	{
		if (m_nRecordEveryMonth == 0)
			_GetAllDataFromDB(enumQueryTypeEveryMonth, m_nRecordEveryMonth);
		if (m_nRecordEveryMonth >= nMonth * 1024)
		{//显示月超标

			if (bIsRunDay)
				_SetDelayTenMuniter();
			else
			{
				if (_HadWaitTenMuniters())
					_ShowDialogRemind(DEFMODEISMONTH, nMonth, nIsInit);
			}

		}
	}
	*/
}

void KMainDlg::_ShowDialogRemind( IN int nMode, IN int nSetSize, int nIsInit )
{

//	SendMessage(WM_UPLIVE_SHOW);
//	CenterWindow();
	int nRet = 0;

	{
		m_statListdlg.SetConfig(nMode, nSetSize);
		if (!m_statListdlg.IsShowDlg())
		{
			nRet = m_statListdlg.DoModal();
			m_statListdlg.SetIsShowDlg(FALSE);
		}
	}
	

	if (nRet == DEFRESULTMORESETTING)//选择修改设置
	{
		OnClickWindowSetting();
		if (nIsInit)
			BtnClose();
			//EndDialog(IDOK);
	}
	else if (nRet == DEFRESULTNETMOINTERCONTRAL)//选择流量控制
	{
		SetItemVisible(TAB_SHOW_FLOW_STAT, FALSE);
		SetTabCurSel(TAB_MAIN, 0);
		SendMessage(WM_UPLIVE_SHOW);
		//SetItemAttribute(TAB_MAIN, "cursel", "0");
	}
	else if (nRet == DEFRESULTSTATINFO) //选择流量统计
	{
		SetTabCurSel(TAB_MAIN, 1);
		SendMessage(WM_UPLIVE_SHOW);
		if (nMode == DEFMODEISMONTH)
		{
			if (!nIsInit)
				OnClickShowStatMonths();
			else
				m_nCurShowType = enumQueryTypeEveryMonth;			
		}
	}
	else if (nRet == IDCANCEL && nIsInit)
	{
		_ReleaseThread();
		EndDialog(IDOK);
	}
		//SetItemAttribute(TAB_MAIN, "cursel", "1");
}

void KMainDlg::_SetStatFlowShow( void )
{
	if (!m_bNetMonEnabled)
	{
		SetRichText(DEF_SHOW_DESCRIBE, BkString::Get(DefStatCloseNetMoniter));
		SetItemVisible(TAB_SHOW_STAT_CSETTING, FALSE);
		return;
	}

	CString strHadUse;
	SetItemVisible(TAB_SHOW_STAT_CSETTING, TRUE);

	int nDay = 0, nMonth = 0;
	_GetRemindInfo(nDay, nMonth);

	SYSTEMTIME systm;
	GetLocalTime(&systm);
	CString strDayQuery;
	CString strMonthQuery;
	strDayQuery.Format(_T("%4d-%02d-%02d"), systm.wYear, systm.wMonth, systm.wDay);
	strMonthQuery.Format(_T("%ld"), systm.wYear * 100 + systm.wMonth);

	/*
	if (nDay > 0)
		{
			if (nMonth > 0)
				FormatRichText(DEF_SHOW_DESCRIBE, BkString::Get(DefStatCloseSetDayM), nDay, nMonth);
			else if (nMonth <= 0)
			{
				if (m_nRecordEveryDays == 0)
					_GetAllDataFromDB(enumQueryTypeEveryDay, m_nRecordEveryDays);
				FormatRichText(DEF_SHOW_DESCRIBE, BkString::Get(DefStatCloseSetDay), nDay, _GetNeedShowData(m_nRecordEveryDays * 1024));
			}
		}
		else if (nMonth > 0)
		{
			if (m_nRecordEveryMonth == 0)
				_GetAllDataFromDB(enumQueryTypeEveryMonth, m_nRecordEveryMonth);
			FormatRichText(DEF_SHOW_DESCRIBE, BkString::Get(DefStatCloseSetMonth), nMonth, _GetNeedShowData(m_nRecordEveryMonth * 1024));
		}*/
	if (nDay > 0)
	{
		if (nMonth > 0)
			FormatRichText(DEF_SHOW_DESCRIBE, BkString::Get(DefStatCloseSetDayM), nDay, nMonth);
		else if (nMonth <= 0)
		{
			LONGLONG nDayValue = 0;
			_GetAllDataFromDB(enumQueryTypeEveryDay, strDayQuery, nDayValue);
			FormatRichText(DEF_SHOW_DESCRIBE, BkString::Get(DefStatCloseSetDay), nDay, _GetNeedShowData(nDayValue * 1024));
		}
	}
	else if (nMonth > 0)
	{
		LONGLONG nMonthValue = 0;
		_GetAllDataFromDB(enumQueryTypeEveryMonth, strMonthQuery, nMonthValue);
		FormatRichText(DEF_SHOW_DESCRIBE, BkString::Get(DefStatCloseSetMonth), nMonth, _GetNeedShowData(nMonthValue * 1024));
	}
	else
		FormatRichText(DEF_SHOW_DESCRIBE, BkString::Get(DefStatCloseSetNone));

}

LRESULT KMainDlg::ShowDay( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	int nDay = 0, nMonth = 0;
	_GetRemindInfo(nDay, nMonth);
	_ShowDialogRemind(DEFMODEISDAY, nDay);	
	return S_OK;
}

LRESULT KMainDlg::ShowMonth( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	int nDay = 0, nMonth = 0;
	_GetRemindInfo(nDay, nMonth);
	_ShowDialogRemind(DEFMODEISMONTH, nMonth);
	return S_OK;
}
//流量统计end
//////////////////////////////////////////////////////////////////////////


BOOL KMainDlg::IsFloatWndStartWithTray()
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
	int result = ::GetPrivateProfileInt(FLOATWND_CONFIG_SECTION_NAME,  FLOATWND_CONFIG_DISPLAY_WITH_TRAY_KEY_NAME, defaultValue, cfgFileNamePath);

	return result == 1;
}

void KMainDlg::OpenFloatWnd()
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
