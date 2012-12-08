#include "StdAfx.h"
#include "DlgViewDetail.h"
#include "BeikeVulfixEngine.h"
#include "../beikesafemsgbox.h"

static int AppendItemInstalledList_WithDate( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	int nItem = listCtrl.Append( pVulItem->strPubDate );
    listCtrl.AppendSubItem(nItem, pVulItem->strPubDate);
	if(pVulItem->nType==VTYPE_SOFTLEAK)
	{
		listCtrl.AppendSubItem(nItem, pVulItem->strName);
		listCtrl.AppendSubItem(nItem, pVulItem->strDesc);
		listCtrl.AppendSubItem(nItem, BkString::Get(IDS_VULFIX_5072), SUBITEM_LINK);
	}
	else
	{
		CString strTitle;
		FormatKBString(pVulItem->nID, strTitle);
		listCtrl.AppendSubItem(nItem, strTitle);
		listCtrl.AppendSubItem(nItem, pVulItem->strName);
		listCtrl.AppendSubItem(nItem, BkString::Get(IDS_VULFIX_5139), SUBITEM_LINK);
	}
	listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);
	return nItem;
}

static int AppendItemInstalledList_NoDate( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	int nItem = -1;
	if(pVulItem->nType==VTYPE_SOFTLEAK)
	{
		nItem = listCtrl.Append( pVulItem->strName );
        listCtrl.AppendSubItem(nItem, pVulItem->strName);
		listCtrl.AppendSubItem(nItem, pVulItem->strDesc);
		listCtrl.AppendSubItem(nItem, BkString::Get(IDS_VULFIX_5072), SUBITEM_LINK);
	}
	else
	{
		CString strTitle;
		FormatKBString(pVulItem->nID, strTitle);
		nItem = listCtrl.Append( strTitle );
        listCtrl.AppendSubItem(nItem, strTitle);
		listCtrl.AppendSubItem(nItem, pVulItem->strName);
		listCtrl.AppendSubItem(nItem, BkString::Get(IDS_VULFIX_5139), SUBITEM_LINK);
	}
	listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);
	return nItem;
}

static int AppendItemIgnoredList( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	CString strTitle, strSummary;
	if(pVulItem->nType==VTYPE_SOFTLEAK)
	{
		strTitle = pVulItem->strName;
		strSummary = pVulItem->strDesc;
	}
	else
	{
		FormatKBString(pVulItem->nID, strTitle);
		strSummary = pVulItem->strName;
	}
	
	int nItem = listCtrl.Append( GetLevelDesc(pVulItem->nWarnLevel), LISTITEM_CHECKBOX );
    listCtrl.AppendSubItem(nItem, GetLevelDesc(pVulItem->nWarnLevel));
	listCtrl.AppendSubItem(nItem, pVulItem->strPubDate);
	listCtrl.AppendSubItem(nItem, strTitle);
	listCtrl.AppendSubItem(nItem, strSummary);
	listCtrl.AppendSubItem(nItem, BkString::Get(IDS_VULFIX_5139), SUBITEM_LINK);
	listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);
	return nItem;
}

static int AppendItemReplacedList( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	CString strTitle;
	FormatKBString(pVulItem->nID, strTitle);

	int nItem = listCtrl.Append( pVulItem->strPubDate );
    listCtrl.AppendSubItem(nItem, pVulItem->strPubDate);
	listCtrl.AppendSubItem(nItem, strTitle);
	listCtrl.AppendSubItem(nItem, pVulItem->strName);
	listCtrl.AppendSubItem(nItem, BkString::Get(IDS_VULFIX_5139), SUBITEM_LINK);
	listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);
	return nItem;
}

static int AppendItemInvalidList( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	CString strTitle;
	FormatKBString(pVulItem->nID, strTitle);

	int nItem = listCtrl.Append( pVulItem->strPubDate );
    listCtrl.AppendSubItem(nItem, pVulItem->strPubDate);
	listCtrl.AppendSubItem(nItem, strTitle);
	listCtrl.AppendSubItem(nItem, pVulItem->strName);
	listCtrl.AppendSubItem(nItem, BkString::Get(IDS_VULFIX_5139), SUBITEM_LINK);
	listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);
	return nItem;
}


CDlgViewDetail::CDlgViewDetail(void)
{
	m_bDirty = FALSE;
	m_bDisplayInstallDate = TRUE;
	m_nTab = m_nCurrentTab = 0;
}

CDlgViewDetail::~CDlgViewDetail(void)
{
}

void CDlgViewDetail::SetInitTab( INT nTab )
{
	m_nTab = nTab;
}

LRESULT CDlgViewDetail::OnInitDialog( HWND /*hWnd*/, LPARAM /*lParam*/ )
{
	// Fixed 
	m_wndListFixed.Create( 
		GetViewHWND(), NULL, NULL, 
		WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL , 
		0, 20010, NULL);
	{
		m_bDisplayInstallDate = !theEngine->IsUsingInterface();
		
		INT nRow = 0;
		if( m_bDisplayInstallDate )
		{
			m_wndListFixed.InsertColumn(nRow++, BkString::Get(IDS_VULFIX_5140), LVCFMT_LEFT, 80);
		}
		m_wndListFixed.InsertColumn(nRow++, BkString::Get(IDS_VULFIX_5141), LVCFMT_LEFT, 80);
		m_wndListFixed.InsertColumn(nRow++, BkString::Get(IDS_VULFIX_5142), LVCFMT_LEFT, 280 + (m_bDisplayInstallDate ? 0:80) );
		m_wndListFixed.InsertColumn(nRow++, BkString::Get(IDS_VULFIX_5143), LVCFMT_LEFT, 80);
	}
	m_wndListFixed.SetObserverWindow( m_hWnd );
	m_wndListFixed.SetEmptyString( BkString::Get(IDS_VULFIX_5144) );
	
	// Ignored 
	m_wndListIgnored.Create( 
		GetViewHWND(), NULL, NULL, 
		WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL , 
		0, 30010, NULL);
	{
		m_wndListIgnored.InsertColumn(0, BkString::Get(IDS_VULFIX_5145), LVCFMT_LEFT, 90);
		m_wndListIgnored.InsertColumn(1, BkString::Get(IDS_VULFIX_5146), LVCFMT_LEFT, 70);
		m_wndListIgnored.InsertColumn(2, BkString::Get(IDS_VULFIX_5147), LVCFMT_LEFT, 70);
		m_wndListIgnored.InsertColumn(3, BkString::Get(IDS_VULFIX_5148), LVCFMT_LEFT, 235);
		m_wndListIgnored.InsertColumn(4, BkString::Get(IDS_VULFIX_5149), LVCFMT_LEFT, 60);
	}
	m_wndListIgnored.SetObserverWindow( m_hWnd );
	m_wndListIgnored.SetEmptyString( BkString::Get(IDS_VULFIX_5150) );
	
	// Superseded 
	m_wndListSuperseded.Create( 
		GetViewHWND(), NULL, NULL, 
		WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL , 
		0, 40010, NULL);
	{
        m_wndListSuperseded.InsertColumn(0, BkString::Get(IDS_VULFIX_5146), LVCFMT_LEFT, 80);
        m_wndListSuperseded.InsertColumn(1, BkString::Get(IDS_VULFIX_5147), LVCFMT_LEFT, 100);
        m_wndListSuperseded.InsertColumn(2, BkString::Get(IDS_VULFIX_5148), LVCFMT_LEFT, 260);
        m_wndListSuperseded.InsertColumn(3, BkString::Get(IDS_VULFIX_5149), LVCFMT_LEFT, 80);
	}
	m_wndListSuperseded.SetObserverWindow( m_hWnd );
	m_wndListFixed.SetEmptyString( BkString::Get(IDS_VULFIX_5151) );
	
	// Invalid
	m_wndListInvalid.Create( 
		GetViewHWND(), NULL, NULL, 
		WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL , 
		0, 50010, NULL);
	{
        m_wndListInvalid.InsertColumn(0, BkString::Get(IDS_VULFIX_5146), LVCFMT_LEFT, 70);
        m_wndListInvalid.InsertColumn(1, BkString::Get(IDS_VULFIX_5147), LVCFMT_LEFT, 70);
        m_wndListInvalid.InsertColumn(2, BkString::Get(IDS_VULFIX_5148), LVCFMT_LEFT, 300);
        m_wndListInvalid.InsertColumn(3, BkString::Get(IDS_VULFIX_5149), LVCFMT_LEFT, 80);
	}
	m_wndListInvalid.SetObserverWindow( m_hWnd );
	m_wndListInvalid.SetEmptyString( BkString::Get(IDS_VULFIX_5152) );

	// 
	m_ctlTitleFixed.Create(GetViewHWND(), NULL, NULL, WS_VISIBLE|WS_CHILD|SS_NOTIFY, 0, 20000, NULL);
	m_ctlTitleIgnored.Create(GetViewHWND(), NULL, NULL, WS_VISIBLE|WS_CHILD|SS_NOTIFY, 0, 30000, NULL);	
	m_ctlTitleSuperseded.Create(GetViewHWND(), NULL, NULL, WS_VISIBLE|WS_CHILD|SS_NOTIFY, 0, 40000, NULL);
	m_ctlTitleInvalid.Create(GetViewHWND(), NULL, NULL, WS_VISIBLE|WS_CHILD|SS_NOTIFY, 0, 50000, NULL);
	
	COLORREF clrBackground = BACKGROUND_COLOR;
	m_ctlTitleFixed.SetBackgroudColor( clrBackground );
	m_ctlTitleIgnored.SetBackgroudColor( clrBackground );
	m_ctlTitleSuperseded.SetBackgroudColor( clrBackground );
	m_ctlTitleInvalid.SetBackgroudColor( clrBackground );

	m_bLoadedFixed = m_bLoadedIgnored = m_bLoadedSuperseded = m_bLoadedInvalid = FALSE;
	PostMessage(WMH_INIT_SCAN, 0, 0);
	return 0;
}

LRESULT CDlgViewDetail::OnInitScan( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	SetTabCurSel(IDC_TAB_MAIN, m_nTab);
	return 0;
}

LRESULT CDlgViewDetail::OnListLinkClicked( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/ )
{	
	CListViewCtrlEx *pList = NULL;
	switch( m_nCurrentTab )
	{
	case 0:
		pList = &m_wndListFixed;
		break;
	case 1:
		pList = &m_wndListIgnored;
		break; 
	case 2:
		pList = &m_wndListSuperseded;
		break;
	case 3:
		pList = &m_wndListInvalid;
		break;
	}
	if(!pList)
		return 0;
	
	T_VulListItemData *pItem = (T_VulListItemData *)pList->GetItemData( wParam );
	if(pItem)
	{
		// 已安装的Tab 软件的点击肯定是Enable COM
		if( m_nCurrentTab==0 && pItem->nType==VTYPE_SOFTLEAK)
		{
			if( theEngine->m_pVulScan )
			{
				theEngine->m_pVulScan->EnableVulCOM( pItem->nID, TRUE );
				pList->DeleteItem( wParam );
				m_bDirty = TRUE;
				_UpdateFixedTitle();
			}
		}
		else
		{
			//NOTE: 由于这里获取的 pItem->strWebPage , 由于为了加速, 并没有从数据库中读取出来, 所以直接使用kb url 访问
			CString strPage;
			FormatKBWebUrl( strPage, pItem->nID );
			ShellExecute(NULL, _T("open"), strPage, NULL, NULL, SW_SHOW);
		}
	}
	return 0;
}

BOOL CDlgViewDetail::OnBkTabMainSelChange( int nTabItemIDOld, int nTabItemIDNew )
{
	DEBUG_TRACE(L"Tab Change %d, %d\r\n", nTabItemIDOld, nTabItemIDNew);	
	switch (nTabItemIDNew)
	{
	case 0:
		_FillFixed();
		break;
	case 1:
		_FillIgnored();
		break;
	case 2:
		_FillSuperseded();
		break;
	case 3: 
		_FillInvalid();
		break;
	}
	m_nCurrentTab = nTabItemIDNew;
	return TRUE;
}

void CDlgViewDetail::OnBkBtnClose()
{
	EndDialog(m_bDirty ? IDOK : IDCANCEL);
}

void CDlgViewDetail::OnBkBtnOpenHotfixDiretory()
{
	CString strPath;
	GetDownloadPath( strPath );
	if ( !PathIsDirectory(strPath) )
		CreateDirectoryNested(strPath);
	
	if(PathIsDirectory(strPath))
		ShellExecute(NULL, _T("open"), strPath, NULL, NULL, SW_SHOW);
	else
		CBkSafeMsgBox::Show( BkString::Get(IDS_VULFIX_5153), NULL, MB_OK | MB_ICONERROR);
}

void CDlgViewDetail::OnBkBtnSelectAll()
{
	m_wndListIgnored.CheckAll();
}

void CDlgViewDetail::OnBkBtnSelectNone()
{
	m_wndListIgnored.CleanCheck();
}

void CDlgViewDetail::OnBkBtnCancelIgnore()
{
	CSimpleArray<int> arr;
	if(GetListCheckedItems(m_wndListIgnored, arr))
	{
		m_bDirty = TRUE;
		theEngine->IgnoreVuls(arr, false);
		ListDeleteItems(m_wndListIgnored, arr);
	}
	else
		CBkSafeMsgBox::Show( BkString::Get(IDS_VULFIX_5154), NULL, MB_OK | MB_ICONWARNING );
	_UpdateIgnoredTitle();
}

void CDlgViewDetail::OnBkBtnExport()
{
    ::SendMessage(::GetParent(m_hWnd), MSG_USER_VUL_EXPORT, NULL, NULL);
}

void CDlgViewDetail::_AppendItem2ListCtrl( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	if(m_bDisplayInstallDate)
		AppendItemInstalledList_WithDate( listCtrl, pVulItem );
	else
		AppendItemInstalledList_NoDate( listCtrl, pVulItem );
}

void CDlgViewDetail::_FillFixed()
{
	if(m_bLoadedFixed)
		return;
	m_bLoadedFixed = TRUE;
	ResetListCtrl(m_wndListFixed);
	
	if(!theEngine->m_pVulScan)
		return ;
	
	const CSimpleArray<LPTVulSoft>& arrSoft = theEngine->m_pVulScan->GetSoftVuls();
	const CSimpleArray<TItemFixed*>& arr = theEngine->m_pVulScan->GetFixedVuls();
	
	int nFixedSoft = CountItems(arrSoft, SoftComRepaired);
	// fixed soft by com 
	if(nFixedSoft)
	{
		for(int i=0; i<arrSoft.GetSize(); ++i)
		{
			LPTVulSoft pSoft = arrSoft[i];
			if( SoftComRepaired(pSoft) )
			{
				_AppendItem2ListCtrl( m_wndListFixed, CreateListItem(pSoft) );
			}
		}
	}
	
	for(int i=0; i<arr.GetSize(); ++i)
	{
		TItemFixed* pItem = arr[i];
		_AppendItem2ListCtrl( m_wndListFixed, CreateListItem(pItem) );
	}
	
	_UpdateFixedTitle();
}

void CDlgViewDetail::_UpdateFixedTitle()
{
	int count = m_wndListFixed.GetItemCount();
	CString strPath;
	GetDownloadPath( strPath );
	INT64 folderSize = GetFolderSize(strPath, FALSE, IsBkDownloadFile);

	CString strTitle;
	if( folderSize>0 )
	{
		CString strSize;
		FormatSizeString(folderSize, strSize);
		strTitle.Format(BkString::Get(IDS_VULFIX_5155), count, strSize );
	}
	else
	{
		strTitle.Format(BkString::Get(IDS_VULFIX_5156), count );
	}
	m_ctlTitleFixed.SetMText( strTitle );
}

void CDlgViewDetail::_FillIgnored()
{
	if(m_bLoadedIgnored)
		return;
	m_bLoadedIgnored = TRUE;
	if(!theEngine->m_pVulScan)
		return ;
	
	ResetListCtrl(m_wndListIgnored);
	const CSimpleArray<LPTUpdateItem>& arr = theEngine->m_pVulScan->GetIgnoredVuls();
	for(int i=0; i<arr.GetSize(); ++i)
	{
		LPTUpdateItem pItem = arr[i];
		AppendItemIgnoredList( m_wndListIgnored, CreateListItem(pItem) );
	}
	_UpdateIgnoredTitle();
}

void CDlgViewDetail::_UpdateIgnoredTitle()
{
	int count = m_wndListIgnored.GetItemCount();
	CString strTitle;
	if( 0==count )
	{
		strTitle = BkString::Get(IDS_VULFIX_5157);
		
	}
	else
		strTitle.Format(BkString::Get(IDS_VULFIX_5158), count );
	m_ctlTitleIgnored.SetMText( strTitle );	
	_EnableIgnoreSelectLink( count>0 );
	_EnableIgnoreButton( count>0 );
}

void CDlgViewDetail::_FillSuperseded()
{
	if(m_bLoadedSuperseded)
		return;
	m_bLoadedSuperseded = TRUE;
	if(!theEngine->m_pVulScan)
		return ;
	
	ResetListCtrl(m_wndListSuperseded);
	const CSimpleArray<TReplacedUpdate*>& arr = theEngine->m_pVulScan->GetReplacedVuls();			
	ResetListCtrl(m_wndListSuperseded);
	for(int i=0; i<arr.GetSize(); ++i)
	{
		TReplacedUpdate *pItem = arr[i];
		AppendItemReplacedList( m_wndListSuperseded, CreateListItem(pItem) );
	}

	CString strTitle;
	if( arr.GetSize() )
		strTitle.Format(BkString::Get(IDS_VULFIX_5159), arr.GetSize() );
	else
		strTitle = BkString::Get(IDS_VULFIX_5160), arr.GetSize();
	m_ctlTitleSuperseded.SetMText( strTitle );
}

void CDlgViewDetail::_FillInvalid()
{
	if(m_bLoadedInvalid)
		return;
	m_bLoadedInvalid = TRUE;
	if(!theEngine->m_pVulScan)
		return ;
	
	ResetListCtrl(m_wndListInvalid);
	const CSimpleArray<LPTUpdateItem>& arr = theEngine->m_pVulScan->GetInvalidVuls();
	for(int i=0; i<arr.GetSize(); ++i)
	{
		LPTUpdateItem pItem = arr[i];
		AppendItemInvalidList( m_wndListInvalid, CreateListItem(pItem) );
	}
	CString strTitle;
	if( 0==arr.GetSize() )
		strTitle = BkString::Get(IDS_VULFIX_5161);
	else
		strTitle.Format(BkString::Get(IDS_VULFIX_5162), arr.GetSize() );
	m_ctlTitleInvalid.SetMText( strTitle );
}

void CDlgViewDetail::_EnableIgnoreButton( BOOL bChecked )
{
	EnableItem(30502, bChecked);
}

void CDlgViewDetail::_EnableIgnoreSelectLink( BOOL bChecked )
{
	EnableItem(30500, bChecked);
	EnableItem(30501, bChecked);
	EnableItem(30510, bChecked);	
}