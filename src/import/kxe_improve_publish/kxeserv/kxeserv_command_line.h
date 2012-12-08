//////////////////////////////////////////////////////////////////////
///		@file		kxeserv_command_line.h
///		@author		luopeng
///		@date		2009-3-11 09:07:58
///
///		@brief		命令行定义
//////////////////////////////////////////////////////////////////////

#pragma once
/**
 * @defgroup kxeserv_command_line KXEngine Commandline Define
 * @section kxeserv_command_line_install_service 服务安装
 * kxeserv /debug     调试状态的运行 \n
 * kxeserv /install   安装服务\n
 * kxeserv /uninstall 卸载服务\n
 * kxeserv /start     启动服务\n
 * kxeserv /stop      停止服务\n
 * kxeserv /start_until_running 直到服务完全启动后才返回\n
 * kxeserv /stop_until_stopped 直接服务完全停止后才返回\n
 * @section kxeserv_command_line_install_product  产品安装
 * kxeserv /install_product_by_file [config_file_name]   安装产品，若安装过了会失败\n
 * kxeserv /reinstall_product_by_file [config_file_name] 再次安装，若安装过了也会再次安装\n
 * kxeserv /uninstall_product_by_file [config_file_name] 卸载产品\n
 * @section kxeserv_command_line_install_product_config_file 配置文件格式
 * 其中所有带路径的部分，除了Path这个字段外，其他字段都可以是相对路径，相对的是Path部分
 * 比如如下的File0=test.dll，其完整路径就是d:\\kingsoft\\test.dll
 * @code 
 *

[Product]
ID=0x10000000
Parent=0x20000000
Recommend=
Path=d:\kingsoft
PluginPath=plugin // 完整路径将使用Path作为根路径，plugin是相对路径，结果就是d:\kingsoft\plugin
WebPath=web       // 完整路径将使用Path作为根路径，web是相对路径，结果就是d:\kingsoft\web

[ChildProduct]
Count=1
Product0=0x30000000

[Component]
Count=1
File0=test.dll  // 完整路径将使用Path作为根路径，test.dll是相对路径，结果就是d:\kingsoft\test.dll

[ServiceProvider]
Count=1
File0=test.dll  // 完整路径将使用Path作为根路径，test.dll是相对路径，结果就是d:\kingsoft\test.dll

 *
 * @endcode
 */
