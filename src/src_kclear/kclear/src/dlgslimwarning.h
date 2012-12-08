#ifndef KCLEAR_DLGSLIMWARNING_H_
#define KCLEAR_DLGSLIMWARNING_H_

//////////////////////////////////////////////////////////////////////////

#include <wtlhelper/whwindow.h>
#include "kscmain.h"
#include "kuires.h"

//////////////////////////////////////////////////////////////////////////

class SlimWarningDlg 
    : public CKuiDialogImpl<SlimWarningDlg>
    , public CWHRoundRectFrameHelper<SlimWarningDlg>
{
public:
    SlimWarningDlg(const CString& strWarning);
    virtual ~SlimWarningDlg();

    BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/);
    void OnOK();
    void OnCancel();

    BOOL UndoEnable() const;

    KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        KUI_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnCancel)
        KUI_NOTIFY_ID_COMMAND(IDOK, OnOK)
        KUI_NOTIFY_ID_COMMAND(IDCANCEL, OnCancel)
    KUI_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(SlimDlg)
        MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CKuiDialogImpl<SlimWarningDlg>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<SlimWarningDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

private:
    CString m_strWarning;
    BOOL m_bUndoEnable;
};

//////////////////////////////////////////////////////////////////////////

#endif // KCLEAR_DLGSLIMWARNING_H_
