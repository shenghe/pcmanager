//////////////////////////////////////////////////////////////////////
///		@file		ikxepipeserver.h
///		@author		luopeng
///		@date		2009-3-17 09:07:58
///
///		@brief		命名管理封装的服务器实现
///		
//////////////////////////////////////////////////////////////////////

#pragma once
#include <Unknwn.h>
#include "../kxeipc/ikxedispatchmanage.h"

/**
 * @defgroup kxe_pipe_interface_define KXEngine Pipe Define in Proudct Manage 
 * @{
 */

/**
 * @brief 命名管理的服务管理者
 */
MIDL_INTERFACE("EC6F568C-0DA2-4539-B474-52A9A20553CE")
IKxEPipeServer : public IUnknown
{
	/**
	* @brief 启动以lpszPipeName命名的管道，并记录pCallback回调进行处理
	* @param[in] lpszPipeName 启动时使用的命名管道
	* @param[in] pDispatchManage 用于管理 SP 相关信息的类指针
	*/
	virtual int __stdcall Start(
		LPCWSTR lpszPipeName,
		IKxEDispatchManage* pDispatchManage
		) = 0;

	/**
	 * @brief 停止该服务
	 */
	virtual int __stdcall Stop() = 0;
};

/**
 * @}
 */
