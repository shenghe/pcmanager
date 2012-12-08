#include "StdAfx.h"
#include "avproductmgr.h"
#include "libheader/libheader.h"
#include <Msi.h>
#include "vulfix\BeikeUtils.h"
#include "kgetantivirusproductinfo.h"


#pragma comment(lib, "Msi.lib")
#pragma warning(disable:4018)


#define PRODUCT_GUID_PATH L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\S-1-5-18\\Products"

CAVProductMgr::CAVProductMgr(void)
{
	m_iCompantPos = NULL;
	m_iProductPos = NULL;
}

CAVProductMgr::~CAVProductMgr(void)
{
}

LPCWSTR STDMETHODCALLTYPE CAVProductMgr::Check()
{
	m_iCompantPos = NULL;
	m_iProductPos = NULL;

	POSITION iPos = m_CompanyList.GetHeadPosition();
	while ( iPos != NULL )
	{
		const COMPANY_INFO& ci = m_CompanyList.GetNext(iPos);
		if ( ci.strExpression.IsEmpty() || EvaluateCondition(ci.strExpression) )
		{
			ATLTRACE(_T("EvaluateCondition: Company.strExpression=%s"), ci.strExpression);
			
			POSITION jPos = ci.ProductList.GetHeadPosition();
			while( jPos != NULL )
			{
				const PRODUCT_INFO& pi = ci.ProductList.GetNext(jPos);
				CString strExpression(pi.strExpression);

				ProcessGUID(pi.strGUIDName, strExpression);
				if ( EvaluateCondition(strExpression) )
				{
					ATLTRACE(_T("EvaluateCondition: Product.strExpression=%s"), strExpression);

					m_iCompantPos = iPos;
					m_iProductPos = jPos;
					return pi.strName;
				}
			}
		}
	}

	return NULL;
}

UINT32 STDMETHODCALLTYPE CAVProductMgr::UnInstall()
{
	if ( m_iCompantPos == NULL || m_iProductPos == NULL )
		return 0;

	const COMPANY_INFO& ci = m_CompanyList.GetNext(m_iCompantPos);
	const PRODUCT_INFO& pi = ci.ProductList.GetNext(m_iProductPos);

	if ( !pi.strUninstallCMD.IsEmpty() )
	{
		CUninstall uninstller;
		return uninstller.ProcessUninstall(pi.strUninstallCMD);
	}

	return 1;	
}


LPCWSTR STDMETHODCALLTYPE CAVProductMgr::GetInstallAVs()
{
	m_strAVNames.Empty();

	POSITION iPos = m_CompanyList.GetHeadPosition();
	while ( iPos != NULL )
	{
		const COMPANY_INFO& ci = m_CompanyList.GetNext(iPos);
		if ( ci.strExpression.IsEmpty() || EvaluateCondition(ci.strExpression) )
		{
			ATLTRACE(_T("EvaluateCondition: Company.strExpression=%s"), ci.strExpression);

			POSITION jPos = ci.ProductList.GetHeadPosition();
			while( jPos != NULL )
			{
				const PRODUCT_INFO& pi = ci.ProductList.GetNext(jPos);
				CString strExpression(pi.strExpression);

				ProcessGUID(pi.strGUIDName, strExpression);
				if ( EvaluateCondition(strExpression) )
				{
					if ( m_strAVNames.Find(ci.strName) == -1 )
					{
						m_strAVNames.AppendFormat(_T("%s;"), pi.strName);
					}
				}
			}
		}
	}

	// 用类似微软安全中心的方法检测
	if (m_strAVNames.GetLength() == 0)
	{
		ANTIVIRUSPRODUCTINFO AvInfo;
		BOOL bResult = GetAntivirusProductInfo(AvInfo);
		if ( bResult && (!AvInfo.strProductName.empty()))
		{
			m_strAVNames = AvInfo.strProductName.c_str();
		}
	}

	return (m_strAVNames.GetLength() == 0) ? NULL : (LPCWSTR)m_strAVNames;
}

UINT32 STDMETHODCALLTYPE CAVProductMgr::LoadAVLib( LPCWSTR lpszPath)
{
	BkDatLibContent2	ct2;

	SetLocale2GBK();

	if(!CDataFileLoader().GetLibDatContent2(lpszPath, ct2))
		return FALSE;

	if(m_libParser.ParseXML(ct2.strXML))
		m_libParser.GetCompanyInfo(m_CompanyList);

	return LoadSoftGuid();
}
BOOL CAVProductMgr::LoadSoftGuid()
{
	//部分GUID会枚举不到
	for(int i = 0; ; i++)
	{
		INSTALLED_SOFT_ITEM soft_item;

		DWORD dwRet = MsiEnumProducts(i, soft_item.strGUID.GetBuffer(MAX_GUID_CHARS + 1));
		soft_item.strGUID.ReleaseBuffer();
		if( dwRet != ERROR_SUCCESS )
			break;

		DWORD dwLen = MAX_PATH;
		dwRet = MsiGetProductInfo(soft_item.strGUID ,
			INSTALLPROPERTY_INSTALLEDPRODUCTNAME ,
			soft_item.strName.GetBuffer(MAX_PATH + 1),
			&dwLen  
			);
		soft_item.strName.ReleaseBuffer();

		if( dwRet == ERROR_SUCCESS )
		{
			m_GuidList.AddTail(soft_item);
		}

	}

	//直接通过注册表枚举
	CEnumKey enumProduct(HKEY_LOCAL_MACHINE, PRODUCT_GUID_PATH);
	WCHAR szFullPath[MAX_PATH*2] = {0};
	WCHAR szValue[MAX_PATH] = {0};
	CRegFun regFunc;
	for(int i = 0; enumProduct.Enum(i); i++)
	{
		INSTALLED_SOFT_ITEM soft_item;

		StringCbPrintf(szFullPath, sizeof(szFullPath), L"%s\\%s\\InstallProperties", enumProduct.m_szPath, enumProduct.m_szSubKey);

		BOOL bOK = regFunc.GetValue(enumProduct.m_hRootKey, szFullPath, L"DisplayName", REG_SZ, soft_item.strName.GetBuffer(MAX_PATH + 1), MAX_PATH * sizeof (TCHAR));
		soft_item.strName.ReleaseBuffer();

		if( bOK )
		{		
			if(regFunc.GetValue(enumProduct.m_hRootKey, szFullPath, L"UninstallString", REG_SZ, szValue, sizeof(szValue)))
			{
				WCHAR* pos1 = NULL;
				WCHAR* pos2 = NULL;
				pos1 = StrChr(szValue, L'{');
				pos2 = StrChr(szValue, L'}');
				if(pos1 && pos2)
				{
					DWORD dwLen = (DWORD)(pos2-pos1+1) * sizeof(WCHAR);

					soft_item.strGUID = CString(pos1, dwLen);
					if( soft_item.strGUID.GetLength() != 0 )
						m_GuidList.AddTail(soft_item);
				}
			}
		}
	}

	POSITION iPos = m_GuidList.GetHeadPosition();
	while ( iPos != NULL )
	{
		const INSTALLED_SOFT_ITEM& si = m_GuidList.GetNext(iPos);

		ATLTRACE(L"%s, %s\n", si.strName, si.strGUID);
	}

	return TRUE;
}

UINT32 STDMETHODCALLTYPE CAVProductMgr::IsCanUninstall()
{
	if ( m_iCompantPos == NULL || m_iProductPos == NULL )
	{
		return FALSE;
	}
	
	const COMPANY_INFO& ci = m_CompanyList.GetNext(m_iCompantPos);
	const PRODUCT_INFO& pi = ci.ProductList.GetNext(m_iProductPos);

	return !pi.strUninstallCMD.IsEmpty();
}

void CAVProductMgr::ProcessGUID(const CString &strGuids, CString &strExpression)
{
	if (strExpression.Find(L"%guid%") != -1) 
	{
		CAtlArray<CString> arrGuids;
		SplitSent(strGuids ,arrGuids, L";");

		for (int i=0; i<arrGuids.GetCount(); i++)
		{
			POSITION jPos = m_GuidList.GetHeadPosition();
			while ( jPos != NULL )
			{
				const INSTALLED_SOFT_ITEM& isi = m_GuidList.GetNext(jPos);

				if (arrGuids[i].CompareNoCase(isi.strName) == 0)
				{
					while(strExpression.Find(L"%guid%") != -1)
						strExpression.Replace(L"%guid%", isi.strGUID);
				}
			}
		}
	}

}

BOOL CUninstall::regGetPath( CString strKey , CString strSubKey , CString strRegValue , WCHAR* wszValue , DWORD cbSize )
{
	BOOL bRet = FALSE;
	do 
	{
		if( strKey.IsEmpty() || strSubKey.IsEmpty() || strRegValue.IsEmpty() )
		{
			break;
		}
		HKEY hKey = NULL;
		if( !GetKeyFromString( strKey , hKey ) )
		{
			break;
		}
		DWORD dwType = REG_SZ;
		DWORD dwSize;

		if( ERROR_SUCCESS != SHGetValue( hKey , strSubKey , strRegValue ,  &dwType , NULL , &dwSize ) )
		{
			//			DWORD dwError = GetLastError();
			break;
		}
		if( dwType != 4 && dwType != 5 && dwType != 11 )
		{//如果注册表键值类型是字符类型的在这里读取
			WCHAR* wszBuf = new WCHAR[dwSize+10];
			memset( wszBuf , 0 , dwSize+10 );
			if( ERROR_SUCCESS != SHGetValue(hKey , strSubKey , strRegValue , &dwType , wszBuf , &dwSize) )
			{
				delete []wszBuf;
				wszBuf = NULL;
				break;
			}
			StringCbCopy(wszValue, cbSize, wszBuf);
			delete []wszBuf;
			wszBuf = NULL;		
			bRet = TRUE;
		}
		else
		{//注册表键值类型为数值类型的
			if( dwType == 11 )
			{//64位值在这个读取
				__int64 int64 = 0;
				dwSize = sizeof(__int64);
				if( ERROR_SUCCESS != SHGetValue( hKey , strSubKey , strRegValue , &dwType , &int64 , &dwSize))
				{
					break;
				}
				StringCbPrintf( wszValue , cbSize , L"%Iu64" , int64 );
			}
			else
			{//32位值在这里读取
				DWORD dw32 = 0;
				dwSize = sizeof(DWORD);
				if( ERROR_SUCCESS != SHGetValue( hKey , strSubKey , strRegValue , &dwType , &dw32 , &dwSize))
				{
					break;
				}
				StringCbPrintf( wszValue , cbSize , L"%d" , dw32 );
			}
		}
		bRet = TRUE;
	} while (0);
	return bRet;
}

BOOL CUninstall::regGetPath( CString strKey , CString strSubKey , CString strRegValue , CString& strPath )
{
	BOOL bRet = FALSE;
	strPath = L"";
	WCHAR wszBuf[MAX_PATH] = {0};
	if( regGetPath( strKey , strSubKey , strRegValue , wszBuf , sizeof(wszBuf) ) )
	{
		strPath = CString(wszBuf);
		bRet = TRUE;
	}

	return bRet;
}

BOOL CUninstall::GetKeyFromString( LPCWSTR lpKey , HKEY& hKey )
{
	if( NULL == lpKey )
		return FALSE ;

	BOOL bRet = TRUE ;
	if( wcsicmp( lpKey , L"HKLM" ) == 0 )
		hKey = HKEY_LOCAL_MACHINE ;
	else if( wcsicmp( lpKey , L"HKCU" ) == 0 )
		hKey = HKEY_CURRENT_USER ;
	else if( wcsicmp( lpKey , L"HKCR" ) == 0 )
		hKey = HKEY_CLASSES_ROOT ;
	else if( wcsicmp( lpKey , L"HKU" ) == 0 )
		hKey = HKEY_USERS ;
	else if( wcsicmp( lpKey , L"HKCC" ) == 0 )
		hKey = HKEY_CURRENT_CONFIG ;
	else
	{
		bRet = FALSE ;
		hKey = NULL ;
	}

	return bRet ;
}

BOOL CUninstall::regUnInstallPath( CString param1 , CString param2 , CString param3 , WCHAR* wszUnInstallcmd , DWORD cbSize )
{
	BOOL bRet = FALSE;
	CString str;
	if( regGetPath( param1 , param2 , param3 , wszUnInstallcmd , cbSize ) )
	{
		bRet = TRUE;
	}
	return bRet;
}

BOOL CUninstall::regCMDUnInstallPath( CString param1 , CString param2 , CString param3 , CString param4 , CString param5 , WCHAR* wszUnInstallcmd , DWORD cbSize )
{
	BOOL bRet = FALSE;
	do 
	{
		if( param4.IsEmpty() || param5.IsEmpty() )
			break;
		WCHAR bstr[MAX_PATH] = {0} ;
		CString strPath ;
		if( !regGetPath( param1 , param2 , param3 , strPath ) )
		{//获取卸载文件的路径
			break;
		}
		TCHAR pszPath[MAX_PATH] = {0};
		StringCbPrintf(pszPath , sizeof(pszPath) , L"%s" , strPath );
		PathAppend(pszPath , param4 );
		if( !IsFileExist( pszPath ) )
		{//判断卸载文件是否存在
			break;
		}

		StringCbPrintf(wszUnInstallcmd, cbSize, L"%s %s", pszPath, param5);

		ATLTRACE(wszUnInstallcmd);

		bRet = TRUE;
	} while (0);

	return bRet;
}

BOOL CUninstall::ProcessUninstall( CString strExpression )
{
	int nPos = strExpression.Find(_T("("));

	CString strFuncName = strExpression.Mid(0, nPos);
	CString strParams	= strExpression.Mid(nPos);
	strParams.Trim(_T("("));strParams.Trim(_T(")"));

	CAtlArray<CString> arrParams;

	SplitSent(strParams, arrParams, _T(","), _T("\""));

	CString strUninstallCMD;
	BOOL bRet = FALSE;
	if (strFuncName.Compare(_T("ru")) == 0)
	{
		if(arrParams.GetCount() == 3)
			bRet = regUnInstallPath(arrParams[0], arrParams[1], arrParams[2], strUninstallCMD.GetBufferSetLength(MAX_PATH), MAX_PATH);
		else if(arrParams.GetCount() == 4)
			bRet = regUnInstallPathEx(arrParams[0], arrParams[1], arrParams[2], arrParams[3], strUninstallCMD.GetBufferSetLength(MAX_PATH), MAX_PATH);
	}
	else if (strFuncName.Compare(_T("rcu")) == 0)
	{
		if(arrParams.GetCount() == 5)
			regCMDUnInstallPath( arrParams[0], arrParams[1], arrParams[2], arrParams[3], arrParams[4], strUninstallCMD.GetBufferSetLength(MAX_PATH), MAX_PATH);
	}

	if (!bRet || strUninstallCMD.IsEmpty() )
		return FALSE;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	if (CreateProcess(NULL, (LPTSTR)(LPCTSTR)strUninstallCMD,	NULL, NULL,	TRUE,NULL,
		NULL,NULL,&si, 	&pi) )
		WaitForSingleObject(pi.hProcess, INFINITE);
	else
		return FALSE;

	return TRUE;
}

BOOL CUninstall::regUnInstallPathEx( CString param1 , CString param2 , CString param3 , CString param4, WCHAR* wszUnInstallcmd , DWORD cbSize )
{
	BOOL bRet = FALSE;
	CAtlArray<CString> arrSubPath;
	SplitSent(param3, arrSubPath, L";");
	TCHAR pszFullPath[MAX_PATH*2] = {0};
	for(int i = 0; i < arrSubPath.GetCount(); i++)
	{
		StringCbPrintf(pszFullPath, sizeof(pszFullPath), L"%s\\%s", param2, arrSubPath[i]);
		if( regGetPath( param1 , pszFullPath , param4 , wszUnInstallcmd , cbSize ) )
		{
			bRet = TRUE;
			break;
		}
	}
	return bRet;

}
