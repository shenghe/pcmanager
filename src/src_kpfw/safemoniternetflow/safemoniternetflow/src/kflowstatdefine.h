#pragma once
#include "bkres/bkres.h"
#include <wtlhelper/whwindow.h>
#define		DEFOKMAINBUTTON				104
#define		DEFCANCELMAINBUTTON			105
class KFlowStatDefine
	: public CBkDialogImpl<KFlowStatDefine>,
	public CWHRoundRectFrameHelper<KFlowStatDefine>
{

public:
	KFlowStatDefine()
		: CBkDialogImpl<KFlowStatDefine>(IDR_BK_STAT_DEFINE)
	{

	}

	~KFlowStatDefine(){}

public:
	void			BtnClose(void);
	void			BtnCancel(void);
	void			BtnOK(void);
	void			GetRangleQuery(OUT CString& strFrom, OUT CString& strTo);

	BOOL			OnInitDialog(CWindow wndFocus, LPARAM lInitParam);		


	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(DEFCLOSEMAINBUTTON, BtnClose)
		BK_NOTIFY_ID_COMMAND(DEFOKMAINBUTTON, BtnOK)
		BK_NOTIFY_ID_COMMAND(DEFCANCELMAINBUTTON, BtnCancel)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(KFlowStatDefine)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<KFlowStatDefine>)
		CHAIN_MSG_MAP(CBkDialogImpl<KFlowStatDefine>)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()



private:
	CDateTimePickerCtrl m_TimeFrom;
	CDateTimePickerCtrl m_TimeTo;
	CString				m_strTimeFrom;
	CString				m_strTimeTo;
};