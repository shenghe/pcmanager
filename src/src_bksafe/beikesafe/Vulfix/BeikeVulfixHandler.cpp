#include "StdAfx.h"
#include "BeikeVulfixHandler.h"
#include <process.h>
#include <algorithm>
#include <functional>
#include <bksafe/bksafeconfig.h>
#include <safemon/safetrayshell.h>
#include "../beikesafemsgbox.h"

#include "BeikeVulfixEngine.h"
#include "DlgViewDetail.h"
#include "DlgShutdownPrompt.h"
#include "../beikesafemaindlg.h"
#include "DlgTipDetail.h"


static bool NeedRepair( LPTUpdateItem pItem )
{
	return !pItem->isIgnored && pItem->nWarnLevel>=0;
}

static bool NeedRepair( LPTVulSoft pItem )
{
	return pItem->state.comState!=COM_ALL_DISABLED;
}

static int AppendItemSoftVulList( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
    CString strSize;
	int state = GetSoftItemState( pVulItem );
	LPCTSTR lpszWarnDesc = GetLevelDesc(pVulItem->nWarnLevel);
	int nItem = listCtrl.Append( lpszWarnDesc, LISTITEM_CHECKBOX );	
	listCtrl.AppendSubItem(nItem, pVulItem->strName);
    FormatSizeString(pVulItem->nFileSize, strSize);    
	listCtrl.AppendSubItem(nItem, strSize);
	listCtrl.AppendSubItem(nItem, pVulItem->strPubDate);
//	listCtrl.AppendSubItem(nItem,BkString::Get(IDS_VULFIX_5016) );

	if(pVulItem->nWarnLevel>=EWarn_Serious)
		listCtrl.SetSubItemColor(nItem, 0, red, false);

	/*listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);*/
	listCtrl.SetItemData(nItem, (DWORD)pVulItem);
	listCtrl.SetCheckState(nItem, true);
	return nItem;
}

static int AppendItem2VulList( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	if ( pVulItem->nType == VTYPE_SOFTLEAK )
	{
		return AppendItemSoftVulList(listCtrl, pVulItem);
	}

	CString strTitle;
	FormatKBString(pVulItem->nID, strTitle);
	
	BOOL bUseRadio = pVulItem->nWarnLevel==0 && IsFlagOn( VFLAG_SERVICE_PATCH, pVulItem->dwFlags );	// SP 
	LPCTSTR lpszWarnDesc = GetLevelDesc(pVulItem->nWarnLevel);
	int nItem = listCtrl.Append(lpszWarnDesc, bUseRadio ? LISTITEM_RADIOBOX : LISTITEM_CHECKBOX );
    listCtrl.AppendSubItem(nItem, pVulItem->strName, SUBITEM_TEXT, ST_LEFT, TRUE);
	listCtrl.AppendSubItem(nItem, strTitle);
    CString strSize;
    FormatSizeString(pVulItem->nFileSize, strSize);
    listCtrl.AppendSubItem(nItem, strSize, SUBITEM_TEXT, ST_RIGHT);	
    listCtrl.AppendSubItem(nItem, pVulItem->strPubDate, SUBITEM_TEXT, ST_RIGHT);


// 	if ( pVulItem->nWarnLevel == -1 )
// 		listCtrl.AppendSubItem(nItem,BkString::Get(IDS_VULFIX_5017) );
// 	else if ( pVulItem->nWarnLevel == 0 )
// 		listCtrl.AppendSubItem(nItem,BkString::Get(IDS_VULFIX_5016) );
// 	else if ( pVulItem->nWarnLevel > 0 )
// 		listCtrl.AppendSubItem(nItem,BkString::Get(IDS_VULFIX_5016) );
	
	if(pVulItem->nWarnLevel>=EWarn_Serious)
		listCtrl.SetSubItemColor(nItem, 0, red, false);

	if (pVulItem->nWarnLevel>0)
		listCtrl.SetCheckState(nItem,TRUE);

	/*listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);*/
	listCtrl.SetItemData(nItem, (DWORD)pVulItem);
	return nItem;
}

static int AppendItem2RepairList( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	CString strFileSize;
	FormatSizeString(pVulItem->nFileSize, strFileSize);
	
	CString strTitle, strSummary = L"   ";
	if(pVulItem->nType==VTYPE_SOFTLEAK)
	{
		strTitle = pVulItem->strName;
		strSummary += pVulItem->strDesc;
	}
	else
	{
		FormatKBString(pVulItem->nID, strTitle);
		strSummary += pVulItem->strName;
	}
	
	int nItem = listCtrl.Append( GetLevelDesc(pVulItem->nWarnLevel) );
	listCtrl.AppendSubItem(nItem, strSummary);
	listCtrl.AppendSubItem(nItem, strTitle, SUBITEM_TEXT, ST_CENTER);
	listCtrl.AppendSubItem(nItem, strFileSize, SUBITEM_TEXT, ST_CENTER);
//  listCtrl.AppendSubItem(nItem, BkString::Get(IDS_VULFIX_5164));
// 	listCtrl.AppendSubItem(nItem, _T(""));
	/*listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);*/
	listCtrl.SetItemData(nItem, (DWORD)pVulItem);
	return nItem;
}

template<typename T, typename Func>
void AppendVuls(CListViewCtrlEx &listCtrl, const CSimpleArray<T>& arr, Func fn)
{
	for(int i=0; i<arr.GetSize(); ++i)
	{
		if( fn( arr[i] ))
		{
			AppendItem2VulList( listCtrl, CreateListItem( arr[i] ) );
		}
	}
}

bool IsMustWindowsVul(LPTUpdateItem pItem)
{
	return !pItem->isIgnored && pItem->nWarnLevel>0 && !pItem->isExclusive;
}

bool IsSPVul(LPTUpdateItem pItem)
{
	return !pItem->isIgnored && pItem->nWarnLevel==0 && (pItem->isExclusive || IsFlagOn( VFLAG_SERVICE_PATCH, pItem->dwVFlags ) );
}

bool IsOptionVul(LPTUpdateItem pItem)
{
	return !pItem->isIgnored && pItem->nWarnLevel==0 && !(pItem->isExclusive || IsFlagOn( VFLAG_SERVICE_PATCH, pItem->dwVFlags ) );
}

bool IsSoftVulInstallable( LPTVulSoft pItem )
{
	return pItem->state.comState!=COM_ALL_DISABLED;
}

bool SelectMustOnly(T_VulListItemData *pItem)
{
	return pItem->nWarnLevel>0 && !pItem->isExclusive;
}

bool SelectSuggest(T_VulListItemData *pItem)
{
	if(pItem->nType!=VTYPE_SOFTLEAK && pItem->nType>=0)
	{
		return pItem->nWarnLevel>0;
	}
	return false;
}

bool SelectOptinal(T_VulListItemData *pItem)
{
	if(pItem->nType!=VTYPE_SOFTLEAK && pItem->nType>=0)
	{
		return pItem->nWarnLevel==0;
	}
	return false;
}

CBeikeVulfixHandler::CBeikeVulfixHandler( CEmbeddedView &mainDlg ) 
	: m_hMainWnd(NULL), CBaseViewHandler<CEmbeddedView>(mainDlg)
{
	m_dwPos = 0;
	m_firstInited = TRUE;
	
	m_nCurrentRelateInfoItem = -1;
	
	m_nTotalItem = 0;
	m_nCurrentItem = 0;
	m_nRepairTotal = m_nRepairInstalled = m_nRepairDownloaded = m_nRepairProcessed = 0;
    m_nInstallProcessed = 0;
	m_nNumMust = m_nNumOption = m_nNumSP = 0;
	m_MainDlg = NULL;
    m_pTipDetailDlg = NULL;
}

CBeikeVulfixHandler::~CBeikeVulfixHandler(void)
{
}

void CBeikeVulfixHandler::SetMainDlg( CBeikeSafeMainDlg *pMainDlg )
{
	m_MainDlg = pMainDlg;
}

BOOL CBeikeVulfixHandler::Init(HWND hMainWnd, HWND hWndParent)
{
	m_hMainWnd = hMainWnd;
	ATLASSERT( IsWindow(m_hMainWnd) );
	ATLASSERT( IsWindow(hWndParent) );
	// vul list 
	m_wndListCtrlVul.Create( 
		hWndParent, NULL, NULL, 
		//WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_NOCOLUMNHEADER, 
		WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_OWNERDRAWFIXED, 
		0, IDC_LST_VULFIX_RESULT_LIST, NULL);
	{
//		m_wndListCtrlVul.InsertColumn(0, _T("选"), LVCFMT_LEFT, 26);
        m_wndListCtrlVul.InsertColumn(0, BkString::Get(IDS_VULFIX_5020), LVCFMT_CENTER, 460);
        m_wndListCtrlVul.InsertColumn(1, BkString::Get(IDS_VULFIX_5019), LVCFMT_CENTER, 100);
        m_wndListCtrlVul.InsertColumn(2, BkString::Get(IDS_VULFIX_5017), LVCFMT_CENTER, 100);
        m_wndListCtrlVul.InsertColumn(3, BkString::Get(IDS_VULFIX_5021), LVCFMT_CENTER, 100);
//		m_wndListCtrlVul.InsertColumn(4, BkString::Get(IDS_VULFIX_5022), LVCFMT_LEFT, 70);
		m_wndListCtrlVul.SetItemHeight(30);
	}
		
	// repairing list 
	m_wndListCtrlVulFixing.Create( 
		hWndParent, NULL, NULL, 
		//WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_NOCOLUMNHEADER, 
		WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL , 
		0, IDC_LST_VULFIX_FIXING_LIST, NULL);
	{
        m_wndListCtrlVulFixing.InsertColumn(0, BkString::Get(IDS_VULFIX_5020), LVCFMT_CENTER, 480);
        m_wndListCtrlVulFixing.InsertColumn(1, BkString::Get(IDS_VULFIX_5019), LVCFMT_CENTER, 130);
        m_wndListCtrlVulFixing.InsertColumn(2, BkString::Get(IDS_VULFIX_5022), LVCFMT_CENTER, 155);
//         m_wndListCtrlVulFixing.InsertColumn(3, BkString::Get(IDS_VULFIX_5023), LVCFMT_LEFT, 140);
//         m_wndListCtrlVulFixing.InsertColumn(4, BkString::Get(IDS_VULFIX_5022), LVCFMT_LEFT, 80);
// 		m_wndListCtrlVulFixing.InsertColumn(5, BkString::Get(IDS_VULFIX_5024), LVCFMT_CENTER, 60);
		m_wndListCtrlVulFixing.SetItemHeight(30);
	}
	
// 	m_ctlRichEdit.FirstInitialize( hWndParent, IDC_TXT_VULFIX_VUL_DESCRIPTION );
// 	m_ctlRichEdit.SetBackgroundColor( BACKGROUND_COLOR );
	
// 	m_ctlNaviLink.Create(hWndParent, NULL, NULL, WS_VISIBLE|WS_CHILD|SS_NOTIFY, 0, 1016, NULL);
// 	m_ctlNaviLink.SetFlags( DT_RIGHT );
// 	m_ctlNaviLink.SetMText( BkString::Get(IDS_VULFIX_5014) );
	return TRUE;
}

void CBeikeVulfixHandler::InitEnv()
{
	if(!theEngine)
	{
		theEngine = new CVulEngine;	
		theEngine->_InitFunctions();

		m_WinInfo.Init();
		//m_WinInfo64 = IsWin64();
	}
	PostMessage( WMH_SCAN_START, 0, 0);
}

void CBeikeVulfixHandler::OnBkBtnIgnore()
{
	BOOL bIsRadio = FALSE;
	CSimpleArray<int> arr;
	if( GetListCheckedItems(m_wndListCtrlVul, arr, &bIsRadio) )
	{
		LPCTSTR lpszConfirm = BkString::Get(IDS_VULFIX_5025);		
		CBkSafeMsgBox	dlg;
		if(IDYES==dlg.ShowMsg( lpszConfirm, NULL, MB_YESNO | MB_ICONQUESTION, NULL, m_hMainWnd))
		{
			theEngine->IgnoreVuls(arr, true);
			OnBkBtnScan();
		}
	}
	else
		CBkSafeMsgBox::Show( BkString::Get(IDS_VULFIX_5026), NULL, MB_OK | MB_ICONWARNING, NULL );
}

void CBeikeVulfixHandler::OnBkBtnScan()
{
	if(m_firstInited)
	{
		// Clean downloaded files 
		BOOL bSave = BKSafeConfig::Get_Vulfix_SaveDownFile();
		if( !bSave )
			theEngine->m_fixLog.CleanFiles(FALSE, NULL);
		m_firstInited = FALSE;
		SetItemVisible(1015, !theEngine->IsSystemSupported());
	}
	
	ResetListCtrl(m_wndListCtrlVul);
	if( theEngine->ScanVul( m_RefWin.m_hWnd ) )
	{
		m_dwScanBeginTime = GetTickCount();
		m_bScanStarted = FALSE;

		m_nScanState = 0;
		m_nTotalItem = 0;
		m_nCurrentItem = 0;
		_SetDisplayState(SCANSTATE_SCANNING);
		_SetScanProgress( 0 );
		m_RefWin.SetTimer(0, 200, NULL);
		m_wndListCtrlVul.SetEmptyString(BkString::Get(IDS_VULFIX_5027));
		m_RefWin.StartIconAnimate( IDC_IMG_VULFIX_SCAN_ANIMATION, IDC_PROGRESS_VULFIX_SCANNING, 300);
		SetItemDWordAttribute(IDC_PROGRESS_VULFIX_SCANNING, "showpercent", 0);
	}
}

void CBeikeVulfixHandler::OnBkBtnCancelScan()
{
	theEngine->CancelScanVul();
	m_RefWin.StopIconAnimate();
	m_wndListCtrlVul.SetEmptyString(BkString::Get(IDS_VULFIX_5028));
	_SetDisplayState(SCANSTATE_DISPLAY);
    SetItemText(IDC_TXT_VULFIX_SELECT_NUM, BkString::Get(IDS_VULFIX_5176));
    SetItemText(IDC_TXT_VULFIX_SELCET_SIZE, _T(""));
}

STATE_CLOSE_CHECK CBeikeVulfixHandler::CloseCheck()
{
	if (IsWindow(m_hMainWnd))
	{
		_NotifyTrayIcon(NULL, 0);
		if(theEngine && theEngine->m_isRepairing)
		{
			CBkSafeMsgBox2	dlg;
			LPCTSTR szMsg = BkString::Get(IDS_VULFIX_5029);
			dlg.AddButton( BkString::Get(IDS_VULFIX_5031), IDOK);
			dlg.AddButton( BkString::Get(IDS_VULFIX_5030), IDNO);
			UINT nRet = dlg.ShowMutlLineMsg(szMsg, NULL, MB_BK_CUSTOM_BUTTON|MB_ICONQUESTION, NULL);
			if( nRet==IDOK )
				return CLOSE_CHECK_FORCE_CLOSE;
			else if(nRet==IDNO)
			{
				OnBkBtnRunBackground();
				return CLOSE_CHECK_CANCEL_CLOSE;
			}
			return CLOSE_CHECK_CANCEL_CLOSE;
		}
	}
	
	return CLOSE_CHECK_CONTINUE;
}

void CBeikeVulfixHandler::_SetDisplayState( TScanSoftState st, TRepairSubState subst, LPCTSTR szMsg )
{
	static int viewnds[] = {
        IDC_DIV_VULFIX_DISPLAY, 
        IDC_DIV_VULFIX_SCANNING, 
        IDC_DIV_VULFIX_REPAIRING
    };
	int count = sizeof(viewnds) / sizeof(int);
	for(int i = 0; i < count; ++ i)
	{
// 		if(SCANSTATE_REPAIRING == st)
// 		{
// 			for(int j=0; j<=REPAIRSTATE_FAIL; ++j)
// 			{
// 				SetItemVisible(801+j, j==subst);
// 			}
// 		}

		SetItemVisible(viewnds[i], st == i);
	}

	if ( SCANSTATE_REPAIRING == st )
	{
		if ( subst == REPAIRSTATE_DONE )
		{
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_INIT,FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_DONE, TRUE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_FAILED, FALSE);

			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_INIT, FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_DONE, TRUE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_DONE_PART, FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_FAILED, FALSE);
		}
		else if ( subst == REPAIRSTATE_FAIL )
		{
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_INIT,FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_DONE, FALSE);
			//SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_FAILED, TRUE,FALSE);		
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_FAILED, FALSE);
			CString strNum;
			strNum.Format(_T("%d"), m_nRepairTotal);
			SetItemText(10210, strNum);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_INIT, FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_DONE, FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_DONE_PART, FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_FAILED, TRUE);

			if(m_nRepairDownloaded>0)
			{
				SetItemVisible( 10215, TRUE);
				SetItemVisible( 10216, FALSE);
			}
			else
			{
				SetItemVisible( 10215, FALSE);
				SetItemVisible( 10216, TRUE);
			}
		}
		else if ( subst == REPAIRSTATE_DONE_PART )
		{
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_INIT,FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_DONE, FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_FAILED, FALSE);

			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_INIT, FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_DONE, FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_DONE_PART, TRUE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_FAILED, FALSE);
		}
		else if ( subst == REPAIRSTATE_ING )
		{
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_INIT,TRUE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_DONE, FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_FAILED, FALSE);

			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_INIT, TRUE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_DONE, FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_DONE_PART, FALSE);
			SetItemVisible( IDC_DIV_VULFIX_REPAIR_TOP_FAILED, FALSE);
		}
	}
    
// 	if (SCANSTATE_DISPLAY == st)
//     {
//         SetPanelXml(IDC_DIV_VULFIX_DISPLAY_LIST_AND_DETAIL, IDR_BK_VUL_DISPLAY_SHOW_DETAIL);
//     }

    Redraw();
}

void CBeikeVulfixHandler::OnBkBtnSwitchRelateInfo()
{
	m_bRelateInfoShowing = !m_bRelateInfoShowing;
	if(m_bRelateInfoShowing)
	{
        SetPanelXml(IDC_DIV_VULFIX_DISPLAY_LIST_AND_DETAIL, IDR_BK_VUL_DISPLAY_SHOW_DETAIL);

		int i = m_nCurrentRelateInfoItem;
		if (i>=0)
		{
			m_nCurrentRelateInfoItem = -1;
			_DisplayRelateVulFixInfo(i);
		}
	}
	else
	{
        SetPanelXml(IDC_DIV_VULFIX_DISPLAY_LIST_AND_DETAIL, IDR_BK_VUL_DISPLAY_HIDE_DETAIL);
	}
}

void CBeikeVulfixHandler::OnBkBtnSelectAll()
{
    BOOL bChecked = GetItemCheck(IDC_LBL_VULFIX_RESULT_CHECK_ALL);
    
    if (bChecked)
    {
	    BOOL bSelectAll = TRUE;
	    if(m_nNumMust>0 && m_nNumOption>0)
	    {
		    LPCTSTR szMsg = BkString::Get(IDS_VULFIX_5032);
		    LPCTSTR szBtnYes = BkString::Get(IDS_VULFIX_5033);
		    LPCTSTR szBtnNo = BkString::Get(IDS_VULFIX_5034);
    		
		    CBkSafeMsgBox dlg;		
		    dlg.AddButton( szBtnYes, IDYES);
		    dlg.AddButton( szBtnNo, IDCANCEL);
		    UINT nRet = dlg.ShowMsg(szMsg, NULL, MB_BK_CUSTOM_BUTTON|MB_ICONQUESTION, NULL);
		    if(nRet==IDYES)
		    {
			    bSelectAll = FALSE;
		    }
	    }
    	
	    if(bSelectAll)
		    m_wndListCtrlVul.CheckAll();
	    else
		    m_wndListCtrlVul.CheckAll( std::ptr_fun(SelectMustOnly) );
    }
    else
    {
        m_wndListCtrlVul.CleanCheck();
    }
    PostMessage(MSG_USER_VUL_SELECT_CHANGE,NULL, NULL);
}

void CBeikeVulfixHandler::OnBkBtnSelectNone()
{
	m_wndListCtrlVul.CheckAll( std::ptr_fun(SelectSuggest) );
    PostMessage(MSG_USER_VUL_SELECT_CHANGE,NULL, NULL);
}

void CBeikeVulfixHandler::OnBkBtnSelectAllSuggested()
{
	m_wndListCtrlVul.CheckAll( std::ptr_fun(SelectSuggest) );
}

void CBeikeVulfixHandler::OnBkBtnSelectAllOptional()
{
	m_wndListCtrlVul.CheckAll( std::ptr_fun(SelectOptinal) );	
}

bool CBeikeVulfixHandler::_RepairSingle( int arrVulIds, T_VulListItemData* pItemData )
{
	if(!theEngine->CheckRepairPrequisite(pItemData->nType==VTYPE_OFFICE))
		return FALSE;
	
	CSimpleArray<int>	vulId;
	CSimpleArray<int>	softId;
	
	if ( pItemData->nType == VTYPE_SOFTLEAK )
		softId.Add(arrVulIds);
	else
		vulId.Add(arrVulIds);

	m_nRepairTotal = 1;
	m_nRepairInstalled = 0;
	m_nRepairDownloaded = 0;
	m_nRepairProcessed = 0;
    m_nInstallProcessed = 0;
	m_nRepairCurrentRate = 0;

	m_RefWin.StartIconAnimate(30100);

	_SetDisplayState(SCANSTATE_REPAIRING);
	_UpdateRepairTitle();

	// Fill the list ctrl 
	ResetListCtrl(m_wndListCtrlVulFixing);
	AppendItem2RepairList( m_wndListCtrlVulFixing, new T_VulListItemData( *pItemData ) );
	
	// Fix 	
	return theEngine->RepairAll( m_RefWin.m_hWnd, vulId, softId);
}

void CBeikeVulfixHandler::OnBkBtnBeginRepair()
{
	int nChecked = 0; 
	CSimpleArray<int> arrSoft, arrVulDesc;
	CSimpleArray<T_VulListItemData*> arrSoftItem, arrVulItemDesc;
	for(int i = 0; i < m_wndListCtrlVul.GetItemCount(); ++i)
	{
		if(m_wndListCtrlVul.GetCheckState(i))
		{
			++ nChecked;
			T_VulListItemData *pItem = (T_VulListItemData*) m_wndListCtrlVul.GetItemData( i );	
			ATLASSERT(pItem);
			if(pItem)
			{
				if(pItem->nType == VTYPE_SOFTLEAK)
				{
					arrSoftItem.Add( pItem );
					arrSoft.Add( pItem->nID );
				}
				else
				{
					ATLASSERT( pItem->nType==VTYPE_WINDOWS || pItem->nType==VTYPE_OFFICE );
					arrVulItemDesc.Add( pItem );
					arrVulDesc.Add( pItem->nID );
				}
			}
		}
	}

	if( nChecked==0)
	{
		CBkSafeMsgBox::Show( BkString::Get(IDS_VULFIX_5035), NULL, MB_OK | MB_ICONWARNING, NULL );
	}
	else
	{
		BOOL hasOfficeVul = FALSE;
		for(int i=0; i<arrVulItemDesc.GetSize(); ++i)
		{
			T_VulListItemData *pItem = arrVulItemDesc[i];
			if(pItem->nType==VTYPE_OFFICE)
			{
				hasOfficeVul = TRUE;
				break;
			}
		}
		if(!theEngine->CheckRepairPrequisite(hasOfficeVul))
			return;


		m_nRepairTotal = arrVulItemDesc.GetSize() + arrSoft.GetSize();
		m_nRepairInstalled = 0;
		m_nRepairDownloaded = 0;
		m_nRepairProcessed = 0;
        m_nInstallProcessed = 0;
		m_nRepairCurrentRate=0;
        m_uCurrentDownSize = 0;
        m_uTempSize = 0;

		m_RefWin.StartIconAnimate(30100);

		SetItemCheck(IDC_CHK_VULFIX_REPAIR_TOP_DONERESTART, FALSE);	// 每次都重置完成后关机 
		_SetDisplayState(SCANSTATE_REPAIRING);
		_UpdateRepairTitle();

		// Fill the list ctrl 
		ResetListCtrl(m_wndListCtrlVulFixing);
		for(int i = 0; i < arrSoftItem.GetSize(); ++i)
		{
			T_VulListItemData *pItem = arrSoftItem[i];
			AppendItem2RepairList( m_wndListCtrlVulFixing, new T_VulListItemData( *pItem ) );
		}
		for(int i = 0; i < arrVulItemDesc.GetSize(); ++i)
		{
			T_VulListItemData *pItem = arrVulItemDesc[i];
			AppendItem2RepairList( m_wndListCtrlVulFixing, new T_VulListItemData( *pItem ) );
		}

		theEngine->RepairAll( m_RefWin.m_hWnd, arrVulDesc, arrSoft);
	}
}
// void CBeikeVulfixHandler::OnBkBtnBeginRepair()
// {
// 	int nChecked = 0;
// 	// 
// 	CSimpleArray<int> arrSoft, arrVulDesc, arrVulAsc;
// 	CSimpleArray<T_VulListItemData*> arrSoftItem, arrVulItemDesc, arrVulItemAsc;
// 	for(int i=0; i<m_wndListCtrlVul.GetItemCount(); ++i)
// 	{
// 		if(m_wndListCtrlVul.GetCheckState(i))
// 		{
// 			++ nChecked;
// 
// 			T_VulListItemData *pItem = (T_VulListItemData*) m_wndListCtrlVul.GetItemData( i );	
// 			ATLASSERT(pItem);
// 			if(pItem)
// 			{
// 				if(pItem->nType==VTYPE_SOFTLEAK)
// 				{
// 					arrSoftItem.Add( pItem );
// 					arrSoft.Add( pItem->nID );
// 				}
// 				else
// 				{
// 					ATLASSERT( pItem->nType==VTYPE_WINDOWS || pItem->nType==VTYPE_OFFICE );
// 					arrVulItemDesc.Add( pItem );
// 					arrVulDesc.Add( pItem->nID );
// 				}
// 			}
// 		}
// 	}
// 	
// 	if( nChecked==0)
// 	{
// 		CBkSafeMsgBox::Show( BkString::Get(IDS_VULFIX_5035), NULL, MB_OK | MB_ICONWARNING, NULL );
// 	}
// 	else
// 	{
// 		BOOL hasOfficeVul = FALSE;
// 		for(int i=0; i<arrVulItemDesc.GetSize(); ++i)
// 		{
// 			T_VulListItemData *pItem = arrVulItemDesc[i];
// 			if(pItem->nType==VTYPE_OFFICE)
// 			{
// 				hasOfficeVul = TRUE;
// 				break;
// 			}
// 		}
// 		if(!theEngine->CheckRepairPrequisite(hasOfficeVul))
// 			return;
// 		
// 		// 对系统补丁逆序 
// 		ATLASSERT(arrVulDesc.GetSize()==arrVulItemDesc.GetSize());
// 		for(int i=arrVulItemDesc.GetSize()-1; i>=0; --i)
// 		{
// 			arrVulAsc.Add( arrVulDesc[i] );
// 			arrVulItemAsc.Add( arrVulItemDesc[i] );
// 		}
// 		
// 		m_nRepairTotal = arrVulItemAsc.GetSize() + arrSoft.GetSize();
// 		m_nRepairInstalled = 0;
// 		m_nRepairDownloaded = 0;
// 		m_nRepairProcessed = 0;
// 		m_nRepairCurrentRate=0;
// 		
// 		m_RefWin.StartIconAnimate(30100);
// 		
// 		SetItemCheck(IDC_CHK_VULFIX_REPAIR_TOP_DONERESTART, FALSE);	// 每次都重置完成后关机 
// 		_SetDisplayState(SCANSTATE_REPAIRING);
// 		_UpdateRepairTitle();
// 		
// 		// Fill the list ctrl 
// 		ResetListCtrl(m_wndListCtrlVulFixing);
// 		for(int i=0; i<arrSoftItem.GetSize(); ++i)
// 		{
// 			T_VulListItemData *pItem = arrSoftItem[i];
// 			AppendItem2RepairList( m_wndListCtrlVulFixing, new T_VulListItemData( *pItem ) );
// 		}
// 		for(int i=0; i<arrVulItemAsc.GetSize(); ++i)
// 		{
// 			T_VulListItemData *pItem = arrVulItemAsc[i];
// 			AppendItem2RepairList( m_wndListCtrlVulFixing, new T_VulListItemData( *pItem ) );
// 		}
// 
// 		theEngine->RepairAll( m_RefWin.m_hWnd, arrVulAsc, arrSoft);
// 	}
// }

void CBeikeVulfixHandler::OnBkBtnCancelRepair()
{
	LPCTSTR szPrompt = BkString::Get(IDS_VULFIX_5036);
	if(IDYES==CBkSafeMsgBox::Show(szPrompt, NULL, MB_YESNO|MB_ICONQUESTION))
	{
		m_RefWin.StopIconAnimate();
		theEngine->CancelRepair();
		OnBkBtnScan();
	}
}

void CBeikeVulfixHandler::OnBkBtnRunBackground()
{
	SetWindowRunBackGround(m_hMainWnd, TRUE);
	
	LPCTSTR szMsg = BkString::Get(IDS_VULFIX_5037);
	_NotifyTrayIcon( szMsg, 10, FALSE );
}

void CBeikeVulfixHandler::OnBkBtnReboot()
{
	_ShutdownComputer(TRUE);
}

void CBeikeVulfixHandler::OnBkBtnEnableRelateCOM()
{
	_EnableRelateCOM(TRUE);
	OnBkBtnScan();
}

void CBeikeVulfixHandler::OnBkBtnDisableRelateCOM()
{
	_EnableRelateCOM(FALSE);
	OnBkBtnScan();
}

void CBeikeVulfixHandler::OnBkBtnViewInstalled()
{
	_ViewDetail( 0 );
}

void CBeikeVulfixHandler::OnBkBtnViewIgnored()
{
	_ViewDetail( 1 );
}

void CBeikeVulfixHandler::OnBkBtnViewSupersede()
{
	_ViewDetail( 2 );
}

void CBeikeVulfixHandler::OnBkBtnViewInvalid()
{
	_ViewDetail( 3 );
}

LRESULT CBeikeVulfixHandler::OnListBoxVulFixNotify( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
	if(pnmh->code==LVN_ITEMCHANGED)
	{
		LPNMLISTVIEW pnmv = (LPNMLISTVIEW) pnmh; 
		if( pnmv->uNewState & LVIS_SELECTED )
		{
			_DisplayRelateVulFixInfo( pnmv->iItem );
		}
	}
	bHandled = FALSE;
	return 0;
}

void CBeikeVulfixHandler::_DisplayRelateVulFixInfo( int nItem )
{
	if(m_nCurrentRelateInfoItem==nItem)
		return; 
	
	m_nCurrentRelateInfoItem = nItem;
	T_VulListItemData *pItemData = NULL;
	if(nItem>=0)
	{
		pItemData = (T_VulListItemData*)m_wndListCtrlVul.GetItemData( nItem );
	}
	
	if(pItemData)
	{
		SetRelateInfo(m_ctlRichEdit, pItemData, TRUE);
		SetItemVisible(3003, FALSE);
		SetItemVisible(3004, TRUE);

		SetItemVisible(IDC_DIV_VULFIX_RIGHT_SHOW_SEL_NONE,FALSE);
		SetItemVisible(IDC_DIV_VULFIX_RIGHT_SHOW_SELECT,TRUE);
	}
    else
	{
		m_nCurrentRelateInfoItem = -1;
		SetItemVisible(3003, TRUE);
		SetItemVisible(3004, FALSE);
		SetItemVisible(IDC_DIV_VULFIX_RIGHT_SHOW_SEL_NONE,TRUE);
		SetItemVisible(IDC_DIV_VULFIX_RIGHT_SHOW_SELECT,FALSE);
	}
}

LRESULT CBeikeVulfixHandler::OnVulFixEventHandle( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	int evt = uMsg - WMH_VULFIX_BASE;
	if(evt==EVulfix_ScanBegin||evt==EVulfix_ScanProgress)
	{
		if(evt==EVulfix_ScanBegin)
		{
			SetItemDWordAttribute(IDC_PROGRESS_VULFIX_SCANNING, "showpercent", 1);
			m_RefWin.StopProgressAnimate();
			m_bScanStarted = TRUE;

			if(wParam==VTYPE_OFFICE)
				m_nScanState = 0;
			else 
				++m_nScanState;
			m_nTotalItem = lParam>0 ? lParam : 1;
			m_nCurrentItem = 0;
		}
		else //EVulfix_ScanProgress:
		{
			if(lParam>(m_nCurrentItem+m_nTotalItem*0.1))
			{
				m_nCurrentItem = lParam;
				int nPos = m_nScanState*100 + (m_nCurrentItem*100)/m_nTotalItem;
				_SetScanProgress(nPos);
			}
		}
	}
	else if(evt==EVulfix_Task_Complete||evt==EVulfix_Task_Error)
	{
		;
	}
	else
	{
		if(evt!=EVulfix_DownloadProcess)
			DEBUG_TRACE(_T("Evt:%d W:%d L:%d\n"), evt, wParam, lParam);
		int nKbId = wParam;
		int nIndex = FindListItem( nKbId );
		if(nIndex==-1)
			return 0; 
		
		unsigned int nDownloadPercent = 0;
		int nSubitem = -1, nSubitem2=-1;
		CString strTitle, strTitle2;
		COLORREF clr = black, clr2=black;
		switch (evt)
		{
		case EVulfix_DownloadBegin:
		case EVulfix_DownloadProcess:
		case EVulfix_DownloadEnd:
		case EVulfix_DownloadError:
		
			// 更新下载文件日志
			if( EVulfix_DownloadEnd==evt )
			{
				BSTR bstr = NULL;
				if( theEngine->m_pRepairVul && SUCCEEDED(theEngine->m_pRepairVul->GetItemFilePath( nKbId, &bstr )))
				{
					theEngine->m_fixLog.UpdateDownload( nKbId, CString(bstr) );
					SysFreeString(bstr);
				}
			}

            if (EVulfix_DownloadProcess!=evt && EVulfix_DownloadBegin!=evt)
            {
                T_VulListItemData *pItemData = (T_VulListItemData*)m_wndListCtrlVulFixing.GetItemData( nIndex );
                ATLASSERT(pItemData);
                m_uCurrentDownSize += pItemData->nFileSize;
            }
			
			//nSubitem = 3;
			if( EVulfix_DownloadProcess==evt )
			{
				T_VulListItemData *pItemData = (T_VulListItemData*)m_wndListCtrlVulFixing.GetItemData( nIndex );
				ATLASSERT(pItemData);
				if(pItemData && pItemData->nFileSize>0)
				{
					CString strFileSize, strDownloadedSize;
					FormatSizeString(lParam, strDownloadedSize);
					FormatSizeString(pItemData->nFileSize, strFileSize);
                    m_uTempSize = lParam;
					strTitle.Format(_T("%s/%s"), strDownloadedSize, strFileSize);
					nSubitem2 = 2;
					nDownloadPercent = GetPercent(lParam, pItemData->nFileSize);
					strTitle2.Format(BkString::Get(IDS_VULFIX_5038), nDownloadPercent );
					m_nRepairCurrentRate=nDownloadPercent;

				}
			}
			else if(EVulfix_DownloadEnd==evt)
			{
				m_nRepairCurrentRate=0;
				strTitle = BkString::Get(IDS_VULFIX_5039);
				++m_nRepairDownloaded;

				nSubitem2 = 2;
				strTitle2 = BkString::Get(IDS_VULFIX_5040);
                ++ m_nRepairProcessed;
			}
			else if(EVulfix_DownloadError==evt)
			{
				m_nRepairCurrentRate=0;
				if(lParam)
				{
					clr = black;
					strTitle = BkString::Get(IDS_VULFIX_5041);
				}
				else
				{
					clr = red;
					strTitle = BkString::Get(IDS_VULFIX_5042);
				}
				++ m_nRepairProcessed;

				nSubitem2 = 2;
				clr2 = clr;
				strTitle2 = strTitle;
			}
			else
			{
				strTitle = BkString::Get(IDS_VULFIX_5043);
				m_nRepairCurrentRate=0;
			}
			break;

		case EVulfix_InstallBegin:
		case EVulfix_InstallBeginUser:
		case EVulfix_InstallEnd:
		case EVulfix_InstallError:
			if(EVulfix_InstallEnd==evt || EVulfix_InstallError==evt)
			{
				theEngine->m_fixLog.UpdateInstalled( nKbId, EVulfix_InstallEnd==evt );
			}
			nSubitem = 2;
			if(EVulfix_InstallBegin==evt)
			{
				clr = black;
				strTitle = BkString::Get(IDS_VULFIX_5044);
			}
			else if(EVulfix_InstallBeginUser==evt)
			{
				clr = brown;
				strTitle = BkString::Get(IDS_VULFIX_5045);
			}
			else if(EVulfix_InstallEnd==evt)
			{
				clr = black;
				strTitle = BkString::Get(IDS_VULFIX_5046);
				++m_nRepairInstalled;
			}
			else if(EVulfix_InstallError==evt)
			{
				clr = black;
				INT instcode = lParam;
				if( instcode==EINST_FAIL_NOT_APPLICABLE )
					strTitle = BkString::Get(IDS_VULFIX_5047);
				else if( instcode==EINST_FAIL_REQUIRE_REBOOT )
                    strTitle = BkString::Get(IDS_VULFIX_5048);
				else if( instcode==EINST_FAIL_FILEBUSY )
                    strTitle = BkString::Get(IDS_VULFIX_5049);
#ifdef _DEBUG
				else if( instcode==EINST_FAIL_HIJACKED )
                    strTitle = BkString::Get(IDS_VULFIX_5050);
#endif
				else 
				{	
					const int nMaxFail = 2;
					static CBeikeVulfixFileLog vulfixLog;
					INT nFailed = vulfixLog.IncHotfixError(nKbId);
					if( nFailed>=nMaxFail)
					{
						if (NULL != theEngine)
							theEngine->IgnoreVuls(nKbId, TRUE);
						vulfixLog.ClearHotfixError(nKbId);
						strTitle = BkString::Get(IDS_VULFIX_5166);//_T("已忽略");
					}
					else
						strTitle = BkString::Get(IDS_VULFIX_5052);//_T("安装失败");

 					m_wndListCtrlVulFixing.SetSubItem(nIndex, 5, BkString::Get(IDS_VULFIX_5051), SUBITEM_LINK, FALSE);
//                     strTitle = BkString::Get(IDS_VULFIX_5052);
				}
			}
			if(EVulfix_InstallError==evt || EVulfix_InstallEnd==evt)
				++ m_nInstallProcessed;
			break;
	
		case EVulfix_Task_Complete:
		case EVulfix_Task_Error:
		default:
			ATLASSERT(FALSE);
			return 0;
			break;
		}
		if(nSubitem>=0)
		{
			m_wndListCtrlVulFixing.SetSubItem(nIndex, nSubitem, strTitle, SUBITEM_TEXT, FALSE);
			m_wndListCtrlVulFixing.SetSubItemColor(nIndex, nSubitem, clr);
		}
		if(nSubitem2>=0)
		{
			m_wndListCtrlVulFixing.SetSubItem(nIndex, nSubitem2, strTitle2, SUBITEM_TEXT, FALSE);
			m_wndListCtrlVulFixing.SetSubItemColor(nIndex, nSubitem2, clr2);
		}
		if(evt==EVulfix_DownloadBegin || evt==EVulfix_DownloadError 
			|| evt==EVulfix_InstallEnd || evt==EVulfix_InstallError
			|| evt==EVulfix_DownloadProcess || EVulfix_DownloadEnd==evt)
			// 更新Title 
		{
			DWORD	nProgerss=0;
			
			if(m_nRepairTotal==0)
				nProgerss=100;
			else
			{
				float fProc= (float)m_nRepairCurrentRate*0.9f;
				fProc+=((m_nRepairDownloaded-m_nInstallProcessed)*90);
				fProc+=(m_nInstallProcessed*100);
				fProc/=(m_nRepairTotal);
				nProgerss=(DWORD)fProc;
			}

//			if(evt!=EVulfix_DownloadProcess)
			_UpdateRepairTitle();
			_SetRepairProgress( nProgerss );

			if( (evt==EVulfix_InstallEnd || evt==EVulfix_InstallError )
				&& IsWindowRunBackGround(m_hMainWnd) )
			{
				// 如果后台运行, 则每当安装成功或者失败, 提示用户
				CString str;
				if( m_nRepairProcessed==m_nRepairTotal )
				{
					if( m_nInstallProcessed==m_nRepairInstalled )
					{
						str.Format(BkString::Get(IDS_VULFIX_5053));
					}
					else
					{
						if( m_nRepairInstalled>0 )
							str.Format(BkString::Get(IDS_VULFIX_5054), m_nRepairTotal, m_nRepairInstalled);
						else
							str.Format(BkString::Get(IDS_VULFIX_5055), m_nRepairTotal);
					}
				}
				else
				{
					if( m_nInstallProcessed==m_nRepairInstalled  )
					{
						str.AppendFormat(BkString::Get(IDS_VULFIX_5056), m_nRepairTotal, m_nRepairInstalled);
					}
					else
					{
						str.AppendFormat(BkString::Get(IDS_VULFIX_5057), m_nRepairTotal, m_nRepairInstalled, m_nRepairProcessed-m_nRepairInstalled);
					}
				}
				_NotifyTrayIcon(str, 10);
			}
		}
	}
	return 0;
}

int CBeikeVulfixHandler::FindListItem( int nID )
{
	static int nItem = -1;

	// if cache meet 
	if(nItem>=0 && nItem<m_wndListCtrlVulFixing.GetItemCount())
	{
		T_VulListItemData *pItemData = (T_VulListItemData *)m_wndListCtrlVulFixing.GetItemData(nItem);
		if(pItemData && pItemData->nID==nID)
			return nItem;
	}

	// find all 
	for(int i=0; i<m_wndListCtrlVulFixing.GetItemCount(); ++i)
	{
		T_VulListItemData *pItemData = (T_VulListItemData *)m_wndListCtrlVulFixing.GetItemData(i);
		if(pItemData && pItemData->nID==nID)
			return nItem = i;
	}
	return nItem = -1;
}


void CBeikeVulfixHandler::_ShutdownComputer( BOOL bReboot )
{
	ShutDownComputer( bReboot );
}

void CBeikeVulfixHandler::_UpdateRepairTitle()
{
	CString strTitle;
    if (m_nRepairProcessed != m_nRepairTotal)
    {
        CString strTotalSize;
        CString strCurrentSize;
        CString strFormat;
        FormatSizeString(m_uTempSize + m_uCurrentDownSize, strCurrentSize);
        FormatSizeString(m_uDownTotalSize, strTotalSize);
        strFormat.Format(L"%s/%s", strCurrentSize, strTotalSize);
	    strTitle.Format(BkString::Get(IDS_VULFIX_5058), m_nRepairProcessed + 1, m_nRepairTotal , strFormat);
    }
    else
    {
        strTitle.Format(BkString::Get(IDS_VULFIX_5175), m_nInstallProcessed + 1, m_nRepairTotal );
    }
    SetItemText(30101, strTitle);
}

void CBeikeVulfixHandler::_SetScanProgress( int nPos )
{
	SetItemDWordAttribute(IDC_PROGRESS_VULFIX_SCANNING, "value", nPos);
}

void CBeikeVulfixHandler::_SetRepairProgress( int nPos )
{
	static int last_pos = -1;
	if(last_pos!=nPos)
	{
		last_pos = nPos;
		SetItemDWordAttribute(IDC_PROGRESS_VULFIX_REPAIR, "value", nPos);
	}
}

LRESULT CBeikeVulfixHandler::OnScanStart( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	m_bScanDone = FALSE;
	OnBkBtnScan();
	return 0;
}

void GetCurrentDateTime(CString &str)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	str.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

LRESULT CBeikeVulfixHandler::OnScanDone( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	m_bScanDone = TRUE;
	
	m_nNumMust = m_nNumOption = m_nNumSP = 0;
	_SetDisplayState(SCANSTATE_DISPLAY);
	if( FAILED(lParam) || wParam )
	{
		CString strErrMsg;
		if(wParam)
			strErrMsg = BkString::Get(IDS_VULFIX_5059);
		else
			strErrMsg = FormatErrorMessage( lParam );
		m_wndListCtrlVul.SetEmptyString( strErrMsg );
		_UpdateScanResultTitle(0, strErrMsg);
//		_UpdateViewDetailBtnsNumber(0, 0, 0, 0);
	}
	else if(wParam==0)
	{
		// Update Last scan time 
		CString strNow;
		CString strLastScanTime;
		BKSafeConfig::Get_Vulfix_LastScanTime( m_strLastScanTime );
		GetCurrentDateTime( strNow );
		if( m_strLastScanTime.IsEmpty() )
			m_strLastScanTime = strNow;
		BKSafeConfig::Set_Vulfix_LastScanTime( strNow );
		
		ATLASSERT(theEngine->m_pVulScan);
		ResetListCtrl(m_wndListCtrlVul);
		
		const CSimpleArray<LPTUpdateItem>& arr2 = theEngine->m_pVulScan->GetResults();
		const CSimpleArray<LPTVulSoft>& arr = theEngine->m_pVulScan->GetSoftVuls();
		
		int nSoft = CountItems(arr, std::ptr_fun(IsSoftVulInstallable));
		int nMustWindows = CountItems(arr2, std::ptr_fun(IsMustWindowsVul));
		m_nNumSP = CountItems(arr2, std::ptr_fun(IsSPVul));
		m_nNumOption = CountItems(arr2, std::ptr_fun(IsOptionVul));
		m_nNumMust = nSoft + nMustWindows;
		int	nAll = m_nNumMust + m_nNumOption + m_nNumSP;
		
		INT nTipIcon = 0;
		CString	strTips;
		if (nAll > 0)
		{
			if( m_nNumMust > 0)
			{
				nTipIcon = 1;
				strTips.Format(BkString::Get(IDS_VULFIX_5060),nAll, m_nNumMust);
			}
			else
			{
				nTipIcon = 3;
				//strTips.Format(BkString::Get(IDS_VULFIX_5061), m_nNumOption+m_nNumSP);
                strTips.Format(BkString::Get(IDS_VULFIX_5061));
			}
		}
		else
		{
			nTipIcon = 2;
			strTips.Format(BkString::Get(IDS_VULFIX_5062));
		}
		_UpdateScanResultTitle(nTipIcon, strTips);
		
// 		if(m_nNumMust==0)
// 		{
// 			E_TitleType emTitle;
// 			emTitle = TITLE_MUST;
// 			int	i=m_wndListCtrlVul.AppendTitle(_T(""), RGB(0,0,0), 0, emTitle);
// 			CListViewCtrlEx::TListItem*	pItem=m_wndListCtrlVul._GetItemData(i);
// 			if (pItem)
// 			{
// 				pItem->clrBg=BACKGROUND_COLOR;
// 				pItem->clrBtmGapLine=BACKGROUND_COLOR;
// 			}			
// 			i=m_wndListCtrlVul.AppendTitle(BkString::Get(IDS_VULFIX_5063), RGB(0,115,0), 0, emTitle);
// 			pItem=m_wndListCtrlVul._GetItemData(i);
// 			if (pItem)
// 			{
// 				pItem->nLeftmargin=120;
// 				pItem->clrBg=BACKGROUND_COLOR;
// 				pItem->clrBtmGapLine=BACKGROUND_COLOR;
// 				pItem->bBold=TRUE;
// 				pItem->nHeightAdd=3;
// 			}
// 
// 			CString strNote;
// 			strNote.Format( BkString::Get(IDS_VULFIX_5064), m_strLastScanTime );
// 			i=m_wndListCtrlVul.AppendTitle(strNote, RGB(0,0,0));
// 			pItem=m_wndListCtrlVul._GetItemData(i);
// 			if (pItem)
// 			{
// 				pItem->nLeftmargin=120;
// 				pItem->clrBg=BACKGROUND_COLOR;
// 				pItem->nTopMargin=1;
// 			}
// 		}
		
		CString strTitle;
		if(m_nNumMust)
		{
			E_TitleType emTitle;
			emTitle = TITLE_MUST;
			int nLeft = 125;
			if ( m_nNumMust < 10)
				nLeft = 125;
			else if ( m_nNumMust < 100 )
				nLeft = 135;
			else 
				nLeft = 140;

			strTitle.Format(BkString::Get(IDS_VULFIX_5065), m_nNumMust);
			COLORREF clr = RGB(255,0,0);
			
			INT i = m_wndListCtrlVul.AppendTitle( strTitle, clr, LISTITEM_BOLD, emTitle );
			m_wndListCtrlVul.AppendTitleItem(i, _T("")/*BkString::Get(IDS_VULFIX_5066)*/, CRect(nLeft,0,-20,28),SUBITEM_TEXT, clr, NULL );//第一个栏目  高危补丁
			AppendVuls(m_wndListCtrlVul, arr, std::ptr_fun(IsSoftVulInstallable) );
			AppendVuls(m_wndListCtrlVul, arr2, std::ptr_fun(IsMustWindowsVul) );
		}
		if(m_nNumOption>0)
		{
			E_TitleType emTitle;
			emTitle = TITLE_OPTION;
			int nLeft = 105;
			if ( m_nNumOption < 10)
				nLeft = 105;
			else if ( m_nNumOption < 100 )
				nLeft = 115;
			else 
				nLeft = 120;


			COLORREF clr = RGB(0,128,0);
			strTitle.Format(BkString::Get(IDS_VULFIX_5067), m_nNumOption);
			int i = m_wndListCtrlVul.AppendTitle( strTitle, clr, LISTITEM_BOLD, emTitle );
			m_wndListCtrlVul.AppendTitleItem(i, _T("")/*BkString::Get(IDS_VULFIX_5068)*/, CRect(nLeft,0,-20,28),SUBITEM_TEXT, clr, NULL );
			AppendVuls(m_wndListCtrlVul, arr2, std::ptr_fun(IsOptionVul) );
			
// 			if(m_nNumMust>0)
// 				m_wndListCtrlVul.ExpandGroup( i, FALSE);
			m_wndListCtrlVul.ExpandGroup( i, FALSE);
		}
		if(m_nNumSP>0)
		{
			E_TitleType emTitle;
			emTitle = TITLE_SP;
			COLORREF clr = RGB(0,128,0);
			strTitle.Format(BkString::Get(IDS_VULFIX_5069), m_nNumSP);
			int i = m_wndListCtrlVul.AppendTitle( strTitle, clr, LISTITEM_BOLD, emTitle );
			m_wndListCtrlVul.AppendTitleItem(i, _T("")/*BkString::Get(IDS_VULFIX_5070)*/, CRect(105,0,-20,28),SUBITEM_TEXT, clr, NULL );
			AppendVuls(m_wndListCtrlVul, arr2, std::ptr_fun(IsSPVul) );
// 			if(m_nNumMust>0)
// 				m_wndListCtrlVul.ExpandGroup( i, FALSE);
			m_wndListCtrlVul.ExpandGroup( i, FALSE);				
		}
//		_DisplayRelateVulFixInfo(-1);
		
		{
			INT nFixed=0, nIgnored=0, nReplaced=0, nInvalid=0;
			if( theEngine->m_pVulScan )
			{
				nFixed += CountItems(theEngine->m_pVulScan->GetSoftVuls(), SoftComRepaired);
				nFixed += theEngine->m_pVulScan->GetFixedVuls().GetSize();
				nIgnored = theEngine->m_pVulScan->GetIgnoredVuls().GetSize();
				nReplaced = theEngine->m_pVulScan->GetReplacedVuls().GetSize();
				nInvalid = theEngine->m_pVulScan->GetInvalidVuls().GetSize();
			}
//			_UpdateViewDetailBtnsNumber(nFixed, nIgnored, nReplaced, nInvalid);
		}
		
		// Notify the dashboard 
		if(m_nNumMust==0 && m_MainDlg)
			m_MainDlg->RemoveFromTodoList(BkSafeExamItem::SystemLeakScan);
        
        if (m_wndListCtrlVul.GetItemCount() > 0)
            m_wndListCtrlVul.SelectItem(1);

        if (nAll)
        {
            CString str;
            str.Format(BkString::Get(IDS_VULFIX_5177), nAll);
            SetItemText(IDC_TXT_VULFIX_SELECT_NUM, str);
            SetItemText(IDC_TXT_VULFIX_SELCET_SIZE, _T(""));
        }
        else
        {
            SetItemText(IDC_TXT_VULFIX_SELECT_NUM, _T(""));
            SetItemText(IDC_TXT_VULFIX_SELCET_SIZE, _T(""));
        }

        m_nTotalVulNum = m_nNumMust + m_nNumOption;
        PostMessage(MSG_USER_VUL_SELECT_CHANGE,NULL, NULL);
    }
	_EnableSelectedOpBtn( m_nNumMust, m_nNumSP, m_nNumOption );
	m_RefWin.StopIconAnimate();
	
	return 0;
}

LRESULT CBeikeVulfixHandler::OnRepaireDone( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	BOOL bFailed = FAILED(lParam);
	BOOL bcancel		= (BOOL)wParam;
	BOOL bCheckShutdown = GetItemCheck(IDC_CHK_VULFIX_REPAIR_TOP_DONERESTART);
	BOOL bRunBackground = IsWindowRunBackGround(m_hMainWnd);
	
	//先停止图标刷新
	m_RefWin.StopIconAnimate();	
	if(bcancel)
		return 0;

	DWORD dwFlagsIcon = 0;
	CRect rcXml;
	CString strMsg, strMsgXml;
	strMsg = BkString::Get(IDS_VULFIX_5077);
	if( bFailed )
	{
		dwFlagsIcon = MB_ICONERROR;
		strMsg = FormatErrorMessage( lParam );
		_SetDisplayState(SCANSTATE_REPAIRING, REPAIRSTATE_FAIL, strMsg);
	}
	else
	{
		if(m_nRepairInstalled==m_nRepairTotal)
		{
			dwFlagsIcon = MB_ICONINFORMATION;
			strMsg = BkString::Get(IDS_VULFIX_5078);
			_SetDisplayState(SCANSTATE_REPAIRING, REPAIRSTATE_DONE);
		}
		else if(m_nRepairInstalled>0)
		{
			dwFlagsIcon = MB_ICONWARNING;
			strMsg.Format(BkString::Get(IDS_VULFIX_5079), m_nRepairInstalled, m_nRepairTotal-m_nRepairInstalled);
			strMsgXml.Format(BkString::Get(IDS_VULFIX_5080), m_nRepairInstalled, m_nRepairTotal-m_nRepairInstalled);
			rcXml= CRect(0,0,300,50);
			_SetDisplayState(SCANSTATE_REPAIRING, REPAIRSTATE_DONE_PART);
		}
		else 
		{
			if(m_nRepairDownloaded>0)
			{
				dwFlagsIcon = MB_ICONERROR;
				strMsg.Format(BkString::Get(IDS_VULFIX_5081), m_nRepairTotal);
                strMsgXml.Format(BkString::Get(IDS_VULFIX_5082), m_nRepairTotal);
                rcXml=CRect(0,0,180,50);
				_SetDisplayState(SCANSTATE_REPAIRING, REPAIRSTATE_FAIL, strMsg);
			}
			else
			{
				dwFlagsIcon = MB_ICONERROR;
				strMsg = BkString::Get(IDS_VULFIX_5083);
				strMsgXml.Format(BkString::Get(IDS_VULFIX_5084), m_nRepairTotal);
				rcXml=CRect(0,0,180,50);
				_SetDisplayState(SCANSTATE_REPAIRING, REPAIRSTATE_FAIL, strMsg);
			}
		}
	}

	if( TRUE || bRunBackground )
	{
		_NotifyTrayIcon( strMsg, 20 );
		// 显示相应的按钮: 立即重启, 重新扫描 
		SetItemVisible( IDC_DIV_VULFIX_REPAIR_BTM_FAILED, TRUE);
		if(m_nRepairDownloaded>0)
		{
			SetItemVisible( 1050, TRUE);
			SetItemVisible( 1051, FALSE);
		}
		else
		{
			SetItemVisible( 1050, FALSE);
			SetItemVisible( 1051, TRUE);
		}
	}
	else
	{
		CBkSafeMsgBox	dlg;
		UINT_PTR		nRet;
		if( dwFlagsIcon==MB_ICONERROR )
		{
			dlg.AddButton( BkString::Get(IDS_MSGBOX_OK), IDCANCEL);
		}
		else
		{
			dlg.AddButton( BkString::Get(IDS_VULFIX_5085), IDOK);
			dlg.AddButton( BkString::Get(IDS_VULFIX_5086), IDCANCEL);
		}
		strMsg.Append( BkString::Get(IDS_VULFIX_5087) );
		if( strMsgXml.IsEmpty() )
			nRet = dlg.ShowMsg(strMsg, NULL, MB_BK_CUSTOM_BUTTON|dwFlagsIcon, NULL);
		else
			nRet = dlg.ShowPanelMsg(strMsgXml, &rcXml, NULL, MB_BK_CUSTOM_BUTTON|dwFlagsIcon, NULL);

		if(nRet==IDOK)
		{
			_ShutdownComputer(TRUE);
		}
		else
		{
			OnBkBtnScan();
		}
	}

	if (bCheckShutdown)
	{
		// not canceled , check if need shutdown  
		_PromptAutoShutdown();
	}

	return 0;
}

void CBeikeVulfixHandler::_EnableRelateCOM( BOOL bEnable )
{
	if(m_nCurrentRelateInfoItem>=0)
	{
		T_VulListItemData *pItemData = NULL;
		pItemData = (T_VulListItemData*)m_wndListCtrlVul.GetItemData( m_nCurrentRelateInfoItem );
		theEngine->m_pVulScan->EnableVulCOM( pItemData->nID, bEnable );
	}
}

LRESULT CBeikeVulfixHandler::OnRichEditLink( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
	ENLINK *pLink = (ENLINK*)pnmh;
	if(pLink->msg==WM_LBUTTONUP)
	{
		// 点击了了解更多
		T_VulListItemData *pItemData = NULL;
		if(m_nCurrentRelateInfoItem>=0)
			pItemData = (T_VulListItemData*)m_wndListCtrlVul.GetItemData( m_nCurrentRelateInfoItem );
		
		if(pItemData)
		{
			TCHAR	szBuffer[MAX_PATH];
			CString str;
			m_ctlRichEdit.GetTextRange(pLink->chrg.cpMin,pLink->chrg.cpMax, szBuffer);
			str=szBuffer;
			if(str==BkString::Get(IDS_VULFIX_5088))
			{
				if(!pItemData->strWebPage.IsEmpty())
					ShellExecute(NULL, _T("open"), pItemData->strWebPage, NULL, NULL, SW_SHOW);
			}
			else if(str==BkString::Get(IDS_VULFIX_5089))
			{
				if(!pItemData->strDownloadUrl.IsEmpty())
					ShellExecute(NULL, _T("open"), pItemData->strDownloadUrl, NULL, NULL, SW_SHOW);
			}			
		}
	}
	return 0;
}

LRESULT CBeikeVulfixHandler::OnListLinkClicked( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	int iItem		= (int)wParam;
	int iSubItem	= (int)lParam;
	
	if( theEngine && m_wndListCtrlVulFixing.IsWindowVisible() )
	{
		// 修复过程中点击 "反馈问题" 
		T_VulListItemData *pItem = (T_VulListItemData *)m_wndListCtrlVulFixing.GetItemData(iItem);
		if(pItem)
		{
			CString strName;
			LPCTSTR pslash = _tcsrchr(pItem->strDownloadUrl, _T('/'));
			if(pslash && _tcslen(pslash)>0)
			{
				strName = ++pslash;
			}
			CString strUrl;
			strUrl.Format(_T("http://www.ijinshan.com/safe/leak_help.html?fr=client&kb=%s"), strName);
			::ShellExecute(NULL,_T("open"), strUrl, NULL, NULL, SW_SHOW);
		}
	}
	else
	{
		CListViewCtrlEx::TListItem*	pListItem = m_wndListCtrlVul._GetItemData(iItem);

		if ( pListItem )
		{
			if ( pListItem->dwFlags&LISTITEM_TITLE )
			{
				const CListViewCtrlEx::TListSubItem*	pSubItem = m_wndListCtrlVul._GetSubItemData(iItem,iSubItem);
				if (pSubItem && !pSubItem->strUrl.IsEmpty() )
				{
					::ShellExecute(NULL,_T("open"),pSubItem->strUrl,NULL,NULL,SW_SHOW);
				}
			}
			else
			{
				T_VulListItemData *pItem = (T_VulListItemData *)m_wndListCtrlVul.GetItemData(iItem);
				if ( pItem )
				{
                    const CListViewCtrlEx::TListSubItem* pSubItem = m_wndListCtrlVul._GetSubItemData(iItem,iSubItem);
					if ( pItem->nType == VTYPE_SOFTLEAK )
					{
						int state = GetSoftItemState( pItem );
						if(state==VUL_DISABLE_COM)
						{
							theEngine->m_pVulScan->EnableVulCOM( pItem->nID, FALSE );
							OnBkBtnScan();
						}
						else if (state==VUL_UPDATE)
						{
							_RepairSingle(pItem->nID,pItem);
						}
					}
                    else if (pSubItem->bCheckDetail)
                    {
                        //MessageBox(NULL, L"123", NULL, NULL);
                        //::PostMessage(m_wndListCtrlVul.m_hWnd, )
                        CPoint pt;
                        GetCursorPos(&pt);
                        if (m_pTipDetailDlg)
                        {
                            m_pTipDetailDlg->DestroyWindow();
                            m_pTipDetailDlg = NULL;
                        }
                        m_pTipDetailDlg = new CDlgTipDetail();
                        m_pTipDetailDlg->Create(NULL, CWindow::rcDefault, _T(""),
                            WS_POPUP);
                        m_pTipDetailDlg->Initilize(m_hMainWnd, pItem, pt);
                        
                        m_pTipDetailDlg->ShowWindow(SW_SHOW);
                    }
					else
					{
						// 可选补丁安装
						_RepairSingle(pItem->nID,pItem);
					}
				}
			}
		}
	}
	return S_OK;
}

LRESULT CBeikeVulfixHandler::OnStnClickedNavi( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
{
	CPoint pt;
	GetCursorPos( &pt );
	m_ctlNaviLink.ScreenToClient( &pt );
	INT id = 0;
	if( m_ctlNaviLink.GetCurrentLinkID( pt, id) )
	{
		if( id>=1 && id<=4)
		{
			m_ctlNaviLink.ReleaseMouse();
			_ViewDetail( id-1 );
		}
	}
	return 0;
}

void CBeikeVulfixHandler::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
}

void CBeikeVulfixHandler::OnBkBtnErrorRepair()
{
	// 安装此补丁有问题，我要反馈
	T_VulListItemData *pItemData = NULL;
	if(m_nCurrentRelateInfoItem>=0)
		pItemData = (T_VulListItemData*)m_wndListCtrlVul.GetItemData( m_nCurrentRelateInfoItem );
	
	if(pItemData)
	{
		::ShellExecute(NULL,_T("open"), _T("http://www.ijinshan.com/safe/leak_help.html?fr=client"), NULL, NULL, SW_SHOW);
	}
}

void CBeikeVulfixHandler::OnBkBtnVisitVulPurposePage()
{
	// 详细 链接 
	::ShellExecute(NULL,_T("open"), _T("http://www.ijinshan.com/safe/leakfix_intro.html?fr=client"), NULL, NULL, SW_SHOW);
}


void CBeikeVulfixHandler::OnBkBtnExport()
{
	// TODO : 导出所有的?? 无法分出所选
	TCHAR szFilters[]=_T("Html files(*.html)\0*.html\0All files(*.*)\0*.*\0\0");
	CFileDialog dlg(FALSE,_T("html"),_T("ksafevul.html"), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,szFilters);
	if(dlg.DoModal()!=IDOK)
		return;
	
	CString strFileName=dlg.m_szFileName;
	T_ExportInfoArray arrExportInfo;
	for(int i=0; i<m_wndListCtrlVul.GetItemCount(); ++i)
	{
		//if(m_wndListCtrlVul.GetCheckState(i))
		{
			T_VulListItemData *pItem = (T_VulListItemData*) m_wndListCtrlVul.GetItemData( i );
			if(pItem)
			{
				T_ExportInfo ex;
				ex.nType = pItem->nType;
				ex.nWarnLevel = pItem->nWarnLevel;
				ex.dwFlags = pItem->dwFlags;
				if(pItem->nType==VTYPE_SOFTLEAK)
				{
					ex.strName = pItem->strName;
					ex.strDesc = pItem->strDesc;
				}
				else
				{
					ATLASSERT( pItem->nType==VTYPE_WINDOWS || pItem->nType==VTYPE_OFFICE );
					FormatKBString(pItem->nID, ex.strName);
					ex.strDesc = pItem->strName;
				}
				ex.strDate = pItem->strPubDate;
				ex.strPageUrl = pItem->strWebPage;
				ex.strDownloadUrl = pItem->strDownloadUrl;
				arrExportInfo.push_back( ex );
			}
		}
	}

	if( !_ExportResult(strFileName, arrExportInfo) )
	{
		CBkSafeMsgBox::Show(BkString::Get(IDS_VULFIX_5090), NULL, MB_OK|MB_ICONERROR);
	}
}

void CBeikeVulfixHandler::OnListReSize( CRect rcWnd )
{
	int iWidth = rcWnd.Width()-100-100-100-20;
// 	m_wndListCtrlVul.SetColumnWidth(0, 60);
// 	m_wndListCtrlVul.SetColumnWidth(1, 80);
// 	m_wndListCtrlVul.SetColumnWidth(2, iWidth);
// 	m_wndListCtrlVul.SetColumnWidth(3, 70);
// 	m_wndListCtrlVul.SetColumnWidth(4, 60);
	m_wndListCtrlVul.SetColumnWidth(0, iWidth);
	m_wndListCtrlVul.SetColumnWidth(1, 100);
	m_wndListCtrlVul.SetColumnWidth(2, 100);
	m_wndListCtrlVul.SetColumnWidth(3, 100);
// 	m_wndListCtrlVul.SetColumnWidth(4, 60);
}

void CBeikeVulfixHandler::OnDownListReSize( CRect rcWnd )
{
	int iWidth = rcWnd.Width()-130-150-20;
	m_wndListCtrlVulFixing.SetColumnWidth(0, iWidth);
	m_wndListCtrlVulFixing.SetColumnWidth(1, 130);
	m_wndListCtrlVulFixing.SetColumnWidth(2, 150);
// 	m_wndListCtrlVulFixing.SetColumnWidth(3, 120);
// 	m_wndListCtrlVulFixing.SetColumnWidth(4, 100);
// 	m_wndListCtrlVulFixing.SetColumnWidth(5, 60);
}

void CBeikeVulfixHandler::_EnableSelectedOpBtn( INT nMust, INT nSP, INT nOption )
{
	// Enable Window 
	BOOL bEnableBtn = (nMust+nOption+nSP)>0;
	EnableItem(IDC_LBL_VULFIX_RESULT_FIX, bEnableBtn);
	EnableItem(IDC_LBL_VULFIX_RESULT_IGNORE, bEnableBtn);	
	EnableItem(2001, bEnableBtn);
	
	BOOL bEnableCheck = nMust>0 || nOption>0;
	EnableItem(IDC_LBL_VULFIX_RESULT_CHECK_ALL, bEnableCheck);
	EnableItem(IDC_LBL_VULFIX_RESULT_UNCHECK_ALL, bEnableCheck);
	EnableItem(IDC_LBL_VULFIX_RESULT_CHECK_ALL_SLASH, bEnableCheck);
}

void CBeikeVulfixHandler::_NotifyTrayIcon( LPCTSTR szText, INT nTimeOut, BOOL bDelayShow )
{
	CSafeMonitorTrayShell shell;
	shell.SetTrayTipInfo( szText!=NULL ? szText:_T(""), nTimeOut, bDelayShow );
}

void CBeikeVulfixHandler::_ViewDetail( INT nSelTab )
{
	CDlgViewDetail dlg;
	dlg.Load(IDR_BK_VULDLG_DETAIL);
	dlg.SetInitTab( nSelTab );
	if( IDOK==dlg.DoModal(m_hMainWnd) )
		OnBkBtnScan();
}

void CBeikeVulfixHandler::_UpdateViewDetailBtnsNumber( INT nFixed, INT nIgnored, INT nReplaced, INT nInvalid )
{
	CString strNavi;
	strNavi.Format( BkString::Get(IDS_VULFIX_5015), nFixed, nIgnored, nReplaced, nInvalid );
//	m_ctlNaviLink.SetMText( strNavi );
}

void CBeikeVulfixHandler::_PromptAutoShutdown()
{
	CDlgShutdownPrompt dlg;
	dlg.Load(IDR_BK_SHUTDOWNPROMPT);
	if( IDOK==dlg.DoModal(m_hMainWnd) )
		_ShutdownComputer(FALSE);
}

void CBeikeVulfixHandler::_UpdateScanResultTitle( INT nTipIcon, LPCTSTR szTips )
{
	SetItemIntAttribute(1042, "sub", nTipIcon);	
	SetRichText(1041, szTips);
}

BOOL CBeikeVulfixHandler::_ExportResult( LPCTSTR szFileName, T_ExportInfoArray &arrExportInfo )
{
	LPCTSTR szHeader    = BkString::Get(IDS_VULFIX_5091);
	LPCTSTR szBodyBegin = BkString::Get(IDS_VULFIX_5092);
	LPCTSTR szHtmlEnd   = BkString::Get(IDS_VULFIX_5093);
	LPCTSTR szTblBegin  = BkString::Get(IDS_VULFIX_5094);
	LPCTSTR szTblEnd    = BkString::Get(IDS_VULFIX_5095);
	LPCTSTR szTblHeader = BkString::Get(IDS_VULFIX_5096);
	LPCTSTR szTblLine   = BkString::Get(IDS_VULFIX_5097);
	
	CAtlFile file;	
	if( SUCCEEDED( file.Create(szFileName, GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS) ) )
	{
		CString strDate;

		SYSTEMTIME st;
		GetLocalTime(&st);
		strDate.Format(_T("%04d-%02d-%02d"), st.wYear, st.wMonth, st.wDay);

		CString strTmp;
		strTmp.Format(szHeader, strDate);
		strTmp.Append(szBodyBegin);
		strTmp.Append(szTblBegin);
		strTmp.Append(szTblHeader);

		for(int i=0; i<arrExportInfo.size(); ++i)
		{
			T_ExportInfo& ex		= arrExportInfo[i];
			LPCTSTR szLevel_Must	= BkString::Get(IDS_VULFIX_5098);
			LPCTSTR szLevel_Opt		= BkString::Get(IDS_VULFIX_5099);
			LPCTSTR szLevel_SP		= BkString::Get(IDS_VULFIX_5100);
			LPCTSTR szLevel = (ex.nType==VTYPE_SOFTLEAK || ex.nWarnLevel>0) ? szLevel_Must : (ex.dwFlags>0 ? szLevel_SP : szLevel_Opt);

			strTmp.AppendFormat(szTblLine, szLevel, ex.strName, ex.strDesc, ex.strDate, ex.strDownloadUrl,ex.strDownloadUrl);
		}
		strTmp.Append(szTblEnd);
		strTmp.Append(szHtmlEnd);

		USES_CONVERSION;
		DWORD dwWritten = 0;
		CStringA p = CT2CA(strTmp, CP_UTF8);
		file.Write(p, p.GetLength());
		file.Close();
		return TRUE;
	}
	return FALSE;
}
/*
int CBeikeVulfixHandler::_LoadLocalLib(BKSafeVulFixLib::LocalLibInfo& libLocal)
{
	int nRet = -1;
	CString strLibFilePath;
	CAppPath::Instance().GetLeidianCfgPath(strLibFilePath, TRUE);
	strLibFilePath += LOCALLIB_FILE_NAME;

	CAtlFile atlFile;
	if (S_OK == atlFile.Create(strLibFilePath, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING))
	{
		ULONGLONG uSize = 0;
		DWORD dwLen = 0;
		if (S_OK != atlFile.GetSize(uSize))
			return nRet;
		dwLen = reinterpret_cast<DWORD&>(uSize);
		if (dwLen > 0)
		{
			PBYTE pValue = new BYTE[dwLen+4];
			ZeroMemory(pValue, dwLen+4);
			DWORD nByteRet = 0;
			if (S_OK == atlFile.Read((void*)pValue, dwLen, nByteRet))
			{
				if (NULL != pValue)
				{
					CString strValue((LPTSTR)pValue);
					BDX_FromJson(strValue, libLocal, FALSE);
				}
			}
			if (NULL != pValue)
			{
				delete []pValue;
				pValue = NULL;
			}
		}
	}
	DeleteFile(strLibFilePath);
	return nRet;
}

void CBeikeVulfixHandler::_FixVulFromLib(BKSafeVulFixLib::LocalLibInfo& leakLib)
{
	CSimpleArray<T_VulListItemData*> array_ListItem;
	
	if(m_firstInited)
	{
		// Clean downloaded files 
		BOOL bSave = BKSafeConfig::Get_Vulfix_SaveDownFile();
		if( !bSave )
			theEngine->m_fixLog.CleanFiles(FALSE, NULL);
		m_firstInited = FALSE;
		SetItemVisible(1015, !theEngine->IsSystemSupported());
	}

	for(int i = 0;i < leakLib.array_Libinfo.GetCount();i++)
	{
		T_VulListItemData *pItem = new T_VulListItemData;
		pItem->nID = leakLib.array_Libinfo[i].dwLeakId;
		pItem->nType = leakLib.array_Libinfo[i].dwType;
		pItem->nWarnLevel = leakLib.array_Libinfo[i].Level;
		pItem->strDesc = leakLib.array_Libinfo[i].LeakDescription;
		pItem->strName = leakLib.array_Libinfo[i].strUniqName;
		array_ListItem.Add(pItem);
	}
	
	m_RefWin.StartIconAnimate(30100);
	m_nRepairTotal = leakLib.array_Libinfo.GetCount();
	m_nRepairInstalled = 0;
	m_nRepairDownloaded = 0;
	m_nRepairProcessed = 0;
	m_nRepairCurrentRate = 0;

	SetItemCheck(IDC_CHK_VULFIX_REPAIR_TOP_DONERESTART, FALSE);	// 每次都重置完成后关机 
	_SetDisplayState(SCANSTATE_REPAIRING);
	_UpdateRepairTitle();

	// Fill the list ctrl 
	ResetListCtrl(m_wndListCtrlVulFixing);

	CSimpleArray<int> arrSoft,arrVulDesc;

	for(int i=0; i<array_ListItem.GetSize(); ++i)
	{
		T_VulListItemData *pItem = array_ListItem[i];
		AppendItem2RepairList( m_wndListCtrlVulFixing, new T_VulListItemData( *pItem ) );
		if(pItem->nType==VTYPE_SOFTLEAK)
		{
			arrSoft.Add( pItem->nID );
		}
		else
		{
			ATLASSERT( pItem->nType==VTYPE_WINDOWS || pItem->nType==VTYPE_OFFICE );
			arrVulDesc.Add( pItem->nID );
		}
	}
	
	theEngine->RepairAll( m_RefWin.m_hWnd,arrSoft,arrVulDesc);
}
*/
DWORD pfnScanLeakFromTray(void* pParam)
{
	DWORD dwRet = 0;
	CBeikeVulfixHandler* _pThis = static_cast<CBeikeVulfixHandler*>(pParam);
	if (NULL == _pThis)
		return dwRet;

	//等待扫描完成
	while (TRUE)
	{
		Sleep(100);
		if (TRUE == _pThis->IsScanOK())
		{
			break;
		}
	}

	//FIX
	if (NULL != _pThis->GetMainDlgPtr())
		_pThis->GetMainDlgPtr()->SendMessage(MSG_APP_TRAY_CALL_FIXLEAK, 0, 0);

	return dwRet;
}

CBkNavigator* CBeikeVulfixHandler::OnNavigate( CString &strChildName )
{
	if (0 == strChildName.CompareNoCase(BKSFNS_LEAKSCAN_FIX_FROMLIB))
	{
		//走到这里的时候已经开始了漏洞扫描，单独开启线程等待扫描完成
		HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pfnScanLeakFromTray, this, 0, 0);
		if (NULL != hThread)
			CloseHandle(hThread);
		hThread = NULL;
	}
	return NULL;
}

LRESULT CBeikeVulfixHandler::OnTrayCallScanLeak(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HRESULT hr = S_OK;
	
	if (theEngine->m_isRepairing == FALSE)
		OnBkBtnBeginRepair();	

	return hr;
}

VOID CBeikeVulfixHandler::CloseSuccess( BOOL bSucClose )
{
	if (!bSucClose)
	{
		if (m_MainDlg)
		{
			m_MainDlg->SetTabCurSel(IDC_TAB_MAIN, 3);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
LRESULT CBeikeVulfixHandler::OnVulTipDestory(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (m_pTipDetailDlg != NULL)
    {
        m_pTipDetailDlg->DestroyWindow();
        m_pTipDetailDlg = NULL;
    }
    return TRUE;
}
LRESULT CBeikeVulfixHandler::OnVulTipIgnore(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int nID = (int)wParam;
    theEngine->IgnoreVuls(nID, true);
    OnBkBtnScan();
    return TRUE;
}
LRESULT CBeikeVulfixHandler::OnSelectedChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int nCount;
    CString strSize;
    CString strFSize;
    CString strCount;
    int total = m_wndListCtrlVul.GetItemCount(); 

    nCount = 0;
    m_uDownTotalSize = 0;
    m_uCurrentDownSize = 0;

    for (int i = 0; i < total; ++i)
    {
        if (m_wndListCtrlVul.GetCheckState(i) &&
            !(m_wndListCtrlVul._GetItemData(i)->dwFlags & (LISTITEM_TITLE|LISTITEM_RADIOBOX)))
        {
            T_VulListItemData* pItem = (T_VulListItemData*)m_wndListCtrlVul.GetItemData(i);
            m_uDownTotalSize += pItem->nFileSize;
            nCount ++;
        }
//         else if (m_wndListCtrlVul._GetItemData(i)->dwFlags & (LISTITEM_TITLE|LISTITEM_RADIOBOX))
//         {
//             total--;
//         }
    }
    if (nCount == m_nTotalVulNum && m_nTotalVulNum != 0)
        SetItemCheck(IDC_LBL_VULFIX_RESULT_CHECK_ALL, TRUE);
    else
        SetItemCheck(IDC_LBL_VULFIX_RESULT_CHECK_ALL, FALSE);

    strCount.Format(BkString::Get(IDS_VULFIX_5173), nCount);
    FormatSizeString(m_uDownTotalSize, strFSize);
    strSize.Format(BkString::Get(IDS_VULFIX_5174), strFSize);

    SetItemText(IDC_TXT_VULFIX_SELECT_NUM, strCount);
    SetItemText(IDC_TXT_VULFIX_SELCET_SIZE, strSize);

    return TRUE;
}

LRESULT CBeikeVulfixHandler::OnBkBtnExportMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    OnBkBtnExport();
    return TRUE;
}
//////////////////////////////////////////////////////////////////////////
