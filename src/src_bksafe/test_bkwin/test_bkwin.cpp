// test_bkwin.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "test_bkwin.h"
#include <wtlhelper/whwindow.h>

class CTestDlg
    : public CBkRichWinImpl<CTestDlg>
    , public CWHRoundRectFrameHelper<CTestDlg>
{
public:
    CTestDlg()
        : /*CBkRichWinImpl<CTestDlg>(IDR_BK_TEST_DIALOG)*/
        /*, */m_dwPos(0)
    {

    }

protected:

    DWORD m_dwPos;
    CListViewCtrl m_wndListCtrl;
    CWHHtmlContainerWindow m_ieCtrl;
    CWHEdit m_wndEditXml;

    void OnBkBtnClose()
    {
        EndDialog(IDCANCEL);
    }

    void OnBkBtnMax()
    {
        if (WS_MAXIMIZE == (GetStyle() & WS_MAXIMIZE))
        {
            SendMessage(WM_SYSCOMMAND, SC_RESTORE | HTCAPTION, 0);
            SetItemIntAttribute(IDBK_BTN_MAX, "src", IDB_BTN_SYS_MAX);
        }
        else
        {
            SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE | HTCAPTION, 0);
            SetItemIntAttribute(IDBK_BTN_MAX, "src", IDB_BTN_SYS_RESTORE);
        }
    }

    void OnBkBtnMin()
    {
        SendMessage(WM_SYSCOMMAND, SC_MINIMIZE | HTCAPTION, 0);
    }

    void OnBkBtnUgly()
    {
        CBkSimpleRichDlg dlg(0);

        dlg.SetXml("<layer width=300 height=200><header class=mainall width=full height=full><button id=1 pos=\"40,40,-40,-40\" class=uglybtn>Close</button></header></layer>");

        dlg.DoModal();
    }

    void OnBkLblLink()
    {
        SetTabCurSel(IDBK_TAB_MAIN, 1);
    }

    void OnBkBtnCheckCheckBox2()
    {
        if (GetItemCheck(IDBK_CHK_CHECKBOX2))
            MessageBox(L"Yes");
        else
            MessageBox(L"No");
    }

    BOOL OnBkTabMainSelChange(int nTabItemIDOld, int nTabItemIDNew)
    {
        if (2 == nTabItemIDNew)
            SetTimer(1, 100, NULL);
        else
            KillTimer(1);

        return TRUE;
    }

    void OnBkCheckBox1()
    {
        if (GetItemCheck(IDBK_CHK_CHECKBOX1))
            SetTimer(1, 100, NULL);
        else
            KillTimer(1);
    }

    void OnBkBtnSkinned()
    {
        SetItemText(IDBK_BTN_SKINNED, L"而且宽度还是自适应的", TRUE);
    }

    void OnBkBtnTest()
    {
        SetItemVisible(IDBK_LST_TEST, !IsItemVisible(IDBK_LST_TEST));
    }

    void OnBkBtnTestDisable()
    {
        EnableItem(IDBK_BTN_DISABLED, !IsItemEnable(IDBK_BTN_DISABLED));
        EnableItem(IDBK_DLG_DISABLED, !IsItemEnable(IDBK_DLG_DISABLED));
    }

    void OnBkBtnDisabledInDlg()
    {
        MessageBox(L"Click");
    }

    void OnBkBtnChangeXml()
    {
        CString strText;

        m_wndEditXml.GetWindowText(strText);

        SetPanelXml(IDBK_DIV_CHANGE_XML, CW2A(strText, CP_UTF8));
    }

    BOOL m_bSplitterShowDetail;

    void OnBkBtnSplitter()
    {
        m_bSplitterShowDetail = !m_bSplitterShowDetail;

        SetPanelXml(IDBK_DLG_SPLITTEST, m_bSplitterShowDetail ? IDR_BK_VIEW_DETAIL_PANEL : IDR_BK_HIDE_DETAIL_PANEL);
    }

    LRESULT OnInitDialog(HWND /*hWnd*/, LPARAM /*lParam*/)
    {
        SetItemCheck(IDBK_CHK_CHECKBOX1, TRUE, FALSE);
        SetItemCheck(IDBK_RDO_RADIO1_2, TRUE, FALSE);

        m_wndEditXml.Create(
            GetViewHWND(), NULL, NULL, 
            WS_CHILD | ES_READONLY | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN, 0, IDBK_TXT_XML);
        m_wndEditXml.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));

        m_wndEditXml.SetBkColor(RGB(0xFF, 0xFF, 0x00));

        m_wndEditXml.SetWindowText(L"<text pos=\"10,10\" crtext=FF0000>Test SetPanelXml</text>");

        m_wndListCtrl.Create(
            GetViewHWND(), NULL, NULL, 
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 0, IDBK_LST_TEST);

        m_wndListCtrl.InsertColumn(0, BkString::Get(IDS_BK_COLUMN_1), LVCFMT_LEFT, 100);
        m_wndListCtrl.InsertColumn(1, BkString::Get(IDS_BK_COLUMN_2), LVCFMT_LEFT, 100);
        m_wndListCtrl.InsertColumn(2, BkString::Get(IDS_BK_COLUMN_3), LVCFMT_CENTER, 80);

        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");
        m_wndListCtrl.AddItem(0, 0, L"fasdfasfas");

        m_ieCtrl.Create(GetViewHWND(), IDBK_IE_ONLINEINFO);

        m_ieCtrl.Show(L"http://www.beike.cn");

        m_bSplitterShowDetail = TRUE;

        SetPanelXml(IDBK_DLG_SPLITTEST, IDR_BK_VIEW_DETAIL_PANEL, FALSE);

        SetTimer(1, 100);

        return TRUE;
    }

    void OnTimer(UINT_PTR nIDEvent)
    {
        static int s_v = 0;
        SetItemIntAttribute(12345, "sub", (s_v ++) % 3);
//         SetItemDWordAttribute(IDBK_PROGRESSBAR, "value", m_dwPos += 50, TRUE);
//         if (m_dwPos == 10000)
//             m_dwPos = -50;
    }

public:

    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        BK_NOTIFY_ID_COMMAND(IDBK_BTN_CLOSE, OnBkBtnClose)
        BK_NOTIFY_ID_COMMAND(IDBK_BTN_UGLY, OnBkBtnUgly)
        BK_NOTIFY_ID_COMMAND(IDBK_LBL_LINK, OnBkLblLink)
        BK_NOTIFY_ID_COMMAND(IDBK_CHK_CHECKBOX1, OnBkCheckBox1)
        BK_NOTIFY_ID_COMMAND(IDBK_BTN_CHECK_CHECKBOX2, OnBkBtnCheckCheckBox2)
        BK_NOTIFY_ID_COMMAND(IDBK_BTN_SKINNED, OnBkBtnSkinned)
        BK_NOTIFY_ID_COMMAND(IDBK_BTN_CLOSE, OnBkBtnClose)
        BK_NOTIFY_ID_COMMAND(IDBK_BTN_MAX, OnBkBtnMax)
        BK_NOTIFY_ID_COMMAND(IDBK_BTN_MIN, OnBkBtnMin)
        BK_NOTIFY_ID_COMMAND(IDBK_BTN_TEST, OnBkBtnTest)
        BK_NOTIFY_ID_COMMAND(IDBK_BTN_TEST_DISABLE, OnBkBtnTestDisable)
        BK_NOTIFY_ID_COMMAND(IDBK_BTN_DISABLED_IN_DLG, OnBkBtnDisabledInDlg)
        BK_NOTIFY_ID_COMMAND(IDBK_BTN_CHANGE_XML, OnBkBtnChangeXml)
        BK_NOTIFY_ID_COMMAND(IDBK_BTN_SPLITTER, OnBkBtnSplitter)
        BK_NOTIFY_TAB_SELCHANGE(IDBK_TAB_MAIN, OnBkTabMainSelChange)
    BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CTestDlg)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CBkRichWinImpl<CTestDlg>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CTestDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_TIMER(OnTimer)
    END_MSG_MAP()
};

CAppModule _Module;

void _Main();

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
//     _Main();
// 
//     return 0;
// 
    BkSkin::LoadSkins(IDR_BK_SKIN_DEF);
    BkStyle::LoadStyles(IDR_BK_STYLE_DEF);
    BkString::Load(IDR_BK_STRING_DEF);

    CTestDlg dlg;

    dlg.Load(IDR_BK_TEST_DIALOG);

    dlg.DoModal();

    return 0;
}

#include <bdx/bdx.h>
    
struct _struct_test 
{
    int nSize;
    int nCount;
    DWORD dwCount;
    CString strFileName;
    CString strName;
};

struct _struct_test2 
{
    int nsize;
    _struct_test stru;
    CAtlArray<int> arrInt;
    CAtlList<int> lstInt;
    CAtlMap<int, CString> mapIntStr;
};

BDX_DEF_BEGIN(_struct_test)
    BDX_DEF(nSize, L"size")
    BDX_DEF(nCount, L"count")
    BDX_DEF(dwCount, L"dwcount")
    BDX_DEF(strFileName, L"filename")
    BDX_DEF(strName, L"name")
BDX_DEF_END()

BDX_DEF_BEGIN(_struct_test2)
    BDX_DEF(nsize, L"size")
    BDX_DEF(stru, L"stru")
    BDX_DEF(arrInt, L"array")
    BDX_DEF(lstInt, L"list")
    BDX_DEF(mapIntStr, L"map")
BDX_DEF_END()


typedef struct _BK_FILE_RESULT
{
    CString		FileName;		                //文件名
    DWORD		Type;                           //扫描结果类型
    DWORD		Status;			                //文件状态（需要清除、清除成功、清除失败等）
    DWORD		Track;			                //文件来源（内存，启动项、系统关键目录或者是普通文件枚举等）
    DWORD		VirusType;		                //病毒类型
    CString		MD5;			                //文件MD5
    DWORD		CRC;			                //文件CRC
    CString		VirusName;		                //病毒名
}BK_FILE_RESULT, *PBK_FILE_RESULT;

BDX_DEF_BEGIN(BK_FILE_RESULT)
    BDX_DEF_LAZY(FileName)
    BDX_DEF_LAZY(Type)
    BDX_DEF_LAZY(Status)
    BDX_DEF_LAZY(Track)
    BDX_DEF_LAZY(VirusType)
    BDX_DEF_LAZY(MD5)
    BDX_DEF_LAZY(CRC)
    BDX_DEF_LAZY(VirusName)
BDX_DEF_END()


void _Main()
{
//     _struct_test2 st, st2;
// 
//     st.nsize = 5;
// 
//     st.stru.nSize = 1;
//     st.stru.dwCount = -1;
//     st.stru.nCount = 2;
//     st.stru.strFileName = L"abcdefg";
//     st.stru.strName = L"name";
// 
//     st.arrInt.SetCount(3);
// 
//     st.arrInt[0] = 9;
//     st.arrInt[1] = 10;
//     st.arrInt[2] = 11;
// 
//     st.lstInt.AddTail(20);
//     st.lstInt.AddTail(21);
//     st.lstInt.AddTail(22);
// 
//     st.mapIntStr[1] = L"lsdajflskjdf";
//     st.mapIntStr[2] = L"bxcvbx";
//     st.mapIntStr[3] = L"ruye";

    BK_FILE_RESULT st,st2;

    st.FileName = L"fasdfasdfasdfasfsd";

    CString strText;

    BDX_ToJson(st, strText);

    OutputDebugString(strText);
    OutputDebugString(L"\r\n");

    BDX_FromJson(strText, st2);

    CString strDebug;

    BDX_ToJson(st2, strDebug);

    OutputDebugString(strDebug);
    OutputDebugString(L"\r\n");
}