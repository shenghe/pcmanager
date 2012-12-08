#pragma once
#include "atlwin.h"
#include "resource.h"

class CDlgInputUrl :
	public CDialogImpl<CDlgInputUrl>
{
public:
	enum { IDD = IDD_DIALOG_INPUTURL };
	
	CDlgInputUrl(void);
	~CDlgInputUrl(void);

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}
	
	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);


public:
	CString m_strUrl;
};
