#pragma once
#include "vulfix/Defines.h"
#include "Utils.h"
#include <map>
#include <common/registrywow.h>
#include <communits/CommFuns.h>

struct T_Environment
{
	T_Environment()
	{
		Init();
	}
	
	void Init()
	{
		lang = GetLangID();
		//isWin64 = IsWin64();
	}
	
public:
	LANGID lang;
	BOOL isVistaAfter;
	BOOL isWin64;
};

int ParseVerString( LPCTSTR lpszVer, TCHAR chDelemeter=_T('.') );

class ISoftInfo
{
public:
	ISoftInfo() : m_nVer(-1), m_nSP(-1){} 
	virtual ~ISoftInfo() {} 
	virtual BOOL TryReadInfo() = 0;
	virtual BOOL TestCondition(const TOptionCondition &condition, DWORD &dwLangRequired) = 0;
	
public:
	int m_nVer, m_nSP;
};

struct TOfficeVersion
{
	TOfficeVersion()
	{
		type = 0;
		strName = strVFrom = strVTo = _T("");
	}
	TOfficeVersion(int tp, LPCTSTR szName, LPCTSTR szVFrom, LPCTSTR szVTo)
		: type(tp), strName(szName), strVFrom(szVFrom), strVTo(szVTo)
	{
	}

	int		type;			// office 类型
	CString strName;		// office 版本描述, 用以匹配 office 
	CString strVFrom, strVTo; // office 版本区间
};

template<INT T_REGISTRY>
class COfficeInfo;
class COfficeInfoWrapper;
BOOL IsAdministratorUser();

struct CSysEnv
{
public:
	void Init();
	BOOL CheckPrequisites( const TOptionCondition &condition, DWORD &dwLang );
	void InitOffice( const CSimpleArray<TOfficeVersion> &officeTypes );
	HRESULT IsSupported( BOOL bRequireAdmin );

public:
	BOOL IsLangSupported();
	BOOL IsOsSupported();
	BOOL IsOfficeLangSupported();

public:
	BOOL isAdmin;
	BOOL isWin64;
	LANGID lang;
	int m_WinVer, m_WinSP;
	
	CSimpleArray<ISoftInfo*> m_arrSofts;
	COfficeInfoWrapper *m_pOfficeInfo;

public:
	CSysEnv() {} 
};

class CWindowsInfo : public ISoftInfo 
{
public:
	CWindowsInfo(){
		m_nVer = WINUNKNOWN;
		m_nSP = 0;
	}
	
	virtual BOOL TryReadInfo()
	{
		GetWinVerInfo(m_nVer, m_nSP);
		return TRUE;
	}
	
	virtual BOOL TestCondition(const TOptionCondition &condition, DWORD &dwLangRequired)
	{
		if (condition.nWinVer!=-1 && condition.nWinVer!=m_nVer )
			return FALSE;
		if (condition.nWinSP!=-1 && condition.nWinSP!=m_nSP)
			return FALSE;
		return TRUE;
	}
};

class CInternetExplorer : public ISoftInfo 
{
public:
	virtual BOOL TryReadInfo()
	{
		return _ReadIEVersion() && _ReadIESP();
	}
	
	virtual BOOL TestCondition(const TOptionCondition &condition, DWORD &dwLangRequired)
	{
		// ie 
		if (condition.nIEVer!=-1 && condition.nIEVer!=m_nVer )
			return FALSE;
		if (condition.nIESP!=-1 && condition.nIESP!=m_nSP )
			return FALSE;
		return TRUE;
	}
	
protected:
	BOOL _ReadIEVersion();

	BOOL _ReadIESP()
	{
		CString strVer; 
		if( ReadRegString(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), _T("MinorVersion"), strVer) )
		{
			if( strVer.Left(3).CompareNoCase(_T(";SP"))==0 )
			{
				m_nSP = _ttoi( strVer.Mid(3) );
			}
		}
		return TRUE;
	}
};

class CWSScriptInfo : public ISoftInfo 
{
public:
	virtual BOOL TryReadInfo()
	{
		CString strVer;
		if(ReadRegString(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\{4F645220-306D-11D2-995D-00C04F98BBC9}"), _T("Version"), strVer))
		{
			m_nVer = ParseVerString( strVer, _T(',') );
			return TRUE;
		}
		return FALSE;
	}
	
	virtual BOOL TestCondition(const TOptionCondition &condition, DWORD &dwLangRequired)
	{
		if (condition.nJS!=-1 && condition.nJS!=m_nVer)
			return FALSE;
		return TRUE;
	}
};

class CMediaplayerInfo : public ISoftInfo 
{
public:
	virtual BOOL TryReadInfo()
	{
		CString strVer;
		if(ReadRegString(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\MediaPlayer\\PlayerUpgrade"), _T("PlayerVersion"), strVer))
		{
			// 9,0,0,4503
			m_nVer = ParseVerString( strVer, _T(',') );
			return TRUE;
		}
		return 0;
	}

	virtual BOOL TestCondition(const TOptionCondition &condition, DWORD &dwLangRequired)
	{
		if (condition.nMediaplayer!=-1 && condition.nMediaplayer!=m_nVer)
			return FALSE;
		return TRUE;
	}
};

class CDirectXInfo : public ISoftInfo 
{
public:
	virtual BOOL TryReadInfo()
	{
		DWORD dwMajor = 0, dwMinor = 0;
		if( FAILED(GetDirectXVerionViaFileVersions(&dwMajor, &dwMinor, NULL)) )
			return FALSE;
		m_nVer = dwMajor*1000 + dwMinor;
		return TRUE;
	}
	
	virtual BOOL TestCondition(const TOptionCondition &condition, DWORD &dwLangRequired)
	{
		if (condition.nDX!=-1 && condition.nDX!=m_nVer)
			return FALSE;
		return TRUE;
	}
};

class COutLookExpressInfo : public ISoftInfo 
{
public:
	virtual BOOL TryReadInfo()
	{
		CString strVer;
		if(ReadRegString(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Outlook Express\\Version Info"), _T("Current"), strVer))
		{
			// 6,0,2900,5512
			m_nVer = ParseVerString( strVer, _T(',') );
			return TRUE;
		}
		return FALSE;
	}
	
	virtual BOOL TestCondition(const TOptionCondition &condition, DWORD &dwLangRequired)
	{
		if (condition.nOE!=-1 && condition.nOE!=m_nVer)
			return FALSE;
		return TRUE;
	}
};

class CDataAccessInfo : public ISoftInfo 
{
public:
	virtual BOOL TryReadInfo()
	{
		CString strVer;
		if(ReadRegString(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\DataAccess"), _T("Version"), strVer))
		{
			// 2.81.1132.0
			m_nVer = ParseVerString( strVer );
			return TRUE;
		}
		return FALSE;
	}

	virtual BOOL TestCondition(const TOptionCondition &condition, DWORD &dwLangRequired)
	{
		if (condition.nDataAccess!=-1 && condition.nDataAccess!=m_nVer)
			return FALSE;
		return TRUE;
	}
};

class CDotNetFrameworkInfo : public ISoftInfo 
{
public:
	virtual BOOL TryReadInfo()
	{
		HKEY hKey;	
		if( ERROR_SUCCESS!=RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
			_T("SOFTWARE\\Microsoft\\.NETFramework\\policy\\standards\\v1.0.0"), 0, KEY_READ, &hKey) )
			return FALSE;
		
		TCHAR szKey[MAX_PATH];
		TCHAR szValue[MAX_PATH];
		for(DWORD dwIndex = 0;; dwIndex++)
		{
			// v2.0.50727
			DWORD dwcbKey = MAX_PATH;
			DWORD dwcbValue = MAX_PATH;
			
			DWORD nValueType;
			LONG lResult = SHEnumValue(hKey, dwIndex, szKey, &dwcbKey, &nValueType, szValue, &dwcbValue);
			
			if(ERROR_SUCCESS!=lResult)
				break;
			if(REG_DWORD==nValueType)
			{
				m_Versions.Add( ParseVerString( szKey+1, _T('.') ) );
			}
		}
		RegCloseKey(hKey);
		return TRUE;
	}
	
	virtual BOOL TestCondition(const TOptionCondition &condition, DWORD &dwLangRequired)
	{
		if (condition.nFramework!=-1)
		{
			return m_Versions.Find( condition.nFramework )!=-1;
		}
		return TRUE;
	}
	
protected:
	CSimpleArray<int> m_Versions;
};

class CXmlCoreInfo : public ISoftInfo 
{
public:
	virtual BOOL TryReadInfo()
	{
		HKEY hKey;
		if( ERROR_SUCCESS!=RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
			_T("SOFTWARE\\Classes\\TypeLib\\{F5078F18-C551-11D3-89B9-0000F81FE221}"), 0, KEY_READ, &hKey) )
			return FALSE;
		
		CString strVer;
		TCHAR szName[MAX_PATH];
		for(DWORD dwIndex = 0;; dwIndex++)
		{
			DWORD dwcbName = MAX_PATH;
			LONG lResult = SHEnumKeyEx(hKey, dwIndex, szName, &dwcbName);
			if (lResult != ERROR_SUCCESS)
				break;
			m_Versions.Add( ParseVerString(szName) );
		}
		RegCloseKey(hKey);
		return TRUE;
	}
	
	virtual BOOL TestCondition(const TOptionCondition &condition, DWORD &dwLangRequired)
	{
		if (condition.nXML!=-1)
		{
			return m_Versions.Find( condition.nXML )!=-1;
		}
		return TRUE;
	}

protected:
	CSimpleArray<int> m_Versions;
};

struct T_OfficeFileEntry
{
	T_OfficeFileEntry()
	{
		type = VTYPE_UNKOWN;
		dwLangID = 2056;
		strFilePath = _T("");
		lVersion.HighPart = 0;
		lVersion.LowPart = 0;
		nOfficeVer = 0;
	}
	int				type;			//Office 类型
	DWORD			dwLangID;		//语言版本
	CString			strFilePath;	//文件路径
	LARGE_INTEGER	lVersion;		//文件的版本
	int				nOfficeVer;		// office 版本
};

template<INT T_REGISTRY>
class COfficeInfo : public ISoftInfo 
{
public:
	COfficeInfo() : m_Inited(FALSE)
	{		
	} 
	
	virtual BOOL TryReadInfo()
	{
		return TRUE;
	}
	
	virtual BOOL TestCondition(const TOptionCondition &condition, DWORD &dwLangRequired)
	{
		if(condition.strOffice.IsEmpty() || !m_Inited)
			return TRUE;
		
		CSimpleArray<CString> versions;
		if( Split(condition.strOffice, versions, _T(',')) )
		{
			CObjGuard guard(m_objLock);
			for(int i=0; i<versions.GetSize(); ++i)
			{
				int nOfficeType = 0;
				if(!GetOfficeInfo(versions[i], nOfficeType, dwLangRequired))
					return FALSE;
			}
		}
		return TRUE;
	}
	
	void Init(const CSimpleArray<TOfficeVersion> &arrOfficeVersions);
	BOOL GetOfficeInfo(LPCTSTR lpszOfficeVersion, int &nOfficeType, DWORD &dwLang) const;
	BOOL IsLangSupported();

	BOOL ReloadOfficeInfo();

protected:
	BOOL _FoundOfficeProduct( int nOfficeVer, int nOfficeType, DWORD dwLang, LPCTSTR szFilename );
	DWORD _GetOfficeLang( int nOfficeVer );
	BOOL _CheckOfficeExists( int nVer, CString &strOfficePath );
	void _IsProductExists( int nVer, LPCTSTR lpRegKey, LPCTSTR lpExeName, int officetype );
	BOOL _GetCommonFile( int nVer, CString &strFilename );
	BOOL _CheckOfficeProductsPath(LPCTSTR lpRegName, LPCTSTR lpPath);
	BOOL _ReloadVisioInfo( int nOfficeVer, const CString& strOfficePath, DWORD dwLang, int nIndex, BOOL bOnlyCheckIfExist = FALSE );
	
protected:
public:
	CSimpleArray<T_OfficeFileEntry> m_arrVIFiles;
	struct _TOfficeInfo {
		int nOfficeType;	
		DWORD dwLang;		
	};
	typedef std::map<CString, _TOfficeInfo> _TMapOfficeversionLang;
	_TMapOfficeversionLang m_OfficeLangs;

	BOOL m_Inited;

	CObjLock	m_objLock;
	CRegistryWow<T_REGISTRY>		m_reg;
};

class COfficeInfoWrapper : public ISoftInfo
{
public:
	#define OFFICE_INFO_X64_FLAG _T("x64")
	
	COfficeInfoWrapper() 
	{
		m_bWinX64		= IsWin64();
	}
	
	virtual BOOL TryReadInfo()
	{ ///> 原 COfficeInfo没有处理，仅返回TRUE，这里无须处理
		return TRUE;
	}
	
	virtual BOOL TestCondition(const TOptionCondition &condition, DWORD &dwLangRequired)
	{
		ATLTRACE( L"[TestCondition<win64:%d>] strOffice:%s", m_bWinX64, condition.strOffice);
		
		if(condition.strOffice.IsEmpty() || !m_officeReg32.m_Inited)
			return TRUE;
		
		CSimpleArray<CString> versions;
		if( Split(condition.strOffice, versions, _T(',')) )
		{
			CObjGuard guard(m_objLock);
			for(int i=0; i<versions.GetSize(); ++i)
			{
				int nOfficeType = 0;
				if(!GetOfficeInfo(versions[i], nOfficeType, dwLangRequired))
					return FALSE;
			}
		}
		return TRUE;
	}
	
	BOOL GetOfficeInfo(LPCTSTR lpszOfficeVersion, int &nOfficeType, DWORD &dwLang) const
	{
		// ATLTRACE( L"[GetOfficeInfo<win64:%d>] strOffice:%s", m_bWinX64, lpszOfficeVersion);

		if ( m_bWinX64 )
		{ ///> 

			BOOL b64 = FALSE;
			CString strOffice = lpszOfficeVersion;
			strOffice.MakeLower();
			if ( !strOffice.IsEmpty() 
				&& ( -1 != strOffice.Find( OFFICE_INFO_X64_FLAG ) ) )
			{
				strOffice.Replace(OFFICE_INFO_X64_FLAG, _T(""));
				b64 = TRUE;
			}
			return ( b64 ? m_officeReg64.GetOfficeInfo( strOffice, nOfficeType, dwLang ) 
				: m_officeReg32.GetOfficeInfo( strOffice, nOfficeType, dwLang ) );
		}
		else
		{
			return m_officeReg32.GetOfficeInfo( lpszOfficeVersion, nOfficeType, dwLang );
		}
	}

	void Init(const CSimpleArray<TOfficeVersion> &arrOfficeVersions)
	{
		if ( m_bWinX64 )
		{
			m_officeReg32.Init( arrOfficeVersions );
			m_officeReg64.Init( arrOfficeVersions );
		}
		else
		{///> x86下无须考虑x64问题
			m_officeReg32.Init( arrOfficeVersions );
		}
	}
	

	BOOL IsLangSupported()
	{
		if ( m_bWinX64 )
		{ ///> 
			BOOL bRet = FALSE;
			bRet |= m_officeReg64.IsLangSupported();
			bRet |= m_officeReg32.IsLangSupported();

			return bRet;
		}
		else
		{
			return m_officeReg32.IsLangSupported();
		}
	}

protected:
	BOOL m_bWinX64;
													///>    32位系统注册表中对应的office        64位系统注册表中对应的office 
	COfficeInfo<REGISTRY_NORMAL>	m_officeReg32;	///>			32									64
	COfficeInfo<REGISTRY_WOW>		m_officeReg64;	///>			xxx									32

	CObjLock	m_objLock;
};