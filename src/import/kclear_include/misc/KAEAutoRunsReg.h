/********************************************************************
//	File Name	:	KAEAutoRunsReg.h
//	Version		:	1.0
//	Datetime	:	2006-6-27	14:59	--	Created.
//	Author		:	Chenghu
//	Comment		:	
*********************************************************************/

#ifndef _KAEAutoRunsReg_h_
#define _KAEAutoRunsReg_h_

#include <AccCtrl.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MAX_STRING_SIZE
#define MAX_STRING_SIZE 1024
#endif

unsigned int KAERegOpenKey(
                           /* [In] */ HKEY hKey,
                           /* [In] */ LPWSTR lpSubKey,
                           /* [Out] */ PHKEY phkResult,
                           /* [In] */ DWORD AceessType = KEY_ALL_ACCESS
                           );

unsigned int KAERegCreateKey(
                             /* [In] */ HKEY hKey,
                             /* [In] */ LPWSTR lpSubKey,
                             /* [Out] */ PHKEY phkResult
                             );

unsigned int KAERegCloseKey(
                            /* [In] */ HKEY hKey
                            );

unsigned int KAERegQueryValue(
                              /* [In] */ HKEY hKey,
                              /* [In] */ LPWSTR lpSubKey,
                              /* [Out] */ LPWSTR lpValue,
                              /* [In, Out] */ PLONG lpcbValue
                              );

unsigned int KAERegQueryValueEx(
                                /* [In] */ HKEY hKey,
                                /* [In] */ LPWSTR lpValueName,
                                /* [NULL] */ LPDWORD lpReserved,
                                /* [Out] */ LPDWORD lpType,
                                /* [Out] */ LPBYTE lpData,
                                /* [In, Out] */ LPDWORD lpcbData
                                );

unsigned int KAERegEnumKey(
                           /* [In] */ HKEY hKey,
                           /* [In] */ DWORD dwIndex,
                           /* [Out] */ LPWSTR lpName,
                           /* [In] */ DWORD cchName
                           );

unsigned int KAERegEnumValue(
                             /* [In] */ HKEY hKey,
                             /* [In] */ DWORD dwIndex,
                             /* [Out] */ LPWSTR lpValueName,
                             /* [In, Out] */ LPDWORD lpcValueName,
                             /* [NULL] */ LPDWORD lpReserved,
                             /* [Out] */ LPDWORD lpType,
                             /* [Out] */ LPBYTE lpData,
                             /* [In, Out] */ LPDWORD lpcbData
                             );

unsigned int KAERegSetValue(
                            /* [In] */ HKEY hKey,
                            /* [In] */ LPWSTR lpSubKey,
                            /* [In] */ DWORD dwType,
                            /* [In] */ LPWSTR lpData,
                            /* [In] */ DWORD cbData
                            );

unsigned int KAERegSetValueEx(
                              /* [In] */ HKEY hKey,
                              /* [In] */ LPWSTR lpValueName,
                              /* [0] */ DWORD Reserved,
                              /* [In] */ DWORD dwType,
                              /* [In] */ const BYTE* lpData,
                              /* [In] */ DWORD cbData
                              );

unsigned int KAERegDeleteKey(
                             /* [In] */ HKEY hKey,
                             /* [In] */ LPWSTR lpSubKey
                             );

//Delete key and all sub keys.
unsigned int KAERegDeleteKeyEx(
                               /* [In] */ HKEY hKey,
                               /* [In] */ LPWSTR lpSubKey
                               );

unsigned int KAERegDeleteValue(
                               /* [In] */ HKEY hKey,
                               /* [In] */ LPWSTR lpValueName
                               );


unsigned int KAGRegUpKeySecurity(
                                 /* [In] */ HKEY hKey,
                                 /* [In] */const wchar_t* pwszKey
                                 );

unsigned int KAERegGetKeySecurity(
                                  HKEY hKey,
                                  SECURITY_INFORMATION SecurityInformation,
                                  PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                  LPDWORD lpcbSecurityDescriptor
                                  );

unsigned int KAERegSetKeySecurity(
                                  HKEY hKey,
                                  SECURITY_INFORMATION SecurityInformation,
                                  PSECURITY_DESCRIPTOR pSecurityDescriptor
                                  );

unsigned int KAERegCopyKey(HKEY hKey, wchar_t *pszFromKeys, wchar_t *pszToKeys);

int GetRealClsidFile(wchar_t *pwszClsid, wchar_t *pwszFileName, wchar_t *pwszTreatAsClsid, int nControl);

BOOL SetRegKeyPrivilege(
                        LPWSTR lpRegKey, 
                        DWORD AccessPermissions, 
                        ACCESS_MODE AccessMode
                        );

#endif //_KAEAutoRunsReg_h_