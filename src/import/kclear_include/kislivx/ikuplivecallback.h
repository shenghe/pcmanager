// uplive的总回调。
#ifndef		_I_K_UPLIVE_CALL_BACK_H_ 
#define		_I_K_UPLIVE_CALL_BACK_H_ 

#include "kcallbackdatadef.h"
#include "ikanalyzecallback.h"
#include "ikdownloadcallback.h"
#include "ikupdatecallback.h"

typedef enum
{
	EVT_K_UPLIVE_SERVER_CONFIG_IS_NOT_EXIST = -1,	//Uplive.svr 不存在
	EVT_K_UPLIVE_SET_PARAM_FAILED,					//UpdateLogic设置参数失败
	EVT_K_UPLIVE_DISK_SPACE_NO_ENOUGH,				//磁盘空间不足

	EVT_K_UPLIVE_USER_BREAK,						//用户中断

	EVT_K_UPLIVE_START_VERCTRL,						//开始进行版本判断
	EVT_K_UPLIVE_DETECTING_VERCTRL_FAILED,			//版本判断失败
	EVT_K_UPLIVE_DETECTING_VERCTRL_FINISHED,		//版本判断成功

	EVT_K_UPLIVE_START_DEMANDBANDWIDTH,				//开始进行带宽申请
	EVT_K_UPLIVE_DETECTING_DEMANDBANDWIDTH_FAILED,	//带宽申请不通过
	EVT_K_UPLIVE_DETECTING_DEMANDBANDWIDTH_FINISHED,	//带宽申请成功
	
	EVT_K_UPLIVE_ANALYS_INDEX_START,				//开始分析Index
	EVT_K_UPLIVE_ANALYS_INDEX_FAILED,				//Index分析失败
	EVT_K_UPLIVE_ANALYS_INDEX_FINISHED,				//Index分析完成

	EVT_K_UPLIVE_DOWNLOAD_FILE_INIT,				//下载模块初始化。
	EVT_K_UPLIVE_DOWNLOAD_FILE_START,				//开始下载文件。
	EVT_K_UPLIVE_DOWNLOAD_FILE_FINISHED,			//下载文件结束。
	
	EVT_K_UPLIVE_UPDATE_FILE_START,					// 开始更新文件。
	EVT_K_UPLIVE_UPDATE_FILE_FINISHED,				// 更新文件结束。

	EVT_K_UPLIVE_VERIFYING_PASSPORT_START,			//验证通行证
	EVT_K_UPLIVE_VERIFY_PASSPORT_FAILED,			//验证通行证失败
	EVT_K_UPLIVE_VERIFY_PASSPORT_EXPIRED,			//通行证过期
	EVT_K_UPLIVE_UPDATE_TOO_MUCH,					//升级次数过多
	EVT_K_UPLIVE_VERIFY_PASSPORT_SUCCESS,			//验证通行证成功

	
	EVT_K_UPLIVE_PRODUCT_CHANNEL_INFO,				//产品各频道信息传送到界面


	EVT_K_UPLIVE_GET_CURRENT_POS_X,
	EVT_K_UPLIVE_GET_CURRENT_POS_Y,

	EVT_K_UPLIVE_AUTO_EXIT_APP,						// 自动退出uplive
	EVT_K_UPLIVE_RELOAD_APP,						//升级程序需要自更新
	//EVT_K_UPLIVE_THREAD_EXITED,					//告诉线程已结束(已不需要该回调，因该回调只表示工作线程正常退出)。

	EVT_K_UPLIVE_OFFLINE_PACKAGE_START,				// 离线升级包开始。
	EVT_K_UPLIVE_OFFLINE_PACKAGE_FINISHED,			// 离线升级包完成。
	EVT_K_UPLIVE_OFFLINE_PACKAGE_DISK_ERROR,		// 离线升级包磁盘不可用。
	EVT_K_UPLIVE_OFFLINE_PACKAGE_INDEX_ERROR,		// 离线升级包index文件不可用（不存在）。
	EVT_K_UPLIVE_OFFLINE_PACKAGE_DISK_FULL,			// 离线升级包磁盘空间不足。
	EVT_K_UPLIVE_WIN_PE_INIT_FAILED,				// WinPE同步初始失败，未安装毒霸，Windows毒霸配置错误。

} K_UPLIVE_EVT;


typedef enum
{
	EVT_K_DATA_SET_OPTION_AutoRunFlag = 0,			//是否后台升级
	EVT_K_DATA_SET_OPTION_SelfContinueFlag,				//自更新继续。
	EVT_K_DATA_SET_OPTION_ReUpdateFlag,				//重新更新文件。
	EVT_K_DATA_SET_OPTION_Proxy,					//代理选项
	EVT_K_DATA_SET_OPTION_UpdateMethod,				//升级方式
	EVT_K_DATA_SET_OPTION_UpdateMode,				//升级模式
	EVT_K_DATA_SET_OPTION_NotifyPPWizShowDlg,	//通知PPWsiz显示窗体
	EVT_K_DATA_SET_OPTION_OnTopWhenRun,				//是否置顶显示
	EVT_K_DATA_SET_OPTION_SyncToHpa,				//同方杀毒平台
	EVT_K_DATA_SET_OPTION_UpdateSelf,				//是否是自更新
	EVT_K_DATA_SET_OPTION_LanPath,					//局域网升级路径
	EVT_K_DATA_SET_OPTION_KISCommon_ProductName,	//框架名。
	EVT_K_DATA_SET_OPTION_KISCommon_IndexName,		//框架inde名。
	EVT_K_DATA_SET_OPTION_OfflinePackage_SpaceSize,		//离线升级包需要的空间大小。
	EVT_K_DATA_SET_OPTION_OfflinePackage_FileCount,		//离线升级包需要的文件大小。

}K_KISDATA_EVT;

typedef struct tagUpdateStatus
{
	DWORD				dwID;
	DWORD				dwSize;
	char*				pszBuf;
} K_UPDATE_STATUS;


class IKUpliveCallBack 
	: public IKAnalyzeCallBack, 
	  public IKDownloadCallBack, 
	  public IKUpdateCallBack
{
public:
	virtual int		OnUpliveEvent(K_UPLIVE_EVT nEvent, const KCallBackParam & stKCallBackParams = KCallBackParam()) = 0;
	virtual int		OnKisDataEvent(K_KISDATA_EVT nEvent, const KCallBackParam & stKCallBackParams = KCallBackParam()) = 0;
};

#endif		// #ifndef _I_K_UPLIVE_CALL_BACK_H_