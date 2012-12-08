#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "BkSoftMgrPlugInfoDlg.h"
#include "beikesafesoftmgrHeader.h"
#include <skylark2/midgenerator.h>


void CBkPlugInfoDlg::OnBkBtnClose()
{
	EndDialog(IDCANCEL);
}

void CBkPlugInfoDlg::OnBkBtnOK()
{
	EndDialog(IDOK);
}

void CBkPlugInfoDlg::OnBkBtnCancel()
{
	EndDialog(IDCANCEL);
}

void CBkPlugInfoDlg::OnBkBtnNoPointOutPlug()
{
	BOOL bCheck = this->GetItemCheck( IDC_DOWN_CHECK_NO_POINT_OUT_PLUG );
	BKSafeConfig::SetHintPlugin(bCheck?0:1);
}

BOOL CBkPlugInfoDlg::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	m_IconDlg.Create(this->GetViewHWND(), NULL, NULL, WS_CHILD | WS_TABSTOP, NULL, IDC_DOWN_POINT_OUT_ICON);
	m_IconDlg.SetDataRes(m_pData->m_pImage);
	m_IconDlg.SetSmall(TRUE);

	this->FormatRichText(
		IDC_DOWN_POINT_OUT_TITLE, 
		BkString::Get(IDC_DOWN_POINT_OUT_TITLE_FORMAT), 
		m_pData->m_strName );

	int nPos = 50;
	CStringA strPos;
	if (m_pData->m_bCharge == TRUE)
	{
		SetItemVisible(IDC_DOWN_POINT_OUT_IS_CHARGE, TRUE);
		strPos.Format("12,%d,-12,%d", nPos,nPos+12);
		this->SetItemAttribute( IDC_DOWN_POINT_OUT_IS_CHARGE, "pos",strPos );
		nPos += 22;
	}
	else
		SetItemVisible(IDC_DOWN_POINT_OUT_IS_CHARGE, FALSE);

	if (m_pData->m_bPlug == TRUE)
	{
		SetItemVisible(IDC_DOWN_POINT_OUT_WITH_PLUG, TRUE);
		strPos.Format("12,%d,-12,%d", nPos,nPos+12);
		this->SetItemAttribute( IDC_DOWN_POINT_OUT_WITH_PLUG, "pos",strPos );
		nPos += 22;
	}
	else
		SetItemVisible(IDC_DOWN_POINT_OUT_WITH_PLUG, FALSE);

	if ((m_pData->m_attri&SA_Beta) == SA_Beta)
	{
		SetItemVisible(IDC_DOWN_POINT_OUT_IS_BETA, TRUE);
		strPos.Format("12,%d,-12,%d", nPos,nPos+12);
		this->SetItemAttribute( IDC_DOWN_POINT_OUT_IS_BETA, "pos",strPos );
	}
	else
		SetItemVisible(IDC_DOWN_POINT_OUT_IS_BETA, FALSE);

	return TRUE;
}

UINT_PTR CBkPlugInfoDlg::DoModal(HWND hWndParent)
{
	return __super::DoModal(hWndParent);
}
