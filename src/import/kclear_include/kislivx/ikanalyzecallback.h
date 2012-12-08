// 分析模块的回调。
#ifndef		_I_K_ANALYZE_CALL_BACK_H_ 
#define		_I_K_ANALYZE_CALL_BACK_H_ 
#include "kcallbackdatadef.h"


typedef enum
{
	EVT_K_ANALYS_FILE_DAT_IS_NOT_EXIST = -1,			//Index.dat不存在
	EVT_K_ANALYS_FILE_IS_NOT_EXIST,						//Index.txt不存在
	EVT_K_ANALYS_ERROR_ACCESS_DENIED,					//Index访问失败
	EVT_K_ANALYS_PROCSS,								//分析进行中
	EVT_K_ANALYS_LAN_UPDATE_NOT_WHOLE_DATA,				//升级数据不完整
	EVT_K_ANALYS_WIN_SYNC_ITEM,							//Win同步一个更新项。
	EVT_K_ANALYS_WIN_SYNC_ITEM_ERR_TIP					//Win同步一个更新项错误提示。
} K_ANALYZE_EVT;


class IKAnalyzeCallBack
{
public:
	virtual int		OnAnalyzeEvent(K_ANALYZE_EVT nEvent, const KCallBackParam & stKCallBackParams = KCallBackParam()) = 0;
};


#endif		// #ifndef _I_K_ANALYZE_CALL_BACK_H_