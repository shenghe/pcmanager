#pragma once
#include <wtlhelper/whwindow.h>
#include "kuires.h"
#include "ldlistviewctrl.h"
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include "bigfiledoingdlg.h"
#include "bigfilemovedlg.h"
#include "kscbase/kscfilepath.h"
#include "bigfiledelleftdlg.h"

struct OperItemData 
{
	int nType;
	ULONGLONG nSize;
	CString strName;
	CString strInfo;
	CString strTime;
	CString strScrPath;
	CString strDestPath;
};


typedef struct _MoveHistory {
	CString strPathNow;
	CString strPathOriginal;
	SYSTEMTIME moveTime;


	_MoveHistory& operator = (const _MoveHistory& history)
	{

		this->strPathOriginal = history.strPathOriginal;
		this->strPathNow = history.strPathNow;
		memcpy(&this->moveTime, &history.moveTime, sizeof(history.moveTime));
		return *this;
	}

} MoveHistory;
class CKscShowDestDir
	: public CKuiDialogImpl<CKscShowDestDir>
	, public CWHRoundRectFrameHelper<CKscShowDestDir>
{
public:
	CKscShowDestDir(CSimpleArray<OperItemData>& listoper, CAtlList<MoveHistory>& history, int nFlag)
		: CKuiDialogImpl<CKscShowDestDir>("IDR_DLG_SHOW_DEST_DIR")
	{
		if (nFlag == 1)
		{
			m_Arraylist.RemoveAll();
			m_Arraylist = listoper;
		}
		else if ( nFlag == 0)
		{
			m_Movelist.RemoveAll();
			POSITION pos = NULL;

			pos = history.GetHeadPosition();
			while (pos)
			{
				const MoveHistory& _history = history.GetNext(pos);
				m_Movelist.AddTail(_history);
			}

		}
		m_nFlag = nFlag;
		m_bDelToRecy = TRUE;
		m_bDelDirect = FALSE;
	}

	~CKscShowDestDir(void){}

	BOOL m_bDelToRecy;
	BOOL m_bDelDirect;

protected:
	CLDListViewCtrl         m_List;
	CSimpleArray<OperItemData> m_Arraylist;
	CAtlList<MoveHistory> m_Movelist;
	int m_nFlag;

	CSimpleArray<CString> m_arrayDel;
	CString m_strDestDir;

public:

	void OnBkBtnClose()
	{
		EndDialog(0);
	}

	BOOL IsNotRecycle()
	{
		if (m_bDelToRecy)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
	
	void OnBkBtnOk()
	{
		int nCount = 0;
		nCount = m_List.GetItemCount();
		for (int i = 0; i< nCount; i++)
		{
			if (m_List.GetCheckState(i))
			{
				LPLDListViewCtrl_Item itemData = (LPLDListViewCtrl_Item)m_List.GetItemData(i);
//				DeleteFile(itemData->_pszExInfo);
				m_arrayDel.Add(itemData->_pszExInfo);
//				m_List.DeleteItem(i);
			}
		}
		EndDialog(0);
	}
	void GetDelFile(CSimpleArray<CString>& listDel)
	{
		listDel = m_arrayDel;
	}
	void OnBkBtnClear()
	{
		return;
	}

	void OnBkRadioToRecy()
	{
		m_bDelToRecy = TRUE;
		m_bDelDirect = FALSE;
	}

	void OnBkRadioToDel()
	{
		m_bDelToRecy = FALSE;
		m_bDelDirect = TRUE;
	}

	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{
		HWND hWnd = GetViewHWND();
		if (NULL == hWnd)
			return FALSE;
		DWORD dwStyle = WS_CHILD|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_SINGLESEL|LVS_OWNERDRAWFIXED;
		m_List.Create(hWnd, NULL, NULL, dwStyle, 0, 4420, NULL);
		m_List.SetExpandRectHand(TRUE);

		DWORD dwExStyle = LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT;
		dwExStyle |= m_List.GetExtendedListViewStyle();
		m_List.SetExtendedListViewStyle(dwExStyle);
		m_List.SetItemHeight(28);
		m_List.SetNotifyHwnd(this->m_hWnd);
		int nFlag = m_nFlag;
		SetColumnInfo(nFlag);
		m_List.SetFocus();
		m_List.SetFont(KuiFontPool::GetFont(FALSE, FALSE, FALSE));

		if (nFlag == 1)
		{
			SetItemText(4428, L"删除文件确认");
			SetItemVisible(2, TRUE);
			SetItemVisible(4425, TRUE);
			SetItemVisible(4426, TRUE);
			SetItemVisible(4424, TRUE);
			SetItemVisible(4427, FALSE);
			SetItemVisible(3, TRUE);
			SetItemVisible(4, TRUE);
			SetItemVisible(5, TRUE);
			SetItemCheck(3, TRUE);
			SetItemText(4424, L"下列勾选的文件即将被删除，你确认要删除这些文件吗？");

			SetItemText(2, L"确认删除");
			SetItemText(1, L"取消");
			InsertDeleteItemEx(m_Arraylist);
		}

		if (nFlag == 0)
		{
			SetItemText(4428, L"查看迁移历史");
			SetItemVisible(2, FALSE);
			SetItemVisible(4425, FALSE);
			SetItemVisible(4426, FALSE);
			SetItemVisible(4424, FALSE);
			SetItemVisible(4427, TRUE);
			SetItemVisible(3, FALSE);
			SetItemVisible(4, FALSE);
			SetItemVisible(5, FALSE);
			SetItemText(1, L"返回");
			InsertMoveItemEx(m_Movelist);
		}
		
		return TRUE;
	}

	void InsertMoveItemEx(CAtlList<MoveHistory>& itemEx)
	{
		POSITION pos = NULL;

		pos = itemEx.GetHeadPosition();
		while (pos)
		{
			MoveHistory _history = itemEx.GetNext(pos);
			LPLDListViewCtrl_Item pItemData = NULL;
			LPLDListViewCtrl_SubItem pSubItemData = NULL;
			WCHAR szDestPath[MAX_PATH] = {0};
			WCHAR szSrcPath[MAX_PATH] = {0};
			int nPos = -1;
			CString strInfo, strName, strSrcPath, strTime, strCurrentPath;
//			strInfo = _history.strInfo;
			strTime.Format(_T("%d.%d.%d"), _history.moveTime.wYear, _history.moveTime.wMonth, _history.moveTime.wDay);
			strCurrentPath = _history.strPathNow; 

			DWORD dwAttr = GetFileAttributes(strCurrentPath);
			if (dwAttr == INVALID_FILE_ATTRIBUTES)
			{
                _DeleteMoveHistory(strCurrentPath);
				continue;
			}
//			strName = itemInfo.strName;
			strSrcPath = _history.strPathOriginal;


			nPos = strCurrentPath.ReverseFind('\\');

			strName = strCurrentPath.Right(strCurrentPath.GetLength() - nPos - 1);

			nPos = strName.ReverseFind('.');
			if (nPos != -1)
			{
				strInfo = strName.Right(strName.GetLength() - nPos -1);
			}

			wcscpy(szSrcPath, strSrcPath);
			wcscpy(szDestPath, strCurrentPath);
			PathRemoveFileSpec(szDestPath);
			PathRemoveFileSpec(szSrcPath);

			pItemData = new LDListViewCtrl_Item;
			pItemData->_uGroup = 0;
			pItemData->_bShowCheckBox = FALSE;
			pItemData->_bCheck = FALSE;
			PRINTF_STR(pItemData->_pszExInfo, strCurrentPath);
			PRINTF_STR(pItemData->_pszExInfo2, strSrcPath);

			pSubItemData = new LDListViewCtrl_SubItem;
			PRINTF_STR(pSubItemData->_pszSubItemInfo, strName);
			pSubItemData->_pszSubItemInfoBackup = strCurrentPath;
			pSubItemData->_nGroupID = 0;
			pSubItemData->_enumType = SUBITEM_TYPE_WITH_IMAGE_EX;
			pItemData->PushSubItem(pSubItemData);//name

			//pSubItemData = new LDListViewCtrl_SubItem;
			//PRINTF_STR(pSubItemData->_pszSubItemInfo, strInfo);
			//pSubItemData->_nGroupID = 0;
			//pItemData->PushSubItem(pSubItemData);//path

			pSubItemData = new LDListViewCtrl_SubItem;
			PRINTF_STR(pSubItemData->_pszSubItemInfo, strTime);
			pSubItemData->_nGroupID = 0;
			pItemData->PushSubItem(pSubItemData);//path

		
			pSubItemData = new LDListViewCtrl_SubItem;
			PRINTF_STR(pSubItemData->_pszSubItemInfo, szSrcPath);
			pSubItemData->_nGroupID = 0;
			pItemData->PushSubItem(pSubItemData);//path

			pSubItemData = new LDListViewCtrl_SubItem;
			PRINTF_STR(pSubItemData->_pszSubItemInfo, szDestPath);
			pSubItemData->_nGroupID = 0;
			pItemData->PushSubItem(pSubItemData);//path

			pSubItemData = new LDListViewCtrl_SubItem;
			PRINTF_STR(pSubItemData->_pszSubItemInfo, L"<URL>打开文件夹|撤销迁移</URL>");
			pSubItemData->_colorUrlTxt = URL_COLOR2;
			pSubItemData->_nGroupID = 0;
			//pSubItemData->_enumType = SUBITEM_TYPE_BUTTON;
			pItemData->PushSubItem(pSubItemData);//path
			if (FALSE == ::IsWindow(m_List.m_hWnd))
				return ;
			m_List.InsertItem(m_List.GetItemCount(), pItemData);

		}
		return ;
	}

	void InsertDeleteItemEx(CSimpleArray<OperItemData>& itemEx)
	{
		int nCount = itemEx.GetSize();
		for (int n = 0; n < nCount; n++)
		{
			OperItemData itemInfo;
			itemInfo = itemEx[n];
			LPLDListViewCtrl_Item pItemData = NULL;
			LPLDListViewCtrl_SubItem pSubItemData = NULL;
			CString strInfo, strName, strFullPath;
			strInfo = itemInfo.strInfo;
			strName = itemInfo.strName;
			strFullPath = itemInfo.strScrPath;
			//	strSize.Format(_T("<TI>%s|%d</TI>"), itemInfo.strSize, itemInfo.nPercent);

			WCHAR szPathRe[MAX_PATH] = {0};
			wcscpy(szPathRe, strFullPath);
			PathRemoveFileSpec(szPathRe);

			pItemData = new LDListViewCtrl_Item;
			pItemData->_uGroup = 0;
			pItemData->_bShowCheckBox = TRUE;
			pItemData->_bCheck = TRUE;
			pItemData->nSize = itemInfo.nSize;
			PRINTF_STR(pItemData->_pszExInfo, itemInfo.strScrPath);

			pSubItemData = new LDListViewCtrl_SubItem;
			PRINTF_STR(pSubItemData->_pszSubItemInfo, strName);
			pSubItemData->_pszSubItemInfoBackup = itemInfo.strScrPath;
			pSubItemData->_nGroupID = 0;
			pSubItemData->_enumType = SUBITEM_TYPE_WITH_IMAGE_EX;
			pItemData->PushSubItem(pSubItemData);//name

			pSubItemData = new LDListViewCtrl_SubItem;
			PRINTF_STR(pSubItemData->_pszSubItemInfo, strInfo);
			pSubItemData->_nGroupID = 0;
			pSubItemData->nSize = itemInfo.nSize;
			pItemData->PushSubItem(pSubItemData);//path

			pSubItemData = new LDListViewCtrl_SubItem;
			PRINTF_STR(pSubItemData->_pszSubItemInfo, szPathRe);
			pSubItemData->_nGroupID = 0;
			pItemData->PushSubItem(pSubItemData);//path
				
			if (FALSE == ::IsWindow(m_List.m_hWnd))
				return ;
			m_List.InsertItem(m_List.GetItemCount(), pItemData);
			m_List.SetColumnWidth(2, 350);
			m_List.SetColumnWidth(0, 150);
		}
		return;
	}

	void SetColumnInfo(int nFlag)
	{
		if (nFlag == 0)
		{
			m_List.InsertColumn(0, L"文件名", LVCFMT_LEFT, 130); 
			//m_List.InsertColumn(1, L"文件格式", LVCFMT_LEFT, 60); 
			m_List.InsertColumn(1, L"迁移时间", LVCFMT_LEFT, 80); 
			m_List.InsertColumn(2, L"原始路径", LVCFMT_LEFT, 170);
			m_List.InsertColumn(3, L"目标路径", LVCFMT_LEFT, 150);
			m_List.InsertColumn(4, L"操作", LVCFMT_LEFT, 130 );
		}
		else if (nFlag == 1)
		{
			m_List.InsertColumn(0, L"文件名", LVCFMT_LEFT, 180); 
			m_List.InsertColumn(1, L"文件大小", LVCFMT_LEFT, 100);  
			m_List.InsertColumn(2, L"原始路径", LVCFMT_LEFT, 500);
		}
	}

	HRESULT OnLDListViewClickUrl(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int nRet = -1;

		DWORD dwIndex = LOWORD(wParam);
		DWORD dwOper = LOWORD(lParam);
		CString strFullPath;

		int nSel , nSubItem;//= int(wParam);
		nSel = LOWORD(wParam);
		nSubItem = HIWORD(wParam);
		if (nSel < 0)
			return nRet;
		LPLDListViewCtrl_Item itemData;
		itemData = (LPLDListViewCtrl_Item)m_List.GetItemData(dwIndex);
		if (itemData->_pszExInfo == NULL)
		{
			return 1;
		}
		
		if ( dwOper == 0)
		{
			WCHAR strDir[MAX_PATH] = {0};
			wcscpy(strDir, itemData->_pszExInfo);
			m_List.LocateFilePath(strDir);
		/*	::PathRemoveFileSpec(strDir); 
			ShellExecute( m_hWnd, 
				_T("open"), 
				strDir, 
				NULL, NULL, SW_SHOWNORMAL );*/
		}
		else if (dwOper == 1)
		{
			CString strSrc;
			CString strDest;
			strSrc = itemData->_pszExInfo2;
			strDest = itemData->_pszExInfo;
			ULONGLONG qwSize = 0;
			HANDLE hFileSrc = INVALID_HANDLE_VALUE;
			DWORD dwSizeHight;
			DWORD dwSizeLow;
			CMoveJob job;
			hFileSrc = KscOpenFile(strDest, 0);   // Open ReadOnly 
			if (hFileSrc == INVALID_HANDLE_VALUE)
			{
				goto clean0;
			}
			if (!KscGetFileSize(hFileSrc, dwSizeLow, dwSizeHight))
			{
				goto clean0;
			}

			qwSize = (ULONGLONG)dwSizeHight << 32;
			qwSize |= dwSizeLow;

clean0:
			if (hFileSrc != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hFileSrc);
				hFileSrc = INVALID_HANDLE_VALUE;
			}

			job.AddJob(strSrc, strDest);

			if (_DoMoveJobsEx(job, qwSize))
			{
				m_List.DeleteItem(dwIndex);
				_DeleteMoveHistory(strDest);
			}
			
		}

		return 0;
	}

	 HRESULT OnSelectOne(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	 {
		 if (IsAllNotSelect())
		 {
			 EnableItem(2, FALSE);
		 }
		 else
		 {
			 EnableItem(2, TRUE);
		 }
		return 0;
	 }

	BOOL  IsAllNotSelect()
	 {
		 BOOL bRet = FALSE;
		 int nCount = m_List.GetItemCount();

		 for (int n = 0; n< nCount; n++)
		 {
			 if (m_List.GetCheckState(n))
			 {
				 bRet = FALSE;
				 goto _exit_;
			 }
		 }
		 bRet = TRUE;
	_exit_:
		 return bRet;
	 }

    BOOL _DeleteMoveHistory(CString strFilePath)
    {
        BOOL retval = FALSE;
        TiXmlDocument xmlDoc;
        TiXmlElement *pXmlHistory = NULL;
        TiXmlElement *pXmlMove = NULL;
        KFilePath xmlPath = KFilePath::GetFilePath(_Module.m_hInst);

        xmlPath.RemoveFileSpec();
        xmlPath.Append(L"cfg\\movehistory.xml");
        if (!xmlDoc.LoadFile(UnicodeToAnsi(xmlPath.value()).c_str(), TIXML_ENCODING_UTF8))
            goto clean0;

        pXmlHistory = xmlDoc.FirstChildElement("history");
        if (!pXmlHistory)
            goto clean0;

        pXmlMove = pXmlHistory->FirstChildElement("move");
        while (pXmlMove)
        {

            CString strPathNow = KUTF8_To_UTF16(pXmlMove->Attribute("now"));
            if(strPathNow.CompareNoCase(strFilePath)==0)
            {
                pXmlHistory->RemoveChild((TiXmlNode*)pXmlMove);
                break;
            }
            pXmlMove = pXmlMove->NextSiblingElement("move");
        }

        xmlDoc.SaveFile(UnicodeToAnsi(xmlPath.value()).c_str());
        retval = TRUE;

clean0:
        return retval;
    }

	BOOL _DoMoveJobsEx(CMoveJob& job, ULONGLONG llSize)
	{
		BOOL bRet = TRUE;
		CBigFileDoingDlg dlg(_T("正在迁移..."), _T("正在迁移"), job, llSize);
		UINT_PTR uRes = dlg.DoModal();
		const CAtlList<CMoveJob::MoveTask>& moveList = dlg.m_pMoveJob->GetSuccess();
		const CAtlList<CMoveJob::MoveTask>& moveCanceled = dlg.m_pMoveJob->GetCanceled();

		if (uRes == 101)
		{
			bRet = FALSE;
			goto clean0;
		}

		if (IDCANCEL == uRes)
		{
			POSITION pos;
			pos = moveCanceled.GetHeadPosition();
			while (pos)
			{
				const CMoveJob::MoveTask& task = moveCanceled.GetNext(pos);
				::DeleteFile(task.strDest);
			}
			CKuiMsgBox msg;
			msg.ShowMsg(L"您已取消了本次迁移操作，所有的文件已被恢复到\n\n初始目录中。", L"已取消迁移", MB_OK|MB_ICONINFORMATION);
			bRet = FALSE;
			goto clean0;
		}

		if (moveList.GetCount() == 0)
		{
			bRet = FALSE;
			goto clean0;
		}

		const CAtlList<CMoveJob::MoveTask>& moveListLeft = dlg.m_pMoveJob->GetFailed();
		if (moveListLeft.GetCount() == 1)
		{
			CKuiMsgBox mgsBox;
			POSITION pos;
			pos = moveListLeft.GetHeadPosition();
			while (pos)
			{
				CString str;
				const CMoveJob::MoveTask& taskTemp = moveListLeft.GetNext(pos);
				str.Format(_T("%s\n正在被其他程序使用，暂时无法迁移此文件，您可以在程序退出时再次尝试迁移此文件。"),taskTemp.strSrc);
				CKuiMsgBox msg;
				msg.ShowMsg(str, L"文件被占用，暂时无法迁移", MB_OK|MB_ICONINFORMATION);
				break;
			}
			bRet = FALSE;
			goto clean0;
		}
		if (moveListLeft.GetCount() > 1)
		{
			CKuiMsgBox mgsBox;
			POSITION pos;
			pos = moveListLeft.GetHeadPosition();
			CString str;
			while (pos)
			{
				const CMoveJob::MoveTask& taskTemp = moveListLeft.GetNext(pos);
				str += taskTemp.strSrc;
				str += L"\n";
				//str.Format(_T("%s正在被其他文件使用，暂时无法迁移此文件，\n您可以在文件退出时再次尝试迁移此文件"),taskTemp.strSrc);
				/*CKuiMsgBox msg;
				msg.ShowMsg(str, L"文件被占用，暂时无法删除", MB_OK|MB_ICONINFORMATION);
				break;*/
			}
			CBigFileDelLeftDlg dlgEx(L"下表中的文件可能被其他程序占用，未能成功迁移，您可以在关掉程序之后再尝试迁移这些文件。", str, L"文件未迁移", 0);
			dlgEx.DoModal();
			bRet = FALSE;
			goto clean0;
		}
clean0:
		return bRet;
	}


public:
	KUI_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		KUI_NOTIFY_ID_COMMAND(1, OnBkBtnClose)
		KUI_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		KUI_NOTIFY_ID_COMMAND(2, OnBkBtnOk)
		KUI_NOTIFY_ID_COMMAND(3, OnBkRadioToRecy)
		KUI_NOTIFY_ID_COMMAND(4, OnBkRadioToDel)
		KUI_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CKscShowDestDir)
		MSG_KUI_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CKuiDialogImpl<CKscShowDestDir>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CKscShowDestDir>)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER(MSG_USER_LDLISTVIEWCTRL_CHECKBOX, OnSelectOne)
		MESSAGE_HANDLER(MSG_USER_LDLISTVIEWCTRL_LBP_URL, OnLDListViewClickUrl)
		END_MSG_MAP()
};
