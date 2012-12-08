
#include "stdafx.h"
#include "bksafeexamcall.h"
#include "beikesafe.h"
#include <wtlhelper/whwindow.h>
#include <common/utility.h>
#include <safemon/safetrayshell.h>
typedef bool (__stdcall*SetNotifyHwnd_t)(HWND);

DWORD g_dwMonitorIDList[] = {
	SM_ID_RISK, 
	SM_ID_PROCESS, 
	SM_ID_UDISK, 
	SM_ID_LEAK,
};
DWORD g_dwNetMonitorIDList[] = {  
    SM_ID_KWS_SAFE_DOWN,
    SM_ID_KWS
};

CBkSafeExamCaller::CBkSafeExamCaller()
: m_hWndNotify(NULL)
, m_uMsgNotifyItem(0)
, m_uMsgNotifyProgress(0)
, m_hEventStop(NULL)
, m_bExpressExam(FALSE)
, m_bExamCanceled(FALSE)
, m_piExaminer(NULL)
, m_hExamModule(NULL)
, m_nUnSafeMonitorCount(0)
{
	::InitializeCriticalSection(&m_lockExamResult);
	::InitializeCriticalSection(&m_lockExamResult2);
	LoadExamModule();
}

CBkSafeExamCaller::~CBkSafeExamCaller()
{
	::DeleteCriticalSection(&m_lockExamResult);
	::DeleteCriticalSection(&m_lockExamResult2);
	UnLoadExamModule();
}

//体检入口
void CBkSafeExamCaller::Start(BOOL bExpress, HWND hWndNotify, UINT uMsgNotifyItem, UINT uMsgNotifyProgress)
{
	if (m_hEventStop)
		return;

	m_bExpressExam = bExpress;
	m_bExamCanceled = FALSE;

	m_hEventStop = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hWndNotify            = hWndNotify;
	m_uMsgNotifyItem        = uMsgNotifyItem;
	m_uMsgNotifyProgress    = uMsgNotifyProgress;

	::EnterCriticalSection(&m_lockExamResult);
	m_mapExamResult.RemoveAll();
	::LeaveCriticalSection(&m_lockExamResult);

	SetNotifyHwndToTrash();
	HANDLE hExamThread = ::CreateThread(NULL, 0, _SafeExaminationThreadProc, this, 0, NULL);

	::CloseHandle(hExamThread);
	hExamThread = NULL;
}

//修复
void CBkSafeExamCaller::FixItem(int nItemID, void* pData /*= NULL*/)
{
	::EnterCriticalSection(&m_lockExamResult);
	const CRBMap<int, BKSAFE_EXAM_RESULT_ITEM>::CPair *pPair = m_mapExamResult.Lookup(nItemID);
	::LeaveCriticalSection(&m_lockExamResult);

	REMOVE_EXAM_RESULT_ITEM RemoveItemInfo;
	RemoveItemInfo.uRemoveItem.RemoveItemInfo.Reset();
	RemoveItemInfo.uRemoveItem.RemoveItemInfo.dwItemID = nItemID;
	if (NULL != pPair)
	{
		if (BkSafeExamItem::HandShake == nItemID)
		{//握手
            CWHRoundRectDialog<CBkSimpleDialog> dlg;
            dlg.Load(IDR_BK_GETVER_DLG);
			dlg.DoModal();			
		}else if (BkSafeExamItem::EXAM_GPEDIT_RUN == nItemID ||
			BkSafeExamItem::EXAM_GUEST_RUN == nItemID ||
			BkSafeExamItem::EXAM_HIDE_FOLDER_RUN == nItemID ||
			BkSafeExamItem::EXAM_REMOTE_DESKTOP == nItemID ||
			BkSafeExamItem::EXAM_DRIVERORDIRECTORYOPEN == nItemID)
		{//guest，远程桌面。。。
			if (NULL == m_piExaminer)
				return;
			m_piExaminer->FixItem(nItemID, pData);
			::SendMessage(m_hWndNotify, m_uMsgNotifyItem, (WPARAM)BkSafeExamItemExOperate::RemoveItem, (LPARAM)&RemoveItemInfo);
		}
		else if (BkSafeExamItem::EXAM_SHARE_RESOUCE == nItemID)
		{//共享资源
			if (NULL == m_piExaminer)
				return;
			m_piExaminer->FixItem(nItemID, pData);
			int nCount = -1;
			m_piExaminer->GetItemCount(nItemID, nCount);
			if (0 == nCount)
			{
				::SendMessage(m_hWndNotify, m_uMsgNotifyItem, (WPARAM)BkSafeExamItemExOperate::RemoveItem, (LPARAM)&RemoveItemInfo);
			}
		}
		else if (nItemID == BkSafeExamItem::ConfigSystemMonitor &&
					   1 == GetUnSafeMonitorCount())
		{//监控
			SetAllMonitorRun(TRUE);
			::SendMessage(m_hWndNotify, m_uMsgNotifyItem, (WPARAM)BkSafeExamItemExOperate::RemoveItem, (LPARAM)&RemoveItemInfo);
		}
		else if (BkSafeExamItem::EXAM_HOSTS_FILE == nItemID)
		{//host文件
			if (NULL == m_piExaminer)
				return;
			if (S_OK == m_piExaminer->FixItem(nItemID, pData))
				::SendMessage(m_hWndNotify, m_uMsgNotifyItem, (WPARAM)BkSafeExamItemExOperate::RemoveItem, (LPARAM)&RemoveItemInfo);
		}
		else
		{//调用其他模块来修复
            CString strNavigate = pPair->m_value.strItemNavigateString;
            if (nItemID == 34)  // 垃圾清理
            {
                strNavigate += L"scan";
            }

			if (0 == strNavigate.CompareNoCase(L"ui_protection_common"))
			{
				strNavigate = L"ui_IEFix_IEFixSm";
			}

			_Module.Navigate(strNavigate);
		}
		
	}
}


/*
void CBkSafeExamCaller::SetIgnoreItems(CSimpleArray<int> arrItems)
{
	m_arrIgnoreItems.RemoveAll();
	int nCount = arrItems.GetSize();
	{
		for (int i = 0; i < nCount; i++)
		{
			m_arrIgnoreItems.Add(arrItems[i]);
		}
	}
}
*/

/*
void CBkSafeExamCaller::ResetIgnoreArray()
{
	m_arrIgnoreItems.RemoveAll();
}
*/
//握手接口
void CBkSafeExamCaller::Echo(HWND hWndNotify, UINT uMsgNotify)
{
	_ECHO_THREAD_PARAM *pParam = new _ECHO_THREAD_PARAM;

	pParam->hWndNotify  = hWndNotify;
	pParam->uMsgNotify  = uMsgNotify;
	pParam->pThis       = this;

	HANDLE hExamThread = ::CreateThread(NULL, 0, _EchoThreadProc, pParam, 0, NULL);

	::CloseHandle(hExamThread);

	hExamThread = NULL;
}

int CBkSafeExamCaller::GetItemCount(int nItemID, int& nCount)
{
	int nRet = -1;
	if (NULL == m_piExaminer)
		return nRet;

	if (S_OK == m_piExaminer->GetItemCount(nItemID, nCount))
		nRet = 0;

	return nRet;
}
int CBkSafeExamCaller::EnumItemData(int nItemID, int nIndex, void** pData)
{
	int nRet = -1;

	if (NULL == m_piExaminer)
		return nRet;

	if (S_OK == m_piExaminer->EnumItemData(nItemID, nIndex, pData))
		nRet = 0;

	return nRet;
}

int CBkSafeExamCaller::SetItemSafe(int nItemID, void** pData)
{
	int nRet = -1;

	if (S_OK == m_piExaminer->SetItemSafe(nItemID, pData))
		nRet = 0;

	return nRet;
}

void CBkSafeExamCaller::SetHostItemFix(int nIndex)
{
	if (NULL == m_piExaminer)
		return;

	m_piExaminer->SetHostItemFix(nIndex);
}

int CBkSafeExamCaller::RemoveLocalLibItem(int nItemID, void* pValue)
{
	int nRet = -1;
	if (NULL == m_piExaminer)
		return nRet;

	if (S_OK == m_piExaminer->RemoveLocalLibItem(nItemID, pValue))
		nRet = 0;

	return nRet;
}


BOOL CBkSafeExamCaller::GetResultItem(int nOrder, BKSAFE_EXAM_RESULT_ITEM &item)
{
	#if 0

	BOOL bRet = FALSE;

	::EnterCriticalSection(&m_lockExamResult);

	if ((int)m_mapExamResult.GetCount() <= nOrder)
		goto Exit0;

	POSITION pos = m_mapExamResult.GetHeadPosition();
	const CRBMap<int, BKSAFE_EXAM_RESULT_ITEM>::CPair *pPair = NULL;

	while (pos && -1 != nOrder)
	{
		pPair = m_mapExamResult.GetNext(pos);

		nOrder --;
	}

	if (NULL == pPair)
		goto Exit0;

	item = pPair->m_value;

	bRet = TRUE;

Exit0:

	::LeaveCriticalSection(&m_lockExamResult);

	return bRet;
	#endif
	return TRUE;
}

//其他模块在做完清理之后，调用这个接口来告诉体检来刷新界面
BOOL CBkSafeExamCaller::RemoveItem(int nItemID)
{
	if (m_hEventStop)
		return FALSE;

	if (BkSafeExamItem::ConfigSystemMonitor == nItemID)
	{
		CSafeMonitorTrayShell safeMonitor;
		if ((0 != IsSafeMonitorAllRun()) /*|| (0 != _IsSafeMonitorAllRun())*/ )
			return FALSE;
	}

	BOOL bRet = FALSE;
	int nLevel = 0;
	BOOL bHasRisk = FALSE, bHasCriticalRisk = FALSE;

	::EnterCriticalSection(&m_lockExamResult);

#if 0
	bRet = m_mapExamResult.RemoveKey(nItemID);
#endif

	if (::IsWindow(m_hWndNotify))
	{
		REMOVE_EXAM_RESULT_ITEM RemoveItemInfo;
		RemoveItemInfo.uRemoveItem.RemoveItemInfo.Reset();
		RemoveItemInfo.uRemoveItem.RemoveItemInfo.dwItemID = nItemID;
		::SendMessage(m_hWndNotify, m_uMsgNotifyItem, (WPARAM)BkSafeExamItemExOperate::RemoveItem, (LPARAM)&RemoveItemInfo);
	}

	::LeaveCriticalSection(&m_lockExamResult);

#if 0
	//屏蔽到原因是：因为关于IE的扫描使用的是网盾的引擎，
	//所以有可能一个项目扫描出多个来，所以如果这样的话这
	//段代码就不能一项选择修复多次，这段代码的原始功能是
	//为了todolist使用的，现在todolist已经不用了，所以这
	//段代码可以屏蔽掉
	if (bRet)
	{
		POSITION pos = m_mapExamResult.GetHeadPosition();
		const CRBMap<int, BKSAFE_EXAM_RESULT_ITEM>::CPair *pPair = NULL;

		while (pos)
		{
			pPair = m_mapExamResult.GetNext(pos);

			nLevel = pPair->m_value.nLevel;

			if (BkSafeExamItemLevel::Safe != nLevel) 
			{
				bHasRisk          |= (BkSafeExamItemLevel::Info != nLevel && BkSafeExamItemLevel::RiskInfo != nLevel);
				bHasCriticalRisk  |= (BkSafeExamItemLevel::Critical == nLevel);
			}
		}
	}

	m_bHasRisk          = bHasRisk;
	m_bHasCriticalRisk  = bHasCriticalRisk;

	::LeaveCriticalSection(&m_lockExamResult);

	return bRet;
#endif
	

	return TRUE;
}
//其他模块在做完清理之后，调用这个接口来告诉体检来刷新界面
BOOL CBkSafeExamCaller::RemoveItem2(REMOVE_EXAM_RESULT_ITEM RemoveExamItem)
{
	if (m_hEventStop)
		return FALSE;

	if (BkSafeExamItem::ConfigSystemMonitor == RemoveExamItem.uRemoveItem.RemoveItemInfo.dwItemID)
	{
		CSafeMonitorTrayShell safeMonitor;
		if ((0 != IsSafeMonitorAllRun()) || (0 != _IsSafeMonitorAllRun()))
			return FALSE;
	}

	::EnterCriticalSection(&m_lockExamResult2);

	if (::IsWindow(m_hWndNotify))
	{
		::SendMessage(m_hWndNotify, m_uMsgNotifyItem, (WPARAM)BkSafeExamItemExOperate::RemoveItem, (LPARAM)(&RemoveExamItem));
	}

	::LeaveCriticalSection(&m_lockExamResult2);

	return TRUE;
}


#if 0
int CBkSafeExamCaller::GetItemLevel(int nItemID)
{
	if (m_hEventStop)
		return BkSafeExamItemLevel::Unknown;

	BOOL bRet = FALSE;

	::EnterCriticalSection(&m_lockExamResult);

	const CRBMap<int, BKSAFE_EXAM_RESULT_ITEM>::CPair *pPair = m_mapExamResult.Lookup(nItemID);

	::LeaveCriticalSection(&m_lockExamResult);

	if (NULL == pPair)
		return BkSafeExamItemLevel::Safe;

	return pPair->m_value.nLevel;
}
#endif

HRESULT WINAPI CBkSafeExamCaller::OnItemExamFinish(
										int     nItem, 
										int     nLevel, 
										LPCWSTR lpszItemNoticeText, 
										LPCWSTR lpszOperationText, 
										LPCWSTR lpszItemNavigateString, 
										LPCWSTR lpszItemDetailUrl, 
										LPCWSTR lpszDetail)
{
	BKSAFE_EXAM_RESULT_ITEM item = { nLevel, lpszItemNoticeText, lpszOperationText, lpszItemNavigateString };

	if (BkSafeExamItemLevel::Unknown != nLevel && BkSafeExamItemLevel::Safe != nLevel)
	{
		::EnterCriticalSection(&m_lockExamResult);

		m_mapExamResult.SetAt(nItem, item);

		::LeaveCriticalSection(&m_lockExamResult);
	}

// 	if (BkSafeExamItemLevel::Safe != nLevel) 
// 	{
// 		m_bHasRisk          |= (BkSafeExamItemLevel::Info != nLevel && BkSafeExamItemLevel::RiskInfo != nLevel);
// 		m_bHasCriticalRisk  |= (BkSafeExamItemLevel::Critical == nLevel);
// 	}

	if (nItem == BkSafeExamItem::ConfigSystemMonitor)
	{
		CSafeMonitorTrayShell safeMonitor;
		m_nUnSafeMonitorCount = safeMonitor.GetSafeMonitorCount() - safeMonitor.GetSafeMonitorRunCount();
/*
		if (TRUE == IsWin64())
		{
			if (FALSE == safeMonitor.GetMonitorRun(SM_ID_PROCESS))
				--m_nUnSafeMonitorCount;
			if (m_nUnSafeMonitorCount < 0)
				m_nUnSafeMonitorCount = 0;
		}*/

		
	}
	item.SafeExamItemEx  = *((BKSafeExamItemEx::BKSafeExamEx*)(lpszDetail));

	if (::IsWindow(m_hWndNotify))
		::SendMessage(m_hWndNotify, m_uMsgNotifyItem, (WPARAM)nItem, (LPARAM)&item);

	return S_OK;
}
//体检线程
DWORD WINAPI CBkSafeExamCaller::_SafeExaminationThreadProc(LPVOID pvParam)
{
	CBkSafeExamCaller *pThis = (CBkSafeExamCaller *)pvParam;
//	IBkSafeExaminer *piExaminer = NULL;

	if (NULL == pThis)
		return 0;

	HWND hWndNotify         = pThis->m_hWndNotify;
	UINT uMsgNotifyItem     = pThis->m_uMsgNotifyItem;
	UINT uMsgNotifyProgress = pThis->m_uMsgNotifyProgress;
	BOOL bExpress = pThis->m_bExpressExam;
	pThis->m_bExpressExam = NULL;

	if (pThis->m_piExaminer)
	{
		pThis->m_piExaminer->Uninitialize();
		pThis->m_piExaminer->Release();
		pThis->m_piExaminer = NULL;
	}

	if (NULL == pThis->m_hExamModule)
	{
		if (0 != pThis->LoadExamModule())
			return 0;
	}

	PFN_BkSafeCreateObject BkSafeCreateObject = (PFN_BkSafeCreateObject)::GetProcAddress(pThis->m_hExamModule, FN_BkSafeCreateObject);
	if (NULL == BkSafeCreateObject)
		goto Exit0;

	HRESULT hRet = BkSafeCreateObject(BK_SAFEEXAM_DLL_VERSION, __uuidof(IBkSafeExaminer), (LPVOID *)&pThis->m_piExaminer);
	if (S_OK != hRet)
	{
		if (::IsWindow(pThis->m_hWndNotify))
			::SendMessage(pThis->m_hWndNotify, pThis->m_uMsgNotifyItem, (WPARAM)BkSafeExamItemExOperate::ErrorMsg, 1);
		goto Exit0;
	}

	if (NULL == pThis->m_piExaminer)
		goto Exit0;

	hRet = pThis->m_piExaminer->Initialize(NULL);
	if (FAILED(hRet))
		goto Exit0;


	if (bExpress)
	{//这里暂时已经不使用了，所以bExpress永远都是FALSE
		pThis->m_piExaminer->ExpressExamine(pThis, pThis->m_hEventStop);
	}
	else
	{
		pThis->m_piExaminer->Examine(pThis, pThis->m_hEventStop);
	}

	DEBUG_TRACE(L"Exam Finish\r\n");

	if (::IsWindow(hWndNotify))
		::SendMessage(hWndNotify, uMsgNotifyItem, BkSafeExamItem::Invalid, NULL);

Exit0:
	if (pThis->m_hEventStop)
	{
		::CloseHandle(pThis->m_hEventStop);
		pThis->m_hEventStop = NULL;
	}
	pThis->m_bExpressExam = FALSE;

	return 0;
}

int CBkSafeExamCaller::LoadExamModule()
{
	int nRet = -1;
	m_hExamModule = NULL;
	m_hExamModule = ::LoadLibrary(L"ksafeexam.dll");
	if(NULL != m_hExamModule)
		nRet = 0;

	return nRet;
}

int CBkSafeExamCaller::SetNotifyHwndToTrash()
{
	DWORD dwRet = 0;
	WCHAR szPath[MAX_PATH] = {0};
	SetNotifyHwnd_t NotifyHwndToTrash= NULL;
	HMODULE hModule = NULL;

	GetModuleFileName(NULL, szPath, MAX_PATH);
	PathRemoveFileSpec(szPath);
	PathAppend(szPath, L"kclear.dll");

	hModule = LoadLibrary(szPath);
	if (hModule == NULL)
	{
		dwRet = 1;
		goto clean0;
	}
	NotifyHwndToTrash = (SetNotifyHwnd_t)GetProcAddress(hModule, "SetTrashNotifyHwnd");

	if (NotifyHwndToTrash == NULL)
	{
		dwRet = 1;
		goto clean0;
	}
	NotifyHwndToTrash(m_hWndNotify);

clean0:
	if (hModule)
	{
		FreeLibrary(hModule);
		hModule = NULL;
	}
	return dwRet;
}

void CBkSafeExamCaller::UnLoadExamModule()
{
	if(NULL != m_piExaminer)
		m_piExaminer->Uninitialize();
	if (m_hExamModule)
	{
		::FreeLibrary(m_hExamModule);
		m_hExamModule = NULL;
	}

}
int CBkSafeExamCaller::GetUnSafeMonitorCount()
{
	return m_nUnSafeMonitorCount;
}

BOOL CBkSafeExamCaller::IsWin64()
{
	BOOL bRet = FALSE;
	static HMODULE hKernel32 = NULL;
	if( !hKernel32 )
		hKernel32 = ::LoadLibrary(TEXT("Kernel32.DLL"));
	if( !hKernel32 )
		return FALSE;

	typedef BOOL (WINAPI *FunctionIsWow64Process)(HANDLE hProcess, PBOOL Wow64Process);
	FunctionIsWow64Process pfnIsWow64Process = NULL;
	pfnIsWow64Process = (FunctionIsWow64Process)GetProcAddress(hKernel32, "IsWow64Process");

	if (NULL == pfnIsWow64Process)
		return FALSE;

	HANDLE hCurrentProcess = GetCurrentProcess();
	pfnIsWow64Process(hCurrentProcess, &bRet);
	return bRet;
}

void CBkSafeExamCaller::SetAllMonitorRun(BOOL bRun/*= TRUE*/)
{
	//如果是64位系统体检发现时进程监控或者网页监控没有开启的话，就认为没有风险,暂时64为系统下不支持这两个监控

	CSafeMonitorTrayShell safeMonitor;
	/*if (TRUE == IsWin64())
	{
		for (int i = 0; i < ARRAYSIZE(g_dwMonitorIDList); i ++)
		{
			if (SM_ID_PROCESS == g_dwMonitorIDList[i])
				continue;

			safeMonitor.SetMonitorRun(g_dwMonitorIDList[i], bRun);
		}
	}
	else*/
	{
		safeMonitor.SetAllMonitorRun(bRun);
	}

	/*safeMonitor.SetAutorunTray(bRun);*/
}

void CBkSafeExamCaller::_SetAllMonitorRun(BOOL bRun/*= TRUE*/)
{
    //如果是64位系统体检发现时进程监控或者网页监控没有开启的话，就认为没有风险,暂时64为系统下不支持这两个监控

    CSafeMonitorTrayShell safeMonitor;
  /*  if (TRUE == IsWin64())
    {
        for (int i = 0; i < ARRAYSIZE(g_dwNetMonitorIDList); i ++)
        {
            if (SM_ID_PROCESS == g_dwNetMonitorIDList[i])
                continue;

            safeMonitor.SetMonitorRun(g_dwNetMonitorIDList[i], bRun);
        }
    }
    else*/
    {
        safeMonitor._SetAllMonitorRun(bRun);
    }

    /*safeMonitor.SetAutorunTray(bRun);*/
}

int CBkSafeExamCaller::IsSafeMonitorAllRun()
{
	CSafeMonitorTrayShell safeMonitor;
	int bAllRun = 0;

/*
	if (TRUE == IsWin64())
	{
		for (int i = 0; i < ARRAYSIZE(g_dwMonitorIDList); i ++)
		{
			if (
				SM_ID_PROCESS == g_dwMonitorIDList[i])
				continue;

			if (FALSE == safeMonitor.GetMonitorRun(g_dwMonitorIDList[i]))
			{
				bAllRun = -1;
				break;
			}
		}
	}
	else*/

	{
		if (FALSE == safeMonitor.IsAllMonitorRun())
			bAllRun = -1;
	}

	return bAllRun;
}

int CBkSafeExamCaller::_IsSafeMonitorAllRun()
{
    CSafeMonitorTrayShell safeMonitor;
    int bAllRun = 0;
/*
    if (TRUE == IsWin64())
    {
        for (int i = 0; i < ARRAYSIZE(g_dwNetMonitorIDList); i ++)
        {
            if (
                SM_ID_PROCESS == g_dwNetMonitorIDList[i])
                continue;

            if (FALSE == safeMonitor.GetMonitorRun(g_dwNetMonitorIDList[i]))
            {
                bAllRun = -1;
                break;
            }
        }
    }
    else*/

    {
        if (FALSE == safeMonitor._IsAllMonitorRun())
            bAllRun = -1;
    }

    return bAllRun;
}

int CBkSafeExamCaller::GetSafeLibItemCount(int& nCount)
{
	int nRet = -1;
	if (NULL == m_piExaminer)
		return nRet;

	if (S_OK == m_piExaminer->GetSafeLibItemCount(nCount))
		nRet = 0;

	return nRet;
}
int CBkSafeExamCaller::EnumSafeLibItem(int nIndex, BkSafeExamLib::LocalLibExport& libItem)
{
	int nRet = -1;

	if (NULL == m_piExaminer)
		return nRet;

	if (S_OK == m_piExaminer->EnumSafeLibItem(nIndex, libItem))
		nRet = 0;
	
	return nRet;
}

int CBkSafeExamCaller::LocalLibIsHaveItem(int nItemID, BOOL& bHave)
{
	int nRet = -1;
	if (NULL == m_piExaminer)
		return nRet;

	if (S_OK == m_piExaminer->LocalLibIsHaveItem(nItemID, bHave))
		nRet = 0;

	return nRet;
}

int CBkSafeExamCaller::StartupAnUse(int nItemID)
{
	int nRet = -1;
	if (NULL == m_piExaminer)
		return nRet;

	if (S_OK == m_piExaminer->StartupAnUse(nItemID))
		nRet = 0;

	return nRet;

}

int CBkSafeExamCaller::ConvertKWSIDToExamID(int nIndex)
{
	return ExamPubFunc::CExamPubFunc::GetPubFuncPtr()->ConvertKWSIDToExamID(nIndex);
}

int CBkSafeExamCaller::ExamItem(int nItemID, BKSafeExamItemEx::BKSafeExamEx& ExamItemData)
{
	int nRet = -1;
	if (NULL == m_piExaminer)
		return nRet;

	if (S_OK == m_piExaminer->ExamItem(nItemID, ExamItemData))
		nRet = 0;

	return nRet;
}