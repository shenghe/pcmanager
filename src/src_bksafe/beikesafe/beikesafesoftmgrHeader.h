#pragma once


#include <bkres/bkres.h>
#include "bkwin/bklistbox.h"
#include <runoptimize/interface.h>
//#include "bksoftmgruninstall.h"
#include "SoftMgrListView.h"
#include "SoftMgrItemData.h"
#include "softmgrqueryedit.h"
#include "softmgrdetail.h"
#include "TypeListBox.h"
#include <softmgr\ISoftManager.h>
#include "softmgr\ISoftChecker.h"
#include <softmgr\IDTManager.h>
#include <libdownload\libDownload.h>
#include <softmgr\URLEncode.h>
#include <softmgr\ISoftChecker.h>
#include "softmgr\ISoftUninstall.h"
#include "softmgr\ISoftDesktop.h"
#include "softmgr\KSoftMgrUpdateInfo.h"
#include "softmgrDownListView.h"
#include "SoftMgrUpdateRollWnd.h"
#include "SoftMgrUniExpandListView.h"
#include "bkmsgdefine.h"
#include "beikesafesoftmgrnecess.h"
#include "kws/ctrls/kcombobox.h"
#include "kws/setting_hlp.h"
#include "SoftInstProxy.h"
#include <softmgr/ISoftInstall.h>
#include "LockHelper.h"
#include "bksafeuihandlerbase.h"

#include <iostream>
#include <fstream>

#include <map>
#include <vector>
#include "Wildcard.h"

typedef BOOL (*pCreateObject)( REFIID , void** );

#define IDC_UNINSTALL_LISTVIEW	11111
#define XX_MSG_DLG_WIDTH	300
#define XX_MSG_DLG_HEIGHT	100

//
// 手机必备开关
//
#define PHONE_NESS_SWITCH	1

#if PHONE_NESS_SWITCH
#		define	TAB_INDEX_TUIJIAN		0
#		define	TAB_INDEX_BIBEI			1
#		define	TAB_INDEX_DAQUAN		2
#		define	TAB_INDEX_UPDATE		3
#		define	TAB_INDEX_UNINSTALL		4
#		define	TAB_INDEX_PHONE			5
#		define	TAB_INDEX_GAME			6
#else
#		define	TAB_INDEX_TUIJIAN		-1
#		define	TAB_INDEX_BIBEI			0
#		define	TAB_INDEX_DAQUAN		1
#		define	TAB_INDEX_UPDATE		2
#		define	TAB_INDEX_UNINSTALL		3
#		define	TAB_INDEX_GAME			4
#		define	TAB_INDEX_PHONE			-1
#endif

#if PHONE_NESS_SWITCH
class CWebBrowserExternal : public CWHHtmlContainerWindow
{
public:
	HRESULT SetExternalDispatch(IDispatch* pDisp) 
	{ return m_wndIE.SetExternalDispatch(pDisp); }
};
#endif

//
// 已升级列表（防止重复提示升级）
//
class CSoftUpdatedList
{
public:
	void Startup(const CString &kSafePath);
	// 软件升级后添加
	void Add(int softId);
	// 判断是否已经升级
	BOOL InList(int softId);
	void Cleanup();

private:
	struct SoftInfo
	{
		int _id;
		__time32_t _time;

		SoftInfo() {_id = 0; _time = 0;}
		SoftInfo(int id, __time32_t time) {_id = id; _time = time;}
	};

private:
	CString _filePath;
	CComCriticalSection _csList;
	CAtlArray<SoftInfo> _softInfoList;
};

//下载管理参数结构体
struct threadParam
{
	void *pDlg;
	void *pData;
	int flag;	//0继续，1暂停，2取消, 3删除
};

struct softInfo 
{
	int nState; //状态：1暂停, 2取消
	int nProgress;
};

//安装队列的元素
struct installParam 
{
	CString strPath;
	CSoftListItemData *pData;
};


//记录卸载路径
typedef std::map<CString, int>	t_mapUnInitPath;
typedef t_mapUnInitPath::iterator	t_iterUnInitPath;


class CBeikeSafeMainDlg;
class CBeikeSoftMgrPowerSweepDlg;
class CBkSafeSoftMgrUnstDlg;

enum UniType
{
	UNI_TYPE_UNKNOW = 0,
	UNI_TYPE_ALL,
	UNI_TYPE_START,
	UNI_TYPE_QUICK,
	UNI_TYPE_DESKTOP,
	UNI_TYPE_PROCESS,
	UNI_TYPE_TRAY,
};
enum SoftActionFrom
{
	ACT_SOFT_FROM_DAQUAN = 0,
	ACT_SOFT_FROM_NECESS,
	ACT_SOFT_FROM_UPDATE,
};

class CBeikeSafeSoftmgrUIHandler
	:public ISoftMgrCallback,
	public ILstSoftUniExpandCallback,
	public ISoftMgrUpdateCallback,
	public IClickCallback,
	public IEditCallBack,
	public IHttpAsyncObserver,
	public CBkNavigator,
	public ksm::ISoftUnincallNotify,
	public ksm::ISoftInfoQueryNotify,
	public IKSAutoInstCallBack2,
	public CBkSafeUIHandlerBase
#if PHONE_NESS_SWITCH
	,public IDispatch
#endif
{
public:
	friend class CBeikeSafeSoftmgrNecessHandler;

public:
	CBeikeSafeSoftmgrUIHandler(CBeikeSafeMainDlg *pDialog)
		: m_pDlg(pDialog)
		, m_necessUIHandler(pDialog,this)
	{
		m_bInitInterface = FALSE;
		m_bCachExist = FALSE;
		m_bViewUninstall = FALSE;
		m_bLoadFail = FALSE;
		m_bInitData = FALSE;
		m_bViewDetail = FALSE;
		m_bInit = FALSE;
		m_bQuery = FALSE;
		m_bQueryUni = FALSE;
		m_nTab = 0;
		m_nDown = -1;
		m_nIndexUpdate = 0;
		m_hWndListUpdate = NULL;
		m_hDownIcon = (HANDLE)-1;
		m_hInitData = (HANDLE)-1;
		m_hDownload = (HANDLE)-1;
		m_strCurType = BkString::Get(IDS_SOFTMGR_8018);
		m_nCurType = 0;
		m_nCurTypeUni = 0;
		m_pSoftMgr = NULL;
		m_pSoftChecker = NULL;
		m_pUninstall = NULL;
		m_nCountUpdateIgnore = 0;
		m_bInitUpdate = FALSE;
		m_nSoftIdByCmd = 0;

		m_bInitInfoQuery = FALSE;
		m_pInfoQuery = NULL;

		m_bShowDownloaded	= FALSE;
		m_bViewAll			= FALSE;
		m_bCleaned			= FALSE;
		m_bViewDestTop = FALSE;

		m_bStartMonitor = FALSE;
		m_hUpdSoftInfoThread = NULL;
		m_hCheckInstallThread = NULL;
		m_hCheckUpdateThread = NULL;

		m_hSignLoadFinish = NULL;

		m_nUniBtnId = 0;

		m_bInitInstProxy = FALSE;

		m_pPowerSweepDlg = NULL;
		m_pImge = NULL;
		m_hInstall = (HANDLE)-1;
		m_font.Attach(BkFontPool::GetFont(BKF_DEFAULTFONT));

		m_arrCheckUpdate.RemoveAll();

		m_pSelSoftUniData = NULL;
		m_pSoftRubbishSweep = NULL;
		m_bInitUni = FALSE;
		m_dlgSoftMgrUninstall = NULL;
		m_nUniType = UNI_TYPE_ALL;
		m_bUniRefresh = TRUE;
		m_nUniLastClickHeaderIndex = -1;
		m_nDaquanLastClickHeaderIndex = -1;
		m_pDTManagerOneKey = NULL;

		CreateObject = NULL;
#if PHONE_NESS_SWITCH
		m_phoneLoaded = FALSE;
		m_recommendLoaded = FALSE;
		m_pPhoneDTMgr = NULL;
#endif
	}

	~CBeikeSafeSoftmgrUIHandler();

public:
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	LRESULT OnRefreshNetPage(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	CString GetSoftLibraryVersion();//得到软件库的版本信息
	CString GetUninstLibVersion();
	HRESULT GetKSafeDirectory(CString &strDir);//得到安装目录
	HRESULT GetKSoftDirectory(CString &strDir);//得到目录
	HRESULT GetKSoftDataDir(CString &strDir);//得到data目录
	HRESULT GetKSoftIconDir(CString &strDir);//得到icon目录
	void SetDownloadDetail();			//显示下载详情
	void OnListReSize(CRect rcWnd);		// 根据主窗口大小改变右侧listbox大小
	void InitDownloadDir();
	void Uninit();
	BOOL IsRecommend(CSoftListItemData *pData);
	void OnEditMouseWheelMsg(WPARAM wParam, LPARAM lParam);

	STATE_CLOSE_CHECK	CloseCheck();


	VOID _RefershItemBySoftIDMainThread(CString strSoftID);
	VOID _RefershItemBySoftIDWorkerThread(CString strSoftID);
	LRESULT OnRefershItemBySoftID(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnRefershRubbishInfo(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnRefershUniAllSoftFinish(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSoftmgrUEDataCompleted(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSoftmgrFreshTotal(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSoftmgrUESortByName(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSoftmgrRefreshUniType(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSoftmgrUEShowUniDataPage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSoftmgrUEHideHeaderArrow(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCheckSoftCanInstall(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnNotifySoftInstall(UINT uMsg, WPARAM wParam, LPARAM lParam);


	void OnViewDetail(DWORD dwIndex);	//查看详情	
	void OnViewNew(DWORD dwIndex);	//查看软件更新	
	void OnDownLoad(DWORD dwIndex);		//开始下载操作
	void _downLoadSoft(CSoftListItemData* pSoftData, SoftActionFrom nFrom = ACT_SOFT_FROM_DAQUAN, BOOL bUpdate = FALSE);
	void OnCancel(DWORD dwIndex);		//下载取消操作
	void _CancelDownLoad(CSoftListItemData* pSoftData, SoftActionFrom nFrom = ACT_SOFT_FROM_DAQUAN);
	void OnContinue(DWORD dwIndex);		//下载继续操作
	void _ContinueDownLoad(CSoftListItemData* pData, SoftActionFrom nFrom = ACT_SOFT_FROM_DAQUAN);
	void OnPause(DWORD dwIndex);		//下载暂停操作
	void _PauseDownLoad(CSoftListItemData* pData, SoftActionFrom nFrom = ACT_SOFT_FROM_DAQUAN);
	void OnDownLoadIcon(CString strID);	//下载ICON
	
	void OnFreebackFromList(DWORD dwIndex);//反馈
	void _FreebackFromList(CSoftListItemData* pData,SoftActionFrom nFrom = ACT_SOFT_FROM_DAQUAN);
	void OnTautilogyFromList(DWORD dwIndex);//重试
	void _TautilogyFromList(CSoftListItemData* pData,SoftActionFrom nFrom = ACT_SOFT_FROM_DAQUAN);
	void OnSwitchPage(DWORD dwPage);
	void OnDaquanSoftMark(DWORD dwIndex);
	void OnBiBeiSoftMark(CSoftListItemData* pData);
	void OnViewNewInfo(CSoftListItemData* pData);

	void OnUpdateViewDetail(DWORD dwIndex, HWND hWndList); //查看软件详情
	void OnUpdateViewNew(DWORD dwIndex, HWND hWndList); //查看软件更新
	void OnUpdateDownLoad(DWORD dwIndex, HWND hWndList);	//下载
	void OnBtnClick( DWORD dwIndex, int nType );
	void OnUpdateContinue(DWORD dwIndex, HWND hWndList);	//继续
	void OnUpdatePause(DWORD dwindex, HWND hWndList);	//暂停
	void OnUpdateCancel(DWORD dwIndex, HWND hWndList);	//取消
	void OnUpdateDownLoadIcon(CString strID);
	void OnUpdateFreebackFromList(DWORD dwIndex, HWND hWndList);
	void OnUpdateTautilogyFromList(DWORD dwIndex, HWND hWndList);
	void OnUpdateSoftMark(DWORD dwIndex, HWND hWndList);
	void OnUpdateIgnore(DWORD dwIndex, HWND hWndList);

	void OnUniExpandDownLoadIcon(CString strID);
	BOOL OnClickSoftUniExpand(DWORD nIndex, SOFT_UNI_INFO* pInfo);

	void OnClick( int nListId, CTypeListItemData * pData);//点击左侧list

	void OnEditEnterMsg(DWORD nEditId);	//查询软件回调

	BOOL OnHttpAsyncEvent( IDownload* pDownload, ProcessState state, LPARAM lParam ); //下载ICON回调接口

	// 卸载，软件数据事件
	virtual void __stdcall SoftDataEvent(ksm::UninstEvent event, ksm::ISoftDataEnum *pEnum);
	// 卸载，软件卸载事件
	virtual void __stdcall SoftDataSweepEvent(ksm::UninstEvent event, ksm::ISoftDataSweep *pSweep);
	// 卸载，快捷方式进程事件
	virtual void __stdcall SoftLinkEvent(ksm::UninstEvent event, ksm::SoftItemAttri type, ksm::ISoftLinkEnum *pData);
	// 卸载，残留项事件
	virtual void __stdcall SoftRubbishEvent(ksm::UninstEvent event, ksm::ISoftRubbishEnum *pEnum);
	// 卸载，残留项清扫事件
	virtual void __stdcall SoftRubbishSweepEvent(ksm::UninstEvent event, ksm::ISoftRubbishSweep *pSweep);

	virtual void __stdcall SoftQueryNotify(ksm::ISoftInfoEnum *pEnum, void *pParams);

	BOOL OnTabSoftMgrSelChange(int nTabItemIDOld, int nTabItemIDNew);//切换二级tab

	LRESULT OnSoftmgrRefreshBottom(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdateSoftCount(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
	//隐藏加载软件信息的提示（由于是多线程操作，必须通过消息来实现，否从出现刷新问题）
	LRESULT OnSoftmgrInitFinish(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
	LRESULT OnOnSoftmgrRefreshLoadTip(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam);
	LRESULT OnSetCheckListNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLoadUpdateCache(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLoadUpdateReal(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnUpdateUniType(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChangeUninstallEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnChangeDaquanEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnUniComboBoxSelect(WORD wNotifyCode, WORD wID, HWND hWndCtl);

	void OnUpdateCheckAll();		//升级页是否全选
	void OnUpdateBtnIgnore();	
	void OnUpdateDetailBack();
	void OnUpdateSelectSoft();
	void OnUpdateFromeDetail();

	void DeleteFileAfterTime();
	void OnBackFromQuery();
	void OnBack();
	void OnSetting();					//设置
	void OnRefresh();					//刷新
	void OnQuerySoft();					//查询软件
	void OnClearDaquanQeuryEdit();
	void OnClearUninstallQeuryEdit();
	void OnDownloadMgr();				//下载管理
//	void OnBatchDownSoft();				//下载勾选软件	
	void OnShowNoPlugSoft();				//选择所有软件
	void OnShowFreeSoft();		//只显示免费无插件软件
	void OnDownLoadFromeDetail();		//从下载页面点击下载
	void OnTransferFromQuery();			//没有查询到软件的时候跳转函数
	void OnTransferUninstall();			//没有查询到需要卸载的软件跳转函数

	void OnQueryUninstall();					//查询卸载

	BOOL InsertSoftInfo(CSoftListItemData *pData, BOOL bPlugCheck, BOOL bFreeCheck, BOOL& bQueryMark);
	void GetSoftStateFromCach(CString strFilePath); //从缓存中读取软件的信息
	void GetUpdateStateFromCache(CString strFilePath);	//从缓存中读取升级信息
	void UpdateCach();	//更新缓存文件
	void SaveUpdatePointOutInfo();
	void SaveDownloadSoftInfo(CString strSoftInfo);
	void SaveSoftInfoLog(CString strFilePath, CSimpleArray<CString>& arr);
	void LoadSoftInfoLog(CString strFilePath, CSimpleArray<CString>& arr);
	void SaveUpdateIgnoreList();
	void LoadUpdateIgnoreList();
	void ShowLoadInstallInfoTip();	//加载软件信息提示
	void OnTimer(UINT_PTR nIDEvent);	//定时器定时刷新底部下载信息
	void OnSize(UINT nType, CSize size);

	void OnUninstallQueryBack(BOOL bClearEditText = FALSE);
	void OnUninstallDetailBack();
	void OnUninstallOpenDir();
	void OnDetailUninstall();

	void OnBackFromInsQuery();

	void OnBtnUniAll();
	void OnBtnUniDesk();
	void OnBtnUniStartMenu();
	void OnBtnUniQuickLan();
	void OnBtnUniProcess();
	void OnBtnUniTray();

	void SortUninstallInfoBySpace( BOOL bIncrease );
	void SortUninstallInfoByTime( BOOL bIncrease );

	CString GetCurrentSystemVersion();	//判断当前系统的版本
	BOOL SelDir(CString& strPath);

	//初始化
	void Init();		

	void InitSwitch();			// 切换TAB的时候才init的操作
	void InitSlow();			// 启动比较慢的线程
	void InitLeftTypeList();
	void InitRightSoftList();
	void InitUninstallSoftList();
	BOOL InitInterface();
	void InitLinks();

	IDTManager*	GetDTManagerForOneKey();

	void	GetUninstallDataFromMap( CUninstallSoftInfo * pData, void * mp );
	void	LoadUninstallImage( CUninstallSoftInfo * pData );

	void ShowLoadingTip();
	void HideLoadingTip();
	void RollTheTip();
	void ShowUniDataPage();

	void GetSoftInfoByCondition();

	//读取软件数据
	void ShowItemsBySoftType(CString strSoftType);

	CSoftListItemData* GetDataBySoftID(CString strID);
	BOOL IsDataFilterByCheck(CSoftListItemData* pData, BOOL bFree, BOOL bPlug);
	BOOL ShowDataToList(CAtlList<CSoftListItemData*>& arr, BOOL bShowTypeInfo = FALSE);
	
	void LoadNecessData();	// 从sqlite里面读取所有的装机必备软件信息
	void LoadAllSoftData();//从sqlite里面读取所有的软件信息

	void ConvertMapToSoftItemData(CSoftListItemData* pData, CAtlMap<CString,CString>& soft);

	void ClickDaquanType(CString strNewType);
	int GetDaquanType(CString strType);
	void OnClickDaquanType(UINT uCmdId); 
	void OnClickXiezaiType(UINT uCmdId); 
	CString GetTypeUI(CString strTypeData);
	CString GetTypeData(CString strTypeUI);
	void LoadUniType(CSimpleArray<CString>& soft);
	void OnSubmitSoft(); //向金山卫士提交软件
	void OnFreeBack(); //反馈
	void GetConfig();	//下载卸载详情需要的INI文件
	void CheckDownloadState(); //检测下载连接的状态

	void OnReStart();	//重启卫士
	void OnIgnoreReStartTip();	// 设置不再提醒软件库更新需要重启卫士并关闭tip
	void OnCloseReStartTip();   // 关闭软件库更新需要重启卫士的tip

	void ShowPicSoftMgr();	//在加载信息的时候展示图片
	void OnBtnSwitchWatch(); //开关监控
	void OnLnkCurrentBack();

	void OnBtnResidualClear(); //立即清理残留项
	void OnBtnRubbishClear();
	void OnBtnSystemSlim();

	void OnBtnSmrDaquanSortByName(); 
	void OnBtnSmrDaquanSortBySize(); 
	void OnBtnSmrDaquanSortByTime(); 

	void OnBtnSmrUniSortByName(); 
	void OnBtnSmrUniSortBySize(); 
	void OnBtnSmrUniSortByTime(); 

	void OnSetIEMainhome();
	void OnSetDesktopLink();
	void UpdateDesktopIcon(int nNum);

	//读写缓存
	void GetDownloadCachInfo();
	void SetDownloadCachInfo();

	void FreshTotal();		// 刷新已安装文件总数， 以及系统盘可用空间
	void ShowUninstallInfoBar(BOOL bShow);
	void OnBtnSearchBack();	
	void OnUninstallBack(BOOL bClearEditText = FALSE);	
	void RefreshUniPage();
	void GetTheShowPic(int nIndex);     //加载软件信息的时候按照索引显示图片

	void ShowQueryAndRefreshBtn( BOOL bShow ); //是否显示排序按钮

	void CheckSoftInstallState(const CSimpleArray<CString>& strSoftIDS);

	static void _CheckSoftInstallStateProc(LPVOID lpParam);

	static void DownloadInfoCallBack(int64 cur,int64 all,int64 speed,DWORD time,void* para);

	static unsigned __stdcall ThreadInitDataProc(LPVOID lpVoid);//初始化数据线程函数
	static unsigned __stdcall ThreadProc(LPVOID lpVoid);//下载操作线程函数
	static unsigned __stdcall InstallProc(LPVOID lpVoid);//安装线程函数
	static unsigned __stdcall ThreadIconProc(LPVOID lpVoid);//下载ICON线程函数
	static unsigned __stdcall ThreadIniProc(LPVOID lpVoid);//下载INI文件
	static unsigned __stdcall ThreadDownloadProc(LPVOID lpVoid);//下载软件线程函数
	static unsigned __stdcall ThreadCheckInstallProc(LPVOID lpVoid); //检测软件安装
	static unsigned __stdcall ThreadUpdateSoftInfoProc(LPVOID lpVoid); //更新所有软件信息
	static unsigned __stdcall ThreadCheckUpateProc(LPVOID lpVoid);	//检测软件升级

	virtual HRESULT OnInstSoftComplete(HRESULT hr);

	virtual HRESULT BeforeInstall(const S_KSAI_SOFT_INFO* pstruSoftInfo);
	virtual HRESULT OnProgress(const S_KSAI_STATUS* pstruStatus);
	virtual HRESULT AfterInstall(const S_KSAI_INST_RESULT* pstruInstResult);

public:
	// 当文件存在的时候进行安装
	BOOL	_InstallSoftExistFile(CSoftListItemData* pData);
	BOOL	CheckSoftCanInstall(CSoftListItemData* pData, BOOL bFromDownOK);		// 当下载完毕的时候，是否可以进行安装

	//
	// 回传软件卸载分类信息
	//

	// 得到库版本
	CString GetLibVersion(LPCWSTR pLibPath);

	BOOL IsNeedUpdate(CSoftListItemData* pData);
	BOOL IsSearchInfo(SOFT_UNI_INFO * pInfo, CWildcard &wildcard);

	CBkNavigator* OnNavigate(CString &strChildName);

	void OnKeyDownReturn(MSG* pMsg);
	VOID QuerySoftMark(LPCTSTR lpstrSoftID);
	void AddToNewInst(LPCTSTR pszSoftId );

protected:
	BOOL _IsChildCtrlMessage(MSG* pMsg, HWND hParent);
	//
	// 通知体检软件更新数量发生变化
	//
	void NotifySoftUpdateCount(int cnt);

	void _UpdateSoftInfo();
	void _CheckSoftInstall();
	void _LoadPinYinLib();
	void _CheckSoftUpdate();
	void _ChangeArrayToList(CSimpleArray<CSoftListItemData*>& arrIn, CAtlList<CSoftListItemData*>& arrOut);
	void _SortArrayByOrder(CSimpleArray<CSoftListItemData*>& arrIn, CAtlList<CSoftListItemData*>& arrOut);
	void _SortArrayByLastUpdate(CSimpleArray<CSoftListItemData*>& arrIn, CAtlList<CSoftListItemData*>& arrOut);
	void _MoveMaijorFirst(CAtlList<CSoftListItemData*>& arrOut);
	SOFT_UNI_INFO * UniAddOrUpdateItemToList(ksm::SoftData& sd);
	CString GetSoftType(LONG nType);

	void _QuerySoftMark(LPCTSTR lpstrSoftID);
	void _LoadUpdate();

	void _ReloadTypeUni(BOOL bChangeType = TRUE);
	void _ReloadListUni();
	BOOL IsTestSoftPhone( CString& strUrl );

	void DoBibeiCmd(LPCTSTR lpstrCmd);
	void DoDaquanCmd(LPCTSTR lpstrCmd);
	void OnQuerySoftByID(LPCTSTR lpstrSoftId);

public:
	BOOL			m_bInitInterface;	//标志是否已经初始化加载softmgr.dll, 创建对象, 加载softmgr.dat, rank.dat
	BOOL			m_bInit;			// 标志是否已经初始化
	BOOL			m_bViewDetail;		//标识是否正在查看详情
	BOOL			m_bViewUninstall;	//查看卸载详情
	BOOL			m_bInitData;		//是否已经加载数据
	CString			m_strBibeiCmd;		//命令行跳转
	CString			m_strDaquanCmd;		//命令行跳转
	BOOL			m_bLoadFail;		//加载数据失败
	BOOL			m_bCachExist;		//缓存文件是否存在

	BOOL			m_bStartMonitor;	//监控是否已经开启

	BOOL			m_bCleaned;			
	int				m_nUniBtnId;

	CString			m_strErrorHtml;
	CString			m_strAllSoftKeyWord;
	CString			m_strUninstKeyWord;

	CBeikeSafeMainDlg *m_pDlg;			//主窗口指针
	Gdiplus::Image *m_pImge;			//缺省图片	

	Gdiplus::Image *m_pImageDir;
	Gdiplus::Image *m_pImageFile;
	Gdiplus::Image *m_pImageReg;

	CDetailSoftMgrDlg m_IconDlg;		//显示软件图标
	CDetailSoftMgrDlg m_IconUninstallSoft;	//卸载详情里面显示软件图标
	CDetailSoftMgrDlg m_IconUpdateSoft;		//升级详情里面显示软件图标

	CBeikeSoftMgrPowerSweepDlg * m_pPowerSweepDlg;

public:
	CSoftMgrQueryEdit				m_editUninstall;	//卸载查询控件
	CSoftMgrQueryEdit				m_edit;				//大全查询控件
	CSoftMgrListView				m_list;				//右侧显示软件信息listbix
	int								m_nDaquanLastClickHeaderIndex;
	CTypeListBox					m_SoftTypeList;		//左侧软件类型listbox

	CTypeListBox					m_TypeUni;		
	CSoftMgrUniExpandListView		m_UniExpandSoftList;
	CComboBox						m_comBoBoxUni;
	int								m_nUniLastClickHeaderIndex;
	UniType							m_nUniType;
	BOOL							m_bUniRefresh;
	ksm::ISoftRubbishSweep*			m_pSoftRubbishSweep;	//清理系统残留的控制接口（临时性）

	CSoftUpdatedList				m_softUpdatedList;
	CSoftMgrUpdateRollWnd			m_UpdateSoftList;
	CWHHtmlContainerWindow			m_IEGame;			// 热门游戏
	CWHHtmlContainerWindow			m_IEDetail;			//详情页面IE控件
	CWHHtmlContainerWindow			m_IEUninstallDetail;	// 卸载详情页面
	CWHHtmlContainerWindow			m_IEUpdateDetail;		// 升级详情页面
	CFont m_font;					//设置list字体
	CString m_strCurType;				//当前选择的软件分类
    int m_nCurType;
	CAtlMap<CString,void*>			m_arrTask;		//下载管理
	CAtlMap<void*, CString>			m_arrTaskMap;	//下载管理
	CAtlMap<CString, CString>		m_arrTypeMap; //从typeID到TypeName的映射

	CAtlMap<CString, softInfo*>		m_arrCachInfo;	//下载缓存信息

	CURLEncode url_encode;				
	BOOL	m_bQuery;					//是否顯示為搜索頁面
	BOOL	m_bQueryUni;				//是否顯示為搜索頁面
	int 	m_nDown;					//当前查看详情的软件
	CString	m_strDetailSoftID;			//当前查看详情的软件ID
	int		m_nIndexUpdate;				//升级页，查看详情软件索引
	CString	m_strUpdateDetailSoftID;	//升级页，查看详情软件ID
	HWND	m_hWndListUpdate;			//升级页，查看详情软件索引所在的列表控件
	HANDLE	m_hInstall;					//安装软件句柄
	HANDLE  m_hDownIcon;				//下载ICON线程句柄
	HANDLE  m_hInitData;				//初始化线程句柄
	HANDLE  m_hDownload;				//下载线程句柄
	int  m_nTab;						//当前tab
	CComCriticalSection		m_cs;		//临界区
	CComCriticalSection		m_csUpdCacheLock; //升级缓存锁
	CComCriticalSection		m_csUpdCheckLock; //升级检测锁	
	CLock					m_lockIconDown;		// 下载图标锁

	BOOL	m_bViewAll;					// 正在查看全部软件
	BOOL	m_bViewDestTop;				

	CAtlList<CSoftListItemData *>		m_arrDownIconData;		//需要下载ICON的软件
	CAtlMap<CString, CSoftListItemData*> m_arrDataMap;			//数据映射
	CSimpleArray<CSoftListItemData*>	m_arrData;              // 软件大全中所有软件信息
	CSimpleArray<CSoftListItemData*>	m_arrDataSub;           // 软件大全中当前分类软件信息
	CSimpleArray<CSoftListItemData*>	m_arrDataNew;           // 软件大全中最新更新分类软件信息（生成过程有点慢，牺牲内存提高性能，by 黄锐坤,2011-1-8)

	BOOL								m_bInitUni;
	CString								m_strCurTypeUni;	
	int									m_nCurTypeUni;
	CSimpleArray<SOFT_UNI_INFO*>		m_arrSoftUniDataSub;	//当前卸载软件
	CSimpleArray<SOFT_UNI_INFO*>		m_arrSoftUniData;		//所有可卸载软件
	SOFT_UNI_INFO*						m_pSelSoftUniData;		//当前选中的卸载信息的指针
	CAtlMap<CString, SOFT_UNI_INFO*>	m_arrSoftUniDataMap;	//卸载软件的列表key和数据映射
	CSimpleArray<SOFT_RUB_INFO*>		m_arrRubData;			//系统残留项目列表

	CSimpleArray<CString>				m_arrNewDown;			//记录新下载（非更新，非重新安装）
	CAtlList<installParam *>			m_arrInstall;			//下载后需要安装的软件---安装地址,数据
	CAtlList<CSoftListItemData *>		m_arrDownload;			//下载后需要安装的软件---安装地址,数据
	CSimpleArray<CSoftListItemData*>	m_arrCachSetup;			//缓存中记录的已安装软件的信息
	CSimpleArray<CSoftListItemData*>	m_arrCheckSetup;		//检测后发现已安装的软件
	CSimpleArray<CSoftListItemData*>	m_arrUpdCache;			//升级缓存		
	CAtlList<CSoftListItemData*>		m_listUpdTemp;			//升级检测线程的软件队列
	CSimpleArray<CSoftListItemData*>	m_arrCheckUpdate;		//检测后发现可以升级的软件
	BOOL								m_bInitUpdate;
	int									m_nCountUpdateIgnore;	
	CSimpleArray<CString>				m_arrIgnore;			//忽略更新ID
	int									m_nSoftIdByCmd;			//命令行过来要求升级的软件ID;

	BOOL								m_bShowDownloaded;		// 标志是否打开“查看详情”时跳到“已下载”
	CSoftItemData						m_ItemData;		//加快查询速度

	//软件相关接口
	HINSTANCE				hInstance;
	pCreateObject			CreateObject;

	ISoftManager*			m_pSoftMgr;			
	ISoftCheckerEx*			m_pSoftChecker;
	IDTManager*				m_pDTManager;
	IDTManager*				m_pDTManagerOneKey;		// 一键装机的

	IDownload *				m_pDownload;
	ksm::ISoftUninstall2*	m_pUninstall;

	BOOL					m_bInitInfoQuery;
	ksm::ISoftInfoQuery*	m_pInfoQuery;

	CBkDialogView			m_dlgTip;
	CBkDialogView			m_SoftListTip;

	CBkSafeSoftMgrUnstDlg*	m_dlgSoftMgrUninstall;	// 软件卸载对话框

	CBeikeSafeSoftmgrNecessHandler	m_necessUIHandler;
	ksm::CSoftInstProxy				m_softInstEng;
	BOOL							m_bInitInstProxy;

#if PHONE_NESS_SWITCH
	//
	// 手机必备
	//
private:
	enum PhoneDownState
	{
		PDS_NONE	= 0,
		PDS_INIT,
		PDS_DOWNING,
		PDS_DONE,
		PDS_PAUSED,
		PDS_ERROR_MD5,
		PDS_ERROR,
	};

	typedef struct PhoneSoft
	{
		void*		idDown;		// 下载标识
		PhoneDownState
					state;		// 下载状态
		ULONG		speed;		// 速度
		ULONG		recved;		// 已下载字节数
	} *PPhoneSoft;

	typedef CAtlMap<void*, LONG> Dt2IdMap;
	typedef CAtlMap<LONG, PhoneSoft> Id2PhoneSoftMap;

	typedef Dt2IdMap::CPair* Dt2IdIter;
	typedef Id2PhoneSoftMap::CPair* Id2PhoneSoftIter;

	typedef CComCritSecLock<CComCriticalSection> CriticalSectionScoped;

private:
	BOOL				m_phoneLoaded;
	BOOL				m_recommendLoaded;
	IDTManager			*m_pPhoneDTMgr;
	CWebBrowserExternal	m_IESoftRecommend;
	CWebBrowserExternal	m_IEPhoneNess;
	CComCriticalSection	m_csPhoneNess;
	Dt2IdMap			m_dt2Id;
	Id2PhoneSoftMap		m_id2PhoneSoft;

public:
	// 获取下载接口（仅在必要时才创建下载管理对象）
	IDTManager* GetDTMgrForPhone();
	static void PhoneSoftDownInfoCallback(__int64 cur,__int64 all,__int64 speed,DWORD time,void* para);
	static void PhoneSoftDownCallback(DTManager_Stat st,void* tk,void* para);
	void PhoneSoftDownProcess(DTManager_Stat st, void *tk);
	// Js接口
	HRESULT PhoneSoftExists(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PhoneStartDown(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PhonePauseDown(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PhoneResumeDown(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PhoneStopDown(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PhoneGetDownState(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT PhoneOpenDownedSoft(DISPID, DISPPARAMS*, VARIANT*);

	HRESULT PhoneNavigatePage(DISPID, DISPPARAMS*, VARIANT*);
	HRESULT ReportInfo(DISPID, DISPPARAMS*, VARIANT*);

private:
	// window.external实现接口
	STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
	STDMETHOD_(ULONG, AddRef)() { return 1; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) { return E_NOTIMPL; }
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) { return E_NOTIMPL; }
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);
#endif

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_REALWND_RESIZED(IDC_LIST_SOFTMGR, OnListReSize)
		BK_NOTIFY_ID_COMMAND(IDC_UPDATE_CHECKBOX_ALL, OnUpdateCheckAll)
		BK_NOTIFY_ID_COMMAND(IDC_UPDATE_IGNORE_TEXT, OnUpdateBtnIgnore)
		BK_NOTIFY_ID_COMMAND(IDC_UPDATE_VIEW_IGNORE, OnUpdateBtnIgnore)
		BK_NOTIFY_ID_COMMAND(IDC_UPDATE_BACK_DETAIL, OnUpdateDetailBack)
		BK_NOTIFY_ID_COMMAND(IDC_UPDATE_SELECT_SOFT, OnUpdateSelectSoft )
		BK_NOTIFY_ID_COMMAND(IDC_UPDATE_BTN_DETAIL, OnUpdateFromeDetail)
		BK_NOTIFY_ID_COMMAND(IDC_DOWNLOADMGR_SOFTMGR, OnDownloadMgr)
		BK_NOTIFY_ID_COMMAND(IDC_CHECKBOX_DAQUAN_NOPLUGSOFT, OnShowNoPlugSoft)
		BK_NOTIFY_ID_COMMAND(IDC_CHECKBOX_DAQUAN_FREESOFT, OnShowFreeSoft)
		BK_NOTIFY_ID_COMMAND(IDC_SETTING_SOFTMGR, OnSetting)
		BK_NOTIFY_ID_COMMAND(IDC_REFRESH_SOFTMGR, OnRefresh)
		BK_NOTIFY_ID_COMMAND(IDC_QUERY_SOFTMGR, OnQuerySoft)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_BTN2_QUERY, OnQuerySoft)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_DAQUAN_BTN_QEURY_EDIT, OnClearDaquanQeuryEdit)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_UNISTALL_BTN_QEURY_EDIT, OnClearUninstallQeuryEdit)

		BK_NOTIFY_ID_COMMAND(IDS_BACK_DETAIL_SOFTMGR, OnBack)
		BK_NOTIFY_ID_COMMAND(IDC_BACK_QUERY_SOFTMGR, OnBackFromQuery)
		BK_NOTIFY_ID_COMMAND(IDC_BACK_QUERY_SOFTMGR_ICO, OnBackFromQuery)
		BK_NOTIFY_ID_COMMAND(IDC_BACK_QUERY_UNI_SOFTMGR, OnBtnSearchBack)
		BK_NOTIFY_ID_COMMAND(IDC_BACK_QUERY_UNI_SOFTMGR_ICO, OnBtnSearchBack)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SEARCH_GOBACK, OnBackFromInsQuery)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_GOBACK, OnUninstallDetailBack)
		BK_NOTIFY_ID_COMMAND(IDC_LNK_CUR_PAGE, OnLnkCurrentBack)
		BK_NOTIFY_ID_COMMAND( IDC_LINK_UNINSTALL_OPENDIR, OnUninstallOpenDir )
		BK_NOTIFY_ID_COMMAND(IDC_DOWN_BTN_DETAIL_SOFTMGR, OnDownLoadFromeDetail)
		BK_NOTIFY_ID_COMMAND(IDC_FREEBACK_SOFTMGR, OnFreeBack)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_UNINSTALL_DO, OnDetailUninstall)
		BK_NOTIFY_ID_COMMAND(IDS_LINK_TIP2_SOFTMGR, OnTransferFromQuery)
		BK_NOTIFY_ID_COMMAND(IDC_TXT_SEARCH_CON3, OnTransferUninstall)
		BK_NOTIFY_ID_COMMAND(IDS_LINK_SUBMIT_SOFTMGR, OnSubmitSoft)
		BK_NOTIFY_ID_COMMAND( IDC_BTN_UNI_ALL, OnBtnUniAll )
		BK_NOTIFY_ID_COMMAND( IDC_BTN_UNI_DESK, OnBtnUniDesk )
		BK_NOTIFY_ID_COMMAND( IDC_BTN_UNI_STARTMENU, OnBtnUniStartMenu )
		BK_NOTIFY_ID_COMMAND( IDC_BTN_UNI_QUICKLAN, OnBtnUniQuickLan )
		BK_NOTIFY_ID_COMMAND( IDC_BTN_UNI_PROCESS, OnBtnUniProcess )
		BK_NOTIFY_ID_COMMAND( IDC_BTN_UNI_TRAY, OnBtnUniTray )
		BK_NOTIFY_ID_COMMAND( IDC_LNK_WATCH_LASTUSE, OnBtnSwitchWatch)
		BK_NOTIFY_ID_COMMAND( IDC_DLG_RESIDUAL_CLEAR, OnBtnResidualClear)
		BK_NOTIFY_ID_COMMAND( IDC_LINK_RUBBISH_CLEAR, OnBtnRubbishClear)
		BK_NOTIFY_ID_COMMAND( IDC_LINK_SYSTEM_SLIM, OnBtnSystemSlim)

		BK_NOTIFY_ID_COMMAND(IDC_SOFT_DAQUAN_LEFT_RESTART,OnReStart)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_DAQUAN_LEFT_IGNORE_VER_TIP,OnIgnoreReStartTip)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_DAQUAN_LEFT_CLOSE_VER_TIP,OnCloseReStartTip)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_UPDATE_LEFT_RESTART,OnReStart)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_UPDATE_LEFT_IGNORE_VER_TIP,OnIgnoreReStartTip)
		BK_NOTIFY_ID_COMMAND(IDC_SOFT_UPDATE_LEFT_CLOSE_VER_TIP,OnCloseReStartTip)

		BK_NOTIFY_ID_COMMAND_EX(IDC_SOFTMGR_DAQUAN_TYPE_FIRST, IDC_SOFTMGR_DAQUAN_TYPE_LAST, OnClickDaquanType)
		BK_NOTIFY_ID_COMMAND_EX(IDC_SOFTMGR_XIEZAI_TYPE_FIRST, IDC_SOFTMGR_XIEZAI_TYPE_LAST, OnClickXiezaiType)

		BK_NOTIFY_ID_COMMAND( IDC_SMR_DAQUAN_HEADER_NAME, OnBtnSmrDaquanSortByName)
		BK_NOTIFY_ID_COMMAND( IDC_SMR_DAQUAN_HEADER_SIZE, OnBtnSmrDaquanSortBySize)
		BK_NOTIFY_ID_COMMAND( IDC_SMR_DAQUAN_HEADER_TIME, OnBtnSmrDaquanSortByTime)
		BK_NOTIFY_ID_COMMAND( IDC_SMR_UNI_HEADER_NAME, OnBtnSmrUniSortByName)
		BK_NOTIFY_ID_COMMAND( IDC_SMR_UNI_HEADER_SIZE, OnBtnSmrUniSortBySize)
		BK_NOTIFY_ID_COMMAND( IDC_SMR_UNI_HEADER_TIME, OnBtnSmrUniSortByTime)
		BK_NOTIFY_ID_COMMAND(IDC_SET_IE_MAINHOME_SOFTMGR, OnSetIEMainhome)
		BK_NOTIFY_ID_COMMAND(IDC_SET_DESKTOP_LINK_SOFTMGR, OnSetDesktopLink)
		BK_NOTIFY_ID_COMMAND( IDC_UNINSTALL_QUERYBACK_LNK, OnUninstallDetailBack )
		BK_NOTIFY_TAB_SELCHANGE(IDC_TAB_SOFTMGR, OnTabSoftMgrSelChange)

		BK_NOTIFY_ID_COMMAND(IDC_SOFT_UNISTALL_BTN2_QEURY_EDIT, OnQueryUninstall)

		BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CBeikeSafeSoftmgrUIHandler)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP_MEMBER(m_necessUIHandler)
		COMMAND_HANDLER(IDC_SOFT_UNISTALL_EDT_QEURY_EDIT, EN_CHANGE, OnChangeUninstallEdit)
		COMMAND_HANDLER(IDC_EDIT_SOFTMGR, EN_CHANGE, OnChangeDaquanEdit)
		COMMAND_HANDLER_EX(IDS_CLASS_COMBO_UNI_SOFTMGR, CBN_SELCHANGE, OnUniComboBoxSelect)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_SIZE(OnSize)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)

		MESSAGE_HANDLER(WM_NET_ERROR_REFRESH, OnRefreshNetPage)

		MESSAGE_HANDLER_EX(WM_BOTTOM_REFRESH, OnSoftmgrRefreshBottom)
		MESSAGE_HANDLER_EX(WM_LOADTIP_REFRESH, OnOnSoftmgrRefreshLoadTip)
		MESSAGE_HANDLER_EX(WM_SOFT_INIT_FINISH, OnSoftmgrInitFinish)
		MESSAGE_HANDLER(WM_USER_SET_CHECK_LIST, OnSetCheckListNotify)
		MESSAGE_HANDLER(MSG_APP_LOAD_UPDATE_CACHE, OnLoadUpdateCache)
		MESSAGE_HANDLER(MSG_APP_LOAD_UPDATE_REAL, OnLoadUpdateReal)
		MESSAGE_HANDLER(MSG_APP_UPDATE_UNI_TYPE, OnUpdateUniType)
		MESSAGE_HANDLER_EX(WM_UPDATE_SOFT_COUNT, OnUpdateSoftCount)
		MESSAGE_HANDLER_EX(WM_SOFT_REFRESH_ITEM, OnRefershItemBySoftID)
		MESSAGE_HANDLER_EX(WM_SOFT_REFRESH_CANLIU,OnRefershRubbishInfo)
		MESSAGE_HANDLER_EX(WM_SOFT_UNI_ALL_REFRESH_FINISH,OnRefershUniAllSoftFinish)
		MESSAGE_HANDLER_EX(WM_SOFT_UE_DATA_COMPLETED,OnSoftmgrUEDataCompleted)
		MESSAGE_HANDLER_EX(WM_SOFT_UE_HIDE_HEADER_ARROW,OnSoftmgrUEHideHeaderArrow)
		MESSAGE_HANDLER_EX(WM_SOFT_FRESH_TOTAL,OnSoftmgrFreshTotal)
		MESSAGE_HANDLER_EX(WM_SOFT_UE_SORT_BY_NAME,OnSoftmgrUESortByName)
		MESSAGE_HANDLER_EX(WM_SOFT_REFRESH_UNI_TYPE,OnSoftmgrRefreshUniType)
		MESSAGE_HANDLER_EX(WM_SOFT_UE_SHOW_UNIDATAPAGE,OnSoftmgrUEShowUniDataPage)
		MESSAGE_HANDLER_EX(WM_SOFT_INSTALL_CHECK,OnCheckSoftCanInstall)
		MESSAGE_HANDLER_EX(WM_SOFT_INSTALL_NOTIFY,OnNotifySoftInstall)
		END_MSG_MAP()

private:
	HANDLE	m_hUpdSoftInfoThread;		//获取软件信息的线程句柄
	HANDLE	m_hCheckInstallThread;		//检查软件安装的线程句柄
	HANDLE	m_hCheckUpdateThread;		//检查软件升级的线程句柄

	HANDLE	m_hSignLoadFinish;		//特征加载完成
};
