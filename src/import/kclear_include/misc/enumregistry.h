//////////////////////////////////////////////////////////////////////////
// FileName:	EnumRegistry.h
// Author:		Jonathan Xiang
// DateTime:	April 18th, 2008
// Description:
//////////////////////////////////////////////////////////////////////////

#ifndef ENUM_REGISTRY_H_20080418
#define ENUM_REGISTRY_H_20080418

//////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "wow64helper.h"

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" 
{

typedef bool (__cdecl *ENUMREGSUBKEYPROC)(HKEY hKey, const TCHAR* lpszSubKeyName);
bool	EnumRegSubKey(HKEY hRootKey, const TCHAR* lpszKeyName, ENUMREGSUBKEYPROC fnProc);

typedef bool (__cdecl *ENUMREGVALUEPROC)(HKEY hKey, const TCHAR* lpszValueName, const void* lpValueData, DWORD dwType, DWORD dwDataLen);
bool	EnumRegValue(HKEY hRootKey, const TCHAR* lpszKeyName, ENUMREGVALUEPROC fnProc);


typedef bool (__cdecl *ENUMREGSUBKEYPROC_WITHCALLBACK)(HKEY hKey, const TCHAR* lpszSubKeyName, LPVOID param, Wow64Type wow64type);
bool	EnumRegSubKey_WithCallBack(HKEY hRootKey, const TCHAR* lpszKeyName, ENUMREGSUBKEYPROC_WITHCALLBACK fnProc, LPVOID param, Wow64Type wow64type);

typedef bool (__cdecl *ENUMREGVALUEPROC_WITHCALLBACK)(HKEY hKey, const TCHAR* lpszValueName, const void* lpValueData, DWORD dwType, DWORD dwDataLen, LPVOID param, Wow64Type wow64type);
bool	EnumRegValue_WithCallBack(HKEY hRootKey, const TCHAR* lpszKeyName, ENUMREGVALUEPROC_WITHCALLBACK fnProc, LPVOID param, Wow64Type wow64type);

};
#endif // __cplusplus

#endif // ENUM_REGISTRY_H_20080418
