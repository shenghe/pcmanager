#ifndef _REGKEYSTRING_H_
#define _REGKEYSTRING_H_

//////////////////////////////////////////////////////////////////////////
//kpfwfrm
#define		KPFW_ROOT_PATH_KEY					TEXT("SOFTWARE\\Kingsoft\\kpfw")
#define		KPFW_COM_PATH_KEY					TEXT("SOFTWARE\\Kingsoft\\KIS\\CLASSES_ROOT")
#define		KPFW_FRM_PATH_KEY					TEXT("SOFTWARE\\Kingsoft\\kpfw\\kpfwfrm")
#define	    KCOMMON_DEFAULT_BROWSER_REY_PATH	TEXT("HTTP\\shell\\open\\command")
#define		KPFW_CHANGE_TYPE_RENAME				TEXT("bChangeTypeRename")
#define		KCOMMON_AUTO_RUN_REY_VALUE			TEXT("kpfwtray")
#define		KCOMMON_AUTO_RUN_REY_PATH			TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")

//////////////////////////////////////////////////////////////////////////
//log
#define KIS_LOG_APP_KEY					TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Kpfwlog\\applog")
#define KIS_LOG_ATTACK_KEY				TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Kpfwlog\\attacklog")
#define KIS_LOG_NETACT_KEY				TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Kpfwlog\\netactlog")
#define KIS_LOG_APP						TEXT("applog")
#define KIS_LOG_ATTACK					TEXT("attacklog")
#define KIS_LOG_NETACT					TEXT("netactlog")
#define KIS_LOG_KPFW_SOURCE				TEXT("KAVPFW")
#define KIS_LOG_PATH_KEY				TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Kpfwlog\\")
#define KIS_LOG_KPFW_DISPLAY_NAME		TEXT("%SystemRoot%\\system32\\els.dll")
#define KIS_LOG_KPFW_FILE				TEXT("%SystemRoot%\\system32\\config\\Kpfwlog.evt")
#define KIS_LOG_MAXSIZE_KEY				TEXT("MaxSize")
#define KIS_LOG_RETENTION_KEY			TEXT("Retention")
#define KIS_LOG_PAGECNT_KEY				TEXT("PageCount")
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//kpfw record option
#define KRO_TOP5_LISTBOX_KEY			TEXT("top5listbox")
#define KRO_FLUX_TYPE_KEY				TEXT("fluxtype")
#define KRO_SAFE_AREA_KEY				TEXT("safeareaindex")
#define KRO_APP_RULE_KEY				TEXT("appruleindex")
#define KRO_MAIN_TAB_KEY				TEXT("maintabindex")

//////////////////////////////////////////////////////////////////////////
//antivirus
#define KIS_ANTIVIRUS_KEY				TEXT("SOFTWARE\\Kingsoft\\AntiVirus")
#define KIS_ANTIVIRUS_ITEM				TEXT("ProgramPath")

/////////////////////////////////////////////////////////////////////////
//antivirus 2010
#define KIS_10_KIS_KEY					TEXT("SOFTWARE\\Kingsoft\\KISCommon\\Install\\kis")
#define KIS_10_KISCOMMON_KEY					TEXT("SOFTWARE\\Kingsoft\\KISCommon\\Install\\kiscommon")

//////////////////////////////////////////////////////////////////////////
#define KIS_KISCOMMON_KEY				TEXT("SOFTWARE\\Kingsoft\\KISCommon")
#define KIS_LANG_KEY					TEXT("Lang")

#define KIS_OEM_KEY_PATH		TEXT("Software\\Kingsoft\\ANTIVIRUS")
#define KIS_OEM_VALUE_NAME		TEXT("VersionTypes")

#endif
