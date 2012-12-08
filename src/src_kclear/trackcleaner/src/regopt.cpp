#include "stdafx.h"
#include "regopt.h"

/************************************************************************/
//枚举当前所有值可递归
/************************************************************************/

void CRegOpt::DoRegEnumeration(HKEY rootKey,LPCTSTR lpItemPath,EnumerateRegFun fnRegFun,void* pUserData/* =NULL */)
{	
	if(s_bUserBreak) return;

	HKEY hKey = NULL;
	LONG lResult;

	LPTSTR lpName = new TCHAR[MAX_PATH];
	LPBYTE lpValue = new BYTE[MAX_PATH * 20];
	if (lpValue == NULL)
	{
		goto clean0;
	}
	if (lpName == NULL)
	{
		goto clean0;
	}

	lResult = RegOpenKeyEx(rootKey,
		lpItemPath,
		NULL,
		KEY_READ,
		&hKey
		);

	if(lResult != ERROR_SUCCESS)
	{	
		m_iErrCode = ::GetLastError();
		goto clean0;

	}
	
	/************************************************************************/
	/*枚举值
	/************************************************************************/
	BEGIN
	DWORD dwIndex=0;
	do
	{	
		if (lpName == NULL||lpValue ==NULL)
		{
			m_iErrCode = ::GetLastError();
			goto clean0;
		}

		memset(lpName,0,sizeof(lpName));
		memset(lpValue,0,sizeof(lpValue));
		DWORD dwName  = MAX_PATH;
		DWORD dwValue = MAX_PATH * 20;		
		DWORD dwType;

		lResult = RegEnumValue(
			hKey,
			dwIndex,
			lpName,
			&dwName,
			NULL,
			&dwType,
			lpValue,
			&dwValue
			);

		if (lResult == ERROR_MORE_DATA)
		{	
			//delete[] lpName;
			delete[] lpValue;
			//lpName = NULL;
			lpValue = NULL;

			//lpName = new TCHAR[dwName+2];
			lpValue = new BYTE [dwValue+10];
			//memset(lpName,0,sizeof(lpName));
			memset(lpValue,0,sizeof(lpValue) + 10);

			if (lpName == NULL||lpValue ==NULL)
			{
				m_iErrCode = ::GetLastError();
				goto clean0;
			}

			lResult = RegEnumValue(
				hKey,
				dwIndex,
				lpName,
				&dwName,
				NULL,
				&dwType,
				lpValue,
				&dwValue
				);

		}

		if (lResult != ERROR_SUCCESS)
		{
			if (lResult == ERROR_NO_MORE_ITEMS)
			{	
				break;
			}
			else
			{	
				m_iErrCode = GetLastError();
				goto clean0;
			}
		}
		
		if(0==wcscmp(lpName,_T("")))
		{
			wcscpy(lpName,_T("默认值"));
		}

		BOOL bRet= fnRegFun(rootKey, lpItemPath, lpName, dwName ,lpValue,dwValue, dwType, pUserData);

		//是否继续枚举
		if (bRet == FALSE)	
		{	
			s_bUserBreak = TRUE;
			goto clean0;
		}

		dwIndex++;

	} while (1);
	END	
	
	/************************************************************************/
	/*枚举键
	/************************************************************************/
	BEGIN
	DWORD dwIndex=0;
	do 
	{	
		TCHAR szKey[MAX_PATH]={0};
		DWORD dwKey = sizeof(szKey);
		lResult =RegEnumKey(hKey,dwIndex,szKey,dwKey);
		dwIndex++;

		if (lResult != ERROR_SUCCESS)
		{
			if (lResult == ERROR_NO_MORE_ITEMS)
			{	
// 				RegCloseKey(hKey);
// 				return ;
				goto clean0;
			}
			else
			{	
				m_iErrCode = GetLastError();
// 				RegCloseKey(hKey);
// 				return ;
				goto clean0;
			}
		}
		
		//!!由于递归过深有可能溢出
//#define MAX_REGPATH		2048

		//TCHAR szRoot[MAX_REGPATH]={0};
		
		CString strRoot;
		if (wcscmp(lpItemPath,_T("")) == 0 )
		{	
			strRoot.Format(_T("%s"),szKey);
			//wsprintf(szRoot,_T("%s"),szKey);
		}
		else
		{	
			strRoot.Format(_T("%s\\%s"),lpItemPath,szKey);
			//wsprintf(szRoot,_T("%s\\%s"),lpItemPath,szKey);
		}

		if (fnRegFun(rootKey, strRoot.GetBuffer(), NULL,NULL,NULL,NULL, -1, pUserData))
		{
			DoRegEnumeration(rootKey, strRoot.GetBuffer(), fnRegFun, pUserData);
		}
		

	} while (1);
	END
clean0:
	if (hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}
	if (lpName)
	{
		delete[] lpName;
		lpName = NULL;
	}
	if (lpValue)
	{
		delete[] lpValue;
		lpValue = NULL;
	}

	return ;
}

/************************************************************************/
/* 递归枚举所有子键
/************************************************************************/

BOOL CRegOpt::DoEnumCurrnetSubKeyEx(HKEY hRootKey,LPCTSTR lpcKey,CSimpleArray<CString>& vec_Key,BOOL bRes,BOOL bFullPath)
{
	HKEY hKey;
	LONG lResult;

	//打开键
	lResult = RegOpenKeyEx(hRootKey,
		lpcKey,
		NULL,
		KEY_READ,
		&hKey
		);

	if(lResult != ERROR_SUCCESS)
	{	
		m_iErrCode = lResult;
		return FALSE;
	}

	//枚举键名
	BOOL  bRet = TRUE;
	DWORD dwIndex=0;
	do 
	{	
		TCHAR szKey[MAX_PATH]={0};
		DWORD dwKey = sizeof(szKey);
		lResult =RegEnumKey(hKey,dwIndex,szKey,dwKey);
		dwIndex++;

		if (lResult != ERROR_SUCCESS)
		{
			if (lResult == ERROR_NO_MORE_ITEMS)
			{	
				bRet = TRUE;
				break;
			}
			else
			{	
				bRet = FALSE;
				m_iErrCode = lResult;
				break;
			}
		}
		
		if(bFullPath == TRUE)
		{
			CString strRegFullPath;
			strRegFullPath = lpcKey;
			strRegFullPath.Append(_T("\\"));
			strRegFullPath.Append(szKey);
			
			vec_Key.Add(strRegFullPath);
		}
		else
			vec_Key.Add(szKey);
		
		if (TRUE == bRes)
		{
			CString strSub=lpcKey;
			strSub.Append(_T("\\"));
			strSub.Append(szKey);
			DoEnumCurrnetSubKeyEx(hRootKey,strSub,vec_Key,bRes,bFullPath);
		}



	} while (1);

	RegCloseKey(hKey);

	return bRet;
}

/************************************************************************/
//获得当前键下的所有子键
/************************************************************************/

BOOL CRegOpt::DoEnumCurrnetSubKey(HKEY hRootKey,LPCTSTR lpcKey,CSimpleArray<CString>& vec_Key)
{
	HKEY hKey;
	LONG lResult;
	
	//打开键
	lResult = RegOpenKeyEx(hRootKey,
		lpcKey,
		NULL,
		KEY_READ,
		&hKey
		);

	if(lResult != ERROR_SUCCESS)
	{	
		m_iErrCode = lResult;
		return FALSE;
	}
	
	//枚举键名
	BOOL  bRet = TRUE;
	DWORD dwIndex=0;
	do 
	{	
		TCHAR szKey[MAX_PATH]={0};
		DWORD dwKey = sizeof(szKey);
		lResult =RegEnumKey(hKey,dwIndex,szKey,dwKey);
		dwIndex++;

		if (lResult != ERROR_SUCCESS)
		{
			if (lResult == ERROR_NO_MORE_ITEMS)
			{	
				bRet = TRUE;
				break;
			}
			else
			{	
				bRet = FALSE;
				m_iErrCode = lResult;
				break;
			}
		}
	
		vec_Key.Add(szKey);

	} while (1);

	RegCloseKey(hKey);

	return bRet;
}

BOOL CRegOpt::DoEnumCurrnetSubKey(HKEY hRootKey,LPCTSTR lpcKey,CAtlMap<CString,char>& vec_Key)
{
	HKEY hKey;
	LONG lResult;
	
	//打开键
	lResult = RegOpenKeyEx(hRootKey,
		lpcKey,
		NULL,
		KEY_READ,
		&hKey
		);

	if(lResult != ERROR_SUCCESS)
	{	
		m_iErrCode = lResult;
		return FALSE;
	}

	//枚举键名
	BOOL  bRet = TRUE;
	DWORD dwIndex=0;
	do 
	{	
		TCHAR szKey[MAX_PATH]={0};
		DWORD dwKey = sizeof(szKey);
		lResult =RegEnumKey(hKey,dwIndex,szKey,dwKey);

		if (lResult != ERROR_SUCCESS)
		{
			if (lResult == ERROR_NO_MORE_ITEMS)
			{	
				bRet = TRUE;
				break;
			}
			else
			{	
				bRet = FALSE;
				m_iErrCode = lResult;
				break;
			}
		}
		
		
		vec_Key.SetAt(szKey,'1');
		dwIndex++;

	} while (1);

	RegCloseKey(hKey);

	return bRet;
}
/************************************************************************/
//获得当前键下的所有值名
/************************************************************************/

BOOL CRegOpt::DoEnumCurrnetValue(HKEY hRootKey,LPCTSTR lpcKey,CSimpleArray<REGVALUE>& vec_Value,BOOL bRes)
{
	HKEY hKey;
	LONG lResult;
	
	//打开键
	lResult = RegOpenKeyEx(hRootKey,
		lpcKey,
		NULL,
		KEY_READ,
		&hKey
		);

	if(lResult != ERROR_SUCCESS)
	{	
		m_iErrCode = ::GetLastError();
		return FALSE;
	}
	
	BOOL  bRet = TRUE;
	DWORD dwIndex=0;

	do
	{	

		LPTSTR lpName = new TCHAR[MAX_PATH];
		LPBYTE  lpValue = new BYTE[MAX_PATH];
		if (lpName == NULL||lpValue ==NULL)
		{
			m_iErrCode = ERROR_NOMEMOEY
			bRet = FALSE;
			break;
		}

		memset(lpName,0,sizeof(lpName));
		memset(lpValue,0,sizeof(lpValue));
		DWORD dwName  = MAX_PATH;
		DWORD dwValue = MAX_PATH;		
		DWORD dwType;

		lResult = RegEnumValue(
			hKey,
			dwIndex,
			lpName,
			&dwName,
			NULL,
			&dwType,
			lpValue,
			&dwValue
			);

		if (lResult == ERROR_MORE_DATA)
		{	
			delete[] lpName;
			delete[] lpValue;
			lpName = NULL;
			lpValue = NULL;

			lpName = new TCHAR[dwName+1];
			lpValue = new BYTE [dwValue+1];
			memset(lpName,0,sizeof(lpName));
			memset(lpValue,0,sizeof(lpValue));

			if (lpName == NULL||lpValue ==NULL)
			{
				m_iErrCode = ERROR_NOMEMOEY
				bRet = FALSE;
				break;
			}

			lResult = RegEnumValue(
				hKey,
				dwIndex,
				lpName,
				&dwName,
				NULL,
				&dwType,
				lpValue,
				&dwValue
				);
		}

		if (lResult == ERROR_NO_MORE_ITEMS)
		{	
			bRet = TRUE;
			break;
		}
			
		if (dwName == 0)
		{
			wcscpy(lpName,_T("默认值"));
		}


		REGVALUE regValue;
		regValue.strValueName = lpName;
		regValue.dwType = dwType;

		switch (dwType)
		{
		case REG_SZ:
		case REG_EXPAND_SZ:
			{	
				regValue.strValue = (wchar_t*) lpValue;		
			}
			break;
		case REG_BINARY:
		case REG_DWORD_LITTLE_ENDIAN:
		case REG_DWORD_BIG_ENDIAN:							
		case REG_LINK:									
		case REG_MULTI_SZ:
		case REG_NONE:
		case REG_QWORD_LITTLE_ENDIAN:
			regValue.strValue = _T("");	
			break;
		}
		
		vec_Value.Add(regValue);

		//释放相应空间
		delete[] lpName;
		delete[] lpValue;
		
		
		lpName = NULL;
		lpValue = NULL;
		dwIndex++;

	} while (1);

	if (bRes == FALSE)
	{	
		RegCloseKey(hKey);
		return TRUE;
	}
	//向下层枚举
	BEGIN
	DWORD dwIndex=0;
	do 
	{	
		TCHAR szKey[MAX_PATH]={0};
		DWORD dwKey = sizeof(szKey);
		lResult =RegEnumKey(hKey,dwIndex,szKey,dwKey);
		dwIndex++;

		if (lResult != ERROR_SUCCESS)
		{
			if (lResult == ERROR_NO_MORE_ITEMS)
			{	
				RegCloseKey(hKey);
				return TRUE;
			}
			else
			{	
				m_iErrCode = GetLastError();
				RegCloseKey(hKey);
				return TRUE;
			}
		}

//		//!!由于递归过深有可能溢出
//#define MAX_REGPATH		2048
//
//		TCHAR szRoot[MAX_REGPATH]={0};
//
//
//		if (wcscmp(lpcKey,_T("")) == 0 )
//			wsprintf(szRoot,_T("%s"),szKey);
//		else
//			wsprintf(szRoot,_T("%s\\%s"),lpcKey,szKey);
	
	
		CString strRoot;
		if (wcscmp(lpcKey,_T("")) == 0 )
		{	
			strRoot.Format(_T("%s"),szKey);
			//wsprintf(szRoot,_T("%s"),szKey);
		}
		else
		{	
			strRoot.Format(_T("%s\\%s"),lpcKey,szKey);
			//wsprintf(szRoot,_T("%s\\%s"),lpItemPath,szKey);
		}

		DoEnumCurrnetValue(hRootKey, strRoot.GetBuffer(), vec_Value, bRes);


	} while (1);
	END

	RegCloseKey(hKey);
	return TRUE;

}

/************************************************************************/
//删除子键
/************************************************************************/

BOOL CRegOpt::RegDelnode(HKEY hKeyRoot, LPCTSTR lpSubKey)
{
	TCHAR szDelKey[2 * MAX_PATH];
	lstrcpy (szDelKey, lpSubKey);
	return RegDelnodeRecurse(hKeyRoot, szDelKey);
}


BOOL CRegOpt::RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey)
{
	LPTSTR lpEnd;
	LONG lResult;
	DWORD dwSize;
	TCHAR szName[MAX_PATH];
	HKEY hKey;
	FILETIME ftWrite;

	lResult = RegDeleteKey(hKeyRoot, lpSubKey);
	if (lResult == ERROR_SUCCESS) 
	{	
		m_iErrCode = GetLastError();
		return TRUE;
	}
	lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);
	if (lResult != ERROR_SUCCESS) 
	{
		if (lResult == ERROR_FILE_NOT_FOUND)
		{
			return TRUE;
		} 
		else 
		{	
			m_iErrCode = GetLastError();
			return FALSE;
		}
	}
	lpEnd = lpSubKey + lstrlen(lpSubKey);
	if (*(lpEnd - 1) != '\\') 
	{
		*lpEnd = '\\';
		lpEnd++;
		*lpEnd = '\0';
	}
	dwSize = MAX_PATH;
	lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);
	if (lResult == ERROR_SUCCESS) 
	{
		do {
			lstrcpy (lpEnd, szName);
			if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) 
			{
				break;
			}
			dwSize = MAX_PATH;
			lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,NULL, NULL, &ftWrite);
		} while (lResult == ERROR_SUCCESS);
	}

	lpEnd--;
	*lpEnd = TEXT('\0');
	RegCloseKey (hKey);
	lResult = RegDeleteKey(hKeyRoot, lpSubKey);

	if (lResult == ERROR_SUCCESS) 
		return TRUE;

	return FALSE;
}

BOOL CRegOpt::RegDelValue (HKEY hKeyRoot,LPCTSTR lpSubKey,LPCTSTR lpValue)
{	

	HKEY hKey = NULL;
	DWORD dwRet;
	BOOL bRet = FALSE;

	if ((dwRet=RegOpenKeyEx(hKeyRoot,lpSubKey,0,KEY_ALL_ACCESS,&hKey))==ERROR_SUCCESS)
	{
		DWORD cbdata;
		if ((dwRet=RegQueryValueEx(hKey,lpValue,NULL,NULL,NULL,&cbdata))==ERROR_SUCCESS)
		{
			if (RegDeleteValue(hKey,lpValue)==ERROR_SUCCESS)
			{
				bRet = TRUE;
				goto clean0;
			}	
		}
		else
		{
			m_iErrCode = dwRet;
			bRet = FALSE;
			goto clean0;
		}
	}
	else
	{
		m_iErrCode = dwRet;
		bRet = FALSE;
	}

clean0:
	if (hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}
	return TRUE;
}	

/************************************************************************/
//是否存键
/************************************************************************/
BOOL CRegOpt::FindKey(HKEY hKeyRoot,LPCTSTR lpSubKey)
{
	HKEY hKey = NULL;
	DWORD dwRet;
	BOOL bRet = FALSE;
	if ((dwRet=RegOpenKeyEx(hKeyRoot,lpSubKey,0,KEY_ALL_ACCESS,&hKey))==ERROR_SUCCESS)
	{	
		bRet = TRUE;
		goto clean0;
	}
	if (dwRet == ERROR_FILE_NOT_FOUND ) //找不到指定文件
	{
		bRet = FALSE;
		dwRet = GetLastError();
		goto clean0;
	}

clean0:
	m_iErrCode = dwRet;
	return TRUE;
}


/************************************************************************/
//获得错误代码
/************************************************************************/

DWORD CRegOpt::GetErrorCode()
{
	return m_iErrCode;
}

BOOL CRegOpt::CrackRegKey(wchar_t* regstring,HKEY& root, wchar_t*& subkey,wchar_t*& value)
{	
	if (!(subkey = wcsstr(regstring,L"\\")))
	{	
		return FALSE; //No valid root key
	}
	

	*subkey = 0;
	++subkey;

	if (!lstrcmpi(regstring,L"HKEY_CURRENT_USER"))root = HKEY_CURRENT_USER;
	else
		if (!lstrcmpi(regstring,L"HKEY_LOCAL_MACHINE"))root = HKEY_LOCAL_MACHINE;
		else
			if (!lstrcmpi(regstring,L"HKEY_USERS")) root = HKEY_USERS;
			else
				if (!lstrcmpi(regstring,L"HKEY_CLASSES_ROOT")) root = HKEY_CLASSES_ROOT;
				else  //No valid root key
				{	
					m_iErrCode = ERROR_NOFAILKEY;
					return FALSE;
				}
				//value = wcsstr(subkey,L"\\");
				//if (value)
				//{
				//	//value can point to real value or to '\0', meaning empty std::wstring
				//	*value = 0;
				//	++value;
				//}
				////value points to NULL

				return TRUE;
}

BOOL CRegOpt::CrackRegKey(CString& strRegPath,HKEY& root,CString& strSubKey)

{	
	int nInx = strRegPath.Find(_T("\\"));

	if (nInx == -1)
		return FALSE;

	strSubKey = strRegPath.Mid(nInx+1);
	
	CString strRootKey = strRegPath.Mid(0,nInx);
	
	if (-1!=strRootKey.Find(L"HKEY_CURRENT_USER"))
		root = HKEY_CURRENT_USER;
	else if(-1!=strRootKey.Find(L"HKEY_LOCAL_MACHINE"))
		root = HKEY_LOCAL_MACHINE;
	else if(-1!=strRootKey.Find(L"HKEY_USERS"))
		root = HKEY_USERS;
	else if(-1!=strRootKey.Find(L"HKEY_CLASSES_ROOT"))
		root = HKEY_CLASSES_ROOT;
	else 
	{
		m_iErrCode = ERROR_NOFAILKEY;
		return FALSE;
	}
	return TRUE;
}

//获得指定键的默认值
BOOL CRegOpt::GetDefValue(HKEY hKeyRoot,LPCTSTR lpSubKey,CString& strValueName,CString& strValue)
{
	HKEY hKey;
	DWORD	dwRet;
	BOOL bRet = FALSE;

	if ((dwRet=RegOpenKeyEx(hKeyRoot,lpSubKey,0,KEY_ALL_ACCESS,&hKey))!=ERROR_SUCCESS)
	{	
		m_iErrCode = dwRet;
		bRet = FALSE;
		goto clean0;
	}

	DWORD	cbData =MAX_PATH;
	LPBYTE	lpValue = new BYTE[MAX_PATH];
	memset(lpValue,0,MAX_PATH);
	DWORD	dwType;

	dwRet = RegQueryValueEx(hKey,
		strValueName.GetBuffer(),
		NULL,
		&dwType,
		(LPBYTE) lpValue,
		&cbData );
	
	if ( dwRet== ERROR_MORE_DATA )
	{
		lpValue = new BYTE[cbData+2];
		
		if (lpValue ==NULL)
		{	
			m_iErrCode = ERROR_NOMEMOEY;
			bRet = FALSE;
			goto clean0;
		}

		dwRet = RegQueryValueEx(hKey,
			strValueName.GetBuffer(),
			NULL,
			NULL,
			(LPBYTE) lpValue,
			&cbData );
 	}

	if (dwRet != ERROR_SUCCESS)
	{
		m_iErrCode  = ERROR_NODEF;
		bRet = FALSE;
		goto clean0;
	}
	
	if (REG_SZ == dwType||
		REG_EXPAND_SZ== dwType)
	{
		strValue = (TCHAR*)lpValue;
	}
	else
		strValue = _T("");
	bRet = TRUE;

clean0:
	RegCloseKey(hKey);
	return bRet;

}

BOOL CRegOpt::GetHKEYToString(HKEY hRootKey,LPCTSTR lpszSubKey,CString& strRegFullPath)
{	

	strRegFullPath = _T("");
	if (hRootKey == HKEY_CURRENT_USER)
	{
		strRegFullPath.Append(L"HKEY_CURRENT_USER\\");
	}
	else if (hRootKey == HKEY_LOCAL_MACHINE)
	{
		strRegFullPath.Append(L"HKEY_LOCAL_MACHINE\\");
	}
	else if (hRootKey == HKEY_USERS)
	{
		strRegFullPath.Append(L"HKEY_USERS\\");
	}
	else if (hRootKey == HKEY_CLASSES_ROOT)
	{
		strRegFullPath.Append(L"HKEY_CLASSES_ROOT\\");
	}
	else
	{
		return FALSE;
	}

	strRegFullPath.Append(lpszSubKey);
	return TRUE;
}
