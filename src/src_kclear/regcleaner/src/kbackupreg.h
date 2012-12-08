#ifndef _KBACKUPREG_H_
#define _KBACKUPREG_H_
#include <windows.h>
#include <map>
#include <string>
#include "sqlite3.h"
using namespace std;

typedef std::map<std::wstring, std::wstring, std::greater<std::wstring> >	MSSGS;

class CKBackupReg
{
public:
	CKBackupReg(void);
	~CKBackupReg(void);

public:
	BOOL Init();

	BOOL Unit();

	BOOL BackupValueEx(const int nType, const char* szRoot, const char* szSub, 
		const char* szValue, const DWORD szValueType, const void* szData, const DWORD dwDataSize);

	BOOL GetBackupValue(wstring strDataid);

	BOOL BackupKey(const char* szRoot, const char* szSubKey);

	BOOL ConvertKeyHandle(string strRoot, HKEY& hKey);

	BOOL ConvertHandleKey(string& strRoot, HKEY hKey);

	BOOL BackupValue(HKEY hKey, wstring strSubKey, wstring strValueName);

	BOOL QueryKey(HKEY hKey, wstring strSubKey);

	BOOL CreateRegSubkey(HKEY hKey, wstring strSubKey, HKEY& hRetKey);

	BOOL KscSetRegValue(string strRoot, string strSubKey, string strValueName, DWORD dwValueType, const void* szData, DWORD dwSize);

	BOOL GenBackupFileName();

	BOOL KCreateLongDir(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

	BOOL KscBackupReg(HKEY hKey, wstring strSubKey, wstring strValueName);

	BOOL KscGetBackupInfo(MSSGS& m_mwcsFileid, MSSGS& m_mwcsExid);

	BOOL GenDataID();
	wstring GetSysDateString();

	BOOL DeleteData(wstring strDataid);

	BOOL DeleteData(HKEY hKey, wstring strSubKey, wstring strValueName);

private:
	BOOL			m_bInit;
	HKEY			m_hKey;
	wstring			m_strBackupDb;
	sqlite3*		m_pDB;
	wstring			m_strBackupFileName;
	wstring			m_strBackupPath;
	int				m_nCount;
	string			m_strDataId;
};

#endif
