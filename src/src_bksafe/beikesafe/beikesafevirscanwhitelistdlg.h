#pragma once

#include <wtlhelper/whwindow.h>
#include <common/whitelist.h>
#include <comproxy/bkutility.h>
#include "kws/ctrls/whitelistctrl.h"
#include "kws/bkwhilelistimp.h"
#include "kws/ctrls/ScanResultListPanel.h"

#define MSG_LOAD_LOG_FINISH     (WM_APP + 1)
#define MSG_CLEAR_LOG_FINISH    (WM_APP + 2)
#define VS_SCRIPT_FLAG			TEXT("#\\|")

class CBeikeSafeVirScanWhiteListDlg
    : public CBkDialogImpl<CBeikeSafeVirScanWhiteListDlg>
    , public CWHRoundRectFrameHelper<CBeikeSafeVirScanWhiteListDlg>
{
public:
    CBeikeSafeVirScanWhiteListDlg(int nFirstPage = CBeikeSafeVirScanWhiteListDlg::WHITE_FILE_PAGE)
        : CBkDialogImpl<CBeikeSafeVirScanWhiteListDlg>(IDR_BK_VIRSCAN_WHITE_LIST_DLG),
		  m_nCurPage(nFirstPage)
    {
    }

	enum {
		WHITE_FILE_PAGE,
		WHITE_URL_PAGE,
		WHITE_SYSTEM_REPAIR_ITEM_PAGE
	};

protected:

    CWhiteListCtrl m_wndFileList;
	CWhiteListCtrl m_wndUrlList;
	CWhiteListCtrl m_wndScriptList;
	CEdit			m_wndEdit;
	int             m_nCurPage;

    CAtlArray<CWhiteListItem> m_whiteFilelist;
	CAtlArray<CWhiteListItem> m_whiteUrllist;
	CAtlArray<CWhiteListItem> m_whiteScriptlist;

	void OnLblWhiteListAddUrl()
	{
		ATL::CString strUrl;
		m_wndEdit.GetWindowText( strUrl );
		strUrl.TrimLeft();
		strUrl.TrimRight();
		if( CheckInput( strUrl ) )
		{
			if( !HasRepeat(strUrl) )
			{
				std::wstring szUlr = strUrl;
				AddTrustItem( szUlr, enum_TRUST_ITEM_TYPE_URL );
				CWhiteListItem listItem;
				listItem.m_filepath = strUrl;
				m_whiteUrllist.Add( listItem );
				m_wndUrlList.SetItemCount( m_whiteUrllist.GetCount() );
				m_wndEdit.SetWindowText( TEXT("") );
				EnableItem( IDC_LBL_WHITELIST_ADD_URL, FALSE );
			}
			else
			{
				CBkSafeMsgBox::Show( L"\r\n您输入的网址已经存在，不用再次输入。", L"提示！" );
				m_wndEdit.SetWindowText( TEXT("") );
				EnableItem( IDC_LBL_WHITELIST_ADD_URL, FALSE );
			}
			
		}
		else
		{
			CBkSafeMsgBox::Show( L"\r\n您输入的网址有误，请重新输入。", L"提示！" );
		}	
	}

	BOOL HasRepeat(ATL::CString& str)
	{
		BOOL bRet = FALSE;

		for(int i=0;i< m_whiteUrllist.GetCount();i++)
		{
			CString url = m_whiteUrllist[i].m_filepath;

			if (url.CompareNoCase(str) == 0)
			{
				bRet = TRUE;
				break;
			}
		}
		return bRet;
	}

	BOOL CheckInput( ATL::CString& str )
	{
		int nIndex = str.Find( TEXT(".") );
		if( nIndex == 0 )
			return FALSE;
		if( nIndex == -1 )
			return FALSE;
		if( nIndex > str.GetLength() - 3 )
			return FALSE;

		//LPCTSTR pszUrl = str;
		//pszUrl = pszUrl + nIndex + 1;

		LPCTSTR pFind = _tcsrchr( str, TEXT('.') );
		if( !pFind )
			return FALSE;

		int nLen = _tcslen( pFind );
		if( nLen < 2 )
			return FALSE;

		/* 注意: URL匹配前需去除前面的http:// */

		str = str.MakeLower();

		if (str.Find(_T("http://")) == 0)
			str = str.Mid(7);

		return TRUE;
	}
    void OnLblWhiteListAddFile()
    {
		wchar_t wcsFilter[128] = {0};
		wcsncpy(wcsFilter,L"All Files (*.*)\0*.*\0\0",21);
		CFileDialog dlg(TRUE,NULL,NULL,OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ENABLESIZING, 
			wcsFilter,m_hWnd);

        INT_PTR nRet = dlg.DoModal(m_hWnd);
        if (IDOK == nRet)
        {
			std::wstring strFile = dlg.m_ofn.lpstrFile;
			if( !IsFileHasExist( strFile.c_str() ) )
			{
				//AddFileToBk( strFile.c_str() );
				AddTrustItem( strFile, enum_TRUST_ITEM_TYPE_FILE );
				CWhiteListItem listItem;
				listItem.m_filepath = dlg.m_ofn.lpstrFile;
				m_whiteFilelist.Add( listItem );
				m_wndFileList.SetItemCount( m_whiteFilelist.GetCount() );

			}
			else
			{
				CBkSafeMsgBox::Show( L"\r\n您选择的文件已经存在，不用再次添加。", L"提示！" );
			}
		}
    }

	BOOL IsFileHasExist( LPCTSTR pszFile )
	{
		BOOL bRet = FALSE;

		for ( int i = 0; i < m_whiteFilelist.GetCount(); i++ )
		{
			CWhiteListItem& item = m_whiteFilelist[i];
			if( item.m_filepath == pszFile )
			{
				return TRUE;
				break;
			}
		}

		return bRet;
	}

	/*
	void AddFileToBk( LPCTSTR pszFile )
	{
		CBkWhileListImp imp;
		imp.AddBKWhileList( pszFile );
	}
	*/

	BOOL MsgDeleteFile( CString& strFile, int nType )
	{

		return TRUE;
		/*
		CString strMsg;
		CString strCompress;
		CompressString( strFile,strCompress,URL_WIDTH_IN_COMMONDDLG, nType);

		CBkSafeMsgBox msgbox;
		msgbox.AddButton(L"是", IDOK);
		msgbox.AddButton(L"否", IDCANCEL);
		if( nType == enum_TRUST_ITEM_TYPE_URL )
		{
			strMsg.Format(_T("您确定添加网址 %s 为信任\r\n项吗？(添加后将不再处理此项)"), strCompress);
		}
		else
		{
			strMsg.Format(_T("您确定添加文件 %s 为信任\r\n项吗？(添加后将不再处理此项)"), strCompress);
		}

		UINT_PTR uRet = msgbox.ShowMsg(strMsg, NULL, MB_BK_CUSTOM_BUTTON | MB_ICONWARNING);

		return uRet == IDOK;
		*/
	}

	void CompressString(const CString& strOrig,CString& strDes,int nWidth, int emLinkType)
	{
		strDes = strOrig;

		CClientDC dcClient(m_hWnd);
		HFONT hOldFont = dcClient.SelectFont( BkFontPool::GetFont(BKF_DEFAULTFONT) );
		CSize sizeExtent;
		dcClient.GetTextExtent(strOrig,-1,&sizeExtent);

		if(sizeExtent.cx<nWidth)
			goto Exit0 ;

		if(emLinkType == enum_TRUST_ITEM_TYPE_URL)	//尾部截断型
		{
			CString strCompress = strOrig;
			int nPosRight = strCompress.GetLength()-1;
			int nPosLeft = 0;
			int nPos = nPosRight/2;	

			while((nPosLeft != nPosRight) && (1 != nPosLeft-nPosRight) && (-1 != nPosLeft-nPosRight))
			{
				dcClient.GetTextExtent(strOrig.Left(nPos-3)+L"...",-1,&sizeExtent);

				if(sizeExtent.cx < nWidth)
				{
					nPosLeft = nPos;
					nPos = nPosRight - (nPosRight - nPos)/2;
				}
				else if(sizeExtent.cx > nWidth)
				{
					nPosRight = nPos;
					nPos = nPosLeft + (nPos - nPosLeft)/2;
				}
				else
				{
					nPosLeft = nPos;
					nPosRight = nPos;
				}

			}
			strDes = strOrig.Left(nPosLeft-3)+L"...";
		}
		else if(emLinkType == enum_TRUST_ITEM_TYPE_FILE)		//直接取文件名
		{
			int nStart = strOrig.ReverseFind('\\');
			CString strTemp;
			if(nStart>=0)
			{
				strTemp = strOrig.Right(strOrig.GetLength() - nStart -1);
				//strTemp = strOrig;
			}
			else
			{
				strTemp = strOrig;
			}
			CompressString(strTemp,strDes,FILE_WIDTH_IN_COMMONDDLG, enum_TRUST_ITEM_TYPE_FILE );
		}

Exit0:
		dcClient.SelectFont(hOldFont);
		return ;
	}

	LRESULT OnLVNFileListGetDispInfo(LPNMHDR pnmh)
	{
		NMLVDISPINFO *pdi = (NMLVDISPINFO*)pnmh;
		pdi->item.state = 0;

		if( pdi->item.iItem < 0 || pdi->item.iItem >= m_whiteFilelist.GetCount() )
			return 0;

		CWhiteListItem &log = m_whiteFilelist[ pdi->item.iItem ];

		if (pdi->item.mask & LVIF_TEXT)
		{
			CString strItem;

			switch (pdi->item.iSubItem)
			{
			case 0:
				{
					strItem = log.m_filepath;
				}
				break;
			case 1:
				strItem = TEXT("删除");
				break;
			}

			wcsncpy(pdi->item.pszText, strItem, min(strItem.GetLength() + 1, pdi->item.cchTextMax - 1));
		}

		return 0;
	}

	LRESULT OnLVNUrlListGetDispInfo(LPNMHDR pnmh)
	{
		NMLVDISPINFO *pdi = (NMLVDISPINFO*)pnmh;
		pdi->item.state = 0;

		if( pdi->item.iItem < 0 || pdi->item.iItem >= m_whiteUrllist.GetCount() )
			return 0;

		CWhiteListItem &log = m_whiteUrllist[pdi->item.iItem];

		if (pdi->item.mask & LVIF_TEXT)
		{
			CString strItem;

			switch (pdi->item.iSubItem)
			{
			case 0:
				{
					strItem = log.m_filepath;
				}
				break;
			case 1:
				strItem = TEXT("删除");
				break;
			}

			wcsncpy(pdi->item.pszText, strItem, min(strItem.GetLength() + 1, pdi->item.cchTextMax - 1));
		}

		return 0;
	}

	LRESULT OnLVNScriptListGetDispInfo(LPNMHDR pnmh)
	{
		NMLVDISPINFO *pdi = (NMLVDISPINFO*)pnmh;
		pdi->item.state = 0;

		if( pdi->item.iItem < 0 || pdi->item.iItem >= m_whiteScriptlist.GetCount() )
			return 0;

		CWhiteListItem &log = m_whiteScriptlist[pdi->item.iItem];

		if (pdi->item.mask & LVIF_TEXT)
		{
			CString strItem;

			switch (pdi->item.iSubItem)
			{
			case 0:
				{
					GetScriptName( log.m_filepath, strItem );
				}
				break;
			case 1:
				strItem = TEXT("删除");
				break;
			}

			wcsncpy(pdi->item.pszText, strItem, min(strItem.GetLength() + 1, pdi->item.cchTextMax - 1));
		}

		return 0;
	}

	void GetScriptName( CString &str, CString& strName )
	{
		int nPos = str.Find( VS_SCRIPT_FLAG );
		strName = str.Mid( nPos + _tcslen(VS_SCRIPT_FLAG) );
	}

	void GetScriptGUID( CString &str, CString& strGuid )
	{
		int nPos = str.Find( VS_SCRIPT_FLAG );
		strGuid = str.Left( nPos );
	}

    void DelFileFromBk( LPCTSTR pszFile )
    {
		CBkWhileListImp imp;
		imp.DelBKWhileList( pszFile );
    }


    void OnBkBtnClose()
    {
        EndDialog(IDCANCEL);
    }

    void OnBtnVirScanClearLog()
    {
        ::SetCursor(::LoadCursor(NULL, IDC_WAIT));

        HANDLE hThread = ::CreateThread(NULL, 0, _ClearLogThreadProc, m_hWnd, 0, NULL);
        ::CloseHandle(hThread);
        hThread = NULL;
    }

    BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
    {
		InitCtrls();

        _EnableButtons(FALSE);

        HANDLE hThread = ::CreateThread(NULL, 0, _LoadLogThreadProc, this, 0, NULL);
        ::CloseHandle(hThread);
        hThread = NULL;

        return TRUE;
    }


	void InitCtrls()
	{
		m_wndFileList.SetRowHeight(28);
		m_wndFileList.Create(
			GetViewHWND(), NULL, NULL, 
			WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA |LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 
			0, IDC_LST_LIST);

		m_wndFileList.HeaderSubclassWindow();
		m_wndFileList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER , 
			LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER );

		m_wndFileList.InsertColumn(0, L"信任文件",	LVCFMT_LEFT, 395);
		m_wndFileList.InsertColumn(1, L"操作",		LVCFMT_CENTER, 60);
		m_wndFileList.SetDataPrt( &m_whiteFilelist );
		m_wndFileList.SetNotifyWnd( m_hWnd );

		//////////////////////////////////////////////////////////////////////////
		m_wndScriptList.SetRowHeight(28);
		m_wndScriptList.Create(
			GetViewHWND(), NULL, NULL, 
			WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA |LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 
			0, IDC_LST_SCRIPT_LIST );

		m_wndScriptList.HeaderSubclassWindow();
		m_wndScriptList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER , 
			LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER );

		m_wndScriptList.InsertColumn(0, L"信任修复项",	LVCFMT_LEFT, 395);
		m_wndScriptList.InsertColumn(1, L"操作",		LVCFMT_CENTER, 60);
		m_wndScriptList.SetDataPrt( &m_whiteScriptlist );
		m_wndScriptList.SetNotifyWnd( m_hWnd );
		//////////////////////////////////////////////////////////////////////////

		m_wndUrlList.SetRowHeight(28);
		m_wndUrlList.Create(
			GetViewHWND(), NULL, NULL, 
			WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA |LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 
			0, IDC_LST_URL_LIST);

		m_wndUrlList.HeaderSubclassWindow();
		m_wndUrlList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER , 
			LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER );

		m_wndUrlList.InsertColumn(0, L"信任网址",	LVCFMT_LEFT, 395);
		m_wndUrlList.InsertColumn(1, L"操作",		LVCFMT_CENTER, 60);
		m_wndUrlList.SetDataPrt( &m_whiteUrllist );
		m_wndUrlList.SetNotifyWnd( m_hWnd );

		m_wndEdit.Create( GetViewHWND(), 
			NULL,
			NULL,
			WS_VISIBLE | WS_CHILDWINDOW | ES_AUTOHSCROLL | ES_MULTILINE,
			0,
			IDC_EDIT_URL);
		m_wndEdit.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));

		SetTabCurSel(IDC_LOG_TAB_CTRL, m_nCurPage);
	}

    

    LRESULT OnLoadLogFinish(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        m_wndFileList.SetItemCount(m_whiteFilelist.GetCount());
		m_wndUrlList.SetItemCount(m_whiteUrllist.GetCount());
		m_wndScriptList.SetItemCount( m_whiteScriptlist.GetCount() );

		SetItemVisible( IDC_DIV_WAITTING, FALSE );
		SetItemVisible( IDC_DIV_WHILE_LIST, TRUE );
		SetItemVisible( IDC_DIV_WHILE_URL_LIST, TRUE );
		SetItemVisible( IDC_DIV_BTN_AND_TEXT, TRUE );
		SetItemVisible( IDC_DIV_CTRLS, TRUE );
		EnableItem( IDC_LBL_WHITELIST_ADD_URL, FALSE );

        _EnableButtons(TRUE);

        return 0;
    }

    void _EnableButtons(BOOL bEnable)
    {
        EnableItem(IDC_LBL_WHITELIST_ADD_FILE, bEnable);
        EnableItem(IDC_BTN_SYS_CLOSE, bEnable);
    }

	void DoLoadLog()
	{
		KLogic logic;
		std::vector<std::wstring> vecFile;
		logic.QueryUserTrustList( enum_TRUST_ITEM_TYPE_FILE, vecFile );
		for ( int i = 0; i < vecFile.size(); i++ )
		{
			CWhiteListItem wli;
			wli.m_filepath = vecFile[i].c_str();
			m_whiteFilelist.Add( wli );
		}

		std::vector<std::wstring> vecUrl;
		logic.QueryUserTrustList( enum_TRUST_ITEM_TYPE_URL, vecUrl );
		for ( int i = 0; i < vecUrl.size(); i++ )
		{
			CWhiteListItem wli;
			wli.m_filepath = vecUrl[i].c_str();
			m_whiteUrllist.Add( wli );
		}

		std::vector<std::wstring> vecScript;
		logic.QueryUserTrustList( enum_TRUST_ITEM_TYPE_SCRIPT, vecScript );
		for ( int i = 0; i < vecScript.size(); i++ )
		{
			CWhiteListItem wli;
			wli.m_filepath = vecScript[i].c_str();
			m_whiteScriptlist.Add( wli );
		}

		SendMessage( MSG_LOAD_LOG_FINISH, NULL, NULL );
	}

    static DWORD WINAPI _LoadLogThreadProc(LPVOID pvParam)
    {
		CBeikeSafeVirScanWhiteListDlg* pThis = (CBeikeSafeVirScanWhiteListDlg*)pvParam;        
	
		if( pThis )
			pThis->DoLoadLog();

        return 0;
    }

    static DWORD WINAPI _ClearLogThreadProc(LPVOID pvParam)
    {
        HWND hWndNotify = (HWND)pvParam;

        if (::IsWindow(hWndNotify))
            ::SendMessage(hWndNotify, MSG_CLEAR_LOG_FINISH, 0, 0);

        return 0;
    }

	LRESULT OnDeleteListItem(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		HWND hWnd = (HWND)lParam;
		int nItem = (int)wParam;
		if( hWnd == m_wndFileList.m_hWnd )
		{
			if( nItem >= 0 && nItem < m_whiteFilelist.GetCount() )
			{
				if( MsgDeleteFile(m_whiteFilelist[nItem].m_filepath, enum_TRUST_ITEM_TYPE_FILE ) )
				{
					m_wndFileList.DeleteItem( nItem );
					std::wstring strFile = m_whiteFilelist[nItem].m_filepath;
					m_whiteFilelist.RemoveAt( nItem );
					DeleteTrustItem( strFile, enum_TRUST_ITEM_TYPE_FILE );
					DelFileFromBk( strFile.c_str() );
				}
				
			}
		}
		else if( hWnd == m_wndUrlList.m_hWnd )
		{
			if( nItem >= 0 && nItem < m_whiteUrllist.GetCount() )
			{
				if( MsgDeleteFile(m_whiteUrllist[nItem].m_filepath, enum_TRUST_ITEM_TYPE_URL ) )
				{
					m_wndUrlList.DeleteItem( nItem );
					std::wstring strFile = m_whiteUrllist[nItem].m_filepath;
					DeleteTrustItem( strFile, enum_TRUST_ITEM_TYPE_URL );
					m_whiteUrllist.RemoveAt( nItem );
				}
			}
		}
		else if ( hWnd == m_wndScriptList.m_hWnd )
		{
			if( nItem >= 0 && nItem < m_whiteScriptlist.GetCount() )
			{
				if( MsgDeleteFile(m_whiteScriptlist[nItem].m_filepath, enum_TRUST_ITEM_TYPE_SCRIPT ) )
				{
					m_wndScriptList.DeleteItem( nItem );
					//CString strGuid;
					//GetScriptGUID( , strGuid );
					std::wstring strFile = m_whiteScriptlist[nItem].m_filepath;
					DeleteTrustItem( strFile, enum_TRUST_ITEM_TYPE_SCRIPT );
					m_whiteScriptlist.RemoveAt( nItem );
				}
			}
		}
		return 0;
	}

	void DeleteTrustItem( std::wstring& strItem, EM_TRUST_ITEM_TYPE nType )
	{
		KLogic logic;
		S_TRUST_LIST trustList;
		trustList.operation = enum_TRUST_LIST_DELETE;
		trustList.itemType = nType;
		trustList.vecItemList.push_back( strItem );
		HRESULT hr = E_FAIL;
		hr = logic.SetUserTrustList( trustList );

		CSafeMonitorTrayShell::WhiteListLibUpdated();
	}

	void AddTrustItem( std::wstring& strItem, EM_TRUST_ITEM_TYPE nType )
	{
		KLogic logic;
		S_TRUST_LIST trustList;
		trustList.operation = enum_TRUST_LIST_ADD;
		trustList.itemType = nType;
		trustList.vecItemList.push_back( strItem );
		logic.SetUserTrustList( trustList );

		CSafeMonitorTrayShell::WhiteListLibUpdated();
	}

	LRESULT OnEditChange(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
	{
		if( m_wndEdit.IsWindow() )
		{
			ATL::CString strUrl;
			m_wndEdit.GetWindowText( strUrl );
			EnableItem( IDC_LBL_WHITELIST_ADD_URL, strUrl.GetLength() > 0 );
		}

		return TRUE;
	}

	BOOL OnBkTabCtrlChange( int nTabItemIDOld, int nTabItemIDNew )
	{
		//file
		if( nTabItemIDNew == 0 )
		{
			SetItemVisible( IDC_LBL_WHITELIST_ADD_FILE, TRUE );
			SetItemVisible( IDC_DIV_ADD_URL, FALSE );
		} // url
		else if ( nTabItemIDNew == 1 )
		{
			SetItemVisible( IDC_LBL_WHITELIST_ADD_FILE, FALSE );
			SetItemVisible( IDC_DIV_ADD_URL, TRUE );
		} // script
		else if ( nTabItemIDNew == 2 )
		{
			SetItemVisible( IDC_LBL_WHITELIST_ADD_FILE, FALSE );
			SetItemVisible( IDC_DIV_ADD_URL, FALSE );
		}

		return TRUE;
	}

	

public:

    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE,				OnBkBtnClose)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_VIRSCAN_CLEAR_LOG,		OnBtnVirScanClearLog)
        BK_NOTIFY_ID_COMMAND(IDCANCEL,						OnBkBtnClose)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_WHITELIST_ADD_FILE,	OnLblWhiteListAddFile)
        BK_NOTIFY_ID_COMMAND(IDC_LBL_WHITELIST_ADD_URL,		OnLblWhiteListAddUrl)
		BK_NOTIFY_TAB_SELCHANGE(IDC_LOG_TAB_CTRL,			OnBkTabCtrlChange)
    BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBeikeSafeVirScanWhiteListDlg)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CBkDialogImpl<CBeikeSafeVirScanWhiteListDlg>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBeikeSafeVirScanWhiteListDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
		NOTIFY_HANDLER_EX(IDC_LST_LIST,			LVN_GETDISPINFO,		OnLVNFileListGetDispInfo)
		NOTIFY_HANDLER_EX(IDC_LST_URL_LIST,		LVN_GETDISPINFO,		OnLVNUrlListGetDispInfo)
		NOTIFY_HANDLER_EX(IDC_LST_SCRIPT_LIST,	LVN_GETDISPINFO,		OnLVNScriptListGetDispInfo)
        MESSAGE_HANDLER_EX(MSG_LOAD_LOG_FINISH,						OnLoadLogFinish)
		MESSAGE_HANDLER_EX(WM_ITEM_BUTTON_CLICK,					OnDeleteListItem);
		COMMAND_HANDLER(IDC_EDIT_URL, EN_CHANGE,				OnEditChange)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};