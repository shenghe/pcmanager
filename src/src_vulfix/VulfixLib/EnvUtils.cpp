#include "StdAfx.h"
#include "EnvUtils.h"
#include "Utils.h"
#include <ShlObj.h>

int ParseVerString( LPCTSTR lpszVer, TCHAR chDelemeter )
{
	int nMajor, nMinor;
	LPCTSTR lpszFormat = NULL;
	if(chDelemeter==_T('.'))
		lpszFormat = _T("%d.%d");
	else if(chDelemeter==_T(','))
		lpszFormat = _T("%d,%d");
	else
		ATLASSERT(FALSE);

	_stscanf(lpszVer, lpszFormat, &nMajor, &nMinor);
	return nMajor*1000 + nMinor;
}

BOOL IsAdministratorUser()
{
	static HMODULE hModule = NULL;
	if( !hModule )
		hModule = LoadLibrary(_T("shell32.dll"));
	if( !hModule )
		return TRUE;

	typedef BOOL (__stdcall *FunctionIsUserAdmin)();
	FunctionIsUserAdmin pfnIsUserAnAdmin = (FunctionIsUserAdmin)GetProcAddress(hModule, "IsUserAnAdmin");
	if (pfnIsUserAnAdmin)
		return pfnIsUserAnAdmin();
	return TRUE;
}

void CSysEnv::Init()
{
	static CThreadGuard	locker;
	autolock<CThreadGuard> _lk(locker);

	lang = GetLangID();
	isAdmin = IsAdministratorUser();
	isWin64 = IsWin64();
	
	static ISoftInfo *pWindowsInfo=NULL;

	m_pOfficeInfo = NULL;
	if(m_arrSofts.GetSize()==0)
	{
		pWindowsInfo = new CWindowsInfo;
		m_arrSofts.Add( pWindowsInfo );
		m_arrSofts.Add( new CInternetExplorer );

		m_arrSofts.Add( new CWSScriptInfo );
		m_arrSofts.Add( new CMediaplayerInfo );
		m_arrSofts.Add( new CDirectXInfo );
		m_arrSofts.Add( new COutLookExpressInfo );
		m_arrSofts.Add( new CDataAccessInfo );

		m_arrSofts.Add( new CDotNetFrameworkInfo );
		m_arrSofts.Add( new CXmlCoreInfo );
	}
	
	for(int i=0; i<m_arrSofts.GetSize(); ++i)
	{
		m_arrSofts[i]->TryReadInfo();
	}
	
	if(pWindowsInfo)
	{
		m_WinVer = pWindowsInfo->m_nVer;
		m_WinSP = pWindowsInfo->m_nSP;
	}

#if 0
	CString strAll;
	// 系统
	static LPCTSTR szTitles[] = {
		_T("Windows"),
		_T("IE"),
		_T("Script"),
		_T("MediaPlayer"),
		_T("DirectX"),
		_T("Outlook"),
		_T("DataAccess"),
		_T("DotFramework"),
		_T("Xml"),
	};
	
	strAll.Format(_T("ISAdmin:%d  IsWin64:%d  Lang:%d\n"), isAdmin?1:0, isWin64?1:0, lang);
	for(int i=0; i<m_arrSofts.GetSize(); ++i)
	{
		strAll.AppendFormat(_T("%s : %d - %d \n"), szTitles[i], m_arrSofts[i]->m_nVer, m_arrSofts[i]->m_nSP);
	}
	MessageBox(NULL, strAll, NULL, MB_OK);
#endif 
}

BOOL CSysEnv::CheckPrequisites( const TOptionCondition &condition, DWORD &dwLang )
{
	BOOL bRet = TRUE;
		
	dwLang = 0;
	for(int i=0; i<m_arrSofts.GetSize(); ++i)
	{
		if( !m_arrSofts[i]->TestCondition(condition, dwLang) )
		{
			bRet = FALSE;
			break;
		}
	}
	
	if(bRet)
	{
		if( m_pOfficeInfo && !m_pOfficeInfo->TestCondition(condition, dwLang) )
			bRet = FALSE;
	}
	
	if(bRet)
	{
		if(dwLang == 0)
			dwLang = lang;
	}

	ATLTRACE(_T("Env::CheckPrequisites(office=%s, condition=%s) -> lang:%d, ret:%d \n"), condition.strOffice, condition.m_strCondition, dwLang, bRet);
	return bRet;
}

void CSysEnv::InitOffice( const CSimpleArray<TOfficeVersion> &officeTypes )
{
	if(!m_pOfficeInfo)
		m_pOfficeInfo = new COfficeInfoWrapper;
	if(m_pOfficeInfo)
		m_pOfficeInfo->Init( officeTypes );
}

HRESULT CSysEnv::IsSupported( BOOL bRequireAdmin )
{
	HRESULT err = KERR_GENERAL;
	// 移到CImplVulfix::Scan(DWORD dwFlags) 中去了, load 系统库的时候进行校验 

	//if( !IsLangSupported() )
	//	return KERR_LANG_NOT_SUPPORTED;

	//if( !IsOsSupported() )
	//	return KERR_OS_NOT_SUPPORTED;
	
	if( bRequireAdmin && !IsAdministratorUser() )
		return KERR_ADMIN_REQUIRED;

	return KERR_NONE;
}

BOOL CSysEnv::IsLangSupported()
{
	DWORD dwLang = GetLangID();
	return dwLang==ESYSLang_CN || dwLang==ESYSLang_EN;
}

BOOL CSysEnv::IsOsSupported()
{
	return WIN2K==m_WinVer||WINXP==m_WinVer||WIN2003==m_WinVer||WINVISTA==m_WinVer||WIN7==m_WinVer;
}

BOOL CSysEnv::IsOfficeLangSupported()
{
	return m_pOfficeInfo && m_pOfficeInfo->IsLangSupported();
}

BOOL CInternetExplorer::_ReadIEVersion()
{
	LARGE_INTEGER ll;
	CString strVer;
	CString strInterExplorerExefile = _T("@PROGRAM_FILES\\Internet Explorer\\IEXPLORE.EXE");
	ExpandFilePath(strInterExplorerExefile);
	ll.QuadPart = 0;
	if(PathFileExists(strInterExplorerExefile) && GetFileVersion(strInterExplorerExefile, ll))
	{
		strVer.Format(
			_T("%d.%d.%d.%d"), 
			HIWORD(ll.HighPart), LOWORD(ll.HighPart), 
			HIWORD(ll.LowPart), LOWORD(ll.LowPart)
			);
	}
	else
		ReadRegString(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Internet Explorer"), _T("Version"), strVer);

	m_nVer = ParseVerString(strVer);
	return m_nVer>0;
}


struct T_PairStrStr
{
	int nType;
	LPCTSTR  szKey1, szKey2;
};
static T_PairStrStr _office_products[] =
{
	{1, _T("access"),		_T("msaccess")	},
	{2, _T("excel"),		_T("excel")		},
	{3, _T("outlook"),		_T("outlook")	},
	{4, _T("powerpoint"),	_T("powerpnt")	},
	{5, _T("word"),			_T("winword")	},
	{6, _T("visio"),		_T("visio")		},
};

#define OFFICE_PRODUCTS_VISIO_TYPE _office_products[5].nType

template<INT T_REGISTRY>
void COfficeInfo<T_REGISTRY>::Init(const CSimpleArray<TOfficeVersion> &arrOfficeVersions)
{
	CObjGuard guard(m_objLock);

	ReloadOfficeInfo();	
	m_OfficeLangs.clear();
	for( int i = 0; i < m_arrVIFiles.GetSize(); i ++ )
	{
		const T_OfficeFileEntry &verfile = m_arrVIFiles[i];

		for(int j=0; j<arrOfficeVersions.GetSize(); ++j)
		{
			const TOfficeVersion &ver = arrOfficeVersions[j];

			// 如果符合区间
			if( ver.type==verfile.type )
			{
				// 检测office 的大版本
				int nOfficeVer = 0;
				if(!ver.strVFrom.IsEmpty())
					nOfficeVer = _ttoi(ver.strVFrom);
				else if(!ver.strVTo.IsEmpty())
					nOfficeVer = _ttoi(ver.strVTo);
				
				if( nOfficeVer!=0 && nOfficeVer==verfile.nOfficeVer && VersionInRange(verfile.lVersion, ver.strVFrom, ver.strVTo) )
				{
					DWORD dwLang = 0;
					dwLang = verfile.dwLangID;

					_TOfficeInfo _officeinfo = {ver.type, dwLang};				
					CString strVer = ver.strName;
					strVer.MakeLower();
					m_OfficeLangs[ strVer ] = _officeinfo;
					break;
				}
			}
		}
	}

#ifdef _DEBUG
	for( _TMapOfficeversionLang::iterator it=m_OfficeLangs.begin(); it!=m_OfficeLangs.end(); ++it )
	{
		ATLTRACE( L"[Init<%d>] ver:%s, lang:%d, type:%d",
			T_REGISTRY,
			it->first,
			it->second.dwLang,
			it->second.nOfficeType );
	}
#endif

	m_Inited = TRUE;
}

template<INT T_REGISTRY>
BOOL COfficeInfo<T_REGISTRY>::GetOfficeInfo( LPCTSTR lpszOfficeVersion, int &nOfficeType, DWORD &dwLang ) const 
{
	if(lpszOfficeVersion==NULL) return FALSE;

	CString strVer = lpszOfficeVersion;
	strVer.MakeLower();
	_TMapOfficeversionLang::const_iterator it = m_OfficeLangs.find( strVer );
	if(it==m_OfficeLangs.end())
		return FALSE;
	nOfficeType = it->second.nOfficeType;
	dwLang = it->second.dwLang;
	return	TRUE;
}

template<INT T_REGISTRY>
BOOL COfficeInfo<T_REGISTRY>::IsLangSupported()
{
	for(int i=0; i<m_arrVIFiles.GetSize(); ++i)
	{
		const T_OfficeFileEntry &verfile = m_arrVIFiles[i];
		if(ESYSLang_CN==verfile.dwLangID || ESYSLang_EN==verfile.dwLangID)
			return TRUE;
	}
	return FALSE;
}

template<INT T_REGISTRY>
BOOL COfficeInfo<T_REGISTRY>::ReloadOfficeInfo() 
{
	m_arrVIFiles.RemoveAll();

	HKEY hKey = NULL;
	m_reg.OpenKeyEx( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Office"), 0, KEY_READ, &hKey );
	if( !hKey ) 
		return FALSE;
	
	DWORD cchName=MAX_PATH, dwIndex;
	TCHAR szName[MAX_PATH];

	dwIndex = 0;
	while( ERROR_SUCCESS==RegEnumKey(hKey, dwIndex++, szName, cchName) )
	{
		if( !IsNumber(szName) )
			continue;
		int nOfficeVer = _ttoi(szName);
		CString strOfficePath;
		if(_CheckOfficeExists(nOfficeVer, strOfficePath))
		{
			DWORD dwLang = _GetOfficeLang(nOfficeVer);
			if(ESYSLang_CN==dwLang || ESYSLang_EN==dwLang)
			{
				// add office info  
				CString strFilename;
				_GetCommonFile(nOfficeVer, strFilename);
				_FoundOfficeProduct(nOfficeVer, 0, dwLang, strFilename);

				// loop office products  
				for(int i=0; i< (sizeof(_office_products)/sizeof(_office_products[0])); ++i)
				{
					int officetype = _office_products[i].nType;
					LPCTSTR lpRegKey = _office_products[i].szKey1;
					LPCTSTR lpExeName = _office_products[i].szKey2;
					strFilename.Format(_T("%s\\%s.exe"), strOfficePath, lpExeName);
					if ( OFFICE_PRODUCTS_VISIO_TYPE == officetype )
					{
						_ReloadVisioInfo( nOfficeVer, strOfficePath, dwLang, i );
					}
					else
					{
						if( PathFileExists(strFilename) )
						{
							_FoundOfficeProduct(nOfficeVer, officetype, dwLang, strFilename);
						}
					}
				}
			}			
		}
		cchName=MAX_PATH;
	}
	RegCloseKey(hKey);


#ifdef _DEBUG
	for ( int i = 0 ; i < m_arrVIFiles.GetSize() ; i ++ )
	{
		T_OfficeFileEntry officeFileEntry = m_arrVIFiles[i];
		ATLTRACE( L"[ReloadOfficeInfo<%d>] ver:%d, lang:%d, type:%d, fileVer:%I64d, filePath:%s",
			T_REGISTRY,
			officeFileEntry.nOfficeVer,
			officeFileEntry.dwLangID,
			officeFileEntry.type,
			officeFileEntry.lVersion.QuadPart, 
			officeFileEntry.strFilePath );
	}
#endif 

	return ( m_arrVIFiles.GetSize() > 0 );
}

template<INT T_REGISTRY>
BOOL COfficeInfo<T_REGISTRY>::_ReloadVisioInfo( int nOfficeVer, const CString& strOfficePath, DWORD dwLang, int nIndex, BOOL bOnlyCheckIfExist /* = FALSE */ )
{
	if ( nIndex < 0 )
	{
		return FALSE;
	}

	CString strFilename;
	if ( 11 == nOfficeVer )
	{
		///> visio2003的installRoot放在与office common中会导致
		///> visio2003先安装时，这个路径会被office改写， 这种情况下，仅支持默认路径安装的visio2003
		///> visio2003后安装时，这个路径会被visio改写，  这种情况下，visio2003可以正常地支持
		
		///> 1、看office2003安装目录的并行目录是否存在
		strFilename = strOfficePath;
		strFilename.TrimRight( _T('\\') );
		int nTrim = strFilename.ReverseFind( _T('\\') );
		if ( -1 != nTrim )
		{
			strFilename = strFilename.Left( nTrim + 1 );
			strFilename += _T("Visio11\\");
			strFilename.AppendFormat( L"%s.exe", _office_products[ nIndex ].szKey2 );

			if ( PathFileExists(strFilename) )
			{
				return bOnlyCheckIfExist ? TRUE : _FoundOfficeProduct( nOfficeVer, _office_products[ nIndex ].nType, dwLang, strFilename );
			}
		}

		strFilename.Empty();


		///> 2、看注册表中写入的installRoot路径的并行目录是否存在
		CString strSubKey;	
		strSubKey = _T("SOFTWARE\\Microsoft\\Office\\11.0\\Common\\InstallRoot");
		if( !m_reg.ReadString( HKEY_LOCAL_MACHINE, strSubKey, _T("Path"), strFilename ) )
			return FALSE;

		strFilename.TrimRight( _T('\\') );
		nTrim = strFilename.ReverseFind( _T('\\') );
		if ( -1 != nTrim )
		{
			strFilename = strFilename.Left( nTrim + 1 );
			strFilename += _T("Visio11\\");
			strFilename.AppendFormat( L"%s.exe", _office_products[ nIndex ].szKey2 );
			if ( PathFileExists(strFilename) )
			{
				return bOnlyCheckIfExist ? TRUE : _FoundOfficeProduct( nOfficeVer, _office_products[ nIndex ].nType, dwLang, strFilename );
			}
		}
		
		return FALSE;
	}
	else
	{
		///> visio2003之后的版本，默认会安装到office的目录

		///> 1、office的安装目录
		strFilename.Format( _T("%s\\%s.exe"), strOfficePath, _office_products[ nIndex ].szKey2 );
		if ( PathFileExists(strFilename) )
		{
			return bOnlyCheckIfExist ? TRUE : _FoundOfficeProduct( nOfficeVer, _office_products[ nIndex ].nType, dwLang, strFilename );
		}

		///> 2、注册表中得出的路径 HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Office\%d.0\Visio\InstallRoot
		CString strSubKey;	
		strFilename.Empty();
		strSubKey.Format( _T("SOFTWARE\\Microsoft\\Office\\%d.0\\Visio\\InstallRoot"), nOfficeVer );
		if( !m_reg.ReadString( HKEY_LOCAL_MACHINE, strSubKey, _T("Path"), strFilename ) )
			return FALSE;

		strFilename.TrimRight( _T('\\') );
		strFilename.AppendFormat( _T("\\%s.exe"), _office_products[ nIndex ].szKey2 );
		if ( PathFileExists(strFilename) )
		{
			return bOnlyCheckIfExist ? TRUE : _FoundOfficeProduct( nOfficeVer, _office_products[ nIndex ].nType, dwLang, strFilename );
		}
	}

	return FALSE;
}

template<INT T_REGISTRY>
BOOL COfficeInfo<T_REGISTRY>::_CheckOfficeExists( int nVer, CString &strOfficePath )
{
	CString strSubKey;	
	CString strPath;

	// 1. common file 
	_GetCommonFile(nVer, strPath);
	if( !IsFileExist(strPath) )
		return FALSE;
	
	// 2. common dir  
	strSubKey.Format(_T("SOFTWARE\\Microsoft\\Office\\%d.0\\Common\\InstallRoot"), nVer);
	if( !m_reg.ReadString(HKEY_LOCAL_MACHINE, strSubKey, _T("Path"), strPath)
		|| !PathIsDirectory(strPath))
		return FALSE;
	strOfficePath = strPath;

	// 3. 检测有否office 产品安装 
	for(int i=0; i< (sizeof(_office_products)/sizeof(_office_products[0])); ++i)
	{
		int officetype = _office_products[i].nType;
		LPCTSTR lpRegKey = _office_products[i].szKey1;
		LPCTSTR lpExeName = _office_products[i].szKey2;
		
		if ( OFFICE_PRODUCTS_VISIO_TYPE == officetype )
		{
			if ( _ReloadVisioInfo( nVer, strOfficePath, 0, i, TRUE ) )
			{
				return TRUE;
			}
		}
		else
		{
			// 直接检查产品文件是否存在, 而不去检测注册表了: 主要针对精简版!!! 
			CString strProductExe = strPath;
			strProductExe.AppendFormat(_T("\\%s.exe"), lpExeName);
			if( PathFileExists(strProductExe) )
				return TRUE;
		}
	}
	return FALSE;
}

template<INT T_REGISTRY>
BOOL COfficeInfo<T_REGISTRY>::_GetCommonFile( int nVer, CString &strFilename )
{
	if(nVer==9)
	{
		if(!m_reg.ReadString(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Office\\9.0\\Common\\InstallRoot"), _T("path"), strFilename))
			return FALSE;
		strFilename.Append( _T("\\mso9.dll") );
	}
	else
	{
		m_reg.GetProgramfilePath(strFilename.GetBuffer(MAX_PATH), MAX_PATH);
		strFilename.ReleaseBuffer();
		strFilename.AppendFormat( _T("\\Common Files\\Microsoft Shared\\Office%d\\mso.dll"), nVer );
	}
	return TRUE;
}


template<INT T_REGISTRY>
DWORD COfficeInfo<T_REGISTRY>::_GetOfficeLang( int nOfficeVer)
{
	DWORD dwLang = 0;
	CString strSubKey;
	strSubKey.Format( _T("SOFTWARE\\Microsoft\\Office\\%d.0\\Common\\LanguageResources"), nOfficeVer );					
	if( nOfficeVer==9 )
		m_reg.ReadDWord(HKEY_CURRENT_USER, strSubKey, _T("UILanguage"), dwLang);
	else 
		m_reg.ReadDWord(HKEY_LOCAL_MACHINE, strSubKey, _T("SKULanguage"), dwLang );
	return dwLang;
}

template<INT T_REGISTRY>
BOOL COfficeInfo<T_REGISTRY>::_FoundOfficeProduct( int nOfficeVer, int nOfficeType, DWORD dwLang, LPCTSTR szFilename )
{
	LARGE_INTEGER lgVersion;
	if(!GetFileVersion(szFilename, lgVersion))
		return FALSE;

	T_OfficeFileEntry verfile;
	verfile.type = nOfficeType;
	verfile.strFilePath = szFilename;
	verfile.dwLangID = dwLang;
	verfile.lVersion = lgVersion;
	verfile.nOfficeVer = nOfficeVer;
	m_arrVIFiles.Add( verfile );
	return TRUE;
}
