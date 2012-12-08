#pragma once
#include "winmod/wintrustverifier.h"

class CSignVerifer
{
public:
    CSignVerifer(void);
    ~CSignVerifer(void);
    static CSignVerifer& Instance()
    {
        static CSignVerifer _ins;
        return  _ins;
    }

    HRESULT Initialize()
    {
        return m_verifer.TryLoadDll();
    }

    HRESULT VerifyFile( LPCTSTR lpPath );
    BOOL    IsTrustSign( LPCTSTR lpIsserName );
private:
private:
    WinMod::CWinTrustVerifier   m_verifer;
};
