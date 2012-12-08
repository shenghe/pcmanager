//////////////////////////////////////////////////////////////////////
///		@file		kxeserviceutility.h
///		@author		luopeng
///		@date		2008-12-18 09:07:58
///
///		@brief		将服务相关函数提出
//////////////////////////////////////////////////////////////////////

#pragma once
#include <Windows.h>

/**
 * @defgroup kxe_service_utility KXEngine Service Utility
 * @{
 */

/**
 * @brief 安装服务
 * @param[in] pszPath 服务的二进制文件路径
 * @param[in] pszInternalName 服务名，用于启停
 * @param[in] pszServiceName 服务描述名
 * @param[in] pszDescriptiveName 服务的描述
 * @param[in] pSvcGroupOrder 加载的顺序
 * @return 0 成功， 其他为失败
 */
int KxEInstallService(
	TCHAR* pszPath, 
	TCHAR* pszInternalName, 
	TCHAR* pszServiceName,
	TCHAR* pszDescriptiveName,
	TCHAR* pSvcGroupOrder
	);

/**
 * @brief 卸载服务
 * @param[in] pszInternalName 服务名，用于启停 
 * @return 0 成功， 其他为失败
 */
int KxEUninstallService(
	TCHAR *pszInternalName
	);

/**
 * @brief 启动服务
 * @param[in] pszInternalName 服务名，用于启停 
 * @param[in] bWaitRunning 是否等待至服务启动完成后返回
 * @return 0 成功， 其他为失败
 */
int KxEStartService(
	TCHAR *pszInternalName,
	bool bWaitRunning
	);

/**
 * @brief 停止服务
 * @param[in] pszInternalName 服务名，用于启停 
 * @param[in] bWaitStopped 是否等待至服务停止完成后返回
 * @return 0 成功， 其他为失败
 */
int KxEStopService(
	TCHAR *pszInternalName,
	bool bWaitStopped
	);


/**
* @defgroup kxe_service_utility KXEngine Service Utility
* @{
*/

/**
* @brief 安装交互式服务
* @param[in] pszPath 服务的二进制文件路径
* @param[in] pszInternalName 服务名，用于启停
* @param[in] pszServiceName 服务描述名
* @param[in] pszDescriptiveName 服务的描述
* @param[in] pSvcGroupOrder 加载的顺序
* @return 0 成功， 其他为失败
*/
int KxEInstallServiceInteractive(
					  TCHAR* pszPath, 
					  TCHAR* pszInternalName, 
					  TCHAR* pszServiceName,
					  TCHAR* pszDescriptiveName,
					  TCHAR* pSvcGroupOrder
					  );


#include "kxeserviceutility.inl"

/**
 * @}
 */

