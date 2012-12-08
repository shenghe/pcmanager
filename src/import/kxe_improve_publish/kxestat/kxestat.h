//////////////////////////////////////////////////////////////////////
///		@file		kxestat.h
///		@author		luopeng
///		@date		2008-9-10 09:07:58
///
///		@brief		查杀系统的SDK提供
//////////////////////////////////////////////////////////////////////

#pragma once
#include "../include/kxebase_plugin.h"
#include "../include/kxe.h"

/**
 * @defgroup kxestat_subsystem_interface_group KXEngine Activity Statical Subsystem
 * @{
 */

/**
 * @brief 通知服务器，当前产品的安装状态
 * @param[in] pnProductIdArray 当前安装产品的列表，将按照安装时间及捆绑关系排序
 * @param[in] uIdCount pnProductIdArray的数量
 * @return 0 成功，其他为失败错误码
 */
KXEAPI
int __stdcall kxe_stat_notify_product_setup(
	int* pnProductIdArray,
	unsigned int uIdCount
	);

/**
 * @brief 通知服务器，大引擎活跃了，并报上当前安装的所有产品
 * @param[in] pnProductIdArray 当前安装产品的列表，将按照安装时间及捆绑关系排序
 * @param[in] uIdCount pnProductIdArray的数量
 * @return 0 成功，其他为失败错误码
 */
KXEAPI
int __stdcall kxe_stat_notify_kxengine_activity(
	int* pnProductIdArray,
	unsigned int uIdCount
	);

/**
 * @brief 通知服务器，某个产品活跃了
 * @param[in] nProductId 活跃的产品ID
 * @return 0 成功，其他为失败错误码
 */
KXEAPI
int __stdcall kxe_stat_notify_product_activity(
	int nProductId
	);

/**
 * @}
 */
