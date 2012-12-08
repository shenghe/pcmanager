#include "stdafx.h"
#include "WUAUpdateInfo.h"
#include "Utils.h"
#include "HotfixUtils.h"
#include <common/registrywow.h>
#pragma comment(lib, "Version.lib")

#define sizeofarray(x) (sizeof(x)/sizeof(x[0]))

BOOL _ReadOfficePatchInfo(HKEY hKey, LPCTSTR lpSubKey, DWORD dwIndex, LPCTSTR lpszKey, LPVOID lpData)
{
	ATLASSERT(lpData);
	CString strSubKey;
	strSubKey.Format(_T("%s\\%s"), lpSubKey, lpszKey);

	CString strName;
	if( ReadRegString(hKey, strSubKey, _T("DisplayName"), strName) )
	{
		LPCTSTR szMagic = _T("(KB");
		LPCTSTR szPos = _tcsstr(strName, szMagic);
		if(szPos)
		{
			TItemFixed item;
			szPos += _tcslen(szMagic);
			item.nID = _ttoi( szPos );
			item.strName = strName;
			CString strInstalled;
			ReadRegString(hKey, strSubKey, _T("Installed"), strInstalled );
			item.strInstallDate = strInstalled;
			
			M_KBInfo &arr = *(M_KBInfo*)lpData;
			arr[item.nID] = item;

			ATLTRACE(_T("GOT Office.Installed KB %d %s\r\n"), item.nID, item.strName);
		}
	}
	return TRUE;
}

INT ReadInstalledOffice( M_KBInfo &arr )
{
	LPCTSTR lpszPathFmt = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Products\\%s\\Patches");
	LPCTSTR products_ids[] = 
	{
		_T("4080110900063D11C8EF10054038389C"),
		_T("00002109110000000000000000F01FEC"),
	};
	
	INT nOld = arr.size();
	for( int i=0; i<sizeofarray(products_ids); ++i )
	{
		CString strPath;
		strPath.Format(lpszPathFmt, products_ids[i]);
		EnumRegKey(HKEY_LOCAL_MACHINE, strPath, _ReadOfficePatchInfo, &arr);
	}
	return arr.size() - nOld;
}

void CWUAUpdateInfoXP::Init(DWORD dwFlags)
{	
	m_arrKBID.clear();
	_GetKBIDsByReg(_T("SOFTWARE\\Microsoft\\Updates"), m_arrKBID);
	ReadInstalledOffice( m_arrKBID );
}

BOOL CWUAUpdateInfoXP::IsUpdateAvailable( int nKBID )
{
	return TRUE;
}

BOOL CWUAUpdateInfoXP::IsUpdateInstalled( int nKBID )
{
	return m_arrKBID.find( nKBID )!=m_arrKBID.end();
}

M_KBInfo & CWUAUpdateInfoXP::GetInstalledInfo()
{
	return m_arrKBID;
}

void CWUAUpdateInfoXP::_GetKBIDsByReg( LPCTSTR szKeyPath, M_KBInfo &arrKBID )
{
	HKEY hKey;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, KEY_READ, &hKey);
	for(DWORD nIndex=0;; ++nIndex )
	{
		CString strKey;
		DWORD dwLen = MAX_PATH;
		BOOL bOk = ERROR_SUCCESS==SHEnumKeyEx(hKey, nIndex, strKey.GetBuffer(255), &dwLen);
		strKey.ReleaseBuffer();
		if(!bOk)
			break;
		
		if(strKey.GetLength()>=5)
		{
			CString strL1 = strKey.Left(1);
			CString strL2 = strKey.Left(2);
			if(strL2.CompareNoCase(_T("KB"))==0 
				|| (strL1.CompareNoCase(_T("Q"))==0 && _tcsstr(szKeyPath, _T("MSXML"))) )
				// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Updates\Windows XP\SP4\KB942288-v3
				// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Updates\Windows XP\SP4\KB950582
			{
				CString strSubpathKB = szKeyPath;
				strSubpathKB.AppendFormat(_T("\\%s"), strKey);

				INT nKBID = 0;
				// Parse kb id 
				{
					CString strKB;
					if(strL1.CompareNoCase(_T("Q"))==0)
						strKB = strKey.Mid(1);
					else
						strKB = strKey.Mid(2);

					nKBID = ParseKBString( strKB );
					// 对于 HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Updates\Windows Media Player\KB952069_WM9 
					// 需要检查 DisplayVersion 获取版本 
					// WMP11 的KB 不带 _WM 
					strKB = strKB.MakeUpper();
					if( strKB.Find(_T("_WM"))!=-1 || _tcsstr(szKeyPath, _T("Windows Media ")) )
					{
						CString _sv;
						if( ReadRegString(HKEY_LOCAL_MACHINE, strSubpathKB, _T("DisplayVersion"), _sv) )
						{
							INT nv = _ttoi(_sv);
							if(nv>1)
							{
								nKBID += (KB_VER_BASE*nv);
							}
						}
					}
				}

				if( arrKBID.find(nKBID)==arrKBID.end() )
				{
					// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Updates\Windows XP\SP4\KB969947@InstalledDate : 11/28/2009
					TItemFixed item;
					CString strInstalledDate;						
					ReadRegString(HKEY_LOCAL_MACHINE, strSubpathKB, _T("Description"), item.strName);
					if( ReadRegString(HKEY_LOCAL_MACHINE, strSubpathKB, _T("InstalledDate"), strInstalledDate) )
					{
						INT year=0, month=0, day=0;
						if( _stscanf( strInstalledDate, _T("%u/%u/%u"), &month, &day, &year )==3 )
						{
							strInstalledDate.Format(_T("%u-%u-%u"), year, month, day);
						}
						item.strInstallDate = strInstalledDate;
						//else
						//	strDate = _T("");
					}
					item.nID = nKBID;
					arrKBID[ nKBID ]  = item;
				}
				continue;
			}
		}
		CString strSubKey = szKeyPath;
		strSubKey.AppendFormat(_T("\\%s"), strKey);
		_GetKBIDsByReg( strSubKey, arrKBID );
	};
	RegCloseKey(hKey);
}

BOOL _RegScanInstalled( M_KBInfo &arrKBID, FILETIME &lastestUpdateTime )
{
	LPCTSTR szBaseKeyPath = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\Packages");
	LPCTSTR szMagicPrefix = _T("Package_");
	LPCTSTR szMagicPrefixKB = _T("_for_KB");
	
	CRegistryWow<REGISTRY_WOW> reg;
	HKEY hKey;
	if( ERROR_SUCCESS!=reg.OpenKeyEx(HKEY_LOCAL_MACHINE, szBaseKeyPath, 0, KEY_READ, &hKey) )
		return FALSE;

	lastestUpdateTime.dwHighDateTime = lastestUpdateTime.dwLowDateTime = 0;
	for(DWORD nIndex=0;; ++nIndex )
	{
		CString strKey;
		DWORD dwLen = MAX_PATH;
		BOOL bOk = ERROR_SUCCESS==SHEnumKeyEx(hKey, nIndex, strKey.GetBuffer(255), &dwLen);
		strKey.ReleaseBuffer();
		if(!bOk)
			break;
		
		if(strKey.GetLength()>=10)
		{
			LPCTSTR s1=NULL, s2=NULL;
			if( (s1=_tcsstr(strKey, szMagicPrefix))!=NULL && (s2=_tcsstr(s1+_tcslen(szMagicPrefix), szMagicPrefixKB))!=NULL )
			{
				s2 += _tcslen( szMagicPrefixKB );
				int nKBID = _ttoi(s2);
				
				if( arrKBID.find(nKBID)==arrKBID.end() )
				{
					TItemFixed fixedItem;

					// get update time 
					DWORD dwHigh(0), dwLow(0);
					CString _subpath;
					_subpath.Format(_T("%s\\%s"), szBaseKeyPath, strKey);
					ReadRegString(HKEY_LOCAL_MACHINE, _subpath, _T("InstallName"), fixedItem.strName );
					BOOL hasInstallTime = ReadRegDWord(HKEY_LOCAL_MACHINE, _subpath, _T("InstallTimeHigh"), dwHigh )
						&& ReadRegDWord(HKEY_LOCAL_MACHINE, _subpath, _T("InstallTimeLow"), dwLow );

					CString strDate;
					if( hasInstallTime )
					{
						FILETIME ft;
						SYSTEMTIME systime={0};
						ft.dwHighDateTime = dwHigh;
						ft.dwLowDateTime = dwLow;
						FileTimeToSystemTime(&ft, &systime);
						strDate.Format(_T("%04d-%02d-%02d"), systime.wYear, systime.wMonth, systime.wDay);
					}
					
					if(dwHigh>lastestUpdateTime.dwHighDateTime 
						|| (dwHigh==lastestUpdateTime.dwHighDateTime&&dwLow>lastestUpdateTime.dwLowDateTime) )
					{
						lastestUpdateTime.dwHighDateTime = dwHigh;
						lastestUpdateTime.dwLowDateTime = dwLow;
						DEBUG_TRACE(_T("[INSTALL DATE]KB%d %s\n"), nKBID, strDate);
					}

					// KB 必须有Install Time 
					if( hasInstallTime )
					{
						fixedItem.nID = nKBID;
						fixedItem.strInstallDate = strDate;
						arrKBID[nKBID] = fixedItem;
					}
				}
			}
		}
	};
	RegCloseKey(hKey);
	return TRUE;
}


DWORD WINAPI CWUAUpdateInfoVistaLater::ThreadFunc_Scan( LPVOID lpParam )
{
	CWUAUpdateInfoVistaLater *pThis = (CWUAUpdateInfoVistaLater*) lpParam;
	ATLASSERT( lpParam );
	T_ComInit __init__com__;
	pThis->_Scan();
	pThis->SetCompleted( TRUE );
	return 0;
}

CWUAUpdateInfoVistaLater::CWUAUpdateInfoVistaLater()
	: m_bDataAvailable(FALSE)
{
	m_LastestUpdateTime.dwHighDateTime = m_LastestUpdateTime.dwLowDateTime = 0;
	m_hThread = NULL;
	m_bCompleted = FALSE;
}

CWUAUpdateInfoVistaLater::~CWUAUpdateInfoVistaLater()
{
	if(m_hThread)
	{
		SafeTerminateThread( m_hThread );
	}
}

void CWUAUpdateInfoVistaLater::Init(DWORD dwFlags)
{
	if( IsIUpdateResultAvailable() )
	{
		return;
	}
		
	// 如果线程还存在的话, 则等待结果 
	if(m_hThread)
	{
		DWORD dwExitCode;
		if( GetExitCodeThread( m_hThread , &dwExitCode ) )
		{
			if( dwExitCode != STILL_ACTIVE )
			{
				CloseHandle(m_hThread);
				m_hThread = NULL;
			}
		}
	}
	
	SetCompleted(FALSE);
	if(!m_hThread)
		m_hThread = (HANDLE) CreateThread(NULL, 0, ThreadFunc_Scan, (void*)this, 0, NULL);
	return;
}

BOOL CWUAUpdateInfoVistaLater::IsUpdateInstalled( int nKBID )
{
	nKBID = GetRealKBID(nKBID);
	return m_arrInstalledKB.find( nKBID )!=m_arrInstalledKB.end();
}

BOOL CWUAUpdateInfoVistaLater::IsUpdateAvailable( int nKBID )
{
	nKBID = GetRealKBID(nKBID);
	return m_arrInstallableKB.find( nKBID )!=m_arrInstallableKB.end();
}

M_KBInfo & CWUAUpdateInfoVistaLater::GetInstalledInfo()
{
	return m_arrInstalledKB;
}

void CWUAUpdateInfoVistaLater::_Scan()
{
	m_bDataAvailable = FALSE;
	m_arrInstalledKB.clear();
	m_arrInstallableKB.clear();
	
	ATLVERIFY( _RegScanInstalled( m_arrInstalledKB, m_LastestUpdateTime ) );	
	ReadInstalledOffice( m_arrInstalledKB );
	try
	{
		WUApiLib::IUpdateSessionPtr spUpdateSession;
		spUpdateSession.CreateInstance( __uuidof(WUApiLib::UpdateSession) );
		if(!spUpdateSession)
		{
			DEBUG_TRACE(_T("!spUpdateSession \n"));
			return; 
		}
		
		WUApiLib::IUpdateSearcherPtr spUpdateSearch;
		spUpdateSearch = spUpdateSession->CreateUpdateSearcher();
		if(!spUpdateSearch)
		{
			DEBUG_TRACE(_T("!spUpdateSearch \n"));
			return; 
		}

		DEBUG_TRACE(_T("!Begin Get \n"));
		_ReadUpdateInfo(spUpdateSearch, TRUE, m_arrInstalledKB);
		_ReadUpdateInfo(spUpdateSearch, FALSE, m_arrInstallableKB);
		m_bDataAvailable = TRUE;
		m_dwLastResultTime = GetTickCount();
		DEBUG_TRACE(_T("!Get Update DONE \n"));

#ifdef _DEBUG
		M_KBInfo::iterator it;

		DEBUG_TRACE(_T("\n!Installed (%d)\n"), m_arrInstalledKB.size());
		for(it=m_arrInstalledKB.begin(); it!=m_arrInstalledKB.end(); ++it)
		{
			DEBUG_TRACE(_T("%d "), it->second.nID );
		}

		DEBUG_TRACE(_T("\n!Not Installed (%d)\n"), m_arrInstallableKB.size());
		for(it=m_arrInstallableKB.begin(); it!=m_arrInstallableKB.end(); ++it)
		{
			DEBUG_TRACE(_T("%d "), it->second.nID );
		}
		DEBUG_TRACE(_T("\n\n"));
#endif

	}
	catch (...)
	{
		_tcprintf(_T("\n!CWUAUpdateInfoVistaLater::Init exception \n"));
	}
}

BOOL CWUAUpdateInfoVistaLater::_ReadUpdateInfo( WUApiLib::IUpdateSearcher *spUpdateSearch, BOOL installed, M_KBInfo &arrKBID )
{
	spUpdateSearch->Online = VARIANT_TRUE;
	
	CString strQuery;
	strQuery.Format(_T("IsInstalled=%d"), installed ? 1 : 0 );
	_bstr_t bstr = strQuery;
	
	WUApiLib::ISearchResultPtr spSearchResult;		
	spSearchResult = spUpdateSearch->Search( bstr );
	DEBUG_TRACE(_T("\n!spUpdateSearch->Search( bstr ) 0x%x %d\n"), spSearchResult, spSearchResult->ResultCode);

	//yykxx  orcSucceededWithErrors 说明扫完但是不完整
	if(!spSearchResult /*|| WUApiLib::orcSucceeded != spSearchResult->ResultCode*/)
		return FALSE; 
	
	if( WUApiLib::orcSucceeded != spSearchResult->ResultCode && 
		WUApiLib::orcSucceededWithErrors != spSearchResult->ResultCode )
		return FALSE;

	DEBUG_TRACE(_T("\n!spUpdateSearch->Search( bstr ) \n"));

	WUApiLib::IUpdateCollectionPtr spColl;
	spColl = spSearchResult->Updates;
	if(!spColl)
		return FALSE;
	
	DEBUG_TRACE(_T("\n!spSearchResult->Updates \n"));
	long nCount = spColl->Count;			
	for(long i=0; i<nCount; ++i)
	{
		WUApiLib::IUpdatePtr spUpdate;
		spUpdate = spColl->Item[i];
		if(!spUpdate) continue;

		WUApiLib::IStringCollectionPtr spStrColl;
		spStrColl = spUpdate->KBArticleIDs;
		if(!spStrColl) continue;

		long nKBArticlesCount = spStrColl->Count;
		for(long kb = 0; kb < nKBArticlesCount; ++kb)
		{
			// 这里的KB 不带-v2 字样 
			int nKBID = _ttoi( (LPCTSTR)spStrColl->Item[kb] );
			if( nKBID>0 && arrKBID.find(nKBID)==arrKBID.end() )
			{
				TItemFixed fixedItem;
				fixedItem.nID = nKBID;
				arrKBID[ nKBID ] = fixedItem;
			}
		}
	}
	return TRUE;
}

BOOL CWUAUpdateInfoVistaLater::IsIUpdateResultAvailable()
{
	return m_bDataAvailable && m_dwLastModifiedTime<=m_dwLastResultTime;
}

void CWUAUpdateInfoVistaLater::SetCompleted( BOOL bCompleted )
{
	CObjGuard _guard_(m_objLock);
	m_bCompleted = bCompleted;
}

BOOL CWUAUpdateInfoVistaLater::IsCompleted()
{
	return m_bCompleted;
}

CWUAUpdateInfoVistaLaterReg::CWUAUpdateInfoVistaLaterReg()
{

}

void CWUAUpdateInfoVistaLaterReg::Init( DWORD dwFlags )
{
	m_arrInstalledKB.clear();
	_RegScanInstalled( m_arrInstalledKB, m_LastestUpdateTime );	
	ReadInstalledOffice( m_arrInstalledKB );
}

BOOL CWUAUpdateInfoVistaLaterReg::IsUpdateAvailable( int nKBID )
{
	return !IsUpdateInstalled(nKBID);
}

BOOL CWUAUpdateInfoVistaLaterReg::IsUpdateInstalled( int nKBID )
{
	nKBID = GetRealKBID(nKBID);
	return m_arrInstalledKB.find( nKBID )!=m_arrInstalledKB.end();
}

M_KBInfo & CWUAUpdateInfoVistaLaterReg::GetInstalledInfo()
{
	return m_arrInstalledKB;
}
