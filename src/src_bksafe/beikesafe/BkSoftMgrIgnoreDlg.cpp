#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "BkSoftMgrIgnoreDlg.h"
#include "beikesafesoftmgrHeader.h"
#include "BkSoftMgrNewInfoDlg.h"
#include <skylark2/midgenerator.h>

#define SOFT_LIST_ITEM_HEIGHT		55

void CBkIgnoreDlg::OnBkBtnClose()
{
	EndDialog(IDCANCEL);
}

void CBkIgnoreDlg::OnIgnoreSelect()
{
	for (int i = 0; i < m_arrData.GetSize(); i++)
	{
		CSoftListItemData *pData = m_arrData[i];
		if (pData)
		{
			if (pData->m_bCheck && pData->m_bIgnore == TRUE)
				pData->m_bIgnore = FALSE;
		}
	}

	EndDialog(IDOK);
}

void CBkIgnoreDlg::OnIgnoreCheckAll()
{
	BOOL bCheckAll = this->GetItemCheck(IDC_UPDATE_CHECK_IGNORE_ALL);

	for (int i = 0; i < m_arrData.GetSize(); i++)
	{
		CSoftListItemData *pData = m_arrData[i];
		if (pData)
		{
			pData->m_bCheck = bCheckAll;
		}
	}

	m_necessList->ForceRefresh();

	this->EnableItem( IDC_CTL_UPDATE_IGNORE_SEL, bCheckAll);
}

BOOL CBkIgnoreDlg::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	int nIgnoreCount = m_arrData.GetSize();
	this->FormatRichText(
		IDC_CTL_UPDATE_IGNORE_TITLE, 
		BkString::Get(IDS_DLG_UPDATE_IGNORE_COUNT_FORMAT), 
		nIgnoreCount );

	m_necessList = new CBkListBox;
	m_necessList->Create( this->GetViewHWND() , IDC_CTL_UPDATE_IGNORE_WND);
	m_necessList->Load(IDR_BK_SOFTMGR_IGNORE_TEMPLATE);
	m_necessList->SetItemFixHeight(SOFT_LIST_ITEM_HEIGHT);

	m_necessList->SetItemCount( m_arrData.GetSize() );
	m_necessList->InvalidateRect(NULL);

	BOOL bSelect = FALSE;
	for (int i = 0; i < m_arrData.GetSize(); i++)
	{
		CSoftListItemData *pData = m_arrData[i];
		if (pData && pData->m_bCheck)
		{
			bSelect = TRUE;
			break;
		}
	}
	if (bSelect)
		this->EnableItem( IDC_CTL_UPDATE_IGNORE_SEL, TRUE);
	else
		this->EnableItem( IDC_CTL_UPDATE_IGNORE_SEL, FALSE);

	return TRUE;
}

BOOL CBkIgnoreDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect rcWin;
	if ( m_necessList && m_necessList->IsWindow() && m_necessList->IsWindowVisible() )
	{
		m_necessList->GetWindowRect(&rcWin);

		if (rcWin.PtInRect(pt))
			SendMessage(m_necessList->m_hWnd, WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
	}

	SetMsgHandled(FALSE);
	return FALSE;
}

UINT_PTR CBkIgnoreDlg::DoModal(HWND hWndParent)
{
	return __super::DoModal(hWndParent);
}

LRESULT CBkIgnoreDlg::OnListBoxGetDispInfo(LPNMHDR pnmh)
{
	BKLBMGETDISPINFO* pdi = (BKLBMGETDISPINFO*)pnmh;

	if ( pdi->nListItemID >= m_arrData.GetSize() )
		return 0;

	CSoftListItemData*	pSoftData = m_arrData[pdi->nListItemID];
	if (pSoftData)
	{
		m_necessList->SetItemAttribute(IDC_CTL_UPDATE_IGNORE_ITEM,"crbg",pdi->bSelect?"EBF5FF":"FFFFFF");

		// 图标的绘制
		{
			CStringA	strMem;
			strMem.Format("%d",pSoftData->m_pImage);
			m_necessList->SetItemAttribute( IDC_CTL_UPDATE_IGNORE_ICON, "mempointer",strMem );
		}

		// 下载图标
		if (!pSoftData->m_bIcon && m_pHandler)
			m_pHandler->OnDownLoadIcon( pSoftData->m_strSoftID );

		if (pSoftData->m_bCheck)
			m_necessList->SetItemCheck(IDC_CTL_UPDATE_IGNORE_CHECK, TRUE);
		else
			m_necessList->SetItemCheck(IDC_CTL_UPDATE_IGNORE_CHECK, FALSE);
		m_necessList->SetItemText(IDC_CTL_UPDATE_IGNORE_NAME, pSoftData->m_strName);
		m_necessList->SetItemText(IDC_CTL_UPDATE_IGNORE_DES, pSoftData->m_strPublished);
		CString strLocalVer;
		strLocalVer.Format(L"当前版本:%s", pSoftData->m_strVersion);
		m_necessList->SetItemText(IDC_CTL_UPDATE_IGNORE_CUR_VER, strLocalVer);
		CString strNewVer;
		strNewVer.Format(L"可更新至:%s", pSoftData->m_strNewVersion);
		m_necessList->SetItemText(IDC_CTL_UPDATE_IGNORE_NEW_VER, strNewVer);
	}

	return TRUE;
}

LRESULT CBkIgnoreDlg::OnListBoxGetmaxHeight(LPNMHDR pnmh)
{
	BKLBITEMCALCMAXITEM *pdi = (BKLBITEMCALCMAXITEM*)pnmh;
	pdi->nMaxHeight = SOFT_LIST_ITEM_HEIGHT;
	return 0;
}

LRESULT CBkIgnoreDlg::OnListBoxGetItemHeight(LPNMHDR pnmh)
{
	BKLBITEMCACLHEIGHT *pdi	= (BKLBITEMCACLHEIGHT*)pnmh;
	if ( pdi->nListItemId >= m_arrData.GetSize() )
		return 0;

	pdi->nHeight = SOFT_LIST_ITEM_HEIGHT;
	return 0;
}

LRESULT CBkIgnoreDlg::OnBkListBoxClickCtrl(LPNMHDR pnmh)
{
	LPBKLBMITEMCLICK pnms = (LPBKLBMITEMCLICK)pnmh;
	if ( pnms->nListItemID >= m_arrData.GetSize() )
		return 0;

	CSoftListItemData*	datalist = m_arrData[pnms->nListItemID];
	if (datalist)
	{
		switch( pnms->uCmdID )
		{
		case IDC_CTL_UPDATE_IGNORE_CHECK:
			{
				datalist->m_bCheck = m_necessList->GetItemCheck(IDC_CTL_UPDATE_IGNORE_CHECK);

				int nCheckCount = 0;
				BOOL bSelect = FALSE;
				for (int i = 0; i < m_arrData.GetSize(); i++)
				{
					CSoftListItemData *pData = m_arrData[i];
					if (pData && pData->m_bCheck)
					{
						bSelect = TRUE;
						nCheckCount++;
					}
				}
				if (bSelect)
					this->EnableItem( IDC_CTL_UPDATE_IGNORE_SEL, TRUE);
				else
					this->EnableItem( IDC_CTL_UPDATE_IGNORE_SEL, FALSE);
				if (nCheckCount == m_arrData.GetSize())
					this->SetItemCheck(IDC_UPDATE_CHECK_IGNORE_ALL, TRUE);
				else
					this->SetItemCheck(IDC_UPDATE_CHECK_IGNORE_ALL, FALSE);
			}
			break;

		case IDC_CTL_UPDATE_IGNORE_NEW_INFO:
			{
				if (!m_pHandler)
					return 0;

				CBkNewInfoDlg	dlg( m_pHandler, datalist->m_strSoftID, datalist->m_strName );
				UINT_PTR uRet = dlg.DoModal(GetActiveWindow());
				if (IDOK == uRet)
				{
					NeedUpdateSoft(datalist);
					m_datalistUpdate = datalist;
					EndDialog(IDOK);
				}
			}
			break;

		case IDC_CTL_UPDATE_IGNORE_NEED_UPDATE:
			{
				NeedUpdateSoft(datalist);
			}
			break;
		default:
			break;
		}
	}
	return 0;
}

void CBkIgnoreDlg::NeedUpdateSoft(CSoftListItemData* datalist)
{
	if (datalist->m_bIgnore)
		datalist->m_bIgnore = FALSE;
	CSimpleArray<CSoftListItemData*> arrData;
	for (int i = 0; i < m_arrData.GetSize(); i++)
	{
		CSoftListItemData *pData = m_arrData[i];
		if (pData && pData->m_bIgnore)
			arrData.Add(pData);
	}
	m_arrData.RemoveAll();
	for (int i = 0; i < arrData.GetSize(); i++)
	{
		CSoftListItemData *pData = arrData[i];
		if (pData && pData->m_bIgnore)
			m_arrData.Add(pData);
	}

	int nIgnoreCount = m_arrData.GetSize();
	this->FormatRichText(
		IDC_CTL_UPDATE_IGNORE_TITLE, 
		BkString::Get(IDS_DLG_UPDATE_IGNORE_COUNT_FORMAT), 
		nIgnoreCount );

	m_necessList->SetItemCount(nIgnoreCount);
	if (nIgnoreCount == 0)
	{
		this->SetItemVisible(IDC_CTL_UPDATE_IGNORE_WND,FALSE);
		this->SetItemVisible(IDC_CTL_UPDATE_IGNORE_NONE,TRUE);

		this->EnableItem( IDC_UPDATE_CHECK_IGNORE_ALL, FALSE);
		this->EnableItem( IDC_CTL_UPDATE_IGNORE_SEL, FALSE);
	}
}