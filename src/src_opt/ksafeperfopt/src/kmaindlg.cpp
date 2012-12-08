#include "stdafx.h"
#include "kmaindlg.h"
#include "kpfw/kpfw_def.h"
#include "kflowtray.h"
#include "common/KCheckInstance.h"
#include "kpfw/netflowformat.h"
#include "resource.h"
#include <algorithm>

#include "src/beikesafemsgbox.h"
#include "communits/VerifyFileFunc.h"
#include "softmgr/URLEncode.h"
#include "perfmon/processpath_64.h"
#include "kws/urlmondef.h"
#include "kpfw/netfluxdef.h"
#include "iefix/cinifile.h"
#include "dialogsysperfopt.h"
#include "dialogsysperfopt.h"
#include "wndoptdlg.h"

#define		FLOAT_WND_CFG_FILE		L"\\cfg\\floatwnd.ini"


KMainDlg::~KMainDlg()
{
	//KillTimer(ID_TIMER_REFRESH_ONKEY_HISTOR);
}

void KMainDlg::BtnClose( void )
{
	EndDialog(0);
}

BOOL KMainDlg::OnInitDialog( CWindow /*wndFocus*/, LPARAM /*lInitParam*/ )
{
	SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_BEIKESAFE)));
	SetIcon(::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_SMALL)), FALSE);
	KCheckInstance::Instance()->CfgFirstInstance(NULL, this->m_hWnd, FALSE);

	m_hEventExit = ::CreateEvent(
		NULL,
		FALSE,
		FALSE,
		EVENT_SYSTEM_PERFORMANCE_FLOATWND_EXIT
		);

	KFlowTray prot_shell(TRUE);
	prot_shell.ShellTray();

	m_wow64Swithcer.Init();
	m_bIsWin64 = m_wow64Swithcer.IsWin64();
	m_ProcPerfMgr.Init();
	if (m_bIsWin64)
	{
		HANDLE hThread2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pfnWMIQueryProcInfo, this, 0, 0 );
		if (NULL != hThread2)
			CloseHandle(hThread2);
		hThread2 = NULL;
	}
	if (!m_pModuleMgr)
	{
		m_pModuleMgr = new KModuleMgr;
		m_pModuleMgr->Init();
	}

	WCHAR buffer[MAX_PATH] = {0};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	PathRemoveFileSpec(buffer);
	m_strAppFilePath = buffer;
	m_strCfgFilePath = m_strAppFilePath;
	m_strCfgFilePath.Append(FLOAT_WND_CFG_FILE);

	CListBoxItemData::GetDataPtr()->GetStateArray()[m_nCurColum] = 0;	//_SetColumState()会自动+1
	_SetColumState(m_nCurColum);	//默认按内存降序排列

	_GetProcPerfData(m_processInfoList);
	_InitPerfMonList();
	m_uTimer = SetTimer(ID_TIMER_UPDATE_PROCPERF_MON, UPDATE_PROCPERF_MON_INTERVAL);
/*	SetTimer(ID_TIMER_REFRESH_TOTAL_USAGE, REFRESH_TOTAL_USAGE_INTERVAL);*/

	m_bFloatWndIsOpen = GetFloatWndDisplayStatus();
	_HideOrShowTip(m_bFloatWndIsOpen, FALSE);
	SetTimer(ID_TIMER_UPDATE_FLOATWND_STATE, UPDATA_FLOATWND_STATE);
	SetTimer(ID_TIMER_CHECK_EXIT, CHECK_EXIT_EVENT_INTERVAL);


	//////////////////////////////////////////////////////////////////////////
	InitUI();
	_SwitchUI(0);
	SwitchLinkText(PERF_MONITOR_CTRL_ID_807);
	
	UpDateAppWndListData();
	_InitWndProcList();

	CString strAppWndInfo;
	strAppWndInfo.Format(BkString::Get(PERF_MONITOR_STR_525), m_vecWndListItemData.size());
	SetRichText(PERF_MONITOR_CTRL_ID_815, strAppWndInfo);

	SetTimer(ID_TIMER_DO_ONEKEYSPEEDUP_AT_RUN, 2500);
	SetTimer(ID_TIMER_REFRESH_WNDPROC_DATA, UPDATE_WNDPROC_DATA_INTERVAL);
	SetTimer(ID_TIMER_REFRESH_ONKEY_HISTOR, 60000, NULL);
	UpDateOneKeyFreeMemHistory();
	return TRUE;
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

	if(timerId == ID_TIMER_UPDATE_PROCPERF_MON)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hEventGetNetData, 300))
		{
			_GetProcPerfData(m_processInfoList);
			_RefreshListBoxData(m_processInfoList);
		}
	}

	if (timerId == ID_TIMER_UPDATE_FLOATWND_STATE)
	{
		m_bFloatWndIsOpen = IsSysPerfFloatWndVisable();
		_HideOrShowTip(m_bFloatWndIsOpen, FALSE);
	}

	if (timerId == ID_TIMER_CHECK_EXIT)
	{
		DWORD dwReCoder = WaitForSingleObject(m_hEventExit, 0);

		if (WAIT_OBJECT_0 == dwReCoder  )
		{
			ResetEvent(m_hEventExit);
			EndDialog(0);
		}
	}

	if (timerId == ID_TIMER_REFRESH_ONKEY_HISTOR)
	{
		UpDateOneKeyFreeMemHistory();
	}

	if(timerId == ID_TIMER_REFRESH_WNDPROC_DATA)
	{
		_GetProcPerfData(m_processInfoList);
		UpDateAppWndListData();
		_RefreshAppWndListData(m_vecWndListItemData);
		CString strAppWndInfo;
		strAppWndInfo.Format(BkString::Get(PERF_MONITOR_STR_525), m_vecWndListItemData.size());
		SetRichText(PERF_MONITOR_CTRL_ID_815, strAppWndInfo);
		if (0 >= m_vecWndListItemData.size())
		{
			SetItemVisible(IDC_LIST_WNDPROC_APP, FALSE);
			EnableItem(PERF_MONITOR_CTRL_ID_808, FALSE);
		}
		else
		{
			SetItemVisible(IDC_LIST_WNDPROC_APP, TRUE);
			EnableItem(PERF_MONITOR_CTRL_ID_808, TRUE);
		}
	}

	if (timerId == ID_TIMER_DO_ONEKEYSPEEDUP_AT_RUN)
	{
		if (m_bDoOnekeySpeedUpAtRun == TRUE)
		{
			m_bDoOnekeySpeedUpAtRun = FALSE;
			OnOneKeySpeedUp();
		}
	}

// 	if (timerId == ID_TIMER_REFRESH_TOTAL_USAGE)
// 	{
// 		_RefreshSysCpu_Mem_IO_Usage();
// 	}

	return TRUE;
}

void KMainDlg::SetNotActiveTime(HWND hWnd)
{
/*	std::map<HWND, DWORD>::iterator it = m_mapNotActiveTime.find(hWnd);*/
	std::vector<KWndTimeItem>::iterator it;
	BOOL bFind = FALSE;
	for(it = m_vecWndTimeInfo.begin(); it != m_vecWndTimeInfo.end() ;it++ )
	{
		if (it->hWnd == hWnd)
		{
			m_mapNotActiveTime[hWnd] = it->dwUnActiveTime;
			bFind = TRUE;
			break;
		}
	}

	if (!bFind)
		m_mapNotActiveTime[hWnd] = 0;
}

LRESULT KMainDlg::OnDestroy()
{
	m_ProcPerfMgr.UnInit();
	if (m_pModuleMgr)
	{
		m_pModuleMgr->UnInit();
		delete m_pModuleMgr;
		m_pModuleMgr = NULL;
	}
	return TRUE;
}

void KMainDlg::OnSysCommand( UINT nID, CPoint pt )
{
	if ( nID == SC_CLOSE )
	{
		EndDialog(0);
	}
	else if (nID == SC_MAXIMIZE)
		return;
	else if (nID == SC_MINIMIZE)
		DefWindowProc();
	DefWindowProc();
}


void KMainDlg::pfnWMIQueryProcInfo()
{
	KX64ProcPathQuery::Instance().GetProcessFullPath_x64();
}
void KMainDlg::_GetProcPerfData(vector<PerfDataPair>& vetPerfData)
{
	KProcInfoMonCacheReader cacherReader;
	m_processInfoList.clear();

	m_ProcPerfMgr.Update(Update_Perf_Data_And_Calc);
	m_mapPerfData.clear();
	m_mapPerfData = m_ProcPerfMgr.GetPerfData();

	if (m_mapPerfData.size() > 0)
	{//为了使用sort排序，再把它们放到vector里面，以后的操作基于vector
		m_processInfoList = std::vector<PerfDataPair>(m_mapPerfData.begin(), m_mapPerfData.end());
	}

	// 从服务获取全路径和安全信息
	if ( SUCCEEDED(cacherReader.Init()) )
	{
		int nCount = -1;
		KPocessInfoList* pProcInfoList = cacherReader.GetProcessPerfList();

		PBYTE pBuffer = NULL;
		do 
		{
			if (NULL == pProcInfoList)
				break;

			pProcInfoList->m_lock.LockRead();
			nCount = 0;

			KProcInfoItem* pItemInfo = (KProcInfoItem*)&(pProcInfoList->m_Items[0]);
			if (NULL == pItemInfo)
				break;

			int nItemCount = pProcInfoList->m_nSize;
			if (0 == nItemCount)
				break;

			if (nItemCount > pProcInfoList->m_nMaxCnt)
				nItemCount = pProcInfoList->m_nMaxCnt;

			int nTotalByteSize = nItemCount*sizeof(KProcInfoItem);
			pBuffer = new BYTE[nTotalByteSize+1];
			if (NULL == pBuffer)
				break;

			ZeroMemory(pBuffer, nTotalByteSize+1);
			memcpy(pBuffer, pItemInfo, nTotalByteSize);

			pProcInfoList->m_lock.UnLockRead();

			nCount = nItemCount;

			pItemInfo = (KProcInfoItem*)pBuffer;

			for (int i = 0; i < nItemCount; i++)
			{
				map<DWORD, KProcessPerfData*>::iterator item = m_mapPerfData.find( pItemInfo[ i ].m_nProcessID );
				if ( item != m_mapPerfData.end() )
				{
					item->second->SetProcessPath( pItemInfo[ i ].m_strProcPath );
					item->second->SetTrustMode( pItemInfo[ i ].m_nTrustMode );

					if (item->second->GetProcessID() == 0 || item->second->GetProcessID() == 4)
						item->second->SetTrustMode(enumTM_Safe);
				}
			}

		} while (FALSE);

		if (0 == nCount)
			pProcInfoList->m_lock.UnLockRead();

		if (NULL != pBuffer)
			delete []pBuffer;
		pBuffer = NULL;
	}

	// 补全拿不到的路径和安全信息，服务不能提供的，自己扫描获取
	std::vector<PerfDataPair>::iterator it;
	for (it = m_processInfoList.begin(); it != m_processInfoList.end(); it++)
	{
		KProcessPerfData* pPerfData = it->second;
		if (!pPerfData->IsSpecialProcess())
		{
			if (pPerfData->GetProcessPath().GetLength() == 0)
			{
				CString strPath;
				// 先尝试使用OpenProcess的方法
				if (KX64ProcPathQuery::Instance().GetProcPathByID(pPerfData->GetProcessID(), 
					pPerfData->GetProcessName(),
					strPath))
				{
					pPerfData->SetProcessPath(strPath);
				}
				else
				{
					// OpenProcess 不行，尝试wmi的方法
					strPath = KX64ProcPathQuery::Instance().GetProcessFullPathByPid(pPerfData->GetProcessID());
					pPerfData->SetProcessPath(strPath);
				}
			}

			if ((pPerfData->GetProcessTrustMode() == 0) && m_pModuleMgr)
			{
				// if (::PathFileExists(pPerfData->GetProcessPath()))
				{
					KModuleInfo* pModule = NULL;
					ULONGLONG nID = 0;
					m_pModuleMgr->AddModule(pPerfData->GetProcessPath(), nID, pModule);
					if (pPerfData->IsNewCreate())
						pModule->SetTestChange(TRUE);
					if (pModule)
					{
						pPerfData->SetTrustMode(pModule->GetSecurityState());
					}
				}
			}
		}
	}

	// 过滤掉不需要显示的进程
	if (m_bHideSystem == TRUE)
	{
		std::vector<PerfDataPair>::iterator it;
		for (it = m_processInfoList.begin(); it != m_processInfoList.end(); )
		{
			KPerfMonListItemData ItemData(it->second);
			KSProcessInfo ProcInfo ;
			int nProcType = 0;
			if (CListBoxItemData::GetDataPtr()->QueryProcInfo(ItemData.GetProcessFullPath(), ProcInfo) == 0)
			{
				nProcType = ProcInfo.nProcessType;
			}

			if (ItemData._GetProcessType() == enumProcessTypeOS ||
				nProcType == enum_ProcessType_SystemCore ||
				nProcType == enum_ProcessType_SystemProgram ||
				nProcType == enum_ProcessType_SystemService)
			{
				it = m_processInfoList.erase(it);
			}
			else
				++it;
		}
	}

	_SortListData(m_nCurColum);
}

void KMainDlg::_InitPerfMonList()
{
	m_pPerfMonitorListBox = new CBkNetMonitorListBox;
	if (NULL == m_pPerfMonitorListBox)
		return;

	//ListBox
	m_pPerfMonitorListBox->Create( GetViewHWND(), IDC_LST_PERF_MONITOR_LISTBOX);
	m_pPerfMonitorListBox->Load(IDR_BK_LISTBOX_PERF_MONITOR);
	m_pPerfMonitorListBox->SetCanGetFocus(FALSE);

	_RefreshListBoxData(m_processInfoList);

	//Menu
	m_hPerfMonMenu = GetSubMenu(AtlLoadMenu(MAKEINTRESOURCE(IDR_MENU_PERF_MONITOR)), 0);
}

LRESULT KMainDlg::ShowUI( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (this->IsIconic())
		this->ShowWindow(SW_RESTORE);

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

	if (wParam == 1001)	//按cpu降序
	{
		CListBoxItemData::GetDataPtr()->GetStateArray()[LISTBOX_COLUM_CPU] = 0;
		_SetColumState(LISTBOX_COLUM_CPU);
		_SortListData(LISTBOX_COLUM_CPU);
		_RefreshListBoxData(m_processInfoList);
	}
	if (wParam == 1002)	//按mem降序
	{
		CListBoxItemData::GetDataPtr()->GetStateArray()[LISTBOX_COLUM_MEM] = 0;
		_SetColumState(LISTBOX_COLUM_MEM);
		_SortListData(LISTBOX_COLUM_MEM);
		_RefreshListBoxData(m_processInfoList);
	}
	if (wParam == 1003)	//一键优化
	{
		OnOneKeySpeedUp();
	}

	return 0;
}

void KMainDlg::BtnMin( void )
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE | HTCAPTION, 0);
}

BOOL KMainDlg::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	NotifyMouseWhellToListBox(m_pPerfMonitorListBox, nFlags, zDelta, pt);

	NotifyMouseWhellToListBox(m_pWndProcList, nFlags, zDelta, pt);

	SetMsgHandled(FALSE);
	return TRUE;	 
}

LRESULT KMainDlg::OnListBoxGetDispInfo(LPNMHDR pnmh)
{
	HRESULT hRet = S_OK;
	if (m_pPerfMonitorListBox == NULL)
		return E_POINTER;

	BKLBMGETDISPINFO* pdi = (BKLBMGETDISPINFO*)pnmh;

	if (CListBoxItemData::GetDataPtr()->GetMaxHeightSel() == pdi->nListItemID)
	{
		pdi->nHeight = HEIGHT_ITEM_MAX;
		m_pPerfMonitorListBox->SetItemVisible(IDC_DIV_PERFMON_LIST_ITEM_EXP, TRUE);
	}
	else
	{
		m_pPerfMonitorListBox->SetItemVisible(IDC_DIV_PERFMON_LIST_ITEM_EXP, FALSE);
		pdi->nHeight = HEIGHT_ITEM_MIN;
	}

	if (pdi->nListItemID >= m_processInfoList.size())
		return E_FAIL;

	DWORD dwStyle = m_pPerfMonitorListBox->GetStyle();			
	if (FALSE == ::IsWindowVisible(m_pPerfMonitorListBox->m_hWnd))
	{
		m_pPerfMonitorListBox->ModifyStyle(0, WS_VISIBLE);
		CListBoxItemData::GetDataPtr()->GetLogModule().WriteLog(L"ListBox visable = false");
	}

	KProcessPerfData* perfItemData = m_processInfoList[pdi->nListItemID].second;
	KPerfMonListItemData ItemData = KPerfMonListItemData(perfItemData);

	CListBoxItemData::GetDataPtr()->GetLogModule().WriteLog(L"ListCtrl VISABLE：%d ", IsItemVisible(IDC_LST_PERF_MONITOR_LISTBOX));

	_RefreshSysCpu_Mem_IO_Usage();
	//进程名字
	_RefreshProcName(ItemData);
	//-进程描述，如果在进程库里面没有查找到的话就是用程序本身的
	_RefreshProcDes(pdi->nListItemID, ItemData);

	//危险等级
	_RefreshProcSercurityLevel(pdi->nListItemID, ItemData);

	//CPU占用
	_RefreshProcCPUUsage(pdi->nListItemID, ItemData);

	//内存占用
	_RefreshProcMemUsage(pdi->nListItemID, ItemData);

	//更新进程磁盘IO
	_RefreshProcDiskIO(pdi->nListItemID, ItemData);

	//进程PID
	_RefreshProcID(ItemData);

	//显示路径
	_RefreshProcPath(pdi->nListItemID, ItemData);

	//更新图标
	_RefreshProcICON(ItemData);

	//更新操作按钮
	_RefreshProcButton(pdi->nListItemID, ItemData);
	m_pPerfMonitorListBox->SetItemAttribute(IDC_DIV_PERFMON_LIST_ITEM, "crbg", pdi->bSelect ? "ECF9FF":"FFFFFF");

	return hRet;
}

LRESULT KMainDlg::OnListBoxGetMaxHeight(LPNMHDR pnmh)
{
	HRESULT hRet = S_OK;
	BKLBITEMCALCMAXITEM *pdi = (BKLBITEMCALCMAXITEM*)pnmh;
	pdi->nMaxHeight = HEIGHT_ITEM_MAX;
	return hRet;
}

LRESULT KMainDlg::OnListBoxGetItemHeight(LPNMHDR pnmh)
{
	HRESULT hRet = S_OK;
	BKLBITEMCACLHEIGHT *pdi	= (BKLBITEMCACLHEIGHT*)pnmh;
	if (CListBoxItemData::GetDataPtr()->GetMaxHeightSel() == pdi->nListItemId)
	{
		pdi->nHeight = HEIGHT_ITEM_MAX;
	}
	else
	{
		pdi->nHeight = HEIGHT_ITEM_MIN;
	}
	return hRet;
}

LRESULT KMainDlg::OnListBoxClickCtrl(LPNMHDR pnmh)
{
	HRESULT hRet = S_OK;

	LPBKLBMITEMCLICK pnms = (LPBKLBMITEMCLICK)pnmh;
	if (pnms->nListItemID >= m_pPerfMonitorListBox->GetCount())
		return E_FAIL;
	if (pnms->nListItemID >= m_processInfoList.size())
		return E_FAIL;

	KProcessPerfData* perfItemData = m_processInfoList[pnms->nListItemID].second;
	KPerfMonListItemData ItemData = KPerfMonListItemData(perfItemData);

	if (pnms->uCmdID == IDC_IMGBTN_PERFMON_MENU)
	{
		KProcessPerfData* perfItemData = m_processInfoList[pnms->nListItemID].second;
		KPerfMonListItemData ItemData = KPerfMonListItemData(perfItemData);
		CListBoxItemData::GetDataPtr()->SetCurSelData(ItemData);
		OnClickPerfOptMenuBtn();
		goto Exit0;
	}
	else if (pnms->uCmdID == IDC_LNK_PROCESS_OPEN_DIR)
	{
		if (m_bIsWin64)
		{
			m_wow64Swithcer.Close();
			DWORD dwPID = ItemData.GetProcessId();
			CString strFilePath = KX64ProcPathQuery::Instance().GetProcessFullPathByPid(dwPID);
			ItemData.SetProcessPath(strFilePath);
		}

		OnClickOpenDir(ItemData);

		if (m_bIsWin64)
		{
			m_wow64Swithcer.Revert();
		}
		goto Exit0;
	}
	else if (pnms->uCmdID == IDC_LNK_CLOSE_PROCESS)
	{
		if (m_bIsWin64)
		{
			m_wow64Swithcer.Close();
			DWORD dwPID = ItemData.GetProcessId();
			CString strFilePath = KX64ProcPathQuery::Instance().GetProcessFullPathByPid(dwPID);
			ItemData.SetProcessPath(strFilePath);
		}

		OnClickKillProcess(ItemData);

		if (m_bIsWin64)
		{
			m_wow64Swithcer.Revert();
		}
		goto Exit0;
	}

	if (CListBoxItemData::GetDataPtr()->GetMaxHeightSel() != pnms->nListItemID)
	{//并且是不能选择元素的情况下才展开
		CListBoxItemData::GetDataPtr()->SetMaxHeightSel(pnms->nListItemID);
		m_pPerfMonitorListBox->SetItemHeight(CListBoxItemData::GetDataPtr()->GetMaxHeightSel(), HEIGHT_ITEM_MAX);
	}
	else
	{
		CListBoxItemData::GetDataPtr()->SetMaxHeightSel(-1);
		m_pPerfMonitorListBox->SetItemHeight(pnms->nListItemID, HEIGHT_ITEM_MIN);
		m_pPerfMonitorListBox->ForceRefresh();
		m_pPerfMonitorListBox->ResetMouseMoveMsg();
	}

Exit0:
	return hRet;
}

LRESULT KMainDlg::OnListBoxRClickCtrl(LPNMHDR pnmh)
{
	LPBKRBMITEMCLICK pnms = (LPBKRBMITEMCLICK)pnmh;
	if (pnms->nListItemID >= m_pPerfMonitorListBox->GetCount())
		return E_FAIL;
	if (pnms->nListItemID >= m_processInfoList.size())
		return E_FAIL;

	KProcessPerfData* perfItemData = m_processInfoList[pnms->nListItemID].second;
	KPerfMonListItemData ItemData = KPerfMonListItemData(perfItemData);

	CListBoxItemData::GetDataPtr()->SetCurSelData(ItemData);
	OnClickPerfOptMenuBtn();

	return 0;
}

BOOL KMainDlg::GetFloatWndDisplayStatus()
{
	int nRet = 0;

	IniFileOperate::CIniFile cfgFile(GetAppCfgPath());

	if (IsFloatWndStartWithTray() == FALSE)
		return FALSE;

	cfgFile.GetIntValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_OPEN_SYSPERFORMANCE_FLOATWND, nRet);	
	return (nRet != 0);
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
//和GetFloatWndDisplayStatus一样，这里判断都是读取floadwnd.ini，应该淡出剥离出来，和悬浮窗共用，悬浮窗读取的时候默认值为1，逻辑太绕
BOOL KMainDlg::IsSysPerfFloatWndVisable()
{
	int nRet = 0;
	IniFileOperate::CIniFile cfgFile(GetAppCfgPath());
	
	cfgFile.GetIntValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_DISPLAY_WITH_TRAY_KEY_NAME, nRet);
	if (1 != nRet)
		return FALSE;

	cfgFile.GetIntValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_OPEN_SYSPERFORMANCE_FLOATWND, nRet);
	return (nRet != 0);
}

BOOL KMainDlg::IsFloatWndStartWithTray()
{
	if (_CheckDriverIsOk() == FALSE)
		return FALSE;
	int nRet = 0;
	IniFileOperate::CIniFile cfgFile(GetAppCfgPath());
	cfgFile.GetIntValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_DISPLAY_WITH_TRAY_KEY_NAME, nRet);
	return nRet == 1;
}

BOOL KMainDlg::_CheckDriverIsOk()
{
	KUrlMonCfgReader urlmonCfgReader;

	// 配置文件初始化成功，且流量信息初始化失败，则驱动加载失败。
	BOOL ret = SUCCEEDED(KNetFluxCacheReader::Instance().Init());

	return (SUCCEEDED(urlmonCfgReader.Init()) && ret);;
}

void KMainDlg::_SetSysPerfFloatWndIniCfg(int nValue)
{
	IniFileOperate::CIniFile iniFile(GetAppCfgPath());
	iniFile.SetIntValue(FLOATWND_CONFIG_SECTION_NAME, FLOATWND_CONFIG_OPEN_SYSPERFORMANCE_FLOATWND, nValue);
}

void KMainDlg::OpenSysPerfFloatWnd()
{
	HANDLE hEventOpenFloatWndDlg = ::OpenEvent(
		EVENT_ALL_ACCESS,
		FALSE,
		EVENT_SYSTEM_PERFORMANCE_DLG_UPDATE_FLOATWND_STATUS
		);

	if (hEventOpenFloatWndDlg == NULL)
	{
		/*		netmon_log(L"_OpenFloatWnd  open EVENT_OPEN_FLOATWND_DLG failed");*/
		return;
	}

	if( FALSE == IsFloatWndStartWithTray() )
	{
		OpenFloatWnd();
	}

	SetEvent(hEventOpenFloatWndDlg);
	CloseHandle(hEventOpenFloatWndDlg);

	/*	netmon_log(L"SetEvent:EVENT_OPEN_FLOATWND_DLG");*/
	return;

}

void KMainDlg::CloseSysPerfFloatWnd()
{
	HANDLE hEventCloseFloatWndDlg = ::OpenEvent(
		EVENT_ALL_ACCESS,
		FALSE,
		EVENT_SYSTEM_PERFORMANCE_CLOSE_TIP_DLG
		);

	if (hEventCloseFloatWndDlg == NULL)
	{
		/*		netmon_log(L"_CloseFloatWnd  open EVENT_CLOSE_FLOATWND_DLG failed");*/
		return;
	}

	SetEvent(hEventCloseFloatWndDlg);
	CloseHandle(hEventCloseFloatWndDlg);

	/*	netmon_log(L"SetEvent:EVENT_CLOSE_FLOATWND_DLG");*/
	return;
}

void KMainDlg::_ShowFloatWnd( BOOL bShow )
{
	if(bShow)
	{
		OpenSysPerfFloatWnd();
	}
	else
	{
		CloseSysPerfFloatWnd();
	}
	_SetSysPerfFloatWndIniCfg(bShow);
	m_bFloatWndIsOpen = bShow;
	_HideOrShowTip(m_bFloatWndIsOpen, FALSE);
}

void KMainDlg::_HideOrShowTip(IN BOOL bIsShowTip, IN BOOL bIsNeedAct)
{

	if (!bIsShowTip)
	{
		FormatItemText(IDC_TXT_CURRENT_SETTING, BkString::Get(DefString9), BkString::Get(DefString11));
		SetItemText(IDC_LNK_TEXT_SHOW_OR_HIDE_TIP, BkString::Get(DefStringShow));
	}
	else
	{
		FormatItemText(IDC_TXT_CURRENT_SETTING, BkString::Get(DefString9), BkString::Get(DefString10));
		SetItemText(IDC_LNK_TEXT_SHOW_OR_HIDE_TIP, BkString::Get(DefStringHide));
	}

	if (bIsNeedAct)
		_ShowFloatWnd(!bIsShowTip);
}


void KMainDlg::OnClickShowOrHideTip()
{
	_HideOrShowTip(m_bFloatWndIsOpen);
}

void KMainDlg::OnBkCheckHideSystemClick()
{
	m_bHideSystem = GetItemCheck(IDC_CHECK_HIDE_SYSTEM);
	OnTimer(ID_TIMER_UPDATE_PROCPERF_MON);
}

LRESULT KMainDlg::OnPerfMonCommand(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	KPerfMonListItemData ItemData = CListBoxItemData::GetDataPtr()->GetCurSelData();
	switch(wParam)
	{
	case ID_PERFOPT_KILLPROC:
		{//关闭进程
			if (m_bIsWin64)
			{
				m_wow64Swithcer.Close();
				DWORD dwPID = ItemData.GetProcessId();
				CString strFilePath = KX64ProcPathQuery::Instance().GetProcessFullPathByPid(dwPID);
				ItemData.SetProcessPath(strFilePath);
			}

			if (ItemData.KillCurrentSelProcess() == TRUE)
			{
				OnTimer(ID_TIMER_UPDATE_PROCPERF_MON);
			}

			if (m_bIsWin64)
			{
				m_wow64Swithcer.Revert();
			}
		}
		break;
	case ID_PERFOPT_OPENDIR:
		{//打开文件路径
			if (m_bIsWin64)
			{
				m_wow64Swithcer.Close();
				DWORD dwPID = ItemData.GetProcessId();
				CString strFilePath = KX64ProcPathQuery::Instance().GetProcessFullPathByPid(dwPID);
				ItemData.SetProcessPath(strFilePath);
			}

			ItemData.OpenDirector();

			if (m_bIsWin64)
			{
				m_wow64Swithcer.Revert();
			}
		}
		break;
	case ID_PERFOPT_FILEATTRI:
		{//查看文件属性
			if (m_bIsWin64)
			{
				m_wow64Swithcer.Close();
				DWORD dwPID = ItemData.GetProcessId();
				CString strFilePath = KX64ProcPathQuery::Instance().GetProcessFullPathByPid(dwPID);
				ItemData.SetProcessPath(strFilePath);
			}

			ItemData.OpenFileAttribute();

			if (m_bIsWin64)
			{
				m_wow64Swithcer.Revert();
			}
		}
		break;
	default:
		break;
	}
	SetMsgHandled(FALSE);
	return 0;
}

void KMainDlg::_RefreshListBoxData(vector<PerfDataPair>& vetPerfData)
{
	CAutoLocker locker(m_locker);

	//更新列表
	if (m_pPerfMonitorListBox->GetCount() != vetPerfData.size())
	{
		static int a = 1;
		//if (1 == a)
		m_pPerfMonitorListBox->SetItemCount(vetPerfData.size());
		m_pPerfMonitorListBox->RefereshVScroll();
		a = 0;
	}
	else
	{
		m_pPerfMonitorListBox->ForceRefresh();
		m_pPerfMonitorListBox->ResetMouseMoveMsg();
	}

	SetEvent(m_hEventGetNetData);
}

void KMainDlg::_SortListData(int nColum)
{
	int nColumSortAsc = CListBoxItemData::GetDataPtr()->GetStateArray()[m_nCurColum];
	if (m_processInfoList.size() >0 )
		std::sort(m_processInfoList.begin(), m_processInfoList.end(), KPerfDataSorter(nColumSortAsc, nColum));
}

void KMainDlg::_CreateCacheData(vector<KProcInfoItem>& vetNetData, CAtlMap<DWORD, CPerfMonitorCacheData>& mapCacheData)
{
	int nColum = -1, nState = -1;

	//直接使用STL的排序
	//std::sort(vetNetData.begin(), vetNetData.end(), stl_SortPerfInfo);

	mapCacheData.RemoveAll();
	//复制新数据到缓存
	for (int i = 0; i < vetNetData.size(); i++)
	{
		mapCacheData[i] = CPerfMonitorCacheData(vetNetData[i], i);
	}
}

void KMainDlg::_RefreshSysCpu_Mem_IO_Usage()
{
	m_fSysCpuUsage = m_ProcPerfMgr.GetSysCpuUsage();
	m_fSysMemUsage = m_ProcPerfMgr.GetSysMemUsage();
	m_nProcessNum  = m_ProcPerfMgr.GetProcessNum();
	m_ullTotalIo = m_ProcPerfMgr.GetTotalIo();

	CString strProcNum;
	strProcNum.Format(BkString::Get(PERF_MONITOR_STR_512), m_nProcessNum);
	SetItemText(IDC_TXT_PROCESS_SUM_COUNT, strProcNum);

	if (m_fSysCpuUsage >= 0.0 && m_fSysCpuUsage <= 100.0)
	{
		CString strCpuUsage;
		strCpuUsage.Format(BkString::Get(PERF_MONITOR_STR_513), int(m_fSysCpuUsage));
		SetItemText(IDC_TXT_SYSTEM_SUM_CPUUSAGE, strCpuUsage);
	}

	if (m_fSysMemUsage >= 0.0 && m_fSysMemUsage <= 100.0)
	{
		CString strMemUsage;
		strMemUsage.Format(BkString::Get(PERF_MONITOR_STR_514), int(m_fSysMemUsage));
		SetItemText(IDC_TXT_SYSTEM_SUM_MEMUSAGE, strMemUsage);
	}

	if (m_ullTotalIo > 0)
	{
		CString strTotalIo;
		strTotalIo.Format(BkString::Get(PERF_MONITOR_STR_515), m_ullTotalIo);
		SetItemText(IDC_TXT_SYSTEM_SUM_IO, strTotalIo);
	}
}

int KMainDlg::_RefreshProcName(KPerfMonListItemData ItemData)
{
	CString strProcCaption = ItemData.GetProcessName();
	CString strFilePath;
	DWORD dwPID = ItemData.GetProcessId();
	if (m_bIsWin64)
	{
		m_wow64Swithcer.Close();
		strFilePath = KX64ProcPathQuery::Instance().GetProcessFullPathByPid(dwPID);
	}
	else
		strFilePath = ItemData.GetProcessFullPath();

	KSProcessInfo	ProcInfo;
	if ((strFilePath.GetLength() > 0) && 
		(0 == CListBoxItemData::GetDataPtr()->QueryProcInfo(strFilePath, ProcInfo)) && 
		ProcInfo.strDisplayName.length() > 0)
	{//查找进程库,这里和描述分别都做了查找操作，不过不会影响到性能，因为本地有缓存
		strProcCaption = ProcInfo.strDisplayName.c_str();
		CListBoxItemData::GetDataPtr()->GetLogModule().WriteLog(L"_RefreshProcName: find %s", strProcCaption);
	}
	else
	{
		CListBoxItemData::GetDataPtr()->GetLogModule().WriteLog(L"_RefreshProcName: not find %s", strProcCaption);
	}

	if (m_bIsWin64)
	{
		m_wow64Swithcer.Revert();
	}
	m_pPerfMonitorListBox->SetItemText(IDC_TXT_PROCESS_CAPTION, strProcCaption);
	m_pPerfMonitorListBox->SetItemStringAttribute(IDC_TXT_PROCESS_CAPTION, "tip", strProcCaption);

	return 0;
}

int KMainDlg::_RefreshProcDes(int nCurSel, KPerfMonListItemData ItemData)
{
	CString	 strProcDesc = L"";
	KSProcessInfo ProcInfo ;
	CStringA strPos = _T("50,32,270,44");

	CString strFilePath;
	DWORD dwPID = ItemData.GetProcessId();
	if (m_bIsWin64)
	{
		m_wow64Swithcer.Close();
		strFilePath = KX64ProcPathQuery::Instance().GetProcessFullPathByPid(dwPID);
	}
	else
		strFilePath = ItemData.GetProcessFullPath();

	if ((strFilePath.GetLength() > 0) &&
		(0 == CListBoxItemData::GetDataPtr()->QueryProcInfo(strFilePath, ProcInfo)) && 
		(0 != ProcInfo.strDesc.length()))
	{//查找进程库
		strProcDesc = ProcInfo.strDesc.c_str();
	}
	else
	{//使用程序自身的
		strProcDesc = ItemData.GetProcessDescription();
		CListBoxItemData::GetDataPtr()->GetLogModule().WriteLog(L"信息缺失：%s ", strProcDesc);
	}

	if (nCurSel == CListBoxItemData::GetDataPtr()->GetMaxHeightSel())
		strPos = _T("50,32,-10,44");

	m_pPerfMonitorListBox->SetItemAttribute(IDC_TXT_PROCESS_DESCRIPTION, "pos", strPos);
	m_pPerfMonitorListBox->SetItemAttribute(IDC_TXT_PROCESS_DESCRIPTION, "class", "filename");
	m_pPerfMonitorListBox->SetItemStringAttribute(IDC_TXT_PROCESS_DESCRIPTION, "tip", strProcDesc);
	m_pPerfMonitorListBox->SetItemText(IDC_TXT_PROCESS_DESCRIPTION, strProcDesc);

	if (m_bIsWin64)
	{
		m_wow64Swithcer.Revert();
	}

	return 0;
}

int KMainDlg::_RefreshProcSercurityLevel(int nCurSel, KPerfMonListItemData ItemData)
{
	CStringA strPos = _T("270,22,370,34");
	if (nCurSel == CListBoxItemData::GetDataPtr()->GetMaxHeightSel())
		strPos = _T("270,10,370,22");

	m_pPerfMonitorListBox->SetItemAttribute(IDC_TXT_PROCESS_SECURITY_LEVEL, "pos", strPos);
	m_pPerfMonitorListBox->SetItemText(IDC_TXT_PROCESS_SECURITY_LEVEL, ItemData.GetTrust());
	m_pPerfMonitorListBox->SetItemColorAttribute(IDC_TXT_PROCESS_SECURITY_LEVEL, "crtext", ItemData.GetLevelColor());

	return 0;
}

int KMainDlg::_RefreshProcCPUUsage(int nCurSel, KPerfMonListItemData ItemData)
{
	CStringA strPos = _T("370,22,470,34");
	if (nCurSel == CListBoxItemData::GetDataPtr()->GetMaxHeightSel())
		strPos = _T("370,10,470,22");
	m_pPerfMonitorListBox->SetItemAttribute(IDC_TXT_PROCESS_CPU_USAGE, "pos", strPos);
	m_pPerfMonitorListBox->SetItemText(IDC_TXT_PROCESS_CPU_USAGE, ItemData.GetProcessCpuUsage());
	return 0;
}

int KMainDlg::_RefreshProcMemUsage(int nCurSel, KPerfMonListItemData ItemData)
{
	CStringA strPos = _T("470,22,570,34");
	if (nCurSel == CListBoxItemData::GetDataPtr()->GetMaxHeightSel())
		strPos = _T("470,10,570,22");
	m_pPerfMonitorListBox->SetItemAttribute(IDC_TXT_PROCESS_MEM_USAGE, "pos", strPos);
	m_pPerfMonitorListBox->SetItemText(IDC_TXT_PROCESS_MEM_USAGE, ItemData.GetProcessMemUsage());
	return 0;
}

int KMainDlg::_RefreshProcICON(KPerfMonListItemData ItemData)
{
	char pszValue[MAX_PATH] = {0};
	m_pPerfMonitorListBox->SetItemAttribute(IDC_ICO_PROCESS_FILE_ICON, "iconhandle", "0");
	m_pPerfMonitorListBox->SetItemAttribute(IDC_ICO_PROCESS_FILE_ICON, "srcfile", "0");

	CString strFilePath;
	DWORD dwPID = ItemData.GetProcessId();
	if (m_bIsWin64)
	{
		//		m_wow64Swithcer.Close();
		strFilePath = KX64ProcPathQuery::Instance().GetProcessFullPathByPid(dwPID);
	}
	else
		strFilePath = ItemData.GetProcessFullPath();

	if (TRUE == PathFileExists(strFilePath))
	{
		HICON hIcon = CListBoxItemData::GetDataPtr()->GetIcon(strFilePath);
		if (hIcon != NULL)
		{
			_snprintf_s(pszValue, sizeof(pszValue), "%d", hIcon);
			m_pPerfMonitorListBox->SetItemAttribute(IDC_ICO_PROCESS_FILE_ICON, "iconhandle", pszValue);
		}
		else
		{
			_snprintf_s(pszValue, sizeof(pszValue), "%d", CListBoxItemData::GetDataPtr()->GetDefaultIcon());
			m_pPerfMonitorListBox->SetItemAttribute(IDC_ICO_PROCESS_FILE_ICON, "iconhandle", pszValue);
		}
	}
	else
	{
		_snprintf_s(pszValue, sizeof(pszValue), "%d", CListBoxItemData::GetDataPtr()->GetDefaultIcon());
		m_pPerfMonitorListBox->SetItemAttribute(IDC_ICO_PROCESS_FILE_ICON, "iconhandle", pszValue);
	}

	if (m_bIsWin64)
	{
		//		m_wow64Swithcer.Revert();
	}

	return 0;
}

int KMainDlg::_RefreshProcID(KPerfMonListItemData ItemData)
{
	CString  strProcID		= L"";

	CString strFilePath;
	DWORD dwPID = ItemData.GetProcessId();

	if (m_bIsWin64)
	{
		m_wow64Swithcer.Close();
		strFilePath = KX64ProcPathQuery::Instance().GetProcessFullPathByPid(dwPID);
	}
	else
		strFilePath = ItemData.GetProcessFullPath();

	if (0 != ItemData.GetProcessId())
	{
		strProcID.Format(BkString::Get(PERF_MONITOR_STR_511) , ItemData.GetProcessId());
		m_pPerfMonitorListBox->SetItemText(IDC_TXT_PROCESS_PID, strProcID);
		if (TRUE == PathFileExists(strFilePath))
			m_pPerfMonitorListBox->SetItemAttribute(IDC_TXT_PROCESS_PID, "pos", "50,22");
		else
			m_pPerfMonitorListBox->SetItemAttribute(IDC_TXT_PROCESS_PID, "pos", "50,0");
	}
	else
	{
		strProcID.Format(BkString::Get(PERF_MONITOR_STR_511) , 0);
		m_pPerfMonitorListBox->SetItemText(IDC_TXT_PROCESS_PID, strProcID);
		if (TRUE == PathFileExists(strFilePath))
			m_pPerfMonitorListBox->SetItemAttribute(IDC_TXT_PROCESS_PID, "pos", "50,22");
		else
			m_pPerfMonitorListBox->SetItemAttribute(IDC_TXT_PROCESS_PID, "pos", "50,0");
	}

	if (m_bIsWin64)
	{
		m_wow64Swithcer.Revert();
	}

	return 0;
}

int KMainDlg::_RefreshProcDiskIO(int nCurSel, KPerfMonListItemData ItemData)
{
	CStringA strPos = _T("570,22,670,34");
	if (nCurSel == CListBoxItemData::GetDataPtr()->GetMaxHeightSel())
		strPos = _T("570,10,670,22");
	m_pPerfMonitorListBox->SetItemAttribute(IDC_TXT_PROCESS_DISK_IO, "pos", strPos);
	m_pPerfMonitorListBox->SetItemText(IDC_TXT_PROCESS_DISK_IO, ItemData.GetProcessIOSpeed());
	return 0;
}

int KMainDlg::_RefreshProcPath(int nCurSel, KPerfMonListItemData ItemData)
{
	CString  strFilePathShow = L"";
	CDC dcx					= ::GetDC(GetViewHWND());
	HFONT hFntTmp			= NULL;
	CStringA strPos			= "";
	CRect	 rcSize(0,0,0,0);
	//显示路径
	m_pPerfMonitorListBox->SetItemVisible(IDC_TXT_PROCESS_FILE_PATH, FALSE);
	CListBoxItemData::GetDataPtr()->GetLogModule().WriteLog(
		L"RefreshProcPath, Name = %s",
		ItemData.GetProcessName()
		);

	CString strFilePath;
	DWORD dwPID = ItemData.GetProcessId();
	if (m_bIsWin64)
	{
		m_wow64Swithcer.Close();
		strFilePath = KX64ProcPathQuery::Instance().GetProcessFullPathByPid(dwPID);
	}
	else
		strFilePath = ItemData.GetProcessFullPath();

	if (CListBoxItemData::GetDataPtr()->GetMaxHeightSel() == nCurSel && 
		TRUE == PathFileExists(strFilePath))
	{
		m_pPerfMonitorListBox->SetItemVisible(IDC_TXT_PROCESS_FILE_PATH, TRUE);
		m_pPerfMonitorListBox->SetItemVisible(IDC_LNK_PROCESS_OPEN_DIR, TRUE);

		strFilePathShow.Format(BkString::Get(PERF_MONITOR_STR_508), strFilePath);
		dcx.DrawText( strFilePathShow, -1, &rcSize, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
		rcSize.OffsetRect(50, 0);
		int nOffsetLeft = 636;
		m_pPerfMonitorListBox->SetItemAttribute(IDC_TXT_PROCESS_FILE_PATH, "class", "filename");
		if (rcSize.right >= nOffsetLeft)
		{
			rcSize.right = nOffsetLeft;
		}
		//程序路径
		strPos.Format("%d,0,%d,12", rcSize.left, rcSize.right);
		m_pPerfMonitorListBox->SetItemAttribute(IDC_TXT_PROCESS_FILE_PATH, "pos", strPos);
		m_pPerfMonitorListBox->SetItemText(IDC_TXT_PROCESS_FILE_PATH, strFilePathShow);
		m_pPerfMonitorListBox->SetItemStringAttribute(IDC_TXT_PROCESS_FILE_PATH, "tip", strFilePathShow);

		//查看路径
		strPos.Format("%d,0,%d,12", rcSize.right+20, rcSize.right + 92);
		m_pPerfMonitorListBox->SetItemAttribute(IDC_LNK_PROCESS_OPEN_DIR, "pos", strPos);
		m_pPerfMonitorListBox->SetItemText(IDC_LNK_PROCESS_OPEN_DIR, BkString::Get(PERF_MONITOR_STR_501));
	}
	else
	{
		m_pPerfMonitorListBox->SetItemVisible(IDC_TXT_PROCESS_FILE_PATH, FALSE);
		m_pPerfMonitorListBox->SetItemVisible(IDC_LNK_PROCESS_OPEN_DIR, FALSE);
	}

	if (m_bIsWin64)
	{
		m_wow64Swithcer.Revert();
	}
	return 0;
}

int	KMainDlg::_RefreshProcButton(int nCurSel, KPerfMonListItemData ItemData)
{
	CStringA strPos1 = _T("680,22,728,34");
	CStringA strPos2 = _T("745,22");

	if (nCurSel == CListBoxItemData::GetDataPtr()->GetMaxHeightSel())
	{
		strPos1 = _T("680,10,728,22");
		strPos2 = _T("745,10");
	}
	m_pPerfMonitorListBox->SetItemAttribute(IDC_LNK_CLOSE_PROCESS, "pos", strPos1);
	m_pPerfMonitorListBox->SetItemAttribute(IDC_IMGBTN_PERFMON_MENU, "pos", strPos2);
	return 0;
}

void KMainDlg::_SetColumState(int nColum)
{
	if (nColum < 0 || nColum >= CListBoxItemData::GetDataPtr()->GetStateArray().GetCount())
		return;

	m_nCurColum = nColum;
	//set colum state
	CListBoxItemData::GetDataPtr()->GetStateArray()[nColum] += 1;
	if (CListBoxItemData::GetDataPtr()->GetStateArray()[nColum] > 1)
		CListBoxItemData::GetDataPtr()->GetStateArray()[nColum] = 0;

	//get img pos
	int nLeft = 0;
	for (int i = 0; i <= nColum; i++)
	{
		nLeft += m_arrColumWidth[i];
	}
	//reset colum state
	for (int i = 0; i < CListBoxItemData::GetDataPtr()->GetStateArray().GetCount(); i++)
	{
		if (nColum != i)
			CListBoxItemData::GetDataPtr()->GetStateArray()[i] = -1;
	}
	//
	SetItemVisible(IDC_IMG_COLUM_LISTSORT, TRUE);
	CStringA strPos = "";
	strPos.Format("%d,5,%d,-0", nLeft-12, nLeft);
	SetItemAttribute(IDC_IMG_COLUM_LISTSORT, "pos", strPos);
	SetItemIntAttribute(IDC_IMG_COLUM_LISTSORT, "sub", CListBoxItemData::GetDataPtr()->GetStateArray()[nColum]);
}

void KMainDlg::OnBkClickListSortImg()
{
	_SetColumState(m_nCurColum);
	_SortListData(m_nCurColum);
}

void KMainDlg::OnClickPerfOptMenuBtn()
{
	POINT pt;
	GetCursorPos(&pt);

	SetForegroundWindow(m_hWnd);
	TrackPopupMenu(m_hPerfMonMenu,
		TPM_RIGHTBUTTON,
		pt.x,
		pt.y,
		0,
		m_hWnd,
		NULL
		);
}

void KMainDlg::OnClickOpenDir(KPerfMonListItemData ItemData)
{
	ItemData.OpenDirector();
}

void KMainDlg::OnClickKillProcess(KPerfMonListItemData ItemData)
{
	if (ItemData.KillCurrentSelProcess() == TRUE)
	{
		OnTimer(ID_TIMER_UPDATE_PROCPERF_MON);
	}
}

void KMainDlg::OnClickColumTitle()
{
	_SetColumState(0);
	_SortListData(KPerfDataSorter::enum_Sort_Name);
	_RefreshListBoxData(m_processInfoList);
}

void KMainDlg::OnClickColumLevel()
{
	_SetColumState(1);
	_SortListData(KPerfDataSorter::enum_Sort_SecLevel);
	_RefreshListBoxData(m_processInfoList);
}

void KMainDlg::OnClickColumCPU()
{
	_SetColumState(2);
	_SortListData(KPerfDataSorter::enum_Sort_CPU);
	_RefreshListBoxData(m_processInfoList);
}

void KMainDlg::OnClickColumMEM()
{
	_SetColumState(3);
	_SortListData(KPerfDataSorter::enum_Sort_Mem);
	_RefreshListBoxData(m_processInfoList);
}

void KMainDlg::OnClickColumIO()
{
	_SetColumState(4);
	_SortListData(KPerfDataSorter::enum_Sort_IO);
	_RefreshListBoxData(m_processInfoList);
}

//////////////////////////////////////////////////////////////////////////

int KMainDlg::_PushUIGroup(DWORD dwGroupID, UI_Group uiGroup)
{
	int nCount = 0;
	m_mapUI[dwGroupID]	= uiGroup;

	return m_mapUI.GetCount();
}

int KMainDlg::_SwitchUI(DWORD dwGroupID)
{
	int nRet = -1;

	POSITION pos = m_mapUI.GetStartPosition();

	BOOL bFlag = FALSE;
	while(NULL != pos)
	{
		UI_Group ui = m_mapUI.GetValueAt(pos);

		bFlag = FALSE;
		if (dwGroupID == ui.dwGroupID)
		{
			bFlag = TRUE;
		}

		//visable
		for (int i = 0; i < ui.arrCtrlID.GetSize(); i++)
		{
			SetItemVisible(ui.arrCtrlID[i], bFlag);
		}

		m_mapUI.GetNext(pos);
	}

	return nRet;
}

void KMainDlg::InitUI()
{
	UI_Group ui;
	ui.dwGroupID = 0;
	ui.arrCtrlID.Add(PERF_MONITOR_CTRL_ID_801);
	ui.arrCtrlID.Add(PERF_MONITOR_CTRL_ID_802);
	ui.arrCtrlID.Add(PERF_MONITOR_CTRL_ID_803);
	_PushUIGroup(0, ui);

	ui.Reset();
	ui.dwGroupID = 1;
	ui.arrCtrlID.Add(PERF_MONITOR_CTRL_ID_804);
	ui.arrCtrlID.Add(PERF_MONITOR_CTRL_ID_805);
	ui.arrCtrlID.Add(PERF_MONITOR_CTRL_ID_810);
	ui.arrCtrlID.Add(PERF_MONITOR_CTRL_ID_811);
	_PushUIGroup(1, ui);
}

void KMainDlg::SwitchLinkText(DWORD dwTextID)
{
	LPCSTR pszLink1 = "linktext",
		   pszLink2 = "linktextclicked";
	DWORD   dwID[] = {PERF_MONITOR_CTRL_ID_807,
					  PERF_MONITOR_CTRL_ID_808};

	for (int i = 0; i < sizeof(dwID)/sizeof(DWORD); i++)
	{
		if (dwTextID == dwID[i])
			SetItemAttribute(dwID[i], "class", pszLink2);
		else
			SetItemAttribute(dwID[i], "class", pszLink1);
	}
}

void KMainDlg::OnLinkWndProc()
{
	SwitchLinkText(PERF_MONITOR_CTRL_ID_808);

	KillTimer(ID_TIMER_UPDATE_PROCPERF_MON);
	_SwitchUI(1);
}

void KMainDlg::OnLinkAllProc()
{
	SwitchLinkText(PERF_MONITOR_CTRL_ID_807);

	SetTimer(ID_TIMER_UPDATE_PROCPERF_MON, UPDATE_PROCPERF_MON_INTERVAL, NULL);
	_SwitchUI(0);
}

void KMainDlg::_InitWndProcList()
{
	m_pWndProcList = new CBkNetMonitorListBox;
	if (NULL == m_pWndProcList)
		return;

	//ListBox
	m_pWndProcList->Create( GetViewHWND(), IDC_LIST_WNDPROC_APP);
	m_pWndProcList->Load(IDR_BK_LISTBOX_WNDPROC_MONITOR);
	m_pWndProcList->SetCanGetFocus(FALSE);
	/*
	 * Bug备注：
	 * 如果List高度是不可变的，就需要设置每一项的固定高度。	否则在滚动条出现或消失时，List会变黑。也不需要响应
	 * BKLBM_CALC_MAX_HEIGHT和BKLBM_CALC_ITEM_HEIGHT消息。
	 * 如果List高度是可变的，就需要响应BKLBM_CALC_MAX_HEIGHT和BKLBM_CALC_ITEM_HEIGHT消息。
	 */
	m_pWndProcList->SetItemFixHeight(HEIGHT_ITEM_WNDPROC_LIST);
	_RefreshAppWndListData(m_vecWndListItemData);
	//Menu
	m_hAppWndMenu = GetSubMenu(AtlLoadMenu(MAKEINTRESOURCE(IDR_MENU_APP_WNDS)), 0);
}

LRESULT	KMainDlg::OnWndProcListBoxGetDispInfo(LPNMHDR pnmh)
{
	HRESULT hRet = S_OK;

	if (m_pWndProcList == NULL)
		return E_POINTER;

	BKLBMGETDISPINFO* pdi = (BKLBMGETDISPINFO*)pnmh;

	if (pdi->nListItemID >= m_vecWndListItemData.size())
		return E_FAIL;

	KAppWndListItemData ItemData = m_vecWndListItemData[pdi->nListItemID];

	_RefreshAppWndListItemIcon(ItemData);

	_RefreshAppWndListItemWndTitle(ItemData);

	_RefreshAppWndListItemProcIcon(ItemData);

	_RefreshAppWndListItemProcExe(ItemData);

	_RefreshAppWndListItemWndState(ItemData);

	_RefreshAppWndListItemButton(ItemData);

	m_pWndProcList->SetItemAttribute(IDC_WNDSTATE_LIST_ITEM, "crbg", pdi->bSelect ? "ECF9FF":"FFFFFF");

	return S_OK;
}

LRESULT	KMainDlg::OnWndProcListBoxClickCtrl(LPNMHDR pnmh)
{
	HRESULT hRet = S_OK;

	LPBKLBMITEMCLICK pnms = (LPBKLBMITEMCLICK)pnmh;
	if (pnms->nListItemID >= m_pWndProcList->GetCount())
		return E_FAIL;
	if (pnms->nListItemID >= m_vecWndListItemData.size())
		return E_FAIL;

	KAppWndListItemData ItemData = m_vecWndListItemData[pnms->nListItemID];
	CListBoxItemData::GetDataPtr()->SetCurSelAppWndData(ItemData);

	if (pnms->uCmdID == IDC_IMGBTN_WND_OPERATES)
	{
		OnClickWndProcMenuBtn();
		return S_OK;
	}
	if (pnms->uCmdID == IDC_LNK_CLOSE_WINDOW)
	{
		_CloseWindows(ItemData);
		return S_OK;
	}

	return S_OK;
}

LRESULT	KMainDlg::OnWndProcListBoxRClickCtrl(LPNMHDR pnmh)
{
	LPBKRBMITEMCLICK pnms = (LPBKRBMITEMCLICK)pnmh;
	if (pnms->nListItemID >= m_pWndProcList->GetCount())
		return E_FAIL;
	if (pnms->nListItemID >= m_vecWndListItemData.size())
		return E_FAIL;

	KAppWndListItemData ItemData = m_vecWndListItemData[pnms->nListItemID];
	CListBoxItemData::GetDataPtr()->SetCurSelAppWndData(ItemData);

	OnClickWndProcMenuBtn();
	return S_OK;
}

void KMainDlg::NotifyMouseWhellToListBox(CBkListBox* pList, UINT nFlags, short zDelta, CPoint pt)
{
	if (NULL == pList)
		return;

	CRect rcWin;
	if ( pList && pList->IsWindow() && pList->IsWindowVisible() )
	{
		pList->GetWindowRect(&rcWin);

		if (rcWin.PtInRect(pt))
			SendMessage(pList->m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
	}
}

void KMainDlg::UpDateOneKeyFreeMemHistory()
{
	IniFileOperate::CIniFile iniFile(m_strCfgPath);
	ULONGLONG uTime = 0, uByte = 0, uNowTime = 0;
	TCHAR pszTime[50] = {0}, pszByte[50] = {0};
	iniFile.GetStrValue(L"FreeMem", L"LastFreeSize", pszByte, sizeof(pszByte));
	iniFile.GetStrValue(L"FreeMem", L"FreeTime", pszTime, sizeof(pszTime));

	uTime = _wtoi64_l(pszTime, 0);
	uByte = _wtoi64_l(pszByte, 0);
	
	uNowTime = _time64(NULL);
	CTimeSpan spanTime(uNowTime - uTime);
	
	CString strValue = L"", strMem = L"";

	NetFlowToString(uByte, strMem);
	if (0 == uTime || 0 == uByte)
	{//以前没有做过优化
		strValue = BkString::Get(PERF_MONITOR_STR_518);
	}else if (spanTime.GetTotalMinutes() <= 30)
	{//30分钟之内使用了系统优化
		strValue.Format(BkString::Get(PERF_MONITOR_STR_522), strMem);
	}
	else
	{
		int nDays = spanTime.GetDays(), nHours = spanTime.GetTotalHours(), nMins = spanTime.GetTotalMinutes();
		if (nDays >= 1)
		{//N天前
			strValue.Format(BkString::Get(PERF_MONITOR_STR_519), nDays, strMem);
		}else if (nHours >= 1)
		{//N小时前
			strValue.Format(BkString::Get(PERF_MONITOR_STR_520), nHours, strMem);
		}else if (nMins >= 30)
		{//N分钟前
			strValue.Format(BkString::Get(PERF_MONITOR_STR_521), nMins, strMem);
		}
	}

	if (FALSE == strValue.IsEmpty())
	{
		SetRichText(PERF_MONITOR_CTRL_ID_813, strValue);
		SetRichText(PERF_MONITOR_CTRL_ID_814, strValue);
	}
}

BOOL KMainDlg::IsNeedCheckWnd()
{
	BOOL bRet = FALSE;
	int nValue = 0;
	IniFileOperate::CIniFile iniFile(m_strCfgPath);
	iniFile.GetIntValue( TEXT("perfopt"), TEXT("check"), nValue );

	return nValue != 0 ? TRUE : FALSE;
}

BOOL KMainDlg::IsLongTimeUse()
{
	ULONGLONG uHistoryByte = 0;
	TCHAR pszByte[50] = {0};

	IniFileOperate::CIniFile iniFile(m_strCfgPath);
	iniFile.GetStrValue(L"FreeMem", L"FreeTime", pszByte, sizeof(pszByte));
	uHistoryByte = _wtoi64_l(pszByte, 0);
	__time64_t t_Now = _time64(NULL);
	if( t_Now - uHistoryByte > 5 * 60 )
	{
		return TRUE;
	}

	return FALSE;
}

void KMainDlg::OnOneKeySpeedUp()
{
	KPerfMonListItemData data;

	if( IsNeedCheckWnd() )
	{
		if( HasWndNeedClose() )
		{
			if (!m_bWinOptDlgHasDoModal)
			{
				m_bWinOptDlgHasDoModal = TRUE;
				CBKSafeWndOptdlg wndOptDlg(this);
				wndOptDlg.UpdateWndOptInfo( m_vecWndListItemData );
				wndOptDlg.Load( IDR_BK_WND_OPT_DLG );
				wndOptDlg.DoModal();

				UpDateOneKeyFreeMemHistory();
			}
		}
		else
		{
			if (!m_bPerfOptDlgIsDoModal)
			{
				m_bPerfOptDlgIsDoModal = TRUE;
				CBKSafeSysPerfOpt PerfOpt(this);
				PerfOpt.Load(IDR_BK_SYSPERF_DLG);
				PerfOpt.DoModal();

				UpDateOneKeyFreeMemHistory();
			}
		}
	}
	else
	{
		if( IsLongTimeUse() )
		{
			if (!m_bPerfOptDlgIsDoModal)
			{
				m_bPerfOptDlgIsDoModal = TRUE;
				CBKSafeSysPerfOpt PerfOpt(this);
				PerfOpt.Load(IDR_BK_SYSPERF_DLG);
				PerfOpt.DoModal();

				UpDateOneKeyFreeMemHistory();
			}
		}
		else
		{
			CBkSafeMsgBox2 msgBox;
			msgBox.ShowMutlLineMsg( BkString::Get(STR_MSG_NO_OPT), BkString::Get(7), MB_OK | MB_ICONEXCLAMATION );
		}
	}
}

void KMainDlg::SetPerfOptDlgHasDoModal(BOOL bDoModal)
{
	m_bPerfOptDlgIsDoModal = bDoModal;
}

void KMainDlg::SetWinOptDlgHasDoModal(BOOL bDoModal)
{
	m_bWinOptDlgHasDoModal = bDoModal;
}

void KMainDlg::OnChangeSetting()
{
	CString strInstallPath;
	CAppPath::Instance().GetLeidianAppPath(strInstallPath);
	strInstallPath += L"\\KSafe.exe";
	ShellExecute(NULL, _T("open"), strInstallPath, L"-do:UI_Setting_SysOpt", NULL, SW_SHOW);
}

BOOL KMainDlg::HasWndNeedClose()
{
	UpDateAppWndListData();

	for( int i = 0; i < m_vecWndListItemData.size(); i++ )
	{
		KAppWndListItemData& item = m_vecWndListItemData[i];
		if( item.GetItemState() == 1 || 
			( item.GetItemState() == 3 && !IsDownLoadTitle( item.GetItemExePath() ) ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

int KMainDlg::UpDateAppWndListData()
{
	if(FAILED(m_appWndStateMgr.Init()))
		return -1;
	m_vecAppWndStateData.clear();
	m_vecWndListItemData.clear();
	m_appWndStateMgr.UpdateAppWndState();
	m_appWndStateMgr.GetAppWndStateData(m_vecAppWndStateData);
	m_vecWndTimeInfo.clear();
	KWndTimeCacheReader cacheReader;
	if (SUCCEEDED(cacheReader.Init()))
		cacheReader.GetWndTimeInfo(&m_vecWndTimeInfo);

	std::vector<LPAPP_WND_STATE_ITEM>::iterator it;
	for (it = m_vecAppWndStateData.begin(); it != m_vecAppWndStateData.end(); it++)
	{
		LPAPP_WND_STATE_ITEM pItem = *it;
		KAppWndListItemData ItemData(pItem);
		if (pItem->hWnd == this->m_hWnd)
			continue;

		map<DWORD, KProcessPerfData*>::iterator item = m_mapPerfData.find(pItem->dwPID);
		if (item == m_mapPerfData.end())
			return -1;
		KProcessPerfData* pPerfData = m_mapPerfData[pItem->dwPID];
		CString strExePath = pPerfData->GetProcessPath();
		CString strExeName = PathFindFileName(strExePath);
		if (strExeName.CompareNoCase(_T("dwm.exe")) == 0)
			continue;

		//if (strExeName.CompareNoCase(_T("explorer.exe")) == 0)
		//	continue;

		ItemData.SetItemExePath(strExePath);

/*		m_hActiveWnd = GetForegroundWindow();*/
		
		HWND hCurWnd = ItemData.GetItemHwnd();

		SetNotActiveTime(hCurWnd);
		ItemData.SetItemNotActiveTime(m_mapNotActiveTime[hCurWnd]);
		m_vecWndListItemData.push_back(ItemData);
	}

	return m_vecAppWndStateData.size();
}


void KMainDlg::_RefreshAppWndListData(vector<KAppWndListItemData>& vecListData)
{
	CAutoLocker locker(m_locker);
	//更新列表
	if (m_pWndProcList->GetCount() != vecListData.size())
	{
		m_pWndProcList->SetItemCount(vecListData.size());
		m_pWndProcList->RefereshVScrollEx(HEIGHT_ITEM_WNDPROC_LIST);
	}
	else
	{
		m_pWndProcList->ForceRefresh();
		m_pWndProcList->ResetMouseMoveMsg();
	}
}

int KMainDlg::_RefreshAppWndListItemIcon(KAppWndListItemData ItemData)
{
	char pszValue[MAX_PATH] = {0};
	m_pWndProcList->SetItemAttribute(IDC_ICO_WINDOW_TITLE, "iconhandle", "0");
	m_pWndProcList->SetItemAttribute(IDC_ICO_WINDOW_TITLE, "srcfile", "0");

	HICON hIcon = CListBoxItemData::GetDataPtr()->GetWndIcon(ItemData);
	if (hIcon != NULL)
	{
		_snprintf_s(pszValue, sizeof(pszValue), "%d", hIcon);
		m_pWndProcList->SetItemAttribute(IDC_ICO_WINDOW_TITLE, "iconhandle", pszValue);
	}
	else
	{
		CString strFilePath = ItemData.GetItemExePath();
		char pszValue[MAX_PATH] = {0};
		m_pWndProcList->SetItemAttribute(IDC_ICO_WINDOW_TITLE, "iconhandle", "0");
		m_pWndProcList->SetItemAttribute(IDC_ICO_WINDOW_TITLE, "srcfile", "0");

		HICON hIcon = CListBoxItemData::GetDataPtr()->GetSmallIcon(strFilePath);
		if (hIcon != NULL)
		{
			_snprintf_s(pszValue, sizeof(pszValue), "%d", hIcon);
			m_pWndProcList->SetItemAttribute(IDC_ICO_WINDOW_TITLE, "iconhandle", pszValue);
		}
		else
		{
			_snprintf_s(pszValue, sizeof(pszValue), "%d", CListBoxItemData::GetDataPtr()->GetDefaultIcon());
			m_pWndProcList->SetItemAttribute(IDC_ICO_WINDOW_TITLE, "iconhandle", pszValue);
		}
	}

	return 0;
}

int KMainDlg::_RefreshAppWndListItemWndTitle(KAppWndListItemData ItemData)
{
	m_pWndProcList->SetItemText(IDC_TXT_WINDOW_TITLE, ItemData.GetItemWndTitle());
	m_pWndProcList->SetItemStringAttribute(IDC_TXT_WINDOW_TITLE, "tip", ItemData.GetItemWndTitle());
	return 0;
}

int KMainDlg::_RefreshAppWndListItemProcIcon(KAppWndListItemData ItemData)
{
	CString strFilePath = ItemData.GetItemExePath();
	char pszValue[MAX_PATH] = {0};
	m_pWndProcList->SetItemAttribute(IDC_ICO_WINDOW_PROCESS, "iconhandle", "0");
	m_pWndProcList->SetItemAttribute(IDC_ICO_WINDOW_PROCESS, "srcfile", "0");

	HICON hIcon = CListBoxItemData::GetDataPtr()->GetSmallIcon(strFilePath);
	if (hIcon != NULL)
	{
		_snprintf_s(pszValue, sizeof(pszValue), "%d", hIcon);
		m_pWndProcList->SetItemAttribute(IDC_ICO_WINDOW_PROCESS, "iconhandle", pszValue);
	}
	else
	{
		_snprintf_s(pszValue, sizeof(pszValue), "%d", CListBoxItemData::GetDataPtr()->GetDefaultIcon());
		m_pWndProcList->SetItemAttribute(IDC_ICO_WINDOW_PROCESS, "iconhandle", pszValue);
	}

	return 0;
}

int KMainDlg::_RefreshAppWndListItemProcExe(KAppWndListItemData ItemData)
{
	CString strCaption;
	CString strFilePath = ItemData.GetItemExePath();
	CString strName = PathFindFileName(strFilePath);
	KSProcessInfo	ProcInfo;
	if ((strFilePath.GetLength() > 0) && 
		(0 == CListBoxItemData::GetDataPtr()->QueryProcInfo(strFilePath, ProcInfo)) && 
		ProcInfo.strDisplayName.length() > 0)
	{
		strCaption = ProcInfo.strDisplayName.c_str();
	}

	CString strShow;
	if (strCaption.IsEmpty())
	{
		strShow = strName;
	}
	else
	{
		strShow.Format(_T("%s（%s）"), strCaption, strName);
	}
	
	m_pWndProcList->SetItemText(IDC_TXT_WINDOW_PROCESS_EXENAME, strShow);
	m_pWndProcList->SetItemStringAttribute(IDC_TXT_WINDOW_PROCESS_EXENAME, "tip", strShow);
	return 0;
}

int KMainDlg::_RefreshAppWndListItemWndState(KAppWndListItemData ItemData)
{
	m_pWndProcList->SetItemText(IDC_TXT_WINDOW_STATE, ItemData.GetItemStateStr());
	m_pWndProcList->SetItemColorAttribute(IDC_TXT_WINDOW_STATE, "crtext", ItemData.GetItemStateColor());
	return 0;
}

int KMainDlg::_RefreshAppWndListItemButton(KAppWndListItemData ItemData)
{
	return 0;
}

void KMainDlg::OnClickWndProcMenuBtn()
{
	POINT pt;
	GetCursorPos(&pt);

	SetForegroundWindow(m_hWnd);
	TrackPopupMenu(m_hAppWndMenu,
		TPM_RIGHTBUTTON,
		pt.x,
		pt.y,
		0,
		m_hWnd,
		NULL
		);
}

DWORD WINAPI KMainDlg::pfnThreadProcCloseWnd(__in LPVOID lpParameter)
{
	HWND hWnd = (HWND)lpParameter;
	DWORD dwTimeOut = 0;

	::PostMessage(hWnd, WM_CLOSE, 0, 0);
	while(TRUE)
	{
		BOOL bWndExist = ::IsWindow(hWnd) && ::IsWindowVisible(hWnd);
		if (!bWndExist)
			break;
		Sleep(100);
		dwTimeOut++;
		if (dwTimeOut >= 10)
			break;
	}
	return 0;
}

CString KMainDlg::FormatLongExeName(const CString& strIn)
{
	DWORD dwLength = strIn.GetLength();
	if (dwLength > 61)
	{
		CString strOut = strIn.Left(61);
		strOut.Append(_T("..."));
		return strOut;
	}
	else
		return strIn;
}

void KMainDlg::_CloseWindows(KAppWndListItemData ItemData)
{
	CString strName = FormatLongExeName(PathFindFileName(ItemData.GetItemExePath()));
	CBkSafeMsgBox2 msgBox;
	CString strInfo;
	DWORD dwType;
	if (ItemData.GetItemState() == 1)//已卡死的窗口
	{
		strInfo.Format(BkString::Get(DefString54), strName);
		dwType = MB_YESNO | MB_ICONEXCLAMATION;
		if (msgBox.ShowMutlLineMsg(strInfo, BkString::Get(7), dwType) == IDYES)
		{
			_KillProcess2(ItemData.GetItemPID());
		}
	}
	else
	{
		HANDLE hThread = CreateThread(NULL, 0, pfnThreadProcCloseWnd, ItemData.GetItemHwnd(), 0, NULL);
		WaitForSingleObject(hThread, 1000);
		CloseHandle(hThread);
		hThread = NULL;
		BOOL bWndExist = ::IsWindow(ItemData.GetItemHwnd()) && ::IsWindowVisible(ItemData.GetItemHwnd());

		if (bWndExist)
		{
			strInfo.Format(BkString::Get(DefString55), strName);
			dwType = MB_YESNO | MB_ICONEXCLAMATION;
			if (msgBox.ShowMutlLineMsg(strInfo, BkString::Get(7), dwType) == IDYES)
			{
				_KillProcess2(ItemData.GetItemPID());
			}
		}
	}
}

BOOL KMainDlg::_SwitchToWindow(HWND hWnd)
{
	typedef void (WINAPI *PROCSWITCHTOTHISWINDOW) (HWND, BOOL);
	PROCSWITCHTOTHISWINDOW SwitchToThisWindow;

	HMODULE hUser32 = GetModuleHandleW(L"USER32");
	SwitchToThisWindow = (PROCSWITCHTOTHISWINDOW)GetProcAddress(hUser32, "SwitchToThisWindow");
	if (SwitchToThisWindow) 
	{
		SwitchToThisWindow(hWnd, TRUE);
	}
	else 
	{
		if (::IsIconic(hWnd))
			::ShowWindow(hWnd, SW_RESTORE);
		::BringWindowToTop(hWnd);
		SetForegroundWindow(hWnd);
	}
	/*::ShowWindow(m_hWnd, SW_MINIMIZE);*/
	return TRUE;
}

LRESULT KMainDlg::OnAppWndCommand(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	KAppWndListItemData ItemData = CListBoxItemData::GetDataPtr()->GetCurSelAppWndData();
	switch(wParam)
	{
	case ID_APPWND_CLOSEWND:
		_CloseWindows(ItemData);
		break;
	case ID_APPWND_CLOSEEXE:
		_KillProcess(ItemData.GetItemPID());
		break;
	case ID_APPWND_OPENWND:
		_SwitchToWindow(ItemData.GetItemHwnd());
		break;
	default:
		break;
	}
	SetMsgHandled(FALSE);
	return 0;
}

BOOL KMainDlg::_KillProcess(DWORD dwPID)
{
	map<DWORD, KProcessPerfData*>::iterator item = m_mapPerfData.find( dwPID );
	if (item == m_mapPerfData.end())
		return FALSE;
	KProcessPerfData* pPerfData = m_mapPerfData[dwPID];
	KPerfMonListItemData ItemData(pPerfData);
	return ItemData.KillCurrentSelProcess();
}

BOOL KMainDlg::_KillProcess2(DWORD dwPID)
{
	KPerfMonListItemData ItemData;
	return ItemData._KillProcess(dwPID);
}
