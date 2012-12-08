/********************************************************************
	created:	2010/07/12
	created:	12:7:2010   19:10
	filename: 	ionekeyclean.h
	author:		Jiang Fengbing
	
	purpose:	一键优化的内部接口
*********************************************************************/

#ifndef IONEKEYCLEAN_INC_
#define IONEKEYCLEAN_INC_

//////////////////////////////////////////////////////////////////////////

[
    uuid(c8e7fa72-cb34-49b6-bfc1-8b194dc9eefb)
]
interface ICleanCallback
{
    // @brief 开始清理
    virtual void __stdcall OnBegin(
        ) = 0;

    // @brief 清理结束
    virtual void __stdcall OnEnd(
        ) = 0;

    // @brief 清理被取消
    virtual void __stdcall OnCancel(
        ) = 0;

    // @brief 报告清理项目
    // @param szType: 类型字符串
    // @param szItem: 具体内容的字符串
    // @param nSize: 如果是清理垃圾，这个是清理的大小，单位为M
    virtual void __stdcall OnCleanEntry(
        /* [in] */ int nType,
        /* [in, size_is(nType)] */const wchar_t* szType,
        /* [in] */ int nItem,
        /* [in, size_is(nItem)] */const wchar_t* szItem,
        /* [in] */ double nSize
        ) = 0;
};

//////////////////////////////////////////////////////////////////////////

[
    uuid(701adf0b-4096-476a-9382-e01a431389d3)
]
interface ICleanTask
{
    // @brief 设置清除回调
    virtual void __stdcall SetCallback(
        ICleanCallback* piCallback
        ) = 0;

    // @brief 开始清理
    // @param nTask: szTask的字符串长度
    // @param szTask: 清理任务, 例如: "ie_temp,win_temp"
    virtual bool __stdcall StartClean(
        /* [in] */ int nTask,
        /* [in, size_is(nTask)] */const char* szTask
        ) = 0;

    // @brief 停止清理
    virtual bool __stdcall StopClean(
        ) = 0;

    // @brief 销毁自己
    virtual void __stdcall Release(
        ) = 0;
};

//////////////////////////////////////////////////////////////////////////

[
    uuid(c8e7fa72-cb34-49b6-bfc1-8b194dc9eefb)
]
interface ICleanCallback2
{
    // @brief 开始扫描
    virtual void __stdcall OnScanBegin(
        ) = 0;

    // @brief 开始清理
    virtual void __stdcall OnCleanBegin(
        ) = 0;

    // @brief 扫描结束
    virtual void __stdcall OnScanEnd(
        ) = 0;

    // @brief 清理结束
    virtual void __stdcall OnCleanEnd(
        ) = 0;

    // @brief 扫描被取消
    virtual void __stdcall OnScanCancel(
        ) = 0;

    // @brief 清理被取消
    virtual void __stdcall OnCleanCancel(
        ) = 0;

    // @brief 报告扫描项目
    // @param szType: 类型字符串
    // @param szItem: 具体内容的字符串
    // @param nSize: 如果是清理垃圾，这个是扫描的大小，单位为M
    virtual void __stdcall OnScanEntry(
        /* [in] */ int nType,
        /* [in, size_is(nType)] */const wchar_t* szType,
        /* [in] */ int nItem,
        /* [in, size_is(nItem)] */const wchar_t* szItem,
        /* [in] */ double nSize
        ) = 0;

    // @brief 报告清理项目
    // @param szType: 类型字符串
    // @param szItem: 具体内容的字符串
    // @param nSize: 如果是清理垃圾，这个是清理的大小，单位为M
    virtual void __stdcall OnCleanEntry(
        /* [in] */ int nType,
        /* [in, size_is(nType)] */const wchar_t* szType,
        /* [in] */ int nItem,
        /* [in, size_is(nItem)] */const wchar_t* szItem,
        /* [in] */ double nSize
        ) = 0;
};

//////////////////////////////////////////////////////////////////////////

[
    uuid(701adf0b-4096-476a-9382-e01a431389d3)
]
interface ICleanTask2
{
    // @brief 设置扫描和清除回调
    virtual void __stdcall SetCallback(
        ICleanCallback2* piCallback
        ) = 0;

    // @brief 开始扫描
    // @param nTask: szTask的字符串长度
    // @param szTask: 扫描任务, 例如: "ie_temp,win_temp"
    virtual bool __stdcall StartScan(
        /* [in] */ int nTask,
        /* [in, size_is(nTask)] */const char* szTask
        ) = 0;

    // @brief 开始清理
    // @param nTask: szTask的字符串长度
    // @param szTask: 清理任务, 例如: "ie_temp,win_temp"
    virtual bool __stdcall StartClean(
        /* [in] */ int nTask,
        /* [in, size_is(nTask)] */const char* szTask
        ) = 0;

    // @brief 停止扫描
    virtual bool __stdcall StopScan(
        ) = 0;

    // @brief 停止清理
    virtual bool __stdcall StopClean(
        ) = 0;

    // @brief 销毁自己
    virtual void __stdcall Release(
        ) = 0;
};

//////////////////////////////////////////////////////////////////////////

typedef bool (__cdecl *GetCleaner_t)(const GUID& riid, void** ppv);
bool __cdecl GetCleaner(const GUID& riid, void** ppv);

//////////////////////////////////////////////////////////////////////////

#endif  // IONEKEYCLEAN_INC_
