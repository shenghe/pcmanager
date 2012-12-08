//=============================================================================
/**
* @file GetTokenUtil.h
* @brief 
* @author qiuruifeng <qiuruifeng@kingsoft.com>
* @date 2008-5-22   16:37
*/
//=============================================================================
#ifndef GETTOKENUTIL_H
#define GETTOKENUTIL_H

#include <Windows.h>

/**
 * @brief 通过进程映象名获取进程id
 * @remark 没有考虑多用户登录的情况,如果需要考虑,可以通过该进程id获取token,然后获取session信息
 * @param[in] lpName 进程映象名
 * @param[out] dwPid 输出的进程id
 * @return 成功返回TURE,失败返回FALSE
 */
BOOL GetPidByName(LPTSTR lpName, DWORD& dwPid);

// 得到Active Session ID. 成功则返回TRUE, whereas return FALSE if failed.
BOOL GetActiveSessionID(DWORD& sessionId);

// 从某个Session中得到一个pid. 成功则返回TRUE,失败则返回FALSE
BOOL GetOnePidFromSession(DWORD& dwPid,  DWORD dwSessionId);

/**
 * @brief 从pid中获取token并复制
 * @remark 
 * @param[in] dwPid 
 * @param[out] hToken 
 * @return 
 */
BOOL GetPrimaryTokenFromPid(DWORD dwPid, HANDLE& hToken);

/**
 * @brief 从pid中获取token并复制
 * @remark 
 * @param[in] dwPid 
 * @param[out] hToken 
 * @return 
 */
BOOL GetImpersonationTokenFromPid(DWORD dwPid, HANDLE& hToken);

#endif //GETTOKENUTIL_H
