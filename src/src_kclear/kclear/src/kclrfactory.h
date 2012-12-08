#ifndef KCLRFACTORY_H_
#define KCLRFACTORY_H_

//////////////////////////////////////////////////////////////////////////

#include "kclrmon.h"
#include "kclrmonsetting.h"

//////////////////////////////////////////////////////////////////////////

class KClearFactory
{
public:
    static KClearFactory& Instance();

    // 创建监控对象
    IKClearMonitor* CreateMonitor();

    // 创建监控设置对象
    IKClearMonitorSetting* CreateMonitorSetting();

private:
    KClearFactory();
    ~KClearFactory();
};

#endif // KCLRFACTORY_H_
