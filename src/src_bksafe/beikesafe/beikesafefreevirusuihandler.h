
#pragma once

#include "virusscan/htmlcontainerwindow.h"
#include "downloadtoolsdlg.h"

class CBeikeSafeMainDlg;
class CBeikeSafeFreeVirusUIHandler :
	public CBkNavigator
{
public:
	CBeikeSafeFreeVirusUIHandler(CBeikeSafeMainDlg* refDialog)
		: m_pDlg(refDialog),
		m_bShowOnLineScan(FALSE),
		m_pDownInstaller(NULL)
	{
	}

	virtual ~CBeikeSafeFreeVirusUIHandler()
	{

	}

protected:
	CBeikeSafeMainDlg*				m_pDlg;
	CWHHtmlContainerWindowEx		m_wndHtmlOnlineScan;
	BOOL							m_bShowOnLineScan;
	CToolsDownloadDlg*				m_pDownInstaller;

public:
	void Init();
	BOOL FirstShow();

	HRESULT GetErrorHtmlPath(CString &strDir);
	LRESULT OnCallDubaClick(UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	virtual CBkNavigator* OnNavigate(CString &strChildName);
	BOOL	IsNeedDownLoad( CString& strFile, LPCTSTR pszUrl );
public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBeikeSafeFreeVirusUIHandler)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		MESSAGE_HANDLER_EX(WM_MSG_CALL_DUBA,OnCallDubaClick)
	END_MSG_MAP()
};