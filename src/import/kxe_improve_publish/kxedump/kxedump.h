//////////////////////////////////////////////////////////////////////
///		@file		kxedump.h
///		@author		luopeng
///		@date		2008-12-13 09:07:58
///
///		@brief		dump加载接口
//////////////////////////////////////////////////////////////////////

#pragma once

/**
 * @defgroup kxedump_interface_group KXEngine Dump Interface
 * @{
 */

/**
 * @brief 初始化加载kxedump.dll，所有的异常将被接管
 * @remark 加此此dll后，如果需要注册异常处理函数，请调用kxe_base_register_unhandled_exception_filter,
 * kxe_base_unregister_unhandled_exception_filter
 * @see kxe_base_register_unhandled_exception_filter, kxe_base_unregister_unhandled_exception_filter
 * @return 0 成功，其他为失败错误码
 */
int InitializeDump();


#include "kxedump.inl"
/**
 * @}
 */
