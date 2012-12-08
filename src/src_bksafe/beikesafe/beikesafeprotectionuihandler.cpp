#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "beikesafeprotectionuihandler.h"
#include <common/utility.h>

CBeikeSafeProtectionUIHandler::~CBeikeSafeProtectionUIHandler()
{
	m_bWarningTrunOff = TRUE;
    m_wndProtectionNotify.DestroyWindow();
}

void CBeikeSafeProtectionUIHandler::Init()
{
    m_wndProtectionNotify.Create(m_pDlg->m_hWnd, MSG_APP_PROTECTION_SWITCH_CHANGE);
    
	m_bWarningTrunOff = BKSafeConfig::Get_Protection_Trunoff_Warning();
    try
    {
  //      _LoadLog();
    }
    catch (...)
    {
    }
}
/*
void CBeikeSafeProtectionUIHandler::_LoadLog()
{
    CAtlFile hMonitorLog;
    CString strFileName, strLog;

    CSafeMonitorTrayShell tray;

    DWORD dwCount = 0;

    CAppPath::Instance().GetLeidianLogPath(strFileName);

    strFileName += L"\\ksfmon.log";

    HRESULT hRet = hMonitorLog.Create(strFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING);
    if (SUCCEEDED(hRet))
    {
        ULONGLONG ullSize = 0;
        hMonitorLog.GetSize(ullSize);

        if (0 != ullSize)
        {
            DWORD dwRead = 10 * 1024;
            BOOL bCut = ullSize > dwRead;

            if (bCut)
                hMonitorLog.Seek(ullSize - dwRead);
            else
                dwRead = (DWORD)ullSize;

            LPBYTE byBuffer = new BYTE[dwRead + 1];

            if (byBuffer)
            {
                hRet = hMonitorLog.Read(byBuffer, dwRead);
                if (SUCCEEDED(hRet))
                {
                    byBuffer[dwRead] = 0;

                    char *psz = (char *)byBuffer;
                    if (bCut)
                    {
                        while ('\n' != *psz)
                            psz ++;

                        psz ++;
                    }

					sscanf(psz, "block_count=%d", &dwCount);

					char* pszContent = strchr(psz, '\n');
					if ( pszContent != NULL )
					{
						strLog = CA2W(pszContent);
					}
                }

                delete[] byBuffer;
            }
        }
    }

    int nStart = 0;

    CAtlList<CString> listLines;

    while (TRUE)
    {
        CString strLine = strLog.Tokenize(L"\r\n", nStart);
        if (-1 == nStart)
            break;

        if (strLine.GetLength() < 35)
            continue;

        listLines.AddTail(strLine);

        if (listLines.GetCount() == 7 + 2)
            listLines.RemoveHead();
    }

    struct _LogItemID 
    {
        UINT uTimeID;
        UINT uTypeID;
        UINT uOpID;
    };

    _LogItemID items[] = {
        { IDC_LBL_PROTECTION_LOG_TIME_0, IDC_LBL_PROTECTION_LOG_TYPE_0, IDC_LBL_PROTECTION_LOG_OP_0 },
        { IDC_LBL_PROTECTION_LOG_TIME_1, IDC_LBL_PROTECTION_LOG_TYPE_1, IDC_LBL_PROTECTION_LOG_OP_1 },
        { IDC_LBL_PROTECTION_LOG_TIME_2, IDC_LBL_PROTECTION_LOG_TYPE_2, IDC_LBL_PROTECTION_LOG_OP_2 },
        { IDC_LBL_PROTECTION_LOG_TIME_3, IDC_LBL_PROTECTION_LOG_TYPE_3, IDC_LBL_PROTECTION_LOG_OP_3 },
        { IDC_LBL_PROTECTION_LOG_TIME_4, IDC_LBL_PROTECTION_LOG_TYPE_4, IDC_LBL_PROTECTION_LOG_OP_4 },
        { IDC_LBL_PROTECTION_LOG_TIME_5, IDC_LBL_PROTECTION_LOG_TYPE_5, IDC_LBL_PROTECTION_LOG_OP_5 },
        { IDC_LBL_PROTECTION_LOG_TIME_6, IDC_LBL_PROTECTION_LOG_TYPE_6, IDC_LBL_PROTECTION_LOG_OP_6 },
    };

    POSITION pos = listLines.GetTailPosition();
    int i = 0;

    while (pos)
    {
		// [风险拦截][2010-02-25 17:55 51][放行][http://xxxx222xxxxx222/][浏览器首页被修改！]
        const CString& strLine = listLines.GetPrev(pos);

        int nTypeSize = strLine.Find(L']', 1) - 1;
        if (nTypeSize < 0)
            continue;

        int nOpSize = strLine.Find(L']', nTypeSize + 24) - nTypeSize - 24;
        if (nOpSize < 0)
            continue;

        CString strType = strLine.Mid(1, nTypeSize);
        CString strTime = strLine.Mid(nTypeSize + 3, 16);
        CString strOp = strLine.Mid(nTypeSize + 24, nOpSize);

        m_pDlg->SetItemText(items[i].uTimeID, strTime, FALSE);
        m_pDlg->SetItemText(items[i].uTypeID, strType, FALSE);
        m_pDlg->SetItemText(items[i].uOpID, strOp, FALSE);

        i ++;
    }

	if ( dwCount == 0 )
	{
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_LOG_TIP1, TRUE, FALSE);
		m_pDlg->SetItemText(IDC_LBL_PROTECTION_LOG_COUNT, _T("金山安全实时保护尚未拦截到风险"), FALSE);
	}
	else
	{
		CString strText;

		strText.Format(_T("已拦截风险 %d 次"), dwCount);
		m_pDlg->SetItemText(IDC_LBL_PROTECTION_LOG_COUNT, strText, FALSE);

		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_LOG_TIP2, TRUE, FALSE);
	}

    m_pDlg->SetItemVisible(IDC_LBL_PROTECTION_LOG_SHOW, i != 0, FALSE);
}
*/

LRESULT CBeikeSafeProtectionUIHandler::OnAppProtectionSwitchChange(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
    BOOL bOn = (BOOL)lParam;
    UINT uIDLblStatus = 0, uIDImgStatus = 0, uIDLblTurnOn = 0, uIDLblTurnOff = 0;
	UINT uIDTxtTurnOn = 0, uIDTxtTurnOff = 0;
	UINT uIDTxt = 0;

	switch ( m_wndProtectionNotify.GetMonitorStatus() )
	{
	case 0:
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_OK, TRUE);
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_WARNING1, FALSE);
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_WARNING2, FALSE);
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_WARNING3, FALSE);
		m_pDlg->SetItemIntAttribute(IDC_IMG_PROTECTION_STATUS, "sub", 2);
		break;

	case 1:
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_OK, FALSE);
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_WARNING1, TRUE);
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_WARNING2, FALSE);
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_WARNING3, FALSE);
		m_pDlg->SetItemIntAttribute(IDC_IMG_PROTECTION_STATUS, "sub", 1);
		break;

	case 2:
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_OK, FALSE);
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_WARNING1, FALSE);
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_WARNING2, TRUE);
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_WARNING3, FALSE);
		m_pDlg->SetItemIntAttribute(IDC_IMG_PROTECTION_STATUS, "sub", 1);
		break;

	case 3:
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_OK, FALSE);
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_WARNING1, FALSE);
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_WARNING2, FALSE);
		m_pDlg->SetItemVisible(IDC_DIV_PROTECTION_WARNING3, TRUE);
		m_pDlg->SetItemIntAttribute(IDC_IMG_PROTECTION_STATUS, "sub", 0);
		break;
	}

    switch (wParam)
    {
    case SM_ID_INVAILD:
        m_pDlg->RemoveFromTodoList(BkSafeExamItem::ConfigSystemMonitor);
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

    }

    if (bOn)
    {
        m_pDlg->SetItemIntAttribute(uIDImgStatus, "sub", 0);
        m_pDlg->SetItemText(uIDLblStatus, BkString::Get(IDS_PROTECTION_STATUS_ON));
        m_pDlg->SetItemAttribute(uIDLblStatus, "class", "safetext");
        m_pDlg->SetItemVisible(uIDLblTurnOff, TRUE);
        m_pDlg->SetItemVisible(uIDLblTurnOn, FALSE);

		m_pDlg->SetItemVisible(uIDTxtTurnOff, FALSE);
		m_pDlg->SetItemVisible(uIDTxtTurnOn, TRUE);

		m_pDlg->SetItemColorAttribute(uIDTxt, "crtext", RGB(0, 0, 0));
		m_pDlg->SetItemColorAttribute(uIDTxt + 1, "crtext", RGB(0, 0, 0));
    }
    else
    {
        m_pDlg->SetItemIntAttribute(uIDImgStatus, "sub", 1);
        m_pDlg->SetItemText(uIDLblStatus, BkString::Get(IDS_PROTECTION_STATUS_OFF));
        m_pDlg->SetItemAttribute(uIDLblStatus, "class", "dangertext");
        m_pDlg->SetItemVisible(uIDLblTurnOff, FALSE);
        m_pDlg->SetItemVisible(uIDLblTurnOn, TRUE);

		m_pDlg->SetItemVisible(uIDTxtTurnOff, TRUE);
		m_pDlg->SetItemVisible(uIDTxtTurnOn, FALSE);

		m_pDlg->SetItemColorAttribute(uIDTxt, "crtext", RGB(128, 128, 128));
		m_pDlg->SetItemColorAttribute(uIDTxt + 1, "crtext", RGB(128, 128, 128));
    }

	m_pDlg->ModifyMonitorState( -1 );
    return 0;
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionTurnOffLeak()
{
	TrunOn(SM_ID_LEAK, FALSE);
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionTurnOffRegister()
{
	TrunOn(SM_ID_RISK, FALSE);
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionTurnOffProcess()
{
	TrunOn(SM_ID_PROCESS, FALSE);
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionTurnOffUDisk()
{
	TrunOn(SM_ID_UDISK, FALSE);
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionTurnOffBrowserProtect()
{
	TrunOn(SM_ID_KWS, FALSE);
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionTurnOnLeak()
{
	TrunOn(SM_ID_LEAK, TRUE);
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionTurnOnRegister()
{
	TrunOn(SM_ID_RISK, TRUE);
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionTurnOnProcess()
{
	TrunOn(SM_ID_PROCESS, TRUE);
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionTurnOnUDisk()
{
	TrunOn(SM_ID_UDISK, TRUE);
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionTurnOnBrowserProtect()
{
	TrunOn(SM_ID_KWS, TRUE);
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionLogShow()
{
    CString strFileName, strLog;

    CAppPath::Instance().GetLeidianLogPath(strFileName);

    strFileName += L"\\ksfmon.log";

    ::ShellExecute(NULL, L"open", L"notepad.exe", strFileName, NULL, SW_SHOWNORMAL);
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionScan()
{
	_Module.Navigate(BKSFNS_MAKE(BKSFNS_UI, BKSFNS_PAGE_VIRSCAN));
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionTrunOnAll()
{
	m_wndProtectionNotify.TurnOnAll();
}

void CBeikeSafeProtectionUIHandler::OnLblProtectionSetting()
{
	m_wndProtectionNotify.TurnOnAll();
//	_Module.Navigate(BKSFNS_MAKE_3(BKSFNS_UI, BKSFNS_SETTING, BKSFNS_SETTING_PAGE_COMMON));
}

void CBeikeSafeProtectionUIHandler::TrunOn(DWORD dwMonitorId, BOOL bOn)
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
				lpName = _T("漏洞防护");
				break;

			case SM_ID_RISK:
				lpName = _T("注册表防护");
				break;

			case SM_ID_PROCESS:
				lpName = _T("进程防护");
				break;

			case SM_ID_UDISK:
				lpName = _T("U 盘防护");
				break;

			case SM_ID_KWS:
				lpName = _T("金山网盾网页安全防护");
				break;

			default:
				assert(FALSE);
			}

			strText.Format(_T("关闭“%s”，会降低实时保护效果，给系统安全带来风险。\r\n\r\n确定要关闭吗？"), lpName);
			UINT nID = CBkSafeMsgBox::Show(strText, _T("提示"), MB_OKCANCEL | MB_ICONWARNING, &bRember);
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