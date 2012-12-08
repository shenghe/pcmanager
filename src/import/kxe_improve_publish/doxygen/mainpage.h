//////////////////////////////////////////////////////////////////////
///		@file		mainpage.h
///		@author		luopeng
///		@date		2008-9-25 09:07:58
///	
///		@brief		doxygen的主文件
//////////////////////////////////////////////////////////////////////

#pragma once

/**
  @mainpage KXEngine Architecture

  @section kxengine_architecture_interface 1. 架构接口描述
  - @subpage kxe_ipc_interface "IPC调用相关接口"
   - @ref kxeipc_service_provider_interface_group
   - @ref kxeipc_idl_interface_group
   - @ref kxepipe_idl_interface_group
   - @ref kxehttp_idl_interface_group

  - @subpage architecture_product_manage_interface "服务框架产品管理接口"
   - @ref kxeservice_manager_service_provider_interface_group "产品管理的HTTP-RPC接口"
   - @ref kxeserv_command_line "产品管理的命令行接口"

  - @subpage architecture_interface "架构接口"
   - @ref kxebase "架构基础接口SDK"
   - @ref kxebase_plugin_sdk "子系统SDK扩展接口"
   - @ref data_struct "数据结构定义"
   - @ref json_data_struct "Json相关数据结构定义"
   - @ref kxe_file_register_interface "架构相关组件及服务提供者安装接口"

  - @subpage architecture_core_component_interface "架构核心组件接口"
   - @ref kxethread_component_interface  "线程管理组件接口"
   - @ref kxeconfig_component_interface  "配置管理组件接口"
   - @ref kxeevent_component_interface   "事件管理组件接口"
   - @ref kxe_pipe_interface_define "进程分离通讯接口"

  - @subpage architecture_wow64_interface "架构针对64位的兼容处理"
   - @ref kxereg_wow64_interface "注册表访问部分"
   - @ref kxefile_wow64_interface "文件访问部分"

  - @subpage architecture_src_interface "架构辅助功能接口"
   - @ref kxebase_src_thread_group "线程相关辅助函数及接口"
   - @ref kxearchitecture_md5_group "MD5计算接口"

  - @subpage example "架构接口使用示例"
   - @ref kxeevent_example "事件使用示例"
   - @ref kxeconfig_example "配置使用示例"
   - @ref kxehttp_example "HTTP-RPC使用示例"
   - @ref kxeservice_provider_example "服务提供者创建示例"
   - @ref kxeplugin_sdk_example "子系统SDK扩展定义示例"

  - @subpage architecture_service_provider_interface "架构服务提供者基础接口"
   - @ref kxe_service_provider_interface_group
   - @ref kxe_service_provider_clsid_group
   - @ref kxe_service_provider_export_group

 @section kxengine_starfish_plan_application 2. 海星计划
  - @subpage kxengine_starfish_product_application

 @section kxengine_product_application 3. 产品应用
  - @subpage kxengine_highperf_product_application "极速版应用"
*/

/**
 * @page kxengine_starfish_product_application KXEngine in Starfish Product
  @section kxengine_starfish_architecture 1. 架构结构图
   @image html starfish_arch.png

  @section kxengine_common_component 1. 公共组件接口定义
   - @subpage kxeservice_manager_define "服务管理器接口定义"
    - @ref kxeservice_manager_service_provider_interface_group
    - @ref kxeservice_manager_event_define_group
    - @ref kxeservice_manager_data_struct_group
    - @ref kxeservice_manager_config_data_struct_group
    - @ref kxeserv_command_line
  
  @section kxengine_security_center_component 2. 安全中心接口定义
   - @ref kxengine_starfish_security_center_architecture
   - @ref kxe_security_center_service_provider_interface_group
   - @ref kxe_security_center_data_struct
   - @ref kxe_sc_tray_interface_group
   - @ref kxepassport_service_provider_interface_star_fish_group
   - @ref kxe_security_center_downloader_service_provider_interface_group
   - @ref kxe_security_center_installer_service_provider_interface_group

  @section kxengine_security_core_component 2. 安全核心组件接口定义
   - @subpage kxescan_subsystem_define "查杀子系统服务接口及SDK定义"
    - @ref kxescan_subsystem_interface_group
    - @ref kxescan_subsystem_data_struct
    - @ref kxescan_service_provider_interface_group
    - @ref kxescan_event_define_group

   - @subpage kxewhite_subsystem_define "白名单子系统服务接口及SDK定义"
    - @ref kxewhite_subsystem_interface_group
    - @ref kxewhite_subsystem_data_struct_group
    - @ref kxewhite_service_provider_interface_group
    - @ref kxewhite_event_define_group

   - @subpage kxefilemon_subsystem_define "文件监控子系统服务接口及SDK定义"
    - @ref kxefilemon_subsystem_interface_group
    - @ref kxefilemon_subsystem_data_struct_group
    - @ref kxefilemon_service_provider_interface_group
    - @ref kxefilemon_event_define_group
    - @ref kxefilemon_subsystem_config_group

   - @subpage kxedefendengine_subsystem_define "防御引擎服务接口及SDK定义"
    - @ref kxedefendengine_service_provider_interface_group
  
  @section kxengine_security_application_component 3. 安全应用组件接口定义
   - @subpage kxe_popo_subsytem_define_2 "泡泡服务接口定义"
    - @ref kxe_pop_server_service_provider_interface_group
    - @ref kxe_pop_server_data_define_group

  @section kxengine_security_product_component 4. 安全产口组件接口定义
   - @subpage kxenameinfo_subsystem_define "病毒名查询服务接口定义"
    - @ref kxe_virus_name_info_service_provider_interface_group
	- @ref kxe_virus_name_info_plugin_sdk_interface_group
    - @ref kxe_virus_name_info_data_group

   - @subpage kxe_leakscan_subsystem_define "漏洞扫描服务接口定义"
    - @ref kxe_leak_scan_service_provider_interface_group
    - @ref kxe_leak_scan_data_define_group

   - @subpage kxe_2010_product_define "2010产品相关接口定义"
    - @ref kxe_2010_product_sercice_provider_interface
	- @ref kxe_2010_product_data_define_interface


 */

/**
 * @page kxengine_starfish_security_center_architecture KXEngine Security Center Architecture in Startfish
 *  @image html kxe_sc_architecture.png
 */

/**
 * @page kxengine_highperf_product_application KXEngine in High Perfoncmance Product
    @image html highperf_arch.png

   - @ref kxegnine_architecture_file_list_table "文件列表描述"

  - @subpage kxeservice_manager_define "服务管理器接口定义"
   - @ref kxeservice_manager_service_provider_interface_group
   - @ref kxeservice_manager_event_define_group
   - @ref kxeservice_manager_data_struct_group
   - @ref kxeservice_manager_config_data_struct_group
   - @ref kxeserv_command_line

  - @subpage kxepassport_subsystem_define "Passport子系统服务接口及SDK定义"
   - @ref kxepassport_subsystem_interface_group
   - @ref kxepassport_subsystem_data_struct_group
   - @ref kxepassport_service_provider_interface_group
   - @ref kxepassport_event_define_group

  - @subpage kxescan_subsystem_define "查杀子系统服务接口及SDK定义"
   - @ref kxescan_subsystem_interface_group
   - @ref kxescan_subsystem_data_struct
   - @ref kxescan_service_provider_interface_group
   - @ref kxescan_event_define_group

  - @subpage kxewhite_subsystem_define "白名单子系统服务接口及SDK定义"
   - @ref kxewhite_subsystem_interface_group
   - @ref kxewhite_subsystem_data_struct_group
   - @ref kxewhite_service_provider_interface_group
   - @ref kxewhite_event_define_group

  - @subpage kxefilemon_subsystem_define "文件监控子系统服务接口及SDK定义"
   - @ref kxefilemon_subsystem_interface_group
   - @ref kxefilemon_subsystem_data_struct_group
   - @ref kxefilemon_service_provider_interface_group
   - @ref kxefilemon_event_define_group
   - @ref kxefilemon_subsystem_config_group

  - @subpage kxepopo_subsystem_define "泡泡子系统服务接口及SDK定义"
   - @ref kxepopo_subsystem_interface_group
   - @ref kxepopo_subsystem_data_struct_group
   - @ref kxepopo_service_provider_interface_group
   - @ref kxepopo_event_define_group

  - @subpage kxeupdate_subsystem_define "升级子系统服务接口及SDK定义"
   - @ref kxeupdate_subsystem_interface_group
   - @ref kxeupdate_subsystem_data_struct_group
   - @ref kxeupdate_service_provider_interface_group
   - @ref kxeupdate_event_define_group
   - @ref kxeupdate_subsystem_config_data_struct_group

  - @subpage kxebase_system_define "系统服务提供者接口"
   - @ref kxebase_system_service_provider
   - @ref kxebase_system_service_provider_data_struct 
  
  - @subpage kxeexp_system_define "产品中间层接口定义"
   - @ref kxeexp_interface_group

  - @subpage kxedefendengine_subsystem_define "防御引擎服务接口及SDK定义"
   - @ref kxedefendengine_service_provider_interface_group
*/

/**
 * @page example KXEngine Architecture Example
 * - @subpage kxeevent_example "事件使用示例
 * - @subpage kxeconfig_example "配置使用示例"
 * - @subpage kxehttp_example "HTTP-RPC使用示例"
 * - @subpage kxeservice_provider_example "服务提供者创建示例"
 * - @subpage kxeplugin_sdk_example "子系统SDK定义示例"
 * - @subpage kxebase_thread_example "线程SDK使用示例"
 * - @subpage kxearchitecture_md5_example_page "MD5函数使用示例"
 */

/**
* @page json_data_struct KXEngine Architecture Json Data Struct
* - @subpage kxehttp_json_data_struct "HTTP-RPC传输的Json数据结构"
* - @subpage kxeevent_json_data_struct "事件服务传输的Json数据结构"
*/

/**
 * @page data_struct KXEngine Architecture Data Struct
 * - @subpage json_data_struct
 */

/**
 * @page architecture_interface KXEngine Architecture Interface
 *  - @subpage kxebase "架构基础接口SDK"
 *  - @subpage kxebase_plugin_sdk "子系统SDK扩展接口"
 *  - @subpage architecture_service_provider_interface "架构服务提供者基础接口"
 *  - @subpage data_struct "数据结构定义"
 */

/**
 * @page kxebase_system_define KXEngine Architecture System Service Provider Define
 * - @ref kxebase_system_service_provider
 * - @ref kxebase_system_service_provider_data_struct 
 */

/**
 * @page kxeservice_manager_define KXEngine Service Manager Define
 * - @ref kxeservice_manager_service_provider_interface_group
 * - @ref kxeservice_manager_event_define_group
 * - @ref kxeservice_manager_data_struct_group
 * - @ref kxeservice_manager_config_data_struct_group
 * - @ref kxeserv_command_line
 */

/**
 * @page kxescan_subsystem_define KXEngine Scan Subsystem Define
 *  - @ref kxescan_subsystem_interface_group
 *  - @ref kxescan_subsystem_data_struct
 *  - @ref kxescan_service_provider_interface_group
 *  - @ref kxescan_event_define_group
 */

/**
 * @page kxewhite_subsystem_define KXEngine WhiteFile SubSystem Define
 *  - @ref kxewhite_subsystem_interface_group
 *  - @ref kxewhite_subsystem_data_struct_group
 *  - @ref kxewhite_service_provider_interface_group
 *  - @ref kxewhite_event_define_group
 */

/**
 * @page kxefilemon_subsystem_define KXEngine Filemon SubSystem Define
 *  - @ref kxefilemon_subsystem_interface_group
 *  - @ref kxefilemon_subsystem_data_struct_group
 *  - @ref kxefilemon_service_provider_interface_group
 *  - @ref kxefilemon_event_define_group
 *  - @ref kxefilemon_subsystem_config_group
 */

/**
 * @page kxepopo_subsystem_define KXEngine Popo SubSystem Define
 *  - @ref kxepopo_subsystem_interface_group
 *  - @ref kxepopo_subsystem_data_struct_group
 *  - @ref kxepopo_service_provider_interface_group
 *  - @ref kxepopo_event_define_group
 */

/**
 * @page kxeupdate_subsystem_define KXEngine Update SubSystem Define
 *  - @ref kxeupdate_subsystem_interface_group
 *  - @ref kxeupdate_subsystem_data_struct_group
 *  - @ref kxeupdate_service_provider_interface_group
 *  - @ref kxeupdate_event_define_group
 *  - @ref kxeupdate_subsystem_config_data_struct_group
 */

/**
 * @page architecture_src_interface KXEngine Architecture Assistant Interface
 *  - @ref kxebase_src_thread_group "线程相关辅助函数及接口"
 *  - @ref kxearchitecture_md5_group "MD5计算接口"
 */

/**
 * @page architecture_core_component_interface KXEngine Architecture Core Component Interface 
 *  - @ref kxethread_component_interface  "线程管理组件接口"
 *  - @ref kxeconfig_component_interface  "配置管理组件接口"
 *  - @ref kxeevent_component_interface   "事件管理组件接口"
 */

/**
 * @page architecture_wow64_interface KXEngine Architecture WOW64 Mode
 *  - @ref kxereg_wow64_interface "注册表访问部分"
 *  - @ref kxefile_wow64_interface "文件访问部分"
 */

/**
 * @page kxeexp_system_define KXEngine Architecture Express Layer Define
 *  - @ref kxeexp_interface_group
 */

/**
 * @page kxepassport_subsystem_define KXEngine Passport SubSystem Define
 *  - @ref kxepassport_subsystem_interface_group
 *  - @ref kxepassport_subsystem_data_struct_group
 *  - @ref kxepassport_service_provider_interface_group
 *  - @ref kxepassport_event_define_group
 */

/**
 * @page kxedefendengine_subsystem_define KXEngine DefendEngine Define
 *  - @ref kxedefendengine_service_provider_interface_group
 */

/**
 * @page architecture_product_manage_interface KXEngine Product Manage interface
 *  - @ref kxeservice_manager_service_provider_interface_group "产品管理的HTTP-RPC接口"
 *  - @ref kxeserv_command_line "产品管理的命令行接口
 */

/**
 * @page kxegnine_architecture_file_list_table KXEngine File List Table
 @code
+-KXEngine
    |
    +-driver
        |
        +-kwatch64.sys  64位文件监控驱动
	+-kxeweb
	    |
        +-kavweb.dat  主程序使用的网页文件压缩包，使用zip压缩
        +-kpopweb.dat 托盘使用的网页文件压缩包，使用zip压缩
	+-ksg     病毒库文件,与单机文件形成统一
    +-kxecore 框架核心文件
        |
        +-kxecfg.dll   配置管理
        +-kxeevent.dll 事件管理
        +-kxehttp.dll  HTTP管理
        +-kxelog.dll   Debug日志管理
		+-kxesvc.dll   Service Call模块，为kissvc.dll的复制品
        +-kxethr.dll   线程管理
    +-lang 语言文件(由于单机的kavlog2.exe, krecycle.exe使用)
		|
		+-chs
		   |
		   +-kavlog2.xml kavlog2.exe使用的资源文件
		   +-kavppwiz.xml kavppwiz.dll使用的资源文件
		   +-kis.dat 单机使用的配置文件
		   +-kolhtml.xml kolhtml.dll使用的资源文件
		   +-krecycle.xml krecycle.exe使用的资源文件
		   +-license.txt
		   +-uplive.svr 升级服务器配置文件
		   +-uplive.xml 升级使用的资源文件
    +-plugin 框架扩展SDK文件
        |
        +-ksesdk.dll   查杀的SDK
        +-kxeexp.dll   产品中间层,提供功能到产品之间的转换
        +-kxefm.dll    文件监控的SDK
        +-kxepop.dll   泡泡的SDK
        +-kxewhite.dll 白名单的SDK
    +-serviceprovider 服务提供者目录
        |
        +-kxefmsp.dll  文件监控的sp
        +-kxepopsp.dll 泡泡的sp
        +-kxeppsp.dll  passport的sp
        +-kxesansp.dll 查杀的sp
        +-kxesyssp.dll 系统服务的sp
        +-kxeupdsp.dll 升级服务的sp
        +-kxewfssp.dll 白名单服务的sp
    +-subsystem 子系统目录
        |
        +-kxefilemon 文件监控子系统目录
             |
             +- kxefmsys.dll 文件监控子系统文件
        +-kxepopo    泡泡子系统目录
             |
             +-kxeposys.dll   泡泡子系统文件
        +-kxescan    查杀子系统目录
             |
             +-config  查杀子系统配置文件
                |
                +-ksecore.addon.xml            查杀子系统插件列表
                +-ksecore.config.filemon.xml   查杀子系统文件监控扫描引擎配置信息
                +-ksecore.config.normal.xml    查杀子系统一般扫描引擎配置信息
                +-ksecore.config.top.xml       查杀子系统顶层配置信息
                +-ksecore.sln.xml              查杀子系统查杀解决方分解列表
                +-kspfeng.polman.xml           查杀子系统文件引擎病毒库加载策略文件管理器配置信息
             +-kae     文件引擎文件
                |
                +-7z.dll        支持多种压缩格式的压缩算法实现模块, 被解压接口封装模块调用
                +-kaecore.dat   文件识别, 判定和处理的封装接口模块, 提供接口IKSGetFileType, IKSBWManager和IKSSPManager
                +-kaecore.ini   黑白机配置文件
                +-kaecorea.dat  通用黑白机(也称脚本引擎), 被引擎接口封装模块调用
				+-kaeolea.dat   处理ole的模块
                +-kaeunpack.dat 旧引擎的脱壳模块, 被脱壳接口封装模块调用
                +-kaeunpak.dat  带壳文件的的还原处理模块, 提供接口IKSUnpack
                +-kaext2.dat
                +-kaextend.dat
                +-karchive.dat  压缩包, 自解压文件, 安装包文件的还原处理, 提供接口IKSArchive
             +-kas     清理专家引擎文件
                +-kasengine.dll
             +-kavquara.dll 单机隔离文件
             +-kis.dll      单机基础库
             +-ksecore.dll  查杀子系统核心框架
             +-ksejob.dll   查杀子系统线程调度模块
             +-kspfeng.dll  查杀子系统文件扫描插件
             +-kspkas.dll   查杀子系统恶意软件扫描插件
             +-kssingle.dll 查杀子系统处理工具包
        +-kxewhite 白名单子系统目录
             |
             +-kavrep.dll 白名单服务器验证文件
             +-kxewfsys.dll 白名单子系统文件
		+-kxeupdate 升级子系统目录
		     |
			 +-kxeupchk.dll 实时升级调度模块
		+-userlog   用户日志目录
		     |
			 +-kxeurlog.dll 用于写用户日志的dll，为kavevent.dll的复制品(目前已经不再使用，2009-6-25,liusheng)
    +-driver64.exe 64位驱动安装卸载程序(唯一的64位程序)
	+-kaearcha.dat 文件引擎(老版本)
	+-kaeboot.dat  文件引擎(老版本)
	+-kaecall2.dll 文件引擎(老版本)
	+-kaecentr.dat 文件引擎(老版本)
	+-kaecore.dat  文件引擎(老版本)
	+-kaecorea.dat 文件引擎(老版本)
	+-kaemaldt.dll 文件引擎(老版本)
	+-kaememex.dll 文件引擎(老版本)
	+-kaengcfg.dat 文件引擎(老版本)
	+-kaengine.dat 文件引擎(老版本)
	+-kaentrya.dat 文件引擎(老版本)
	+-kaeolea.dat  文件引擎(老版本)
	+-kaeplat.dll  文件引擎(老版本)
	+-kaeplata.dll 文件引擎(老版本)
	+-kaeprev.dll  文件引擎(老版本)
	+-kaeproxy.dat 文件引擎(老版本)
	+-kaesgnld.dll 文件引擎(老版本)
	+-kaeunpack.dat文件引擎(老版本)
	+-kaext2.dat   文件引擎(老版本)
	+-kaextend.dat 文件引擎(老版本)
	+-karchive.dat 文件引擎(老版本)
	+-karescfg.dat 文件引擎(老版本)
    +-kav64.exe    主程序
    +-kavevent.dll 单机用户日志输出文件
    +-kavipc2.dll  模拟单机的ipc接口实现的中间转换文件
    +-kavlog2.exe  用户日志查看器
    +-kavpass2.dll passport验证模块
    +-kavpassp.dll passport验证模块
	+-kavpid.kid   升级用于判定模块的文件,内容不关心
	+-kapolicy.exe 将signs.ini转换为kaesign.pol的工具
    +-kavppwiz.dll passport的wiz模块
	+-kdevmgr.dll  驱动安装卸载模块(供uplive.exe调用)
	+-kiscall.exe  模块调用程序，用于降权限调用
    +-kis.dll      单机的基础模块
    +-kissvc.dll   单机的Service Call模块
	+-kolhtml.dll  充值验证嵌入的网页模块
    +-krecycle.exe 隔离模看模块
    +-kstart64.exe 托盘模块
    +-kxebase.dll  框架基础模块
	+-kxecolct.exe 错误处理信息收集模块
	+-kxedump.dll  崩溃处理模块
    +-kxeerror.dll 错误码查看模块
    +-kxeserv.exe  框架服务程序
	+-kxeupsrv.exe 升级服务程序，目前仅用来加载Service Call
    +-scom.dll     COM组件管理模块
    +-scomregsvr.exe COM组件注册卸载模块
	+-signs.ini    病毒库配置文件
    +-updateex.dll 单机升级扩展模块
    +-uplive.exe   单机升级模块
    +-microsoft.vc80.crt 微软的crt的manifest
    +-msvcm80.dll  微软crt模块
    +-msvcp80.dll  微软crt模块
    +-msvcr80.dll  微软crt模块
 @endcode

  @page kxe_ipc_interface "IPC调用相关接口"
   - @ref kxeipc_service_provider_interface_group
   - @ref kxeipc_idl_interface_group
   - @ref kxepipe_idl_interface_group
   - @ref kxehttp_idl_interface_group

 */

/**
  @page kxenameinfo_subsystem_define KXEngine virus name info service
   - @ref kxe_virus_name_info_service_provider_interface_group
   - @ref kxe_virus_name_info_data_group
 */

/**
  @page kxe_leakscan_subsystem_define KXEngine leakscan service
   - @ref kxe_leak_scan_service_provider_interface_group
   - @ref kxe_leak_scan_data_define_group
 */

/**
  @page kxe_popo_subsytem_define_2 KXEngine popo system service
   - @ref kxe_pop_server_service_provider_interface_group
   - @ref kxe_pop_server_data_define_group
 */
