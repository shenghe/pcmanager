///////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <time.h>
#include "kuires.h"
#include "kcheckinstalled.h"
#include "kscmaindlg.h"
#include "stubbornfiles.h"
#include "misc/kppuserlog.h"
#include "misc/kregfunction.h"
#include "misc/inieditor.h"
#include "kcheckinstalled.h"
#include "findfiletraverse.h"
#include "deletefiletraverse.h"
#include "trashcleanerfeedback.h"
#include "trashpublicfunc.h"
#include "dlgsetting.h"
#include "safeexam/beikesafeexam.h"
#include "beikesafe/bkmsgdefine.h"
#include "beikesafe/bksafeexamcall.h"
#include "uihandler_trash.h"
#include "examimp.h"
#include "filtertool.h"
#include "kscbase/kcslogging.h"


//////////////////////////////////////////////////////////////////////////

CFindFileTraverse find;

UINT g_nCloseID;
//////////////////////////////////////////////////////////////////////////
CUIHandlerTrash::CUIHandlerTrash(CKscMainDlg* refDialog) : m_dlg(refDialog)
{
	m_bFinishClean = FALSE;
	m_bCleaning = FALSE;
	m_bScaning = FALSE;
	m_nFlashCount = 0;
	m_uConfigTotalSize = 0;
	m_nTotalSize = 0;
	m_nDelTotalSize = 0;
	m_lastTime = 0;
	m_ufirstDay = 0;
	m_bFirstUse = FALSE;
    m_bFinishInitilize = FALSE;
    m_bExamNeedScan = FALSE;
	CStubbornFiles::Instance().Init();
	TCHAR szValueBuffer[MAX_PATH] = { 0 };
	DWORD len = MAX_PATH;
	DWORD dwType = REG_DWORD;

	GetRegistryValue(HKEY_CURRENT_USER,
		L"SOFTWARE\\KSafe\\KClear",
		L"DbgLog",
		&dwType,
		(LPBYTE)szValueBuffer,
		&len
		);

	if(wcslen(szValueBuffer)>0)
	{
		KppUserLog::UserLogInstance().m_bWrite = TRUE;
	}

	wchar_t szAppPath[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, szAppPath, MAX_PATH);
	PathRemoveFileSpecW(szAppPath);
	::PathAppend(szAppPath,L"cfg\\lastrecord.inf");
	m_strCacheFile = szAppPath;
}

CUIHandlerTrash::~CUIHandlerTrash()
{

}
//////////////////////////////////////////////////////////////////////////
void CUIHandlerTrash::Init()
{
	m_trashCtrl.Create(
		m_dlg->GetRichWnd(), 
		NULL, 
		NULL, 
		WS_CHILD|WS_VISIBLE, 
		0, 
		IDC_CTR_TRASH_MAIN);

    m_trashCtrl.Initialize(m_dlg->m_hWnd);

    m_skipCtrl.Create(
        m_dlg->GetRichWnd(),
        NULL,
        NULL,
        WS_CHILD|WS_VISIBLE,
        0,
        IDC_CTR_TRASH_SKIP_MAIN);

    m_skipCtrl.Initialize(m_dlg->m_hWnd);
	
    _UpdateUI(TRASH_BEGIN);

    m_dlg->EnableItem(IDC_BTN_TRASH_BEGIN_SCAN, FALSE);
    
    HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, InitilizeThread, this, 0, NULL);

    if (handle != NULL)
    {
        CloseHandle(handle);
        handle = NULL;
    }
}
void CUIHandlerTrash::UnInit()
{
	KClearSettingDlg clearSetting;
	std::vector<UINT> selectedItems;

	m_trashCtrl.GetSelectedItems(selectedItems);

	clearSetting.SetCustomSelected(selectedItems);

	clearSetting.SetClearTotalSize(m_uConfigTotalSize);

	clearSetting.SetClearLastDay(m_lastTime);

	clearSetting.SetLastSize(m_nDelTotalSize);

	clearSetting.SetFirstDay(m_ufirstDay);

	clearSetting.SaveSetting();
}

BOOL CUIHandlerTrash::IsFinishInitilize()
{
    return m_bFinishInitilize;
}
void CUIHandlerTrash::SetFinishExamNeedScan(BOOL bNeedScan /* = TRUE */)
{
    m_bExamNeedScan = bNeedScan;
}
//////////////////////////////////////////////////////////////////////////
void CUIHandlerTrash::TraverseFile(LPFINDFILEDATA pFileData)
{	

	::SendMessage(m_dlg->m_hWnd, WM_TRAVERSEFILE, NULL, (LPARAM)pFileData);
}

void CUIHandlerTrash::TraverseProcess(unsigned long uCurPos)
{
	
// 	if (m_nProIndex == uCurPos)
// 		return TRUE;

// 	m_nProIndex = m_nCurrentFindIndex;
// 	m_nCurrentFindIndex = uCurPos;
	// 设置某项“正在扫描。。。”字样  “正在清理”？？？？
	::SendMessage(m_dlg->m_hWnd, WM_TRAVERSEPROCESS, uCurPos, NULL);

}

void CUIHandlerTrash::TraverseProcessEnd(unsigned long uCurPos, CString vedioPath)
{
// 	if (m_nProIndex == uCurPos)
// 	{
// 	}
// 	else
// 	{
// 		m_nProIndex = m_nCurrentFindIndex;
// 		m_nCurrentFindIndex = uCurPos;
// 	}

    ::SendMessage(m_dlg->m_hWnd, WM_TRAVERSEPROCESSEND, uCurPos,(LPARAM)vedioPath.GetBuffer());
}

void CUIHandlerTrash::TraverseFinished()
{
	::SendMessage(m_dlg->m_hWnd, WM_TRAVERSEFINISHED, NULL, NULL);
}

LRESULT CUIHandlerTrash::OnTraverseFile(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    KAutoLock lock(m_lock);
	LPFINDFILEDATA pFileData = (LPFINDFILEDATA)lParam;
	m_nTotalSize += m_FileListData.AddTrashFile(m_nCurrentFindIndex, pFileData);

	if (m_nCurrentFindIndex == WIN_HUISHOUZHAN)
	{
		m_trashCtrl.SetItemStatusInfo(WIN_HUISHOUZHAN, 0, pFileData->nCount, pFileData->uFileSize);
		m_nTotalCount += pFileData->nCount;
	}
	else
	{
		m_nTotalCount++;
	}

	return TRUE;
}
LRESULT CUIHandlerTrash::OnTraverseProcess(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	unsigned long uCurPos = (unsigned long)wParam;
	CString strName;
	KAppRes& res = KAppRes::Instance();
    CString strFor = res.GetString("IDS_TRASH_SCANING_ITEM");
    CString strOutput;

	if (m_nProIndex == uCurPos)
		return TRUE;

    m_nProIndex = m_nCurrentFindIndex;
    m_nCurrentFindIndex = uCurPos;

	m_trashCtrl.GetItemName(uCurPos, strName);
	
    strName = " : " + strName;
	strOutput.Format(strFor, strName);
	
    m_dlg->SetItemText(IDC_TXT_TRASH_DESCRPTION, strOutput);
	
    m_nItemCounter ++;
	
    m_trashCtrl.SetItemShowDetail(uCurPos);
	
    m_bFindFileRunningSkip = FALSE;
    
    m_trashCtrl.SetSelectedItem(uCurPos);
	m_trashCtrl.SetStatus(uCurPos, Scaning);
	
    return TRUE;
}
LRESULT CUIHandlerTrash::OnTraverseProcessEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_nProgressPos = (int)((m_nItemCounter*1.0) / m_nTotalItem * 100);

	unsigned long uCurPos = (unsigned long)wParam;

	CString strPath = (LPCTSTR)lParam;

	if (m_nProIndex == uCurPos)
	{
	}
	else
	{
		m_nProIndex = m_nCurrentFindIndex;
		m_nCurrentFindIndex = uCurPos;
	}


    {
        KAutoLock lock(m_lock);
	    ULONGLONG uSize = m_FileListData.m_itemArray.at(uCurPos).ulAllFileSize;
	    UINT nCount = (UINT)m_FileListData.m_itemArray.at(uCurPos).itemArray.size();
    
	    if (uCurPos != WIN_HUISHOUZHAN)
	    {
		    m_trashCtrl.SetItemStatusInfo(m_nCurrentFindIndex, 0, nCount, uSize);
	    }
    }



	if (m_bFindFileRunningSkip)
	{
		m_trashCtrl.SetStatus(uCurPos, SkipRunning);
		::SendMessage(m_dlg->m_hWnd, WM_TRASH_TIPS_SHOW_STATUS, (WPARAM)TRUE, NULL);
	}
	else
	{	
		m_trashCtrl.SetStatus(uCurPos, FinishScan);
	}
	m_trashCtrl.Invalidate();
	return TRUE;
}
LRESULT CUIHandlerTrash::OnTraverseFinished(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_bScaning)
	{
        KAutoLock lock(m_lock);
		if (m_FileListData.m_nTotalsize == 0 && m_nTotalCount == 0)
		{
			_UpdateUI(TRASH_SCANING_NONE);
		}
		else
		{
			_UpdateUI(TRASH_SCANING_FINISH);

		}
	}
	else
	{
		_UpdateUI(TRASH_SCANING_CANCEL);
	}
	m_bScaning = FALSE;
	KillTimer(m_dlg->m_hWnd, DEF_TRASH_FLASH_EVENT);
	KillTimer(m_dlg->m_hWnd, DEF_TRASH_PROGRESS_EVENT);
    KillTimer(m_dlg->m_hWnd, DEF_TRASH_LIST_UPDATE_EVENT);

// 	m_dlg->EnableItem(IDC_CHK_TRASH_CHECK_ALL, TRUE);
// 	m_dlg->EnableItem(IDC_LNK_TRASH_RECOMMEND, TRUE);
	_SetNeedEnableItems(TRUE);
	return TRUE;
}

LRESULT CUIHandlerTrash::OnTrashClearBegin(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_bCleaning = TRUE;
	m_bScaning = FALSE;
	m_nFlashProgress = 0;

    m_trashCtrl.SetStatus(-1, WaitClear);

	_UpdateUI(TRASH_CLEANING);

	SetTimer(m_dlg->m_hWnd, DEF_TRASH_FLASH_EVENT, DEF_TRASH_FLASH_TIME, NULL);
	SetTimer(m_dlg->m_hWnd, DEF_TRASH_PROGRESS_EVENT, DEF_TRASH_PRO_TIME, NULL);
    SetTimer(m_dlg->m_hWnd, DEF_TRASH_LIST_UPDATE_EVENT, DEF_TRASH_PRO_TIME, NULL);

    _SetNeedEnableItems(FALSE);
    return TRUE;
}
LRESULT CUIHandlerTrash::OnTrashClearFinish(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
	m_uConfigTotalSize += m_nDelTotalSize;
	time(&m_lastTime);

	if (m_ufirstDay == 0)
	{
		time(&m_ufirstDay);
	}


	if (m_bCleaning)
	{
		_UpdateUI(TRASH_CLEAN_FINISH);
	}
	else
	{
		_UpdateUI(TRASH_CLEAN_CANCEL);
	}	



	m_bCleaning = FALSE;
	KillTimer(m_dlg->m_hWnd, DEF_TRASH_FLASH_EVENT);
	KillTimer(m_dlg->m_hWnd, DEF_TRASH_PROGRESS_EVENT);
    KillTimer(m_dlg->m_hWnd, DEF_TRASH_LIST_UPDATE_EVENT);

// 	m_dlg->EnableItem(IDC_CHK_TRASH_CHECK_ALL, TRUE);
// 	m_dlg->EnableItem(IDC_LNK_TRASH_RECOMMEND, TRUE);
    _SetNeedEnableItems(TRUE);

	return TRUE;
}
LRESULT CUIHandlerTrash::OnTrashClearProgress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	unsigned long uCurPos = (unsigned long)wParam;
	CString strName;
	KAppRes& res = KAppRes::Instance();
	m_trashCtrl.GetItemName(uCurPos, strName);
    strName = " : " + strName;
	CString strFor = res.GetString("IDS_TRASH_CLEANING_ITEM");
	CString strOutput;
	strOutput.Format(strFor, strName);
	m_dlg->SetItemText(IDC_TXT_TRASH_DESCRPTION, strOutput);

    m_trashCtrl.SetSelectedItem(uCurPos);
    m_trashCtrl.SetStatus(uCurPos, Clearing);

	return TRUE;
}

LRESULT CUIHandlerTrash::OnTrashClearItemEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    unsigned long uCurPos = (unsigned long)wParam;
    BOOL bSkip = (BOOL)lParam;

    if (bSkip)
    {
        m_trashCtrl.SetStatus(uCurPos, SkipRunning);
    }
    else
    {
        m_trashCtrl.SetStatus(uCurPos, FinishClear);
    }

    return TRUE;
}

LRESULT CUIHandlerTrash::OnShowTips(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL bShow = (BOOL)wParam;

	m_dlg->SetItemVisible(IDC_DLG_TRASH_TIPS, bShow);

	return TRUE;
}
LRESULT CUIHandlerTrash::OnCheckedAllChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_trashCtrl.CheckItemAllSelected())
	{
		m_dlg->SetItemCheck(IDC_CHK_TRASH_CHECK_ALL, TRUE);
	}
	else
	{
		m_dlg->SetItemCheck(IDC_CHK_TRASH_CHECK_ALL, FALSE);
	}
    {
        KClearSettingDlg setting;
        std::vector<UINT> vSelectedItems;
        m_trashCtrl.GetSelectedItems(vSelectedItems);

        setting.SetCustomSelected(vSelectedItems);

        setting.SaveSetting();
    }
	return 0;
}
LRESULT CUIHandlerTrash::OnCheckItemDetail(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    unsigned int uCurIndex = (unsigned int) wParam;
    std::vector<TRASHFILEITEM> itemArray = 
        m_FileListData.m_itemArray[uCurIndex].itemArray;

    if (uCurIndex == WIN_HUISHOUZHAN)
    {
        ShellExecute(NULL, L"open", 
            L"explorer", 
            L"::{645FF040-5081-101B-9F08-00AA002F954E}", 
            NULL, SW_SHOWNORMAL);
    }
    else if (itemArray.size() > 0)
    {
        size_t iCount;
        CString strName = KAppRes::Instance().GetString("IDS_TRASH_DETAIL");
        CString strItemName;
        CString strDetail;
        CString strDes;

        m_trashCtrl.GetItemName(uCurIndex, strItemName);

        strDes += strItemName;
        UINT nItemID = m_trashCtrl.GetItemParent(uCurIndex);
        if (nItemID != -1)
        {
            m_trashCtrl.GetItemName(nItemID, strItemName);
            strDes = strItemName + L" - " + strDes;
        }

        for (iCount = 0; iCount < itemArray.size(); ++iCount)
        {
            CString strSize;
            GetFileSizeTextString(itemArray[iCount].ulFileSize , strSize);
            strDetail += itemArray[iCount].strFileName;
//            strDetail += L"(" + strSize + L")";
            strDetail += L"\r\n";
        }
        
        CKscViewLog viewLog(strName, strDes, strDetail);
        viewLog.DoModal();

    }

    return 0;
}
LRESULT CUIHandlerTrash::OnOneKeyClean(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DWORD low = (DWORD)lParam;
    ULONGLONG height = (ULONGLONG)wParam;
    ULONGLONG delTotalSize = (height << 32) | low;

	m_uConfigTotalSize += delTotalSize;

	time(&m_lastTime);
	if (m_ufirstDay == 0)
	{
		time(&m_ufirstDay);
	}
	m_nDelTotalSize = delTotalSize;
	//更新 界面数据

	_SetCompleteSize();
	_SetLastTimeAndSize();
	return 0;
}
LRESULT CUIHandlerTrash::OnInitilize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // 读取配置 文件~~设置初始界面目标	
    _InitilizePageInfo();

    m_trashCtrl.Invalidate();

    m_dlg->EnableItem(IDC_BTN_TRASH_BEGIN_SCAN, TRUE);

    if (m_bExamNeedScan)
    {
        m_bExamNeedScan = FALSE;
        OnRescan();
    }

    m_bFinishInitilize = TRUE;

    return 0;
}

LRESULT CUIHandlerTrash::OnTrashRescan(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_skipCtrl.StopCountdown();

    _UpdateUI(TRASH_BEGIN);

    OnRescan();

    return TRUE;
}

LRESULT CUIHandlerTrash::OnCloseItemTask(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT nItemID = (UINT)wParam;
    LPCTSTR lpProcess = (LPCTSTR)lParam;
    
    g_nCloseID = nItemID;

    PROCESSENTRY32 pe;
    DWORD id = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    pe.dwSize = sizeof(PROCESSENTRY32);
    
    if( !Process32First(hSnapshot,&pe) )
        return 0;
    
    while(1)
    {
        pe.dwSize = sizeof(PROCESSENTRY32);
        if( Process32Next(hSnapshot,&pe)==FALSE )
            break;
        if(wcsicmp(pe.szExeFile, lpProcess) == 0)
        {
            {
                id = pe.th32ProcessID;
                ::OutputDebugString(pe.szExeFile);
                EnumWindows((WNDENUMPROC)EnumWndProc, (LPARAM)id);
            }
        }

    }
    CloseHandle(hSnapshot);

    //remove the item by nItemID
    m_skipCtrl.RemoveItemByProcess(lpProcess);
    m_dlg->SetItemVisible(IDC_BTN_TRASH_SKIP_RESCAN, TRUE);
    if (m_skipCtrl.GetSize() == 0)
    {
        m_skipCtrl.StartCountdown();
    }
    else
    {
        m_skipCtrl.Invalidate();
    }

    return TRUE;
}

BOOL CALLBACK EnumWndProc(HWND hwnd, LPARAM lParam)
{
    BOOL bRet = TRUE;

    DWORD dwID ; 

    GetWindowThreadProcessId(hwnd, &dwID) ; 

    if (dwID == (DWORD)lParam && IsWindowVisible(hwnd) && GetParent(hwnd) == NULL) 
    { 
        ::PostMessage(hwnd, WM_CLOSE, NULL, NULL);
        bRet = FALSE;
    } 
    else if (dwID == (DWORD)lParam && g_nCloseID != BROWER_FIREFOX)
    {
        ::PostMessage(hwnd, WM_CLOSE, NULL,NULL);
    }

    return bRet;
}
//////////////////////////////////////////////////////////////////////////
DWORD CUIHandlerTrash::ClearFileThread(LPVOID lpVoid)
{
	CUIHandlerTrash* pThis = (CUIHandlerTrash*)lpVoid;
    ULONGLONG uBeginTime, uEndTime;

	::SendMessage(pThis->m_dlg->m_hWnd, WM_TRASH_BEGIN_CLEAR, NULL, NULL);
	
    uBeginTime = GetTickCount();

    pThis->RunClearFile();

	::CloseHandle(pThis->m_hThread);
	InterlockedExchange((LONG*)&pThis->m_hThread, NULL);
	InterlockedExchange((LONG*)&pThis->m_dwThreadID, 0);
	
    uEndTime = GetTickCount();

    pThis->m_uCleanTotalTime = (uEndTime - uBeginTime);

	::SendMessage(pThis->m_dlg->m_hWnd, WM_TRASH_FINISH_CLEAR, NULL, NULL);

	return 0;
}

void CUIHandlerTrash::RunClearFile()
{
	int nPos = 0;

	ULONGLONG uClearFileSize = 0;

	CFileDelete deleteFile;


	int nPosEx = 1;
	int nRemainingCount = m_nDelTotalCount;
	ULONGLONG nRemainingSize = m_nDelTotalSize;
	CString strRemaining;

    m_skipCtrl.Clear();
    m_nCurClearTotalNum = 0;

	for (int j = 0;j < (int)m_configData.m_ConfigData.size();j++)
	{
		if (m_configData.m_ConfigData[j].bIsLeaf)
		{
			if (m_configData.m_ConfigData[j].bDefaultSelect)
			{
				std::vector<TRASHFILEITEM> itemArray = 
					m_FileListData.m_itemArray[m_configData.m_ConfigData[j].id].itemArray;

                m_nCurrentFindIndex = m_configData.m_ConfigData[j].id;

				ULONGLONG ulCurSize = m_FileListData.m_itemArray[m_configData.m_ConfigData[j].id].ulAllFileSize;
				::SendMessage(m_dlg->m_hWnd, WM_TRASH_CLEAN_PRO, m_configData.m_ConfigData[j].id, NULL);
                
                BOOL bRunning = IsItemProcessRunning(m_configData.m_ConfigData[j].id);
                m_nCurItemClearNum = 0;

				for (int i = 0; i < (int)itemArray.size(); i++)
				{
//                    ::Sleep(20); // test
                    if (!m_bCleaning)
                        goto clean0;

                    if (bRunning)
                    {
                        _AddItemToSkipCtrl(m_nCurrentFindIndex, ulCurSize);

                        nRemainingCount -= (int)itemArray.size();
                        m_nProgressPos = (int)(100 - ((double)nRemainingCount / m_nDelTotalCount) * 100); 
                        break;
                    }
					if (itemArray[i].bIsClear)
					{
						int delRes = 0;
						if (m_configData.m_ConfigData[j].id == WIN_HUISHOUZHAN)
						{
							SHEmptyRecycleBin(NULL, NULL, SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND);
							delRes = Delete_File_Success;
						}
						else
						{
							delRes = deleteFile.TrashFileDelete(itemArray[i].strFileName);
						}
						if (delRes==Delete_File_Failed)
						{
							CStubbornFiles::Instance().AddFile(itemArray[i].strFileName);
                            std::wstring strLog = itemArray[i].strFileName;

                            KCLEAR_LOG(LOG_LEVEL_TRASHCLEAN) << "Clean Failed: " << UnicodeToAnsi(strLog);
						}
						else 
						{
                            std::wstring strLog = itemArray[i].strFileName;

                            if (m_configData.m_ConfigData[j].id == WIN_HUISHOUZHAN)
                            {
                                strLog = L"WIN_HUISHOUZHAN";
                            }

                            KCLEAR_LOG(LOG_LEVEL_TRASHCLEAN) << "Clean Succeed: " << UnicodeToAnsi(strLog);
						}
                        {
                            KAutoLock lock(m_lock);
					        uClearFileSize += itemArray[i].ulFileSize;
					        m_FileListData.m_itemArray[m_configData.m_ConfigData[j].id].ulAllFileSize -= itemArray[i].ulFileSize;
					        m_FileListData.m_itemArray[m_configData.m_ConfigData[j].id].itemArray[i].ulFileSize = 0;
					        m_FileListData.m_itemArray[m_configData.m_ConfigData[j].id].itemArray[i].bIsClear = false;
                            ++m_nCurItemClearNum;
                            --nRemainingCount;
                            if (m_configData.m_ConfigData[j].id == WIN_HUISHOUZHAN)
                            {
                                ULONGLONG uSize;
                                int nCount;
                                m_trashCtrl.GetDetailItemData(WIN_HUISHOUZHAN, uSize, nCount);
                                m_nCurClearTotalNum += nCount;
                            }
                            else
                            {
                                ++m_nCurClearTotalNum;
                            }                            
                            m_nProgressPos = (int)(100 - ((double)nRemainingCount / m_nDelTotalCount) * 100); 
                        }
					}
				}

                ::SendMessage(m_dlg->m_hWnd, WM_TRASH_CLEAN_END, m_configData.m_ConfigData[j].id, bRunning);
			}

		}

	}
clean0:
	m_nDelTotalSize = uClearFileSize;
 
	CStubbornFiles::Instance().Sync();

	REMOVE_EXAM_RESULT_ITEM RemoveItemInfo;
	RemoveItemInfo.uRemoveItem.RemoveItemInfo.Reset();
	RemoveItemInfo.uRemoveItem.RemoveItemInfo.dwItemID = BkSafeExamItem::EXAM_TRASH;

	::SendMessage(GetTrashNotifyHwnd(), MSG_APP_EXAM_SCORE_CHANGE, BkSafeExamItemExOperate::RemoveItem, (LPARAM)&RemoveItemInfo );

	return;
}
DWORD CUIHandlerTrash::WaitThread(LPVOID lpVoid)
{
	CUIHandlerTrash* pthis = (CUIHandlerTrash*)lpVoid;
	if(pthis->is_havespecsuffix)
	{
		if(CKuiMsgBox::Show(TEXT("网络播放器缓存可以让您在不联网状态下播放已缓冲完毕的视频\n\n或音乐，确认清理这些缓存吗？"), L"提示",MB_YESNO | MB_ICONINFORMATION , NULL, pthis->m_dlg->m_hWnd)== IDYES)
			pthis->is_ok = true;
		else pthis->is_ok = false;
	}
	else
	{
		if(CKuiMsgBox::Show(TEXT("您选择了清理看视频和听音乐产生的缓存，这些缓存可以让您在不联网\n\n状态下播放已缓冲完毕的视频或音乐，确认清理这些缓存吗？"), L"提示",MB_YESNO | MB_ICONINFORMATION , NULL, pthis->m_dlg->m_hWnd)== IDYES)
			pthis->is_ok = true;
		else pthis->is_ok = false;

	}

	if(pthis->is_ok)
	{
		pthis->m_hThread = CreateThread(NULL, 
			0, 
			ClearFileThread, 
			(LPVOID)pthis,
			NULL,
			&(pthis->m_dwThreadID));
	}

	::CloseHandle(pthis->m_tmpThread);
	InterlockedExchange((LONG*)&pthis->m_tmpThread, NULL);
	InterlockedExchange((LONG*)&pthis->m_dwThreadID, 0);

	return 0;
}

UINT CUIHandlerTrash::InitilizeThread(LPVOID lpVoid)
{
    CUIHandlerTrash* pThis = (CUIHandlerTrash*)lpVoid;
    ULONGLONG uBeginTime, uEndTime;

    uBeginTime = GetTickCount();

    BOOL nRetCode = pThis->m_configData.ReadConfig();

    if (nRetCode != RC_SUCCESS ||
        !pThis->m_configData.InitFiltDir()|| 
        !pThis->m_configData.ConvToLocatePath())
    {
        goto Clear0;
    }
    else
    {	    
        pThis->_InitListConfig();
        {
            KAutoLock lock(pThis->m_lock);
            pThis->m_FileListData.m_itemArray.resize(ENUM_ID_END);
        }
        uEndTime = GetTickCount();

        CString strTime;
        strTime.Format(L"load time  %I64d\n",(uEndTime - uBeginTime));
        ::OutputDebugString(strTime);

        ::PostMessage(pThis->m_dlg->m_hWnd, WM_FINISH_INITILIZE, NULL, NULL);
    }
Clear0:
    return 0;
}
//////////////////////////////////////////////////////////////////////////
void CUIHandlerTrash::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == DEF_TRASH_FLASH_EVENT)
	{
		_OnFlashImgEvent();
	}

	if (nIDEvent == DEF_TRASH_PROGRESS_EVENT)
	{
		_OnProgressEvent();
	}

    if (nIDEvent == DEF_TRASH_LIST_UPDATE_EVENT)
    {
        _OnUpdateListEvent();
    }

	SetMsgHandled(FALSE);
}
LRESULT CUIHandlerTrash::OnOwnMsgBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	BOOL is_ok;
	if(CKuiMsgBox::Show((LPCTSTR)wParam, L"提示", (UINT)lParam, NULL, m_dlg->m_hWnd)==IDOK)
		is_ok = true;
	else 
		is_ok = false;
	bHandled = TRUE;
	return TRUE;
}

LRESULT CUIHandlerTrash::OnSetClearSetting(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	KClearSettingDlg config;
	std::vector<std::wstring> extItems;
	BOOL bShow = config.IsExtScanEnable();
    BOOL bHasShow = m_trashCtrl.GetItemShow(SYS_TRASH);
	
	if (bShow && !bHasShow)
	{
        m_trashCtrl.SetItemChecked(SYS_TRASH, TRUE);   
    }
    if (bShow)
    {
        config.GetExtList(extItems);
        _SetDeepSearchConfig(extItems);
    }
    m_trashCtrl.SetItemShow(SYS_TRASH, bShow);
    m_trashCtrl.Invalidate();

    CFilterTool::Instance().SetTimeFilter(config.IsFilterVedioTime());

	return 0;
}
//////////////////////////////////////////////////////////////////////////
//

void CUIHandlerTrash::OnCtrlReSize(CRect rcWnd)
{
	if (IsWindow(m_trashCtrl))
		m_trashCtrl.SetWindowPos(NULL, rcWnd, 0);
}
//////////////////////////////////////////////////////////////////////////
void CUIHandlerTrash::OnClickPushApp1()
{
	// 转向 软件卸载页面
   KAppRes& appRes = KAppRes::Instance();
   std::wstring strNav, strName;

   appRes.GetString("IDS_PUSH_APP1_NAV", strNav);
   appRes.GetString("IDS_PUSH_APP1_NAME", strName);

   WCHAR szPath[MAX_PATH] = { 0 };
   WCHAR szCmdline[MAX_PATH * 2] = { 0 };
   ::GetModuleFileName(NULL, szPath, MAX_PATH);

   CString strExepath = szPath;
   if (L'\\' == strExepath.GetAt(strExepath.GetLength() - 1))
   {
       strExepath.Delete(strExepath.GetLength() -1);
   }

   wsprintf(szCmdline, L"\"%s\" -do:%s", strExepath.GetBuffer(), strNav.c_str());
   WinExec(CW2A(szCmdline), SW_HIDE);
}

void CUIHandlerTrash::OnClickPushApp2()
{
	// 转向 插件清理页面
    KAppRes& appRes = KAppRes::Instance();
    std::wstring strNav, strName;

    appRes.GetString("IDS_PUSH_APP2_NAV", strNav);
    appRes.GetString("IDS_PUSH_APP2_NAME", strName);

    WCHAR szPath[MAX_PATH] = { 0 };
    WCHAR szCmdline[MAX_PATH * 2] = { 0 };
    ::GetModuleFileName(NULL, szPath, MAX_PATH);

    CString strExepath = szPath;
    if (L'\\' == strExepath.GetAt(strExepath.GetLength() - 1))
    {
        strExepath.Delete(strExepath.GetLength() -1);
    }

    wsprintf(szCmdline, L"\"%s\" -do:%s", strExepath.GetBuffer(), strNav.c_str());
    WinExec(CW2A(szCmdline), SW_HIDE);
}

void CUIHandlerTrash::OnClickPushApp3()
{
	// 转向 开机加速页面
    KAppRes& appRes = KAppRes::Instance();
    std::wstring strNav, strName;

    appRes.GetString("IDS_PUSH_APP3_NAV", strNav);
    appRes.GetString("IDS_PUSH_APP3_NAME", strName);

    WCHAR szPath[MAX_PATH] = { 0 };
    WCHAR szCmdline[MAX_PATH * 2] = { 0 };
    ::GetModuleFileName(NULL, szPath, MAX_PATH);

    CString strExepath = szPath;
    if (L'\\' == strExepath.GetAt(strExepath.GetLength() - 1))
    {
        strExepath.Delete(strExepath.GetLength() -1);
    }

    wsprintf(szCmdline, L"\"%s\" -do:%s", strExepath.GetBuffer(), strNav.c_str());
    WinExec(CW2A(szCmdline), SW_HIDE);
}

void CUIHandlerTrash::OnClickPushApp4()
{
    // 转向 开机加速页面
    KAppRes& appRes = KAppRes::Instance();
    std::wstring strNav, strName;

    appRes.GetString("IDS_PUSH_APP4_NAV", strNav);
    appRes.GetString("IDS_PUSH_APP4_NAME", strName);

    WCHAR szPath[MAX_PATH] = { 0 };
    WCHAR szCmdline[MAX_PATH * 2] = { 0 };
    ::GetModuleFileName(NULL, szPath, MAX_PATH);

    CString strExepath = szPath;
    if (L'\\' == strExepath.GetAt(strExepath.GetLength() - 1))
    {
        strExepath.Delete(strExepath.GetLength() -1);
    }

    wsprintf(szCmdline, L"\"%s\" -do:%s", strExepath.GetBuffer(), strNav.c_str());
    WinExec(CW2A(szCmdline), SW_HIDE);
}
void CUIHandlerTrash::OnClickPushApp5()
{
    // 转向 开机加速页面
    KAppRes& appRes = KAppRes::Instance();
    std::wstring strNav, strName;

    appRes.GetString("IDS_PUSH_APP5_NAV", strNav);
    appRes.GetString("IDS_PUSH_APP5_NAME", strName);

    WCHAR szPath[MAX_PATH] = { 0 };
    WCHAR szCmdline[MAX_PATH * 2] = { 0 };
    ::GetModuleFileName(NULL, szPath, MAX_PATH);

    CString strExepath = szPath;
    if (L'\\' == strExepath.GetAt(strExepath.GetLength() - 1))
    {
        strExepath.Delete(strExepath.GetLength() -1);
    }

    wsprintf(szCmdline, L"\"%s\" -do:%s", strExepath.GetBuffer(), strNav.c_str());
    WinExec(CW2A(szCmdline), SW_HIDE);
}
void CUIHandlerTrash::OnClickPushApp6()
{
    // 转向 开机加速页面
    KAppRes& appRes = KAppRes::Instance();
    std::wstring strNav, strName;

    appRes.GetString("IDS_PUSH_APP6_NAV", strNav);
    appRes.GetString("IDS_PUSH_APP6_NAME", strName);

    WCHAR szPath[MAX_PATH] = { 0 };
    WCHAR szCmdline[MAX_PATH * 2] = { 0 };
    ::GetModuleFileName(NULL, szPath, MAX_PATH);

    CString strExepath = szPath;
    if (L'\\' == strExepath.GetAt(strExepath.GetLength() - 1))
    {
        strExepath.Delete(strExepath.GetLength() -1);
    }

    wsprintf(szCmdline, L"\"%s\" -do:%s", strExepath.GetBuffer(), strNav.c_str());
    WinExec(CW2A(szCmdline), SW_HIDE);
}

//////////////////////////////////////////////////////////////////////////

void CUIHandlerTrash::OnClickCancelWork()
{
	KillTimer(m_dlg->m_hWnd, DEF_TRASH_FLASH_EVENT);
	KillTimer(m_dlg->m_hWnd, DEF_TRASH_PROGRESS_EVENT);
    KillTimer(m_dlg->m_hWnd, DEF_TRASH_LIST_UPDATE_EVENT);

	if (m_bScaning)
	{
		m_bScaning = FALSE;
		find.StopFileTraverse();

	}
	else if (m_bCleaning)
	{
		m_bCleaning = FALSE;
		m_bFinishClean = FALSE;
	}
	else
	{
		// ERROR
		_SetShowBeginPageShow();
	}
	
//	m_trashCtrl.SetStatus(-1, FinishWork);

	m_dlg->EnableItem(IDC_CHK_TRASH_CHECK_ALL, TRUE);
	m_dlg->EnableItem(IDC_LNK_TRASH_RECOMMEND, TRUE);
}

void CUIHandlerTrash::OnClickBeginClean()
{

	SetMsgHandled(TRUE);
	BOOL bEnable = FALSE;
	BOOL bEnable1 = FALSE;
	BOOL bEnable2 = FALSE;
	BOOL bIsVideo = FALSE;
    ULONGLONG ulTotalJunkFileSize = 0;
	is_finish = false;
	is_ok = false;
	is_havespecsuffix = false;
	int ClearCount = 0;

	if(!_GetConfigSelected())
	{
		::PostMessage(m_dlg->m_hWnd, WM_OWN_MSGBOX, (WPARAM)TEXT("\n您尚未选择需要清理的项目，请先选择，然后再清理！"), 
			(LPARAM)(MB_OK | MB_ICONINFORMATION));
		return;
	}

	for(int j = 0;j < (int)m_configData.m_ConfigData.size();j++)
	{
		if( m_configData.m_ConfigData[j].bIsLeaf)
		{
			if(m_configData.m_ConfigData[j].bDefaultSelect)
			{
                KAutoLock lock(m_lock);
				bEnable = TRUE;
				if(m_mapclearflag[m_configData.m_ConfigData[j].id] == 0)
					bEnable1 = TRUE;
				if (m_mapclearflag[m_configData.m_ConfigData[j].id] == 1 && 
					m_FileListData.m_itemArray.at(m_configData.m_ConfigData[j].id).itemArray.size()>0)
				{
					bEnable2 = TRUE;

					ulTotalJunkFileSize += m_FileListData.m_itemArray.at(m_configData.m_ConfigData[j].id).ulAllFileSize;

                    if (m_configData.m_ConfigData[j].parent == VIDEO_TRASH && m_configData.m_ConfigData[j].id != VIDEO_SOGOU)
                        bIsVideo = TRUE;

                    if (m_configData.m_ConfigData[j].id == WIN_HUISHOUZHAN)
                    {
                        ULONGLONG uSize;
                        int nCount;
                        m_trashCtrl.GetDetailItemData(WIN_HUISHOUZHAN, uSize, nCount);
                        ClearCount += nCount;
                    }
					else
                    {
                        ClearCount += (int)m_FileListData.m_itemArray[m_configData.m_ConfigData[j].id].itemArray.size();
                    }
                }
			}
		}
	}

	if(bEnable1 && !bEnable2)
	{
		::PostMessage(m_dlg->m_hWnd, WM_OWN_MSGBOX, (WPARAM)TEXT("\n您选择的项目中存在未扫描的项目，建议您先扫描，再进行清理！"), 
			(LPARAM)(MB_OK | MB_ICONINFORMATION));
		return;
	}


	if(ClearCount == 0)//
	{
		::PostMessage(m_dlg->m_hWnd, WM_OWN_MSGBOX, (WPARAM)TEXT("\n在你指定的清理选项中未发现垃圾文件！"), 
			(LPARAM)(MB_OK | MB_ICONINFORMATION));
		return;
	}


	ULONGLONG dwCurTotalSize = 0;
	DWORD dwFileCount = 0;

	for (int j = 0;j < (int)m_configData.m_ConfigData.size();j++)
	{
		if (m_configData.m_ConfigData[j].bIsLeaf)
		{
			if (m_configData.m_ConfigData[j].bDefaultSelect)
			{
                KAutoLock lock(m_lock);
				dwFileCount += (int)m_FileListData.m_itemArray[m_configData.m_ConfigData[j].id].itemArray.size();
				dwCurTotalSize += m_FileListData.m_itemArray[m_configData.m_ConfigData[j].id].ulAllFileSize;
			}
		}
	}
	m_nDelTotalCount = dwFileCount;
	m_nDelTotalSize = dwCurTotalSize;

	if(is_havespecsuffix)
	{
		m_tmpThread = CreateThread(NULL, 
			0, 
			WaitThread, 
			(LPVOID)this,
			NULL,
			&m_dwThreadID);

		return;
	}


	if(bIsVideo)
	{ 
		m_tmpThread = CreateThread(NULL, 
			0, 
			WaitThread, 
			(LPVOID)this,
			NULL,
			&m_dwThreadID);

		return;
	}

	m_hThread = CreateThread(NULL, 
		0, 
		ClearFileThread, 
		(LPVOID)this,
		NULL,
		&m_dwThreadID);

	//////////////////////////////////////////////////////////////////////////
}

void CUIHandlerTrash::OnClickScanBegin()
{
	if (!_GetConfigSelected())
	{
		::PostMessage(m_dlg->m_hWnd, WM_OWN_MSGBOX, (WPARAM)TEXT("\n您尚未选择需要扫描的项目，请先选择，然后进行扫描！"), 
			(LPARAM)(MB_OK | MB_ICONINFORMATION));
		goto Clear0;
	}
	m_nTotalItem = 0;
	m_nItemCounter = 0;
	for (int i = 0; i < (int)m_configData.m_ConfigData.size(); i++)
	{
		if (m_configData.m_ConfigData[i].bIsLeaf)
		{
			if(m_configData.m_ConfigData[i].bDefaultSelect)
			{
				m_mapclearflag[m_configData.m_ConfigData[i].id] = 1;
				m_nTotalItem++;
			}
			else
			{ 
				m_mapclearflag[m_configData.m_ConfigData[i].id] = 0;
			}
		}
	}
	m_trashCtrl.ClearAllStatus();
	m_trashCtrl.SetStatus(-1, WaitScan);
	m_bScaning = TRUE;
	m_nTotalSize = 0;
	m_nTotalCount = 0;

	_StartFind();
	
	m_trashCtrl.Invalidate();
	_UpdateUI(TRASH_SCANING);

	m_nProgressPos = 1;
	m_nFlashProgress = 0;

	SetTimer(m_dlg->m_hWnd, DEF_TRASH_FLASH_EVENT, DEF_TRASH_FLASH_TIME, NULL);
	SetTimer(m_dlg->m_hWnd, DEF_TRASH_PROGRESS_EVENT, DEF_TRASH_PRO_TIME, NULL);
    SetTimer(m_dlg->m_hWnd, DEF_TRASH_LIST_UPDATE_EVENT, DEF_TRASH_PRO_TIME, NULL);

Clear0:
	return;
}
void CUIHandlerTrash::OnCheckAll()
{
	BOOL bCheckAll;

	bCheckAll = m_dlg->GetItemCheck(IDC_CHK_TRASH_CHECK_ALL);

	m_trashCtrl.SetCheckAll(bCheckAll);

	m_trashCtrl.Invalidate();
}

void CUIHandlerTrash::OnRescan()
{
    _UpdateUI(TRASH_BEGIN);

    m_skipCtrl.StopCountdown();

	OnClickScanBegin();
}
void CUIHandlerTrash::OnReturnBeginPage()
{
	m_bScaning = FALSE;
	m_bCleaning = FALSE;
    
    m_skipCtrl.StopCountdown();

	m_trashCtrl.SetStatus();

	_UpdateUI(TRASH_BEGIN);

    _SetLastTimeAndSize();
}

void CUIHandlerTrash::OnClickRecommend()
{
	int iCount;
	
	for (iCount = 0; iCount < (int)m_configData.m_ConfigData.size(); ++iCount)
	{
		m_trashCtrl.SetItemChecked(m_configData.m_ConfigData[iCount].id, m_configData.m_ConfigData[iCount].bRSelect);
	}
	m_trashCtrl.Invalidate();

	SendMessage(m_dlg->m_hWnd, WM_SELCETED_CHECKED_CHANGE, NULL, NULL);
}

void CUIHandlerTrash::OnTipsClose()
{
	::SendMessage(m_dlg->m_hWnd, WM_TRASH_TIPS_SHOW_STATUS, (WPARAM)FALSE, NULL);
}

void CUIHandlerTrash::OnSkipRescan()
{
    _UpdateUI(TRASH_BEGIN);
    
    OnRescan();
}
void CUIHandlerTrash::OnCheckTrashCleanDetail()
{
    CString strTitle = KAppRes::Instance().GetString("IDS_TRASH_DETAIL");
    CString strText = L"本次清理详情";
    CString strDetail;
    size_t iCount, jCount;

    for (iCount = 0; iCount < m_configData.m_ConfigData.size(); ++iCount)
    {
        if (m_configData.m_ConfigData[iCount].bDefaultSelect)
        {
            std::vector<TRASHFILEITEM> itemArray = 
                m_FileListData.m_itemArray[m_configData.m_ConfigData[iCount].id].itemArray;

            if (m_configData.m_ConfigData[iCount].id == WIN_HUISHOUZHAN)
            {
                ULONGLONG uSize;
                int nCount;
                CString strSize;
                m_trashCtrl.GetDetailItemData(WIN_HUISHOUZHAN, uSize, nCount);
                GetFileSizeTextString(uSize, strSize);
                if (nCount > 0)
                {
                    strDetail += L"Windows 系统回收站";
                    strDetail += L"（" + strSize + L"）";
                    strDetail += L"\r\n";
                }
                continue;
            }
            
            for (jCount = 0; jCount < itemArray.size(); ++jCount)
            {
                if (!itemArray[jCount].bIsClear)
                {
                    strDetail += itemArray[jCount].strFileName;
                    strDetail += L"\r\n";
                }
            }
        }
    }

    CKscViewLog viewLog(strTitle, strText, strDetail);
    viewLog.DoModal();
}
//////////////////////////////////////////////////////////////////////////
// protected
void CUIHandlerTrash::_InitilizePageInfo()
{
	KAppRes& res = KAppRes::Instance();
	KClearSettingDlg clearConfig;
	std::vector<UINT> selectedItems;
	CString firstTime;
	CString strTitle;
	CString strSince;
	CString strInfo;
	BOOL bFirst = FALSE;

	if (clearConfig.GetCustomSelected(selectedItems))
	{
		for (int i = 0; i < (int)selectedItems.size(); ++i)
		{
			m_trashCtrl.SetItemChecked(selectedItems[i], TRUE);
		}
        ::SendMessage(m_dlg->m_hWnd, WM_SELCETED_CHECKED_CHANGE, NULL, NULL);
	}
	else
	{
		if(::PathFileExists(m_strCacheFile.GetBuffer()))
		{
			m_bFirstUse = TRUE;
			IniEditor IniEdit;
			IniEdit.SetFile(m_strCacheFile.GetBuffer());

			int checkNum = 0;

			for (int j = 0;j < (int)m_configData.m_ConfigData.size();j++)
			{
				int nSelect = 0;
				BOOL bOk = IniEdit.ReadDWORD(L"kclear_checke_laji",m_configData.m_ConfigData[j].strFriendName.GetBuffer(),nSelect);

				if(!bOk)
					break;

				if (nSelect == 1)
				{
// 					m_config.m_ConfigData[j].bDefaultSelect = 1;
					m_trashCtrl.SetItemChecked(m_configData.m_ConfigData[j].id, TRUE);
					checkNum ++;
				}
				else 
				{
// 					m_config.m_ConfigData[j].bDefaultSelect = 0;
                    continue;
				}
			}

			if(checkNum == m_configData.GetConfigData().size() - 3)
			{
//				m_check_all.SetCheck(TRUE);
				m_dlg->SetItemCheck(IDC_CHK_TRASH_CHECK_ALL, TRUE);
			}
            else if (checkNum == 0)
            {
                OnClickRecommend();
            }
		}
		else
		{
			OnClickRecommend();
		}
	}



	if (clearConfig.GetClearLastDay(m_lastTime) &&
		clearConfig.GetLastSize(m_nDelTotalSize))
	{
		_SetLastTimeAndSize();
	}
	else
	{
		strTitle = res.GetString("IDS_TRASH_FIRST_VIEW");	
		m_dlg->SetItemText(IDC_TXT_TRASH_LAST_INFO, strTitle);
		bFirst = TRUE;
	}


	
	if (clearConfig.GetClearTotalSize(m_uConfigTotalSize) &&
		clearConfig.GetClearFirstDay(firstTime) && !bFirst)
	{
		swscanf(firstTime.GetBuffer(), L"%I64d", &m_ufirstDay);

		_SetCompleteSize();

	}
	else
	{
		m_dlg->SetItemText(IDC_TXT_TRASH_CLEAN_TOTAL, L"");
	}



	if (clearConfig.IsExtScanEnable())
	{
		std::vector<std::wstring> extItems;
		m_trashCtrl.SetItemShow(SYS_TRASH, TRUE);
		clearConfig.GetExtList(extItems);
		_SetDeepSearchConfig(extItems);
	}

    CFilterTool::Instance().SetTimeFilter(clearConfig.IsFilterVedioTime());
}

void CUIHandlerTrash::_SetDeepSearchConfig(std::vector<std::wstring>& vecConfig)
{
	int iCount;
	int nItem = (int)m_configData.m_ConfigData.size();
	nItem -= 1;
	m_configData.m_ConfigData[nItem].strExtArray.clear();

	for (iCount = 0; iCount < (int)vecConfig.size(); ++iCount)
	{
		CString str = vecConfig[iCount].c_str();
/*		str.Delete(0);*/
		m_configData.m_ConfigData[nItem].strExtArray.push_back(str);
	}

}
void CUIHandlerTrash::_SetCompleteSize()
{
	KAppRes& res = KAppRes::Instance();
	CString strSince = L"";
	CString strSize;
	CString strInfo;
	time_t t;
	
	time(&t);
	strInfo = res.GetString("IDS_TRASH_LAST_TIME");
	GetFileSizeTextString(m_uConfigTotalSize, strSize);
	if (m_ufirstDay == 0)
	{
		m_dlg->SetItemText(IDC_TXT_TRASH_CLEAN_TOTAL, strSince);
	}
	else if (1 <= (t - m_ufirstDay) / 60/ 60/ 24)
	{
		time_t temp(m_ufirstDay);	

		CHAR buff[100] = {0};

		strftime(buff,100, "%Y年%m月%d日", localtime(&temp));
		strSince.Format(strInfo, AnsiToUnicode(buff).c_str(), strSize);
		m_dlg->SetItemText(IDC_TXT_TRASH_CLEAN_TOTAL, strSince);
	}
	else
	{
		strSince.Format(res.GetString("IDS_TRASH_FIRSTDAY"), strSize);
		m_dlg->SetItemText(IDC_TXT_TRASH_CLEAN_TOTAL, strSince);
	}

}

void CUIHandlerTrash::_SetLastTimeAndSize()
{
	KAppRes& res = KAppRes::Instance();
	BOOL bFirst;
	CString strInfo;
	CString strTitle;

	strInfo = res.GetString("IDS_TRASH_LAST_INFO");
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);		
	if (sysTime.wYear > 2030 || sysTime.wYear < 2000)
	{
		CString strF = L"啊！您是怎么穿越到%d年的？";
		strTitle.Format(strF, sysTime.wYear);
		bFirst = TRUE;
	}
	else if (m_lastTime == 0)
	{
		strTitle = res.GetString("IDS_TRASH_FIRST_VIEW");
		bFirst = TRUE;
	}
	else
	{
		// 计算时间
		time_t t;
		time(&t);


		CTimeSpan spanTime(t);
		CTimeSpan lastTime(m_lastTime);
		CTimeSpan betTime = spanTime - lastTime;

		CString strDay;
		CString strSize;
		GetFileSizeTextString(m_nDelTotalSize, strSize);
		if (betTime.GetDays() > 0)
		{
			strDay.Format(L"%d天前", betTime.GetDays());
			strTitle.Format(strInfo, strDay, strSize);
		}
		else if (betTime.GetHours() > 0)
		{
			strDay.Format(L"%d小时前", betTime.GetHours());
			strTitle.Format(strInfo, strDay, strSize);
		}
		else if (betTime.GetMinutes() > 0)
		{
			strDay.Format(L"%d分钟前", betTime.GetMinutes());
			strTitle.Format(strInfo, strDay, strSize);
		}
		else
		{
			strDay = res.GetString("IDS_TRASH_JUST_FINISH");
			strTitle.Format(strDay, strSize);
		}
	}
	m_dlg->SetItemText(IDC_TXT_TRASH_LAST_INFO, strTitle);
//	return bFirst;
}
void CUIHandlerTrash::_SetNeedEnableItems(BOOL bEnbale)
{
	m_dlg->EnableItem(IDC_CHK_TRASH_CHECK_ALL, bEnbale);
	m_dlg->EnableItem(IDC_LNK_TRASH_RECOMMEND, bEnbale);
	m_dlg->EnableItem(IDC_LNK_SETTING, bEnbale);
}
void CUIHandlerTrash::_StartFind()
{
	m_nCurrentFindIndex = -1;
	m_nProIndex = -1;

    {
        KAutoLock lock(m_lock);
	
	    // 清空各目录的文件列表
	    for (int i = 0; i < (int)m_FileListData.m_itemArray.size(); i++)
	    {
		    m_FileListData.m_itemArray.at(i).ulAllFileSize = 0;
		    m_FileListData.m_itemArray.at(i).itemArray.clear();
		    m_FileListData.m_itemArray.at(i).strfileList = L"";
	    }
	    m_FileListData.m_nTotalsize = 0;
    }

	if (!find.StartFindFileTraverse(m_configData, this, FALSE))
	{
		_SetNeedEnableItems(TRUE);
	}
	else
	{
		_SetNeedEnableItems(FALSE);
	}
}

BOOL CUIHandlerTrash::_GetConfigSelected()
{
	BOOL bRet = FALSE;
	int iCount, jCount;
	std::vector<UINT> selectItems;

	m_trashCtrl.GetSelectedItems(selectItems);
	
	if (selectItems.size() == 0)
		goto Clear0;

	for (iCount = 0; iCount < (int)m_configData.m_ConfigData.size(); ++iCount)
	{
		m_configData.m_ConfigData[iCount].bDefaultSelect = FALSE;
	}

	for (iCount = 0; iCount < (int)selectItems.size(); ++iCount)
	{
		for (jCount = 0; jCount < (int)m_configData.m_ConfigData.size(); ++jCount)
		{
			if (selectItems[iCount] == m_configData.m_ConfigData[jCount].id)
			{
				m_configData.m_ConfigData[jCount].bDefaultSelect = TRUE;
				break;
			}
		}
	}
	bRet = TRUE;

Clear0:
	return bRet;
}

void CUIHandlerTrash::_OnFlashImgEvent()
{
	if (m_bScaning)
	{
		m_dlg->SetItemIntAttribute(IDC_IMG_TRASH_FLASHING_SCAN, DEF_KUI_ATT_SUB, m_nFlashCount);
		++m_nFlashCount;
		if (m_nFlashCount >= DEF_TRASH_FLASH_NUM)
			m_nFlashCount = 0;
	}
	if (m_bCleaning)
	{
		m_dlg->SetItemIntAttribute(IDC_IMG_TRASH_FLASHING_CLEAN, DEF_KUI_ATT_SUB, m_nFlashCount);
		++m_nFlashCount;
		if (m_nFlashCount >= 4)
			m_nFlashCount = 0;
	}
}

void CUIHandlerTrash::_OnProgressEvent()
{
    if (m_bScaning)
    {
	    m_nFlashProgress ++;
	    if (m_nFlashProgress >= DEF_TRASH_PROGRESS_COUNT)
		    m_nFlashProgress = 0;
	    m_dlg->SetItemIntAttribute(IDC_PRO_TRASH, DEF_KUI_ATT_SUB, m_nFlashProgress);
    }
    else if (m_bCleaning)
    {
	    m_dlg->SetItemIntAttribute(IDC_PRO_TRASH_PERCENT, DEF_KUI_ATT_VALUE, m_nProgressPos);
    }
}

void CUIHandlerTrash::_OnUpdateListEvent()
{
    KAutoLock lock(m_lock);
    ULONGLONG uSize;
    UINT nCount;

    if (m_nCurrentFindIndex < 0 || m_nCurrentFindIndex >= (int)m_FileListData.m_itemArray.size())
        goto Clear0;

    uSize = m_FileListData.m_itemArray.at(m_nCurrentFindIndex).ulAllFileSize;
    nCount = (UINT)m_FileListData.m_itemArray.at(m_nCurrentFindIndex).itemArray.size();

    if (m_bCleaning)
    {
        nCount = nCount - m_nCurItemClearNum;
        if (nCount < 0)
            nCount = 0;
    }

    if (m_nCurrentFindIndex != WIN_HUISHOUZHAN)
    {
        m_trashCtrl.SetItemStatusInfo(m_nCurrentFindIndex, 0, nCount, uSize);
        m_trashCtrl.Invalidate();
    }

Clear0:
    return;
}
BOOL CUIHandlerTrash::_AddItemToSkipCtrl(UINT nCurrentIndex, ULONGLONG uSize)
{
    BOOL bRet = FALSE;
    CString strName;
    CString strProcess;
    std::vector<CString> vecProcess;
    std::vector<CString> vecRunningProcess;
    std::vector<CString>::iterator ite;

    GetProcessNamesByID(nCurrentIndex, vecProcess);

    if (vecProcess.size() == 0)
        goto Clear0;

    _GetRunningProcessName(vecProcess, vecRunningProcess);

    for (ite = vecRunningProcess.begin(); ite != vecRunningProcess.end(); ++ite)
    {
        LPTRASH_SKIP_ITEM lpItem = NULL;
        strProcess = (CString)(*ite);
        _GetItemProcessName(nCurrentIndex, strProcess, strName);

        if (m_skipCtrl.GetItemByProcessName(strProcess, lpItem))
        {
            if (lpItem == NULL)
                continue;
            if (nCurrentIndex == BROWER_SOGO || nCurrentIndex == VIDEO_SOGOU)
            {
                lpItem->uAddSize += uSize;
            }
            else if (nCurrentIndex == BROWER_MATHRON)
            {
                lpItem->uAddSize += uSize;
            }
            continue;
        }

        m_skipCtrl.AddItem(nCurrentIndex, strName, strProcess, uSize);
    }

    bRet = TRUE;
Clear0:
    return bRet;
}

BOOL CUIHandlerTrash::_GetItemProcessName(UINT nIndex, CString strProcess, CString& strName)
{
    BOOL bRet = FALSE;

    if (nIndex == BROWER_IE)
    {
        if (!strProcess.CompareNoCase(_T("iexplore.exe")))
        {
            strName = _T("IE 浏览器");
        }
        else if (!strProcess.CompareNoCase(_T("360se.exe")))
        {
            strName = _T("360安全 浏览器");
        }
        else if (!strProcess.CompareNoCase(_T("TTraveler.exe")))
        {
            strName = _T("腾讯TT 浏览器");
        }
        else if (!strProcess.CompareNoCase(_T("TheWorld.exe")))
        {
            strName = _T("世界之窗 浏览器");
        }
        else if (!strProcess.CompareNoCase(_T("maxthon.exe")))
        {
            //m_trashCtrl.GetItemName(BROWER_MATHRON, strName);
            strName = _T("遨游 浏览器");
        }
        else if (!strProcess.CompareNoCase(_T("sogouexplorer.exe")))
        {
            m_trashCtrl.GetItemName(BROWER_SOGO, strName);
        }
        else
        {
            goto Clear0;
        }
    }
    else if (nIndex == VIDEO_SOGOU)
    {
        m_trashCtrl.GetItemName(BROWER_SOGO, strName);
    }
    else
    {
        m_trashCtrl.GetItemName(nIndex, strName);
    }
    
    bRet = TRUE;
Clear0:
    return bRet;
}

BOOL CUIHandlerTrash::_GetRunningProcessName(std::vector<CString>& vecProcess, std::vector<CString>& vecRunning)
{
    BOOL bRet = FALSE;
    std::vector<CString>::iterator ite;
    PROCESSENTRY32 pe;
    DWORD id = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

    if (vecProcess.size() == 0)
        goto Clear0;


    pe.dwSize = sizeof(PROCESSENTRY32);
    if( !Process32First(hSnapshot,&pe) )
        goto Clear0;

    while(1)
    {
        pe.dwSize = sizeof(PROCESSENTRY32);
        if( Process32Next(hSnapshot,&pe)==FALSE )
            break;
        for (ite = vecProcess.begin(); ite != vecProcess.end(); ++ite)
        {
            CString strTemp = (*ite);
            if (strTemp.CompareNoCase(pe.szExeFile) == 0)
            {
                vecRunning.push_back(strTemp);
                vecProcess.erase(ite);
                break;
            }
        }
        if (vecProcess.size() == 0)
            break;
        
    }

Clear0:
    if (hSnapshot != NULL)
    {
        CloseHandle(hSnapshot);
        hSnapshot = NULL;
    }
    return bRet;
}
//////////////////////////////////////////////////////////////////////////
void CUIHandlerTrash::_UpdateUI(Trash_status nStatus)
{
	switch (nStatus)
	{
	case TRASH_BEGIN:
		{
			_SetShowBeginPageShow();
		}
		break;
	case TRASH_SCANING:
		{
			_SetDoingScanPageShow();
			m_dlg->SetItemVisible(IDC_DLG_TRASH_TIPS, FALSE);
            m_dlg->SetItemVisible(IDC_PRO_TRASH_PERCENT, FALSE);
            m_dlg->SetItemVisible(IDC_PRO_TRASH, TRUE);
		}
		break;
	case TRASH_SCANING_CANCEL:
		{			
			_SetScanFinishPageShow();
			m_dlg->SetItemVisible(IDC_DLG_TRASH_SCAN_CANECL, TRUE);
			m_dlg->SetItemVisible(IDC_DLG_TRASH_SCAN_FINISH, FALSE);
			m_dlg->SetItemVisible(IDC_DLG_TRASH_SCANING_NONE, FALSE);
			m_dlg->EnableItem(IDC_BTN_TRASH_BEGIN_CLEAN, TRUE);
			KAppRes& res = KAppRes::Instance();
			CString strFormat = res.GetString("IDS_TRASH_SCAN_CANCEL_SIZE");
			CString strSizeInfo;
			CString strSize;
            KAutoLock lock(m_lock);
			GetFileSizeTextString(m_FileListData.m_nTotalsize, strSize);
			strSizeInfo.Format(strFormat, strSize);
			m_dlg->SetRichText(IDC_TXT_TRASH_SCAN_SIZE, strSizeInfo);
		}
		break;
	case TRASH_SCANING_FINISH:
		{			
			_SetScanFinishPageShow();
			m_dlg->SetItemVisible(IDC_DLG_TRASH_SCAN_FINISH, TRUE);
			m_dlg->SetItemVisible(IDC_DLG_TRASH_SCAN_CANECL, FALSE);
			m_dlg->SetItemVisible(IDC_DLG_TRASH_SCANING_NONE, FALSE);
			m_dlg->EnableItem(IDC_BTN_TRASH_BEGIN_CLEAN, TRUE);
            KAppRes& res = KAppRes::Instance();
            CString strFormat = res.GetString("IDS_TRASH_SCAN_SIZE");
            CString strSizeInfo;
            CString strSize;
            KAutoLock lock(m_lock);
            GetFileSizeTextString(m_FileListData.m_nTotalsize, strSize);
            strSizeInfo.Format(strFormat, strSize);
            m_dlg->SetRichText(IDC_TXT_TRASH_SCAN_SIZE_RESULT, strSizeInfo);
            strFormat = res.GetString("IDS_TRASH_SCAN_COUNT");
            CString strCountInfo;
            CString strCount;
            GetFileCountText(m_nTotalCount, strCount);
            strCountInfo.Format(strFormat, strCount);
            m_dlg->SetRichText(IDC_TXT_TRASH_SCAN_FILE_NUM, strCountInfo);

		}
		break;
	case TRASH_SCANING_NONE:
		{			
			_SetScanFinishPageShow();
			m_dlg->SetItemVisible(IDC_DLG_TRASH_SCANING_NONE, TRUE);
			m_dlg->SetItemVisible(IDC_DLG_TRASH_SCAN_CANECL, FALSE);
			m_dlg->SetItemVisible(IDC_DLG_TRASH_SCAN_FINISH, FALSE);
//			m_dlg->EnableItem(IDC_BTN_TRASH_BEGIN_CLEAN, FALSE);
            m_dlg->SetItemVisible(IDC_BTN_TRASH_RESCAN, TRUE);
            m_dlg->SetItemVisible(IDC_BTN_TRASH_BEGIN_CLEAN, FALSE);
            m_dlg->SetItemVisible(IDC_LNK_TRASH_RESCAN, FALSE);
		}
		break;
	case TRASH_CLEANING:
		{
			_SetDoingCleanPageShow();
			m_dlg->SetItemVisible(IDC_DLG_TRASH_TIPS, FALSE);
            m_dlg->SetItemVisible(IDC_PRO_TRASH_PERCENT, TRUE);
            m_dlg->SetItemVisible(IDC_PRO_TRASH, FALSE);
		}
		break;
	case TRASH_CLEAN_CANCEL:
		{
			_SetCleanFinishPageShow();
			m_dlg->SetItemStringAttribute(IDC_IMG_TRASH_FINISH_STU, 
				DEF_KUI_ATT_SKIN, 
				DEF_TRASH_SKIN_FINISH_CANCEL_R);
			m_dlg->SetItemVisible(IDC_IMG_TRASH_CLEAN_CANCEL, TRUE);
            m_dlg->SetItemVisible(IDC_IMG_TRASH_RISK, FALSE);
			m_dlg->SetItemVisible(IDC_IMG_TRASH_FINISH_HEAD, FALSE);
// 			m_dlg->SetItemStringAttribute(IDC_IMG_TRASH_FINISH_HEAD,
// 				DEF_KUI_ATT_SKIN,
// 				DEF_TRASH_SKIN_FINISH_HEAD_F);
			KAppRes& res = KAppRes::Instance();
			CString strFormat = res.GetString("IDS_TRASH_CLEAN_CANCEL");
			CString strSizeInfo;
			CString strSize;
			GetFileSizeTextString(m_nDelTotalSize, strSize);
			strSizeInfo.Format(strFormat, strSize);
			m_dlg->SetRichText(IDC_TXT_TRASH_CLEAN_DES, strSizeInfo);
            if (m_skipCtrl.GetSize() == 0)
            {
                m_dlg->SetItemText(IDC_TXT_TRASH_FINISH_THIS, strSize);

                GetFileSizeTextString(m_uConfigTotalSize, strSize);
                m_dlg->SetItemText(IDC_TXT_TRASH_FINISH_TOTAL, strSize);
            }
            else
            {
                m_dlg->SetItemVisible(IDC_DLG_TRASH_SKIP, TRUE);
                m_dlg->SetItemVisible(IDC_DLG_TRASH_FINISH, FALSE);
            }
			_SetCompleteSize();
		}
		break;
	case TRASH_CLEAN_FINISH:
		{		
			m_dlg->SetItemVisible(IDC_IMG_TRASH_CLEAN_CANCEL, FALSE);
            m_dlg->SetItemVisible(IDC_IMG_TRASH_RISK, FALSE);

			_SetCleanFinishPageShow();
			m_dlg->SetItemStringAttribute(IDC_IMG_TRASH_FINISH_STU, 
				DEF_KUI_ATT_SKIN, 
				DEF_TRASH_SKIN_FINISH_SUCESS_R);
// 			m_dlg->SetItemStringAttribute(IDC_IMG_TRASH_FINISH_HEAD,
// 				DEF_KUI_ATT_SKIN,
// 				DEF_TRASH_SKIN_FINISH_HEAD_S);

			m_dlg->SetItemVisible(IDC_IMG_TRASH_FINISH_HEAD, TRUE);

            KAppRes& res = KAppRes::Instance();
            CString strFormat = res.GetString("IDS_TRASH_CLEAN_FINISH");
            CString strSizeInfo;
            CString strSize;
            GetFileSizeTextString(m_nDelTotalSize, strSize);
             
            if (m_skipCtrl.GetSize() == 0)
            {
                strSizeInfo.Format(strFormat, strSize);
                m_dlg->SetRichText(IDC_TXT_TRASH_CLEAN_DES, strSizeInfo);
                m_dlg->SetItemText(IDC_TXT_TRASH_FINISH_THIS, strSize);

                GetFileSizeTextString(m_uConfigTotalSize, strSize);
                m_dlg->SetItemText(IDC_TXT_TRASH_FINISH_TOTAL, strSize);
            }
            else
            {
                strFormat = res.GetString("IDS_TRASH_CLEAN_FINISH_HAS_SKIP");
                strSizeInfo.Format(strFormat, strSize, m_skipCtrl.GetSize());
                m_dlg->SetRichText(IDC_TXT_TRASH_CLEAN_DES, strSizeInfo);
                m_dlg->SetItemVisible(IDC_BTN_TRASH_SKIP_RESCAN, FALSE);
                m_dlg->SetItemVisible(IDC_DLG_TRASH_SKIP, TRUE);
                m_dlg->SetItemVisible(IDC_DLG_TRASH_FINISH, FALSE);
                m_dlg->SetItemVisible(IDC_IMG_TRASH_RISK, TRUE);
                m_dlg->SetItemVisible(IDC_IMG_TRASH_FINISH_HEAD, FALSE);
                
            }

			_SetCompleteSize();

		}
		break;
	default:
		break;
	}
}
void CUIHandlerTrash::_SetShowBeginPageShow()
{
	m_dlg->SetItemVisible(IDC_DLG_TRASH_MAIN,			TRUE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_BEGIN,			TRUE);

	m_dlg->SetItemVisible(IDC_CHK_TRASH_CHECK_ALL, TRUE);
	m_dlg->SetItemVisible(IDC_LNK_TRASH_RECOMMEND, TRUE);

	m_dlg->EnableItem(IDC_CHK_TRASH_CHECK_ALL, TRUE);
	m_dlg->EnableItem(IDC_LNK_TRASH_RECOMMEND, TRUE);
	m_dlg->EnableItem(IDC_LNK_SETTING, TRUE);

	m_dlg->SetItemVisible(IDC_DLG_TRASH_TIPS,			FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_FINISH,			FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_FINISH_HEAD,	FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_WORKING,		FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_BEGIN_CLEAN,	FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_SCANING_NONE,	FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_SCAN_CANECL,	FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_SCAN_FINISH,	FALSE);
    m_dlg->SetItemVisible(IDC_DLG_TRASH_SKIP,           FALSE);

}

void CUIHandlerTrash::_SetDoingScanPageShow()
{	
	m_dlg->SetItemVisible(IDC_DLG_TRASH_WORKING,		TRUE);	

	m_dlg->SetItemVisible(IDC_IMG_TRASH_FLASHING_SCAN,  TRUE);
	m_dlg->SetItemVisible(IDC_IMG_TRASH_FLASHING_CLEAN, FALSE);

	m_dlg->SetItemVisible(IDC_DLG_TRASH_BEGIN,			FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_FINISH,			FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_FINISH_HEAD,	FALSE);

	m_dlg->SetItemVisible(IDC_DLG_TRASH_SCANING_NONE,	FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_SCAN_FINISH,	FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_SCAN_CANECL,	FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_BEGIN_CLEAN,	FALSE);
}
void CUIHandlerTrash::_SetDoingCleanPageShow()
{
	m_dlg->SetItemVisible(IDC_DLG_TRASH_WORKING, TRUE);

	m_dlg->SetItemVisible(IDC_IMG_TRASH_FLASHING_CLEAN, TRUE);
	m_dlg->SetItemVisible(IDC_IMG_TRASH_FLASHING_SCAN, FALSE);

	m_dlg->SetItemVisible(IDC_DLG_TRASH_BEGIN, FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_FINISH, FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_FINISH_HEAD, FALSE);

	m_dlg->SetItemVisible(IDC_DLG_TRASH_SCANING_NONE,	FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_SCAN_FINISH,	FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_SCAN_CANECL,	FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_BEGIN_CLEAN,	FALSE);
}

void CUIHandlerTrash::_SetCleanFinishPageShow()
{
	m_dlg->SetItemVisible(IDC_DLG_TRASH_FINISH_HEAD, TRUE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_FINISH, TRUE);

	m_dlg->SetItemVisible(IDC_CHK_TRASH_CHECK_ALL, FALSE);
	m_dlg->SetItemVisible(IDC_LNK_TRASH_RECOMMEND, FALSE);
	// 设置正在扫描页面不可以见

	m_dlg->SetItemVisible(IDC_DLG_TRASH_WORKING, FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_BEGIN, FALSE);
	m_dlg->SetItemVisible(IDC_DLG_TRASH_MAIN, FALSE);

    KAppRes& res = KAppRes::Instance();
    CString strFormat = res.GetString("IDS_TRASH_TIME_A_NUM");
    CString str;
    CString strCount;
    GetFileCountText(m_nCurClearTotalNum ,strCount);

    str.Format(strFormat, m_uCleanTotalTime / 1000.0, strCount);

    m_dlg->SetRichText(IDC_TXT_TRASH_CLEAN_SIZE, str);

    if (m_nCurClearTotalNum == 0)
    {
        m_dlg->SetItemVisible(IDC_LNK_TRASH_CLEAN_DETAIL, FALSE);
    }
    else
    {
        m_dlg->SetItemVisible(IDC_LNK_TRASH_CLEAN_DETAIL, TRUE);
    }
}

void CUIHandlerTrash::_SetScanFinishPageShow()
{
	m_dlg->SetItemVisible(IDC_DLG_TRASH_BEGIN_CLEAN,		TRUE);

	m_dlg->SetItemVisible(IDC_DLG_TRASH_WORKING,			FALSE);

	m_dlg->EnableItem(IDC_CHK_TRASH_CHECK_ALL, FALSE);
	m_dlg->EnableItem(IDC_LNK_TRASH_RECOMMEND, FALSE);

    //
    m_dlg->SetItemVisible(IDC_BTN_TRASH_RESCAN, FALSE);
    m_dlg->SetItemVisible(IDC_BTN_TRASH_BEGIN_CLEAN, TRUE);
    m_dlg->SetItemVisible(IDC_LNK_TRASH_RESCAN, TRUE);
}

// 设置配置
void CUIHandlerTrash::_InitListConfig()
{	
	{
		TRASH_ITEM item;

		item.strItemName = L"上网产生的垃圾文件";
		item.strItemDes = L"上网过程中浏览器产生的缓存文件，定期清理可节省较多磁盘空间，并提高上网速度。";
        item.strBrief = L"浏览器（如IE、傲游、火狐等）上网时产生的垃圾文件。";
		item.nIconID = 0;
		item.bShowDetail = TRUE;
		item.uItemId = BROWER_TRASH;
//		if (CheckSoftInstalled(BROWER_IE))
		{
			TrashDetailItem detailItem;
			detailItem.nItemId = BROWER_IE;
			detailItem.strName = L"IE（含IE内核）浏览器";
			
			item.vecItems.push_back(detailItem);
		}
		if (CheckSoftInstalled(BROWER_SOGO) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(BROWER_SOGO).strDirectory))
		{
			TrashDetailItem detailItem;
			detailItem.nItemId = BROWER_SOGO;
			detailItem.strName = L"搜狗浏览器 高速模式";
			item.vecItems.push_back(detailItem);
		}
        if (CheckSoftInstalled(BROWER_MATHRON) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(BROWER_MATHRON).strDirectory))
		{
			TrashDetailItem detailItem;
			detailItem.nItemId = BROWER_MATHRON;
			detailItem.strName = L"傲游浏览器3 极速模式";
			item.vecItems.push_back(detailItem);
		}
        if (CheckSoftInstalled(BROWER_FIREFOX) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(BROWER_FIREFOX).strDirectory))
		{
			TrashDetailItem detailItem;
			detailItem.nItemId = BROWER_FIREFOX;
			detailItem.strName = L"火狐 Firefox";
			item.vecItems.push_back(detailItem);
		}
        if (CheckSoftInstalled(BROWER_CHROME) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(BROWER_CHROME).strDirectory))
		{
			TrashDetailItem detailItem;
			detailItem.nItemId = BROWER_CHROME;
			detailItem.strName = L"谷歌浏览器 Chrome";
			item.vecItems.push_back(detailItem);
		}
        if (CheckSoftInstalled(BROWER_OPERA) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(BROWER_OPERA).strDirectory))
		{
			TrashDetailItem detailItem;
			detailItem.nItemId = BROWER_OPERA;
			detailItem.strName = L"Opera浏览器";
			item.vecItems.push_back(detailItem);
		}
        if (CheckSoftInstalled(BROWER_SAFARI) ||
            CheckSoftPathExist(m_configData.GetConfigItemByID(BROWER_SAFARI).strDirectory))
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = BROWER_SAFARI;
            detailItem.strName = L"苹果 Safari";
            item.vecItems.push_back(detailItem);
        }
		if (item.vecItems.size() > 0)
		{
			m_trashCtrl.AddItem(item);
		}
	}
	// 视频
	{
		TRASH_ITEM item;

		item.strItemName = L"看视频和听音乐产生的缓存";
		item.strItemDes = L"网络播放器所下载的视频或音乐缓存，定期清理视频和音乐缓存可释放较多磁盘空间。";
		item.strBrief = L"在线视频和音频播放器（如迅雷看看、酷我音乐盒等）产生的缓存文件。";
        item.nIconID = 1;
		item.uItemId = VIDEO_TRASH;

        {
            TrashDetailItem detailItem;
            detailItem.nItemId = VIDEO_MEDIA_PALYER;
            detailItem.strName = L"Windows Media Player";
            item.vecItems.push_back(detailItem);
        }

        if (CheckSoftInstalled(VIDEO_PPTV) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_PPTV).strDirectory))
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = VIDEO_PPTV;
			detailItem.strName = L"PPTV 网络电视";
			item.vecItems.push_back(detailItem);
		}
        if (CheckSoftInstalled(VIDEO_QQLIVE) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_QQLIVE).strDirectory))
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = VIDEO_QQLIVE;
			detailItem.strName = L"QQLive 网络电视";
			item.vecItems.push_back(detailItem);
		}
        if (CheckSoftInstalled(VIDEO_XUNLEI) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_XUNLEI).strDirectory))
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = VIDEO_XUNLEI;
			detailItem.strName = L"迅雷看看";
			item.vecItems.push_back(detailItem);
		}
        if (CheckSoftInstalled(VIDEO_FENGXING) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_FENGXING).strDirectory))
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = VIDEO_FENGXING;
			detailItem.strName = L"风行网络电视";
			item.vecItems.push_back(detailItem);
		}
        if (CheckSoftInstalled(VIDEO_YOUKU) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_YOUKU).strDirectory))
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = VIDEO_YOUKU;
			detailItem.strName = L"优酷加速器";
			item.vecItems.push_back(detailItem);
		}
        if (CheckSoftInstalled(VIDEO_TUDOU) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_TUDOU).strDirectory))
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = VIDEO_TUDOU;
			detailItem.strName = L"飞速土豆";
			item.vecItems.push_back(detailItem);
		}
        if (CheckSoftInstalled(VIDEO_KU6) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_KU6).strDirectory))
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = VIDEO_KU6;
			detailItem.strName = L"极速酷6";
			item.vecItems.push_back(detailItem);
		}
        if (CheckSoftInstalled(VIDEO_STORM))
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = VIDEO_STORM;
            detailItem.strName = L"暴风影音";
            item.vecItems.push_back(detailItem);
        }
        if (CheckSoftInstalled(VIDEO_QVOD) ||
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_QVOD).strDirectory))
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = VIDEO_QVOD;
            detailItem.strName = L"快播 QVOD";
            item.vecItems.push_back(detailItem);
        }

        if (CheckSoftInstalled(VIDEO_PPS)||
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_PPS).strDirectory))
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = VIDEO_PPS;
            detailItem.strName = L"PPS 网络电视";
            item.vecItems.push_back(detailItem);
        }


        if (CheckSoftInstalled(VIDEO_QQMUSIC) ||
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_QQMUSIC).strDirectory))
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = VIDEO_QQMUSIC;
            detailItem.strName = L"QQ音乐";
            item.vecItems.push_back(detailItem);
        }
        
        if (CheckSoftInstalled(VIDEO_KUWO) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_KUWO).strDirectory))
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = VIDEO_KUWO;
            detailItem.strName = L"酷我音乐盒";
            item.vecItems.push_back(detailItem);
        }

        if (CheckSoftInstalled(VIDEO_KUGOO) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_KUGOO).strDirectory))
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = VIDEO_KUGOO;
            detailItem.strName = L"酷狗音乐";
            item.vecItems.push_back(detailItem);
        }
        if (CheckSoftInstalled(VIDEO_PIPI) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_PIPI).strDirectory))
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = VIDEO_PIPI;
            detailItem.strName = L"皮皮影视";
            item.vecItems.push_back(detailItem);
        }       
        if (CheckSoftInstalled(VIDEO_TTPLAYER) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(VIDEO_TTPLAYER).strDirectory))
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = VIDEO_TTPLAYER;
            detailItem.strName = L"千千静听";
            item.vecItems.push_back(detailItem);
        }

        if (CheckSoftInstalled(BROWER_SOGO) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(BROWER_SOGO).strDirectory))
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = VIDEO_SOGOU;
            detailItem.strName = L"搜狗浏览器视频加速缓存";
            item.vecItems.push_back(detailItem);
        }
		if (item.vecItems.size() > 0)
		{
			m_trashCtrl.AddItem(item);
		}
	}

	{
		TRASH_ITEM item;

		item.strItemName = L"Windows系统产生的垃圾文件";
		item.strItemDes = L"系统运行过程中产生的垃圾文件，建议用推荐选项定期进行清理。";
		item.strBrief = L"Windows系统运行过程中产生的垃圾文件。";
        item.nIconID = 2;
		item.uItemId = WIN_TRASH;
		
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = WIN_HUISHOUZHAN;
            detailItem.strName = L"回收站";
            item.vecItems.push_back(detailItem);
        }
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = WIN_USERTEMP;
			detailItem.strName = L"用户临时文件夹";
			item.vecItems.push_back(detailItem);
		}		
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = WIN_TEMP;
			detailItem.strName = L"Windows临时文件夹";
			item.vecItems.push_back(detailItem);
		}
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = WIN_SUOLIETU;
			detailItem.strName = L"缩略图缓存";
			item.vecItems.push_back(detailItem);
		}
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = WIN_ERRPORT;
			detailItem.strName = L"Windows 错误报告";
			item.vecItems.push_back(detailItem);
		}
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = WIN_UPDATE;
			detailItem.strName = L"Windows 更新补丁";
			item.vecItems.push_back(detailItem);
		}
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = WIN_PREFETCH;
			detailItem.strName = L"Windows 预读文件";
			item.vecItems.push_back(detailItem);
		}
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = WIN_DOWNLOAD;
			detailItem.strName = L"系统已下载程序文件";
			item.vecItems.push_back(detailItem);
		}
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = WIN_DUMP;
			detailItem.strName = L"内存转储文件";
			item.vecItems.push_back(detailItem);
		}
		{	
			TrashDetailItem detailItem;
			detailItem.nItemId = WIN_LOG;
			detailItem.strName = L"系统日志文件";
			item.vecItems.push_back(detailItem);
		}
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = WIN_REMOTE_DESKTOP;
            detailItem.strName = L"远程桌面缓存";
            item.vecItems.push_back(detailItem);
        }

		m_trashCtrl.AddItem(item);
	}

    {
        TRASH_ITEM item;

        item.strItemName = L"其他常用软件产生的垃圾文件";
        item.strItemDes = L"常用第三方软件产生的垃圾文件，定期清理可释放较多磁盘空间。";
        item.strBrief = L"其他常用软件（如谷歌 Picasa、Windows Live Message等）产生的垃圾文件。";
        item.nIconID = 3;
        item.bShow = TRUE;
        item.uItemId = COMMON_SOFTWARE;

        {
            TrashDetailItem detailItem;
            detailItem.nItemId = SOFT_KSAFE;
            detailItem.strName = L"金山卫士漏洞修复补丁";
            item.vecItems.push_back(detailItem);
        }

        if (CheckSoftInstalled(WIN_OFFICE))
        {	
            TrashDetailItem detailItem;
            detailItem.nItemId = WIN_OFFICE;
            detailItem.strName = L"Office 安装缓存";
            item.vecItems.push_back(detailItem);
        }
        
        if (CheckSoftInstalled(WIN_LIVE_MGR) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(WIN_LIVE_MGR).strDirectory))
        {
		    TrashDetailItem detailItem;
		    detailItem.nItemId = WIN_LIVE_MGR;
		    detailItem.strName = L"Windows Live Messenger";
		    item.vecItems.push_back(detailItem);
        }

        if (CheckSoftInstalled(SOFT_PICASA) || 
            CheckSoftPathExist(m_configData.GetConfigItemByID(SOFT_PICASA).strDirectory))
        {
            TrashDetailItem detailItem;
            detailItem.nItemId = SOFT_PICASA;
            detailItem.strName = L"谷歌 Picasa";
            item.vecItems.push_back(detailItem);
        }

        if (item.vecItems.size() > 0)
        {
            m_trashCtrl.AddItem(item);
        }
    }

	{
		TRASH_ITEM item;

		item.strItemName = L"系统盘深度清理";
		item.strItemDes = L"根据设置中指定的垃圾文件名规则，搜索整个磁盘分析获得的垃圾文件列表。建议谨慎操作。";
		item.strBrief = L"根据指定文件名规则在系统盘上深度搜索出的各种垃圾文件。";
        item.nIconID = 4;
		item.bShow = FALSE;
		item.uItemId = SYS_TRASH;

		TrashDetailItem detailItem;
		detailItem.nItemId = SYS_TRASH;
		detailItem.strName = L"指定类型垃圾文件";
		item.vecItems.push_back(detailItem);

		m_trashCtrl.AddItem(item);
	}
}
//////////////////////////////////////////////////////////////////////////
