//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "guidef.h"
#include "maindlg.h"
#include "regbackupdlg.h"
#include "kscver.h"
#include "kuimsgbox.h"
#include "misc/IniEditor.h"
#include "stubbornregs.h"
#include "kscbase/kscsys.h"

//////////////////////////////////////////////////////////////////////////

#define LIST_COLUMN_WIDTH_OFFSET		((4 * 100) + 26)
#define	WND_LEFT_MARGIN					5
#define WND_RIGHT_MARGIN				5
#define WND_TOP_MARGIN					5
#define WND_BOTTOM_MARGIN				10

#define	PROGRESS_MAX_RANGE				100			// 按10s扫描时间计算
#define TIMER_ID_SCAN					0x1001
#define TIMER_ID_CLEAN					0x1002


static CMainDlg*	gs_lpThisDlg = NULL;

//////////////////////////////////////////////////////////////////////////

CMainDlg::CMainDlg() :
	m_bStop(false), m_hScanThread(NULL), m_hCleanThread(NULL), m_scStatus(CMainDlg::SCS_IDLE),
	m_nRegistryCount(0), m_bParentKsafe(false), m_dwCleanTickCount(0),
	m_hOwnTimerThread(NULL), m_lpTxtDescProc(NULL), m_bTxtDescPaintClr(false)
{
	gs_lpThisDlg = this;
	m_AppVerModule.Init(NULL);
	wchar_t szAppPath[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, szAppPath, MAX_PATH);
	PathRemoveFileSpecW(szAppPath);
	::PathAppend(szAppPath,L"cfg\\lastrecord.inf");
	m_strCacheFile = szAppPath;
	CStubbornRegs::Instance().Init();
}

//////////////////////////////////////////////////////////////////////////

CMainDlg::~CMainDlg()
{
    if ( m_hScanThread != NULL )
    {
        ::CloseHandle(m_hScanThread);
        m_hScanThread = NULL;
    }
    if(NULL != m_hCleanThread)
    {
        CloseHandle(m_hCleanThread);
        m_hCleanThread = NULL;
    }
	m_AppVerModule.UnInit();

	IniEditor IniEdit;
	IniEdit.SetFile(m_strCacheFile.GetBuffer());

	std::map<int,int>::iterator iter = m_bSelectMap.begin();
	for (; iter!=m_bSelectMap.end(); iter++)
	{
		TCHAR szFlag[32] = {0};
		_itow_s(iter->first,szFlag,32);
		IniEdit.WriteDWORD(L"kclear_checke_reg",szFlag,iter->second);
	}

	CStubbornRegs::Instance().UnInit();
}

//////////////////////////////////////////////////////////////////////////

LRESULT CMainDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	HWND	hwndParent = GetParent();
	
	// 判断父窗口是否为卫士
	if ( ::IsWindow(hwndParent) )
	{
		TCHAR	szClass[40] = {};

		::GetClassName(hwndParent, szClass, 40);
		if ( ::_tcsnicmp(szClass, TEXT("KscTool"), 7) == 0 )
			m_bParentKsafe = true;
	}

	// 初始化控件
	SetMainWndSize();
	InitControl();
	InitListCtrl();
	SetControlPos();

	// 自动开始扫描
	//StartScan();
	SetScStatus(CMainDlg::SCS_IDLE);

	bHandled = TRUE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

void CMainDlg::OnPaint(HDC hDC)
{
	CPaintDC	PaintDC(m_hWnd);
	CRect		WndRect;
	CRect		TempRect;
	CBrush		FrameBrush;

	GetClientRect(WndRect);
	PaintDC.FillSolidRect(WndRect, DIALOG_BK_COLOR);
}


void CMainDlg::OnSize(UINT wParam, const _WTYPES_NS::CSize& size)
{
	if ( !m_bParentKsafe )
	{
		return;
	}

	switch( wParam )
	{
	case SIZE_MINIMIZED:
		return;
		break;
	default:
		SetMainWndSize();
		SetControlPos();
		break;
	}
}


//////////////////////////////////////////////////////////////////////////

LRESULT CMainDlg::OnListClick(int idCtrl, LPNMHDR pnmh,  BOOL &bHandled)
{
	CListNotify*				pNotify = (CListNotify *)pnmh;
	UINT						unTemp = 0;
	CRegistryData::SCAN_RESULT*	lpResult = NULL;

	if ( pNotify->m_nItem == NULL_ITEM )
		return 0;
	
	unTemp = m_ListCtrl.GetItemFormat(pNotify->m_nItem, 0);
	if ( unTemp & ITEM_FORMAT_FULL_LINE )
	{
		return 0;
	}

	return 0;
}


LRESULT CMainDlg::OnListSelect(int idCtrl, LPNMHDR pnmh,  BOOL &bHandled)
{
	CListNotify*				pNotify = (CListNotify *)pnmh;
	UINT						unTemp = 0;
	CRegistryData::SCAN_RESULT*	lpResult = NULL;

	if ( pNotify->m_nItem == NULL_ITEM )
		return 0;

	// 检查是否选择了单选框
	unTemp = m_ListCtrl.GetItemFormat(pNotify->m_nItem, 0);
	if ( !(unTemp & ITEM_FORMAT_CHECKBOX) ) {
		return 0;
	}
	
	// 设置是否清除
	m_ListCtrl.GetItemData(pNotify->m_nItem, lpResult);
	if ( lpResult == NULL )
		return 0;
	
	lpResult->SetToClean( m_ListCtrl.GetItemCheck(pNotify->m_nItem, 0) ? true : false );
	return 0;
}


//////////////////////////////////////////////////////////////////////////

LRESULT CMainDlg::OnCtlColorStatic(HDC hDC, HWND hWnd)
{
	HBRUSH		hbr = NULL;
	CRect		rc;

	::GetClientRect(hWnd, rc);
	hbr = ::CreateSolidBrush(DIALOG_BK_COLOR);
	::FillRect(hDC, rc, hbr);
	::DeleteObject(hbr);

	return (LRESULT)GetStockObject(NULL_BRUSH);
}

//////////////////////////////////////////////////////////////////////////

void CMainDlg::OnClose()
{	
	
	EndDialog(0);
}


void CMainDlg::OnScanCleanTimer(UINT_PTR nIDEvent)
{
	int				nPos = 0;
	int				nStep = 1;
	static int		nTimer = 0;

	if ( nIDEvent != TIMER_ID_CLEAN && nIDEvent != TIMER_ID_SCAN )
		return;

	// 更新进度条
	/*nPos = m_ScanCleanProgress.GetPos();
	nStep = m_ScanCleanProgress.GetStep();
	m_ScanCleanProgress.SetPos((nPos >= PROGRESS_MAX_RANGE - 1 - nStep) ? (PROGRESS_MAX_RANGE - 1 - nStep) : nPos + nStep);
	*/

	// 更新动画
	m_bmpLoading.mnIndex = (nTimer++ % 8);
	m_bmpLoading.ShowWindow(SW_SHOW);
	m_bmpLoading.Invalidate(TRUE);

	// 更新任务条
	{
		KAutoLock	lockTips(m_lockTips);
		::SetWindowText(m_txtDesc, m_strTxtDesc.c_str());
	}
}


//////////////////////////////////////////////////////////////////////////

BOOL CMainDlg::SetMainWndSize()
{
	HWND		hwndParent = GetParent();
	CRect		rcParent;
	int			nWidth = 0, nHeight = 0;
	
	// 修改窗口尺寸
	if ( ::IsWindow(hwndParent) )
	{
		::GetClientRect(hwndParent, rcParent);

		if ( m_bParentKsafe )
		{
			nWidth = rcParent.Width();
			nHeight = rcParent.Height();
		}
		else
		{
			nWidth = MAIN_WIDTH;
			nHeight = MAIN_HEIGHT;
		}
	}
	else
	{
		nWidth = MAIN_WIDTH;
		nHeight = MAIN_HEIGHT;
	}
	
	SetWindowPos(NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER);
	GetClientRect(m_rcMainClient);

	return TRUE;
}


void CMainDlg::InitControl()
{
	CRect		rc, rcScan;
	int			nVScrollWidth = 0;

	nVScrollWidth = ::GetSystemMetrics(SM_CXVSCROLL);

	// 提示图标
	m_bmpTips.SubclassWindow(GetDlgItem(IDC_PIC_TIPS));
	m_imgListTips.CreateFromImage(IDB_TIPS, 15, 8, RGB(255, 0, 255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
	m_bmpTips.mpImageList = &m_imgListTips;
	m_bmpTips.mnIndex = 0;
	m_bmpTips.ShowWindow(SW_HIDE);

	// 动态图标
	m_bmpLoading.SubclassWindow(GetDlgItem(IDC_PIC_LOADING));
	m_imgListLoading.CreateFromImage(IDB_LOADING, 16, 8, RGB(255, 0, 255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
	m_bmpLoading.mpImageList = &m_imgListLoading;
	m_bmpLoading.mnIndex = 0;
	m_bmpLoading.ShowWindow(SW_HIDE);

	// 修改扫描文本提示
	m_txtDesc = GetDlgItem(IDC_TXT_DESC);
	m_lpTxtDescProc = (LPWNDPROC)(LPVOID)(ULONG_PTR)::SetWindowLong(m_txtDesc, GWL_WNDPROC, (LONG)(LONG_PTR)&TxtDescWndProc);
	/*m_txtDesc.SubclassWindow(GetDlgItem(IDC_TXT_DESC));
	m_txtDesc.SetWindowText(L"请单击\"重新扫描\"按钮开始扫描！");
	m_txtDesc.SetTransparent(TRUE);
	m_txtDesc.SetFontName(TEXT("Microsoft Sans Serif"));
	m_txtDesc.SetTextColor(RGB(0, 0, 0));
	m_txtDesc.ShowWindow(SW_SHOW);*/
	
	// 进度条
	m_ScanCleanProgress.SubclassWindow(GetDlgItem(IDC_PROGRESS), IDB_PROGRESS);
	m_ScanCleanProgress.ShowWindow(SW_HIDE);
	
	// 全选
	m_chkSelectAll = GetDlgItem(IDC_SELECTALL);
	m_txtSelAll = GetDlgItem(IDC_SELALL_T);

	// 推荐选项
	SET_HYPERCR_LINK(m_btnDefault, IDC_DEFAULT);

	// 历史
	SET_HYPERCR_LINK(m_btnHistory, IDC_BACKUP);

	// 子类化图片按钮: 开始扫描
	m_btnStartScan.SubclassWindow(GetDlgItem(IDC_BTN_SCAN));
	m_btnStartScan.SetBitmapButtonExtendedStyle(BMPBTN_HOVER);
	m_btnStartScan.SetBtnImages(IDB_BTNNORMAL_B, IDB_BTNDOWN_B, IDB_BTNHOVER_B, IDB_BTNDISABLE_B);
	m_btnStartScan.SetImages(0, 1, 2, 3);
	m_btnStartScan.SetCursor((UINT)(ULONG_PTR)IDC_HAND);
	m_btnStartScan.SetTitle(IDS_SCAN);
	m_btnStartScan.SetFont(TEXT("宋体"), 90);
	
	// 停止
	m_btnStopScan.SubclassWindow(GetDlgItem(IDC_BTN_STOP));
	m_btnStopScan.SetBitmapButtonExtendedStyle(BMPBTN_HOVER);
	m_btnStopScan.SetBtnImages(IDB_BTNNORMAL_B, IDB_BTNDOWN_B, IDB_BTNHOVER_B, IDB_BTNDISABLE_B);
	m_btnStopScan.SetImages(0, 1, 2, 3);
	m_btnStopScan.SetCursor((UINT)(ULONG_PTR)IDC_HAND);
	m_btnStopScan.SetTitle(IDS_STOP);
	::ShowWindow(m_btnStopScan, SW_SHOW);
	m_btnStartScan.SetFont(TEXT("宋体"), 90);

	// 清除
	m_btnClean.SubclassWindow(GetDlgItem(IDC_BTN_CLEAN));
	m_btnClean.SetBitmapButtonExtendedStyle(BMPBTN_HOVER);
	m_btnClean.SetBtnImages(IDB_BTNNORMAL_B, IDB_BTNDOWN_B, IDB_BTNHOVER_B, IDB_BTNDISABLE_B);
	m_btnClean.SetImages(0, 1, 2, 3);
	m_btnClean.SetCursor((UINT)(ULONG_PTR)IDC_HAND);
	m_btnClean.SetTitle(IDS_CLEAN);
	m_btnStartScan.SetFont(TEXT("宋体"), 90);
}


//////////////////////////////////////////////////////////////////////////

void CMainDlg::InitListCtrl()
{
	CString		sText;
	CRect		rcClient;
	COLORREF	color = RGB(222, 239, 214);

	m_ListCtrl.RegisterClass();
	m_ListCtrl.SubclassWindow(GetDlgItem(IDC_LIST));
	m_ListCtrl.ShowWindow(SW_HIDE);

	if ( m_bParentKsafe )
		color = RGB(185, 219, 255);
	else
		color = RGB(222, 239, 214);
	m_ListCtrl.SetSelectedItemColor(color);

	sText = TEXT("类别");
	m_ListCtrl.AddColumn(sText, 160);

	sText = TEXT("注册表位置");
	m_ListCtrl.AddColumn(sText, 350);

	sText = TEXT("问题描述");
	m_ListCtrl.AddColumn(sText, 258);

	// TreeView
	WTL::CBitmap	bmIconList;
	bmIconList.LoadBitmap( IDB_LISTITEMS );
	m_imlTreeIcons.Create( 16, 16, ILC_COLOR32 | ILC_MASK, 12, 1 );
	m_imlTreeIcons.Add( bmIconList, RGB( 255, 0, 255 ) );

	m_tree.SubclassWindow( GetDlgItem( IDC_TREE ) );
	m_tree.SetImageList( m_imlTreeIcons, TVSIL_NORMAL );
	m_tree.SetItemHeight( 20 );

	// 添加元素
	HTREEITEM	hTreeItem = NULL;
	for( int i = CRegistryData::RST_BEGIN; i < (int)CRegistryData::RST_END; ++i )
	{
		hTreeItem = m_tree.InsertItem( 
			CRegistryData::GetTitle((CRegistryData::REG_SCAN_TYPE)i), 
			GetTypeIconIndex((CRegistryData::REG_SCAN_TYPE)i), GetTypeIconIndex((CRegistryData::REG_SCAN_TYPE)i),
			NULL, 
			NULL,  
			KUIMulStatusTree::EM_TVIS_CHECK);

		m_tree.SetItemData( hTreeItem, -1 );
		m_TreeRootMap[(CRegistryData::REG_SCAN_TYPE)i] = hTreeItem;
	}

	// 使用默认配置
	if(::PathFileExists(m_strCacheFile.GetBuffer()))
	{
		IniEditor IniEdit;
		IniEdit.SetFile(m_strCacheFile.GetBuffer());

		TREE_ROOT_MAP::iterator	iter;
		int checkNum = 0;

		for ( iter = m_TreeRootMap.begin(); iter != m_TreeRootMap.end(); ++iter )
		{
			int nSelect = 0;
			TCHAR szFlag[32] = {0};
			_itow_s(iter->first,szFlag,32);
			BOOL bOk = IniEdit.ReadDWORD(L"kclear_checke_reg",szFlag,nSelect);
			if(!bOk)
				goto _exit_;
			if (1==nSelect)
			{
				m_tree.SetCheck(iter->second, KUIMulStatusTree::EM_TVIS_CHECK);
				checkNum ++;
			}
			else
				m_tree.SetCheck(iter->second, KUIMulStatusTree::EM_TVIS_UNCHECK);
		}


		if(checkNum == 10)
		{
			::SendMessage(m_chkSelectAll, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		}
		else 
			::SendMessage(m_chkSelectAll, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);


	}
	else
	{
	_exit_:
		SelectDefaultItems();
	}
}


void CMainDlg::SetControlPos()
{
	CRect		rc, rcLast;
	int			nVScrollWidth = 0;
	int			nBmpWidth = 72, nBmpHeight = 26;
	int			nLastCtrlLeft = 0;
	const int	nLeftPadding = 10, nRightPadding = 10;

	if ( !::IsWindow(m_bmpTips) )
		return;

	nVScrollWidth = ::GetSystemMetrics(SM_CXVSCROLL);

	// 提示图标
	m_bmpTips.SetWindowPos(NULL, nLeftPadding, 8, 15, 15, SWP_NOZORDER);
	m_bmpTips.GetWindowRect(rc);
	ScreenToClient(rc);

	// 动态图标
	m_bmpLoading.SetWindowPos(NULL, nLeftPadding, 8, 16, 16, SWP_NOZORDER);

	rcLast = rc;
	::GetWindowRect(m_txtDesc, rc);
	ScreenToClient(rc);
	::SetWindowPos(m_txtDesc, NULL, nLeftPadding + rcLast.Width() + 5, 10, m_rcMainClient.Width() - 50, rc.Height(), 
		SWP_NOZORDER);
	::GetWindowRect(m_txtDesc, m_rcTopArea);
	ScreenToClient(m_rcTopArea);

	// 进度条
	m_ScanCleanProgress.GetWindowRect(rc);
	ScreenToClient(rc);
	m_ScanCleanProgress.MoveWindow(m_rcTopArea.left + 100, m_rcTopArea.top, 
		(m_rcMainClient.Width() - 200 > 0) ? (m_rcMainClient.Width() - 200) : 100, 16);

	// 树型视图
	GetClientRect(rc);
	::SetWindowPos(m_tree, NULL, nLeftPadding, 30, 
		rc.Width() - nLeftPadding - nRightPadding, 
		rc.Height() - 30 - 35,
		SWP_NOZORDER);

	// 全选
	::GetWindowRect(m_chkSelectAll, rc);
	ScreenToClient(rc);
	nLastCtrlLeft = nLeftPadding;
	::SetWindowPos(m_chkSelectAll, NULL, nLastCtrlLeft, 
		m_rcMainClient.bottom - 13 - 9,
		rc.Width(), 13, SWP_NOSIZE | SWP_NOZORDER);
	::GetWindowRect(m_chkSelectAll, rc);
	ScreenToClient(rc);

	::SetWindowPos(m_txtSelAll, NULL, nLeftPadding + rc.Width() + 3, rc.top - 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	rc.right += 26;

	// 推荐
	rcLast = rc;
	::GetWindowRect(m_btnDefault, rc);
	ScreenToClient(rc);
	nLastCtrlLeft += rcLast.Width() + 15;
	::SetWindowPos(m_btnDefault, NULL, nLastCtrlLeft, 
		m_rcMainClient.bottom - 10 - rc.Height(),
		rc.Width(), rc.Height(), SWP_NOZORDER);

	// 历史按钮
	rcLast = rc;
	::GetWindowRect(m_btnHistory, rc);
	ScreenToClient(rc);
	nLastCtrlLeft += rcLast.Width() + 10;
	::SetWindowPos(m_btnHistory, NULL,  nLastCtrlLeft, 
		m_rcMainClient.bottom - 10 - rc.Height(), 
		rc.Width(), rc.Height(), SWP_NOZORDER);

	// 立即清除
	m_btnClean.GetWindowRect(m_rcBtbArea);
	ScreenToClient(m_rcBtbArea);
	m_btnClean.GetWindowRect(rc);
	ScreenToClient(rc);
	::SetWindowPos(m_btnClean, NULL, 0, 0, nBmpWidth, nBmpHeight, SWP_NOMOVE | SWP_NOZORDER);

	m_btnClean.GetWindowRect(rc);
	ScreenToClient(rc);
	nLastCtrlLeft =  m_rcMainClient.right - nRightPadding - rc.Width();
	::SetWindowPos(m_btnClean, NULL,  nLastCtrlLeft, 
		m_rcMainClient.bottom - 4 - rc.Height(), 
		0, 0, SWP_NOSIZE | SWP_NOZORDER);

	// 停止扫描
	m_btnStopScan.GetWindowRect(rc);
	ScreenToClient(rc);
	::SetWindowPos(m_btnStopScan, NULL, 0, 0, nBmpWidth, nBmpHeight, SWP_NOMOVE | SWP_NOZORDER);

	m_btnStopScan.GetWindowRect(rc);
	ScreenToClient(rc);
	nLastCtrlLeft -= 10 + rc.Width();
	::SetWindowPos(m_btnStopScan, NULL, nLastCtrlLeft, 
		m_rcMainClient.bottom - 4 - rc.Height(), 
		0, 0, SWP_NOSIZE | SWP_NOZORDER);

	// 开始扫描
	::GetWindowRect(m_btnStartScan, rc);
	ScreenToClient(rc);
	::SetWindowPos(m_btnStartScan, NULL, 0, 0, nBmpWidth, nBmpHeight, SWP_NOMOVE | SWP_NOZORDER);

	::GetWindowRect(m_btnStartScan, rc);
	ScreenToClient(rc);
	//nLastCtrlLeft =  m_rcMainClient.right - nRightPadding - rc.Width();
	//nLastCtrlLeft -= 10 + rc.Width();
	::SetWindowPos(m_btnStartScan, NULL, nLastCtrlLeft, 
		m_rcMainClient.bottom - 4 - rc.Height(), 
		0, 0, SWP_NOSIZE | SWP_NOZORDER);

	

	
}


void CMainDlg::SetScStatus(SC_STATUS status)
{
	if ( status < CMainDlg::SCS_IDLE || status > CMainDlg::SCS_CLEANED )
		return;

	// 保存当前状态
	m_scStatus = status;

	if ( status == CMainDlg::SCS_IDLE )
	{
		CRect		rc;

		m_bmpTips.mnIndex = 3;
		m_bmpTips.ShowWindow(SW_SHOW);
		m_bmpLoading.ShowWindow(SW_HIDE);

		::SetWindowText(m_txtDesc, TEXT("清理注册表可加快系统速度，建议定期清理！（清理之前，注册表会自动备份，以便恢复）"));
		::ShowWindow(m_txtDesc, SW_SHOW);
		::GetWindowRect(m_txtDesc, rc);
		::SetWindowPos(m_txtDesc, NULL, 0, 0, m_rcMainClient.Width() - 50, rc.Height(), SWP_NOZORDER | SWP_NOMOVE);

		m_ScanCleanProgress.ShowWindow(SW_HIDE);
		m_btnClean.ShowWindow(SW_SHOW);
		m_btnHistory.ShowWindow(SW_SHOW);
		m_btnStartScan.ShowWindow(SW_SHOW);
		m_btnStopScan.ShowWindow(SW_HIDE);

		::EnableWindow(m_chkSelectAll, TRUE);
		::EnableWindow(m_txtSelAll, TRUE);
		::EnableWindow(m_btnDefault, TRUE);
		
		m_btnClean.EnableWindow(FALSE);
		m_btnStartScan.EnableWindow(TRUE);

		m_tree.EnableCheck(TRUE);
		return;
	}

	if ( status == CMainDlg::SCS_SCAN )
	{
		CRect	rc;

		m_bmpTips.ShowWindow(SW_HIDE);
		m_bmpLoading.ShowWindow(SW_SHOW);
		
		m_bTxtDescPaintClr = false;
		m_strTxtDesc = TEXT("正在扫描...");
		::SetWindowText(m_txtDesc, TEXT("正在扫描..."));
		::ShowWindow(m_txtDesc, SW_SHOW);
		::GetWindowRect(m_txtDesc, rc);
		::SetWindowPos(m_txtDesc, NULL, 0, 0, m_rcMainClient.Width() - 50, rc.Height(), SWP_NOZORDER | SWP_NOMOVE);
		::InvalidateRect(m_txtDesc, NULL, FALSE);
		
		m_ScanCleanProgress.SetRange(0, PROGRESS_MAX_RANGE);
		m_ScanCleanProgress.SetStep(2);
		m_ScanCleanProgress.SetPos(0);
		m_ScanCleanProgress.ShowWindow(SW_HIDE);
		
		//m_hOwnTimerThread = ::CreateThread(NULL, 0, &CMainDlg::OwnTimerThreadProc, this, 0, NULL);
		SetTimer(TIMER_ID_SCAN, 100, NULL);

		//m_btnClean.ShowWindow(SW_HIDE);
		m_btnHistory.ShowWindow(SW_HIDE);
		m_btnStartScan.ShowWindow(SW_HIDE);
		m_btnStopScan.SetTitle(IDS_STOP);
		m_btnStopScan.ShowWindow(SW_SHOW);

		::EnableWindow(m_chkSelectAll, FALSE);
		::EnableWindow(m_txtSelAll, FALSE);
		::EnableWindow(m_btnDefault, FALSE);
		::EnableWindow(m_btnClean, FALSE);

		m_tree.EnableCheck(FALSE);

		return;
	}

	if ( status == CMainDlg::SCS_SCANNED )
	{
		CString		sText;
		CRect		rc;

		// 销毁滚动条定时器
		/*if ( m_hOwnTimerThread != NULL )
		{
			::TerminateThread(m_hOwnTimerThread, 0);
			::CloseHandle(m_hOwnTimerThread);
			m_hOwnTimerThread = NULL;
		}*/
		KillTimer(TIMER_ID_SCAN);

		m_ScanCleanProgress.ShowWindow(SW_HIDE);
		m_bmpLoading.ShowWindow(SW_HIDE);

		if ( m_nRegistryCount > 0 )
		{
			m_bmpTips.mnIndex = 0;
			sText.Format(L"扫描已完成，共发现%d个冗余注册表项，建议立即清理！", m_nRegistryCount);
			m_bTxtDescPaintClr = true;

			::EnableWindow(m_btnClean, TRUE);
			::EnableWindow(m_btnStartScan, TRUE);
		}
		else 
		{
			m_bmpTips.mnIndex = 2;
			sText = TEXT("在您指定的选项下未发现可清理的注册表项。");

			::EnableWindow(m_btnClean, TRUE);
			::EnableWindow(m_btnStartScan, TRUE);
		}

		m_bmpTips.ShowWindow(SW_SHOW);
		
		::SetWindowText(m_txtDesc, sText);
		::ShowWindow(m_txtDesc, SW_SHOW);
		::GetWindowRect(m_txtDesc, rc);
		::SetWindowPos(m_txtDesc, NULL, 0, 0, m_rcMainClient.Width() - 50, rc.Height(), SWP_NOZORDER | SWP_NOMOVE);
		::InvalidateRect(m_txtDesc, NULL, FALSE);

		m_btnClean.ShowWindow(SW_SHOW);
		m_btnHistory.ShowWindow(SW_SHOW);
		m_btnStartScan.SetTitle(IDS_RESCAN);
		m_btnStartScan.ShowWindow(SW_SHOW);
		m_btnStopScan.ShowWindow(SW_HIDE);

		::EnableWindow(m_chkSelectAll, TRUE);
		::EnableWindow(m_txtSelAll, TRUE);
		::EnableWindow(m_btnDefault, TRUE);

		m_tree.EnableCheck(TRUE);
		return;
	}

	if ( status == CMainDlg::SCS_CLEAN )
	{
		CRect		rc;

		m_bmpTips.ShowWindow(SW_HIDE);
		m_bmpLoading.ShowWindow(SW_SHOW);

		m_bTxtDescPaintClr = false;
		m_strTxtDesc = TEXT("正在清理...");
		::SetWindowText(m_txtDesc, TEXT("正在清理..."));
		::ShowWindow(m_txtDesc, SW_SHOW);
		::GetWindowRect(m_txtDesc, rc);
		::SetWindowPos(m_txtDesc, NULL, 0, 0, m_rcMainClient.Width() - 50, rc.Height(), SWP_NOZORDER | SWP_NOMOVE);
		::InvalidateRect(m_txtDesc, NULL, FALSE);

		m_ScanCleanProgress.SetRange(0, PROGRESS_MAX_RANGE);
		m_ScanCleanProgress.SetPos(0);
		m_ScanCleanProgress.SetStep(1);
		m_ScanCleanProgress.ShowWindow(SW_HIDE);
		SetTimer(TIMER_ID_CLEAN, 200, NULL);

		//m_btnClean.ShowWindow(SW_HIDE);
		m_btnStartScan.ShowWindow(SW_HIDE);
		m_btnHistory.ShowWindow(SW_HIDE);
		m_btnStopScan.SetTitle(IDS_STOPCLEAN);
		m_btnStopScan.ShowWindow(SW_SHOW);

		::EnableWindow(m_chkSelectAll, FALSE);
		::EnableWindow(m_txtSelAll, FALSE);
		::EnableWindow(m_btnDefault, FALSE);
		::EnableWindow(m_btnClean, FALSE);

		m_tree.EnableCheck(FALSE);

		return;
	}

	if ( status == CMainDlg::SCS_CLEANED )
	{
		CRect		rc;

		// 销毁滚动条定时器
		m_ScanCleanProgress.ShowWindow(SW_HIDE);
		KillTimer(TIMER_ID_CLEAN);

		m_bmpTips.mnIndex = 2;
		m_bmpTips.ShowWindow(SW_SHOW);
		m_bmpLoading.ShowWindow(SW_HIDE);
		
		if(m_rdScanner.m_bHaveUndelReg)
			::SetWindowText(m_txtDesc, TEXT("注册表清理已完成，部分清除失败。建议7天之内再次进行清理!"));
		else
			::SetWindowText(m_txtDesc, TEXT("注册表清理已完成。建议7天之内再次进行清理!"));
		::ShowWindow(m_txtDesc, SW_SHOW);
		::GetWindowRect(m_txtDesc, rc);
		::SetWindowPos(m_txtDesc, NULL, 0, 0, m_rcMainClient.Width() - 50, rc.Height(), SWP_NOZORDER | SWP_NOMOVE);
		::InvalidateRect(m_txtDesc, NULL, FALSE);
		
		m_btnClean.ShowWindow(SW_SHOW);
		m_btnHistory.ShowWindow(SW_SHOW);
		m_btnStartScan.ShowWindow(SW_SHOW);
		m_btnStopScan.ShowWindow(SW_HIDE);
		
		::EnableWindow(m_chkSelectAll, TRUE);
		::EnableWindow(m_txtSelAll, TRUE);
		::EnableWindow(m_btnDefault, TRUE);
		::EnableWindow(m_btnClean, TRUE);

		m_tree.EnableCheck(TRUE);
		return;
	}
}


LRESULT CMainDlg::OnScanClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
{
	m_bStop = false;
// 	if (IsX64System())
// 	{
// 		::PostMessage(*this, WM_OWN_MSGBOX, (WPARAM)TEXT("注册表清理还不支持64位操作系统！"), 
// 			(LPARAM)NULL);
// 		goto Label_Ret;	
// 	}

	CheckTreeSelectStatus();
	if ( !m_bSelectedRoot )
	{
		::PostMessage(*this, WM_OWN_MSGBOX, (WPARAM)TEXT("您尚未选择需要扫描的项目，请先选择，然后进行扫描！"), 
			(LPARAM)NULL);
		goto Label_Ret;	
	}
	StartScan();

Label_Ret:
	bHandled = TRUE;
	return 0;
}


LRESULT CMainDlg::OnStopClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
{
	m_bStop = true;
	m_rdScanner.Stop();

	bHandled = TRUE;
	return 0;
}

LRESULT CMainDlg::OnBackupClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
{
	CRegBackupDlg regDlg;
	regDlg.DoModal(m_hWnd);
	return 0;
}

LRESULT CMainDlg::OnCleanClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
{
	m_bStop = false;

	CheckTreeSelectStatus();
	if ( !m_bSelectedRoot && !m_bSelectedChild )
	{
		// 未选择项
		::PostMessage(*this, WM_OWN_MSGBOX, (WPARAM)TEXT("您尚未选择需要清理的项目，请先选择，然后再清理！"), (LPARAM)NULL);
		goto Label_Ret;
	}
	else if ( m_bSelectedUnScaned && !m_bSelectedScanned && !m_bSelectedUnCleaned )
	{
		// 未扫描项+已清理项
		::PostMessage(*this, WM_OWN_MSGBOX, (WPARAM)TEXT("您选择的项目中存在未扫描的项目，建议您先扫描，再进行清理！"), (LPARAM)NULL);
		goto Label_Ret;
	}
	else if ( !m_bSelectedScanned && !m_bSelectedUnCleaned && m_bSelectedCleaned )
	{
		// 只有已清理项
		::PostMessage(*this, WM_OWN_MSGBOX, (WPARAM)TEXT("您选中的项目已全部清理完成，请选择其他项目，然后重新扫描！"), (LPARAM)NULL);
		goto Label_Ret;
	}
	else if (!m_bSelectedChild)
	{
		::PostMessage(*this, WM_OWN_MSGBOX, (WPARAM)TEXT("在您指定的选项下未发现可清理的注册表项！"), (LPARAM)NULL);
		goto Label_Ret;
	}


	if ( m_hCleanThread != NULL )
	{
		::CloseHandle(m_hCleanThread);
		m_hCleanThread = NULL;
	}
	
	m_iterLastItem = m_TreeRootMap.end();

	// 读取被选中清除的项目
	EnumSelectedTreeItem();

	// 设置为清除中状态
	SetScStatus(CMainDlg::SCS_CLEAN);

	// 创建清除线程
	m_hCleanThread = ::CreateThread(NULL, 0, &CMainDlg::CleanThreadProc, this, 0, NULL);
	if ( m_hCleanThread == NULL )
	{
		::MessageBox(m_hWnd, TEXT("初始化时发生错误，请点击清除按钮重试。"), TEXT("内部错误!"), 
			MB_OK | MB_ICONINFORMATION);

		SetScStatus(CMainDlg::SCS_IDLE);
	}
	else
	{
		m_dwCleanTickCount = ::GetTickCount();
	}

Label_Ret:
	bHandled = TRUE;
	return 0;
}


LRESULT CMainDlg::OnSelectAllClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
{
	LRESULT					lCheck = 0;
	HTREEITEM				hTreeItem = NULL;
	TREE_ROOT_MAP::iterator	iter;

	lCheck = ::SendMessage(m_chkSelectAll, BM_GETCHECK, 0, 0);
	m_bSelectMap.clear();

	if ( lCheck == BST_CHECKED )
	{
        ::SendMessage(m_chkSelectAll, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		// 全选
		for ( iter = m_TreeRootMap.begin(); iter != m_TreeRootMap.end(); ++iter )
		{
			m_tree.SetCheck(iter->second, KUIMulStatusTree::EM_TVIS_CHECK);
			m_bSelectMap[iter->first] = 1;
		}
	}
	else if ( lCheck == BST_UNCHECKED )
	{
		// 全不选
        ::SendMessage(m_chkSelectAll, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
		for ( iter = m_TreeRootMap.begin(); iter != m_TreeRootMap.end(); ++iter )
		{
			m_tree.SetCheck(iter->second, KUIMulStatusTree::EM_TVIS_UNCHECK);
			m_bSelectMap[iter->first] = 0;
		}
	}
	else
	{
		// 若用户点击选中第三状态，设置为全不选
		::SendMessage(m_chkSelectAll, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
		
		// 全不选
		for ( iter = m_TreeRootMap.begin(); iter != m_TreeRootMap.end(); ++iter )
		{
			m_tree.SetCheck(iter->second, KUIMulStatusTree::EM_TVIS_UNCHECK);
		}
	}

	bHandled = FALSE;
	return 0;
}


LRESULT CMainDlg::OnSelectDefaultClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
{
	SelectDefaultItems();
	bHandled = TRUE;
	::SendMessage(m_chkSelectAll, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	return 0;
}


LRESULT CMainDlg::OnScanCallback(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CRegistryData::SCAN_RESULT*		result = NULL;
	int								nIndex = -1;
	CRegistryData::REG_SCAN_TYPE	rstType;
	tstring							lpTitle, lpProblem;
	CRegistryData::REG_DATA			lpData;
	tstring							strKeyPath;
	LPCTSTR							lpRootKey = TEXT("HKLM");

	bHandled = TRUE;

	// 获取扫描到的数据
	result = (CRegistryData::SCAN_RESULT*)lParam;
	if ( result == NULL )
		return 0;

	result->GetInfo(rstType, lpData, lpProblem);
	lpTitle = CRegistryData::GetTitle(rstType);
	lpData.GetKeyFullPath(strKeyPath, true);
	if ( strKeyPath.size() > 1 && strKeyPath[strKeyPath.size() - 1] == TEXT('\\') )
		strKeyPath.erase(strKeyPath.size() - 1);
	
	// 添加到Tree中
	TREE_ROOT_MAP::iterator		iter;
	HTREEITEM					hTreeItem = NULL;

	// 检查类别是否有效
	iter = m_TreeRootMap.find(rstType);
	if ( iter == m_TreeRootMap.end() )
		return 0;

	// 添加项
	hTreeItem = m_tree.InsertItem(strKeyPath.c_str(), 22, 22, 
		iter->second, NULL, KUIMulStatusTree::EM_TVIS_CHECK);
	m_tree.SetItemData(hTreeItem, (DWORD_PTR)(ULONG_PTR)result);

	// 更新计数
	m_nRegistryCount++;

	// 更新文本提示
	lpTitle = TEXT("正在扫描：");
	lpTitle += strKeyPath;
	if ( lpTitle.size() > 80 )
	{
		lpTitle.erase(40, lpTitle.size() - 83);
		lpTitle.insert(40, TEXT("..."));
	}
	m_strTxtDesc = lpTitle;

	return 0;
}


LRESULT CMainDlg::OnScanTypeCallback(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	tstring						strTitle;
	TREE_ROOT_MAP::iterator		iter;
	int							nNum = 0;
	std::vector<TCHAR>			buffer(30, 0);

	iter = m_SelectedRootMap.find((CRegistryData::REG_SCAN_TYPE)wParam);
	if ( iter == m_SelectedRootMap.end() )
		goto Label_Ret;

	// 扫描到第一个子节点时，添加正在扫描中的状态，并恢复前一节点的状态
	strTitle = CRegistryData::GetTitle(iter->first);
	strTitle += TEXT(" (正在扫描...)");
	m_tree.SetItemText(iter->second, strTitle.c_str());
	m_tree.SelectItem(iter->second);

	// 切换图标
	m_tree.SetItemImage(iter->second, 23, 23);

	// 保存节点
	m_iterLastItem = m_TreeRootMap.find(iter->first);

	if ( --iter != m_SelectedRootMap.end() )
	{
		// 添加上个类别的数据
		AddTypeScanData(iter->first);

		// 统计上个类别的数据
		CalcChildNum(iter->second, nNum);
		::_itot_s(nNum, &buffer[0], buffer.size(), 10);

		strTitle = CRegistryData::GetTitle(iter->first);
		strTitle += TEXT(" (共扫描到 ");
		strTitle += &buffer[0];
		strTitle += TEXT(" 项)");
		m_tree.SetItemText(iter->second, strTitle.c_str());
		m_tree.SetItemImage(iter->second, GetTypeIconIndex(iter->first), 
			GetTypeIconIndex(iter->first));
	}

Label_Ret:
	bHandled = TRUE;
	return 0;
}


LRESULT CMainDlg::OnScanEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	// 设置Tree项目的状态
	SetScannedTreeItem();
	SetScStatus(CMainDlg::SCS_SCANNED);

	bHandled = TRUE;
	return 0;
}

LRESULT CMainDlg::OnCleanTypeCallback(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	int							nCheck = KUIMulStatusTree::EM_TVIS_CHECK;
	tstring						strTitle;
	TREE_ROOT_MAP::iterator		iter, iterLast;
	int							nChildItem = 0;

	// 检查是否为选中的类别
	iter = m_SelectedRootMap.find((CRegistryData::REG_SCAN_TYPE)wParam);
	if ( iter == m_SelectedRootMap.end() )
		goto Label_Ret;
	
	// 设置描述条
	strTitle = TEXT("正在清除：");
	strTitle += CRegistryData::GetTitle(iter->first);
	strTitle += TEXT("...");
	m_strTxtDesc = strTitle;

	// 设置当前正被清除的类别的状态
	strTitle = CRegistryData::GetTitle(iter->first);
	strTitle += TEXT(" (正在清除...)");
	m_tree.SetItemText(iter->second, strTitle.c_str());
	m_tree.SetItemImage(iter->second, 23, 23);
	m_tree.SelectItem(iter->second);
	DeleteSelectedChild(iter->second);

	// 若当前根元素为半选中状态，设置为未选中状态（已选中的元素被删除了）
	m_tree.GetCheck(iter->second, nCheck);
	if ( nCheck == KUIMulStatusTree::EM_TVIS_INDETERMINING )
		m_tree.SetCheck(iter->second, KUIMulStatusTree::EM_TVIS_UNCHECK);

	// 记录当前类别
	iterLast = m_iterLastItem;
	m_iterLastItem = m_TreeRootMap.find(iter->first);

	// 恢复前一个被清除的类别的状态
	if ( iterLast != m_TreeRootMap.end() )
	{
		nChildItem = 0;
		CalcChildNum(iterLast->second, nChildItem);

		strTitle =  CRegistryData::GetTitle(iterLast->first);
		if ( nChildItem <= 0 )
			strTitle += TEXT(" (已清除)");
		else
			strTitle += TEXT(" (部分项目已清除)");
		m_tree.SetItemText(iterLast->second, strTitle.c_str());
		m_tree.SetItemImage(iterLast->second, GetTypeIconIndex(iterLast->first), GetTypeIconIndex(iterLast->first));

		// 清除被选中的子节点
	}

Label_Ret:
	bHandled = TRUE;
	return 0;
}


LRESULT CMainDlg::OnCleanEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	// 清空被选择清除的子项
	SetCleanedTreeItem();

	// 重设控件状态
	SetScStatus(CMainDlg::SCS_CLEANED);

	// 检查树型控件选择状态，设置按钮状态
	CheckTreeSelectStatus();
	
	bHandled = TRUE;
	return 0;
}


LRESULT CMainDlg::OnOwnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	OnScanCleanTimer(TIMER_ID_SCAN);

	bHandled = TRUE;
	return 0;
}


LRESULT CMainDlg::OnOwnMsgBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CKuiMsgBox::Show((LPCTSTR)wParam, (LPCTSTR)lParam, MB_OK | MB_ICONINFORMATION, NULL, *this);
	bHandled = TRUE;
	return 0;
}


// 当用户钩选了checkbox时，调用此函数
LRESULT CMainDlg::OnTreeItemCheckChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	HTREEITEM					hTreeItem = (HTREEITEM)wParam;
	int							nCheck = (int)lParam;
	CRegistryData::SCAN_RESULT*	lpResult = NULL;

	CheckTreeSelectStatus();

    if(nCheck!=3)
    ::SendMessage(m_chkSelectAll, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
   
	
	// 检查是否为子节点的check
	bHandled = TRUE;
	if ( nCheck != KUIMulStatusTree::EM_TVIS_UNCHECK && nCheck != KUIMulStatusTree::EM_TVIS_CHECK )
		return 0;

	// 设置是否清除
	lpResult = (CRegistryData::SCAN_RESULT*)m_tree.GetItemData(hTreeItem);
	if ( lpResult == NULL || (ULONG_PTR)lpResult == (ULONG_PTR)-1 )
		return 0;

	lpResult->SetToClean( (nCheck == KUIMulStatusTree::EM_TVIS_CHECK) ? true : false );
	return 0;
}


// 响应TVN_GETINFOTIPS消息，设置ToolTip显示的文本
LRESULT CMainDlg::OnTreeItemInfoTip(int idCtrl, LPNMHDR pnmh, BOOL &bHandled)
{
	LPNMTVGETINFOTIP				lpGetInfoTip = (LPNMTVGETINFOTIP)pnmh;
	CRegistryData::SCAN_RESULT*		lpResult = NULL;
	HTREEITEM						hTreeItem = NULL;
	tstring							strProblem;

	// 取节点关联的数据
	hTreeItem = lpGetInfoTip->hItem;
	lpResult = (CRegistryData::SCAN_RESULT*)(ULONG_PTR)m_tree.GetItemData(hTreeItem);
	if ( lpResult == NULL || (ULONG_PTR)lpResult == (ULONG_PTR)-1 )
		return 0;

	// 缓冲区一般有INFOTIPSIZE
	lpResult->GetProblem(strProblem);
	if ( lpGetInfoTip->cchTextMax > 1 && strProblem.size() > 0 )
	{
		::memset(lpGetInfoTip->pszText, 0, lpGetInfoTip->cchTextMax * sizeof(TCHAR));
		::_tcsncpy(lpGetInfoTip->pszText, strProblem.c_str(), lpGetInfoTip->cchTextMax - 1);
	}

	return 0;
}


HRESULT CMainDlg::StartScan()
{
	// 释放前面的扫描线程
	if ( m_hScanThread != NULL )
	{
		::CloseHandle(m_hScanThread);
		m_hScanThread = NULL;
	}

	// 清除前一次的扫描结果
	ClearAllTreeItem();
	m_iterLastItem = m_TreeRootMap.end();
	{
		KAutoLock		lockTree(m_lockTree);
		m_scanResultArray.clear();
	}

	// 计数清零
	m_nRegistryCount = 0;

	// 读取被选择的项目列表
	EnumSelectedTreeItem();

	// 设置扫描状态
	SetScStatus(CMainDlg::SCS_SCAN);

	// 创建扫描线程
	m_hScanThread = ::CreateThread(NULL, 0, &CMainDlg::ScanThreadProc, this, 0, NULL);
	if ( m_hScanThread == NULL )
	{
		::MessageBox(m_hWnd, TEXT("初始化扫描时发生错误，请点击扫描按钮重试。"), TEXT("内部错误!"), 
			MB_OK | MB_ICONINFORMATION);
		
		SetScStatus(CMainDlg::SCS_IDLE);
		return E_FAIL;
	}

	return S_OK;
}


DWORD WINAPI CMainDlg::ScanThreadProc(LPVOID lpParameter)
{
	HRESULT		hr = E_FAIL;
	DWORD		dwTick = 0;
	CMainDlg*	lpThis = static_cast<CMainDlg*>(lpParameter);

	if ( lpThis == NULL )
		return -1;

	// 检查距上次清理的间隔.若很短，不再扫描出内容
	/*dwTick = GetTickCount();
	if ( lpThis->m_dwCleanTickCount <= dwTick && dwTick - lpThis->m_dwCleanTickCount < 60000  )
	{
		::Sleep(3000);
		lpThis->SetScStatus(CMainDlg::SCS_SCANNED);
		return 0;
	}
	else if ( lpThis->m_dwCleanTickCount > dwTick && dwTick < 60000 )
	{
		::Sleep(3000);
		lpThis->SetScStatus(CMainDlg::SCS_SCANNED);
		return 0;
	}*/

	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

	// 扫描
	hr = lpThis->m_rdScanner.Scan(&CMainDlg::ScanTypeCallBack, lpThis, &CMainDlg::ScanCallback, lpThis);
	
	// 扫描完成
	::PostMessage(*lpThis, WM_SCAN_END, 0, 0);
	return 0;
}


DWORD WINAPI CMainDlg::CleanThreadProc(LPVOID lpParameter)
{
	HRESULT		hr = E_FAIL;
	CMainDlg*	lpThis = static_cast<CMainDlg*>(lpParameter);

	if ( lpThis == NULL )
		return -1;

	// 清理
	hr = lpThis->m_rdScanner.Clean(&CMainDlg::CleanTypeCallBack, lpThis);

	// 清理完毕
	::PostMessage(*lpThis, WM_CLEAN_END, 0, 0);
	return 0;
}


DWORD WINAPI CMainDlg::OwnTimerThreadProc(LPVOID lpParameter)
{
	HRESULT		hr = E_FAIL;
	CMainDlg*	lpThis = static_cast<CMainDlg*>(lpParameter);

	if ( lpThis == NULL )
		return -1;

	while (true)
	{
		::Sleep(300);
		::PostMessage(*lpThis, WM_OWN_TIMER, 0, 0);
	}

	return 0;
}


// 扫描回调函数
bool CMainDlg::ScanCallback(CRegistryData::SCAN_RESULT* result, LPVOID lpParam)
{
	CMainDlg*		lpThis = static_cast<CMainDlg*>(lpParam);
	CRegistryData::REG_SCAN_TYPE	rstType;
	tstring			lpTitle, lpProblem, strKeyPath;
	CRegistryData::REG_DATA	lpData = {};

	if ( lpThis == NULL || result == NULL )
		return false;

	// 通知窗口扫描到新项目
	//::PostMessage(lpThis->m_hWnd, WM_SCAN_CALLBACK, 0, (LPARAM)result);
	//return true;

	// 添加到内部存储空间中
	{
		KAutoLock	lockTree(lpThis->m_lockTree);
		lpThis->m_scanResultArray[result->rstType].push_back(result);
	}

	result->GetInfo(rstType, lpData, lpProblem);
	lpTitle = CRegistryData::GetTitle(rstType);
	lpData.GetKeyFullPath(strKeyPath, true);
	if ( strKeyPath.size() > 1 && strKeyPath[strKeyPath.size() - 1] == TEXT('\\') )
		strKeyPath.erase(strKeyPath.size() - 1);

	lpTitle = TEXT("正在扫描：");
	lpTitle += strKeyPath;
	if ( lpTitle.size() > 80 )
	{
		lpTitle.erase(40, lpTitle.size() - 83);
		lpTitle.insert(40, TEXT("..."));
	}

	// 更新文本提示
	{
		KAutoLock	lockTips(lpThis->m_lockTips);
		lpThis->m_strTxtDesc = lpTitle;
	}

	return true;
}


bool CMainDlg::ScanTypeCallBack(CRegistryData::REG_SCAN_TYPE rstType, LPVOID lpParam)
{
	CMainDlg*				lpThis = static_cast<CMainDlg*>(lpParam);
	TREE_ROOT_MAP::iterator	iter;

	if ( lpThis == NULL )
		return false;

	// 若不在列表中，跳过此类型的扫描
	iter = lpThis->m_SelectedRootMap.find(rstType);
	if ( iter == lpThis->m_SelectedRootMap.end() )
		return false;

	// 建立此类别的数据
	{
		KAutoLock		lockTree(lpThis->m_lockTree);
		VSR				v;
		MTRP::iterator	iterArray;

		iterArray = lpThis->m_scanResultArray.find(rstType);
		if ( iterArray == lpThis->m_scanResultArray.end() )
			lpThis->m_scanResultArray.insert(std::make_pair(rstType, v));
	}
	
	// 通知窗口，此类型的扫描开始了
	::PostMessage(lpThis->m_hWnd, WM_SCANTYPE_CALLBACK, (WPARAM)rstType, 0);
	return true;
}


bool CMainDlg::CleanTypeCallBack(CRegistryData::REG_SCAN_TYPE rstType, LPVOID lpParam)
{
	CMainDlg*		lpThis = static_cast<CMainDlg*>(lpParam);

	if ( lpThis == NULL )
		return true;

	// 若不在列表中，返回
	if ( lpThis->m_SelectedRootMap.find(rstType) == lpThis->m_SelectedRootMap.end() )
		return true;

	// 通知窗口，此类型的清除开始了
	::PostMessage(*lpThis, WM_CLEANTYPE_CALLBACK, (WPARAM)rstType, 0);
	return true;
}


void CMainDlg::SetScannedTreeItem()
{
	CMainDlg::TREE_ROOT_MAP::iterator	iter;
	int									nCheck = KUIMulStatusTree::EM_TVIS_UNCHECK;
	tstring								strText;

	// 设置被扫描项的状态
	for ( iter = m_SelectedRootMap.begin(); iter != m_SelectedRootMap.end(); ++iter )
	{
		// 收起子项
		m_tree.Expand(iter->second, TVE_COLLAPSE);

		// 若选中项未被扫描（扫描中止），显示未扫描
		if ( m_iterLastItem == m_TreeRootMap.end() || m_iterLastItem->first < iter->first )
		{
			strText =  CRegistryData::GetTitle(iter->first);
			strText += TEXT(" (未扫描)");
			m_tree.SetItemText(iter->second, strText.c_str());
		}
		// 若被选中且无子项，设置只有0项
		else if ( m_tree.GetChildItem(iter->second) == NULL )
		{
			strText =  CRegistryData::GetTitle(iter->first);
			strText += TEXT(" (共扫描到 0 项)");
			m_tree.SetItemText(iter->second, strText.c_str());
		}

		// 换回图标
		m_tree.SetItemImage(iter->second, GetTypeIconIndex(iter->first), GetTypeIconIndex(iter->first));
	}

	// 恢复最后扫描项的文本
	if ( m_iterLastItem != m_TreeRootMap.end() )
	{
		int					nNum = 0;
		std::vector<TCHAR>	buffer(30, 0);
		tstring				lpTitle;

		AddTypeScanData(m_iterLastItem->first);

		CalcChildNum(m_iterLastItem->second, nNum);
		::_itot_s(nNum, &buffer[0], buffer.size(), 10);

		lpTitle = CRegistryData::GetTitle(m_iterLastItem->first);
		lpTitle += TEXT(" (共扫描到 ");
		lpTitle += &buffer[0];
		lpTitle += TEXT(" 项)");
		m_tree.SetItemText(m_iterLastItem->second, lpTitle.c_str());
	}

	// 选中第一项
	iter = m_TreeRootMap.begin();
	if ( iter != m_TreeRootMap.end() )
		m_tree.SelectItem(iter->second);
}


HRESULT CMainDlg::SetCleanedTreeItem()
{
	tstring				strTitle;
	HTREEITEM			hTreeItem = NULL;
	int					nChildItem = 0;
	CMainDlg::TREE_ROOT_MAP::iterator	iter;
	std::vector<TCHAR>	buffer(100, 0);

	// 清空Tree中初扫描到的内容，仅保留类型节点, 恢复初始文本
	for( iter = m_SelectedRootMap.begin(); iter != m_SelectedRootMap.end(); ++iter )
	{
		// 若中途停止，扫描项后的项不作处理
		if ( m_bStop && m_iterLastItem != m_TreeRootMap.end() && iter->first > m_iterLastItem->first )
			break;

		// 跳过未扫描项或已清除项
		::memset(&buffer[0], 0, buffer.size() * sizeof(TCHAR));
		m_tree.GetItemText(iter->second, &buffer[0], (int)buffer.size());
		if ( ::_tcsstr(&buffer[0], TEXT("共扫描到")) == NULL )
			continue;

		// 设置项目已清除
		nChildItem = 0;
		CalcChildNum(iter->second, nChildItem);

		strTitle =  CRegistryData::GetTitle(iter->first);
		if ( nChildItem <= 0 )
			strTitle += TEXT(" (已清除)");
		else
			strTitle += TEXT(" (部分项目已清除)");
		m_tree.SetItemText(iter->second, strTitle.c_str());
		m_tree.SetItemImage(iter->second, GetTypeIconIndex(iter->first), GetTypeIconIndex(iter->first));
	}

	// 恢复最后扫描项的文本
	if ( m_iterLastItem != m_TreeRootMap.end() )
	{
		nChildItem = 0;
		CalcChildNum(m_iterLastItem->second, nChildItem);

		strTitle = CRegistryData::GetTitle(m_iterLastItem->first);
		if ( nChildItem <= 0 )
			strTitle += TEXT(" (已清除)");
		else
			strTitle += TEXT(" (部分项目已清除)");
		m_tree.SetItemText(m_iterLastItem->second, strTitle.c_str());
		m_tree.SetItemImage(m_iterLastItem->second, GetTypeIconIndex(m_iterLastItem->first),
			GetTypeIconIndex(m_iterLastItem->first));
	}

	// 选中第一项
	iter = m_TreeRootMap.begin();
	if ( iter != m_TreeRootMap.end() )
		m_tree.SelectItem(iter->second);

	return S_OK;
}


void CMainDlg::SelectDefaultItems()
{
	TREE_ROOT_MAP::iterator	iter;
	m_bSelectMap.clear();

	for ( iter = m_TreeRootMap.begin(); iter != m_TreeRootMap.end(); ++iter )
	{
		if (iter->first <= CRegistryData::RST_APPPATH)
		{
			m_tree.SetCheck(iter->second, KUIMulStatusTree::EM_TVIS_CHECK);
			m_bSelectMap[iter->first] = 1;
		}
		else
		{
			m_tree.SetCheck(iter->second, KUIMulStatusTree::EM_TVIS_UNCHECK);
			m_bSelectMap[iter->first] = 0;
		}
	}
}


// 计算节点的子节点数目
bool CMainDlg::CalcChildNum(HTREEITEM hParent, int& nNum)
{
    HTREEITEM   hChild = NULL;

    nNum = 0;
    hChild = m_tree.GetChildItem(hParent);
    while ( hChild != NULL )
    {
		++nNum;
        hChild = m_tree.GetNextSiblingItem(hChild);
    }

	return true;
}

// 计算节点的被选中的子节点数目
bool CMainDlg::CalcSelectedChildNum(HTREEITEM hParent, int& nNum)
{
	HTREEITEM		hChild = NULL;
	int				nCheck = KUIMulStatusTree::EM_TVIS_UNCHECK;

	nNum = 0;
	hChild = m_tree.GetChildItem(hParent);
	while ( hChild != NULL )
	{
		// ?觳槭欠癖谎≈?
		m_tree.GetCheck(hChild, nCheck);
		if ( nCheck == KUIMulStatusTree::EM_TVIS_CHECK )
			++nNum;

		hChild = m_tree.GetNextSiblingItem(hChild);
	}

	return true;
}


// 删除节点的所有子节点
bool CMainDlg::DeleteAllChild(HTREEITEM hParent)
{
	bool			bReturn = false;
	HTREEITEM		hChild = NULL;
	HTREEITEM		hItem = NULL;

	bReturn = true;
	hChild = m_tree.GetChildItem(hParent);
	while ( hChild != NULL )
	{
		hItem = hChild;
		hChild = m_tree.GetNextSiblingItem(hChild);
		if ( !m_tree.DeleteItem(hItem) )
		{
			bReturn = false;
			goto Label_Ret;
		}
	}

Label_Ret:
	return bReturn;
}


// 删除所有被选中的子节点
bool CMainDlg::DeleteSelectedChild(HTREEITEM hParent)
{
	bool			bReturn = false;
	HTREEITEM		hChild = NULL;
	HTREEITEM		hItem = NULL;
	int				nCheck = KUIMulStatusTree::EM_TVIS_UNCHECK;

	bReturn = true;
	hChild = m_tree.GetChildItem(hParent);
	while ( hChild != NULL )
	{
		hItem = hChild;
		hChild = m_tree.GetNextSiblingItem(hChild);
		
		// 检查是否被选中
		m_tree.GetCheck(hItem, nCheck);
		if ( nCheck != KUIMulStatusTree::EM_TVIS_CHECK )
			continue;

		// 若选中，删除节点
		if ( !m_tree.DeleteItem(hItem) )
		{
			bReturn = false;
			goto Label_Ret;
		}
	}

Label_Ret:
	return bReturn;
}


// 读取被选择的项目，保存到列表中
void CMainDlg::EnumSelectedTreeItem()
{
	int			nCheck = KUIMulStatusTree::EM_TVIS_UNCHECK;
	CMainDlg::TREE_ROOT_MAP::iterator	iter;

	m_SelectedRootMap.clear();
	for ( iter = m_TreeRootMap.begin(); iter != m_TreeRootMap.end(); ++iter )
	{
		if ( !m_tree.GetCheck(iter->second, nCheck) )
			continue;

		if ( nCheck != KUIMulStatusTree::EM_TVIS_CHECK && nCheck != KUIMulStatusTree::EM_TVIS_INDETERMINING )
			continue;

		m_SelectedRootMap.insert(std::make_pair(iter->first, iter->second));
	}
}


// 清空Tree中的内容，仅保留类型节点, 恢复初始文本
void CMainDlg::ClearAllTreeItem()
{
	HTREEITEM		hTreeItem = NULL;
	CMainDlg::TREE_ROOT_MAP::iterator	iter;

	for( iter = m_TreeRootMap.begin(); iter != m_TreeRootMap.end(); ++iter )
	{
		DeleteAllChild(iter->second);
		m_tree.SetItemText(iter->second, CRegistryData::GetTitle(iter->first));
	}

}


// 检查tree中项目的选中情况，设置各窗口控件的状态(扫描、清除、全选)
void CMainDlg::CheckTreeSelectStatus()
{
	bool					bRootAllSelected = true;
	int						nSelectRootElem = 0;
	int						nSelectedChildElem = 0, nTotalSelectedChildElem = 0;
	int						nCheck = KUIMulStatusTree::EM_TVIS_UNCHECK;
	std::vector<TCHAR>		buffer(100,0);
	CMainDlg::TREE_ROOT_MAP::iterator	iter;

	// 初始化状态
	m_bSelectedRoot = false;
	m_bSelectedChild = false;
	m_bSelectedUnScaned = false;
	m_bSelectedScanned = false;
	m_bSelectedCleaned = false;
	m_bSelectedUnCleaned = false;

	nTotalSelectedChildElem = 0;
	m_bSelectMap.clear();
	int nIndex = 0;
	for ( iter = m_TreeRootMap.begin(); iter != m_TreeRootMap.end(); ++iter)
	{
		// 检查根元素的选中状态
		if ( !m_tree.GetCheck(iter->second, nCheck) )
		{
			
			continue;
		}
		if ( nCheck == KUIMulStatusTree::EM_TVIS_CHECK || nCheck == KUIMulStatusTree::EM_TVIS_INDETERMINING )
		{
			++nSelectRootElem;
			m_bSelectMap[iter->first] = 1;

			nSelectedChildElem = 0;
			CalcSelectedChildNum(iter->second, nSelectedChildElem);
			nTotalSelectedChildElem += nSelectedChildElem;

			// 检查文本
			::memset(&buffer[0], 0, buffer.size() * sizeof(TCHAR));
			m_tree.GetItemText(iter->second, &buffer[0], (int)buffer.size());
			if ( ::_tcsstr(&buffer[0], TEXT("共扫描到")) != NULL )
			{
				m_bSelectedScanned = true;
				m_bSelectedUnCleaned = true;
			}
			else if ( ::_tcsstr(&buffer[0], TEXT("已清")) != NULL )
			{
				if ( nSelectedChildElem <= 0 )
					m_bSelectedCleaned = true;
				else
					m_bSelectedUnCleaned = true;
			}
			else if ( ::_tcsstr(&buffer[0], TEXT("未扫描")) != NULL )
			{
				m_bSelectedUnScaned = true;
			}
			else if ( ::_tcsstr(&buffer[0], TEXT("(")) == NULL )
			{
				m_bSelectedUnScaned = true;
			}
		}

		if(nCheck == KUIMulStatusTree::EM_TVIS_UNCHECK)
			m_bSelectMap[iter->first] = 0;

		if ( nCheck == KUIMulStatusTree::EM_TVIS_INDETERMINING )
			bRootAllSelected = false;
	}

	// 全选
    nCheck = (int)::SendMessage(m_chkSelectAll, BM_GETCHECK, 0, 0);
	if ( nSelectRootElem == m_TreeRootMap.size() && bRootAllSelected )
	{
		::SendMessage(m_chkSelectAll, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
	}
	else if ( nSelectRootElem == 0 )
	{
		::SendMessage(m_chkSelectAll, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	}
   

	// 扫描
	if ( m_scStatus == CMainDlg::SCS_IDLE || m_scStatus == CMainDlg::SCS_SCANNED || m_scStatus == CMainDlg::SCS_CLEANED )
	{	
		if ( nSelectRootElem == 0 )
		{
			m_bSelectedRoot = false;
			//m_btnStartScan.EnableWindow(FALSE);
		}
		else
		{
			m_bSelectedRoot = true;
			m_btnStartScan.EnableWindow(TRUE);
		}
	}
	
	// 清除
	if ( m_scStatus == CMainDlg::SCS_SCANNED || m_scStatus == CMainDlg::SCS_CLEANED )
	{
		if ( nTotalSelectedChildElem == 0 )
		{
			m_bSelectedChild = false;
			//m_btnClean.EnableWindow(FALSE);
		}
		else
		{
			m_bSelectedChild = true;
			m_btnClean.EnableWindow(TRUE);
		}
	}
}


int CMainDlg::GetTypeIconIndex(CRegistryData::REG_SCAN_TYPE rstType)
{
	int		nIndex = 0;

	switch (rstType)
	{
	case CRegistryData::RST_UNINSTALL:
		nIndex = 12;
		break;
	case CRegistryData::RST_SHAREDDLL:
		nIndex = 14;
		break;
	case CRegistryData::RST_OPENSAVEMRU:
		nIndex = 20;
		break;
	case CRegistryData::RST_FILEASSOC:
		nIndex = 18;
		break;
	case CRegistryData::RST_COM:
		nIndex = 16;
		break;
	case CRegistryData::RST_EXTHISTORY:
		nIndex = 20;
		break;
	case CRegistryData::RST_STARTMENU:
		nIndex = 21;
		break;
	case CRegistryData::RST_MUICACHE:
		nIndex = 17;
		break;
	case CRegistryData::RST_HELP:
		nIndex = 13;
		break;
	case CRegistryData::RST_APPPATH:
		nIndex = 19;
		break;
	default:
		nIndex = 6;
		break;
	}

	return nIndex;
}


void CMainDlg::AddTypeScanData(CRegistryData::REG_SCAN_TYPE rstType)
{
	CRegistryData::SCAN_RESULT*		result = NULL;
	int								nIndex = -1;
	tstring							lpTitle, lpProblem;
	CRegistryData::REG_DATA			lpData;
	tstring							strKeyPath;
	LPCTSTR							lpRootKey = TEXT("HKLM");
	TREE_ROOT_MAP::iterator			iterTree;
	HTREEITEM						hTreeItem = NULL;
	MTRP::iterator					iterMap;
	VSR::iterator					iterArray;
	KAutoLock						lockTree(m_lockTree);

	// 检查类别是否有效
	iterTree = m_TreeRootMap.find(rstType);
	if ( iterTree == m_TreeRootMap.end() )
		return;

	iterMap = m_scanResultArray.find(rstType);
	if ( iterMap == m_scanResultArray.end() )
		return;

	for ( iterArray = iterMap->second.begin(); iterArray != iterMap->second.end(); ++iterArray )
	{
		// 获取扫描到的数据
		result = (CRegistryData::SCAN_RESULT*)*iterArray;
		if ( result == NULL )
			return;

		result->GetInfo(rstType, lpData, lpProblem);
		lpTitle = CRegistryData::GetTitle(rstType);
		lpData.GetKeyFullPath(strKeyPath, true);
		if ( strKeyPath.size() > 1 && strKeyPath[strKeyPath.size() - 1] == TEXT('\\') )
			strKeyPath.erase(strKeyPath.size() - 1);

		// 添加到Tree中
		hTreeItem = m_tree.InsertItem(strKeyPath.c_str(), 22, 22, 
			iterTree->second, NULL, KUIMulStatusTree::EM_TVIS_CHECK);
		m_tree.SetItemData(hTreeItem, (DWORD_PTR)(ULONG_PTR)result);
	}

	// 更新计数
	m_nRegistryCount += (int)iterMap->second.size();
	iterMap->second.clear();

	return;
}


LRESULT CALLBACK CMainDlg::TxtDescWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc = NULL;
	HDC			hdcMem = NULL;
	HBITMAP		hbmp = NULL;
	CRect		rc;
	tstring		strText;
	int			nX = 0;
	CFont		fontText;
	CString		sText;
	HBRUSH		hbr = NULL;
	TEXTMETRIC	tm = {};
	SIZE		sz = {};

	// 跳过非PAINT消息
	if ( uMsg != WM_PAINT )
		return (*gs_lpThisDlg->m_lpTxtDescProc)(hwnd, uMsg, wParam, lParam);

	::GetClientRect(hwnd, rc);
	hdc = ::GetDC(hwnd);
	hdcMem = ::CreateCompatibleDC(hdc);
	hbmp = ::CreateCompatibleBitmap(hdc, rc.Width(), rc.Height());
	::SelectObject(hdcMem, hbmp);

	hbr = ::CreateSolidBrush(DIALOG_BK_COLOR);
	::FillRect(hdcMem, rc, hbr);
	::DeleteObject(hbr);

	fontText.CreatePointFont(90, TEXT("MS Shell Dlg"));
	::SelectObject(hdcMem, fontText);
	::GetTextMetrics(hdcMem, &tm);

	::SetBkMode(hdcMem, TRANSPARENT);
	::SetBkColor(hdcMem, DIALOG_BK_COLOR);

	// 绘制文本
	if ( !gs_lpThisDlg->m_bTxtDescPaintClr )
	{
		std::vector<TCHAR>		buf(500, 0);

		::GetWindowText(hwnd, &buf[0], (int)buf.size());
		::SetTextColor(hdcMem, RGB(0,0,0));
		::TextOut(hdcMem, 0, 0, &buf[0], (int)::_tcslen(&buf[0]));
	}
	else
	{
		nX = 0;
		strText = TEXT("扫描已完成，共发现  ");
		::SetTextColor(hdcMem, RGB(0,0,0));
		::TextOut(hdcMem, nX, 0, strText.c_str(), (int)strText.size());
		
		if ( GetTextExtentPoint32(hdcMem, strText.c_str(), (int)strText.size(), &sz) )
			nX += sz.cx - tm.tmOverhang;
		else
			nX += (int)(strText.size() * 12);
		sText.Format(TEXT("%d"), gs_lpThisDlg->m_nRegistryCount);
		strText = sText;
		::SetTextColor(hdcMem, RGB(255,0,0));
		::TextOut(hdcMem, nX, 0, strText.c_str(), (int)strText.size());

		if ( GetTextExtentPoint32(hdcMem, strText.c_str(), (int)strText.size(), &sz) )
			nX += sz.cx - tm.tmOverhang;
		else
			nX += (int)strText.size() * 8;
		strText = TEXT("  个冗余注册表项，建议立即清理！");
		::SetTextColor(hdcMem, RGB(0,0,0));
		::TextOut(hdcMem, nX, 0, strText.c_str(), (int)strText.size());
	}

	// 复制到
	::BitBlt(hdc, 0, 0, rc.Width(), rc.Height(), hdcMem, 0, 0, SRCCOPY);

	::DeleteObject(hbmp);
	::DeleteDC(hdcMem);
	::ReleaseDC(hwnd, hdc);
	::ValidateRect(hwnd, NULL);

	return 0;
}
