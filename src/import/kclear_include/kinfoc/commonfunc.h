#ifndef _COMMONFUNC_H
#define _COMMONFUNC_H
#include <windows.h>
#include <string>
using namespace std;
LONG GetValueStringEx(const wstring& strSubKey, const wstring& strValueName, wstring& strValue);

LONG SetValueStringEx(const wstring& strSubKey, const wstring& strValueName, wstring strValue);

wstring GetSysDateString();

wstring ReadString(const wstring& strSec, const wstring& strKey);

BOOL ReadDWORD(const wstring& strSec, const wstring& strKey, int& dwVal );

BOOL isInstall();

BOOL IsPublicUpreportFileExist();

BOOL IsReportUnStartup();

BOOL IsReportUnKnownPlugIn();

#endif