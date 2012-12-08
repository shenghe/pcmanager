//=============================================================================
/**
* @file GetAdminToken.cpp
* @brief 
* @author qiuruifeng <qiuruifeng@kingsoft.com>
* @date 2008-5-22   17:00
*/
//=============================================================================
#include "stdafx.h"
#include "GetTokenUtil.h"
#include "CreateToken.h"
//#include "KASEngLogMgr.h"
#include "GetAdminToken.h"

BOOL GetWindowsVersion(DWORD& dwPlatformId, DWORD &dwMajor, DWORD &dwMinor);

HANDLE GetAdminToken(int* pnErrCode /* =NULL */)
{
	BOOL	bResult		= FALSE;

	HANDLE	hToken		= NULL;

	HANDLE	hBasicToken	= NULL;
	DWORD	dwBasicPid	= 0;

	HANDLE  hCsrssToken = NULL;

	DWORD	dwCsrssPid	= 0;

	DWORD	dwActiveSessionId	= 0;
	DWORD	dwSessionIdLen = sizeof(DWORD);

	PTOKEN_PRIVILEGES pTokenPrivileges = NULL;

	DWORD	dwPlatformId	= 0;
	DWORD	dwMajorVersion	= 0;
	DWORD	dwMinorVersion	= 0;

	int nErrCode = 0;

	//if (!GetPidByName(_T("Explorer.EXE"), dwBasicPid))
	//{
	//	nErrCode = -2;
	//	goto Exit0;
	//}

	if ( !GetActiveSessionID(dwActiveSessionId) )
	{
		nErrCode = -4;
		goto Exit0;
	}

	if ( !GetOnePidFromSession(dwBasicPid, dwActiveSessionId) )
	{
		nErrCode = -2;
		goto Exit0;
	}

	if (!GetPrimaryTokenFromPid(dwBasicPid, hBasicToken))
	{
		nErrCode = -3;
		goto Exit0;
	}

	GetWindowsVersion(dwPlatformId, dwMajorVersion, dwMinorVersion);

	EnableCurrentProcessPrivilege(SE_CREATE_TOKEN_NAME, TRUE);

	if (IsWin2k3System(dwPlatformId, dwMajorVersion, dwMinorVersion) ||
		IsVistaSystem(dwPlatformId, dwMajorVersion, dwMinorVersion))
	{
		//////////////////////////////////////////////////////////////////////////
		//在windows 2003以及以上的系统services.exe的SE_CREATE_TOKEN_NAME特权被删除
		//导致被services.exe启动的服务无法启用SE_CREATE_TOKEN_NAME特权
		//所以这里获取csrss的token并模拟,让当前环境有SE_CREATE_TOKEN_NAME特权

		bResult = GetPidByName(_T("csrss.EXE"), dwCsrssPid);
		//if ( !bResult )
		//{
		//	KASEngLogMgr::Instance().Logger().WriteLog(_T("GetPidByeName failed."));
		//}
		//KASEngLogMgr::Instance().Logger().WriteLog(_T("GetPidByName returned PID : %d"), dwCsrssPid);

		bResult = GetPrimaryTokenFromPid(dwCsrssPid, hCsrssToken);
		//if ( !bResult )
		//{
		//	KASEngLogMgr::Instance().Logger().WriteLog(_T("GetPrimaryTokenFromPid failed."));
		//}
		//KASEngLogMgr::Instance().Logger().WriteLog(_T("GetPrimaryTokenFromPid returned a HANDLE : %ul"), hCsrssToken);

		if (hCsrssToken)
			EnableTokenPrivilege(hCsrssToken, SE_CREATE_TOKEN_NAME, TRUE);

		//copy csrss的token的特权
		if ( 0 == GetTokenInfo(hCsrssToken, TokenPrivileges, (PVOID*)&pTokenPrivileges, NULL) )
		{
			// KASEngLogMgr::Instance().Logger().WriteLog(_T("GetAdminToken.cpp:GetTokenInfo failed."));
			nErrCode = GetLastError();
			goto Exit0;
		}
	}
	else
	{
		//copy Explorer的token的特权
		if ( 0 == GetTokenInfo(hBasicToken, TokenPrivileges, (PVOID*)&pTokenPrivileges, NULL) )
		{
			// KASEngLogMgr::Instance().Logger().WriteLog(_T("GetAdminToken.cpp:GetTokenInfo failed."));
			nErrCode = GetLastError();
			goto Exit0;
		}
	}

	//暂时只考虑一个用户登录 
	//bResult = GetSessionIdFromToken(hBasicToken, &dwSessionId);
	//bResult = MakeKASPrivilege(&pTokenPrivileges);

	if (hCsrssToken)
		ImpersonateLoggedOnUser(hCsrssToken);
	//////////////////////////////////////////////////////////////////////////

	hToken = CreateAdminToken(hBasicToken, pTokenPrivileges, NULL, &nErrCode);

	//把session id 设置成active session id
	if (hToken)
	{
		if ( 0 == SetTokenInformation(hToken, TokenSessionId, &dwActiveSessionId, dwSessionIdLen) )
		{
			// KASEngLogMgr::Instance().Logger().WriteLog(_T("GetAdminToken.cpp:SetTokenInformation failed."));
			nErrCode = GetLastError();
		}
	}
Exit0:

	if (pnErrCode && nErrCode)
		*pnErrCode = nErrCode;

	if (hCsrssToken)
		::RevertToSelf();

	if (hCsrssToken)           
		::CloseHandle(hCsrssToken);

	if (hBasicToken)
		::CloseHandle(hBasicToken);

	if (pTokenPrivileges)
		FreeTokenInfo(pTokenPrivileges);

	//if (pTokenPrivileges)
	//	FreeKASPrivilege(pTokenPrivileges);

	return hToken;
}


// void FreeKASPrivilege(PTOKEN_PRIVILEGES pTokenPrivileges)
// {
// 	free(pTokenPrivileges);
// }
// BOOL MakeKASPrivilege(PTOKEN_PRIVILEGES* ppTokenPrivileges)
// { 
// 	if (NULL == ppTokenPrivileges)
// 		return FALSE;
// 
// 	TOKEN_PRIVILEGES* pTokenPrivileges = NULL;
// 	DWORD PrivilegeCount = 23;
// 	//分配多一个LUID_AND_ATTRIBUTES的空间(只是为了感觉好点^_^)
// 	DWORD dwAllocSize = sizeof(TOKEN_PRIVILEGES) + PrivilegeCount * sizeof(LUID_AND_ATTRIBUTES);
// 	pTokenPrivileges = (PTOKEN_PRIVILEGES)malloc(dwAllocSize);
// 
// 	if(pTokenPrivileges == NULL)
// 		return FALSE;
// 
// 	ZeroMemory(pTokenPrivileges, dwAllocSize);
// 
// 	pTokenPrivileges->PrivilegeCount = PrivilegeCount;
// 
// 	LookupPrivilegeValue(NULL, SE_TCB_NAME, &(pTokenPrivileges->Privileges[0].Luid));
// 	pTokenPrivileges->Privileges[0].Attributes = 0;
// 
// 	LookupPrivilegeValue(NULL, SE_CREATE_TOKEN_NAME, &(pTokenPrivileges->Privileges[1].Luid));
// 	pTokenPrivileges->Privileges[1].Attributes = 0;
// 
// 	LookupPrivilegeValue(NULL, SE_TAKE_OWNERSHIP_NAME, &(pTokenPrivileges->Privileges[2].Luid));
// 	pTokenPrivileges->Privileges[2].Attributes = 0;
// 
// 	LookupPrivilegeValue(NULL, SE_CREATE_PAGEFILE_NAME, &(pTokenPrivileges->Privileges[3].Luid));
// 	pTokenPrivileges->Privileges[3].Attributes = 0;
// 
// 	LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &(pTokenPrivileges->Privileges[4].Luid));
// 	pTokenPrivileges->Privileges[4].Attributes = 0;
// 
// 	LookupPrivilegeValue(NULL, SE_ASSIGNPRIMARYTOKEN_NAME, &(pTokenPrivileges->Privileges[5].Luid));
// 	pTokenPrivileges->Privileges[5].Attributes = 0;
// 
// 	LookupPrivilegeValue(NULL, SE_INCREASE_QUOTA_NAME, &(pTokenPrivileges->Privileges[6].Luid));
// 	pTokenPrivileges->Privileges[6].Attributes = 0; 
// 
// 	LookupPrivilegeValue(NULL, SE_INC_BASE_PRIORITY_NAME, &(pTokenPrivileges->Privileges[7].Luid));
// 	pTokenPrivileges->Privileges[7].Attributes = 0;
// 
// 	LookupPrivilegeValue(NULL, SE_CREATE_PERMANENT_NAME, &(pTokenPrivileges->Privileges[8].Luid));
// 	pTokenPrivileges->Privileges[8].Attributes = 0;
// 
// 	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &(pTokenPrivileges->Privileges[9].Luid));
// 	pTokenPrivileges->Privileges[9].Attributes = (SE_PRIVILEGE_ENABLED_BY_DEFAULT |  SE_PRIVILEGE_ENABLED);  
// 
// 	LookupPrivilegeValue(NULL, SE_AUDIT_NAME, &(pTokenPrivileges->Privileges[10].Luid));
// 	pTokenPrivileges->Privileges[10].Attributes = 0;
// 
// 	LookupPrivilegeValue(NULL, SE_SECURITY_NAME, &(pTokenPrivileges->Privileges[11].Luid));
// 	pTokenPrivileges->Privileges[11].Attributes = 0;    // disabled, not enabled by default
// 
// 	LookupPrivilegeValue(NULL, SE_SYSTEM_ENVIRONMENT_NAME, &(pTokenPrivileges->Privileges[12].Luid));
// 	pTokenPrivileges->Privileges[12].Attributes = 0;    // disabled, not enabled by default
// 
// 	LookupPrivilegeValue(NULL, SE_CHANGE_NOTIFY_NAME, &(pTokenPrivileges->Privileges[13].Luid));
// 	pTokenPrivileges->Privileges[13].Attributes = (SE_PRIVILEGE_ENABLED_BY_DEFAULT |  SE_PRIVILEGE_ENABLED); 
// 
// 	LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &(pTokenPrivileges->Privileges[14].Luid));
// 	pTokenPrivileges->Privileges[14].Attributes = 0;    // disabled, not enabled by default
// 
// 	LookupPrivilegeValue(NULL, SE_RESTORE_NAME, &(pTokenPrivileges->Privileges[15].Luid));
// 	pTokenPrivileges->Privileges[15].Attributes = 0;    // disabled, not enabled by default
// 
// 	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &(pTokenPrivileges->Privileges[16].Luid));
// 	pTokenPrivileges->Privileges[16].Attributes = 0;    // disabled, not enabled by default
// 
// 	LookupPrivilegeValue(NULL, SE_LOAD_DRIVER_NAME, &(pTokenPrivileges->Privileges[17].Luid));
// 	pTokenPrivileges->Privileges[17].Attributes = 0;    // disabled, not enabled by default
// 
// 	LookupPrivilegeValue(NULL, SE_PROF_SINGLE_PROCESS_NAME, &(pTokenPrivileges->Privileges[18].Luid));
// 	pTokenPrivileges->Privileges[18].Attributes = 0;
// 
// 	LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME, &(pTokenPrivileges->Privileges[19].Luid));
// 	pTokenPrivileges->Privileges[19].Attributes = 0;    // disabled, not enabled by default
// 
// 	LookupPrivilegeValue(NULL, SE_UNDOCK_NAME, &(pTokenPrivileges->Privileges[20].Luid));
// 	pTokenPrivileges->Privileges[20].Attributes = 0 ;   // disabled, not enabled by default
// 
//	//Windows XP, Windows 2000 SP3 and earlier, and Windows NT 没有这个特权
// 	LookupPrivilegeValue(NULL, SE_IMPERSONATE_NAME, &(pTokenPrivileges->Privileges[21].Luid));
// 	pTokenPrivileges->Privileges[21].Attributes = (SE_PRIVILEGE_ENABLED_BY_DEFAULT |  SE_PRIVILEGE_ENABLED); 
// 
//	//windows 2003新增的特权
// 	LookupPrivilegeValue(NULL, SE_CREATE_GLOBAL_NAME, &(pTokenPrivileges->Privileges[22].Luid));
// 	pTokenPrivileges->Privileges[22].Attributes = (SE_PRIVILEGE_ENABLED_BY_DEFAULT |  SE_PRIVILEGE_ENABLED);
// 
// 	*ppTokenPrivileges = pTokenPrivileges;
// 
// 	return TRUE;
// }
