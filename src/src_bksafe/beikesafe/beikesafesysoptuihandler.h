
#pragma once

#include "runoptlistview.h"
#include "runautooptlist.h"
#include <runoptimize/interface.h>
#include "bkmsgdefine.h"
#include "bkwin/bklistbox.h"
#include <list>
#include <algorithm>
using namespace std;
#include "htmlcallcpp.h"
#include "calltools.h"

#define STATE_RUNOPT_NULL		0
#define STATE_RUNOPT_SCAN		1
#define STATE_RUNOPT_CANCEL		2

struct RUNOPT_HISTORY_DATA
{
public:
	RUNOPT_HISTORY_DATA()
	{
		strDate = "";
		bTitle	= TRUE;
		pInfo	= NULL;
	}

	CString			strDate;
	BOOL			bTitle;
	CKsafeRunInfo*	pInfo;
};

class CBeikeSafeMainDlg;
class CBeikeSafeSysOptUIHandler :
	public CBkNavigator,
	public IEnumRunCallBackNull2,
	public IAutoOptLstCBack,
	public IListOpCallback,
	public IBKCallSysTools
{
public:
	CBeikeSafeSysOptUIHandler(CBeikeSafeMainDlg* refDialog)
		: m_dlg(refDialog)
		, m_pRunOptEng(NULL)
		, m_nState(STATE_RUNOPT_NULL)
		, m_bFirstShowed(FALSE)
		, m_bFirstScanRunEnd(FALSE)
		, m_hOptimizeThread(NULL)
		, m_bOpting(FALSE)
		, m_bRefreshList(FALSE)
		, m_nIgnoredCount(0)
		, m_bReportOnekeyOpt(TRUE)
		, m_bRunScanEnd(FALSE)
		, m_bSrvScanEnd(FALSE)
		, m_bTaskScanEnd(FALSE)
		, m_hThreadFirstIn(NULL)
		, m_bDelayRunTipShowed(FALSE)
		, m_bDelayRunValid(TRUE)
		, m_nDelayCount(0)
		, m_bRefreshAutoOptList(FALSE)
		, m_bHasShowDelayTip(FALSE)
	{
		m_hIconSystem = ::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_RUN_SYSTEM));
		m_hIconUnknow = ::LoadIcon((HMODULE)&__ImageBase, MAKEINTRESOURCE(IDI_RUN_UNKNOWN));
	}

	virtual ~CBeikeSafeSysOptUIHandler()
	{
		if (m_pRunOptEng!=NULL)
		{
			CRunOptLoader::GetGlobalPtr()->FreeOptEng(m_pRunOptEng);
			m_pRunOptEng = NULL;
		}
		if (m_hOptimizeThread != NULL)
		{
			::CloseHandle(m_hOptimizeThread);
			m_hOptimizeThread = NULL;
		}
		if (m_pHistoryList != NULL)
		{
			delete m_pHistoryList;
			m_pHistoryList = NULL;
		}
		if (m_hIconSystem != NULL)
		{
			::DestroyIcon(m_hIconSystem);
			m_hIconSystem = NULL;
		}
		if (m_hIconUnknow != NULL)
		{
			::DestroyIcon(m_hIconUnknow);
			m_hIconUnknow = NULL;
		}
	}

public:
	BOOL IsOpPermit()
	{
		return (m_nState==STATE_RUNOPT_NULL);
	}

	void Init();
	void FirstShow();
	void SecondShow();
	BOOL OnBkRunTypeSelChange(int nTabItemIDOld, int nTabItemIDNew);
	BOOL OnBkSysOptSelChange(int nTabItemIDOld, int nTabItemIDNew);

	void InitEng()
	{
		if (m_pRunOptEng==NULL)
			m_pRunOptEng = CRunOptLoader::GetGlobalPtr()->GetOptEng2();
	}

//	BOOL FindRunItem(CKSafeSysoptRegInfo* pInfo) { return TRUE; };
	BOOL FindRunItem(CKsafeRunInfo* pInfo);
	BOOL FindRunItem(CKSafeSysoptRegInfo* pInfo);
	BOOL _FindRunItem(CKsafeRunInfo* pInfo);
	void EndScan(DWORD dwFlags = 0);
	void RestoreEnd();
	void PostRestoreRunItem(DWORD nType,BOOL bEnable,LPCTSTR lpName);

	LRESULT OnWmScanEnd(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmRestoreEnd(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmFindItem(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmFirstIn(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnOnekeyOptEnd(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnRefreshOnekeyOpt(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnRestoreDelayRun(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmRefreshItemList(UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL OnClickButton(CKsafeRunInfo* pInfo, DWORD nIndex);
	BOOL OnDeleteItem(CKsafeRunInfo* pInfo);
	BOOL OnClickLabel(DWORD nIndex, CKsafeRunInfo* pInfo);
	void ClickCheck();

	BOOL OnClickAddIgnore(DWORD nIndex, DWORD dwID, int nItem);

    void UpdateClearInterface(int nSel);

public:

	void OnBkShowDisableItem();	//IDC_CHECKBOX_SHOW_DISABLE,IDC_CHECKBOX_SHOW_DISABLE2的响应事件
	void OnListReSize(CRect rcWnd);
	void RefreshItemList(DWORD dwFlags = 0);
	void OnBkCancelAll();
	void OnBkLnkCancelAll();	//一键优化Tab页的“撤销全部更改”
	void OnBkBtnWaitMore();
	void OnBkRunOptTopMore();

	void RefreshCLRState(int nSel=-1, BOOL bDowning=FALSE);
    void RefreshUpdateState(int nSel=-1, BOOL bDowning=FALSE);

	void RefreshTabNumber(DWORD nNewTab=-1);
	void SetTopRunDiv(DWORD nType);
	void GetRunListByCheckState();

	virtual CBkNavigator* OnNavigate(CString &strChildName);

	void InitAutoOptList();
	void OnBkBtnOnkeyOpt();
	void OnAutoOptCheckAll();
	void OnAutoOptCheckAll2();
	void OnBkBtnRefresh();
	void OnBkLnkViewRun();
	void OnBkLnkViewOnekeyOpt();
	void OnTimer(UINT_PTR nIDEvent);//优化时定时器刷新显示loading动画
	void _ShowOneKeyOptDoing();
	void _ShowOneKeyOptScanning();
	void OnBkLnkCleanTrash();
	void OnBkLnkSoftMgr_ZJBJ();
	void OnBkCheckHideSystem();
	void ShowNoTaskInfo();
	static DWORD WINAPI	OneKeyOptProc(LPVOID lpParam);
	BOOL Is3XXInstalled();
	void SetIniRunOnce();
	void OnBkLnkShowIgnoredDlg();
	BOOL OnBkLnkAddToIgnored(DWORD dwID, int iItem);
	void SaveIgnoreID(int nID);
	void LoadIgnoredID();
	void GetIgnoredItem();
	void OnBkLnkRefreshList();
	void GenerateHistoryList();
	void ShowNoHistoryInfo();
	void OnBkLnkHistoryCancelAll();
	void OnBkLnkHistoryRefresh();
	BOOL GetSubScanEnd(int nType);
	void SetSubScanning(int nType,BOOL bIsScanning);
	void RefreshIgnoredText();
	void PopupOperateMenu();
	void ShowDelayTipDlg();
	void RefreshTabNumber_History();
	static unsigned _stdcall ThreadProc_ShowDelayRunTip(PVOID param);
	void _ShowDelayTip();
	void OnBkLnkShowSetting();
	void OnBkLnkNetMon();
	// bksafe tools [12/24/2010 zhangbaoliang]
	public:
		void		callBackCheck();
		void		callBackExec();
		void		callBackInstall();
protected:
	LRESULT OnListBoxGetDispInfo(LPNMHDR pnmh);
	LRESULT OnListBoxGetMaxHeight(LPNMHDR pnmh);
	LRESULT OnListBoxGetItemHeight(LPNMHDR pnmh);
	LRESULT OnListBoxClickCtrl(LPNMHDR pnmh);
	
protected:
	CBeikeSafeMainDlg*		m_dlg;
	CRunOptListView			m_RunList;
	CAutoRunOptList			m_OneKeyOptList;
	CBkListBox*				m_pHistoryList;
	IRunOptimizeEng2*		m_pRunOptEng;
	DWORD					m_nState;
	BOOL					m_bFirstShowed;
	BOOL					m_bFirstScanRunEnd;
	BOOL					m_bOpting;
	BOOL					m_bRefreshList;		//runlist刷新后置为TRUE,为TRUE则InitAutoOptList,再置为FALSE
	BOOL					m_bReportOnekeyOpt;	//进入一键优化tab，是否要上报
	HANDLE					m_hOptimizeThread;
	CString					m_strIniFilePath;
	CString					m_strIgnoredIniPath;
	int						m_nIgnoredCount;
	CSimpleArray<int>		m_arrayIgnoredID;
	CSimpleArray<CKsafeRunInfo*>	m_arrayItem;			//所有启动项
	CSimpleArray<CKsafeRunInfo*>	m_arrayIgnoredItem;		//忽略项
	CSimpleArray<CKsafeRunInfo*>	m_arrayOnekeyOptItem;	//除去忽略项之后的启动项
	vector<RUNOPT_HISTORY_DATA*>	m_vecHistoryItem;	//优化历史项
	HICON					m_hIconSystem;
	HICON					m_hIconUnknow;
	BOOL					m_bRunScanEnd;
	BOOL					m_bSrvScanEnd;
	BOOL					m_bTaskScanEnd;
	HANDLE					m_hThreadFirstIn;
	BOOL					m_bDelayRunTipShowed;
	BOOL					m_bDelayRunValid;	//标记一键优化是否处理可延迟的优化项
	BOOL					m_bHasShowDelayTip;	//标记是否已经显示过延迟启动介绍框
	int						m_nDelayCount;
	BOOL					m_bRefreshAutoOptList;
	CSimpleArray<CKSafeSysoptRegInfo*> m_arraySysCfgItem;
	CSimpleArray<CKSafeSysoptRegInfo*> m_arrayAutooptSysCfgItem;
	CSimpleArray<CKSafeSysoptRegInfo*> m_arrayIgnoredSysCfgItem;

	// 系统工具集 [12/22/2010 zhangbaoliang]
	CWHHtmlContainerWindow2	m_hwndHtmlCallCpp;
	BKCallSysTools			m_bksafeTools;
	void					_InitKSafeToolsPage();
	void					_OpenKSafePage();
public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_TAB_SELCHANGE(IDC_TAB_RUNOPT_TYPE,OnBkRunTypeSelChange)
		BK_NOTIFY_TAB_SELCHANGE(IDC_TAB_SYSOPT_LEFT,OnBkSysOptSelChange)
		BK_NOTIFY_ID_COMMAND(IDC_CHECKBOX_SHOW_DISABLE,OnBkShowDisableItem)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_RUNOPT_REFRESH,OnBkLnkRefreshList)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYSOPT_WAIT_MORE,OnBkBtnWaitMore)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_RUNOPT_CANCEL_ALL,OnBkCancelAll)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_RUNOPT_TOP_MORE,OnBkRunOptTopMore)
		
		BK_NOTIFY_ID_COMMAND(IDC_BTN_RUN_AUTOOPT_DO,OnBkBtnOnkeyOpt)
		BK_NOTIFY_ID_COMMAND(IDC_CHECKBOX_AUTOOPT_ALL,OnAutoOptCheckAll)

		BK_NOTIFY_ID_COMMAND(IDC_LNK_AUTOOPT_TOP_REFRESH,OnBkBtnRefresh)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_SHOW_IGNORED,OnBkLnkShowIgnoredDlg)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_VIEW_RUN,OnBkLnkViewRun)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_NETMON,OnBkLnkNetMon)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_TRASHCLEANER2,OnBkLnkCleanTrash)

		BK_NOTIFY_ID_COMMAND(IDC_LNK_TRASHCLEANER,OnBkLnkCleanTrash)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_ZHUANGJIBIBEI,OnBkLnkSoftMgr_ZJBJ)
		BK_NOTIFY_ID_COMMAND(IDC_CHECKBOX_RUNOPT_HIDE_SYSTEM,OnBkCheckHideSystem)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_RUNOPT_HISTORY_ONEKEYOPT,OnBkLnkViewOnekeyOpt)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_RUNOPT_HISTORY_CANCELALL,OnBkLnkHistoryCancelAll)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_SYSOPT_SETTING, OnBkLnkShowSetting)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_AUTOOPT_CANCELALL, OnBkLnkCancelAll)
		BK_NOTIFY_REALWND_RESIZED(IDC_LST_RUNOPT, OnListReSize)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBeikeSafeSysOptUIHandler)
		MSG_WM_TIMER(OnTimer)	//优化时定时器刷新显示loading动画
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MESSAGE_HANDLER_EX(MSG_SYSOPT_RUN_SCAN_END,		OnWmScanEnd)
		MESSAGE_HANDLER_EX(MSG_SYSOPT_RUN_RESTORE_END,	OnWmRestoreEnd)
		MESSAGE_HANDLER_EX(MSG_SYSOPT_RUN_SCAN_FIND,	OnWmFindItem)
		MESSAGE_HANDLER_EX(MSG_SYSOPT_RUN_FIRST_IN,		OnWmFirstIn)
		MESSAGE_HANDLER_EX(MSG_SYSOPT_ONEKEYOPT_END,	OnOnekeyOptEnd)
		MESSAGE_HANDLER_EX(MSG_SYSOPT_REFRESH_ONEKEYOPT,	OnRefreshOnekeyOpt)
		MESSAGE_HANDLER_EX(MSG_SYSOPT_RESTORE_DELAYRUN,		OnRestoreDelayRun)
		MESSAGE_HANDLER_EX(MSG_SYSOPT_REFRESH_ITEMLIST,		OnWmRefreshItemList)
		NOTIFY_HANDLER_EX(IDC_LST_RUNOPT_HISTORY, BKLBM_GET_DISPINFO,OnListBoxGetDispInfo)
		NOTIFY_HANDLER_EX(IDC_LST_RUNOPT_HISTORY, BKLBM_CALC_MAX_HEIGHT,OnListBoxGetMaxHeight)
		NOTIFY_HANDLER_EX(IDC_LST_RUNOPT_HISTORY, BKLBM_CALC_ITEM_HEIGHT,OnListBoxGetItemHeight)
		NOTIFY_HANDLER_EX(IDC_LST_RUNOPT_HISTORY, BKLBM_ITEMCLICK,OnListBoxClickCtrl)
	END_MSG_MAP()
};