#pragma once
#include "bkres/bkres.h"
//#include "treelistctrlex.h"
#include "kpfw/netfluxdef.h"
#include <vector>
#include "common/KCheckInstance.h"
#include <wtlhelper/whwindow.h>
using namespace std;
#include "bklistbox.h"
#include "common/bksafelog.h"
#include "resource.h"
#include "kprocesslib/interface.h"
#include "netflowmonlistitemdata.h"
#include "netmonitorutility.h"
#include "communits/Locker.h"
#include "kflowstat.h"
#include "kispublic/klocker.h"
#include "kpfw/netstastic.h"
#include <map>
#include "kflowstatlist.h"
#include "kstatlimitdlg.h"


#define ID_TIMER_CHECK_EXIT  1
#define CHECK_EXIT_INTERVAL  500

#define ID_TIMER_UPDATE_NETFlOW_MON		2
#define ID_TIMER_UPDATE_STAT_INFO		3

#define ID_TIMER_REFRESH_FLOATWND_STATUS	4

// 表示多久获取一次数据, 驱动中是1s一次，这里改成更小也没意义
#define UPDATE_NETFLOW_MON_INTERVAL		    1000

#define MSG_NETMONITOR_LIMIT	WM_APP+0x100
//class KNetFlowMonList;
#define DEF_PAGE_COUNT			12


class KMainDlg
    : public CBkDialogImpl<KMainDlg>,
	public CWHRoundRectFrameHelper<KMainDlg>
{
public:
	enum enumUpLogType
	{
		enumUnknownProc = 0,
		enumUnknown = -1
	};
public:
    KMainDlg()
        : CBkDialogImpl<KMainDlg>(IDR_BK_NETMONITOR_DIALOG)
		, m_pNetMonitorListBox(NULL)
		, m_hNetMonitorMenu	(NULL)
		, m_hEventGetNetData(NULL)
		, m_nCurShowType(enumQueryTypeEveryDay)
		, m_pNetStatListBox(NULL)
		, m_hEventRptThreadExit(NULL)
		, m_hThreadReport(NULL)
		, m_bRptThreadWorking(FALSE)
    {
		m_FluxSys.nRecvSpeed = 0;
		m_FluxSys.nSendSpeed = 0;
		m_FluxSys.nTotalRecv = 0;
		m_FluxSys.nTotalSend = 0;
		
		m_arrColumWidth.SetCount(10);
		m_arrColumWidth[0] = 250;
		m_arrColumWidth[1] = 70;
		m_arrColumWidth[2] = 70;
		m_arrColumWidth[3] = 70;
		m_arrColumWidth[4] = 100;
		m_arrColumWidth[5] = 85;
		m_arrColumWidth[6] = 90;
		//m_arrColumWidth[7] = 60;
		m_arrColumWidth[7] = 40;
		m_arrColumWidth[8] = 0;
		m_arrColumWidth[9] = 0;

		for (int i = 0; i < 5; i++)
			m_nCurPage[i] = 1;
		

		m_nCurShowStr = 49;

		m_hDefaultIcon	   = NULL;
		//state
		_ResetArray();
		//::InitializeCriticalSection(&m_Lock);
		m_hEventGetNetData = CreateEvent(NULL, FALSE, TRUE, NULL);
		m_mapReported.RemoveAll();
		m_mapNeedReportProc.RemoveAll();

/*		m_nTotalRecvBegin = 0;
		m_nTotalSendBegin = 0;
		m_nTotalAllBegin = 0;
		m_nTimeWatchBegin = 0;
		m_nRecordEveryDays = 0;
		m_nRecordEveryMonth = 0;
		m_nTotalAllBeginForMonth = 0;
*/
		m_pShowStatLock = new KLockerCS();

    }

    ~KMainDlg();

	///////////////////////////////////////////////////////////////////////////////////////
	//公共使用部分

	void			BtnClose(void);
	void			BtnMin(void);
	void			BtnNetMonSwitch(void);
	void            BtnFloatWndSwitch(void);

	BOOL			OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/);
	void			OnSize(UINT nType, CSize size);

	void			OnSysCommand(UINT nID, CPoint pt);

	LRESULT			ShowUI(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT			ShowDay(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT			ShowMonth(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	// 如果驱动没有问题，返回true，如果加载失败返回false
	BOOL			CheckDriver();

	BOOL			CheckServericeIsOK();

	static BOOL		GetNetMonIsEnabled();

	//点击tab
	BOOL			OnBkTabMainSelChange(int nTabItemIDOld, int nTabItemIDNew);


	BOOL			IsFloatWndStartWithTray();
	void			OpenFloatWnd();
private:
	LRESULT			OnTimer(UINT timerId);
	LRESULT			OnDestroy();
	BOOL			_CheckDriverIsFailed();
	BOOL			_CheckForceDisableDriver();
	BOOL			_CheckHasUnCompatibleDriver();
	BOOL			_CheckTdxDriver();
	int				_IsVistaPlatform();
	BOOL			_CheckHasUnCompatibleSofware();
	BOOL			_CheckDriverVersion();

	BOOL			_IsDriverExist(LPCWSTR strDriverServiceName);
	DWORD			_GetServiceStatus(LPCWSTR strServiceName, int nMaxQueryMilliseconds);
	
	CLocker			m_locker;
	//公共使用部分end
	///////////////////////////////////////////////////////////////////////////////////////
	
    
	///////////////////////////////////////////////////////////////////////////////////////
	//流量监控部分
public:
	BOOL			_DisableDelayScan();

	// 2.0ui [11/29/2010 zhangbaoliang]
	//static CAtlArray<UINT>				m_arrColumState;//用来判读是否排序的时候使用需要排序0，升序1，降序
	//刷新LISTBOX中每行的数据
	LRESULT			OnListBoxGetDispInfo(LPNMHDR pnmh);

	//设置LISTBOX中行高的最大值
	LRESULT			OnListBoxGetmaxHeight(LPNMHDR pnmh);

	LRESULT			OnListBoxGetItemHeight(LPNMHDR pnmh);

	//LISTBOX中获取点击消息
	LRESULT			OnBkListBoxClickCtrl(LPNMHDR pnmh);

	//LISTBOX中获取右键点击消息
	LRESULT			OnBkListBoxRClickCtrl(LPNMHDR pnmh);

	BOOL			OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//开始自己写的排序，效率很低暂不使用
	static			bool SortNetInfo(KProcFluxItem& ItemData1, KProcFluxItem& ItemData2, int nColum, int nState);

	//使用STL的排序
	static			bool stl_SortNetInfo(KProcFluxItem& ItemData1, KProcFluxItem& ItemData2);

	//对应每一列的排序操作
	static			int  Compare( KProcFluxItem& ItemData1, KProcFluxItem& ItemData2, int nsubIndex = -1);



	//static			bool vetFind_if(KProcFluxItem& findItem);
	//一下都是点击列所产生的消息
	//每列默认默认状态时-1,0表示降序，1表示升序
	void			OnClickColumTitle();
	void			OnClickColumLevel();
	void			OnClickColumDownLoadSpeed();
	void			OnClickColumUpLoadSpeed();
	void			OnClickColumNetLimit();
	void			OnClickColumTotalDownload();
	void			OnClickColumTotalUp();
	void			OnClickColumPID();
	// end click colum
	void			OnClickNetSetting();
	void			OnClickNetLockOpen();
	void			OnClickNetLockClose();
	void			OnClickNetLimitText();
	void			OnClickOpenDir();

	//推荐调用网络测速
	void			OnStartNetSpeed();

	//对应的操作菜单
	LRESULT			OnNetMonitorCommand(UINT uMsg, WPARAM wParam, LPARAM lParam);

	//设置限速是弹出的限速窗口在关闭时发消息到这里
	LRESULT			OnNetMonitorLimitMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

	
	//悬浮窗设置
	void			OnClickWindowSetting(void);

	void			OnClickShowList1(void);
	void			OnClickShowList2(void);
	void			OnClickShowList3(void);
	void			OnClickShowList4(void);
	void			OnClickIsShowTip(void);

	void			_ShowListClick(IN int nCurPage);
	void			_ShowListInfo(void);

  	//设置右边“今日网络流量占用排行榜的”的相关东西
	BOOL			_SetFlowList(IN int nPos, IN const CString& strSrcFilePath, IN const CString& strNeedShowStr, IN const CString& strShowSize);

private:
	static CString _GetNeedShowData(IN ULONGLONG fData);

	// 更新流量汇总信息
	void _UpdateNetFlowSummaryWnd();

	//设置多少个进程正在访问网络
	void _SetAccessNetCount(IN int nCount);

	//设置下载总流量和上传总流量（传入的单位为B）
	void _SetDownAndUpdateSum(IN ULONGLONG uDownData, IN ULONGLONG uUpdata);

	//设置下载速度（传入单位为B）
	void _SetDownSpeed(IN ULONGLONG uDownSpeed);

	//设置上传速度（传入单位为B）
	void _SetUpSpeed(IN ULONGLONG uUpSpeed);

	// 打开悬浮窗
	void _OpenFloatWnd();

	// 关闭悬浮窗
	void _CloseFloatWnd();

	// 获取悬浮窗是否显示
	BOOL			_GetFloatWndDisplayStatus();
	BOOL			_GetIsAutoCloseFloatWndWhenDisableNetmon();
	BOOL			_SaveIsAutoCloseFloatwndWhenDisableNetmon(BOOL isClose);
	BOOL			_GetIsRememberAutoCloseFloatWndWhenDisableNetmon();
	BOOL			_SaveIsRememberAutoCloseFloatWndWhenDisableNetmon(BOOL isRemember);
	CString			_GetFloatWndConfigFilePath();

	void			_SetNetMonSwitchBtn(BOOL isEnabled);
	void			_SetNetMonSwitchHintText(BOOL isEnabled);

	BOOL			_EnableNetMon(BOOL isEnable);
	void			_InitNetMonSwitch();
	void			_InitFloatWndSwitch();
	void			_ShowFloatWnd(BOOL isShow);

	void			_OpenFlux();

	void			_HideOrShowTip(IN BOOL bIsShowTip, IN BOOL bIsNeedAct = TRUE);

private:
	//    WTL::CImageList         m_imagelist;
	// listView控件中的数据
	//    KNetFlowMonList*		m_pNetFlowMonList;

	UINT					m_uTimer;
	KPFWFLUX				m_FluxSys;// 系统总流量
	vector<KProcFluxItem>	m_processInfoList;// 保存进程的流量信息
	bool                    m_bFloatWndIsOpen;// 当前悬浮窗状态， true打开，false关闭
	HANDLE                  m_hEventExit; 
	HANDLE                  m_hEventChangeFlowatWndDisplayStatusText;    
	BOOL                    m_bNetMonEnabled;
	int                     m_nNetFlowListRefreshCountAfterDisable;
	HANDLE					m_hEventRptThreadExit;
	HANDLE					m_hThreadReport;
	BOOL					m_bRptThreadWorking;

	//新修改 2.0 [12/8/2010 zhangbaoliang]
	CBkNetMonitorListBox*							m_pNetMonitorListBox;
	HMENU											m_hNetMonitorMenu;		
	CAtlArray<UINT>									m_arrColumWidth;

	CAtlMap<DWORD, CNetMonitorCacheData>			m_mapProcNetData;			//对应LISTBOX中的每行数据
	CAtlMap<DWORD, CNetMonitorCacheData>			m_mapSpeedLimitProcNetData;	//暂未使用
	CRITICAL_SECTION								m_Lock;						//防止调用_RefreshArray导致数据重入
	HICON											m_hDefaultIcon;
	CAtlMap<CString, CNetMonitorCacheData>			m_mapNeedReportProc;
	CAtlMap<CString, CNetMonitorCacheData>			m_mapReported;
	int												m_nCurShowStr;
	enumProcessMode									m_enumProcessMode;


	//因为在现实数据之前要对数据进行处理，所以为了同步TIMER中的取数据和操作数据设置
	//有可能是出现的情况是我上次的数据还没有处理完，TIMER中有要开始这次取数据了，这样就容易引起我正在处理的数据发上变化
	//所以要保证我处理完了上次的数据才能取这次的数据
	HANDLE											m_hEventGetNetData;			
	//CAtlMap<DWORD, PFN>				m_mapMenuFunc;

	void					_ReleaseThread();
	// 创建显示列表 (LISTBOX)
	void					_InitNetMonitorListBox();

	//重置列表
	void					_ResetArray();

	//更新流量数据
	//包括对数据进行排序,如果数据个数合上一次有变化就更新列表函数，没有直接强制刷新数据
	void					_RefreshListBoxData(vector<KProcFluxItem>& vetNetData);

	//如果有需要排序，更新每一列的状态,包括排序图标消失等
	void					_SetColumState(int nColum);	

	//把获取到的数据缓存到一个本地map中
	//这个MAP是与现实列表相关联，每一项对应一行
	void					_CreateCacheData(vector<KProcFluxItem>& vetNetData, CAtlMap<DWORD, CNetMonitorCacheData>& mapCacheData);

	//调用Ipc通知驱动有限速,禁用等操作
	void					_NetSpeedLimit(CNetMonitorCacheData* pNetData);

	//调用限速功能是，调用这个函数显示限速窗口
	int						_CallNetLimitDialog(CNetMonitorCacheData& NetData);

	//当更新了某行的数据时，调用这个接口吧对应的现实列表中的数据更新缓存数据
	//更新，暂时没有使用
	void					_UpdateCacheData(CNetMonitorCacheData& NetData);

	//更新进程名
	int						_RefreshProcName(int nLeft, int nRight, KNetFlowMonListItemData ItemData);

	//更新进程描述
	int						_RefreshProcDes(int nLeft, int nRight, int nHeight, CNetMonitorCacheData NetData);

	//更新进程文件等级
	int						_RefreshProcLevel(int nLeft, int nRight, KNetFlowMonListItemData ItemData);

	//更新进程上传速度
	int						_RefreshProcUpSpeed(int nLeft, int nRight, KNetFlowMonListItemData ItemData);

	//更新进程下载速度
	int						_RefreshProcDownSpeed(int nLeft, int nRight, KNetFlowMonListItemData ItemData);

	//更新进程限速度
	int						_RefreshProcLimitSpeed(int nLeft, int nRight, KNetFlowMonListItemData ItemData);

	//更新进程总上传流量
	int						_RefreshProcTotalUp(int nLeft, int nRight, KNetFlowMonListItemData ItemData);

	//更新进程总下流量
	int						_RefreshProcTotalDown(int nLeft, int nRight, KNetFlowMonListItemData ItemData);

	//更新进程ID
	int						_RefreshProcID(int nLeft, int nRight, KNetFlowMonListItemData ItemData);

	//更新进程图标
	int						_RefreshProcICO(int nLeft, int nRight, KNetFlowMonListItemData ItemData);

	//更新进程路径
	int						_RefreshProcPath(int nLeft, int nRight, int nCurSel, KNetFlowMonListItemData ItemData);

	//更新进程操作图标
	int						_RefreshProcSetting(int nLeft, int nRight,KNetFlowMonListItemData ItemData);

	//////////////////////////////////////////////////////////////////////////
	//获取主程序路径
	CString					_GetAppPath();

	//////////////////////////////////////////////////////////////////////////
	//pid
	//int						_RefreshProcID(int nLeft, int nRight,KNetFlowMonListItemData ItemData);

	//流量监控部分end
	///////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////////
	//流量统计部分

	//点击按日显示之类的
	void			OnClickShowStatDays(void);
	void			OnClickShowStatWeeks(void);
	void			OnClickShowStatMonths(void);
	void			OnClickShowStatDefineSelf(void);

	//更改设置
	void			OnClickChangeSetting(void);

	//点击首页、上一页、下一页、尾页等操作
	void			OnClickShowFirstPage(void);
	void			OnClickShowBeforePage(void);
	void			OnClickShowNextPage(void);
	void			OnClickShowEndPage(void);

	//点击 “查看更多”
	void			OnClickShowMore(void);

private:

	//响应list消息
	LRESULT			OnStatListGetDispInfo(LPNMHDR pnmh);
	LRESULT			OnStatListGetmaxHeight(LPNMHDR pnmh);
	LRESULT			OnStatListGetItemHeight(LPNMHDR pnmh);
	
	//获取“时间”列的数据
	CString			_GetStatListInfoTime( IN CString strTimeMin, IN CString strTimeMax, IN int nMode);
	//获取“上传总流量”列的数据
	CString			_GetStatListInfoUpAllFlow(IN LONGLONG lUpAllFlow);
	//获取“下载总流量”列的数据
	CString			_GetStatListInfoDownAllFlow(IN LONGLONG lDownAllFlow);
	//获取“总流量”列的数据
	CString			_GetStatListInfoAllFlow(IN LONGLONG lAllFlow);
	//获取“监控时间”列的数据
	CString			_GetStatListMoniterTime(IN long lMoniterTime);

	BOOL			_GetCurLogInfo(IN enumQueryType enumType);
	BOOL			_GetCurLogInfo(IN enumQueryType enumType, IN CString strFrwo, IN CString strTo);
	BOOL			_GetCurLogInfo(OUT LONGLONG& uUpData, OUT LONGLONG& uDownData, OUT LONGLONG& uAllData);

	BOOL			_InitStatList(void);
	void			_ShowStatList(IN int nCode);
	CString			_GetEveryTime(IN long& lMoniterTime, IN long lBeCalcResidue,IN CString strUnit);

	//设置今日网络流量使用统计
	void			_SetFlowStatInfo(IN CString strAllFlow, IN CString strUpFlow, IN CString strDownFlow);
	void			_ShowPageForList();

	//获取当前数据
	void			_GetAndShowProcessInfo(void);

	//排序
	static bool		stl_SortPorcessList(KFluxStasticProcItemEx& ItemData1, KFluxStasticProcItemEx& ItemData2);

	void			_ShowRemindInfo(void);
	void			_GetRemindInfo(OUT int& nDay, OUT int& nMonth);
	void			_GetRemindMarkInfo( OUT int& nDayMark, OUT int& nMonthMark );
	void			_GetRemindLastData( OUT int& nDayData, OUT int& nMonthData );

	void			_InitFlowRemindInfo(void);
	int				_GetHadWaitTenMuniters(void);
	//是否显示流量监控统计提醒对话框
	void			_ShowNetMintorRemindDlg(IN int nIsInit = FALSE);

	void			_ShowDialogRemind(IN int nMode, IN int nSetSize, int nIsInit = FALSE);

	//显示流量统计的第二行内容
	void			_SetStatFlowShow(void);

	//从数据库中读取数据
	BOOL			_GetAllDataFromDB(IN enumQueryType enumType, IN CString strCurTime, OUT LONGLONG& nRet);

	//
	BOOL			_SetDelayTenMuniter(void);
	BOOL			_HadWaitTenMuniters(void);
	long			_GetFormatTime(int nYear, int nMonth, int nDay);

	void			_ReSetAddData(IN ULONGLONG nTotalRecv, IN ULONGLONG nTotalSend, IN ULONGLONG nTotalAll, IN __int64 nTimeWatch);
	void			_AddNewData(void);
private:
/*
	LONGLONG					m_nTotalRecvBegin;
	LONGLONG					m_nTotalSendBegin;
	LONGLONG					m_nTotalAllBegin;
	__int64						m_nTimeWatchBegin;
	__int64						m_nRecordTimeBegin;
	LONGLONG					m_nRecordEveryDays;
	LONGLONG					m_nRecordEveryMonth;
	LONGLONG					m_nTotalAllBeginForMonth;
*/
	KStatListDlg				m_statListdlg;
//	KNetFluxStasticCacheReader	m_fluxStatRead;
//	KStasticFluxProcessList*	m_pFluxStatRead;
	int							m_nCurShowType;
	int							m_nCurPage[5];

	KFlowStat					m_FlowStatLog;
	CBkNetMonitorListBox*		m_pNetStatListBox;
	struct stLogListInfo 
	{
		CString strMinTime;
		CString strMaxTime;
		LONGLONG lUpFlow;
		LONGLONG lDownFlow;
		LONGLONG lAllFlow;
		LONGLONG lAllTime;
		void Init(void)
		{
			strMinTime = _T("");
			strMaxTime = _T("");
			lUpFlow = 0;
			lDownFlow = 0;
			lAllFlow = 0;
			lAllTime = 0;
		}
	};

	std::vector<stLogListInfo> m_vLogListInfo;
	ILockerMode*			   m_pShowStatLock;
	std::vector<KFluxStasticProcItemEx> m_vFlowStatPList;
	CString						m_strQueryBegin;
	CString						m_strQueryEnd;
	typedef std::pair<CString, KFluxStasticProcItemEx> StatMapPair;
	std::map<CString, KFluxStasticProcItemEx>	m_statMap;

	//流量统计部分end
	///////////////////////////////////////////////////////////////////////////////////////
	
public:
    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		//公共部分
		BK_NOTIFY_ID_COMMAND(DEFCLOSEMAINBUTTON, BtnClose)
		BK_NOTIFY_ID_COMMAND(DEF_MIN_BTN, BtnMin)
		BK_NOTIFY_TAB_SELCHANGE(TAB_MAIN, OnBkTabMainSelChange)
		//公共部分end

		//流量监控部分
		BK_NOTIFY_ID_COMMAND(DEF_NETFLOW_SWITCH, BtnNetMonSwitch)
		BK_NOTIFY_ID_COMMAND(DEF_FLOATWND_SWITCH, BtnFloatWndSwitch)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_219, OnClickColumTitle)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_220, OnClickColumLevel)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_221, OnClickColumDownLoadSpeed)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_222, OnClickColumUpLoadSpeed)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_223, OnClickColumNetLimit)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_224, OnClickColumTotalDownload)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_225, OnClickColumTotalUp)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_226, OnClickColumPID)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_211, OnClickNetSetting)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_229, OnClickNetLockOpen)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_230, OnClickNetLockClose)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_207, OnClickNetLimitText)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_216, OnClickOpenDir)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_239, OnStartNetSpeed)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_242, OnClickShowList1)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_244, OnClickShowList2)	
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_246, OnClickShowList3)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_249, OnClickIsShowTip)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_307, OnClickShowList4)
		
	//	BK_NOTIFY_ID_COMMAND(DEV_VIEW_DETAIL, OnClickWindowSetting)
		//流量监控部分end

		//流量统计部分
		BK_NOTIFY_ID_COMMAND(TAB_SHOW_STAT_DAYS, OnClickShowStatDays)
		BK_NOTIFY_ID_COMMAND(TAB_SHOW_STAT_WEEKS, OnClickShowStatWeeks)	
		BK_NOTIFY_ID_COMMAND(TAB_SHOW_STAT_MONTHS, OnClickShowStatMonths)
		BK_NOTIFY_ID_COMMAND(TAB_SHOW_STAT_DEFINESELF, OnClickShowStatDefineSelf)
		BK_NOTIFY_ID_COMMAND(DEF_NETFLOW_SWITCH2, BtnNetMonSwitch)
		BK_NOTIFY_ID_COMMAND(TAB_SHOW_STAT_CSETTING, OnClickWindowSetting)
		BK_NOTIFY_ID_COMMAND(DEF_SHOW_FIRST, OnClickShowFirstPage)
		BK_NOTIFY_ID_COMMAND(DEF_SHOW_BEFORE, OnClickShowBeforePage)	
		BK_NOTIFY_ID_COMMAND(DEF_SHOW_NEXT, OnClickShowNextPage)
		BK_NOTIFY_ID_COMMAND(DEF_SHOW_ENDP_PAGE, OnClickShowEndPage)
		BK_NOTIFY_ID_COMMAND(DEF_SHOW_MORE, OnClickShowMore)
		//流量统计部分end
    BK_NOTIFY_MAP_END()


    BEGIN_MSG_MAP_EX(KMainDlg)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<KMainDlg>)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		//公共部分
		MSG_WM_SIZE(OnSize)
		CHAIN_MSG_MAP(CBkDialogImpl<KMainDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		MESSAGE_HANDLER(WM_UPLIVE_SHOW, ShowUI )
		MESSAGE_HANDLER(WM_SHOW_RUN1, ShowDay )
		MESSAGE_HANDLER(WM_SHOW_RUN2, ShowMonth )
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MESSAGE_HANDLER_EX(WM_COMMAND,		OnNetMonitorCommand)

		//流量监控部分
		NOTIFY_HANDLER_EX(NET_MONITOR_201, BKLBM_GET_DISPINFO, OnListBoxGetDispInfo)
		NOTIFY_HANDLER_EX(NET_MONITOR_201, BKLBM_CALC_MAX_HEIGHT, OnListBoxGetmaxHeight)
		NOTIFY_HANDLER_EX(NET_MONITOR_201, BKLBM_CALC_ITEM_HEIGHT, OnListBoxGetItemHeight)
		NOTIFY_HANDLER_EX(NET_MONITOR_201, BKLBM_ITEMCLICK, OnBkListBoxClickCtrl)
		NOTIFY_HANDLER_EX(NET_MONITOR_201, BKRBM_ITEMCLICK, OnBkListBoxRClickCtrl)
		MESSAGE_HANDLER_EX(MSG_NETMONITOR_LIMIT, OnNetMonitorLimitMsg)

		//流量统计部分
		NOTIFY_HANDLER_EX(TAB_SHOW_STAT_WINDOW, BKLBM_GET_DISPINFO, OnStatListGetDispInfo)
		NOTIFY_HANDLER_EX(TAB_SHOW_STAT_WINDOW, BKLBM_CALC_MAX_HEIGHT, OnStatListGetmaxHeight)
		NOTIFY_HANDLER_EX(TAB_SHOW_STAT_WINDOW, BKLBM_CALC_ITEM_HEIGHT, OnStatListGetItemHeight)

		REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
	
};
