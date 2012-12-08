#pragma  once

#include <bkwin/bkdlgview.h>
#include <wtlhelper/whwindow.h>
#include <wtlhelper/bkfolderbrowsectrl.h>
#include <bksafe/bksafeconfig.h>
#include "SoftMgrUniExpandListView.h"
#include <softmgr/ISoftUninstall.h>
#include "bkmsgdefine.h"


// 强力清扫中使用的tree， 与上面的稍有不同， 在子节点全选中时父节点变成 checked， 而不是mixed
class CWH3StateCheckTreeCtrl2
	: public CWindowImpl<CWH3StateCheckTreeCtrl2, CTreeViewCtrl>
{
public:

	enum {
		CHECK_STATE_NONE      = 0,
		CHECK_STATE_UNCHECKED,
		CHECK_STATE_CHECKED,
		CHECK_STATE_MIXED,
	};

	BOOL GetCheckState(HTREEITEM hItem)
	{
		return CHECK_STATE_CHECKED == (GetItemState(hItem, TVIS_STATEIMAGEMASK) >> 12);
	}

	BOOL HasCheckedItem()
	{
		HTREEITEM hItem = GetChildItem(TVI_ROOT);

		while (NULL != hItem)
		{
			switch (GetItemState(hItem, TVIS_STATEIMAGEMASK) >> 12)
			{
			case CHECK_STATE_UNCHECKED:
				break;

			case CHECK_STATE_CHECKED:
			case CHECK_STATE_MIXED:
				return TRUE;

			case CHECK_STATE_NONE:
				hItem = GetChildItem(hItem);
				continue;
			}

			HTREEITEM hNextItem = GetNextSiblingItem(hItem);
			if (NULL == hNextItem)
			{
				HTREEITEM hParentItem = hItem;

				do 
				{
					hParentItem = GetParentItem(hParentItem);
					if (NULL == hParentItem)
						break;

					hNextItem = GetNextSiblingItem(hParentItem);

				} while (NULL == hNextItem);
			}

			hItem = hNextItem;
		}

		return FALSE;
	}

	void Init3State()
	{
		_Init3State();
	}

protected:

	CWH3StateCheckImageList m_wnd3StateCheckImgList;

	void _Init3State()
	{
		m_wnd3StateCheckImgList.Create();

		SetImageList(m_wnd3StateCheckImgList, TVSIL_STATE);
	}

	void _CheckSelfAndAllChilds(HTREEITEM hItem, BOOL bCheck)
	{
		SetCheckState(hItem, bCheck);

		HTREEITEM hItemChild = GetChildItem(hItem);
		while (NULL != hItemChild)
		{
			_CheckSelfAndAllChilds(hItemChild, bCheck);

			hItemChild = GetNextSiblingItem(hItemChild);
		}
	}

	void _ResetParentState(HTREEITEM hItem)
	{
		HTREEITEM hItemParent = GetParentItem(hItem);

		if (NULL == hItemParent)
			return;

		UINT uCheck = (GetItemState(hItemParent, TVIS_STATEIMAGEMASK) >> 12);

		if (CHECK_STATE_NONE == uCheck)
			return;

		HTREEITEM hItemParentChild = GetChildItem(hItemParent);

		BOOL bHasCheck = FALSE, bHasUncheck = FALSE;

		while (NULL != hItemParentChild)
		{
			uCheck = (GetItemState(hItemParentChild, TVIS_STATEIMAGEMASK) >> 12);

			bHasCheck |= CHECK_STATE_CHECKED == uCheck || CHECK_STATE_MIXED == uCheck;
			bHasUncheck |= CHECK_STATE_UNCHECKED == uCheck || CHECK_STATE_MIXED == uCheck;

			hItemParentChild = GetNextSiblingItem(hItemParentChild);
		}

		if (bHasCheck)
		{
			if (bHasUncheck)
			{
				uCheck = CHECK_STATE_MIXED;
			}         
			else
			{
				uCheck = CHECK_STATE_CHECKED;
			}

		}
		else
		{
			if (bHasUncheck)
				uCheck = CHECK_STATE_UNCHECKED;
			else
				uCheck = CHECK_STATE_NONE;
		}

		SetItemState(hItemParent, INDEXTOSTATEIMAGEMASK(uCheck), TVIS_STATEIMAGEMASK);

		_ResetParentState(hItemParent);
	}

	void _ToggleItem(HTREEITEM hItem)
	{
		UINT uState = GetItemState(hItem, TVIS_STATEIMAGEMASK) >> 12;

		if (CHECK_STATE_NONE == uState)
			return;

		SetRedraw(FALSE);

		BOOL bCheck = !(CHECK_STATE_CHECKED == uState);

		_CheckSelfAndAllChilds(hItem, bCheck);
		_ResetParentState(hItem);

		SetRedraw(TRUE);

		NMHDR nms;
		nms.code = WH3STVN_ITEMCHECKCHANGE;
		nms.hwndFrom = m_hWnd;
		nms.idFrom = GetDlgCtrlID();

		LRESULT lRet = ::SendMessage(GetParent(), WM_NOTIFY, (LPARAM)nms.idFrom, (WPARAM)&nms);
	}

	void OnLButtonDown(UINT nFlags, CPoint point)
	{
		TVHITTESTINFO htinfo;

		ZeroMemory(&htinfo, sizeof(htinfo));

		htinfo.pt = point;

		HitTest(&htinfo);

		if (NULL == htinfo.hItem || TVHT_ONITEMSTATEICON != htinfo.flags)
		{
			SetMsgHandled(FALSE);
			return;
		}

		_ToggleItem(htinfo.hItem);
	}

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (VK_SPACE != nChar)
		{
			SetMsgHandled(FALSE);
			return;
		}

		HTREEITEM hItem = GetSelectedItem();
		if (NULL == hItem)
		{
			SetMsgHandled(FALSE);
			return;
		}

		_ToggleItem(hItem);
	}

public:

	BEGIN_MSG_MAP_EX(CBkFolderBrowseCtrl)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDown)
		MSG_WM_KEYDOWN(OnKeyDown)
		END_MSG_MAP()
};


class CBeikeSafeSoftmgrUIHandler;
class CBeikeSoftMgrPowerSweepDlg
	: public CBkDialogImpl<CBeikeSoftMgrPowerSweepDlg>
	, public CWHRoundRectFrameHelper<CBeikeSoftMgrPowerSweepDlg>
{
public:
	CBeikeSoftMgrPowerSweepDlg( CBeikeSafeSoftmgrUIHandler * pData )
		: CBkDialogImpl<CBeikeSoftMgrPowerSweepDlg>(IDR_BK_SOFTMGR_POWERSWEEP)
	{
		m_nRudimental = 0;
		m_pHandler = pData; 
		m_nStopPowerSweep = 0;
		m_bDeleteFile = FALSE;
		m_bDeleteReg = FALSE;
		m_strTittle = BkString::Get(IDS_SOFTMGR_8054);
		m_nSweepedNumber = 0;
		m_bHasClearAction = FALSE;		//是否有清除动作
		m_prubbishSweep  = NULL;
		m_bSweeping = FALSE;
	}

	~CBeikeSoftMgrPowerSweepDlg()
	{
	}

	// SetTreeData
	CWH3StateCheckTreeCtrl2			m_Tree;
	CBeikeSafeSoftmgrUIHandler*		m_pHandler;
	int								m_nStopPowerSweep;
	BOOL							m_bDeleteFile;
	BOOL							m_bDeleteReg;
	CString							m_strTittle;
	int								m_nRudimental;//残留项, 描述残留项个数，从卸载残留项点击过来才会非0

	CSimpleArray<SOFT_RUB_INFO*>	m_rubList;
	CString							m_strName;
	int								m_nCheckNum;
	ksm::ISoftDataSweep*			m_psoftDataSweep;
	ksm::ISoftRubbishSweep*			m_prubbishSweep;
	int								m_nSweepedNumber;	// 已经清扫的个数，用来显示个数
	BOOL							m_bHasClearAction;
	BOOL							m_bSweeping;

public:

	// 软件卸载的
	VOID CopySoftRubInfo( CSimpleArray<SOFT_RUB_INFO*>& rubList,ksm::ISoftDataSweep* pDataSweep, LPCTSTR lpName)
	{
		m_rubList			= rubList;
		m_nRudimental		= 0;
		m_psoftDataSweep	= pDataSweep;
		m_strName			= lpName;
	}
	VOID CopySysRubInfo(CSimpleArray<SOFT_RUB_INFO*>& rubList, ksm::ISoftRubbishSweep* pRubSweep)
	{
		m_rubList			= rubList;
		m_nRudimental		= m_rubList.GetSize();
		m_prubbishSweep		= pRubSweep;
	}

	BOOL IsCleared()
	{
		return m_bHasClearAction;
	}

	BOOL IsClearAll(DWORD & nClearNumber)
	{
		if ( !IsCleared() )
			return FALSE;

		nClearNumber = m_nCheckNum;
		
		if ( m_nCheckNum < m_rubList.GetSize() )
			return FALSE;

		return TRUE;
	}

protected:
	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		bool	bRegistry = false;

		// 显示上面的提示串
		SetItemText(IDC_TEXT_TITTLE_POWERSWEEP, m_strTittle);

		CString	tmp;
		tmp.Format( _T( " %s " ), m_strName );

		CString strText = tmp;
		CString str = tmp ;
		CRect rcCalc;
		CDC dc = GetDC();
		dc.DrawText(str, -1, &rcCalc, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
		int nWidth = rcCalc.Width();
		ReleaseDC(dc);

		if ( nWidth > 165)
		{
			strText = strText.Left( 16 ) + _T("...");
		}

		if (m_nRudimental)
		{
			strText.Format(L"%d", m_nRudimental);
			FormatRichText(
				IDC_TEXT_TIP_POWERSWEEP, 
				BkString::Get(IDS_TXT_INTRO_PS_FMT_ALL), 
				strText );
		}
		else
		{
			FormatRichText(
				IDC_TEXT_TIP_POWERSWEEP, 
				BkString::Get(IDS_TXT_INTRO_PS_FMT), 
				strText );
		}

		//开始不显示进度
		SetItemVisible(IDC_TEXT_TIP_POWERSWEEP, TRUE);
		SetItemVisible(IDC_PROG_POWERSWEEP, FALSE);

		/*FormatRichText(
			IDC_TXT_INTRO_PS, 
			BkString::Get(IDS_TXT_INTRO_PS_FMT), 
			tmp );*/
		
		// 创建树型控件
		m_Tree.Create( GetViewHWND(), NULL, NULL, WS_TABSTOP | WS_CHILD | TVS_CHECKBOXES | TVS_HASLINES | TVS_HASBUTTONS | TVS_FULLROWSELECT | TVS_LINESATROOT | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS, 
			0, IDC_TREE_PS_ITEMS, NULL );
		m_Tree.Init3State();

		TVINSERTSTRUCT tvInsert;
		tvInsert.hParent = NULL;
		tvInsert.hInsertAfter = NULL;
		tvInsert.item.mask = TVIF_TEXT;
		tvInsert.item.pszText = (LPWSTR)BkString::Get(IDS_SOFTMGR_8019);

		HTREEITEM hRoot = m_Tree.InsertItem( &tvInsert );
		CAtlMap<CString, HTREEITEM>		typeMap;
		HTREEITEM		hCurrent = NULL;
		HTREEITEM		hDir	 = NULL;
		CString			strDir;

		HTREEITEM		hFirst = NULL;
		HTREEITEM		hSecond	= NULL;

		if (m_nRudimental)
		{
			// 残留项目的强力清扫
			for ( int i=0; i<m_rubList.GetSize(); i++ )
			{
				SOFT_RUB_INFO& item = *(m_rubList[i]);

				if ( !bRegistry && item.nType == ksm::SIA_Registry )
					bRegistry = true;

				CString	strCaption = GetSysRubCaption( item.strData );
				if ( typeMap.Lookup( strCaption ) != NULL )
				{
					//hCurrent = typeMap[p->name];
					//m_Tree.InsertItem( p->m_strPath, hCurrent, TVI_LAST );

					//////////////////////////////////////////////////////////////////////////
					hCurrent = typeMap[strCaption];
					hDir = FindTreeItem(hCurrent, item.strData);
					if (hDir)
					{
						m_Tree.InsertItem( item.strData, hDir, TVI_LAST );
					}
					else
					{
						m_Tree.InsertItem( item.strData, hCurrent, TVI_LAST );
					}
				}
				else
				{
					hCurrent = m_Tree.InsertItem( strCaption, hRoot, TVI_LAST );
					typeMap[strCaption] = hCurrent;

					m_Tree.InsertItem( item.strData, hCurrent, TVI_LAST );
				}
			}

			hFirst = m_Tree.GetNextItem( hRoot, TVGN_CHILD );
			while ( hFirst != NULL )
			{
				m_Tree.Expand( hFirst );
				hFirst = m_Tree.GetNextItem( hFirst, TVGN_NEXT );
			}
		}
		else
		{
			// 正常卸载项卸载项目的强力清扫
			for ( int i=0; i<m_rubList.GetSize(); i++ )
			{
				SOFT_RUB_INFO& item = *m_rubList[i];

				CString	strPath;
				strPath = item.strData;
				strPath.TrimRight(_T( "\\" ));
				strPath.MakeLower();

				if ( !bRegistry && item.nType == ksm::SIA_Registry )
					bRegistry = TRUE;

				CString	strParentDir;
				strParentDir = strPath.Left( strPath.ReverseFind( _T( '\\' ) ) );

				if ( typeMap.Lookup( strParentDir ) != NULL )
				{				
					hDir = m_Tree.InsertItem( item.strData, typeMap[strParentDir], TVI_LAST );

					if ( item.nType == ksm::SIA_Directory )//目录
					{
						typeMap[strPath] = hDir;	
					}
				}
				else
				{
					CString typeName;
					if ( item.nType != ksm::SIA_Registry )//目录或文件
					{
						typeName = BkString::Get(IDS_SOFTMGR_8083);//目录文件
					}
					else
					{
						typeName = BkString::Get(IDS_SOFTMGR_8084);//注册表项
					}

					// 首先判断类型是否已经存在
					if ( typeMap.Lookup( typeName ) != NULL )
					{
						hCurrent = typeMap[typeName];
					}
					else
					{
						if ( typeMap.IsEmpty() )
						{
							hCurrent = m_Tree.InsertItem(TVIF_TEXT, typeName, 0, 0, 0, 0, 0, hRoot, NULL);
						}
						else
						{
							hCurrent = m_Tree.InsertItem( typeName, 0, 0, hRoot, hCurrent );
						}

						typeMap[typeName] = hCurrent;
					}
				
					if ( item.nType == ksm::SIA_Directory )	// 目录
					{
						typeMap[strPath] = m_Tree.InsertItem( item.strData, hCurrent, TVI_LAST );
					}
					else
					{
						m_Tree.InsertItem( item.strData, hCurrent, TVI_LAST );
					}
				}
			}

			hCurrent = m_Tree.GetNextItem( hRoot, TVGN_CHILD );
			while ( hCurrent != NULL )
			{
				m_Tree.Expand( hCurrent );
				hCurrent = m_Tree.GetNextItem( hCurrent, TVGN_NEXT );
			}
		}

		m_Tree.Expand( hRoot );
		m_Tree.Select( hRoot, TVGN_FIRSTVISIBLE );
		m_Tree.SetScrollPos( SB_HORZ, 0, TRUE );

		// 如有注册表项，修改描述
		if ( bRegistry )
		{
			if (m_nRudimental)
			{
				FormatRichText(
					IDC_TEXT_TIP_POWERSWEEP, 
					BkString::Get(IDS_SOFTMGR_SWEEP_FILE_REG_ALL), 
					strText );
			}
			else
			{
				FormatRichText(
					IDC_TEXT_TIP_POWERSWEEP, 
					BkString::Get(IDS_SOFTMGR_SWEEP_FILE_REG), 
					strText );
			}
		}

		// 启用删除按钮
		EnableItem( IDC_BTN_DEL_SELECTED, FALSE );

		// 创建进度条定时器
		SetTimer( 1, 500, NULL );

		// 设置删除方法单选钮
		if ( BKSafeConfig::GetPowerSweepFile() == 0 )
			SetItemCheck(IDC_DELETE_TO_RECYCLEBIN, TRUE);
		else
			SetItemCheck(IDC_DELETE_DIRECTLY, TRUE);

		return TRUE;
	}

	HTREEITEM FindTreeItem(HTREEITEM hCurrent, CString strPath)
	{
		CString	strParentDir;
		strParentDir = strPath.Left( strPath.ReverseFind( _T( '\\' ) ) );

		HTREEITEM hChild = NULL;
		hChild = m_Tree.GetNextItem( hCurrent, TVGN_CHILD );
		while ( hChild != NULL )
		{
			CString strText;
			m_Tree.GetItemText(hChild, strText);

			if (strText.CompareNoCase(strParentDir) == 0)
			{
				return hChild;
			}
			else
			{
				HTREEITEM hFind = NULL;
				hFind = FindTreeItem(hChild, strPath);
				if (hFind)
					return hFind;
			}

			hChild = m_Tree.GetNextItem( hChild, TVGN_NEXT );
		}

		return NULL;
	}

	void OnBkBtnClose()
	{
		if (m_bSweeping)
			return;

		EndDialog( IDCANCEL );
	}

	void OnDelSelected();

	LRESULT OnProgStep(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		int	nPercent = ( int ) wParam;
		
		static	int	nCount = 0; 
		
		if ( ( int )lParam != -1 )
		{
			nCount = ( int ) lParam;
		}
		
		SetItemIntAttribute(IDC_PROG_POWERSWEEP, "value", nPercent );

		// 清理完成
		if ( 100 == nPercent )
		{
			if (m_nRudimental)
			{
				// 清理完成的提示
				FormatRichText(IDC_TXT_CLEAN_OVER, BkString::Get(IDC_TXT_CLEAN_OVER_FMT), nCount );

				// 禁用清除按钮
				EnableItem( IDC_BTN_DEL_SELECTED, FALSE );

				// 显示结果页面
				//SetItemVisible(DLG_SOFTMGR_UNIN_SWEEP_TREE, FALSE);
				m_Tree.ShowWindow(SW_HIDE);
				SetItemVisible(DLG_SOFTMGR_UNIN_SWEEP_RESULT, TRUE);
			}
			else
			{
				// 清理完成的提示
				FormatRichText(IDC_TXT_CLEAN_OVER, BkString::Get(IDC_TXT_CLEAN_OVER_FMT), nCount );

				// 清空视图控件中的子元素
				m_Tree.DeleteAllItems();

				// 禁用清除按钮
				EnableItem( IDC_BTN_DEL_SELECTED, FALSE );

				// 显示结果页面
				//SetItemVisible(DLG_SOFTMGR_UNIN_SWEEP_TREE, FALSE);
				m_Tree.ShowWindow(SW_HIDE);
				SetItemVisible(DLG_SOFTMGR_UNIN_SWEEP_RESULT, TRUE);
			}
			RefreshExplorer();

			m_bSweeping = FALSE;
			EnableItem(IDCANCEL,TRUE);
		}

		return  0;
	}

	int	GetTreeCheckedCount( HTREEITEM h, BOOL bSet )
	{
		int		nChecked = 0;

		if ( m_Tree.GetCheckState( h ) )
		{
			CString	tmp;
			m_Tree.GetItemText( h, tmp );

			if ( tmp == BkString::Get(IDS_SOFTMGR_8019)
				|| tmp == BkString::Get(IDS_SOFTMGR_8083)
				|| tmp == BkString::Get(IDS_SOFTMGR_8084)
				|| tmp.Find(BkString::Get(IDS_SOFTMGR_SWEEP_FILE_NAME)) != -1
				)
			{
				nChecked = 0;
			}
			else
			{
				nChecked = 1;
			}
			

			if ( bSet )
			{
				for ( int i=0; i<m_rubList.GetSize(); i++ )
				{
					SOFT_RUB_INFO&	item = *(m_rubList[i]);

					if ( item.strData == tmp )
					{
						item.bCheck = TRUE;
						if ( item.nType != ksm::SIA_Registry )
						{
							m_bDeleteFile = TRUE;
						}
						else
						{
							m_bDeleteReg = TRUE;
						}
						break;
					}
				}
			}
		}

		HTREEITEM	hChild = m_Tree.GetNextItem( h, TVGN_CHILD );
		while ( hChild != NULL )
		{
			nChecked += GetTreeCheckedCount( hChild, bSet );

			hChild = m_Tree.GetNextItem( hChild, TVGN_NEXT );
		}

		return nChecked;
	}

	void	OnTimer(UINT_PTR nIDEvent)
	{
		if ( nIDEvent == 1 )
		{
			HTREEITEM	hRoot = m_Tree.GetRootItem();
			m_nCheckNum = GetTreeCheckedCount( hRoot, FALSE );

			if ( m_nCheckNum > 0 )
			{
				EnableItem( IDC_BTN_DEL_SELECTED, TRUE );	
			}
			else
			{
				EnableItem( IDC_BTN_DEL_SELECTED, FALSE );	
			}
		}
	}

public:
	void SetDlgTittle(CString strTittle)
	{
		m_strTittle = strTittle;
	}

	// 系统残留项清扫事件
	void SoftRubbishSweepEvent(ksm::UninstEvent event, ksm::ISoftRubbishSweep *pSweep)
	{
		if (m_bSweeping && !::IsWindow(m_hWnd) )
			return;

		switch ( event )
		{
		case ksm::UE_Sweeping:
			{
				ATLASSERT(m_nCheckNum!=0);
				DWORD	nProgress = 0;
				m_nSweepedNumber++;
				nProgress = m_nSweepedNumber*100/m_nCheckNum;
				if (nProgress>=100)
					nProgress=99;
				SendMessage(MSG_APP_POWERSWEEP_PROG_STEP, (WPARAM)nProgress, (LPARAM)m_nSweepedNumber );
			}
			break;
		case ksm::UE_Sweep_End:
			{
				SendMessage(MSG_APP_POWERSWEEP_PROG_STEP, (WPARAM)100, (LPARAM)m_nCheckNum );
			}
			break;
		default:
			break;
		}
	}


	// 软件卸载残留项清扫事件
	void SoftDataSweepEvent(ksm::UninstEvent event, ksm::ISoftDataSweep *pSweep)
	{
		switch ( event )
		{
		case ksm::UE_Sweeping:
			{
				ATLASSERT(m_nCheckNum!=0);
				DWORD	nProgress = 0;
				m_nSweepedNumber++;
				nProgress = m_nSweepedNumber*100/m_nCheckNum;
				SendMessage(MSG_APP_POWERSWEEP_PROG_STEP, (WPARAM)nProgress, (LPARAM)m_nSweepedNumber );
			}
			break;
		case ksm::UE_Sweep_End:
			{
				SendMessage(MSG_APP_POWERSWEEP_PROG_STEP, (WPARAM)100, (LPARAM)m_nCheckNum );
			}
			break;
		default:
			break;
		}
	}

	CString	GetSysRubCaption(LPCTSTR lpFilePath)
	{
		LPCTSTR	lpName = _tcsrchr( lpFilePath, _T('\\') );

		if (lpName == NULL)
			lpName = lpFilePath;
		else
			lpName++;

		CString	strCaption = BkString::Get(IDS_SOFTMGR_SWEEP_FILE_NAME);
		strCaption += lpName;
		return strCaption;
	}

protected:

	void RefreshExplorer()
	{
		::SHChangeNotify( SHCNE_ASSOCCHANGED , SHCNF_IDLIST | SHCNF_FLUSH, 0, 0); 

		HWND hWndProgram = ::FindWindow( _T("Progman"), NULL); 
		HWND hWndDefView = ::FindWindowEx( hWndProgram, NULL, _T("SHELLDLL_DefView"), NULL); 
		HWND hWndListView= ::FindWindowEx( hWndDefView, NULL, _T("SysListView32"),    NULL); 

		::PostMessage( hWndListView, WM_KEYDOWN, VK_F5, 0); 
		::PostMessage( hWndListView, WM_KEYUP,    VK_F5, 0); 
	}

	void _PowerSweepThread()
	{

		static int nDelNum = 0;
		static int nPercent;

		nDelNum = 0;
		if ( m_nCheckNum > 0 )
		{
			for ( int i=0; i< m_rubList.GetSize(); i++ )
			{
				if ( m_rubList[i]->bCheck )
				{
					ksm::SoftRubbish	rub;
					rub._pData	= m_rubList[i]->strData;
					rub._type	= (ksm::SoftItemAttri)m_rubList[i]->nType;

					if ( m_nRudimental == 0 )
						m_psoftDataSweep->DataSetSweep(&rub);
					else
						m_prubbishSweep->RubbishSetSweep(&rub);
				}
			}

			if ( m_nRudimental == 0 )
				m_psoftDataSweep->DataSweep( !BKSafeConfig::GetPowerSweepFile() );	
			else
				m_prubbishSweep->RubbishSweep( !BKSafeConfig::GetPowerSweepFile() );
		}
	}

public:
	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDCANCEL, OnBkBtnClose)

		// button
		BK_NOTIFY_ID_COMMAND(IDC_BTN_DEL_SELECTED, OnDelSelected)
		BK_NOTIFY_ID_COMMAND(IDC_DELETE_TO_RECYCLEBIN, OnDeleteToRecycleBin)
		BK_NOTIFY_ID_COMMAND(IDC_DELETE_DIRECTLY, OnDeleteDirectly)
	BK_NOTIFY_MAP_END()

		BEGIN_MSG_MAP_EX(CBeikeSoftMgrPowerSweepDlg)
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		
		CHAIN_MSG_MAP(CBkDialogImpl<CBeikeSoftMgrPowerSweepDlg>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBeikeSoftMgrPowerSweepDlg>)
		
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_TIMER(OnTimer)
		
		MESSAGE_HANDLER_EX(MSG_APP_POWERSWEEP_PROG_STEP, OnProgStep)
		
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()

private:
	void OnDeleteToRecycleBin()
	{
		BKSafeConfig::SetPowerSweepFile(0);
	}

	void OnDeleteDirectly()
	{
		BKSafeConfig::SetPowerSweepFile(1);
	}
};
