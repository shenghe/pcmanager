
#include "stdafx.h"
#include "wndoptdlg.h"

BOOL IsDownLoadTitle( CString& strTitle )
{
	if(	_tcsncmp( strTitle, TEXT("下载"), 2 ) == 0 || 
		_tcsncmp( strTitle, TEXT("安全下载"), 4 ) == 0 || 
		_tcsncmp( strTitle, TEXT("已完成"), 3 ) == 0 ||
		_tcsnicmp( strTitle, TEXT("download"),_tcslen(TEXT("download")) ) == 0 ||
		strTitle.Find(TEXT("个文件正在下载")) != -1 )
		return TRUE;
	else
		return FALSE;
}
//如果是金山卫士的主程序或对话框，就过滤掉
BOOL IsKSafeWindow(KAppWndListItemData itemData)
{
	CString strFileName = PathFindFileName(itemData.GetItemExePath());
	CString strWndTitle = itemData.GetItemWndTitle();
	if (strWndTitle.Find(TEXT("金山卫士")) != -1)
	{
		return TRUE;
	}
	else
		return FALSE;
}

LRESULT	CBKSafeWndOptdlg::OnStartFixItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle )
{
	int nIndex = (int)wParam;
	WndOptItem* pItem = (WndOptItem*)lParam;

	if( nIndex >0 && nIndex < m_aryListItem.GetCount() )
	{
		if( m_nFixAllCnt > 0)
			m_nPrcess = m_nFixCurIndex * 100 / m_nFixAllCnt;
		SetItemIntAttribute( IDC_WND_OPT_PROGRESS, "value", m_nPrcess );
		m_nFixCurIndex++;

		m_nFixingIndex = nIndex;

		WndOptItem& item = m_aryListItem[nIndex];
		item.m_nFixState = FIX_ST_FIXING;
		
		if( pItem )
			SetItemText( IDC_WND_OPT_NAME, pItem->m_strName );

		m_wndOptList.RedrawItem( nIndex );
	}

	return TRUE;
}

LRESULT	CBKSafeWndOptdlg::OnEndFixItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle )
{
	int nIndex = (int) wParam;
	WndOptItem* pItem = (WndOptItem*)lParam;

	if( nIndex >0 && nIndex < m_aryListItem.GetCount() )
	{
		WndOptItem& item = m_aryListItem[nIndex];

		if( pItem )
		{
			item.m_nFixState = pItem->m_nFixState;
			item.m_nCheckState = pItem->m_nCheckState;
		}
		else
		{
			item.m_nFixState = FIX_ST_FIXED;
			item.m_nCheckState = CHK_ST_FIXED;
		}

		if( m_nFixAllCnt > 0)
			m_nPrcess = m_nFixCurIndex * 100 / m_nFixAllCnt;
		SetItemIntAttribute( IDC_WND_OPT_PROGRESS, "value", m_nPrcess );

		m_wndOptList.RedrawItem( nIndex );
	}

	return TRUE;
}

int CBKSafeWndOptdlg::GetAllItemCnt()
{
	int nCnt = 0;

	for ( int i = 0; i < m_aryListItem.GetCount(); i++ )
	{
		WndOptItem& item = m_aryListItem[i];

		if( item.m_nType != ITEM_TYPE_TITLE )
			nCnt ++;
	}

	return nCnt;
}

LRESULT	CBKSafeWndOptdlg::OnFinishFix(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle )
{
	ULONGLONG*	pUValue = 0;
	ULONGLONG	nValue = 0;
	BOOL		bStop = (BOOL)lParam;

	SetItemIntAttribute( IDC_WND_OPT_PROGRESS, "value", 100);

	if( wParam )
	{
		pUValue = (ULONGLONG*)(wParam);
		nValue = *pUValue;
	}

	int nLess = GetRestUnFixCnt();

	m_uOptSize += nValue;
	
	//正常结束
	if( bStop == FALSE )
	{	
		FreeMemFinished( m_uOptSize, nLess );
	}

	UpdateCheckAllState();

	if( nLess != 0 )
	{
		SetItemIntAttribute( IDC_WND_OPT_EXAM_ICON, "sub", 1 );
		SetItemText( IDC_BTN_START_WND_OPT, BkString::Get(STR_BTN_CONTINUE_OPT) );	
	}
	else
	{
		EnableItem( IDC_BTN_START_WND_OPT, TRUE );
		EnableItem( IDC_WND_OPT_CHECKALL,	FALSE );

		SetItemIntAttribute( IDC_WND_OPT_EXAM_ICON, "sub", 0 );
		SetItemText( IDC_BTN_START_WND_OPT, BkString::Get(STR_BTN_EXIT_TEXT) );
	}
	
	SetItemVisible( IDC_DIV_CHECKING_WND,		FALSE );
	SetItemVisible( IDC_DIV_BEFORE_CHECK_WND,	TRUE );

	return TRUE;
}

void CBKSafeWndOptdlg::OnCheckAll()
{
	BOOL bCheck = GetItemCheck( IDC_WND_OPT_CHECKALL );

	for ( int i = 0; i < m_aryListItem.GetCount(); i++ )
	{
		WndOptItem& item = m_aryListItem[i];
		
		if( bCheck )
		{
			if( item.m_nType != ITEM_TYPE_TITLE && item.m_nCheckState == CHK_ST_UNCHECK )
				item.m_nCheckState = CHK_ST_CHECK;
		}
		else
		{
			if( item.m_nType != ITEM_TYPE_TITLE && item.m_nCheckState == CHK_ST_CHECK )
				item.m_nCheckState = CHK_ST_UNCHECK;
		}
	}

	m_wndOptList.Referesh();
	UpdateCheckAllState();
}

void CBKSafeWndOptdlg::FreeMemFinished(ULONGLONG uSize, int nUnFixCnt)
{
	CString m_strCfgPath, strValue, strHistoryValue, strXmlValue;
	CAppPath::Instance().GetLeidianCfgPath(m_strCfgPath, TRUE);
	m_strCfgPath.Append(L"\\bksafe.ini");

	IniFileOperate::CIniFile iniFile(m_strCfgPath);
	ULONGLONG uHistoryByte = 0;
	TCHAR pszByte[50] = {0};

	//获取历史
	iniFile.GetStrValue(L"FreeMem", L"FreeSize", pszByte, sizeof(pszByte));
	uHistoryByte = _wtoi64_l(pszByte, 0);
	NetFlowToString(uHistoryByte + uSize, strHistoryValue);

	strXmlValue.Format(BkString::Get(STR_OPT_COMPLTE_TEXT2), strHistoryValue);
	SetRichText( IDC_WND_OPT_TEXT_LINE2, strXmlValue);

	//写入新的
	strValue.Format(L"%I64d", uSize + uHistoryByte);
	iniFile.SetStrValue(L"FreeMem", L"FreeSize", strValue);

	strValue.Format(L"%I64d", _time64(NULL));
	iniFile.SetStrValue(L"FreeMem", L"FreeTime", strValue);

	strValue.Format(L"%I64d", uSize);
	if( uSize > 0 )
	{
		//记录下最新的记录
		iniFile.SetStrValue(L"FreeMem", L"LastFreeSize", strValue);
	}

	NetFlowToString(uSize, strValue);

	if( nUnFixCnt > 0 )
	{
		strXmlValue.Format(BkString::Get(STR_OPT_CONTINUE_TEXT1), nUnFixCnt, strValue);
	}
	else
	{
		strXmlValue.Format(BkString::Get(STR_OPT_COMPLTE_TEXT1), strValue);
	}
	
	SetRichText( IDC_TEXT_BEFORE_SCAN, strXmlValue);

}

BOOL CBKSafeWndOptdlg::OnInitDialog(HWND wParam, LPARAM lParam)
{
	InitCtrl();
	UpdateInfoText();
	SetItemCheck( IDC_WND_OPT_CHECKALL, TRUE );

	m_uOptSize = 0;

	return TRUE;
}

void CBKSafeWndOptdlg::UpdateInfoText()
{
	CString strText;
	if( m_nPidCnt > 0 )
		strText.Format( BkString::Get(STR_WND_NOREP_TEXT), 1 + m_nWndCnt , m_nPidCnt );
	else if( m_nWndCnt > 0 )
		strText.Format( BkString::Get(STR_WND_NOUSER_TEXT), m_nWndCnt + 1, m_nWndCnt );
	else
		strText.Format( BkString::Get(STR_OPT_COMPLTE_TEXT1), TEXT("10GB") );

	SetRichText( IDC_TEXT_BEFORE_SCAN, strText );
}

void CBKSafeWndOptdlg::AddMemoryItem( vector<KAppWndListItemData>& vecListItem )
{
	WndOptItem item;
	item.m_nType = ITEM_TYPE_TITLE;
	item.m_strName = TEXT("优化系统内存（1）");
	m_aryListItem.Add( item );

	item.m_nType = ITEM_TYPE_MEM;
	item.m_strName = TEXT("释放更多空闲内存给需要的程序，加速系统运行");
	item.m_nCheckState = CHK_ST_CHECK;
	item.m_nFixState = FIX_ST_UNFREE;
	m_aryListItem.Add( item );
}

int CBKSafeWndOptdlg::GetWndCnt( vector<KAppWndListItemData>& vecListItem )
{	
	int nCnt = 0;
	
	for ( int i = 0; i < vecListItem.size(); i++ )
	{
		if( vecListItem[i].GetItemState() == 1 || 
			( vecListItem[i].GetItemState() == 3 && 
			  !IsDownLoadTitle(vecListItem[i].GetItemWndTitle()) && 
			  !IsKSafeWindow(vecListItem[i])) )
		{
			nCnt++;
		}
	}
	
	return nCnt;
}

int CBKSafeWndOptdlg::GetPidCnt( vector<KAppWndListItemData>& vecListItem )
{	
	int nCnt = 0;

	for ( int i = 0; i < vecListItem.size(); i++ )
	{
		if( vecListItem[i].GetItemState() == 1 )
		{
			nCnt++;
		}
	}

	return nCnt;
}

void CBKSafeWndOptdlg::AddWndItem(vector<KAppWndListItemData>& vecListItem)
{
	m_nWndCnt = GetWndCnt(vecListItem);
	m_nPidCnt = GetPidCnt(vecListItem);
	if( m_nWndCnt > 0 )
	{
		WndOptItem item;
		item.m_nType = ITEM_TYPE_TITLE;
		item.m_strName.Format( TEXT("建议关闭的窗口（%d）"), m_nWndCnt );
		m_aryListItem.Add( item );
	}

	for ( int i = 0; i < vecListItem.size(); i++ )
	{
		KAppWndListItemData& item = vecListItem[i];
		if( item.GetItemState() == 3 && 
			!IsDownLoadTitle( item.GetItemWndTitle()) && 
			!IsKSafeWindow(item))
		{
			WndOptItem optItem;
			optItem.m_dwPId = item.GetItemPID();
			optItem.m_hIcon = GetWndIcon(item);;
			optItem.m_hWnd  = item.GetItemHwnd();
			optItem.m_nCheckState = CHK_ST_CHECK;
			optItem.m_nFixState = FIX_ST_UNFIX;
			optItem.m_nWndState = item.GetItemState();
			optItem.m_strName = item.GetItemWndTitle();
			optItem.m_nType = ITEM_TYPE_WND;
			optItem.m_strFileName = item.GetItemExePath();

			m_aryListItem.Add( optItem );
		}
	}

	for ( int i = 0; i < vecListItem.size(); i++ )
	{
		KAppWndListItemData& item = vecListItem[i];
		if( item.GetItemState() == 1 )
		{
			WndOptItem optItem;
			optItem.m_dwPId = item.GetItemPID();
			optItem.m_hIcon = GetWndIcon(item);
			optItem.m_hWnd  = item.GetItemHwnd();
			optItem.m_nCheckState = CHK_ST_CHECK;
			optItem.m_nFixState = FIX_ST_UNFIX;
			optItem.m_nWndState = item.GetItemState();
			optItem.m_strName = item.GetItemWndTitle();
			optItem.m_nType = ITEM_TYPE_WND;
			optItem.m_strFileName = item.GetItemExePath();

			m_aryListItem.Add( optItem );
		}
	}
}

LRESULT	CBKSafeWndOptdlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle )
{
	if( m_hFixThread != NULL && m_hFixThread != INVALID_HANDLE_VALUE )
		CloseHandle( m_hFixThread );

	for ( int i = 0; i < m_aryListItem.GetCount(); i++ )
	{
		WndOptItem& item = m_aryListItem[i];
		if( item.m_hIcon )
			DestroyIcon( item.m_hIcon );
	}

	return TRUE;
}

void CBKSafeWndOptdlg::OnBtnStartWndOpt()
{

	CString strText = GetItemText( IDC_BTN_START_WND_OPT );
	if( strText == BkString::Get(STR_BTN_EXIT_TEXT) )
	{
		m_pMainDlg->SetWinOptDlgHasDoModal(FALSE);
		EndDialog( 0 );
		return;
	}

	SetItemVisible( IDC_DIV_BEFORE_CHECK_WND,	FALSE );
	SetItemVisible( IDC_DIV_CHECKING_WND,		TRUE );
	SetItemIntAttribute( IDC_BMP_OPTING, "sub", 0 );

	SetTimer( TIMER_ID_CHECK_WND, 100, NULL );

	GetFixArray();

	if( m_hFixThread != NULL && m_hFixThread != INVALID_HANDLE_VALUE )
		CloseHandle( m_hFixThread );

	m_nFixAllCnt	= m_aryFixList.GetCount();
	m_bStopFix		= FALSE;
	m_nFixCurIndex	= 0;
	m_nPrcess		= 0;
	m_nFixingIndex  = 0;

	m_hFixThread = CreateThread(NULL, 0, FixWndProc, this, 0, NULL);

}

void CBKSafeWndOptdlg::GetFixArray()
{
	m_aryFixList.RemoveAll();

	for ( int i = 0; i < m_aryListItem.GetCount(); i++ )
	{
		WndOptItem& item = m_aryListItem[i];
		if( item.m_nCheckState == CHK_ST_CHECK && item.m_nType != ITEM_TYPE_TITLE )
		{
			item.m_nIndex = i;
			m_aryFixList.Add( item );
		}
	}
}

int CBKSafeWndOptdlg::GetRestUnFixCnt()
{
	int nCnt = 0;
	for ( int i = 0; i < m_aryListItem.GetCount(); i++ )
	{
		WndOptItem& item = m_aryListItem[i];
		if( ( item.m_nCheckState == CHK_ST_CHECK ||
			item.m_nCheckState == CHK_ST_UNCHECK ) &&
			item.m_nType != ITEM_TYPE_TITLE )
		{
			nCnt++;
		}
	}

	return nCnt;
}

void CBKSafeWndOptdlg::OnBtnStopWndOpt()
{
	m_bStopFix = TRUE;
}

DWORD WINAPI CBKSafeWndOptdlg::FixWndProc(__in  LPVOID lpParameter)
{
	CBKSafeWndOptdlg* pThis = (CBKSafeWndOptdlg*)lpParameter;
	if( pThis )
		pThis->DoFixWnd();

	return TRUE;
}

void CBKSafeWndOptdlg::DoFixWnd()
{
	ULONGLONG uSize = 0;
	MEMORYSTATUSEX statex = {0};
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);
	ULONGLONG nMem = statex.ullAvailPhys;
	ULONGLONG nFreeSize = 0;

	for ( int i = 0; i < m_aryFixList.GetCount(); i++ )
	{
		if( m_bStopFix )
			break;

		SendMessage( MSG_START_FIX_ITEM, (WPARAM)m_aryFixList[i].m_nIndex, 	(LPARAM)(&m_aryFixList[i]) );

		WndOptItem& item = m_aryFixList[i];
		
		if( item.m_nType == ITEM_TYPE_MEM )
		{
			uSize = FreeMemory( item );
		}
		else
		{
			CloseOneWnd( item );
		}

		SendMessage( MSG_END_FIX_ITEM, (WPARAM)m_aryFixList[i].m_nIndex, (LPARAM)(&m_aryFixList[i]) );

		if( m_bStopFix )
			break;
	}

	GlobalMemoryStatusEx (&statex);
	if (statex.ullAvailPhys > nMem)
	{
		uSize = statex.ullAvailPhys - nMem;
	}

	SendMessage( MSG_FINISH_FIX, (WPARAM)&uSize, (LPARAM)m_bStopFix );
}

BOOL CBKSafeWndOptdlg::CloseOneWnd( WndOptItem& item )
{
	item.m_nFixState = FIX_ST_FIXED;
	item.m_nCheckState = CHK_ST_FIXED;

	if( item.m_nWndState == 1 )
	{
		if( !m_bStopFix )
			_KillProcess2( item.m_dwPId );
	}
	else
	{
		if( !m_bStopFix )
			_DoCloseWnd( item.m_hWnd );
	}

	DWORD dwTimeOut = 0;
	while(TRUE)
	{
		BOOL bWndExist = ::IsWindow( item.m_hWnd ) && ::IsWindowVisible(item.m_hWnd );

		if (!bWndExist)
			break;

		if( m_bStopFix )
			break;

		Sleep(100);
		dwTimeOut++;
		if (dwTimeOut >= 30)
			break;
	}

	if( ::IsWindow(item.m_hWnd) && ::IsWindowVisible(item.m_hWnd) )
	{
		item.m_nFixState = FIX_ST_FIXFAILED;
		item.m_nCheckState = CHK_ST_FIXFALED;
	}

	return ( item.m_nFixState == FIX_ST_FIXED ) ;
}

//这里做关闭窗口的处理，可以很简单也可以很复杂
void CBKSafeWndOptdlg::_DoCloseWnd( HWND hWnd )
{
	::PostMessage( hWnd, WM_CLOSE, 0, 0 );
}

BOOL CBKSafeWndOptdlg::_KillProcess2(DWORD dwPID)
{
	KPerfMonListItemData ItemData;
	return ItemData._KillProcess(dwPID);
}


ULONGLONG CBKSafeWndOptdlg::FreeMemory( WndOptItem& item )
{
	item.m_nFixState = FIX_ST_FREED;
	item.m_nCheckState = CHK_ST_FIXED;
	ULONGLONG uSize = CSysPerfOpt::Instance()->ForceFreeMem( FALSE );
	return uSize;
}

LRESULT	CBKSafeWndOptdlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle )
{
	UINT uIdEvent = (UINT)wParam;
	//static int s_nProces = 0;

	if( uIdEvent == TIMER_ID_CHECK_WND )
	{
		SetItemIntAttribute( IDC_BMP_OPTING, "sub", m_nWndGif );
		m_nWndGif ++;
		//s_nProces ++;

		if( m_nFixAllCnt ==  0)
			return TRUE;

		m_nPrcess += 5 / m_nFixAllCnt ;

		if( 100 * m_nFixCurIndex / m_nFixAllCnt > m_nPrcess && m_nPrcess < 100 )
		{
			
			SetItemIntAttribute( IDC_WND_OPT_PROGRESS, "value", (int)m_nPrcess);
		}

		if( m_nWndGif > 11 )
			m_nWndGif = 0;

		m_wndOptList.RedrawItem( m_nFixingIndex );
	}

	return TRUE;
}

void CBKSafeWndOptdlg::AddProcessItem(vector<KAppWndListItemData>& vecListItem)
{
	//m_nPidCnt = GetPidCnt(vecListItem);
	if( m_nPidCnt > 0 )
	{
		WndOptItem item;
		item.m_nType = ITEM_TYPE_TITLE;
		item.m_strName.Format( TEXT("建议关闭的程序（%d）"), m_nPidCnt );
		m_aryListItem.Add( item );
	}
	
	for ( int i = 0; i < vecListItem.size(); i++ )
	{
		KAppWndListItemData& item = vecListItem[i];
		if( item.GetItemState() == 1 )
		{
			WndOptItem optItem;
			optItem.m_dwPId = item.GetItemPID();
			optItem.m_hIcon = GetWndIcon(item);
			optItem.m_hWnd  = item.GetItemHwnd();
			optItem.m_nCheckState = CHK_ST_CHECK;
			optItem.m_nFixState = FIX_ST_UNFIX;
			optItem.m_nWndState = item.GetItemState();
			optItem.m_strName = item.GetItemWndTitle();
			optItem.m_nType = ITEM_TYPE_PRO;
			optItem.m_strFileName = item.GetItemExePath();

			m_aryListItem.Add( optItem );
		}
	}
}

HICON CBKSafeWndOptdlg::GetWndIcon(KAppWndListItemData& ItemData)
{
	HICON hIcon = CListBoxItemData::GetDataPtr()->GetWndIcon(ItemData);
	if( hIcon == NULL )
	{
		hIcon = CListBoxItemData::GetDataPtr()->GetSmallIcon( ItemData.GetItemExePath() );
	}

	if (hIcon == NULL)
	{
		hIcon = CListBoxItemData::GetDataPtr()->GetDefaultIcon();
	}

	if( hIcon )
		return CopyIcon( hIcon );
	else
		return NULL;
}


void CBKSafeWndOptdlg::UpdateWndOptInfo(vector<KAppWndListItemData>& vecListItem)
{
	AddMemoryItem(vecListItem);
	AddWndItem(vecListItem);
	//AddProcessItem(vecListItem);
}

void CBKSafeWndOptdlg::InitCtrl()
{
	m_wndOptList.Create( GetViewHWND(), IDC_WND_OPT_LIST);
	m_wndOptList.Load(IDR_BK_WND_OPT_LIST);
	m_wndOptList.SetItemFixHeight(27);
	m_wndOptList.SetBkColor(RGB(0xFF,0xFF,0xFF));

	m_wndOptList.SetItemCount( m_aryListItem.GetCount() );
}

LRESULT	CBKSafeWndOptdlg::OnListBoxLClickCtrl(LPNMHDR pnmh)
{

	LPBKLBMITEMCLICK pnms = (LPBKLBMITEMCLICK)pnmh;
	if ( pnms->nListItemID >= m_aryListItem.GetCount() && pnms->nListItemID < 0 )
		return E_FAIL;

	WndOptItem& Item = m_aryListItem[pnms->nListItemID];

	if (pnms->uCmdID == IDC_LIST_BMP_CHECK || pnms->uCmdID == IDC_LIST_MEM_BMP_CHECK)
	{
		if( Item.m_nCheckState == CHK_ST_CHECK )
			Item.m_nCheckState = CHK_ST_UNCHECK;
		else if( Item.m_nCheckState == CHK_ST_UNCHECK )
			Item.m_nCheckState = CHK_ST_CHECK;

		UpdateCheckAllState();

		m_wndOptList.RedrawItem( pnms->nListItemID );

		return S_OK;
	}

	return S_OK;
}

BOOL CBKSafeWndOptdlg::IsAnyCheck()
{
	for ( int i = 0; i <  m_aryListItem.GetCount(); i++ )
	{
		WndOptItem& item = m_aryListItem[i];
		if( item.m_nType != ITEM_TYPE_TITLE && item.m_nCheckState == CHK_ST_CHECK )
			return TRUE;
	}

	return FALSE;
}

BOOL CBKSafeWndOptdlg::IsAllCheck()
{
	for ( int i = 0; i <  m_aryListItem.GetCount(); i++ )
	{
		WndOptItem& item = m_aryListItem[i];
		if( item.m_nType != ITEM_TYPE_TITLE && item.m_nCheckState == CHK_ST_UNCHECK )
			return FALSE;
	}

	return TRUE;
}

void CBKSafeWndOptdlg::UpdateCheckAllState()
{
	SetItemCheck( IDC_WND_OPT_CHECKALL, IsAllCheck() );
	EnableItem( IDC_BTN_START_WND_OPT, IsAnyCheck() );
}

LRESULT CBKSafeWndOptdlg::OnBkLvmGetDispInfo( LPNMHDR pnmh )
{
	BKLBMGETDISPINFO* pnms = (BKLBMGETDISPINFO*)pnmh;
	if( pnms->nListItemID < 0 || pnms->nListItemID >= m_aryListItem.GetCount() )
		return S_OK;
	
	WndOptItem& item = m_aryListItem[pnms->nListItemID];

	//is title
	if( item.m_nType == ITEM_TYPE_TITLE )
	{
		m_wndOptList.SetItemVisible( IDC_LIST_DIV_TITLE,	TRUE );
		m_wndOptList.SetItemVisible( IDC_LIST_NO_TITLE,		FALSE );
		m_wndOptList.SetItemVisible( IDC_LIST_DIV_MEMORY,	FALSE );
		m_wndOptList.SetItemText( IDC_LIST_TITLE,			item.m_strName );
	}
	//内存检测
	else if( item.m_nType == ITEM_TYPE_MEM)
	{

		m_wndOptList.SetItemAttribute(IDC_LIST_DIV_MEMORY, "crbg", pnms->bSelect ? "ECF9FF":"FFFFFF");

		m_wndOptList.SetItemVisible( IDC_LIST_DIV_TITLE,	FALSE );
		m_wndOptList.SetItemVisible( IDC_LIST_NO_TITLE,		FALSE );
		m_wndOptList.SetItemVisible( IDC_LIST_DIV_MEMORY,	TRUE );
		m_wndOptList.SetItemText( IDC_LIST_TEXT_MEM,		item.m_strName );

		if( item.m_nFixState == FIX_ST_FIXING )
		{
			m_wndOptList.SetItemAttribute(IDC_LIST_MEM_BMP_CHECK, "skin", "loading" );
			m_wndOptList.SetItemIntAttribute(IDC_LIST_MEM_BMP_CHECK, "sub", m_nWndGif);
		}
		else
		{
			m_wndOptList.SetItemAttribute(IDC_LIST_MEM_BMP_CHECK, "skin", "listcheck" );

			switch( item.m_nCheckState )
			{
			case CHK_ST_UNCHECK:
				m_wndOptList.SetItemIntAttribute(IDC_LIST_MEM_BMP_CHECK, "sub", 1);
				break;
			case CHK_ST_CHECK:
				m_wndOptList.SetItemIntAttribute(IDC_LIST_MEM_BMP_CHECK, "sub", 0);
				break;
			case CHK_ST_FIXED:
				m_wndOptList.SetItemIntAttribute(IDC_LIST_MEM_BMP_CHECK, "sub", 2);
				break;
			case CHK_ST_FIXFALED:
				m_wndOptList.SetItemIntAttribute(IDC_LIST_MEM_BMP_CHECK, "sub", 3);
				break;
			}
		}
		
		CString strFixState = BkString::Get( STR_FIX_ST_CLOSED );
		COLORREF clrText = RGB(0x00, 0xCC, 0x00);
		switch( item.m_nFixState )
		{
		case FIX_ST_UNFIX:
			strFixState = BkString::Get( STR_FIX_ST_UNCLOSE );
			clrText = RGB(0xCC, 0x66, 0x00);
			break;
		case FIX_ST_UNFREE:
			strFixState = BkString::Get( STR_FIX_ST_UNFREE );
			clrText = RGB(0xCC, 0x66, 0x00);
			break;
		case FIX_ST_FIXING:
			strFixState = BkString::Get( STR_FIX_ST_FIXING );
			clrText = RGB(0x00, 0xCC, 0xFF);
			break;
		case FIX_ST_FIXED:
			strFixState = BkString::Get( STR_FIX_ST_CLOSED );
			clrText = RGB(0x00, 0xCC, 0x00);
			break;
		case FIX_ST_FREED:
			strFixState = BkString::Get( STR_FIX_ST_FREED );
			clrText = RGB(0x00, 0xCC, 0x00);
			break;
		case FIX_ST_FIXFAILED:
			strFixState = BkString::Get( STR_FIX_ST_FIXFAILED );
			clrText = RGB(0xFC, 0x03, 0x00);
			break;
		}

		m_wndOptList.SetItemText( IDC_LIST_MEM_STATE, strFixState );
		m_wndOptList.SetItemColorAttribute(IDC_LIST_MEM_STATE, "crtext", clrText );
	}
	//窗口和进程
	else
	{
		m_wndOptList.SetItemAttribute(IDC_LIST_NO_TITLE, "crbg", pnms->bSelect ? "ECF9FF":"FFFFFF");
		m_wndOptList.SetItemVisible( IDC_LIST_DIV_TITLE,	FALSE );
		m_wndOptList.SetItemVisible( IDC_LIST_DIV_MEMORY,	FALSE );
		m_wndOptList.SetItemVisible( IDC_LIST_NO_TITLE,		TRUE );
		m_wndOptList.SetItemText(IDC_LIST_WND_NAME, item.m_strName );

		if( item.m_hIcon )
		{
			char pszValue[MAX_PATH] = {0};
			m_wndOptList.SetItemAttribute(IDC_LIST_WND_ICON,	"iconhandle", "0");
			m_wndOptList.SetItemAttribute(IDC_LIST_WND_ICON,	"srcfile", "0");
			_snprintf_s(pszValue, sizeof(pszValue), "%d", item.m_hIcon);
			m_wndOptList.SetItemAttribute(IDC_LIST_WND_ICON,	"iconhandle", pszValue);

		}

		if( item.m_nWndState == WND_ST_NORESPONSE )
		{
			m_wndOptList.SetItemText( IDC_LIST_WND_STATE, BkString::Get( STR_WND_ST_NORESPONSE) );
			m_wndOptList.SetItemColorAttribute(IDC_LIST_WND_STATE, "crtext", RGB(0xFC, 0x03, 0x00) );
		}
		else
		{
			m_wndOptList.SetItemText( IDC_LIST_WND_STATE, BkString::Get( STR_WND_ST_NOUSE) );
			m_wndOptList.SetItemColorAttribute(IDC_LIST_WND_STATE, "crtext", RGB(0x00, 0xCC, 0x00) );
		}

		if( item.m_nFixState == FIX_ST_FIXING )
		{
			m_wndOptList.SetItemAttribute(IDC_LIST_BMP_CHECK, "skin", "loading" );
			m_wndOptList.SetItemIntAttribute(IDC_LIST_BMP_CHECK, "sub", m_nWndGif);
		}
		else
		{
			m_wndOptList.SetItemAttribute(IDC_LIST_BMP_CHECK, "skin", "listcheck" );

			switch( item.m_nCheckState )
			{
			case CHK_ST_UNCHECK:
				m_wndOptList.SetItemIntAttribute(IDC_LIST_BMP_CHECK, "sub", 1);
				break;
			case CHK_ST_CHECK:
				m_wndOptList.SetItemIntAttribute(IDC_LIST_BMP_CHECK, "sub", 0);
				break;
			case CHK_ST_FIXED:
				m_wndOptList.SetItemIntAttribute(IDC_LIST_BMP_CHECK, "sub", 2);
				break;
			case CHK_ST_FIXFALED:
				m_wndOptList.SetItemIntAttribute(IDC_LIST_BMP_CHECK, "sub", 3);
				break;
			}
		}

		CString strFixState = BkString::Get( STR_FIX_ST_CLOSED );
		COLORREF clrText = RGB(0x00, 0xCC, 0x00);
		switch( item.m_nFixState )
		{
		case FIX_ST_UNFIX:
			strFixState = BkString::Get( STR_FIX_ST_UNCLOSE );
			clrText = RGB(0xCC, 0x66, 0x00);
			break;
		case FIX_ST_UNFREE:
			strFixState = BkString::Get( STR_FIX_ST_UNFREE );
			clrText = RGB(0xCC, 0x66, 0x00);
			break;
		case FIX_ST_FIXING:
			strFixState = BkString::Get( STR_FIX_ST_FIXING );
			clrText = RGB(0x00, 0xCC, 0xFF);
			break;
		case FIX_ST_FIXED:
			strFixState = BkString::Get( STR_FIX_ST_CLOSED );
			clrText = RGB(0x00, 0xCC, 0x00);
			break;
		case FIX_ST_FREED:
			strFixState = BkString::Get( STR_FIX_ST_FREED );
			clrText = RGB(0x00, 0xCC, 0x00);
			break;
		case FIX_ST_FIXFAILED:
			strFixState = BkString::Get( STR_FIX_ST_FIXFAILED );
			clrText = RGB(0xFC, 0x03, 0x00);
			break;
		}

		m_wndOptList.SetItemText( IDC_LIST_WND_CHECK_STATE, strFixState );
		m_wndOptList.SetItemColorAttribute(IDC_LIST_WND_CHECK_STATE, "crtext", clrText );
	}

	return TRUE;
}

void CBKSafeWndOptdlg::OnBkClose()
{
	m_pMainDlg->SetWinOptDlgHasDoModal(FALSE);
	EndDialog(0);
}

void CBKSafeWndOptdlg::OnOk()
{
	m_pMainDlg->SetWinOptDlgHasDoModal(FALSE);
	EndDialog(0);
}
