

#pragma once


#include <wtlhelper/whwindow.h>
#include <common/utility.h>
#include "perfmon/kperfmonmem.h"
#include "kmaindlg.h"

#define MSG_PROGRESS		WM_APP+0x400
#define MSG_FINISHED		WM_APP+0x401


class CBKSafeSysPerfOpt
	: public CBkDialogImpl<CBKSafeSysPerfOpt>
	, public CWHRoundRectFrameHelper<CBKSafeSysPerfOpt>

{
public:
	CBKSafeSysPerfOpt(){}
	CBKSafeSysPerfOpt(KMainDlg* pMainDlg){m_pMainDlg = pMainDlg;}

	CBKSafeSysPerfOpt(HWND hNotifyWnd, DWORD dwNotifyMsg)
	{
		m_hWndNotify = hNotifyWnd;
	}
	~CBKSafeSysPerfOpt(){}

protected:
		BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
			BK_NOTIFY_ID_COMMAND(102, OnBkClose)
			BK_NOTIFY_ID_COMMAND(301, OnBkClose)
			BK_NOTIFY_ID_COMMAND(203, OnBkClose)
		BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CBKSafeSysPerfOpt)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBKSafeSysPerfOpt>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBKSafeSysPerfOpt>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER(MSG_PROGRESS, OnProgress)
		MESSAGE_HANDLER(MSG_FINISHED, OnFinished)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()


	LRESULT OnProgress(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnFinished(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

		BOOL OnInitDialog(HWND wParam, LPARAM lParam);

		virtual void OnBkClose();

		void OnOk();
		void UpDateOneKeyFreeMemHistory();

		void SetProgress(int nTotal, int nCur);

		void FreeMemFinished(ULONGLONG uSize);
private:
	HWND		m_hWndNotify;
	KMainDlg*	m_pMainDlg;
//	CSysPerfOpt	m_sysPerf;
};