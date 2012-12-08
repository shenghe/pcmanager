/************************************************************************
* @file      : kregfunction.h
* @author    : Zhaoxinxing <zhaoxinxing@kingsoft.com>
* @date      : 2010/3/17 15:57:42
* @brief     : 
*
* $Id: $
/************************************************************************/
#ifndef __KREGFUNCTION_H__
#define __KREGFUNCTION_H__

#define HKEY_CLASSES_ROOT_STRA		L"HKEY_CLASSES_ROOT"
#define HKEY_CURRENT_USER_STRA		L"HKEY_CURRENT_USER"
#define HKEY_LOCAL_MACHINE_STRA		L"HKEY_LOCAL_MACHINE"
#define HKEY_USERS_STRA				L"HKEY_USERS"
#define HKEY_CURRENT_CONFIG_STRA	L"HKEY_CURRENT_CONFIG"
//just for 9x
#define HKEY_DYN_DATA_STRA			L"HKEY_DYN_DATA"


#define HKEY_CLASSES_ROOT_PATH_STRA		L"HKEY_CLASSES_ROOT\\"
#define HKEY_CURRENT_USER_PATH_STRA		L"HKEY_CURRENT_USER\\"
#define HKEY_LOCAL_MACHINE_PATH_STRA	L"HKEY_LOCAL_MACHINE\\"
#define HKEY_USERS_PATH_STRA			L"HKEY_USERS\\"
#define HKEY_CURRENT_CONFIG_PATH_STRA	L"HKEY_CURRENT_CONFIG\\"
#define HKEY_DYN_DATA_PATH_STRA			L"HKEY_DYN_DATA\\"

#define HKEY_CLASSES_ROOT_PATH_LEN		((sizeof(HKEY_CLASSES_ROOT_PATH_STRA)) - 1)
#define HKEY_CURRENT_USER_PATH_LEN		((sizeof(HKEY_CURRENT_USER_PATH_STRA)) - 1)
#define HKEY_LOCAL_MACHINE_PATH_LEN		((sizeof(HKEY_LOCAL_MACHINE_PATH_STRA)) - 1)
#define HKEY_USERS_PATH_LEN				((sizeof(HKEY_USERS_PATH_STRA)) - 1)
#define HKEY_CURRENT_CONFIG_PATH_LEN	((sizeof(HKEY_CURRENT_CONFIG_PATH_STRA)) - 1)
#define HKEY_DYN_DATA_PATH_LEN			((sizeof(HKEY_DYN_DATA_PATH_STRA)) - 1)


#define HKEY_CLASSES_ROOT_STRW		L"HKEY_CLASSES_ROOT"
#define HKEY_CURRENT_USER_STRW		L"HKEY_CURRENT_USER"
#define HKEY_LOCAL_MACHINE_STRW		L"HKEY_LOCAL_MACHINE"
#define HKEY_USERS_STRW				L"HKEY_USERS"
#define HKEY_CURRENT_CONFIG_STRW	L"HKEY_CURRENT_CONFIG"
//just for 9x
#define HKEY_DYN_DATA_STRW			L"HKEY_DYN_DATA"


#define HKEY_CLASSES_ROOT_PATH_STRW		L"HKEY_CLASSES_ROOT\\"
#define HKEY_CURRENT_USER_PATH_STRW		L"HKEY_CURRENT_USER\\"
#define HKEY_LOCAL_MACHINE_PATH_STRW	L"HKEY_LOCAL_MACHINE\\"
#define HKEY_USERS_PATH_STRW			L"HKEY_USERS\\"
#define HKEY_CURRENT_CONFIG_PATH_STRW	L"HKEY_CURRENT_CONFIG\\"
#define HKEY_DYN_DATA_PATH_STRW			L"HKEY_DYN_DATA\\"


#define HKEY_CLASSES_ROOT_SHORT						L"HKCR"
#define HKEY_CURRENT_USER_LOCAL_MACHINE_SHORT		L"LMCU"
#define HKEY_LOCAL_MACHINE_SHORT					L"HKLM"
#define HKEY_CURRENT_USER_SHORT						L"HKCU"

#define VALUE_SPACE_L L"  ["
#define VALUE_SPACE_R L"] : "

#define H_CLSID L"CLSID"
#define H_CLSID_STRW L"CLSID\\"
#define H_RUN L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define H_RUN_STRW L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\"
#define H_SERVICE L"SYSTEM\\CurrentControlSet\\Services"
#define H_SERVICE_STRW L"SYSTEM\\CurrentControlSet\\Services\\"
#define H_MICROSOFT L"Software\\Microsoft"
#define H_MICROSOFT_STRW L"Software\\Microsoft\\"
#define H_MICROSOFT_WIN L"Software\\Microsoft\\Windows"
#define H_MICROSOFT_WIN_STRW L"Software\\Microsoft\\Windows\\"
#define H_MICROSOFT_WINNT L"Software\\Microsoft\\Windows NT"
#define H_MICROSOFT_WINNT_STRW L"Software\\Microsoft\\Windows NT\\"
#include <list>
#include <set>
#include <shlwapi.h>
// -------------------------------------------------------------------------

LONG RegOpenKeyExPrvg(HKEY hKey, 
					  LPCWSTR lpSubKey, 
					  DWORD ulOptions, 
					  REGSAM samDesired, 
					  PHKEY phkResult);

int ExtractRootKeyTypeW(const wchar_t* pszRegPath, 
						HKEY& hRootKey, 
						const wchar_t** ppSubKey);

int ExtractRootKeyTypeByShortValue(const wchar_t* pszRegPath, 
								   HKEY& hRootKey, 
								   HKEY& hRootKeyAother);

const wchar_t* GetRegRootKeyStrW(HKEY hRootKey);

int CheckRegKeyExists(HKEY hRootKey, 
					  const TCHAR* pszSubKey, 
					  bool& bIsExist);

int CheckRegValueExist(HKEY hRootKey, 
					   const TCHAR* pszSubKey, 
					   const TCHAR* pszRegPath, 
					   const TCHAR* pszValueName, 
					   bool& bIsExist);

int CheckRegValueMatch(HKEY hRootKey, 
					   const TCHAR* pszSubKey, 
					   int& nMatchCnt,
					   wchar_t* szMatchValues,
					   DWORD* puValueNamesSize);

int CheckRegValueDataMatch(HKEY hRootKey, 
						   const TCHAR* pszSubKey, 
						   int& nMatchCnt,
						   wchar_t* szMatchValues,
						   DWORD* puValueNamesSize);


HRESULT GetRegistryValue(HKEY hRootKey, 
						 const wchar_t* pszSubKeyName,
						 const wchar_t* pszValueName, 
						 DWORD* pdwType, 
						 BYTE* pszValue, 
						 DWORD* puValueSize);

HRESULT RemoveRegistryKey(HKEY hRootKey, 
						  const WCHAR* pszSubKeyName);

HRESULT RemoveRegistryValue(HKEY hRootKey, 
							const TCHAR* pszSubKeyName, 
							const TCHAR* pszValueName);


int EnumRegKeyValue(HKEY hRootKey, 
					const TCHAR* pszSubKey,
					int& nMatchCnt, 
					wchar_t* pstrValueName,
					DWORD* puValueNamesSize);

HRESULT SetRegistryValue(HKEY hRootKey, 
						 const wchar_t* pszSubKeyName,  
						 const wchar_t* pszValueName, 
						 DWORD dwType, 
						 const BYTE* pValue, 
						 DWORD uValueSize);



int EnumCheckRegValueExist(HKEY hRootKey, 
						   const TCHAR* pszSubKey, 
						   const TCHAR* pszValueName, 
						   bool& bIsExist);

int EnumCheckRegValueDataExist(HKEY hRootKey, 
							   const TCHAR* pszSubKey, 
							   const TCHAR* pszValueName, 
							   const TCHAR* pszValueData,
							   DWORD pszValueDataSize,
							   bool& bIsExist);

int EnumRegKeyValueDATA(HKEY hRootKey, 
						const TCHAR* pszSubKey,
						int& nMatchCnt, 
						wchar_t* pstrValueName,
						DWORD* puValueNamesSize);

int EnumCheckRegValueData(HKEY hRootKey, 
						  const TCHAR* pszSubKey, 
						  const TCHAR* pszValueName, 
						  const TCHAR* pszValueData,
						  DWORD pszValueDataSize,
						  std::list<std::wstring> &listsubkey);

int EnumCheckRegValue(HKEY hRootKey, 
					  const TCHAR* pszSubKey, 
					  const TCHAR* pszValueName, 
					  std::list<std::wstring> &listsubkey);

int EnumGetRegKeyAndValueAndData(HKEY hRootKey, 
								 const TCHAR* pszSubKey,
								 std::set<std::wstring> &listdata,
								 std::set<std::wstring> &listfiledata);
int EnumGetRegValue(HKEY hRootKey, 
					const TCHAR* pszSubKey, 
					std::set<std::wstring> &listvaluedata,
					std::set<std::wstring> &listfiledata);

int EnumRemoveRegKeyValueName(HKEY hRootKey, 
							  const TCHAR* pszSubKey);

int EnumRegKeyValueName(HKEY hRootKey, 
						const TCHAR* pszSubKey,
						bool exits);



// -------------------------------------------------------------------------
// $Log: $

#endif /* __KREGFUNCTION_H__ */
