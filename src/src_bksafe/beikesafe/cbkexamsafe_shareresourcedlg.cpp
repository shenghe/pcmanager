#include "StdAfx.h"
#include "cbkexamsafe_shareresourcedlg.h"
#include "beikesafemsgbox.h"
//#include "cinifile.h"
#include "iefix/cinifile.h"
using namespace IniFileOperate;


#define URL_COLOR RGB(0, 128, 0)
#define URL_COLOR2 RGB(77,113,149)
#define PRINTF_STR(x,y){_sntprintf_s((x), sizeof((x)), TEXT("%s"), (y));}
#define PRINTF_STR2(x,f,y){_sntprintf_s((x), sizeof((x)), (f), (y));}
#define PRINTF_INT(x,y){_sntprintf_s((x), sizeof((x)), TEXT("%d"), (y));}


CBKExamSafe_ShareResourceDlg::~CBKExamSafe_ShareResourceDlg(void)
{
}


BOOL CBKExamSafe_ShareResourceDlg::OnInitDialog(HWND wParam, LPARAM lParam)
{
	m_strExamLogPath = _GetExamLogFilePath();
	_InitCtrl();
 	LoadShareResources();
 	ShowResources();
	this->SetTabCurSel(IDC_EXAM_DLG_TAB, m_nDefSel);
	return TRUE;
}

LRESULT CBKExamSafe_ShareResourceDlg::OnLDListViewReSize(CRect rcSize)
{
	if (rcSize.IsRectEmpty() || FALSE == ::IsWindow(m_ListViewSR.m_hWnd))
		return 0;

	//m_ListViewSR.SetColumnWidth(2, 100);
	m_ListViewSR.SetColumnWidth(0, 200);
	m_ListViewSR.SetColumnWidth(2, 100);

	int nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
	if (0 == nScrollWidth)
		nScrollWidth = 20;

	m_ListViewSR.SetColumnWidth(1, rcSize.Width()-300-nScrollWidth);

	return 0;
}

LRESULT CBKExamSafe_ShareResourceDlg::OnLDLibListViewReSize(CRect rcSize)
{
	if (rcSize.IsRectEmpty() || FALSE == ::IsWindow(m_ListLocalLib.m_hWnd))
		return 0;

	int nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
	if (0 == nScrollWidth)
		nScrollWidth = 20;

	m_ListLocalLib.SetColumnWidth(0, rcSize.Width()-nScrollWidth);

	return 0;
}

void CBKExamSafe_ShareResourceDlg::_InitCtrl()
{
	DWORD dwStyle = WS_CHILD|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_SINGLESEL|LVS_OWNERDRAWFIXED;
	HWND hWnd = GetViewHWND();
	if (NULL == hWnd)
		return ;
	//IE修复 对应的LISTVIEW
	m_ListViewSR.Create(hWnd, NULL, NULL, dwStyle, 0, IDC_EXAM_DLG_SR_LISTVIEW_ID, NULL);
	m_ListViewSR.SetExpandRectHand(TRUE);

	DWORD dwExStyle = LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT;
	dwExStyle |= m_ListViewSR.GetExtendedListViewStyle();
	m_ListViewSR.SetExtendedListViewStyle(dwExStyle);
    m_ListViewSR.InsertColumn(0, BkString::Get(IDS_EXAM_1006), LVCFMT_LEFT, 40); 
	m_ListViewSR.InsertColumn(1, BkString::Get(IDS_EXAM_1007), LVCFMT_LEFT, 120); 
	m_ListViewSR.InsertColumn(2, BkString::Get(IDS_EXAM_1008), LVCFMT_LEFT, 120); 
	m_ListViewSR.SetItemHeight(25);
	m_ListViewSR.SetNotifyHwnd(this->m_hWnd);
	m_ListViewSR.SetFocus();

	//显示本地白名单
	m_ListLocalLib.Create(hWnd, NULL, NULL, dwStyle, 0, IDC_EXAM_LIST_SR_ID, NULL);
	m_ListLocalLib.SetExpandRectHand(TRUE);

	dwExStyle |= m_ListLocalLib.GetExtendedListViewStyle();
	m_ListLocalLib.SetExtendedListViewStyle(dwExStyle);
	m_ListLocalLib.InsertColumn(0, BkString::Get(IDS_EXAM_1006), LVCFMT_LEFT, 40); 
//	m_ListLocalLib.InsertColumn(1, BkString::Get(IDS_EXAM_1008), LVCFMT_LEFT, 120); 
	m_ListLocalLib.SetItemHeight(25);
	m_ListLocalLib.SetNotifyHwnd(this->m_hWnd);
	m_ListLocalLib.SetFocus();

}

int CBKExamSafe_ShareResourceDlg::LoadShareResources()
{
	int nRet = -1;
	//m_arrRes = arrRes;
	m_arrRes.RemoveAll();
	m_ListLocalLib.DeleteAllItems();
	this->SetItemText(IDC_EXAM_BTN_SR_BELIEVE_ID, BkString::Get(IDS_EXAM_1317));
	this->SetItemVisible(IDC_EXAM_CTRL_TMP2_ID, TRUE);
	this->SetItemVisible(IDC_EXAM_CTRL_TMP_ID, TRUE);
	int nCount = 0;
	_Module.Exam.GetItemCount(BkSafeExamItem::EXAM_SHARE_RESOUCE, nCount);

	CString strValue;
	for (int i = 0; i < nCount; i++)
	{
		BKSafeExamItemEx::SHARE_RESOURCES* pRes = NULL;
		if (0 == _Module.Exam.EnumItemData(BkSafeExamItem::EXAM_SHARE_RESOUCE, i, (void**)(&pRes)))
		{
			if (NULL != pRes)
			{
				nRet = 0;
				m_arrRes.Add(*pRes);
			}
		}
	}

	return nRet;
}
int CBKExamSafe_ShareResourceDlg::ShowResources()
{
	int nRet = -1;

	int nCount = m_arrRes.GetSize();
	LPLDListViewCtrl_Item pItemData = NULL;
	LPLDListViewCtrl_SubItem pSubItemData = NULL;

	for (int i = 0; i < nCount; i++)
	{
		pItemData = new LDListViewCtrl_Item;
		pItemData->_bShowCheckBox = TRUE;
		//pItemData->_enumItemType = ITEM_TYPE_TITLE;
		pSubItemData = new LDListViewCtrl_SubItem;
		PRINTF_STR(pSubItemData->_pszSubItemInfo, m_arrRes[i].pszName);
		pSubItemData->_nGroupID = i;
		pItemData->PushSubItem(pSubItemData);//name

		pSubItemData = new LDListViewCtrl_SubItem;
		PRINTF_STR(pSubItemData->_pszSubItemInfo, m_arrRes[i].pszPath);
		pSubItemData->_nGroupID = i;
		pItemData->PushSubItem(pSubItemData);//path

		//op
		pSubItemData = new LDListViewCtrl_SubItem;
		PRINTF_STR(pSubItemData->_pszSubItemInfo, BkString::Get(IDS_EXAM_1003));
		pSubItemData->_colorUrlTxt = URL_COLOR2;
		pSubItemData->_nGroupID = i;
		pItemData->PushSubItem(pSubItemData);//operation

		if (FALSE == ::IsWindow(m_ListViewSR.m_hWnd))
			return nRet;
		m_ListViewSR.InsertItem(m_ListViewSR.GetItemCount(), pItemData);

		nRet = 0;
	}
	if (m_ListViewSR.GetItemCount() > 0)
		m_ListViewSR.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

	return nRet;
}

void CBKExamSafe_ShareResourceDlg::OnCheckAll()
{
	SetCheckAll(TRUE);
}

void CBKExamSafe_ShareResourceDlg::OnUnCheckAll()
{
	SetCheckAll(FALSE);
}

void CBKExamSafe_ShareResourceDlg::SetCheckAll(BOOL bCheck /* = TRUE */)
{
	if (0 == this->GetTabCurSel(IDC_EXAM_DLG_TAB))
	{
		int nCount = m_ListViewSR.GetItemCount();
		for (int i = 0; i < nCount; i++)
		{
			m_ListViewSR.SetCheckState(i, bCheck);
		}
		m_ListViewSR.Invalidate();
	}else if (1 == this->GetTabCurSel(IDC_EXAM_DLG_TAB))
	{
		int nCount = m_ListLocalLib.GetItemCount();
		for (int i = 0; i < nCount; i++)
		{
			m_ListLocalLib.SetCheckState(i, bCheck);
		}
		m_ListLocalLib.Invalidate();
	}
}

void CBKExamSafe_ShareResourceDlg::OnRemoveShareResources()
{
	int nCount = m_ListViewSR.GetItemCount();
	BOOL bFlag = FALSE;
	for (int i = nCount-1; i >= 0; i--)
	{
		if (TRUE == m_ListViewSR.GetCheckState(i))
		{
			bFlag = TRUE;
			if (0 == RemoveItem(i))
				m_ListViewSR.DeleteItem(i);
		}
	}

	if (FALSE == bFlag)
        ShowMsg(BkString::Get(IDS_EXAM_1009));
}

int CBKExamSafe_ShareResourceDlg::RemoveItem(int nSel /* = -1 */)
{
	int nRet = -1;
	if (nSel < 0|| nSel >= m_ListViewSR.GetItemCount())
		return nRet;

	LPLDListViewCtrl_Item pData = (LPLDListViewCtrl_Item)m_ListViewSR.GetItemData(nSel);
	if (NULL == pData)
		return nRet;

	LPLDListViewCtrl_SubItem pSubItemData = pData->_arrSubItem[0];
	if (NULL == pSubItemData || NULL == pSubItemData->_pszSubItemInfo)
		return nRet;

	CIniFile IniFile(m_strExamLogPath);
	BOOL bRember = TRUE;
	IniFile.GetIntValue(TEXT("showmsg"), TEXT("shareres"), bRember);
	if(TRUE == bRember)
	{
		CString strText(L"");
        //strText.Format(BkString::Get(IDS_EXAM_1010), pData->_arrSubItem[1]->_pszSubItemInfo);

		UINT nID = CBkSafeMsgBox2::ShowMultLine(BkString::Get(IDS_EXAM_1010), NULL, MB_OKCANCEL | MB_ICONWARNING, &bRember);
		if (TRUE == bRember)
			IniFile.SetIntValue(TEXT("showmsg"), TEXT("shareres"), 0);
		if (IDOK != nID)
			return nRet;
	}

	m_arrRes.RemoveAt(nSel);	
	_Module.Exam.FixItem(BkSafeExamItem::EXAM_SHARE_RESOUCE, pSubItemData->_pszSubItemInfo);
	nRet = 0;

	return nRet;
}

int CBKExamSafe_ShareResourceDlg::RemoveLocalLibItem(int nSel)
{
	int nRet = -1;
	if (nSel < 0|| nSel >= m_ListLocalLib.GetItemCount())
		return nRet;

	LPLDListViewCtrl_Item pData = (LPLDListViewCtrl_Item)m_ListLocalLib.GetItemData(nSel);
	if (NULL == pData || 1 != pData->_arrSubItem.GetSize())
		return nRet;

	LPLDListViewCtrl_SubItem pSubItemData = pData->_arrSubItem[0];
	if (NULL == pSubItemData || NULL == pSubItemData->_pszSubItemInfo)
		return nRet;


	nRet = _Module.Exam.RemoveLocalLibItem(BkSafeExamItem::EXAM_SHARE_RESOUCE, (void*)(pSubItemData->_pszSubItemInfo));

	return nRet;
}

int CBKExamSafe_ShareResourceDlg::GetShareResources(CSimpleArray<BKSafeExamItemEx::SHARE_RESOURCES>& arrRes)
{
	m_arrRes.RemoveAll();
	arrRes = m_arrRes;
	return m_arrRes.GetSize();

}

LRESULT CBKExamSafe_ShareResourceDlg::OnLDListViewClickUrl(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nRet = -1;

	DWORD dwCtrlID = HIWORD(lParam);
	int nUrlIndex = LOWORD(lParam);
	int nSel , nSubItem;//= int(wParam);
	nSel = LOWORD(wParam);
	nSubItem = HIWORD(wParam);
	if (nSel < 0)
		return nRet;

	if (IDC_EXAM_DLG_SR_LISTVIEW_ID == dwCtrlID)
	{
		if (0 == RemoveItem(nSel))
			m_ListViewSR.DeleteItem(nSel);
	}else if (IDC_EXAM_LIST_SR_ID == dwCtrlID)
	{
		if (0 == RemoveLocalLibItem(nSel))
			m_ListLocalLib.DeleteItem(nSel);
	}

	SetMsgHandled(FALSE);
	return 0;
}

void CBKExamSafe_ShareResourceDlg::OnBelieve()
{
	if (0 == this->GetTabCurSel(IDC_EXAM_DLG_TAB))
	{
		int nCount = m_ListViewSR.GetItemCount();
		BOOL bFlag = FALSE;
		for (int i = nCount-1; i >= 0; i--)
		{
			if (TRUE == m_ListViewSR.GetCheckState(i))
			{
				bFlag = TRUE;
				LPLDListViewCtrl_Item pItemData = (LPLDListViewCtrl_Item)(m_ListViewSR.GetItemData(i));
				if (NULL != pItemData)
				{
					LPTSTR pszValue = pItemData->_arrSubItem[1]->_pszSubItemInfo;
					if (NULL != pszValue)
					{
						_Module.Exam.SetItemSafe(BkSafeExamItem::EXAM_SHARE_RESOUCE, (void**)(&pszValue));
						m_ListViewSR.DeleteItem(i);
					}
				}
			}
		}

		if (FALSE == bFlag)
			ShowMsg(BkString::Get(IDS_EXAM_1011));
		if (0 == m_ListViewSR.GetItemCount())
		{
			_Module.Exam.FixItem(BkSafeExamItem::EXAM_SHARE_RESOUCE);
		}
	}else if (1 == this->GetTabCurSel(IDC_EXAM_DLG_TAB))
	{
		RemoveSelectItems();
	}
}

void CBKExamSafe_ShareResourceDlg::ShowMsg(LPCTSTR pszCaption)
{
	CBkSafeMsgBox2 dlg;
	CString strXml;
	strXml.Format(L"<text pos=\"0,10\">%s</text>", pszCaption);
	CRect rc(0, 0, 0, 0);
	dlg.AddButton(BkString::Get(18), IDOK);
	UINT_PTR uRet = dlg.ShowPanelMsg(strXml, &rc, NULL, MB_BK_CUSTOM_BUTTON | MB_ICONINFORMATION, NULL, GetViewHWND());
}

CString CBKExamSafe_ShareResourceDlg::_GetExamLogFilePath()
{
	CString strLogPath("");
	CAppPath::Instance().GetLeidianLogPath(strLogPath, TRUE);
	strLogPath += TEXT("\\exam.log");

	return strLogPath;
}

int CBKExamSafe_ShareResourceDlg::LoadSafeLibItem()
{
	int nRet = -1;
	int nCount = 0;
	m_ListLocalLib.DeleteAllItems();
	_Module.Exam.GetSafeLibItemCount(nCount);
	if (0 == nCount)
		return nRet;

	BkSafeExamLib::LocalLibExport lib;
	LPLDListViewCtrl_Item pItemData = NULL;
	LPLDListViewCtrl_SubItem pSubItemData = NULL;
	for (int i = 0; i < nCount; i++)
	{
		if (0 == _Module.Exam.EnumSafeLibItem(i, lib))
		{
			if (BkSafeExamItem::EXAM_SHARE_RESOUCE == lib.nType)
			{
				pItemData = new LDListViewCtrl_Item;
				pItemData->_bShowCheckBox = TRUE;
				pSubItemData = new LDListViewCtrl_SubItem;
				PRINTF_STR(pSubItemData->_pszSubItemInfo, lib.pszValue);
				pSubItemData->_nGroupID = i;
				pItemData->PushSubItem(pSubItemData);//name

				if (FALSE == ::IsWindow(m_ListLocalLib.m_hWnd))
					return nRet;
				m_ListLocalLib.InsertItem(m_ListLocalLib.GetItemCount(), pItemData);
			}
		}
		else
		{
			break;
		}
	}
	return nRet;
}

LRESULT CBKExamSafe_ShareResourceDlg::OnBKTabSwitch(int nTabItemIDOld, int nTabItemIDNew)
{
	switch(nTabItemIDNew)
	{
	case 0:
		{
			this->SetItemText(IDC_EXAM_TXT_TITLE, BkString::Get(IDS_EXAM_1311));
			m_ListViewSR.DeleteAllItems();
			LoadShareResources();
			ShowResources();

		}
		break;
	case 1:
		{
			//this->SetItemVisible(IDC_EXAM_BTN_SR_BELIEVE_ID, FALSE);
			this->SetItemText(IDC_EXAM_BTN_SR_BELIEVE_ID, BkString::Get(IDS_EXAM_1316));
			this->SetItemVisible(IDC_EXAM_CTRL_TMP2_ID, FALSE);
			this->SetItemVisible(IDC_EXAM_CTRL_TMP_ID, FALSE);
			this->SetItemText(IDC_EXAM_TXT_TITLE, BkString::Get(IDS_EXAM_1312));
			LoadSafeLibItem();
		}
		break;
	default:
		break;
	}
	return 1;
}

void CBKExamSafe_ShareResourceDlg::RemoveSelectItems()
{
	int nCount = 0;
	BOOL bFlag = FALSE;
	if (0 == this->GetTabCurSel(IDC_EXAM_DLG_TAB))
	{
		nCount = m_ListViewSR.GetItemCount();
		for ( int i = nCount-1; i >= 0; i--)
		{
			if (FALSE == m_ListViewSR.GetCheckState(i))
				continue;
			bFlag = TRUE;
			if (0 == RemoveItem(i))
				m_ListViewSR.DeleteItem(i);
		}
		if (FALSE == bFlag)
			ShowMsg(BkString::Get(IDS_EXAM_1009));
	}else if (1 == this->GetTabCurSel(IDC_EXAM_DLG_TAB))
	{
		nCount = m_ListLocalLib.GetItemCount();
		for ( int i = nCount-1; i >= 0; i--)
		{
			if (FALSE == m_ListLocalLib.GetCheckState(i))
				continue;
			if (0 == RemoveLocalLibItem(i))
				m_ListLocalLib.DeleteItem(i);
			bFlag = TRUE;
		}

		if (FALSE == bFlag)
			ShowMsg(BkString::Get(IDS_EXAM_1315));

	}
}