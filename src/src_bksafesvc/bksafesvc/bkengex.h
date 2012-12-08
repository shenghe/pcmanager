#pragma once
#include "skylark2\bkeng.h"
#include "skylark2\bklog.h"
#include "other.h"
#include "avpperf.h"

extern HANDLE  g_hThread;
extern BOOL    g_bInitFalg;

class CScanEngineProxy
{
public:
	static CScanEngineProxy& Instance()
	{
		static	CScanEngineProxy	ins;

		return ins;
	}

 
	//整个引擎初始化
	HRESULT EngineInit();
	HRESULT EngineUnit();

    HRESULT CheckInitAve();

	//引擎实例初始化
    HRESULT BKEngCreateEngine( REFIID riid, void** ppvObj, DWORD dwBKEngVer = BKENG_CURRENT_VERSION );


    HRESULT EnableAvEngine();
    HRESULT DisableAvEngine();
    HRESULT LoadDaemonAvSign();
    HRESULT UnloadDaemonAvSign();
    HRESULT ReloadAvSign();

private:
    CScanEngineProxy() : 
       m_hModEng( NULL ), 
           m_pfnBKEngInitialize( NULL ), 
           m_pfnBKEngUninitialize( NULL ), 
           m_pfnBKEngCreateObject( NULL )
       {}

       ~CScanEngineProxy()
       {
           // unload dll
           if ( NULL != m_hModEng )
           {
               ::FreeLibrary( m_hModEng );
           }
           m_pfnBKEngInitialize    = NULL;
           m_pfnBKEngUninitialize  = NULL;
           m_pfnBKEngCreateObject  = NULL;
       }
    HRESULT LoadEngine( LPCTSTR pPathDll );
    HRESULT CheckInnerParams();

    HMODULE m_hModEng;
	PFN_BKEngInitialize     m_pfnBKEngInitialize;
	PFN_BKEngUninitialize   m_pfnBKEngUninitialize;
	PFN_BKEngCreateObject   m_pfnBKEngCreateObject;

};



/**
* @brief    初始化扫描模块
*/
HRESULT  BkScanInitialize();



/**
* @brief    反初始化扫描模块
*/

HRESULT  BkScanUninitialize();




class CBkScanEngEx
{
public:
    CBkScanEngEx():m_piScanEng(NULL){}
    ~CBkScanEngEx()
    {
        if ( m_piScanEng )
        {
            m_piScanEng->Uninitialize();
            m_piScanEng->Release();
            m_piScanEng = NULL;
        }
    }
public:

    HRESULT  Initialize(void* pvReserved = NULL)
    {
        return S_OK;
    }



     HRESULT  Uninitialize()
     {
         if ( m_piScanEng )
         {
             Skylark::CAvpEchoPerf::EchoEnginePerf(m_piScanEng);
             m_piScanEng->Uninitialize();
             m_piScanEng->Release();
             m_piScanEng = NULL;
         }
         return S_OK;
     }


     HRESULT SetScanEngSetting( const BK_SCAN_SETTING& Setting )
     {
         if ( FALSE == _CheckOperateData() )
         {
             return E_FAIL;
         }

         HRESULT hr = E_FAIL;
         Skylark::BKENG_SETTINGS ScanSetting = {0};
         hr = m_piScanEng->GetScanSettings( &ScanSetting );
         if ( FAILED(hr) )
         {
             return hr;
         }
         CSetting::TranslateSetting( Setting, ScanSetting );
         //设置扫描策略
         hr = m_piScanEng->SetScanSettings( &ScanSetting );
         if ( FAILED(hr) )
         {
             return hr;
         }

         return hr;
     }



     HRESULT  AppendFile(
        LPCWSTR pszFileFullName,
        const WIN32_FIND_DATAW* pFindData = NULL,
        void*   pvFlag          = NULL,
        DWORD   dwHighWaterMark = 5000)
     {
         if ( NULL == pszFileFullName )
         {
             return E_INVALIDARG;
         }
         if ( FALSE == _CheckOperateData() )
         {
             return E_FAIL;
         }
         return m_piScanEng->AppendFile( pszFileFullName, pFindData, pvFlag, dwHighWaterMark );
     }


     HRESULT  AppendFileEx(
         LPCWSTR                    pszFileFullName,
         Skylark::BKENG_SCAN_PARAM* pScanParam)
     {
         if ( NULL == pszFileFullName || NULL == pScanParam )
         {
             return E_INVALIDARG;
         }
         if ( FALSE == _CheckOperateData() )
         {
             return E_FAIL;
         }
         return m_piScanEng->AppendFileEx( pszFileFullName, pScanParam );
     }



     HRESULT  Commit()
     {
         if ( FALSE == _CheckOperateData() )
         {
             return E_FAIL;
         }
         return m_piScanEng->Commit();
     }




     HRESULT  AsyncScan( Skylark::IBKEngineCallback* piCallback)
     {
         if ( NULL == piCallback )
         {
             return E_INVALIDARG;
         }
         return m_piScanEng->AsyncScan( piCallback );
     }




     HRESULT  NotifyStop()
     {
         if ( FALSE == _CheckOperateData() )
         {
             return E_FAIL;
         }
         return m_piScanEng->NotifyStop();
     }



     DWORD    WaitForFinish(DWORD dwWaitTime)
     {
         if ( FALSE == _CheckOperateData() )
         {
             return E_FAIL;
         }
         return m_piScanEng->WaitForFinish( dwWaitTime );
     }




     HRESULT  Pause()
     {
         if ( FALSE == _CheckOperateData() )
         {
             return E_FAIL;
         }
         return m_piScanEng->Pause();
     }



     HRESULT  Resume()
     {
         if ( FALSE == _CheckOperateData() )
         {
             return E_FAIL;
         }
         return m_piScanEng->Resume();
     }

     HRESULT ReCreateScanEng()
     {
         if ( m_piScanEng )
         {
             m_piScanEng->Uninitialize();
             m_piScanEng->Release();
             m_piScanEng = NULL;
         }
         HRESULT hr = E_FAIL;
         hr = CScanEngineProxy::Instance().BKEngCreateEngine( __uuidof(Skylark::IBKEngine), (void**)&m_piScanEng );
         if ( FAILED(hr) )
         {
             return hr;
         }
         hr = m_piScanEng->Initialize();
         if ( FAILED(hr) )
         {
             m_piScanEng->Release();
             m_piScanEng = NULL;
             return hr;
         }
         return S_OK;
     }
private:
    BOOL _CheckOperateData()
    {
        return (NULL == m_piScanEng) ? FALSE : TRUE ;
    }

private:
    //引擎接口
    Skylark::IBKEngine*		    m_piScanEng;
};


class CEngineLiteEx
{
public:
    CEngineLiteEx():m_piFileScan(NULL){}
    ~CEngineLiteEx()
    {
        if ( m_piFileScan )
        {
            m_piFileScan->Uninitialize();
            m_piFileScan->Release();
            m_piFileScan = NULL;
        }
    }
    
public:
    HRESULT Initialize()
    {
        HRESULT hr = E_FAIL;

        hr = CScanEngineProxy::Instance().BKEngCreateEngine( __uuidof(Skylark::IBKEngineLite), (void**)&m_piFileScan );
        if ( FAILED(hr) )
        {
            return hr;
        }
        hr = m_piFileScan->Initialize();
        if ( FAILED(hr) )
        {
            m_piFileScan->Release();
            m_piFileScan = NULL;
        }
        
        return hr;
    }

    HRESULT Uninitialize()
    {
        if ( m_piFileScan )
        {
            m_piFileScan->Uninitialize();
            m_piFileScan->Release();
            m_piFileScan = NULL;
        }
        return S_OK;
    }
    
    HRESULT Scan(
        LPCWSTR                      lpszFilePath,
        DWORD                        dwScanMask,
        Skylark::BKENG_RESULT*       pScanResult,
        Skylark::IBKEngineCallback*  piCallback = NULL,
        void*               pvReserved = NULL)
    {
        if ( FALSE == _CheckOperateData() )
        {
            return E_FAIL;
        }
        return m_piFileScan->Scan( lpszFilePath, dwScanMask, pScanResult, piCallback, pvReserved );
    }

    HRESULT ScanHash(
        const Skylark::BKENG_HASH_SCAN_PARAM*    pHashScanParam,
        Skylark::BKENG_RESULT*                   pScanResult,
        Skylark::IBKEngineCallback*              piCallback = NULL,
        void*                                    pvReserved = NULL
        )
    {
        if ( FALSE == _CheckOperateData() )
        {
            return E_FAIL;
        }

        return m_piFileScan->ScanHash( pHashScanParam, pScanResult, piCallback, pvReserved );
    }

    HRESULT HintClean(
        LPCWSTR             lpszFilePath,
        DWORD               dwScanMask,
        const Skylark::BKENG_RESULT* pScanResult,
        Skylark::BKENG_RESULT*       pCleanResult,
        Skylark::IBKEngineCallback*  piCallback = NULL,
        void*               pvReserved = NULL)
    {
        if ( FALSE == _CheckOperateData() )
        {
            return E_FAIL;
        }
        return m_piFileScan->HintClean( lpszFilePath, dwScanMask, pScanResult, pCleanResult, piCallback, pvReserved );
    }

    HRESULT SetScanSetting( const BK_SCAN_SETTING& Setting )
    {
        if ( FALSE == _CheckOperateData() )
        {
            return E_FAIL;
        }

        HRESULT hr = E_FAIL;

        Skylark::BKENG_SETTINGS ScanSetting = {0};
        hr = m_piFileScan->GetScanSettings( &ScanSetting );
        if ( FAILED(hr) )
        {
            return hr;
        }
        CSetting::TranslateSetting( Setting, ScanSetting );
        //设置扫描策略
        hr = m_piFileScan->SetScanSettings( &ScanSetting );
        if ( FAILED(hr) )
        {
            return hr;
        }
        return hr;
    }

private:
    BOOL _CheckOperateData()
    {
        return (NULL==m_piFileScan)?(FALSE):(TRUE);
    }
    Skylark::IBKEngineLite*     m_piFileScan;
};