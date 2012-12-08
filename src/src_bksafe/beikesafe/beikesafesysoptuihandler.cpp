#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "beikesafesysoptuihandler.h"
#include "beikesafemsgbox.h"
#include "trayruncommmon.h"
#include <safemon\safetrayshell.h>
#include <bksafe/bksafeconfig.h>
#include "Md5Helper.h"
#include <iefix/iefixeng.h>
#include "beikesafesysoptignoredlg.h"
#include <iefix/cinifile.h>
#include <runoptimize/restorerunitem.h>
#include "msgdlg_disableby3xx.h"
#include "msgdlg_delaytips.h"
#include "msgbox_sysopt.h"
#include <runoptimize/x64helper.h>
#include <kws/KwsSetting.h>
#include "softmgr/URLEncode.h"
using namespace IniFileOperate;

#define FILEPATH			L"\\Cfg\\bksafe.ini"
#define SEC_RUNOPT			L"runopt"
#define KEY_RUNONCE			L"runonce"

#define HISTORY_LIST_TITLE_HEIGHT	25
#define HISTORY_LIST_ITEM_HEIGHT		50

static BOOL IsOpTrayRunItem(DWORD nRunType, LPCTSTR lpstrName)
{
	if (_tcsicmp(lpstrName,_T("KSafeTray"))==0)
	{
		if (IsHKLMRunType(nRunType))
			return TRUE;
	}
	return FALSE;
}

void CBeikeSafeSysOptUIHandler::Init()
{
	m_RunList.SetOpCallback(this);

	m_RunList.Create( 
		m_dlg->GetViewHWND(), NULL, NULL, 
		WS_CLIPCHILDREN | WS_CHILD |WS_VSCROLL | LBS_OWNERDRAWVARIABLE| LBS_HASSTRINGS , 
		0, IDC_LST_RUNOPT, NULL);
	m_RunList.CreateX();

	m_OneKeyOptList.Create( m_dlg->GetViewHWND(), 
		NULL, NULL, 
		WS_CLIPCHILDREN | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_OWNERDRAWFIXED | LVS_NOCOLUMNHEADER, 
		0, IDC_LST_RUN_AUTOOPT, NULL);
	m_OneKeyOptList.CreateToolTip();

	m_OneKeyOptList.SetCallback(this);
	m_OneKeyOptList.InsertColumn(0, BkString::Get(IDS_SYSOPT_4016));
	m_OneKeyOptList.SetColumnWidth(0, 160);
	m_OneKeyOptList.InsertColumn(1, BkString::Get(IDS_SYSOPT_4017));
	m_OneKeyOptList.SetColumnWidth(1, 330);
	m_OneKeyOptList.InsertColumn(2, BkString::Get(IDS_SYSOPT_4018));
	m_OneKeyOptList.SetColumnWidth(2, 100);
	m_OneKeyOptList.InsertColumn(3, BkString::Get(IDS_SYSOPT_4018));
	m_OneKeyOptList.SetColumnWidth(3, 45);

	m_pHistoryList = new CBkListBox;
	m_pHistoryList->Create(m_dlg->GetViewHWND(), IDC_LST_RUNOPT_HISTORY);
	m_pHistoryList->Load(IDR_BK_SYSOPT_HISTORY_LIST);

	CAppPath::Instance().GetLeidianAppPath(m_strIniFilePath);
	m_strIniFilePath.Append(FILEPATH);
	CAppPath::Instance().GetLeidianAppPath(m_strIgnoredIniPath);
	m_strIgnoredIniPath.Append(IGNORED_FILEPATH);

	_InitKSafeToolsPage();

	//延迟加载系统工具集页面
	m_dlg->SetTimer(TIMER_ID_DELAYLOAD_KSAFE_TOOL, 5000);
}

BOOL CBeikeSafeSysOptUIHandler::_FindRunItem( CKsafeRunInfo* pInfo )
{
	CKsafeRunInfo*	pNewInfo = pInfo;
	m_arrayItem.Add(pNewInfo);
	if (pNewInfo->bEnable == KSRUN_START_DELAY)
		m_nDelayCount++;

	SHFILEINFO		hFileInfo = {0};
	if (!pNewInfo->bSystem)
	{
		KWow64Switcher switcher;
		switcher.Init();
		BOOL bIsx64 = switcher.IsWin64();
		if (bIsx64)
			switcher.Close();
		::SHGetFileInfo(pNewInfo->strExePath,FILE_ATTRIBUTE_NORMAL,&hFileInfo,sizeof(SHFILEINFO),SHGFI_ICON);

		if (bIsx64)
			switcher.Revert();
	}

	if ( pInfo->nType == KSRUN_TYPE_STARTUP && IsOpTrayRunItem(pInfo->nRegType,pInfo->strName) )
		m_bDelayRunValid = pInfo->bEnable;

	m_RunList.InsertItemX(pNewInfo,hFileInfo.hIcon);

	if (pInfo->bUnknown && PathFileExists(pInfo->strExePath) && pInfo->bFileIsExist)
	{
		struct RUN_RPT_DATA
		{
			DWORD		nType;
			CString		strname;
			CString		strApp;
		};
		static CSimpleArray<RUN_RPT_DATA>	listReportX;

		BOOL	bFind = FALSE;
		for (int i=0; i<listReportX.GetSize(); i++)
		{
			RUN_RPT_DATA&	item = listReportX[i];

			if (item.nType==pInfo->nType&&
				item.strname.CompareNoCase(pInfo->strName)==0 &&
				item.strApp.CompareNoCase(pInfo->strExePath)==0)
			{
				bFind = TRUE;
			}
		}

	}

	return TRUE;
}

void CBeikeSafeSysOptUIHandler::FirstShow()
{
	::PostMessage(m_dlg->m_hWnd,MSG_SYSOPT_RUN_FIRST_IN,NULL,NULL);
}

BOOL CBeikeSafeSysOptUIHandler::OnBkRunTypeSelChange( int nTabItemIDOld, int nTabItemIDNew )
{
	if (!IsOpPermit())
		return FALSE;

	m_RunList.SetShowType(nTabItemIDNew + 1);
	m_dlg->SetItemVisible(IDC_LBL_RUNOPT_BTM_SERVICE_OP,(nTabItemIDNew == KSRUN_TYPE_SERVICE - 1));
	m_dlg->SetItemVisible(IDC_LBL_RUNOPT_TOP_MORE,!(nTabItemIDNew == 3));
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_BOTTOM, !(nTabItemIDNew == 3));
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_LIST_AREA,!(nTabItemIDNew == 3));
	m_dlg->SetItemVisible(IDC_LST_RUNOPT,!(nTabItemIDNew == 3));
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_HISTORY_BTM, nTabItemIDNew == 3);
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_HISTORY,nTabItemIDNew == 3);

	if (nTabItemIDNew == KSRUN_TYPE_STARTUP - 1)
	{
		if (m_bRunScanEnd == FALSE)
			m_RunList.ShowLoadDlg();
		m_dlg->SetItemVisible(IDC_DIV_RUN_TOPTIP , (0 < m_RunList.GetDefShowTypeCount(KSRUN_TYPE_STARTUP)) );
	}
	else if (nTabItemIDNew == KSRUN_TYPE_SERVICE - 1)
	{	
		if (m_bSrvScanEnd == FALSE)
			m_RunList.ShowLoadDlg(m_bSrvScanEnd == FALSE);
		m_dlg->SetItemVisible(IDC_DIV_RUN_TOPTIP , (0 < m_RunList.GetDefShowTypeCount(KSRUN_TYPE_SERVICE)) );
	}
	else if (nTabItemIDNew == KSRUN_TYPE_TASK - 1)
	{
		if (m_bTaskScanEnd == FALSE)
			m_RunList.ShowLoadDlg(m_bTaskScanEnd == FALSE);
		ShowNoTaskInfo();
	}
	else if (nTabItemIDNew == 3)
	{
		GenerateHistoryList();
		m_pHistoryList->SetItemCount(m_vecHistoryItem.size());
		ShowNoHistoryInfo();
		m_dlg->EnableItem(IDC_LNK_RUNOPT_HISTORY_CANCELALL,m_vecHistoryItem.size() > 0);
	}
	BOOL bHideSystemChecked = m_dlg->GetItemCheck(IDC_CHECKBOX_RUNOPT_HIDE_SYSTEM);
	m_RunList.SetShowSystem(!bHideSystemChecked);

	m_RunList.ResetExpandState(FALSE);//重置所有bExpand = FALSE
	GetRunListByCheckState();
	RefreshTabNumber(nTabItemIDNew);
	SetTopRunDiv(nTabItemIDNew + 1);
	return TRUE;
}

BOOL CBeikeSafeSysOptUIHandler::OnClickButton(CKsafeRunInfo* pInfo, DWORD nIndex)
{
	if (!IsOpPermit())
		return FALSE;

	CString strOperation;
	BOOL bNewState = FALSE;
	
	if (nIndex == CLICK_INDEX_ENABLE)
	{
		bNewState = KSRUN_START_ENABLE;
		strOperation = _T("enable");
	}
	if (nIndex == CLICK_INDEX_DISABLE)
	{
		bNewState = KSRUN_START_DISABELE;
		strOperation = _T("disable");
		if (!pInfo->strWarning.IsEmpty())
		{
			CString	strMsg;
			strMsg.Format(BkString::Get(IDS_SYSOPT_4000), pInfo->strWarning);
			if(CBkSafeMsgBox2::ShowMultLine(strMsg, NULL, MB_OKCANCEL|MB_ICONWARNING) != IDOK)
				return FALSE;
		}
	}
	if (nIndex == CLICK_INDEX_DELAY)
	{
		bNewState = KSRUN_START_DELAY;
		strOperation = _T("delay");

		CMsgBoxSysopt MsgDlg(9, m_dlg);
		//如果卫士的启动项不存在,弹出普通对话框提示卫士被禁止开机启动，提示延迟功能失效
		if (MsgDlg.IsTrayAutorun() == FALSE)
		{
			m_bDelayRunValid = FALSE;
			int nExitCode = MsgDlg.DoModal();
			if (nExitCode == IDCANCEL || nExitCode == IDOK)
				return FALSE;
		}
	}
		
	if (pInfo->bEnable)
	{
		SetIniRunOnce();
	}

	if (bNewState == KSRUN_START_ENABLE)
	{
		if ( Is3XXInstalled() && pInfo->IsDisabledBy3XX() && pInfo->nType == KSRUN_TYPE_STARTUP)
		{
			CString	strMsg;
			strMsg.Format(BkString::Get(IDS_SYSOPT_4058), pInfo->strWarning);
			CBkSafeMsgBox2::ShowMultLine(strMsg, NULL, MB_OK|MB_ICONWARNING);
		}
	}
	else if (bNewState == KSRUN_START_DELAY)
	{	
		if (pInfo->strName.CompareNoCase(_T("360SafeTray")) == 0)
		{
			CMsgBoxSysopt dlgX(2);
			dlgX.DoModal();
			return FALSE;
		}
		else if (pInfo->strName.CompareNoCase(_T("360sd")) == 0)
		{
			CMsgBoxSysopt dlgX(7);
			dlgX.DoModal();
			return FALSE;
		}
		else if (pInfo->strName.CompareNoCase(BkString::Get(IDS_SYSOPT_4100)) == 0)
		{
			CMsgBoxSysopt dlgX(8);
			dlgX.DoModal();
			return FALSE;
		}
		else if(pInfo->CannotDelay_SafeSoft())//如果是安全软件
		{
			CMsgBoxSysopt dlgX(4);
			dlgX.DoModal();
			return FALSE;
		}
		else if (pInfo->CannotDelay_Sys())
		{
			CMsgBoxSysopt dlgX(3);
			dlgX.DoModal();
			return FALSE;
		}
	}

	if (m_pRunOptEng)
	{
		if (m_pRunOptEng->ControlRun(bNewState, pInfo))
		{		
			if ( pInfo->nType == KSRUN_TYPE_STARTUP && IsOpTrayRunItem(pInfo->nRegType,pInfo->strName) )
			{
				KwsSetting setting;
				setting.SetNeedAutorunHit(!pInfo->bEnable);
				m_bDelayRunValid = pInfo->bEnable;
			}
		}
		else
		{	
			if (bNewState == KSRUN_START_DISABELE)
			{
				if (pInfo->strName.CompareNoCase(_T("360SafeTray")) == 0)
				{//禁止360时的提示
					CMsgBoxSysopt dlg(5);
					dlg.DoModal();
				}
				else if (pInfo->strName.CompareNoCase(_T(" QQPCTray")) == 0)
				{//禁止QQ电脑管家时的提示
					CMsgBoxSysopt dlg(6);
					dlg.DoModal();
				}
				else
					CBkSafeMsgBox2::ShowMultLine(BkString::Get(IDS_SYSOPT_4001),NULL,MB_OK|MB_ICONWARNING);
			}
			else
				CBkSafeMsgBox2::ShowMultLine(BkString::Get(IDS_SYSOPT_4001),NULL,MB_OK|MB_ICONWARNING);
		}
	}
	int i=m_RunList.GetItemIndexByKSInfo(pInfo);

	if (i>=0)
	{
		CRect rcRow;
		m_RunList.GetItemRect(i,&rcRow);
		m_RunList.InvalidateRect(rcRow);
	}
	RefreshTabNumber();
	RefreshTabNumber_History();
	return TRUE;
}

void CBeikeSafeSysOptUIHandler::GetRunListByCheckState()
{
	BOOL bCheckState = m_dlg->GetItemCheck(IDC_CHECKBOX_SHOW_DISABLE);
	if (bCheckState)
		m_RunList.SetSubShowType(SUB_SHOW_TYPE_ALL);
	else
		m_RunList.SetSubShowType(SUB_SHOW_TYPE_ENABLE);
}

void CBeikeSafeSysOptUIHandler::OnBkShowDisableItem()
{
	GetRunListByCheckState();
	RefreshTabNumber();
}

void CBeikeSafeSysOptUIHandler::OnListReSize( CRect rcWnd )
{
	m_OneKeyOptList.SetColumnWidth(1,rcWnd.Width()-322);
}

void CBeikeSafeSysOptUIHandler::OnBkLnkRefreshList()
{
	SetSubScanning(0, TRUE);
	RefreshItemList(0);
}

void CBeikeSafeSysOptUIHandler::RefreshItemList(DWORD dwFlags)
{
	if (!IsOpPermit())
		return;
	m_RunList.ShowLoadDlg(TRUE);
	SetSubScanning(dwFlags,TRUE);
	m_RunList.ClearAll();
	m_arrayItem.RemoveAll();

	m_nDelayCount = 0;

	if (m_pRunOptEng!=NULL)
	{
		m_dlg->SetItemVisible(IDC_DIV_RUNOPT_BOTTOM,FALSE);
		m_pRunOptEng->EnumRunInfo(dwFlags,this);
		m_nState = STATE_RUNOPT_SCAN;
	}
	m_bRefreshList = TRUE;
}

void CBeikeSafeSysOptUIHandler::ShowNoTaskInfo()
{
	m_dlg->SetItemVisible(IDC_DIV_NOTASK_INFO , 
		(0 == m_RunList.GetDefShowTypeCount(KSRUN_TYPE_TASK)) );
	m_dlg->SetItemVisible(IDC_DIV_RUN_TOPTIP , 
		(0 < m_RunList.GetDefShowTypeCount(KSRUN_TYPE_TASK)) );
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_LIST_AREA , 
		(0 < m_RunList.GetDefShowTypeCount(KSRUN_TYPE_TASK)) );

	m_dlg->SetItemVisible(IDC_LST_RUNOPT_HISTORY, FALSE);
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_NO_HISTORY, FALSE);
}

void CBeikeSafeSysOptUIHandler::OnBkCancelAll()
{
	if (!IsOpPermit())
		return;

	CString	strShow;

	if ( BKSafeConfig::GetHasRunDelete() )
		strShow = BkString::Get(IDS_SYSOPT_4071);
	else
		strShow = BkString::Get(IDS_SYSOPT_4072);

	if ( CBkSafeMsgBox2::ShowMultLine(strShow, NULL, MB_YESNO|MB_ICONWARNING) != IDYES)
		return;
	
	if (m_pRunOptEng)
	{
		m_nState = STATE_RUNOPT_CANCEL;
		m_bRefreshAutoOptList = FALSE;
		m_pRunOptEng->RestoreRun(KSRUN_TYPE_RESTORE_ALL, this);
		m_dlg->SetItemVisible(IDC_DIV_RUNOPT_BOTTOM, FALSE);
	}
}

void CBeikeSafeSysOptUIHandler::OnBkLnkCancelAll()
{
	if (!IsOpPermit())
		return;

	if ( CBkSafeMsgBox2::ShowMultLine(BkString::Get(IDS_SYSOPT_4111), NULL, MB_YESNO|MB_ICONWARNING) != IDYES)
		return;

	if (m_pRunOptEng == NULL)
		return;
			
	m_arrayItem.RemoveAll();
	m_arrayOnekeyOptItem.RemoveAll();
	m_arrayAutooptSysCfgItem.RemoveAll();
	m_OneKeyOptList.ClearAll();
	m_RunList.ClearAll();
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_BOTTOM,FALSE);
	m_dlg->EnableItem(IDC_LNK_AUTOOPT_TOP_REFRESH, FALSE);
	m_dlg->EnableItem(IDC_LNK_SHOW_IGNORED, FALSE);
	m_dlg->EnableItem(IDC_LNK_AUTOOPT_CANCELALL, FALSE);
	m_nState = STATE_RUNOPT_CANCEL;
	m_pRunOptEng->RestoreSysCfg(KSAFE_SYSCONFIG_RESTORE_INIT, this);
	m_pRunOptEng->RestoreRun(KSRUN_TYPE_RESTORE_ALL, this);
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_BOTTOM, FALSE);
	m_bRefreshAutoOptList = TRUE;
}

void CBeikeSafeSysOptUIHandler::EndScan(DWORD dwFlags)
{
	::PostMessage(m_dlg->m_hWnd,MSG_SYSOPT_RUN_SCAN_END,NULL,LPARAM(dwFlags));
}

void CBeikeSafeSysOptUIHandler::RestoreEnd()
{
	::PostMessage(m_dlg->m_hWnd,MSG_SYSOPT_RUN_RESTORE_END,NULL,NULL);
}

LRESULT CBeikeSafeSysOptUIHandler::OnWmScanEnd( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	m_bFirstShowed = TRUE;
	m_nState = STATE_RUNOPT_NULL;

	SetSubScanning(lParam,FALSE);

	if (m_dlg->GetTabCurSel(IDC_TAB_RUNOPT_TYPE) != 3
		&& GetSubScanEnd(0) == TRUE)
		m_dlg->SetItemVisible(IDC_DIV_RUNOPT_BOTTOM,TRUE);

	if (GetSubScanEnd(m_dlg->GetTabCurSel(IDC_TAB_RUNOPT_TYPE) + 1) == TRUE)
		m_RunList.SetShowDlgState();

	if ((m_dlg->GetTabCurSel(IDC_TAB_RUNOPT_TYPE) == 2)
		&& GetSubScanEnd(0) == TRUE)
		ShowNoTaskInfo();
	if (m_dlg->GetTabCurSel(IDC_TAB_RUNOPT_TYPE) == 3)
	{
		OnBkRunTypeSelChange(0, 3);
	}


	if (GetSubScanEnd(0) == TRUE && m_dlg->GetTabCurSel(IDC_TAB_SYSOPT_LEFT) == 1)
	{
		if (m_nDelayCount > 0)//如果存在延迟启动的项
		{
			//如果是被360禁用，则弹框提示被360禁用
			CMsgBoxDisableByX dlgX(m_dlg);
			if (dlgX.IsDisableAutoRunBy360() == TRUE)
			{
				m_bDelayRunValid = FALSE;
				m_bDelayRunTipShowed = TRUE;
				dlgX.DoModal();
			}
			else
			{
				CMsgBoxSysopt MsgDlg(1, m_dlg);
				//如果卫士的启动项不存在,弹出普通对话框提示卫士被禁止开机启动，提示延迟功能失效
				if (MsgDlg.IsTrayAutorun() == FALSE)
				{
					m_bDelayRunValid = FALSE;
					m_bDelayRunTipShowed = TRUE;
					MsgDlg.DoModal();
				}
				else
					m_bDelayRunValid = TRUE;
			}
		}
	}

	if (GetSubScanEnd(0) == TRUE)
	{
		LoadIgnoredID();
		GetIgnoredItem();
		m_OneKeyOptList.DeleteAllItems();
		m_OneKeyOptList.InitArray(m_arrayOnekeyOptItem, m_bDelayRunValid);
		CString strFormat;
		strFormat.Format(BkString::Get(IDS_SYSOPT_4112), m_OneKeyOptList.GetTotalCount());
		m_dlg->SetRichText(IDC_RICHTEXT_ONEKEYOPT_SCANNING, strFormat);
		m_dlg->EnableItem(IDC_LNK_AUTOOPT_TOP_REFRESH, TRUE);
		m_dlg->EnableItem(IDC_LNK_SHOW_IGNORED, TRUE);
		m_dlg->EnableItem(IDC_LNK_AUTOOPT_CANCELALL, TRUE);
		InitAutoOptList();
	}
	else
	{
		LoadIgnoredID();
		GetIgnoredItem();
		m_OneKeyOptList.DeleteAllItems();
		m_OneKeyOptList.InitArray(m_arrayOnekeyOptItem, m_bDelayRunValid);
		CString strFormat;
		strFormat.Format(BkString::Get(IDS_SYSOPT_4112), m_OneKeyOptList.GetTotalCount());
		m_dlg->SetRichText(IDC_RICHTEXT_ONEKEYOPT_SCANNING, strFormat);
	}

	return S_OK;
}

LRESULT CBeikeSafeSysOptUIHandler::OnWmRestoreEnd( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (m_bRefreshAutoOptList)
	{
		OnBkBtnRefresh();
	}
	else
	{
		m_dlg->SetItemVisible(IDC_DIV_RUNOPT_BOTTOM,TRUE);
		m_nState = STATE_RUNOPT_NULL;
		RefreshItemList(0);
	}
	return S_OK;
}

LRESULT CBeikeSafeSysOptUIHandler::OnWmFindItem( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	_FindRunItem((CKsafeRunInfo*)wParam);
	RefreshTabNumber();
 	RefreshTabNumber_History();
	return S_OK;
}

BOOL CBeikeSafeSysOptUIHandler::FindRunItem( CKsafeRunInfo* pInfo )
{
	CKsafeRunInfo*	pNew = new CKsafeRunInfo(*pInfo);
	::PostMessage(m_dlg->m_hWnd,MSG_SYSOPT_RUN_SCAN_FIND,(WPARAM)pNew,NULL);
	return TRUE;
}

BOOL CBeikeSafeSysOptUIHandler::FindRunItem(CKSafeSysoptRegInfo* pInfo)
{
	CKSafeSysoptRegInfo* pNewInfo = new CKSafeSysoptRegInfo(*pInfo);
	m_arraySysCfgItem.Add(pNewInfo);
	return TRUE;
}

void CBeikeSafeSysOptUIHandler::RefreshTabNumber(DWORD nCurTab)
{
	if (nCurTab==-1)
		nCurTab = m_dlg->GetTabCurSel(IDC_TAB_RUNOPT_TYPE);

	nCurTab++;

	CString	strfomat;
	DWORD	nShowCount = m_RunList.GetShowCount();
	BOOL	bShowSystem = m_RunList.IsShowSystem();

	strfomat.Format(BkString::Get(IDS_SYSOPT_4007), 
		m_RunList.GetEnableTypeCount(KSRUN_TYPE_STARTUP)
		);
	m_dlg->SetTabTitle(IDC_TAB_RUNOPT_TYPE,0,strfomat);

	strfomat.Format(BkString::Get(IDS_SYSOPT_4008), 
		m_RunList.GetEnableTypeCount(KSRUN_TYPE_SERVICE, bShowSystem)
		);
	m_dlg->SetTabTitle(IDC_TAB_RUNOPT_TYPE,1,strfomat);

	strfomat.Format(BkString::Get(IDS_SYSOPT_4009),
		m_RunList.GetEnableTypeCount(KSRUN_TYPE_TASK)
		);
	m_dlg->SetTabTitle(IDC_TAB_RUNOPT_TYPE,2,strfomat);
}

BOOL CBeikeSafeSysOptUIHandler::OnBkSysOptSelChange( int nTabItemIDOld, int nTabItemIDNew )
{
	if (0 == nTabItemIDNew)
	{		
		m_dlg->SetItemVisible(IDC_DIV_NOTASK_INFO , FALSE);
		m_bReportOnekeyOpt = TRUE;
		if (m_bRefreshAutoOptList)
		{
			OnBkBtnRefresh();
		}
	}
	else if ( nTabItemIDNew == 1 )
	{
		if (!m_bHasShowDelayTip)
		{
			_ShowDelayTip();
			m_bHasShowDelayTip = TRUE;
		}
		if (m_dlg->GetTabCurSel(IDC_TAB_RUNOPT_TYPE) == 3)
		{
			OnBkRunTypeSelChange(0, 3);
		}
	}
	else if (nTabItemIDNew == 2 )
	{
		CString strPath = _Module.GetAppDirPath();
		strPath += _T("netmon.exe");

		STARTUPINFO si          = {0};
		PROCESS_INFORMATION pi  = {0};
		int bRet = ::CreateProcess(strPath, NULL,
			NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
		if (bRet)
		{	
			if (pi.hThread)
			{
				CloseHandle(pi.hThread);
				pi.hThread = NULL;
			}
			if (pi.hProcess)
			{
				CloseHandle(pi.hProcess);
				pi.hProcess = NULL;
			}
		}

		return FALSE;
	}
	else if (3 == nTabItemIDNew)
	{
		CString strPath = _Module.GetAppDirPath();
		strPath += _T("perfopt.exe");

		STARTUPINFO si          = {0};
		PROCESS_INFORMATION pi  = {0};
		int bRet = ::CreateProcess(strPath, NULL,
			NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
		if (bRet)
		{	
			if (pi.hThread)
			{
				CloseHandle(pi.hThread);
				pi.hThread = NULL;
			}
			if (pi.hProcess)
			{
				CloseHandle(pi.hProcess);
				pi.hProcess = NULL;
			}
		}

		return FALSE;
	}
	else if (4 == nTabItemIDNew)
	{
		_OpenKSafePage();
	}
	
	m_dlg->SetItemVisible(IDC_DIV_SYSOPT_RUN, nTabItemIDNew == 1);
	m_dlg->SetItemVisible(IDC_DIV_SYSOPT_SYSTOOLS, nTabItemIDNew == 4);
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT, (nTabItemIDNew == 0));
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP, (nTabItemIDNew == 0));
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP2, (nTabItemIDNew == 0));
	m_dlg->SetItemVisible(IDC_DIV_RUN_TOPTIP, (nTabItemIDNew != 0));
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_BTM , nTabItemIDNew == 0);


	return TRUE;
}

void CBeikeSafeSysOptUIHandler::OnBkBtnWaitMore()
{
	::ShellExecute(NULL,_T("open"),_T("http://www.ijinshan.com/safe/optimize_advice.html?fr=client"),NULL,NULL,SW_SHOW);
}

void CBeikeSafeSysOptUIHandler::OnBkRunOptTopMore()
{
	::ShellExecute(NULL,_T("open"),_T("http://www.ijinshan.com/safe/optimize_intro.html?fr=client"),NULL,NULL,SW_SHOW);
}

void CBeikeSafeSysOptUIHandler::SetTopRunDiv( DWORD nType )
{
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_TOP_RUN,nType==1);
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_TOP_SERVICE,nType==2);
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_TOP_TASK,nType==3);
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_TOP_HISTORY,nType==4);
}

LRESULT CBeikeSafeSysOptUIHandler::OnWmFirstIn( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	InitEng();
	if (m_pRunOptEng==NULL)
	{
		CBkSafeMsgBox2::ShowMultLine( BkString::Get(IDS_SYSOPT_4010), NULL, MB_OK|MB_ICONWARNING);
		return S_OK;
	}
	m_nState = STATE_RUNOPT_SCAN;
	m_nDelayCount = 0;
	m_dlg->SetItemCheck(IDC_CHECKBOX_SHOW_DISABLE, TRUE);
	m_dlg->SetItemCheck(IDC_CHECKBOX_RUNOPT_HIDE_SYSTEM, TRUE);
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_BOTTOM, FALSE);
	m_dlg->EnableItem(IDC_LNK_AUTOOPT_TOP_REFRESH, FALSE);
	m_dlg->EnableItem(IDC_LNK_SHOW_IGNORED, FALSE);
	m_dlg->EnableItem(IDC_LNK_AUTOOPT_CANCELALL, FALSE);
	m_dlg->EnableItem(IDC_BTN_RUN_AUTOOPT_DO, FALSE);
	m_dlg->EnableItem(IDC_CHECKBOX_AUTOOPT_ALL, FALSE);
	m_RunList.ShowLoadDlg(TRUE);
	m_dlg->SetTimer(TIMER_ID_ONEKEYOPT_SCAN, 50, NULL);
	SetSubScanning(0,TRUE);
	int nTabSel = m_dlg->GetTabCurSel(IDC_TAB_SYSOPT_LEFT);
	if (nTabSel == 0)
		OnBkSysOptSelChange(1, 0);
	if (nTabSel == 1)
		OnBkSysOptSelChange(0, 1);

	m_pRunOptEng->EnumRunInfo(0,this);

	//如果是在开机加速Tab，才显示延迟启动介绍框
	if ((m_dlg->GetTabCurSel(IDC_TAB_SYSOPT_LEFT) == 1) && !m_bHasShowDelayTip)
	{
		_ShowDelayTip();
		m_bHasShowDelayTip = TRUE;
	}

	return S_OK;
}

void CBeikeSafeSysOptUIHandler::OnBkLnkShowSetting()
{
	_Module.Navigate(BKSFNS_MAKE_3(BKSFNS_UI, BKSFNS_SETTING, BKSFNS_SETTING_PAGE_SYSOPT));
}

void CBeikeSafeSysOptUIHandler::SecondShow()
{
	if ( CTrayRunCommon::GetPtr()->GetChange() )
	{
		//RefreshItemList();
	}
}

void CBeikeSafeSysOptUIHandler::PostRestoreRunItem( DWORD nType,BOOL bEnable,LPCTSTR lpName )
{
	if (IsOpTrayRunItem(nType,lpName))
	{
		KwsSetting setting;
		setting.SetNeedAutorunHit(!bEnable);
	}
}

BOOL CBeikeSafeSysOptUIHandler::OnDeleteItem( CKsafeRunInfo* pInfo )
{
	CString			strMsg;
	BOOL			bNoNotify = BKSafeConfig::GetNoTipRunDelete();

	strMsg.Format(BkString::Get(IDS_SYSOPT_4011), pInfo->strDisplay);

	if ( !bNoNotify )
	{
		if ( CBkSafeMsgBox2::ShowMultLine(strMsg,NULL,MB_YESNO|MB_ICONWARNING,&bNoNotify) != IDYES )
			return FALSE;
		BKSafeConfig::SetNoTipRunDelete(bNoNotify);
	}

	BOOL bRes = m_pRunOptEng->DeleteRun(pInfo);


	if (bRes)
	{
		if ( pInfo->nType == KSRUN_TYPE_STARTUP && IsOpTrayRunItem(pInfo->nRegType,pInfo->strName) )
		{
			KwsSetting setting;
			setting.SetNeedAutorunHit(!pInfo->bEnable);
		}

		if (m_RunList.XBASE_GetItemCount() == 0)
		{
			m_RunList.SetShowDlgState();
			RefreshItemList();
		}
		// 		//将删除的项目从m_arrayItem中移除
		for (int i = 0;i < m_arrayItem.GetSize();i++)
		{
			if ((pInfo->strName == m_arrayItem[i]->strName) &&
				(pInfo->strExePath == m_arrayItem[i]->strExePath) &&
				(pInfo->nID == m_arrayItem[i]->nID))
			{
				m_arrayItem.RemoveAt(i);
				break;
			}
		}
		m_RunList.DeleteItemByKSInfo(pInfo);
		m_RunList.InvalidateRect(NULL);
		RefreshTabNumber();
		m_bRefreshAutoOptList = TRUE;
		BKSafeConfig::SetHasRunDelete(TRUE);
	}
	else
	{
		CBkSafeMsgBox2::ShowMultLine(BkString::Get(IDS_SYSOPT_4012),NULL,MB_OK|MB_ICONWARNING);
	}

	return bRes;
}

BOOL CBeikeSafeSysOptUIHandler::OnClickLabel( DWORD nIndex, CKsafeRunInfo* pInfo )
{
	if (nIndex == CLICK_INDEX_ENABLE || nIndex == CLICK_INDEX_DISABLE || nIndex == CLICK_INDEX_DELAY)
	{
		return OnClickButton(pInfo, nIndex);
	}
	else if (nIndex==CLICK_INDEX_DELETE)
		return OnDeleteItem(pInfo);
	else if (nIndex==CLICK_INDEX_OPEN_FOLDER)
	{

		CString	strFilePath(pInfo->strExePath);
		if (::PathFileExists(strFilePath))
		{
			CString	strParam = _T("/select,\"") + strFilePath + _T("\"");

			KWow64Switcher switcher;
			switcher.Init();
			BOOL bIsx64 = switcher.IsWin64();
			if (bIsx64)
				switcher.Close();

			::ShellExecute( m_RunList.m_hWnd, _T("open"), _T("explorer.exe"), 
				strParam, NULL, SW_SHOW);

			if (bIsx64)
			{
				switcher.Revert();
			}
		}
		else
		{
			CBkSafeMsgBox2::ShowMultLine(BkString::Get(IDS_SYSOPT_4063),NULL,MB_OK|MB_ICONWARNING); 
		}
	}
	return FALSE;
}

BOOL CBeikeSafeSysOptUIHandler::OnClickAddIgnore(DWORD nIndex, DWORD dwID, int nItem)
{
	if (nIndex == CLICK_INDEX_IGNORE)
		return OnBkLnkAddToIgnored(dwID, nItem) ;
	return FALSE;
}

CBkNavigator* CBeikeSafeSysOptUIHandler::OnNavigate( CString &strChildName )
{
	if (0 == strChildName.CompareNoCase(BKSFNS_SYSOPT_RUN_OPT))
	{
		m_dlg->SetTabCurSel(IDC_TAB_SYSOPT_LEFT,1);
		OnBkSysOptSelChange(0, 1);
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_SYSOPT_ONEKEY_RUN_OPT))
	{
		m_dlg->SetTabCurSel(IDC_TAB_SYSOPT_LEFT, 0);
		OnBkSysOptSelChange(1, 0);
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_SYSOPT_SETTING))
	{
		_Module.Navigate(BKSFNS_MAKE_3(BKSFNS_UI, BKSFNS_SETTING, BKSFNS_SETTING_PAGE_SYSOPT));
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_SYSOPT_NET_MONITOR))
	{
		m_dlg->SetTabCurSel(IDC_TAB_SYSOPT_LEFT, 2);
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_SYSOPT_PERFOPT))
	{
		m_dlg->SetTabCurSel(IDC_TAB_SYSOPT_LEFT, 3);
	}
	else if(0 == strChildName.CompareNoCase(BKSFNS_SYSOPT_SYSTOOLS))
	{
		m_dlg->SetTabCurSel(IDC_TAB_SYSOPT_LEFT, 4);
	}

	return NULL;
}

void CBeikeSafeSysOptUIHandler::ClickCheck()
{
	m_dlg->EnableItem(IDC_BTN_RUN_AUTOOPT_DO, m_OneKeyOptList.GetCheckedCount() > 0);
	m_dlg->SetItemCheck(IDC_CHECKBOX_AUTOOPT_ALL,m_OneKeyOptList.IsCheckAll());

}

void CBeikeSafeSysOptUIHandler::InitAutoOptList()
{	
	if (m_pRunOptEng)
	{
		m_arraySysCfgItem.RemoveAll();
		m_pRunOptEng->EnumSysConfigInfo(0, this);
		LoadIgnoredID();
		GetIgnoredItem();
		BOOL bRet2 = m_OneKeyOptList.InitSyscfgArray(m_arrayAutooptSysCfgItem);
		CString strFormat;
		strFormat.Format(BkString::Get(IDS_SYSOPT_4112), m_OneKeyOptList.GetTotalCount());
		m_dlg->SetRichText(IDC_RICHTEXT_ONEKEYOPT_SCANNING, strFormat);
		if (m_OneKeyOptList.GetItemCount() == 0)
		{
			m_dlg->SetItemVisible(IDC_LST_RUN_AUTOOPT, FALSE);
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_NO_OPTITEM, TRUE);
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_INIT, FALSE);
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DOING, FALSE);
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DONE_PART, FALSE);
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DONE, FALSE);
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_SCANNING, FALSE);
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_NOITEM, TRUE);
			m_dlg->SetItemVisible(IDC_CHECKBOX_AUTOOPT_ALL, FALSE);
			m_dlg->SetItemVisible(IDC_LST_RUN_AUTOOPT, FALSE);
			m_dlg->EnableItem(IDC_CHECKBOX_AUTOOPT_ALL, FALSE);
			m_dlg->SetItemVisible(IDC_LNK_AUTOOPT_TOP_REFRESH, TRUE);
		}
		else
		{
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_INIT, TRUE);
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DOING, FALSE);
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DONE_PART, FALSE);
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DONE, FALSE);
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_SCANNING, FALSE);
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_NOITEM, FALSE);
			m_dlg->SetItemVisible(IDC_CHECKBOX_AUTOOPT_ALL, TRUE);
			m_dlg->EnableItem(IDC_CHECKBOX_AUTOOPT_ALL, TRUE);
			m_dlg->SetItemVisible(IDC_BTN_RUN_AUTOOPT_DO, TRUE);
			m_dlg->SetItemVisible(IDC_LNK_AUTOOPT_TOP_REFRESH, TRUE);
			m_dlg->SetItemVisible(IDC_LST_RUN_AUTOOPT, TRUE);
//			m_dlg->SetItemStringAttribute(IDC_BTN_RUN_AUTOOPT_DO, "skin", L"vul_repair");
			m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_NO_OPTITEM, FALSE);
			m_dlg->EnableItem(IDC_CHECKBOX_AUTOOPT_ALL, TRUE);
		}
		RefreshIgnoredText();
	}
	m_bRefreshList = FALSE;
}

void CBeikeSafeSysOptUIHandler::OnTimer(UINT_PTR nIDEvent)
{
	//一键优化显示正在优化详细信息
	if (TIMER_ID_RUNOPT_AUTOOPT_TOPTIP == nIDEvent)
		_ShowOneKeyOptDoing();

	//延迟加载工具集页面
	if (TIMER_ID_DELAYLOAD_KSAFE_TOOL == nIDEvent)
		_OpenKSafePage();
	
	//一键优化显示正在扫描的动态信息
	if(TIMER_ID_ONEKEYOPT_SCAN == nIDEvent)
		_ShowOneKeyOptScanning();
}

void CBeikeSafeSysOptUIHandler::_ShowOneKeyOptDoing()
{
	static int nPos = 0;
	++nPos;
	m_dlg->SetItemIntAttribute(IDC_IMG_AUTOOPT_TOP_DOING, "sub", (nPos)%10);
	m_dlg->SetItemIntAttribute(IDC_IMG_ONEKEYOPT_PROCESS_ANIMATION, "sub", (nPos)%24);
}

void CBeikeSafeSysOptUIHandler::_ShowOneKeyOptScanning()
{
	static int nPos = 0;
	++nPos;
	m_dlg->SetItemIntAttribute(IDC_IMG_ONEKEYOPT_SCAN_ANIMATION, "sub", (nPos)%10);
	m_dlg->SetItemIntAttribute(IDC_IMG_ONEKEYOPT_PROCESS_ANIMATION2, "sub", (nPos)%24);
}

void CBeikeSafeSysOptUIHandler::OnBkBtnRefresh()
{
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DONE_PART, FALSE);
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DOING, FALSE);
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_INIT, FALSE);
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DONE, FALSE);
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_SCANNING, TRUE);
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_NOITEM, FALSE);
	m_dlg->SetTimer(TIMER_ID_ONEKEYOPT_SCAN, 50, NULL);

	m_nDelayCount = 0;
	SetSubScanning(0,TRUE);
	CString strFormat;
	strFormat.Format(BkString::Get(IDS_SYSOPT_4112), 0);
	m_dlg->SetRichText(IDC_RICHTEXT_ONEKEYOPT_SCANNING, strFormat);
	m_arrayItem.RemoveAll();
	m_arrayOnekeyOptItem.RemoveAll();
	m_arrayAutooptSysCfgItem.RemoveAll();
	m_OneKeyOptList.ClearAll();
	m_RunList.ClearAll();
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_BOTTOM, FALSE);
	m_dlg->SetItemVisible(IDC_BTN_RUN_AUTOOPT_DO, FALSE);
	m_dlg->SetItemVisible(IDC_LNK_AUTOOPT_TOP_REFRESH, FALSE);
	m_dlg->EnableItem(IDC_LNK_AUTOOPT_TOP_REFRESH, FALSE);
	m_dlg->EnableItem(IDC_LNK_SHOW_IGNORED, FALSE);
	m_dlg->EnableItem(IDC_LNK_AUTOOPT_CANCELALL, FALSE);
	m_dlg->EnableItem(IDC_BTN_RUN_AUTOOPT_DO, FALSE);
	m_dlg->EnableItem(IDC_CHECKBOX_AUTOOPT_ALL, FALSE);
	
	CMsgBoxSysopt MsgDlg(1, m_dlg);
	m_bDelayRunValid = MsgDlg.IsTrayAutorun();

	m_pRunOptEng->EnumRunInfo(0, this);
	m_nState = STATE_RUNOPT_SCAN;
	m_bRefreshAutoOptList = FALSE;
}

void CBeikeSafeSysOptUIHandler::OnBkBtnOnkeyOpt()
{
	if (m_pRunOptEng==NULL)
		return;
	// 释放之前的一键优化线程
	if ( m_hOptimizeThread != NULL )
	{
		::CloseHandle(m_hOptimizeThread);
		m_hOptimizeThread = NULL;
	}
	if (0 == m_OneKeyOptList.GetCheckedCount())
	{
		CBkSafeMsgBox2::ShowMultLine(BkString::Get(IDS_SYSOPT_4056), NULL, MB_OK|MB_ICONWARNING);
		return;
	}

	SetIniRunOnce();
	m_bOpting = TRUE;
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_INIT,FALSE);
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DONE,FALSE);
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_SCANNING, FALSE);
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DONE_PART,FALSE);
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DOING,TRUE);
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_NOITEM, FALSE);
	m_dlg->EnableItem(IDC_BTN_RUN_AUTOOPT_DO,FALSE);
	m_dlg->EnableItem(IDC_CHECKBOX_AUTOOPT_ALL, FALSE);

	m_dlg->EnableItem(IDC_LNK_AUTOOPT_TOP_REFRESH, FALSE);
	m_dlg->SetItemVisible(IDC_LNK_AUTOOPT_TOP_REFRESH, FALSE);
	m_dlg->EnableItem(IDC_LNK_SHOW_IGNORED, FALSE);
	m_dlg->EnableItem(IDC_LNK_AUTOOPT_CANCELALL, FALSE);
	m_hOptimizeThread = ::CreateThread(NULL, 0, &CBeikeSafeSysOptUIHandler::OneKeyOptProc, this, 0, NULL);
}

DWORD WINAPI CBeikeSafeSysOptUIHandler::OneKeyOptProc(LPVOID lpParam)
{
	CBeikeSafeSysOptUIHandler* pThis = (CBeikeSafeSysOptUIHandler*)lpParam;
	BOOL	bFailed = FALSE;
	CString strTopTip;
	int		nItemCount = pThis->m_OneKeyOptList.GetItemCount();
	pThis->m_dlg->SetTimer(TIMER_ID_RUNOPT_AUTOOPT_TOPTIP,50,NULL);
	pThis->m_OneKeyOptList.ResetStatus(ARUN_STATUS_DOING);
	for ( int i=0; i < nItemCount; i++)
	{
		_ARUN_ITEM_DATA*	px = pThis->m_OneKeyOptList.GetItemParamData(i);

		if (px && px->nStatus==ARUN_STATUS_DOING && px->bCheck )
		{
			if (px->nType == ARUN_TYPE_ITEM )
			{
				strTopTip.Format(BkString::Get(IDS_SYSOPT_4054),px->pInfo->strDisplay);
				pThis->m_dlg->SetItemText(IDC_TXT_AUTOOPT_TOP_DETAIL,strTopTip);
				CRect rcText;
				pThis->m_dlg->GetItemRect(IDC_TXT_AUTOOPT_TOP_DETAIL,rcText);
				pThis->m_dlg->InvalidateRect(rcText);

				BOOL bEnable = (px->bDelay == TRUE)?KSRUN_START_DELAY:KSRUN_START_DISABELE;

				if (pThis->m_pRunOptEng->ControlRun(bEnable, px->pInfo))
				{
					if ( px->pInfo->nType == KSRUN_TYPE_STARTUP && IsOpTrayRunItem(px->pInfo->nRegType,px->pInfo->strName) )
					{
						KwsSetting setting;
						setting.SetNeedAutorunHit(!px->pInfo->bEnable);
					}
					px->nStatus = ARUN_STATUS_SUCCESS;
					px->bCheck = FALSE;
					Sleep(450);
				}
				else
				{
					px->nStatus = ARUN_STATUS_FAILED;
					bFailed = TRUE;
				}
			}
			else if (px->nType == ARUN_TYPE_ITEM_SYSCFG)
			{
				strTopTip.Format(BkString::Get(IDS_SYSOPT_4110),px->pSysCfgInfo->m_strDesc);
				pThis->m_dlg->SetItemText(IDC_TXT_AUTOOPT_TOP_DETAIL,strTopTip);
				CRect rcText;
				pThis->m_dlg->GetItemRect(IDC_TXT_AUTOOPT_TOP_DETAIL,rcText);
				pThis->m_dlg->InvalidateRect(rcText);


				if (pThis->m_pRunOptEng->ControlSysConfig(KSAFE_SYSCONFIG_OPTIMIZE, px->pSysCfgInfo))
				{
					px->nStatus = ARUN_STATUS_SUCCESS;
					px->bCheck = FALSE;
					Sleep(450);
				}
				else
				{
					px->nStatus = ARUN_STATUS_FAILED;
					bFailed = TRUE;
				}
			}
		}
		CRect	rcItem;
		pThis->m_OneKeyOptList.GetItemRect(i,&rcItem,LVIR_BOUNDS);
		pThis->m_OneKeyOptList.InvalidateRect(rcItem);
	}
	
	::PostMessage(pThis->m_dlg->m_hWnd,MSG_SYSOPT_ONEKEYOPT_END,NULL,NULL);
	return 0;
}

LRESULT CBeikeSafeSysOptUIHandler::OnOnekeyOptEnd(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int		nSuccessCount = 0;
	int		nFailedCount  = 0;
	int		nUnOptedCount = 0;
	int		nRunItemCount = m_OneKeyOptList.GetTotalCount();
	CString strTopTip;

	m_dlg->EnableItem(IDC_CHECKBOX_AUTOOPT_ALL, TRUE);
	nSuccessCount = m_OneKeyOptList.GetSuccessNumber();
	nFailedCount  = m_OneKeyOptList.GetFailedNumber();
	if (nSuccessCount == nRunItemCount)
	{
		m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DONE_PART, FALSE);
		m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DOING, FALSE);
		m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_INIT, FALSE);
		m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_SCANNING, FALSE);
		m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DONE, TRUE);
		m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_NOITEM, FALSE);
		m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_NOITEM, FALSE);
		strTopTip.Format(BkString::Get(IDS_SYSOPT_4075), nRunItemCount);
		m_dlg->SetRichText(IDC_TXT_AUTOOPT_TOP_OPT_ALL, strTopTip);
//		m_dlg->SetItemStringAttribute(IDC_BTN_RUN_AUTOOPT_DO, "skin", L"vul_repair");
		m_dlg->EnableItem(IDC_CHECKBOX_AUTOOPT_ALL, FALSE);
		m_bOpting = FALSE;
	}
	else if (nSuccessCount < nRunItemCount)
	{
		if (nFailedCount + nSuccessCount == nRunItemCount)
		{
			m_dlg->EnableItem(IDC_CHECKBOX_AUTOOPT_ALL, FALSE);
			m_bOpting = FALSE;
		}
		m_dlg->EnableItem(IDC_BTN_RUN_AUTOOPT_DO, TRUE);
//		m_dlg->SetItemStringAttribute(IDC_BTN_RUN_AUTOOPT_DO, "skin", L"vul_repair");
		m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DONE_PART, TRUE);
		m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DOING, FALSE);
		m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_INIT, FALSE);
		m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DONE, FALSE);
		m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_SCANNING, FALSE);
		strTopTip.Format(BkString::Get(IDS_SYSOPT_4076), nSuccessCount,nRunItemCount - nSuccessCount);
		m_dlg->SetRichText(IDC_TXT_AUTOOPT_TOP_OPT_PART, strTopTip);
	}

	m_dlg->KillTimer(TIMER_ID_RUNOPT_AUTOOPT_TOPTIP);
	m_dlg->SetItemVisible(IDC_DIV_AUTOOPT_TOP_DOING, FALSE);
	m_dlg->SetItemVisible(IDC_LNK_AUTOOPT_TOP_REFRESH, TRUE);
	m_dlg->EnableItem(IDC_LNK_AUTOOPT_TOP_REFRESH, TRUE);
	m_dlg->EnableItem(IDC_LNK_SHOW_IGNORED, TRUE);
	m_dlg->EnableItem(IDC_LNK_AUTOOPT_CANCELALL, TRUE);
	//做过一键优化之后，看看是不是没有全部优化，如果全部优化掉，就告诉体检我有话完了，更新状态
	if (m_OneKeyOptList.GetAdviceStopCount() == 0)
		m_dlg->RemoveFromTodoList(BkSafeExamItem::EXAM_RUNOPT_ONKEY);

	return 0;
}

LRESULT CBeikeSafeSysOptUIHandler::OnRefreshOnekeyOpt(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr = S_OK;
	RefreshIgnoredText();

	if (wParam == TRUE)
	{
		m_dlg->SetTabCurSel(IDC_TAB_RUNOPT_TYPE, 0);
		OnBkBtnRefresh();
	}

	if (m_OneKeyOptList.GetItemCount() == 0)
	{
		m_dlg->RemoveFromTodoList(BkSafeExamItem::EXAM_RUNOPT_ONKEY);
	}

	return hr;
}

void CBeikeSafeSysOptUIHandler::OnAutoOptCheckAll()
{
	BOOL bAutoOptAllChecked = m_dlg->GetItemCheck(IDC_CHECKBOX_AUTOOPT_ALL);
	m_OneKeyOptList.CheckAll(bAutoOptAllChecked);
	ClickCheck();
}



void CBeikeSafeSysOptUIHandler::OnBkLnkViewRun()
{	
	m_dlg->SetTabCurSel(IDC_TAB_SYSOPT_LEFT, 1);
}

void CBeikeSafeSysOptUIHandler::OnBkLnkViewOnekeyOpt()
{
	m_dlg->SetTabCurSel(IDC_TAB_SYSOPT_LEFT, 0);
}

void CBeikeSafeSysOptUIHandler::OnBkLnkCleanTrash()
{
	SAFE_CALL_FUN(m_dlg, Navigate(BKSFNS_MAKE(BKSFNS_PAGE_SYSTEM_CLEAR, BKSFNS_SYSOPT_CLR_RUBBISH)));
}

void CBeikeSafeSysOptUIHandler::OnBkLnkSoftMgr_ZJBJ()
{
	SAFE_CALL_FUN(m_dlg, Navigate(BKSFNS_MAKE(BKSFNS_SOFTMGR,BKSFNS_SOFTMGR_PAGE_ZJBB)));
}

void CBeikeSafeSysOptUIHandler::OnBkCheckHideSystem()
{
	BOOL bHideSystemChecked = m_dlg->GetItemCheck(IDC_CHECKBOX_RUNOPT_HIDE_SYSTEM);
	m_RunList.SetShowSystem(!bHideSystemChecked);
	RefreshTabNumber();
}

BOOL CBeikeSafeSysOptUIHandler::Is3XXInstalled()
{
	BOOL bResult = FALSE;
	TCHAR szFilePath[MAX_PATH + 1];
	DWORD cbData;
	DWORD dwType;
	LRESULT lResult;
	DWORD dwAttr;
	cbData = MAX_PATH * sizeof (TCHAR);
	lResult = SHGetValue(HKEY_LOCAL_MACHINE, 
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\360safe.exe"),
		_T("Path"), 
		&dwType, 
		szFilePath, 
		&cbData
		);
	if ( lResult == ERROR_SUCCESS && dwType == REG_SZ )
	{
		szFilePath[MAX_PATH] = 0;
		_tcsncat(szFilePath, _T("\\360safe.exe"), MAX_PATH);
		dwAttr = GetFileAttributes(szFilePath);
		if ( dwAttr == INVALID_FILE_ATTRIBUTES )
		{
			bResult = FALSE;
			goto Exit0;
		}
		else
		{
			bResult = dwAttr & FILE_ATTRIBUTE_DIRECTORY ? FALSE:TRUE;
			goto Exit0;
		}
	}
Exit0:
	return bResult;
}

void CBeikeSafeSysOptUIHandler::SetIniRunOnce()
{
	int nHasOpted = 0;
	CIniFile iniFile(m_strIniFilePath);
	iniFile.GetIntValue(SEC_RUNOPT,KEY_RUNONCE,nHasOpted);
	if (0 == nHasOpted)
	{
		iniFile.SetIntValue(SEC_RUNOPT,KEY_RUNONCE,1);
	}
}

void CBeikeSafeSysOptUIHandler::OnBkLnkShowIgnoredDlg()
{
	LoadIgnoredID();
	GetIgnoredItem();
	CBeikeSafeSysoptIgnoreDlg dlg(m_arrayIgnoredItem, m_arrayIgnoredSysCfgItem, m_dlg);
	dlg.DoModal();
}

void CBeikeSafeSysOptUIHandler::RefreshIgnoredText()
{
	LoadIgnoredID();
	GetIgnoredItem();
	CString strIgnored;
	strIgnored.Format(BkString::Get(IDS_SYSOPT_4073),m_nIgnoredCount);
	m_dlg->SetItemText(IDC_LNK_SHOW_IGNORED,strIgnored);

	int nRunItemCount;
	CString strTopTip;
	nRunItemCount = m_OneKeyOptList.GetTotalCount();
	strTopTip.Format(BkString::Get(IDS_SYSOPT_4074),nRunItemCount);
	m_dlg->SetRichText(IDC_TXT_AUTOOPT_TOP_OPT_INIT,strTopTip);
}

BOOL CBeikeSafeSysOptUIHandler::OnBkLnkAddToIgnored(DWORD dwID, int iItem)
{
	SaveIgnoreID(dwID);
	m_OneKeyOptList.DeleteItem(iItem);

	if (m_OneKeyOptList.GetRunCount() == 0)
		m_OneKeyOptList.DeleteTitle(ARUN_TYPE_TITLE_RUN);
	if (m_OneKeyOptList.GetServiceCount() == 0)
		m_OneKeyOptList.DeleteTitle(ARUN_TYPE_TITLE_SERVE);
	if (m_OneKeyOptList.GetTaskCount() == 0)
		m_OneKeyOptList.DeleteTitle(ARUN_TYPE_TITLE_TASK);
	if (m_OneKeyOptList.GetDelayCount() == 0)
		m_OneKeyOptList.DeleteTitle(ARUN_TYPE_TITLE_RUN_DELAY);
	for (int i=0; i<6; i++)
	{
		if (m_OneKeyOptList.GetSysCfgCount(i) == 0)
		{
			m_OneKeyOptList.DeleteTitle(m_OneKeyOptList.GetSysCfgTitleType(i));
		}
	}

	RefreshIgnoredText();

	return ::PostMessage(m_dlg->m_hWnd, MSG_SYSOPT_REFRESH_ONEKEYOPT, WPARAM(FALSE), 0);
}

void CBeikeSafeSysOptUIHandler::SaveIgnoreID(int nNewIgnoreID)
{
	CIniFile ini_IgnoredList(m_strIgnoredIniPath);
	CString strGetValue;
	ini_IgnoredList.GetStrValue(SEC_IGNOREDLIST_MAIN, KEY_IGNOREDLIST_COMMENT, strGetValue.GetBuffer(65536), 65536);
	strGetValue.ReleaseBuffer(65536);

	BOOL bExisted = FALSE;
	for (int n = 0;n < m_arrayIgnoredID.GetSize();n++)
	{
		if (nNewIgnoreID == m_arrayIgnoredID[n])
			bExisted = TRUE;
	}
	if (!bExisted)
	{
		strGetValue.Format(_T("%s%d|"), CString(strGetValue), nNewIgnoreID);
		ini_IgnoredList.SetStrValue(SEC_IGNOREDLIST_MAIN,KEY_IGNOREDLIST_COMMENT, strGetValue);
	}
}

void CBeikeSafeSysOptUIHandler::LoadIgnoredID()
{
	CIniFile ini_IgnoredList(m_strIgnoredIniPath);
	CString strGetValue;
	ini_IgnoredList.GetStrValue(SEC_IGNOREDLIST_MAIN, KEY_IGNOREDLIST_COMMENT, strGetValue.GetBuffer(65536), 65536);
	strGetValue.ReleaseBuffer(65536);

	WCHAR *szValue;
	WCHAR szTemp[50];
	m_arrayIgnoredID.RemoveAll();
	szValue = strGetValue.GetBuffer();
	strGetValue.ReleaseBuffer();
	WCHAR* pToken = wcstok(szValue, L"|");
	while(pToken)
	{
		wcscpy_s(szTemp, pToken);
		m_arrayIgnoredID.Add(_wtoi(szTemp));
		pToken = wcstok(NULL,L"|");
	}
}

void CBeikeSafeSysOptUIHandler::GetIgnoredItem()
{
	m_arrayIgnoredItem.RemoveAll();
	m_arrayOnekeyOptItem.RemoveAll();
	m_arrayIgnoredSysCfgItem.RemoveAll();
	m_arrayAutooptSysCfgItem.RemoveAll();
	m_nIgnoredCount = 0;
	for (int i = 0; i < m_arrayItem.GetSize();i++)
	{
		BOOL bFind = FALSE;
		for (int n = 0; n < m_arrayIgnoredID.GetSize();n++)
		{
			if (m_arrayItem[i]->nID == m_arrayIgnoredID[n]
			&& (m_arrayIgnoredID[n] != 0))
			{
				bFind = TRUE;
				break;
			}
		}
		if (bFind == TRUE)
		{
			m_nIgnoredCount++;
			CKsafeRunInfo*	pNewInfo = m_arrayItem[i];
			m_arrayIgnoredItem.Add(pNewInfo);
		}
		else
		{
			CKsafeRunInfo*	pNewInfo = m_arrayItem[i];
			m_arrayOnekeyOptItem.Add(pNewInfo);
		}
	}
	
	CSimpleArray<DWORD> arrayFindID;
	for (int i = 0; i < m_arraySysCfgItem.GetSize();i++)
	{
		BOOL bFind = FALSE;
		for (int n = 0; n < m_arrayIgnoredID.GetSize();n++)
		{
			if (m_arraySysCfgItem[i]->m_nID == m_arrayIgnoredID[n])
			{
				bFind = TRUE;
				break;
			}
		}
		if (bFind == TRUE)
		{
			BOOL bHasIgnored = FALSE;
			for (int nCount = 0; nCount < arrayFindID.GetSize(); nCount++)
			{
				if (m_arraySysCfgItem[i]->m_nID == arrayFindID[nCount])
				{
					bHasIgnored = TRUE;
					break;
				}
			}
			if (!bHasIgnored)
			{
				m_nIgnoredCount++;
				CKSafeSysoptRegInfo* pNewInfo = m_arraySysCfgItem[i];
				m_arrayIgnoredSysCfgItem.Add(pNewInfo);
				arrayFindID.Add(m_arraySysCfgItem[i]->m_nID);
			}
		}
		else
		{
			CKSafeSysoptRegInfo* pNewInfo = m_arraySysCfgItem[i];
			m_arrayAutooptSysCfgItem.Add(pNewInfo);
		}
	}
}

LRESULT CBeikeSafeSysOptUIHandler::OnListBoxGetMaxHeight(LPNMHDR pnmh)
{
	BKLBITEMCALCMAXITEM *pdi = (BKLBITEMCALCMAXITEM*)pnmh;
	pdi->nMaxHeight = HISTORY_LIST_ITEM_HEIGHT;
	return 0;
}

LRESULT CBeikeSafeSysOptUIHandler::OnListBoxGetItemHeight(LPNMHDR pnmh)
{
	BKLBITEMCACLHEIGHT *pdi = (BKLBITEMCACLHEIGHT*)pnmh;
	if (pdi->nListItemId >= m_vecHistoryItem.size())
		return 0;	

	RUNOPT_HISTORY_DATA* historyData = m_vecHistoryItem[pdi->nListItemId];

	if (historyData->bTitle)
		pdi->nHeight = HISTORY_LIST_TITLE_HEIGHT;
	else
		pdi->nHeight = HISTORY_LIST_ITEM_HEIGHT;

	return 0;
}

LRESULT CBeikeSafeSysOptUIHandler::OnListBoxClickCtrl(LPNMHDR pnmh)
{
	LPBKLBMITEMCLICK pnms = (LPBKLBMITEMCLICK)pnmh;
	if (pnms->nListItemID >= m_vecHistoryItem.size())
		return 0;

	RUNOPT_HISTORY_DATA* history_OptItem = m_vecHistoryItem[pnms->nListItemID];
	if (history_OptItem->pInfo != NULL)
	{
		switch (pnms->uCmdID)
		{
		case IDC_LNK_RUNITEM_CONTROL:
			{
				BOOL bEnable = history_OptItem->pInfo->bEnable;
				if ((bEnable == KSRUN_START_DISABELE || bEnable == KSRUN_START_DELAY) 
					&& m_pRunOptEng)
				{
					if (m_pRunOptEng->ControlRun(KSRUN_START_ENABLE, history_OptItem->pInfo))
					{
						if (history_OptItem->pInfo->nType == KSRUN_TYPE_STARTUP 
							&& IsOpTrayRunItem(history_OptItem->pInfo->nRegType,history_OptItem->pInfo->strName))
						{						
							KwsSetting setting;
							setting.SetNeedAutorunHit(!history_OptItem->pInfo->bEnable);
						}
						GenerateHistoryList();
						m_pHistoryList->SetItemCount(m_vecHistoryItem.size());
					}
					m_bRefreshList = TRUE;
				}
				RefreshTabNumber(history_OptItem->pInfo->nType);
				break;
			}
		case IDC_IMGBTN_SETING_MENU:
			{
				CKsafeRunInfo* pInfo = history_OptItem->pInfo;
				CMenu	menu;
				menu.CreatePopupMenu();
				menu.AppendMenu(MF_BYCOMMAND, CLICK_INDEX_ENABLE, BkString::Get(IDS_SYSOPT_4083));	//设为开机启动
				if (pInfo->nType == KSRUN_TYPE_STARTUP)
				{
					menu.AppendMenu(MF_BYCOMMAND, CLICK_INDEX_DELAY, BkString::Get(IDS_SYSOPT_4084));	//设为延迟启动
				}
				menu.AppendMenu(MF_BYCOMMAND, CLICK_INDEX_DISABLE, BkString::Get(IDS_SYSOPT_4085));	//禁止开机自启动
				if (pInfo->nType != KSRUN_TYPE_SERVICE)
				{
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_BYCOMMAND, CLICK_INDEX_DELETE, BkString::Get(IDS_SYSOPT_4086));	//删除启动项
				}

				if (pInfo->nType == KSRUN_TYPE_SERVICE)
					menu.EnableMenuItem(CLICK_INDEX_DELETE, MF_GRAYED);
				if (pInfo->bEnable == KSRUN_START_DISABELE)
					menu.EnableMenuItem(CLICK_INDEX_DISABLE, MF_GRAYED);
				if (pInfo->bEnable == KSRUN_START_ENABLE)
					menu.EnableMenuItem(CLICK_INDEX_ENABLE, MF_GRAYED);
				if (pInfo->bEnable == KSRUN_START_DELAY ||
					pInfo->CannotDelay_Sys() ||
					pInfo->nType != KSRUN_TYPE_STARTUP
					)
					menu.EnableMenuItem(CLICK_INDEX_DELAY, MF_GRAYED);

				if (pInfo->bFileIsExist == FALSE)
				{
					menu.EnableMenuItem(CLICK_INDEX_ENABLE, MF_GRAYED);
					menu.EnableMenuItem(CLICK_INDEX_DISABLE, MF_GRAYED);
					menu.EnableMenuItem(CLICK_INDEX_DELAY, MF_GRAYED);
				}

				CPoint	ptx;
				::GetCursorPos(&ptx);
				int i = menu.TrackPopupMenu(TPM_RETURNCMD, ptx.x, ptx.y, m_dlg->m_hWnd);

				if (i == CLICK_INDEX_ENABLE ||
					i == CLICK_INDEX_DELETE ||
					i == CLICK_INDEX_DISABLE||
					i == CLICK_INDEX_DELAY)
				{
					OnClickLabel(i, pInfo);
				}

				GenerateHistoryList();
				m_pHistoryList->SetItemCount(m_vecHistoryItem.size());
				RefreshTabNumber(history_OptItem->pInfo->nType);
				m_bRefreshList = TRUE;
				break;
			}
		default:
			break;
		}
	}
	ShowNoHistoryInfo();
	m_dlg->EnableItem(IDC_LNK_RUNOPT_HISTORY_CANCELALL,m_vecHistoryItem.size() > 0);
	return 0;
}

LRESULT CBeikeSafeSysOptUIHandler::OnListBoxGetDispInfo(LPNMHDR pnmh)
{
	BKLBMGETDISPINFO *pdi = (BKLBMGETDISPINFO*)pnmh;
	if (pdi->nListItemID >= m_vecHistoryItem.size())
		return 0;

	RUNOPT_HISTORY_DATA* history_OptItem = m_vecHistoryItem[pdi->nListItemID];

	if (TRUE == history_OptItem->bTitle)
	{
		//标题的绘制
		pdi->nHeight = HISTORY_LIST_TITLE_HEIGHT;
		m_pHistoryList->SetItemVisible(IDC_DIV_SYSOPT_HISTORY_TITLE,TRUE);
		m_pHistoryList->SetItemVisible(IDC_DIV_SYSOPT_HISTORY_ITEM,FALSE);
		CString strDate;
		if (history_OptItem->strDate.IsEmpty() || history_OptItem->strDate.CompareNoCase(_T("-")) == 0)
		{
			strDate = BkString::Get(IDS_SYSOPT_4081);
		}
		else
		{
			strDate.Format(BkString::Get(IDS_SYSOPT_4080),history_OptItem->strDate);
		}
		m_pHistoryList->SetItemText(IDC_TXT_SYSOPT_OPT_DATE,strDate);
	}
	else
	{
		pdi->nHeight = HISTORY_LIST_ITEM_HEIGHT;
		m_pHistoryList->SetItemVisible(IDC_DIV_SYSOPT_HISTORY_TITLE, FALSE);
		m_pHistoryList->SetItemVisible(IDC_DIV_SYSOPT_HISTORY_ITEM, TRUE);
		//画图标
		if (TRUE)
		{
			if (history_OptItem->pInfo->bSystem)
			{
				char pszDir[MAX_PATH] = {0};
				_snprintf_s(pszDir, sizeof(pszDir), "%d", m_hIconSystem);
				m_pHistoryList->SetItemAttribute(IDC_ICO_RUNITEM_ICON, "srcfile", "0");
				m_pHistoryList->SetItemAttribute(IDC_ICO_RUNITEM_ICON, "iconhandle", pszDir);
			}
			else
			{
				if (history_OptItem->pInfo->nRegType > DIR_RUN_TYPE_BEGIN)
				{
					if (TRUE == PathFileExists(history_OptItem->pInfo->strPath))
					{
						m_pHistoryList->SetItemAttribute(IDC_ICO_RUNITEM_ICON, "iconhandle","0");
						m_pHistoryList->SetItemAttribute(IDC_ICO_RUNITEM_ICON,"srcfile",CW2A(history_OptItem->pInfo->strExePath));
					}
					else
					{	
						char pszDir[MAX_PATH] = {0};
						_snprintf_s(pszDir, sizeof(pszDir), "%d", m_hIconUnknow);
						m_pHistoryList->SetItemAttribute(IDC_ICO_RUNITEM_ICON, "srcfile", "0");
						m_pHistoryList->SetItemAttribute(IDC_ICO_RUNITEM_ICON, "iconhandle", pszDir);
					}
				}
				else
				{
					if (TRUE == PathFileExists(history_OptItem->pInfo->strPath))
					{
						m_pHistoryList->SetItemAttribute(IDC_ICO_RUNITEM_ICON, "iconhandle","0");
						m_pHistoryList->SetItemAttribute(IDC_ICO_RUNITEM_ICON,"srcfile",CW2A(history_OptItem->pInfo->strPath));
					}
					else
					{	
						char pszDir[MAX_PATH] = {0};
						_snprintf_s(pszDir, sizeof(pszDir), "%d", m_hIconUnknow);
						m_pHistoryList->SetItemAttribute(IDC_ICO_RUNITEM_ICON, "srcfile", "0");
						m_pHistoryList->SetItemAttribute(IDC_ICO_RUNITEM_ICON, "iconhandle", pszDir);
					}
				}
			}
		}
		//画标题、描述、建议、状态、操作
		if (TRUE)
		{
			m_pHistoryList->SetItemText(IDC_TXT_RUNITEM_DISPLAY, history_OptItem->pInfo->strDisplay);
			m_pHistoryList->SetItemText(IDC_TXT_RUNITEM_DESC, history_OptItem->pInfo->strDesc.IsEmpty()?BkString::Get(IDS_SYSOPT_4033):history_OptItem->pInfo->strDesc);
			m_pHistoryList->SetItemText(IDC_TXT_RUNITEM_ADVICE, history_OptItem->pInfo->GetAdviceStr());
			
			CString strState;
			if (history_OptItem->pInfo->bEnable == KSRUN_START_DISABELE )
				strState = BkString::Get(IDS_SYSOPT_4035);
			if (history_OptItem->pInfo->bEnable == KSRUN_START_DELAY)
				strState = BkString::Get(IDS_SYSOPT_4092);
			
			m_pHistoryList->SetItemText(IDC_TXT_RUNITEM_STATE, strState);
			m_pHistoryList->SetItemText(IDC_LNK_RUNITEM_CONTROL, BkString::Get(IDS_SYSOPT_4039));
		}
		m_pHistoryList->SetItemStringAttribute(IDC_TXT_RUNITEM_DISPLAY,"tip",history_OptItem->pInfo->strDisplay);
		m_pHistoryList->SetItemStringAttribute(IDC_TXT_RUNITEM_DESC,"tip",history_OptItem->pInfo->strDesc.IsEmpty()?BkString::Get(IDS_SYSOPT_4033):history_OptItem->pInfo->strDesc);
		BOOL bDisable = (history_OptItem->pInfo->bEnable == KSRUN_START_DISABELE);
		m_pHistoryList->SetItemAttribute(IDC_TXT_RUNITEM_STATE, "crtext", bDisable?"FF0000":"3064A5");
	}
	m_pHistoryList->SetItemAttribute(IDC_DIV_SYSOPT_HISTORY_ITEM, "crbg", pdi->bSelect ? "EBF5FF":"FFFFFF");

	return 0;
}

void CBeikeSafeSysOptUIHandler::ShowNoHistoryInfo()
{
	int nItemCount = 0;
	for(int i = 0;i < m_vecHistoryItem.size();i++ )
	{
		if (m_vecHistoryItem[i]->bTitle == FALSE)
			nItemCount++;
	}
	CString strTopTip;
	strTopTip.Format(BkString::Get(IDS_SYSOPT_4077),nItemCount);
	m_dlg->SetRichText(IDC_RICHTEXT_RUNOPT_HISTORY_TIP,strTopTip);

	CString strFormat;
	strFormat.Format(BkString::Get(IDS_SYSOPT_4096),
		nItemCount
		);
	m_dlg->SetTabTitle(IDC_TAB_RUNOPT_TYPE, 3, strFormat);

	m_dlg->SetItemVisible(IDC_LST_RUNOPT_HISTORY, (nItemCount > 0));
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_NO_HISTORY, (nItemCount == 0));
	m_dlg->SetItemVisible(IDC_DIV_RUN_TOPTIP, (nItemCount > 0));
	m_dlg->SetItemVisible(IDC_DIV_NOTASK_INFO , FALSE);
	m_dlg->SetItemVisible(IDC_DIV_RUNOPT_LIST_AREA , FALSE);
}

void CBeikeSafeSysOptUIHandler::RefreshTabNumber_History()
{
	GenerateHistoryList();
	int nItemCount = 0;
	for(int i = 0;i < m_vecHistoryItem.size();i++ )
	{
		if (m_vecHistoryItem[i]->bTitle == FALSE)
			nItemCount++;
	}

	CString strFormat;
	strFormat.Format(BkString::Get(IDS_SYSOPT_4096),
		nItemCount
		);
	m_dlg->SetTabTitle(IDC_TAB_RUNOPT_TYPE, 3, strFormat);
}

bool compare(const RUNOPT_HISTORY_DATA* data1,const RUNOPT_HISTORY_DATA* data2)
{
	if (data1->strDate == data2->strDate)
	{
		return data1->bTitle > data2->bTitle;
	}
	else
	{
		return data1->strDate > data2->strDate;
	}
}

void CBeikeSafeSysOptUIHandler::GenerateHistoryList()
{
	m_vecHistoryItem.clear();
	RunLogItem* pLog = new RunLogItem;
	CRestoreRunner* pLogRunner = new CRestoreRunner;
	CString strFileName;
	int nDisableCount = pLogRunner->GetCount();

	for (int nLoop = 0;nLoop < m_arrayItem.GetSize();nLoop++)
	{
		BOOL bFind = FALSE;

		for(int nLoop2 = 0; nLoop2 < nDisableCount;nLoop2++)
		{
			pLog = pLogRunner->GetItem(nLoop2);
			if (m_arrayItem[nLoop]->bEnable == KSRUN_START_DISABELE
				|| m_arrayItem[nLoop]->bEnable == KSRUN_START_DELAY)			
			{
				if ((m_arrayItem[nLoop]->nID == pLog->nID) 
					&& (pLog->nID != 0))
				{
					bFind = TRUE;
					break;
				}
				else if((m_arrayItem[nLoop]->nType == pLog->nType) 
					&&(m_arrayItem[nLoop]->strName.CompareNoCase(pLog->strName) == 0 ))
				{
					bFind = TRUE;
					break;
				}
			}
		}
		if (bFind)
		{
			CKsafeRunInfo* pInfo = m_arrayItem[nLoop];
			RUNOPT_HISTORY_DATA* pX = new RUNOPT_HISTORY_DATA;

			for(int nLoop3 = 0;nLoop3 < m_vecHistoryItem.size();nLoop3++)
			{
				if (pLog->strDate.CompareNoCase(m_vecHistoryItem[nLoop3]->strDate) == 0)
				{
					pX->bTitle = FALSE;
					break;
				}
			}

			if (pX->bTitle == TRUE)
			{
				RUNOPT_HISTORY_DATA* pX2 = new RUNOPT_HISTORY_DATA;
				pX2->strDate = pLog->strDate;
				m_vecHistoryItem.push_back(pX2);
			}
			pX->pInfo	= pInfo;
			pX->strDate = pLog->strDate;
			pX->bTitle	= FALSE;
			m_vecHistoryItem.push_back(pX);
		}
 	}
	sort(m_vecHistoryItem.begin(),m_vecHistoryItem.end(),compare);
}

void CBeikeSafeSysOptUIHandler::OnBkLnkHistoryCancelAll()
{
	if (!IsOpPermit())
		return;

	CString	strShow;
	if ( BKSafeConfig::GetHasRunDelete() )
		strShow = BkString::Get(IDS_SYSOPT_4071);
	else
		strShow = BkString::Get(IDS_SYSOPT_4072);

	if ( CBkSafeMsgBox2::ShowMultLine(strShow, NULL, MB_YESNO|MB_ICONWARNING) != IDYES)
		return;
	for(int n = 0;n < m_vecHistoryItem.size();n++)	
	{
		if (m_vecHistoryItem[n]->pInfo != NULL)
		{
			BOOL bEable = m_vecHistoryItem[n]->pInfo->bEnable;
			if ((bEable == KSRUN_START_DELAY || bEable == KSRUN_START_DISABELE)  
				&& m_pRunOptEng 
				&& m_vecHistoryItem[n]->bTitle == FALSE)
			{
				if (m_pRunOptEng->ControlRun(KSRUN_START_ENABLE, m_vecHistoryItem[n]->pInfo))
				{
					if (m_vecHistoryItem[n]->pInfo->nType == KSRUN_TYPE_STARTUP 
						&& IsOpTrayRunItem(m_vecHistoryItem[n]->pInfo->nRegType,m_vecHistoryItem[n]->pInfo->strName))
					{						
						KwsSetting setting;
						setting.SetNeedAutorunHit(!m_vecHistoryItem[n]->pInfo->bEnable);
					}
					RefreshTabNumber(m_vecHistoryItem[n]->pInfo->nType);
				}
			}
		}
	}
	m_bRefreshList = TRUE;
	GenerateHistoryList();
	m_pHistoryList->SetItemCount(m_vecHistoryItem.size());
	m_dlg->EnableItem(IDC_LNK_RUNOPT_HISTORY_CANCELALL,m_vecHistoryItem.size() > 0);
	ShowNoHistoryInfo();
}

BOOL CBeikeSafeSysOptUIHandler::GetSubScanEnd(int nType)
{
	if (nType == KSRUN_TYPE_STARTUP)
		return m_bRunScanEnd;
	else if (nType == KSRUN_TYPE_SERVICE)
		return m_bSrvScanEnd;
	else if (nType == KSRUN_TYPE_TASK)
		return m_bTaskScanEnd;
	else if (nType == 0)
		return m_bTaskScanEnd && m_bSrvScanEnd && m_bRunScanEnd;
	else
		return FALSE;
}

void CBeikeSafeSysOptUIHandler::SetSubScanning(int nType,BOOL bIsScanning)
{
	if (nType == KSRUN_TYPE_STARTUP)
		m_bRunScanEnd = !bIsScanning;
	if (nType == KSRUN_TYPE_SERVICE)
		m_bSrvScanEnd = !bIsScanning;
	if (nType == KSRUN_TYPE_TASK)
		m_bTaskScanEnd= !bIsScanning;
	if (nType == 0)
	{
		m_bRunScanEnd	= !bIsScanning;
		m_bSrvScanEnd	= !bIsScanning;
		m_bTaskScanEnd	= !bIsScanning;
	}
}

void CBeikeSafeSysOptUIHandler::callBackCheck()
{
// 	CString strPath;
// 	CAppPath::Instance().GetLeidianAppPath(strPath);
// 
//  	DISPPARAMS param;
//  	param.cArgs = 2;
//  	param.cNamedArgs = 0;
//  	param.rgdispidNamedArgs = NULL;
//  	param.rgvarg = new VARIANTARG[2];
//  	param.rgvarg[0].vt = VT_BSTR;
//  	param.rgvarg[0].bstrVal = strPath.AllocSysString();
// 	param.rgvarg[1].vt = VT_BSTR;
// 	param.rgvarg[1].bstrVal = ::SysAllocString(L"I come from: ");
//  	VARIANT retval;
// 
// 	m_hwndHtmlCallCpp.CallJS(L"MM_popupMsg", &param, &retval);

}

void CBeikeSafeSysOptUIHandler::callBackExec()
{

}

void CBeikeSafeSysOptUIHandler::callBackInstall()
{

}

void CBeikeSafeSysOptUIHandler::_InitKSafeToolsPage()
{
	// 系统工具集创建IE控件 [12/22/2010 zhangbaoliang]
	//create ax
	m_hwndHtmlCallCpp.Create(m_dlg->GetViewHWND(), IDC_DIV_SYSOPT_SYSTOOLS_PAGE, TRUE, RGB(0xFB, 0xFC, 0xFD));

	m_bksafeTools.SetCallbackPtr(this);
}

void CBeikeSafeSysOptUIHandler::ShowDelayTipDlg()
{
	CMsgBoxDelayTip dlgX;
	int nShow = 0;
	CString strIniPath;
	CAppPath::Instance().GetLeidianAppPath(strIniPath);
	strIniPath.Append(FILEPATH);
	CIniFile iniFile(strIniPath);
	iniFile.GetIntValue(CONFIG_SEC_NAME, CONFIG_DONOT_SHOW, nShow);
	if (nShow == TRUE)
		return;
	else
		dlgX.DoModal();
}

void CBeikeSafeSysOptUIHandler::_OpenKSafePage()
{

	//保证只加载一次页面
	static BOOL bFlag = FALSE;
	if (TRUE == bFlag)
		return;

	CString strURL = L"", strUrl2 = L"";

	//http://www.ijinshan.com/safe/tools/index.html?type=%d&pid=%s&oem=%s&ver=%s
	//type是为了以后区分来源使用，现在默认是0
	strURL.Format(BkString::Get(IDS_EXAM_1440), 0, _Module.GetPID(), 
		_Module.GetOrgOem(), _Module.GetProductVersion());

	//防止联网失败，调用本地页面
	if ( SUCCEEDED(CAppPath::Instance().GetLeidianDataPath(strUrl2)) )
		strUrl2.Append(_T("\\html\\ksafe_tools\\index.html")); 
	m_hwndHtmlCallCpp.Show2(strURL, strUrl2);

	bFlag = TRUE;

}

LRESULT CBeikeSafeSysOptUIHandler::OnRestoreDelayRun(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr = S_OK;
	RunLogItem* pSetting = NULL;
	CRestoreRunner customSetting;
	CKsafeRunInfo RunInfo;
	int nDisableCount = customSetting.GetCount();

	m_bDelayRunValid = FALSE;
	if (m_pRunOptEng)
	{
		m_nState = STATE_RUNOPT_CANCEL;
		m_dlg->SetItemVisible(IDC_DIV_RUNOPT_BOTTOM, FALSE);
		for (int nLoop = 0; nLoop < nDisableCount; nLoop ++)
		{
			pSetting = customSetting.GetItem(nLoop);

			if (pSetting->dwNewState == KSRUN_START_DELAY)
			{
				RunInfo.nType = pSetting->nType;
				RunInfo.strName = pSetting->strName;
				RunInfo.bSystem = pSetting->bSystem;

				if (RunInfo.nType == KSRUN_TYPE_TASK)
				{
					RunInfo.nJobType = pSetting->nExtType;
				}
				else if (RunInfo.nType==KSRUN_TYPE_STARTUP)
				{
					RunInfo.nRegType = pSetting->nExtType;
					RunInfo.strPath = pSetting->strName;
				}

				BOOL bRet = m_pRunOptEng->ControlRun(TRUE, &RunInfo);
			}
		}
	}
	RestoreEnd();
	return hr;
}

unsigned _stdcall CBeikeSafeSysOptUIHandler::ThreadProc_ShowDelayRunTip(PVOID param)
{
	CBeikeSafeSysOptUIHandler* pThis = (CBeikeSafeSysOptUIHandler*)param;
	if (pThis == NULL)
		return 0;
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE); 

	while(true)
	{
		if (GetMessage(&msg,0,0,0)) //get msg from message queue
		{
			switch(msg.message)
			{
			case MSG_SYSOPT_SHOW_DELAY_TIP:
				//提示延迟启动新功能，如果之前已经弹过框，则不在弹框
				pThis->ShowDelayTipDlg();
				break;
			}
		}
	}
	return 1;
}

void CBeikeSafeSysOptUIHandler::_ShowDelayTip()
{
	HANDLE hThread;
	unsigned nThreadID;

	//start thread
	hThread = (HANDLE)_beginthreadex( NULL, 0 , ThreadProc_ShowDelayRunTip, this, 0, &nThreadID );
	if(hThread == 0) 
		return;

	int count = 0;
	while(true)
	{
		if (::PostThreadMessage(nThreadID , MSG_SYSOPT_SHOW_DELAY_TIP, 0, 0))
			break;
		::Sleep(100);
	}
	CloseHandle(hThread);
}

void CBeikeSafeSysOptUIHandler::OnBkLnkNetMon()
{
	m_dlg->SetTabCurSel(IDC_TAB_SYSOPT_LEFT, 2);
}

LRESULT CBeikeSafeSysOptUIHandler::OnWmRefreshItemList(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pRunOptEng == NULL) // 引擎没有初始化的时候代表整个页面没被点击，不需要刷新
	{
		return S_OK;
	}

	HRESULT hr = S_OK;
	if (m_dlg->GetTabCurSel(IDC_TAB_SYSOPT_LEFT == 0))
	{
		OnBkBtnRefresh();
	}
	else
	{
		SetSubScanning(0, TRUE);
		RefreshItemList(0);
	}
	return hr;	
}
