#pragma once
#include "vulfix/Defines.h"

class DBFilterBase;

DBFilterBase * CreateOSFilter( INT nWinVer, DWORD dwFlags );
BOOL InitOSFilter( DBFilterBase *pFilter, INT nWinVer, DWORD dwFlags );

BOOL EnableCom(LPCTSTR lpClsid, BOOL bEnable);
BOOL GetComState( LPCTSTR lpClsid, BOOL &bEnabled );

void SafeTerminateThread( HANDLE &hThread );

BOOL WriteVulConfig( LPCTSTR lpSectionName, LPCTSTR lpKeyName, LPCTSTR lpString );
BOOL ReadVulConfig( LPCTSTR lpSectionName, LPCTSTR lpKeyName, CString& strValue, LPCTSTR szDefault=NULL );
LCID FixLocale(LCID id=0);