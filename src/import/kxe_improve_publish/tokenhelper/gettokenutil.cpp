//=============================================================================
/**
* @file GetTokenUtil.cpp
* @brief 
* @author qiuruifeng <qiuruifeng@kingsoft.com>
* @date 2008-5-22   16:37
*/
//=============================================================================
#include "stdafx.h"
#include "GetTokenUtil.h"
#include "CreateToken.h"
#include <Windows.h>
#include <TCHAR.H>
#include <Tlhelp32.h>
#include <WtsApi32.h>
//#include "stdafx.h"

#pragma comment(lib, "WtsApi32.lib")

BOOL GetPidByName(LPTSTR lpName, DWORD& dwPid)
{
	HANDLE hProcessSnap		= NULL; 
	BOOL bRetCode			= FALSE; 
	PROCESSENTRY32 pe32     = {0}; 
	//HANDLE hProcess			= INVALID_HANDLE_VALUE;

	if(NULL == lpName)
		return FALSE;

	hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE; 
	}

	pe32.dwSize = sizeof(PROCESSENTRY32); 

	if (::Process32First(hProcessSnap, &pe32)) 
	{  
		do 
		{
			if(0 == _tcsicmp(pe32.szExeFile, lpName))
			{
				dwPid = pe32.th32ProcessID;

				bRetCode = TRUE;
				break;
			}
		} 
		while (::Process32Next(hProcessSnap, &pe32)); 
	} 

	if (hProcessSnap != INVALID_HANDLE_VALUE )
	{
		CloseHandle(hProcessSnap);
	}

	return (bRetCode);
}

BOOL GetActiveSessionID(DWORD& sessionId)
{
	PWTS_SESSION_INFO pSessionInfo         = NULL;
	DWORD             dwCountOfSessionInfo = 0;
//	WTS_CONNECTSTATE_CLASS state;
	BOOL ret = FALSE;

	//当终端服务被关闭时会导致WTSEnumerateSessions调用失败,这时采用默认session,vista下是1,xp,2k是0
	if ( ::WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCountOfSessionInfo) )
	{
		for ( DWORD i = 0; i < dwCountOfSessionInfo; i++ )
		{
			if ( pSessionInfo[i].State == WTSActive )
			{
				sessionId = pSessionInfo[i].SessionId;
				ret = TRUE;
				break;
			}
		}
	}
	else
	{
		//获取windows版本
		DWORD	dwPlatformId	= 0;
		DWORD	dwMajorVersion	= 0;
		DWORD	dwMinorVersion	= 0;
		BOOL	bIsVista  = false;
		ret = GetWindowsVersion(dwPlatformId, dwMajorVersion, dwMinorVersion);
		if (IsVistaSystem(dwPlatformId, dwMajorVersion, dwMinorVersion))
		{
			sessionId = 1;
		}
		else
		{
			sessionId = 0;
		}
	}

	if (NULL != pSessionInfo)
	{
		::WTSFreeMemory(pSessionInfo);
		pSessionInfo = NULL;
	}

	return ret;
}

BOOL GetOnePidFromSession(DWORD& dwPid, DWORD dwSessionId)
{
	HANDLE hProcessSnap		= NULL; 
	HANDLE hProcess         = NULL;
	HANDLE hToken           = NULL;
	BOOL bRetCode			= FALSE; 
	PROCESSENTRY32 pe32     = {0}; 
//	DWORD  dwActiveSessionId  = 0;
	DWORD dwProcessSessionId = 0;
	PVOID pTokenUserBuf = NULL;
	DWORD size = 0;

	// 得到 active session id 先
	//if ( ! GetActiveSessionID(dwActiveSessionId) )
	//{
	//	return FALSE;
	//}

	hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE; 
	}

	pe32.dwSize = sizeof(PROCESSENTRY32); 

	if (::Process32First(hProcessSnap, &pe32)) 
	{  
		do 
		{
			if (hProcess != NULL)
			{
				::CloseHandle(hProcess);
				hProcess = NULL;
			}

			if (hToken != NULL)
			{
				::CloseHandle(hToken);
				hToken = NULL;
			}
		
			// 不是explorer.exe进程就直接 continue
			if (0 != _tcsicmp(pe32.szExeFile, _T("explorer.exe")))
				continue;

			// 找到一个PID, whoes session id is identical with the active session id
			if (FALSE == ::ProcessIdToSessionId(pe32.th32ProcessID, &dwProcessSessionId) 
				|| dwProcessSessionId != dwSessionId )
				continue;

			// 打开进程并且成功得到了token名柄
			if (NULL == (hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID)))
				continue;

			if (FALSE == ::OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken))
				continue;

			pTokenUserBuf	= NULL;
			size			= 0;
			// 成功检索到token的信息
			if (FALSE == GetTokenInfo(hToken, TokenUser, &pTokenUserBuf, &size))
				continue;
	
			// 从token里拿到sid
			// 这个进程的用户不是system时才中止
			if (!IsSystemUserSid(((PTOKEN_USER)pTokenUserBuf)->User.Sid))
			{
				dwPid = pe32.th32ProcessID;
				bRetCode = TRUE;					

				FreeTokenInfo(pTokenUserBuf);
				pTokenUserBuf = NULL;
				break;
			}

			FreeTokenInfo(pTokenUserBuf);
			pTokenUserBuf = NULL;

		}while (::Process32Next(hProcessSnap, &pe32)); 
	} 

	if (hProcess != NULL)
	{
		::CloseHandle(hProcess);
		hProcess = NULL;
	}

	if (hToken != NULL)
	{
		::CloseHandle(hToken);
		hToken = NULL;
	}

	if (hProcessSnap != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hProcessSnap);
	}

	return (bRetCode);
}

BOOL GetTokenFromPid(DWORD dwPid, _TOKEN_TYPE TokenType, HANDLE& hToken)
{
	BOOL bRetCode	= FALSE;
	BOOL bResult	= FALSE;
	HANDLE hProcess = NULL;
	HANDLE hOriToken = NULL;
	hProcess	= ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPid);

	if (NULL == hProcess)
		goto Exit0;

	bResult		= ::OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hOriToken);
	if (!bResult)
		goto Exit0;

	bResult = DuplicateTokenEx(hOriToken,
		TOKEN_ALL_ACCESS,
		NULL,
		SecurityImpersonation,
		TokenType,
		&hToken);

	bRetCode = TRUE;

Exit0:

	if (hOriToken)
	{
		::CloseHandle(hOriToken);
	}

	if (hProcess)
	{
		::CloseHandle(hProcess);
	}

	return bRetCode;
}

BOOL GetPrimaryTokenFromPid(DWORD dwPid, HANDLE& hToken)
{	
	return GetTokenFromPid(dwPid, TokenPrimary, hToken);
}

BOOL GetImpersonationTokenFromPid(DWORD dwPid, HANDLE& hToken)
{
	return GetTokenFromPid(dwPid, TokenImpersonation, hToken);
}

