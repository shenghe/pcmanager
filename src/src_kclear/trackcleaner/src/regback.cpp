#include "regback.h"
#include <time.h>

BOOL CRegBack::RestRegister(LPCTSTR lpcszRegFilePath)
{	
	
	//判断备份目录
	CString strDirectoryPath = m_strAppPath;
	strDirectoryPath.Append(_T("\\AppData\\kclear_reg_bak"));

	DWORD   dwAttr = GetFileAttributes( strDirectoryPath ); 
	if(dwAttr==-1||( dwAttr &FILE_ATTRIBUTE_DIRECTORY )==0)//目录不存在 
	{ 
		if ( !CreateDirectory(strDirectoryPath, NULL)) 
		{ 
			return FALSE; 
		} 
	} 
	
	//还原文件路径
	CString strRegFilePath = lpcszRegFilePath;
	
	//将这次恢复的文件拷贝到备份目录
	if(lpcszRegFilePath == NULL)
	{	
		//Appdata路径
		CString strAppDataPath;
		strAppDataPath.Format(_T("%s\\AppData\\*.reg"),m_strAppPath);
	
		WIN32_FIND_DATA fd;
		HANDLE hFindFile = FindFirstFile(strAppDataPath, &fd);
		if(hFindFile == INVALID_HANDLE_VALUE)
		{	
			::FindClose(hFindFile);
			return FALSE;
		}
		
		strRegFilePath.Append(m_strAppPath);
		strRegFilePath.Append(_T("\\AppData\\"));
		strRegFilePath.Append(fd.cFileName);

	}
	
	CString strRegFileName;
	//获得文件名称
	int iLen = strRegFilePath.ReverseFind('\\');
	if (iLen<=0)
		return FALSE;

	strRegFileName = strRegFilePath.Mid(iLen+1);
	
	//备份目的路径名
	CString strBakPath;
	strBakPath.Format(_T("%s\\AppData\\kclear_reg_bak\\%s"),m_strAppPath,strRegFileName);
	
	if (FALSE==CopyFile(strRegFilePath,strBakPath,TRUE))
	{
		return FALSE;
	}
	
	
	CString strItem(strRegFilePath);
	CString strParameters;

	strParameters = _T("/s \"") + strItem + _T("\"");
	HINSTANCE hErrCode=ShellExecute(NULL,_T("open"),_T("regedit.exe"),
		strParameters,NULL,SW_HIDE);

	if ((INT_PTR)hErrCode<=32)
		return FALSE;

	if (FALSE==DeleteFile(strRegFilePath))
	{
		return FALSE;
	}	

	return TRUE;
}

BOOL CRegBack::AppendValueToString(HKEY Hkey,LPCTSTR lpcszSubKey)
{	
	RegisterEnum(Hkey,lpcszSubKey);

	return TRUE;
}


void CRegBack::RegisterEnum(HKEY hRoot,LPCTSTR lpcszSubKey)
{

	HKEY hKey;
	LONG lResult;

	lResult = RegOpenKeyEx(hRoot,
		lpcszSubKey,
		NULL,
		KEY_READ,
		&hKey
		);

	if(lResult != ERROR_SUCCESS)
		return ;

	/************************************************************************/
	/*枚举值
	/************************************************************************/
	BEGIN
	DWORD dwIndex=0;
	do
	{	

		LPTSTR lpName = new TCHAR[MAX_PATH];
		LPBYTE  lpValue = new BYTE[MAX_PATH];
		if (lpName == NULL||lpValue ==NULL)
			return ;


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
				return ;
		

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
				RegCloseKey(hKey);
				return ;
			}
		}
		
		AppendValueToString(hRoot,lpcszSubKey,lpName);

		delete[] lpName;
		delete[] lpValue;
		lpName = NULL;
		lpValue = NULL;

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
				RegCloseKey(hKey);
				return ;
			}
			else
			{
				RegCloseKey(hKey);
				return ;
			}
		}


		CString strRoot;
		if (wcscmp(lpcszSubKey,_T("")) == 0 )
		{	
			strRoot.Format(_T("%s"),szKey);;
		}
		else
		{	
			strRoot.Format(_T("%s\\%s"),lpcszSubKey,szKey);
		}


		RegisterEnum(hRoot, strRoot.GetBuffer());


	} while (1);
	END
		RegCloseKey(hKey);
	return ;
}

BOOL CRegBack::RegisterExport(LPCTSTR lpcszPath)
{	
	if (m_strRegString == _T("") )
	{
		return TRUE;
	}
	

	//备份注册表
	SYSTEMTIME st;	
	GetLocalTime(&st);
	
	
	//Appdata路径
	CString strAppDataPath;
	strAppDataPath.Format(_T("%s\\AppData"),m_strAppPath);
	
	//备份文件名
	CString strRegName;
	strRegName.Format(_T("%s\\AppData\\kclear_reg_%04d%02d%02d%02d%02d%02d.reg.bak"),
		m_strAppPath,
		st.wYear,st.wMonth,st.wDay,
		st.wHour,st.wSecond,st.wMinute
		);
	//实际文件名
	CString _strRegName;
	_strRegName.Format(_T("%s\\AppData\\kclear_reg_%04d%02d%02d%02d%02d%02d.reg"),
		m_strAppPath,
		st.wYear,st.wMonth,st.wDay,
		st.wHour,st.wSecond,st.wMinute
		);
	
	_tsetlocale(LC_CTYPE, _T("chs")); 

	FILE* fp = _tfopen(strRegName, _T("a,ccs=UNICODE"));
	if(fp == NULL)
	{
		MessageBox(NULL, _T("Error while creating the file"), _T("Registry export"), MB_OK);
		return FALSE;
	}
	//TCHAR szBuf[1];
	//szBuf[0] = 0XFFFE;

	fwprintf(fp,_T("%s\n"),_T("Windows Registry Editor Version 5.00"));
	fwprintf(fp,_T("%s"),m_strRegString);
	

	m_strRegString = _T("");
	fclose(fp);

	//删除以前无用文件然后将新文件改名为*.reg
	CString strfile = strAppDataPath;
	strfile.Append(_T("\\*.reg"));
	WIN32_FIND_DATA fd;
	HANDLE hFindFile = FindFirstFile(strfile, &fd);
	if(hFindFile == INVALID_HANDLE_VALUE)
	{	
		::FindClose(hFindFile); 
	}
	else
	{
		strAppDataPath.Append(_T("\\"));
		strAppDataPath.Append(fd.cFileName);

		if (FALSE==DeleteFile(strAppDataPath))
		{	

		}

	}

	if(FALSE==MoveFile(strRegName,_strRegName))
		return FALSE;

	return TRUE;
}

BOOL CRegBack::AppendValueToString(HKEY Hkey,LPCTSTR lpcszSubKey,LPCTSTR lpcszValueName)
{	
	
	if (wcscmp(lpcszValueName,_T(""))==0||lpcszValueName==NULL)
	{	
		//获得全路径
		CString strRegFullPath;
		g_regOpt.GetHKEYToString(Hkey,lpcszSubKey,strRegFullPath);
		CString _strRegFullPath;
		_strRegFullPath.Format(_T("\n[%s]\n"),strRegFullPath);

		m_strRegString.Append(strRegFullPath);
	}

	//获取值
	HKEY	hKey;
	DWORD	dwRet;

	if ((dwRet=RegOpenKeyEx(Hkey,lpcszSubKey,0,KEY_ALL_ACCESS,&hKey))!=ERROR_SUCCESS)
	{	
		return FALSE;
	}

	DWORD	cbData = MAX_PATH;
	LPBYTE	lpValue = new BYTE[MAX_PATH];
	memset(lpValue,0,MAX_PATH);
	DWORD	dwType;

	dwRet = RegQueryValueEx(hKey,
		lpcszValueName,
		NULL,
		&dwType,
		(LPBYTE) lpValue,
		&cbData );

	if ( dwRet== ERROR_MORE_DATA )
	{
		lpValue = new BYTE[cbData+2];

		if (lpValue ==NULL)
		{	
			return FALSE;
		}

		dwRet = RegQueryValueEx(hKey,
			lpcszValueName,
			NULL,
			NULL,
			(LPBYTE) lpValue,
			&cbData );
	}

	if (dwRet != ERROR_SUCCESS)
	{
		return FALSE;
	}

	//格式化值
	CString strValueData;
	FormatDataWithDataType(dwType,lpValue,cbData,strValueData);	

	//获得全路径
	CString strRegFullPath;
	g_regOpt.GetHKEYToString(Hkey,lpcszSubKey,strRegFullPath);
	
	CString _strRegFullPath;
	_strRegFullPath.Format(_T("\n[%s]\n"),strRegFullPath);

	CString strValue; //Reg文件的值,如果修改默认值则为 @ ="内容",否则 "值名" = 值
	if( 0== wcscmp(lpcszValueName,_T("")))
	{
		strValue.Format(_T("@ = "));
	}
	else
	{
		strValue.Format(_T("\"%s\" = "), lpcszValueName);
		strValue.Replace(_T("\\"),_T("\\\\"));
	}
	strValue.Append(strValueData);

	m_strRegString.Append(_strRegFullPath);
	m_strRegString.Append(strValue);

	return TRUE;
}

void CRegBack::FormatDataWithDataType(DWORD dwKeyType, LPBYTE pbbinKeyData, DWORD dwKeyDataLength, CString &cstrOutput)	
{
	CString cstrTemp, cstrTemp1 ,cstrTemp2;
	int nIndex = 0;
	switch(dwKeyType)
	{
	case REG_SZ:
		{
			cstrTemp.Format(_T("\"%s\"\n"), pbbinKeyData);
			for(int i=0;i<cstrTemp.GetLength();i++)
			{
				cstrTemp1 = cstrTemp.Mid(i,1); 
				if(cstrTemp.Mid(i,1) == _T("\\"))
				{
					cstrTemp1 = cstrTemp.Left(i); 
					cstrTemp2 = cstrTemp.Right(cstrTemp.GetLength()-i-1);
					cstrTemp = cstrTemp1+_T("\\\\")+cstrTemp2;
					i++;
				}
			}
			cstrOutput = cstrTemp;
			break;
		}

	case REG_DWORD: /// same is for REG_DWORD_LITTLE_ENDIAN
		{
			DWORD dwValue;
			memcpy(&dwValue, pbbinKeyData, sizeof DWORD);
			cstrTemp.Format(_T("dword:%08x\n"), dwValue);
			cstrOutput = cstrTemp;
			break;
		}

	case REG_BINARY:
	case REG_MULTI_SZ:
	case REG_EXPAND_SZ:
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_LIST:
	case REG_RESOURCE_REQUIREMENTS_LIST:
		{
			if(dwKeyType != REG_BINARY)
				cstrOutput.Format(_T("hex(%d):"), dwKeyType);
			else
				cstrOutput.Format(_T("hex:"));

			for(DWORD dwIndex = 0; dwIndex < dwKeyDataLength; dwIndex++)
			{
				cstrTemp1.Format(_T("%02x"), pbbinKeyData[dwIndex]);
				if(dwIndex != 0 && (dwIndex % 0x15 == 0))
				{
					cstrTemp += _T(",\\\n");
					cstrTemp += cstrTemp1;
				}
				else
				{
					if( cstrTemp.IsEmpty() )
						cstrTemp = cstrTemp1;
					else
						cstrTemp += ","+cstrTemp1;
				}
			}

			cstrTemp += _T("\n");
			cstrOutput += cstrTemp;
			break;
		}

	case REG_NONE:
	case REG_DWORD_BIG_ENDIAN:
	case REG_LINK:
		//// TODO : add code for these types...
		break;

	}
}
