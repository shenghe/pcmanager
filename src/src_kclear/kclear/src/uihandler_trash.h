//////////////////////////////////////////////////////////////////////////
// Create Author: zhang zexin
// Create Date: 2010-12-16
// Description: 第二版本垃圾清理主界面
// Modify Record:
//////////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////////
#include "configdata.h"
#include "traversefile.h"
#include "trashdefine.h"
#include "filelistdata.h"
#include "kscver.h"
#include "dlgsetting.h"
#include "ktrashlistwin.h"
#include "ktrashskipctrl.h"
#include "kscbase/ksclock.h"


#include <map>
//////////////////////////////////////////////////////////////////////////
#define DEF_KUI_ATT_SKIN					"skin"
#define DEF_KUI_ATT_SUB						"sub"
#define DEF_KUI_ATT_VALUE					"value"


#define DEF_TRASH_SKIN_FINISH_SUCESS_R		_T("trash_finish_success_r")
#define DEF_TRASH_SKIN_FINISH_CANCEL_R		_T("trash_finish_cancel_r")
#define DEF_TRASH_SKIN_FINISH_HEAD_S		_T("trash_ok")
#define DEF_TRASH_SKIN_FINISH_HEAD_F		_T("trash_has")
#define DEF_TRASH_SKIN_SCAN					_T("trash_scanning")
#define DEF_TRASH_SKIN_CLEAR				_T("trash_cleaning")

#define DEF_TRASH_FLASH_NUM					10
#define DEF_TRASH_FLASH_TIME				50 // ms
#define DEF_TRASH_PRO_TIME					200 // ms
#define DEF_TRASH_PROGRESS_COUNT			24


#include "kclearmsg.h"

//////////////////////////////////////////////////////////////////////////
BOOL CALLBACK EnumWndProc(HWND hwnd, LPARAM lParam);

extern UINT g_nCloseID;
//////////////////////////////////////////////////////////////////////////
class CUIHandlerTrash : public ITraverseFile
{
	
enum Trash_status
{
	TRASH_BEGIN = 0,            // 垃圾清理的开始
	TRASH_SCANING,              // 正在扫描
	TRASH_SCANING_FINISH,       // 正常完成扫描且有垃圾
	TRASH_SCANING_CANCEL,       // 扫描被取消
	TRASH_SCANING_NONE,         // 扫描完没有垃圾
	TRASH_CLEANING,             // 正在清理
	TRASH_CLEAN_FINISH,         // 完成清理
	TRASH_CLEAN_CANCEL          // 清理被被取消
};
public:

	CUIHandlerTrash(CKscMainDlg* refDialog);
	virtual ~CUIHandlerTrash(void);

public:
	void Init();
	void UnInit();

    BOOL IsFinishInitilize();
    void SetFinishExamNeedScan(BOOL bNeedScan = TRUE);

protected:
	CKscMainDlg*				m_dlg;
	CTrashCleanCtrl				m_trashCtrl;
	CConfigData					m_configData;
    CTrashSkipCtrl              m_skipCtrl;
	
	ULONGLONG					m_nTotalSize; 
	UINT						m_nTotalCount;
	CString						m_strCacheFile;
	CFileListData				m_FileListData;
	int							m_nCurrentFindIndex;
	int							m_nProIndex;

	BOOL						m_bFinishClean;
	BOOL						m_bScaning;
	BOOL						m_bCleaning;
	BOOL						m_bFindFileRunningSkip;

	int							m_nFlashCount;
	int							m_nFlashProgress;
	int							m_nProgressPos;
	int							m_nTotalItem;
	int							m_nItemCounter;
	BOOL						m_bFirstUse;
	//
	HANDLE                      m_hThread;
	HANDLE                      m_tmpThread;
	DWORD                       m_dwThreadID;
	DWORD                       m_dwInsertThreadID;
	int                         m_nScanStatus;
	bool                        is_init;
	bool                        is_finish;
	bool                        is_ok;
	bool                        is_havespecsuffix;
	int	                        m_nDelTotalCount;
	ULONGLONG                   m_nDelTotalSize;
    std::map<int,int>           m_mapclearflag;

	ULONGLONG                   m_uConfigTotalSize;
	time_t                      m_lastTime;
	time_t                      m_ufirstDay;

    int                         m_nCurItemClearNum;
    ULONGLONG                   m_uCleanTotalTime;
    int                         m_nCurClearTotalNum;

    BOOL                        m_bFinishInitilize;
    BOOL                        m_bExamNeedScan;

public:
	void TraverseFile(LPFINDFILEDATA pFileData);

	void TraverseProcess(unsigned long uCurPos);

	void TraverseProcessEnd(unsigned long uCurPos,CString videoPath);

	void TraverseFinished();

protected:
	static DWORD WINAPI ClearFileThread(LPVOID lpVoid);
	static DWORD WINAPI WaitThread(LPVOID lpVoid);

    static UINT WINAPI InitilizeThread(LPVOID lpVoid);

	void RunClearFile();

public:

	KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_PUSH_APP1,        OnClickPushApp1)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_PUSH_APP2,        OnClickPushApp2)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_PUSH_APP3,        OnClickPushApp3)
        KUI_NOTIFY_ID_COMMAND(IDC_BTN_PUSH_APP4,        OnClickPushApp4)
        KUI_NOTIFY_ID_COMMAND(IDC_BTN_PUSH_APP5,        OnClickPushApp5)
        KUI_NOTIFY_ID_COMMAND(IDC_BTN_PUSH_APP6,        OnClickPushApp6)

		KUI_NOTIFY_ID_COMMAND(IDC_BTN_TRASH_BEGIN_SCAN, OnClickScanBegin)
		KUI_NOTIFY_ID_COMMAND(IDC_CHK_TRASH_CHECK_ALL,	OnCheckAll)
		KUI_NOTIFY_ID_COMMAND(IDC_LNK_TRASH_RETRUN,		OnReturnBeginPage)
		KUI_NOTIFY_ID_COMMAND(IDC_LNK_TRASH_RECOMMEND,	OnClickRecommend)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_TRASH_CANCEL,		OnClickCancelWork)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_TRASH_BEGIN_CLEAN, OnClickBeginClean)
		KUI_NOTIFY_ID_COMMAND(IDC_LNK_TRASH_RESCAN,		OnRescan)
        KUI_NOTIFY_ID_COMMAND(IDC_BTN_TRASH_RESCAN,     OnRescan)
        KUI_NOTIFY_ID_COMMAND(IDC_BTN_TRASH_TIPS_CLOSE,	OnTipsClose)
        KUI_NOTIFY_ID_COMMAND(IDC_LNK_TRASH_CLEAN_DETAIL,OnCheckTrashCleanDetail)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_TRASH_SKIP_RESCAN, OnSkipRescan)

        KUI_NOTIFY_REALWND_RESIZED(IDC_CTR_TRASH_MAIN,	OnCtrlReSize)	
	KUI_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CUIHandlerTrash)
		MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
		MESSAGE_HANDLER(WM_OWN_MSGBOX, OnOwnMsgBox)
		MSG_WM_TIMER(OnTimer)
		MESSAGE_HANDLER(WM_CLEAR_SETTIING,			OnSetClearSetting)
		MESSAGE_HANDLER(WM_TRAVERSEFILE,			OnTraverseFile)
		MESSAGE_HANDLER(WM_TRAVERSEPROCESS,			OnTraverseProcess)
		MESSAGE_HANDLER(WM_TRAVERSEPROCESSEND,		OnTraverseProcessEnd)
		MESSAGE_HANDLER(WM_TRAVERSEFINISHED,		OnTraverseFinished)
		MESSAGE_HANDLER(WM_TRASH_BEGIN_CLEAR,		OnTrashClearBegin)
		MESSAGE_HANDLER(WM_TRASH_FINISH_CLEAR,		OnTrashClearFinish)
		MESSAGE_HANDLER(WM_TRASH_CLEAN_PRO,			OnTrashClearProgress)
        MESSAGE_HANDLER(WM_TRASH_CLEAN_END,         OnTrashClearItemEnd)
		MESSAGE_HANDLER(WM_TRASH_TIPS_SHOW_STATUS,	OnShowTips)

		MESSAGE_HANDLER(WM_TRASH_ONEKEY_CLEAN,		OnOneKeyClean)
        MESSAGE_HANDLER(WM_FINISH_INITILIZE,        OnInitilize);

		MESSAGE_HANDLER(WM_SELCETED_CHECKED_CHANGE, OnCheckedAllChanged)
        MESSAGE_HANDLER(WM_TRASH_CHECK_DETAIL,      OnCheckItemDetail)

        MESSAGE_HANDLER(WM_TRASH_RESCAN,            OnTrashRescan)

        MESSAGE_HANDLER(WM_TRASH_CLOSE_TASK,        OnCloseItemTask)
	END_MSG_MAP()

	void OnTimer(UINT_PTR nIDEvent);
	LRESULT OnOwnMsgBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnSetClearSetting(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	LRESULT OnTraverseFile(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTraverseProcess(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTraverseProcessEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTraverseFinished(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnTrashClearBegin(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTrashClearFinish(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTrashClearProgress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTrashClearItemEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnShowTips(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnCheckedAllChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCheckItemDetail(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    LRESULT OnCloseItemTask(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnTrashRescan(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnOneKeyClean(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnInitilize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void OnCtrlReSize(CRect rcWnd);

	void OnClickPushApp1();
	void OnClickPushApp2();
	void OnClickPushApp3();
    void OnClickPushApp4();
    void OnClickPushApp5();
    void OnClickPushApp6();

	void OnClickCancelWork();
	void OnClickBeginClean();
	void OnClickScanBegin();

	void OnCheckAll();

	void OnReturnBeginPage();
	void OnClickRecommend();
	void OnRescan();
	void OnTipsClose();
    
    void OnCheckTrashCleanDetail();

    void OnSkipRescan();

protected:

	void _InitListConfig();

	void _StartFind();

	void _UpdateUI(Trash_status nStatus);

	BOOL _GetConfigSelected();

	void _OnFlashImgEvent();

	void _OnProgressEvent();

    void _OnUpdateListEvent();

	void _InitilizePageInfo();

	void _SetDeepSearchConfig(std::vector<std::wstring>& vecConfig);
	
	void _SetCompleteSize();
	void _SetLastTimeAndSize();

	void _SetShowBeginPageShow();
	void _SetDoingScanPageShow();
	void _SetDoingCleanPageShow();
	void _SetCleanFinishPageShow();
	void _SetScanFinishPageShow();

	void _SetNeedEnableItems(BOOL bEnbale);

    BOOL _AddItemToSkipCtrl(UINT nCurrentIndex, ULONGLONG uSize);
    BOOL _GetItemProcessName(UINT nIndex, CString strProcess, CString& strName);
    BOOL _GetRunningProcessName(std::vector<CString>& vecProcess, std::vector<CString>& vecRunning);
private:

    KLock m_lock;
};
