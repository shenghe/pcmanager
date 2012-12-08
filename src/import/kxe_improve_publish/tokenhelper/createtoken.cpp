//=============================================================================
/**
* @file CreateToken.cpp
* @brief 
* @author qiuruifeng <qiuruifeng@kingsoft.com>
* @date 2008-5-19   23:07
* @version 2009-7-9 ver 1.2 比上个版本的增强在于可以创建非管理员账户token
*/
//=============================================================================
#include "stdafx.h"
#include <windows.h>
#include <LM.h>
#include <AccCtrl.h>
#include <Aclapi.h>

#include "ntdll.h"
#include "CreateToken.h"

#pragma comment(lib, "Netapi32.lib")

BOOL GetWindowsVersion(DWORD& dwPlatformId, DWORD &dwMajor, DWORD &dwMinor);

#define LOW_MANDATORY_LEVEL_SID		"S-1-16-4096"
#define MEDIUM_MANDATORY_LEVEL_SID	"S-1-16-8192"
#define HIGH_MANDATORY_LEVEL_SID	"S-1-16-12288"
#define SYSTEM_MANDATORY_LEVEL_SID	"S-1-16-16384"
#define ADMINISTRATORS_GROUP_SID	"S-1-5-32-544"
#define SYSTEM_USER_SID				"S-1-5-18"

typedef struct _SID_STRUCT
{
	unsigned char	Revision;
	unsigned char	SubAuthorityCount;
	unsigned char	IdentifierAuthority[6];
	unsigned int	SubAuthority[1];
}SID_STRUCT;

typedef struct _SID_STRUCT_2
{
	unsigned char	Revision;
	unsigned char	SubAuthorityCount;
	unsigned char	IdentifierAuthority[6];
	unsigned int	SubAuthority[2];
}SID_STRUCT_2;

const SID_STRUCT _LowMandatoryLevelSid		= { 1, 1, {0, 0, 0, 0, 0, 16}, {4096} };
const SID_STRUCT _MediumMandatoryLevelSid	= { 1, 1, {0, 0, 0, 0, 0, 16}, {8192} };
const SID_STRUCT _HighMandatoryLevelSid		= { 1, 1, {0, 0, 0, 0, 0, 16}, {12288} };
const SID_STRUCT _SystemMandatoryLevelSid	= { 1, 1, {0, 0, 0, 0, 0, 16}, {16384} };

//也可以用CreateWellKnownSid(WinLocalSystemSid, NULL, pSystemSid, &dwSystemSid)获取
const SID_STRUCT _SystemUserSid				= { 1, 1, {0, 0, 0, 0, 0, 5}, {18} };

//也可以用CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, pAdminSid, &dwAdminSidSize)获取
const SID_STRUCT_2 _AdministratorsGropuSid	= { 1, 2, {0, 0, 0, 0, 0, 5}, {32, 544} };

const SID_STRUCT_2 _UsersGropuSid			= { 1, 2, {0, 0, 0, 0, 0, 5}, {32, 545} };


typedef UINT (WINAPI * pfnZwCreateTokenType)(
			  PHANDLE				TokenHandle,
			  ACCESS_MASK			DesiredAccess,
			  POBJECT_ATTRIBUTES	ObjectAttributes,
			  TOKEN_TYPE			Type,
			  PLUID					AuthenticationId,
			  PLARGE_INTEGER		ExpirationTime,
			  PTOKEN_USER			User,
			  PTOKEN_GROUPS			Groups,
			  PTOKEN_PRIVILEGES		Privileges,
			  PTOKEN_OWNER			Owner,
			  PTOKEN_PRIMARY_GROUP	PrimaryGroup,
			  PTOKEN_DEFAULT_DACL	DefaultDacl,
			  PTOKEN_SOURCE			Source
			  );

typedef UINT (WINAPI * pfnRtlNtStatusToDosErrorType)(UINT dwError);

// already defined in KASProcUtility.h.
BOOL GetWindowsVersion(DWORD& dwPlatformId, DWORD &dwMajor, DWORD &dwMinor)
{
	BOOL bRetCode = TRUE;
	OSVERSIONINFOEX  OsVersionInfoEx;

	ZeroMemory(&OsVersionInfoEx, sizeof(OSVERSIONINFOEX));
	OsVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (!GetVersionEx((LPOSVERSIONINFO)&OsVersionInfoEx))
	{
		OsVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((LPOSVERSIONINFO)&OsVersionInfoEx))
			bRetCode = FALSE;
	}

	if (bRetCode)
	{
		dwPlatformId	= OsVersionInfoEx.dwPlatformId;
		dwMajor			= OsVersionInfoEx.dwMajorVersion;
		dwMinor			= OsVersionInfoEx.dwMinorVersion;
	}

	return bRetCode;
}

BOOL IsLowMandatoryLevelSid(PSID pSid)
{
	return pSid ? 
		(0 == memcmp(pSid, &_LowMandatoryLevelSid, sizeof(SID_STRUCT))) : FALSE;
}

BOOL IsMediumMandatoryLevelSid(PSID pSid)
{
	return pSid ? 
		(0 == memcmp(pSid, &_MediumMandatoryLevelSid, sizeof(SID_STRUCT))) : FALSE;
}

BOOL Is_HighMandatoryLevelSid(PSID pSid)
{
	return pSid ? 
		(0 == memcmp(pSid, &_HighMandatoryLevelSid, sizeof(SID_STRUCT))) : FALSE;
}

BOOL IsSystemMandatoryLevelSid(PSID pSid)
{
	return pSid ? 
		(0 == memcmp(pSid, &_SystemMandatoryLevelSid, sizeof(SID_STRUCT))) : FALSE;
}

BOOL IsAdministratorGroupSid(PSID pSid)
{
	SID_STRUCT* pSidData = (SID_STRUCT*)pSid;
	//由表达式的短路语法保证当pSidData->SubAuthorityCount < 2时
	//不会对pSidData->SubAuthority[1]进行访问 
	return pSidData ? 
		(  1	== pSidData->Revision
		&& 2	== pSidData->SubAuthorityCount
		&& 5	== pSidData->IdentifierAuthority[5]
		&& 32	== pSidData->SubAuthority[0]
		&& 544	== pSidData->SubAuthority[1]) : FALSE;
}

BOOL IsUsersGroupSid(PSID pSid)
{
	SID_STRUCT* pSidData = (SID_STRUCT*)pSid;
	//由表达式的短路语法保证当pSidData->SubAuthorityCount < 2时
	//不会对pSidData->SubAuthority[1]进行访问 
	return pSidData ? 
		(  1	== pSidData->Revision
		&& 2	== pSidData->SubAuthorityCount
		&& 5	== pSidData->IdentifierAuthority[5]
		&& 32	== pSidData->SubAuthority[0]
		&& 545	== pSidData->SubAuthority[1]) : FALSE;
}

BOOL IsSystemUserSid(PSID pSid)
{
	return pSid ? 
		(0 == memcmp(pSid, &_SystemUserSid, sizeof(SID_STRUCT))) : FALSE;
}

BOOL EnableTokenPrivilege(HANDLE hToken, LPCTSTR szPrivName, BOOL bEnable)
{
	if (NULL == hToken || NULL == szPrivName)
		return FALSE;

	TOKEN_PRIVILEGES Tp		= { 0 };
	LUID	Luid			= { 0 };

	if(!LookupPrivilegeValue(NULL, szPrivName, &Luid))
		return FALSE;

	Tp.PrivilegeCount			= 1;
	Tp.Privileges[0].Luid		= Luid;
	Tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
	
	return AdjustTokenPrivileges(hToken, FALSE, &Tp, sizeof(Tp), NULL, NULL);
}

BOOL EnableProcessPrivilege(HANDLE hProcess, LPCTSTR szPrivName, BOOL bEnable)
{
	if (NULL == hProcess || NULL == szPrivName)
		return FALSE;

	BOOL bRetCode			= { 0 };
	TOKEN_PRIVILEGES Tp		= { 0 };
	LUID	Luid			= { 0 };
	HANDLE	hToken			= NULL;

	if(!LookupPrivilegeValue(NULL, szPrivName, &Luid))
		return FALSE;

	if(!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return FALSE;

	Tp.PrivilegeCount			= 1;
	Tp.Privileges[0].Luid		= Luid;
	Tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	bRetCode = AdjustTokenPrivileges(hToken, FALSE, &Tp, sizeof(Tp), NULL, NULL);

	CloseHandle(hToken);

	return bRetCode;
}

BOOL EnableCurrentProcessPrivilege(LPCTSTR szPrivName, BOOL bEnable)
{
	return EnableProcessPrivilege(::GetCurrentProcess(), szPrivName, bEnable);
}

BOOL GetSessionIdFromToken(HANDLE hToken, DWORD* pdwSession)
{
	if (NULL == hToken || NULL == pdwSession)
		return FALSE;

	DWORD dwSessionlen = sizeof(DWORD);
	
	return GetTokenInformation(hToken, TokenSessionId, 
		pdwSession, dwSessionlen, &dwSessionlen);
}

BOOL GetTokenInfo(HANDLE hToken, 
				  TOKEN_INFORMATION_CLASS Tic, 
				  PVOID *pTokenInfoOut, 
				  DWORD* pdwOutSize)
{
	BOOL	bRetCode= TRUE;
	DWORD	dwSize	= 0;
	PVOID	pv		= NULL;

	if(NULL == pTokenInfoOut)
		bRetCode = FALSE;
	else if(!GetTokenInformation(hToken, Tic, 0, 0, &dwSize) 
		&& GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
		bRetCode = FALSE;
	else if(NULL == (pv = malloc(dwSize)))
		bRetCode = FALSE;
	else if(!GetTokenInformation(hToken, Tic, pv, dwSize, &dwSize))
	{		
		free(pv);
		*pTokenInfoOut	= 0;
		bRetCode		= FALSE;
	}
	else
	{
		*pTokenInfoOut = pv;
		
		if (pdwOutSize)
			*pdwOutSize = dwSize;

		bRetCode = TRUE; 
	}

	return bRetCode;
}

void FreeTokenInfo(PVOID pTokenInfo)
{
	free(pTokenInfo);
}

BOOL GetUserNameFromSid(PSID pSid, TCHAR* szName, DWORD* pdwNameSize)
{
	if (NULL == pSid || NULL == pdwNameSize)
		return FALSE;

	SID_NAME_USE SNU;
	TCHAR szDomName[255] = { 0 };
	DWORD dwDomSize      = sizeof(szDomName) / sizeof(TCHAR);
	return LookupAccountSid(NULL, pSid, szName, pdwNameSize, szDomName, &dwDomSize, &SNU);
}

BOOL GetUserSidFromUserName(LPCTSTR pszUserName, PSID* ppSid)
{
	if (NULL == pszUserName || NULL == ppSid)
		return FALSE;

	BOOL	bResult			= FALSE;
	SID*	pSid			= NULL;
	TCHAR*	pszDomainName	= NULL;
	DWORD	dwSidLen		= 0;
	DWORD	dwDomainNameLen = 0;

	SID_NAME_USE SNU;
	// 查找顺序
	// well-known, built-in, local-machine, primary-domain, trusted-domain
	while(FALSE == (bResult = 
		LookupAccountName(NULL, pszUserName, pSid, &dwSidLen, 
		pszDomainName, &dwDomainNameLen, &SNU)))
	{
		if(pSid) 
		{
			free(pSid);
			pSid = NULL;
		}

		if(pszDomainName) 
		{
			free(pszDomainName);
			pszDomainName = NULL;
		}

		if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			break;

		pSid			= (SID*)malloc(dwSidLen);
		pszDomainName	= (TCHAR*)malloc(dwDomainNameLen * sizeof(TCHAR));
	}

	free(pszDomainName);

	if (bResult)
		*ppSid = pSid;

	return bResult;
}

void BuildExplicitAccessWithSid(PEXPLICIT_ACCESS explicitAccessP, 
								PSID trusteeSidP,
								DWORD accessPerm, 
								ACCESS_MODE accessMode,
								DWORD inheritance)
{
	if (explicitAccessP != NULL) 
	{
		explicitAccessP->grfAccessPermissions	= accessPerm;
		explicitAccessP->grfAccessMode			= accessMode;
		explicitAccessP->grfInheritance			= inheritance;
		BuildTrusteeWithSid(&explicitAccessP->Trustee, trusteeSidP);
	}
}

HANDLE CreateAdminToken(HANDLE hBasicToken, 
						PTOKEN_PRIVILEGES pTokenPrivileges, 
						LPCTSTR pszUserName,
						int* pnErrCode)
{
	if (NULL == hBasicToken)
	{
		if (pnErrCode)
			*pnErrCode = -4;

		return NULL;
	}

	int nErrCode	= 0;
	HANDLE	hToken	= NULL;
	PSID	pSid	= NULL;

	if (pszUserName && !GetUserSidFromUserName(pszUserName, &pSid))//获取指定用户的sid
	{
		nErrCode = -5;
		goto Exit0;
	}

	hToken = CreateAdminTokenWithSid(hBasicToken, pTokenPrivileges, pSid, &nErrCode);

Exit0:

	if (pnErrCode)
		*pnErrCode = nErrCode;

	if (pSid)
		free(pSid);

	return hToken;
}



// BOOL GetFirstAdminAccountSid(PSID* ppAdminAccountSid)
// {
// 	unsigned char* pvInfo = NULL;
// 	DWORD dwEntriesread		= 0;
// 	DWORD dwTotalentries	= 0;
// 
// 	NetLocalGroupGetMembers(NULL, 
// 							L"Administrators", 
// 							0, 
// 							&pvInfo, 
// 							MAX_PREFERRED_LENGTH, 
// 							&dwEntriesread, 
// 							&dwTotalentries,
// 							NULL);
// 
// 	if (NULL == pvInfo)
// 		return FALSE;
// 
// 	LOCALGROUP_MEMBERS_INFO_0* pMemberInfo = (LOCALGROUP_MEMBERS_INFO_0*)pvInfo;
// 
// 	GetLengthSid(pMemberInfo[0].lgrmi0_sid);
// 	NetApiBufferFree(pvInfo);
// 	return TRUE;
// }

// 当前进程一定要有SeCreateToken特权,在windows2003下,Administrator默认没有被委派这个权限,
// 所以当在windows2003下,用户进程无法实际获得SeCreateToken特权,可以通过
// 本地安全策略/用户权利指派中设置
// SE_CREATE_TOKEN_NAME       - Create a token object
// SE_ASSIGNPRIMARYTOKEN_NAME - Replace a process level token
// 然后调用EnableProcessPrivilege(GetCurrentProcess(), SE_CREATE_TOKEN_NAME, TRUE)进行权限提升
// 在Windows2003和Vista下,Service.exe的SeCreateToken特权被删除,
// 所以可以采用获取csrss.exe的token然后进行模拟后,再创建token
HANDLE CreateAdminTokenWithSid(HANDLE hBasicToken, 
							   PTOKEN_PRIVILEGES pTokenPrivileges, 
							   PSID pUserSid,
							   int* pnErrCode)
{
	if (NULL == hBasicToken)
	{
		if (pnErrCode)
			*pnErrCode = -4;

		return NULL;
	}

	int nErrCode							= 0;
	HANDLE				hToken				= NULL;
	TOKEN_USER			Token_User			= { 0 };
	LUID				AuthId				= SYSTEM_LUID;
	SECURITY_QUALITY_OF_SERVICE	Sqos		= { sizeof(Sqos), SecurityImpersonation, SECURITY_STATIC_TRACKING, FALSE };
	OBJECT_ATTRIBUTES	ObjAttr				= { sizeof(ObjAttr), 0, 0, 0, 0, &Sqos };
	TOKEN_SOURCE		TokenSrc			= {{'U', 's', 'e', 'r', '3', '2', '0', '0'}, {0, 0}};

//	SID_STRUCT			SystemSid			= _SystemUserSid;
	SID_STRUCT			MandatoryLevelSid	= _HighMandatoryLevelSid;//vista下管理员权限下需要这个组
	SID_STRUCT_2		OwnerGroupSid		= _AdministratorsGropuSid;//Token的拥有者设置为管理员组
	
	PACL				pAcl				= NULL;
	PACL				pNewAcl				= NULL;
	DWORD				dwAclSize			= 0;

	TOKEN_DEFAULT_DACL	TokanDacl			= { 0 };
	TOKEN_OWNER			Token_Owner			= { 0 };

	LARGE_INTEGER		ExpirationTime		= { 0 };

	EXPLICIT_ACCESS		ExplicitAccess		= { 0 };

	PTOKEN_STATISTICS	pTokenStatistics			= NULL;
	PTOKEN_PRIMARY_GROUP pBasicTokenPrimaryGroup	= NULL;
	PTOKEN_USER			pBasicTokenUser				= NULL;
	PTOKEN_PRIVILEGES	pAllocTokenPrivileges		= NULL;

	BOOL				bIsHaveAdminGroup			= FALSE;
	BOOL				bIsVistaSystem				= FALSE;
	unsigned char*		pGroupMemberInfoBuf			= NULL;
	LOCALGROUP_MEMBERS_INFO_0* pGroupMemberInfo		= NULL;

	PTOKEN_GROUPS		pBasicTokenGroups			= NULL;
	PTOKEN_GROUPS		pAllocTokenGroups			= NULL;
	PTOKEN_GROUPS		pTokenGroups				= NULL;

	PTOKEN_DEFAULT_DACL pSelfDacl					= NULL;

	PSID pFirstAdminSid = NULL;

	DWORD dwUsersGroupId	= static_cast<DWORD>(-1);
	DWORD	i				= 0;

	//PTOKEN_DEFAULT_DACL pTokenDefaultDacl	= NULL;
	//PTOKEN_OWNER		pTokenOwner			= NULL;

	DWORD	dwPlatformId	= 0;
	DWORD	dwMajorVersion	= 0;
	DWORD	dwMinorVersion	= 0;
	DWORD	dwCreateResult	= 0;
	DWORD	dwLastError		= 0;
	HMODULE	hNtdllMod		= NULL;
	pfnZwCreateTokenType			pfnZwCreateToken			= NULL;
	pfnRtlNtStatusToDosErrorType	pfnRtlNtStatusToDosError	= NULL;

	//获取ZwCreateToken函数指针
	hNtdllMod					= GetModuleHandle(TEXT("ntdll.dll"));
	pfnZwCreateToken			= (pfnZwCreateTokenType)GetProcAddress(hNtdllMod, "ZwCreateToken");
	pfnRtlNtStatusToDosError	= (pfnRtlNtStatusToDosErrorType)GetProcAddress(hNtdllMod, "RtlNtStatusToDosError");

	if(pfnZwCreateToken == 0 || pfnRtlNtStatusToDosError == 0)
	{
		if (pnErrCode)
			*pnErrCode = -6;

		return NULL;
	}

	//获取windows版本

	GetWindowsVersion(dwPlatformId, dwMajorVersion, dwMinorVersion);

	//设置Token的所有者
	Token_Owner.Owner = &OwnerGroupSid;

	//if (!GetTokenInfo(hBasicToken, TokenOwner, (PVOID*)&pTokenOwner, NULL))
	//	goto Exit0;

	//if (!GetTokenInfo(hBasicToken, TokenDefaultDacl, (PVOID*)&pTokenDefaultDacl, NULL))
	//	goto Exit0;

	//从basictoken中拷贝信息
	if (!GetTokenInfo(hBasicToken, TokenStatistics, (PVOID*)&pTokenStatistics, NULL))
	{
		nErrCode = -7;
		goto Exit0;
	}

	if (!GetTokenInfo(hBasicToken, TokenGroups, (PVOID*)&pBasicTokenGroups, NULL))
	{
		nErrCode = -8;
		goto Exit0;
	}

	if (!GetTokenInfo(hBasicToken, TokenPrimaryGroup, (PVOID*)&pBasicTokenPrimaryGroup, NULL))
	{
		nErrCode = -9;
		goto Exit0;
	}

	//如果没有指定账号,那么拷贝basic token中使用者为账号
	if (NULL == pUserSid && 
		!GetTokenInfo(hBasicToken, TokenUser, (PVOID*)&pBasicTokenUser, NULL))
		goto Exit0;

	//如果外部没有指定特权,那么拷贝basic token中的特权
	if (NULL == pTokenPrivileges && 
		!GetTokenInfo(hBasicToken, TokenPrivileges,	(PVOID*)&pAllocTokenPrivileges, NULL))
	{
		nErrCode = -10;
		goto Exit0;
	}

	if (NULL == pTokenPrivileges)
	{
		pTokenPrivileges = pAllocTokenPrivileges;
	}

	ExpirationTime	= pTokenStatistics->ExpirationTime;

	bIsVistaSystem = IsVistaSystem(dwPlatformId, dwMajorVersion, dwMinorVersion);

	for (i = 0; i < pBasicTokenGroups->GroupCount; ++i)
	{
		if (bIsVistaSystem)//如果是viata系统
		{
			//修改用于UAC控制的sid
			if (IsLowMandatoryLevelSid(pBasicTokenGroups->Groups[i].Sid))
				pBasicTokenGroups->Groups[i].Sid = &MandatoryLevelSid;
			else if (IsMediumMandatoryLevelSid(pBasicTokenGroups->Groups[i].Sid))
				pBasicTokenGroups->Groups[i].Sid = &MandatoryLevelSid;
			else if (IsSystemMandatoryLevelSid(pBasicTokenGroups->Groups[i].Sid))
				pBasicTokenGroups->Groups[i].Sid = &MandatoryLevelSid;
		}

		if (IsUsersGroupSid(pBasicTokenGroups->Groups[i].Sid))
		{
			dwUsersGroupId = i;
		}

		if (IsAdministratorGroupSid(pBasicTokenGroups->Groups[i].Sid))
		{
			bIsHaveAdminGroup = TRUE;
			pBasicTokenGroups->Groups[i].Attributes = 
			SE_GROUP_MANDATORY | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_ENABLED | SE_GROUP_OWNER;
		}
	}//for

	//如果不是Vista系统及以后的系统,则要获取一下当前的AuthId
	//如果basictoken中没有管理员组,那么是标准用户的token,则要获取一下当前的AuthId
	if (!bIsVistaSystem || !bIsHaveAdminGroup)  
	{
		AuthId			= pTokenStatistics->AuthenticationId;
	}
	//if (-1 == dwUsersGroupId)
	//{
	//	nErrCode = -11;
	//	goto Exit0;//没有users组
	//}


	/*
	if (!bIsHaveAdminGroup)//如果basictoken中没有管理员组,那么是标准用户的tokan
	{
		DWORD dwEntriesread		= 0;
		DWORD dwTotalentries	= 0;
		NetLocalGroupGetMembers(NULL, 
			L"Administrators", 
			0, 
			&pGroupMemberInfoBuf, 
			MAX_PREFERRED_LENGTH, 
			&dwEntriesread, 
			&dwTotalentries,
			NULL);

		if (pGroupMemberInfoBuf)
		{
			pGroupMemberInfo = (LOCALGROUP_MEMBERS_INFO_0*)pGroupMemberInfoBuf;
			pFirstAdminSid = pGroupMemberInfo[0].lgrmi0_sid;//取管理员组的第一个成员,一般情况下是Administrator
		}
		
		DWORD dwNewGroupSize = sizeof(TOKEN_GROUPS) + (sizeof(SID_AND_ATTRIBUTES) * pBasicTokenGroups->GroupCount);
		pAllocTokenGroups = PTOKEN_GROUPS(malloc(dwNewGroupSize));
		ZeroMemory(pAllocTokenGroups, dwNewGroupSize);
		pAllocTokenGroups->GroupCount = pBasicTokenGroups->GroupCount + 1;
		DWORD j = 0;
		for (i = 0, j = 0; i < pBasicTokenGroups->GroupCount; ++i, ++j)
		{
			if (i != dwUsersGroupId)
			{
				pAllocTokenGroups->Groups[j].Sid = pBasicTokenGroups->Groups[i].Sid;
				pAllocTokenGroups->Groups[j].Attributes = pBasicTokenGroups->Groups[i].Attributes;
			}
			else
			{
				//在原来token的users组信息的前面加入一个Administrators组信息
				pAllocTokenGroups->Groups[j].Sid = &OwnerGroupSid;
				pAllocTokenGroups->Groups[j].Attributes = 
					SE_GROUP_MANDATORY | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_ENABLED | SE_GROUP_OWNER;

				++j;
				pAllocTokenGroups->Groups[j].Sid = pBasicTokenGroups->Groups[i].Sid;
				pAllocTokenGroups->Groups[j].Attributes = pBasicTokenGroups->Groups[i].Attributes;
			}
		}

		pTokenGroups = pAllocTokenGroups;
	}
	else
	{
	*/
	pTokenGroups = pBasicTokenGroups;
	//}

	Token_User.User.Attributes	= 0;

	if (pUserSid)
	{
		Token_User.User.Sid = pUserSid;//获取指定用户的sid
	}
	else if (pFirstAdminSid)
	{
		Token_User.User.Sid = pFirstAdminSid;
	}
	else
	{
		Token_User.User.Sid = pBasicTokenUser->User.Sid;//如果没有指定帐户,那么使用基本token的所有者为帐号 
	}


	
	//////////////////////////////////////////////////////////////////////////
	BuildExplicitAccessWithSid(&ExplicitAccess, Token_User.User.Sid, GENERIC_ALL,GRANT_ACCESS, NO_INHERITANCE);
	if (!GetTokenInfo(hBasicToken, TokenDefaultDacl, (PVOID*)&pSelfDacl, NULL))
	{
		nErrCode = -21;
		goto Exit0;
	}

	if (ERROR_SUCCESS != SetEntriesInAcl(1, &ExplicitAccess,pSelfDacl->DefaultDacl, &pNewAcl))
	{
		nErrCode = -22;
		goto Exit0;
	}

	TokanDacl.DefaultDacl = pNewAcl;
	//////////////////////////////////////////////////////////////////////////
	/*
	//////////////////////////////////////////////////////////////////////////
	//对于DACL,最好的做法是把基本token的DACL和要设置的内容进行合并,不过稍嫌麻烦就偷个懒
	PSID pSid = &OwnerGroupSid;//pBasicTokenUser->User.Sid;
	if (IsVistaSystem(dwPlatformId, dwMajorVersion, dwMinorVersion)) //vista system
	{
		dwAclSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) * 3 +
			GetLengthSid(pSid) + GetLengthSid(&SystemSid) + GetLengthSid(&SystemSid);

		pAcl = (PACL)malloc(dwAclSize);
		ZeroMemory(pAcl, dwAclSize);
		InitializeAcl(pAcl, dwAclSize, ACL_REVISION);
		AddAccessAllowedAce(pAcl, ACL_REVISION,  GENERIC_ALL, pSid);
		AddAccessAllowedAce(pAcl, ACL_REVISION,  GENERIC_ALL, &SystemSid);
		AddAccessAllowedAce(pAcl, ACL_REVISION,  GENERIC_EXECUTE | GENERIC_READ, &SystemSid);
	}
	else
	{
		dwAclSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) * 2 +
			GetLengthSid(pSid) + GetLengthSid(&SystemSid);

		pAcl = (PACL)malloc(dwAclSize);
		ZeroMemory(pAcl, dwAclSize);
		InitializeAcl(pAcl, dwAclSize, ACL_REVISION);
		AddAccessAllowedAce(pAcl, ACL_REVISION,  GENERIC_ALL, pSid);
		AddAccessAllowedAce(pAcl, ACL_REVISION,  GENERIC_ALL, &SystemSid);
	}
	TokanDacl.DefaultDacl = pAcl;
	*/

	if(!AllocateLocallyUniqueId(&TokenSrc.SourceIdentifier))
	{
		nErrCode = -12;
		goto Exit0;
	}

	dwCreateResult = pfnZwCreateToken(
		&hToken, 
		TOKEN_ALL_ACCESS, 
		&ObjAttr, 
		TokenPrimary,
		(PLUID)&AuthId, 
		(PLARGE_INTEGER)&ExpirationTime,
		&Token_User,
		pTokenGroups,
		pTokenPrivileges,
		&Token_Owner,//pTokenOwner,
		pBasicTokenPrimaryGroup,
		&TokanDacl,//pTokenDefaultDacl,
		&TokenSrc);

	dwLastError = pfnRtlNtStatusToDosError(dwCreateResult);
	SetLastError(dwLastError);

	if (NULL == hToken)
	{
		nErrCode = dwLastError;
	}

Exit0:

	if (pnErrCode)
		*pnErrCode = nErrCode;

	if (pAcl)
		free(pAcl);

	if (pAllocTokenGroups)
		free(pAllocTokenGroups);

	if (pGroupMemberInfoBuf)
		NetApiBufferFree(pGroupMemberInfoBuf);//对应NetLocalGroupGetMembers的调用

	if (pTokenStatistics)
		FreeTokenInfo(pTokenStatistics);
	
	if (pBasicTokenGroups)
		FreeTokenInfo(pBasicTokenGroups);

	if (pBasicTokenPrimaryGroup)
		FreeTokenInfo(pBasicTokenPrimaryGroup);

	if (pBasicTokenUser)
		FreeTokenInfo(pBasicTokenUser);

	if (pAllocTokenPrivileges)
		FreeTokenInfo(pAllocTokenPrivileges);

	if (pSelfDacl)
		FreeTokenInfo(pSelfDacl);

	//if (pTokenDefaultDacl)
	//	FreeTokenInfo(pTokenDefaultDacl);

	//if (pTokenOwner)
	//	FreeTokenInfo(pTokenOwner);

	if (pNewAcl)
		::LocalFree(pNewAcl);

	return hToken;
}
