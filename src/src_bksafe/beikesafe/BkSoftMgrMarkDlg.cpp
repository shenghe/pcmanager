#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "BkSoftMgrMarkDlg.h"
#include "beikesafesoftmgrHeader.h"
#include <skylark2/midgenerator.h>


void CBkMarkDlg::OnBkBtnClose()
{
	EndDialog(IDCANCEL);
}

BOOL CBkMarkDlg::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	SetItemText(IDC_CTL_MARK_TITLE, m_softName);

	m_IEMark.Create(GetViewHWND(), IDC_CTL_IE_MARK, FALSE, RGB(0xFB, 0xFC, 0xFD));

	CString	strURLError;
	if ( m_pHandler && SUCCEEDED(m_pHandler->GetKSoftDirectory(strURLError)) )
		strURLError.Append(_T("\\html\\error.html"));
	else
		strURLError.Empty();

	CString     strMid;
	Skylark::CMidGenerator::Instance().GetMid( strMid );

	CString url = L"http://baike.ijinshan.com/interface/v1_0/vote.html?softId=" + m_softId + L"&mid=" + strMid;
	if (m_softId.GetLength() == 0)
		m_IEMark.Show2(strURLError, strURLError);
	else
		m_IEMark.Show2(url, strURLError);

	return TRUE;
}

UINT_PTR CBkMarkDlg::DoModal(HWND hWndParent)
{
	return __super::DoModal(hWndParent);
}
