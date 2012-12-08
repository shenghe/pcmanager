#pragma once

class CLoadRunInfoDlg
	: public CBkDialogImpl<CLoadRunInfoDlg>
	, public CWHRoundRectFrameHelper<CLoadRunInfoDlg>
{
public:
	CLoadRunInfoDlg()
		: CBkDialogImpl<CLoadRunInfoDlg>(IDR_BK_SYSOPT_LOADING)
	{
	}

	~CLoadRunInfoDlg(void)
	{
	}

	BOOL OnInitDialog(HWND,LPARAM )
	{
		SetTimer(TIMER_ID_LOADING_INFO,50,NULL);
		return TRUE;
	}

	void OnTimer(UINT_PTR nIDEvent)
	{
		ShowOptDetail();
	}

	void ShowOptDetail()
	{
		static int nPos = 0;
		++nPos;
		this->SetItemIntAttribute(4, "sub", (nPos)%12);
	}

	void OnDestory()
	{
		KillTimer(TIMER_ID_LOADING_INFO);
	}

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CLoadRunInfoDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CLoadRunInfoDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CLoadRunInfoDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_DESTROY(OnDestory)
	END_MSG_MAP()
};