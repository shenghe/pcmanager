#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "BkSoftMgrNewInfoDlg.h"
#include "beikesafesoftmgrHeader.h"
#include <skylark2/midgenerator.h>


void CBkNewInfoDlg::OnBkBtnClose()
{
	EndDialog(IDCANCEL);
}

void CBkNewInfoDlg::OnBkBtnOK()
{
	EndDialog(IDOK);
}

void CBkNewInfoDlg::OnBkBtnCancel()
{
	EndDialog(IDCANCEL);
}

BOOL CBkNewInfoDlg::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	SetItemText(IDC_CTL_NEW_INFO_TITLE, m_softName);

	m_IE.Create(GetViewHWND(), IDC_CTL_IE_NEW_INFO, FALSE, RGB(0xFB, 0xFC, 0xFD));

	CString	strURLError;
	if ( m_pHandler && SUCCEEDED(m_pHandler->GetKSoftDirectory(strURLError)) )
		strURLError.Append(_T("\\html\\error.html"));
	else
		strURLError.Empty();

	CString     strMid;
	Skylark::CMidGenerator::Instance().GetMid( strMid );

	CString url = L"http://baike.ijinshan.com/ksafe/client/update/" + m_softId + L".html";
	m_IE.Show2(url, strURLError);

	return TRUE;
}

UINT_PTR CBkNewInfoDlg::DoModal(HWND hWndParent)
{
	return __super::DoModal(hWndParent);
}
