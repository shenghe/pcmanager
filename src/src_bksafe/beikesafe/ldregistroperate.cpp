#include "StdAfx.h"
#include "ldregistroperate.h"

using namespace RegisterOperate;


#define STR_BEGIN
#define STR_CLSID_GETFILE_REGPATH TEXT("CLSID\\%s\\InprocServer32")
#define STR_END


CRegisterOperate::CRegisterOperate(void)
{
}

CRegisterOperate::~CRegisterOperate(void)
{
}

int CRegisterOperate::GetRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, DWORD& dwValue)
{
	int nRet = -1;
	HKEY hOpenKey = NULL;
	hOpenKey = _OpenKey(hKey, pszSubPath);
	if (NULL != hOpenKey)
	{
		DWORD dwType = REG_DWORD;
		DWORD dwSize = sizeof(DWORD);
		if (ERROR_SUCCESS == RegQueryValueEx(hOpenKey, pszName, 0,&dwType , (LPBYTE)&dwValue, &dwSize))
			nRet = 0;
	}
	_CloseKey(hOpenKey);
	
	return nRet;
}

int CRegisterOperate::GetRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, LPTSTR pszValue, DWORD cbSize)
{
	int nRet = -1;
	HKEY hOpenKey = NULL;
	hOpenKey = _OpenKey(hKey, pszSubPath);
	
	if (NULL != hOpenKey)
	{

		DWORD dwType = REG_SZ;
		DWORD dwSize = 0;
//		SHGetValue(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Internet Explorer\\Main"), TEXT("Start Page"), &dwType, (LPVOID)pszValue, &cbSize);
		if (ERROR_SUCCESS != RegQueryValueEx(hOpenKey, pszName, 0, &dwType , (LPBYTE)pszValue, &dwSize))
		{
			if (REG_MULTI_SZ == dwType)
			{//多字节解析
				if (dwSize > cbSize)
				{
					TCHAR* pszBuf = new TCHAR[dwSize+1];
					ZeroMemory(pszBuf, dwSize+1);
					_GetValue_Mul_SZ(hOpenKey, pszName, pszBuf, dwType, dwSize);
					SAFE_DELETE_PTR(pszBuf);
				}
				else
				{
					_GetValue_Mul_SZ(hOpenKey, pszName, pszValue, dwType, cbSize);
				}
			}
			else
			{//解析非多字节字符
				_GetValue(hOpenKey, pszName, pszValue, dwType, cbSize, dwSize);
			}
			nRet = 0;
		}
	}
	_CloseKey(hOpenKey);

	return nRet;
}
HKEY CRegisterOperate::_OpenKey(HKEY hKey, LPCTSTR pszSubPath)
{
	HKEY hRetkey = NULL;
	if (ERROR_SUCCESS == RegOpenKeyEx(hKey, pszSubPath, 0, KEY_READ|KEY_WRITE, &hRetkey))
	{
		return hRetkey;
	}

	int nError = GetLastError();
	return hRetkey;
}

void CRegisterOperate::_CloseKey(HKEY hKey)
{
	if (NULL != hKey)
		RegCloseKey(hKey);
	hKey = NULL;
}

int CRegisterOperate::_GetValue(HKEY hKey, LPCTSTR pszName, LPTSTR pszValue, DWORD dwType, DWORD cbSize, DWORD cbSize2)
{
	int nRet = -1;
	if (cbSize <= cbSize2)
	{
		TCHAR* pszBuf = new TCHAR[cbSize2+1];
		ZeroMemory(pszBuf, cbSize2+1);
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, pszName, 0, &dwType, (LPBYTE)pszBuf, &cbSize2))
		{
			StringCbCopyN(pszValue, cbSize, pszBuf, cbSize-1);
			nRet = 0;
		}
		SAFE_DELETE_PTR(pszBuf);
	}
	else
	{
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, pszName, 0, &dwType, (LPBYTE)pszValue, &cbSize))
		{
			nRet = 0;
		}
	}

	return nRet;
}

int CRegisterOperate::_GetValue_Mul_SZ(HKEY hKey, LPCTSTR pszName, LPTSTR pszValue, DWORD dwType, DWORD cbSize)
{
	int nRet = -1;
	CAtlArray<CString> arrItemStr;
	DWORD dwSize = cbSize;
	if (ERROR_SUCCESS == RegQueryValueEx(hKey, pszName, 0, &dwType, (LPBYTE)pszValue, &dwSize))
	{
		_ParseMulSzBuf(pszValue, arrItemStr);
		ZeroMemory(pszValue, cbSize);
		nRet = 0;
	}
	
	if (0 == nRet)
	{//这里解析多字节字符成功，那么把解析成功的字符合并成字符，中间用空格隔开
		size_t tCount = arrItemStr.GetCount();
		CString strInfo;
		for (size_t i = 0; i < tCount; i++)
		{
			strInfo += arrItemStr[i];
			strInfo += TEXT("     ");
		}
		StringCbCopyN(pszValue, cbSize, strInfo, cbSize);
	}
	return nRet;
}

int CRegisterOperate::_ParseMulSzBuf(LPCTSTR pszBuf, CAtlArray<CString>& arrStr)
{
	int nRet = -1;
	arrStr.RemoveAll();
	if (NULL == pszBuf)
		return nRet;

	while (0 != _tcslen(pszBuf))
	{
		TCHAR pszValue[MAX_PATH] = {0};
		size_t nValue = (_tcslen(pszBuf)+1)*sizeof(TCHAR);
		StringCbCopyN(pszValue, sizeof(pszValue), pszBuf, nValue);
		nValue /= sizeof(TCHAR);
		pszBuf += nValue;
		arrStr.Add(CString(pszValue));
	}
	nRet = 0;

	return nRet;
}

int CRegisterOperate::GetRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, CAtlArray<CString>& arrString)
{
	int nRet = -1;
	HKEY hOpenKey = NULL;
	hOpenKey = _OpenKey(hKey, pszSubPath);
	DWORD dwSize = 0;
	DWORD dwType = REG_MULTI_SZ;
	if (NULL != hOpenKey)
	{
		//先通过调用它来获得需要的缓冲区的长度
		RegQueryValueEx(hOpenKey, pszName, 0,&dwType , NULL, &dwSize);
		if (dwSize > 0)
		{
			TCHAR* pszValue = new TCHAR[dwSize+1];
			ZeroMemory(pszValue, dwSize+1);
			if (ERROR_SUCCESS == RegQueryValueEx(hOpenKey, pszName, 0, &dwType, (LPBYTE)pszValue, &dwSize))
			{
				nRet = _ParseMulSzBuf(pszValue, arrString);
			}
			SAFE_DELETE_PTR(pszValue);
		}
	}
	_CloseKey(hOpenKey);
	return nRet;
}

DWORD CRegisterOperate::GetRegType(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName)
{
	DWORD dwType = 0;
	HKEY hOpenKey = NULL;
	hOpenKey = _OpenKey(hKey, pszSubPath);
	if (NULL != hOpenKey)
	{
		DWORD dwSize = 0;
		if (ERROR_SUCCESS != RegQueryValueEx(hOpenKey, pszName, 0,&dwType , NULL, &dwSize))
		{
			return dwType;
		}
	}
	_CloseKey(hOpenKey);
	return dwType;
}

int CRegisterOperate::EnumSubKey(HKEY hKey, LPCTSTR pszSubPath, CAtlArray<CString>& arrKey)
{
	int nRet = -1;
	if (NULL == pszSubPath)
		return nRet;
	HKEY hOpenKey = NULL;
	arrKey.RemoveAll();
	hOpenKey = _OpenKey(hKey, pszSubPath);
	if (NULL == hOpenKey)
		return nRet;
	DWORD dwIndex = 0;
	TCHAR pszName[FILEPATH_MAX_LEN] = {0};
 	DWORD dwRetSize = 0;
	do 
	{
		ZEROMEM(pszName);
		dwRetSize = sizeof(pszName);
		if (ERROR_SUCCESS != RegEnumKeyEx(hOpenKey, dwIndex, pszName, &dwRetSize, 0, NULL, 0, 0))
			dwRetSize = 0;

		if (dwRetSize > 0)
		{
			arrKey.Add(CString(pszName));
			++dwIndex;
		}
	} while(dwRetSize);

 	nRet = 0;
	_CloseKey(hOpenKey);
	return nRet;
}
int CRegisterOperate::EnumCLSIDAndFile(HKEY hKEY, LPCTSTR pszSubPath, CAtlArray<CLSID_STRUCT>& arrClsid)
{
	int nRet = -1;
	CAtlArray<CString> arrSubKey;

	//这里枚举出所有的CLSID
	if (EnumSubKey(hKEY, pszSubPath, arrSubKey) != 0)
		return nRet;

	size_t tCount = arrSubKey.GetCount();
	CLSID_STRUCT clsidStruct;
	TCHAR _pszFullSubPath[REGPATH_MAX_LEN] = {0};
	for (size_t t = 0; t < tCount; t++)
	{
		ZEROMEM(&clsidStruct);
		clsidStruct.cbSize = sizeof(clsidStruct);
		STRPRINF_STR(clsidStruct._pszCLSID, arrSubKey[t]);//获取CLSID
		StringCbPrintf(clsidStruct._pszFullRegPath, sizeof(clsidStruct._pszFullRegPath),
					   TEXT("%s\\%s"), pszSubPath, clsidStruct._pszCLSID);//拼接出全注册表路径
		//拼接文件路径所在的注册表路径，HKEY_CLASSES_ROOT\CLSID\{0000002F-0000-0000-C000-000000000046}\InprocServer32
		STRPRINF(clsidStruct._pszFileRegPath, STR_CLSID_GETFILE_REGPATH, clsidStruct._pszCLSID);
		GetRegValue(HKEY_CLASSES_ROOT, clsidStruct._pszFileRegPath, TEXT(""), clsidStruct._pszFileFullPath, sizeof(clsidStruct._pszFileFullPath));//获取文件路径
		//--增加到列表中
		arrClsid.Add(clsidStruct);
	}
	nRet = 0;

	return nRet;
}

int CRegisterOperate::SetRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, LPCTSTR pszValue, DWORD cbSize)
{
	int nRet = -1;
	if (NULL == hKey || NULL == pszSubPath || NULL == pszName || NULL == pszValue)
		return nRet;

	HKEY hOpenKey = NULL;
	hOpenKey = _OpenKey(hKey, pszSubPath);
	if (NULL == hOpenKey)
		return nRet;


	if (ERROR_SUCCESS == RegSetValueEx(hOpenKey, pszName, 0, REG_SZ, (LPBYTE)pszValue, cbSize))
	{
		nRet = 0;
	}
	_CloseKey(hOpenKey);
	return nRet;
}

int CRegisterOperate::SetRegValue2(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, LPCTSTR pszValue, DWORD cbSize)
{
	int nRet = -1;
	if (NULL == hKey || NULL == pszSubPath || NULL == pszName || NULL == pszValue)
		return nRet;

	HKEY hOpenKey = NULL;
	hOpenKey = _OpenKey(hKey, pszSubPath);
	if (NULL == hOpenKey)
		return nRet;


	if (ERROR_SUCCESS == RegSetValueEx(hOpenKey, pszName, 0, REG_EXPAND_SZ, (LPBYTE)pszValue, cbSize))
	{
		nRet = 0;
	}
	_CloseKey(hOpenKey);
	return nRet;
}

int CRegisterOperate::SetRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, DWORD dwValue)
{
	int nRet = -1;
	if (NULL == hKey || NULL == pszSubPath || NULL == pszName)
		return nRet;

	HKEY hOpenKey = NULL;
	hOpenKey = _OpenKey(hKey, pszSubPath);
	if (NULL == hOpenKey)
		return nRet;


	if (ERROR_SUCCESS == RegSetValueEx(hOpenKey, pszName, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD)))
	{
		nRet = 0;
	}
	_CloseKey(hOpenKey);
	return nRet;
}

int CRegisterOperate::DeleteRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName)
{
	int nRet = -1;
	if (NULL == hKey || NULL == pszSubPath || NULL == pszName)
		return nRet;

	HKEY hOpenKey = NULL;
	hOpenKey = _OpenKey(hKey, pszSubPath);
	if (NULL == hOpenKey)
		return nRet;


	if (ERROR_SUCCESS == RegDeleteValue(hOpenKey, pszName))
	{
		nRet = 0;
	}
	_CloseKey(hOpenKey);
	return nRet;
}

int CRegisterOperate::DeleteAnKey(HKEY hKey, LPCTSTR pszSubPath)
{
	int nRet = -1;
	if (NULL == hKey || NULL == pszSubPath)
		return nRet;
	HKEY hOpenKey = NULL;
	hOpenKey = _OpenKey(hKey, NULL);
	if (NULL == hOpenKey)
		return nRet;

	
#if 0
	//Requires Windows Vista or Windows XP Professional x64 Edition.
	if (ERROR_SUCCESS == RegDeleteKeyEx(hOpenKey, pszSubPath,KEY_WOW64_32KEY|KEY_WOW64_64KEY, 0))
		nRet;
#else
	if (ERROR_SUCCESS == SHDeleteKey(hKey, pszSubPath))
		nRet = 0;
	_CloseKey(hOpenKey);
#endif
	return nRet ;
}

int CRegisterOperate::KeyIsExist(HKEY hKey, LPCTSTR pszSubPath)
{
	int nRet = -1;
	HKEY hOpenKey = _OpenKey(hKey, pszSubPath);
	if (NULL != hOpenKey)
		nRet = 0;

	_CloseKey(hOpenKey);
	return nRet;
}

int CRegisterOperate::EnumRegValue(HKEY hKey, LPCTSTR pszSubPath, CAtlArray<CString>& arrRegValue)
{
	int nRet = -1;
	if (NULL == hKey || NULL == pszSubPath)
		return nRet;
	HKEY hOpenKey = _OpenKey(hKey, pszSubPath);
	if (NULL == hOpenKey)
		return nRet;
	arrRegValue.RemoveAll();
	int nIndex = 0;
	TCHAR pszRegValue[REGPATH_MAX_LEN] = {0};
	DWORD dwSize = sizeof(pszRegValue);
	DWORD dwType = REG_SZ;
	while(ERROR_SUCCESS == RegEnumValue(hOpenKey, nIndex, pszRegValue, &dwSize, 0, &dwType ,NULL, NULL))
	{
		arrRegValue.Add(pszRegValue);
		++nIndex;
		dwSize = sizeof(pszRegValue);
	}
	nRet = 0;
	return nRet;
}

int CRegisterOperate::GetBinaryRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, LPBYTE pByteValue, DWORD cbSize)
{
	int nRet = -1;
	if (NULL == hKey || NULL == pszSubPath)
		return nRet;

	HKEY hOpenKey = _OpenKey(hKey, pszSubPath);
	if (NULL == hOpenKey)
		return nRet;

	DWORD dwType = REG_BINARY;
	if (ERROR_SUCCESS == RegQueryValueEx(hOpenKey, pszName, 0, &dwType, pByteValue, &cbSize))
	{
		nRet = 0;
	}

	_CloseKey(hOpenKey);
	return nRet;
}

int CRegisterOperate::SetBinaryRegValue(HKEY hKey, LPCTSTR pszSubPath, LPCTSTR pszName, LPCBYTE pByteValue, DWORD cbSize)
{
	int nRet = -1;

	if (NULL == hKey || NULL == pszSubPath)
		return nRet;

	HKEY hOpenKey = _OpenKey(hKey, pszSubPath);
	if (NULL == hOpenKey)
		return nRet;

	if (ERROR_SUCCESS == RegSetValueEx(hOpenKey, pszName, 0, REG_BINARY, pByteValue, cbSize))
	{
		nRet = 0;
	}

	_CloseKey(hOpenKey);
	return nRet;
}

int CRegisterOperate::CreateAnKey(HKEY hKey, LPCTSTR pszSubPath, HKEY &hNewKey)
{
	int nRet = -1;

	if (NULL == hKey || NULL == pszSubPath)
		return nRet;

	if (ERROR_SUCCESS == RegCreateKeyEx(hKey,pszSubPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE,NULL, &hNewKey,NULL))
		nRet = 0;

	return nRet;

}