
#pragma once

#include "runautooptlist.h"
#include <runoptimize/interface.h>


class CRunAutoOptDlg
	: public CBkDialogImpl<CRunAutoOptDlg>
	, public CWHRoundRectFrameHelper<CRunAutoOptDlg>
	, public IAutoOptLstCBack
{
public:
	CRunAutoOptDlg(IRunOptimizeEng* pEng, CSimpleArray<CKsafeRunInfo*>& pInfoArray)
		: CBkDialogImpl<CRunAutoOptDlg>(IDR_BK_RUN_AUTO_OPT_DLG)
	{
		m_arrayItem = &pInfoArray;
		m_pRunEng   = pEng;
		m_nEndCode	= IDCANCEL;
	}

	~CRunAutoOptDlg(void)
	{
	}

protected:
	CAutoRunOptList					m_list;
	IRunOptimizeEng*				m_pRunEng;
	CSimpleArray<CKsafeRunInfo*>*	m_arrayItem;
	DWORD							m_nEndCode;

public:

	void OnBkBtnClose()
	{
		EndDialog(m_nEndCode);
	}

	virtual void ClickCheck()
	{
		EnableItem(IDC_BTN_RUN_AUTO_DO, m_list.GetCheckNumber()!=0);
	}

	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{
		m_list.Create( 
			GetViewHWND(), NULL, NULL, 
			WS_CLIPCHILDREN | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_OWNERDRAWFIXED, 
			0, IDC_LST_RUN_AUTO_OPT, NULL);

		m_list.SetCallback(this);
		m_list.InsertColumn(0,BkString::Get(IDS_SYSOPT_4016));
		m_list.SetColumnWidth(0,205);
		m_list.InsertColumn(1,BkString::Get(IDS_SYSOPT_4017));
		m_list.SetColumnWidth(1,300);
		m_list.InsertColumn(2,BkString::Get(IDS_SYSOPT_4018));
		m_list.SetColumnWidth(2,100);
		
		if ( !m_list.InitArray(*m_arrayItem) )
		{
			SetItemVisible( IDC_DIV_RUN_AUTO_HAVE,FALSE);
			SetItemVisible( IDC_DIV_RUN_AUTO_NO,TRUE);
		}

		return TRUE;
	}

	void CheckAll()
	{
		m_list.CheckAll(TRUE);
		ClickCheck();
	}

	void UnCheckAll()
	{
		m_list.CheckAll(FALSE);
		ClickCheck();
	}

	void DoOptzimine()
	{
		if (m_pRunEng==NULL)
			return;

		BOOL	bFailed = FALSE;
		for ( int i=0; i < m_list.GetItemCount(); i++)
		{
			_ARUN_ITEM_DATA*	px = m_list.GetItemParamData(i);
			if (px && px->nType==ARUN_TYPE_ITEM && px->nStatus==ARUN_STATUS_NULL && px->bCheck )
			{
				if ( m_pRunEng->ControlRun(FALSE,px->pInfo) )
				{
					px->nStatus = ARUN_STATUS_SUCCESS;
					m_nEndCode	= IDOK;
				}
				else
				{
					px->nStatus = ARUN_STATUS_FAILED;
					bFailed = TRUE;
				}
			}
		}
		m_list.InvalidateRect(NULL);
		m_list.SetWinDisable(TRUE);

		SetItemVisible(IDC_DIV_RUN_AUTO_TOP_INIT,FALSE);
		if (!bFailed)
			SetItemVisible(IDC_DIV_RUN_AUTO_TOP_SUCCESS,TRUE);
		else
			SetItemVisible(IDC_DIV_RUN_AUTO_TOP_FAILED,TRUE);

		SetItemVisible(IDC_DIV_RUN_AUTO_BTM_INIT,FALSE);
		SetItemVisible(IDC_DIV_RUN_AUTO_BTM_DONE,TRUE);
	}

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_RUN_AUTO_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_RUN_AUTO_BTM_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_RUN_AUTO_CANCEL, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_RUN_AUTO_CHECK_ALL, CheckAll)
		BK_NOTIFY_ID_COMMAND(IDC_LBL_RUN_AUTO_UNCHECK_ALL, UnCheckAll)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_RUN_AUTO_DO, DoOptzimine)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CRunAutoOptDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CRunAutoOptDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CRunAutoOptDlg>)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()
};
