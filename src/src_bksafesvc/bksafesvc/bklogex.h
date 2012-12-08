#pragma  once

#include "skylark2\bklog.h"

class CBkLogProxy
{
public:
    static CBkLogProxy& Instance()
    {
        static CBkLogProxy _BkLogProxy;
        return _BkLogProxy;
    }

    HRESULT Initialize();

    HRESULT Uninitialize();

    HRESULT BKCacheCreateObject( REFIID riid, void** ppvObj, DWORD dwBKEngVer = BKENG_CURRENT_VERSION );


private:
    CBkLogProxy():m_hModLog(NULL),m_pfnBKCacheCreateObject(NULL){}
    ~CBkLogProxy()
    {
        if ( NULL != m_hModLog )
        {
            ::FreeLibrary( m_hModLog );
            m_hModLog = NULL;
        }
    }

    HRESULT _LoadBkLog( LPCTSTR pPathDll );


    BOOL _CheckOpeateData()
    {
        if ( NULL == m_pfnBKCacheCreateObject )
        {
            return FALSE;
        }
        return TRUE;
    }
    HMODULE m_hModLog;
    PFN_BKCacheCreateObject m_pfnBKCacheCreateObject;
};

class CBkScanLog
{
public:
    CBkScanLog():m_piScanLogWriter(NULL){}
    ~CBkScanLog()
    {
        if ( NULL != m_piScanLogWriter )
        {
            m_piScanLogWriter->SaveToFile();
            m_piScanLogWriter->Release();
            m_piScanLogWriter = NULL;
        }
    }


    HRESULT Initialize()
    {
        HRESULT hr = CBkLogProxy::Instance().BKCacheCreateObject( __uuidof(Skylark::IBKEngScanLogWriter), (void**)&m_piScanLogWriter );
        if ( SUCCEEDED(hr) )
        {
           hr = m_piScanLogWriter->OpenStandard( BKCACHE_STD_SCAN_LOG );
        }
       
        if ( FAILED(hr) )
        {
            m_piScanLogWriter = NULL;
        }
        return hr;
    }


    HRESULT Uninitialize()
    {
        if ( NULL != m_piScanLogWriter )
        {
            m_piScanLogWriter->SaveToFile();
            m_piScanLogWriter->Release();
            m_piScanLogWriter = NULL;
        }
        return S_OK;
    }




    HRESULT WriteResultAsLog( LPCWSTR lpszFilePath, const Skylark::BKENG_RESULT* pResult )
    {
        if ( FALSE == CheckOperateData() )
        {
            return E_FAIL;
        }
        return m_piScanLogWriter->WriteResultAsLog( lpszFilePath, pResult );
    }

    HRESULT WriteLog( const Skylark::BKENG_SCAN_LOG& pLog )
    {
        if ( FALSE == CheckOperateData() )
        {
            return E_FAIL;
        }
        return m_piScanLogWriter->WriteLog( &pLog );
    }

    HRESULT SaveToFile()
    {
        if ( FALSE == CheckOperateData() )
        {
            return E_FAIL;
        }
        return m_piScanLogWriter->SaveToFile();
    }

private:
    BOOL CheckOperateData()
    {
        return (NULL == m_piScanLogWriter) ? FALSE : TRUE;
    }
    //记录日志接口
    Skylark::IBKEngScanLogWriter*    m_piScanLogWriter;
};