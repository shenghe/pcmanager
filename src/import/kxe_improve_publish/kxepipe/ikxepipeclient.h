//////////////////////////////////////////////////////////////////////
///		@file		ikxepipeclient.h
///		@author		luopeng
///		@date		2009-3-17 09:07:58
///
///		@brief		命名管理封装的客户端实现
//////////////////////////////////////////////////////////////////////

#pragma once
#include <Unknwn.h>
#include "../../Publish/kxehttp/ikxerequestdispatch.h"
#include "../kxeipc/kxedispatchmanage.h"
/*
 * @addtogroup kxe_pipe_interface_define
 * @{
 */

/**
 * @brief 命名管道服务的客户端
 */
MIDL_INTERFACE("B727E5E4-B953-4de7-96C9-14E289A5F271")
IKxEPipeClient : public IUnknown
{
	/**
	 * @brief 启动命名管道客户端
	 * @param[in] lpszClientPipeName 客户端命名管道名字，等待服务端发数据时连接
	 * @param[in] lpszServerPipeName 服务端命名管道名字
	 * @param[in] pDispatchManage 用于获取 IRequestDispatch 对象指针以调用其方法 Dispatch
	 * @return 0 成功，其他为失败错误码
	 */
	virtual int __stdcall Start(
		LPCWSTR lpszClientPipeName,
		LPCWSTR lpszServerPipeName,
		KxEDispatchManage *pDispatchManage
		) = 0;

	/**
	 * @brief 停止命名管道
	 * @return 0 成功，其他为失败错误码
	 */
	virtual int __stdcall Stop() = 0;
};

/**
 * @}
 */