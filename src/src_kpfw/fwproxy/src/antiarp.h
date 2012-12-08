#ifndef ANTIARP_INC_
#define ANTIARP_INC_


//////////////////////////////////////////////////////////////////////////

#include "pkthdr.h"
#include "arpcommondef.h"
#include "antiarpinterface.h"
#include "arpcomfunc.h"


//////////////////////////////////////////////////////////////////////////

class CAntiArpHelper
{
public:
    typedef HRESULT (__stdcall *TDllGetClassObject)(
        IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv);

    static CAntiArpHelper& Instance();

    HRESULT GetInstance(const IID& riid, void** ppvObject);
    HRESULT GetInstance(IAntiArpDevC** ppiAntiArpDevC);
    HRESULT GetInstance(IArpTableSafeguard** ppiArpTableSafeguard);

//protected:
    int Initialize();
    void UnInitialize();

private:
    CAntiArpHelper();
    ~CAntiArpHelper();

    HMODULE             m_hModule;
    TDllGetClassObject  m_pDllGetClassObject;
};

//////////////////////////////////////////////////////////////////////////


#endif  // !ANTIARP_INC_

