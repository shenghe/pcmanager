#ifndef KCLEAR_DLGSLIM_H_
#define KCLEAR_DLGSLIM_H_

//////////////////////////////////////////////////////////////////////////

#include <wtlhelper/whwindow.h>
#include "kscmain.h"
#include "kuires.h"
#include "slimcallback.h"
#include "slimdata.h"
#include "kclearmsg.h"

//////////////////////////////////////////////////////////////////////////

class SlimDlg
    : public CKuiDialogImpl<SlimDlg>
    , public CWHRoundRectFrameHelper<SlimDlg>
{
public:
    SlimDlg(SlimItem& slimItem, BOOL bCompressMode = FALSE);
    virtual ~SlimDlg();

    BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/);
    void OnOK();
    void OnCancel();
    void OnTimer(UINT_PTR nIDEvent);

    KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        KUI_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnCancel)
        KUI_NOTIFY_ID_COMMAND(IDOK, OnOK)
        KUI_NOTIFY_ID_COMMAND(IDCANCEL, OnCancel)
    KUI_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(SlimDlg)
        MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CKuiDialogImpl<SlimDlg>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<SlimDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_TIMER(OnTimer)
        MESSAGE_HANDLER(SLIM_WM_ITEM_TREATED, OnTreated)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

    LRESULT OnTreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
    CString GetShortPath(const CString& strPath);
    CString GetTimeString(unsigned int nSeconds);

private:
    BOOL m_bCompressMode;
    BOOL m_bStopFlag;
    SlimItem& m_slimItem;
    DWORD m_dwSpeedCount;
};

//////////////////////////////////////////////////////////////////////////

#endif // KCLEAR_DLGSLIM_H_
