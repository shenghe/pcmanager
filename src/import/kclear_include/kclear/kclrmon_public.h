#ifndef KCLRMON_PUBLIC_H_
#define KCLRMON_PUBLIC_H_

//////////////////////////////////////////////////////////////////////////

#include <unknwn.h>

//////////////////////////////////////////////////////////////////////////
// 垃圾清理监控回调接口
[
    uuid(a63507aa-738c-42e1-939a-66384de2ccae)
]
interface IKClearMonitorCallback
{
    // 系统盘出现空间不足
    virtual HRESULT STDMETHODCALLTYPE OnSystemDriveLacking(
        ) = 0;
};

//////////////////////////////////////////////////////////////////////////
// 垃圾清理监控接口
[
    uuid(3cbcb1d5-5811-433f-8769-4c818dc1e7ff)
]
interface IKClearMonitor : IUnknown
{
    // 初始化
    virtual HRESULT STDMETHODCALLTYPE Initialize(
        IKClearMonitorCallback* piCallback
        ) = 0;

    // 反初始化
    virtual HRESULT STDMETHODCALLTYPE UnInitialize(
        ) = 0;

    // 系统盘是否空间不足
    virtual HRESULT STDMETHODCALLTYPE IsSystemDriveLacking(
        /*[IN]*/ int nSysVol,
        /*[OUT]*/ char* szSysVol,
        /*[OUT]*/ BOOL* pbLacking
        ) = 0;

    // 调用垃圾清理
    virtual HRESULT STDMETHODCALLTYPE LaunchKClear(
        /*[IN_OPT]*/ const char* pSubTab
        ) = 0;

    // 开始监控
    virtual HRESULT STDMETHODCALLTYPE StartMonitor(
        ) = 0;

    // 停止监控
    virtual HRESULT STDMETHODCALLTYPE StopMonitor(
        ) = 0;

    // 获得系统盘剩余空间
    virtual HRESULT STDMETHODCALLTYPE GetSystemDriveFreeSize(
        ULONGLONG* pqwSizeInMB
        ) = 0;
};

//////////////////////////////////////////////////////////////////////////
// 垃圾清理监控设置
[
    uuid(ba15b182-a5e2-4064-9f68-31af57cf0362)
]
interface IKClearMonitorSetting : IUnknown
{
    // 是否启用垃圾清理监控
    virtual HRESULT STDMETHODCALLTYPE GetEnableMointor(
        BOOL* pbEnable
        ) = 0;

    // 设置是否启用垃圾清理监控
    virtual HRESULT STDMETHODCALLTYPE SetEnableMointor(
        BOOL bEnable
        ) = 0;

    // 使用垃圾清理的提醒是否冷却
    virtual HRESULT STDMETHODCALLTYPE IsNotifyCoolDown(
        BOOL* pbCoolDown
        ) = 0;

    // 记录最后提醒时间
    virtual HRESULT STDMETHODCALLTYPE RecordLastNotify(
        ) = 0;
};

//////////////////////////////////////////////////////////////////////////

#endif // KCLRMON_PUBLIC_H_
