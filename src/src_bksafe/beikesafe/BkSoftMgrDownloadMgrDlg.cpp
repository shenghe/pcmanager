#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "BkSoftMgrDownloadMgrDlg.h"
#include "beikesafesoftmgrHeader.h"
#include "_idl_gen/bksafesvc.h"
#include <bksafe/bksafeconfig.h>
#include <safemon/safetrayshell.h>
#include <comproxy/bkutility.h>
#include <shellapi.h>

//////////////////////////////////////////////////////////////////////////
// 项目排序
//
typedef CSoftListItemData				InsItem;
typedef CSimpleArray<InsItem*>			InsItemArray;

// 名称
class InsItemTimeSort
{
public:
	InsItemTimeSort(BOOL asc) :_asc(asc) {}

	bool operator()(const InsItem* pLeft, const InsItem* pRight) const
	{
		ATLASSERT(pLeft != NULL && pRight != NULL);
		if(_asc)
			return (pLeft->m_llDownloadTime < pRight->m_llDownloadTime);
		else
			return (pLeft->m_llDownloadTime > pRight->m_llDownloadTime);
	}

private:
	BOOL _asc;
};

//将字节数直接转换为字符串
CString CBkDownloadMgrDlg::CalcStrFormatByteSize(LONGLONG qdw)
{
	CString retStr;
	StrFormatByteSize64(qdw, retStr.GetBuffer(64), 64);
	retStr.ReleaseBuffer();

	return retStr;
}

void CBkDownloadMgrDlg::OnBkBtnClose()
{
	EndDialog(IDCANCEL);
}

void CBkDownloadMgrDlg::OnClick(int nListId, CTypeListItemData * pData)
{
	if ( pData->nId == 0 )
	{
		SetItemVisible( IDC_DOWNMGR_ED_DLG, FALSE );
		SetItemVisible( IDC_DOWNMGR_ING_DLG, TRUE );
		SetItemVisible( IDG_DOWNMGR_RUBBISH_DLG, FALSE);
		m_nPage = 0;
	}
	else if ( pData->nId == 1 )
	{
		SetItemVisible( IDC_DOWNMGR_ING_DLG, FALSE );
		SetItemVisible( IDC_DOWNMGR_ED_DLG, TRUE );
		SetItemVisible( IDG_DOWNMGR_RUBBISH_DLG, FALSE);
		m_nPage = 1;
	}
	else if ( pData->nId == 2)
	{
		SetItemVisible( IDG_DOWNMGR_RUBBISH_DLG, TRUE);
		SetItemVisible( IDC_DOWNMGR_ING_DLG, FALSE );
		SetItemVisible( IDC_DOWNMGR_ED_DLG, FALSE );
		m_nPage = 2;

		InsertDataToList();
		OnLbuttonUpNotifyEx( 0 );
	}
}

void CBkDownloadMgrDlg::OnDownSetUp(DWORD dwIndex)
{
	CSoftListItemData * p = m_EdList.GetItemDataEx( dwIndex );

	for ( int i=0; i<m_pHandler->m_arrData.GetSize(); i++ )
	{
		CSoftListItemData * pData = m_pHandler->m_arrData[i];
		if ( pData->m_strSoftID == p->m_strSoftID )
		{
			// 开始安装
			CString strStore = pData->m_strPath;
			if( !PathFileExists(strStore))
			{
				BKSafeConfig::GetStoreDir(strStore);
				if (strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
				{
					strStore.Append(_T("\\")+ pData->m_strFileName);
				}
				else
				{
					strStore.Append(pData->m_strFileName);
				}
			}
			if (PathFileExists(strStore))
			{
				pData->m_bDownloading = FALSE;
				p->m_bDownloading = FALSE;
				pData->m_bDownLoad = TRUE;
				p->m_bDownLoad = TRUE;
				pData->m_dwProgress = 100;
				p->m_dwProgress = 100;

				installParam *pParam = new installParam;
				pParam->pData = pData;
				pParam->strPath = strStore;
				m_pHandler->m_arrInstall.AddTail(pParam);
				if ( m_pHandler->m_hInstall == (HANDLE)-1)
				{
					m_pHandler->m_hInstall = (HANDLE)_beginthreadex(NULL, 0, m_pHandler->InstallProc, m_pHandler, 0, NULL);
				}
				pData->m_bInstalling = TRUE;
				p->m_bInstalling = TRUE;

				m_EdList.RefreshItem(dwIndex);
				return;
			}
			else
			{
				DELETEITEMSTRUCT item;
				item.itemID = dwIndex;
				m_EdList.DeleteItem( &item );
				return;
			}
		}
	}

	
}

void CBkDownloadMgrDlg::OnDelFiles()
{
	CBkSafeMsgBox2	msgbox;
	msgbox.AddButton(L"是", IDOK);
	msgbox.AddButton(L"否", IDCANCEL);
	UINT nRet = msgbox.ShowMutlLineMsg(BkString::Get(IDS_SOFTMGR_8079), NULL, MB_BK_CUSTOM_BUTTON|MB_ICONQUESTION, NULL);
	if(nRet==IDCANCEL)
		return;

	CString	strStoreDir;
	BKSafeConfig::GetStoreDir( strStoreDir );

	CString	strFile;

	for ( int i=m_EdList.GetItemCount()-1; i>=0; i-- )
	{
		CSoftListItemData * p = m_EdList.GetItemDataEx( i );
		if ( p->m_bCheck )
		{
			strFile.Format( _T( "%s\\%s" ), strStoreDir, p->m_strFileName );
			// 删除文件, 并更新已下载状态为false, 更新 arrData， 更新List
			if ( p->m_strPath.IsEmpty() )
			{
				// 上次运行时下载的文件， 内存中没有保存位置
				DeleteFile( strFile );
			}
			else
			{
				// 新下载的文件， 在内存中有保存位置
				DeleteFile( p->m_strPath );
			}

			for ( int j=0; j<m_pHandler->m_arrData.GetSize(); j++ )
			{
				if ( p->m_strSoftID == m_pHandler->m_arrData[j]->m_strSoftID )
				{
					m_pHandler->m_arrData[j]->m_bDownLoad = FALSE;
					m_pHandler->m_arrData[j]->m_bDownloading = FALSE;
					break;
				}
			}

			DELETEITEMSTRUCT item;
			item.itemID = i;
			m_EdList.DeleteItem( &item );

		}
	}

	m_nDownloaded = m_EdList.GetItemCount();

	m_list.SetNumber( 1, m_nDownloaded );

	LONGLONG		nDownloadedSize = 0;

	// 计算大小
	for ( int i=0; i<m_EdList.GetItemCount(); i++ )
	{
		CSoftListItemData * p = m_EdList.GetItemDataEx( i );

		nDownloadedSize += p->m_dwSize;
	}

	CString	strDownED;
	strDownED = CalcStrFormatByteSize( nDownloadedSize );

	if ( m_nDownloaded > 0 )
	{
		FormatRichText(
			IDC_DOWNMGR_ED_DESC, 
			BkString::Get(IDC_DOWNMGR_ED_DESC_FMT), 
			m_nDownloaded, strDownED );
	}
	else
	{
		FormatRichText(
			IDC_DOWNMGR_ED_DESC, 
			BkString::Get(IDC_DOWNMGR_ED_DESC_FMT0), 
			m_nDownloaded );
	}

}

void CBkDownloadMgrDlg::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == DOWNMGR_GETDATA_TIMER )
	{
		GetDataFromHandler();
	}
}

BOOL CopyItemData( CSoftListItemData * dst, CSoftListItemData * src )
{ 
	BOOL	bChange = FALSE;

	dst->m_attri			= src->m_attri;
	dst->m_bCharge			= src->m_bCharge;
	dst->m_bContinue		= src->m_bContinue;
	dst->m_bDownLoad		= src->m_bDownLoad;
	dst->m_bDownloading		= src->m_bDownloading;
	dst->m_bFailed			= src->m_bFailed;
	dst->m_bIcon			= src->m_bIcon;
	dst->m_bLinkFailed		= src->m_bLinkFailed;
	dst->m_bLinking			= src->m_bLinking;
	dst->m_bPause			= src->m_bPause;
	dst->m_bPlug			= src->m_bPlug;
	dst->m_bCancel			= src->m_bCancel;

	dst->m_dwDataID			= src->m_dwDataID;
	dst->m_dwID				= src->m_dwID;
	dst->m_dwProgress		= src->m_dwProgress;
	dst->m_dwStartTime		= src->m_dwStartTime;
	dst->m_dwTime			= src->m_dwTime;
	dst->m_fSpeed			= src->m_fSpeed;
	dst->m_pImage			= src->m_pImage;
	dst->m_strDescription	= src->m_strDescription;
	dst->m_strDownURL		= src->m_strDownURL;
	dst->m_strFileName		= src->m_strFileName;
	dst->m_strIconURL		= src->m_strIconURL;
	dst->m_strMD5			= src->m_strMD5;
	dst->m_strName			= src->m_strName;
	dst->m_strOSName		= src->m_strOSName;
	dst->m_strSize			= src->m_strSize;
	dst->m_strSoftID		= src->m_strSoftID;
	dst->m_strSpeed			= src->m_strSpeed;
	dst->m_strType			= src->m_strType;
	dst->m_strURL			= src->m_strURL;
	dst->m_strVersion		= src->m_strVersion;
	dst->m_dwSize			= src->m_dwSize;
	dst->m_bWaitDownload	= src->m_bWaitDownload;
	dst->m_llDownloadTime	= src->m_llDownloadTime;
	//dst->m_bCheck			= FALSE;

	// 安装时会变化的标志
	if ( dst->m_bSetup != src->m_bSetup )
	{
		dst->m_bSetup			= src->m_bSetup;
		bChange = TRUE;
	}

	if ( dst->m_bInstalling != src->m_bInstalling )
	{
		dst->m_bInstalling		= src->m_bInstalling;
		bChange = TRUE; 
	}
	
	return bChange;
}

void CBkDownloadMgrDlg::GetDataFromHandler()
{
	BOOL	bCountChange = FALSE;
	BOOL bNeedSortIngList = FALSE;
	BOOL bNeedSortEdList = FALSE;
	for ( int i=0; i<m_pHandler->m_arrData.GetSize(); i++ )
	{
		CSoftListItemData	* p = m_pHandler->m_arrData[i];

		if ( p->m_bDownloading && !p->m_bDownLoad )
		{
			// 正在下载的
			// 需要将新发现的正在下载的加进来
			BOOL	bFound = FALSE;
			for ( int j=0; j<m_IngList.GetItemCount(); j++ )
			{
				CSoftListItemData * pIng = m_IngList.GetItemDataEx( j );
				if ( pIng != NULL && pIng->m_strSoftID == p->m_strSoftID )
				{
					bFound = TRUE;
					CopyItemData( pIng, p );
					pIng->m_dwID = j;
					m_IngList.RefreshProgress( j );
					break;
				}
			}

			if ( !bFound )
			{
				CSoftListItemData * pIng = new CSoftListItemData;
				
				int	nIndex = m_IngList.GetItemCount();
				m_IngList.InsertItem( nIndex, _T( "" ), nIndex );
				CopyItemData( pIng, p );
				pIng->m_dwID = nIndex;

				m_IngList.SetItemDataEx( nIndex, ( DWORD_PTR ) pIng );
				m_IngList.SetPaintItem(TRUE);

				m_nDownloading ++;
				bCountChange = TRUE;
				bNeedSortIngList = TRUE;
			}
				
		}

		if ( p->m_bDownLoad )
		{
			// 已下载的,发现新的加进来， 发现Downloading里面有的就删除
			BOOL	bFound = FALSE;
			int		nIng = 0;
			for ( int j=0; j<m_IngList.GetItemCount(); j++ )
			{
				CSoftListItemData * pIng = m_IngList.GetItemDataEx( j );
				if ( pIng != NULL && pIng->m_strSoftID == p->m_strSoftID )
				{
					bFound = TRUE;
					nIng = j;
					break;
				}
			}

			if ( bFound )
			{
				// 删除
				DELETEITEMSTRUCT item;
				item.itemID = nIng;
				m_IngList.DeleteItem( &item );

				m_nDownloading --;
				bCountChange = TRUE;
			}

			bFound = FALSE;
			for ( int j=0; j<m_EdList.GetItemCount(); j++ )
			{
				CSoftListItemData * pEd = m_EdList.GetItemDataEx( j );
				if ( pEd != NULL && pEd->m_strSoftID == p->m_strSoftID )
				{
					bFound = TRUE;
					BOOL b = CopyItemData( pEd, p );
					pEd->m_dwID = j;

					if ( b )
					{
						// 刷新已下载文件的安装状态
						m_EdList.RefreshItem( j );

						if (!PathFileExists(p->m_strPath))//设置安装后删除，则删除项
						{
							DELETEITEMSTRUCT item;
							item.itemID = j;
							m_EdList.DeleteItem( &item );

							m_nDownloaded--;
							bCountChange = TRUE;
						}
					}

					break;
				}
			}

			if ( !bFound && PathFileExists(p->m_strPath))
			{
				// 添加到已下载
				int	nIndex = m_EdList.GetItemCount();
				m_EdList.InsertItem( nIndex, _T( "" ), nIndex );

				CSoftListItemData * pEd = new CSoftListItemData;
				CopyItemData( pEd, p );
				pEd->m_dwID = nIndex;
				m_EdList.SetItemDataEx( nIndex, ( DWORD_PTR ) pEd );
				m_EdList.SetPaintItem(TRUE);

				m_nDownloaded ++;
				bCountChange = TRUE;
				bNeedSortEdList = TRUE;
			}
			
		}
	}

	// 对正在下载按下载时间排序
	if (bNeedSortIngList)
	{
		InsItemArray &array = m_IngList.GetItemArray();
		if(array.GetSize() != 0)
		{
			std::sort(array.GetData(), array.GetData() + array.GetSize(), InsItemTimeSort(FALSE));

			m_IngList.UpdateAll();
		}
	}

	// 对已下载按下载时间排序
	if (bNeedSortEdList)
	{
		InsItemArray &array = m_EdList.GetItemArray();
		if(array.GetSize() != 0)
		{
			std::sort(array.GetData(), array.GetData() + array.GetSize(), InsItemTimeSort(FALSE));

			m_EdList.UpdateAll();
		}
	}

	// 显示描述
	if ( bCountChange )
	{
		m_list.SetNumber( 0, m_nDownloading );
		m_list.SetNumber( 1, m_nDownloaded );

		LONGLONG		nDownloadingSize = 0;
		LONGLONG		nDownloadedSize = 0;

		// 计算大小
		for ( int i=0; i<m_IngList.GetItemCount(); i++ )
		{
			CSoftListItemData * p = m_IngList.GetItemDataEx( i );

			nDownloadingSize += p->m_dwSize;
		}

		for ( int i=0; i<m_EdList.GetItemCount(); i++ )
		{
			CSoftListItemData * p = m_EdList.GetItemDataEx( i );

			nDownloadedSize += p->m_dwSize;
		}

		CString	strDownED;
		CString	strDownIng;
		
		strDownIng = CalcStrFormatByteSize( nDownloadingSize );
		strDownED = CalcStrFormatByteSize( nDownloadedSize );


		// 刷新描述
		if ( m_nDownloading > 0 )
		{
			FormatRichText(
				IDC_DOWNMGR_ING_DESC, 
				BkString::Get(IDC_DOWNMGR_ING_DESC_FMT), 
				m_nDownloading, strDownIng );
		}
		else
		{
			FormatRichText(
				IDC_DOWNMGR_ING_DESC, 
				BkString::Get(IDC_DOWNMGR_ING_DESC_FMT0), 
				m_nDownloading );
		}
		
		if ( m_nDownloaded > 0 )
		{
			FormatRichText(
				IDC_DOWNMGR_ED_DESC, 
				BkString::Get(IDC_DOWNMGR_ED_DESC_FMT), 
				m_nDownloaded, strDownED );
		}
		else
		{
			FormatRichText(
				IDC_DOWNMGR_ED_DESC, 
				BkString::Get(IDC_DOWNMGR_ED_DESC_FMT0), 
				m_nDownloaded );
		}


		CRect rcWnd;
		m_EdList.GetWindowRect(&rcWnd);
		if (rcWnd.Height() > m_EdList.GetItemCount() * 54)
		{
			m_EdList.SetColumnWidth(0, rcWnd.Width());
		}
		else if(rcWnd.Width() > 0)
		{
			m_EdList.SetColumnWidth(0, rcWnd.Width() - 18);
		}
		m_EdList.Invalidate(FALSE);

		m_IngList.GetWindowRect(&rcWnd);
		if (rcWnd.Height() > m_IngList.GetItemCount() * 54)
		{
			m_IngList.SetColumnWidth(0, rcWnd.Width());
		}
		else
		{
			m_IngList.SetColumnWidth(0, rcWnd.Width() - 18);
		}
		m_IngList.Invalidate(FALSE);

	}

	// 
	if ( m_IngList.GetItemCount() > 0 )
	{
		SetItemVisible( IDC_DLG_ING_NO_ITEM, FALSE );
		SetItemVisible( IDC_DOWNMGR_ING_RIGHT_LIST, TRUE );
	}
	else
	{
		SetItemVisible( IDC_DOWNMGR_ING_RIGHT_LIST, FALSE );
		SetItemVisible( IDC_DLG_ING_NO_ITEM, TRUE );
	}

	if ( m_EdList.GetItemCount() > 0 )
	{
		SetItemVisible( IDC_DLG_ED_NO_ITEM, FALSE );
		SetItemVisible( IDC_DOWNMGR_ED_RIGHT_LIST, TRUE );
	}
	else
	{
		SetItemVisible( IDC_DOWNMGR_ED_RIGHT_LIST, FALSE );
		SetItemVisible( IDC_DLG_ED_NO_ITEM, TRUE );
	}

	if (m_EdList.GetItemCount() > 0)
	{
		EnableItem(IDC_DOWNMGR_ED_CHECK, TRUE);
	}
	else
	{
		EnableItem(IDC_DOWNMGR_ED_CHECK, FALSE);
	}

	CheckDelBtn();

#if 0
	CString  strCachPath;
	if( SUCCEEDED(CAppPath::Instance().GetLeidianLogPath(strCachPath)) )
	{
		strCachPath.Append(_T("\\ksoft_MSG.txt"));
	}
	char szFilePath[MAX_PATH] = {0};
	FILE* pf = NULL;
	CString strID;
	char *szSoftID = NULL;

	::WideCharToMultiByte( CP_ACP, 0, strCachPath, strCachPath.GetLength(), szFilePath, MAX_PATH-1, NULL, NULL );

	if (!(pf = fopen(szFilePath, "w"))) 
	{
		return ;
	}
	for (int k = 0; k < m_RibList.GetItemCount(); k++)
	{
		CSoftListItemData *pData = m_RibList.GetItemDataEx(k);
		if (pData != NULL)
		{
			DWORD dwLength = ::WideCharToMultiByte(CP_ACP, 0, pData->m_strSoftID, 
				-1, NULL, 0, NULL, FALSE);
			szSoftID = new char[dwLength];
			::WideCharToMultiByte(CP_ACP, 0, pData->m_strSoftID, -1, szSoftID,
				dwLength, NULL, FALSE);
			fwrite(szSoftID, dwLength, 1, pf);
			fwrite("\r\n", strlen("\r\n"), 1, pf);
		}
	}
	fclose(pf);
#endif
}

BOOL CBkDownloadMgrDlg::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{

	InitList();

	GetDataFromHandler();

	if ( m_nDownloading == 0 && m_nDownloaded == 0 )
	{
		CString strDownIng;
		CString strDownED;

		strDownIng = CalcStrFormatByteSize( 0 );
		strDownED = CalcStrFormatByteSize( 0 );


		// 刷新描述
		FormatRichText(
			IDC_DOWNMGR_ING_DESC, 
			BkString::Get(IDC_DOWNMGR_ING_DESC_FMT0), 
			m_nDownloading );

		FormatRichText(
			IDC_DOWNMGR_ED_DESC, 
			BkString::Get(IDC_DOWNMGR_ED_DESC_FMT0), 
			m_nDownloaded );
	}

	EnableItem(IDC_DOWNMGR_ED_BTN_DEL, FALSE);
	if (m_EdList.GetItemCount() > 0)
	{
		EnableItem(IDC_DOWNMGR_ED_CHECK, TRUE);
	}
	else
	{
		EnableItem(IDC_DOWNMGR_ED_CHECK, FALSE);
	}

	for (int i = 0; i < m_EdList.GetItemCount(); i ++)
	{
		CSoftListItemData *pData = m_EdList.GetItemDataEx(i);
		if (pData != NULL )
		{
			if(pData->m_bCheck)
			{
				EnableItem(IDC_DOWNMGR_ED_BTN_DEL, TRUE);
				EnableItem(IDC_DOWNMGR_ED_CHECK, TRUE);
				break;
			}
		}
	}

	SetTimer( DOWNMGR_GETDATA_TIMER, 1000, NULL );

	InsertDataToList();

	if (m_IngList.GetItemCount() > 0)
	{
		m_nPage = 0;
	}
	else if (m_EdList.GetItemCount() > 0)
	{
		m_nPage = 1;
	}
	else if ( m_RibList.GetItemCount() > 0)
	{
		m_nPage = 2;
	}
	else
	{
		m_nPage =0;
	}

	if ( m_nPage == 0 )
	{
		SetItemVisible( IDC_DOWNMGR_ED_DLG, FALSE );
		SetItemVisible( IDC_DOWNMGR_ING_DLG, TRUE );
		SetItemVisible( IDG_DOWNMGR_RUBBISH_DLG, FALSE);
		m_list.SetCurSel( 0 );
	}
	else if ( m_nPage == 1 )
	{
		SetItemVisible( IDC_DOWNMGR_ING_DLG, FALSE );
		SetItemVisible( IDC_DOWNMGR_ED_DLG, TRUE );
		SetItemVisible( IDG_DOWNMGR_RUBBISH_DLG, FALSE);
		m_list.SetCurSel( 1 );
	}
	else if ( m_nPage == 2)
	{
		SetItemVisible( IDC_DOWNMGR_ING_DLG, FALSE );
		SetItemVisible( IDC_DOWNMGR_ED_DLG, FALSE );
		SetItemVisible( IDG_DOWNMGR_RUBBISH_DLG, TRUE);
		m_list.SetCurSel( 2 );
	}

	return TRUE;
}

void CBkDownloadMgrDlg::OnSelAll()
{
	BOOL bChecked = GetItemCheck(IDC_DOWNMGR_ED_CHECK);
	m_EdList.SetCheckedAllItem(bChecked);
	CheckDelBtn();
}

void CBkDownloadMgrDlg::OnOpenStoreDir()
{
	CString	strStoreDir;
	BKSafeConfig::GetStoreDir( strStoreDir );

	if( !PathIsDirectory(strStoreDir) )
		CreateDirectoryNested( strStoreDir );
	if( PathIsDirectory( strStoreDir ) )
		ShellExecute(NULL, _T("open"), strStoreDir, NULL, NULL, SW_SHOW);
	else
	{
		CString strMessage;
		strMessage.Format(BkString::Get(IDS_SOFTMGR_8045), strStoreDir);
		CBkSafeMsgBox::Show(strMessage,BkString::Get(IDS_SOFTMGR_8046), MB_OK | MB_ICONWARNING);
	}
}

UINT_PTR CBkDownloadMgrDlg::DoModal(int nPage, HWND hWndParent)
{
	m_nPage = nPage;

	return __super::DoModal(hWndParent);
}

void CBkDownloadMgrDlg::InitList()
{
	// 初始化正在下载
	InitIngList();

	// 初始化已下载
	InitEdList();

	InitRibList();

	// 初始化 类别列表（数量）
	InitTypeList();

}

void CBkDownloadMgrDlg::InitRibList()
{
	m_RibList.Create(GetViewHWND(), NULL, NULL, WS_CHILD|LVS_REPORT|LVS_NOCOLUMNHEADER|LVS_NOSORTHEADER|LVS_OWNERDRAWFIXED , 
		0, IDC_DOWNMGR_RUBBISH_RIGHT_LIST);
	m_RibList.SetClickLinkCallback(this);

	m_RibList.InsertColumn(0,_T(""), LVCFMT_LEFT, 530);
}
void CBkDownloadMgrDlg::InitIngList()
{
	m_IngList.Create( GetViewHWND(), NULL, NULL, WS_CHILD|LVS_REPORT|LVS_NOCOLUMNHEADER|LVS_NOSORTHEADER|LVS_OWNERDRAWFIXED , 
		0, IDC_DOWNMGR_ING_RIGHT_LIST);
	m_IngList.CreateTipCtrl();
	m_IngList.SetClickLinkCallback(this);

	m_IngList.InsertColumn(0,_T(""), LVCFMT_LEFT, 530);
}

void CBkDownloadMgrDlg::InitEdList()
{
	m_EdList.Create( GetViewHWND(), NULL, NULL, WS_CHILD|LVS_REPORT|LVS_NOCOLUMNHEADER|LVS_NOSORTHEADER|LVS_OWNERDRAWFIXED , 
		0, IDC_DOWNMGR_ED_RIGHT_LIST);
	m_EdList.SetClickLinkCallback(this);

	m_EdList.InsertColumn(0,_T(""), LVCFMT_LEFT, 530);
}

void CBkDownloadMgrDlg::InitTypeList()
{
	m_list.Create(GetViewHWND(), NULL, NULL, WS_CHILD | LBS_OWNERDRAWVARIABLE, 0, IDC_DOWNMGR_LEFT_LIST);

	int nCount = 0;
	m_list.InsertString( 0, BkString::Get(IDS_SOFTMGR_8080));
	CTypeListItemData * pListData = new CTypeListItemData( 13, 0, COL_TYPE_NAME_NORMAL, BkString::Get(IDS_SOFTMGR_8080), 0 );
	m_list.SetItemDataX( 0, ( DWORD_PTR ) pListData );
	m_list.SetItemHeight( 0, 28 );	

	m_list.InsertString( 1, BkString::Get(IDS_SOFTMGR_8081) );
	pListData = new CTypeListItemData( 13, 1, COL_TYPE_NAME_NORMAL,BkString::Get(IDS_SOFTMGR_8081) , 0 );
	m_list.SetItemDataX( 1, ( DWORD_PTR ) pListData );
	m_list.SetItemHeight( 1, 28 );	

	m_list.InsertString( 2, BkString::Get(IDS_SOFTMGR_8082) );
	pListData = new CTypeListItemData( 13, 2, COL_TYPE_NAME_NORMAL, BkString::Get(IDS_SOFTMGR_8082) , 0 );
	m_list.SetItemDataX( 2, ( DWORD_PTR ) pListData );
	m_list.SetItemHeight(2, 28 );

	m_list.SetCallBack( this );
	m_list.SetListID(IDC_DOWNMGR_LEFT_LIST);
	m_list.SetCurSel( 0 );
}

void CBkDownloadMgrDlg::OnListEdReSize(CRect rcWnd)
{
	if (rcWnd.Height() > m_EdList.GetItemCount() * 54)
	{
		m_EdList.SetColumnWidth(0, rcWnd.Width());
	}
	else if(rcWnd.Width() > 0)
	{
		m_EdList.SetColumnWidth(0, rcWnd.Width() - 18);
	}
//	m_EdList.SetColumnWidth(0,rcWnd.Width()-18);
}

void CBkDownloadMgrDlg::OnListIngReSize(CRect rcWnd)
{
	if (rcWnd.Height() > m_IngList.GetItemCount() * 54)
	{
		m_IngList.SetColumnWidth(0, rcWnd.Width());
	}
	else if(rcWnd.Width() > 0)
	{
		m_IngList.SetColumnWidth(0, rcWnd.Width() - 18);
	}
//	m_IngList.SetColumnWidth(0,rcWnd.Width()-18);
}

void CBkDownloadMgrDlg::OnListRibReSize(CRect rcWnd)
{
	if (m_RibList.IsWindow())
	{
		if (rcWnd.Height() > m_RibList.GetItemCount() * 54)
		{
			m_RibList.SetColumnWidth(0, rcWnd.Width());
		}
		else if (rcWnd.Width() > 0)
		{
			m_RibList.SetColumnWidth( 0, rcWnd.Width() - 18);
		}
	}
}

void CBkDownloadMgrDlg::OnMgrCancel(DWORD dwIndex)
{

	CSoftListItemData * p = m_IngList.GetItemDataEx( dwIndex );

	for ( int i=0; i<m_pHandler->m_arrData.GetSize(); i++ )
	{
		CSoftListItemData * pData = m_pHandler->m_arrData[i];
		if ( pData->m_strSoftID == p->m_strSoftID )
		{
			pData->m_bDownloading = FALSE;
			p->m_bDownloading = FALSE;
			pData->m_bDownLoad = FALSE;
			pData->m_bFailed = FALSE;
			pData->m_bCancel = TRUE;
			
			threadParam *param = new threadParam;
			param->pData = pData;
			param->pDlg = m_pHandler;
			param->flag = 2;

			//重新开一个线程处理，防止死锁
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, m_pHandler->ThreadProc, param, 0, NULL);
			if(hThread)
			{
				CloseHandle(hThread);
			}	

			// 从列表删除
			DELETEITEMSTRUCT item;
			item.itemID = dwIndex;
			m_IngList.DeleteItem( &item );

			m_nDownloading = m_IngList.GetItemCount();
			m_list.SetNumber( 0, m_nDownloading );

			LONGLONG		nDownloadingSize = 0;

			// 计算大小
			for ( int i=0; i<m_IngList.GetItemCount(); i++ )
			{
				CSoftListItemData * p = m_IngList.GetItemDataEx( i );

				nDownloadingSize += p->m_dwSize;
			}
		
			CString	strDownIng;

			strDownIng = CalcStrFormatByteSize( nDownloadingSize );

			// 刷新描述
			if ( m_nDownloading > 0 )
			{
				FormatRichText(
					IDC_DOWNMGR_ING_DESC, 
					BkString::Get(IDC_DOWNMGR_ING_DESC_FMT), 
					m_nDownloading, strDownIng );
			}
			else
			{
				FormatRichText(
					IDC_DOWNMGR_ING_DESC, 
					BkString::Get(IDC_DOWNMGR_ING_DESC_FMT0), 
					m_nDownloading );
			}

			InsertDataToList();

			return;
		}
	}
	
}

void CBkDownloadMgrDlg::OnMgrContinue(DWORD dwIndex)
{
	CSoftListItemData * p = m_IngList.GetItemDataEx( dwIndex );

	for ( int i=0; i<m_pHandler->m_arrData.GetSize(); i++ )
	{
		CSoftListItemData * pData = m_pHandler->m_arrData[i];
		if ( pData->m_strSoftID == p->m_strSoftID )
		{
			pData->m_bPause = FALSE;
			p->m_bPause = FALSE;
			pData->m_bContinue = TRUE;
			p->m_bContinue = TRUE;

			if (m_pHandler->m_arrDownload.Find(pData) == NULL)
			{
				m_pHandler->m_cs.Lock();
				m_pHandler->m_arrDownload.AddTail(pData);
				m_pHandler->m_cs.Unlock();
				{
					pData->m_bWaitDownload = TRUE;
				}
				for (int i = 0; i < m_pHandler->m_list.GetItemCount(); i++)
				{
					CSoftListItemData *pItemData = m_pHandler->m_list.GetItemDataEx(i);
					if (pItemData != NULL && pItemData->m_strSoftID == pData->m_strSoftID)
					{
						m_pHandler->m_list.RefreshRight(i);
						break;
					}
				}

				if(m_pHandler->m_hDownload == (HANDLE)-1)
				{
					m_pHandler->m_hDownload = (HANDLE)_beginthreadex(NULL, 0, m_pHandler->ThreadDownloadProc, m_pHandler, 0, NULL);
				}

			}
			else
			{
				threadParam *param = new threadParam;
				param->pData = pData;
				param->pDlg = m_pHandler;
				param->flag = 0;

				//重新开一个线程处理，防止死锁
				HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, m_pHandler->ThreadProc, param, 0, NULL);
				if(hThread)
				{
					CloseHandle(hThread);
				}
			}	

			m_IngList.RefreshItem(dwIndex);
			return;
		}
	}

}

void CBkDownloadMgrDlg::OnMgrDownLoad(DWORD dwIndex)
{
		// over
}

void CBkDownloadMgrDlg::OnMgrFreebackFromList(DWORD dwIndex)
{
	CString strFreeback;
	CSoftListItemData * p = m_IngList.GetItemDataEx( dwIndex );

	for ( int i=0; i<m_pHandler->m_arrData.GetSize(); i++ )
	{
		CSoftListItemData * pData = m_pHandler->m_arrData[i];
		if ( pData->m_strSoftID == p->m_strSoftID )
		{
			static std::wstring strFreeback = _T("");
			if (strFreeback.size() <= 0)
			{
		
				if (KisPublic::Instance()->Init())
				{
					int nRet = KisPublic::Instance()->KQueryOEMLPCWSTR(KIS::KOemKsfe::oemc_BBSUrl, strFreeback);
					if (!nRet)
						strFreeback = L"http://bbs.ijinshan.com/forum-18-1.html";

				}
				else
					strFreeback = L"http://bbs.ijinshan.com/forum-18-1.html";
			}

			::ShellExecute( NULL, _T("open"), strFreeback.c_str(), NULL, NULL , SW_SHOW);
			return;
		}
	}
}

void CBkDownloadMgrDlg::OnMgrPause(DWORD dwindex)
{
	CSoftListItemData * p = m_IngList.GetItemDataEx( dwindex );

	for ( int i=0; i<m_pHandler->m_arrData.GetSize(); i++ )
	{
		CSoftListItemData * pData = m_pHandler->m_arrData[i];
		if ( pData->m_strSoftID == p->m_strSoftID )
		{
			pData->m_bPause = TRUE;
			p->m_bPause = TRUE;
			pData->m_bContinue = FALSE;
			p->m_bContinue = FALSE;

			threadParam *param = new threadParam;
			param->pData = pData;
			param->pDlg = m_pHandler;
			param->flag = 1;

			//重新开一个线程处理，防止死锁
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, m_pHandler->ThreadProc, param, 0, NULL);
			if(hThread)
			{
				CloseHandle(hThread);
			}	

			m_IngList.RefreshItem(dwindex);
			return;
		}
	}

}

void CBkDownloadMgrDlg::OnMgrTautilogyFromList(DWORD dwIndex)
{

	CSoftListItemData * p = m_IngList.GetItemDataEx( dwIndex );

	for ( int i=0; i<m_pHandler->m_arrData.GetSize(); i++ )
	{
		CSoftListItemData * pData = m_pHandler->m_arrData[i];
		if ( pData->m_strSoftID == p->m_strSoftID )
		{
			pData->m_bLinkFailed = FALSE;
			pData->m_bLinking = TRUE;
			p->m_bLinkFailed = FALSE;
			p->m_bLinking = TRUE;

			// 真正执行下载
			if(pData->m_strDownURL != _T(""))
			{
				CAtlArray<CString> *urls = new CAtlArray<CString>;
				CString strURL = pData->m_strDownURL;
				int n = strURL.Find(_T(';'));
				while (n != -1)
				{
					pData->m_nURLCount++;
					urls->Add(strURL.Left(n));
					strURL = strURL.Right(strURL.GetLength() - n - 1);
					n = strURL.Find(_T(';'));
				}
				if (n == -1 && strURL != _T(""))
				{
					pData->m_nURLCount++;
					urls->Add(strURL);
				}

				CString strStore;
				BKSafeConfig::GetStoreDir(strStore);
				strStore.Append(_T("\\")+ pData->m_strFileName);

				m_pHandler->m_arrTask.SetAt(pData->m_strSoftID, m_pHandler->m_pDTManager->NewTask(urls, pData->m_strMD5, strStore));
				pData->m_dwStartTime = GetTickCount();
				
				pData->m_bDownloading = TRUE;
				pData->m_bLinking = TRUE;
				p->m_bDownloading = TRUE;
				p->m_bLinking = TRUE;
			}

			m_IngList.RefreshItem(dwIndex);
			return;
		}
	}
}


void CBkDownloadMgrDlg::GetCurDiskFreeSpace( LPCTSTR szCurrentPath, CString & strFreeSpace )
{
	CString strDisk;
	strDisk = szCurrentPath;
	if ( ! strDisk.IsEmpty() )
	{
		strDisk = strDisk.Left( 3 );

		ULARGE_INTEGER   lpuse = {0}; 
		ULARGE_INTEGER   lptotal = {0};
		ULARGE_INTEGER   lpfree = {0};
		GetDiskFreeSpaceEx(strDisk,&lpuse,&lptotal,&lpfree);

		strFreeSpace = CalcStrFormatByteSize(lpfree.QuadPart);
	}
}

void CBkDownloadMgrDlg::OnLbuttonUpNotify(DWORD dwIndex)
{
	CheckDelBtn();
}

void CBkDownloadMgrDlg::CheckDelBtn()
{
	EnableItem(IDC_DOWNMGR_ED_BTN_DEL, FALSE);

	int n = 0;
	for (int i = 0; i < m_EdList.GetItemCount(); i ++)
	{
		CSoftListItemData *pData = m_EdList.GetItemDataEx(i);
		if (pData != NULL && pData->m_bCheck)
		{
			EnableItem(IDC_DOWNMGR_ED_BTN_DEL, TRUE);
 
			n++ ;
		}
	}

	if (n == m_EdList.GetItemCount())
	{
		SetItemCheck(IDC_DOWNMGR_ED_CHECK, TRUE);
	}
	else
	{
		SetItemCheck(IDC_DOWNMGR_ED_CHECK, FALSE);
	}
}

void CBkDownloadMgrDlg::OnResume(CString strId)
{
	if( m_pHandler->m_arrDataMap.Lookup(strId) != NULL )
	{
		CSoftListItemData *pData = NULL;
		pData = m_pHandler->m_arrDataMap.Lookup(strId)->m_value;
		if ( pData != NULL )
		{
			pData->m_bDownloading = TRUE;
			pData->m_bDownLoad = FALSE;
			pData->m_bContinue = FALSE;
			pData->m_bPause = TRUE;
			pData->m_bCancel = FALSE;

			for (int i = 0; i < m_pHandler->m_list.GetItemCount(); i++)
			{
				CSoftListItemData *pItemData = m_pHandler->m_list.GetItemDataEx(i);
				if (pItemData != NULL && pItemData->m_strSoftID == pData->m_strSoftID)
				{
					m_pHandler->m_list.RefreshRight(i);
					break;
				}
			}
	
			for (int j = 0; j < m_RibList.GetItemCount(); j++)
			{
				CSoftListItemData *pItemData = m_RibList.GetItemDataEx( j );
				if ( pItemData != NULL && pItemData->m_strSoftID == strId)
				{
					DELETEITEMSTRUCT itemDel;
					itemDel.itemID = j;
					m_RibList.DeleteItem( &itemDel );
					break;
				}
			}
		}
	}

	InsertDataToList();
}

void CBkDownloadMgrDlg::OnLbuttonUpNotifyEx(DWORD dwIndex)
{
	EnableItem(IDC_DOWNMGR_RUBBISH_BTN_DEL, FALSE);
	int n = 0;
	for (int i = 0; i < m_RibList.GetItemCount(); i++)
	{
		CSoftListItemData *pData = m_RibList.GetItemDataEx(i);
		if ( pData != NULL && pData->m_bCheck )
		{
			EnableItem(IDC_DOWNMGR_RUBBISH_BTN_DEL, TRUE);
			n++;
		}
	}

	if ( n == m_RibList.GetItemCount())
	{
		SetItemCheck(IDC_DOWNMGR_RUBBISH_CHECK, TRUE);
	}
	else
	{
		SetItemCheck(IDC_DOWNMGR_RUBBISH_CHECK, FALSE);
	}
}

void CBkDownloadMgrDlg::InsertDataToList()
{
	int nCount = 0;
	LONGLONG lSoftSize = 0;
	BOOL bNeedSortRibList = FALSE;

	for( int i = 0; i < m_pHandler->m_arrData.GetSize(); i++)
	{
		CSoftListItemData *pData = m_pHandler->m_arrData[i];
		
		if ( !pData->m_bDownloading && !pData->m_bDownLoad && pData->m_bCancel)
		{
			BOOL bExist = FALSE;
			BOOL bCheck = GetItemCheck(IDC_DOWNMGR_RUBBISH_CHECK);

			for (int k = 0; k < m_RibList.GetItemCount(); k++)
			{
				CSoftListItemData *pItemData = NULL;
				pItemData = m_RibList.GetItemDataEx( k );
				if ( pData != NULL && pItemData != NULL && pItemData->m_strSoftID == pData->m_strSoftID)
				{
					bExist = TRUE;
					CopyItemData(pItemData, pData);
					pItemData->m_dwID = k;
					m_RibList.RefreshItem( k );
					break;
				}
			}

			if ( !bExist )
			{
				CSoftListItemData *pTmp = new CSoftListItemData;
				if ( pTmp != NULL )
				{
					CopyItemData( pTmp, pData );

					int nIndex = m_RibList.GetItemCount();
					m_RibList.InsertItem(nIndex, _T(""), nIndex);
					pTmp->m_dwID = nIndex;
					pTmp->m_bCheck = bCheck;
					m_RibList.SetItemDataEx( nIndex, ( DWORD_PTR ) pTmp );
					m_RibList.SetPaintItem(TRUE);
					bNeedSortRibList = TRUE;
				}
			}

			nCount ++;
			lSoftSize += pData->m_dwSize;
		}		
	}

	if (bNeedSortRibList)
	{
		InsItemArray &array = m_RibList.GetItemArray();
		if(array.GetSize() != 0)
		{
			std::sort(array.GetData(), array.GetData() + array.GetSize(), InsItemTimeSort(FALSE));

			m_RibList.UpdateAll();
		}
	}

	m_list.SetNumber( 2, nCount);	

	// 刷新描述
	if ( m_RibList.GetItemCount() > 0 )
	{
		SetItemVisible(IDC_DOWNMGR_RUBBISH_TEXT_DESC, TRUE);
		SetItemVisible(IDC_DOWNMGR_RUBBISH_RIGHT_LIST, TRUE);
		SetItemVisible(IDC_DOWNMGR_RUBBISH_NO_ITEM, FALSE);
		FormatRichText(
			IDC_DOWNMGR_RUBBISH_TEXT_DESC, 
			BkString::Get(IDS_DOWNMGR_RUBBISH_FMT), 
			nCount, CalcStrFormatByteSize( lSoftSize ));
		EnableItem(IDC_DOWNMGR_RUBBISH_CHECK, TRUE);
	}
	else
	{
		SetItemVisible(IDC_DOWNMGR_RUBBISH_RIGHT_LIST, FALSE);
		SetItemVisible(IDC_DOWNMGR_RUBBISH_NO_ITEM, TRUE);
		SetItemVisible(IDC_DOWNMGR_RUBBISH_TEXT_DESC, TRUE);

		FormatRichText(
			IDC_DOWNMGR_RUBBISH_TEXT_DESC, 
			BkString::Get(IDS_DOWNMGR_RUBBISH_FMT0), 
			nCount );

		EnableItem(IDC_DOWNMGR_RUBBISH_CHECK, FALSE);
	}

	CRect rcWnd;
	m_RibList.GetWindowRect(&rcWnd);
	if (rcWnd.Height() > m_RibList.GetItemCount() * 54)
	{
		m_RibList.SetColumnWidth(0, rcWnd.Width());
	}
	else if(rcWnd.Width() > 0)
	{
		m_RibList.SetColumnWidth(0, rcWnd.Width() - 18);
	}
	m_RibList.Invalidate(FALSE);
}

void CBkDownloadMgrDlg::OnDelDownloadTask()
{
	for (int i = m_RibList.GetItemCount() - 1; i >= 0; i--)
	{
		CSoftListItemData *pData = m_RibList.GetItemDataEx(i);
		if (pData != NULL && pData->m_bCheck)
		{
			if ( m_pHandler->m_arrDownload.Find( pData ) == NULL)
			{
				CSoftListItemData *pSoftData = NULL;
				if ( m_pHandler->m_arrDataMap.Lookup( pData->m_strSoftID ) != NULL )
				{
					pSoftData = m_pHandler->m_arrDataMap.Lookup( pData->m_strSoftID )->m_value;
					m_pHandler->m_cs.Lock();
					pSoftData->m_bCancel = FALSE;
					pSoftData->m_bDownLoad = FALSE;
					pSoftData->m_bDownloading = FALSE;
					pSoftData->m_bInstalling = FALSE;
					pSoftData->m_bLinking = FALSE;
					pSoftData->m_fSpeed = 0.0;
					pSoftData->m_dwTime = 0;
					pSoftData->m_dwStartTime = 0;
					pSoftData->m_dwProgress = 0;
					m_pHandler->m_cs.Unlock();
				}

				//刷新
				for (int i = 0; i < m_pHandler->m_list.GetItemCount(); i++)
				{
					CSoftListItemData *pItemData = m_pHandler->m_list.GetItemDataEx(i);
					if (pItemData != NULL)
					{
						if (pItemData->m_strSoftID == pData->m_strSoftID)
						{
							m_pHandler->m_list.RefreshRight(i);
							break;
						}
					}
				}

				//删除临时文件 ——_kt、_kti
				CString strStore;
				BKSafeConfig::GetStoreDir(strStore);
				if (strStore.ReverseFind(_T('\\')) < (strStore.GetLength() - 1))
				{
					strStore.Append(_T("\\")+ pData->m_strFileName + _T("_kt"));
				}

				if (PathFileExists(strStore))
				{
					DeleteFile(strStore);
				}
				strStore = strStore.Left(strStore.ReverseFind(_T('\\')) + 1);
				strStore.Append(pData->m_strFileName + _T("_kti"));
				if (PathFileExists(strStore))
				{
					DeleteFile(strStore);
				}
				strStore = strStore.Left(strStore.ReverseFind(_T('\\')) + 1);
				strStore.Append(pData->m_strFileName + _T("c"));
				BOOL bRtn ;
				if (PathFileExists(strStore))
				{
					bRtn = DeleteFile(strStore);
				}
			}
			else
			{
				threadParam *param = new threadParam;
				param->pData = pData;
				param->pDlg = m_pHandler;
				param->flag = 3;

				//重新开一个线程处理，防止死锁
				HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, m_pHandler->ThreadProc, param, 0, NULL);
				if(hThread)
				{
					CloseHandle(hThread);
				}
			}

			DELETEITEMSTRUCT itemDel;
			itemDel.itemID = i;
			m_RibList.DeleteItem( &itemDel );
		}			
	}		

	// 刷新描述
	InsertDataToList();
}

void CBkDownloadMgrDlg::OnSelAllListItem()
{
	BOOL bCheck = GetItemCheck( IDC_DOWNMGR_RUBBISH_CHECK );
	m_RibList.SetCheckedAllItem( bCheck );
	EnableItem( IDC_DOWNMGR_RUBBISH_BTN_DEL, bCheck);
}
