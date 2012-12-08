#pragma once


#include <wtlhelper/whwindow.h>
#include "beikesafe.h"
#include <common/utility.h>
#include "bkmsgdefine.h"
#include <safeexam/beikesafeexam.h>
#include "common/callduba.h"

class CBKSafeExam_Guest
	: public CBkDialogImpl<CBKSafeExam_Guest>
	, public CWHRoundRectFrameHelper<CBKSafeExam_Guest>

{
public:
	CBKSafeExam_Guest(){}
	CBKSafeExam_Guest(HWND hNotifyWnd, DWORD dwNotifyMsg)
	{
		m_dwNotifyMsg = dwNotifyMsg;
		m_hWndNotify = hNotifyWnd;
	}
	~CBKSafeExam_Guest(void){};

	virtual void OnBkClose()
	{
		EndDialog(0);
	}

	virtual void OnOk()
	{
		if (TRUE == this->GetItemCheck(IDC_EXAM_RADIO_IGNORE))
		{//ÐÅÈÎ
			CString strValue;
			if (BkSafeExamItem::EXAM_GUEST_RUN == m_dwDialogType)
			{
				strValue = EXAM_GUEST_STR;
			}else if (BkSafeExamItem::EXAM_REMOTE_DESKTOP == m_dwDialogType)
			{
				strValue = EXAM_REMOTEDESK_STR;//L"REMOTEDESK";
			}
			if (FALSE == strValue.IsEmpty())
			{
				_Module.Exam.SetItemSafe(m_dwDialogType, (void**)(strValue.GetBuffer(-1)));
				strValue.ReleaseBuffer(-1);
				REMOVE_EXAM_RESULT_ITEM RemoveItemInfo;
				RemoveItemInfo.uRemoveItem.RemoveItemInfo.Reset();
				RemoveItemInfo.uRemoveItem.RemoveItemInfo.dwItemID = m_dwDialogType;
				::SendMessage(m_hWndNotify, MSG_APP_EXAM_SCORE_CHANGE, (WPARAM)BkSafeExamItemExOperate::RemoveItem, (LPARAM)&RemoveItemInfo);
			}
		}
		else if (TRUE == this->GetItemCheck(IDC_EXAM_RADIO_FIX))
		{//ÐÞ¸´
			_Module.Exam.FixItem(m_dwDialogType);

		}

		EndDialog(0);
	}

	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{
		this->SetItemCheck(IDC_EXAM_RADIO_FIX, TRUE);
		return TRUE;
	}

	DWORD SetDlgType(DWORD dwType)
	{
		DWORD dw = m_dwDialogType;
		m_dwDialogType = dwType;
		return dw;
	}

protected:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_HOSTS_CLOSE_ID, OnBkClose)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_HOSTS_BELIEVE_ID, OnOk)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_HOSTS_RCLOSE_ID, OnBkClose)
		BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CBKSafeExam_Guest)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBKSafeExam_Guest>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBKSafeExam_Guest>)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
public:
	DWORD m_dwDialogType;
	HWND m_hWndNotify;
	DWORD m_dwNotifyMsg;
};

class CBKSafeExam_Duba:public CBKSafeExam_Guest
{
public:
	CBKSafeExam_Duba(HWND hNotifyWnd, DWORD dwNotifyMsg)
	{
		m_dwNotifyMsg = dwNotifyMsg;
		m_hWndNotify = hNotifyWnd;
	}
	~CBKSafeExam_Duba(){}

protected:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_CHECK_841, OnIgnoreCheckAV)
		BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CBKSafeExam_Duba)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBKSafeExam_Guest)
		MSG_WM_INITDIALOG(OnInitDialog)
		END_MSG_MAP()

		void OnIgnoreCheckAV()
	{
		_Module.Exam.SetItemSafe(m_dwDialogType, (void**)EXAM_DUBA);
		REMOVE_EXAM_RESULT_ITEM RemoveItemInfo;
		RemoveItemInfo.uRemoveItem.RemoveItemInfo.Reset();
		RemoveItemInfo.uRemoveItem.RemoveItemInfo.dwItemID = m_dwDialogType;
		::SendMessage(m_hWndNotify, MSG_APP_EXAM_SCORE_CHANGE, (WPARAM)BkSafeExamItemExOperate::RemoveItem, (LPARAM)&RemoveItemInfo);

		_ReportLog(0, L"430");
		EndDialog(0);
	}

	virtual void OnBkClose()
	{
		_ReportLog(0, L"430");
		EndDialog(0);
	}

	void OnOk()
	{
		KCallDuba duba;
		CString strInstaller;
		duba.GetInstallerPath(strInstaller);
		if (TRUE == ::PathFileExists(strInstaller))
		{
	 		duba.CallDubaInstall();
		}
		else
		{
			::SendMessage(m_hWndNotify, WM_MSG_CALL_DUBA, 0, 0);
		}
		
		_ReportLog(0, L"431");
		EndDialog(0);
	}
	void _ReportLog(int ReportType, CString strLog/* = TEXT("")*/)
	{
		CString strURL(TEXT(""));
		switch(ReportType)
		{
		case  0:
			{
			}
			break;
		default:
			{
				strURL.Empty();
			}
			break;
		}
	}
};
