/************************************************************************
* @file      : upreggradeprivilege.h
* @author    : ChenZhiQiang <chenzhiqiang@kingsoft.com>
* @date      : 2010/3/17 16:02:24
* @brief     : 
*
* $Id: $
/************************************************************************/
#ifndef __UPREGGRADEPRIVILEGE_H__
#define __UPREGGRADEPRIVILEGE_H__

#include "accctrl.h"

// -------------------------------------------------------------------------

BOOL UpgradePrivilege(LPCTSTR key, SE_OBJECT_TYPE objectType);

BOOL UpgradePrivilege(HKEY hKey,  LPTSTR lpSubKey); 

// -------------------------------------------------------------------------
// $Log: $

#endif /* __UPREGGRADEPRIVILEGE_H__ */
