//////////////////////////////////////////////////////////////////////
///		@file		kxebase_ipc.h
///		@author		luopeng
///		@date		2008-8-29 09:07:58
///	
///		@brief		kxebase中ipc相关函数定义
//////////////////////////////////////////////////////////////////////
#pragma once
#include "../kxeipc/ikxerequestdispatch.h"

/**
 * @defgroup kxeipc_sdk_interface_group KXEngine Architecture Base SDK IPC Interface
 * @{
 */

/**
* @brief 向IPC服务注册一个SP与回调的指针
* @param[in] pwszSPName 为要注册回调用路径
* @param[in] pDispatch 为当IPC服务接受到pszSPName路径内容时的回调接口
* @remark 
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_register_sp_dispatch(
	const wchar_t* pwszSPName,
	IKxERequestDispatch* pDispatch
	);

/**
* @brief 向HTTP服务器取消一个uri回调
* @param[in] pwszSPName 为要注册回调用路径
* @param[in] pDispatch 为当IPC服务接受到pszSPName路径内容时的回调接口
* @remark 
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_unregister_sp_dispatch(
	const wchar_t* pwszSPName, 
	IKxERequestDispatch* pDispatch
	);

/**
 * @brief 仅仅停止IPC服务，当且仅当成功调用kxe_base_start之后调用
 * @remark HTTP作为对外的数据接受端，不停止，将方便处理内部数据而导致崩溃
 * @return 0 成功， 其他为失败错误码
 */
int __stdcall kxe_base_stop_ipc();

/**
 * @brief 恢复IPC服务，当且仅当kxe_bast_stop_http被调用之后，用于恢复IPC的处理
 * @remark IPC作为对外的数据接受端，不停止，将方便处理内部数据而导致崩溃
 * @return 0 成功， 其他为失败错误码
 */
int __stdcall kxe_base_resume_ipc();

/**
 * @}
 */
