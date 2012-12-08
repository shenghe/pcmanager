#pragma once

#include <wtlhelper/whwindow.h>
#include <common/utility.h>
#include "bkwinres.h"
#include "beikenetmonitorlimitedit.h"
#include "netmonitorutility.h"


class CBKSafeNetMonitorLimit
	: public CBkDialogImpl<CBKSafeNetMonitorLimit>
	, public CWHRoundRectFrameHelper<CBKSafeNetMonitorLimit>
	, public INetMonitorLimitEditCallBack

{
public:
	CBKSafeNetMonitorLimit()
		: CBkDialogImpl<CBKSafeNetMonitorLimit>(IDR_BK_DLG_NETMONITOR_LIMIT)
	{}
	CBKSafeNetMonitorLimit(HWND hNotifyWnd, DWORD dwNotifyMsg, CNetMonitorCacheData& NetData)
		: CBkDialogImpl<CBKSafeNetMonitorLimit>(IDR_BK_DLG_NETMONITOR_LIMIT)
	{
		m_dwNotifyMsg = dwNotifyMsg;
		m_hWndNotify = hNotifyWnd;
		m_NetMonitorData = NetData;
		m_RectProcName.SetRectEmpty();
	}
	~CBKSafeNetMonitorLimit(void){};

	virtual void OnBkClose()
	{
		EndDialog(0);
	}

	virtual void OnOk()
	{
		if (TRUE == ::IsWindow(m_hWndNotify))
		{
			CString strValue = L"";
			m_EditKB.GetWindowText(strValue);
//			if (FALSE == strValue.IsEmpty())
			{
				int nValue = ::StrToInt(strValue);
				if (nValue < 0 ) nValue = 0;
				if (0 != nValue)
				{
					m_NetMonitorData.GetProcNetData().m_nSendLimit = nValue * 1024;

					if (TRUE == GetItemCheck(NET_MONITOR_250))
					{
						//::MessageBox(NULL, L"Limit", NULL, 0);
						m_NetMonitorData.SetRemember(TRUE);
					}
					
					::SendMessage(m_hWndNotify, m_dwNotifyMsg, (WPARAM)&m_NetMonitorData, 0);
				}
				else
				{
					SetItemText(NET_MONITOR_242, BkString::Get(NET_MONITOR_STR_510));
					SetItemAttribute(NET_MONITOR_242, "crtext", "c00000");
					SetItemAttribute(NET_MONITOR_242, "font", "0200");
					return;
				}
			}
		}
		EndDialog(0);
	}
	virtual void OnEditEnterMsg( DWORD nEditId )
	{
		int a = 1;
	}

	virtual void OnEditMouseWheelMsg(WPARAM wParam, LPARAM lParam)
	{
		int a = 10;
	}

	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{
		//

		CString strValue = L"";
		m_EditKB.Create(GetViewHWND(), NULL, NULL, WS_CHILD | WS_TABSTOP, 0/*ES_NOHIDESEL | ES_AUTOHSCROLL | ES_NUMBER*/, NET_MONITOR_236);
		m_EditKB.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
		m_EditKB.SetCallBack(this);
		m_EditKB.SetOnCharDelayTime();
		m_EditKB.LimitText(3);
		m_EditKB.SetFocus();
		m_EditKB.ModifyStyle(ES_LEFT, ES_RIGHT | ES_NUMBER);

		int nLeft = 140, nWidth = 0;
		HWND hWnd = ::GetDesktopWindow();
		::GetWindowRect(hWnd, &m_RectProcName);
		CDC dc = ::GetDC(hWnd);
		HFONT hFntTmp = dc.SelectFont(BkFontPool::GetFont(BKF_BOLDFONT));
	
		//proc name
		KNetFlowMonListItemData Data(m_NetMonitorData.GetProcNetData());

		//上次是否限速
		if (Data.GetSendLimitVaule() > 0)
		{
			CString strValue;
			strValue.Format(L"%d", Data.GetSendLimitVaule());
			m_EditKB.SetWindowText(strValue);
		}

		dc.DrawText(Data.GetProcessName(), -1, &m_RectProcName, DT_VCENTER | DT_SINGLELINE |DT_CALCRECT);
		CStringA strPosA = "";
		//m_RectProcName.right += 6;
		m_RectProcName.right += (Data.GetProcessName().GetLength() / 6 + 0.5) *10;
		//确保不能超出边界
		if (m_RectProcName.right > 174)
			m_RectProcName.right = 174;
		strPosA.Format("140,43,%d,68", nLeft+m_RectProcName.Width());
		SetItemAttribute(NET_MONITOR_234, "pos", strPosA);
		SetItemAttribute(NET_MONITOR_234, "class", "filename");
		SetItemAttribute(NET_MONITOR_234, "crtext", "C80000");
		SetItemAttribute(NET_MONITOR_234, "font", "0204");
		SetItemText(NET_MONITOR_234, Data.GetProcessName());
		nLeft += m_RectProcName.Width();
		//的上传速度为
		//dc.DrawText(BkString::Get(NET_MONITOR_STR_502), -1, &m_RectProcName, DT_VCENTER | DT_SINGLELINE |DT_CALCRECT);
		//m_RectProcName.right += 12;
		nWidth = (_tcslen(BkString::Get(NET_MONITOR_STR_502))+2) * 12;
		strPosA.Format("%d,35,%d,60", nLeft, nLeft + nWidth);
		SetItemAttribute(NET_MONITOR_235, "pos", strPosA);
		SetItemText(NET_MONITOR_235, BkString::Get(NET_MONITOR_STR_502));
		nLeft += nWidth;
		//realwnd Edit
		strPosA.Format("%d,43,%d,63", nLeft, nLeft+40);
		SetItemAttribute(NET_MONITOR_238, "pos", strPosA);
		//m_EditKB.SetWindowText(Data.GetProcessName());
		nLeft += 45;
		//kb/s
		dc.DrawText(BkString::Get(NET_MONITOR_STR_503), -1, &m_RectProcName, DT_VCENTER | DT_SINGLELINE |DT_CALCRECT);
		strPosA.Format("%d,35,%d,60", nLeft, nLeft+m_RectProcName.Width());
		SetItemAttribute(NET_MONITOR_237, "pos", strPosA);
		SetItemText(NET_MONITOR_237, BkString::Get(NET_MONITOR_STR_503));
		//nLeft += m_RectProcName.Width();

		//是否自动限速
		if (TRUE == m_NetMonitorData.GetProcNetData().m_bAutoFixed)
		{
			SetItemCheck(NET_MONITOR_250, TRUE);
		}

		//判断是否是系统进程，如果是系统进程
		KSProcessInfo proInfo;
		if (0 == CListBoxData::GetDataPtr()->QueryProcInfo(Data.GetProcessFullPath(), proInfo))
		{
			if (enum_ProcessType_SystemCore == proInfo.nProcessType ||
				enum_ProcessType_SystemService == proInfo.nProcessType)
			{
				SetItemCheck(NET_MONITOR_250, FALSE);
				SetItemVisible(NET_MONITOR_250, FALSE);
			}

		}


		m_EditKB.SetSel(0,-1);

		dc.SelectFont(hFntTmp);
		::ReleaseDC(hWnd, dc);
		return TRUE;
	}


protected:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_231, OnBkClose)
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_232, OnOk);
		BK_NOTIFY_ID_COMMAND(NET_MONITOR_233, OnBkClose);
		BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CBKSafeNetMonitorLimit)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBKSafeNetMonitorLimit>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBKSafeNetMonitorLimit>)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
public:
	DWORD		m_dwDialogType;
	HWND		m_hWndNotify;
	DWORD		m_dwNotifyMsg;
	CNetMonitorCacheData	m_NetMonitorData;
	CRect		m_RectProcName;
	CBKNetMonitorLimitEdit		m_EditKB;//kb
};
