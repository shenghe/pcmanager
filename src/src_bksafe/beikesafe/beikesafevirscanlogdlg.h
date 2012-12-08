#pragma once

#include <wtlhelper/whwindow.h>
#include "kws/ctrls/loglistctrl.h"
#include <comproxy/bkscanfile.h>
#include <winmod/winpath.h>
#include <common/whitelist.h>
#include <comproxy/bkutility.h>
#include <atlcore.h>
#include <atluser.h>
#include "virusscan/kaquaraloader.h"

#define MSG_LOAD_LOG_FINISH     (WM_APP + 1)
#define MSG_CLEAR_LOG_FINISH    (WM_APP + 2)
#define WM_MSG_LOG_RECOVER		(WM_USER+ 678)

class CBeikeSafeVirScanLogDlg
    : public CBkDialogImpl<CBeikeSafeVirScanLogDlg>
    , public CWHRoundRectFrameHelper<CBeikeSafeVirScanLogDlg>
{
public:
    CBeikeSafeVirScanLogDlg()
        : CBkDialogImpl<CBeikeSafeVirScanLogDlg>(IDR_BK_VIRSCAN_LOG_DLG)
		, m_bShowFileTab(FALSE)
		, m_bDoing(FALSE)
		, m_hLoadLog(NULL)
    {
    }
	S_REMOVE_HISTORY_RECORD_OPTION				m_kwsRemove;
	std::vector<std::wstring>					m_vecRemove;

	void SetShowFileTab( BOOL bFileTab )
	{
		m_bShowFileTab = bFileTab;
	}
	

protected:

	CAtlArray< KWS_SCAN_LOG >			m_arrLogs;
	CAtlArray< KWS_SCAN_LOG >			m_arrFileLogs;
    CLogListCtrl						m_wndListLog;
	CLogListCtrl						m_wndFileLog;
	BOOL								m_bShowFileTab;
	BOOL								m_bDoing;
	KaquaraLoader						m_quaLoader;
	HANDLE								m_hLoadLog;
	KLogic								m_logic;

    void OnBkBtnClose()
    {
		if( !m_bDoing )
			EndDialog(IDCANCEL);
    }


	BOOL OnBkTabCtrlChange( int nTabItemIDOld, int nTabItemIDNew )
	{
		SetItemVisible( IDC_LST_VIRSCAN_LOG,  nTabItemIDNew == 0 );
		SetItemVisible( IDC_ALL_LOG_TEXT,	nTabItemIDNew == 0 );

		SetItemVisible( IDC_FILE_LOG_TEXT,  nTabItemIDNew == 1 );
		SetItemVisible( IDC_FILE_VIRUS_LOG_LIST, nTabItemIDNew == 1 );
		UpdateBtnAndCheck( );		
		return TRUE;
	}

	void UpdateBtnAndCheck( )
	{
		if( m_wndListLog.IsWindowVisible() )
		{
			EnableItem( IDC_BTN_VIRSCAN_CLEAR_LOG, m_wndListLog.IsAnyItemChecked() );
			SetItemCheck( IDC_VIRUS_LOG_CHECK_ALL, m_wndListLog.IsAllItemChecked() );
		}
		else if ( m_wndFileLog.IsWindowVisible() )
		{
			EnableItem( IDC_BTN_VIRSCAN_CLEAR_LOG, m_wndFileLog.IsAnyItemChecked() );
			SetItemCheck( IDC_VIRUS_LOG_CHECK_ALL, m_wndFileLog.IsAllItemChecked() );
		}
	}


    void OnBtnVirScanClearLog()
    {
		CBkSafeMsgBox2 dlg;
		dlg.AddButton(TEXT("删除记录"), IDOK);
		dlg.AddButton(TEXT("取消"),		IDCANCEL);
		UINT nID = dlg.ShowMutlLineMsg( L"删除记录后将无法再执行还原操作，\r\n\r\n您是否确定删除选中的查杀历史记录？", 
			_T("提示"), MB_BK_CUSTOM_BUTTON|MB_ICONINFORMATION, NULL, GetViewHWND() );
		if( nID == IDOK )
		{
			::SetCursor(::LoadCursor(NULL, IDC_WAIT));

			MakeRemoveList( );

			_EnableButtons(FALSE);
			
			m_hLoadLog = ::CreateThread(NULL, 0, _ClearLogThreadProc, this, 0, NULL);
		}
    }

	void MakeRemoveList( )
	{
		CLogListCtrl* pList = NULL;
		CAtlArray< KWS_SCAN_LOG >* pLog = NULL;
		m_vecRemove.clear();

		if( m_wndListLog.IsWindowVisible() )
		{
			pList = &m_wndListLog;
			pLog = &m_arrLogs;
		}
		else
		{
			pList = &m_wndFileLog;
			pLog = &m_arrFileLogs;
		}	

		m_kwsRemove.bClearAll = FALSE;
		for ( int i = pLog->GetCount() - 1; i >= 0; i-- )
		{
			KWS_SCAN_LOG& log = (*pLog)[i];
			
			if( pList->GetCheckState( i ) )
			{
				if( log.bKwsLog )
				{
					S_HISTORY_RECORD_ITEM item;
					item.dwItemID = (*pLog)[i].dwID;
					item.tmTime = (*pLog)[i].tmHistoryTime;
					m_kwsRemove.vecRecords.push_back( item );
				}
				else
				{
					std::wstring strFile( log.szQuaraName );
					m_vecRemove.push_back( strFile );
				}

				RemoveOtherListView( log );

				pLog->RemoveAt( i );
				pList->DeleteItem( i );
			}
		}
	}

	void RemoveOtherListView( KWS_SCAN_LOG& log )
	{
		CLogListCtrl* pOtherList = NULL;
		CAtlArray< KWS_SCAN_LOG >* pOtherLog = NULL;

		if( !m_wndListLog.IsWindowVisible() )
		{
			pOtherList = &m_wndListLog;
			pOtherLog = &m_arrLogs;
		}
		else
		{
			pOtherList = &m_wndFileLog;
			pOtherLog = &m_arrFileLogs;
		}

		for ( int i = pOtherLog->GetCount() - 1; i >= 0; i-- )
		{
			if( log.bKwsLog )
			{
				if( log.tmHistoryTime == (*pOtherLog)[i].tmHistoryTime &&
					log.dwID == (*pOtherLog)[i].dwID )
				{
					pOtherList->DeleteItem( i );
					pOtherLog->RemoveAt( i );
					break;
				}
			}
			else
			{
				UINT uId = (*pOtherLog)[i].dwID;
				if( log.dwID == uId )
				{
					pOtherList->DeleteItem( i );
					pOtherLog->RemoveAt( i );
					break;
				}
			}
		}
	}

	void QuaraToLog( QUAR_FILE_INFO& bklog, KWS_SCAN_LOG& log )
	{
		log.emType = enum_BK_TROJAN_Point;
		log.ActionType = enum_PROCESS_TYPE_CLEAN;
		log.state = enum_HISTORY_ITEM_STATE_FIXED;
		log.emLevel = enum_POINT_LEVEL_TROJAN;

		CTime time( bklog.stSTime);
		log.tmHistoryTime = time.GetTime();
		log.Advice = enum_PROCESS_TYPE_FIX;
		log.bKwsLog = FALSE;
		log.dwID = bklog.uIndex;
		log.uSize = sizeof( KWS_SCAN_LOG );
		_tcsncpy_s( log.szItemName, MAX_PATH*2, bklog.szSurFileName, _TRUNCATE);
		_tcsncpy_s( log.szVirusName, MAX_PATH*2, bklog.szVirusName, _TRUNCATE);
		_tcsncpy_s( log.szQuaraName, MAX_PATH*2, bklog.szQuarFileName, _TRUNCATE);
	}

	void KwsLogToLog( S_HISTORY_ITEM_EX& item )
	{
		for( int i = 0; i < item.FixedList.size(); i++ )
		{
			S_HISTORY_ITEM_INFO& info = item.FixedList[i];
			KWS_SCAN_LOG log;
			memset( &log, 0, sizeof(KWS_SCAN_LOG) );
			log.emType = info.emType;
			log.emLevel = info.emLevel;
			log.ActionType = info.ActionType;
			log.state = info.state;
			log.tmHistoryTime = item.tmHistoryTime;
			log.Advice = info.Advice;
			log.bKwsLog = TRUE;
			log.dwID = info.dwID;
			log.uSize = sizeof( KWS_SCAN_LOG );

			//加入全部历史
			if( info.emLevel == enum_POINT_LEVEL_REG_REMAIN )
			{
				if( info.strItemName.size() == 0 )
					_tcsncpy_s( log.szItemName, MAX_PATH*2, TEXT("[(Default)]"), _TRUNCATE);
				else
					_tcsncpy_s( log.szItemName, MAX_PATH*2, info.strItemName.c_str(), _TRUNCATE);
			}
			else
				_tcsncpy_s( log.szItemName, MAX_PATH*2, info.strItemName.c_str(), _TRUNCATE);

			if( info.vecProcessItems.size() > 0 && m_logic.IsRealVirus( info.emType, info.emLevel ) )
			{
				for ( int j = 0; j < info.vecProcessItems.size(); j++ )
				{	
					S_PROCESS_OBJECT_INFO& objInfo = info.vecProcessItems[j];
					if( (objInfo.processType == enum_PROCESS_AT_BOOT_DELETE_FILE || 
						objInfo.processType == enum_PROCESS_AT_BOOT_CLEAN_INFECTED_FILE ) )
					{
						_tcsncpy_s( log.szVirusName, MAX_PATH*2, objInfo.strVirusName.c_str(), _TRUNCATE);
						_tcsncpy_s( log.szItemName, MAX_PATH*2, objInfo.strInfo_1.c_str(), _TRUNCATE);
						break;
					}
				}
			}
			SortAfterAdd( m_arrLogs, log );

			//加入文件隔离
			for ( int j = 0; j < info.vecProcessItems.size(); j++ )
			{	
				S_PROCESS_OBJECT_INFO& objInfo = info.vecProcessItems[j];
				if( (objInfo.processType == enum_PROCESS_AT_BOOT_DELETE_FILE || 
					 objInfo.processType == enum_PROCESS_AT_BOOT_CLEAN_INFECTED_FILE )
					 && info.emLevel != enum_POINT_LEVEL_REG_REMAIN )
				{
					wmemset( log.szItemName, 0, MAX_PATH*2 );
					wmemset( log.szVirusName, 0, MAX_PATH*2 );
					_tcsncpy_s( log.szItemName, MAX_PATH*2, objInfo.strInfo_1.c_str(),	_TRUNCATE);

					if( objInfo.strVirusName.size() > 0 )
						_tcsncpy_s( log.szVirusName, MAX_PATH*2, objInfo.strVirusName.c_str(), _TRUNCATE);
					else
						_tcsncpy_s( log.szVirusName, MAX_PATH*2, TEXT("可疑文件"), _TRUNCATE);

					SortAfterAdd( m_arrFileLogs, log );
				}
			}
		}
	}

    BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
    {
		m_wndListLog.SetRowHeight(28);
        m_wndListLog.Create(
            GetViewHWND(), NULL, NULL, 
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA |LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 
            0, IDC_LST_VIRSCAN_LOG);

		m_wndListLog.SetLinkIndex( 5 );
		m_wndListLog.HeaderSubclassWindow();
		m_wndListLog.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER , 
												LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER );
		m_wndListLog.ShowCheckBox( TRUE );
		m_wndListLog.InsertColumn(0, L"时间",		LVCFMT_CENTER,	150);
		m_wndListLog.InsertColumn(1, L"异常项",		LVCFMT_LEFT,	225);
		m_wndListLog.InsertColumn(2, L"类型",		LVCFMT_LEFT,	115);
		m_wndListLog.InsertColumn(3, L"级别",		LVCFMT_LEFT,	55);
		m_wndListLog.InsertColumn(4, L"处理方式",	LVCFMT_CENTER,	60);
		m_wndListLog.InsertColumn(5, L"操作",		LVCFMT_LEFT,	55);


		m_wndFileLog.SetRowHeight(28);
		m_wndFileLog.Create(
			GetViewHWND(), NULL, NULL, 
			WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA |LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 
			0, IDC_FILE_VIRUS_LOG_LIST);
		m_wndFileLog.HeaderSubclassWindow();
		m_wndFileLog.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER , 
										LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER );
		m_wndFileLog.ShowCheckBox( TRUE );
		m_wndFileLog.SetLinkIndex( 4 );
		m_wndFileLog.InsertColumn(0, L"时间",		LVCFMT_CENTER,	150);
		m_wndFileLog.InsertColumn(1, L"文件路径",	LVCFMT_LEFT,	280);
		m_wndFileLog.InsertColumn(2, L"病毒名称",	LVCFMT_LEFT,	115);
		m_wndFileLog.InsertColumn(3, L"处理方式",	LVCFMT_CENTER,	60);
		m_wndFileLog.InsertColumn(4, L"操作",		LVCFMT_LEFT,	55);

        _EnableButtons(FALSE);

		m_quaLoader.Init();

        HANDLE hThread = ::CreateThread(NULL, 0, _LoadLogThreadProc, this, 0, NULL);
        ::CloseHandle(hThread);
        hThread = NULL;

        return TRUE;
    }

    LRESULT OnLVNVirScanLogGetDispInfo(LPNMHDR pnmh)
    {
        NMLVDISPINFO *pdi = (NMLVDISPINFO*)pnmh;
		pdi->item.state = 0;

		if( pdi->item.iItem < 0 || pdi->item.iItem >= m_arrLogs.GetCount() )
			return 0;

        KWS_SCAN_LOG &log = m_arrLogs[ pdi->item.iItem ];

        if (pdi->item.mask & LVIF_TEXT)
        {
            CString strItem;
			CString strFileName;
			CString strVirusName;
			
			strFileName = log.szItemName;
			strVirusName = log.szVirusName;
            switch (pdi->item.iSubItem)
            {
            case 0:
                {
					CTime time( log.tmHistoryTime );
					strItem.Format(
						L"%4d-%02d-%02d %02d:%02d:%02d", 
						time.GetYear(),
						time.GetMonth(),
						time.GetDay(),
						time.GetHour(),
						time.GetMinute(),
						time.GetSecond()
						);
                }
                break;
			case 1:
				{
					WinMod::CWinPath path( strFileName );
					strItem = path.GetPathWithoutUnicodePrefix();
				}
				break;
			case 2:
				{
					
					m_logic.GetTypeStringEx( log.emType, log.emLevel, strVirusName, strItem );
				}
				break;
			case 3:
				if( log.bKwsLog )
				{
					m_logic.GetLevelString( log.emLevel, strItem );
				}
				else
				{
					strItem = L"危险";
				}
				break;
			case 4:
				if( log.bKwsLog )
				{
					m_logic.GetActionString( log.ActionType, strItem );
				}
				else
				{
					strItem = L"清除";
				}
				break;

			case 5:
				switch( log.state )
				{
				case enum_HISTORY_ITEM_STATE_FIXED:
					strItem = L"还原";
					break;
				case enum_HISTORY_ITEM_STATE_RESTORED:
					strItem = L"已还原";
					break;
				case enum_HISTORY_ITEM_STATE_CANNOT_BE_RESTORED:
					strItem = L"不能还原";
					break;
				case enum_HISTORY_ITEM_STATE_INVALID:
					strItem = L"还原失败";
					break;
				default:
					strItem = L"还原";
					break;
				}
				break;
            }

            wcsncpy(pdi->item.pszText, strItem, min(strItem.GetLength() + 1, pdi->item.cchTextMax - 1));
        }

        return 0;
    }

	LRESULT OnLVNVirScanLogFileGetDispInfo(LPNMHDR pnmh)
	{
		NMLVDISPINFO *pdi = (NMLVDISPINFO*)pnmh;
		pdi->item.state = 0;

		if( pdi->item.iItem < 0 || pdi->item.iItem >= m_arrFileLogs.GetCount() )
			return 0;

		KWS_SCAN_LOG &log = m_arrFileLogs[ pdi->item.iItem ];

		if (pdi->item.mask & LVIF_TEXT)
		{
			CString strItem;
			CString strFileName = log.szItemName ;
			CString strVirusName = log.szItemName;

			switch (pdi->item.iSubItem)
			{
			case 0:
				{
					CTime time( log.tmHistoryTime );
					strItem.Format(
						L"%4d-%02d-%02d %02d:%02d:%02d", 
						time.GetYear(),
						time.GetMonth(),
						time.GetDay(),
						time.GetHour(),
						time.GetMinute(),
						time.GetSecond()
						);
				}
				break;
			case 1:
				{
					WinMod::CWinPath path( strFileName );
					strItem = path.GetPathWithoutUnicodePrefix();
				}
				break;
			case 2:
				strItem = log.szVirusName;
				break;
			case 3:
				if( log.bKwsLog )
				{
					m_logic.GetActionString( log.ActionType, strItem );
				}
				else
				{
					strItem = L"清除";
				}
				break;

			case 4:
				switch( log.state )
				{
				case enum_HISTORY_ITEM_STATE_FIXED:
					strItem = L"还原";
					break;
				case enum_HISTORY_ITEM_STATE_RESTORED:
					strItem = L"已还原";
					break;
				case enum_HISTORY_ITEM_STATE_CANNOT_BE_RESTORED:
					strItem = L"不能还原";
					break;
				case enum_HISTORY_ITEM_STATE_INVALID:
					strItem = L"还原失败";
					break;
				default:
					strItem = L"还原";
					break;
				}
				break;
			}

			wcsncpy(pdi->item.pszText, strItem, min(strItem.GetLength() + 1, pdi->item.cchTextMax - 1));
		}

		return 0;
	}

	LRESULT OnLoadLogFinish(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_wndListLog.SetItemCount(m_arrLogs.GetCount());
		m_wndListLog.SetDataPrt( &m_arrLogs );

		m_wndFileLog.SetItemCount( m_arrFileLogs.GetCount());
		m_wndFileLog.SetDataPrt( &m_arrFileLogs );

		if( m_bShowFileTab == TRUE )
		{
			SetItemVisible(IDC_FILE_VIRUS_LOG_LIST, TRUE);
			SetItemVisible(IDC_FILE_LOG_TEXT,		TRUE );
			SetItemVisible(IDC_ALL_LOG_TEXT,		FALSE );
			SetTabCurSel( IDC_VIRUS_LOG_TAB_CTRL,	1 );
		}
		else
		{
			SetItemVisible(IDC_LST_VIRSCAN_LOG,		TRUE);
			SetItemVisible(IDC_ALL_LOG_TEXT,		TRUE );
			SetItemVisible(IDC_FILE_LOG_TEXT,		FALSE );
			SetTabCurSel( IDC_VIRUS_LOG_TAB_CTRL,	0 );
		}

		_EnableButtons(TRUE);

		EnableItem( IDC_BTN_VIRSCAN_CLEAR_LOG,	FALSE);

		return 0;
	}

	int GetFileLogCount()
	{
		int nCnt = 0;

	}

    LRESULT OnClearLogFinish(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        _EnableButtons(TRUE);
		UpdateBtnAndCheck( );
        return 0;
    }

    void _EnableButtons(BOOL bEnable)
    {
		m_bDoing = !bEnable;
        EnableItem(IDC_BTN_SYS_CLOSE, bEnable);
		EnableItem(IDC_BTN_SYS_MAX, bEnable );
        EnableItem(IDCANCEL, bEnable);
        EnableItem(IDC_BTN_VIRSCAN_CLEAR_LOG, bEnable );
    }

	void SortAfterAdd( CAtlArray< KWS_SCAN_LOG >& arrLog, KWS_SCAN_LOG& logItem )
	{
		BOOL bFind = FALSE;
		if( arrLog.GetCount() == 0 )
		{
			arrLog.Add( logItem );
			bFind = TRUE;
		}
		else
		{
			for ( int i = 0; i < arrLog.GetCount(); i++ )
			{
				if( logItem.tmHistoryTime > arrLog[i].tmHistoryTime )
				{
					arrLog.InsertAt( i, logItem );
					bFind = TRUE;
					break;
				}
			}
		}

		if( bFind == FALSE )
		{
			arrLog.Add( logItem );
		}
	}

    static DWORD WINAPI _LoadLogThreadProc(LPVOID pvParam)
    {
		CBeikeSafeVirScanLogDlg* pDlg = (CBeikeSafeVirScanLogDlg*)pvParam;
		if( pDlg )
		{
			pDlg->DoLoadLogThread();
		}

		return 0;
	}

	void DoLoadLogThread()
	{
		CAtlArray<KWS_SCAN_LOG> arrKwsLog;
		
		VEC_QUAR vecFile;
		m_quaLoader.LoadQuara( vecFile ); 

		for ( int i = 0 ; i < vecFile.size(); i++ )
		{
			KWS_SCAN_LOG kwsLog;
			memset( &kwsLog, 0, sizeof(KWS_SCAN_LOG) );
			QuaraToLog( vecFile[i], kwsLog );
			SortAfterAdd( m_arrLogs,		kwsLog );
			SortAfterAdd( m_arrFileLogs,	kwsLog );
		}

		std::vector<S_HISTORY_ITEM_EX> HisList;
		m_logic.QueryHistoryListEx( 10, HisList );
		for ( int i = 0; i < HisList.size(); i++ )
		{
			KwsLogToLog( HisList[i] );
		}

		if ( IsWindow())
			SendMessage( MSG_LOAD_LOG_FINISH, NULL, NULL );

		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}

	//
	BOOL InIsoRegion( UINT nCleanResult )
	{
		switch( nCleanResult )
		{
		case BKENG_CLEAN_RESULT_DISINFECT:
		case BKENG_CLEAN_RESULT_DISINFECT_NEED_REBOOT:
		case BKENG_CLEAN_RESULT_DELETE_NEED_REBOOT:
		case BKENG_CLEAN_RESULT_DELETE:
		case BKENG_CLEAN_RESULT_FAILED_TO_DELETE:
		case BKENG_CLEAN_RESULT_FAILED_TO_DISINFECT:
		case BKENG_CLEAN_RESULT_NEED_DELETE:
		case BKENG_CLEAN_RESULT_DISINFECT_NEED_REPLACE:
			return TRUE;
			break;
		default:
			return FALSE;
		}

		return FALSE;
	}

    static DWORD WINAPI _ClearLogThreadProc(LPVOID pvParam)
    {      
		CBeikeSafeVirScanLogDlg* pDlg = (CBeikeSafeVirScanLogDlg*)pvParam;
		if( pDlg )
		{	
			pDlg->_CleanKXELog();
			pDlg->_ClearKwsLog();

			if (pDlg->IsWindow())
				pDlg->SendMessage(MSG_CLEAR_LOG_FINISH, 0, 0);
		}

        return 0;
    }

	void _ClearKwsLog()
	{
		if( m_kwsRemove.vecRecords.size() > 0 )
		{
			m_logic.RemoveHistoryRecord( m_kwsRemove );
			m_kwsRemove.vecRecords.clear();
		}
	}

	void _CleanKXELog()
	{
		m_quaLoader.DeleteQuarantineFile( m_vecRemove );
	}

	LRESULT OnRemoveQuarantineItemNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		int nIndex = (int)wParam;
		HRESULT hRet = (HRESULT)lParam;

		if ( hRet != S_OK)
		{
			if( nIndex < m_arrLogs.GetCount() )
			{
				m_arrLogs[nIndex].state = enum_HISTORY_ITEM_STATE_INVALID;
				m_wndListLog.RedrawItems( nIndex, nIndex );
			}
			else
			{
				ATLASSERT( FALSE );
			}
		}

		return 0;
	}
	
	LRESULT OnItemClick(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL bHandler)
	{

		::PostMessage( m_hWnd, WM_MSG_LOG_RECOVER, wParam, lParam );
		return TRUE;
	}

	LRESULT OnItemCheckbox(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL bHandler)
	{
		int nIndex = (int)wParam;

		if( !m_wndListLog.IsWindowVisible() )
		{
			SetTheSameLogCheck( nIndex );
		}
		
		UpdateBtnAndCheck( );
		return TRUE;
	}

	void SetTheSameLogCheck( int nIndex )
	{
		CAtlArray< KWS_SCAN_LOG >* pCurLog = NULL;
		CLogListCtrl* pCurList = NULL;

		if( m_wndListLog.IsWindowVisible() )
		{
			pCurLog = &m_arrLogs;
			pCurList = &m_wndListLog;
		}
		else
		{
			pCurLog = &m_arrFileLogs;
			pCurList = &m_wndFileLog;
		}

		BOOL bCheck = pCurList->GetCheckState( nIndex );

		if( nIndex < pCurLog->GetCount() && nIndex >= 0 )
		{
			KWS_SCAN_LOG& log = pCurLog->GetAt( nIndex );
			if( log.bKwsLog )
			{
				for ( int i = 0; i < pCurLog->GetCount(); i++ )
				{
					KWS_SCAN_LOG& item = pCurLog->GetAt( i );
					if ( item.bKwsLog && (item.dwID == log.dwID) && ( log.tmHistoryTime == item.tmHistoryTime ) )
					{
						pCurList->SetCheckState( i, bCheck );
					}
				}
			}
			else
			{
				for ( int i = 0; i < pCurLog->GetCount(); i++ )
				{
					KWS_SCAN_LOG& item = pCurLog->GetAt( i );
					if ( !item.bKwsLog && (item.dwID == log.dwID ) )
					{
						pCurList->SetCheckState( i, bCheck );
					}
				}
			}
		}
	}

	LRESULT OnRecover(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL bHandler)
	{
		int nItem = (int)wParam;
		if(nItem<0) return -1;

		CAtlArray< KWS_SCAN_LOG >&	arrLogs = m_wndListLog.IsWindowVisible() ? m_arrLogs : m_arrFileLogs;
		CLogListCtrl&				logList = m_wndListLog.IsWindowVisible() ? m_wndListLog : m_wndFileLog;
		BOOL						bFileLog = !m_wndListLog.IsWindowVisible();
		

		if( nItem <= arrLogs.GetCount() )
		{
			if( arrLogs[nItem].bKwsLog )
			{
				UINT_PTR uRet = IDCANCEL;
				if( bFileLog )
				{
					CWHRoundRectDialog<CBkSimpleDialog> dlgNotice;
					dlgNotice.Load(IDR_BK_QUARANTINE_RETRIEVE_NOTICE_KWS_DLG);
					uRet = dlgNotice.DoModal( );

					if( uRet == IDOK )
					{
						arrLogs[nItem].state = enum_HISTORY_ITEM_STATE_RESTORED;
						logList.RedrawItems( nItem, nItem );
						RecoverItem( arrLogs[nItem].dwID, arrLogs[nItem].tmHistoryTime );
						UpdateTheSameState( arrLogs, nItem, logList );
						UpdateOtherLog( arrLogs[nItem] );
					}

					if (dlgNotice.GetItemCheck(IDC_CHK_MSGBOX_NONOTIFYLATER))
					{
						AddTheSameFileTrust( arrLogs, nItem );
						//AddFileToTrust( arrLogs[nItem].szItemName );

					}
				}
				else
				{
					CBkSafeMsgBox dlg;
					dlg.AddButton(TEXT("确定"),		IDOK);
					dlg.AddButton(TEXT("取消"),		IDCANCEL);
					CString strText(TEXT("还原已经修复的威胁项，有可能使电脑感染\r\n\r\n病毒木马！您确定要继续还原吗？"));
					uRet = dlg.ShowMsg(
						strText, NULL, MB_BK_CUSTOM_BUTTON | MB_ICONWARNING);

					if( uRet == IDOK )
					{
						arrLogs[nItem].state = enum_HISTORY_ITEM_STATE_RESTORED;
						logList.RedrawItems( nItem, nItem );
						RecoverItem( arrLogs[nItem].dwID, arrLogs[nItem].tmHistoryTime );
						UpdateTheSameState( arrLogs, nItem, logList );
						UpdateOtherLog( arrLogs[nItem] );
					}
				}
			}
			else
			{
				CWHRoundRectDialog<CBkSimpleDialog> dlgNotice;
				dlgNotice.Load(IDR_BK_QUARANTINE_RETRIEVE_NOTICE_DLG);
				UINT_PTR uRet = dlgNotice.DoModal( );

				if ( uRet == IDOK)
				{
					HRESULT hr = E_FAIL;
					KWS_SCAN_LOG& log = arrLogs[nItem];
					std::wstring strFile( log.szItemName );
					std::wstring strQuaraName( log.szQuaraName );
					hr = m_quaLoader.RestoreQuarantineFile( strFile, strQuaraName );

					log.state = SUCCEEDED(hr) ? enum_HISTORY_ITEM_STATE_RESTORED : enum_HISTORY_ITEM_STATE_INVALID;
					logList.RedrawItems( nItem, nItem );

					if (dlgNotice.GetItemCheck(IDC_CHK_MSGBOX_NONOTIFYLATER))
					{
						WinMod::CWinPath path( log.szItemName );
						AddFileToTrust( path.GetPathWithoutUnicodePrefix() );
					}

					UpdateOtherLog( log );
				}
			}
		}
		
		return TRUE;
	}

	void UpdateTheSameState( CAtlArray< KWS_SCAN_LOG >& arrLog, int nItem, CLogListCtrl& logList )
	{
		KWS_SCAN_LOG& log = arrLog[nItem];

		for ( int i = 0; i < arrLog.GetCount(); i++ )
		{
			KWS_SCAN_LOG& item = arrLog[i];

			if( item.bKwsLog && 
				item.tmHistoryTime == log.tmHistoryTime && 
				item.dwID == log.dwID &&
				item.state == enum_HISTORY_ITEM_STATE_FIXED )
			{
				item.state = enum_HISTORY_ITEM_STATE_RESTORED;
				logList.RedrawItems( i, i );
			}
		}
	}

	void UpdateOtherLog( KWS_SCAN_LOG& log )
	{
		CAtlArray< KWS_SCAN_LOG >&	arrLogs = !m_wndListLog.IsWindowVisible() ? m_arrLogs : m_arrFileLogs;

		for ( int i = 0; i < arrLogs.GetCount(); i++ )
		{
			KWS_SCAN_LOG& logOther = arrLogs[i];
			if( log.bKwsLog )
			{
				if( log.tmHistoryTime == logOther.tmHistoryTime &&
					log.dwID == logOther.dwID )
				{
					logOther.state = log.state;
					break;
				}
			}
			else
			{
				UINT uId = logOther.dwID;
				if( log.dwID == uId )
				{
					logOther.state = log.state;
					break;
				}
			}
		}
	}

	void AddTheSameFileTrust( CAtlArray< KWS_SCAN_LOG >& arrLog, int nItem )
	{
		KWS_SCAN_LOG& log = arrLog[nItem];

		for ( int i = 0; i < arrLog.GetCount(); i++ )
		{
			KWS_SCAN_LOG& item = arrLog[i];

			if( item.bKwsLog && 
				item.tmHistoryTime == log.tmHistoryTime && 
				item.dwID == log.dwID )
			{
				AddFileToTrust( item.szItemName );
			}
		}
	}

	void AddFileToTrust( LPCTSTR pszFile )
	{
		std::wstring szUrl( pszFile );
		AddTrustItem( szUrl, enum_TRUST_ITEM_TYPE_FILE );
		CSafeMonitorTrayShell::WhiteListLibUpdated();
	}

	void OnDestroy()
	{
		m_wndListLog.DestroyWindow();
		m_wndFileLog.DestroyWindow();

		if( m_hLoadLog )
		{
			if( WaitForSingleObject(m_hLoadLog, 100 ) != WAIT_OBJECT_0 )
				TerminateThread( m_hLoadLog, 0 );

			::CloseHandle( m_hLoadLog );
			m_hLoadLog = NULL;
		}

		m_quaLoader.UnInit();
	}

	void AddTrustItem( std::wstring& strItem, EM_TRUST_ITEM_TYPE nType )
	{
		S_TRUST_LIST trustList;
		trustList.operation = enum_TRUST_LIST_ADD;
		trustList.itemType = nType;
		trustList.vecItemList.push_back( strItem );
		HRESULT hr = m_logic.SetUserTrustList( trustList );
		CSafeMonitorTrayShell::WhiteListLibUpdated();

		ATLASSERT( SUCCEEDED(hr) );
	}

	void OnBtnCheckAll()
	{
		BOOL bCheck = GetItemCheck( IDC_VIRUS_LOG_CHECK_ALL );
		if( m_wndFileLog.IsWindowVisible() )
		{
			m_wndFileLog.CheckAllItems( bCheck );
		}
		else if ( m_wndListLog.IsWindowVisible() )
		{
			m_wndListLog.CheckAllItems( bCheck );
		}

		EnableItem( IDC_BTN_VIRSCAN_CLEAR_LOG, bCheck);
	}

	void RecoverItem( DWORD dwId, __time64_t time )
	{		
		S_RECOVER_ITEM recoverItem;
		recoverItem.tmHistoryTime = time;
		recoverItem.vecItemIDs.push_back( dwId );

		std::vector<S_RECOVER_ITEM> recoverList;
		recoverList.push_back(recoverItem);
		HRESULT hr = m_logic.StartRecoverEx(recoverList);
		assert( SUCCEEDED(hr) );
		
	}

	void OnBkBtnMax()
	{
		if (WS_MAXIMIZE == (GetStyle() & WS_MAXIMIZE))
		{
			SendMessage(WM_SYSCOMMAND, SC_RESTORE | HTCAPTION, 0);
			//SetItemAttribute(IDC_BTN_SYS_MAX, "skin", "maxbtn");
		}
		else
		{
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE | HTCAPTION, 0);
			//SetItemAttribute(IDC_BTN_SYS_MAX, "skin", "restorebtn");
		}
	}

	void OnListVirusScanLog(CRect rcWnd)
	{
		m_wndListLog.SetColumnWidth(0, 150 );
		m_wndListLog.SetColumnWidth(1, rcWnd.Width() - 435 - ::GetSystemMetrics(SM_CXVSCROLL) - 6 );
		m_wndListLog.SetColumnWidth(2, 115 );
		m_wndListLog.SetColumnWidth(3, 55 );
		m_wndListLog.SetColumnWidth(4, 60 );
		m_wndListLog.SetColumnWidth(5, 55 );

		m_wndListLog.Invalidate(TRUE);
	}

	void OnListFileVirusLog(CRect rcWnd)
	{
		m_wndFileLog.SetColumnWidth(0, 150  );
		m_wndFileLog.SetColumnWidth(1, rcWnd.Width() - 380 - ::GetSystemMetrics(SM_CXVSCROLL) - 6 );
		m_wndFileLog.SetColumnWidth(2, 115 );
		m_wndFileLog.SetColumnWidth(3, 60 );
		m_wndFileLog.SetColumnWidth(4, 55 );

		m_wndFileLog.Invalidate(TRUE);
	}

public:

    BK_NOTIFY_MAP(IDC_RICHVIEW_WIN)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_CLOSE, OnBkBtnClose)
		BK_NOTIFY_ID_COMMAND(IDC_BTN_SYS_MAX, OnBkBtnMax)
        BK_NOTIFY_ID_COMMAND(IDC_BTN_VIRSCAN_CLEAR_LOG, OnBtnVirScanClearLog)
		BK_NOTIFY_ID_COMMAND(IDC_VIRUS_LOG_CHECK_ALL, OnBtnCheckAll)
		BK_NOTIFY_TAB_SELCHANGE(IDC_VIRUS_LOG_TAB_CTRL,OnBkTabCtrlChange)
        BK_NOTIFY_ID_COMMAND(IDCANCEL, OnBkBtnClose)
		BK_NOTIFY_REALWND_RESIZED(IDC_LST_VIRSCAN_LOG,		OnListVirusScanLog )
		BK_NOTIFY_REALWND_RESIZED(IDC_FILE_VIRUS_LOG_LIST,	OnListFileVirusLog )
    BK_NOTIFY_MAP_END()

	BEGIN_MSG_MAP_EX(CBeikeSafeVirScanLogDlg)
        MSG_BK_NOTIFY(IDC_RICHVIEW_WIN)
        CHAIN_MSG_MAP(CBkDialogImpl<CBeikeSafeVirScanLogDlg>)
        CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CBeikeSafeVirScanLogDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        NOTIFY_HANDLER_EX(IDC_LST_VIRSCAN_LOG, LVN_GETDISPINFO, OnLVNVirScanLogGetDispInfo)
		NOTIFY_HANDLER_EX(IDC_FILE_VIRUS_LOG_LIST, LVN_GETDISPINFO, OnLVNVirScanLogFileGetDispInfo)
        MESSAGE_HANDLER_EX(MSG_LOAD_LOG_FINISH, OnLoadLogFinish)
        MESSAGE_HANDLER_EX(MSG_CLEAR_LOG_FINISH, OnClearLogFinish)
		MSG_WM_DESTROY(OnDestroy)
		MESSAGE_HANDLER(WM_ITEM_BUTTON_CLICK,	OnRecover)
		MESSAGE_HANDLER(WM_ITEM_CHECKBOX_CLICK,	OnItemCheckbox)
		MESSAGE_HANDLER_EX(MSG_REMOVE_QUARANTINE_ITEM_NOTIFY, OnRemoveQuarantineItemNotify)
		REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};