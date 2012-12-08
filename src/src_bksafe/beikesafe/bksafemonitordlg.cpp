#include "StdAfx.h"
#include "bksafemonitordlg.h"
#include "beikesafemaindlg.h"
#include <safemon/safemonitor.h>
#include "KwsActivate\kactivate.h"
#include "communits/ThreadLangLocale.h"
#include <bksafe/bksafeconfig.h>

#define SAFE_CALL_FUN(x,y) if (NULL != (x)){(x)->y;}

#define MACRO_STR_BEGIN
#define STR_ALERT_OPEN_SAFEMONITOR BkString::Get(IDS_ALERT_OPEN_SAFEMONITOR_X64)
#define MACRO_STR_ENG

CBKSafeMonitorDlg::~CBKSafeMonitorDlg(void)
{
    size_t nCount = m_arrIconTemp.GetCount();

    for (size_t i = 0; i < nCount; i ++)
    {
        if (m_arrIconTemp[i])
            ::DestroyIcon(m_arrIconTemp[i]);
    }

    ::DestroyIcon(m_hIconRegedit);
}

UINT_PTR CBKSafeMonitorDlg::DoModal(int nPage, HWND hWndParent)
{
	m_nPage = nPage;

	return __super::DoModal(hWndParent);
}


BOOL CBKSafeMonitorDlg::OnInitDialog(HWND wParam, LPARAM lParam)
{
	Init();

	SetTabCurSel(IDC_TAB_MAIN, m_nPage);

	return TRUE;
}

void CBKSafeMonitorDlg::Init()
{
// 	if (NULL == m_pNotifyWnd)
// 		return;
	m_wndProtectionNotify.Create(this->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE);
	m_bWarningTrunOff = BKSafeConfig::Get_Protection_Trunoff_Warning();

     m_wndListMonitorCtrl.Create(GetViewHWND(), IDC_LST_PROTECTION_CTRL);
     m_wndListMonitorCtrl.Load(IDR_BK_PROTECTION_CTRL_LIST_TEMPLATE);
 
     m_wndListMonitorLog.Create(GetViewHWND(), IDC_LST_PROTECTION_CTRL_LOG);
     m_wndListMonitorLog.Load(IDR_BK_PROTECTION_LOG_LIST_TEMPLATE);
}


LRESULT CBKSafeMonitorDlg::OnAppProtectionSwitchChange(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	BOOL bOn = (BOOL)lParam;
	UINT uIDLblStatus = 0, uIDImgStatus = 0, uIDLblTurnOn = 0, uIDLblTurnOff = 0;
	UINT uIDTxtTurnOn = 0, uIDTxtTurnOff = 0;
	UINT uIDTxt = 0;

// 	if (NULL == m_pNotifyWnd)
// 		return 0;

	switch ( m_wndProtectionNotify.GetMonitorStatus() )
	{
	case 0:
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_OK, TRUE));
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_WARNING1, FALSE));
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_WARNING2, FALSE));
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_WARNING3, FALSE));
		SAFE_CALL_FUN(this, SetItemIntAttribute(IDC_IMG_PROTECTION_STATUS, "sub", 2));
		break;

	case 1:
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_OK, FALSE));
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_WARNING1, TRUE));
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_WARNING2, FALSE));
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_WARNING3, FALSE));
		SAFE_CALL_FUN(this, SetItemIntAttribute(IDC_IMG_PROTECTION_STATUS, "sub", 1));
		break;

	case 2:
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_OK, FALSE));
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_WARNING1, FALSE));
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_WARNING2, TRUE));
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_WARNING3, FALSE));
		SAFE_CALL_FUN(this, SetItemIntAttribute(IDC_IMG_PROTECTION_STATUS, "sub", 1));
		break;

	case 3:
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_OK, FALSE));
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_WARNING1, FALSE));
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_WARNING2, FALSE));
		SAFE_CALL_FUN(this, SetItemVisible(IDC_DIV_PROTECTION_WARNING3, TRUE));
		SAFE_CALL_FUN(this, SetItemIntAttribute(IDC_IMG_PROTECTION_STATUS, "sub", 0));
		break;
	}

	switch (wParam)
	{
	case SM_ID_INVAILD:
		SAFE_CALL_FUN(m_pNotifyWnd, RemoveFromTodoList(BkSafeExamItem::ConfigSystemMonitor));
		return 0;
		break;

	case SM_ID_RISK:
		uIDImgStatus    = IDC_IMG_PROTECTION_STAT_REGISTER;
		uIDLblStatus    = IDC_LBL_PROTECTION_STATUS_REGISTER;
		uIDLblTurnOn    = IDC_LBL_PROTECTION_TURN_ON_REGISTER;
		uIDLblTurnOff   = IDC_LBL_PROTECTION_TURN_OFF_REGISTER;

		uIDTxtTurnOn    = IDC_TXT_PROTECTION_TURN_ON_REGISTER;
		uIDTxtTurnOff   = IDC_TXT_PROTECTION_TURN_OFF_REGISTER;

		uIDTxt			= IDC_TXT_PROTECTION_RISK1;
		break;

	case SM_ID_PROCESS:
		uIDImgStatus    = IDC_IMG_PROTECTION_STAT_PROCESS;
		uIDLblStatus    = IDC_LBL_PROTECTION_STATUS_PROCESS;
		uIDLblTurnOn    = IDC_LBL_PROTECTION_TURN_ON_PROCESS;
		uIDLblTurnOff   = IDC_LBL_PROTECTION_TURN_OFF_PROCESS;

		uIDTxtTurnOn    = IDC_TXT_PROTECTION_TURN_ON_PROCESS;
		uIDTxtTurnOff   = IDC_TXT_PROTECTION_TURN_OFF_PROCESS;

		uIDTxt			= IDC_TXT_PROTECTION_PROCESS1;
		break;

	case SM_ID_UDISK:
		uIDImgStatus    = IDC_IMG_PROTECTION_STAT_UDISK;
		uIDLblStatus    = IDC_LBL_PROTECTION_STATUS_UDISK;
		uIDLblTurnOn    = IDC_LBL_PROTECTION_TURN_ON_UDISK;
		uIDLblTurnOff   = IDC_LBL_PROTECTION_TURN_OFF_UDISK;

		uIDTxtTurnOn    = IDC_TXT_PROTECTION_TURN_ON_UDISK;
		uIDTxtTurnOff   = IDC_TXT_PROTECTION_TURN_OFF_UDISK;

		uIDTxt			= IDC_TXT_PROTECTION_UDISK1;
		break;

	case SM_ID_LEAK:
		uIDImgStatus    = IDC_IMG_PROTECTION_STAT_LEAK;
		uIDLblStatus    = IDC_LBL_PROTECTION_STATUS_LEAK;
		uIDLblTurnOn    = IDC_LBL_PROTECTION_TURN_ON_LEAK;
		uIDLblTurnOff   = IDC_LBL_PROTECTION_TURN_OFF_LEAK;

		uIDTxtTurnOn    = IDC_TXT_PROTECTION_TURN_ON_LEAK;
		uIDTxtTurnOff   = IDC_TXT_PROTECTION_TURN_OFF_LEAK;

		uIDTxt			= IDC_TXT_PROTECTION_LEAK1;
		break;

	case SM_ID_KWS:
		uIDImgStatus    = IDC_IMG_PROTECTION_STAT_BROWSERTPROTECT;
		uIDLblStatus    = IDC_LBL_PROTECTION_STATUS_BROWSERTPROTECT;
		uIDLblTurnOn    = IDC_LBL_PROTECTION_TURN_ON_BROWSERTPROTECT;
		uIDLblTurnOff   = IDC_LBL_PROTECTION_TURN_OFF_BROWSERTPROTECT;

		uIDTxtTurnOn    = IDC_TXT_PROTECTION_TURN_ON_BROWSERTPROTECT;
		uIDTxtTurnOff   = IDC_TXT_PROTECTION_TURN_OFF_BROWSERTPROTECT;

		uIDTxt			= IDC_TXT_PROTECTION_BROWSERTPROTECT1;
		break;

    //case SM_ID_BWSP_FISHING:
    //    uIDImgStatus    = IDC_IMG_PROTECTION_STAT_BWSP_PHISHING;
    //    uIDLblStatus    = IDC_LBL_PROTECTION_STATUS_BWSP_PHISHING;
    //    uIDLblTurnOn    = IDC_LBL_PROTECTION_TURN_ON_BWSP_PHISHING;
    //    uIDLblTurnOff   = IDC_LBL_PROTECTION_TURN_OFF_BWSP_PHISHING;

    //    uIDTxtTurnOn    = IDC_TXT_PROTECTION_TURN_ON_BWSP_PHISHING;
    //    uIDTxtTurnOff   = IDC_TXT_PROTECTION_TURN_OFF_BWSP_PHISHING;

    //    uIDTxt			= IDC_TXT_PROTECTION_BWSP_PHISHING1;
    //    break;

	}

	if (bOn)
	{
		SAFE_CALL_FUN(this, SetItemIntAttribute(uIDImgStatus, "sub", 0));
		SAFE_CALL_FUN(this, SetItemText(uIDLblStatus, BkString::Get(IDS_PROTECTION_STATUS_ON)));
		SAFE_CALL_FUN(this, SetItemAttribute(uIDLblStatus, "class", "safetext"));
		SAFE_CALL_FUN(this, SetItemVisible(uIDLblTurnOff, TRUE));
		SAFE_CALL_FUN(this, SetItemVisible(uIDLblTurnOn, FALSE));

		SAFE_CALL_FUN(this, SetItemVisible(uIDTxtTurnOff, FALSE));
		SAFE_CALL_FUN(this, SetItemVisible(uIDTxtTurnOn, TRUE));

		SAFE_CALL_FUN(this, SetItemColorAttribute(uIDTxt, "crtext", RGB(0, 0, 0)));
		SAFE_CALL_FUN(this, SetItemColorAttribute(uIDTxt + 1, "crtext", RGB(0, 0, 0)));
	}
	else
	{

		SAFE_CALL_FUN(this, SetItemIntAttribute(uIDImgStatus, "sub", 1));
		SAFE_CALL_FUN(this, SetItemText(uIDLblStatus, BkString::Get(IDS_PROTECTION_STATUS_OFF)));
		SAFE_CALL_FUN(this, SetItemAttribute(uIDLblStatus, "class", "dangertext"));
		SAFE_CALL_FUN(this, SetItemVisible(uIDLblTurnOff, FALSE));
		SAFE_CALL_FUN(this, SetItemVisible(uIDLblTurnOn, TRUE));

		SAFE_CALL_FUN(this, SetItemVisible(uIDTxtTurnOff, TRUE));
		SAFE_CALL_FUN(this, SetItemVisible(uIDTxtTurnOn, FALSE));

		SAFE_CALL_FUN(this, SetItemColorAttribute(uIDTxt, "crtext", RGB(128, 128, 128)));
		SAFE_CALL_FUN(this, SetItemColorAttribute(uIDTxt + 1, "crtext", RGB(128, 128, 128)));
	}

	SAFE_CALL_FUN(m_pNotifyWnd, ModifyMonitorState( -1 ));

	return 0;
}

void CBKSafeMonitorDlg::OnLblProtectionTurnOffLeak()
{
	TrunOn(SM_ID_LEAK, FALSE);
}

void CBKSafeMonitorDlg::OnLblProtectionTurnOffRegister()
{
	TrunOn(SM_ID_RISK, FALSE);
}

void CBKSafeMonitorDlg::OnLblProtectionTurnOffProcess()
{
	TrunOn(SM_ID_PROCESS, FALSE);
}

void CBKSafeMonitorDlg::OnLblProtectionTurnOffUDisk()
{
	TrunOn(SM_ID_UDISK, FALSE);
}

void CBKSafeMonitorDlg::OnLblProtectionTurnOffBrowserProtect()
{
	TrunOn(SM_ID_KWS, FALSE);
}

void CBKSafeMonitorDlg::OnLblProtectionTurnOffBwspPhishing()
{
    //TrunOn(SM_ID_BWSP_FISHING, FALSE);
}

void CBKSafeMonitorDlg::OnLblProtectionTurnOnLeak()
{
	TrunOn(SM_ID_LEAK, TRUE);
}

void CBKSafeMonitorDlg::OnLblProtectionTurnOnRegister()
{
	TrunOn(SM_ID_RISK, TRUE);
}

//如果是64位系统体检发现时进程监控或者网页监控没有开启的话，就认为没有风险 
void CBKSafeMonitorDlg::OnLblProtectionTurnOnProcess()
{
	if (TRUE == _Module.Exam.IsWin64())
	{
		ShowPanelMsg(STR_ALERT_OPEN_SAFEMONITOR);
		return;
	}

	TrunOn(SM_ID_PROCESS, TRUE);
}
void CBKSafeMonitorDlg::OnLblProtectionTurnOnUDisk()
{
	TrunOn(SM_ID_UDISK, TRUE);
}

//如果是64位系统体检发现时进程监控或者网页监控没有开启的话，就认为没有风险 
void CBKSafeMonitorDlg::OnLblProtectionTurnOnBrowserProtect()
{
	if (TRUE == _Module.Exam.IsWin64())
	{
		ShowPanelMsg(STR_ALERT_OPEN_SAFEMONITOR);
		return;
	}

	KActivate Activate;
	if (Activate.ActivateKWS(NULL, FALSE, 3))
		TrunOn(SM_ID_KWS, TRUE);
}

//如果是64位系统体检发现时进程监控或者网页监控没有开启的话，就认为没有风险 
void CBKSafeMonitorDlg::OnLblProtectionTurnOnBwspPhishing()
{
	if (TRUE == _Module.Exam.IsWin64())
	{
		ShowPanelMsg(STR_ALERT_OPEN_SAFEMONITOR);
		return;
	}

	//TrunOn(SM_ID_BWSP_FISHING, TRUE);
}

void CBKSafeMonitorDlg::OnLblProtectionLogShow()
{
	CString strFileName, strLog;

	CAppPath::Instance().GetLeidianLogPath(strFileName);

	strFileName += L"\\ksfmon.log";

	::ShellExecute(NULL, L"open", L"notepad.exe", strFileName, NULL, SW_SHOWNORMAL);
}

void CBKSafeMonitorDlg::OnLblProtectionScan()
{
	SAFE_CALL_FUN(m_pNotifyWnd, PostMessage(MSG_APP_FULL_SCAN_TROJAN, 0, 0))

	EndDialog(0);
}

void CBKSafeMonitorDlg::OnLblProtectionTrunOnAll()
{
	m_wndProtectionNotify.TurnOnAll();
}

void CBKSafeMonitorDlg::OnLblProtectionSetting()
{
	m_wndProtectionNotify.TurnOnAll();
	//	_Module.Navigate(BKSFNS_MAKE_3(BKSFNS_UI, BKSFNS_SETTING, BKSFNS_SETTING_PAGE_COMMON));
}

void CBKSafeMonitorDlg::TrunOn(DWORD dwMonitorId, BOOL bOn)
{
	if ( !bOn )
	{
		if ( m_bWarningTrunOff )
		{
			BOOL bRember = FALSE;
			CString strText;
			LPCTSTR lpName = NULL;

			switch ( dwMonitorId )
			{
			case SM_ID_LEAK:
                lpName = BkString::Get(IDS_MONITOR_NAME_LEAK);
				break;

			case SM_ID_RISK:
				lpName = BkString::Get(IDS_MONITOR_NAME_RISK);
				break;

			case SM_ID_PROCESS:
				lpName = BkString::Get(IDS_MONITOR_NAME_PROCESS);
				break;

			case SM_ID_UDISK:
				lpName = BkString::Get(IDS_MONITOR_NAME_UDISK);
				break;

            case SM_ID_KWS:
                lpName = BkString::Get(IDS_MONITOR_NAME_KWS);
                break;

			default:
				assert(FALSE);
			}

			strText.Format(BkString::Get(IDS_MONITOR_TURN_OFF_NOTICE_FORMAT), lpName);
			UINT nID = CBkSafeMsgBox2::ShowMultLine(strText, NULL, MB_OKCANCEL | MB_ICONWARNING, &bRember);
			// 			if ( nID == IDCANCEL )
			// 			{
			// 				return ;
			// 			}

			if ( nID == IDOK )
			{
				if ( bRember )
				{
					m_bWarningTrunOff = FALSE;
					BKSafeConfig::Set_Protection_Trunoff_Warning(m_bWarningTrunOff);
				}
			}
			else
				return;
		}

		m_wndProtectionNotify.TurnOff(dwMonitorId);
	}
	else
	{
		m_wndProtectionNotify.TurnOn(dwMonitorId);
	}
}
UINT CBKSafeMonitorDlg::ShowPanelMsg(LPCTSTR pszCaption)
{
	CBkSafeMsgBox2 dlg;
	CString strXml;
	strXml.Format(L"<text pos=\"0,10\">%s</text>", pszCaption);
	CRect rc(0, 0, 0, 0);
//	dlg.AddButton(BkString::Get(18), IDOK);
	return dlg.ShowPanelMsg(strXml, &rc, NULL, MB_OK | MB_ICONINFORMATION, NULL, GetViewHWND());
}

BOOL CBKSafeMonitorDlg::OnBkTabMainSelChange(int nTabItemIDOld, int nTabItemIDNew)
{
    switch (nTabItemIDNew)
    {
    case 0:
        if (m_bPageNeverShow[0])
        {
            m_bPageNeverShow[0] = FALSE;
        }

        break;

    case 1:
        if (m_bPageNeverShow[1])
        {
            HANDLE hThread = ::CreateThread(NULL, 0, _LoadProtectionCtrlListThreadProc, m_hWnd, 0, NULL);
            ::CloseHandle(hThread);

            m_bPageNeverShow[1] = FALSE;
        }

        break;
    case 2:
        if (m_bPageNeverShow[2])
        {
            HANDLE hThread = ::CreateThread(NULL, 0, _LoadProtectionLogThreadProc, m_hWnd, 0, NULL);
            ::CloseHandle(hThread);

            m_bPageNeverShow[2] = FALSE;
        }

        break;
    default:
        break;
    }

    return TRUE;
}

LRESULT CBKSafeMonitorDlg::OnBkLvmGetDispInfoProtectionCtrl(LPNMHDR pnmh)
{
    LPBKLVMGETDISPINFO pnms = (LPBKLVMGETDISPINFO)pnmh;

    pnms->nListItemID;

    ATLTRACE(L"GetDispInfo(%d, %d, %d)\r\n", pnms->nListItemID, pnms->bHover, pnms->bSelect);

    if (m_arrProtectionCtrl[pnms->nListItemID].hIcon)
    {
        m_wndListMonitorCtrl.SetItemIconHandle(IDC_ICO_PROTCTRL_LISTITEM_ICON, m_arrProtectionCtrl[pnms->nListItemID].hIcon);
    }
    else
    {
        m_wndListMonitorCtrl.SetItemIconHandle(IDC_ICO_PROTCTRL_LISTITEM_ICON, m_hIconRegedit);
    }

    m_wndListMonitorCtrl.SetItemCheck(IDC_CHK_PROTCTRL_LISTITEM, m_arrProtectionCtrl[pnms->nListItemID].bCheck);
    m_wndListMonitorCtrl.SetItemColorAttribute(IDC_DIV_PROTCTRL_LISTITEM_BG, "crbg", pnms->bSelect ? RGB(0xCC, 0xCC, 0xFF) : (pnms->bHover ? RGB(0xCC, 0xFF, 0xCC) : RGB(0xFF, 0xFF, 0xFF)));

    m_wndListMonitorCtrl.SetItemVisible(IDC_DIV_PROTCTRL_LISTITEM_TRUST, !m_arrProtectionCtrl[pnms->nListItemID].bBlock);
    m_wndListMonitorCtrl.SetItemVisible(IDC_DIV_PROTCTRL_LISTITEM_DENY, m_arrProtectionCtrl[pnms->nListItemID].bBlock);
    m_wndListMonitorCtrl.SetItemText(IDC_LBL_PROTCTRL_LISTITEM_FILENAME, m_arrProtectionCtrl[pnms->nListItemID].strPath);
    m_wndListMonitorCtrl.SetItemStringAttribute(IDC_LBL_PROTCTRL_LISTITEM_FILENAME, "tip", m_arrProtectionCtrl[pnms->nListItemID].strPath);
    m_wndListMonitorCtrl.SetItemText(IDC_LBL_PROTCTRL_LISTITEM_DETAIL, m_arrProtectionCtrl[pnms->nListItemID].strDetail);
    m_wndListMonitorCtrl.SetItemStringAttribute(IDC_LBL_PROTCTRL_LISTITEM_DETAIL, "tip", m_arrProtectionCtrl[pnms->nListItemID].strDetail);

    return 0;
}

class _DeleteProtCtrlListParam
{
public:
    HWND hWndNotify;
    CAtlList<int> lstDelete;
};

LRESULT CBKSafeMonitorDlg::OnBkLvmItemClickProtectionCtrl(LPNMHDR pnmh)
{
    LPBKLVMITEMCLICK pnms = (LPBKLVMITEMCLICK)pnmh;

    pnms->nListItemID;

    CString strMsg;

    switch (pnms->uCmdID)
    {
    case IDC_BTN_PROTCTRL_LISTITEM_CANCEL_TRUST:
    case IDC_BTN_PROTCTRL_LISTITEM_CANCEL_DENY:
        {
            _DeleteProtCtrlListParam *pParam = new _DeleteProtCtrlListParam;
            pParam->hWndNotify = m_hWnd;
            pParam->lstDelete.AddTail(pnms->nListItemID);

            HANDLE hThread = ::CreateThread(NULL, 0, _DeleteProtectionCtrlListThreadProc, pParam, 0, NULL);
            ::CloseHandle(hThread);
        }
        break;
    case IDC_CHK_PROTCTRL_LISTITEM:
        m_arrProtectionCtrl[pnms->nListItemID].bCheck = m_wndListMonitorCtrl.GetItemCheck(IDC_CHK_PROTCTRL_LISTITEM);
        break;
    default:
        return 0;
    }

    return 0;
}


HICON _GetFileIconHandle(LPCTSTR lpFilePath)
{
    SHFILEINFO sfi = { 0 };

    DWORD_PTR dwRet = SHGetFileInfo(lpFilePath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);

    return SUCCEEDED(dwRet) ? sfi.hIcon: NULL;
}


DWORD WINAPI CBKSafeMonitorDlg::_LoadProtectionCtrlListThreadProc(LPVOID pvParam)
{
    CBKSafeMonitorDlg *pThis = (CBKSafeMonitorDlg *)pvParam;
    HWND hWndNotify = (HWND)pvParam;

    CSafeMonitorCtrlQuerierLoader ctrlLoader;
    ISafeMonitorCtrlQuerier *pMonitorCtrlQuerier = NULL;

    CAtlArray<_ProtectionCtrlArrayItem> arrData;

    pMonitorCtrlQuerier = ctrlLoader.NewSafeMonitorCtrlQuerier();
    if (pMonitorCtrlQuerier)
    {
        DWORD dwIndex = 0;
        SM_CTRL_ITEM itemret = {sizeof(SM_CTRL_ITEM)};
        _ProtectionCtrlArrayItem item;

        pMonitorCtrlQuerier->Init();

        while (pMonitorCtrlQuerier->Enum(dwIndex, &itemret))
        {
            item.bCheck = FALSE;
            item.bBlock = IS_SM_CTRL_BLOCK(itemret.dwCtrl);
            item.strPath = itemret.lpFilePath ? itemret.lpFilePath : itemret.lpPath;
            item.strDetail = itemret.lpVirusName ? itemret.lpVirusName : L"";
            item.hIcon = itemret.lpFilePath ? _GetFileIconHandle(itemret.lpFilePath) : NULL;
            
            int i = 0;

            while (TRUE)
            {
                WCHAR ch = item.strPath[i];
                if (L'\\' == ch || L'\0' == ch)
                    break;

                if (ch >= L'a' && ch <= L'z')
                    item.strPath.SetAt(i, ch - L'a' + L'A');

                i ++;
            }

            arrData.Add(item);

            item.hIcon = NULL;

            dwIndex ++;
        }

        pMonitorCtrlQuerier->Uninit();
        ctrlLoader.FreeSafeMonitorCtrlQuerier(&pMonitorCtrlQuerier);
    }

    if (hWndNotify && ::IsWindow(hWndNotify))
        ::SendMessage(hWndNotify, MSG_APP_PROTECTION_CTRL_LOAD_FINISH, (WPARAM)&arrData, 0);

    return 0;
}

LRESULT CBKSafeMonitorDlg::OnAppProtectionCtrlLoadFinish(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
    CAtlArray<_ProtectionCtrlArrayItem> *parrData = (CAtlArray<_ProtectionCtrlArrayItem> *)wParam;

    m_arrProtectionCtrl.Copy(*parrData);

    size_t nCount = m_arrIconTemp.GetCount();

    for (size_t i = 0; i < nCount; i ++)
    {
        if (m_arrIconTemp[i])
            ::DestroyIcon(m_arrIconTemp[i]);
    }

    nCount = m_arrProtectionCtrl.GetCount();

    m_arrIconTemp.SetCount(nCount);

    for (size_t i = 0; i < nCount; i ++)
    {
        m_arrIconTemp[i] = m_arrProtectionCtrl[i].hIcon;
    }

    m_wndListMonitorCtrl.SetItemCount(nCount);

    SetItemVisible(IDC_LST_PROTECTION_CTRL, TRUE);
    FormatItemText(IDC_LBL_PROTECTION_CTRL_COUNT, BkString::Get(IDS_PROTECTION_CTRL_COUNT_FORMAT), m_arrProtectionCtrl.GetCount());

    EnableItem(IDC_TXT_PROTECTION_CTRL_CHECK, nCount != 0);
    EnableItem(IDC_BTN_PROTECTION_CTRL_REMOVE_CHECK, nCount != 0);

    return 0;
}

DWORD WINAPI CBKSafeMonitorDlg::_DeleteProtectionCtrlListThreadProc(LPVOID pvParam)
{
    _DeleteProtCtrlListParam *pParam = (_DeleteProtCtrlListParam *)pvParam;

    POSITION pos = pParam->lstDelete.GetHeadPosition();

    CSafeMonitorCtrlQuerierLoader ctrlLoader;
    ISafeMonitorCtrlQuerier *pMonitorCtrlQuerier = NULL;

    pMonitorCtrlQuerier = ctrlLoader.NewSafeMonitorCtrlQuerier();
    if (pMonitorCtrlQuerier)
    {
        pMonitorCtrlQuerier->Init();

        while (pos)
        {
            int nItemID = pParam->lstDelete.GetNext(pos);

            pMonitorCtrlQuerier->Delete((DWORD)nItemID);
        }

        pMonitorCtrlQuerier->Save();

        pMonitorCtrlQuerier->Uninit();
        ctrlLoader.FreeSafeMonitorCtrlQuerier(&pMonitorCtrlQuerier);

        CSafeMonitorTrayShell::NotifySafeMonitorCtrlUpdated();
    }

    _LoadProtectionCtrlListThreadProc((LPVOID)(pParam->hWndNotify));

    delete pParam;

    return 0;
}

void CBKSafeMonitorDlg::OnLblProtectionCtrlCheckAll()
{
    for (int i = m_arrProtectionCtrl.GetCount() - 1; i >= 0 ; i --)
    {
        if (!m_arrProtectionCtrl[i].bCheck)
        {
            m_arrProtectionCtrl[i].bCheck = TRUE;
            m_wndListMonitorCtrl.RedrawItem(i);
        }
    }
}

void CBKSafeMonitorDlg::OnLblProtectionCtrlUncheckAll()
{
    for (int i = m_arrProtectionCtrl.GetCount() - 1; i >= 0 ; i --)
    {
        if (m_arrProtectionCtrl[i].bCheck)
        {
            m_arrProtectionCtrl[i].bCheck = FALSE;
            m_wndListMonitorCtrl.RedrawItem(i);
        }
    }
}

void CBKSafeMonitorDlg::OnLblProtectionCtrlRemoveCheck()
{
    _DeleteProtCtrlListParam *pParam = new _DeleteProtCtrlListParam;
    pParam->hWndNotify = m_hWnd;

    for (int i = m_arrProtectionCtrl.GetCount() - 1; i >= 0 ; i --)
    {
        if (m_arrProtectionCtrl[i].bCheck)
            pParam->lstDelete.AddTail(i);
    }

    HANDLE hThread = ::CreateThread(NULL, 0, _DeleteProtectionCtrlListThreadProc, pParam, 0, NULL);
    ::CloseHandle(hThread);
}

DWORD WINAPI CBKSafeMonitorDlg::_LoadProtectionLogThreadProc(LPVOID pvParam)
{
    HWND hWndNotify = (HWND)pvParam;

    _LoadLog(hWndNotify);

    return 0;
}

void CBKSafeMonitorDlg::_AppendLogToArray(
    LPCSTR szLine,
    CAtlArray<_ProtectionCtrlLogArrayItem>& arrLog
    )
{
    CString strLineW = CA2W(szLine);
    _ProtectionCtrlLogArrayItem item;

    int nTypeSize = strLineW.Find(L']', 1) - 1;
    if (nTypeSize < 0)
        return;

    int nOpSize = strLineW.Find(L']', nTypeSize + 24) - nTypeSize - 24;
    if (nOpSize < 0)
        return;

    item.strType = strLineW.Mid(1, nTypeSize);
    item.strTime = strLineW.Mid(nTypeSize + 3, 16);
    item.strOp = strLineW.Mid(nTypeSize + 24, nOpSize);

    int nFileNameSize = strLineW.Find(L']', nTypeSize + nOpSize + 26) - nTypeSize - nOpSize - 26;
    if (nFileNameSize < 0)
        return;

    item.strFileName = strLineW.Mid(nTypeSize + nOpSize + 26, nFileNameSize);

    int nDetailSize = strLineW.Find(L']', nTypeSize + nOpSize + nFileNameSize + 28) - nTypeSize - nOpSize - nFileNameSize - 28;
    if (nDetailSize < 0)
        return;

    item.strDetail = strLineW.Mid(nTypeSize + nOpSize + nFileNameSize + 28, nDetailSize);

    //arrLog.Add(item);
	arrLog.InsertAt(0, item);
}

void CBKSafeMonitorDlg::_LoadLog(HWND hWndNotify)
{
	USE_CHINESE_LANG_LOCAL;

    CAtlFile hMonitorLog;
    CString strFileName, strLog;
    CAtlArray<_ProtectionCtrlLogArrayItem> arrLog;

    CSafeMonitorTrayShell tray;

    DWORD dwCount = 0;

    CAppPath::Instance().GetLeidianLogPath(strFileName);

    strFileName += L"\\ksfmon.log";

    HRESULT hRet = hMonitorLog.Create(strFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING);
    if (SUCCEEDED(hRet))
    {
        ULONGLONG ullRemainSize = 0;
        hMonitorLog.GetSize(ullRemainSize);

        CStringA strRead, strLine, strLastLineRemain;

        int nLineNum = 0;

        while (0 != ullRemainSize)
        {
            DWORD dwRead = min(100, (DWORD)ullRemainSize);

            hRet = hMonitorLog.Read(strRead.GetBuffer(dwRead + 1), dwRead);
            if (FAILED(hRet))
            {
                strRead.ReleaseBuffer(0);
                break;
            }

            strRead.ReleaseBuffer(dwRead);

            int nPosNextLine = -1, nPosThisLine = 0;
            DWORD dwCount = 0;
            
            while (TRUE)
            {
                nPosNextLine = strRead.Find('\n', nPosThisLine);

                if (-1 == nPosNextLine)
                {
                    strLastLineRemain += strRead.Mid(nPosThisLine);
                    break;
                }

                strLine = strLastLineRemain + strRead.Mid(nPosThisLine, nPosNextLine - nPosThisLine);
                strLastLineRemain.Empty();

                nLineNum ++;

                if (1 == nLineNum)
                {
                    sscanf(strLine, "block_count=%d", &dwCount);
                    goto _TagNextLine;
                }
                else if (2 == nLineNum)
                    goto _TagNextLine;

                _AppendLogToArray(strLine, arrLog);


            _TagNextLine:

                nPosThisLine = nPosNextLine + 1;
            }

            ullRemainSize -= dwRead;
        }

        if (0 == ullRemainSize)
        {
            _AppendLogToArray(strLastLineRemain, arrLog);
        }
    }

    if (hWndNotify && ::IsWindow(hWndNotify))
        ::SendMessage(hWndNotify, MSG_APP_PROTECTION_LOG_LOAD_FINISH, (WPARAM)&arrLog, 0);
}

LRESULT CBKSafeMonitorDlg::OnAppProtectionLogLoadFinish(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
    CAtlArray<_ProtectionCtrlLogArrayItem> *parrData = (CAtlArray<_ProtectionCtrlLogArrayItem> *)wParam;

    m_arrProtectionCtrlLog.Copy(*parrData);

    m_wndListMonitorLog.SetItemCount(m_arrProtectionCtrlLog.GetCount());

    SetItemVisible(IDC_DIV_PROTECTION_CTRL_LOG, TRUE);

    return 0;
}

LRESULT CBKSafeMonitorDlg::OnBkLvmGetDispInfoProtectionLog(LPNMHDR pnmh)
{
    LPBKLVMGETDISPINFO pnms = (LPBKLVMGETDISPINFO)pnmh;

    pnms->nListItemID;

    m_wndListMonitorLog.SetItemText(IDC_LBL_PROTLOG_LISTITEM_TIME, m_arrProtectionCtrlLog[pnms->nListItemID].strTime);
    m_wndListMonitorLog.SetItemText(IDC_LBL_PROTLOG_LISTITEM_TYPE, m_arrProtectionCtrlLog[pnms->nListItemID].strType);
    m_wndListMonitorLog.SetItemText(IDC_LBL_PROTLOG_LISTITEM_OP, m_arrProtectionCtrlLog[pnms->nListItemID].strOp);
    m_wndListMonitorLog.SetItemText(IDC_LBL_PROTLOG_LISTITEM_FILENAME, m_arrProtectionCtrlLog[pnms->nListItemID].strFileName);
    m_wndListMonitorLog.SetItemStringAttribute(IDC_LBL_PROTLOG_LISTITEM_FILENAME, "tip", m_arrProtectionCtrlLog[pnms->nListItemID].strFileName);
    m_wndListMonitorLog.SetItemText(IDC_LBL_PROTLOG_LISTITEM_DETAIL, m_arrProtectionCtrlLog[pnms->nListItemID].strDetail);
    m_wndListMonitorLog.SetItemStringAttribute(IDC_LBL_PROTLOG_LISTITEM_DETAIL, "tip", m_arrProtectionCtrlLog[pnms->nListItemID].strDetail);

    return 0;
}

LRESULT CBKSafeMonitorDlg::OnBkLvmItemClickProtectionLog(LPNMHDR pnmh)
{
    LPBKLVMITEMCLICK pnms = (LPBKLVMITEMCLICK)pnmh;

    return 0;
}

void CBKSafeMonitorDlg::OnLblProtectionCtrlLogClear()
{
    HANDLE hThread = ::CreateThread(NULL, 0, _ClearProtectionLogThreadProc, m_hWnd, 0, NULL);
    ::CloseHandle(hThread);
}

DWORD WINAPI CBKSafeMonitorDlg::_ClearProtectionLogThreadProc(LPVOID pvParam)
{
    HWND hWndNotify = (HWND)pvParam;
    CString strFileName;

    CAppPath::Instance().GetLeidianLogPath(strFileName);

    strFileName += L"\\ksfmon.log";

    for (int i = 0; i < 3; i ++)
    {
        if (::DeleteFile(strFileName))
            break;

        ::Sleep(100);
    }

    _LoadLog(hWndNotify);

    return 0;
}

void CBKSafeMonitorDlg::OnLblProtectionBwspMoreSetting()
{
	CInstallStc::GetGlobalPtr()->ReportOpenKWS(_Module.GetProductVersion(), 2);
	KActivate Activate;
	Activate.ActivateKWS(_T("/enter ksafe"), TRUE, 3);

    //m_bShowBwspSetting = TRUE;
    //EndDialog(IDOK);
}

CBkNavigator* CBKSafeMonitorNavigator::OnNavigate(CString &strChildName)
{
	int nPage = MonitorSettingPageCommon;

	if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_PROTECTION_COMMON))
	{
		nPage = MonitorSettingPageCommon;
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_PROTECTION_BANDT))
	{
		nPage = MonitorSettingPageBlockAndTrust;
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_PROTECTION_HISTORY))
	{
		nPage = MonitorSettingPageHistory;
	}
	else if (0 == strChildName.CompareNoCase(BKSFNS_PAGE_IEFIX))
	{


		return NULL;
	}

	DoModal(nPage);

	return NULL;
}

UINT_PTR CBKSafeMonitorNavigator::DoModal(int nPage/* = MonitorSettingPageCommon*/, HWND hWndParent/* = ::GetActiveWindow()*/)
{
	CBKSafeMonitorDlg dlg(m_pDlg);

	UINT_PTR uRet = dlg.DoModal(nPage, hWndParent);

	m_pDlg->ModifyMonitorState(0);

	//if (dlg.NeedShowBwspSetting())
	//	Navigate(BKSFNS_MAKE(BKSFNS_PAGE_IEFIX, BKSFNS_PAGE_BWS));

	return uRet;
}