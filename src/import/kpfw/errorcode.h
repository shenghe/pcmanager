////////////////////////////////////////////////////////////////////////////////
//      
//      common include File for kpfw
//      
//      File      : errorcode.h
//      Version   : 1.0
//      Comment   : 定义网镖服务各种出错代码
//      
//      Create at : 2008-4-7
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 

//////////////////////////////////////////////////////////////////////////
// 错误代码格式（和windows 错误代码兼容）
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |  Class    |         Code      |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//		Class - is the facility's function class
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//

// 网镖分类
#define FACILITY_KPFW					0x701

#define ERROR_CODE_BUILDER(level, facility, cls, c)		\
		(( ((int)(level) & 0xf) << 28 ) | ( ((int)facility & 0xfff) << 16 ) | ( ((int)cls & 0x3f) << 10 ) | ( (int)c & 0x3ff ))

#define ERROR_CODE(facility, cls, c)	ERROR_CODE_BUILDER( 0xC, facility, cls, c)

#define WARING_CODE(facility, cls, c)	ERROR_CODE_BUILDER( 0x8, facility, cls, c )

#define KPFW_ERROR(cls, c)				ERROR_CODE( FACILITY_KPFW, cls, c )

#define KPFW_WARNING(cls, c)			WARING_CODE( FACILITY_KPFW, cls, c )

// 网镖错误类型
#define KPFW_ECLASS_IPRULE				0x01
#define KPFW_ECLASS_APPRULE				0x02
#define KPFW_ECLASS_ARP					0x03
#define KPFW_ECLASS_LOG					0x04
#define KPFW_ECLASS_DRIVER				0x05
#define KPFW_ECLASS_AREA				0x06
#define KPFW_ECLASS_BADURL				0x07

//////////////////////////////////////////////////////////////////////////
// 常见错误：

//////////////////////////////////////////////////////////////////////////
// ip规则相关错误:

// ip规则文件打开失败
#define E_IP_FILE_OPEN_FAILED			KPFW_ERROR( KPFW_ECLASS_IPRULE, 0x1)

// ip规则文件被破坏
#define E_IP_FILE_DAMAGED				KPFW_ERROR( KPFW_ECLASS_IPRULE, 0x2)

// 默认ip规则文件打开失败
#define E_DEFAULT_IP_FILE_OPEN_FAILED	KPFW_ERROR( KPFW_ECLASS_IPRULE, 0x3)

// ip规则文件保存失败
#define E_IP_FILE_SAVE_FAILED			KPFW_ERROR( KPFW_ECLASS_IPRULE, 0x4)

// 不可识别的 ip规则文件格式
#define E_IP_FILE_UNKNOWN_FMT			KPFW_ERROR( KPFW_ECLASS_IPRULE, 0x5)

// 不识别文件格式
#define E_BADURL_FILE_UNKNOWN_FMT			KPFW_ERROR( KPFW_ECLASS_BADURL, 0x6)

//////////////////////////////////////////////////////////////////////////
// 驱动相关错误:

// 驱动初始化失败
#define E_DIRVER_INIT_FAILED			KPFW_ERROR( KPFW_ECLASS_DRIVER, 0x1)

// 驱动代理不存在
#define E_DIRVER_PROXY_NOTEXIST			KPFW_ERROR( KPFW_ECLASS_DRIVER, 0x2)

//////////////////////////////////////////////////////////////////////////
// 应用程序规则相关错误:

// 应用程序规则文件打开失败
#define E_APP_FILE_OPEN_FAILED			KPFW_ERROR( KPFW_ECLASS_APPRULE, 0x1)

// 应用程序规则文件被破坏
#define E_APP_FILE_DAMAGED				KPFW_ERROR( KPFW_ECLASS_APPRULE, 0x2)

// 不可识别的应用程序规则文件
#define E_APP_FILE_UNKNOWN_FMT			KPFW_ERROR( KPFW_ECLASS_APPRULE, 0x3)

// 应用程序规则文件保存失败
#define E_APP_FILE_SAVE_FAILED			KPFW_ERROR( KPFW_ECLASS_APPRULE, 0x4)
// 

//////////////////////////////////////////////////////////////////////////
// 区域管理错误

// 区域规则文件打开失败
#define E_AREA_FILE_OPEN_FAILED			KPFW_ERROR( KPFW_ECLASS_AREA, 0x1)

// 区域规则文件被破坏
#define E_AREA_FILE_DAMAGED				KPFW_ERROR( KPFW_ECLASS_AREA, 0x2)

// 区域规则文件保存失败
#define E_AREA_FILE_SAVE_FAILED			KPFW_ERROR( KPFW_ECLASS_AREA, 0x4)

// 不可识别的 区域规则文件格式
#define E_AREA_FILE_UNKNOWN_FMT			KPFW_ERROR( KPFW_ECLASS_AREA, 0x5)
