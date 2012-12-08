#ifndef UIHANDLER_ONEKEY_INC_
#define UIHANDLER_ONEKEY_INC_

//////////////////////////////////////////////////////////////////////////

#include "kuires.h"
#include <string>
#include <vector>
#include "onekeycleanctrl.h"
#include "kclear/ionekeyclean.h"
#include "kscbase/ksclock.h"
#include "kscbase/kcslogging.h"


/////////////////////////////////////////////////////////////////////////

class COneCleanSetting
{
private:
    COneCleanSetting();
    ~COneCleanSetting();

public:
    static COneCleanSetting& Instance();
    void Get(const std::string& strName, std::wstring& strValue);
    void Get(const std::string& strName, int& nValue);
    void Set(const std::string& strName, const std::wstring& strValue);
    void Set(const std::string& strName, int nValue);

private:
    bool Load();
    bool Save();
	bool LoadDefault();

    typedef std::map<std::string, std::wstring> StringStore;
    typedef std::map<std::string, int> IntegerStore;

    StringStore m_vStringStore;
    IntegerStore m_vIntegerStore;
};

//////////////////////////////////////////////////////////////////////////

#include "kclearmsg.h"

// typedef struct tagCurrentCleanEntry {
//     wchar_t m_szType[512];
//     wchar_t m_szItem[1024];
//     double m_nSize;
// } CurrentCleanEntry;

//////////////////////////////////////////////////////////////////////////

class CCleanState : public ICleanCallback
{
public:
    CCleanState()
    {
    }

    ~CCleanState()
    {
    }

    void ReSet()
    {
        m_vCleanEntrys.clear();
        m_strCurrentTask = L"";
    }

    void SetFeedback(HWND hWnd, CleanRecord* pCleanRecord)
    {
        m_hWnd = hWnd;
        m_pCleanRecord = pCleanRecord;
    }

    void PushData()
    {
        _GetData(m_pCleanRecord->m_vEntrys);
        m_pCleanRecord->m_strCurrent = m_strCurrentTask;
        m_pCleanRecord->m_dwEndTime = GetTickCount();
    }

    void _GetData(std::vector<CleanEntry>& vCleanEntrys)
    {
        KAutoLock lock(m_lock);
    
        for (size_t i = 0; i < m_vCleanEntrys.size(); ++i)
        {
            vCleanEntrys.push_back(m_vCleanEntrys[i]);
        }

        m_vCleanEntrys.clear();
    }

    void __stdcall OnBegin()
    {
        SendMessage(
            m_hWnd, 
            WM_CLEAN_STATE, 
            (WPARAM)(PVOID)m_pCleanRecord, 
            0
            );
    }

    void __stdcall OnEnd()
    {
        SendMessage(
            m_hWnd, 
            WM_CLEAN_STATE, 
            (WPARAM)(PVOID)m_pCleanRecord, 
            1
            );
    }

    void __stdcall OnCancel()
    {
        SendMessage(
            m_hWnd, 
            WM_CLEAN_STATE, 
            (WPARAM)(PVOID)(m_pCleanRecord), 
            2
            );
    }

    void __stdcall OnCleanEntry(
        int nType,
        const wchar_t* szType,
        int nItem,
        const wchar_t* szItem,
        double nSize
        )
    {
        KAutoLock lock(m_lock);
        CleanEntry entry;

        if (!szType)
            return;

        m_strCurrentTask = szType;

        if (!szItem)
            return;

        entry.strType = szType;
        entry.strContent = szItem;
        entry.nSize = nSize;

        KCLEAR_LOG(LOG_LEVEL_ONEKEYCLEAN) << UnicodeToAnsi(szType) << (": ") << UnicodeToAnsi(szItem);

        m_vCleanEntrys.push_back(entry);
    }

private:
    HWND m_hWnd;
    CleanRecord* m_pCleanRecord;
    KLock m_lock;
    std::vector<CleanEntry> m_vCleanEntrys;
    std::wstring m_strCurrentTask;
};

//////////////////////////////////////////////////////////////////////////

class CUIHandlerOnekey
{
public:
    CUIHandlerOnekey(CKscMainDlg* refDialog);
    ~CUIHandlerOnekey();

public:
    void Init();
	void InitDelay();
    void UnInit();

protected:
    void _LoadProvider();
    void _FreeProvider();
    BOOL _CheckProvider();

    void UpdateHeader();
    void UpdateMiddle();
    void UpdateButtom();

    void _UpdateIcon();
    void _UpdateTitle();
    void _UpdateButtonAndLink();
    void _UpdateProgress();

    void OnCleanButton();
    void OnCleanLink();
    DWORD OnSetting();
    void OnViewTrashCleanResult();
    void OnViewTrackCleanResult();
    void OnViewRegCleanResult();
    void OnCtlReSize(CRect rcWnd);
    void OnScanAnim();
    void OnScanProgress();
	void OnReturn();

    BOOL LoadSetting();
    BOOL IsFirstUse();
	BOOL IsPopSetting();

    void _FakeClean();
    void _StartClean();
    void _StopClean(BOOL fCancel = TRUE);
    void _SaveReport();
    //static UINT WINAPI _CleanThread(void* pSetting);

    void _CleanTrash();
    void _CleanTrack();
    void _CleanReg();
    void _GetCleanTask();

    LRESULT OnCleanState(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        KUI_NOTIFY_ID_COMMAND(ID_BTN_ONEKEY_MIDDLE, OnCleanButton) // 响应按钮
        KUI_NOTIFY_ID_COMMAND(ID_LNK_ONEKEY_MIDDLE, OnCleanLink) // 响应链接
        KUI_NOTIFY_ID_COMMAND(ID_LNK_ONEKEY_CLEAN_SETTING, OnSetting) // 调整一键清理的范围
        KUI_NOTIFY_REALWND_RESIZED(ID_CTL_ONEKEY_CLEAN, OnCtlReSize)
		KUI_NOTIFY_ID_COMMAND(131, OnReturn)
    KUI_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(CUIHandlerOnekey)
        MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_CLEAN_STATE, OnCleanState)
    END_MSG_MAP()

    CKscMainDlg*        m_dlg;
    COnekeyCleanCtrl    m_ctrlClean;
    BOOL                m_fShowSettingLink;
    //CleanState          m_nCleanState;
    //CleanTask           m_nCurrentTask;
    BOOL                m_fFirstUse;
    BOOL                m_fCanceled;
    BOOL                m_fNeedAdjustSetting;
    DWORD               m_dwStartTime;
    std::wstring        m_strDefault;
    std::wstring        m_strCustom;
    int                 m_nProgress;
    ICleanTask*         m_piTrashCleaner;
    ICleanTask*         m_piTrackCleaner;
    ICleanTask*         m_piRegCleaner;
    HMODULE             m_hRegCleaner;
    HANDLE              m_hCleanThread;
    HANDLE              m_hStopEvent;
    CCleanState         m_cleanStates[3];
    std::string         m_strCleanTasks;
    std::wstring        m_strLastUse;
    std::wstring        m_strLastSummary;
	std::wstring        m_strMakeSure;
	BOOL                m_bSettingByClean;
	time_t              m_theLastPop;
	BOOL                m_bCancel;
};

//////////////////////////////////////////////////////////////////////////

#endif  // UIHANDLER_ONEKEY_INC_
