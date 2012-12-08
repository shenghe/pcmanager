#pragma once

#include <atlcoll.h>
#include "strsafe.h"
#include "avmgr\avmgrinterface.h"
#include "tinyxml\tinyxml.h"

#pragma comment(lib, "strsafe.lib")
#pragma warning(disable:4995)


class CEnumKey  
{
public:
	CEnumKey( HKEY hKey , LPCTSTR lpPath )
	{
		m_hKey = NULL  ;

		StringCbCopy( m_szPath , sizeof(m_szPath) , lpPath ) ;
		m_hRootKey = hKey ;
		RegOpenKey( m_hRootKey , m_szPath ,  &m_hKey ) ;
	}

	virtual ~CEnumKey()
	{
		if( m_hKey )
			RegCloseKey( m_hKey ) ;
	}

	BOOL Enum( DWORD dwIndex )
	{
		if( m_hKey == NULL )
			return FALSE ;

		m_dwKsize = MAX_PATH ;
		m_szSubKey[0] = 0;

		if (ERROR_SUCCESS == RegEnumKey( m_hKey , dwIndex , m_szSubKey , m_dwKsize))
		{
			return TRUE;
		}

		return FALSE;
	}

	TCHAR		m_szSubKey[MAX_PATH] ;
	DWORD		m_dwKsize ;
	HKEY		m_hKey ;
	TCHAR		m_szPath[MAX_PATH] ; 
	HKEY		m_hRootKey ;
};


class CRegFun  
{
public:
	BOOL GetValue( HKEY hKey , LPCTSTR lpPath , LPCTSTR lpName , DWORD type, void* data)
	{
		DWORD dwSize = sizeof(DWORD) ;
		if( REG_SZ == type )
			dwSize = MAX_PATH ;

		if( ERROR_SUCCESS != SHGetValue( hKey , lpPath , lpName , &type , data , &dwSize ) )
			return FALSE ;

		return(TRUE);
	}

	BOOL GetValue( HKEY hKey , LPCTSTR lpPath , LPCTSTR lpName , DWORD type, void* data, DWORD len)
	{
		if (ERROR_SUCCESS != SHGetValue( hKey , lpPath , lpName , &type, data, &len ) )
			return FALSE ;

		return TRUE ;
	}

	BOOL SetValue( HKEY hKey , LPCTSTR lpPath , LPCTSTR lpName , DWORD type , void* data ) 
	{
		DWORD dwSize  ;
		dwSize = sizeof(DWORD) ;
		if( REG_SZ == type )
			dwSize = (DWORD)_tcslen((TCHAR*) data ) ;
		if( ERROR_SUCCESS == SHSetValue( hKey , lpPath , lpName , type , data , dwSize ))
			return TRUE ;

		return FALSE ;
	}

	BOOL SetValue( HKEY hKey , LPCTSTR lpPath , LPCTSTR lpName , DWORD type, void* data, DWORD len)
	{

		if (ERROR_SUCCESS != SHSetValue( hKey, lpPath , lpName , type, data, len))
			return(FALSE);

		return(TRUE);
	}


	BOOL DeleteValue(  HKEY hKey , LPCTSTR lpPath , LPCTSTR lpName )
	{
		if( NULL == hKey || lpPath[0] == 0 )
			return FALSE ;

		if( ERROR_SUCCESS == SHDeleteValue( hKey , lpPath , lpName ) )
			return TRUE ;

		return FALSE ;
	}

	BOOL DeleteKey( HKEY hKey  , LPCTSTR lpPath ) 
	{
		if( NULL == hKey || lpPath[0] == 0 )
			return FALSE;

		if( ERROR_SUCCESS == SHDeleteKey( hKey , lpPath ) )
			return TRUE ;

		return FALSE ;
	}

	BOOL KeyExist( HKEY hKey , LPCTSTR lpPath ) 
	{
		BOOL bRet = FALSE ;
		HKEY hTestKey = NULL ;

		if (ERROR_SUCCESS == RegOpenKeyEx( hKey , lpPath , 0 , KEY_READ, &hTestKey ))
		{
			if( hTestKey )
				RegCloseKey( hTestKey ) ;

			bRet = TRUE ;
		}
		return bRet ;
	}

	BOOL CreateKey(HKEY hKey, LPCTSTR lpSubPath, LPCTSTR lpNewKey,HKEY& hRetKey)
	{
		HKEY hRootKey = NULL;
		DWORD dwRet = 0;
		if(RegOpenKeyEx(hKey, lpSubPath, 0, KEY_READ|KEY_WRITE, &hRootKey) == ERROR_SUCCESS && hRootKey)
		{
			if(RegCreateKeyEx(hRootKey, lpNewKey, 0, REG_NONE, REG_OPTION_NON_VOLATILE,\
				KEY_WRITE|KEY_READ, NULL,&hRetKey, &dwRet) == ERROR_SUCCESS)
			{
				RegCloseKey(hRootKey);
				RegCloseKey(hRetKey);
				return TRUE;
			}
		}

		return FALSE;
	}
};

typedef struct _PRODUCT_INFO
{
	CString strName;
	CString strGUIDName;
	CString strLanguage;
	CString strExpression;
	CString strUninstallCMD;
} PRODUCT_INFO,*PPRODUCT_INFO;

typedef struct _COMPANY_INFO
{
	CString strName;
	CString strURL;
	CString strExpression;
	CAtlList<PRODUCT_INFO> ProductList;

	_COMPANY_INFO() {

	}

	_COMPANY_INFO(const _COMPANY_INFO& ci)
	{
		strName = ci.strName;
		strURL = ci.strURL;
		strExpression = ci.strExpression;

		ProductList.RemoveAll();
		ProductList.AddTailList(&ci.ProductList);
	}
}COMPANY_INFO,*PCOMPANY_INFO;

typedef struct _INSTALLED_SOFT_ITEM
{
	CString strName;
	CString strGUID;
}INSTALLED_SOFT_ITEM, *PINSTALLED_SOFT_ITEM;

#define GETCHILDTEXT(pItem, strAChildName, strRet)\
{\
	TiXmlElement *pChild = pItem->FirstChildElement(strAChildName);\
	if (pChild != NULL)\
{\
	USES_CONVERSION;\
	strRet = A2W(pChild->GetText());\
}\
}

class CAVLibParse
{
public:
	CAVLibParse() {
		m_pXMLDocument = NULL;
	}

	virtual ~CAVLibParse() {
		Close();
	}

	BOOL ParseXML(LPCWSTR lpszXML)
	{
		USES_CONVERSION;

		m_pXMLDocument = new TiXmlDocument();
		m_pXMLDocument->Parse(W2A(lpszXML), NULL, TIXML_DEFAULT_ENCODING);

		return m_pXMLDocument->Error() == 0;
	}

	BOOL GetCompanyInfo(CAtlList<COMPANY_INFO> &arrCompany)
	{
		TiXmlElement *pRootElement = m_pXMLDocument->RootElement();
		if (pRootElement == NULL)
			return FALSE;

		USES_CONVERSION;
		for (TiXmlElement *pCompanyElement = pRootElement->FirstChildElement();
			pCompanyElement != NULL;
			pCompanyElement = pCompanyElement->NextSiblingElement())
		{
 			COMPANY_INFO ci;

			ci.strName = A2W(pCompanyElement->Attribute("name"));
			ci.strURL = A2W(pCompanyElement->Attribute("url"));
			GETCHILDTEXT(pCompanyElement, "expression", ci.strExpression);

			GetProductInfo(pCompanyElement->FirstChildElement("products"), ci.ProductList );

			arrCompany.AddTail(ci);
		}

		return TRUE;
	}

	BOOL GetProductInfo(TiXmlElement *pProductsElement, CAtlList<PRODUCT_INFO> &arrProduct)
	{
		if (pProductsElement == NULL)
			return FALSE;

		USES_CONVERSION;
		for (TiXmlElement *pProductElemet = pProductsElement->FirstChildElement();
			pProductElemet != NULL;
			pProductElemet = pProductElemet->NextSiblingElement())
		{
			PRODUCT_INFO pi;

			pi.strName = A2W(pProductElemet->Attribute("name"));
			pi.strGUIDName = A2W(pProductElemet->Attribute("guidname"));
			pi.strLanguage = A2W(pProductElemet->Attribute("language"));
			GETCHILDTEXT(pProductElemet, "expression", pi.strExpression);
			GETCHILDTEXT(pProductElemet, "uninstallcmd", pi.strUninstallCMD);

			arrProduct.AddTail(pi);
		}

		return TRUE;
	}

	void Close()
	{
		delete m_pXMLDocument;
		m_pXMLDocument = NULL;
	}

private:
	TiXmlDocument *m_pXMLDocument;
};


class CUninstall
{
private:
	BOOL GetKeyFromString( LPCWSTR lpKey , HKEY& hKey );

	BOOL regGetPath(CString strKey , CString strSubKey , CString strRegValue , WCHAR* wszValue , DWORD cbSize );

	BOOL regGetPath(CString strKey , CString strSubKey , CString strRegValue , CString& strPath );

	BOOL regUnInstallPath( CString param1 , CString param2 , CString param3 , WCHAR* wszUnInstallcmd , DWORD cbSize );

	BOOL regCMDUnInstallPath( CString param1 , CString param2 , CString param3 , CString param4 , CString param5 , WCHAR* wszUnInstallcmd , DWORD cbSize );

	BOOL regUnInstallPathEx( CString param1 , CString param2 , CString param3 , CString param4, WCHAR* wszUnInstallcmd , DWORD cbSize );

public:
	//获取卸载参数
	BOOL ProcessUninstall(CString strExpression);

};


static BOOL SplitSent( CString strSent, CAtlArray<CString> &arrParts , CString strSplit, CString strToTrim =_T(""))
{
	int nSpliteLength = strSplit.GetLength();
	int nSpec = strSent.Find(strSplit, 0);

	if (nSpec == -1)
	{
		arrParts.Add(strSent.Trim());

		return FALSE;
	}
	else
	{
		int nStart = 0; //分割符号前一个位置
		while (TRUE)
		{
			CString strItem = strSent.Mid(nStart, nSpec - nStart);
			if (!strItem.IsEmpty())
				arrParts.Add(strItem.Trim(strToTrim));

			nStart = nSpec + nSpliteLength;
			nSpec = strSent.Find(strSplit, nStart);

			if (nSpec == -1)
			{
				strItem = strSent.Mid(nStart);
				arrParts.Add(strItem.Trim(strToTrim));

				break;
			}
		}
	}

	return TRUE;
}

