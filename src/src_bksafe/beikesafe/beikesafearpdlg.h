#pragma once

//////////////////////////////////////////////////////////////////////////
#include "../../publish/kasesc/KComInterfacePtr.h"
#include "../../publish/kpfw/arpsetting_public.h"
#include "../../import/kpfw/antiarp/AntiArpInterface.h"

//////////////////////////////////////////////////////////////////////////


class CArpSetting
{
public:
    static CArpSetting& Instance();
    BOOL Install();
    BOOL UnInstall();
    IArpFwSetting* GetPtr();

protected:
    int KxeBaseInit(void);
    int KxeBaseUnInit(void);

    BOOL m_bKxeBaseInited;

private:
    CArpSetting();
    ~CArpSetting();

    KComInterfacePtr<IArpFwSetting> m_spIArpFwSetting;
};

//////////////////////////////////////////////////////////////////////////


