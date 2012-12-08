#include "stdafx.h"
#include "beikesafe.h"
#include "beikesafemaindlg.h"
#include "beikesafefreevirusuihandler.h"
#include "common/callduba.h"
#include "miniutil/fileversion.h"

#define KV_INSTALLER_URL TEXT("http://dl.ijinshan.com/safe/KAV_ONLINE_95_11.exe")


void CBeikeSafeFreeVirusUIHandler::Init()
{
	m_wndHtmlOnlineScan.SetMsg( m_pDlg->m_hWnd );
	m_wndHtmlOnlineScan.Create( m_pDlg->GetViewHWND(), IDC_ONLINE_SCAN_CTRL, FALSE, RGB(0xFB, 0xFC, 0xFD));

}

CBkNavigator* CBeikeSafeFreeVirusUIHandler::OnNavigate( CString &strChildName )
{
	return NULL;
}

LRESULT CBeikeSafeFreeVirusUIHandler::OnCallDubaClick(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	KCallDuba callDuba;
	UINT uId = IDCANCEL;
	if( callDuba.IsDubaExist() )
	{
		callDuba.CallDuba();
		return TRUE;
	}
	else
	{
		CString strPath, strUrl, strUrlFile;
		callDuba.GetInstallerPath( strPath );

		if( IsNeedDownLoad( strPath, (LPCTSTR)wParam ) )
		{
			if( wParam == NULL )
			{
				strUrlFile = KV_INSTALLER_URL;
			}
			else
			{
				strUrl = (LPCTSTR)wParam;
				int nPos = strUrl.Find( TEXT("?") );
				if( nPos == -1 )
					strUrlFile = strUrl;
				else
					strUrlFile = strUrl.Left( nPos );
			}

			if( !m_pDownInstaller )
			{
				m_pDownInstaller = new CToolsDownloadDlg( strUrlFile, strPath, TEXT(""), TEXT(""), FALSE );
			}

			if( m_pDownInstaller )
			{
				m_pDownInstaller->Load( IDR_BK_KVINSTALL_DOWNLOAD );
				uId = m_pDownInstaller->DoModal();

				delete m_pDownInstaller;
				m_pDownInstaller = NULL;
			}
		}
		else
		{
			callDuba.CallDubaInstall();
		}
	}
	
	return uId == IDOK;
}

BOOL CBeikeSafeFreeVirusUIHandler::IsNeedDownLoad( CString& strFile, LPCTSTR pszUrl )
{
	CString strVer;

	//体检那边会传空过来，表示要下载
	if( pszUrl == NULL )
		return TRUE;

	//如果文件不存在就要下载
	if( !::PathFileExists( strFile ) )
		return TRUE;

	BOOL bRet = GetFileVersion( strFile, strVer );
	if( bRet == FALSE )
		return TRUE;

	CString strUrl = pszUrl;
	CString strUrlVer;
	int nPos = strUrl.Find( TEXT("v=") );
	if( nPos == -1)
		strUrlVer = strUrl;
	else
		strUrlVer = strUrl.Mid( nPos + 2 );

	if( strVer != strUrlVer )
		return TRUE;
	else
		return FALSE;
}

BOOL CBeikeSafeFreeVirusUIHandler::FirstShow()
{
	KCallDuba callDuba;
	if( callDuba.IsDubaExist() )
	{
		callDuba.CallDuba();
		return FALSE;
	}
	else
	{
		if( m_bShowOnLineScan == FALSE )
		{
			m_bShowOnLineScan = TRUE;
			CString	strURL;
			GetErrorHtmlPath( strURL );
			m_wndHtmlOnlineScan.Show2(L"http://www.ijinshan.com/safe/free_kav.html",strURL);
		}
		return TRUE;
	}
}


HRESULT CBeikeSafeFreeVirusUIHandler::GetErrorHtmlPath(CString &strDir)
{
	HRESULT hr = SUCCEEDED(CAppPath::Instance().GetLeidianDataPath(strDir));
	if ( hr )
	{
		strDir = strDir.Left(strDir.ReverseFind(_T('\\')) + 1);
		strDir.Append( _T("KSoft\\html\\error.html") );
	}
	return hr;
}
