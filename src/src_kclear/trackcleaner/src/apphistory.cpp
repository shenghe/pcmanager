#include "stdafx.h"
#include "apphistory.h"

typedef struct tagCSIDL_NAME
{
	LPCTSTR pszCsidlName;
	int     nFolder;
}CSIDL_NAME, *LPCSIDL_NAME;

#ifdef _UNICODE
#define CSIDL_NAME_ENTRY(ID) \
{ L#ID , ID }
#else
#define CSIDL_NAME_ENTRY(ID) \
{ #ID , ID }
#endif

CSIDL_NAME g_csidlnamelist[] = 
{
	CSIDL_NAME_ENTRY(CSIDL_DESKTOP),
	CSIDL_NAME_ENTRY(CSIDL_INTERNET),
	CSIDL_NAME_ENTRY(CSIDL_PROGRAMS),
	CSIDL_NAME_ENTRY(CSIDL_CONTROLS),
	CSIDL_NAME_ENTRY(CSIDL_PRINTERS),
	CSIDL_NAME_ENTRY(CSIDL_PERSONAL),
	CSIDL_NAME_ENTRY(CSIDL_FAVORITES),
	CSIDL_NAME_ENTRY(CSIDL_STARTUP),
	CSIDL_NAME_ENTRY(CSIDL_RECENT),
	CSIDL_NAME_ENTRY(CSIDL_SENDTO),
	CSIDL_NAME_ENTRY(CSIDL_BITBUCKET),
	CSIDL_NAME_ENTRY(CSIDL_STARTMENU),
	CSIDL_NAME_ENTRY(CSIDL_DESKTOPDIRECTORY),
	CSIDL_NAME_ENTRY(CSIDL_DRIVES),
	CSIDL_NAME_ENTRY(CSIDL_NETWORK),
	CSIDL_NAME_ENTRY(CSIDL_NETHOOD),
	CSIDL_NAME_ENTRY(CSIDL_FONTS),
	CSIDL_NAME_ENTRY(CSIDL_TEMPLATES),
	CSIDL_NAME_ENTRY(CSIDL_COMMON_STARTMENU),
	CSIDL_NAME_ENTRY(CSIDL_COMMON_PROGRAMS),
	CSIDL_NAME_ENTRY(CSIDL_COMMON_STARTUP),
	CSIDL_NAME_ENTRY(CSIDL_COMMON_DESKTOPDIRECTORY),
	CSIDL_NAME_ENTRY(CSIDL_APPDATA),
	CSIDL_NAME_ENTRY(CSIDL_PRINTHOOD),
	CSIDL_NAME_ENTRY(CSIDL_ALTSTARTUP),
	CSIDL_NAME_ENTRY(CSIDL_COMMON_ALTSTARTUP),
	CSIDL_NAME_ENTRY(CSIDL_COMMON_FAVORITES),
	CSIDL_NAME_ENTRY(CSIDL_INTERNET_CACHE),
	CSIDL_NAME_ENTRY(CSIDL_COOKIES),
	CSIDL_NAME_ENTRY(CSIDL_HISTORY),
	CSIDL_NAME_ENTRY(CSIDL_LOCAL_APPDATA),
	CSIDL_NAME_ENTRY(CSIDL_COMMON_APPDATA)
};

CAppHistory::CAppHistory()
{
	m_bScan = TRUE; 
}

CAppHistory::~CAppHistory()
{

}

BOOL CAppHistory::CommfunReg(int iType,LPCTSTR lpcszRegPath,CSimpleArray<CString>& vec_valueName)
{	
	
	CString strRegPath = lpcszRegPath;
	try
	{
		USERDATA userData;
		userData.pObject= this;
		userData.iType = iType;

		HKEY	hRootKey;
		CString strSubKey;
		m_regOpt.CrackRegKey(strRegPath,hRootKey,strSubKey);
	
		//处理指定键值的情况
		if (vec_valueName.GetSize()!=0)
		{	
			for (int i=0; i<vec_valueName.GetSize();i++)
			{	

				//先确定键是否存在
				CString strValueName=vec_valueName[i];
				CString strValueData;
				if (TRUE ==	m_regOpt.GetDefValue(hRootKey,strSubKey,strValueName,strValueData)	)
				{
					g_fnScanReg(g_pMain,iType,
						lpcszRegPath,
						vec_valueName[i],
						_T("")
						);
				}
			
			}

			return TRUE;

		}
		
		//处理路径情况
		m_regOpt.DoRegEnumeration(hRootKey,strSubKey.GetBuffer(),myEnumerateReg,&userData);
		strSubKey.ReleaseBuffer();

	}
	catch (...)
	{	
		OutputDebugString(_T("崩溃:CommfunReg"));
		return FALSE;
	}
		
	return TRUE;
}

BOOL CAppHistory::CommfunFile(int iType,LPCTSTR lpcszFilePath,CSimpleArray<CString>& vec_fileName,BOOL bRecursion,BOOL bEnumFiles)
{	
	try
	{	
		CString strPath = lpcszFilePath;
		ConvetPath(strPath);

		USERDATA userData;
		userData.pObject= this;
		userData.iType = iType;
		
		if (vec_fileName.GetSize()==0)
			m_fileOpt.DoFileEnumeration(strPath,bRecursion,bEnumFiles,myEnumerateFile,&userData);
		else
			m_fileOpt.DoFileEnumeration(strPath,vec_fileName,bRecursion,bEnumFiles,myEnumerateFile,&userData);

	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}



BOOL CAppHistory::myEnumerateReg(
									 HKEY rootKey,LPCTSTR lpItemPath,
									 LPCTSTR lpName,DWORD dwValueNameSize,
									 LPBYTE lpValue ,DWORD dwValueSize,DWORD dwType,
									 void* _pUserData
									 )
{	
	if (_pUserData==NULL)
		return TRUE;
	
	//路径情况
	if (lpName == NULL&&lpValue==NULL)
	{
		return TRUE;
	}
	
	//值情况
	USERDATA* pUserData = (USERDATA*) _pUserData;
	CAppHistory* pObject = (CAppHistory*) pUserData->pObject;

	LPTSTR lpValueTmp = (LPTSTR) lpValue;
	LPTSTR lpNameTmp = (LPTSTR) lpName;
	
	switch (dwType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:

		//字串类型
		return pObject->CommfunRegProc(pUserData->iType,rootKey,lpItemPath,lpNameTmp,lpValueTmp);
		break;			//除REG_SZ和REG_EXPAND_SZ其他值均忽略
	case REG_BINARY:
		break;
	case REG_DWORD_LITTLE_ENDIAN:
	case REG_DWORD_BIG_ENDIAN:							
	case REG_LINK:									
	case REG_MULTI_SZ:
	case REG_NONE:
	case REG_QWORD_LITTLE_ENDIAN:
		break;
	}
	
	//其他类型
	return pObject->CommfunRegProc(pUserData->iType,rootKey,lpItemPath,lpNameTmp,_T(""));

}

BOOL CAppHistory::myEnumerateFile(LPCTSTR lpFileOrPath, void* _pUserData,LARGE_INTEGER filesize)
{	
	if (_pUserData==NULL)
		return TRUE;
	
	//过滤所有desk.ini文件
	CString strPath = lpFileOrPath;
	CString strName;
	int nPos = -1;
	nPos = strPath.ReverseFind(L'\\');
	if (nPos != -1)
	{
		strName = strPath.Right(strPath.GetLength() - nPos - 1);
	}
	strPath.MakeLower();
	if (strName == _T("desktop.ini"))
	{
		return TRUE;
	}
	if (strName == _T("info2"))
	{
		return TRUE;
	}
	if (strName == _T("index.dat"))
	{
		return TRUE;
	}

	USERDATA* pUserData = (USERDATA*) _pUserData;
	CAppHistory* pObject = (CAppHistory*) pUserData->pObject;
	
	return pObject->CommfunFileProc(pUserData->iType,lpFileOrPath,filesize);

}

BOOL CAppHistory::CommfunRegProc(int iType,HKEY hRootKey,LPCTSTR lpSubKey,LPCTSTR lpValueName,LPCTSTR lpValueData )
{
	if (m_bScan==FALSE)
		return FALSE;

	CString strRegFullPath;
	m_regOpt.GetHKEYToString(hRootKey,lpSubKey,strRegFullPath);
	BOOL bRet = FALSE;

	bRet = g_fnScanReg(g_pMain,iType,
		strRegFullPath.GetBuffer(),
		lpValueName,
		lpValueData
		);
	strRegFullPath.ReleaseBuffer();
    return bRet;
}

BOOL CAppHistory::CommfunFileProc(int iType,LPCTSTR lpFileOrPath,LARGE_INTEGER filesize)
{
	if (m_bScan==FALSE)
		return FALSE;

	return g_fnScanFile(g_pMain,iType,lpFileOrPath,filesize.LowPart,filesize.LowPart);
}

BOOL CAppHistory::ConvetPath(CString& strSourcPath)
{	
	if (strSourcPath == _T(""))
	{
		return TRUE;
	}

	try
	{
		if(strSourcPath.GetAt(0) == '%')
		{	
			int iEnd = strSourcPath.Find('%',1);
			
			if (-1==iEnd)
				return FALSE;

			CString strEnvPath = strSourcPath.Mid(1,iEnd-1);
			CString strPath = strSourcPath.Mid(iEnd+2);

			wchar_t* pEnv;
			pEnv = _wgetenv(strEnvPath.GetBuffer());
			strEnvPath.ReleaseBuffer();
			if (pEnv!=NULL)
			{
				strEnvPath = pEnv;
				
				int iLen = strEnvPath.GetLength();
				if(strEnvPath.GetAt(iLen-1)!='\\')
				{
					strEnvPath.Append(_T("\\"));
				}

				strEnvPath.Append(strPath.GetBuffer());
				strPath.ReleaseBuffer();
				strSourcPath = strEnvPath;
			}
			else
			{
				strEnvPath = GetFolderPath(strEnvPath);

				int iLen = strEnvPath.GetLength();
				if(strEnvPath.GetAt(iLen-1)!='\\')
				{
					strEnvPath.Append(_T("\\"));
				}

				strEnvPath.Append(strPath.GetBuffer());
				strPath.ReleaseBuffer();
				strSourcPath = strEnvPath;
			}

		}
	}
	catch(...)
	{
		return FALSE;
	}

	return TRUE;
}


CString CAppHistory::GetFolderPath(LPCTSTR pszName)
{
	CString strResult;
	TCHAR szBuffer[MAX_PATH] = { 0 };

	int nFolder = -1;

	int nCount = sizeof(g_csidlnamelist) / sizeof(g_csidlnamelist[0]);

	for (int i = 0; i < nCount; i++)
	{
		if (_tcscmp( pszName, g_csidlnamelist[i].pszCsidlName) == 0)
		{
			nFolder = g_csidlnamelist[i].nFolder;
			break;
		}
	}

	if (nFolder == -1)
		goto Exit0;

	if ( ::SHGetSpecialFolderPath( NULL, szBuffer, nFolder, FALSE) )
		strResult = szBuffer;
Exit0:
	return strResult;
}