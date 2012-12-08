
#pragma once
#include <atlstr.h>
#include <atlrx.h>
#include <atlcoll.h>

#include "runoptlistview.h"
#include "kclear/irunoptimize.h"
#include "msgdefine.h"
#include "kuires.h"
#include "image3tabtree.h"
#include "kclear/icleanmgr.h"
#include "kclear/icleanproc.h"
#include <wtlhelper/whwindow.h>
#include <deque>
#include "kuiwin/kuimsgbox.h"
#include "kclear/ionekeyclean.h"

#define STATE_RUNOPT_NULL		0
#define STATE_RUNOPT_SCAN		1
#define STATE_RUNOPT_CANCEL		2

#include "kclearmsg.h"


#define TREE_BEGIN(x)						(x-100)
#define TREE_END(x)							(x-200)
#define TREE_SCANING(x)							(x-300)

typedef struct _TREE_TY
{
	int iType;
	HTREEITEM hItem;
	HTREEITEM hSubItem;
	CString strReg;
	CString strValue;
	CString strRegValue;
	CString strParent;
	DWORD dwFileLowPart;
	int nType;  //1reg  2file
}Treety, *PTreety;
typedef struct _TREE_CLEAN
{
	int iType;
	CString strReg;
	CString strValue;
	int nType;

}TreeClear, *PTreeClear;

typedef struct _ONE_TY
{
	CString strClearname;
	CString strClearInfo;
	CString strState;
	CString strExtName;
	CString strClear2name;
	int nItem;
	DWORD dwFileNum;
	DWORD dwFilesize;
	DWORD dwJianceNum;
	DWORD dwJiancedNum;
}OneTy, *POneTy;

typedef struct _TREE
{
	int iCheck;
	HTREEITEM hItem;
	CString strItemInfo;
	CString strExtName;
	int nSn;
	int nType;
}TreeInfo, *PTreeInfo;

class CKscMainDlg;
class CUIHandlerOpt :
/*	public CBkNavigator,*/
	public IEnumRunCallBackNull,
	public IListOpCallback,
	public ICleanTask
{
public:
	CUIHandlerOpt(CKscMainDlg* refDialog)
		: m_dlg(refDialog)
		, m_pRunOptEng(NULL)
		, m_nState(STATE_RUNOPT_NULL)
		, m_bFirstShowed(FALSE)
		, m_bFirstScanRunEnd(FALSE)
		, m_nPos(0)
		, m_nCurrentScan(0)
		, m_nScrollMax(0)
		, m_nScanCount(0)
		, m_CurrentType(0)
		, m_bStopScan(FALSE)
	{
		m_hTimer = (HANDLE)-1;
		m_hScan = (HANDLE)-1;
		m_hClean = (HANDLE)-1;
		m_pCleanMgr = NULL;
		m_hScanEx = (HANDLE)-1;
        m_pCleanPrc = NULL;

	}

	virtual ~CUIHandlerOpt()
	{
		if (m_pRunOptEng!=NULL)
		{
			CRunOptLoader::GetGlobalPtr()->FreeOptEng(m_pRunOptEng);
			m_pRunOptEng = NULL;
		}
		if (m_hScan != (HANDLE)-1)
		{
			TerminateThread(m_hScan, NULL);
		}
		if (m_hClean != (HANDLE)-1)
		{
			TerminateThread(m_hClean, NULL);
		}
		if(m_hTimer != (HANDLE)-1)
		{
			TerminateThread(m_hTimer, NULL);
		}
		if (m_hScanEx != (HANDLE)-1)
		{
			TerminateThread(m_hScanEx, NULL);
			m_hScanEx = (HANDLE)-1;
		}
		
	}
public:
	CSimpleArray<ClearTreeType>		m_arrClearType;
	CSimpleArray<ClearTreeInfo>		m_arrClearInfo;
	CAtlArray<TreeClear>			m_arrClear;//清除内容
	CAtlMap<HTREEITEM, CString>     m_amap;//清除父节点
	CAtlMap<CString, CString>		m_RescanCleanmap; //本次清理后清理不掉的。
	CAtlMap<int, int>		m_mapHenjiScanJs;//屏蔽即时生效信息
	bool m_bDllLoadError;
	int m_nCleanNum;			//清理时的文件或注册表的数量
	bool m_bScanBage;
	BOOL m_bOneScan;
	bool m_bOneStop;
	int m_nProgress;
	int m_nNewPro;
	CWHListViewCtrl m_wndListCtrl;
	CComCriticalSection		m_cs;
	CAtlMap<CString, OneTy> m_OneMap;
	CAtlMap<CString, CString> m_OneTypeMap;
	CAtlMap<CString, CString> m_OneScanFinish;
	CString	m_ScanLogFile;
	CString m_CacheFile;
	HANDLE m_hScan;
	HANDLE m_hScanEx;
	HANDLE m_hTimer;
	ICleanMgr					*m_pCleanMgr;
	IKClearProc					*m_pCleanPrc;
	ICleanCallback              *m_pOneCleanCallBack;
	ICleanTask                  *m_pOneCleanTask;

	static unsigned __stdcall ScanProc(LPVOID lpVoid);//扫描线程函数
	HANDLE m_hClean;
	static unsigned __stdcall CleanProc(LPVOID lpVoid);//清理线程函数

	static unsigned __stdcall ScanOneProc(LPVOID lpVoid);//清理线程函数
	static unsigned __stdcall ScanOnTimer(LPVOID lpVoid);
	static unsigned __stdcall ScanTreeOnTimer(LPVOID lpVoid);

	static unsigned __stdcall OneScanProce(LPVOID lpVoid);

	//henji
	CAtlMap<int, TreeInfo>		m_TreeHenjiMap;
	CAtlMap<int, CString>		m_ScanHenjiMap;
	CAtlMap<int, TreeInfo>      m_TreeHenjiMapEx;
	CSimpleArray<Treety>		m_arrScanHenji;
	int							m_nScanHenjiiType;
	int							m_nPos;
	int                         m_nCurrentScan;
	int							m_nScrollMax;
	int                         m_nScanCount;
	int							m_CurrentType;
	CSimpleArray<int>           m_finishScanId;
	std::deque<CString>         m_dOneScanId;


	static BOOL WINAPI myScanRegCallBack(void* pMain,int iType,LPCTSTR lpcszKeyFullPath,LPCTSTR lpcszValueName,LPCTSTR lpcszValueData);
	static BOOL WINAPI myScanFileCallBack(void* pMain,int iType,LPCTSTR lpcszFileFullPath,DWORD dwFileLowPart,LONG dwFileHighPart);
	static BOOL WINAPI SoftTreeCallBack(void* pThis,ClearTreeInfo treeinfo);
	static BOOL WINAPI SoftInfoCallBack(void* pThis,SoftInfo softinfo);
	static BOOL WINAPI myScanFileCallBackError(void* pMainError,int iType,LPCTSTR lpcszFileFullPath,DWORD dwFileLowPart,LONG dwFileHighPart);
	static BOOL WINAPI myScanRegCallBackError(void* pMainError,int iType,LPCTSTR lpcszKeyFullPathError,LPCTSTR lpcszValueNameError);

	DWORD						m_dwFileSize;
	DWORD						m_dwAllFileSize;
	HTREEITEM                   m_RootTree;
public:
	BOOL IsOpPermit()
	{
		return (m_nState==STATE_RUNOPT_NULL);
	}

	void Init();
	void InitDelay();
	void  UnInit()
	{
		WriteOneCache(m_TreeHenjiMap, &m_Tree, _T("kclear_Exit_checke_henji"));
	}
	void FirstShow();
	void SecondShow();
	BOOL OnBkRunTypeSelChange(int nTabItemIDOld, int nTabItemIDNew);
	BOOL OnBkSysOptSelChange(int nTabItemIDOld, int nTabItemIDNew);
	BOOL OnBkClrTopSelChange(int nTabItemIDOld, int nTabItemIDNew);
	bool IsClr();
	OneTy QueryItem(int iItem);
	void OnBkCLROneStop();

	void InitEng()
	{
		if (m_pRunOptEng==NULL)
			m_pRunOptEng = CRunOptLoader::GetGlobalPtr()->GetOptEng();
	}

	BOOL FindRunItem(CKsafeRunInfo* pInfo);
	BOOL _FindRunItem(CKsafeRunInfo* pInfo);
	void EndScan();
	void RestoreEnd();
	void PostRestoreRunItem(DWORD nType,BOOL bEnable,LPCTSTR lpName);

	LRESULT OnWmScanEnd(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmRestoreEnd(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmFindItem(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmFirstIn(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmChangeTop( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmChangeTop2( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmChangeTop3( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmChangeTopEx1( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmChangeTopEx2( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmChangeTopEx3( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmlajiChangeTop( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmREGChangeTop( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmLAJILBtn(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnLVNVirusGetDispInfo(LPNMHDR pnmh);
	LRESULT OnWmRegScanFinish( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmLAJIScanFinish( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmOneScanTxt( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmOneScanPrg( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmOneScanValue( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmHenjiScanFinish( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmHENJILBtn( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmRegClearFinish( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmHenjiClearFinish( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmLajiClearFinish( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmTreeScaning( UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnWmHenjiTreeHong( UINT uMsg, WPARAM wParam, LPARAM lParam );
	LRESULT OnBkClrScanForKIS( UINT uMsg, WPARAM wParam, LPARAM lParam );
	LRESULT OnWmHenjiTreeHei( UINT uMsg, WPARAM wParam, LPARAM lParam );

	BOOL OnClickButton(CKsafeRunInfo* pInfo);
	BOOL OnDeleteItem(CKsafeRunInfo* pInfo);
	BOOL OnClickLabel(DWORD nIndex, CKsafeRunInfo* pInfo);

	void OnBtnAutoOpt();
	HRESULT GetKSoftDirectory(CString &strDir);
	void WriteLog(DWORD dwNum, DWORD dwSize);
	void WriteOneCache(CAtlMap<int, TreeInfo>& TreeMap, CWH3StateCheckTreeCtrl10* Tree, CString strSection);
	void SetOneState(CAtlMap<CString, CString>& OneMap,std::deque<CString>& arr, CString strTypeid);

	BOOL CloseCheck()
	{
		WriteOneCache(m_TreeHenjiMap, &m_Tree, _T("kclear_Exit_checke_henji"));

		if(m_hTimer != (HANDLE)-1)
		{
			CKuiMsgBox	msg;
			CString			strMsg;
			strMsg.Format(_T("正在进行清理工作，您确定要退出吗？"));
			msg.AddButton(_T("确定"), IDYES);
			msg.AddButton(_T("取消"), IDCANCEL);

			if(msg.ShowMsg(strMsg,NULL,MB_YESNO|MB_ICONWARNING) != IDYES) 
				return FALSE;
		}
		return TRUE;
	}

public:
	void OnBkShowEnable();
	void OnBkShowDisable();
	void OnBkShowAll();
	void OnSetting();

	void OnBkHideSystem();
	void OnBkShowSystem();

	void OnListReSize(CRect rcWnd);
	void OnOneListReSize(CRect rcWnd);
	void RefreshItemList();
	void OnBkCancelAll();
	void OnBkBtnWaitMore();
	void OnBkRunOptTopMore();

	void RefreshCLRState(int nSel=-1, BOOL bDowning=FALSE);

	void OnBkClrDafaul();
	void OnBkClrOne();

	void RefreshTabNumber(DWORD nNewTab=-1);
	void SetTopRunDiv(DWORD nType);

	void OnBkClrScan();
	void OnBkClrClean();
	void OnBkClrReScan();
	void OnClrHenjiAllSelect();
	void OnBkClrShowDafaul();


	void ResearchReg(CAtlMap<int, TreeInfo>& TreeMap, CWH3StateCheckTreeCtrl10* Tree);
	void SetCheckTree(CAtlMap<int, TreeInfo>& TreeMap, CString strTypeid, CWH3StateCheckTreeCtrl10* Tree);
	void GetCheckReg(CAtlMap<int, TreeInfo>& TreeMap, CWH3StateCheckTreeCtrl10* Tree,bool bScanFinish=false);
	void OnClrRegAllSelect();

	void InitHenji();
	void InitDLL();
	void InitLaji();
	void InitREG();
	void RebotUserTree();
	void LoadUserCache(CAtlMap<CString, CString>& TreeMap, CString section, bool bOne=false);
	void LoadUserCache(std::deque<CString>& arr, CString section, bool bOne=false);
	bool IsCheckCache(CString section);

	void OnOneScan();

	void OnBkCLROneScan();
	void OnBkCLROneReturn();

	int GetScrollMaxSize(TreeInfo& treeinfo);
	int GetScrollCurrentPos(TreeInfo& treeinfo);
    BOOL IsEableCleanBtn();
	BOOL IsIncludeNoScanItem();
	BOOL IsAllNeedRebootDelete();
	BOOL IsAllNeedRebootDeleteEx();
	BOOL IsNoCleanEmpty();

	CWH3StateCheckTreeCtrl10 m_Tree;
	/*CWH3StateCheckTreeCtrl10 m_TreeLaji;
	CWH3StateCheckTreeCtrl10 m_TreeReg;*/

	void __stdcall SetCallback(ICleanCallback* piCallback);
	bool __stdcall StartClean(int nTask, const char* szTask);
	bool __stdcall StopClean();
	void __stdcall Release();

	bool StopCleanEx();

	BOOL ParaseOneScan(const char* szTask, /*std::deque<int>&vnOneScan, */const int nTask);
/*	virtual CBkNavigator* OnNavigate(CString &strChildName);*/

    static BOOL CheckIEVersioin(int& ieVersion);

protected:
	CKscMainDlg*		    m_dlg;
	CRunOptListView			m_runList;
	IRunOptimizeEng*		m_pRunOptEng;
	DWORD					m_nState;
	BOOL					m_bFirstShowed;
	BOOL					m_bFirstScanRunEnd;
	int						m_n3TabType;//3级TAB显示类别
	std::deque<int>         m_OneScanId;//一键扫描的传入参数id
	CSimpleArray<int>       m_OneKeyId;
	BOOL                    m_bStopScan;
public:
	KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		KUI_NOTIFY_TAB_SELCHANGE(IDC_TAB_RUNOPT_TYPE,OnBkRunTypeSelChange)
		KUI_NOTIFY_TAB_SELCHANGE(IDC_TAB_SYSOPT_LEFT,OnBkSysOptSelChange)
		KUI_NOTIFY_ID_COMMAND(IDC_LBL_RUNOPT_SHOW_ENABEL,OnBkShowEnable)
		KUI_NOTIFY_ID_COMMAND(IDC_LBL_RUNOPT_SHOW_DISABLE,OnBkShowDisable)
		KUI_NOTIFY_ID_COMMAND(IDC_LBL_RUNOPT_SHOW_ALL,OnBkShowAll)
		KUI_NOTIFY_ID_COMMAND(IDC_LBL_RUNOPT_SHOW_ENABEL2,OnBkShowEnable)
		KUI_NOTIFY_ID_COMMAND(IDC_LBL_RUNOPT_SHOW_DISABLE2,OnBkShowDisable)
		KUI_NOTIFY_ID_COMMAND(IDC_LBL_RUNOPT_SHOW_ALL2,OnBkShowAll)

		KUI_NOTIFY_ID_COMMAND(IDC_LBL_RUNOPT_HIDE_SYSTEM,OnBkHideSystem)
		KUI_NOTIFY_ID_COMMAND(IDC_LBL_RUNOPT_SHOW_SYSTEM,OnBkShowSystem)
		KUI_NOTIFY_ID_COMMAND(IDC_LBL_RUNOPT_REFRESH,RefreshItemList)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_SYSOPT_WAIT_MORE,OnBkBtnWaitMore)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_RUNOPT_CANCEL_ALL,OnBkCancelAll)
		KUI_NOTIFY_ID_COMMAND(IDC_LBL_RUNOPT_TOP_MORE,OnBkRunOptTopMore)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_RUNOPT_AUTO_OPT,OnBtnAutoOpt)

		KUI_NOTIFY_ID_COMMAND(IDC_BTN_CLR_SCAN_RESCAN,OnBkCLROneScan)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_CLR_SCAN_RECOME,OnBkCLROneReturn)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_CLR_STOP_SCANING,OnBkCLROneStop)


		KUI_NOTIFY_TAB_SELCHANGE(IDC_TAB_CLR_TOP,OnBkClrTopSelChange)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_CRL_SETTING, OnSetting)

		KUI_NOTIFY_ID_COMMAND(IDC_TXT_CLR_TREEDOWN_DAFU, OnBkClrDafaul)
		KUI_NOTIFY_ID_COMMAND(IDC_TXT_CLR_TREEDOWN_ONE, OnBkClrOne)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_CLR_TREEDOWNBTN_SCAN, OnBkClrScan)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_CLR_TREEDOWNBTN_SCAN_START, OnBkClrScan)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_CLR_TREEDOWNBTN_CLEAN, OnBkClrClean)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_CLR_TREEDOWNBTNRESCAN, OnBkClrReScan)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_CLR_TREEDOWNBTN_CLEAN1, OnBkClrClean)
		KUI_NOTIFY_ID_COMMAND(IDC_CHECK_CLR_HENJI, OnClrHenjiAllSelect)
		KUI_NOTIFY_ID_COMMAND(IDC_TXT_CLR_HENJI_TREEDOWN_SHOWDAFU, OnBkClrShowDafaul)

		KUI_NOTIFY_REALWND_RESIZED(IDC_LST_RUNOPT, OnListReSize)
		KUI_NOTIFY_REALWND_RESIZED(IDC_LST_CLR_SCAN, OnOneListReSize)
	KUI_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CUIHandlerOpt)
		MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
		MESSAGE_HANDLER_EX(MSG_SYSOPT_RUN_SCAN_END,		OnWmScanEnd)
		MESSAGE_HANDLER_EX(MSG_SYSOPT_RUN_RESTORE_END,	OnWmRestoreEnd)
		MESSAGE_HANDLER_EX(MSG_SYSOPT_RUN_SCAN_FIND,	OnWmFindItem)
		MESSAGE_HANDLER_EX(MSG_SYSOPT_RUN_FIRST_IN,		OnWmFirstIn)
		MESSAGE_HANDLER_EX(MSG_CLR_TREETOP,	OnWmChangeTop)
		MESSAGE_HANDLER_EX(MSG_CLR_TREETOP2,	OnWmChangeTop2)
		MESSAGE_HANDLER_EX(MSG_CLR_TREETOP3,	OnWmChangeTop3)
		MESSAGE_HANDLER_EX(MSG_CLR_TREETOP_EX1,	OnWmChangeTopEx1)
		MESSAGE_HANDLER_EX(MSG_CLR_TREETOP_EX2,	OnWmChangeTopEx2)
		MESSAGE_HANDLER_EX(MSG_CLR_TREETOP_EX3,	OnWmChangeTopEx3)
		MESSAGE_HANDLER_EX(MSG_CLR_LAJI_TREETOP,	OnWmlajiChangeTop)
		MESSAGE_HANDLER_EX(MSG_CLR_REG_TREETOP,	OnWmREGChangeTop)
		MESSAGE_HANDLER_EX(MSG_CLR_HENJI_TREELUP,	OnWmHENJILBtn)
		MESSAGE_HANDLER_EX(MSG_CLR_LAJI_TREELUP, OnWmLAJILBtn)
		MESSAGE_HANDLER_EX(MSG_CLR_REG_SCAN_FINISH, OnWmRegScanFinish)
		MESSAGE_HANDLER_EX(MSG_CLR_HENJI_SCAN_FINISH, OnWmHenjiScanFinish)
		MESSAGE_HANDLER_EX(MSG_CLR_LAJI_SCAN_FINISH, OnWmLAJIScanFinish)
		MESSAGE_HANDLER_EX(MSG_CLR_ONE_SCAN_TXT, OnWmOneScanTxt)
		MESSAGE_HANDLER_EX(MSG_CLR_ONE_SCAN_PRG, OnWmOneScanPrg)
		MESSAGE_HANDLER_EX(MSG_CLR_ONE_SCAN_VALUE, OnWmOneScanValue)
		MESSAGE_HANDLER_EX(MSG_CLR_REG_CLEAR_fINISH, OnWmRegClearFinish)
		MESSAGE_HANDLER_EX(MSG_CLR_HENJI_CLEAR_fINISH, OnWmHenjiClearFinish)
		MESSAGE_HANDLER_EX(MSG_CLR_LAJI_CLEAR_fINISH, OnWmLajiClearFinish)
		MESSAGE_HANDLER_EX(MSG_CLR_TREE_SCANING,OnWmTreeScaning)
		MESSAGE_HANDLER_EX(MSG_CLR_HENJI_TREE_SHOW_HONG,OnWmHenjiTreeHong)
		MESSAGE_HANDLER_EX(MSG_CLR_HENJI_TREE_SHOW_HEI,OnWmHenjiTreeHei)
		MESSAGE_HANDLER_EX(KIS_WM_TREEDOWNBTN_SCAN,OnBkClrScanForKIS)
		NOTIFY_HANDLER_EX(IDC_LST_CLR_SCAN, LVN_GETDISPINFO, OnLVNVirusGetDispInfo)

	END_MSG_MAP()
};
