#pragma once

#include <wtlhelper/whwindow.h>
#include "beikesafe.h"
#include "bkmsgdefine.h"

class CBSafeExam_SafeMonStateDlg
	: public CBkDialogImpl<CBSafeExam_SafeMonStateDlg>
	, public CWHRoundRectFrameHelper<CBSafeExam_SafeMonStateDlg>
{
public:
	CBSafeExam_SafeMonStateDlg(void)
		: CBkDialogImpl<CBSafeExam_SafeMonStateDlg>(IDR_BK_EXAM_SAFEMONSTATE)
	{
		m_NotifyHwnd = NULL;
	}
	~CBSafeExam_SafeMonStateDlg(void);

	void OnBkClose()
	{
		EndDialog(0);
	}

	UINT OnOk()
	{
		EndDialog(0);
		return IDOK;
	}
	void OnGotoSafeMon()
	{
		if (NULL != m_NotifyHwnd)
		{
			::SendMessage(m_NotifyHwnd, MSG_APP_GOTO_MAIN_TAB, 0, 6);
		}

		EndDialog(0);
	}

public:
	HWND SetNotifyWnd(HWND hWnd)
	{
		HWND hwnd = m_NotifyHwnd;
		m_NotifyHwnd = hWnd;
		return hwnd;
	}
protected:
	BOOL OnInitDialog(HWND wParam, LPARAM lParam);
//	void OnGotoSafeMon();
protected:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_SAFEMON_CLOSE_ID, OnBkClose)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_SAFEMON_OK_ID, OnOk)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_SHOWMORE_ID, OnGotoSafeMon)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBSafeExam_SafeMonStateDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBSafeExam_SafeMonStateDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBSafeExam_SafeMonStateDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
private:
	HWND m_NotifyHwnd;
};
