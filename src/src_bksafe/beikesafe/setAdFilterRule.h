#pragma once
//设置广告过滤规则
#include "kws/commonfun.h"
#include "common/whitelist.h"
#include "kws/setting_hlp.h"
#include "kws/simpleipc/KwsIPCWarpper.h"
#include "kws/simpleipc/KwsIPCWarpperOld.h"
#include "kws/ctrls/rulelistctrl.h"


#define		SBP_FILE_EXT		TEXT("dat")
#define		SBP_FILE_NAME		TEXT("blacklist")
#define		SBP_FULL_NAME		TEXT("blacklist.dat")
#define		SBP_HEAD_ADDR		TEXT("网址")
#define		SBP_HEAD_TYPE		TEXT("性质")
#define		SBP_DLG_FILTER		TEXT("数据文件 (*.dat)|*.dat|所有文件 (*.*)|*.*||")
#define		SBP_CLEAN_TEXT		TEXT("是否清空所有的黑名单，点击“是”清\r\n空列表，否则点击“否”。")
#define		SBP_VIRUS_WEB		TEXT("带毒网站")
#define		SBP_FISHWEB			TEXT("欺诈网址")
#define		SBP_ADVWEB			TEXT("恶意广告")

class CSetBlackPage 
	: public CBkDialogImpl<CSetBlackPage>
	,public CWHRoundRectFrameHelper<CSetBlackPage>
{
public:
	CSetBlackPage()
		: CBkDialogImpl<CSetBlackPage>(IDR_BK_SAFEKWS_ADRULE_SETTING_DLG)
	{
	}

	~CSetBlackPage(void)
	{
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{				
		return 0;
	}
	BOOL OnInitDialog(HWND wParam, LPARAM lParam)
	{	
		InitCtrls();
		UpdateConfig();
		RefreshTurnOnAntiAdTipState();		
		return TRUE;
	}
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		UninitCtrls();
		return 0;
	}
	void OnBkBtnClose()
	{
		EndDialog(IDCANCEL);
		return;
	}
	void UpdateDelLink()
	{
		if( m_wndEdit.IsWindow() )
		{
			int nCnt = m_wndListView.GetItemCount();
			if (nCnt > 0)
			{
				//导入 导出  清空  全部为可选
				EnableItem( IDC_LINKTEXT_IMPORT, TRUE );
				EnableItem( IDC_LINKTEXT_EXPORT, TRUE );
				EnableItem( IDC_LINKTEXT_CLEAR,	 TRUE );
			}
			else//除导入以外  其他全部为不可选状态
			{
				EnableItem( IDC_LINKTEXT_IMPORT, TRUE );			
				EnableItem( IDC_LINKTEXT_EXPORT, FALSE );
				EnableItem( IDC_LINKTEXT_CLEAR,	 FALSE );
				EnableItem( IDC_LINKTEXT_DELETE, FALSE );
			}
			int nIndex = m_wndListView.GetSelectedIndex();
			if (nIndex != -1)
			{
				EnableItem( IDC_LINKTEXT_DELETE,  TRUE);
			}
			else
			{
				EnableItem( IDC_LINKTEXT_DELETE,  FALSE);
			}
		}
		
		return;
	}
	void UpdateConfig()
 	{
		std::vector<BLACKLISTITEM> vecList;
		CSettingHlp::Instance().GetBlackList( vecList );
		for ( int i = 0; i < vecList.size(); i++ )
		{
			AddListItem(  vecList[i].wType,  vecList[i].strUrl, i );
		}

		UpdateDelLink();
		SetItemVisible(IDC_TXT_ADFILTER_REBOOT, FALSE);		
		EnableItem( IDC_BTN_ADD_ID, FALSE );
		EnableItem( IDC_LINKTEXT_DELETE,  FALSE);
	}
	void InitCtrls()
	{
		m_wndListView.SetRowHeight( 24 );
		m_rtList = CRect( 13, 77, 13 + 463, 77 + 282);
		m_wndListView.Create( GetViewHWND(), m_rtList, NULL, 
			WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 
			0, IDC_LIST_ADRULE_DLG);
		m_wndListView.HeaderSubclassWindow();
		m_rtList.InflateRect( 1, 1 );
		m_wndListView.SetExtendedListViewStyle( LVS_EX_FULLROWSELECT | m_wndListView.GetExtendedListViewStyle() );

		m_wndListView.InsertColumn(0, L"网址",	LVCFMT_LEFT, 340);
		m_wndListView.InsertColumn(1, L"性质",		LVCFMT_CENTER, 100);
		m_wndListView.SetNotifyWnd( m_hWnd );

		m_wndEdit.Create( GetViewHWND(), 
			NULL,
			NULL,
			WS_VISIBLE | WS_CHILDWINDOW | ES_AUTOHSCROLL,
			0,
			IDC_EDITBOX_ADRULE_DLG);
		m_wndEdit.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));
		return;
	}	
	void UninitCtrls()
	{
		if( m_wndListView.IsWindow() )
			m_wndListView.DestroyWindow();

		if( m_wndEdit.IsWindow() )
			m_wndEdit.DestroyWindow();
	}
	LRESULT OnEditChange(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
	{
		if( m_wndEdit.IsWindow() )
		{
			ATL::CString strUrl;
			m_wndEdit.GetWindowText( strUrl );
			if (strUrl.GetLength() > 0)
			{
				EnableItem( IDC_BTN_ADD_ID, TRUE );
			}
			else
			{
				EnableItem( IDC_BTN_ADD_ID, FALSE );
			}
		}

		return TRUE;
	}
	LRESULT OnListViewItemChange( int wParam, LPNMHDR lParam, BOOL& bHandled )
	{
		if( m_wndListView.IsWindow() )
		{
			int nIndex = m_wndListView.GetSelectedIndex();
			if (nIndex != -1)
			{
				EnableItem( IDC_LINKTEXT_DELETE,  TRUE);
			}
			else
			{
				EnableItem( IDC_LINKTEXT_DELETE,  FALSE);
			}
			
		}

		return TRUE;
	}
	//添加网址
	void ShowRestartText()
	{	
		SetItemVisible(IDC_TXT_ADFILTER_REBOOT, TRUE);
		Invalidate();
		return;
	}
	void OnAddUrl()
	{
		ATL::CString strUrl;
		int nType = UT_ADVWEB;//只有恶意的
		m_wndEdit.GetWindowText( strUrl );

		std::wstring szUrl(strUrl);
		std::wstring szFtUrl;

		std::wstring szexist = L"0";
		//在服务进行过滤
		kws_old_ipc::KwsBlackListAddEx( nType, szUrl.c_str(), szFtUrl, szexist );		
		if( szFtUrl != TEXT(""))
		{
			if (CBkSafeMsgBox2::ShowMultLine( L"您是否确定添加这个网址？", L"金山卫士",  MB_OKCANCEL|MB_ICONWARNING) == IDOK)
			{
				AddListItem( nType, szFtUrl.c_str(), m_wndListView.GetItemCount() );
				ShowRestartText();
				m_wndEdit.SetWindowText( TEXT("") );
			}
			
		}
 		else if( 0 == szexist.compare(L"1"))
 		{
			//网址存在
			CBkSafeMsgBox2::ShowMultLine( L"您输入的网址已经存在，请重新输入！", L"金山卫士",  MB_OK|MB_ICONWARNING );
 		}
 		else
		{
			//网址有误  请重新输入
			CBkSafeMsgBox2::ShowMultLine(L"您输入的网址有误，请重新输入！",  L"金山卫士", MB_OK|MB_ICONWARNING);
		} 			
		UpdateDelLink();
		return;
	}
	void AddListItem( int nType, LPCTSTR pszUrl, int nPos )//添加列表
	{
		if ( !m_wndListView.IsWindow() )
			return;

		int j = 0;
		m_wndListView.InsertItem( nPos, TEXT("") );
		m_wndListView.SetItemText( nPos, j++, pszUrl );
		ATL::CString strType;
		GetTypeString( nType, strType );
		m_wndListView.SetItemText( nPos, j++, strType );	
		return;
	}
	void GetTypeString( int nType, ATL::CString &str )//病毒类型
	{
		switch( nType )
		{
		case UT_VIRUSWEB:
			str = SBP_VIRUS_WEB;
			break;
		case UT_FISHWEB:
			str = SBP_FISHWEB;
			break;
		case UT_ADVWEB:
			str = SBP_ADVWEB;
			break;
		default:
			str = SBP_VIRUS_WEB;
		}
		return;
	}
	void ConvertToWtlFileDialogFilter( LPWSTR pwszFilter )
	{
		size_t nCount = wcslen( pwszFilter );
		for( size_t i = 0; i < nCount; i++ )
		{
			if( pwszFilter[i] == '|' )
				pwszFilter[i] = '\0';
		}
		return;
	}
	void OnImport()
	{
		WCHAR szFilter[MAX_PATH] = SBP_DLG_FILTER;
		ConvertToWtlFileDialogFilter(szFilter);
		CFileDialog fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST,
			szFilter, 0);
		fileDlg.m_ofn.lStructSize = (::GetVersion() < 0x80000000) ? 88 : 76;	// 这样才能显示新的风格
		if (IDOK == fileDlg.DoModal())
		{		
			kws_old_ipc::KwsBlackListImport(fileDlg.m_szFileName,  L"0", L"0");
			if( m_wndListView.IsWindow() )
			{
				m_wndListView.DeleteAllItems();
				UpdateConfig();
			}

			ShowRestartText();//显示重启的			
		}
		UpdateDelLink();
		return;
	}
	void OnExport()
	{
		WCHAR szFilter[MAX_PATH] = SBP_DLG_FILTER;
		ConvertToWtlFileDialogFilter(szFilter);
		CFileDialog fileDlg(FALSE, SBP_FILE_EXT, SBP_FILE_NAME, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
			szFilter, 0);
		fileDlg.m_ofn.lStructSize = (::GetVersion() < 0x80000000) ? 88 : 76;	// 这样才能显示新的风格
		if (IDOK == fileDlg.DoModal())
		{		
			kws_old_ipc::KwsBlackListExport(fileDlg.m_szFileName);
		}
		UpdateDelLink();
		return;
	}
	void OnDelete()
	{
		if ( !m_wndListView.IsWindow() )
			return;

		int nIndex = m_wndListView.GetSelectedIndex();
		ATL::CString strUrl;
		if( nIndex != -1 )
		{
			int nRet = 0;
			nRet = CBkSafeMsgBox2::ShowMultLine( L"您是否确定删除选中的网址？", L"金山卫士",  MB_OKCANCEL|MB_ICONWARNING );
			if( nRet == IDCANCEL )
				return;
			m_wndListView.GetItemText( nIndex, 0, strUrl );		
			m_wndListView.DeleteItem( nIndex );
			kws_old_ipc::KwsBlackListDelete( strUrl );
			ShowRestartText();
		}
		UpdateDelLink();
		return;
	}
	void OnClear()
	{
		WCHAR szLog[MAX_PATH] = {0};
		BOOL bRet = FALSE;

		ATL::CString strMsg( SBP_CLEAN_TEXT );
		if (CBkSafeMsgBox2::ShowMultLine( strMsg, L"金山卫士",  MB_YESNO|MB_ICONWARNING) == IDYES)		
		{
			if( m_wndListView.IsWindow() )
				m_wndListView.DeleteAllItems();

			WCHAR szLog[MAX_PATH] = {0};
			BOOL bRet = FALSE;
			bRet = GetAllUserKWSPath(szLog, MAX_PATH);
			wcscat_s(szLog, MAX_PATH, SBP_FULL_NAME );
			kws_old_ipc::KwsDeleteLog(szLog);

			if( m_wndListView.IsWindow() )
				m_wndListView.DeleteAllItems();

			ShowRestartText();
		}
		UpdateDelLink();
		return;
	}
	void OnAntiAdEnable()
	{
		KwsSetting setting;
		setting.SetAntiAdEnable(TRUE);
		RefreshTurnOnAntiAdTipState();
		return;
	}
	void RefreshTurnOnAntiAdTipState()
	{
		KwsSetting setting;
		if( setting.IsAntiAdEnable() )
		{
			SetItemVisible(IDC_LINKTEXT_ADFILTER_ON, FALSE);
			SetItemVisible(IDC_TXT_ADFILTER_ONOFF,  FALSE);
		}
		else
		{
			SetItemVisible(IDC_LINKTEXT_ADFILTER_ON, TRUE);
			SetItemVisible(IDC_TXT_ADFILTER_ONOFF,  TRUE);
		}
		return;
	}

	BK_NOTIFY_MAP(IDC_RICHVIEW_WIN);	
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE,			OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_ADD_ID,			OnAddUrl)//添加网址
		BK_NOTIFY_ID_COMMAND(IDC_LINKTEXT_IMPORT,		OnImport)//导入
		BK_NOTIFY_ID_COMMAND(IDC_LINKTEXT_EXPORT,		OnExport)//导出
		BK_NOTIFY_ID_COMMAND(IDC_LINKTEXT_DELETE,		OnDelete)//删除
		BK_NOTIFY_ID_COMMAND(IDC_LINKTEXT_CLEAR,		OnClear)//清空	
		BK_NOTIFY_ID_COMMAND(IDC_LINKTEXT_ADFILTER_ON,	OnAntiAdEnable)//启用广告过滤功能
	BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CSetBlackPage)	
		MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
		CHAIN_MSG_MAP(CBkDialogImpl<CSetBlackPage>)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CSetBlackPage>)
 		MESSAGE_HANDLER(WM_CREATE,				OnCreate)
 		MESSAGE_HANDLER(WM_DESTROY,				OnDestroy)	
		MSG_WM_INITDIALOG(OnInitDialog)
		NOTIFY_HANDLER( IDC_LIST_ADRULE_DLG, LVN_ITEMCHANGED,	OnListViewItemChange)
		COMMAND_HANDLER(IDC_EDITBOX_ADRULE_DLG, EN_CHANGE,		OnEditChange)

		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
private:
	KRuleListCtrl		m_wndListView;
	CRect				m_rtList;
	CEdit				m_wndEdit;
};