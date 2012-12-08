#pragma once
#include "bkres/bkres.h"
//#include "treelistctrlex.h"
#include <vector>
#include "common/KCheckInstance.h"
#include <wtlhelper/whwindow.h>
using namespace std;
#include "bklistbox.h"
#include "common/bksafelog.h"
#include "resource.h"
#include "kprocesslib/interface.h"
#include "communits/Locker.h"
#include "kispublic/klocker.h"
#include <map>
#include "perfmonutility.h"
#include "perfmonlistitemdata.h"
#include "perfmon/kperfmon.h"
#include "runoptimize/processmon.h"
#include "runoptimize/x64helper.h"
#include "kmodulemgr.h"
#include "wndlayoutmgr.h"
using namespace Wnd_LayOut_Mgr;
#include <miniutil/fileversion.h>
#include "appwndbasedata.h"
#include "appwndlistitemdata.h"
#include "perfmon/wndtimedef.h"

#define ID_TIMER_UPDATE_PROCPERF_MON		2
#define ID_TIMER_UPDATE_FLOATWND_STATE		3
#define ID_TIMER_CHECK_EXIT					4
#define ID_TIMER_REFRESH_ONKEY_HISTOR		5
#define	ID_TIMER_REFRESH_WNDPROC_DATA		6
#define ID_TIMER_DO_ONEKEYSPEEDUP_AT_RUN    7
#define ID_TIMER_REFRESH_TOTAL_USAGE		8

#define UPDATE_PROCPERF_MON_INTERVAL		3000
#define UPDATA_FLOATWND_STATE				2000
#define CHECK_EXIT_EVENT_INTERVAL			500
#define UPDATE_WNDPROC_DATA_INTERVAL		1000
#define REFRESH_TOTAL_USAGE_INTERVAL		1000

#define		HEIGHT_ITEM_MAX				100
#define		HEIGHT_ITEM_MIN				55
#define		HEIGHT_ITEM_WNDPROC_LIST	32

#define		LISTBOX_COLUM_NAME			0
#define		LISTBOX_COLUM_LEVEL			1
#define		LISTBOX_COLUM_CPU			2
#define		LISTBOX_COLUM_MEM			3
#define		LISTBOX_COLUM_IO			4

#define		RUN_UI_ONEKEYSPEED			10

#define		LIST_SORT_ASC				1	//升序
#define		LIST_SORT_DESC				0	//降序

typedef std::pair<DWORD, KProcessPerfData*> PerfDataPair;

class KPerfDataSorter
{
public:
	enum{
		enum_Sort_Name = 0,
		enum_Sort_SecLevel,
		enum_Sort_CPU,
		enum_Sort_Mem,
		enum_Sort_IO
	};

	KPerfDataSorter(BOOL asc, DWORD dwSortKey = enum_Sort_CPU) 
		: m_bAsc(asc)
		, m_SortColum(dwSortKey) 
	{
	}

	bool operator()(const PerfDataPair& pLeft, const PerfDataPair& pRight) const
	{
		bool bResult = false;

		switch(m_SortColum)
		{
		case enum_Sort_Name:
			{
				CString str1 = pLeft.second->GetProcessName();
				CString str2 = pRight.second->GetProcessName();
				bResult = str1 > str2;
			}
			break;	
		case enum_Sort_SecLevel:
			{
				DWORD dwLevel1 = pLeft.second->GetProcessTrustMode();
				DWORD dwLevel2 = pRight.second->GetProcessTrustMode();
				bResult = dwLevel1 > dwLevel2;
			}
			break;
		case enum_Sort_CPU:
			{
				int v1 = pLeft.second->GetCpuUsage();
				int v2 = pRight.second->GetCpuUsage();
				bResult = v1 > v2;
			}
			break;
		case enum_Sort_Mem:
			{
				ULONGLONG u1 = pLeft.second->GetMemUsage();
				ULONGLONG u2 = pRight.second->GetMemUsage();
				bResult = u1 > u2;
			}
			break;
		case enum_Sort_IO:
			{
				ULONGLONG u1 = pLeft.second->GetIOSpeed();
				ULONGLONG u2 = pRight.second->GetIOSpeed();
				bResult = u1 > u2;
			}
			break;
		default:
			break;
		}

		bool bRet = m_bAsc ? bResult : !bResult;
		return bRet;
	}

private:
	BOOL		m_bAsc;
	DWORD		m_SortColum;
};

class KMainDlg
    : public CBkDialogImpl<KMainDlg>,
	public CWHRoundRectFrameHelper<KMainDlg>
{
public:
	enum enumUpLogType
	{
		enumUnknownProc = 0,
		enumOneKeyOpt	= 1,
		enumUnknown = -1
	};
public:
    KMainDlg(DWORD dwRunParamType)
        : CBkDialogImpl<KMainDlg>(IDR_BK_MAIN_DIALOG)
		, m_pPerfMonitorListBox(NULL)
		, m_bHideSystem(FALSE)
		, m_bFloatWndIsOpen(TRUE)
		, m_bIsWin64(FALSE)
		, m_pModuleMgr(NULL)
		, m_pWndProcList(NULL)
/*		, m_hActiveWnd(NULL)*/
		, m_hThreadReport(NULL)
		, m_hEventRptThreadExit(NULL)
		, m_bReportThreadWorking(FALSE)
		, m_bPerfOptDlgIsDoModal(FALSE)
		, m_bWinOptDlgHasDoModal(FALSE)
		/*, m_bSortAscOrDesc(FALSE)*/
    {
		m_hEventGetNetData = CreateEvent(NULL, FALSE, TRUE, NULL);
/*		m_nCurColum = LISTBOX_COLUM_MEM;*/
		m_arrColumWidth.SetCount(10);
		m_arrColumWidth[0] = 270;
		m_arrColumWidth[1] = 100;
		m_arrColumWidth[2] = 100;
		m_arrColumWidth[3] = 100;
		m_arrColumWidth[4] = 100;
		m_arrColumWidth[5] = 114;
		m_arrColumWidth[6] = 0;
		m_arrColumWidth[7] = 0;
		m_arrColumWidth[8] = 0;
		m_arrColumWidth[9] = 0;

		if(dwRunParamType == RUN_UI_ONEKEYSPEED)
		{
			m_nCurColum = LISTBOX_COLUM_MEM;
			m_bDoOnekeySpeedUpAtRun = TRUE;
		}
		else
		{
			m_nCurColum = dwRunParamType;
			m_bDoOnekeySpeedUpAtRun = FALSE;
		}

		m_mapReported.RemoveAll();
		m_mapNeedReportProc.RemoveAll();

		//  [2/26/2011 zhangbaoliang]
		CAppPath::Instance().GetLeidianCfgPath(m_strCfgPath, TRUE);
		m_strCfgPath.Append(L"\\bksafe.ini");
		TCHAR pszFile[1024] = {0};
		::GetModuleFileName(NULL, pszFile, sizeof(pszFile));
		GetFileVersion(pszFile, m_strFileVer);

    }

    ~KMainDlg();

	void			BtnClose(void);
	void			BtnMin(void);
	BOOL			OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/);
	void			OnSize(UINT nType, CSize size);
	void			OnSysCommand(UINT nID, CPoint pt);
	LRESULT			ShowUI(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	int				ReportPerfOptData();
	void			_ReportPerfOptData();
	int				GetCurSortSel(){return m_nCurColum;}

private:
	LRESULT			OnTimer(UINT timerId);
	LRESULT			OnDestroy();
	CLocker			m_locker;
   
public:
	BOOL			OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	static			bool SortPerfInfo(KProcessPerfData* ItemData1, KProcessPerfData* ItemData2, int nColum, int nState);
	//对应每一列的排序操作
//	static			int  Compare( KProcessPerfData* ItemData1, KProcessPerfData* ItemData2, int nsubIndex = -1);
//	static			bool stl_SortPerfInfo(PerfDataPair element1,PerfDataPair element2);
	void			_SortListData(int nColum);

	LRESULT			OnListBoxGetDispInfo(LPNMHDR pnmh);
	LRESULT			OnListBoxGetMaxHeight(LPNMHDR pnmh);
	LRESULT			OnListBoxGetItemHeight(LPNMHDR pnmh);
	LRESULT			OnListBoxClickCtrl(LPNMHDR pnmh);
	LRESULT			OnListBoxRClickCtrl(LPNMHDR pnmh);


	LRESULT			OnPerfMonCommand(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT			OnAppWndCommand(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void			OnClickPerfOptMenuBtn();
	void			OnClickOpenDir(KPerfMonListItemData ItemData);
	void			OnClickKillProcess(KPerfMonListItemData ItemData);

	void			OnClickColumTitle();
	void			OnClickColumLevel();
	void			OnClickColumCPU();
	void			OnClickColumMEM();
	void			OnClickColumIO();
	
	void			OnBkCheckHideSystemClick();
	void			OnClickShowOrHideTip();
	void			OnBkClickListSortImg();
	void			OnChangeSetting();

	void			_ShowFloatWnd( BOOL isShow );
	void			_HideOrShowTip(IN BOOL bIsShowTip, IN BOOL bIsNeedAct = TRUE);
	void			CloseSysPerfFloatWnd();
	void			OpenSysPerfFloatWnd();
	BOOL			GetFloatWndDisplayStatus();
	void			OpenFloatWnd();
	BOOL			IsSysPerfFloatWndVisable();
	BOOL			IsFloatWndStartWithTray();
	BOOL			_CheckDriverIsOk();
	void			_SetSysPerfFloatWndIniCfg(int nValue);
	
	CString			GetAppPath(){return m_strAppFilePath;}
	CString			GetAppCfgPath(){return m_strCfgFilePath;}
	BOOL			IsNeedCheckWnd();
	BOOL			IsLongTimeUse();
	BOOL			HasWndNeedClose();

	//////////////////////////////////////////////////////////////////////////
	void			InitUI();
	void			SwitchLinkText(DWORD dwTextID);

	void			_InitWndProcList();

	void			OnLinkAllProc();
	void			OnLinkWndProc();

	LRESULT			OnWndProcListBoxGetDispInfo(LPNMHDR pnmh);
	LRESULT			OnWndProcListBoxClickCtrl(LPNMHDR pnmh);
	LRESULT			OnWndProcListBoxRClickCtrl(LPNMHDR pnmh);

	void			NotifyMouseWhellToListBox(CBkListBox* pList, UINT nFlags, short zDelta, CPoint pt);

	void			UpDateOneKeyFreeMemHistory();

	void			OnOneKeySpeedUp();
	void			OnClickWndProcMenuBtn();
	void			_CloseWindows(KAppWndListItemData);
	BOOL			_SwitchToWindow(HWND);
	BOOL			_KillProcess(DWORD);
	BOOL			_KillProcess2(DWORD);

	static DWORD WINAPI pfnThreadProcCloseWnd(__in  LPVOID lpParameter);
	CString				FormatLongExeName(const CString& strLongName);
	void				SetNotActiveTime(HWND);
	void				SetPerfOptDlgHasDoModal(BOOL);
	void				SetWinOptDlgHasDoModal(BOOL);

private:
	UINT					m_uTimer;
	vector<PerfDataPair>	m_processInfoList;
	CBkNetMonitorListBox*	m_pPerfMonitorListBox;
	HMENU					m_hPerfMonMenu;
	HMENU					m_hAppWndMenu;
	CAtlMap<DWORD, CPerfMonitorCacheData>			m_mapProcPerfCacheData;			//对应LISTBOX中的每行数据
	HANDLE								m_hEventGetNetData;
	KProcessPerfMgr						m_ProcPerfMgr;
	map<DWORD, KProcessPerfData*>		m_mapPerfData;
	double					m_fSysCpuUsage;
	double					m_fSysMemUsage;
	ULONGLONG				m_ullTotalIo;
	DWORD					m_nProcessNum;
	CAtlArray<UINT>			m_arrColumWidth;
	CAtlMap<DWORD, KProcInfoItem>	m_mapProcInfoItem;
	BOOL					m_bHideSystem;
	BOOL					m_bFloatWndIsOpen;
	int						m_nCurColum;
	HANDLE                  m_hEventExit;
	KWow64Switcher			m_wow64Swithcer;
	BOOL					m_bIsWin64;
	KModuleMgr*				m_pModuleMgr;
	CAtlMap<CString, KPerfMonListItemData>		m_mapNeedReportProc;
	CAtlMap<CString, KPerfMonListItemData>		m_mapReported;
	CString			m_strAppFilePath;
	CString			m_strCfgFilePath;
/*	HWND			m_hActiveWnd;*/
	HANDLE			m_hEventRptThreadExit;
	HANDLE			m_hThreadReport;
	BOOL			m_bReportThreadWorking;
	BOOL			m_bDoOnekeySpeedUpAtRun;
	BOOL			m_bPerfOptDlgIsDoModal;
	BOOL			m_bWinOptDlgHasDoModal;
/*	BOOL			m_bSortAscOrDesc;		//标记排列时是升序还是降序*/

	//  [2/25/2011 zhangbaoliang]
	CAtlMap<DWORD, UI_Group>	m_mapUI;
	CBkNetMonitorListBox*		m_pWndProcList;
	CString						m_strCfgPath;
	CString						m_strFileVer;
	vector<LPAPP_WND_STATE_ITEM>	m_vecAppWndStateData;
	KAppWndStateDataMgr				m_appWndStateMgr;
	vector<KAppWndListItemData>		m_vecWndListItemData;
	map<HWND, DWORD>				m_mapNotActiveTime;
	vector<KWndTimeItem>			m_vecWndTimeInfo;

	int						_PushUIGroup(DWORD dwGroupID, UI_Group uiGroup);

	int						_SwitchUI(DWORD dwGroupID);

	//end
	void					_SetColumState(int nColum);	

	void					_GetProcPerfData(vector<PerfDataPair>& vetPerfData);

	void					_InitPerfMonList();

	void					_CreateCacheData(vector<KProcInfoItem>& vetPerfData, CAtlMap<DWORD, CPerfMonitorCacheData>& mapCacheData);

	void					_RefreshSysCpu_Mem_IO_Usage();
	static void				pfnWMIQueryProcInfo();
	//包括对数据进行排序,如果数据个数合上一次有变化就更新列表函数，没有直接强制刷新数据
	void					_RefreshListBoxData(vector<PerfDataPair>& vetNetData);

	//更新进程标题
	int						_RefreshProcName(KPerfMonListItemData ItemData);

	//更新进程描述
	int						_RefreshProcDes(int nCurSel, KPerfMonListItemData ItemData);

	//更新安全等级
	int						_RefreshProcSercurityLevel(int nCurSel, KPerfMonListItemData ItemData);

	//更新进程CPU占用
	int						_RefreshProcCPUUsage(int nCurSel, KPerfMonListItemData ItemData);

	//更新进程内存占用
	int						_RefreshProcMemUsage(int nCurSel, KPerfMonListItemData ItemData);

	//更新进程磁盘IO
	int						_RefreshProcDiskIO(int nCurSel, KPerfMonListItemData ItemData);

	//更新进程ID
	int						_RefreshProcID(KPerfMonListItemData ItemData);

	//更新进程图标
	int						_RefreshProcICON(KPerfMonListItemData ItemData);

	//更新进程路径
	int						_RefreshProcPath(int nCurSel, KPerfMonListItemData ItemData);

	//更新操作按钮			
	int						_RefreshProcButton(int nCurSel, KPerfMonListItemData ItemData);

	//更新窗口程序数据
	int						UpDateAppWndListData();
	//更新有窗口的列表数据
	void					_RefreshAppWndListData(vector<KAppWndListItemData>& vetListData);

	//更新图标
	int						_RefreshAppWndListItemIcon(KAppWndListItemData);

	//更新窗口标题
	int						_RefreshAppWndListItemWndTitle(KAppWndListItemData);

	//更新窗口对应进程名	
	int						_RefreshAppWndListItemProcIcon(KAppWndListItemData);

	//更新窗口对应进程名	
	int						_RefreshAppWndListItemProcExe(KAppWndListItemData);

	//更新窗口状态
	int						_RefreshAppWndListItemWndState(KAppWndListItemData);

	//更新列表操作区域
	int						_RefreshAppWndListItemButton(KAppWndListItemData);

public:
    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(DEFCLOSEMAINBUTTON, BtnClose)
		BK_NOTIFY_ID_COMMAND(DEF_MIN_BTN, BtnMin)
		BK_NOTIFY_ID_COMMAND(IDC_TXT_COLUM_PROC_NAME, OnClickColumTitle)
		BK_NOTIFY_ID_COMMAND(IDC_TXT_COLUM_PROC_LEVEL, OnClickColumLevel)
		BK_NOTIFY_ID_COMMAND(IDC_TXT_COLUM_PROC_CPU, OnClickColumCPU)
		BK_NOTIFY_ID_COMMAND(IDC_TXT_COLUM_PROC_MEM, OnClickColumMEM)
		BK_NOTIFY_ID_COMMAND(IDC_TXT_CULUM_PROC_IO, OnClickColumIO)
		BK_NOTIFY_ID_COMMAND(IDC_CHECK_HIDE_SYSTEM, OnBkCheckHideSystemClick)
		BK_NOTIFY_ID_COMMAND(IDC_IMG_COLUM_LISTSORT, OnBkClickListSortImg)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_TEXT_SHOW_OR_HIDE_TIP, OnClickShowOrHideTip)
		//  [2/25/2011 zhangbaoliang]
		BK_NOTIFY_ID_COMMAND(PERF_MONITOR_CTRL_ID_807, OnLinkAllProc)
		BK_NOTIFY_ID_COMMAND(PERF_MONITOR_CTRL_ID_808, OnLinkWndProc)
		BK_NOTIFY_ID_COMMAND(PERF_MONITOR_CTRL_ID_812, OnOneKeySpeedUp)
		BK_NOTIFY_ID_COMMAND(PERF_MONITOR_CTRL_ID_811, OnOneKeySpeedUp)
		BK_NOTIFY_ID_COMMAND(PERF_MONITOR_CTRL_ID_816, OnChangeSetting)
    BK_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(KMainDlg)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<KMainDlg>)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MSG_WM_SIZE(OnSize)
		CHAIN_MSG_MAP(CBkDialogImpl<KMainDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		MESSAGE_HANDLER(WM_UPLIVE_SHOW, ShowUI )
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MESSAGE_HANDLER_EX(WM_COMMAND,		OnPerfMonCommand)
		MESSAGE_HANDLER_EX(WM_COMMAND,		OnAppWndCommand)
		NOTIFY_HANDLER_EX(IDC_LST_PERF_MONITOR_LISTBOX, BKLBM_GET_DISPINFO,		OnListBoxGetDispInfo)
		NOTIFY_HANDLER_EX(IDC_LST_PERF_MONITOR_LISTBOX, BKLBM_CALC_MAX_HEIGHT,	OnListBoxGetMaxHeight)
		NOTIFY_HANDLER_EX(IDC_LST_PERF_MONITOR_LISTBOX, BKLBM_CALC_ITEM_HEIGHT,	OnListBoxGetItemHeight)
		NOTIFY_HANDLER_EX(IDC_LST_PERF_MONITOR_LISTBOX, BKLBM_ITEMCLICK,		OnListBoxClickCtrl)
		NOTIFY_HANDLER_EX(IDC_LST_PERF_MONITOR_LISTBOX, BKRBM_ITEMCLICK,		OnListBoxRClickCtrl)

		//  [2/25/2011 zhangbaoliang]
		NOTIFY_HANDLER_EX(IDC_LIST_WNDPROC_APP, BKLBM_GET_DISPINFO,		OnWndProcListBoxGetDispInfo)
		NOTIFY_HANDLER_EX(IDC_LIST_WNDPROC_APP, BKLBM_ITEMCLICK,		OnWndProcListBoxClickCtrl)
		NOTIFY_HANDLER_EX(IDC_LIST_WNDPROC_APP, BKRBM_ITEMCLICK,		OnWndProcListBoxRClickCtrl)

    END_MSG_MAP()
	
};
