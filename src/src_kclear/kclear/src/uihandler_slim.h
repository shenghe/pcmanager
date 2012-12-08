//////////////////////////////////////////////////////////////////////////
// Create Author: zhang zexin
// Create Date: 2011-01-14
// Description: 系统盘瘦身（windows减肥）
// Modify Record:
//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////
#include "miniutil/bkjson.h"
#include "slimlistctrl.h"
#include "slimdata.h"
#include <atlenc.h>


//////////////////////////////////////////////////////////////////////////
enum{
    SLIM_BEGIN_SCAN,
    SLIM_EUNM_END
};
//////////////////////////////////////////////////////////////////////////
#define DEF_SLIM_MENU_ID_BASE           1000
#define DEF_SLIM_MENU_ID_BASE_END       1500
//////////////////////////////////////////////////////////////////////////

class CSysSlicFeedback
{
public:
    typedef struct tagSysSlimInfo {
        UINT nItemID;
        ULONG dwFileSize; // 大小按MB计算
    } SlimFileInfo;

    void AddData(const UINT id, ULONGLONG dwFileSize)
    {
        SlimFileInfo fileInfo;
        fileInfo.nItemID = id;
        fileInfo.dwFileSize = ULONG(dwFileSize/(1024*1024));

        m_slimData.AddTail(fileInfo);
    }

    BOOL Report(LPCTSTR lpVer)
    {
        CString strEncodedList = KANSI_TO_UTF16(GetString());
        DWORD dwReport = 0;
        if (!strEncodedList.GetLength())
            return FALSE;

        CRegKey regKey;
        regKey.Create(HKEY_LOCAL_MACHINE, L"SOFTWARE\\KSafe\\KClear");
        regKey.QueryDWORDValue(L"SlimReport", dwReport);

        if (dwReport != 1)
        {
            regKey.SetDWORDValue(L"SlimReport", (DWORD)1);
        }

        regKey.Close();
        m_slimData.RemoveAll();
        return TRUE;
    }

protected:
    CStringA GetString()
    {
        BkJson::Document jDoc;
        BkJson::Handle jFileList = jDoc[L"slimdata"];
        int i = 0;
        POSITION pos = NULL;
        CStringA strJson;
        int nSize;
        CAtlArray<CHAR> bufEncode;

        pos = m_slimData.GetHeadPosition();
        while (pos)
        {
            const SlimFileInfo& _fileInfo = m_slimData.GetNext(pos);
            BkJson::Handle jFile = jFileList[i++];
            jFile[L"id"] = _fileInfo.nItemID;
            jFile[L"size"] = _fileInfo.dwFileSize;
        }

        strJson = KUTF16_To_UTF8(jDoc.Dump());

        nSize = Base64EncodeGetRequiredLength(strJson.GetLength(), ATL_BASE64_FLAG_NOCRLF);
        bufEncode.SetCount(nSize + 1);
        RtlZeroMemory(bufEncode.GetData(), nSize + 1);
        Base64Encode(
            (const BYTE*)(LPCSTR)strJson, 
            strJson.GetLength(), 
            bufEncode.GetData(),
            &nSize,
            ATL_BASE64_FLAG_NOCRLF
            );

        return bufEncode.GetData();
    }

    CAtlList<tagSysSlimInfo> m_slimData;
};
//////////////////////////////////////////////////////////////////////////
class CUIHandlerSystemSlim
{
public:

    CUIHandlerSystemSlim(CKscMainDlg* refDialog);
    virtual ~CUIHandlerSystemSlim(void);

public:
    void Init();
    void UnInit();
    void OnShow();

public:
    KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)

    KUI_NOTIFY_MAP_END()

    BEGIN_MSG_MAP_EX(CUIHandlerSystemSlim)
        MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
        MSG_WM_COMMAND(OnCommand)
        MESSAGE_HANDLER(SLIM_WM_LOADCOMPLETE, OnLoadComplete)
        MESSAGE_HANDLER(SLIM_WM_ITEM_SCAN_END, OnItemScanComplete)
        MESSAGE_HANDLER(SLIM_WM_ITEM_TREAT, OnTreatItem)
        MESSAGE_HANDLER(SLIM_WM_ITEM_LINK, OnItemLink)
        MSG_WM_TIMER(OnTimer)
    END_MSG_MAP()

    LRESULT OnLoadComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnItemScanComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTreatItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnItemLink(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    void OnTimer(UINT_PTR nIDEvent);
    void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl);

protected:
    CKscMainDlg*    				m_dlg;
    CSlimListCtrl                   m_dataListCtrl;
    SlimData                        m_slimData;
    int                             m_nScanIndex;
    CString                         m_strScanName;
    int                             m_nMaxScanPos;
    int                             m_nScanPos;
    int                             m_nCurrentIndex;

    CSysSlicFeedback                m_feedback;
protected:
    void _UpdateUI(UINT nState);
    void _ResetListData();
    void _UpdateListStatus(int nIndex);

    HMENU _InitFileContextMenu(CAtlArray<CString>& vPaths);
};
