//=============================================================================
/**
* @file CreateToken.h
* @brief 
* @author qiuruifeng <qiuruifeng@kingsoft.com>
* @date 2008-5-20   10:17
*/
//=============================================================================
#ifndef CREATETOKEN_H
#define CREATETOKEN_H

#include <windows.h>
#include <tchar.h>

/**
 * @brief 创建有管理员权限的token,当前进程一定要有SeCreateToken特权
 * @remark 如果没有管理员帐号的token,那么可以参考内部实现,自行精心构造sid组相关信息
 * @param[in] hBasicToken 基本token(需要是管理员账号,如),会从该token的sid组和使用者sid
 * @param[in] pTokenPrivileges token的特权数据,如果为空那么从hBasicToken中拷贝
 * @param[in] pszUserName 管理员帐户名,如果为空,那么会从hBasicToken中获取相应sid
 * @return 成功返回非空的token,失败返回空token
 */
HANDLE CreateAdminToken(HANDLE hBasicToken, PTOKEN_PRIVILEGES pTokenPrivileges, LPCTSTR pszUserName, int* pnErrCode);

/**
* @brief 创建有管理员权限的token,当前进程一定要有SeCreateToken特权
* @remark 如果没有管理员帐号的token,那么可以参考内部实现,自行精心构造sid组相关信息
* @param[in] hBasicToken 基本token(需要是管理员账号,如),会从该token的sid组和使用者sid
* @param[in] pTokenPrivileges token的特权数据,如果为空那么从hBasicToken中拷贝
* @param[in] pUserSid 管理员帐户的sid,如果为空,那么会从hBasicToken中获取相应sid
* @return 成功返回非空的token,失败返回空token
*/
HANDLE CreateAdminTokenWithSid(HANDLE hBasicToken, PTOKEN_PRIVILEGES pTokenPrivileges, PSID pUserSid, int* pnErrCode);

/**
 * @brief 获取系统版本
 * @remark 
 * @param[out] dwPlatformId 平台id
 * @param[out] dwMajor 大版本
 * @param[out] dwMinor 小版本
 * @return 成功返回TURE,失败返回FALSE
 */
BOOL GetWindowsVersion(DWORD& dwPlatformId, DWORD& dwMajor, DWORD& dwMinor);

/**
 * @brief 从token中获取session id
 * @remark 
 * @param[in] hToken 传入令牌
 * @param[out] pdwSession session id指针
 * @return 成功返回TURE,失败返回FALSE
 */
BOOL GetSessionIdFromToken(HANDLE hToken, DWORD* pdwSession);

/**
 * @brief 给指定的token提升权限或降低权限
 * @remark 传入令牌需要TOKEN_ADJUST_PRIVILEGES访问权限
 * @param[in] hToken 传入令牌
 * @param[in] szPrivName 特权名
 * @param[in] bEnable 提升或降低权限标识
 * @return 成功返回TURE,失败返回FALSE
 */
BOOL EnableTokenPrivilege(HANDLE hToken, LPCTSTR szPrivName, BOOL bEnable = TRUE);

/**
 * @brief 给指定的进程提升权限或降低权限
 * @remark 
 * @param[in] hProcess 进程句柄
 * @param[in] szPrivName 特权名
 * @param[in] bEnable 提升或降低权限标识
 * @return 成功返回TURE,失败返回FALSE
 */
BOOL EnableProcessPrivilege(HANDLE hProcess, LPCTSTR szPrivName, BOOL bEnable = TRUE);

/**
 * @brief 给当前进程提升或降低权限
 * @remark 
 * @param szPrivName 特权名
 * @param bEnable 提升或降低权限标识
 * @return 成功返回TURE,失败返回FALSE
 */
BOOL EnableCurrentProcessPrivilege(LPCTSTR szPrivName, BOOL bEnable = TRUE);

/**
 * @brief 判断当前sid是否是vista下用于标识高权限的组sid
 * @remark 
 * @param[in] pSid 传入的sid
 * @return 如果是则返回TRUE
 */
BOOL IsLowMandatoryLevelSid(PSID pSid);

/**
* @brief 判断当前sid是否是vista下用于标识中权限的组sid
* @remark 
* @param[in] pSid 传入的sid
* @return 如果是则返回TRUE
*/
BOOL IsMediumMandatoryLevelSid(PSID pSid);

/**
* @brief 判断当前sid是否是vista下用于标识高权限的组sid
* @remark 
* @param[in] pSid 传入的sid
* @return 如果是则返回TRUE
*/
BOOL IsHighMandatoryLevelSid(PSID pSid);

/**
* @brief 判断当前sid是否是vista下用于标识系统权限的组sid
* @remark 
* @param[in] pSid 传入的sid
* @return 如果是则返回TRUE
*/
BOOL IsSystemMandatoryLevelSid(PSID pSid);

/**
 * @brief 判断当前sid是否是管理员组sid
 * @remark 
 * @param[in] pSid 传入的sid
 * @return 如果是则返回TRUE
 */
BOOL IsAdministratorGroupSid(PSID pSid);

BOOL IsUsersGroupSid(PSID pSid);

BOOL IsSystemUserSid(PSID pSid);
/**
* @brief 获取token信息
* @remark 
* @param[in] hToken 传入令牌
* @param[in] Tic 信息类型
* @param[out] pTokenInfoOut 信息内容,会在内部分配空间,需要使用FreeTokenInfo回收
* @param[out] pdwOutSize 输出内容的大小
* @return 成功返回TURE,失败返回FALSE
*/
BOOL GetTokenInfo(HANDLE hToken, TOKEN_INFORMATION_CLASS Tic, PVOID *pTokenInfoOut, DWORD* pdwOutSize);

/**
 * @brief 
 * @remark 回收GetTokenInfo输出的内容
 * @param[in] pTokenInfo GetTokenInfo输出的内容的指针
 */
void FreeTokenInfo(PVOID pTokenInfo);

inline BOOL IsNt4System(DWORD dwPlatformId, DWORD dwMajor, DWORD dwMinor)
{
	dwMinor;
	return (dwPlatformId == VER_PLATFORM_WIN32_NT && 4 == dwMajor);
}

inline BOOL IsWin2kSystem(DWORD dwPlatformId, DWORD dwMajor, DWORD dwMinor)
{
	return (dwPlatformId == VER_PLATFORM_WIN32_NT && 5 == dwMajor && 0 == dwMinor);
}

inline BOOL IsWinXpSystem(DWORD dwPlatformId, DWORD dwMajor, DWORD dwMinor)
{
	return (dwPlatformId == VER_PLATFORM_WIN32_NT && 5 == dwMajor && 1 == dwMinor);
}

inline BOOL IsWin2k3System(DWORD dwPlatformId, DWORD dwMajor, DWORD dwMinor)
{
	return (dwPlatformId == VER_PLATFORM_WIN32_NT && 5 == dwMajor && 2 == dwMinor);
}

inline BOOL IsVistaSystem(DWORD dwPlatformId, DWORD dwMajor, DWORD dwMinor)
{
	dwMinor;
	return (dwPlatformId == VER_PLATFORM_WIN32_NT && 6 == dwMajor);
}

#endif //CREATETOKEN_H