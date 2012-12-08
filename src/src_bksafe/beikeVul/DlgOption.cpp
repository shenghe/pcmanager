#include "StdAfx.h"
#include "DlgOption.h"


CDlgOption::CDlgOption(void)
{
}

CDlgOption::~CDlgOption(void)
{
}

LRESULT CDlgOption::OnInitDialog( HWND /*hWnd*/, LPARAM /*lParam*/ )
{
	m_ctlEdit.Create( GetViewHWND(), NULL, NULL, 
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_LEFT, WS_EX_NOPARENTNOTIFY|WS_EX_CLIENTEDGE	
		, 20110, NULL);

	LOGFONT lf = {0};
	lf.lfHeight=-100;
	::lstrcpy(lf.lfFaceName,_T("Tahoma"));
	m_font.CreatePointFontIndirect(&lf);
	m_ctlEdit.SetFont(m_font);
	
	OnBkBtnReset();
	return 0;
}

void CDlgOption::OnBkBtnReset()
{
	m_ctlEdit.SetWindowText( m_strPath );
}

void CDlgOption::OnBkBtnCancel()
{
	EndDialog(IDCANCEL);
}

void CDlgOption::OnBkBtnOK()
{
	m_ctlEdit.GetWindowText( m_strPath );
	EndDialog(IDOK);
}

void CDlgOption::OnBkBtnOpenDir()
{
	CString strPath;
	m_ctlEdit.GetWindowText( strPath );
	ShellExecute(NULL, _T("open"), strPath, NULL, NULL, SW_SHOW);
}

void CDlgOption::OnBkBtnChangeDir()
{
	CString sFolderPath;
	m_ctlEdit.GetWindowText( sFolderPath );
	if( SelectFolder(sFolderPath) )
	{
		m_ctlEdit.SetWindowText( sFolderPath );
	}
}