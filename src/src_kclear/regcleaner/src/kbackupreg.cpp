#include "KBackupReg.h"
#include "backupdef.h"
#include "kscconv.h"
#include <shlwapi.h>
#include <tchar.h>
#pragma comment(lib, "shlwapi.lib")

CKBackupReg::CKBackupReg(void)
{
	m_bInit = FALSE;
	m_nCount = 0;
}

CKBackupReg::~CKBackupReg(void)
{
}

BOOL CKBackupReg::Init()
{
	WCHAR strTemp[MAX_PATH] = {0};

	GetModuleFileName(NULL, strTemp, MAX_PATH - 1);
	PathRemoveFileSpec(strTemp);
	PathAppend(strTemp, BACKUPDBNAME);
	m_strBackupPath = strTemp;
	KCreateLongDir(m_strBackupPath.c_str(), NULL);
//	GenBackupFileName();
	GenDataID();
//	PathAppend(strTemp, m_strBackupFileName.c_str());

	m_strBackupDb += strTemp;
	m_strBackupDb += L"\\";
	m_strBackupDb += L"regbackup.db";

	char* szError = NULL;
	char szSql[MAX_PATH] = {0};
	int nResult = -1;

	KUtf16ToUtf8<> szBackup(m_strBackupDb.c_str());
	nResult = sqlite3_open(szBackup, &m_pDB);
	if (nResult != SQLITE_OK)
	{
		//printf("%s", szError);
		m_bInit = FALSE;
		goto _eixt_;
	}

	sprintf_s(szSql, MAX_PATH - 1, "CREATE TABLE datainfo (resid TEXT, type integer, root TEXT, subroot TEXT, \
		valuekey TEXT, valuetype integer, datasize integer, datareg blob)");

	nResult = sqlite3_exec(m_pDB, szSql, NULL, NULL, &szError);

	if (nResult != SQLITE_OK)
	{
		//printf("%s", szError);
	}

	sprintf_s(szSql, MAX_PATH - 1, "CREATE TABLE headinfo (resid TEXT, number integer, time TEXT)");
	nResult = sqlite3_exec(m_pDB, szSql, NULL, NULL, &szError);
	if (nResult != SQLITE_OK)
	{
		//printf("%s", szError);
	}

	m_bInit = TRUE;
_eixt_:
	return m_bInit;
}

BOOL CKBackupReg::Unit()
{
	int nResult = -1;
	char* szError = NULL;
	char szSql[1024 * 2] = {0};
	wstring strTime = GetSysDateString();
	KUtf16ToUtf8<> wcsTime(strTime.c_str());

	sprintf_s(szSql, MAX_PATH - 1, "insert into headinfo values('%s', %d, '%s')",m_strDataId.c_str(),  m_nCount, wcsTime);
	nResult = sqlite3_exec(m_pDB, szSql, NULL, NULL, &szError);
	if (m_pDB)
	{
		sqlite3_close(m_pDB);
	}

	return TRUE;
}

BOOL CKBackupReg::BackupValueEx(const int nType,  const char* szRoot, const char* szSub, 
							  const char* szValue, const DWORD dwValueType, const void* szData, const DWORD dwDataSize)
{
	BOOL bRet = FALSE;
	int nResult = -1;
	char* szError = NULL;
	char szSql[1024 * 2] = {0};

	sqlite3_stmt* sspStart = NULL;
	sprintf_s(szSql, 1024 * 2 - 1, "insert into datainfo values('%s', %d, '%s', '%s', '%s', %d, %d, ?)", 
		m_strDataId.c_str(), nType, szRoot, szSub, szValue, dwValueType, dwDataSize);
	
	nResult = sqlite3_prepare(m_pDB, szSql, -1, &sspStart, 0);

	if(nResult != SQLITE_OK || sspStart == NULL)
	{
	//	printf("%s", szError);
		bRet = FALSE;
		goto _exit_;
	}

	nResult = sqlite3_bind_blob(sspStart, 1, szData, dwDataSize, NULL);

	nResult = sqlite3_step(sspStart);

_exit_:
	if(sspStart)
	{
		sqlite3_finalize(sspStart);
		sspStart = NULL;
	}
	return TRUE;
}

BOOL CKBackupReg::GetBackupValue(wstring strDataid)
{
	sqlite3_stmt* sspStart = NULL;
	int nResult = -1;
	BOOL bRet = FALSE; 

	WCHAR strTemp[MAX_PATH] = {0};
	wstring strBackupPathFile;
	sqlite3* pDB;
	GetModuleFileName(NULL, strTemp, MAX_PATH - 1);
	PathRemoveFileSpec(strTemp);
	PathAppend(strTemp, BACKUPDBNAME);
	PathAppend(strTemp, L"regbackup.db");

	char* szError = NULL;
	char szSql[MAX_PATH] = {0};
	KUtf16ToUtf8<> szDataid(strDataid.c_str());
	KUtf16ToUtf8<> szBackup(strTemp);
	nResult = sqlite3_open(szBackup, &pDB);
	if (nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto _exit_;
	}

	sprintf_s(szSql, MAX_PATH - 1, "select * from datainfo where resid = '%s'", szDataid);
	nResult = sqlite3_prepare(pDB, szSql, -1, &sspStart, 0);
	if(nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto _exit_;
	}
	nResult = sqlite3_step(sspStart);
	while(nResult == SQLITE_ROW)
	{
		const /*unsigned*/ char* szRootEx1 = (char*)sqlite3_column_text(sspStart, 0);
		int nType = sqlite3_column_int(sspStart, 1);
		const /*unsigned*/ char* szRootEx = (char*)sqlite3_column_text(sspStart, 2);
		const /*unsigned*/ char* szSubEx = (char*)sqlite3_column_text(sspStart, 3);
		const /*unsigned*/ char* szValueEx = (char*)sqlite3_column_text(sspStart, 4);
		int szValueType = sqlite3_column_int(sspStart, 5);
		const void* szDataEx = sqlite3_column_blob(sspStart, 7);
		int nSize = sqlite3_column_bytes(sspStart, 7);
		int nDataSize = sqlite3_column_int(sspStart, 6);
		if(nType == 1)
		{
			HKEY hKey;
			HKEY hKeySub;
			ConvertKeyHandle(szRootEx, hKey);
			KUtf8ToUtf16<> strSubKey(szSubEx);
			CreateRegSubkey(hKey, &(*strSubKey), hKeySub);
		}
		else if(nType == 0)
		{
			KscSetRegValue(szRootEx ,szSubEx, szValueEx, szValueType, szDataEx, nDataSize);
		}
		nResult = sqlite3_step(sspStart);
	}
	bRet = TRUE;

_exit_:

	if(sspStart)
	{
		sqlite3_finalize(sspStart);
		sspStart = NULL;
	}
	if(pDB)
	{
		sqlite3_close(pDB);
		pDB = NULL;
	}

	return bRet;
}

BOOL CKBackupReg::BackupKey(const char* szRoot, const char* szSubKey)
{
	int nResult = -1;
	char* szError = NULL;
	char szSql[1024 * 2] = {0};
	int nType = 1;

	sprintf_s(szSql, 1024 * 2, "insert into datainfo(resid, type, root, subroot) values('%s', '%d', '%s', '%s')",m_strDataId.c_str(), nType, szRoot, szSubKey);

	nResult = sqlite3_exec(m_pDB, szSql, NULL, NULL, &szError);

	if(nResult != SQLITE_OK)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CKBackupReg::ConvertKeyHandle(string strRoot, HKEY &hKey)
{
	BOOL bRet = FALSE;
	int nCount = -1;

	nCount = sizeof(g_regHkeyNamelist) / sizeof(g_regHkeyNamelist[0]);

	for (int i = 0; i < nCount; i++)
	{
		if (g_regHkeyNamelist[i].strHandle == strRoot)
		{
			hKey = g_regHkeyNamelist[i].hKey;
			bRet = TRUE;
			break;
		}
	}
	return bRet;
}

BOOL CKBackupReg::ConvertHandleKey(string& strRoot, HKEY hKey)
{
	BOOL bRet = FALSE;
	int nCount = -1;

	nCount = sizeof(g_regHkeyNamelist) / sizeof(g_regHkeyNamelist[0]);

	for (int i = 0; i < nCount; i++)
	{
		if (g_regHkeyNamelist[i].hKey == hKey)
		{
			strRoot = g_regHkeyNamelist[i].strHandle;
			bRet = TRUE;
			break;
		}
	}
	return bRet;
}

BOOL CKBackupReg::KscSetRegValue(string strRoot, string strSubKey, string strValueName, DWORD dwValueType, const void *szData, DWORD dwSize)
{
	HKEY hKey;
	HKEY hSubKey;
	BOOL bRet = FALSE;
	LONG lRet = -1;
	KUtf8ToUtf16<> wcsSubKey(strSubKey.c_str());
	KUtf8ToUtf16<> wcsValueName(strValueName.c_str());

	bRet = ConvertKeyHandle(strRoot, hKey);
	if(bRet == FALSE)
	{
		goto _exit_;
	}

	bRet = CreateRegSubkey(hKey, &(*wcsSubKey), hSubKey);
	if(bRet == FALSE)
	{
		goto _exit_;
	}

	lRet = RegSetValueEx(hSubKey, wcsValueName, NULL, dwValueType, (BYTE*)szData, dwSize);
	if(lRet != ERROR_SUCCESS)
	{
		bRet = FALSE;
		goto _exit_;
	}
	bRet = TRUE;
_exit_:
	return bRet;
}

BOOL CKBackupReg::BackupValue(HKEY hKey, wstring strSubKey, wstring strValueName)
{
	HKEY hKeySub;
	DWORD dwRetType = -1;
	BYTE* bData = NULL;
	DWORD dwRetSize = 0;
	BOOL bRet = FALSE;
	if(RegOpenKeyEx(hKey, strSubKey.c_str(), 0, KEY_READ, &hKeySub) != ERROR_SUCCESS)
	{
		bRet = FALSE;
		goto _exit_;
	}
	if (RegQueryValueEx(hKeySub, strValueName.c_str(), NULL, &dwRetType, NULL, &dwRetSize) != ERROR_SUCCESS)
	{
		bRet = FALSE;
		goto _exit_;
	}
	if(dwRetSize != 0)
	{
		bData = new BYTE[dwRetSize];
		ZeroMemory(bData, dwRetSize);
		RegQueryValueEx(hKeySub,
			strValueName.c_str(),
			NULL,
			&dwRetType,
			bData,
			&dwRetSize
			);
		KUtf16ToUtf8<> szSubKey(strSubKey.c_str());
		KUtf16ToUtf8<> szValue(strValueName.c_str());
		string strKey;
		ConvertHandleKey(strKey, hKey);
		BackupValueEx(0, strKey.c_str(), szSubKey, szValue, dwRetType, bData, dwRetSize);
		delete bData;
		bData = NULL;
	}
	bRet = TRUE;
_exit_:

	return bRet;
}

BOOL CKBackupReg::CreateRegSubkey(HKEY hKey, wstring strSubkey, HKEY& hRetKey)
{
	LONG lRet = -1;

	lRet = RegCreateKeyEx(hKey, strSubkey.c_str(), NULL, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRetKey, NULL);
	if(lRet != ERROR_SUCCESS)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CKBackupReg::QueryKey(HKEY hKey, wstring strSubKey) 
{ 
	WCHAR    achKey[MAX_KEY_LENGTH] = {0};		// buffer for subkey name
	DWORD    cbName;							// size of name string 
	WCHAR	 achClass[MAX_PATH] = {0};			// buffer for class name 
	DWORD    cchClassName = MAX_PATH;			// size of class string 
	DWORD    cSubKeys=0;						// number of subkeys 
	DWORD    cbMaxSubKey = 0;						// longest subkey size 
	DWORD    cchMaxClass = 0;						// longest class string 
	DWORD    cValues = 0;							// number of values for key 
	DWORD    cchMaxValue = 0;						// longest value name 
	DWORD    cbMaxValueData = 0;					// longest value data 
	DWORD    cbSecurityDescriptor =  0;				// size of security descriptor 
	FILETIME ftLastWriteTime;					// last write time 

	HKEY	hSubKey = NULL;
	HKEY    hTempKey = NULL;
	DWORD	i =0, retCode = 0; 
	wstring	strSubSubKey;

	TCHAR  achValue[MAX_VALUE_NAME]; 
	DWORD cchValue = MAX_VALUE_NAME; 

	DWORD dwRetType = -1;
	DWORD dwRetSize = 0;
	BYTE* bData = NULL;
	BOOL bRet = FALSE;
	
	if(RegOpenKeyEx(hKey, strSubKey.c_str(), 0, KEY_READ, &hTempKey) != ERROR_SUCCESS)
	{
		bRet = FALSE;
		goto _exit_;
	}

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hTempKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

	// Enumerate the subkeys, until RegEnumKeyEx fails.

	if (cSubKeys)
	{
		for (i=0; i<cSubKeys; i++) 
		{ 
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hTempKey, i,
				achKey, 
				&cbName, 
				NULL, 
				NULL, 
				NULL, 
				&ftLastWriteTime); 
			if (retCode == ERROR_SUCCESS) 
			{
				printf("(%d) %s\n", i+1, achKey); 
				strSubSubKey = strSubKey;
				strSubSubKey += L"\\";
				strSubSubKey += achKey;
				if( RegOpenKeyEx( hKey,
					strSubSubKey.c_str(),
					0,
					KEY_READ,
					&hSubKey) == ERROR_SUCCESS
					)
				{
					QueryKey(hKey, strSubSubKey);
				}
				if(hSubKey)
				{
					RegCloseKey(hSubKey);
				    hSubKey = NULL;
				}
			}
		}
	} 

	// Enumerate the key values. 

	if (cValues) 
	{
		for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) 
		{ 
			cchValue = MAX_VALUE_NAME; 
			achValue[0] = '\0'; 
			retCode = RegEnumValue(hTempKey, i, 
				achValue, 
				&cchValue, 
				NULL, 
				NULL,
				NULL,
				NULL);

			if (retCode == ERROR_SUCCESS ) 
			{ 
				RegQueryValueEx(hTempKey,
					achValue,
					NULL,
					&dwRetType,
					NULL,
					&dwRetSize
					);
				if(dwRetSize != 0)
				{
					bData = new BYTE[dwRetSize];
					ZeroMemory(bData, dwRetSize);
					RegQueryValueEx(hTempKey,
						achValue,
						NULL,
						&dwRetType,
						bData,
						&dwRetSize
						);
					KUtf16ToUtf8<> szSubKey(strSubKey.c_str());
					KUtf16ToUtf8<> szValue(achValue);
					string strKey;
					ConvertHandleKey(strKey, hKey);
					BackupValueEx(0, strKey.c_str(), szSubKey, szValue, dwRetType, bData, dwRetSize);
					delete bData;
					bData = NULL;
				}
//				printf("(%d) %s\n", i+1, achValue); 
			} 
		}
	}
	if (!cSubKeys && !cValues)
	{
		string strKey;
		KUtf16ToUtf8<> szSubKey(strSubKey.c_str());
		ConvertHandleKey(strKey, hKey);
		BackupKey(strKey.c_str(), szSubKey);
	}
_exit_:
	if(hTempKey)
	{
		RegCloseKey(hTempKey);
		hTempKey = NULL;
	}
	return TRUE;
}

BOOL CKBackupReg::GenBackupFileName()
{
	unsigned long ulFileNameNum = 0;
	WCHAR szFileName[MAX_PATH] = { 0 };
	WCHAR szTempString[MAX_PATH] = {0};
	wstring strBackupPath;

	strBackupPath = m_strBackupPath;

	ulFileNameNum = rand() % 100000000L;
	do 
	{
		ulFileNameNum= (++ulFileNameNum)% 100000000L;
		_sntprintf_s(szFileName, MAX_PATH - 1, (sizeof(szFileName) / sizeof(TCHAR)), TEXT("%08dREG"), ulFileNameNum);

		::wcsncpy_s(szTempString, strBackupPath.c_str(), MAX_PATH - 1);
		::PathAppend(szTempString, szFileName);
		::wcscat(szTempString, L".DAT");
		if (_taccess(szTempString, 0) != 0)
		{
			//文件不存在,生成的文件名可用
			break;
		}
		else
		{
			//生成的文件名已经存在,更新文件名种子
			ulFileNameNum = rand() % 100000000L;
			//printf(" RenameSeed %d ", m_ulFileNameSeed);
		}
	} while(TRUE);

	m_strBackupFileName = szFileName;
	return TRUE;
}

BOOL CKBackupReg::KCreateLongDir(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	BOOL bRet = FALSE;
	if (lpPathName == NULL)
	{
		bRet = FALSE;
		goto _exit_;
	}
	if (::PathIsDirectory(lpPathName))
	{
		bRet = TRUE;
		goto _exit_;
	}
	else 
	{
		TCHAR szFatherPath[MAX_PATH];
		::lstrcpyn(szFatherPath, lpPathName, MAX_PATH);
		LPTSTR lpSlash = ::_tcsrchr(szFatherPath, TEXT('\\'));
		if (lpSlash != NULL)
		{
			*lpSlash = 0;
		}
		// 递归，先创建基层目录，再创建子目录。
		if (lpSlash == NULL || KCreateLongDir(szFatherPath, lpSecurityAttributes))
		{
			if (::CreateDirectory(lpPathName, lpSecurityAttributes))
			{
				bRet = TRUE;
				goto _exit_;
			}
		}
		bRet = FALSE;
		goto _exit_;
	}

_exit_:
	return bRet;
}

BOOL CKBackupReg::KscBackupReg(HKEY hKey, wstring strSubKey, wstring strValueName)
{
	BOOL bRet = FALSE;
	if(strValueName.size() == 0)
	{
		bRet = QueryKey(hKey, strSubKey);
	}
	else
	{
		bRet = BackupValue(hKey, strSubKey, strValueName);
	}
	m_nCount++;

	return bRet;
}

BOOL CKBackupReg::KscGetBackupInfo(MSSGS& m_mwcsFileid, MSSGS& m_mwcsExid)
{
	BOOL bRet = FALSE;
	WCHAR strTemp[MAX_PATH] = {0};
	wstring strBackupPathFile;
	sqlite3* pDB;

	sqlite3_stmt* sspStart = NULL;
	int nResult = -1;
	WCHAR strBuff[MAX_PATH] = {0};

	// 查找regbackup.db
	GetModuleFileName(NULL, strTemp, MAX_PATH - 1);
	PathRemoveFileSpec(strTemp);
	PathAppend(strTemp, BACKUPDBNAME);
	PathAppend(strTemp, L"regbackup.db");

	char* szError = NULL;
	char szSql[MAX_PATH] = {0};

	KUtf16ToUtf8<> szBackup(strTemp);
	nResult = sqlite3_open(szBackup, &pDB);
	if (nResult != SQLITE_OK)
	{
// 		printf("%s", szError);
// 		return FALSE;
		bRet = FALSE;
		goto _exit_;
	}

	// 读取每条记录: id, 数目，时间
	nResult = sqlite3_prepare(pDB, "select * from headinfo", -1, &sspStart, 0);
	if(nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto _exit_;
	}

	nResult = sqlite3_step(sspStart);
	while(nResult == SQLITE_ROW)
	{
		const char*		szDataId= (char*)sqlite3_column_text(sspStart, 0);
		int				nCount = sqlite3_column_int(sspStart, 1);
		const char*		szTime = (char*)sqlite3_column_text(sspStart, 2);
		KUtf8ToUtf16<>	wcsTime(szTime);
		KUtf8ToUtf16<>	wcsDataid(szDataId);

		swprintf_s(strBuff, MAX_PATH -1, L"此次共备份了 %d 个注册表项", nCount);
		m_mwcsFileid.insert(pair<wstring, wstring>(&(*wcsTime), &(*wcsDataid)));
		m_mwcsExid.insert(pair<wstring, wstring>(&(*wcsTime), strBuff));
		nResult = sqlite3_step(sspStart);
	}

_exit_:
	if(sspStart)
	{
		sqlite3_finalize(sspStart);
		sspStart = NULL;
	}

	if(pDB)
	{
		sqlite3_close(pDB);
		pDB = NULL;
	}

	return bRet;
}

BOOL CKBackupReg::GenDataID()
{
	SYSTEMTIME sysTime = {0};

	char szDate[MAX_PATH] = {0};
	GetLocalTime(&sysTime);

	sprintf_s(szDate, "%u%02u%02u%02u%02u%02u", sysTime.wYear, sysTime.wMonth, 
		sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	m_strDataId = szDate;
	return TRUE;
}

wstring CKBackupReg::GetSysDateString()
{
	SYSTEMTIME	sysTime = {0};
	WCHAR		szDate[MAX_PATH] = {0};
	
	GetLocalTime(&sysTime);
	swprintf_s(szDate, L"%u年%02u月%02u日 %02u:%02u:%02u", sysTime.wYear, sysTime.wMonth, 
		sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
	return wstring(szDate);
}


BOOL CKBackupReg::DeleteData(wstring strDataid)
{
	int nResult = -1;

	WCHAR strTemp[MAX_PATH] = {0};
	wstring strBackupPathFile;
	sqlite3* pDB;
	sqlite3_stmt* sspStart = NULL;
	BOOL bRet = FALSE;

	GetModuleFileName(NULL, strTemp, MAX_PATH - 1);
	PathRemoveFileSpec(strTemp);
	PathAppend(strTemp, BACKUPDBNAME);
	PathAppend(strTemp, L"regbackup.db");

	KUtf16ToUtf8<> szBackup(strTemp);
	nResult = sqlite3_open(szBackup, &pDB);

	char* szError = NULL;
	char szSql[MAX_PATH] = {0};
	KUtf16ToUtf8<> szDataid(strDataid.c_str());

	sprintf_s(szSql, MAX_PATH - 1, "delete from datainfo where resid = '%s'", szDataid);

	nResult = sqlite3_prepare(pDB, szSql, -1, &sspStart, 0);

	if(nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto _exit_;
	}
	nResult = sqlite3_step(sspStart);

	sprintf_s(szSql, MAX_PATH - 1, "delete from headinfo where resid = '%s'", szDataid);
	nResult = sqlite3_exec(pDB, szSql, 0, 0, &szError);
	bRet = FALSE;
_exit_:

	if(sspStart)
	{
		sqlite3_finalize(sspStart);
		sspStart = NULL;
	}
	
	if(pDB)
	{
		sqlite3_close(pDB);
		pDB = NULL;
	}

	return TRUE;
}


BOOL CKBackupReg::DeleteData(HKEY hKey, wstring strSubKey, wstring strValueName)
{
	int nResult = -1;

	WCHAR strTemp[MAX_PATH] = {0};
	wstring strBackupPathFile;
	sqlite3* pDB;
	sqlite3_stmt* sspStart = NULL;
	BOOL bRet = FALSE;

	GetModuleFileName(NULL, strTemp, MAX_PATH - 1);
	PathRemoveFileSpec(strTemp);
	PathAppend(strTemp, BACKUPDBNAME);
	PathAppend(strTemp, L"regbackup.db");

	KUtf16ToUtf8<> szBackup(strTemp);
	nResult = sqlite3_open(szBackup, &pDB);

	char* szError = NULL;
	//char szSql[MAX_PATH] = {0};
	std::string szSql;
	std::string szSubKey = UnicodeToUtf8(strSubKey);
	std::string szValue = UnicodeToUtf8(strValueName);
	string strKey;
	ConvertHandleKey(strKey, hKey);

// 	sprintf_s(szSql, MAX_PATH - 1, "delete from datainfo where root = '%s' and subroot = '%s' and valuekey = '%s'", 
// 		strKey.c_str(),szSubKey.c_str(),szValue.c_str());
	szSql = "delete from datainfo where root = \'";
	szSql += strKey;
	szSql += "\' and subroot = \'";
	szSql += szSubKey;
	szSql += "\' and valuekey = \'";
	szSql += szValue;
	szSql += "\'";
	nResult = sqlite3_prepare(pDB, szSql.c_str(), -1, &sspStart, 0);

	if(nResult != SQLITE_OK)
	{
		bRet = FALSE;
		goto _exit_;
	}
	nResult = sqlite3_step(sspStart);

	m_nCount--;

_exit_:

	if(sspStart)
	{
		sqlite3_finalize(sspStart);
		sspStart = NULL;
	}

	if(pDB)
	{
		sqlite3_close(pDB);
		pDB = NULL;
	}

	return TRUE;
}

