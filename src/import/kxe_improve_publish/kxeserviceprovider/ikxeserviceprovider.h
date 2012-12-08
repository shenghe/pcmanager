//////////////////////////////////////////////////////////////////////
///		@file		ikxeserviceprovider.h
///		@author		luopeng
///		@date		2008-9-16 14:07:58
///
///		@brief		Service Provider的接口定义
//////////////////////////////////////////////////////////////////////

#pragma once
#include <Unknwn.h>

/**
 * @defgroup kxe_service_provider_interface_group KXEngine Service Provider Interface
 * @{
 */

/**
 * @brief Serivice Provider公共接口
 */
MIDL_INTERFACE("23031722-F263-4074-80FB-5F766AEC49C1")
IKxEServiceProvider : public IUnknown
{
	/**
	 * @brief 初始化Service Provider,由服务管理器kxeserv调用
	 * @return 0 为成功, 其它为失败
	 * @remark 如果有SDK，并且要初始化，则必须在此函数内就完成初始化动作，
	 *         同时在此函数内允许调用非框架内的SDK
	 */
	virtual int __stdcall InitializeService() = 0;

	/**
	* @brief 反初始化Service Provider,由服务管理器kxeserv调用
	* @return 0 为成功, 其它为失败
	* @remark 在此函数不允许再调用非框架内的SDK，同时要清理自己提供的SDK资源
	*/
	virtual int __stdcall UninitializeService() = 0;
	
	/**
	* @brief 启动Service Provider,由服务管理器kxeserv调用
	* @return 0 为成功, 其它为失败
	* @remark 在此函数内已经可以调用非框架内的SDK,启动成功后，服务管理器会对外发布事件
	*/
	virtual int __stdcall StartService() = 0;
	
	/**
	* @brief 关闭Service Provider,由服务管理器kxeserv调用
	* @return 0 为成功, 其它为失败
	* @remark 在此函数被调用后，所有的均不允许再调用非框架内的SDK。另外，
	*         也不允许在此反初始化SDK相关内容
	*/
	virtual int __stdcall StopService() = 0;	
};

/**
 * @}
 */
