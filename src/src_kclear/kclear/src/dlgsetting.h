#ifndef KCLEAR_DLGSETTING_H_
#define KCLEAR_DLGSETTING_H_

//////////////////////////////////////////////////////////////////////////

#include <wtlhelper/whwindow.h>
#include "kscmain.h"
#include "kuires.h"
#include "extlist.h"

//////////////////////////////////////////////////////////////////////////

class KClearSettingDlg
    : public CKuiDialogImpl<KClearSettingDlg>
    , public CWHRoundRectFrameHelper<KClearSettingDlg>
{
public:
    KClearSettingDlg();
    virtual ~KClearSettingDlg();

    BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/);
    void OnOK();
    void OnCancel();
    void OnCleanVCacheByTime();
    void OnExtScan();

    BOOL GetExtList(std::vector<std::wstring>& vExts);  // 对外
    BOOL CleanVideoCacheByTime();                       // 对外
    BOOL IsExtScanEnable();                             // 对外
	void SetCustomSelected(std::vector<UINT>& selectedItems);
	BOOL GetCustomSelected(std::vector<UINT>& selectedItems);
	void SetClearTotalSize(ULONGLONG& qSize);
	BOOL GetClearTotalSize(ULONGLONG& qSize);
	void SetClearLastDay(time_t& lastTime);
	BOOL GetClearLastDay(time_t& lastTime);
	void SetLastSize(ULONGLONG& qSize);
	BOOL GetLastSize(ULONGLONG& qSize);
	void SetFirstDay(time_t& qFirstDay);
	BOOL GetClearFirstDay(CString& t);

    BOOL GetReportSize(int& nSize);
    BOOL CheckCustomSelect();
    BOOL IsFilterVedioTime();

    void InitUi();
    void FillData();
    BOOL LoadDefaultSetting();
    BOOL LoadSetting();
    BOOL SaveSetting();
    void OnExtListSize(CRect rcZone);

    void OnComboboxReSize(CRect rcZone);

    KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        KUI_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnCancel)
        KUI_NOTIFY_ID_COMMAND(IDOK, OnOK)
        KUI_NOTIFY_ID_COMMAND(IDCANCEL, OnCancel)
        KUI_NOTIFY_ID_COMMAND(IDCANCEL, OnCancel)
        KUI_NOTIFY_ID_COMMAND(IDC_CHK_CLEAN_VCACHE_BYTIME, OnCleanVCacheByTime)
        KUI_NOTIFY_ID_COMMAND(IDC_CHK_EXT_SCAN, OnExtScan)
        KUI_NOTIFY_REALWND_RESIZED(IDC_CTL_TRASH_LIMIT_COMBOX, OnComboboxReSize)
    KUI_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(KClearSettingDlg)
        MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
		MESSAGE_HANDLER(WM_NONE_CHECKED, OnNoneChecked)
        CHAIN_MSG_MAP(CKuiDialogImpl<KClearSettingDlg>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<KClearSettingDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

protected:
    void SetChecked(const std::wstring& name, bool checked);
	LRESULT OnNoneChecked(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    CExtListCtrl m_extListCtrl;
    std::vector<ExtEntry> m_extList;
    BOOL m_bCleanVCacheByTime;
    BOOL m_bEnableExtScan;

	BOOL m_bLoadSettiing;
	std::string m_strTotalSize;
	std::string m_strSelected;
	std::string m_strFirstDay;
	std::string m_strLastSize;
	std::string m_strLastDay;

    CComboBox m_boxSizeLimit;
    BOOL m_bScanCustomSelect;
    int m_nReportLimit;
    BOOL m_bFilterVedioTime;
    BOOL m_bEnableMonitor;

    BOOL m_bDefaultCustom;
};

//////////////////////////////////////////////////////////////////////////

#endif // KCLEAR_DLGSETTING_H_
