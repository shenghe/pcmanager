#include "stdafx.h"
#include "desktopinfo.h"
#include <shobjidl.h>
#include <atltime.h>

CDesktopInfo::CDesktopInfo()
{
	
}

CDesktopInfo::~CDesktopInfo()
{

}

HRESULT CDesktopInfo::CollectTypeInfo( DeskTypeCnt& info )
{
	HRESULT hr = _CollectTypeInfo( info, CSIDL_DESKTOPDIRECTORY );
	hr =_CollectTypeInfo( info, CSIDL_COMMON_DESKTOPDIRECTORY );
	return hr;
}

HRESULT CDesktopInfo::CollectRecentInfo( DeskTypeCnt& info )
{
	HRESULT hr = _CollectTypeInfo( info, CSIDL_RECENT, TRUE );
	return hr;
}

HRESULT CDesktopInfo::CollectNoUseInfo( DeskNoUseCnt& info )
{
	HRESULT hr = _CollectTimeInfo( info, CSIDL_DESKTOPDIRECTORY );
	hr = _CollectTimeInfo( info, CSIDL_COMMON_DESKTOPDIRECTORY );
	return hr;
}

HRESULT CDesktopInfo::_CollectTimeInfo( DeskNoUseCnt& info, int nCsidl )
{
	HRESULT hr = E_FAIL;
	TCHAR tszUserDskPath[MAX_PATH*2] = {0};

	hr = SHGetSpecialFolderPath( NULL, tszUserDskPath, nCsidl, FALSE );
	ATLASSERT( SUCCEEDED(hr) );

	if( FAILED(hr) )
		return hr;

	if( !PathIsDirectory(tszUserDskPath) )
		return E_FAIL;

	CFindFile finder;
	_tcscat( tszUserDskPath, TEXT("\\*.*") );
	BOOL bWorking = finder.FindFile( tszUserDskPath );
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if( !bWorking)
			break;

		if( finder.IsDots() )
			continue;

		if( _tcsicmp( L"Desktop.ini", finder.GetFileName()) == 0 )
			continue;

		CalTimeCnt( info, finder.GetFilePath() );
	}
	hr = S_OK;
	return hr;
}

void CDesktopInfo::CalTimeCnt( DeskNoUseCnt& info, CString& strFile )
{
	__time64_t tNow = CTime::GetCurrentTime().GetTime();
	__time64_t tLastVisit = 0;

	 if( !GetFileLastVisit( strFile, tLastVisit ) )
		 return;
	
	 if( tNow - tLastVisit >= 30 * 24 * 3600 )
		 info.nOm ++;
	 else if( tNow - tLastVisit >= 15 * 24 * 3600 )
		 info.nM ++;
	 else if( tNow - tLastVisit >= 7 * 24 * 3600 )
		 info.nTw ++;
	 else 
		 info.nW ++;
}

BOOL CDesktopInfo::GetFileLastVisit(LPCTSTR lpszFileName, __time64_t& tVisit )
{
	BOOL bRet = FALSE;
	FILETIME ftVisit;
	FILETIME ftWrite;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	hFile = ::CreateFile(
		lpszFileName, GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
		);

	if (INVALID_HANDLE_VALUE == hFile)
		goto Exit0;

	bRet = ::GetFileTime(hFile, NULL, &ftVisit, &ftWrite );
	if (!bRet)
		goto Exit0;

	CTime timeVisit( ftVisit );
	CTime timeWrite( ftWrite );

	if( timeVisit.GetTime() >= timeWrite.GetTime() )
		tVisit = timeVisit.GetTime();
	else
		tVisit = timeWrite.GetTime();

	bRet = TRUE;
Exit0:

	if (INVALID_HANDLE_VALUE != hFile)
	{
		::CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return bRet;
}

HRESULT CDesktopInfo::_CollectTypeInfo( DeskTypeCnt& info, int nCsidl, BOOL bChange )
{
	HRESULT hr = E_FAIL;
	TCHAR tszUserDskPath[MAX_PATH*2] = {0};

	hr = SHGetSpecialFolderPath( NULL, tszUserDskPath, nCsidl, FALSE );
	ATLASSERT( SUCCEEDED(hr) );

	if( FAILED(hr) )
		return hr;

	if( !PathIsDirectory(tszUserDskPath) )
		return E_FAIL;

	CFindFile finder;
	_tcscat( tszUserDskPath, TEXT("\\*.*") );
	BOOL bWorking = finder.FindFile( tszUserDskPath );
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if( !bWorking)
			break;

		if( finder.IsDots() )
			continue;

		if( _tcsicmp( L"Desktop.ini", finder.GetFileName()) == 0 )
			continue;

		CalTypeCnt( info, finder.GetFilePath(), bChange );
	}
	hr = S_OK;
	return hr;
}

void CDesktopInfo::CalTypeCnt( DeskTypeCnt& info, CString& strFile, BOOL bChange )
{
	CPath path( strFile );
	//在获取recent时，没有文件都是快捷方式
	if ( path.IsDirectory() && bChange == FALSE )
	{
		info.nFld++;
	}
	else
	{
		BOOL bChangeAndIsDir = FALSE;
		CString strExten( path.GetExtension().MakeLower() );
		if( bChange )
		{
			if( _tcsicmp( strExten, TEXT(".lnk")) == 0 )
			{
				TCHAR tszFilePath[MAX_PATH] = {0};
				if ( SUCCEEDED(LnkToRealPath( strFile, tszFilePath )) )
				{
					CPath newFile( tszFilePath );
					if( newFile.IsDirectory() )
					{
						info.nFld ++;
						bChangeAndIsDir = TRUE;
					}
					else
						strExten = newFile.GetExtension().MakeLower();
				}
			}
		}
		
		if( bChangeAndIsDir == FALSE )
		{
			if( IsDoc( strExten ) )
				info.nDoc ++;
			else if( IsMv( strExten ) )
				info.nMv ++;
			else if( IsPic( strExten ) )
				info.nPic ++;
			else if( IsZip( strExten ) )
				info.nZip ++;
			else if( _tcsicmp( strExten, TEXT(".lnk")) == 0 )
				info.nLnk ++;
			else
				info.nOt ++;
		}
	}
}

BOOL CDesktopInfo::IsDoc( CString& strExten )
{
	for ( int i = 0 ; i < 23; i++ )
	{
		if( _tcsicmp( strExten, g_tszExDoc[i] ) == 0 )
			return TRUE;
	}

	return FALSE;
}

BOOL CDesktopInfo::IsPic( CString& strExten )
{
	for ( int i = 0 ; i < 48; i++ )
	{
		if( _tcsicmp( strExten, g_tszEXPic[i] ) == 0 )
			return TRUE;
	}

	return FALSE;
}

BOOL CDesktopInfo::IsMv( CString& strExten )
{
	for ( int i = 0 ; i < 91; i++ )
	{
		if( _tcsicmp( strExten, g_tszExMv[i] ) == 0 )
			return TRUE;
	}

	return FALSE;
}

BOOL CDesktopInfo::IsZip( CString& strExten )
{
	for ( int i = 0 ; i < 32; i++ )
	{
		if( _tcsicmp( strExten, g_tszExZip[i] ) == 0 )
			return TRUE;
	}

	return FALSE;
}

HRESULT CDesktopInfo::LnkToRealPath( LPCTSTR pszLnk, LPTSTR tszFilePath )
{
	HRESULT hr = E_FAIL;
	IPersistFile* pPFile = NULL;
	IShellLink* pSLnk = NULL;
	WIN32_FIND_DATA wFd;
	memset( &wFd, 0, sizeof(wFd) );

	::CoInitialize( NULL );

	hr = CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pSLnk );
	if( FAILED(hr) || !pSLnk )
		goto Exit0;

	hr = pSLnk->QueryInterface( IID_IPersistFile, (void**)&pPFile );
	if( FAILED(hr) || !pPFile )
		goto Exit0;

	hr = pPFile->Load( pszLnk, STGM_READ );
	if( FAILED(hr) )
		goto Exit0;
	hr = pSLnk->GetPath( tszFilePath, MAX_PATH, &wFd, SLGP_SHORTPATH );

Exit0:
	if( pPFile )
		pPFile->Release();

	if( pSLnk )
		pSLnk->Release();

	::CoUninitialize();

	return hr;
}

