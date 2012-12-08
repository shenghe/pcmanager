/************************************************************************
* @file      : kregfunction.cpp
* @author    : ChenZhiQiang <chenzhiqiang@kingsoft.com> 
* @date      : 2010/3/22 17:46:00
* @brief     : modify by zhaoxinxing <zhaoxinxing@kingsoft.com>
*
* $Id: $
/************************************************************************/

#include "stdafx.h"

#include <atlrx.h>
#include "KAEAutoRunsReg.h"
#include "upreggradeprivilege.h"
#include "kregfunction.h"




#pragma warning(disable : 4311) 


// -------------------------------------------------------------------------

// 下面的四个带_prvg后缀的注册表函数会在第一次尝试打开失败时去掉acl的拒绝权限并设置相应的访问权限并再次尝试
LONG RegOpenKeyExPrvg(HKEY hKey, 
					  LPCWSTR lpSubKey,
					  DWORD ulOptions, 
					  REGSAM samDesired, 
					  PHKEY phkResult)
{
    LONG ret = 0;
    int i = 0;

    while(i < 2)
    {
        if ( ERROR_SUCCESS == (ret=::RegOpenKeyExW(hKey, 
												   lpSubKey,
												   ulOptions, 
								                   samDesired, 
								                   phkResult)))
        {
            break;
        }
        if ( i == 0 )
        {
            UpgradePrivilege(hKey, CW2T(lpSubKey));
        }

        i++;
    }

    return ret;
}


LONG RegCreateKeyExPrvgW(HKEY hKey,
						 LPCWSTR lpSubKey,
						 DWORD Reserved,
						 LPWSTR lpClass,
						 DWORD dwOptions,
						 REGSAM samDesired,
						 LPSECURITY_ATTRIBUTES lpSecurityAttributes,
						 PHKEY phkResult,
						 LPDWORD lpdwDisposition)
{
	LONG ret = 0;
	for ( int i = 0; i < 2; i++ )
	{
		if ( ERROR_SUCCESS == (ret = ::RegCreateKeyExW(
			hKey, lpSubKey, Reserved, lpClass, 
			dwOptions, samDesired, lpSecurityAttributes, phkResult,
			lpdwDisposition)) )
		{
			break;
		}
		if ( i == 0 )
		{
			UpgradePrivilege(hKey, CW2T(lpSubKey));
		}		
	}
	return ret;
}


int ExtractRootKeyTypeW(const wchar_t* pszRegPath, 
						HKEY& hRootKey, 
						const wchar_t** ppSubKey)
{
	
	if (NULL == pszRegPath)
		return -1;

	int nRetCode = 0;
	if (0 == _wcsnicmp(pszRegPath, HKEY_LOCAL_MACHINE_PATH_STRW, HKEY_LOCAL_MACHINE_PATH_LEN))
	{
		hRootKey = HKEY_LOCAL_MACHINE;
		if (ppSubKey)
			*ppSubKey = pszRegPath + HKEY_LOCAL_MACHINE_PATH_LEN;
	}
	else if (0 == _wcsnicmp(pszRegPath, HKEY_CURRENT_USER_PATH_STRW, HKEY_CURRENT_USER_PATH_LEN))
	{
		hRootKey = HKEY_CURRENT_USER;
		if (ppSubKey)
			*ppSubKey = pszRegPath + HKEY_CURRENT_USER_PATH_LEN;
	}
	else if (0 == _wcsnicmp(pszRegPath, HKEY_CLASSES_ROOT_PATH_STRW, HKEY_CLASSES_ROOT_PATH_LEN))
	{
		hRootKey = HKEY_CLASSES_ROOT;
		if (ppSubKey)
			*ppSubKey = pszRegPath + HKEY_CLASSES_ROOT_PATH_LEN;
	}
	else if (0 == _wcsnicmp(pszRegPath, HKEY_USERS_PATH_STRW, HKEY_USERS_PATH_LEN))
	{
		hRootKey = HKEY_USERS;
		if (ppSubKey)
			*ppSubKey = pszRegPath + HKEY_USERS_PATH_LEN;
	}
	else if (0 == _wcsnicmp(pszRegPath, HKEY_CURRENT_CONFIG_PATH_STRW, HKEY_CURRENT_CONFIG_PATH_LEN))
	{
		hRootKey = HKEY_CURRENT_CONFIG;
		if (ppSubKey)
			*ppSubKey = pszRegPath + HKEY_CURRENT_CONFIG_PATH_LEN;
	}
	else if (0 == _wcsnicmp(pszRegPath, HKEY_DYN_DATA_PATH_STRW, HKEY_DYN_DATA_PATH_LEN))
	{
		hRootKey = HKEY_DYN_DATA;
		if (ppSubKey)
			*ppSubKey = pszRegPath + HKEY_DYN_DATA_PATH_LEN;
	}
	else
	{
		nRetCode = -1;
	}
	return nRetCode;
}


int ExtractRootKeyTypeByShortValue(const wchar_t* pszRegPath, 
								   HKEY& hRootKey, 
						           HKEY& hRootKeyAother)
{
	int nRetCode = 0;

	if (NULL == pszRegPath)
		return -1;

	if (0 == _wcsnicmp(pszRegPath, HKEY_CURRENT_USER_LOCAL_MACHINE_SHORT, sizeof(HKEY_CURRENT_USER_LOCAL_MACHINE_SHORT)))
	{
		hRootKey = HKEY_LOCAL_MACHINE;
		hRootKeyAother = HKEY_CURRENT_USER;
	}
	else if (0 == _wcsnicmp(pszRegPath, HKEY_CURRENT_USER_SHORT, sizeof(HKEY_CURRENT_USER_SHORT)))
	{
		hRootKey = HKEY_CURRENT_USER;
		hRootKeyAother = NULL;
	}
	else if (0 == _wcsnicmp(pszRegPath, HKEY_CLASSES_ROOT_SHORT, sizeof(HKEY_CLASSES_ROOT_SHORT)))
	{
		hRootKey = HKEY_CLASSES_ROOT;
		hRootKeyAother = NULL;
	}
	else if (0 == _wcsnicmp(pszRegPath, HKEY_LOCAL_MACHINE_SHORT, sizeof(HKEY_LOCAL_MACHINE_SHORT)))
	{
		hRootKey = HKEY_LOCAL_MACHINE;
		hRootKeyAother = NULL;
	}
	else
	{
		nRetCode = -1;
	}
	return nRetCode;
}


const wchar_t* GetRegRootKeyStrW(HKEY hRootKey)
{
	const wchar_t* pszRootKeyStr = NULL;
	switch((DWORD)hRootKey)
	{
	case HKEY_LOCAL_MACHINE:
		pszRootKeyStr = HKEY_LOCAL_MACHINE_STRW;
		break;
	case HKEY_CURRENT_USER:
		pszRootKeyStr = HKEY_CURRENT_USER_STRW;
		break;
	case HKEY_CLASSES_ROOT:
		pszRootKeyStr = HKEY_CLASSES_ROOT_STRW;
		break;
	case HKEY_USERS:
		pszRootKeyStr = HKEY_USERS_STRW;
		break;
	case HKEY_CURRENT_CONFIG:
		pszRootKeyStr = HKEY_CURRENT_CONFIG_STRW;
		break;
	case HKEY_DYN_DATA:
		pszRootKeyStr = HKEY_DYN_DATA_STRW;
		break;
	default:
		pszRootKeyStr = NULL;
		break;
	}
	return pszRootKeyStr;
}

const TCHAR* GetRegPath(const TCHAR* pszRegPath, 
						HKEY hRootKey, 
						const TCHAR* pszSubKey, 
						std::wstring &strRegPath)
{
	if (NULL == pszRegPath)
	{
		const TCHAR* pszRootKeyPath = GetRegRootKeyStrW(hRootKey);
		if (pszRootKeyPath != NULL)
		{
			strRegPath = pszRootKeyPath;
			strRegPath += pszSubKey;
			pszRegPath = strRegPath.c_str();
		}
	}	
	return pszRegPath;
}



int CheckRegKeyExists(HKEY hRootKey, 
					  const TCHAR* pszSubKey, 
					  bool& bIsExist)
{
    HKEY hSubKey = NULL; 
    bIsExist = false;

    if (ERROR_SUCCESS == RegOpenKeyExPrvg(hRootKey, 
										  pszSubKey, 
										  0, 
										  KEY_QUERY_VALUE, 
										  &hSubKey))
    {
        bIsExist = true;
        ::RegCloseKey(hSubKey);	
    }

    return 0;
}

int CheckRegValueExist(HKEY hRootKey, 
					   const TCHAR* pszSubKey, 
					   const TCHAR* pszRegPath, 
					   const TCHAR* pszValueName, 
					   bool& bIsExist)
{
	int nRetCode = 0;
	std::wstring strRegPath;
	pszRegPath = GetRegPath(pszRegPath, hRootKey, pszSubKey, strRegPath);

	if (NULL == hRootKey || NULL == pszSubKey || NULL == pszRegPath)
	{
		nRetCode = -1;
	}
	else
	{
		bIsExist = false;
		HKEY hSubKey = NULL;

		if(ERROR_SUCCESS == RegOpenKeyExPrvg(hRootKey, 
											 pszSubKey, 
											 0, 
											 KEY_QUERY_VALUE, 
											 &hSubKey))
		{
			if (ERROR_SUCCESS == ::RegQueryValueEx(hSubKey, 
												   pszValueName, 
												   NULL, 
												   NULL, 
												   NULL, 
												   NULL))
			{
				bIsExist = true;
			}
			::RegCloseKey(hSubKey);
		}
		
	}
	return nRetCode;
}

int EnumCheckRegValueExist(HKEY hRootKey, 
					       const TCHAR* pszSubKey, 
						   const TCHAR* pszValueName, 
					       bool& bIsExist)
{
	int nRetCode = 0;
	
	if (NULL == hRootKey || NULL == pszSubKey)
	{
		nRetCode = -1;
	}
	else
	{
		bIsExist = false;
		HKEY hSubKey = NULL;

		if(ERROR_SUCCESS == RegOpenKeyExPrvg(hRootKey, 
											 pszSubKey, 
											 0, 
											 KEY_READ, 
											 &hSubKey))
		{
			
			wchar_t wszSubSubKey[MAX_STRING_SIZE];;
			int nIndex = 0;

			ZeroMemory(wszSubSubKey,MAX_STRING_SIZE);

			while(KAERegEnumKey(hSubKey, nIndex, wszSubSubKey,MAX_STRING_SIZE))
			{
				std::wstring subkey(L"");

				if(NULL !=  StrStrI(wszSubSubKey,L"Your Image File Name Here without a path"))
				{
					nIndex++;
					continue;
				}

				
				subkey+=pszSubKey;
				subkey+=L"\\";
				subkey+=wszSubSubKey;


				CheckRegValueExist(hRootKey,
								   subkey.c_str(),
								   NULL,
								   pszValueName,
								   bIsExist);
				
				if(bIsExist)
					break;

				nIndex++;
				ZeroMemory(wszSubSubKey,MAX_STRING_SIZE);
			}
			::RegCloseKey(hSubKey);

		}

	}
	return nRetCode;
}


int EnumCheckRegValueDataExist(HKEY hRootKey, 
							   const TCHAR* pszSubKey, 
						       const TCHAR* pszValueName, 
						       const TCHAR* pszValueData,
						       DWORD pszValueDataSize,
						       bool& bIsExist)
{
	HRESULT nRetCode = S_OK;

	if (NULL == hRootKey || NULL == pszSubKey)
	{
		nRetCode = S_FALSE;
	}
	else
	{
		bIsExist = false;
		HKEY hSubKey = NULL;

		if(ERROR_SUCCESS == RegOpenKeyExPrvg(hRootKey, pszSubKey, 0, KEY_READ, &hSubKey))
		{
			DWORD dwSize = MAX_STRING_SIZE;
			WCHAR wszSubSubKey[MAX_STRING_SIZE];
			DWORD dwIndex=0;

			ZeroMemory(wszSubSubKey,MAX_STRING_SIZE);

			while (KAERegEnumKey(hSubKey, dwIndex, wszSubSubKey, dwSize))
			{
				std::wstring subkey(L"");
				bool valueexist=FALSE;

				subkey+=pszSubKey;
				subkey+=L"\\";
				subkey+=wszSubSubKey;

				CheckRegValueExist(hRootKey,
								   subkey.c_str(),
								   NULL,
								   pszValueName,
								   valueexist);

				if(valueexist)
				{
					WCHAR buffer[MAX_PATH * 2];
					ZeroMemory(buffer, sizeof(buffer));
					DWORD len = sizeof(buffer);

					nRetCode = GetRegistryValue(hRootKey, subkey.c_str(), pszValueName, NULL, (LPBYTE)buffer, &len);
					
                    if(SUCCEEDED(nRetCode))
                    {
					    if(0 ==  lstrcmpi(buffer, pszValueData))
					    {
						    bIsExist = TRUE;
						    break;
					    }
                    }
				}

				ZeroMemory(wszSubSubKey,MAX_STRING_SIZE);
				dwIndex ++;
			}
			::RegCloseKey(hSubKey);
		}
        else nRetCode = S_FALSE;

	}
	return nRetCode;
}


int EnumCheckRegValueData(HKEY hRootKey, 
						  const TCHAR* pszSubKey, 
						  const TCHAR* pszValueName, 
						  const TCHAR* pszValueData,
						  DWORD pszValueDataSize,
						  std::list<std::wstring> &listsubkey)
{
	int nRetCode = 0;

	if (NULL == hRootKey || NULL == pszSubKey)
	{
		nRetCode = -1;
	}
	else
	{
		HKEY hSubKey = NULL;

		if(ERROR_SUCCESS == RegOpenKeyExPrvg(hRootKey, 
											 pszSubKey, 
											 0, 
											 KEY_READ, 
											 &hSubKey))
		{

			DWORD dwSize = MAX_STRING_SIZE;
			WCHAR wszSubSubKey[MAX_STRING_SIZE];
			DWORD dwIndex=0;

			ZeroMemory(wszSubSubKey,MAX_STRING_SIZE);

			while (KAERegEnumKey(hSubKey, dwIndex, wszSubSubKey, dwSize))
			{
				std::wstring subkey(L"");
				bool valueexit=FALSE;

				subkey+=pszSubKey;
				subkey+=L"\\";
				subkey+=wszSubSubKey;
				
				CheckRegValueExist(hRootKey,
								   subkey.c_str(),
								   NULL,
								   pszValueName,
								   valueexit);
				if(valueexit)
				{
					WCHAR buffer[MAX_PATH*2];
					ZeroMemory(buffer, sizeof(buffer));
					DWORD len = sizeof(buffer);

					GetRegistryValue(hRootKey, 
									 subkey.c_str(), 
									 pszValueName, 
									 NULL, 
									 (LPBYTE)buffer, 
									 &len);


					if(0 ==  lstrcmpi(buffer, pszValueData ))
					{
						listsubkey.push_back(wszSubSubKey);
					}

				}

				ZeroMemory(wszSubSubKey,1024);
				dwIndex ++;
			}
			::RegCloseKey(hSubKey);
		}

	}
	return nRetCode;
}


int EnumCheckRegValue(HKEY hRootKey, 
					  const TCHAR* pszSubKey, 
					  const TCHAR* pszValueName, 
					  std::list<std::wstring> &listsubkey)
{
	int nRetCode = 0;

	if (NULL == hRootKey || NULL == pszSubKey)
	{
		nRetCode = -1;
	}
	else
	{

		HKEY hSubKey = NULL;
		if(ERROR_SUCCESS == RegOpenKeyExPrvg(hRootKey, 
							 pszSubKey, 
							 0, 
							 KEY_READ, 
							 &hSubKey))
		{
			wchar_t wszSubSubKey[MAX_STRING_SIZE];
			int nIndex = 0;
			ZeroMemory(wszSubSubKey,MAX_STRING_SIZE);

			while(KAERegEnumKey(hSubKey, nIndex, wszSubSubKey,MAX_STRING_SIZE))
			{
				std::wstring subkey(L"");
				bool bIsExist = FALSE;

				subkey+=pszSubKey;
				subkey+=L"\\";
				subkey+=wszSubSubKey;

				CheckRegValueExist(hRootKey,
								   subkey.c_str(),
								   NULL,
								   pszValueName,
								   bIsExist);

				if(bIsExist)
					listsubkey.push_back(wszSubSubKey);

				nIndex++;
				ZeroMemory(wszSubSubKey,MAX_STRING_SIZE);
				
			}
			::RegCloseKey(hSubKey);
		}

	}
	return nRetCode;
}


HRESULT GetRegistryValue(HKEY hRootKey, 
                        const wchar_t* pszSubKeyName,
                        const wchar_t* pszValueName, 
                        DWORD* pdwType, 
                        BYTE* pszValue, 
                        DWORD* puValueSize)
{
    if (NULL == pszSubKeyName || ((pszValue != NULL) && (NULL == puValueSize)))
        return E_FAIL;

    HRESULT hr = S_OK;
    HKEY hSubKey = NULL;
    long lResult = ERROR_SUCCESS;

    lResult = RegOpenKeyExPrvg(hRootKey, pszSubKeyName, 0, KEY_READ, &hSubKey);
    if (ERROR_SUCCESS == lResult)
    {
        lResult = ::RegQueryValueEx(hSubKey, pszValueName, 0, pdwType, pszValue, puValueSize);
        if (ERROR_SUCCESS != lResult)
        {
            hr = E_FAIL;
        }
    }

    if (hSubKey != NULL)
    {
        ::RegCloseKey(hSubKey);
        hSubKey = NULL;
    }

    return hr;
}

HRESULT RemoveRegistryKey(HKEY hRootKey, 
						  const WCHAR* pszSubKeyName)
{
    if (NULL == pszSubKeyName)
        return E_FAIL;

    HRESULT hr = S_OK;
    long lResult = ERROR_SUCCESS;

   

    if(lstrcmpi(pszSubKeyName,H_CLSID) == 0
        ||lstrcmpi(pszSubKeyName,H_CLSID_STRW) == 0
        ||lstrcmpi(pszSubKeyName,H_RUN) == 0
        ||lstrcmpi(pszSubKeyName,H_RUN_STRW) == 0
        ||lstrcmpi(pszSubKeyName,H_SERVICE) == 0
        ||lstrcmpi(pszSubKeyName,H_SERVICE_STRW) == 0
        ||lstrcmpi(pszSubKeyName,H_MICROSOFT) == 0
        ||lstrcmpi(pszSubKeyName,H_MICROSOFT_STRW) == 0
        ||lstrcmpi(pszSubKeyName,H_MICROSOFT_WIN) == 0
        ||lstrcmpi(pszSubKeyName,H_MICROSOFT_WIN_STRW) == 0
        ||lstrcmpi(pszSubKeyName,H_MICROSOFT_WINNT) == 0
        ||lstrcmpi(pszSubKeyName,H_MICROSOFT_WINNT_STRW) == 0)
        return E_FAIL;

    KAGRegUpKeySecurity(hRootKey, pszSubKeyName);

    if ((lResult = ::SHDeleteKey(hRootKey, pszSubKeyName)) != ERROR_SUCCESS)
    {
        if (lResult == ERROR_FILE_NOT_FOUND)
            hr = S_FALSE;
        else
            hr = E_FAIL;
    }
    return hr;
}

HRESULT RemoveRegistryValue(HKEY hRootKey, 
							const TCHAR*  pszSubKeyName, 
							const TCHAR*  pszValueName)
{
	if (NULL == pszSubKeyName || NULL == pszValueName)
		return E_FAIL;

	HRESULT hr = S_OK;
	long lResult = ERROR_SUCCESS;
	HKEY hSubKey = NULL;
	if ((lResult = RegOpenKeyExPrvg(hRootKey, 
									pszSubKeyName, 
									0, 
									KEY_READ| KEY_WRITE, 
									&hSubKey)) != ERROR_SUCCESS)
	{
		if (lResult == ERROR_FILE_NOT_FOUND)
			hr = S_FALSE;
		else
			hr = E_FAIL;
	}
	else if ((lResult = ::RegDeleteValue(hSubKey, pszValueName)) != ERROR_SUCCESS)
	{
		if (lResult == ERROR_FILE_NOT_FOUND)
			hr = S_FALSE;
		else
			hr = E_FAIL;
	}
 

	if (hSubKey != NULL)
	{
		::RegCloseKey(hSubKey);
		hSubKey = NULL;
	}

	
	return hr;
}


HRESULT SetRegistryValue(HKEY hRootKey, 
						 const wchar_t* pszSubKeyName,  
						 const wchar_t* pszValueName, 
						 DWORD dwType, 
						 const BYTE* pValue, 
						 DWORD uValueSize)
{
	if (NULL == pszSubKeyName || NULL == pValue)
		return E_FAIL;

	HRESULT hr = S_OK;
	HKEY hSubKey = NULL;

	long lResult = RegCreateKeyExPrvgW(hRootKey, 
									   pszSubKeyName, 
									   0, 
									   NULL, 
									   REG_OPTION_NON_VOLATILE, 
									   KEY_READ| KEY_WRITE, 
									   NULL, 
									   &hSubKey, 
									   NULL);
	if (lResult != ERROR_SUCCESS)
	{
		hr = E_FAIL;
	}
	else if ((lResult = 
		::RegSetValueExW(hSubKey,
						 pszValueName, 
						 0, 
						 dwType, 
						 pValue, 
						 uValueSize)) != ERROR_SUCCESS)
	{
		hr = E_FAIL;
	}

	if (hSubKey != NULL)
	{
		::RegCloseKey(hSubKey);
		hSubKey = NULL;
	}
	return hr;
}


int CheckRegValueMatch(HKEY hRootKey, 
					   const TCHAR* pszSubKey, 
					   int& nMatchCnt,
					   wchar_t* szMatchValues,
					   DWORD* puValueNamesSize)
{
	int nRetCode = 0;
	nMatchCnt = 0;

	if (NULL == hRootKey || NULL == pszSubKey)
	{
		nRetCode = -1;
	}
	else
	{
		nRetCode = EnumRegKeyValue(hRootKey, pszSubKey, nMatchCnt, szMatchValues, puValueNamesSize);
	}
	return nRetCode;
}


int EnumRegKeyValue(HKEY hRootKey, 
					const TCHAR* pszSubKey,
					int& nMatchCnt, 
					wchar_t* pstrValueName,
					DWORD* puValueNamesSize)
{
	int nRetCode = 0;
	HKEY hSubKey = NULL;

	nMatchCnt = 0;
	
	if (ERROR_SUCCESS != 
		RegOpenKeyExPrvg(hRootKey, 
						 pszSubKey, 
						 0, 
						 KEY_READ, 
						 &hSubKey))
	{
		nRetCode = -1;
	}
	else
	{
		DWORD dwId					= 0;
		DWORD dwValueCount			= 0;
		DWORD dwMaxValueNameLen		= 0;
		DWORD dwCurrentValueNameLen = MAX_PATH;	
		WCHAR pszCurrentValueName[MAX_PATH];

		nRetCode = ::RegQueryInfoKey(hSubKey, 
									 NULL, 
									 NULL, 
									 NULL, 
									 NULL, 
									 NULL, 
									 NULL, 
									 &dwValueCount, 
									 &dwMaxValueNameLen,
									 NULL,
									 NULL,
									 NULL);

		if (ERROR_SUCCESS == nRetCode)
		{
			for (dwId = 0; dwId < dwValueCount; ++dwId)
			{
				::RtlZeroMemory(pszCurrentValueName, MAX_PATH * sizeof(WCHAR));
				if (ERROR_SUCCESS == ::RegEnumValue(hSubKey, 
													dwId, 
													pszCurrentValueName, 
													&dwCurrentValueNameLen, 
													NULL, 
													NULL, 
													NULL, 
													NULL))
				{
					WCHAR buffer[MAX_PATH*2];
					ZeroMemory(buffer, sizeof(buffer));
					DWORD len = sizeof(buffer);
					GetRegistryValue(hRootKey, 
									 pszSubKey,
									 pszCurrentValueName,
									 NULL,
									 (LPBYTE)buffer,
									 &len);
					if( 0 ==  lstrcmpi(buffer, pstrValueName))
						nMatchCnt++;			
				}
			}
		}
		if (NULL != hSubKey)
		{
			::RegCloseKey(hSubKey);
			hSubKey = NULL;
		}		
	}
	return nRetCode;
}


int EnumGetRegKeyAndValueAndData(HKEY hRootKey, 
						  const TCHAR* pszSubKey,
						  std::set<std::wstring> &listdata,
						  std::set<std::wstring> &listfiledata)
{
	int nRetCode = 0;

	if (NULL == hRootKey || NULL == pszSubKey)
	{
		nRetCode = -1;
	}
	else
	{

		HKEY hSubKey = NULL;
		if(ERROR_SUCCESS == 
			RegOpenKeyExPrvg(hRootKey, pszSubKey, 0, KEY_READ, &hSubKey))
		{
			
			std::wstring keyspace(L"");
            
			listdata.insert(keyspace+GetRegRootKeyStrW(hRootKey)+L"\\"+pszSubKey);
			EnumGetRegValue(hRootKey,
							pszSubKey,
							listdata,
							listfiledata);
            
			DWORD dwRtn = ERROR_SUCCESS;
			DWORD dwSize = MAX_STRING_SIZE;
			WCHAR wszSubSubKey[MAX_STRING_SIZE];
			DWORD dwIndex=0;
			ZeroMemory(wszSubSubKey,MAX_STRING_SIZE);

			while ( ERROR_SUCCESS == dwRtn)
			{
				dwRtn = ::RegEnumKeyW(hSubKey, dwIndex, wszSubSubKey, dwSize);
				std::wstring subkey(pszSubKey);
				
				if(wcslen(wszSubSubKey)>0)
				{
					subkey+=L"\\";
					subkey+=wszSubSubKey;
				}
				

				if(dwRtn == ERROR_NO_MORE_ITEMS)
				{
					EnumGetRegValue(hRootKey,subkey.c_str(),listdata,listfiledata);
					break;
				}
				else if(dwRtn == ERROR_SUCCESS)
				{
					
					listdata.insert(keyspace+GetRegRootKeyStrW(hRootKey)+L"\\"+subkey);
					dwRtn = EnumGetRegKeyAndValueAndData(hRootKey,subkey.c_str(),listdata,listfiledata);
				
				}

				ZeroMemory(wszSubSubKey,MAX_STRING_SIZE);
				dwIndex ++;
			}
			::RegCloseKey(hSubKey);
		}

	}
	return nRetCode;
}



int EnumGetRegValue(HKEY hRootKey, 
					const TCHAR* pszSubKey, 
					std::set<std::wstring> &listvaluedata, 
					std::set<std::wstring> &listfiledata)
{
	int nRetCode = 0;

	if (NULL == hRootKey || NULL == pszSubKey)
	{
		nRetCode = -1;
	}
	else
	{
	
		DWORD dwId					= 0;
		DWORD dwValueCount			= 0;
		DWORD dwMaxValueNameLen		= 0;
		DWORD dwCurrentValueNameLen = MAX_PATH;	
		WCHAR pszCurrentValueName[MAX_PATH];
		HKEY hSubKey = NULL;
		std::wstring rootkey(GetRegRootKeyStrW(hRootKey));

		RegOpenKeyExPrvg(hRootKey, 
						 pszSubKey, 
						 0, 
						 KEY_READ, 
						 &hSubKey);

		nRetCode = ::RegQueryInfoKey(hSubKey, 
									NULL,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL, 
									&dwValueCount,
									&dwMaxValueNameLen,
									NULL, 
									NULL,
									NULL);

		if (ERROR_SUCCESS == nRetCode)
		{
			for (dwId = 0; dwId < dwValueCount; ++dwId)
			{
				::RtlZeroMemory(pszCurrentValueName, MAX_PATH * sizeof(WCHAR));
				if (ERROR_SUCCESS == ::RegEnumValue(hSubKey, 
													dwId, pszCurrentValueName, 
													&dwCurrentValueNameLen,
													NULL, 
													NULL,
													NULL, 
													NULL))
				{
					BYTE buffer[MAX_PATH];
					ZeroMemory(buffer, sizeof(buffer));
					DWORD len = sizeof(buffer);
                    DWORD dwtype = 0;

					GetRegistryValue(hRootKey, 
									 pszSubKey,
									 pszCurrentValueName,
									 &dwtype, 
									 buffer,
									 &len);

					std::wstring tempvalue,tempsubkey(pszSubKey),keyspace(L"\\"),valuespace(L"  ["),valuespacer(L"]  ");
                    
                    if(REG_BINARY == dwtype)
                        tempvalue=L"Binary Type Data";
                    else if(REG_DWORD == dwtype)
                        tempvalue=L"DWORD Type Data";
                    else
					    tempvalue.assign((wchar_t*)buffer);
                    if(tempvalue.find(L"\\") != std::wstring::npos && tempvalue.at(wcslen(tempvalue.c_str())-1)!='\\' && tempvalue.find(L":") != std::wstring::npos)
						listfiledata.insert(tempvalue);
					else 
					{
						std::wstring valuename(L"");
						if(wcslen(pszCurrentValueName)==0)
							valuename = L"Default";
						else valuename.assign(pszCurrentValueName);
						if(tempvalue.size()==0)
							tempvalue = L"NULL";
						listvaluedata.insert(rootkey+keyspace+tempsubkey+valuespace+
											 valuename+valuespacer+
											 tempvalue);
					}
				}
			}
		}
		if (NULL != hSubKey)
		{
			::RegCloseKey(hSubKey);
			hSubKey = NULL;
		}		

	}
	return nRetCode;
}



int EnumRemoveRegKeyValueName(HKEY hRootKey, 
					const TCHAR* pszSubKey)
{
	int nRetCode = 0;
	HKEY hSubKey = NULL;


	if (ERROR_SUCCESS != 
		RegOpenKeyExPrvg(hRootKey, 
		pszSubKey, 
		0, 
		KEY_READ, 
		&hSubKey))
	{
		nRetCode = -1;
	}
	else
	{
		DWORD dwId					= 0;
		DWORD dwValueCount			= 0;
		DWORD dwMaxValueNameLen		= 0;
		DWORD dwCurrentValueNameLen = MAX_PATH;	
		WCHAR pszCurrentValueName[MAX_PATH];

		nRetCode = ::RegQueryInfoKey(hSubKey, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			&dwValueCount, 
			&dwMaxValueNameLen,
			NULL,
			NULL,
			NULL);

		if (ERROR_SUCCESS == nRetCode)
		{
			for (dwId = 0; dwId < dwValueCount; ++dwId)
			{
				::RtlZeroMemory(pszCurrentValueName, MAX_PATH * sizeof(WCHAR));
				if (ERROR_SUCCESS == ::RegEnumValue(hSubKey, 
					dwId, 
					pszCurrentValueName, 
					&dwCurrentValueNameLen, 
					NULL, 
					NULL, 
					NULL, 
					NULL))
				{
					
					if(wcslen(pszCurrentValueName)>0 && L'{' == pszCurrentValueName[0])
					{
						::RegDeleteValue(hSubKey, pszCurrentValueName);
					}
									
				}
			}
		}
		if (NULL != hSubKey)
		{
			::RegCloseKey(hSubKey);
			hSubKey = NULL;
		}		
	}
	return nRetCode;
}

int EnumRegKeyValueName(HKEY hRootKey, 
						const TCHAR* pszSubKey,
						bool exits)
{
	int nRetCode = 0;
	HKEY hSubKey = NULL;


	if (ERROR_SUCCESS != 
		RegOpenKeyExPrvg(hRootKey, 
		pszSubKey, 
		0, 
		KEY_READ, 
		&hSubKey))
	{
		nRetCode = -1;
	}
	else
	{
		DWORD dwId					= 0;
		DWORD dwValueCount			= 0;
		DWORD dwMaxValueNameLen		= 0;
		DWORD dwCurrentValueNameLen = MAX_PATH;	
		WCHAR pszCurrentValueName[MAX_PATH];

		nRetCode = ::RegQueryInfoKey(hSubKey, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			NULL, 
			&dwValueCount, 
			&dwMaxValueNameLen,
			NULL,
			NULL,
			NULL);

		if (ERROR_SUCCESS == nRetCode)
		{
			for (dwId = 0; dwId < dwValueCount; ++dwId)
			{
				::RtlZeroMemory(pszCurrentValueName, MAX_PATH * sizeof(WCHAR));
				if (ERROR_SUCCESS == ::RegEnumValue(hSubKey, 
					dwId, 
					pszCurrentValueName, 
					&dwCurrentValueNameLen, 
					NULL, 
					NULL, 
					NULL, 
					NULL))
				{

					if(wcslen(pszCurrentValueName)>0 && L'{' == pszCurrentValueName[0])
					{
						exits = TRUE;
						break;
					}

				}
			}
		}
		if (NULL != hSubKey)
		{
			::RegCloseKey(hSubKey);
			hSubKey = NULL;
		}		
	}
	return nRetCode;
}



// -------------------------------------------------------------------------

