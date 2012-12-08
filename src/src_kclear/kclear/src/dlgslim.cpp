#include "stdafx.h"
#include "dlgslim.h"
#include "kuiwin/kuimsgbox.h"

//////////////////////////////////////////////////////////////////////////

SlimDlg::SlimDlg(SlimItem& slimItem, BOOL bCompressMode) 
    : m_bCompressMode(bCompressMode)
    , CKuiDialogImpl<SlimDlg>("IDR_DLG_SLIM_DOING")
    , m_bStopFlag(FALSE)
    , m_slimItem(slimItem)
    , m_dwSpeedCount(0)
{
}

SlimDlg::~SlimDlg()
{
}

//////////////////////////////////////////////////////////////////////////

BOOL SlimDlg::OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
{
    // 开始工作
    CString strTitle;
    strTitle.Format(_T("正在处理“%s”"), m_slimItem.Name());
    SetItemText(601, strTitle);
    if (m_slimItem.Method() == SLIM_DELETE_FILE)
        SetItemText(602, _T("正在删除文件："));
    else if (m_slimItem.Method() == SLIM_COMPRESS_FILE)
        SetItemText(602, _T("正在压缩文件："));


    m_slimItem.BeginProcess(m_hWnd);
    SetTimer(0, 500);

    return TRUE;
}

void SlimDlg::OnOK()
{
    EndDialog(IDOK);
}

void SlimDlg::OnCancel()
{
    m_slimItem.CancelProcess();
    EndDialog(IDCANCEL);
}

void SlimDlg::OnTimer(UINT_PTR nIDEvent)
{
    CString strFilePath;
    ULONGLONG qwSpeed = 0;

    if (nIDEvent != 0)
        return;

    m_slimItem.GetCurrentFile(strFilePath);
    SetItemText(603, GetShortPath(strFilePath));

    ULONGLONG qwMaySave = m_slimItem.SizeMaySave();
    ULONGLONG qwSaved = m_slimItem.SizeSaved();
    int nPos = 0;
    if (qwSaved >= qwMaySave)
    {
        nPos = 99;
    }
    else
    {
        if (qwMaySave)
        {
            nPos = (int)((qwSaved * 100) / qwMaySave);
            if (0 == nPos)
                nPos = 1;
        }
    }
    SetItemIntAttribute(605, "value", nPos);

    if (++m_dwSpeedCount > 20 && m_dwSpeedCount)
    {
        qwSpeed = qwSaved / m_dwSpeedCount;
        if (qwSaved >= qwMaySave)
        {
            SetItemText(604, _T("即将完成"));
        }
        else
        {
            if (qwSpeed)
            {
                unsigned int nSeconds = unsigned int((qwMaySave - qwSaved) / qwSpeed) / 2;
                SetItemText(604, GetTimeString(nSeconds));
            }
        }
    }
}

LRESULT SlimDlg::OnTreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (!wParam)
    {
        CKuiMsgBox::Show(TEXT("处理失败，该项文件可能正被占用，您\n可以尝试在安全模式下进行本项操作！"), L"系统盘瘦身", MB_OK|MB_ICONWARNING);
    }

    EndDialog(IDOK);
    return 0L;
}

CString SlimDlg::GetShortPath(const CString& strPath)
{
    if (strPath.GetLength() < 40)
        return strPath;

    CString strLeft = strPath.Left(15);
    CString strRight = strPath.Right(15);
    return (strLeft + _T("...") + strRight);
}

CString SlimDlg::GetTimeString(unsigned int nSeconds)
{
    CString strTime;

    if (nSeconds > 60 * 60)
    {
        strTime.Format(_T("%d 小时 %d分"),
                       nSeconds / (60 * 60),
                       (nSeconds % (60 * 60)) / 60);
        return strTime;
    }

    if (nSeconds > 60)
    {
        strTime.Format(_T("%d 分 %d 秒"),
                       nSeconds / 60,
                       nSeconds % 60);
        return strTime;
    }

    strTime.Format(_T("%d 秒"), nSeconds);
    return strTime;
}

//////////////////////////////////////////////////////////////////////////
