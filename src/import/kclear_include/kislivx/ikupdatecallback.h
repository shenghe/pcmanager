// 更新模块的回调。
#ifndef		_I_K_UPDATE_CALL_BACK_H_ 
#define		_I_K_UPDATE_CALL_BACK_H_ 
#include "kcallbackdatadef.h"

enum K_UPDATE_EVT
{
	EVT_K_UPDATE_NEED_UPDATE_BEGIN,					// 需要更新开始。
	EVT_K_UPDATE_NEED_UPDATE_END,					// 需要更新结束。
	EVT_K_UPDATE_NOT_UPDATE_FOR_DOWNLOAD_FAILED,	// 因下载失败，故不进行更新。
	EVT_K_UPDATE_NOT_UPDATE_FOR_NOT_NEED_UPDATE,	// 因无文件更新，故不进行更新。
	EVT_K_UPDATE_ONE_FILE_UPDATE_BEGIN,				// 更新一个文件开始。
	EVT_K_UPDATE_ONE_FILE_UPDATE_END,				// 更新一个文件结束。
	EVT_K_UPDATE_NEED_REBOOOT,						// 需要重启。
	EVT_K_UPDATE_SUCCESSED,							// 更新成功。
	EVT_K_UPDATE_FAILED,							// 更新失败。
	EVT_K_UPDATE_NEED_UPDATE_SELF,					// 需要自更新。
	EVT_K_UPDATE_SHOW_TITLE_RESULT,
	EVT_K_UPDATE_SHOW_BOX_ITEM_RESULT,
	EVT_K_UPDATE_SHOW_BOX_ITEM_SUMMARY,
	EVT_K_UPDATE_SHOW_VIRUS_INFOS,					// 传出病毒库版本、新增特征数。
	EVT_K_UPDATE_SHOW_BOX_SUCCESSED,				// ShowBox更新成功状态。
	EVT_K_UPDATE_SHOW_BOX_FAILED,					// ShowBox更新失败状态。
	EVT_K_UPDATE_SHOW_BOX_NOTIFY_FAILED,			// ShowBox更新Windows/WinPe状态。
	EVT_K_UPDATE_SHOW_BOX_END,						// ShowBox结束。
	EVT_K_UPDATE_REUPDATE_BEGIN	= 40,				// ReUpdate更新开始。
	EVT_K_UPDATE_REUPDATE_END,						// ReUpdate更新结束。
	EVT_K_UPDATE_REUPDATE_SHOW_PROGRESS,			// 显示ReUpdate更新进度条。
	EVT_K_UPDATE_REUPDATE_NOTIFY_UI,				// 显示ReUpdate更新进度条。
	EVT_K_UPDATE_REUPDATE_FINISH,					// 显示ReUpdate更新进度条。
};

class IKUpdateCallBack
{
public:
	virtual int	OnUpdateEvent(K_UPDATE_EVT enumEvent, const KCallBackParam & stKCallBackParams = KCallBackParam()) = 0;
};

#endif		// _I_K_UPDATE_CALL_BACK_H_
