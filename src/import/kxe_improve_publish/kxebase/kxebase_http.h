//////////////////////////////////////////////////////////////////////
///		@file		kxebase_http.h
///		@author		luopeng
///		@date		2008-8-29 09:07:58
///	
///		@brief		kxebase中http相关函数定义
//////////////////////////////////////////////////////////////////////
#pragma once
#include "../kxeipc/ikxerequestdispatch.h"

/**
 * @defgroup kxehttp_sdk_interface_group KXEngine Architecture Base SDK HTTP Interface
 * @{
 */

/**
* @brief 向HTTP服务器注册一个uri回调
* @param[in] pszUri 为要注册回调用路径
* @param[in] pDispatch 为当HTTP服务接受到pszUri路径内容时的回调接口
* @remark 
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_register_uri_dispatch(
	const char* pszUri, 
	IKxERequestDispatch* pDispatch
	);

/**
* @brief 向HTTP服务器取消一个uri回调
* @param[in] pszUri 为要注册回调用路径
* @param[in] pDispatch 为当HTTP服务接受到pszUri路径内容时的回调接口
* @remark 
* @return 0 成功, 其他值为失败的错误码，
*/
int __stdcall kxe_base_unregister_uri_dispatch(
	const char* pszUri, 
	IKxERequestDispatch* pDispatch
	);

/**
 * @brief 仅仅停止HTTP服务，当且仅当成功调用kxe_base_start之后调用
 * @remark HTTP作为对外的数据接受端，不停止，将方便处理内部数据而导致崩溃
 * @return 0 成功， 其他为失败错误码
 */
int __stdcall kxe_base_stop_http();

/**
 * @brief 恢复HTTP服务，当且仅当kxe_bast_stop_http被调用之后，用于恢复HTTP的处理
 * @remark HTTP作为对外的数据接受端，不停止，将方便处理内部数据而导致崩溃
 * @return 0 成功， 其他为失败错误码
 */
int __stdcall kxe_base_resume_http();


/**
 * @brief 设置web路径的别名
 * @param[in] pszName 别名，放在alias\\pszName\\other_name上，现在该值一般使用ProductID
 * @param[in] pszDirectory 别名指向的目录，转换结果为pszDirectory\\other_name
 * @remark 当子目录中存在kxeweb时，其下一级目录可为.dat扩展名的zip压缩包，比如转换后\n
 * 结果为d:\\kingsoft\\kxeweb\\kavweb\\index.htm，它可以转换为\n
 * d:\\kingsoft\\kxeweb\\kavweb.dat中的index.htm文件
 * @return 0 成功，其他为失败错误码
 */
int __stdcall kxe_base_register_web_path(const char* pszName,
										 const char* pszDirectory);

/**
 * @brief 取水web路径的别名
 * @param[in] pszName 别名，放在alias\\pszName\\other_name上
 * @return 0 成功，其他为失败错误码
 */
int __stdcall kxe_base_unregister_web_path(const char* pszName);
/**
 * @}
 */
