#ifndef _GETINSTALLPATH_H_
#define _GETINSTALLPATH_H_
#include <string>
#include "../../kclear/src/kscgetpathfromuninst.h"
using namespace std;
class KSearchSoftwareStruct
{
public:

	LPCTSTR pszMainFileName;      // 该软件根目录下的任意一文件名


	int     nDefPathFolderCSIDL;  // 软件默认安全路径的CSIDL, 如: CSIDL_PROGRAMS为 C:\Program files, -1表示不使用该项数据
	LPCTSTR pszDefPath;           // 软件默认安全路径除去CSIDL部分.


	HKEY    hRegRootKey;          // 很多软件会在注册表中记录安装路径 根键
	LPCTSTR pszRegSubKey;         // 很多软件会在注册表中记录安装路径 子键
	LPCTSTR pszPathValue;         // 很多软件会在注册表中记录安装路径 记录路径的值
	BOOL    bFolder;              // TRUE 保存的是目录  FALSE 保存的是文件


	LPCTSTR pszUninstallListName; // 程序在卸载列表中的名称

	LPCTSTR pszFileDesc;          // 指定主程序的版本描述信息, 从MUICache中查找



public:
	KSearchSoftwareStruct()
	{
		pszMainFileName      = NULL;      
		nDefPathFolderCSIDL  = -1; 
		pszDefPath           = NULL;   
		hRegRootKey          = NULL;       
		pszRegSubKey         = NULL; 
		pszPathValue         = NULL;     
		bFolder              = TRUE;      
		pszUninstallListName = NULL; 
		pszFileDesc          = NULL; 
	}
};

inline LONG SimpleRegister_QueryStringValue( 
									 HKEY    hRootKey, 
									 LPCTSTR pszSubkey, 
									 LPCTSTR pszValue,
									 LPTSTR  pszBuffer, 
									 ULONG   ulBufSize 
									 )
{
	LONG lRet = ERROR_SUCCESS;

	CRegKey reg;
	lRet = reg.Open( hRootKey, pszSubkey, KEY_READ );
	if( lRet != ERROR_SUCCESS )
		goto clean0;


	lRet = reg.QueryStringValue( pszValue, pszBuffer, &ulBufSize );
	if( lRet != ERROR_SUCCESS )
		goto clean0;

clean0:
	reg.Close();
	return lRet;
}

inline BOOL SearchSoftwarePath(const KSearchSoftwareStruct* psss, wstring& strPath)
{
	if( psss == NULL || psss->pszMainFileName == NULL || psss->pszMainFileName[0] == 0 )
		return FALSE;


	BOOL  bRet = FALSE;
	TCHAR szFolderPath[MAX_PATH] = {0};


	//-------------------------------------
	// 在默认安装路径下查找
	if( psss->pszDefPath != NULL && psss->pszDefPath[0] != 0 )
	{
		if( psss->nDefPathFolderCSIDL != -1 )
		{
			if( ::SHGetSpecialFolderPath( NULL, szFolderPath, CSIDL_PROGRAM_FILES, FALSE ) )
				::PathAddBackslash( szFolderPath );
		}


		::PathAppend( szFolderPath, psss->pszDefPath );
		::PathAddBackslash( szFolderPath );

		strPath = szFolderPath;
		strPath += psss->pszMainFileName;

		if( ::PathFileExists( strPath.c_str()) )
			return TRUE;
	}



	//-------------------------------------
	// 如果注册表中保存了程序安装路径, 则读取注册表获取路径
	if( psss->hRegRootKey  != NULL && 
		psss->pszRegSubKey != NULL && 
		psss->pszPathValue != NULL &&
		psss->pszRegSubKey[0] != 0 
		)
	{
		LONG lRet = SimpleRegister_QueryStringValue( 
			psss->hRegRootKey, 
			psss->pszRegSubKey,
			psss->pszPathValue,
			szFolderPath,
			MAX_PATH
			);

		if( lRet == ERROR_SUCCESS )
		{
			if( !psss->bFolder )
				::PathRemoveFileSpec( szFolderPath );

			::PathAddBackslash( szFolderPath );
			strPath = szFolderPath;
			strPath += psss->pszMainFileName ;

			if( ::PathFileExists( strPath.c_str()) )
				return TRUE;
		}
	}


	////-------------------------------------
	//// 从卸载列表中查找
	if( psss->pszUninstallListName != NULL && psss->pszUninstallListName[0] != 0 )
	{
		KSearchSoftwarePathFromUninstallList searchpath;
		BOOL bRet = searchpath.SearchSoftwarePathFromUninstallList( 
			psss->pszUninstallListName, 
			psss->pszMainFileName, 
			szFolderPath,
			MAX_PATH
			);

		if( bRet )
		{
			strPath = szFolderPath;
			return TRUE;
		}
	}


	////--------------------------------------
	//// 从MUICache中查找
	//if( psss->pszFileDesc != NULL && psss->pszFileDesc[0] != 0 )
	//{
	//	KSearchSoftwarePathFromMUICache searchpath;
	//	BOOL bRet = searchpath.SearchSoftwarePathFromMUICache(
	//		psss->pszMainFileName,
	//		psss->pszFileDesc,
	//		szFolderPath,
	//		MAX_PATH
	//		);

	//	if( bRet )
	//	{
	//		strPath = szFolderPath;
	//		return TRUE;
	//	}
	//}


	////--------------------------------------
	//// 从App Paths中查找
	//{
	//	KSearchSoftwarePathFromAppPaths searchpath;
	//	BOOL bRet = searchpath.SearchSoftwarePathFromAppPaths(
	//		psss->pszMainFileName,
	//		szFolderPath,
	//		MAX_PATH
	//		);

	//	if( bRet )
	//	{
	//		strPath = szFolderPath;
	//		return TRUE;
	//	}
	//}



	strPath = TEXT( "" );
	return FALSE;
}

#endif