#pragma once

class CBKSafeNetMonForbidDlg
	: public CBkDialogImpl<CBKSafeNetMonForbidDlg>
	, public CWHRoundRectFrameHelper<CBKSafeNetMonForbidDlg>
{
public:
	CBKSafeNetMonForbidDlg()
		: CBkDialogImpl<CBKSafeNetMonForbidDlg>(IDR_BK_MESSAGEBOX5)
	{}
	CBKSafeNetMonForbidDlg(HWND hNotifyWnd, DWORD dwNotifyMsg, CNetMonitorCacheData& NetData)
		: CBkDialogImpl<CBKSafeNetMonForbidDlg>(IDR_BK_MESSAGEBOX5)
	{
		m_dwNotifyMsg = dwNotifyMsg;
		m_hWndNotify = hNotifyWnd;
		m_NetMonitorData = NetData;
	}

	~CBKSafeNetMonForbidDlg(void){};

	virtual void OnBkClose()
	{
		EndDialog(0);
	}

	virtual void OnOk()
	{
		if (TRUE == ::IsWindow(m_hWndNotify))
		{

			m_NetMonitorData.GetProcNetData().m_nDisable = 1;

			if (TRUE == GetItemCheck(IDC_CHECK_REMEMBER_FORBID))
			{
				m_NetMonitorData.SetRemember(TRUE);
			}

			::SendMessage(m_hWndNotify, m_dwNotifyMsg, (WPARAM)&m_NetMonitorData, 0);
		}
		EndDialog(0);
	}

	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{
		KNetFlowMonListItemData Data(m_NetMonitorData.GetProcNetData());
		CString strInfo;
		strInfo.Format(BkString::Get(DefString60), Data.GetProcessName());
		SetRichText(IDC_TXT_FORBID_INFO, strInfo);
		return TRUE;
	}

protected:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_FORBID_DLG_CLOSE, OnBkClose)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_FORBID_DLG_CANCEL, OnBkClose)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_FORBID_DLG_OK, OnOk)
		BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CBKSafeNetMonForbidDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBKSafeNetMonForbidDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBKSafeNetMonForbidDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
public:
	HWND		m_hWndNotify;
	DWORD		m_dwNotifyMsg;
	CNetMonitorCacheData	m_NetMonitorData;
};
