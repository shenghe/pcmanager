//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "dlgslimwarning.h"
//////////////////////////////////////////////////////////////////////////

SlimWarningDlg::SlimWarningDlg(const CString& strWarning) 
    : CKuiDialogImpl<SlimWarningDlg>("IDR_DLG_SLIM_WARNING")
    , m_strWarning(strWarning)
    , m_bUndoEnable(TRUE)
{
}

SlimWarningDlg::~SlimWarningDlg()
{
}

//////////////////////////////////////////////////////////////////////////

BOOL SlimWarningDlg::OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
{
    SetRichText(701, m_strWarning);
    
    // 默认删除到回收站
    SetItemCheck(702, TRUE);
    m_bUndoEnable = TRUE;

    return TRUE;
}

void SlimWarningDlg::OnOK()
{
    m_bUndoEnable = GetItemCheck(702);
    EndDialog(IDOK);
}

void SlimWarningDlg::OnCancel()
{
    EndDialog(IDCANCEL);
}

//////////////////////////////////////////////////////////////////////////

BOOL SlimWarningDlg::UndoEnable() const
{
    return m_bUndoEnable;
}

//////////////////////////////////////////////////////////////////////////
