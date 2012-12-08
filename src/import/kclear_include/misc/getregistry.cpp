//////////////////////////////////////////////////////////////////////////
// FileName:	GetRegistry.cpp
// Author:		Jonathan Xiang
// DateTime:	April 18th, 2008
// Description:
//////////////////////////////////////////////////////////////////////////

#include "GetRegistry.h"



//////////////////////////////////////////////////////////////////////////
// 获取注册表键值的类型
// Return value:
//	true	: succeeded
//	false	: failed
//////////////////////////////////////////////////////////////////////////

bool GetRegValueType(HKEY hRootKey, const TCHAR* lpszKeyName, const TCHAR* lpszValueName, LPDWORD lpdwType)
{
	bool		bReturn = false;
	HKEY		hKey = NULL;
	LONG		lResult = ERROR_SUCCESS;

	if ( lpdwType == static_cast<DWORD *>(0) )
	{
		bReturn = false;
		goto Label_Ret;
	}

	// 打开子键
	hKey = NULL;
	lResult = ::RegOpenKeyEx(
		hRootKey, 
		lpszKeyName, 
		0, 
		KEY_QUERY_VALUE | KEY_READ,
		&hKey);
	if (lResult != ERROR_SUCCESS)
	{
		bReturn = false;
		goto Label_Ret;
	}

	// 查询键值类型
	*lpdwType = REG_SZ;
	lResult = ::RegQueryValueEx(
		hKey,
		lpszValueName,
		NULL,
		lpdwType,
		NULL,
		NULL);
	if (lResult != ERROR_SUCCESS)
	{
		bReturn = false;
		goto Label_Ret;
	}

	bReturn = true;

Label_Ret:
	if (hKey != NULL)
		::RegCloseKey(hKey);

	return bReturn;

}


//////////////////////////////////////////////////////////////////////////
// 获取注册表键值数据的长度。 
// 对于REG_SZ, REG_EXPAND_SZ类型，此长度包括结尾的'\0'; 对于REG_MULTI_SZ, 此长度包括所有的'\0'    
// Return value:
//	true	: succeeded
//	false	: failed
//////////////////////////////////////////////////////////////////////////

bool GetRegValueLength(HKEY hRootKey, const TCHAR* lpszKeyName, const TCHAR* lpszValueName, LPDWORD lpcbData)
{
	bool		bReturn = false;
	HKEY		hKey = NULL;
	LONG		lResult = ERROR_SUCCESS;
	
	// 检查传入的指针是否有效
	if ( lpcbData == static_cast<DWORD *>(0) )
	{
		bReturn = false;
		goto Label_Ret;
	}

	// 打开子键
	hKey = NULL;
	lResult = ::RegOpenKeyEx(
		hRootKey, 
		lpszKeyName, 
		0, 
		KEY_QUERY_VALUE | KEY_READ,
		&hKey);
	if (lResult != ERROR_SUCCESS)
	{
		bReturn = false;
		goto Label_Ret;
	}

	// 查询键值数据长度。对于REG_SZ, REG_EXPAND_SZ类型，此长度包括结尾的'\0'; 对于REG_MULTI_SZ, 此长度包括所有的'\0'
	*lpcbData = 0;
	lResult = ::RegQueryValueEx(
		hKey,
		lpszValueName,
		NULL,
		NULL,
		NULL,
		lpcbData);
	if (lResult != ERROR_SUCCESS)
	{
		bReturn = false;
		goto Label_Ret;
	}

	bReturn = true;

Label_Ret:
	if (hKey != NULL)
		::RegCloseKey(hKey);

	return bReturn;

}


//////////////////////////////////////////////////////////////////////////
// 获取注册表键值的数据
// Return value:
//	true	: succeeded
//	false	: failed
//////////////////////////////////////////////////////////////////////////

bool GetRegValueData(HKEY hRootKey, const TCHAR* lpszKeyName, const TCHAR* lpszValueName, LPBYTE lpData, DWORD cbData)
{
	bool		bReturn = false;
	HKEY		hKey = NULL;
	LONG		lResult = ERROR_SUCCESS;
	DWORD		dwType  = REG_SZ;

	if ( lpData == static_cast<BYTE *>(0) )
	{
		bReturn = false;
		goto Label_Ret;
	}

	// 打开子键
	hKey = NULL;
	lResult = ::RegOpenKeyEx(
		hRootKey, 
		lpszKeyName, 
		0, 
		KEY_QUERY_VALUE | KEY_READ,
		&hKey);
	if (lResult != ERROR_SUCCESS)
	{
		bReturn = false;
		goto Label_Ret;
	}

	// 查询键值
	::memset(lpData, 0, cbData);
	lResult = ::RegQueryValueEx(
		hKey,
		lpszValueName,
		NULL,
		&dwType,
		(LPBYTE)lpData,
		&cbData);
	if (lResult != ERROR_SUCCESS)
	{
		bReturn = false;
		goto Label_Ret;
	}

	bReturn = true;

Label_Ret:
	if (hKey != NULL)
		::RegCloseKey(hKey);

	return bReturn;
}


//////////////////////////////////////////////////////////////////////////
// 获取注册表键值所有的属性
// Return value:
//	true	: succeeded
//	false	: failed
//////////////////////////////////////////////////////////////////////////

bool GetRegValueAll(HKEY hRootKey, 
                  const TCHAR* lpszKeyName, 
                  const TCHAR* lpszValueName, 
                  LPDWORD lpdwType, 
                  LPBYTE lpData, 
                  LPDWORD lpcbData,
                  Wow64Type wow64type)
{
	bool		bReturn = false;
	HKEY		hKey = NULL;
	LONG		lResult = ERROR_SUCCESS;


	if ( lpData != static_cast<LPBYTE>(0) && lpcbData == static_cast<LPDWORD>(0) )
	{
		bReturn = true;
		goto Label_Ret;
	}

	// 打开子键
	hKey = NULL;
	lResult = ::RegOpenKeyEx(
		hRootKey, 
		lpszKeyName, 
		0, 
		KEY_QUERY_VALUE | KEY_READ | GetRegAccess(wow64type),
		&hKey);
	if (lResult != ERROR_SUCCESS)
	{
		bReturn = false;
		goto Label_Ret;
	}

	// 查询键值
	if ( lpData != static_cast<LPBYTE>(0) )
		::memset(lpData, 0, *lpcbData);

	if ( lpdwType != static_cast<LPDWORD>(0) )
		*lpdwType = REG_SZ;

	lResult = ::RegQueryValueEx(
		hKey,
		lpszValueName,
		NULL,
		lpdwType,
		(LPBYTE)lpData,
		lpcbData);
	if (lResult != ERROR_SUCCESS)
	{
		bReturn = false;
		goto Label_Ret;
	}

	bReturn = true;

Label_Ret:
	if (hKey != NULL)
		::RegCloseKey(hKey);

	return bReturn;
}
