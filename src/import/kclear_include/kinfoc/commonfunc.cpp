#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include "commonfunc.h"
#include "inieditor.h"

 #pragma comment(lib,"shlwapi.lib")  
LONG GetValueStringEx(const wstring& strSubKey, const wstring& strValueName, wstring& strValue)
{
	HKEY hKey= NULL;
	size_t lRet = ERROR_SUCCESS;

	DWORD keyType    = 0;
	BYTE szVal[MAX_PATH] = {0};
	DWORD cbData = sizeof(szVal);

	lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey.c_str(), 0, KEY_READ, &hKey);
	if(lRet == ERROR_SUCCESS)
	{
		lRet = ::RegQueryValueEx(hKey, strValueName.c_str(), 0U, &keyType, szVal, &cbData);
		if (lRet == ERROR_SUCCESS && keyType == REG_SZ)
		{
			strValue =  wstring(reinterpret_cast<WCHAR*>(szVal));
		}
		::RegCloseKey(hKey);
	}
	return (LONG)lRet;
}

LONG SetValueStringEx(const wstring& strSubKey, const wstring& strValueName, wstring strValue)
{
	HKEY hKey= NULL;
	size_t lRet = ERROR_SUCCESS;

	DWORD dwlen = (DWORD)strValue.size() * sizeof(WCHAR) + 1;

	lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey.c_str(), 0, KEY_ALL_ACCESS, &hKey);
	if(lRet == ERROR_SUCCESS)
	{
		lRet = ::RegSetValueEx
			(hKey, strValueName.c_str(), NULL, REG_SZ, (PBYTE)strValue.c_str(), dwlen);
		::RegCloseKey(hKey);
	}
	return (LONG)lRet;
}

wstring GetSysDateString()
{
	SYSTEMTIME sysTime = {0};
	WCHAR szDate[MAX_PATH] = {0};

	::GetSystemTime(&sysTime);
	::swprintf_s(szDate, L"%u-%02u-%u", sysTime.wYear, sysTime.wMonth, sysTime.wDay );
	return wstring(szDate);
}

#define PRODUCTINFO L"productidinfo.ini"
wstring ReadString(const wstring& strSec, const wstring& strKey )
{
	WCHAR strBuf[ 65536 ] = {0};
	wstring wstrFile;
	WCHAR strFileName[MAX_PATH] = {0};

	GetModuleFileName(NULL, strFileName, MAX_PATH - 1);
	PathRemoveFileSpec(strFileName);
	PathAppend(strFileName, PRODUCTINFO);
	wstrFile = strFileName;

	DWORD dwLen = GetPrivateProfileString( 
		strSec.c_str(), 
		strKey.c_str(), 
		L"", 
		strBuf, 
		sizeof(strBuf),
		wstrFile.c_str() 
		);

	/// 对于超出长度的串一律返空
	if ( sizeof( strBuf ) - 1 == dwLen )
		return L"";

	return strBuf;
}

BOOL ReadDWORD(const wstring& strSec, const wstring& strKey, int& dwVal )
{
	wstring strVal = ReadString( strSec, strKey );
	int nRet = swscanf_s( strVal.c_str(), L"%u", &dwVal );
	return ( 0 != nRet && EOF != nRet );
}

#define KSCSVC_SUBKEY		L"SOFTWARE\\Kingsoft\\KSCSVC"
#define INSTALL_FLAG		L"installdone"						
BOOL isInstall()
{
	wstring str;
	LONG lRet;

	lRet = GetValueStringEx(KSCSVC_SUBKEY, INSTALL_FLAG, str);

	if(lRet == 0)
	{
		return FALSE;
	}
	str = GetSysDateString();
	lRet = SetValueStringEx(KSCSVC_SUBKEY, INSTALL_FLAG, str);
	return TRUE;
}
#define KSC_UPLIVE_PUBLIC L"Kingsoft\\KIS\\kclt\\public_ksc.inf"
BOOL IsPublicUpreportFileExist()
{
	WCHAR wszFilePath[MAX_PATH * 2] = {0};
	SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_DEFAULT, wszFilePath);
	PathAppend(wszFilePath, KSC_UPLIVE_PUBLIC);

	DWORD dwAttrs = ::GetFileAttributes(wszFilePath);

	if ( dwAttrs != INVALID_FILE_ATTRIBUTES)
	{
		return true;
	}

	return false;
}

#define REPORTINI L"report\\reportconfig.ini"
#define UNKNOWNSTARTUP L"startup"
#define UNKNOWNPLUGIN L"plugin"
#define OFDATE L"date"
#define WEEK L"week"
BOOL IsReportUnStartup()
{
	BOOL bRet = FALSE;
	SYSTEMTIME sysTime= {0};
	wstring sysOld;
	wstring sysCurrent;
	int nDayOfWeek = 0;
	IniEditor IniEdit;
	WCHAR strPath[MAX_PATH] = {0};
	WCHAR strDate[MAX_PATH] = {0};
	GetModuleFileName(NULL, strPath, MAX_PATH - 1);
	PathRemoveFileSpec(strPath);
	PathAppend(strPath, REPORTINI);

	GetSystemTime(&sysTime);
	::swprintf_s(strDate, L"%u-%02u-%u", sysTime.wYear, sysTime.wMonth, sysTime.wDay );
	sysCurrent = strDate;
	
	IniEdit.SetFile(strPath);
	sysOld = IniEdit.ReadString(UNKNOWNSTARTUP, OFDATE);

	if (sysOld == sysCurrent)
	{
		bRet = FALSE;
		goto _exit_;
	}
	if (sysTime.wDayOfWeek == 3)
	{
		bRet = TRUE;
		IniEdit.WriteString(UNKNOWNSTARTUP, OFDATE, sysCurrent);
	}
	
_exit_:
	return bRet;
}

BOOL IsReportUnKnownPlugIn()
{
    BOOL bRet = FALSE;
    SYSTEMTIME sysTime= {0};
    wstring sysOld;
    wstring sysCurrent;
    int nDayOfWeek = 0;
    IniEditor IniEdit;
    WCHAR strPath[MAX_PATH] = {0};
    WCHAR strDate[MAX_PATH] = {0};
    GetModuleFileName(NULL, strPath, MAX_PATH - 1);
    PathRemoveFileSpec(strPath);
    PathAppend(strPath, REPORTINI);

    GetSystemTime(&sysTime);
    ::swprintf_s(strDate, L"%u-%02u-%u", sysTime.wYear, sysTime.wMonth, sysTime.wDay );
    sysCurrent = strDate;

    IniEdit.SetFile(strPath);
    sysOld = IniEdit.ReadString(UNKNOWNPLUGIN, OFDATE);

    if (sysOld == sysCurrent)
    {
        bRet = FALSE;
        goto _exit_;
    }
  
    if (sysTime.wDayOfWeek == 4)
    {
        bRet = TRUE;
        IniEdit.WriteString(UNKNOWNPLUGIN, OFDATE, sysCurrent);
        goto _exit_;
    }
    

_exit_:
    return bRet;
}
