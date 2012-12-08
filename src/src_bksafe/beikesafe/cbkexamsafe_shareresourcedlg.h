#pragma once

#include <wtlhelper/whwindow.h>
#include "beikesafe.h"
#include <common/utility.h>
//#include "ldlistviewctrl.h"
#include "iefix/ldlistviewctrl.h"
#include "bkmsgdefine.h"


class CBKExamSafe_ShareResourceDlg
	: public CBkDialogImpl<CBKExamSafe_ShareResourceDlg>
	, public CWHRoundRectFrameHelper<CBKExamSafe_ShareResourceDlg>
{
public:
	CBKExamSafe_ShareResourceDlg(void)
		: CBkDialogImpl<CBKExamSafe_ShareResourceDlg>(IDR_BK_EXAM_SHARERESOURCES)
	{
		m_nDefSel = 0;
	}
	~CBKExamSafe_ShareResourceDlg(void);

	void OnBkClose()
	{
		EndDialog(0);
	}

	void OnOk()
	{
		EndDialog(0);
	}

	int LoadShareResources();
	int GetShareResources(CSimpleArray<BKSafeExamItemEx::SHARE_RESOURCES>& arrRes);
	void OnCheckAll();
	void OnUnCheckAll();
	void OnRemoveShareResources();
	LRESULT OnLDListViewClickUrl(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBelieve();
	int SetDefSel(int nSel = 0)
	{
		int n = m_nDefSel;
		m_nDefSel = nSel;
		return n;
	}

protected:
	BOOL OnInitDialog(HWND wParam, LPARAM lParam);
	LRESULT OnLDListViewReSize(CRect rcSize);
	LRESULT OnLDLibListViewReSize(CRect rcSize);
	LRESULT OnBKTabSwitch(int nTabItemIDOld, int nTabItemIDNew);//×ó²àµÄTAB
	void RemoveSelectItems();
protected:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_SR_RCLOSE_ID, OnBkClose)
		//BK_NOTIFY_ID_COMMAND(IDC_BTN_PLUG_RPTQQ_CLOSE, OnBkClose)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_SR_CLOSE_ID, OnOk)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_SR_CHECKALL_ID, OnCheckAll);
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_SR_UNCHECKALL_ID, OnUnCheckAll);
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_SR_CANCEL_ID, OnRemoveShareResources);
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_SR_BELIEVE_ID, OnBelieve);
		BK_NOTIFY_REALWND_RESIZED(IDC_EXAM_DLG_SR_LISTVIEW_ID, OnLDListViewReSize);
		BK_NOTIFY_REALWND_RESIZED(IDC_EXAM_LIST_SR_ID, OnLDLibListViewReSize);
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_CTRL_TMP_ID, RemoveSelectItems);
		BK_NOTIFY_TAB_SELCHANGE(IDC_EXAM_DLG_TAB, OnBKTabSwitch);
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBKExamSafe_ShareResourceDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBKExamSafe_ShareResourceDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBKExamSafe_ShareResourceDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER_EX(MSG_USER_LDLISTVIEWCTRL_LBP_URL, OnLDListViewClickUrl)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
	
private:
	CLDListViewCtrl m_ListViewSR;
	CLDListViewCtrl m_ListLocalLib;
	CSimpleArray<BKSafeExamItemEx::SHARE_RESOURCES> m_arrRes;
	CString m_strExamLogPath;
	int m_nDefSel;
	void SetCheckAll(BOOL bCheck = TRUE);
	void _InitCtrl();
	int ShowResources();
	void ShowMsg(LPCTSTR pszCaption);
	int RemoveItem(int nSel = -1);
	CString _GetExamLogFilePath();
	int LoadSafeLibItem();
	int RemoveLocalLibItem(int nSel);
};
