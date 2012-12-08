#include "StdAfx.h"
#include "ViewSoftVulHandler.h"
#include <process.h>
#include <algorithm>
#include <functional>

#include "DlgMain.h"
#include "BeikeVulfixEngine.h"
#include "DlgIgnoredVuls.h"
#include "DlgInstalledVuls.h"

static bool NeedRepair( LPTUpdateItem pItem )
{
	return !pItem->isIgnored && pItem->nWarnLevel>=0;
}

static bool NeedRepair( LPTVulSoft pItem )
{
	return pItem->state.comState!=COM_ALL_DISABLED;
}

template<typename T, typename Func>
int CountItems(const CSimpleArray<T>& arr, Func fn)
{
	int count = 0;
	for(int i=0; i<arr.GetSize(); ++i)
	{
		if( fn(arr[i]) )
			++count;
	}
	return count;
}

static int AppendItem2VulList( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	CString strTitle;
	strTitle.Format(_T("KB%d"), pVulItem->nID);
	
	LPCTSTR lpszWarnDesc = GetLevelDesc(pVulItem->nWarnLevel);
	
	int nItem = listCtrl.Append(lpszWarnDesc, pVulItem->nWarnLevel>=0);
	listCtrl.AppendSubItem(nItem, strTitle);
	listCtrl.AppendSubItem(nItem, pVulItem->strName);
	listCtrl.AppendSubItem(nItem, pVulItem->strPubDate);
	listCtrl.AppendSubItem(nItem, pVulItem->nWarnLevel==-1 ? _T("无需修复") : (pVulItem->nWarnLevel>0 ? _T("未修复") : _T("可不修复")) );
	
	if(pVulItem->nWarnLevel>3)
		listCtrl.SetSubItemColor(nItem, 0, red, false);
	
	listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);
	return nItem;
}

static int AppendItemSoftVulList( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	int state = GetSoftItemState( pVulItem );
	int nItem = listCtrl.Append( pVulItem->strName, state!=VUL_COM_DISABLED );
	listCtrl.AppendSubItem(nItem, pVulItem->strAffects);
	listCtrl.AppendSubItem(nItem, SoftVulStatusDesc[state]);
	listCtrl.AppendSubItem(nItem, state==VUL_COM_DISABLED ? _T("无需修复") : _T("未修复") );
	listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);
	listCtrl.SetCheckState(nItem, true);
	return nItem;
}

static int AppendItem2RepairList( CListViewCtrlEx &listCtrl, T_VulListItemData * pVulItem )
{
	CString strFileSize;
	FormatSizeString(pVulItem->nFileSize, strFileSize);
	
	int nItem = listCtrl.Append( GetLevelDesc(pVulItem->nWarnLevel) );
	listCtrl.AppendSubItem(nItem, pVulItem->strName);
	listCtrl.AppendSubItem(nItem, pVulItem->strDesc);
	listCtrl.AppendSubItem(nItem, strFileSize);
	listCtrl.AppendSubItem(nItem, _T("未修复"));
	listCtrl.AppendSubItem(nItem, _T("反馈问题"), SUBITEM_LINK);
	listCtrl.SetItemData(nItem, (DWORD_PTR)pVulItem);
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

bool IsMustVul(LPTUpdateItem pItem)
{
	return !pItem->isIgnored && pItem->nWarnLevel>0;
}

bool IsOptionVul(LPTUpdateItem pItem)
{
	return !pItem->isIgnored && pItem->nWarnLevel==0;
}

bool IsSoftVulInstallable( LPTVulSoft pItem )
{
	return pItem->state.comState!=COM_ALL_DISABLED;
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
	if(pItem->nType!=VTYPE_SOFTLEAK && pItem->nType>0)
	{
		return pItem->nWarnLevel==0;
	}
	return false;
}


CViewSoftVulHandler::CViewSoftVulHandler( CInnerView &mainDlg ) : CBaseViewHandler<CInnerView>(mainDlg)
{
	m_dwPos = 0;
	m_firstInited = TRUE;
	
	m_nCurrentRelateInfoItem = -1;

	m_nTotalItem = 0;
	m_nCurrentItem = 0;
	m_nRepairTotal = m_nRepairInstalled = m_nRepairDownloaded = m_nRepairProcessed = 0;
}

CViewSoftVulHandler::~CViewSoftVulHandler(void)
{
}

BOOL CViewSoftVulHandler::Init(HWND hWndParent)
{
	ATLASSERT( IsWindow(hWndParent) );
	// vul list 
	m_wndListCtrlVul.Create( 
		hWndParent, NULL, NULL, 
		//WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_NOCOLUMNHEADER, 
		WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL , 
		0, 30011, NULL);
	{
		m_wndListCtrlVul.InsertColumn(0, _T("选择"), LVCFMT_LEFT, 55);
		m_wndListCtrlVul.InsertColumn(1, _T("补丁名称"), LVCFMT_LEFT, 65);
		m_wndListCtrlVul.InsertColumn(2, _T("补丁描述"), LVCFMT_LEFT, 230);
		m_wndListCtrlVul.InsertColumn(3, _T("发布日期"), LVCFMT_LEFT, 75);
		m_wndListCtrlVul.InsertColumn(4, _T("状态"), LVCFMT_LEFT, 70);
	}
	
	// repairing list 
	m_wndListCtrlVulFixing.Create( 
		hWndParent, NULL, NULL, 
		//WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_NOCOLUMNHEADER, 
		WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL , 
		0, 30200, NULL);
	{
		m_wndListCtrlVulFixing.InsertColumn(0, _T("重要"), LVCFMT_LEFT, 40);
		m_wndListCtrlVulFixing.InsertColumn(1, _T("补丁名称"), LVCFMT_LEFT, 160);
		m_wndListCtrlVulFixing.InsertColumn(2, _T("补丁描述"), LVCFMT_LEFT, 250);
		m_wndListCtrlVulFixing.InsertColumn(3, _T("补丁进度"), LVCFMT_LEFT, 100);
		m_wndListCtrlVulFixing.InsertColumn(4, _T("状态"), LVCFMT_LEFT, 60);
		m_wndListCtrlVulFixing.InsertColumn(5, _T("反馈问题"), LVCFMT_CENTER, 60);
	}
	
	m_ctlRichEdit.FirstInitialize( hWndParent, 30040 );
	m_ctlRichEdit.SetBackgroundColor( RGB(240, 244, 250) );
	return TRUE;
}

BOOL CViewSoftVulHandler::OnViewActived( INT nTabId, BOOL bActive )
{
	if(bActive)
	{
		if(m_firstInited)
		{
			m_firstInited = FALSE;
			OnBkBtnScan();
		}
	}
	return TRUE;
}

void CViewSoftVulHandler::OnBkBtnScan()
{
	if( theEngine->ScanVul( m_RefWin.m_hWnd ) )
	{
		m_nScanState = 0;
		m_nTotalItem = 0;
		m_nCurrentItem = 0;
		_SetDisplayState(SCANSTATE_SCANNING);
		_SetScanProgress( 0 );
		m_RefWin.SetTimer(0, 200, NULL);
		m_RefWin.StartIconAnimate( 30050 );
	}
}

void CViewSoftVulHandler::OnBkBtnCancelScan()
{
	theEngine->CancelScanVul();
	m_RefWin.StopIconAnimate();
	_SetDisplayState(SCANSTATE_DISPLAY);
}

void CViewSoftVulHandler::_SetDisplayState( TScanSoftState st, TRepairSubState subst )
{
	static int viewids[] = {3001, 3005, 3006};
	int count = sizeof(viewids)/sizeof(int);
	for(int i=0; i<count; ++i)
	{
		if(st==SCANSTATE_REPAIRING)
		{
			for(int j=0; j<=REPAIRSTATE_FAIL; ++j)
			{
				SetItemVisible(801+j, j==subst);
			}
		}

		SetItemVisible(viewids[i], st==i, i==count-1);
	}
}

void CViewSoftVulHandler::OnBkBtnSwitchRelateInfo()
{
	m_bRelateInfoShowing = !m_bRelateInfoShowing;
	if(m_bRelateInfoShowing)
	{
		SetItemText(30001, _T(">"), FALSE);
		SetItemVisible(301, TRUE);
		SetItemAttribute(300301, "pos", "5,35,-255,-5");
	}
	else
	{
		SetItemText(30001, _T("<"), FALSE);
		SetItemVisible(301, FALSE);
		SetItemAttribute(300301, "pos", "5,35,-5,-5");
	}
}

void CViewSoftVulHandler::OnBkBtnSelectAll()
{
	SetListCheckedAll(m_wndListCtrlVul, true);
}

void CViewSoftVulHandler::OnBkBtnSelectNone()
{
	SetListCheckedAll(m_wndListCtrlVul, false);
}

void CViewSoftVulHandler::OnBkBtnSelectAllSuggested()
{
	SetListCheckedAll( m_wndListCtrlVul, std::ptr_fun(SelectSuggest) );
}

void CViewSoftVulHandler::OnBkBtnSelectAllOptional()
{
	SetListCheckedAll( m_wndListCtrlVul, std::ptr_fun(SelectOptinal) );	
}

void CViewSoftVulHandler::OnBkBtnBeginRepair()
{
	int nComRepaired = RepairCOMVul(m_wndListCtrlVul, false);
	
	int nChecked = 0;
	// 
	CSimpleArray<int> arrVul, arrSoft;
	CSimpleArray<T_VulListItemData*> arrItemsUpdate;
	for(int i=0; i<m_wndListCtrlVul.GetItemCount(); ++i)
	{
		if(m_wndListCtrlVul.GetCheckState(i))
		{
			++ nChecked;

			T_VulListItemData *pItem = (T_VulListItemData*) m_wndListCtrlVul.GetItemData( i );	
			ATLASSERT(pItem);
			if(pItem)
			{
				if(pItem->nType==VTYPE_SOFTLEAK)
				{
					int state = GetSoftItemState( pItem );
					if(state==VUL_UPDATE)
					{
						arrItemsUpdate.Add( pItem );
						arrSoft.Add( pItem->nID );
					}
				}
				else
				{
					arrItemsUpdate.Add( pItem );
					arrVul.Add( pItem->nID );
				}
			}
		}
	}
	
	if( nChecked==0)
	{
		::MessageBox(NULL, _T("没有选择内容"), NULL, MB_OK);
	}
	else
	{
		if(nComRepaired>0 && arrSoft.GetSize()==0 && arrVul.GetSize()==0 )
			OnBkBtnScan();
		
		if(arrSoft.GetSize()>0 || arrVul.GetSize()>0)
		{
			m_nRepairTotal = arrSoft.GetSize() + arrVul.GetSize();
			m_nRepairInstalled = 0;
			m_nRepairDownloaded = 0;
			m_nRepairProcessed = 0;
		
			m_RefWin.StartIconAnimate(30100);

			_SetDisplayState(SCANSTATE_REPAIRING);
			_UpdateRepairTitle();
			
			// Fill the list ctrl 
			ResetListCtrl(m_wndListCtrlVulFixing);
			for(int i=0; i<arrItemsUpdate.GetSize(); ++i)
			{
				T_VulListItemData *pItem = arrItemsUpdate[i];
				AppendItem2RepairList( m_wndListCtrlVulFixing, new T_VulListItemData( *pItem ) );
			}
			
			// Fix 
			theEngine->RepairAll( m_RefWin.m_hWnd, arrVul, arrSoft);
		}
	}
}

void CViewSoftVulHandler::OnBkBtnCancelRepair()
{
	m_RefWin.StopIconAnimate();
	theEngine->CancelRepair();
	OnBkBtnScan();
}

void CViewSoftVulHandler::OnBkBtnRunBackground()
{
	//::ShowWindow(m_RefWin.m_hWnd, SW_SHOWMINIMIZED);
}

void CViewSoftVulHandler::OnBkBtnReboot()
{
	//m_RefWin.ShutDownComputer(TRUE);
}

void CViewSoftVulHandler::OnBkBtnEnableRelateCOM()
{
	_EnableRelateCOM(TRUE);
	OnBkBtnScan();
}

void CViewSoftVulHandler::OnBkBtnDisableRelateCOM()
{
	_EnableRelateCOM(FALSE);
	OnBkBtnScan();
}

void CViewSoftVulHandler::OnBkBtnViewIgnored()
{
	CDlgIgnoredVuls dlg;
	dlg.Load(IDR_BK_IGNORED_VUL_DIALOG);
	if( IDOK==dlg.DoModal() )
		OnBkBtnScan();
}

void CViewSoftVulHandler::OnBkBtnViewInstalled()
{
	CDlgInstalledVuls dlg;
	dlg.Load(IDR_BK_INSTALLED_VUL_DIALOG);
	dlg.DoModal();
}


LRESULT CViewSoftVulHandler::OnListBoxVulFixNotify( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
{
	if(pnmh->code==LVN_ITEMCHANGED)
	{
		LPNMLISTVIEW pnmv = (LPNMLISTVIEW) pnmh; 
		DEBUG_TRACE(_T("ItemChanged (%d %d,%d,%d)\n"), pnmv->iItem, pnmv->uOldState, pnmv->uNewState, pnmv->uChanged);

		if( pnmv->uNewState & LVIS_SELECTED)
		{
			_DisplayRelateVulFixInfo( pnmv->iItem );
		}
	}
	bHandled = FALSE;
	return 0;
}

void CViewSoftVulHandler::_DisplayRelateVulFixInfo( int nItem )
{
	if(m_nCurrentRelateInfoItem==nItem)
		return; 
	
	m_nCurrentRelateInfoItem = nItem;
	T_VulListItemData *pItemData = NULL;
	if(nItem>=0)
		pItemData = (T_VulListItemData*)m_wndListCtrlVul.GetItemData( nItem );
	
	if(pItemData)
	{
		SetRelateInfo(m_ctlRichEdit, pItemData, TRUE);
		SetItemVisible(3003, FALSE);
		SetItemVisible(3004, TRUE);
	}
	else
	{
		SetItemVisible(3003, FALSE);
		SetItemVisible(3004, TRUE);
	}
}

LRESULT CViewSoftVulHandler::OnVulFixEventHandle( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	int evt = uMsg - WMH_VULFIX_BASE;
	if(evt==EVulfix_ScanBegin||evt==EVulfix_ScanProgress)
	{
		if(evt==EVulfix_ScanBegin)
		{
			if(wParam==VTYPE_WINDOWS)
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
	else
	{
		int nKbId = wParam;
		int nIndex = FindListItem( nKbId );
		if(nIndex==-1)
			return 0; 
		
		int nDownloadPercent = 0;
		int nSubitem = -1, nSubitem2=-1;
		CString strTitle, strTitle2;
		COLORREF clr = black, clr2=black;
		switch (evt)
		{
		case EVulfix_DownloadBegin:
		case EVulfix_DownloadProcess:
		case EVulfix_DownloadEnd:
		case EVulfix_DownloadError:
			nSubitem = 3;
			if( EVulfix_DownloadProcess==evt )
			{
				T_VulListItemData *pItemData = (T_VulListItemData*)m_wndListCtrlVulFixing.GetItemData( nIndex );
				ATLASSERT(pItemData);
				if(pItemData)
				{
					CString strFileSize, strDownloadedSize;
					FormatSizeString(lParam, strDownloadedSize);
					FormatSizeString(pItemData->nFileSize, strFileSize);
					strTitle.Format(_T("%s/%s"), strDownloadedSize, strFileSize);

					nSubitem2 = 4;
					nDownloadPercent = (100*lParam)/pItemData->nFileSize;
					strTitle2.Format(_T("已下载%d%%"), nDownloadPercent );	
				}
			}
			else if(EVulfix_DownloadEnd==evt)
			{
				strTitle = _T("已下载");
				++m_nRepairDownloaded;

				nSubitem2 = 4;
				strTitle2 = strTitle;
			}
			else if(EVulfix_DownloadError==evt)
			{
				clr = red;
				strTitle = _T("下载失败 ");
				++ m_nRepairProcessed;

				nSubitem2 = 4;
				clr2 = red;
				strTitle2 = strTitle;
			}
			else
				strTitle = _T("正连接 ");
			break;

		case EVulfix_InstallBegin:
		case EVulfix_InstallEnd:
		case EVulfix_InstallError:
			nSubitem = 4;
			if(EVulfix_InstallBegin==evt)
			{
				clr = blue;
				strTitle = _T("正安装 ");
			}
			else if(EVulfix_InstallEnd==evt)
			{
				clr = green;
				strTitle = _T("已安装");
				++m_nRepairInstalled;
			}
			else if(EVulfix_InstallError==evt)
			{
				clr = red;
				strTitle = _T("安装失败");
			}

			if(EVulfix_InstallError==evt || EVulfix_InstallEnd==evt)
				++ m_nRepairProcessed;
			break;

		case EVulfix_Task_Complete:
		case EVulfix_Task_Error:
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
		if(evt==EVulfix_DownloadBegin || evt==EVulfix_DownloadError || evt==EVulfix_InstallEnd || evt==EVulfix_InstallError)
			// 更新Title 
		{
			_UpdateRepairTitle();
			_SetRepairProgress( m_nRepairTotal==0 ? 100 : (100*m_nRepairProcessed)/m_nRepairTotal );
		}
	}
	return 0;
}

int CViewSoftVulHandler::FindListItem( int nID )
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

void CViewSoftVulHandler::_UpdateRepairTitle()
{
	CString strTitle;
	strTitle.Format(_T("正在修复软件漏洞(%d, %d/%d)..."), m_nRepairInstalled, m_nRepairDownloaded, m_nRepairTotal );
	SetItemText(30101, strTitle);
}

void CViewSoftVulHandler::_SetScanProgress( int nPos )
{
	SetItemDWordAttribute(30051, "value", nPos, TRUE);
}

void CViewSoftVulHandler::_SetRepairProgress( int nPos )
{
	SetItemDWordAttribute(30102, "value", nPos, TRUE);
}

LRESULT CViewSoftVulHandler::OnScanStart( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	OnBkBtnScan();
	return 0;
}

LRESULT CViewSoftVulHandler::OnScanDone( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	_SetDisplayState(SCANSTATE_DISPLAY);
	if(wParam==0)
	{
		ATLASSERT(theEngine->m_pVulScan && theEngine->m_pSoftVulScan);
		ResetListCtrl(m_wndListCtrlVul);
		
		const CSimpleArray<LPTUpdateItem>& arr2 = theEngine->m_pVulScan->GetResults();
		AppendVuls(m_wndListCtrlVul, arr2, std::ptr_fun(IsMustVul) );
		AppendVuls(m_wndListCtrlVul, arr2, std::ptr_fun(IsOptionVul) );

		const CSimpleArray<LPTVulSoft>& arr = theEngine->m_pSoftVulScan->GetResults();
		AppendVuls(m_wndListCtrlVul, arr, std::ptr_fun(IsSoftVulInstallable) );
		
		_DisplayRelateVulFixInfo(-1);
	}
	m_RefWin.StopIconAnimate();
	return 0;
}

LRESULT CViewSoftVulHandler::OnRepaireDone( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	bool toShutdown = !wParam && GetItemCheck(30105);
	
	if(toShutdown)
	{
		// not canceled , check if need shutdown  
		//TODO : shutdown the computer 
		//ShutDownComputer(FALSE);
	}
	else
	{
		m_RefWin.StopIconAnimate();
		if(m_nRepairInstalled==m_nRepairTotal)
			_SetDisplayState(SCANSTATE_REPAIRING, REPAIRSTATE_DONE);
		else if(m_nRepairInstalled>0)
			_SetDisplayState(SCANSTATE_REPAIRING, REPAIRSTATE_DONE_PART);
		else
			_SetDisplayState(SCANSTATE_REPAIRING, REPAIRSTATE_FAIL);
	}	
	return 0;
}

void CViewSoftVulHandler::_EnableRelateCOM( BOOL bEnable )
{
	if(m_nCurrentRelateInfoItem>=0)
	{
		T_VulListItemData *pItemData = NULL;
		pItemData = (T_VulListItemData*)m_wndListCtrlVul.GetItemData( m_nCurrentRelateInfoItem );
		theEngine->m_pSoftVulScan->EnableVulCOM( pItemData->nID, bEnable );
	}
}

LRESULT CViewSoftVulHandler::OnRichEditLink( int idCtrl, LPNMHDR pnmh, BOOL& bHandled )
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
			bool founded=false;
			bool needRescan = false;
			m_ctlRichEdit.SetSel( pLink->chrg );
		
			CString str;
			m_ctlRichEdit.GetSelText(str);
			for(int i=0; i<=VUL_DISABLE_COM; ++i)
			{
				if( str==SoftVulStatusOP[i] )
				{
					founded = true;
					if(i==VUL_UPDATE)
					{
						// TODO : 下载exe 
						ShellExecute(NULL, _T("open"), pItemData->strWebPage, NULL, NULL, SW_SHOW);
					}
					else if(i==VUL_COM_DISABLED)
					{
						theEngine->m_pSoftVulScan->EnableVulCOM( pItemData->nID, TRUE );
						needRescan = true;
					}
					else if(i==VUL_DISABLE_COM)
					{
						theEngine->m_pSoftVulScan->EnableVulCOM( pItemData->nID, FALSE );
						needRescan = true;
					}
				}
			}
			if(needRescan)
			{
				OnBkBtnScan();
			}
			else if(!founded)
			{
				ShellExecute(NULL, _T("open"), pItemData->strWebPage, NULL, NULL, SW_SHOW);
			}
		}
	}
	return 0;
}
