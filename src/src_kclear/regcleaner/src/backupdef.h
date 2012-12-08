#ifndef _BACKUPDEF_H_
#define _BACKUPDEF_H_
#include <windows.h>
#include <winreg.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#define TYPE		"type"
#define ROOT		"root"
#define SUBKEY		"subkey"
#define VALUE		"value"
#define VALUETYPE	"valuetype"
#define DATA		"data"

#define MAX_VALUE_NAME	1024
#define	MAX_KEY_LENGTH	1024
#define BACKUPDBNAME	L"regbackup"
#define BACKUPHEAD		L"headinfo"
#define BACKUPDATA		L"datainfo"

typedef struct tagRegHkeyName
{
	string strHandle;
	HKEY hKey;
}HKEY_NAME, *PHKE_YNAME;

#define HKEY_NAME_ENTRY(ID) { #ID , ID }

__declspec(selectany) HKEY_NAME g_regHkeyNamelist[] = 
{
	HKEY_NAME_ENTRY(HKEY_CLASSES_ROOT),
	HKEY_NAME_ENTRY(HKEY_CURRENT_CONFIG),
	HKEY_NAME_ENTRY(HKEY_CURRENT_USER),
	HKEY_NAME_ENTRY(HKEY_LOCAL_MACHINE),
	HKEY_NAME_ENTRY(HKEY_PERFORMANCE_DATA),
	HKEY_NAME_ENTRY(HKEY_USERS)
};
#endif