#pragma once

#include <atltime.h>
#include <winmod/winpath.h>
#include <wtlhelper/whwindow.h>
#include <common/whitelist.h>
#include <comproxy/bkutility.h>
#include <bksafe/bksafequarantine.h>
#include <scan/ksrusertrustlist.h>
#include "scan/KLogic.h"

#define MSG_LOAD_QUARANTINE_FINISH          (WM_APP + 1)
#define MSG_REMOVE_QUARANTINE_FINISH        (WM_APP + 2)
#define MSG_RETRIEVE_QUARANTINE_FINISH      (WM_APP + 3)
#define MSG_REMOVE_QUARANTINE_ITEM_NOTIFY   (WM_APP + 4)
#define MSG_RETRIEVE_QUARANTINE_ITEM_NOTIFY (WM_APP + 5)

class CBeikeSafeQuarantineDlg
    : public CBkDialogImpl<CBeikeSafeQuarantineDlg>
    , public CWHRoundRectFrameHelper<CBeikeSafeQuarantineDlg>
{
public:
    CBeikeSafeQuarantineDlg()
        : CBkDialogImpl<CBeikeSafeQuarantineDlg>(IDR_BK_QUARANTINE_DLG)
        , m_bReady(FALSE)
    {
    }

protected:

    BOOL m_bReady;
    CWHListViewCtrl m_wndListQuarantine;

    CAtlArray<BkSafeQuarantineInfo> m_arrQuarantinedFiles;

    void OnBkBtnClose()
    {
        EndDialog(IDCANCEL);
    }

    void OnChkQuarantineCheckAll()
    {
        m_wndListQuarantine.CheckAllItems(GetItemCheck(IDC_CHK_QUARANTINE_CHECK_ALL));
    }

    void OnBtnQuarantineRemove()
    {
        BOOL bHasCheck = FALSE;
        int nCount = (int)m_arrQuarantinedFiles.GetCount();

        for (int i = 0; i < nCount; i ++)
        {
            if (m_wndListQuarantine.GetCheckState(i))
            {
                bHasCheck = TRUE;
                break;
            }
        }

        if (!bHasCheck)
            return;

        CString strText;
        strText.Format(BkString::Get(IDS_VIRSCAN_2500));
        UINT_PTR uRet = CBkSafeMsgBox::Show(strText, NULL, MB_OKCANCEL | MB_ICONWARNING);
        if (IDCANCEL == uRet)
            return;

        ::SetCursor(::LoadCursor(NULL, IDC_WAIT));

        _EnableButtons(FALSE);

        HANDLE hThread = ::CreateThread(NULL, 0, _QuarantineRemoveThreadProc, _CreateWorkThreadParam(MSG_REMOVE_QUARANTINE_ITEM_NOTIFY), 0, NULL);
        ::CloseHandle(hThread);
        hThread = NULL;
    }

    void OnBtnQuarantineRetrieve()
    {
        BOOL bHasCheck = FALSE;
        int nCount = (int)m_arrQuarantinedFiles.GetCount();

        for (int i = 0; i < nCount; i ++)
        {
            if (m_wndListQuarantine.GetCheckState(i))
            {
                bHasCheck = TRUE;
                break;
            }
        }

        if (!bHasCheck)
            return;

        CWHRoundRectDialog<CBkSimpleDialog> dlgNotice;

        dlgNotice.Load(IDR_BK_QUARANTINE_RETRIEVE_NOTICE_DLG);

        UINT_PTR uRet = dlgNotice.DoModal();

        if (IDCANCEL == uRet)
            return;

        ::SetCursor(::LoadCursor(NULL, IDC_WAIT));

        _EnableButtons(FALSE);

        if (dlgNotice.GetItemCheck(IDC_CHK_MSGBOX_NONOTIFYLATER))
        {
            CWhiteList wl;

            CWhiteListItem wli;

            wl.Initialize();

            for (int i = 0; i < nCount; i ++)
            {
                if (m_wndListQuarantine.GetCheckState(i))
                {
                    {
                        WinMod::CWinPath path(m_arrQuarantinedFiles[nCount - i - 1].strPath);

                        wli.m_filepath = path.GetPathWithoutUnicodePrefix();
                    }

                    wl.AddWhiteItem(wli);
					std::wstring szUlr =  wli.m_filepath;
					AddTrustItem( szUlr, enum_TRUST_ITEM_TYPE_FILE );
                }
            }

            wl.Uninitialize();

            CBkUtility util;

            util.Initialize();
            util.WhiteListUpdate();
            util.Uninitialize();

			CSafeMonitorTrayShell::WhiteListLibUpdated();
        }

        HANDLE hThread = ::CreateThread(NULL, 0, _QuarantineRetrieveThreadProc, _CreateWorkThreadParam(MSG_RETRIEVE_QUARANTINE_ITEM_NOTIFY), 0, NULL);
        ::CloseHandle(hThread);
        hThread = NULL;
    }

	void AddTrustItem( std::wstring& strItem, EM_TRUST_ITEM_TYPE nType )
	{
		KLogic logic;
		S_TRUST_LIST trustList;
		trustList.operation = enum_TRUST_LIST_ADD;
		trustList.itemType = nType;
		trustList.vecItemList.push_back( strItem );
		HRESULT hr = logic.SetUserTrustList( trustList );
		CSafeMonitorTrayShell::WhiteListLibUpdated();

		ATLASSERT( SUCCEEDED(hr) );
	}

    BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
    {
        m_wndListQuarantine.Create(
            GetViewHWND(), NULL, NULL, 
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 
            0, IDC_LST_QUARANTINE);

        m_wndListQuarantine.InsertColumn(1, BkString::Get(IDS_LIST_VIRUS_COLUMN_FILE_NAME_TITLE), LVCFMT_LEFT, 380);
        m_wndListQuarantine.InsertColumn(2, BkString::Get(IDS_LIST_VIRUS_COLUMN_VIRUS_NAME_TITLE), LVCFMT_LEFT, 130);
        m_wndListQuarantine.InsertColumn(3, BkString::Get(IDS_VIRSCAN_2501), LVCFMT_CENTER, 140);
        m_wndListQuarantine.ShowCheckBox(TRUE);
        m_wndListQuarantine.SetItemHeight(28);

        _EnableButtons(FALSE);

        HANDLE hThread = ::CreateThread(NULL, 0, _LoadQuarantineThreadProc, m_hWnd, 0, NULL);
        ::CloseHandle(hThread);
        hThread = NULL;

        return TRUE;
    }

    LRESULT OnLVNQuarantineGetDispInfo(LPNMHDR pnmh)
    {
        NMLVDISPINFO *pdi = (NMLVDISPINFO*)pnmh;

        BkSafeQuarantineInfo &info = m_arrQuarantinedFiles[m_arrQuarantinedFiles.GetCount() - pdi->item.iItem - 1];

        if (pdi->item.mask & LVIF_TEXT)
        {
            CString strItem;

            switch (pdi->item.iSubItem)
            {
            case 0:
                {
                    switch (info.hRet)
                    {
                    case E_SKYLARK_RETRIEVE_FAILED_TO_OVERWRITE_DIRECTORY:
                    case E_SKYLARK_RETRIEVE_FAILED_TO_OVERWRITE_FILE:
                        strItem = BkString::Get(IDS_VIRSCAN_2502);
                        break;
                    case E_SKYLARK_RETRIEVE_MISSING_BACKUP_FILE:
                    case E_SKYLARK_RETRIEVE_FAILED_TO_EXTRACT_FILE:
                    case E_SKYLARK_RETRIEVE_FAILED_TO_EXTRACT_BACKUP_INFO:
                        strItem = BkString::Get(IDS_VIRSCAN_2503);
                        break;
                    case S_OK:
                        strItem = BkString::Get(IDS_VIRSCAN_2504);
                        break;
                    case S_FALSE:
                        break;
                    default:
                        strItem.Format(BkString::Get(IDS_VIRSCAN_2505), info.hRet);
                        break;;
                    }

                    WinMod::CWinPath path(info.strPath);
                    strItem += path.GetPathWithoutUnicodePrefix();
                }
                break;
            case 1:
                strItem = info.info.cVirusName;
                break;
            case 2:
                {
                    CTime timeLog(info.info.ftBackupTime);

                    strItem.Format(
                        L"%4d-%02d-%02d %02d:%02d:%02d", 
                        timeLog.GetYear(),
                        timeLog.GetMonth(),
                        timeLog.GetDay(),
                        timeLog.GetHour(),
                        timeLog.GetMinute(),
                        timeLog.GetSecond()
                        );
                }
                break;
            }

            wcsncpy(pdi->item.pszText, strItem, min(strItem.GetLength() + 1, pdi->item.cchTextMax - 1));
        }

        return 0;
    }

    LRESULT OnLVNQuarantineItemChanged(LPNMHDR pnmh)
    {
        LPNMLISTVIEW pnms = (LPNMLISTVIEW)pnmh;

        if (0 == (LVIS_STATEIMAGEMASK & (pnms->uOldState ^ pnms->uNewState)))
        {
            SetMsgHandled(FALSE);
            return 0;
        }

        SetItemCheck(IDC_CHK_QUARANTINE_CHECK_ALL, m_wndListQuarantine.IsAllItemChecked());

        return 0;
    }

    LRESULT OnLoadQuarantineFinish(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CAtlArray<BkSafeQuarantineInfo> *parrLogs = (CAtlArray<BkSafeQuarantineInfo> *)wParam;
        int nCount = (int)lParam;

        m_arrQuarantinedFiles.Copy(*parrLogs);

        m_wndListQuarantine.SetItemCount(nCount);

        m_wndListQuarantine.CheckAllItems(FALSE, TRUE, FALSE);

        SetItemVisible(IDC_LST_QUARANTINE, TRUE);
        SetItemCheck(IDC_CHK_QUARANTINE_CHECK_ALL, FALSE);

        _EnableButtons(TRUE);

        m_bReady = TRUE;

        return 0;
    }

    LRESULT OnRemoveQuarantineFinish(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
//         SetItemVisible(IDC_LST_QUARANTINE, FALSE);
// 
//         m_wndListQuarantine.SetItemCount(0);
//         m_arrQuarantinedFiles.RemoveAll();
// 
//         HANDLE hThread = ::CreateThread(NULL, 0, _LoadQuarantineThreadProc, m_hWnd, 0, NULL);
//         ::CloseHandle(hThread);
//         hThread = NULL;
        SetItemCheck(IDC_CHK_QUARANTINE_CHECK_ALL, FALSE);
        m_wndListQuarantine.CheckAllItems(FALSE);

        _EnableButtons(TRUE);

        return 0;
    }

    LRESULT OnRetrieveQuarantineFinish(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
//         SetItemVisible(IDC_LST_QUARANTINE, FALSE);

//         m_wndListQuarantine.SetItemCount(0);
//         m_arrQuarantinedFiles.RemoveAll();
        SetItemCheck(IDC_CHK_QUARANTINE_CHECK_ALL, FALSE);
        m_wndListQuarantine.CheckAllItems(FALSE);

        _EnableButtons(TRUE);

//         HANDLE hThread = ::CreateThread(NULL, 0, _LoadQuarantineThreadProc, m_hWnd, 0, NULL);
//         ::CloseHandle(hThread);
//         hThread = NULL;

        return 0;
    }

    LRESULT OnRemoveQuarantineItemNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        int nIndex = (int)wParam;
        HRESULT hRet = (HRESULT)lParam;

        if (SUCCEEDED(hRet))
        {
            int nCount = m_arrQuarantinedFiles.GetCount();
            m_arrQuarantinedFiles.RemoveAt(nIndex);

            m_wndListQuarantine.SetItemCount(nCount - 1);
        }

        return 0;
    }

    LRESULT OnRetrieveQuarantineItemNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        int nIndex = (int)wParam;
        HRESULT hRet = (HRESULT)lParam;

        if (SUCCEEDED(hRet))
        {
            int nCount = m_arrQuarantinedFiles.GetCount();
            m_arrQuarantinedFiles.RemoveAt(nIndex);

            m_wndListQuarantine.SetItemCount(nCount - 1);
        }
        else
        {
            m_arrQuarantinedFiles[nIndex].hRet = hRet;
            m_wndListQuarantine.RedrawItems(nIndex, nIndex);
        }


        return 0;
    }

    void _EnableButtons(BOOL bEnable)
    {
        EnableItem(IDC_BTN_SYS_CLOSE, bEnable);

        if (0 == m_wndListQuarantine.GetItemCount())
            bEnable = FALSE;

        EnableItem(IDCANCEL, bEnable);
        EnableItem(IDC_CHK_QUARANTINE_CHECK_ALL, bEnable);
        EnableItem(IDC_BTN_QUARANTINE_REMOVE, bEnable);
        EnableItem(IDC_BTN_QUARANTINE_RETRIEVE, bEnable);

        m_wndListQuarantine.ShowCheckBox(bEnable);
    }

    static DWORD WINAPI _LoadQuarantineThreadProc(LPVOID pvParam)
    {
        HWND hWndNotify = (HWND)pvParam;
        CBkSafeQuarantine quarantine;

        CAtlArray<BkSafeQuarantineInfo> arrQuarantines;

        quarantine.Load(arrQuarantines);

        int nCount = (int)arrQuarantines.GetCount();

        if (::IsWindow(hWndNotify))
            ::SendMessage(hWndNotify, MSG_LOAD_QUARANTINE_FINISH, (WPARAM)&arrQuarantines, nCount);

        ::SetCursor(::LoadCursor(NULL, IDC_ARROW));

        return 0;
    }

    struct _WorkThreadParam 
    {
        HWND hWndNotify;
        UINT uMsgNotify;
        UINT uMsgItemResult;
        CAtlList<BkSafeQuarantineOperateItemInfo> ids;
    };

    _WorkThreadParam* _CreateWorkThreadParam(UINT uMsgNotify)
    {
        _WorkThreadParam* pParam = new _WorkThreadParam;
        int nCount = (int)m_arrQuarantinedFiles.GetCount();

        pParam->hWndNotify = m_hWnd;
        pParam->uMsgNotify = uMsgNotify;

        for (int i = 0; i < nCount; i ++)
        {
            if (m_wndListQuarantine.GetCheckState(i))
            {
				BkSafeQuarantineInfo&	itemInfo = m_arrQuarantinedFiles[nCount - i - 1];

                BkSafeQuarantineOperateItemInfo item;
				item.id = itemInfo.id;
				item.nIndex  = nCount - i - 1;
				item.bHashed = itemInfo.info.bHashed;
				memcpy(item.szMd5,itemInfo.info.byMD5,SKYLARK_MD5_BYTES);

                pParam->ids.AddTail(item);
            }
        }

        return pParam;
    }

    static DWORD WINAPI _QuarantineRemoveThreadProc(LPVOID pvParam)
    {
        _WorkThreadParam *pParam = (_WorkThreadParam *)pvParam;

        if (NULL == pParam)
            return -1;

        CBkSafeQuarantine quarantine;

        quarantine.Remove(pParam->ids, pParam->hWndNotify, pParam->uMsgNotify);

        if (::IsWindow(pParam->hWndNotify))
            ::SendMessage(pParam->hWndNotify, MSG_REMOVE_QUARANTINE_FINISH, 0, 0);

        delete pParam;

        return 0;
    }

    static DWORD WINAPI _QuarantineRetrieveThreadProc(LPVOID pvParam)
    {
        _WorkThreadParam *pParam = (_WorkThreadParam *)pvParam;

        if (NULL == pParam)
            return -1;

        CBkSafeQuarantine quarantine;

        quarantine.Retrieve(pParam->ids, pParam->hWndNotify, pParam->uMsgNotify);

        if (::IsWindow(pParam->hWndNotify))
            ::SendMessage(pParam->hWndNotify, MSG_RETRIEVE_QUARANTINE_FINISH, 0, 0);

        delete pParam;

        return 0;
    }

public:

    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
        BK_NOTIFY_ID_COMMAND(IDC_CHK_QUARANTINE_CHECK_ALL, OnChkQuarantineCheckAll)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_QUARANTINE_REMOVE, OnBtnQuarantineRemove)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_QUARANTINE_RETRIEVE, OnBtnQuarantineRetrieve)
        BK_NOTIFY_ID_COMMAND(IDCANCEL, OnBkBtnClose)
    BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBeikeSafeQuarantineDlg)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CBkDialogImpl<CBeikeSafeQuarantineDlg>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBeikeSafeQuarantineDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        NOTIFY_HANDLER_EX(IDC_LST_QUARANTINE, LVN_GETDISPINFO, OnLVNQuarantineGetDispInfo)
        NOTIFY_HANDLER_EX(IDC_LST_QUARANTINE, LVN_ITEMCHANGED, OnLVNQuarantineItemChanged)
        MESSAGE_HANDLER_EX(MSG_LOAD_QUARANTINE_FINISH, OnLoadQuarantineFinish)
        MESSAGE_HANDLER_EX(MSG_REMOVE_QUARANTINE_FINISH, OnRemoveQuarantineFinish)
        MESSAGE_HANDLER_EX(MSG_RETRIEVE_QUARANTINE_FINISH, OnRetrieveQuarantineFinish)
        MESSAGE_HANDLER_EX(MSG_REMOVE_QUARANTINE_ITEM_NOTIFY, OnRemoveQuarantineItemNotify)
        MESSAGE_HANDLER_EX(MSG_RETRIEVE_QUARANTINE_ITEM_NOTIFY, OnRetrieveQuarantineItemNotify)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};