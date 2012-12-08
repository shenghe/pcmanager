#ifndef KCLRMON_H_
#define KCLRMON_H_

//////////////////////////////////////////////////////////////////////////

#include "kclear/kclrmon_public.h"
#include "common/kcomobject.h"

//////////////////////////////////////////////////////////////////////////

class KClearMonitor : public IKClearMonitor
{
public:
    KClearMonitor();
    virtual ~KClearMonitor();

    // 初始化
    HRESULT STDMETHODCALLTYPE Initialize(
        IKClearMonitorCallback* piCallback
        );

    // 反初始化
    HRESULT STDMETHODCALLTYPE UnInitialize(
        );

    // 系统盘是否空间不足
    HRESULT STDMETHODCALLTYPE IsSystemDriveLacking(
        /*[IN]*/ int nSysVol,
        /*[OUT]*/ char* szSysVol,
        /*[OUT]*/ BOOL* pbLacking
        );

    // 调用垃圾清理
    HRESULT STDMETHODCALLTYPE LaunchKClear(
        /*[IN_OPT]*/ const char* pSubTab
        );

    // 开始监控
    HRESULT STDMETHODCALLTYPE StartMonitor(
        );

    // 停止监控
    HRESULT STDMETHODCALLTYPE StopMonitor(
        );

    // 获得系统盘剩余空间
    HRESULT STDMETHODCALLTYPE GetSystemDriveFreeSize(
        ULONGLONG* pqwSizeInMB
        );

    KAS_BEGIN_COM_MAP(KClearMonitor)
        KAS_COM_INTERFACE_ENTRY(IKClearMonitor)
    KAS_END_COM_MAP()

protected:
    BOOL IsSystemDriveLacking(BOOL& bLacking);
    static UINT WINAPI TimerThreadProc(void* pParam);
    static ULONGLONG GigaByte(double size);

private:
    HANDLE m_hTimerThread;
    HANDLE m_hExitEvent;
    IKClearMonitorCallback* m_piCallback;
    BOOL m_bVistaOrLater;
    CStringA m_strSysDrive;
    ULONGLONG m_qwSysDriveFreeSizeInMB;
};

//////////////////////////////////////////////////////////////////////////

#endif // KCLRMON_H_
