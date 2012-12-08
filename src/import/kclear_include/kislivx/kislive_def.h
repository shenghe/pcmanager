//datadef.h
//升级程序公共数据定义。
//

#pragma once
#ifndef __UPLIVE_COMMON_DATA_DEF_H__
#define __UPLIVE_COMMON_DATA_DEF_H__

#include "uplive_common.h"
#include "kaesign.h"
#include "updateself_def.h"

#ifdef USE_UPLIVE_DEBUGLOG
	#define defKISLIVX_DLL L"kislivx_log.dll"
#else
	#define defKISLIVX_DLL L"kislivx.dll"
#endif


// kislivx.dll版本（普通版、WinPE版、迷你版、修复版本）
#define defKISLIVX_VERSION_Normal				_T("{33D555E6-44E7-46de-927E-48B81CCF7E6B}")
#define defKISLIVX_VERSION_WinPE				_T("{661923A0-C8AE-4a94-87E3-40B503822DFD}")
#define defKISLIVX_VERSION_Mini					_T("{77F22B8E-DD7E-412a-85D5-2A93AE92E59D}")
#define defKISLIVX_VERSION_Fix					_T("{77FBC834-F70F-4084-8823-1F5FB4575E0D}")
#define defKISLIVX_VERSION_Kws3					_T("{7A4B23C8-3C71-4fe0-97B2-613F8F94D0C9}")
#define defKISLIVX_VERSION_Ksc3					defKISLIVX_VERSION_Kws3

#define defKAV_REG_PATH							KIS_CFG_VAL_ANTIVIRUS_KEY
#define defKAV_UPLIVE_REG_PATH					KIS_CFG_VAL_UPDATE_KEY

#define defSET_SHOW_TIME_OUT					1

#define defINDEX_TXT_FILE_NAME					_T("index.txt")
#define defINDEX_FILE_NAME						_T("index.dat")

#define defINDEXALLSIGNS_FILE_NAME				_T("indexall.dat")
#define defINDEXALLSIGNS_TXT_FILE_NAME			_T("indexall.txt")

#define defUPDATE_DIRECTORY						_T("update\\")
#define defUPLIVE_DIRECTORY						_T("uplive\\")

#define defKISLIVE_CFG_FILE						_T("kislive.dat")

#define defUPDATE_SERVER_NAME					_T("Kingsoft Antivirus Update Server")	// 升级服务器的默认名称
#define defUPLIVE_FILE_MAPPING_NAME				_T("Kingsoft Antivirus Update File Mapping")
#define defKAV_UPLIVE_MUTEX_NAME				_T("KAV2000 Update 2.0")
#define defKISLIVE_WND_MSG_STRING				_T("UpliveWnd-{017BCAE9-657B-47b8-AAC6-76E012ACF5FD}")

#define defINDEX_INFO							_T("info")	// index.txt 文件中info项
#define defINDEX_INFO_Version					_T("v")	// index.txt 文件中info项
#define defUPLIVE_SINGS_INI						_T("signs.ini")
#define defUPLIVE_FILELIST_INI					_T("filelist.ini")
#define defUPLIVE_VINFO_INI						_T("vinfo.ini")

// 2010版本 在signs.ini中用多种病毒库描述。
#define defKAV_SIGN_Sign_SECTION				_T("Sign")
#define defKAV_SIGN_MpsmSign_SECTION			_T("MpsmSign")
#define defKAV_SIGN_ObjSign_SECTION				_T("ObjSign")
#define defKAV_SIGN_FsSign_SECTION				_T("FsSign")
#define defKAV_SIGN_VERSION_MAJOR_KEY			_T("Version")
#define defKAV_SIGN_VERSION_MINORVERSION_KEY	_T("BuildNum")
#define defKAV_SIGN_KSG_COUNT_KEY				_T("Count")

#define defKAV_SIGN_KSG_NAME_FORMAT				_T("File%d")
#define defKAV_SIGN_KSG_SIZE_FORMAT				_T("Size%d")
#define defKAV_SIGN_KSG_CRC_FORMAT				_T("Crc%d")
#define defKAV_SIGN_KSG_TIMESTAMP_FORMAT		_T("Timestamp%d")

#define defKAV_TEST_UPDATE_PATH					 _T("tryout")

#define defKAV_KPFW_CFG_FILE_NAME				_T("kpfwcfg.dat") 
#define defKAV_KSA_DATA_FILE_NAME				_T("ksadata.ksa") 

#define defKISLIVE_PASSP_VERIFY					_T("kislive")

#define PROXY_VAR_LEN							50
#define PROXY_SERVER_VAR_LEN					80

#define defVERSION_SIZE							20
#define defSECTION_SIZE                     (10 * 1024)

//////////////////////////////////////////////////////////////////////////
//
//
enum enumKislivxVersion
{
	enumKislivxVer_Err			= 404,
	enumKislivxVer_Normal		= 405,	// 正常版本。
	enumKislivxVer_WinPE		= 406,  // WinPE版本。
	enumKislivxVer_Mini			= 407,	// 迷你版本，采用VC6.0编译。
	enumKislivxVer_Fix			= 408,  // 升级修复版本。
	enumKislivxVer_Kws3			= 409,  // 网盾3.0升级，即正常版本的复制版！
	enumKislivxVer_Ksc3			= 409,
};

// Update Method
enum enumUpdateMethod
{
	// 在线升级。
	enumUpdateMethod_Online		= 0,
	enumUpdateMethod_LAN		= 1,
	// 是否是离线升级包模式，离线升级应是lan升级的子模式。	
	enumUpdateMethod_OffLine	= 2,
	enumUpdateMethod_WinSyn		= 3,
};

// Update Mode
enum enumUpdateMode
{
	// 升级模式。
	enumUpdateMode_Prompt		= 0,
	enumUpdateMode_Custom		= 1,
};

enum CurProcessStep
{
	enumAnalyseStart = 0,	// 分析文件的开始
	enumAnalyseCS3Ver,      // cs3 版本请求
	enumAnalysePrepearIndex, //准备Index
	enumAnalyseIndex,	// 分析 C:\KIS2008\Update\bin\index.txt
	enumAnalyseBandWidth,   //处理带宽申请
	enumAnalyseComplete		// 分析完所有 index.txt 的标识
};

enum enumKProxyMethod
{
	enumProxyMethod_DIRECT	= 0,
	enumProxyMethod_USEIE	= 1,
	enumProxyMethod_CUSTOM	= 2,
};

enum enumKProxyMode
{
	enumProxyMode_Socks4	= 0,
	enumProxyMode_Socks5	= 5, //必须是5，兼容旧版本。
	enumProxyMode_HTTP		= 1, //必须是1，兼容旧版本。
};

// Struct of Engine file header
typedef struct tagKAVEngineFileHeader
{
	char  cszDescription[48];
	DWORD dwSizeOfHeader;		
	DWORD dwMajorVersion;		    //Major Version
	DWORD dwMinorVersion;		    //Minor Version
	DWORD dwOrignMajorVersion;      //Orign PE File Major Version
	DWORD dwOrignMinorVersion;      //Orign PE File Minor Version
	DWORD dwOrignLength;
	DWORD dwCompressFlag;           //defKSCOMPRESS_FLAG
	DWORD dwSizePacked;             //Use for Local Second File Position
	DWORD dwCheckSum;
	char cszReserve[64];
}KAVEngineFileHeader, *PKAVEngineFileHeader;

// Struct of sign file header
typedef struct tagKAVSignFileHeader
{
	char  cszDescription[0x4E];     //"Kingsoft AntiVirus Database. Copyright (c) 1999, 2000 Kingsoft AntiVirus Lab.\x1A"
	unsigned short wDateBuildNum;

	DWORD dwVersion;                //dat file version
	DWORD dwDateStamp;              //Update Date, High 16 bit is year, Low 16 bit is Month(High 8 bit),Day(Low 8 bit)
	DWORD dwProcessVirusNum;        //Number of virus can be process
	WORD  wPackMethod;              //0: data is not pack 1: pack by Aplib
	WORD  wEncryptMethod;           //0: data is not Encrypt
	DWORD dwCheckSum;               //CheckSum value for data, CRC32
	DWORD dwDataAreaEntry;          //Data Area Entry offset in file
	DWORD dwDataAreaLen;            //Total Length of Data Area
	DWORD dwSignDataItemCount;

	KAE_KAVSIGNDATA_ITEM SignDataItem[defMAXSIGNDATAITEM];

} KAVSIGNFILEHEADER;

// 升级版本扩展信息结构。
struct KExVerInfo 
{
	enumKislivxVersion enVerInfo;
	u_int			   uSubVerInfo;
	u_int			   dwReserve[4];
	LPCWSTR			   pszReserve[4];
	KExVerInfo()
	{
		enVerInfo				= enumKislivxVer_Err;
		uSubVerInfo				= 0;
		ZeroMemory(dwReserve, sizeof(dwReserve));
		ZeroMemory(pszReserve, sizeof(pszReserve));
	}
};

// 升级版本扩展信息结构。
struct KExReserve 
{
	u_int			   dwReserve[4];
	LPCWSTR			   pszReserve[4];
};

struct KPROXY_SETTING
{
	BOOL			  bIsUseProxy;				// 是否使用代理。
	enumKProxyMethod  enumProxyMethod;              //DIRECT/USEIE/CUSTOM
	enumKProxyMode 	  enumProxyMode;                //PROXY_MODE_... when PROXY_METHOD_CUSTOM
												// Socks4 = 0 ; Socks5 = 1; HTTP = 2
	BOOL			  bEnableIEProxy;			//是否允许使用IE代理
	BOOL			  bProxyRequireAuthorization; 

	WCHAR szHostAddr[PROXY_SERVER_VAR_LEN+1];
	int  nHostPort;

	WCHAR szUserName[PROXY_VAR_LEN+1];
	WCHAR szPassword[PROXY_VAR_LEN+1];

	KPROXY_SETTING()
	{
		bIsUseProxy = FALSE;
		enumProxyMethod = enumProxyMethod_DIRECT;
		enumProxyMode = enumProxyMode_HTTP;
		nHostPort = 0;
		bEnableIEProxy = FALSE;
		bProxyRequireAuthorization = FALSE;

		ZeroMemory(szHostAddr, sizeof(szHostAddr));
		ZeroMemory(szUserName, sizeof(szUserName));
		ZeroMemory(szPassword, sizeof(szPassword));
	}

	KPROXY_SETTING & operator = (const KPROXY_SETTING & src)
	{
		// 指定长为defBufferMaxSize， 超过就会截断！！by ZC. 2010-3-26
		_tcsncpy(this->szHostAddr, src.szHostAddr, PROXY_SERVER_VAR_LEN);
		_tcsncpy(this->szUserName, src.szUserName, PROXY_VAR_LEN);
		_tcsncpy(this->szPassword, src.szPassword, PROXY_VAR_LEN);

		//
		this->bIsUseProxy = src.bIsUseProxy ? TRUE : FALSE;
		this->bEnableIEProxy = src.bEnableIEProxy ? TRUE : FALSE;
		this->nHostPort = src.nHostPort;
		this->bProxyRequireAuthorization = src.bProxyRequireAuthorization ? TRUE : FALSE;

		switch (src.enumProxyMethod)
		{
		case enumProxyMethod_DIRECT:
		case enumProxyMethod_USEIE:
		case enumProxyMethod_CUSTOM:
			this->enumProxyMethod = src.enumProxyMethod;
			break;
		default:
			this->enumProxyMethod = enumProxyMethod_DIRECT;
		}

		switch (src.enumProxyMode)
		{
		case enumProxyMode_Socks4:
		case enumProxyMode_Socks5:
		case enumProxyMode_HTTP:
			this->enumProxyMode = src.enumProxyMode;
			break;
		default:
			this->enumProxyMode = enumProxyMode_HTTP;
		}

		return (*this);
	}
};

struct KUPULIVE_SETTING 
{
	KPROXY_SETTING ProxySetting;

	TCHAR szLanPath[MAX_PATH];
	BOOL bIsAutoRebootFlag;
	BOOL bIsCleanTempDir;
	BOOL bIsResume;
	BOOL bIsUpdateFromLANServer;
	DWORD dwUpdateMode;
	DWORD dwUpdateMethod;
	BOOL bIsNeedReboot;
	BOOL bPopNotifyUser;

	KUPULIVE_SETTING()
	{
		bIsAutoRebootFlag = FALSE;
		bIsCleanTempDir = FALSE;
		bIsResume = FALSE;
		bIsUpdateFromLANServer = FALSE;
		dwUpdateMode = FALSE;
		bIsNeedReboot = FALSE;
		bPopNotifyUser = FALSE;
		ZeroMemory(szLanPath, sizeof(szLanPath));
	}
};

struct BandWidth 
{
	DWORD dwAvgBandWidth;
	DWORD dwSwatchAccount;

	BandWidth()
	{
		dwAvgBandWidth = 0;
		dwSwatchAccount = 0;
	}
};

const int cnCaptionSize = 300;
struct KNEWS_ITEM
{
	WCHAR szCaption[cnCaptionSize];
	WCHAR szLink[cnCaptionSize];
	WCHAR szTip[cnCaptionSize];
	int nColor;
	int nIcon;              
};

#pragma pack(push, 1)
struct FILE_MAPPING
{
	// 主界面的窗口句柄。
	HWND hWnd;
	// 是否是后台模式。
	BOOL bCurInAutoRunMode;
	// 保留。
	BOOL bReserve;
	// 主界面的窗口句柄是不可用。
	BOOL bInvalidInstance;
	// 程序句柄。
	HANDLE hInstance;
	// 保留。
	HWND hReserve;
};
#pragma pack(pop)

typedef FILE_MAPPING * PFILE_MAPPING;

#endif	//__UPLIVE_COMMON_DATA_DEF_H__
