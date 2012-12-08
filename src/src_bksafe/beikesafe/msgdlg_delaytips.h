#pragma once

#include "ignoredlist.h"
#include <iefix/cinifile.h>
using namespace IniFileOperate;

#define CONFIG_FILEPATH			L"\\Cfg\\bksafe.ini"
#define CONFIG_SEC_NAME			L"runopt"
#define CONFIG_DONOT_SHOW		L"notshowdelaytip"

class CMsgBoxDelayTip
	: public CBkDialogImpl<CMsgBoxDelayTip>
	, public CWHRoundRectFrameHelper<CMsgBoxDelayTip>
{
public:
	CMsgBoxDelayTip()
		: CBkDialogImpl<CMsgBoxDelayTip>(IDR_BK_RUNOPT_DELAY_TIP)
	{
	}

	~CMsgBoxDelayTip(void)
	{
	}

	BOOL OnInitDialog(HWND,LPARAM)
	{
		DWORD dwStyle = ::GetWindowLong(m_hWnd,GWL_EXSTYLE);
		dwStyle |= WS_EX_TOOLWINDOW;
		dwStyle &= ~WS_EX_APPWINDOW;
		::ShowWindow(m_hWnd, FALSE);
		::SetWindowLong(m_hWnd,GWL_EXSTYLE, dwStyle);
		::ShowWindow(m_hWnd, TRUE);
		CAppPath::Instance().GetLeidianAppPath(m_strIniPath);
		m_strIniPath.Append(CONFIG_FILEPATH);
		return TRUE;
	}

	void OnBkBtnOK()
	{
		BOOL bChecked = GetItemCheck(IDC_CHECK_NOT_SHOW_AGAIN);
		CIniFile iniFile(m_strIniPath);
		iniFile.SetIntValue(CONFIG_SEC_NAME,CONFIG_DONOT_SHOW,bChecked);
		EndDialog(IDOK);
	}

private:
	CString m_strIniPath;

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnOK)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_CLOSE_CTRL2, OnBkBtnOK)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CMsgBoxDelayTip)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CMsgBoxDelayTip>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CMsgBoxDelayTip>)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()
};