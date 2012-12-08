#ifndef		_I_K_UPDATE_CALL_BACK_DATA_DEF_H_ 
#define		_I_K_UPDATE_CALL_BACK_DATA_DEF_H_ 


enum enumKCallBackID
{
	enumKCallBackID_Default			= 0,
	enumKCallBackID_Analyze,			
	enumKCallBackID_Download,
	enumKCallBackID_Download_OneFile,
	enumKCallBackID_Analyze_StepInfo,
	enumKCallBackID_Download_Init,
	enumKCallBackID_Download_OneFile_End,
	enumKCallBackID_Update_File_Result,
	enumKCallBackID_String_Data,
	enumKCallBackID_Number_Data,
	enumKCallBackID_Proxy_Data,
	enumKCallBackID_Channel_Data,
	enumKCallBackID_NotifyReUpdateUI,
	enumKCallBackID_ThreadExited,
	enumKCallBackID_NotifyReUpdateFinish,
	enumKCallBackID_NotifyShowBox,
	enumKCallBackID_VirusInfosShowBox
};	

struct K_ANALYZE_STATUS
{
	enumKCallBackID dwID;
	K_ANALYZE_STATUS()
		:dwID(enumKCallBackID_Analyze)
	{
	}
} ;

#define defBufferMaxSize (127)

struct K_DOWNLOAD_STATUS
{
	enumKCallBackID dwID;
	//WCHAR szFileName[defBufferMaxSize];				//下载文件名
	DWORD dwCurFileSize;    								//文件大小
	DWORD dwCurDownedSize;   								//已下载大小

	K_DOWNLOAD_STATUS()
		:dwID(enumKCallBackID_Download)
	{
	}
} ;

struct KDOWNLOAD_ONE_FILE_END 
{
	enumKCallBackID dwID;
	DWORD dwCurFileSize;
	BOOL bSuccess;
	KDOWNLOAD_ONE_FILE_END()
	{
		dwID = enumKCallBackID_Download_OneFile_End;
		bSuccess = FALSE;
		dwCurFileSize = 0;
	}
};

struct KDOWNLOAD_INIT_INFO 
{
	enumKCallBackID dwID;
	int nTotalFileSize;
	
	KDOWNLOAD_INIT_INFO()
	{
		dwID = enumKCallBackID_Download_Init;
		nTotalFileSize = 0;
	}
};

//该结构用于逻辑层向界面层传递要下载的文件信息。
struct KDOWNLOAD_FILE_INFO 
{
	enumKCallBackID dwID;
	int					nFileSize;								//下载文件大小。
	WCHAR 				szFileName[defBufferMaxSize + 1];			//下载文件名。
	WCHAR				szDescription[defBufferMaxSize + 1];		//下载项描述。

	KDOWNLOAD_FILE_INFO()
	{
		dwID = enumKCallBackID_Download_OneFile;
		ZeroMemory(szFileName, sizeof(szFileName));
		ZeroMemory(szDescription, sizeof(szDescription));
	}
};

// 消息	WM_ANALYSE_ONE_FILE	使用的结构体
struct StepInfo
{
	enumKCallBackID dwID;
	CurProcessStep m_PS;	// 当前在分析的 index.txt 文件
	DWORD m_dwTotalFileCount;	// 当前 index.txt 中的 UpdateItemCount 数目
	StepInfo()
	{
		dwID = enumKCallBackID_Analyze_StepInfo;
		m_PS = (CurProcessStep)0;
		m_dwTotalFileCount = 0;
	}
};


struct KString_Data
{	
	enumKCallBackID dwID;
	LPCTSTR lpStr;
	KString_Data()
	{
		dwID = enumKCallBackID_String_Data;
		lpStr = NULL;
	}
};

struct KNumber_Data 
{
	enumKCallBackID dwID;
	double dNumber;
	KNumber_Data()
	{
		dwID = enumKCallBackID_Number_Data;
		dNumber = 0;
	}
};

struct KPROXY_SETTING_CallBack : public KPROXY_SETTING
{
	enumKCallBackID dwID;

	KPROXY_SETTING_CallBack()
	{
		dwID = enumKCallBackID_Proxy_Data;
	}
};

struct Channel_Data_CallBack
{
	enumKCallBackID dwID;
	int		nChannelID;             //由产品ID * 1000 + channelID构成
	WCHAR	szChannelText[defBufferMaxSize + 1];
	size_t  stChannelSize;

	Channel_Data_CallBack()
	{
		dwID = enumKCallBackID_Channel_Data;
		ZeroMemory(szChannelText, sizeof(szChannelText));
	}
};

struct NotifyReUpdateUI
{
	enumKCallBackID dwID;
	BOOL bIsReboot;
	BOOL bHasUI;
	LPCTSTR pszNotifyMsg;
	u_int  uNotifyMsgSize;
	DWORD dwReserve[3];
	NotifyReUpdateUI()
	{
		dwID = enumKCallBackID_NotifyReUpdateUI;

		pszNotifyMsg = NULL;
		uNotifyMsgSize = 0;
		bIsReboot = FALSE;
		bHasUI = FALSE;
		dwReserve[0] = 0;
		dwReserve[1] = 0;
		dwReserve[2] = 0;
	}
};

struct NotifyReUpdateFi
{
	enumKCallBackID dwID;
	BOOL bIsReboot;
	BOOL bSucceed;
	DWORD dwReserve[3];

	NotifyReUpdateFi()
	{
		dwID = enumKCallBackID_NotifyReUpdateFinish;
		
		bIsReboot = FALSE;
		bSucceed = FALSE;
		dwReserve[0] = 0;
		dwReserve[1] = 0;
		dwReserve[2] = 0;
	}
};

struct ThreadExited_CallBack
{
	enumKCallBackID dwID;
	DWORD dwThreadId;
	ThreadExited_CallBack()
	{
		dwID = enumKCallBackID_ThreadExited;
		dwThreadId = 0;
	}
};

struct NotifyShowBox
{
	enumKCallBackID dwID;
	BOOL bIsFailedShowBox;
	DWORD dwErrCode;
	DWORD dwDownloadErrCode;
	DWORD dwReserve[10];
	NotifyShowBox()
	{
		dwID = enumKCallBackID_NotifyShowBox;
		this->bIsFailedShowBox = FALSE;
		this->dwErrCode = 0;
		this->dwDownloadErrCode = 0;
		ZeroMemory(dwReserve, sizeof(dwReserve));
	}
};

struct VirusInfosShowBox
{
	enumKCallBackID dwID;
	LPCWSTR pszVirusVer;
	long	lIncremenVirusCount;
	DWORD dwReserve[10];
	VirusInfosShowBox()
	{
		dwID = enumKCallBackID_VirusInfosShowBox;
		this->pszVirusVer = NULL;
		this->lIncremenVirusCount = 0;
		ZeroMemory(dwReserve, sizeof(dwReserve));
	}
};
//////////////////////////////////////////////////////////////////////////
// 回调参数定义

struct KCallBackParam
{
	enumKCallBackID dwID;
	byte * pbyBuffer;
	DWORD dwBufferSize;

	KCallBackParam()
		:dwID(enumKCallBackID_Default),
		pbyBuffer(NULL),
		dwBufferSize(0)
	{
	}
};


//////////////////////////////////////////////////////////////////////////
// 宏定义。
#define Struct2KCallBack(_struct, _callback) \
		_callback.dwID = _struct.dwID; \
		_callback.dwBufferSize = sizeof(_struct); \
		_callback.pbyBuffer = (byte*)&_struct; 

#if _MSC_VER <= 1200 // vc6.0			
#define KCallBack2Struct(_callback, _struct) \
			if (_callback.dwID == _struct.dwID && _callback.pbyBuffer && _callback.dwBufferSize == sizeof(_struct)) \
				memcpy(&_struct, _callback.pbyBuffer, _callback.dwBufferSize); 
#else
#define KCallBack2Struct(_callback, _struct) \
			if (_callback.dwID == _struct.dwID && _callback.pbyBuffer && _callback.dwBufferSize == sizeof(_struct)) \
				memcpy_s(&_struct, sizeof(_struct), _callback.pbyBuffer, _callback.dwBufferSize); 
#endif

#endif		// _I_K_UPDATE_CALL_BACK_DATA_DEF_H_
