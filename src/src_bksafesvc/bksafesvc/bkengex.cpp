#include "stdafx.h"
#include "common.h"
#include "autorunex.h"
#include "bkengex.h"
#include "bklogex.h"
#include "backupex.h"

//#define AVS_DIR L"avs"

NS_SKYLARK_USING

#define ENGINE_DLL_NAME L"ksafeeng.dll"
HANDLE  g_hThread = NULL;
BOOL    g_bInitFalg = FALSE;

HRESULT CScanEngineProxy::LoadEngine( LPCTSTR pPathDll )
{
	if ( NULL == pPathDll )
	{
		return E_INVALIDARG;
	}



	HRESULT hr = E_FAIL;

	do 
	{
		if ( NULL == m_hModEng )
		{
			m_hModEng = ::LoadLibrary( pPathDll );
			if ( NULL == m_hModEng )
			{
				break;
			}
		}

		if ( NULL == m_pfnBKEngInitialize )
		{
			m_pfnBKEngInitialize = (PFN_BKEngInitialize)::GetProcAddress( m_hModEng, FN_BKEngInitialize );
			if ( NULL == m_pfnBKEngInitialize )
			{
				break;
			}
		}

		if ( NULL == m_pfnBKEngUninitialize )
		{
			m_pfnBKEngUninitialize = (PFN_BKEngUninitialize)::GetProcAddress( m_hModEng, FN_BKEngUninitialize );
			if ( NULL == m_pfnBKEngUninitialize )
			{
				break;
			}
		}

		if ( NULL == m_pfnBKEngCreateObject )
		{
			m_pfnBKEngCreateObject = (PFN_BKEngCreateObject)::GetProcAddress( m_hModEng, FN_BKEngCreateObject );
			if ( NULL == m_pfnBKEngCreateObject )
			{
				break;
			}
		}

		hr = S_OK;
	}while( false );

	if ( FAILED( hr ) )
	{
		m_pfnBKEngCreateObject = NULL;
		m_pfnBKEngInitialize       = NULL;
		m_pfnBKEngUninitialize     = NULL;
		if ( NULL != m_hModEng )
		{
			::FreeLibrary( m_hModEng );
			m_hModEng = NULL;
		}
	}

	return hr;
}


HRESULT CScanEngineProxy::CheckInnerParams()
{
	if ( (NULL == m_pfnBKEngInitialize) 
		|| (NULL == m_pfnBKEngUninitialize)
		|| (NULL == m_pfnBKEngCreateObject) )
	{
		return E_FAIL;
	}
	return S_OK;
}


HRESULT CScanEngineProxy::BKEngCreateEngine( REFIID riid, void** ppvObj, DWORD dwBKEngVer )
{
    HRESULT hr = CheckInnerParams();
	if ( FAILED(hr) )
	{
		return hr;
	}
 
	hr = m_pfnBKEngCreateObject( 
		riid, 
		ppvObj, 
		BKENG_CURRENT_VERSION 
		);

    return hr;
}


HRESULT CScanEngineProxy::EnableAvEngine()
{
    CComPtr<IBKEngAveController> spiAveController;
    HRESULT hr = BKEngCreateEngine(__uuidof(IBKEngAveController), (void**)&spiAveController);
    if (FAILED(hr))
    {
        return hr;
    }


    BKENG_AVE_LOAD aveLoad;
    BKENG_INIT(&aveLoad);
    return spiAveController->LoadAvEngine(&aveLoad);
}

HRESULT CScanEngineProxy::DisableAvEngine()
{
    CComPtr<IBKEngAveController> spiAveController;
    HRESULT hr = BKEngCreateEngine(__uuidof(IBKEngAveController), (void**)&spiAveController);
    if (FAILED(hr))
    {
        return hr;
    }


    return spiAveController->UnloadAvEngine();
}

HRESULT CScanEngineProxy::LoadDaemonAvSign()
{
    CComPtr<IBKEngAveController> spiAveController;
    HRESULT hr = BKEngCreateEngine(__uuidof(IBKEngAveController), (void**)&spiAveController);
    if (FAILED(hr))
    {
        return hr;
    }


    return spiAveController->LoadDaemonSign();
}

HRESULT CScanEngineProxy::UnloadDaemonAvSign()
{
    CComPtr<IBKEngAveController> spiAveController;
    HRESULT hr = BKEngCreateEngine(__uuidof(IBKEngAveController), (void**)&spiAveController);
    if (FAILED(hr))
    {
        return hr;
    }


    return spiAveController->UnloadDaemonSign();
}

HRESULT CScanEngineProxy::ReloadAvSign()
{
    CComPtr<IBKEngAveController> spiAveController;
    HRESULT hr = BKEngCreateEngine(__uuidof(IBKEngAveController), (void**)&spiAveController);
    if (FAILED(hr))
    {
        return hr;
    }


    return spiAveController->ReloadSign();
}



HRESULT CScanEngineProxy::EngineInit()
{
	HRESULT hr = E_FAIL;

    WinMod::CWinPath AvsPath;
    do 
    { 
        WinMod::CWinPath    Path;
        hr = CAppPath::Instance().GetLeidianAppPath( Path.m_strPath ); 
        if ( SUCCEEDED(hr) )
        {   
            Path.Append( ENGINE_DLL_NAME );
            hr = LoadEngine( Path.m_strPath );
            if ( SUCCEEDED(hr) )
            {
                hr = CAppPath::Instance().GetLeidianAvsPath( AvsPath.m_strPath );
                if ( SUCCEEDED( hr ) )
                {
                    hr = S_OK;
                    break;
                }
            }
        }
        //执行到这说名之前失败。
        hr = Path.GetModuleFileName(NULL);
        if ( SUCCEEDED(hr) )
        {
            Path.RemoveFileSpec();
            Path.Append( ENGINE_DLL_NAME );
            hr = LoadEngine( Path.m_strPath );
            if ( SUCCEEDED(hr) )
            {
                hr = AvsPath.GetModuleFileName( NULL );
                if ( SUCCEEDED(hr) )
                {
                    AvsPath.RemoveFileSpec();
                    AvsPath.Append( AVS_DIR );
                    hr = S_OK;
                    break;
                }

            }
        }
    } while ( false );

    if ( FAILED(hr) )
    {
        return hr;
    }

   
    Skylark::BKENG_PLATFORM_INIT InitData;
    BKENG_INIT( &InitData);
    //InitData.pAVERootDir = ( LPCWSTR )AvsPath;
    hr = m_pfnBKEngInitialize( &InitData );
    if (FAILED(hr))
    {
        return hr;
    }


    return S_OK;
}


HRESULT CScanEngineProxy::EngineUnit()
{
	HRESULT hr = CheckInnerParams();
	if ( FAILED(hr) )
	{
		return hr;
	}
	hr = m_pfnBKEngUninitialize();
    if ( SUCCEEDED(hr) )
    {
        if ( m_hModEng )
        {
            ::FreeLibrary( m_hModEng );
            m_hModEng = NULL;
        }
        m_pfnBKEngInitialize = NULL;
        m_pfnBKEngUninitialize = NULL;
        m_pfnBKEngCreateObject = NULL;
    }
    return hr;
}

HRESULT CScanEngineProxy::CheckInitAve()
{
    // 判断是否需要加载引擎
    CRegKey hRegKey;
    // 共存0, 注册表路径需要替换
    LONG lRet = hRegKey.Open(
        HKEY_LOCAL_MACHINE,
        KOpKsafeReg::ReplaceRegStr(L"SOFTWARE\\KSafe\\KEng").c_str(),
        KEY_READ);
    if (ERROR_SUCCESS == lRet)
    {
        DWORD dwInstallAve = 0;
        lRet = hRegKey.QueryDWORDValue(L"Install", dwInstallAve);
        if (ERROR_SUCCESS == lRet && dwInstallAve)
        {   // 启用毒霸引擎
            EnableAvEngine();

            // 暂时不加载常驻病毒库
            // LoadDaemonAvSign();
        }
        else
        {   // 禁用毒霸引擎
            DisableAvEngine();
        }
    }

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI EngInitThread()
{
    HRESULT hr = E_FAIL;

    do 
    {
        hr = CScanEngineProxy::Instance().CheckInitAve();
        if ( FAILED(hr) )
        {
            break;
        }

        hr = S_OK;
    } while ( false );

    return 0;
}

HRESULT  BkScanInitialize( )
{
    HRESULT hr = S_OK;

    hr = CScanEngineProxy::Instance().EngineInit();
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = CAutorunProxy::Instance().Initialize();
    if ( FAILED(hr) )
    {
        return hr;
    }
    hr = CBkLogProxy::Instance().Initialize();
    if ( FAILED(hr) )
    {
        return hr;
    }
    hr = CBkBackupProxy::Instance().Initialize();
    if ( FAILED(hr) )
    {
        return hr;
    }

    g_bInitFalg = TRUE;

    g_hThread = CreateThread( 
        NULL, 
        0, 
        ( LPTHREAD_START_ROUTINE )EngInitThread, 
        NULL,
        0,
        NULL
        );
    if ( NULL == g_hThread )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }

    return hr;
}


HRESULT  BkScanUninitialize()
{
    if ( g_hThread )
    {
        WaitForSingleObject( g_hThread, INFINITE );

        if (g_bInitFalg)
        {
            CBkBackupProxy::Instance().Uninitialize();
            CBkLogProxy::Instance().Uninitialize();
            CAutorunProxy::Instance().Uninitialize();
            CScanEngineProxy::Instance().EngineUnit();
        }

        g_bInitFalg = FALSE;
    }

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
