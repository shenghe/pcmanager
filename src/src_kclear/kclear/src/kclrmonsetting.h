#ifndef KCLRMONSETTING_H_
#define KCLRMONSETTING_H_

//////////////////////////////////////////////////////////////////////////

#include "kclear/kclrmon_public.h"
#include "common/kcomobject.h"

//////////////////////////////////////////////////////////////////////////

class KClearMonitorSetting : public IKClearMonitorSetting
{
public:
    KClearMonitorSetting();
    virtual ~KClearMonitorSetting();

    // 是否启用垃圾清理监控
    virtual HRESULT STDMETHODCALLTYPE GetEnableMointor(
        BOOL* pbEnable
        );

    // 设置是否启用垃圾清理监控
    virtual HRESULT STDMETHODCALLTYPE SetEnableMointor(
        BOOL bEnable
        );

    // 使用垃圾清理的提醒是否冷却
    virtual HRESULT STDMETHODCALLTYPE IsNotifyCoolDown(
        BOOL* pbCoolDown
        );

    // 记录最后提醒时间
    virtual HRESULT STDMETHODCALLTYPE RecordLastNotify(
        );

    KAS_BEGIN_COM_MAP(KClearMonitorSetting)
        KAS_COM_INTERFACE_ENTRY(IKClearMonitorSetting)
    KAS_END_COM_MAP()

private:
    CStringA m_strCfgFile;
};

//////////////////////////////////////////////////////////////////////////

#endif // KCLRMONSETTING_H_
