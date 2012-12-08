#pragma once

#include <wtlhelper/bkfolderbrowsectrl.h>

class CBeikeSafeFolderBrowseDlg
    : public CBkDialogImpl<CBeikeSafeFolderBrowseDlg>
    , public CWHRoundRectFrameHelper<CBeikeSafeFolderBrowseDlg>
{
public:
    CBeikeSafeFolderBrowseDlg()
        : CBkDialogImpl<CBeikeSafeFolderBrowseDlg>(IDR_BK_FOLDER_BROWSE_DLG)
        , m_bNeverShowed(TRUE)
    {
    }

    CAtlList<CString>& GetSelectedPathes()
    {
        return m_lstSelectedPathes;
    }

    void EndDialog(UINT uRetCode)
    {
        if (IDOK == uRetCode)
            m_wndFolderTreeCtrl.GetSelectedPathes(m_lstSelectedPathes);

        __super::EndDialog(uRetCode);
    }

protected:

    BOOL m_bNeverShowed;

    CBkFolderBrowseCtrl m_wndFolderTreeCtrl;

    CAtlList<CString> m_lstSelectedPathes;

    void OnBkBtnClose()
    {
        EndDialog(IDCANCEL);
    }

    void OnOK()
    {
        if (m_wndFolderTreeCtrl.HasCheckedItem())
            EndDialog(IDOK);
    }

    BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
    {
        m_wndFolderTreeCtrl.Create(GetViewHWND(), IDC_TRV_FOLDER_BROWSE, WS_CHILD | WS_VISIBLE);

        m_wndFolderTreeCtrl.SetFocus();

        EnableItem(IDOK, FALSE);

        return TRUE;
    }

    void OnShowWindow(BOOL bShow, UINT nStatus)
    {
        if (bShow && m_bNeverShowed)
        {
            m_bNeverShowed = FALSE;
        }
    }

    LRESULT OnTreeFolderBrowseItemCheckChange(LPNMHDR pnmh)
    {
        EnableItem(IDOK, m_wndFolderTreeCtrl.HasCheckedItem());

        return 0;
    }

public:

    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
        BK_NOTIFY_ID_COMMAND(IDOK, OnOK)
        BK_NOTIFY_ID_COMMAND(IDCANCEL, OnBkBtnClose)
    BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBeikeSafeFolderBrowseDlg)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CBkDialogImpl<CBeikeSafeFolderBrowseDlg>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBeikeSafeFolderBrowseDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_SHOWWINDOW(OnShowWindow)
        NOTIFY_HANDLER_EX(IDC_TRV_FOLDER_BROWSE, WH3STVN_ITEMCHECKCHANGE, OnTreeFolderBrowseItemCheckChange)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};