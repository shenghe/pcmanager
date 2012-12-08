#pragma once
//#include "uplivedef.h"

// 标准更新回调接口
class IStdUpdateCallback
{
public:
    /**
    * @brief    更新类型的回调
    * @param    dwUpdType        当前类型，库还是bin，两者分开来升级
    */
    virtual void STDMETHODCALLTYPE UpdateType( DWORD dwUpdType ) = 0;
    /**
    * @brief    更新的回调
    * @param    dwStatus        当前的状态
    * @param    lpInformation   字符串信息,具体含义取决于dwStatus
    * @param    dwPercentage    完成的百分比,取值从0到100
    * @retval   TRUE            继续升级
    * @retval   FALSE           中断升级
    */
    virtual BOOL STDMETHODCALLTYPE OnProgress(
        DWORD   dwStatus,
        DWORD   dwPercentage
        ) = 0;

    /**
    * @brief    更新错误的回调
    * @param    dwStatus        当前的状态
    * @param    hErrCode        错误码,定义见上面
    * @param    lpInformation   字符串信息,具体含义取决于dwStatus
    */
    virtual void STDMETHODCALLTYPE OnError(
        DWORD   dwStatus,
        HRESULT hErrCode, 
        LPCWSTR lpInformation
        ) = 0;

    /**
    * @brief    文件更新前通知
    * @param    lpszModuleName      模块名
    * @param    lpszFileName        文件名
    * @param    nType               文件类型（升级包、二进制文件、完整库文件、差量库文件等）
    * @param    lpszExtend          扩展字符串
    */
    virtual void STDMETHODCALLTYPE OnFilePreUpdated(
        LPCWSTR lpszModuleName, 
        LPCWSTR lpszFileName,
        int     nType,
        LPCWSTR lpszVersion = NULL,
        LPCWSTR lpszExtend = NULL
        ) = 0;

    /**
    * @brief    开始替换文件
    * @param    dwUpdType        当前类型，库还是bin，两者分开来升级
    * @retval   TRUE            继续升级
    * @retval   FALSE           中断升级
    */
    virtual BOOL STDMETHODCALLTYPE OnBeginUpdateFiles( DWORD dwUpdType ) = 0;

    /**
    * @brief    文件替换通知
    * @param    lpszModuleName      模块名
    * @param    lpszFileName        文件名
    * @param    nType               文件类型（升级包、二进制文件、完整库文件、差量库文件等）
    * @param    nOperation          操作类型（备份、替换、重启替换等）
    * @param    dwErrorCode         标准LastError错误码，0为成功其余为失败
    */
    virtual void STDMETHODCALLTYPE OnFileReplace(
        LPCWSTR lpszModuleName, 
        LPCWSTR lpszFileName,
        int     nType,
        int     nOperation,
        DWORD   dwErrorCode
        ) = 0;

    /**
    * @brief    文件更新完成通知
    * @param    lpszModuleName      模块名
    * @param    lpszFileName        文件名
    * @param    nType               文件类型（升级包、二进制文件、完整库文件、差量库文件等）
    */
    virtual void STDMETHODCALLTYPE OnFileUpdated(
        LPCWSTR lpszModuleName, 
        LPCWSTR lpszFileName,
        int     nType
        ) = 0;

	/**
    * @brief    下载更新文件之前通知
    * @param    lpszDownloadUrl      下载路径
    * @param    lpszFileName         下载文件名字
    * @param    lpszTargetFileName    目标文件名字
	* @param    lpszTargetPath		  目标路径
	* @param    lpszCRC			      下载文件CRC
	* @param    lpszHint			  描述信息
	* @param    lpszVer				  下载版本
	* @param    dwSize				  文件大小
	* @param    m_b3rd				  是否第三方
	* @param    nType				  更新类型
    */
    virtual void STDMETHODCALLTYPE OnPreDownloadUpdated(
		LPCWSTR lpszDownloadUrl,
		LPCWSTR lpszFileName,
		LPCWSTR lpszTargetFileName,
		LPCWSTR lpszTargetPath,
		LPCWSTR lpszCRC,
		LPCWSTR lpszHint,
		LPCWSTR lpszVer,
		DWORD   dwSize,
		BOOL    b3rd,
		int     nType
        ) = 0;
};

class   IUpdatePolicyCallback
{
public:
    //
    //  消息配置文件升级结果通知
    //  lpwszConfigPath 被下载到的路径
    //  hr              结果 小于0为失败码
    //
    virtual void MessageConfigUpdate( LPCWSTR lpwszConfigPath, HRESULT hr ) = 0;
    virtual void VirusLibUpdated( ULONGLONG ullVersion, BOOL bSuccessful = FALSE ) = 0;
    virtual void NotifyLibUpdate( LPCWSTR lpwszFileName ) = 0;
};

class   IUpdateDownloadFileCallback
{
public:
    virtual void BeginDownload() = 0;
    virtual void Retry( DWORD dwTimes ) = 0;
    // 返回FALSE终止下载
    virtual BOOL DownloadProgress(
        DWORD dwTotalSize, 
        DWORD dwReadSize 
        ) = 0;
    virtual void FinishDownload( HRESULT hError ) = 0;
};

class __declspec(uuid("DA8FE310-1A9A-4471-B693-EBCFBC7B103F"))
IUplive : public IUnknown
{
public:
    virtual HRESULT Initialize() = 0;
    virtual HRESULT Uninitialize() = 0;
    virtual void    SetCallback( 
        IStdUpdateCallback* piStd, 
        IUpdatePolicyCallback* piUpdPolicy 
        ) = 0;
    virtual HRESULT Start() = 0;
    virtual void    Stop() = 0;
    virtual void    WaitExit( DWORD dwWaitTime ) = 0;
    virtual HRESULT NotifyUpdateMessage() = 0;
    virtual HRESULT NotifyUpdate( int nType ) = 0;
};

class __declspec(uuid("C9E767AB-DC5D-4c77-8344-2520FB29A306"))
IUpdUtility : public IUnknown
{
public:
    //
    //  lpwszUrl : 目标文件URL
    //  lpwszTargetPath : 存放路径
    //  bPack : 是否为zlib的压缩包
    //  lpwszHashCheck : 校验hash，不为空则根据下载文件的hash与此比较，不相同则返回失败
    //
    virtual HRESULT DownloadFile( 
        LPCWSTR lpwszUrl, 
        LPCWSTR lpwszTargetPath, 
        BOOL bPack, 
        LPCWSTR lpwszHashCheck = NULL,
        IUpdateDownloadFileCallback* piCallback = NULL
        ) = 0;

    //
    //  lpwszExpression : 表达式
    //  TRUE : 表示成功或者符合表达式条件
    //  FALSE : 表示失败或者不符合表达式条件
    virtual BOOL DoExpression( LPCWSTR lpwszExpression ) = 0;
};

#define FN_CreateObject			"CreateObject"

//EXTERN_C HRESULT WINAPI CreateObject( REFIID riid, void** ppvObj );
typedef HRESULT (WINAPI *PFN_CreateObject)( REFIID riid, void** ppvObj );
