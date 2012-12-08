#pragma  once

#include "skylark2\bkatrun.h"

class CAutorunProxy
{
public:
    static CAutorunProxy& Instance()
    {
        static CAutorunProxy _AurorunProxy;
        return _AurorunProxy;
    }

    HRESULT Initialize();

    HRESULT Uninitialize();

    HRESULT BkCreateAutorunHandle( REFIID riid, void** ppvObj, DWORD dwBKEngVer = BKENG_CURRENT_VERSION );


private:
    CAutorunProxy():m_hModAutorun(NULL),m_pfnBKAtrunCreateObject(NULL){}
    ~CAutorunProxy()
    {
        if ( NULL != m_hModAutorun )
        {
            ::FreeLibrary( m_hModAutorun );
            m_hModAutorun = NULL;
        }
        m_pfnBKAtrunCreateObject = NULL;
    }

    HRESULT _LoadAutorun( LPCTSTR pPathDll );


    BOOL _CheckOpeateData()
    {
        return (NULL==m_pfnBKAtrunCreateObject) ? FALSE : TRUE;
    }
    HMODULE m_hModAutorun;
    PFN_BKAtrunCreateObject m_pfnBKAtrunCreateObject;
};


