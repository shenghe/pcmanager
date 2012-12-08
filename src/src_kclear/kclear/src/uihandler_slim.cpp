#include "stdafx.h"
#include "kuires.h"
#include "kscbase/kscsys.h"
#include "kscmaindlg.h"
#include <time.h>
#include "slimhelper.h"
#include "dlgslim.h"
#include "dlgslimwarning.h"

//////////////////////////////////////////////////////////////////////////

CUIHandlerSystemSlim::CUIHandlerSystemSlim(CKscMainDlg* refDialog) : m_dlg(refDialog)
{    
 
}

CUIHandlerSystemSlim::~CUIHandlerSystemSlim(void)
{

}

//////////////////////////////////////////////////////////////////////////

void CUIHandlerSystemSlim::Init()
{
    slimhelper::GainSecurityPrivilege1();
    slimhelper::GainSecurityPrivilege2();

    m_dataListCtrl.Create(m_dlg->GetViewHWND(), NULL, NULL,
        WS_CHILD|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_SINGLESEL|LVS_OWNERDATA|LVS_OWNERDRAWFIXED,
        0, IDC_CTR_SYSTEM_SLIM_LIST, NULL);
    DWORD dwExStyle = LVS_EX_FULLROWSELECT;
    dwExStyle |= m_dataListCtrl.GetExtendedListViewStyle();
    m_dataListCtrl.SetExtendedListViewStyle(dwExStyle);
    m_dataListCtrl.SetFont(KuiFontPool::GetFont(KUIF_DEFAULTFONT));
    m_dataListCtrl.InsertColumn(0, L"名称", LVCFMT_CENTER, 405);
    m_dataListCtrl.InsertColumn(1, L"占用空间", LVCFMT_CENTER, 168);
//    m_dataListCtrl.InsertColumn(2, L"瘦身后", LVCFMT_CENTER, 84); 
    m_dataListCtrl.InsertColumn(2, L"文件位置", LVCFMT_CENTER, 84); 
    m_dataListCtrl.InsertColumn(3, L"操作", LVCFMT_CENTER, 110);    
    m_dataListCtrl.SetNotifyHwnd(m_dlg->m_hWnd);
    m_dataListCtrl.Initialize();

}

void CUIHandlerSystemSlim::UnInit()
{

}

void CUIHandlerSystemSlim::OnShow()
{
    if (m_slimData.IsEmpty())
    {
        m_dlg->SetRichText(IDC_TXT_HANDLE_INFO, _T("正在加载，请稍候..."));
        m_slimData.LoadData(m_dlg->m_hWnd);
    }
}

//////////////////////////////////////////////////////////////////////////
// 数据加载完毕
LRESULT CUIHandlerSystemSlim::OnLoadComplete(
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam, 
    BOOL& bHandled
    )
{
    // todo: SetTimer(...);
    m_dlg->SetRichText(IDC_TXT_HANDLE_INFO, _T("开始扫描..."));

    m_dataListCtrl.SetEnable(FALSE);

    _ResetListData();

    // 开始扫描第一项
    m_nScanIndex = 0;
    m_slimData.Scan(m_nScanIndex, m_dlg->m_hWnd);

    return 0L;
}

//////////////////////////////////////////////////////////////////////////
// 子项目扫描完毕
LRESULT CUIHandlerSystemSlim::OnItemScanComplete(
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam, 
    BOOL& bHandled
    )
{
    CString strMsg;
    int nItemCount = m_slimData.GetCount();


    // todo: 更新List, Disable  always
    _UpdateListStatus(m_nScanIndex);

    // 上报数据
    if (m_nScanIndex >= 0 && m_nScanIndex < nItemCount)
    {
        SlimItem item(m_slimData.GetItem(m_nScanIndex));

        m_feedback.AddData(item.Id(), item.SizeOnDisk());
    }

    // 更新当前项目的状态，进行下一个项目的扫描
    if (++m_nScanIndex != nItemCount)
    {
        m_strScanName = m_slimData.GetName(m_nScanIndex);
        m_nMaxScanPos = int(((double)100 / nItemCount) * (m_nScanIndex + 1)) - 1;
        m_nScanPos = int(((double)100 / nItemCount) * m_nScanIndex);
        m_slimData.Scan(m_nScanIndex, m_dlg->m_hWnd);
    }
    else
    {
         // 全部扫描完成
        _UpdateUI(0);
        // todo: 更新List, Disable = > Enable
        m_dataListCtrl.SetEnable(TRUE);
        m_dataListCtrl.Invalidate();
        m_feedback.Report(_Module.GetAppVersion());
    }

    return 0L;
}
LRESULT CUIHandlerSystemSlim::OnTreatItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int nItem = (int)wParam;

    if (nItem < 0 || nItem >= m_slimData.GetCount())
        return 0L;

    SlimItem& item = m_slimData.GetItem(nItem);
    SlimMethod nMethod = item.Method();
    m_dataListCtrl.EnableWindow(FALSE);
    if (SLIM_DELETE_FILE == nMethod)
    {
        // 删除文件
        SlimWarningDlg dlgWarning(item.Warning());
        if (dlgWarning.DoModal() == IDOK)
        {
            BOOL bUndoEnable = dlgWarning.UndoEnable();
            // report data
            item.SetUndoEnable(bUndoEnable);
            SlimDlg dlgDel(item, FALSE);
            dlgDel.DoModal();
        }
        else
        {
        }
    }
    else if (SLIM_COMPRESS_FILE == nMethod)
    {
        // 压缩文件
        SlimDlg dlgCompress(item, TRUE);
        dlgCompress.DoModal();
    }
    m_dataListCtrl.EnableWindow(TRUE);
    
    _UpdateUI(0);
    _UpdateListStatus(nItem);

    return 0L;
}

LRESULT CUIHandlerSystemSlim::OnItemLink(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int nItem = (int)wParam;
    CAtlArray<CString> vPaths;
    CString strPath;    

    if (nItem < 0 || nItem >= m_slimData.GetCount())
        goto Clear0;

    SlimItem& item = m_slimData.GetItem(nItem);

    m_nCurrentIndex = nItem;

    item.GetPaths(vPaths);

    if (vPaths.GetCount() <= 0)
        goto Clear0;

    if (item.Id() == 0) //壁纸 暂时先这样写
    {
//        ExpandEnvironmentStrings(L"%Systemroot%\\Web\\Wallpaper\\", temp, MAX_PATH * 2);
        TCHAR szLocate[MAX_PATH * 2] = { 0 };
        _tcscpy_s(szLocate, MAX_PATH * 2, vPaths[0]);
        if (KGetWinVersion() == WINVERSION_WIN7)
        {
            PathRemoveFileSpec(szLocate);   
        }
        PathRemoveFileSpec(szLocate);

        vPaths.RemoveAll();
        vPaths.Add(szLocate);
    }
    else if (item.Id() == 1 || item.Id() == 3 || item.Id() == 2 || item.Id() == 4)
    {
        CString strPath = vPaths[0];
        TCHAR szLocate[MAX_PATH * 2] = { 0 };

        _tcscpy_s(szLocate, MAX_PATH * 2, strPath);

        PathRemoveFileSpec(szLocate);

        vPaths.RemoveAll();
        vPaths.Add(szLocate);
    }

    if (vPaths.GetCount() > 0)
    {
        // 弹出目录框
        HMENU hMenu = NULL;
        CPoint pos;

        hMenu = _InitFileContextMenu(vPaths);
        if (!hMenu)
            goto Clear0;

        GetCursorPos(&pos);
        ::TrackPopupMenu(
            hMenu,
            TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
            pos.x, 
            pos.y,
            0,
            m_dlg->m_hWnd,
            NULL
            );

    }
Clear0:
    return 0L;
}

void CUIHandlerSystemSlim::OnTimer(UINT_PTR nIDEvent)
{
    CString strMsg;

    if (nIDEvent != 750)
        return;

    if (m_nScanPos + 1 < m_nMaxScanPos)
    {
        m_nMaxScanPos += 1;
    }

    strMsg.Format(_T("正在计算“%s”大小。总体已完成 <font color=red>%s</font> ..."),
                  m_strScanName,
                  m_nScanPos);

    m_dlg->SetRichText(IDC_TXT_HANDLE_INFO, strMsg);
}

void CUIHandlerSystemSlim::OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
{
    BOOL bRet = FALSE;
    
    if (nID >= DEF_SLIM_MENU_ID_BASE && nID <= DEF_SLIM_MENU_ID_BASE_END)
    {
        SlimItem& slimItem = m_slimData.GetItem(m_nCurrentIndex);
        CAtlArray<CString> vPaths;
        int index = nID - 1000;

        slimItem.GetPaths(vPaths);

        if (slimItem.Id() == 0) //壁纸 暂时先这样写
        {
            //        ExpandEnvironmentStrings(L"%Systemroot%\\Web\\Wallpaper\\", temp, MAX_PATH * 2);
            TCHAR szLocate[MAX_PATH] = { 0 };
            _tcscpy_s(szLocate, MAX_PATH, vPaths[0]);
            if (KGetWinVersion() != WINVERSION_XP)
            {
                PathRemoveFileSpec(szLocate);   
            }
            PathRemoveFileSpec(szLocate);

            vPaths.RemoveAll();
            vPaths.Add(szLocate);
        }
        else if (slimItem.Id() == 1 || slimItem.Id() == 3 || slimItem.Id() == 2|| slimItem.Id() == 4)
        {
            CString strPath = vPaths[0];
            TCHAR szLocate[MAX_PATH] = { 0 };

            _tcscpy_s(szLocate, MAX_PATH, strPath);

            PathRemoveFileSpec(szLocate);

            vPaths.RemoveAll();
            vPaths.Add(szLocate);
        }

        slimhelper::LocateFile(vPaths[index]);
    }
    SetMsgHandled(FALSE);
}
//////////////////////////////////////////////////////////////////////////
void CUIHandlerSystemSlim::_UpdateUI(UINT nState)
{
    CString strSize;
    CString strMsg;

    if (m_slimData.GetEnalbeCount() == 0)
    {
        strMsg = _T("恭喜你，未发现可瘦身项！");
    }
    else
    {
        bigfilehelper::GetFileSizeString(m_slimData.GetSizeMaySaved(), strSize);

        strMsg.Format(_T("为您找到%d个可瘦身项，预计可释放系统盘空间约 <font color=red>%s</font> 。"),
            m_slimData.GetEnalbeCount(),
            strSize);
    }

    m_dlg->SetRichText(IDC_TXT_HANDLE_INFO, strMsg);
}

void CUIHandlerSystemSlim::_ResetListData()
{
    int iCount = 0;

    m_dataListCtrl.RemoveAll();

    for (iCount = 0; iCount < m_slimData.GetCount(); ++iCount)
    {
        SlimItem item(m_slimData.GetItem(iCount));
        SLIM_DRAW_ITEM drawItem;
        CAtlArray<CString> vPaths;

        drawItem.nItemID = item.Id();
        drawItem.strName = item.Name();
        drawItem.strDescription = item.Description();
        drawItem.nTreatment = item.Method();
        drawItem.bInit = FALSE;

        m_dataListCtrl.AddItem(drawItem);
    }
}

void CUIHandlerSystemSlim::_UpdateListStatus(int nIndex)
{
    if (nIndex < 0 || nIndex >= m_slimData.GetCount())
        return;
    
    SlimItem item(m_slimData.GetItem(nIndex));

    m_dataListCtrl.SetItemStatus(item.Id(), item.IsEnable(), item.SizeSaved(), item.SizeOnDisk(), TRUE);
}

HMENU CUIHandlerSystemSlim::_InitFileContextMenu(CAtlArray<CString>& vPaths)
{
    HMENU retval = NULL;
    CString strItem;
    MENUITEMINFO menuItemInfo = { 0 };
    BOOL bRet = FALSE;
    int iCount;
    int nIndex = 0;

    retval = ::CreatePopupMenu();
    if (!retval)
        goto Clear0;

    for (iCount = 0; iCount < (int)vPaths.GetCount(); ++iCount)
    {
//         if (iCount != 0)
//         {
//             // 分隔符
//             menuItemInfo.cbSize = sizeof(menuItemInfo);
//             menuItemInfo.fMask = MIIM_ID | MIIM_STRING;
//             menuItemInfo.fType = MFT_SEPARATOR;
//             menuItemInfo.wID = 0;
//             menuItemInfo.dwTypeData = NULL;  
//             menuItemInfo.cch = 0; 
//             bRet = ::InsertMenuItem(retval, nIndex++, TRUE, &menuItemInfo);
//             if (!bRet)
//                 goto Clear0;
//         }
        strItem = vPaths[iCount].GetBuffer();
        menuItemInfo.cbSize = sizeof(menuItemInfo);
        menuItemInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
        menuItemInfo.fType = MFT_STRING;
        menuItemInfo.fState = MFS_ENABLED;
        menuItemInfo.wID = DEF_SLIM_MENU_ID_BASE + iCount;
        menuItemInfo.dwTypeData = (LPTSTR)(LPCTSTR)strItem;  
        menuItemInfo.cch = strItem.GetLength(); 
        bRet = ::InsertMenuItem(retval, nIndex++, TRUE, &menuItemInfo);

        if (!bRet)
            goto Clear0;
    }

Clear0:
    return retval;
}
//////////////////////////////////////////////////////////////////////////
