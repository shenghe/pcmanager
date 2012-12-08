//////////////////////////////////////////////////////////////////////
///		@file		kxeproduct_id.h
///		@author		zhaoxinxing
///		@date		2010-9-7
///
///		@brief		产品相关ID的定义
//////////////////////////////////////////////////////////////////////

#pragma once

/**
 * @defgroup kxe_product_id_group KXEngine Product ID
 * @{
 */

/**
 * 产品ID直接为16位，分别为高8位，与低8位
 * 0                  7                  15
 * ---------------------------------------
 * |   Product_Type   |  Product_Version |
 * ---------------------------------------
 */
//金山卫士 清理模块
#define KXE_NEW_KSAFE_KCLEAR_PRODUCT_TYPE  0x111000


/**
 * @}
 */
