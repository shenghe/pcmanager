#ifndef	 _REGBACK_H_
#define  _REGBACK_H_

#include "regopt.h"
#include <locale.h>
extern CRegOpt g_regOpt;

class CRegBack
{
public:
	CRegBack()
	{	

		TCHAR   szAppPathName[MAX_PATH*4]; 
		memset(szAppPathName,0,MAX_PATH*4);
		GetModuleFileName(NULL,szAppPathName,MAX_PATH*4);
		CString strAppPath=szAppPathName;
		int iCount = strAppPath.ReverseFind('\\');
		if (iCount<=0)
		{
			return ;
		}
		m_strAppPath = strAppPath.Mid(0,iCount);

	};

	~CRegBack(){};
	BOOL RestRegister(LPCTSTR lpcszPath);
	BOOL RegisterExport(LPCTSTR lpcszPath);
	//BOOL IsSubKey(HKEY hKey,LPCTSTR lpSubKey);
	//BOOL IsExist(HKEY hKey,LPCTSTR lpSubKey);
	void RegisterEnum(HKEY hKey,LPCTSTR lpcszSubKey);
	BOOL AppendValueToString(HKEY Hkey,LPCTSTR lpcszSubKey);
	BOOL AppendValueToString(HKEY Hkey,LPCTSTR lpcszSubKey,LPCTSTR lpcszValueName);
	void FormatDataWithDataType(DWORD dwKeyType, LPBYTE pbbinKeyData, DWORD dwKeyDataLength, CString &cstrOutput);
private:
	CString m_strRegString;
	CString m_strAppPath;

};

#endif