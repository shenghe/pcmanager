#include "StdAfx.h"
#include "bksafeexam_showhostsfile.h"
#include "beikesafemsgbox.h"
#include "iefix/cinifile.h"
#include "strsafe.h"
using namespace IniFileOperate;

#define URL_COLOR RGB(0, 128, 0)
#define URL_COLOR2 RGB(77,113,149)
#define PRINTF_STR(x,y){_sntprintf_s((x), sizeof((x)), TEXT("%s"), (y));}
#define PRINTF_STR2(x,f,y){_sntprintf_s((x), sizeof((x)), (f), (y));}
#define PRINTF_INT(x,y){_sntprintf_s((x), sizeof((x)), TEXT("%d"), (y));}



BOOL IsChar(TCHAR ch)
{
	BOOL bRet = FALSE;
	if ((ch >= L'A' && ch <= L'Z') ||
		(ch >= L'a' && ch <= L'z'))
		bRet = TRUE;

	return bRet;
}

BOOL IsNum(TCHAR ch)
{
	BOOL bRet = FALSE;
	if (ch >= L'0' && ch <= L'9')
		bRet = TRUE;

	return bRet;
}

CBKSafeExam_ShowHostsFileDlg::~CBKSafeExam_ShowHostsFileDlg(void)
{

};

BOOL CBKSafeExam_ShowHostsFileDlg::OnInitDialog(HWND wParam, LPARAM lParam)
{
	CAppPath::Instance().GetLeidianLogPath(m_strExamLogPath, TRUE);
	m_strExamLogPath += TEXT("\\exam.log");

	InitCtrl();
	LoadHostItem();
	this->SetTabCurSel(IDC_EXAM_DLG_TAB, m_nDefSel);
	return TRUE;
}

LRESULT CBKSafeExam_ShowHostsFileDlg::OnLDListViewReSize(CRect rcSize)
{
	if (FALSE == ::IsWindow(m_ListHosts.m_hWnd) || rcSize.IsRectEmpty())
		return -1;

	m_ListHosts.SetColumnWidth(0, 200);
	m_ListHosts.SetColumnWidth(2, 100);
	int nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
	if (0 == nScrollWidth)
		nScrollWidth = 20;

	m_ListHosts.SetColumnWidth(1, rcSize.Width()-300-nScrollWidth);


	return 0;
}

LRESULT CBKSafeExam_ShowHostsFileDlg::OnLDLibListViewReSize(CRect rcSize)
{
	if (rcSize.IsRectEmpty() || FALSE == ::IsWindow(m_ListLocalLib.m_hWnd))
		return 0;

	m_ListLocalLib.SetColumnWidth(0, 200);
//	m_ListLocalLib.SetColumnWidth(2, 100);
	int nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
	if (0 == nScrollWidth)
		nScrollWidth = 20;

	m_ListLocalLib.SetColumnWidth(1, rcSize.Width()-200-nScrollWidth);

	return 0;
}

void CBKSafeExam_ShowHostsFileDlg::InitCtrl()
{
	DWORD dwStyle = WS_CHILD|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_SINGLESEL|LVS_OWNERDRAWFIXED;
	HWND hWnd = GetViewHWND();
	if (NULL == hWnd)
		return ;
	//IE修复 对应的LISTVIEW
	m_ListHosts.Create(hWnd, NULL, NULL, dwStyle, 0, IDC_EXAM_LIST_HOSTS_ID, NULL);
	m_ListHosts.SetExpandRectHand(TRUE);

	DWORD dwExStyle = LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT;
	dwExStyle |= m_ListHosts.GetExtendedListViewStyle();
	m_ListHosts.SetExtendedListViewStyle(dwExStyle);
    m_ListHosts.InsertColumn(0, BkString::Get(IDS_EXAM_1000), LVCFMT_LEFT, 40); 
	m_ListHosts.InsertColumn(1, BkString::Get(IDS_EXAM_1001), LVCFMT_LEFT, 120); 
	m_ListHosts.InsertColumn(2, BkString::Get(IDS_EXAM_1002), LVCFMT_LEFT, 120); 
	m_ListHosts.SetItemHeight(25);
	m_ListHosts.SetNotifyHwnd(this->m_hWnd);
	m_ListHosts.SetFocus();

	//显示本地白名单
	m_ListLocalLib.Create(hWnd, NULL, NULL, dwStyle, 0, IDC_EXAM_LIST_SR_ID, NULL);
	m_ListLocalLib.SetExpandRectHand(TRUE);

	dwExStyle |= m_ListLocalLib.GetExtendedListViewStyle();
	m_ListLocalLib.SetExtendedListViewStyle(dwExStyle);
	m_ListLocalLib.InsertColumn(0, BkString::Get(IDS_EXAM_1000), LVCFMT_LEFT, 40); 
	m_ListLocalLib.InsertColumn(1, BkString::Get(IDS_EXAM_1001), LVCFMT_LEFT, 120); 
//	m_ListLocalLib.InsertColumn(2, BkString::Get(IDS_EXAM_1002), LVCFMT_LEFT, 120); 
	m_ListLocalLib.SetItemHeight(25);
	m_ListLocalLib.SetNotifyHwnd(this->m_hWnd);
	m_ListLocalLib.SetFocus();
}

HRESULT CBKSafeExam_ShowHostsFileDlg::OnLDListViewClickUrl(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nRet = -1;

	DWORD dwCtrlID = HIWORD(lParam);
	int nUrlIndex = LOWORD(lParam);
	int nSel , nSubItem;//= int(wParam);
	nSel = LOWORD(wParam);
	nSubItem = HIWORD(wParam);
	if (nSel < 0)
		return nRet;

	if (IDC_EXAM_LIST_HOSTS_ID == dwCtrlID)
	{
		if (0 == FixHostItem(nSel))
			m_ListHosts.DeleteItem(nSel);
	}else if (IDC_EXAM_LIST_SR_ID == dwCtrlID)
	{
		if (0 == RemoveLocalLibItem(nSel))
			m_ListLocalLib.DeleteItem(nSel);
	}

	SetMsgHandled(FALSE);
	return 0;
}

void CBKSafeExam_ShowHostsFileDlg::OnCheckAll()
{
	SetCheckAll(TRUE);
}

void CBKSafeExam_ShowHostsFileDlg::OnUnCheckAll()
{
	SetCheckAll(FALSE);
}

void CBKSafeExam_ShowHostsFileDlg::SetCheckAll(BOOL bCheck /* = TRUE */)
{
	if (0 == this->GetTabCurSel(IDC_EXAM_DLG_TAB))
	{
		int nCount = m_ListHosts.GetItemCount();
		for (int i = 0; i < nCount; i++)
		{
			m_ListHosts.SetCheckState(i, bCheck);
		}
		m_ListHosts.Invalidate();

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

int CBKSafeExam_ShowHostsFileDlg::LoadHostItem()
{
	int nCount = 0;
	this->SetItemText(IDC_EXAM_BTN_HOSTS_BELIEVE_ID, BkString::Get(IDS_EXAM_1317));
	this->SetItemVisible(IDC_EXAM_CTRL_TMP2_ID, TRUE);
	this->SetItemVisible(IDC_EXAM_CTRL_TMP_ID, TRUE);

	_Module.Exam.GetItemCount(BkSafeExamItem::EXAM_HOSTS_FILE, nCount);
	BkSafeExamLib::HostFileItem HostItem;
	LPVOID pData = NULL;
	for (int i = 0; i < nCount; i++)
	{
		if (S_OK == _Module.Exam.EnumItemData(BkSafeExamItem::EXAM_HOSTS_FILE, i, &pData))
		{
			HostItem = *((BkSafeExamLib::HostFileItem*)(pData));
			ShowHostItem(HostItem);
		}
	}
	if (m_ListHosts.GetItemCount() > 0)
		m_ListHosts.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	return 0;
}

void CBKSafeExam_ShowHostsFileDlg::ShowHostItem(BkSafeExamLib::HostFileItem& HostItem)
{
	int nRet = -1;

	if (TRUE == HostItem.strHostItem.IsEmpty() || 0 == HostItem.nLevel)
		return;

	LPLDListViewCtrl_Item pItemData = NULL;
	LPLDListViewCtrl_SubItem pSubItemData = NULL;
	CString strIP, strName;
	ParseHostItem(HostItem.strHostItem, strIP, strName);
	{
		pItemData = new LDListViewCtrl_Item;
		pItemData->_uGroup = HostItem.nItemID;
		pItemData->_bShowCheckBox = TRUE;
		//pItemData->_enumItemType = ITEM_TYPE_TITLE;
		pSubItemData = new LDListViewCtrl_SubItem;
		PRINTF_STR(pSubItemData->_pszSubItemInfo, strIP);
		pSubItemData->_nGroupID = HostItem.nItemID;
		pItemData->PushSubItem(pSubItemData);//name

		pSubItemData = new LDListViewCtrl_SubItem;
		PRINTF_STR(pSubItemData->_pszSubItemInfo, strName);
		pSubItemData->_nGroupID = HostItem.nItemID;
		pItemData->PushSubItem(pSubItemData);//path

		//op
		pSubItemData = new LDListViewCtrl_SubItem;
		PRINTF_STR(pSubItemData->_pszSubItemInfo, BkString::Get(IDS_EXAM_1314));
		pSubItemData->_colorUrlTxt = URL_COLOR2;
		pSubItemData->_nGroupID = HostItem.nItemID;
		pItemData->PushSubItem(pSubItemData);//path


		if (FALSE == ::IsWindow(m_ListHosts.m_hWnd))
			return ;
		m_ListHosts.InsertItem(m_ListHosts.GetItemCount(), pItemData);

		nRet = 0;
	}
}

void CBKSafeExam_ShowHostsFileDlg::OnBelieve()
{
	if (0 == this->GetTabCurSel(IDC_EXAM_DLG_TAB))
	{
		int nCount = m_ListHosts.GetItemCount();
		BOOL bFlag = FALSE;
		for (int i = nCount-1; i >= 0; i--)
		{
			if (TRUE == m_ListHosts.GetCheckState(i))
			{
				LPLDListViewCtrl_Item pItemData = (LPLDListViewCtrl_Item)m_ListHosts.GetItemData(i);
				if (NULL != pItemData)
				{
					//_Module.Exam.set
					BkSafeExamLib::HostFileItem HostItem;
					HostItem.nItemID = pItemData->_uGroup;
					HostItem.nLevel = 0;
					HostItem.strHostItem.Format(TEXT("\"%s\",\"%s\""), pItemData->_arrSubItem[0]->_pszSubItemInfo, pItemData->_arrSubItem[1]->_pszSubItemInfo);
					BkSafeExamLib::HostFileItem* pData = &HostItem;
					_Module.Exam.SetItemSafe(BkSafeExamItem::EXAM_HOSTS_FILE, (void**)(&pData));
					m_ListHosts.DeleteItem(i);
					bFlag = TRUE;
				}
			}
		}

		if (FALSE == bFlag)
			ShowMsg(BkString::Get(IDS_EXAM_1004));
	}else if (1 == this->GetTabCurSel(IDC_EXAM_DLG_TAB))
	{
		RemoveSelectItems();
	}

}

void CBKSafeExam_ShowHostsFileDlg::OnUnBelieve()
{
	BOOL bFlag = FALSE;
	int nCount = m_ListHosts.GetItemCount();
	for (int i = nCount-1; i >= 0; i--)
	{
		if (TRUE == m_ListHosts.GetCheckState(i))
		{
			if (0 == FixHostItem(i))
			{
				m_ListHosts.DeleteItem(i);
			}
			bFlag = TRUE;
		}
	}
	if (FALSE == bFlag)
		ShowMsg(BkString::Get(IDS_EXAM_1004));

}

int CBKSafeExam_ShowHostsFileDlg::ParseHostItem(CString strHostItem, CString& strIP, CString& strName)
{
	int nRet = -1;
	if (TRUE == strHostItem.IsEmpty())
		return nRet;

	CString ip, name;
	int nCount = strHostItem.GetLength();
	
	for (int i = 0; i < nCount; i++)
	{
		if (TRUE == IsNum(strHostItem[i]) ||
			L'.' == strHostItem[i])
		{
			strIP += strHostItem[i];
		}
		else
		{
			break;
		}
	}
	if (FALSE == strIP.IsEmpty())
	{
		strName = strHostItem;
		strName.Replace(strIP, L"");
		strName.Replace(L"\t", L"");
		strName.TrimLeft();
		nRet = 0;
	}

	return nRet;
}


void CBKSafeExam_ShowHostsFileDlg::ShowMsg(LPCTSTR pszCaption)
{
	CBkSafeMsgBox2 dlg;
	dlg.ShowMsg(pszCaption, NULL, MB_OK|MB_ICONINFORMATION, NULL, GetViewHWND());
}

int CBKSafeExam_ShowHostsFileDlg::FixHostItem(int nSel /* = -1 */)
{
	int nRet = -1;
	if (nSel < 0 || nSel >= m_ListHosts.GetItemCount())
		return nRet;

		LPLDListViewCtrl_Item pItemData = (LPLDListViewCtrl_Item)m_ListHosts.GetItemData(nSel);
		if (NULL != pItemData)
		{
			CIniFile IniFile(m_strExamLogPath);
			BOOL bRember = TRUE;
			IniFile.GetIntValue(TEXT("showmsg"), TEXT("host"), bRember);
			if(TRUE == bRember)
			{
				CString strText(L"");
                strText.Format(BkString::Get(IDS_EXAM_1005), pItemData->_arrSubItem[0]->_pszSubItemInfo, pItemData->_arrSubItem[1]->_pszSubItemInfo);

				UINT nID = CBkSafeMsgBox::Show(strText, NULL, MB_OKCANCEL | MB_ICONWARNING, &bRember);
				if (TRUE == bRember)
					IniFile.SetIntValue(TEXT("showmsg"), TEXT("host"), 0);
				if (IDOK != nID)
					return nRet;
			}

			_Module.Exam.SetHostItemFix(pItemData->_uGroup);
			nRet = 0;
		}

	return nRet;
}

int CBKSafeExam_ShowHostsFileDlg::ParsexmlCmdLine(LPCTSTR pszCmdLine, CAtlArray<CString>& arrStr)
{
	int nRet = -1;
	if (NULL == pszCmdLine)
		return nRet;

	arrStr.RemoveAll();
	LPTSTR _pszCmdLine = StrDupW(pszCmdLine);
	if (NULL == _pszCmdLine)
		return nRet;

	LPTSTR pValue = _pszCmdLine;

	LPTSTR pos = NULL;
	LPTSTR NewPos = NULL;
	NewPos = StrStr(_pszCmdLine, TEXT("\","));
	while(NULL != NewPos && _tcslen(NewPos) > 2)
	{
		pos = NewPos+2;
		*NewPos = 0;
		_pszCmdLine += 1;
		arrStr.Add((_pszCmdLine));
		_pszCmdLine = pos;
		NewPos = StrStr(_pszCmdLine, TEXT("\","));
	}
	if (NULL != _pszCmdLine && _tcslen(_pszCmdLine) > 1)
	{
		_pszCmdLine += 1;
		pos = StrRChr(_pszCmdLine, NULL, TEXT('\"'));
		if (NULL != pos)
			*pos = 0;
		arrStr.Add(_pszCmdLine);
	}

	nRet = 0;
	if (NULL != pValue)
	{
		LocalFree(pValue);
		pValue = NULL;
	}
	return nRet;
}
int CBKSafeExam_ShowHostsFileDlg::LoadSafeLibItem()
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
	CAtlArray<CString> arrItem;
	for (int i = 0; i < nCount; i++)
	{
		if (0 == _Module.Exam.EnumSafeLibItem(i, lib))
		{
			arrItem.RemoveAll();
			if (BkSafeExamItem::EXAM_HOSTS_FILE == lib.nType &&
				0 == ParsexmlCmdLine(lib.pszValue, arrItem))
			{
				pItemData = new LDListViewCtrl_Item;
				pItemData->_bShowCheckBox = TRUE;
				pSubItemData = new LDListViewCtrl_SubItem;
				PRINTF_STR(pSubItemData->_pszSubItemInfo, arrItem[0]);
				pSubItemData->_nGroupID = i;
				pItemData->PushSubItem(pSubItemData);//ip
				//host
				pSubItemData = new LDListViewCtrl_SubItem;
				PRINTF_STR(pSubItemData->_pszSubItemInfo, arrItem[1]);
				pSubItemData->_nGroupID = i;
				pItemData->PushSubItem(pSubItemData);//ip
				//op
				pSubItemData = new LDListViewCtrl_SubItem;
				PRINTF_STR(pSubItemData->_pszSubItemInfo, BkString::Get(IDS_EXAM_1308));
				pSubItemData->_colorUrlTxt = URL_COLOR2;
				pSubItemData->_nGroupID = i;
				pItemData->PushSubItem(pSubItemData);//operation

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

LRESULT CBKSafeExam_ShowHostsFileDlg::OnBKTabSwitch(int nTabItemIDOld, int nTabItemIDNew)
{
	switch(nTabItemIDNew)
	{
	case 0:
		{
			this->SetItemText(IDC_EXAM_TXT_TITLE, BkString::Get(IDS_EXAM_1309));
			m_ListHosts.DeleteAllItems();
			LoadHostItem();
		}
		break;
	case 1:
		{
			this->SetItemText(IDC_EXAM_BTN_HOSTS_BELIEVE_ID, BkString::Get(IDS_EXAM_1316));
			this->SetItemVisible(IDC_EXAM_CTRL_TMP2_ID, FALSE);
			this->SetItemVisible(IDC_EXAM_CTRL_TMP_ID, FALSE);
			this->SetItemText(IDC_EXAM_TXT_TITLE, BkString::Get(IDS_EXAM_1310));
			LoadSafeLibItem();
		}
		break;
	default:
		break;
	}
	return 1;
}
int CBKSafeExam_ShowHostsFileDlg::RemoveLocalLibItem(int nSel)
{
	int nRet = -1;
	if (nSel < 0|| nSel >= m_ListLocalLib.GetItemCount())
		return nRet;

	LPLDListViewCtrl_Item pData = (LPLDListViewCtrl_Item)m_ListLocalLib.GetItemData(nSel);
	if (NULL == pData || 3 != pData->_arrSubItem.GetSize())
		return nRet;

	LPLDListViewCtrl_SubItem pSubItemData = pData->_arrSubItem[0];
	LPLDListViewCtrl_SubItem pSubItemData2 = pData->_arrSubItem[1];
	if (NULL == pSubItemData || NULL == pSubItemData->_pszSubItemInfo||
		NULL == pSubItemData2 || NULL == pSubItemData2->_pszSubItemInfo)
		return nRet;

	BkSafeExamLib::HostFileItem HostItem;
	HostItem.nItemID = pData->_uGroup;
	HostItem.nLevel = 3;
	HostItem.strHostItem.Format(L"\"%s\",\"%s\"", pSubItemData->_pszSubItemInfo, pSubItemData2->_pszSubItemInfo);
// 	BkSafeExamLib::HostFileItem* pHostData = &HostItem;
// 
// 	TCHAR pszLib[1024] = {0};
// 	StringCbPrintf(pszLib, sizeof(pszLib), L"\"%s\",\"%s\"", pSubItemData->_pszSubItemInfo, pSubItemData2->_pszSubItemInfo);

	_Module.Exam.RemoveLocalLibItem(BkSafeExamItem::EXAM_HOSTS_FILE, (void*)&HostItem);
	nRet = 0;

	return nRet;
}

void CBKSafeExam_ShowHostsFileDlg::RemoveSelectItems()
{
	int nCount = 0;
	BOOL bFlag = FALSE;
	if (0 == this->GetTabCurSel(IDC_EXAM_DLG_TAB))
	{
		nCount = m_ListHosts.GetItemCount();
		for ( int i = nCount-1; i >= 0; i--)
		{
			if (FALSE == m_ListHosts.GetCheckState(i))
				continue;
			bFlag = TRUE;
			if (0 == FixHostItem(i))
				m_ListHosts.DeleteItem(i);
		}
		if (FALSE == bFlag)
			ShowMsg(BkString::Get(IDS_EXAM_1004));
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
			ShowMsg(BkString::Get(IDS_EXAM_1313));

	}
}