#include "StdAfx.h"
#include "DlgInputUrl.h"

CDlgInputUrl::CDlgInputUrl(void)
{
}

CDlgInputUrl::~CDlgInputUrl(void)
{
}

LRESULT CDlgInputUrl::OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
	// center the dialog on the screen
	CenterWindow();

	SetDlgItemText(IDC_EDIT_URL, _T("http://"));
	return 0;
}

LRESULT CDlgInputUrl::OnOK( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
	GetDlgItemText(IDC_EDIT_URL, m_strUrl);
	EndDialog(IDOK);
	return 0;
}