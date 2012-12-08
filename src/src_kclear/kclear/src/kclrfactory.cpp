#include "stdafx.h"
#include "common/kcomobject.h"
#include "kclrfactory.h"

//////////////////////////////////////////////////////////////////////////

KClearFactory::KClearFactory()
{
}

KClearFactory::~KClearFactory()
{
}

//////////////////////////////////////////////////////////////////////////

KClearFactory& KClearFactory::Instance()
{
    static KClearFactory singleton;
    return singleton;
}

//////////////////////////////////////////////////////////////////////////

// 创建监控对象
IKClearMonitor* KClearFactory::CreateMonitor()
{
    IKClearMonitor* pObject = new KComObject<KClearMonitor>();
    pObject->AddRef();
    return pObject;
}

// 创建监控设置对象
IKClearMonitorSetting* KClearFactory::CreateMonitorSetting()
{
    IKClearMonitorSetting* pObject = new KComObject<KClearMonitorSetting>();
    pObject->AddRef();
    return pObject;
}

//////////////////////////////////////////////////////////////////////////
