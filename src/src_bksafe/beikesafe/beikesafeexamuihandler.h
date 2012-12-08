#pragma once
#include <iefix/ldlistviewctrl.h>
#include "kws/KTimingCheckSecurityCloudTask.h"
#include "common/bksafelog.h"
#include "communits/ProcessSharedData.h"

extern CRITICAL_SECTION g_StartExam_cs;

class CBeikeSafeMainDlg;


#if 0
class LastTrojanScan
{
public:
	BOOL bScanned;
	DWORD dwScanMode;
	SYSTEMTIME stStartLast;
	SYSTEMTIME stStartFast;
	SYSTEMTIME stStartFull;
	SYSTEMTIME stStartCustom;
	DWORD dwVirusCount;
	DWORD dwRemainVirusCount;
	LastTrojanScan()
	{
		bScanned = FALSE;
	}
	LastTrojanScan(BOOL bScanned, DWORD dwScanMode, SYSTEMTIME stStartLast, 
		SYSTEMTIME stStartFast, SYSTEMTIME stStartFull, 
		SYSTEMTIME stStartCustom, DWORD dwVirusCount, 
		DWORD dwRemainVirusCount)
	{
		this->bScanned = bScanned;
		this->dwScanMode = dwScanMode;
		this->stStartLast = stStartLast;
		this->stStartFast = stStartFast;
		this->stStartFull = stStartFull;
		this->stStartCustom = stStartCustom;
		this->dwVirusCount = dwVirusCount;
		this->dwRemainVirusCount = dwRemainVirusCount;
	}
};
struct UI_CTRL_EX
{
	ULONG uCtrlID;
	ULONG uExParam;
	UI_CTRL_EX()
	{
		uCtrlID = -1;
		uExParam = -1;
	}
};
struct UI_TYPE_STRUCT
{
	ULONG uUIType;//现在默认情况下窗口分为5中，1,IE修复精简窗口 2，IE修复复杂窗口 3，IE设置 4，IE清理简单窗口 5，IE修复复杂窗口
	CSimpleArray<UI_CTRL_EX> arrCtrlParam;//每一种窗口模式下，存在的控件ID
};

#endif

class CBeikeSafeExamUIHandler
	: public CBkNavigator
	, public CBkSafeUIHandlerBase
{
public:
	class EXAM_ITEM
	{
	public:
		EXAM_ITEM(DWORD a, LPCTSTR b, LPCTSTR c)
		{
			_dwItemID       = a;
			_pszItemCaption = b;
			_pszInitState   = c;
		}

		DWORD _dwItemID;
		CString _pszItemCaption;
		CString _pszInitState;
	};
	class EXAM_SCAN_RESULT
	{
	public:
		EXAM_SCAN_RESULT(DWORD a, LPCTSTR b, LPCTSTR c, LPCTSTR d, LPCTSTR e, 
			LPCTSTR f, LPCTSTR g, LPCTSTR h,
			LPCTSTR i = NULL , LPCTSTR j = NULL, LPCTSTR k = NULL)
		{
			_dwItemID					= a;
			_strDefaultCaption			= b == NULL ? L"" : b;	//default
			_strSafeOperate				= c == NULL ? L"" : c;
			_strDangerItemCaption		= d == NULL ? L"" : d;	//danger
			_strDangerDes				= e == NULL ? L"" : e;
			_strDangerOperate			= f == NULL ? L"" : f;
			_strCancelCaption			= g == NULL ? L"" : g;
			_strCancelOperate			= h == NULL ? L"" : h;	//cancel
			_strErrorCaption			= i == NULL ? L"" : i;	//error
			_strErrorOperate			= j == NULL ? L"" : j;
			_strErrorDes				= k == NULL ? L"" : k;

		}

		DWORD   _dwItemID;				//体检项目ID
		CString _strDefaultCaption;		//体检项目标题
		CString _strSafeOperate;		//体检项目安全提示的操作
		CString _strDangerItemCaption;	//危险项目显示的文字
		CString _strDangerOperate;		//体检项目危险提示的操作
		CString _strDangerDes;			//危险信息描述
		CString _strCancelCaption;		//体检被取消的提示文字
		CString _strCancelOperate;		//每次取消时的推荐操作
		CString _strErrorCaption;		//体检时失败提示的文字
		CString _strErrorOperate;		//体检失败是提示的操作
		CString _strErrorDes;
		//		CString _pszRisk_OOperate;		//体检项目危险提示的操作-->历史遗留
	};
	// 	enum enumLogState
	// 	{
	// 		scaned = 0,						//没有扫描过
	// 		days30,							//360没有进行过扫描
	// 		days7,							//7没有进行扫描
	// 		trojan,							//发现木马
	// 		upload,							//有上传文件日志
	// 		installing,
	// 		kill,
	// 		unknown							//未知状态
	// 	};
	//区分不同的统计信息使用
	enum enumUpLogType
	{
		installeng = 0,					//安装本地引擎->0
		scantrojan,						//查杀木马->1
		startexam,						//开始体检->2
		reexam,							//重新体检->3
		noautoexam,						//取消自动体检->4
		examtime,						//体检完成->5
		unknownhosts,					//->6
		examresult,						//->7
		examfixitem,					//->8
		examautostart,					//->9
		examechofalied,					//->10
		homepageopt,					//11->首页推荐功能
	};

	class SAFE_MONITOR_ITEM
	{
	public:
		SAFE_MONITOR_ITEM(DWORD a, LPCTSTR b)
		{
			dwItemID    = a;
			strCaption  = b;
		}

		DWORD dwItemID;
		CString strCaption;
	};

public:
	CBeikeSafeExamUIHandler(CBeikeSafeMainDlg *pDialog)
		: m_pDlg(pDialog)
		, m_nShowProgress(0)		//当前显示的进度
		, m_nRealProgress(0)
		, m_bExamFinished(FALSE)	//体检结束
		, m_hStartExamEvent(NULL)	//开始体检事件
		, m_bEcho(TRUE)				//握手
		, m_bUpdateLog(FALSE)		
		//		, m_logState(unknown)
		, m_strDangerIDs(TEXT(""))	//扫描到的危险项的ID，回传日志试用合格
		, m_examType(examautostart) //体检类型
		, m_bKWSEnable(FALSE)
		, m_bKwsExist(FALSE)
		, m_nFixCount(0)
		, m_nDangerCount(0)
		, m_nCancel(0)
		, m_hExamShareDataEvent(NULL)
// 		, m_nShareDataCount(0)
// 		, m_nIndexShareData(0)
	{
		m_bkSafeLog.SetLogForModule(BKSafeLog::enumBKSafeMainDlgLog);
		m_nMaxScore = 100;//默认是100
		m_nMinScore = 100;
		m_nMinScore2 = 100;
		m_nStartExamTime = 0;
		m_ExamItemSort.RemoveAll();
		InitializeCriticalSection(&g_StartExam_cs);
		m_hStartExamEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		_ResetMapVector();
		m_hExamShareDataEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_ExamPSD.Create(NAME_EXAM_SHARE_MEM, SIZE_EXAM_SHARE_MEM);
		::InitializeCriticalSection(&m_LockShareData);
		//m_arrExamShareData.RemoveAll();
		m_arrScanData.RemoveAll();
		m_arrFixData.RemoveAll();
	}

	~CBeikeSafeExamUIHandler();

	BKSafeLog::CBKSafeLog m_bkSafeLog;
	//主界面过来的回调函数
	void	ModifyMonitorState(int nType);								//刷新监控状态信息
	void	UpdatSafemonStateCaption(BOOL bKWSStatusRefresh = TRUE);
	void	UpdateExamSetting();										//更新当前体检的状态
//	void	ModifyBwsMonitorState();									//更新浏览器监控状态
	void	UpdateDubaState();
	void	Echo();														//做一次握手操作
	CBkNavigator* OnNavigate( CString &strChildName );
	STATE_CLOSE_CHECK    CloseCheck();


	//内部成员函数
	void	Init(BOOL bStartExam);
	void	InitCtrl();
	void	StartExam(BOOL bExpress = FALSE);
	void	RefreshTodoList();
	HANDLE	m_hStartExamEvent;
	HANDLE									 m_hExamRecvEvent;
	//CAtlArray<BKSafe_Exam_ShareData>		 m_arrExamShareData;
	CRITICAL_SECTION						 m_LockShareData;
	CProcessSharedData						 m_ExamPSD;
	HANDLE									 m_hExamShareDataEvent;
// 	int										 m_nShareDataCount;
// 	int										 m_nIndexShareData;
	CAtlMap<int, BKSafe_Exam_ShareData>    m_arrScanData;
	CAtlMap<int, BKSafe_Exam_ShareData>	 m_arrFixData;


protected:
	void	FullScan();							//调用全盘扫描
	void	OnBkLblExamRestart();				//重新体检
	void	OnBkBtnExamStop();					//停止扫描
	void	OnExamSet();						//手动点击体检设置
	void	OnListViewResize(CRect rcSize);		//设置体检区域的list的大小
	void	OnBkStartExam();					//手动点击体检
	void	OnTimer(UINT_PTR nIDEvent);								
	void	OnBkOperate1();						//右上角的推荐操作1-5
	void	OnBkOperate2();
	void	OnBkOperate3();
	void	OnBkOperate4();
	void	OnBkOperate5();
	void	OnBkOperate6();
	void	OnBkOperate7();
	void	OnBkOperate8();
	void	OnBkOperate9();
	void	OnBkSetSafemon();					//右上角的设置实时保护
	void	OnBkSetKwsSafemon();				//右上角浏览器保护
	void	NotityScanEntra( int nItem );



	LRESULT OnAppExamScoreChange(UINT uMsg, WPARAM wParam, LPARAM lParam);		//体检模块回调结果
	LRESULT OnAppExamOnlyEcho(UINT uMsg, WPARAM wParam, LPARAM lParam);			
	LRESULT OnAppEchoFinish(UINT uMsg, WPARAM wParam, LPARAM lParam);			//握手结束
	LRESULT OnAppExamProgress(UINT uMsg, WPARAM wParam, LPARAM lParam);			//体检进度
	LRESULT OnLDListViewClickUrl(UINT uMsg, WPARAM wParam, LPARAM lParam);		//点击体检结果列表中的文字连接
	LRESULT OnLDListViewExpandItem(UINT uMsg, WPARAM wParam, LPARAM lParam);	//体检区域展开收起
	LRESULT OnExamErrorMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);			//体检模块中检测到的错误信息
	LRESULT	OnFullScanTrojan(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//	LRESULT OnBkExamTabSel(int nTabItemIDOld, int nTabItemIDNew);				//体检右上角的推荐功能按钮
	LRESULT OnLDListViewClickExTxt(UINT uMsg, WPARAM wParam, LPARAM lParam);	//点击体检结果列表中的文字连接
	LRESULT OnSignal(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);//检测毒霸云引擎是否连接成功
	LRESULT OnStartExam(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_EXAM_RESTART,					OnBkLblExamRestart)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_EXAM_STOP,						OnBkBtnExamStop)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_SETTING_ID,				OnExamSet)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_EXAM,						OnBkStartExam)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_CLEAN_OP1_ID,				OnBkOperate1);
	BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_CLEAN_OP2_ID,				OnBkOperate2);
	BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_CLEAN_OP3_ID,				OnBkOperate3);
	BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_CLEAN_OP4_ID,				OnBkOperate4);
	BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_CLEAN_OP5_ID,				OnBkOperate5);
	BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_OPT_570,					OnBkOperate6);
	BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_OPT_571,					OnBkOperate7);
	BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_OPT_572,					OnBkOperate8);
	BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_OPT_573,					OnBkOperate9);
	BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_MODIFY_SAFEMON_ID,		OnBkSetSafemon);
	BK_NOTIFY_REALWND_RESIZED(IDC_EXAM_LISTVIEW_ID,				OnListViewResize)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_SETTIMGID_3,				OnExamSet)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_TXT_569,					OnBkSetKwsSafemon)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_BTN_530,					OnBkLblExamRestart)
		BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CBeikeSafeExamUIHandler)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MSG_WM_TIMER(OnTimer)
		MESSAGE_HANDLER_EX(MSG_APP_EXAM_SCORE_CHANGE,				OnAppExamScoreChange)
		MESSAGE_HANDLER_EX(MSG_APP_EXAM_ONLY_ECHO,					OnAppExamOnlyEcho)
		MESSAGE_HANDLER_EX(MSG_APP_ECHO_FINISH,						OnAppEchoFinish)
		MESSAGE_HANDLER_EX(MSG_APP_EXAM_PROGRESS,					OnAppExamProgress)
		MESSAGE_HANDLER_EX(MSG_USER_LDLISTVIEWCTRL_LBP_URL,			OnLDListViewClickUrl)
		MESSAGE_HANDLER_EX(MSG_USER_LDLISTVIEWCTRL_EXPAND,			OnLDListViewExpandItem)
		MESSAGE_HANDLER_EX(MSG_APP_EXAM_ERROR,						OnExamErrorMsg)
		MESSAGE_HANDLER_EX(MSG_USER_LDLISTVIEWCTRL_LBP_EXTXT,		OnLDListViewClickExTxt)
		MESSAGE_HANDLER_EX(MSG_APP_FULL_SCAN_TROJAN,				OnFullScanTrojan);
		MESSAGE_HANDLER(MSG_SIGNAL,									OnSignal);
		MESSAGE_HANDLER(MSG_APP_START_EXAM,							OnStartExam)
		END_MSG_MAP()

private:
	CSimpleArray<int>						 m_arrKwsExamItem;		//体检中需要调用网盾的项目
	BOOL									 m_bEcho;				
	BOOL									 m_bUpdateLog;			//是否需要上传日志
	BOOL									 m_bUserCancelExam;		//是否用户手动取消体检
	DWORD									 m_dwStartExamTime;		//开始体检的时间，统计用
	CString									 m_strDangerIDs;		//体检中发现危险项目的ID列表
	enumUpLogType							 m_examType;			//当前用户的体检类型
	BOOL									 m_bKWSEnable;			//网盾是否开启
	BOOL									 m_bKwsExist;			//网盾是否存在
	BOOL									 m_bExamFinished;		//体检扫描结束标记
	int										 m_nShowProgress;		//体检扫描显示进度
	int										 m_nRealProgress;		
	CLDListViewCtrl							 m_ExamListView;		//显示体检项目和结果的list列表
	CBeikeSafeMainDlg*						 m_pDlg;				//主程序窗口指针
	KTimingCheckSecurityCloudTask			 m_CheckSecurityCloudTimer; //检测是否连接毒霸云安全
	KLogic									 m_Logic;
	CSimpleArray<UINT>						 m_ExamItemSort;			//初始化存放体检中各个危险项目的顺序
	int										 m_nMaxScore;				//体检分数的最大值
	int										 m_nMinScore;
	int										 m_nMinScore2;
	int										 m_nDangerCount;		
	int										 m_nCancel;
	int										 m_nFixCount;
	int										 m_nStartExamTime;
	CLocker									 m_lockRemove;


	CSimpleMap<int, BKSAFE_EXAM_RESULT_ITEM> m_mapSafeScanResult;	//体检安全项目
	CSimpleMap<int, BKSAFE_EXAM_RESULT_ITEM> m_mapCancelScanResult;	//体检被取消的项目
	CSimpleMap<int, BKSAFE_EXAM_RESULT_ITEM> m_mapRasikScanResult;	//体检高风险项目
	CSimpleMap<int, BKSAFE_EXAM_RESULT_ITEM> m_mapDangerScanResult2;//体检中危险项目
	CSimpleMap<int, BKSAFE_EXAM_RESULT_ITEM> m_mapOptimization;
	CSimpleMap<int, BKSAFE_EXAM_RESULT_ITEM> m_mapAllDangerItem;
	CSimpleMap<DWORD, DWORD>				 m_mapItemScoreCoft;	//体检每项的系数
	CSimpleMap<DWORD, DWORD>				 m_mapExamLevelMinScore;//体检每个分数区间的得分最小值

	void		_LoadExamData();														//加载体检项目到显示列表中
	void		_UpdateExamData(int nItem=-1,BKSAFE_EXAM_RESULT_ITEM *pData=NULL);		//更新体检扫描结果
	void		_GetTextWithItemLevel(int nItem, int nLevel, CString& strOperate,		//根据不同体检项目的危险等级显示不同的提示文字信息
		CString& strResult, BOOL bResult = FALSE);
	void		_RefreshExamList(int nItem, CString strUpdate, CString strScanState,	//刷新体检结果列表(扫描过程中会调用)
		COLORREF extxtcolor, COLORREF urlcolor, int nImageID);
	int			_GetImageID(int nLevel);												//根据危险等级显示不同的图标
	void		_ShowExamResultData();								//扫描结束之后，对扫描结果重新归类,体检结束会调用
	int			_ExamFixOperate(int nItem);												//修复体检中某项
	void		_UpdateExamSetTxt();													//根据用户选择不同的体检类型显示不同的文字
	void		_WriteExamLogToIni();													//体检信息记录到ini文件中
	void		_GetExamLogFromIni(CString& strTime, int& dwScore, int& nFixCount, 
		int& nDangerCount, int& nTime, int& nCancel);						//通过体检日志获取上次体检信息
	CString 	_GetExamLogFilePath();													//获取体检日志文件的路径
	LONG		_CompareFileTime(CString strHistoryTime);								//时间比较
	void		_UpdateHistoryInfo();													//根据日志文件更新上一次体检信息
	void		_StartHandShake();														//进行一次握手操作
	COLORREF	_GetTxtColor(int nLevel);												//根据危险等级敌营文字颜色

	int			_FixItem(REMOVE_EXAM_RESULT_ITEM RemoveExamItem);						//更新修复后的状态信息

	void		_SortMap(CSimpleMap<int, BKSAFE_EXAM_RESULT_ITEM>& mapData);			//对列表进行排序，根据危险等级
//	void		_InsertShowMoreData(int nGroup);										//插入显示更多信息的文字
	BOOL		_GroupIDExist(int nGroupID);											//检测某项信息是否显示
	void		_ResetMapVector();														//重置扫描结果列表
	void		_InsertSafeResultData(LPCTSTR pszTitleCaption, int nGroupID,			//显示安全的体检结果数据
									CSimpleMap<int, BKSAFE_EXAM_RESULT_ITEM>& mapData);
	void		_InsertDangerResultData(LPCTSTR pszTitleCaption, int nGroupID,			//危险
									CSimpleMap<int, BKSAFE_EXAM_RESULT_ITEM>& mapData);
	void		_InsertCancelResultData(LPCTSTR pszTitleCaption, int nGroupID,			//取消
									CSimpleMap<int, BKSAFE_EXAM_RESULT_ITEM>& mapData);				
//	BOOL		_IsHaveGroupHideInfo(int nGroupID);										//是否有隐藏信息
	LPLDListViewCtrl_Item _InsertResultData(int nGroupID, int nItem,					//向list中插入数据
		BKSAFE_EXAM_RESULT_ITEM exam, BOOL bReloadCaption, int nInsertAt = -1);
	LPLDListViewCtrl_Item _InsertResultDataTitle(int nGroupID, LPCTSTR pszTitleCaption, //危险或者安全分组的标题
		int nLevel, int nImageID = 6, ENUM_LISTVIEW_FONT fontType = FONT_BOLD);

	void		_InsertOptResultData(LPCTSTR pszTitleCaption, int nGroupID,			//显示可优化的体检结果数据
									CSimpleMap<int, BKSAFE_EXAM_RESULT_ITEM>& mapData);

	int			_GetExamType();															//检测当前用户设置的体检类型，返回0表示启动是自动体检
	void		_ReportLog(enumUpLogType ReportType, CString strLog = TEXT(""));		//反馈
	int			_GetSafeMonitorItemCaption(CString& strCaption, int nItemID = -1);						//获取未开启的监控的名字
	int			_RetryEcho();															//重试握手
	int			_IsKwsExam(int nItem);													//检测是否是通过网盾扫描到的
// 	void		_CheckKwsExamResult();													//网盾上网安全检测
 	int			_IsAllKwsOpen();														//检测网盾全部功能是否开启
	void		_LoadGlobalData();														//加载体检显示列表中的信息
	void		_UpdateExamResultTitle();												//更新显示体检结果顶部的结论信息
	void		_InitExamItemSort();													//初始化体检处理各个项目的现实顺序

	//int			_FileSignerIsValid(LPCTSTR pszFile, CAtlArray<CString>& arrSinger);		//验证文件签名和给定的是否相同，如果没有指定就只验证有效性
	void		_StartProcess(LPCTSTR pszFileFullPath, LPCTSTR pszParam = NULL);		//启动第三方进程
	int			_CheckDuBa();															//检测毒霸
	int			_CheckSoftWareUpdate(int& nUpdateCount);
	void		_SetExamMinScore(int nItemID, int nLevel);
	void		_CalcExamScore();
	void		_UpdateExamScoreImage();
	void		_ParseScore(int nScore, CSimpleArray<int> &arr, CSimpleArray<int>& arr2);
	void		_ParseScoreForUpdateUI(int nScore, CStringA& strSkinNum, 
										CString& strText1, CString& strText2, 
										int& dwImage, int& dwTextColor, int& dwScoreText);
	void		_ParseScoreForUpdateUI(int nScore, int& nLevel, int& nImage, int& nTxtColor);
	void		_ShareExamData(UINT uMsg, WPARAM wParam, LPARAM lParam);
	CString     _ConverColorToStr(COLORREF color);
	void		_UpdateExamResult();
	void		_InitArrayForShareData();
	DWORD		_GetTitleTxtColor(int nLevel);


};


