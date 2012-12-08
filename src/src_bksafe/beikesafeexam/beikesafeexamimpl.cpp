#include "stdafx.h"
#include "beikesafeexamimpl.h"
#include <bkres/bkres.h>
#include <miniutil/bkhandle.h>
#include <bkres/bkstringpool.h>
#include "_idl_gen/bksafesvc.h"
#include <comproxy/bkscanfile.h>
#include <bksafe/bksafeconfig.h>
#include <plugin/interface.h>
#include <comproxy/bkutility.h>
#include <skylark2/bkengdef.h>
#include <vulfix/Vulfix.h>
#include <atltime.h>
#include <atlsync.h>
#include <atlfile.h>

#include <safemon\safemonitor.h>
#include <safemon\safetrayshell.h>
#include "strsafe.h"
#include "ldedittxtfile.h"
#include <winmod/wintrustverifier.h>
using namespace WinMod;
#include <miniutil/fileversion.h>
#include "iefix/cinifile.h"
#include "avmgr/avmgrinterface.h"
#include "kws/KwsSetting.h"

BOOL g_bStringLoaded = FALSE;
#define EXAM_LOG
#define LOCALLIB_FILE_NAME TEXT("\\LocalLib.dat")
#define LOCALLIB_KEY 0xff
#define ERROR_FILE_VER TEXT("1.0.0.1095")
#define		UPDATE_SEC		L"update"
#define		UPDATE_COUNT	L"updatecount"
#define		UPDATE_TIME		L"updatetime"

//#define OUTPUT_LOG
#define USES_TWO_THREAD
typedef bool (__stdcall*HasTrash_t)();

CCriticalSection g_lockStringMap;
IKSWebIEFixEng*	m_pKSWebIeEng = NULL;//new eng


void LoadString()
{
	g_lockStringMap.Enter();
	if (!g_bStringLoaded)
	{
		BkString::Load(IDR_BK_STRING_DEF);
		g_bStringLoaded = TRUE;
	}
	g_lockStringMap.Leave();
}

class CTraceLog
{
public:
	CTraceLog()
	{
		::InitializeCriticalSection(&m_lockLog);
	}

	~CTraceLog()
	{
		::DeleteCriticalSection(&m_lockLog);
	}

	void Log(LPCSTR format, ...)
	{
		BOOL RetVal = FALSE;
		CStringA strLog, strLogText;

		va_list args;

		va_start(args, format);
		strLogText.FormatV(format, args);
		va_end(args);

		SYSTEMTIME st;

		::GetLocalTime(&st);

		strLog.Format(
			"%04d-%02d-%02d %02d:%02d:%02d %s\r\n", 
			st.wYear, st.wMonth, 
			st.wDay, st.wHour, 
			st.wMinute, st.wSecond, 
			strLogText);

		CAtlFile file;

		::EnterCriticalSection(&m_lockLog);

		HRESULT hRet = file.Create(L"C:\\ld_exam_trace.log", FILE_APPEND_DATA, FILE_SHARE_READ, OPEN_EXISTING);
		if (SUCCEEDED(hRet))
		{
			file.Write((LPVOID)(LPCSTR)strLog, strLog.GetLength());
			file.Close();
		}

		::LeaveCriticalSection(&m_lockLog);
	}

private:

	CRITICAL_SECTION m_lockLog;
};

CTraceLog g_TraceLog;

class CCostTimeTracer
{
public:

	void Start()
	{
		m_dwTick = ::GetTickCount();
		m_dwStart = m_dwTick;
	}

	void Log(LPCSTR lpszItemName)
	{
		m_dwTick = ::GetTickCount() - m_dwTick;
		g_TraceLog.Log("%s %d", lpszItemName, m_dwTick);
	}

	DWORD TotalTime()
	{
		return ::GetTickCount() - m_dwStart;
	}

private:

	DWORD m_dwTick;
	DWORD m_dwStart;
};

CBkSafeExaminer::~CBkSafeExaminer()
{
	//	_SaveToLocalLib(m_bkLocalLib);
	//	SAFE_CALL_FUN(m_pKSWebIeEng, Delete());
}

HRESULT WINAPI CBkSafeExaminer::Initialize(LPVOID pvParam)
{
	LoadString();

	DWORD dwRet = ::GetModuleFileName((HMODULE)&__ImageBase, m_strAppPath.GetBuffer(MAX_PATH + 1), MAX_PATH);
	if (0 == dwRet)
	{
		m_strAppPath.ReleaseBuffer(0);
	}
	else
	{
		m_strAppPath.ReleaseBuffer();

		m_strAppPath.Truncate(m_strAppPath.ReverseFind(L'\\') + 1);
	}

	return S_OK;
}

HRESULT WINAPI CBkSafeExaminer::Uninitialize()
{
	_SaveToLocalLib(m_bkLocalLib);
	return S_OK;
}

HRESULT WINAPI CBkSafeExaminer::Examine(IBkSafeExamineCallback *piCallback, HANDLE hStopEvent, BOOL bSlowMode/* = FALSE*/)
{
	m_bSlowMode = bSlowMode;

	m_piCallback		= piCallback;
	m_hEventStop		= hStopEvent;
	m_nKwsProgress		= 0;
	m_nTrojanProgress	= 0;
	//m_nInitComErrorCode = -1;
	_ResetArray();

	CKSafeExamLog::GetLogPtr()->WriteLog(L"Examine: Start");

	if (bSlowMode)
	{
		_ExaminationThreadProc_1(this);
		_ExaminationThreadProc_2(this);
#ifndef USES_TWO_THREAD
		_ExaminationThreadProc_3(this);
		_ExaminationThreadProc_4(this);
		_ExaminationThreadProc_6(this);//EX
#endif 
	}
	else
	{

#ifndef USES_TWO_THREAD
#       define EXAM_THREAD_COUNT 6
#else
#       define EXAM_THREAD_COUNT 2
#endif


		DWORD dwRet = 0;
		HANDLE hExamThreads[EXAM_THREAD_COUNT] = {0};

		hExamThreads[0] = ::CreateThread(NULL, 0, _ExaminationThreadProc_1, this, 0, NULL);
		hExamThreads[1] = ::CreateThread(NULL, 0, _ExaminationThreadProc_2, this, 0, NULL);

#ifndef USES_TWO_THREAD
		hExamThreads[2] = ::CreateThread(NULL, 0, _ExaminationThreadProc_3, this, 0, NULL);
		hExamThreads[3] = ::CreateThread(NULL, 0, _ExaminationThreadProc_4, this, 0, NULL);
		hExamThreads[5] = ::CreateThread(NULL, 0, _ExaminationThreadProc_6, this, 0, NULL);//EX
#endif


		//如果20S内木马扫描没有完成，直接干掉这个线程
		/*
		dwRet = ::WaitForSingleObject(hExamThreads[1], 20*1000);
		if (WAIT_TIMEOUT == dwRet && 0 != m_nInitComErrorCode)
		{//com初始化超时就直接干掉线程
		if (NULL != hExamThreads[1])
		{
		m_nTrojanProgress = 60;
		TerminateThread(hExamThreads[1], 0);
		}
		//木马线程被干掉，其他线程还是需要等待
		dwRet = ::WaitForSingleObject(hExamThreads[0], INFINITE);
		}
		else if (WAIT_OBJECT_0 == dwRet)
		{//如果木马扫描正常返回，就等待其他扫描完成
		dwRet = ::WaitForSingleObject(hExamThreads[0], INFINITE);
		}
		else
		*/

		{//如果木马扫描时超时返回，就和其他扫描线程一起等待
			while (TRUE)
			{
				dwRet = ::WaitForMultipleObjects(EXAM_THREAD_COUNT, hExamThreads, TRUE, INFINITE);
				if (WAIT_OBJECT_0 == dwRet)
					break;

				if (WAIT_TIMEOUT != dwRet)
				{
					DEBUG_TRACE(L"Wait hExamThreads Return %d\r\n", dwRet);
					break;
				}
			}
		}


		for (int i = 0; i < EXAM_THREAD_COUNT; i ++)
		{
			if (NULL != hExamThreads[i])
			{
				::CloseHandle(hExamThreads[i]);
				hExamThreads[i] = NULL;
			}
		}

	}

	if (m_piCallback && !_IsStopping(m_hEventStop))
		m_piCallback->OnFinish();

	return S_OK;
}

HRESULT WINAPI CBkSafeExaminer::ExpressExamine(IBkSafeExamineCallback *piCallback, HANDLE hStopEvent)
{
	m_piCallback = piCallback;
	m_hEventStop = hStopEvent;

	_LoadLocalInfo();

	return S_OK;
}

HRESULT WINAPI CBkSafeExaminer::Echo(IBkSafeExamineCallback *piCallback, HANDLE hStopEvent)
{
	m_piCallback = piCallback;
	m_hEventStop = hStopEvent;

	_Echo(hStopEvent);

	return S_OK;
}

BOOL CBkSafeExaminer::_IsStopping(HANDLE hStopEvent, DWORD dwWait/* = 0*/)
{
	if (hStopEvent)
		return (WAIT_TIMEOUT != ::WaitForSingleObject(hStopEvent, dwWait));

	return FALSE;
}

void CBkSafeExaminer::_ReportResult(int nItem, int nLevel, BKSafeExamItemEx::BKSafeExamEx* pExamItemEx/* = NULL*/)
{
	using namespace BkSafeExamItem;
	using namespace BkSafeExamItemLevel;

	UINT uResStrNotice = 0, nResStrOperationText = 0, nResStrNavigateString = 0;
	BKSafeExamItemEx::BKSafeExamEx bkSafeExamItemEx;
	if (NULL != pExamItemEx)
		bkSafeExamItemEx = *pExamItemEx;

	switch (nItem)
	{
	case CriticalPathVirusScan:
		uResStrNotice = IDS_NOTICE_CRITICALPATHVIRUSSCAN;
		nResStrOperationText = IDS_OPERATION_CRITICALPATHVIRUSSCAN;
		nResStrNavigateString = IDS_NAVIGATE_STR_CRITICALPATHVIRUSSCAN;
		break;
	case SystemLeakScan:
		uResStrNotice = IDS_NOTICE_SYSTEMLEAKSCAN;
		nResStrOperationText = IDS_OPERATION_SYSTEMLEAKSCAN;
		nResStrNavigateString = IDS_NAVIGATE_STR_SYSTEMLEAKSCAN;
		break;
	case MalwareScan:
		uResStrNotice = IDS_NOTICE_MALWARESCAN;
		nResStrOperationText = IDS_OPERATION_MALWARESCAN;
		nResStrNavigateString = IDS_NAVIGATE_STR_MALWARESCAN;
		break;
	case ConfigAutoReport:
		uResStrNotice = IDS_NOTICE_CONFIGAUTOREPORT;
		nResStrOperationText = IDS_OPERATION_CONFIGAUTOREPORT;
		nResStrNavigateString = IDS_NAVIGATE_STR_CONFIGAUTOREPORT;
		break;
	case HandShake:
		uResStrNotice = (Critical == nLevel) ? IDS_NOTICE_HANDSHAKE_SELFCHECK_FAIL : IDS_NOTICE_HANDSHAKE;
		nResStrOperationText = IDS_OPERATION_HANDSHAKE;
		nResStrNavigateString = (Critical == nLevel) ? IDS_NAVIGATE_STR_HANDSHAKE_SELFCHECK_FAIL : IDS_NAVIGATE_STR_HANDSHAKE;
		break;
	case ConfigAutoUpdate:
		uResStrNotice = IDS_NOTICE_CONFIGAUTOUPDATE;
		nResStrOperationText = IDS_OPERATION_CONFIGAUTOUPDATE;
		nResStrNavigateString = IDS_NAVIGATE_STR_CONFIGAUTOUPDATE;
		break;
	case ConfigSystemMonitor:
		uResStrNotice = IDS_NOTICE_CONFIGSYSTEMMONITOR;
		nResStrOperationText = IDS_OPERATION_CONFIGSYSTEMMONITOR;
		nResStrNavigateString = IDS_NAVIGATE_STR_CONFIGSYSTEMMONITOR;
		break;
	case DenyChangeIEMainPage:
		uResStrNotice = IDS_NOTICE_DENYCHANGEIEMAINPAGE;
		nResStrOperationText = IDS_OPERATION_DENYCHANGEIEMAINPAGE;
		nResStrNavigateString = IDS_NAVIGATE_STR_DENYCHANGEIEMAINPAGE;
		break;
	case LongTimeNoScan:
		uResStrNotice = IDS_NOTICE_LONGTIMENOSCAN;
		nResStrOperationText = IDS_OPERATION_LONGTIMENOSCAN;
		nResStrNavigateString = IDS_NAVIGATE_STR_LONGTIMENOSCAN;
		break;
	case EXAM_HOMEPAGE:			
	case EXAM_IE_LNKFILE_PARAM:
	case EXAM_IE_RIGHT_MENU:		
	case EXAM_IE_TOOLBAR_ICON:	
	case EXAM_IE_TOOLBAR:	
	case EXAM_IE_DEFAULT_SEARCH_ENG:
	case EXAM_IE_DIY_SEARCH_ENG:
	case EXAM_IE_SEARCH_PAGE:
	case EXAM_HOSTS_FILE:	
	case EXAM_IE_SAFE_WEB:		
	case EXAM_IE_BROWSE:			
	case EXAM_HTML_FILE:			
	case EXAM_PROTOCOL_DEFAULT:
	case EXAM_PROXY_SERVER:	
	case EXAM_HOMEPAGE_CAN_SET:		
	case EXAM_REGEDIT_RUN:	
	case EXAM_GPEDIT_RUN:		
	case EXAM_GUEST_RUN:			
	case EXAM_HIDE_FOLDER_RUN:
	case EXAM_SHARE_RESOUCE:		
	case EXAM_REMOTE_DESKTOP:
	case EXAM_CONNECT_SRV:
	case EXAM_DRIVERORDIRECTORYOPEN:
	case EXAM_FAVORITES_URL:					//检测收藏夹中是否存在恶意推广的网址->36
	case EXAM_SYSTEM_EXCEPTION:					//是否存在系统异常->37
	case EXAM_NOTIE_LNK:						//非IE浏览器快捷方式是否被篡改->38
	case EXAM_DESKTOP_SYSTEM_EXCEPTION_ICON:	//检测桌面图标->39
	case EXAM_DESKTOP_EXCEPTION_ICON:			//异常的桌面图标->40

		{
			uResStrNotice = IDS_NOTICE_STR_IEFIX;
			nResStrOperationText = IDS_OPERATION_IEFIX;
			nResStrNavigateString = IDS_NAVIGATE_STR_IEFIX;
		}
		break;
	case EXAM_TRASH:
		{
			uResStrNotice = IDS_NOTICE_STR_TRASH;
			nResStrOperationText = IDS_OPERATION_TRASH;
			nResStrNavigateString = IDS_NAVIGATE_STR_TRASH;
		}
		break;
	case EXAM_RUNOPT_ONKEY:
		{
			uResStrNotice = IDS_RUNOPT_STR_ONEKEY;
			nResStrOperationText = IDS_RUNOPT_STR_OPT;
			nResStrNavigateString = IDS_RUNOPT_STR_NAVIGATE;

		}
		break;
	case EXAM_CHECK_UPDATE:
		{
			uResStrNotice = IDS_UPDATE_STR_TITLE;
			nResStrOperationText = IDS_UPDATE_STR_OPT;
			nResStrNavigateString = IDS_UPDATE_STR_NAVIGATE;

		}
		break;
	case EXAM_CHECK_AV:
		{
			uResStrNotice = IDS_AV_STR_TITLE;
			nResStrOperationText = IDS_AV_STR_OPT;
			nResStrNavigateString = IDS_AV_STR_NAVIGATE;
		}
		break;
	case EXAM_KWS_MON:
		{
			uResStrNotice = IDS_KWS_STR_TITLE;
			nResStrOperationText = IDS_KWS_STR_OPT;
			nResStrNavigateString = IDS_KWS_STR_NAVIGATE;
		}
		break;
	}

	if (m_piCallback && !_IsStopping(m_hEventStop))
	{
		m_piCallback->OnItemExamFinish(
			nItem, nLevel,
			BkString::Get(uResStrNotice), 
			BkString::Get(nResStrOperationText), 
			BkString::Get(nResStrNavigateString),
			NULL, 
			(LPCWSTR)(&bkSafeExamItemEx)
			);
	}
}
#if 0
void CBkSafeExaminer::_ReportLocalResult(
	int     nItem, 
	int     nLevel, 
	LPCWSTR lpszItemNoticeText, 
	LPCWSTR lpszOperationText, 
	LPCWSTR lpszItemNavigateString, 
	LPCWSTR lpszItemDetailUrl)
{

	if (m_piCallback && !_IsStopping(m_hEventStop))
	{
		m_piCallback->OnItemExamFinish(
			nItem, nLevel,
			lpszItemNoticeText, 
			lpszOperationText, 
			lpszItemNavigateString,
			lpszItemDetailUrl,
			NULL
			);
	}

}
#endif

//检测实时保护状态，官方版本
DWORD WINAPI CBkSafeExaminer::_ExaminationThreadProc_1(LPVOID pvParam)
{

	CBkSafeExaminer *pThis = (CBkSafeExaminer *)pvParam;
	CBkHandle hEventStop;
	CAtlArray<CString> arrFile;

	hEventStop.Clone(pThis->m_hEventStop);

	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: _ExaminationThreadProc_1");

	// 检查监控状态
	if (FALSE == pThis->ItemIsIgnored(BkSafeExamItem::ConfigSystemMonitor))
	{
		BOOL bAllMonitorOn = TRUE;
		DWORD dwMonitorIDList[] = {
			SM_ID_RISK, 
			SM_ID_PROCESS, 
			SM_ID_UDISK, 
			SM_ID_LEAK,
// 			SM_ID_KWS_SAFE_DOWN,
// 			SM_ID_KWS
		};

		BKSafeExamItemEx::BKSafeExamEx ExamResultEx;
		ExamResultEx.Reset();
		ExamResultEx.u.BKSafeMonitor.Reset();
		CSafeMonitorTrayShell MonitorShell;
		CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: %d", BkSafeExamItem::ConfigSystemMonitor);
		int nIndex = 0;

		for (int i = 0; i < ARRAYSIZE(dwMonitorIDList)/* && bAllMonitorOn*/; i ++)
		{
			//如果是64位系统体检发现时进程监控或者网页监控没有开启的话，就认为没有风险,暂时64为系统下不支持这两个监控
			// 			if (TRUE == CCommon::GetCommonPtr()->IsWin64() &&
			// 				(
			// 				SM_ID_PROCESS == dwMonitorIDList[i]))
			// 			{
			// 				continue;
			// 			}
			bAllMonitorOn &= MonitorShell.GetMonitorRun(dwMonitorIDList[i]);
			if (FALSE == MonitorShell.GetMonitorRun(dwMonitorIDList[i]))
			{
				ExamResultEx.bInvalid = TRUE;
				ExamResultEx.u.BKSafeMonitor.nDisableCount++;
				ExamResultEx.u.BKSafeMonitor.arrDisableID[nIndex++] = dwMonitorIDList[i];
			}
		}


		//实时监控是不是全部打开
		pThis->_ReportResult(BkSafeExamItem::ConfigSystemMonitor, bAllMonitorOn ? BkSafeExamItemLevel::Safe : BkSafeExamItemLevel::Risk_0, &ExamResultEx);
	}

	if (_IsStopping(hEventStop))
		goto Exit0;

	pThis->_CheckKwsExamResult();

#if SHOW_TIPS
	pThis->_LoadLocalInfo();

	if (_IsStopping(hEventStop))
		goto Exit0;
#endif


	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: %d", BkSafeExamItem::HandShake);
	CCostTimeTracer tracer;
	tracer.Start();

	//验证官方版本
	arrFile.SetCount(10);
	arrFile[0] = pThis->m_strAppPath + L"KSafe.exe";
	arrFile[1] = pThis->m_strAppPath + L"KSafeSvc.exe";
	arrFile[2] = pThis->m_strAppPath + L"cp1\\KSafeSvc.exe";
	arrFile[3] = pThis->m_strAppPath + L"cp2\\KSafeSvc.exe";
	arrFile[4] = pThis->m_strAppPath + L"kscanner.dll";
	arrFile[5] = pThis->m_strAppPath + L"ksreng3.dll";
	arrFile[6] = pThis->m_strAppPath + L"ksscore.dll";
	arrFile[7] = pThis->m_strAppPath + L"kse\\ksbwdet2.dll";
	arrFile[8] = pThis->m_strAppPath + L"kse\\ksecorex.dll";
	arrFile[9] = pThis->m_strAppPath + L"KSafeTray.exe";

#if _DEBUG 
	if (TRUE)
#else
	if (0 == CCommon::GetCommonPtr()->VerifyFile(arrFile))
#endif
	{
		pThis->_ReportResult(BkSafeExamItem::HandShake, BkSafeExamItemLevel::Safe);
	}
	else
	{
		pThis->_ReportResult(BkSafeExamItem::HandShake, BkSafeExamItemLevel::Risk_0);
	}


	tracer.Log("safever");


Exit0:

#ifdef USES_TWO_THREAD
	if (_IsStopping(hEventStop))
		goto Exit1;

	//软件升级
	_ExaminationThreadProc_9(pvParam);
	if (_IsStopping(hEventStop))
		goto Exit1;

	//漏洞
	_ExaminationThreadProc_3(pvParam);
	if (_IsStopping(hEventStop))
		goto Exit1;

	//插件
	_ExaminationThreadProc_4(pvParam);
	if (_IsStopping(hEventStop))
		goto Exit1;

	//垃圾清理
	_ExaminationThreadProc_7(pvParam);
	if (_IsStopping(hEventStop))
		goto Exit1;

	//启动项
	_ExaminationThreadProc_8(pvParam);
	if (_IsStopping(hEventStop))
		goto Exit1;

	//杀软
	_ExaminationThreadProc_10(pvParam);
#endif

Exit1:
	return 0;
}
//握手
void CBkSafeExaminer::_Echo(HANDLE hEventStop)
{
	::CoInitializeEx(NULL, COINIT_MULTITHREADED);

	CBkEcho echo;
	CAtlArray<CString> arrEchoFiles;
	BK_ECHO_RESULT result;
	int nRet = BkSafeExamItemLevel::Risk_0;

	result.dwEchoStatus = BKENG_ECHO_LOCAL_COMM_ERROR;
	result.hEchoCode = E_FAIL;

	HRESULT hRet = echo.Initialize();
	if (FAILED(hRet))
		goto Exit0;

	arrEchoFiles.SetCount(2);
	arrEchoFiles[0] = m_strAppPath + L"KSafe.exe";
	arrEchoFiles[1] = m_strAppPath + L"KSafeSvc.exe";

	hRet = echo.Echo(arrEchoFiles);
	if (FAILED(hRet))
		goto Exit0;

	do
	{
		hRet = echo.GetEchoResult(result);
		if (FAILED(hRet))
			goto Exit0;

		::Sleep(200);
	}
	while (S_OK != hRet);

	switch (result.dwEchoStatus)
	{
	case BKENG_ECHO_SUCCESS:
#ifdef _DEBUG
	case BKENG_ECHO_UNRECOGNISED_EXE:
#endif
		nRet = BkSafeExamItemLevel::Safe;
		break;
#ifndef _DEBUG
	case BKENG_ECHO_UNRECOGNISED_EXE:
#endif
	case BKENG_ECHO_BLACK_EXE:
		nRet = BkSafeExamItemLevel::Critical;
		break;
	default:
		nRet = BkSafeExamItemLevel::Risk_0;
		break;
	}

Exit0:

	if (FAILED(hRet))
		result.hEchoCode = hRet;

	echo.Uninitialize();

	::CoUninitialize();

	_ReportResult(BkSafeExamItem::EXAM_CONNECT_SRV, nRet);

}
//体检中木马扫描快速检测
DWORD WINAPI CBkSafeExaminer::_ExaminationThreadProc_2(LPVOID pvParam)
{
	CCostTimeTracer tracer;
	tracer.Start();


	CBkSafeExaminer *pThis = (CBkSafeExaminer *)pvParam;
	CBkHandle hEventStop;

	if ((TRUE == pThis->ItemIsIgnored(BkSafeExamItem::LongTimeNoScan)) && 
		(TRUE == pThis->ItemIsIgnored(BkSafeExamItem::CriticalPathVirusScan)))
		return 0;

	hEventStop.Clone(pThis->m_hEventStop);

	// 扫描内存及关键启动项
	HRESULT hRet = E_FAIL;
	BOOL bFoundVirus = FALSE;
	BOOL bSuccess = FALSE;

	KLogic logic( enum_BC_exam );
	S_SCAN_STATUS status;
	std::vector<S_FIX_ITEM_EX> vecThreatList;
	std::vector<S_FIX_ITEM_EX>::iterator iterThreatList;



	//先判断是否有网络连接，如果没有就直接返回没有木马
	if (FALSE == CCommon::GetCommonPtr()->HaveInternetConnection())
	{
		goto Exit0;
	}


	if(pThis->ItemIsIgnored(BkSafeExamItem::CriticalPathVirusScan))
	{
		goto Exit0;
	}

	//没有忽略木马扫描
	// 等待最多等待 10 秒钟
	for (int i = 0; i != 100; ++i)
	{
		if ( logic.WaitSVCStartSingle() )
		{
			bSuccess = TRUE;
			break;
		}

		::Sleep(100);
	}

	if (!bSuccess)
	{
		goto Exit0;
	}

	bSuccess = FALSE;
	for (int i = 0; i != 100; ++i)
	{
		if ( logic.WaitQuickScanEngReadySingle() )
		{
			bSuccess = TRUE;
			break;
		}

		::Sleep(100);
	}

	if (!bSuccess)
	{
		goto Exit0;
	}

	logic.InitStartScanState();

	bSuccess = FALSE;
	for (int i = 0; i != 100; ++i)
	{
		if ( logic.StartScanSingle(enum_Memory_And_Critical_Startup_point) )
		{
			bSuccess = TRUE;
			break;
		}

		::Sleep(100);
	}

	if (!bSuccess)
	{
		goto Exit0;
	}

	while(true)
	{
		status.CurrentTarget.clear();
		status.mapFoundThreats.clear();
		status.HotPointList.clear();

		vecThreatList.clear();

		hRet = logic.QuerySessionStatus(status, vecThreatList);
		if (FAILED(hRet))
			goto Exit0;

		// 保证进度先设
		if (pThis->m_piCallback)
		{
			//DEBUG_TRACE(L"OnProgress(%d)\r\n", statistic.ScanInfo.Progress);
			pThis->m_nTrojanProgress = status.dwFinishedQuantity * 6 / 10;
			SAFE_CALL_FUN(pThis->m_piCallback, OnProgress( pThis->m_nTrojanProgress + pThis->m_nKwsProgress));
		}

		if (!vecThreatList.empty())	// 发现黑文件
		{
			for (iterThreatList = vecThreatList.begin();
				iterThreatList != vecThreatList.end();
				++iterThreatList)
			{
				// 仅当真正发现病毒时体检才报“发现木马病毒”
				if (pThis->_IsVirusLevel(iterThreatList->emLevel))
				{
					bFoundVirus = TRUE;
					goto Exit0;
				}
			}
		}
		else if (em_ScanStatus_Complete == status.emSessionStatus)	// 扫描过程完毕
		{
			goto Exit0;
		}

		if (_IsStopping(hEventStop, 200))
		{
			goto Exit0;
		}
	};

Exit0:
	logic.StopScan();
	//pThis->m_nInitComErrorCode = 0;
	pThis->_ReportResult(BkSafeExamItem::CriticalPathVirusScan, bFoundVirus ? BkSafeExamItemLevel::Critical : BkSafeExamItemLevel::Safe);

	tracer.Log("VirusScan");
	g_TraceLog.Log("VirusScan Files %d", status.dwBlackCnt);

	// 木马扫描完毕后立马开始浏览器修复
#ifdef USES_TWO_THREAD
	_ExaminationThreadProc_5(pvParam);
#endif

	return 0;
}
//体检模块中的漏洞扫描
DWORD WINAPI CBkSafeExaminer::_ExaminationThreadProc_3(LPVOID pvParam)
{
	CCostTimeTracer tracer;
	tracer.Start();

	CBkSafeExaminer *pThis = (CBkSafeExaminer *)pvParam;
	CBkHandle hEventStop;
	IVulCollection *piVolCollection = NULL;

	if(TRUE == pThis->ItemIsIgnored(BkSafeExamItem::SystemLeakScan))
		return 0;

	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: %d", BkSafeExamItem::SystemLeakScan);

	hEventStop.Clone(pThis->m_hEventStop);
	if (TRUE == _IsStopping(hEventStop))
		return 0;



	HMODULE hModVulFix = NULL;
	IVulScan *pscan = NULL;
	INT nLevel = BkSafeExamItemLevel::Safe;
	//新接口
	do 
	{
		BKSafeExamItemEx::BKSafeExamEx bkSafeExamLeak;
		bkSafeExamLeak.bInvalid = FALSE;
		CString strFileVer(TEXT(""));
		GetFileVersion(pThis->m_strAppPath + L"ksafevul.dll", strFileVer);
		if (TRUE == strFileVer.IsEmpty())
			break;//如果获取文件失败了，可能文件已经被破坏掉了，也不进行检测了

		if (TRUE == VersionLess(strFileVer, ERROR_FILE_VER))//文件版本不匹配直接认为没有漏洞
			break;

		if (TRUE == _IsStopping(hEventStop))
			break;

		hModVulFix = ::LoadLibrary(pThis->m_strAppPath + L"ksafevul.dll");
		if (NULL == hModVulFix)
			break;
		FnCreateObject pfnCreateObject = NULL;
		pfnCreateObject = (FnCreateObject)GetProcAddress(hModVulFix, FUNCNAME_CreateObject);
		if (NULL == pfnCreateObject)
			break;
		try
		{
			if(SUCCEEDED(pfnCreateObject(__uuidof(IVulScan), (VOID**)&pscan)) && NULL != pscan)
			{
				IVulCollection *pCollection = NULL;
				if(SUCCEEDED(pscan->ScanLeakEx(&pCollection, VULSCAN_EXPRESS_SCAN)) && NULL != pCollection)
				{
					if (TRUE == _IsStopping(hEventStop))
						break;
					//enum leak
					LONG nStatus = 0;	
#if 0
					bkSafeExamLeak.bInvalid = TRUE;
					bkSafeExamLeak.u.ExamLeakEx.nHightLeakCount = 2;
#endif
					if(SUCCEEDED(pCollection->get_Status(&nStatus)))
					{
						nLevel = nStatus <= 0 ? BkSafeExamItemLevel::Safe : BkSafeExamItemLevel::Critical;
					}
					else
					{
						bkSafeExamLeak.bInvalid = TRUE;
						ATLVERIFY(SUCCEEDED(pCollection->get_VulCount(&bkSafeExamLeak.u.ExamLeakEx.nHightLeakCount, &bkSafeExamLeak.u.ExamLeakEx.nOptionLeakCount, &bkSafeExamLeak.u.ExamLeakEx.nSpLeakCount)));
						CString strMsg;
						strMsg.Format(TEXT("Hight = %d Option = %d SP = %d"), bkSafeExamLeak.u.ExamLeakEx.nHightLeakCount,bkSafeExamLeak.u.ExamLeakEx.nOptionLeakCount,bkSafeExamLeak.u.ExamLeakEx.nSpLeakCount);
						//OutputDebugString(strMsg);
						CKSafeExamLog::GetLogPtr()->WriteLog(strMsg);
						if (bkSafeExamLeak.u.ExamLeakEx.nHightLeakCount > 0)
						{
							nLevel = BkSafeExamItemLevel::Critical;
						}
					}
					//end enum leak
					if (NULL != pCollection)
						pCollection->Release();
				}
				if (NULL != pscan)
					pscan->Release();
			}
		}
		catch (...)
		{
			;
		}
		pThis->_ReportResult(BkSafeExamItem::SystemLeakScan, nLevel, &bkSafeExamLeak);

	} while (FALSE);

	if (NULL != hModVulFix)
		::FreeLibrary(hModVulFix);
	hModVulFix = NULL;


	tracer.Log("VulScan");

	return 0;
}
//体检功能，插件快速扫描
DWORD WINAPI CBkSafeExaminer::_ExaminationThreadProc_4(LPVOID pvParam)
{
	CCostTimeTracer tracer;
	tracer.Start();

	CBkSafeExaminer *pThis = (CBkSafeExaminer *)pvParam;
	if(TRUE == pThis->ItemIsIgnored(BkSafeExamItem::MalwareScan))
		return 0;
	CBkHandle hEventStop;
	hEventStop.Clone(pThis->m_hEventStop);

	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: %d", BkSafeExamItem::MalwareScan);

	// 调用插件扫描
	class __CPlugCallback : public CPlugNullCallback
	{
	public:
		__CPlugCallback(HANDLE hEventStop)
			: m_piEng(NULL)
			, m_hEventStop(hEventStop)
			, m_bFoundDanger(FALSE)
		{
		}

		void SetEngine(IPlugEng *piEng)
		{
			CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: SetEngine...");
			m_piEng = piEng;
		}

		BOOL FoundDanger()
		{
			CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: FoundDanger = %d", m_bFoundDanger);
			return m_bFoundDanger;
		}

	protected:

		IPlugEng *m_piEng;
		HANDLE m_hEventStop;
		BOOL m_bFoundDanger;


		virtual HRESULT ScanItemStart(DWORD nIndex, DWORD nTotal, LPCTSTR lpName)
		{
			CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: ScanItemStart = %s", lpName);
			if (m_piEng && (WAIT_TIMEOUT != ::WaitForSingleObject(m_hEventStop, 0)))
				m_piEng->StopScan();

			return S_OK;
		}

		virtual HRESULT FindItemExist(DWORD nItemId)
		{
			m_bFoundDanger = TRUE;

			if (m_piEng)
				m_piEng->StopScan();

			DEBUG_TRACE(L"Found Plugin %d\r\n", nItemId);
			CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: FindItemExist = %d", nItemId);

			return S_OK;
		}

	};

	__CPlugCallback callBack(hEventStop);

	IPlugEng* piPlugEng = GlobalGetPluginLoader(pThis->m_strAppPath)->GetPlugEngine();
	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: piPlugEng = %x", piPlugEng);
	if (NULL == piPlugEng)
		goto Exit0;

	HRESULT hRet = piPlugEng->SetCallBack(&callBack, NULL);
	if (FAILED(hRet))
		goto Exit0;

	piPlugEng->SetUseExtend(FALSE);
	hRet = piPlugEng->QuickScan(TRUE, TRUE);
	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: ScanRetValue = %x", hRet);
	if (FAILED(hRet))
		goto Exit0;

	pThis->_ReportResult(BkSafeExamItem::MalwareScan, callBack.FoundDanger() ? BkSafeExamItemLevel::Critical : BkSafeExamItemLevel::Safe);

Exit0:

	if (piPlugEng)
	{
		GlobalGetPluginLoader()->FreePlugEngine(piPlugEng);
		piPlugEng = NULL;
	}

	tracer.Log("PluginScan");

	return 0;
}

void CBkSafeExaminer::_LoadLocalInfo()
{
#if 0
	CString strPath;
	DWORD dwRet = 0;

	HRESULT hRet = CAppPath::Instance().GetLeidianDataPath(strPath);
	if (FAILED(hRet))
		return;

	strPath += L'\\';
	strPath += BKSF_EXAM_LOCAL_INFO_FILE_NAME;

#   define APP_EXAM                 L"tips"
#   define KEY_INFO_COUNT           L"count"
#   define APP_EXAM_INFO            L"tips%u"
#   define KEY_EXAM_INFO_WARNING    L"warning"
#   define KEY_EXAM_INFO_INFTEXT    L"inf"
#   define KEY_EXAM_INFO_BUTTONTEXT L"btn"
#   define KEY_EXAM_INFO_NAVIGATE   L"nav"
#   define KEY_EXAM_INFO_DETAILURL  L"url"

	UINT uCount = ::GetPrivateProfileInt(APP_EXAM, KEY_INFO_COUNT, 0, strPath);
	CString strAppName;
	CString strNotice, strOperationText, strNavigate, strDetailUrl;
	BOOL bWarning = FALSE;

	for (UINT i = 0 ; i < uCount; i ++)
	{
		strAppName.Format(APP_EXAM_INFO, i);

		bWarning = ::GetPrivateProfileInt(strAppName, KEY_EXAM_INFO_WARNING, FALSE, strPath);
		dwRet = ::GetPrivateProfileString(strAppName, KEY_EXAM_INFO_INFTEXT, NULL, strNotice.GetBuffer(MAX_PATH + 1), MAX_PATH, strPath);
		strNotice.ReleaseBuffer(dwRet);
		dwRet = ::GetPrivateProfileString(strAppName, KEY_EXAM_INFO_BUTTONTEXT, NULL, strOperationText.GetBuffer(MAX_PATH + 1), MAX_PATH, strPath);
		strOperationText.ReleaseBuffer(dwRet);
		dwRet = ::GetPrivateProfileString(strAppName, KEY_EXAM_INFO_NAVIGATE, NULL, strNavigate.GetBuffer(MAX_PATH + 1), MAX_PATH, strPath);
		strNavigate.ReleaseBuffer(dwRet);
		dwRet = ::GetPrivateProfileString(strAppName, KEY_EXAM_INFO_DETAILURL, NULL, strDetailUrl.GetBuffer(MAX_PATH + 1), MAX_PATH, strPath);
		strDetailUrl.ReleaseBuffer(dwRet);

		_ReportLocalResult(
			BkSafeExamItem::LocalInfo + i, 
			bWarning ? BkSafeExamItemLevel::RiskInfo : BkSafeExamItemLevel::Info, 
			strNotice, 
			strOperationText, 
			strNavigate,
			strDetailUrl
			);
	}

	struct __default_item_res 
	{
		UINT uIDNotice;
		UINT uIDOperation;
		UINT uIDNavigate;
	};

	__default_item_res _default_item[] = {
		{IDS_NOTICE_DEFAULT_1, IDS_OPERATION_DEFAULT_1, IDS_NAVIGATE_DEFAULT_1},
		{IDS_NOTICE_DEFAULT_2, IDS_OPERATION_DEFAULT_2, IDS_NAVIGATE_DEFAULT_2},
	};

	for (UINT i = 0; i + uCount < 2; i ++)
	{
		strNotice = BkString::Get(_default_item[i].uIDNotice);
		strOperationText = BkString::Get(_default_item[i].uIDOperation);
		strNavigate = BkString::Get(_default_item[i].uIDNavigate);
		strDetailUrl.Empty();

		_ReportLocalResult(
			BkSafeExamItem::LocalInfo + i, 
			BkSafeExamItemLevel::Info, 
			strNotice, 
			strOperationText, 
			strNavigate,
			strDetailUrl
			);
	}
#endif
}


//调用IE修复的接口
DWORD WINAPI CBkSafeExaminer::_ExaminationThreadProc_5(LPVOID pvParam)
{
	DWORD dwRet = 0;
	CBkSafeExaminer* _pThis = static_cast<CBkSafeExaminer*>(pvParam);
	if (NULL == _pThis)
		return 1;
	CBkHandle hEventStop;
	CCostTimeTracer tracer;
	tracer.Start();


	hEventStop.Clone(_pThis->m_hEventStop);

	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: %d", BkSafeExamItem::EXAM_GPEDIT_RUN);
	// 体检扩展功能  [3/28/2010 zhangbaoliang]
	_pThis->ExamItemEx();



	// new iefix eng [8/20/2010 zhangbaoliang]
	if (NULL == m_pKSWebIeEng)
	{
		m_pKSWebIeEng = CNewIEFixLoader::GetPtr(_pThis->m_strAppPath)->GetKswebIEFixEng();
	}

	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: KwsInterface = 0x%x", m_pKSWebIeEng);

	if (TRUE == _IsStopping(hEventStop))
		return dwRet;
	if (NULL != m_pKSWebIeEng)
	{
		_pThis->m_arrKwsScanResult.RemoveAll();
		_pThis->m_arrKwsScanResult.SetCount(50);
		m_pKSWebIeEng->Init(_pThis, 2);//enum_BC_exam
		m_pKSWebIeEng->BeginScan(NEW_IE_FIX_SCAN_TYPE_FAST, TRUE);
		CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: KwsInterface, BeginScan");
	}
	if ( m_pKSWebIeEng != NULL )
	{
		CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: KwsInterface, ScanOver");
		m_pKSWebIeEng->Delete();
		m_pKSWebIeEng = NULL;
	}


	//释放掉
	_pThis->m_IEFixEng.IEFix_Reset();

	tracer.Log(("iefix"));

	return dwRet;
}

#if 0
void CBkSafeExaminer::ReportScanResult(CSimpleArray<SCAN_RESULT_EXPORT> arrScanResult)
{

	int nCount = arrScanResult.GetSize();
	if (nCount <= 0)
		return;

	int nItemID = -1;
	nItemID = GetExamItemIDFromGroupID(arrScanResult[0]._nGroupID, arrScanResult[0]._nSubItemID);

	int nLevel = BkSafeExamItemLevel::Safe;

	for (int i = 0; i < nCount; i++)
	{
		SCAN_RESULT_EXPORT pScanData = arrScanResult[i];
		if (1 == pScanData._nLevel ||
			2 == pScanData._nLevel)
		{
			// 			if (BkSafeExamItem::EXAM_HOSTS_FILE == nItemID)
			// 			{//对hosts文件过本地白名单
			// 
			// 			}
			nLevel = BkSafeExamItemLevel::Risk_0;
			break;
		}
	}
	if (nCount > 0)
	{
		if (nItemID >= 0)
			_ReportResult(nItemID, nLevel);
	}
}
#endif

//调用扩展扫描的接口
DWORD WINAPI CBkSafeExaminer::_ExaminationThreadProc_6(LPVOID pvParam)
{
	DWORD dwRet = 0;
	CBkSafeExaminer* _pThis = static_cast<CBkSafeExaminer*>(pvParam);
	if (NULL == _pThis)
		return dwRet;	

	//	_pThis->ExamItemEx();
	return dwRet;
}

#if 0
int WINAPI CBkSafeExaminer::IEFixNotifyMessage(ULONG uMsg, WPARAM wParam, LPARAM lParam, void* lpConext)
{
	int nRet = -1;


	CBkSafeExaminer* _pThis = static_cast<CBkSafeExaminer*>(lpConext);
	if (NULL == _pThis)
		return nRet;

	SCAN_RESULT_EXPORT* pScanData = (SCAN_RESULT_EXPORT*)(lParam);
	if (NULL == pScanData)
		return nRet;

	if (23 == pScanData->_nGroupID)
	{
		BkSafeExamLib::HostFileItem HostItem;
		HostItem.nItemID = pScanData->_nScanIndex;
		HostItem.strHostItem = pScanData->_pszValue;
		HostItem.nGroupID = pScanData->_nGroupID;
		while(HostItem.strHostItem.Find(L"  ") > 0)
			HostItem.strHostItem.Replace(L"  ", L" ");

		if (0 != pScanData->_nLevel)
		{//HOSTS文件过滤本地白名单库文件
			if (0 == _pThis->FilterWithLocalLib(BkSafeExamItem::EXAM_HOSTS_FILE, pScanData->_pszValue))
				pScanData->_nLevel = 0;
		}
		HostItem.nLevel = pScanData->_nLevel;
		_pThis->m_arrHostFileItem.Add(HostItem);
	}
	_pThis->m_arrScanResult.Add(*pScanData);


	return nRet;
}
#endif

#if 0
int CBkSafeExaminer::GetExamItemIDFromGroupID(int nGroupID, int nSubItemID)
{
	int nItemID = -1;
	if (nGroupID < 0)
		return nItemID;

	int nCount = sizeof(g_bksafe_exam_Item)/sizeof(BkSafeExamItem::STRUCT_BKSAFE_EXAM_ITEM);
	for (int i = 0; i < nCount; i++)
	{
		if (nGroupID == g_bksafe_exam_Item[i].nGroupID)
		{
			if (g_bksafe_exam_Item[i].nSubItemID < 0)
			{
				nItemID = g_bksafe_exam_Item[i].dwID;
				break;
			}
			else if (nSubItemID == g_bksafe_exam_Item[i].nSubItemID)
			{
				nItemID = g_bksafe_exam_Item[i].dwID;
				break;
			}

		}
	}

	return nItemID;
}
#endif

HRESULT CBkSafeExaminer::FixItem(int nItemID, void* pExData/* = NULL*/)
{
	HRESULT hr = E_FAIL;
	if (nItemID < 0)
		return hr;

	switch(nItemID)
	{
	case BkSafeExamItem::EXAM_GPEDIT_RUN:
		{
			if (0 == m_examEx.SetGpeditEnable())
				hr = S_OK;
		}
		break;
	case BkSafeExamItem::EXAM_GUEST_RUN:
		{
			if (0 == m_examEx.SetGuestUserDisable())
				hr = S_OK;
		}
		break;
	case BkSafeExamItem::EXAM_HIDE_FOLDER_RUN:
		{
			if (0 == m_examEx.ShowHideFileEnable())
				hr = S_OK;
		}
		break;
	case BkSafeExamItem::EXAM_SHARE_RESOUCE:
		{
			if (0 == m_examEx.RemoveShareResource((LPCTSTR)pExData))
				hr = S_OK;
		}
		break;
	case BkSafeExamItem::EXAM_REMOTE_DESKTOP:
		{
			if (0 == m_examEx.SetRemoteDesktopDisable())
				hr = S_OK;
		}
		break;
	case  BkSafeExamItem::EXAM_HOSTS_FILE:
		{
			if (0 == SaveHostFile())
				hr = S_OK;
		}
		break;
	case BkSafeExamItem::EXAM_DRIVERORDIRECTORYOPEN:
		{
			if (0 == m_examEx.DriverOrDirectoryOpenEnable())
				hr = S_OK;
		}
	default:
		break;
	}

	return hr;
}

HRESULT CBkSafeExaminer::ResetIgnoreArray()
{
	m_arrIgnoreItems.RemoveAll();
	return S_OK;
}


HRESULT CBkSafeExaminer::PushAnIgnoreItem(int nItemID)
{
	HRESULT hr = S_FALSE;

	if (nItemID <= 0)
		return hr;

	m_arrIgnoreItems.Add(nItemID);
	hr = S_OK;

	return hr;
}


HRESULT CBkSafeExaminer::GetItemCount(int nItemID, int& nCount)
{
	HRESULT hr = S_OK;
	if (nItemID == BkSafeExamItem::EXAM_SHARE_RESOUCE)
	{
		nCount = m_examEx.GetShareResourceCount();
	}else if (nItemID == BkSafeExamItem::EXAM_HOSTS_FILE)
	{
		nCount = m_arrHostFileItem.GetSize();
	}

	return hr;
}

HRESULT CBkSafeExaminer::EnumItemData(int nItemID, int nIndex, void** pData)
{
	HRESULT hr = S_OK;
	if (nItemID == BkSafeExamItem::EXAM_SHARE_RESOUCE)
	{
		static BKSafeExamItemEx::SHARE_RESOURCES res;
		//	ZeroMemory(&res, sizeof(res));

		res = m_examEx.EnumAnResources(nIndex);
		if (TRUE == res.pszPath.IsEmpty())
			hr = S_FALSE;
		(*pData) = &res;
	}else if (nItemID == BkSafeExamItem::EXAM_HOSTS_FILE)
	{
		if (nIndex >= m_arrHostFileItem.GetSize())
		{
			hr = S_FALSE;
		}
		else
		{
			(*pData) = (void*)&(m_arrHostFileItem[nIndex]);
		}
	}
	return hr;
}


BOOL CBkSafeExaminer::ItemIsIgnored(int nItemID)
{
	BOOL bRet = FALSE;
	if (nItemID < 0 )
		return bRet;

	if (m_arrIgnoreItems.Find(nItemID) >= 0)
		bRet = TRUE;

	return bRet;
}



int CBkSafeExaminer::_SaveToLocalLib(BkSafeExamLib::LocalLibInfo& libLocal)
{
	int nRet = -1;

	CString strLibFilePath;
	CAppPath::Instance().GetLeidianCfgPath(strLibFilePath, TRUE);
	strLibFilePath += LOCALLIB_FILE_NAME;

	if (libLocal.arrLib.GetCount() < 1)
	{
		DeleteFile(strLibFilePath);
		return nRet;
	}


	CString strLibFileValue;
	BDX_ToJson(libLocal, strLibFileValue, FALSE);
	if (FALSE == strLibFileValue.IsEmpty())
	{
		int nLen = strLibFileValue.GetLength()*sizeof(TCHAR);
		PBYTE pValue = (PBYTE)strLibFileValue.GetBuffer(nLen);
		for (int i = 0; i < nLen; i++)
		{
			pValue[i] ^= LOCALLIB_KEY;
		}
		CAtlFile atlFile;
		if (S_OK == atlFile.Create(strLibFilePath, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS))
		{
			if (S_OK == atlFile.Write((void*)pValue, nLen))
				nRet = 0;
		}
		strLibFileValue.ReleaseBuffer(nLen);
	}

	return nRet;
}

int CBkSafeExaminer::_LoadLocalLib(BkSafeExamLib::LocalLibInfo& libLocal)
{
	int nRet = -1;
	CString strLibFilePath;
	CAppPath::Instance().GetLeidianCfgPath(strLibFilePath, TRUE);
	strLibFilePath += LOCALLIB_FILE_NAME;

	CAtlFile atlFile;
	if (S_OK == atlFile.Create(strLibFilePath, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING))
	{
		ULONGLONG uSize = 0;
		DWORD dwLen = 0;
		if (S_OK != atlFile.GetSize(uSize))
			return nRet;
		dwLen = reinterpret_cast<DWORD&>(uSize);
		if (dwLen > 0)
		{
			PBYTE pValue = new BYTE[dwLen+4];
			ZeroMemory(pValue, dwLen+4);
			DWORD nByteRet = 0;
			if (S_OK == atlFile.Read((void*)pValue, dwLen, nByteRet))
			{
				for (int i = 0; i < dwLen; i++)
				{
					pValue[i] ^= LOCALLIB_KEY;
				}
				if (NULL != pValue)
				{
					CString strValue((LPTSTR)pValue);
					BDX_FromJson(strValue, libLocal, FALSE);
				}
			}
			if (NULL != pValue)
			{
				delete []pValue;
				pValue = NULL;
			}
		}
	}

	return nRet;
}

int CBkSafeExaminer::ExamItemEx()
{
	int nRet = -1;

	//组策略
	if (FALSE == ItemIsIgnored(BkSafeExamItem::EXAM_GPEDIT_RUN))
	{
		if (0 == m_examEx.GpeditIsDisable())
		{
			_ReportResult(BkSafeExamItem::EXAM_GPEDIT_RUN, BkSafeExamItemLevel::Risk_0);
		}
		else
		{
			_ReportResult(BkSafeExamItem::EXAM_GPEDIT_RUN, BkSafeExamItemLevel::Safe);
		}
	}

	//GUEST账户
	if (FALSE == ItemIsIgnored(BkSafeExamItem::EXAM_GUEST_RUN))
	{
		if (0 == m_examEx.GuestUserIsEnable())
		{
			DWORD dwLevel = BkSafeExamItemLevel::Risk_0;
			if (0 == FilterWithLocalLib(BkSafeExamItem::EXAM_GUEST_RUN, EXAM_GUEST_STR))
				dwLevel = BkSafeExamItemLevel::Safe;

			_ReportResult(BkSafeExamItem::EXAM_GUEST_RUN, dwLevel);

		}
		else
		{
			_ReportResult(BkSafeExamItem::EXAM_GUEST_RUN, BkSafeExamItemLevel::Safe);
		}
	}

	//共享资源
	if (FALSE == ItemIsIgnored(BkSafeExamItem::EXAM_SHARE_RESOUCE))
	{
		int nRet = m_examEx.IsHaveShareResource();
		if (0 == nRet)
		{
			int nCount = m_examEx.GetShareResourceCount();
			for (int i = nCount-1; i >= 0; i--)
			{
				SHARE_RESOURCES res = m_examEx.EnumAnResources(i);
				if (0 == FilterWithLocalLib(BkSafeExamItem::EXAM_SHARE_RESOUCE, res.pszPath))
				{
					m_examEx.SetItemShareResourceSafe(res.pszPath);
				}
			}
		}

		if (m_examEx.GetShareResourceCount())
		{
			_ReportResult(BkSafeExamItem::EXAM_SHARE_RESOUCE, BkSafeExamItemLevel::Risk_0);
		}
		else
		{
			_ReportResult(BkSafeExamItem::EXAM_SHARE_RESOUCE, BkSafeExamItemLevel::Safe);
		}
	}

	//远程桌面
	if (FALSE == ItemIsIgnored(BkSafeExamItem::EXAM_REMOTE_DESKTOP))
	{
		if (0 == m_examEx.RemoteDesktopIsEnable())
		{
			DWORD dwLevel = BkSafeExamItemLevel::Risk_0;
			if (0 == FilterWithLocalLib(BkSafeExamItem::EXAM_REMOTE_DESKTOP, EXAM_REMOTEDESK_STR))
				dwLevel = BkSafeExamItemLevel::Safe;

			_ReportResult(BkSafeExamItem::EXAM_REMOTE_DESKTOP, dwLevel);
		}
		else
		{
			_ReportResult(BkSafeExamItem::EXAM_REMOTE_DESKTOP, BkSafeExamItemLevel::Safe);
		}
	}
	//隐藏文件夹
	if (FALSE == ItemIsIgnored(BkSafeExamItem::EXAM_HIDE_FOLDER_RUN))
	{
		if (0 == m_examEx.ShowHideFileIsEnable())
		{
			_ReportResult(BkSafeExamItem::EXAM_HIDE_FOLDER_RUN, BkSafeExamItemLevel::Risk_0);
		}
		else
		{
			_ReportResult(BkSafeExamItem::EXAM_HIDE_FOLDER_RUN, BkSafeExamItemLevel::Safe);
		}
	}

	//Driver,Driectory
	if (0 == m_examEx.DriverOrDirectoryOpenIsEnable())
	{
		_ReportResult(BkSafeExamItem::EXAM_DRIVERORDIRECTORYOPEN, BkSafeExamItemLevel::Risk_0);
	}
	else
	{
		_ReportResult(BkSafeExamItem::EXAM_DRIVERORDIRECTORYOPEN, BkSafeExamItemLevel::Safe);
	}


	return nRet;
}
//现在只有两种需要本地库文件判断(hosts文件和共享资源)
int CBkSafeExaminer::FilterWithLocalLib(int nGroupID, CString strValue)
{
	int nRet = -1;
	if (nGroupID < 0 || TRUE == strValue.IsEmpty())
		return nRet;

	int nCount = m_bkLocalLib.arrLib.GetCount();//m_arrHostFileItem.GetSize();
	BkSafeExamLib::LocalLib libLocal;
	for (int i = 0; i < nCount; i++)
	{
		libLocal = m_bkLocalLib.arrLib[i];
		if (nGroupID == libLocal.nType)
		{
			if (BkSafeExamItem::EXAM_HOSTS_FILE == nGroupID)
			{//比较hosts文件 
				nRet = _HostsItemIsSafe(strValue ,libLocal.strValue);
			}
			else
			{
				nRet = strValue.CompareNoCase(libLocal.strValue);
			}
		}
		if (0 == nRet)
			break;
	}

	return nRet;
}


int CBkSafeExaminer::_HostsItemIsSafe(LPCTSTR pszHostsItem, LPCTSTR pszLibItem)
{
	int nRet = -1;
	CAtlArray<CString> arrItem;
	//Lib hosts ("127.0.0.1","pvs360.com")
	if (0 != CCommon::GetCommonPtr()->ParsexmlCmdLine(pszLibItem, arrItem))
		return nRet;

	size_t tCount = arrItem.GetCount();
	if (2 != tCount)
		return nRet;


	CString strValue1(TEXT("")), strValue2(TEXT("")), strValue(TEXT(""));
	strValue1 = arrItem[0];
	strValue2 = arrItem[1];
	strValue = pszHostsItem;


	if (FALSE == strValue1.IsEmpty() && 
		FALSE == strValue2.IsEmpty())
	{//ip+url
		if (0 == strValue.Find(strValue1) && 
			strValue.Find(strValue2) > 0)
			nRet = 0;
	}else if (FALSE == strValue1.IsEmpty())
	{//ip,
		if (strValue.Find(strValue1) == 0)//确保是从开始位置找到的
			nRet = 0;
	}else if (FALSE == strValue2.IsEmpty() )
	{//url
		if (strValue.Find(strValue2) > 0)
			nRet = 0;
	}

	return nRet;
}

LRESULT CBkSafeExaminer::SetItemSafe(int nItemID, void** pData)
{
	HRESULT hr = S_FALSE;
	if (nItemID < 0)
		return hr;

	BkSafeExamLib::LocalLib libLocal;

	if (nItemID == BkSafeExamItem::EXAM_SHARE_RESOUCE)
	{
		LPTSTR pszValue = (LPTSTR)(*pData);
		if (NULL == pszValue)
			return hr;
		if (0 == m_examEx.SetItemShareResourceSafe(pszValue))
		{
			libLocal.strValue = pszValue;
			hr = S_OK;
		}

	}else if (nItemID == BkSafeExamItem::EXAM_HOSTS_FILE)
	{
		BkSafeExamLib::HostFileItem* pHostItem = NULL;
		pHostItem = (BkSafeExamLib::HostFileItem*)(*pData);

		if (NULL == pHostItem)
			return hr;

		if (0 == _SetHostItemSafe(pHostItem))
		{
			libLocal.strValue = pHostItem->strHostItem;
			hr = S_OK;
		}
	}else if (nItemID == BkSafeExamItem::EXAM_GUEST_RUN)
	{
		libLocal.strValue = (LPTSTR)(pData);//L"GUEST";
		hr = S_OK;
	}else if (nItemID == BkSafeExamItem::EXAM_REMOTE_DESKTOP)
	{
		libLocal.strValue = (LPTSTR)(pData);//L"REMOTEDESK";
		hr = S_OK;
	}else if (nItemID == BkSafeExamItem::EXAM_CHECK_AV)
	{
		libLocal.strValue = (LPTSTR)(pData);//L"Duba";
		hr = S_OK;
	}

	if (S_OK == hr)
	{
		libLocal.nType = nItemID;
		m_bkLocalLib.arrLib.Add(libLocal);
	}

	return hr;
}

int CBkSafeExaminer::_SetHostItemSafe(BkSafeExamLib::HostFileItem* pHostItem, int nLevel/* = 0*/)
{
	int nRet = -1;
	if (NULL == pHostItem)
		return nRet;
	int nCount = m_arrHostFileItem.GetSize();

	for (int i = 0; i < nCount; i++)
	{
		if (m_arrHostFileItem[i].nItemID == pHostItem->nItemID)
		{
			m_arrHostFileItem[i].nLevel = nLevel;
			nRet = 0;
			break;
		}
	}

	return nRet;
}

int CBkSafeExaminer::_SetHostItemUnSafe(BkSafeExamLib::HostFileItem* pHostItem)
{
	int nRet = -1;
	if (NULL == pHostItem)
		return nRet;
	CAtlArray<CString> arrHostItem;
	CCommon::GetCommonPtr()->ParsexmlCmdLine(pHostItem->strHostItem, arrHostItem);
	if (2 != arrHostItem.GetCount())
		return nRet;

	CString strValue;
	strValue.Format(L"%s %s", arrHostItem[0], arrHostItem[1]);
	int nCount = m_arrHostFileItem.GetSize();

	for (int i = nCount-1; i >= 0; i--)
	{
		//if (m_arrHostFileItem[i].nItemID == pHostItem->nItemID)
		if (0 == strValue.CompareNoCase(m_arrHostFileItem[i].strHostItem) && 
			0 == m_arrHostFileItem[i].nLevel)
		{
			m_arrHostFileItem[i].nLevel = 1;
			nRet = 0;
			break;
		}
	}
	return nRet;
}

int CBkSafeExaminer::SaveHostFile()
{
	int nRet = 0;

	TCHAR pszHostFilePath[FILEPATH_MAX_LEN] = {0};
	if (FALSE == SHGetSpecialFolderPath(NULL, pszHostFilePath, CSIDL_SYSTEM, FALSE) || 
		0 == _tcslen(pszHostFilePath) ||
		FALSE == PathAppend(pszHostFilePath, TEXT("drivers\\etc\\hosts")))
		return nRet;

	CBkHandle hEventStop;
	hEventStop.Clone(m_hEventStop);

	CLDEditTxtFile hostsFile(pszHostFilePath);	
	int nCount = m_arrHostFileItem.GetSize();
	CSimpleArray<CString> arrItem;

	for (int i = 0; i < nCount; i++)
	{

		if (_IsStopping(hEventStop))
			return nRet;

		BkSafeExamLib::HostFileItem HostItem = m_arrHostFileItem[i];
		CString strValue = HostItem.strHostItem;
		if (FALSE == strValue.IsEmpty())
		{
			strValue.TrimLeft();
			if (3 == HostItem.nLevel)
			{
				HostItem.strHostItem.Format(TEXT("#%s"), strValue);
			}
			else if (1 == HostItem.nLevel ||
				2 == HostItem.nLevel)
			{
				nRet = -1;
			}
		}
		arrItem.Add(HostItem.strHostItem);
	}
	if (0 != nCount)
	{//文件有修改的时候才做这个处理
		CString strBakFile;
		strBakFile.Format(TEXT("%s_bak"), pszHostFilePath);
		//DeleteFile(strBakFile);//删除备份文件
		CopyFile(pszHostFilePath, strBakFile, FALSE);
		//DeleteFile(pszHostFilePath);//删除原来的文件
#if 0
		hostsFile.WriteItemsToFile(TEXT("g:\\hosts"), arrItems);
#else
		hostsFile.WriteItemsToFile(pszHostFilePath, arrItem);
#endif
		//m_arrHostFileItem.RemoveAll();

	}
	return nRet;
}

void CBkSafeExaminer::_ResetArray()
{
	m_arrHostFileItem.RemoveAll();
	m_arrIgnoreItems.RemoveAll();
	m_arrScanResult.RemoveAll();
}

HRESULT CBkSafeExaminer::SetHostItemFix(int nIndex)
{
	HRESULT hr = S_FALSE;
	if (nIndex < 0)
		return hr;

	int nCount = m_arrHostFileItem.GetSize();

	for (int i = 0; i < nCount; i++)
	{
		if (m_arrHostFileItem[i].nItemID == nIndex)
		{
			m_arrHostFileItem[i].nLevel = 3;
			hr = S_OK;
			break;
		}
	}
	return hr;
}


HRESULT CBkSafeExaminer::RemoveLocalLibItem(int nItemID, void* pValue)
{
	HRESULT hr = E_FAIL;
	if (NULL == pValue || nItemID < 0)
		return hr;
	//BkSafeExamLib::LocalLibInfo m_bkLocalLib;
	BOOL bFound = FALSE;
	BkSafeExamLib::HostFileItem* pData = NULL;
	CString strValue = L"";
	if (nItemID == BkSafeExamItem::EXAM_HOSTS_FILE)
	{
		pData = (BkSafeExamLib::HostFileItem*)pValue;
		if (NULL != pData)
			strValue = pData->strHostItem;
	}
	else
	{
		strValue = (LPTSTR)pValue;
	}

	int nCount = m_bkLocalLib.arrLib.GetCount();
	for (int i = nCount-1; i >= 0; i--)
	{
		BkSafeExamLib::LocalLib lib = m_bkLocalLib.arrLib[i];
		if (nItemID == lib.nType && 0 == lib.strValue.CompareNoCase(strValue))
		{
			bFound = TRUE;
			if (nItemID == BkSafeExamItem::EXAM_SHARE_RESOUCE)
			{
				m_examEx.SetItemShareResourceUnSafe(strValue.GetBuffer(-1));
				strValue.ReleaseBuffer(-1);
			}else if (nItemID == BkSafeExamItem::EXAM_HOSTS_FILE)
			{
				_SetHostItemUnSafe(pData);
			}
			m_bkLocalLib.arrLib.RemoveAt(i);
			//break;
		}
	}
	if (TRUE == bFound)
		hr = S_OK;

	return hr;

}
HRESULT CBkSafeExaminer::GetSafeLibItemCount(int& nCount)
{
	HRESULT hr = E_FAIL;
	nCount = m_bkLocalLib.arrLib.GetCount();
	return S_OK;
}

HRESULT CBkSafeExaminer::EnumSafeLibItem(int nIndex, BkSafeExamLib::LocalLibExport& libItem)
{
	HRESULT hr = E_FAIL;
	int nCount = m_bkLocalLib.arrLib.GetCount();
	if (0 == nCount ||
		nIndex >= nCount)
		return hr;

	ZeroMemory(&libItem, sizeof(libItem));
	libItem.nType = m_bkLocalLib.arrLib[nIndex].nType;
	StringCbPrintf(libItem.pszValue, 1024, L"%s", m_bkLocalLib.arrLib[nIndex].strValue);

	hr = S_OK;
	return hr;
}
//体检垃圾
DWORD WINAPI CBkSafeExaminer::_ExaminationThreadProc_7(LPVOID pvParam)
{
	DWORD retval = 0;
	CBkSafeExaminer* pThis = static_cast<CBkSafeExaminer*>(pvParam);
	DWORD dwLevel = BkSafeExamItemLevel::Safe;
	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: %d", BkSafeExamItem::EXAM_TRASH);

	int nRet = pThis->HasTrash();
	if ( nRet < 0)
		dwLevel = BkSafeExamItemLevel::Scan_Error;
	else if (nRet > 0)
		dwLevel = BkSafeExamItemLevel::Optimization;

	pThis->_ReportResult(BkSafeExamItem::EXAM_TRASH, dwLevel);//返回检测结果

	return retval;
}

HRESULT CBkSafeExaminer::LocalLibIsHaveItem(int nItemID, BOOL& bHave)
{
	HRESULT hr = E_FAIL;
	bHave = FALSE;
	int nCount = m_bkLocalLib.arrLib.GetCount();
	if (0 == nCount ||
		nItemID < 0)
		return hr;

	for (int i = 0; i < nCount; i++)
	{
		if (nItemID == m_bkLocalLib.arrLib[i].nType)
		{
			bHave = TRUE;
			break;
		}
	}
	hr = S_OK;

	return hr;
}

HRESULT CBkSafeExaminer::StartupAnUse(int nItemID)
{
	HRESULT hr = E_FAIL;
	if (nItemID < 0)
		return hr;
	if ( BkSafeExamItem::EXAM_GUEST_RUN == nItemID)
	{//启用GUEST账户
		CExamCheckItemsEx Exam;
		if (0 == Exam.StartupGuest())
			hr = S_OK;
	}else if (BkSafeExamItem::EXAM_REMOTE_DESKTOP == nItemID)
	{
		CExamCheckItemsEx Exam;
		if (0 == Exam.StartupRemoteDesktop())
			hr = S_OK;
	}
	return hr;
}

void CBkSafeExaminer::KsIeFix_ScanProgress(KSWebIEFixProgressData* pData)
{
	if (NULL == pData)
		return;

	CBkHandle hEventStop;
	hEventStop.Clone(m_hEventStop);

	int nExamID = -1;
	KSWebIeScanData *pKSWebIeScanData = m_arrKwsScanResult.GetData();
	for (int i = 0; i < pData->nsize && NULL != pKSWebIeScanData; i++)
	{		
		memcpy((pKSWebIeScanData+i), &pData->arrayx[i], sizeof(KSWebIeScanData));
		//_NotifyKwsScanResult(i, pData->arrayx[i]);
	}

	m_nKwsProgress = pData->nRate * 3 / 10;
	SAFE_CALL_FUN(m_piCallback, OnProgress( m_nTrojanProgress + m_nKwsProgress));

	if (TRUE == _IsStopping(hEventStop) && 
		NULL != m_pKSWebIeEng)
		m_pKSWebIeEng->StopScan();

}
void CBkSafeExaminer::KsIeFix_ScanEnd(BOOL bStopped)
{
	for (int i = 0; i < m_arrKwsScanResult.GetCount(); i++)
	{
		_NotifyKwsScanResult(i, m_arrKwsScanResult[i]);
	}
	m_arrKwsScanResult.RemoveAll();
}
void CBkSafeExaminer::KsIeFix_ScanStart()
{
	//m_arrKwsScanResult.RemoveAll();
}

int CBkSafeExaminer::_NotifyKwsScanResult(int nItem, KSWebIeScanData& scanData)
{
	int nLevel = BkSafeExamItemLevel::Safe, nExamID = -1;

	do 
	{
		if (FALSE == scanData.bScanning &&
			TRUE == scanData.bScanned)
		{//确认某项扫描完成，就要告诉主界面程序
			nExamID = ExamPubFunc::CExamPubFunc::GetPubFuncPtr()->ConvertKWSIDToExamID(nItem);
			if (-1 == nExamID)
				break;
			//continue;

			BKSafeExamItemEx::BKSafeExamEx bkSafeExamLeak;
			bkSafeExamLeak.bInvalid = FALSE;
			bkSafeExamLeak.u.ExamKwsResultEx.Reset();
			bkSafeExamLeak.u.ExamKwsResultEx.nDangerCount = scanData.nDanger;
			bkSafeExamLeak.u.ExamKwsResultEx.nItemID = nExamID;


			if (scanData.nDanger > 0)
			{
				CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: _NotifyKwsScanResult --> examid = %d  is Danger  DangerCount = %d", nExamID, scanData.nDanger);
				bkSafeExamLeak.bInvalid = TRUE;
				nLevel = BkSafeExamItemLevel::Risk_0;
			}
			if (-1 != nExamID)
				_ReportResult(nExamID, nLevel, &bkSafeExamLeak);
		}
	} while (FALSE);

	return nExamID;
}

HRESULT WINAPI CBkSafeExaminer::ExamItem(int nItemID, BKSafeExamItemEx::BKSafeExamEx& ExamItemData)
{
	HRESULT hr = E_FAIL;
	if (BkSafeExamItem::EXAM_TRASH == nItemID)
	{
		int nValue = HasTrash();
		if (nValue >= 0)
		{
			ExamItemData.bInvalid = TRUE;
			ExamItemData.u.Trash.nSize = nValue;
			hr = S_OK;
		}			
	}
	return hr;
}

int CBkSafeExaminer::HasTrash()
{
	int nRet = -1;
	HMODULE hTrashCleaner = NULL;
	WCHAR szExePath[MAX_PATH] = { 0 };
	WCHAR szDllPath[MAX_PATH] = { 0 };
	HasTrash_t HasTrash = NULL; 
	BOOL fHasTrash = FALSE;
	wcscpy_s(szDllPath, MAX_PATH, m_strAppPath);
	PathAppendW(szDllPath, L"kclear.dll");

	if (GetFileAttributesW(szDllPath) == INVALID_FILE_ATTRIBUTES)
	{
		goto clean0;
	}

	hTrashCleaner = ::LoadLibraryW(szDllPath);
	if (!hTrashCleaner)
	{
		goto clean0;
	}

	HasTrash = (HasTrash_t)::GetProcAddress(hTrashCleaner, "HasTrash");
	if (!HasTrash )
		goto clean0;

	fHasTrash = HasTrash();


	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: fHasTrash = %d", fHasTrash);
	nRet = fHasTrash;

clean0:
	if (hTrashCleaner)
	{
		FreeLibrary(hTrashCleaner);
		hTrashCleaner = NULL;
	}

	return nRet;
}

//启动优化
DWORD WINAPI CBkSafeExaminer::_ExaminationThreadProc_8(LPVOID pvParam)
{
	DWORD retval = 0;
	CBkSafeExaminer* pThis = static_cast<CBkSafeExaminer*>(pvParam);
	if (NULL == pThis)
		return 0;

	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: %d", BkSafeExamItem::EXAM_RUNOPT_ONKEY);

	DWORD dwLevel = BkSafeExamItemLevel::Safe;
	static CBkHandle hEventStop;
	hEventStop.Clone(pThis->m_hEventStop);


	DWORD dwFlag = 0;
	static BOOL bScanEnd = FALSE;
	static DWORD dwRunCount = 0, dwOneKeyRunOpt = 0;
	static CString strPath = pThis->m_strAppPath;

	class CFindRunOptItem:public IEnumRunCallBackNull
	{
	public:
		CFindRunOptItem()
		{
			dwRunCount = 0;
			dwOneKeyRunOpt = 0;
			pRunOptEng = NULL;
			bScanEnd = FALSE;
			pRunOptEng = CRunOptLoader::GetGlobalPtr(strPath)->GetOptEng();
			CKSafeExamLog::GetLogPtr()->WriteLog(L"_ExaminationThreadProc_8: runoptInterface = 0x%x", pRunOptEng);
		}
		~CFindRunOptItem()
		{
			if (pRunOptEng!=NULL)
			{
				CRunOptLoader::GetGlobalPtr()->FreeOptEng(pRunOptEng);
				pRunOptEng = NULL;
			}
		}
		void StartFind()
		{
			if (NULL != pRunOptEng)
				pRunOptEng->EnumRunInfoForExam(0, this);
			else
				bScanEnd = TRUE;
		}
		virtual BOOL FindRunItem(CKsafeRunInfo* pInfo)
		{
			if (_IsStopping(hEventStop) || 0 != dwOneKeyRunOpt)
			{
				bScanEnd = TRUE;
				return TRUE;
			}

			if (NULL != pInfo)
			{
				CString strLog;
				if ( TRUE == pInfo->bEnable)
				{
					dwRunCount++;
					if ( pInfo->IsAdviseStop() && !pInfo->bHasIgnored)
					{
						CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: _ExaminationThreadProc_8 --> %s", pInfo->strName);
						dwOneKeyRunOpt++;
						return TRUE;
					}
				}
			}
			return FALSE;
		}
		virtual void BeginScan(DWORD nType)
		{
			dwRunCount = 0;
			dwOneKeyRunOpt= 0;
		}
		virtual void EndScan()
		{
			bScanEnd = TRUE;
		}
		virtual void RestoreEnd(){return;}
		virtual void PostRestoreRunItem(DWORD nType,BOOL bEnable,LPCTSTR lpName){return;}
	private:
		IRunOptimizeEng* pRunOptEng;

	};
	CFindRunOptItem RunItem;
	RunItem.StartFind();
	while (TRUE)
	{//等待启动项扫描完成
		Sleep(100);
		if (TRUE == bScanEnd)
		{
			break;
		}
	}

	if ( 0 != dwOneKeyRunOpt )
		dwLevel = BkSafeExamItemLevel::Optimization;


	pThis->_ReportResult(BkSafeExamItem::EXAM_RUNOPT_ONKEY, dwLevel);//返回检测结果

	return retval;
}

//检测软件升级
DWORD WINAPI CBkSafeExaminer::_ExaminationThreadProc_9(LPVOID pvParam)
{
	DWORD retval = 0;
	CBkSafeExaminer* pThis = static_cast<CBkSafeExaminer*>(pvParam);
	if (NULL == pThis)
		return 0;

	DWORD dwLevel = BkSafeExamItemLevel::Safe;

	int nValue = -1;

	if (0 != pThis->CheckSoftWareUpdate(nValue))
		goto Exit0;

	BKSafeExamItemEx::BKSafeExamEx bkSafeExamData;
	bkSafeExamData.bInvalid = TRUE;
	if (nValue > 0)
	{
		dwLevel = BkSafeExamItemLevel::Optimization;
		bkSafeExamData.u.BKSafeUpdate.Reset();
		bkSafeExamData.u.BKSafeUpdate.nNeedUpdateCount = nValue;
	}

Exit0:

	pThis->_ReportResult(BkSafeExamItem::EXAM_CHECK_UPDATE, dwLevel, &bkSafeExamData);//返回检测结果

	return retval;
}

int CBkSafeExaminer::CheckSoftWareUpdate(int& nUpdateCount)
{
	int nRet = -1;

	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: CheckSoftWareUpdate");

	// 	CString strIniFile;
	// 	CAppPath::Instance().GetLeidianCfgPath(strIniFile);
	// 	strIniFile.Append(L"\\bksoftmgr.ini");
	// 	IniFileOperate::CIniFile IniFile(strIniFile);
	// 
	// 	if (FALSE == PathFileExists(strIniFile))
	// 		return nRet;

	nUpdateCount = 0;

	DWORD dwUpdateTime = 0;
	BKSafeConfig::GetSoftUpdateCount2(nUpdateCount, dwUpdateTime);

	CTimeSpan span((DWORD)_time32(NULL)-dwUpdateTime);
	DWORD dwValue = _time32(NULL);
	CKSafeExamLog::GetLogPtr()->WriteLog(L"logtime: %s", span.Format(L"day :%D, hours: %H, mins: %M, secs: %S"));

	CKSafeExamLog::GetLogPtr()->WriteLog(L"BksafeExam: CheckSoftWareUpdate count = %d, ntime = %d ntime2 = %d,ndays=%d", nUpdateCount, dwUpdateTime, dwValue, span.GetDays());

	if (span.GetDays() > 3)
		return nRet;
	if ( span.GetDays() <= 3 &&
		nUpdateCount > 0)
	{
		nRet = 0;
	}

	return nRet;

}

DWORD CBkSafeExaminer::_ExaminationThreadProc_10(LPVOID pvParam)
{
	CBkSafeExaminer* _pThis = static_cast<CBkSafeExaminer*>(pvParam);
	if (NULL == pvParam)
		return 0;

	CKSafeExamLog::GetLogPtr()->WriteLog(L"_ExaminationThreadProc_10: Start Exam");

	DWORD dwLevel = BkSafeExamItemLevel::Safe;
	CString strLibPath = L"", strAVInfo = L"";
	AVProductMgr::IAVProductMgr* ptrIngerface = AVProductMgr::CAVMgrInterface::GetPtr(_pThis->m_strAppPath)->CrateInterface();
	if (NULL == ptrIngerface)
		goto Exit0;

	CAppPath::Instance().GetLeidianDataPath(strLibPath);
	strLibPath.Append(L"\\knavlib.dat");
	CKSafeExamLog::GetLogPtr()->WriteLog(L"_ExaminationThreadProc_10: LoadAVLib %s", strLibPath);
	if (FALSE == ptrIngerface->LoadAVLib(strLibPath))
	{
		CKSafeExamLog::GetLogPtr()->WriteLog(L"_ExaminationThreadProc_10: LoadAVLib FALSE");
		//goto Exit0;
	}

	strAVInfo = ptrIngerface->GetInstallAVs();
	CKSafeExamLog::GetLogPtr()->WriteLog(L"_ExaminationThreadProc_10: GetInstallAVs => %s", strAVInfo);

#if _DEBUG
	if (TRUE)
#else
	if (TRUE == strAVInfo.IsEmpty())
#endif
	{
		dwLevel = BkSafeExamItemLevel::Risk_0;
		if (0 == _pThis->FilterWithLocalLib(BkSafeExamItem::EXAM_CHECK_AV, EXAM_DUBA))
			dwLevel = BkSafeExamItemLevel::Safe;

	}

	_pThis->_ReportResult(BkSafeExamItem::EXAM_CHECK_AV, dwLevel);//返回检测结果

Exit0:

	return 0;
}

void CBkSafeExaminer::_CheckKwsExamResult()
{
	BKSAFE_EXAM_RESULT_ITEM kwsItem;
	BOOL m_bKwsExist = TRUE;//( _IsExistKWS() == 0 );

	//如果体检未安装网盾超过2次，则不检测网盾
// 	int nKwsCnt = BKSafeConfig::GetKwsNoUseCnt();
// 	if(  nKwsCnt > 2 && !m_bKwsExist)
// 		return;

	BOOL bKwsAllOpen = _IsAllKwsOpen() == 0;
// 	if( !m_bKwsExist )
// 		BKSafeConfig::SetKwsNoUseCnt( ++nKwsCnt );

	if( bKwsAllOpen && m_bKwsExist )
	{//安全
		kwsItem.nLevel = BkSafeExamItemLevel::Safe;
	}
	else
	{//危险
		kwsItem.nLevel = BkSafeExamItemLevel::Risk_0;
	}
	_ReportResult(BkSafeExamItem::EXAM_KWS_MON, kwsItem.nLevel);//返回检测结果
}


int CBkSafeExaminer::_IsAllKwsOpen()
{
	//kws_old_ipc::KwsSetting setting;
	KwsSetting setting;
	int nErrorCount = 0;
	//网页保护
	BOOL bKwsEnable = setting.IsAntiTrojanEnable();
	if( !bKwsEnable )
		nErrorCount++;

	//钓鱼保护
	BOOL bFishing = setting.IsFishingEnable();
	if( !bFishing )
		nErrorCount++;

	//下载保护
	// 	if (FALSE == setting.IsAutoScanEnable())
	// 		nErrorCount++;

	//	DWORD dwVal;
	//搜索引擎保护
	// 	setting.GetSearchEngine(dwVal);
	// 	BOOL bSearch  = (dwVal > 0);
	// 	if( !bSearch )
	// 		nErrorCount++;

	//主页锁定
	// 	if (FALSE == setting.IsLockSPEnable())
	// 		nErrorCount++;

	/* 广告过滤不影响安全状态
	//广告过滤
	if (FALSE == setting.IsAnyRuleSubscribed())
	nErrorCount++;
	*/
	return nErrorCount;
}

BOOL CBkSafeExaminer::_IsVirusLevel(const EM_POINT_LEVEL& emLevel)
{
	BOOL bIsVirus = FALSE;
	switch(emLevel)
	{
	case enum_POINT_LEVEL_VIRUS:
	case enum_POINT_LEVEL_TROJAN:
	case enum_POINT_LEVEL_BACKDOOR:
	case enum_POINT_LEVEL_WORM:
	case enum_POINT_LEVEL_RISK:
	case enum_POINT_LEVEL_ROOTKIT:
	case enum_POINT_LEVEL_DANGER:
		bIsVirus = TRUE;
		break;

	default:
		break;
	}

	return bIsVirus;
}
