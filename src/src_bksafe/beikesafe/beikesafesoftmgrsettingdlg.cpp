#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "beikesafesoftmgrsettingdlg.h"
#include "_idl_gen/bksafesvc.h"
#include <bksafe/bksafeconfig.h>
#include <safemon/safetrayshell.h>
#include <comproxy/bkutility.h>
#include <shellapi.h>

#define  IDC_EDIT_SOFT_PATH  8880

CBeikeSafeSoftMgrSettingDlg::~CBeikeSafeSoftMgrSettingDlg()
{

}

BOOL CBeikeSafeSoftMgrSettingDlg::BkSafeSoftMgrSettingChanged()
{
	return	m_bBkSafeSettingChanged;
}

BOOL CBeikeSafeSoftMgrSettingDlg::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	InitList();

	DWORD dwEditFlags = WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY;
	m_ctlEditStorePath.Create( GetViewHWND(), NULL, NULL, dwEditFlags, WS_EX_NOPARENTNOTIFY, IDC_EDIT_STORE_DIR, NULL);
	m_ctlEditStorePath.SetFont(BkFontPool::GetFont(BKF_DEFAULTFONT));

	LoadSettings();

	CString		strFreeDiskSpace;
	CString		strStorePath;
	m_ctlEditStorePath.GetWindowText( strStorePath );
	GetCurDiskFreeSpace( strStorePath, strFreeDiskSpace );
	SetItemText( IDC_LBL_DISK_SPACE, strFreeDiskSpace );

	return	TRUE;
}

void CBeikeSafeSoftMgrSettingDlg::InitList()
{
	m_list.Create(GetViewHWND(), NULL, NULL, WS_CHILD|WS_VISIBLE | LBS_OWNERDRAWVARIABLE, 0, IDC_SETTING_LIST);

	m_list.InsertString( 0, BkString::Get(IDS_SOFTMGR_8075) );
	CTypeListItemData * pListData = new CTypeListItemData( 20, 0, COL_TYPE_NAME_NORMAL, BkString::Get(IDS_SOFTMGR_8075), -1 );
	m_list.SetItemDataX( 0, ( DWORD_PTR ) pListData );
	m_list.SetItemHeight( 0, 28 );		

	m_list.InsertString( 1, BkString::Get(IDS_SOFTMGR_8076) );
	pListData = new CTypeListItemData( 20, 1, COL_TYPE_NAME_NORMAL, BkString::Get(IDS_SOFTMGR_8076), -1 );
	m_list.SetItemDataX( 1, ( DWORD_PTR ) pListData );
	m_list.SetItemHeight( 1, 28 );	

	m_list.InsertString( 2, BkString::Get(IDS_SOFTMGR_8077) );
	pListData = new CTypeListItemData( 20, 2, COL_TYPE_NAME_NORMAL, BkString::Get(IDS_SOFTMGR_8077), -1 );
	m_list.SetItemDataX( 2, ( DWORD_PTR ) pListData );
	m_list.SetItemHeight( 2, 28 );	

	m_list.SetFocusOnMouseMove( FALSE );		// 不再自动获得焦点
	m_list.SetCallBack( this );
	m_list.SetCurSel( 0 );

	// Tab页的ID从101开始
	SetItemVisible( IDC_DLG_INSTALL_SET, FALSE );
	SetItemVisible( IDC_DLG_DELFILE_SET, FALSE );
	SetItemVisible( IDC_DLG_DOWNLOAD_SET, TRUE );
}

void CBeikeSafeSoftMgrSettingDlg::OnClick( int nListId, CTypeListItemData * pData)
{
	SetItemVisible( IDC_DLG_DOWNLOAD_SET, FALSE );
	SetItemVisible( IDC_DLG_INSTALL_SET, FALSE );
	SetItemVisible( IDC_DLG_DELFILE_SET, FALSE );
	SetItemVisible( IDC_DLG_UPDATE_SOFT_SET, FALSE );

	SetItemVisible( 101 + pData->nId, TRUE );
}

void CBeikeSafeSoftMgrSettingDlg::GetCurDiskFreeSpace( LPCTSTR szCurrentPath, CString & strFreeSpace )
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

		if ( lpfree.QuadPart > ( 1024 * 1024 * 1024 ) )
		{
			double	freeGB = lpfree.QuadPart / ( 1024 * 1024 * 1024.0 );
			strFreeSpace.Format( _T( "%.2fGB" ), freeGB );
		}
		else
		{
			ULONGLONG		freeMB = lpfree.QuadPart / ( 1024 * 1024 );
			strFreeSpace.Format( _T( "%dMB" ), freeMB );
		}
	}
}

void CBeikeSafeSoftMgrSettingDlg::LoadSettings()
{
	CString	strStoreDir;
	BKSafeConfig::GetStoreDir( strStoreDir );
	m_ctlEditStorePath.SetWindowText( strStoreDir );

	int		nRet;

	nRet = BKSafeConfig::GetHintPlugin();
	if ( nRet == 0 )
	{
		SetItemCheck( IDC_CHECK_NO_HINT, TRUE );
	}
	else
	{
		SetItemCheck( IDC_CHECK_SHOWHINT, TRUE );
	}

	nRet = BKSafeConfig::GetAutoInstall();
	if ( nRet == 0 )
	{
		SetItemCheck( IDC_CHECK_NO_AUTO_INSTALL, TRUE );
	}
	else
	{
		SetItemCheck( IDC_CHECK_AUTO_INSTALL, TRUE );
	}

	nRet = BKSafeConfig::GetPowerSweepFile();
	if ( nRet == 0 )
	{
		SetItemCheck( IDC_CHECK_DEL_TO_RECY, TRUE );
	}
	else
	{
		SetItemCheck( IDC_CHECK_DEL_REAL, TRUE );
	}

	nRet = BKSafeConfig::GetDelFile();
	switch ( nRet )
	{
	case 0: SetItemCheck( IDC_CHECK_DEL_NEVER, TRUE ); break;
	case 1: SetItemCheck( IDC_CHECK_DEL_RIGHTNOW, TRUE ); break;
	case 2: SetItemCheck( IDC_CHECK_DEL_WEEK, TRUE ); break;
	default: break;
	}

	nRet = BKSafeConfig::GetUpdatePointOut();
	switch ( nRet )
	{
	case 0: SetItemCheck( IDC_CHECK_DAY_POINT_OUT, TRUE ); break;
	case 1: SetItemCheck( IDC_CHECK_WEEK_POINT_OUT, TRUE ); break;
	case 2: SetItemCheck( IDC_CHECK_NEVER_POINT_OUT, TRUE ); break;
	default: break;
	}
}

void CBeikeSafeSoftMgrSettingDlg::OnBkSafeSoftMgrSettingChanged()
{
	m_bBkSafeSettingChanged = TRUE;
}

void CBeikeSafeSoftMgrSettingDlg::OnBkBtnOK()
{
	if ( m_bBkSafeSettingChanged )
	{
		SaveSettings();
	}
	
	EndDialog( IDOK );
}

void CBeikeSafeSoftMgrSettingDlg::OnBkBtnClose()
{
	EndDialog( IDCANCEL );
}

void CBeikeSafeSoftMgrSettingDlg::SaveSettings()
{
	int		nValue;
	
	nValue = GetItemCheck( IDC_CHECK_SHOWHINT ) ? 1 : 0;
	BKSafeConfig::SetHintPlugin( nValue );

	nValue = GetItemCheck( IDC_CHECK_AUTO_INSTALL ) ? 1 : 0;
	BKSafeConfig::SetAutoInstall( nValue );

	nValue = GetItemCheck( IDC_CHECK_DEL_TO_RECY ) ? 0 : 1;
	BKSafeConfig::SetPowerSweepFile( nValue );

	if ( GetItemCheck( IDC_CHECK_DEL_NEVER ) )
	{
		nValue = 0;
	}
	else if ( GetItemCheck( IDC_CHECK_DEL_RIGHTNOW ) )
	{
		nValue = 1;
	}
	else if ( GetItemCheck( IDC_CHECK_DEL_WEEK ) )
	{
		nValue = 2;
	}
	BKSafeConfig::SetDelFile( nValue );

	CString	strStoreDir;
	m_ctlEditStorePath.GetWindowText( strStoreDir );
	BKSafeConfig::SetStortDir( strStoreDir );

	if ( GetItemCheck( IDC_CHECK_DAY_POINT_OUT ) )
	{
		nValue = 0;
	}
	else if ( GetItemCheck( IDC_CHECK_WEEK_POINT_OUT ) )
	{
		nValue = 1;
	}
	else if ( GetItemCheck( IDC_CHECK_NEVER_POINT_OUT ) )
	{
		nValue = 2;
	}
	BKSafeConfig::SetUpdatePointOut( nValue );

}

void CBeikeSafeSoftMgrSettingDlg::OnSelDir()
{
	CString sFolderPath, sFolderNew;
	m_ctlEditStorePath.GetWindowText( sFolderPath );	
	sFolderNew = sFolderPath;
	while( SelectFolderEx(sFolderNew, m_hWnd) )
	{

		// 检查路径是否过长
		if ( sFolderNew.GetLength() > 200 )
		{
			CBkSafeMsgBox::Show( BkString::Get(IDS_SOFTMGR_8078), NULL, MB_OK | MB_ICONWARNING);
		}
		else
		{
			m_bBkSafeSettingChanged = TRUE;
			m_ctlEditStorePath.SetWindowText( sFolderNew );

			CString		strFreeDiskSpace;
			GetCurDiskFreeSpace( sFolderNew, strFreeDiskSpace );
			SetItemText( IDC_LBL_DISK_SPACE, strFreeDiskSpace );

			break;
		}	
	}
}

void CBeikeSafeSoftMgrSettingDlg::OnOpenStoreDir()
{
	CString strPath;
	m_ctlEditStorePath.GetWindowText( strPath );
	
	if( !PathIsDirectory(strPath) )
		CreateDirectoryNested( strPath );
	if( PathIsDirectory( strPath ) )
		ShellExecute(NULL, _T("open"), strPath, NULL, NULL, SW_SHOW);
	else
	{
		CString strMessage;
		strMessage.Format(BkString::Get(IDS_SOFTMGR_8045), strPath);
		CBkSafeMsgBox::Show(strMessage,BkString::Get(IDS_SOFTMGR_8046), MB_OK | MB_ICONWARNING);
	}
}

void CBeikeSafeSoftMgrSettingDlg::OnRestoreDefaultDir()
{
	WCHAR szSystemPath[MAX_PATH] = {0};
	GetSystemDirectory(szSystemPath, MAX_PATH-1);
	CString strSysPath;
	strSysPath.Format(_T("%s"), szSystemPath);
	strSysPath = strSysPath.Left(3);
	strSysPath.Append( _T( "ksDownloads" ) );

	m_ctlEditStorePath.SetWindowText( strSysPath );

	m_bBkSafeSettingChanged = TRUE;

	CString		strFreeDiskSpace;
	GetCurDiskFreeSpace( strSysPath, strFreeDiskSpace );
	SetItemText( IDC_LBL_DISK_SPACE, strFreeDiskSpace );
}

UINT_PTR CBeikeSafeSoftMgrSettingDlg::DoModal(int nPage, HWND hWndParent)
{
	m_nPage = nPage;

	return __super::DoModal(hWndParent);
}

CBkNavigator* CBeikeSafeSoftMgrSettingNavigator::OnNavigate(CString &strChildName)
{
	int nPage = SettingPageDownload;

	//if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_PAGE_COMMON))
	//{
	//	nPage = SettingPageCommon;
	//}
	//else if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_PAGE_VIRSCAN))
	//{
	//	nPage = SettingPageVirScan;
	//}
	//else if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_PAGE_VULSCAN))
	//{
	//	nPage = SettingPageVulScan;
	//}
	//else if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_PAGE_PROTECTION))
	//{
	//	nPage = SettingPageProtection;
	//}
	//else if (0 == strChildName.CompareNoCase(BKSFNS_SETTING_PAGE_IEFIX))
	//{
	//	nPage = SettingPageIEFix;
	//}

	DoModal(nPage);

	return NULL;
}

UINT_PTR CBeikeSafeSoftMgrSettingNavigator::DoModal(int nPage/* = SettingPageDownload*/, HWND hWndParent/* = ::GetActiveWindow()*/)
{
	CBeikeSafeSoftMgrSettingDlg dlg;

	UINT_PTR uRet = dlg.DoModal(nPage, hWndParent);

	if (dlg.BkSafeSoftMgrSettingChanged())
	{
		//m_pDlg->SettingChanged();
	}

	return uRet;
}