#pragma once

#include <wtlhelper/whwindow.h>
#include "beikesafe.h"
#include <common/utility.h>
//#include "ldlistviewctrl.h"
#include "iefix/ldlistviewctrl.h"
#include "bkmsgdefine.h"

class CBKSafeExam_ShowHostsFileDlg
	: public CBkDialogImpl<CBKSafeExam_ShowHostsFileDlg>
	, public CWHRoundRectFrameHelper<CBKSafeExam_ShowHostsFileDlg>
{
public:
	CBKSafeExam_ShowHostsFileDlg(void)
		: CBkDialogImpl<CBKSafeExam_ShowHostsFileDlg>(IDR_BK_EXAM_SHOWHOSTSFILEDLG)
		, m_strExamLogPath(L"")
	{
		m_nDefSel = 0;
	}
	~CBKSafeExam_ShowHostsFileDlg(void);

	void OnBkClose()
	{
		_Module.Exam.FixItem(BkSafeExamItem::EXAM_HOSTS_FILE);
		EndDialog(0);
	}

	UINT OnOk()
	{
		_Module.Exam.FixItem(BkSafeExamItem::EXAM_HOSTS_FILE);
		EndDialog(0);
		return IDOK;
	}

	int SetDefTabSel(int nSel = 0)
	{
		int n = m_nDefSel;
		m_nDefSel = nSel;
		return n;
	}

	void OnCheckAll();
	void OnUnCheckAll();
	void OnRemoveShareResources();
	LRESULT OnLDListViewClickUrl(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBelieve();
	void OnUnBelieve();

protected:
	BOOL OnInitDialog(HWND wParam, LPARAM lParam);
	LRESULT OnLDListViewReSize(CRect rcSize);
	LRESULT OnLDLibListViewReSize(CRect rcSize);
	LRESULT OnBKTabSwitch(int nTabItemIDOld, int nTabItemIDNew);//×ó²àµÄTAB
	void RemoveSelectItems();
protected:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_HOSTS_RCLOSE_ID, OnBkClose)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_HOSTS_CLOSE_ID, OnOk)
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_HOSTS_CHECKALL_ID, OnCheckAll);
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_LNK_HOSTS_UNCHECKALL_ID, OnUnCheckAll);
		BK_NOTIFY_REALWND_RESIZED(IDC_EXAM_LIST_HOSTS_ID, OnLDListViewReSize);
		BK_NOTIFY_REALWND_RESIZED(IDC_EXAM_LIST_SR_ID, OnLDLibListViewReSize);
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_HOSTS_BELIEVE_ID, OnBelieve);
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_BTN_HOSTS_UNBELIEVE_ID, OnUnBelieve);
		BK_NOTIFY_ID_COMMAND(IDC_EXAM_CTRL_TMP_ID, RemoveSelectItems);
		BK_NOTIFY_TAB_SELCHANGE(IDC_EXAM_DLG_TAB, OnBKTabSwitch);
	BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CBKSafeExam_ShowHostsFileDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBKSafeExam_ShowHostsFileDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBKSafeExam_ShowHostsFileDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER_EX(MSG_USER_LDLISTVIEWCTRL_LBP_URL, OnLDListViewClickUrl)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()

private:
	CLDListViewCtrl m_ListLocalLib;
	CLDListViewCtrl m_ListHosts;
	CString m_strExamLogPath;
	int m_nDefSel;
	void InitCtrl();
	void SetCheckAll(BOOL bCheck  = TRUE );
	int LoadHostItem();
	void ShowHostItem(BkSafeExamLib::HostFileItem& HostItem);
	int ParseHostItem(CString strHostItem, CString& strIP, CString& strName);
	void ShowMsg(LPCTSTR pszCaption);
	int FixHostItem(int nSel = -1);
	int LoadSafeLibItem();
	int ParsexmlCmdLine(LPCTSTR pszCmdLine, CAtlArray<CString>& arrStr);
	int RemoveLocalLibItem(int nSel);	
};
