#ifndef __BUILDCFG_BUILDCFG_H__
#define __BUILDCFG_BUILDCFG_H__

/*

1   打开本工程的主资源文件，删除原有的版本信息，将version.rc添加到工程

2   打开 trunk/builder/buildcfg 目录下对应的buildcfg.h文件
	找到该工程所在位置，请用英语定义以下三个宏，如：	

#ifdef ETM_ETDBE // size : 20000
	#pragma comment(linker, "/base:0x4DA00000")   
	#pragma message("\t    ETM_ETDBE")
	#define VI_FILEDESCRIPTION	"Data Import"	//文件描述
	#define VI_INTERNALNAME		"dbe"			//内部名称
	#define VI_ORIGINALFILENAME	"etdbe.dll"		//原始DLL文件名
#endif

3.  在工程设置的 Resources 页的"Preprocessor definitions"中添加本工程对应的宏，如 ETM_ETDBE
	请注意所有配置(Debug/Release/...)都要加上
	在"Additionalresource include directories"中加上Coding/include目录的相对位置
	如 "../../builder/buildcfg"
	
 */

// -------------------------------------------------------------------------

#ifdef _WINDOWS_
#error "include buildcfg.h before include all other files"
#endif

/////////////////////////////////
//exe
/**
here is a exe example:
#ifdef KISM_KAV32
	#pragma message("\t    KISM_KAV32")
	#define VI_FILEDESCRIPTION	"Kingsoft AntiVirus"
	#define VI_INTERNALNAME		"kav32"
	#define VI_ORIGINALFILENAME	"kav32.exe"
#endif
*/

#ifdef BEIKEBC_SYS
#pragma message("\t    BEIKEBC_SYS")
#define VI_FILEDESCRIPTION	"ksafebc Components"
#define VI_INTERNALNAME		"ksafebc"
#define VI_ORIGINALFILENAME	"ksafebc.sys"
#endif

#ifdef BKENG_DLL
#pragma message("\t    BKENG_DLL")
#define VI_FILEDESCRIPTION	"KSafe Scan Engine"
#define VI_INTERNALNAME		"ksafeeng"
#define VI_ORIGINALFILENAME	"ksafeeng.dll"
#endif

#ifdef BKDB_DLL
#pragma message("\t    BKDB_DLL")
#define VI_FILEDESCRIPTION	"KSafe DB Engine"
#define VI_INTERNALNAME		"ksafedb"
#define VI_ORIGINALFILENAME	"ksafedb.dll"
#endif

#ifdef BKAVE_DLL
#pragma message("\t    BKAVE_DLL")
#define VI_FILEDESCRIPTION	"KSafe AntiVirus Engine"
#define VI_INTERNALNAME		"ksafeave"
#define VI_ORIGINALFILENAME	"ksafeave.dll"
#endif

#ifdef BKENGVER_DLL
#pragma message("\t    BKENGVER_DLL")
#define VI_FILEDESCRIPTION	"KSafe Engine Version"
#define VI_INTERNALNAME		"bkengver"
#define VI_ORIGINALFILENAME	"bkengver.dll"
#endif

#ifdef BKCACHE_DLL
#pragma message("\t    BKCACHE_DLL")
#define VI_FILEDESCRIPTION	"KSafe Cache Engine"
#define VI_INTERNALNAME		"kcache"
#define VI_ORIGINALFILENAME	"kcache.dll"
#endif

#ifdef BKBAK_DLL
#pragma message("\t    BKBAK_DLL")
#define VI_FILEDESCRIPTION	"KSafe Backup Module"
#define VI_INTERNALNAME		"ksafebak"
#define VI_ORIGINALFILENAME	"ksafebak.dll"
#endif

#ifdef BKENGWIN_EXE
#pragma message("\t    BKENGWIN_EXE")
#define VI_FILEDESCRIPTION	"KSafe Engine Demo"
#define VI_INTERNALNAME		"bkengwin"
#define VI_ORIGINALFILENAME	"bkengwin.exe"
#endif

#ifdef BKKSUP_DLL
#pragma message("\t    BKENGWIN_EXE")
#define VI_FILEDESCRIPTION	"KSafe Signature Updater"
#define VI_INTERNALNAME		"ksignup"
#define VI_ORIGINALFILENAME	"ksignup.dll"
#endif


#ifdef BK_AUTORUN_DLL
#pragma message("\t    BK_AUTORUN_DLL")
#define VI_FILEDESCRIPTION	"KSafe Autorun scanner"
#define VI_INTERNALNAME		"katrun"
#define VI_ORIGINALFILENAME	"katrun.dll"
#endif

#ifdef BEIKE_SAFE_EXE
	#pragma message("\t    BEIKE_SAFE_EXE")
	#define VI_FILEDESCRIPTION	"金山卫士主程序"
	#define VI_INTERNALNAME		"KSafe"
	#define VI_ORIGINALFILENAME	"KSafe.exe"
#endif

#ifdef BKSAFE_EXAM_DLL
    #pragma message("\t    BKSAFE_EXAM_DLL")
    #define VI_FILEDESCRIPTION	"KSafe Examination "
    #define VI_INTERNALNAME		"ksafeexam"
    #define VI_ORIGINALFILENAME	"ksafeexam.dll"
#endif

#ifdef IESCAN_ENG_DLL
    #pragma message("\t    IESCAN_ENG_DLL")
    #define VI_FILEDESCRIPTION	"KSafe IEScan Engine"
    #define VI_INTERNALNAME		"kiefixeng"
    #define VI_ORIGINALFILENAME	"kiefixeng.dll"
#endif

#ifdef VUL_FIX_DLL
    #pragma message("\t    VUL_FIX_DLL")
    #define VI_FILEDESCRIPTION	"KSafe Vulnerability Fix"
    #define VI_INTERNALNAME		"ksafevul"
    #define VI_ORIGINALFILENAME	"ksafevul.dll"
#endif

#ifdef BKVER_DLL
    #pragma message("\t    BKVER_DLL")
    #define VI_FILEDESCRIPTION	"KSafe version"
    #define VI_INTERNALNAME		"ksafever"
    #define VI_ORIGINALFILENAME	"ksafever.dll"
#endif

#ifdef BKSAFESVC_EXE
	#pragma message("\t    BKSAFESVC_EXE")
	#define VI_FILEDESCRIPTION	"金山卫士服务程序"
	#define VI_INTERNALNAME		"KSafeSvc"
	#define VI_ORIGINALFILENAME	"KSafeSvc.exe"
#endif

#ifdef PLUGIN_ENGINE_DLL
	#pragma message("\t    PLUGIN_ENGINE_DLL")
	#define VI_FILEDESCRIPTION	"KSafe Plugin Engine"
	#define VI_INTERNALNAME		"kplugeng"
	#define VI_ORIGINALFILENAME	"kplugeng.dll"
#endif

#ifdef PLUG_EXTEND_DLL
#pragma message("\t    PLUG_EXTEND_DLL")
#define VI_FILEDESCRIPTION	"KSafe Plug Extend"
#define VI_INTERNALNAME		"kplugext"
#define VI_ORIGINALFILENAME	"kplugext.dll"
#endif

#ifdef SAFE_MONITOR_DLL
	#pragma message("\t    SAFE_MONITOR_DLL")
	#define VI_FILEDESCRIPTION	"KSafe Monitor"
	#define VI_INTERNALNAME		"ksfmon"
	#define VI_ORIGINALFILENAME	"ksfmon.dll"
#endif

#ifdef SAFE_TRAY_EXE
#pragma message("\t    SAFE_TRAY_EXE")
#define VI_FILEDESCRIPTION	"金山卫士实时保护模块"
#define VI_INTERNALNAME		"KSafeTray"
#define VI_ORIGINALFILENAME	"KSafeTray.exe"
#endif

#ifdef UPLIVE_DLL
#pragma message("\t    UPLIVE_DLL")
#define VI_FILEDESCRIPTION	"KSafe Uplive"
#define VI_INTERNALNAME		"ksafeup"
#define VI_ORIGINALFILENAME	"ksafeup.dll"
#endif

#ifdef KSAFE_RUN_OPT
#pragma message("\t    KSAFE_RUN_OPT")
#define VI_FILEDESCRIPTION	"KSafe Run Manager"
#define VI_INTERNALNAME		"krunopt"
#define VI_ORIGINALFILENAME	"krunopt.dll"
#endif
	
#ifdef SOFTMGR_DLL 
	#pragma message("\t    SOFTMGR_DLL")
	#define VI_FILEDESCRIPTION	"SoftManager"	//文件描述
	#define VI_INTERNALNAME		"sftmgr"			//内部名称
	#define VI_ORIGINALFILENAME	"softmgr.dll"		//原始DLL文件名
#endif

#ifdef KWS_CONTROLLER_DLL 
#pragma message("\t    KWS_CONTROLLER_DLL")
#define VI_FILEDESCRIPTION	"kwsctrl"			//文件描述
#define VI_INTERNALNAME		"kwsctrl"			//内部名称
#define VI_ORIGINALFILENAME	"kwsctrl.dll"		//原始DLL文件名
#endif

#ifdef KSAFE_INSTALLER
#pragma message("\t    KSAFE_INSTALLER")
#define VI_FILEDESCRIPTION	"KSafe Downloader"
#define VI_INTERNALNAME		"ksafedownloader"
#define VI_ORIGINALFILENAME	"ksafedownloader.exe"
#endif

/** Begin 清理模块 */
// 主程序
#ifdef KSC_MAIN_DLL
#	pragma message("\t    KSC_MAIN_DLL")
#	define VI_FILEDESCRIPTION	"Kingsoft System Cleaner"
#	define VI_INTERNALNAME		"kclear.dll"
#	define VI_ORIGINALFILENAME	"kclear.dll"
#endif

// 垃圾清理
#ifdef KSC_TRASHCLEANER_DLL
#pragma message("\t    KSC_TRASHCLEANER_DLL")
#define VI_FILEDESCRIPTION	"Kingsoft System Cleaner"
#define VI_INTERNALNAME		"trashcleaner.dll"
#define VI_ORIGINALFILENAME	"trashcleaner.dll"
#endif

// 注册表清理
#ifdef KSC_REGCLEANER_DLL
#pragma message("\t    KSC_REGCLEANER_DLL")
#define VI_FILEDESCRIPTION	"Kingsoft System Cleaner RegCleaner Module"
#define VI_INTERNALNAME		"regcleaner.dll"
#define VI_ORIGINALFILENAME	"regcleaner.dll"
#endif

// 痕迹清理
#ifdef KSC_TRACKCLEANER_DLL
#pragma message("\t    KSC_TRACKCLEANER_DLL")
#define VI_FILEDESCRIPTION	"Kingsoft System Cleaner RegCleaner Module"
#define VI_INTERNALNAME		"trackcleaner.dll"
#define VI_ORIGINALFILENAME	"trackcleaner.dll"
#endif

// 文件大小缓存
#ifdef KSC_FCACHE_DLL
#pragma message("\t    KSC_FCACHE_DLL")
#define VI_FILEDESCRIPTION	"Kingsoft System Cleaner Cache Module"
#define VI_INTERNALNAME		"fcache.dll"
#define VI_ORIGINALFILENAME	"fcache.dll"
#endif

/** End 清理模块 */

#ifdef KSC_KWSSP_DLL
#pragma message("\t    KSC_KWSSP_DLL")
#define VI_FILEDESCRIPTION	"Kingsoft WebShield kwwsp Module"
#define VI_INTERNALNAME		"kwssp.dll"
#define VI_ORIGINALFILENAME	"kwssp.dll"
#endif

#ifdef KSC_KSWEBSHIELD_DLL
#pragma message("\t    KSC_KSWEBSHIELD_DLL")
#define VI_FILEDESCRIPTION	"Kingsoft WebShield kswebshield Module"
#define VI_INTERNALNAME		"kswebshield.dll"
#define VI_ORIGINALFILENAME	"kswebshield.dll"
#endif

#ifdef KSC_KSWBC_DLL
#pragma message("\t    KSC_KSWBC_DLL")
#define VI_FILEDESCRIPTION	"Kingsoft WebShield kswbc Module"
#define VI_INTERNALNAME		"kswbc.dll"
#define VI_ORIGINALFILENAME	"kswbc.dll"
#endif

#ifdef KSC_KWSUI_DLL
#pragma message("\t    KSC_KWSUI_DLL")
#define VI_FILEDESCRIPTION	"Kingsoft WebShield kwsui Module"
#define VI_INTERNALNAME		"kwsui.dll"
#define VI_ORIGINALFILENAME	"kwsui.dll"
#endif

#ifdef KSC_PEDOWNUI_DLL
#pragma message("\t    KSC_PEDOWNUI_DLL")
#define VI_FILEDESCRIPTION	"Kingsoft WebShield pedownui Module"
#define VI_INTERNALNAME		"pedownui.dll"
#define VI_ORIGINALFILENAME	"pedownui.dll"
#endif
	
#ifdef KSC_KWSPLUGIN_DLL
#pragma message("\t    KSC_KWSPLUGIN_DLL")
#define VI_FILEDESCRIPTION	"Kingsoft WebShield kwsplugin Module"
#define VI_INTERNALNAME		"pedownui.dll"
#define VI_ORIGINALFILENAME	"pedownui.dll"
#endif

#ifdef FIREWALL_PROXY
	#pragma comment(linker, "/base:0x44B00000")
	#pragma message("\t    FIREWALL_PROXY")
	#define VI_FILEDESCRIPTION	"Kingsoft Firewall Proxy"
	#define VI_INTERNALNAME		"fwproxy"
	#define VI_ORIGINALFILENAME	"fwproxy.dll"
#endif

#ifdef KSC_SAFEMONITERFLOW // size : 20000
#pragma message("\t    KSC_SAFEMONITERFLOW")
#define VI_FILEDESCRIPTION	"Kingsoft Internet Flow"	//文件描述
#define VI_INTERNALNAME		"netmon.exe"			//内部名称
#define VI_ORIGINALFILENAME	"netmon.exe"		    //原始EXE文件名
#endif
#ifdef FIREWALL_TDIFILT
	#pragma message("\t    FIREWALL_TDIFILT")
	#define VI_FILEDESCRIPTION	"Kingsoft Firewall TdiFilter Driver"
	#define VI_INTERNALNAME		"KTdiFilt"
	#define VI_ORIGINALFILENAME	"ktdifilt.sys"
#endif  // FIREWALL_TDIFILT

#ifdef KFILE_DOWN
#pragma message("\t    KFILE_DOWN")
#define VI_FILEDESCRIPTION	"Kingsoft download module"
#define VI_INTERNALNAME		"kfiled.dll"
#define VI_ORIGINALFILENAME	"kfiled.dll"
#endif

// 杀软检测
#ifdef BKSAFE_AV_MGR_DLL
#pragma message("\t    BKSAFE_AV_MGR_DLL")
#define VI_FILEDESCRIPTION	"Kingsoft Anti-Virus Software Manager Module"
#define VI_INTERNALNAME		"kavmgr.dll"
#define VI_ORIGINALFILENAME	"kavmgr.dll"
#endif

// Hotopt
#ifdef BKSAFE_HOTOPT_EXE
#pragma message("\t    BKSAFE_HOTOPT_EXE")
#define VI_FILEDESCRIPTION	"KSafe Hot Optimize Module"
#define VI_INTERNALNAME		"hotopt"
#define VI_ORIGINALFILENAME	"hotopt.exe"
#endif

#ifdef BEIKE_KIS_PUBLIC // size : 20000
#pragma message("\t    BEIKE_KIS_PUBLIC")
#define VI_FILEDESCRIPTION	"Kingsoft's public lib"	//文件描述
#define VI_INTERNALNAME		"kispublic.dll"			//内部名称
#define VI_ORIGINALFILENAME	"kispublic.dll"		//原始DLL文件名
#endif

#ifdef BEIKE_MSPEED_EXE
#pragma message("\t    BEIKE_MSPEED_EXE")
#define VI_FILEDESCRIPTION	"金山卫士网络测速"
#define VI_INTERNALNAME		"kmspeed.exe"
#define VI_ORIGINALFILENAME	"kmspeed.exe"
#endif

#ifdef WD_SWITCH
#pragma message("\t    WD_SWITCH")
#define VI_FILEDESCRIPTION	"webshiled switch"
#define VI_INTERNALNAME		"kwds.dll"
#define VI_ORIGINALFILENAME	"kwds.dll"
#endif
#ifdef BEIKE_U11
#pragma message("\t    BEIKE_U11")
#define VI_FILEDESCRIPTION	"金山平滑升级"
#define VI_INTERNALNAME		"u11.exe"
#define VI_ORIGINALFILENAME	"u11.exe"
#endif

#ifdef KPROCLIB_DLL
#pragma message("\t    KPROCLIB_DLL")
#define VI_FILEDESCRIPTION	"KSafe ProcLib Module"
#define VI_INTERNALNAME		"kproclib.dll"
#define VI_ORIGINALFILENAME	"kproclib.dll"
#endif

#ifdef NETSTAT_DLL
#pragma message("\t    NETSTAT_DLL")
#define VI_FILEDESCRIPTION	"ksafe netflux module"
#define VI_INTERNALNAME		"netstat.dll"
#define VI_ORIGINALFILENAME	"netstat.dll"
#endif

#ifdef KPCFILEOPEN_EXE
#pragma message("\t    KPCFILEOPEN_EXE")
#define VI_FILEDESCRIPTION	"softmgr unknown file open"		
#define VI_INTERNALNAME		"kpcfileopen"					
#define VI_ORIGINALFILENAME	"kpcfileopen.exe"				
#endif

#ifdef KSCAN_DLL
#pragma message("\t    KSCAN_DLL")
#define VI_FILEDESCRIPTION	"ksafe scan module"		
#define VI_INTERNALNAME		"kscan"					
#define VI_ORIGINALFILENAME	"kscan.dll"				
#endif

#ifdef KSSHIELD_DLL
#pragma message("\t    KSSHIELD_DLL")
#define VI_FILEDESCRIPTION	"ksafe webshield module"		
#define VI_INTERNALNAME		"ksshield"					
#define VI_ORIGINALFILENAME	"ksshield.dll"				
#endif

#ifdef KSAFE_PERFOPT_EXE //系统加速
#pragma message("\t    KSAFE_PERFOPT_EXE")
#define VI_FILEDESCRIPTION	"金山卫士 系统加速"	//文件描述
#define VI_INTERNALNAME		"perfopt.exe"			//内部名称
#define VI_ORIGINALFILENAME	"perfopt.exe"		    //原始EXE文件名
#endif

#ifdef SAFE_MON_SYS
#pragma message("\t    SAFE_MON_SYS")
#define VI_FILEDESCRIPTION	"金山卫士实时保护驱动"
#define VI_INTERNALNAME		"ksfmonsys"
#define VI_ORIGINALFILENAME	"ksfmonsys"
#endif

#endif /* __BUILDCFG_BUILDCFG_H__ */

