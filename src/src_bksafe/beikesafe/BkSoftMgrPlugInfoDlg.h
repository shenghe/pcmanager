#pragma  once

#include "SoftMgrItemData.h"
#include "softmgrdetail.h"

class CBkPlugInfoDlg
	: public CBkDialogImpl<CBkPlugInfoDlg>
	, public CWHRoundRectFrameHelper<CBkPlugInfoDlg>
{
public:
	CBkPlugInfoDlg(CSoftListItemData* pData)
		: CBkDialogImpl<CBkPlugInfoDlg>(IDR_BK_PLUG_INFO_DLG)
		, m_pData(pData)
	{
	}

	~CBkPlugInfoDlg()
	{
	}

public:
	UINT_PTR DoModal(HWND hWndParent);

protected:
	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void OnBkBtnClose();
	void OnBkBtnOK();
	void OnBkBtnCancel();
	void OnBkBtnNoPointOutPlug();

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDOK, OnBkBtnOK)
		BK_NOTIFY_ID_COMMAND(IDCANCEL, OnBkBtnCancel)
		BK_NOTIFY_ID_COMMAND(IDC_DOWN_CHECK_NO_POINT_OUT_PLUG, OnBkBtnNoPointOutPlug)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBkPlugInfoDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CBkPlugInfoDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBkPlugInfoDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

public:
	CSoftListItemData* m_pData;
	CDetailSoftMgrDlg m_IconDlg;
};

