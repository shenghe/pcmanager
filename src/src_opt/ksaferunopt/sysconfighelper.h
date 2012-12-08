#pragma once

class CSysConfigOper
{
public:
	CSysConfigOper(){}
	~CSysConfigOper(){}

public:
	BOOL OptimizeSysConfig(
		DWORD dwFlags, 
		HKEY hRootKey, 
		CString strSubKey, 
		CString strValueName, 
		CString strDefaultValue, 
		CString strOptmizeValue,
		CString strInitValue,
		DWORD dwRegValueType
		)
	{
		BOOL bRet = FALSE;
		if (dwFlags == KSAFE_SYSCONFIG_RESTORE_SYSDEF)
		{
			if (strDefaultValue.CompareNoCase(KSAFE_REGSYSOPT_KEY_NULL) == 0)
			{
				bRet = _DeleteRegValue(hRootKey, strSubKey, strValueName, KEY_WRITE);
				goto Exit0;
			}
			else
			{
				LRESULT lRet;
				if (dwRegValueType == REG_SZ)
				{
					lRet = SHSetValue(
						hRootKey, 
						strSubKey.GetString(), 
						strValueName.GetString(), 
						dwRegValueType, 
						strDefaultValue.GetString(),
						strDefaultValue.GetLength() * sizeof(TCHAR)
						);
				}
				else if (dwRegValueType == REG_DWORD)
				{
					DWORD dwData = _ttoi(strDefaultValue.GetString());
					lRet = SHSetValue(
						hRootKey, 
						strSubKey.GetString(), 
						strValueName.GetString(), 
						dwRegValueType, 
						LPVOID(&dwData),
						sizeof(DWORD)
						);
				}
				else
				{
					bRet = FALSE;
					goto Exit0;
				}

				bRet =(lRet == ERROR_SUCCESS); 
			}
		}
		else if (dwFlags == KSAFE_SYSCONFIG_OPTIMIZE)
		{
			LRESULT lRet;

			if (dwRegValueType == REG_SZ)
			{
				lRet= SHSetValue(
					hRootKey,
					strSubKey.GetString(),
					strValueName.GetString(),
					dwRegValueType,
					(LPBYTE)strOptmizeValue.GetString(),
					strOptmizeValue.GetLength() * sizeof(TCHAR)
					);
			}
			else if (dwRegValueType == REG_DWORD)
			{
				DWORD dwData = _ttoi(strOptmizeValue.GetString());
				lRet= SHSetValue(
					hRootKey,
					strSubKey.GetString(),
					strValueName.GetString(),
					dwRegValueType,
					(LPBYTE)(&dwData),
					sizeof(DWORD)
					);
			}
			else
			{
				bRet = FALSE;
				goto Exit0;
			}

			bRet =(lRet == ERROR_SUCCESS) ;
		}
		else if (dwFlags == KSAFE_SYSCONFIG_RESTORE_INIT)
		{
			if (strInitValue.CompareNoCase(KSAFE_REGSYSOPT_KEY_NULL) == 0)
			{
				bRet = _DeleteRegValue(hRootKey, strSubKey, strValueName, KEY_WRITE);
				goto Exit0;
			}
			else
			{
				LRESULT lRet;
				if (dwRegValueType == REG_SZ)
				{
					lRet = SHSetValue(
						hRootKey, 
						strSubKey.GetString(), 
						strValueName.GetString(), 
						dwRegValueType, 
						strInitValue.GetString(),
						strInitValue.GetLength() * sizeof(TCHAR)
						);
				}
				else if (dwRegValueType == REG_DWORD)
				{
					DWORD dwData = _ttoi(strInitValue.GetString());
					lRet = SHSetValue(
						hRootKey, 
						strSubKey.GetString(), 
						strValueName.GetString(), 
						dwRegValueType, 
						LPVOID(&dwData),
						sizeof(DWORD)
						);
				}
				else
				{
					bRet = FALSE;
					goto Exit0;
				}

				bRet = (lRet == ERROR_SUCCESS); 
			}
		}
		else
		{
			bRet = FALSE;
		}
Exit0:
		return bRet;
	}

	BOOL _DeleteRegValue(HKEY hRoot, LPCTSTR lpKey, LPCTSTR lpValue, DWORD samDesired)
	{
		HKEY hKey = NULL;
		DWORD dwRet = FALSE;

		if (ERROR_SUCCESS == (dwRet = ::RegOpenKeyEx(hRoot, lpKey, NULL, samDesired, &hKey)))
		{
			dwRet = ::RegDeleteValue(hKey, lpValue);
			::RegCloseKey(hKey);
		}

		return dwRet==ERROR_SUCCESS;
	}
};