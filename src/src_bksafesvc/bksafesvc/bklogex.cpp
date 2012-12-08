#include "stdafx.h"
#include "common.h"
#include "bklogex.h"

#define CACHE_DLL_NAME  L"kcache.dll"

HRESULT CBkLogProxy::Initialize()
{
    HRESULT hr = E_FAIL;

    WinMod::CWinPath    Path;

    hr = CAppPath::Instance().GetLeidianAppPath( Path.m_strPath ); 
    if ( SUCCEEDED(hr) )
    {   
        Path.Append( CACHE_DLL_NAME );
        hr = _LoadBkLog( Path.m_strPath );
        if ( SUCCEEDED(hr) )
        {
            //成功返回
            return hr;
        }
    }
    //执行到这说名之前价值失败。
    hr = Path.GetModuleFileName(NULL);
    if ( SUCCEEDED(hr) )
    {
        Path.RemoveFileSpec();
        Path.Append( CACHE_DLL_NAME );
        hr = _LoadBkLog( Path.m_strPath );
        if ( SUCCEEDED(hr) )
        {
            //成功返回
            return hr;
        }
    }

    return hr;
}

HRESULT CBkLogProxy::Uninitialize()
{
    if ( NULL != m_hModLog )
    {
        ::FreeLibrary( m_hModLog );
        m_hModLog = NULL;
    }
    m_pfnBKCacheCreateObject = NULL;

    return S_OK;
}


HRESULT CBkLogProxy::BKCacheCreateObject( REFIID riid, void** ppvObj, DWORD dwBKEngVer )
{
    if ( FALSE == _CheckOpeateData() )
    {
        return E_FAIL;
    }
    return m_pfnBKCacheCreateObject( riid, ppvObj, dwBKEngVer );
}


HRESULT CBkLogProxy::_LoadBkLog( LPCTSTR pPathDll )
{
    if ( NULL == pPathDll )
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;

    do
    {
        if ( NULL == m_hModLog )
        {        
            m_hModLog = ::LoadLibrary( pPathDll );
            if ( NULL == m_hModLog )
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                break;
            }
        }
        if ( NULL == m_pfnBKCacheCreateObject )
        {
            m_pfnBKCacheCreateObject = (PFN_BKCacheCreateObject)::GetProcAddress( m_hModLog, FN_BKCacheCreateObject );
            if ( NULL == m_pfnBKCacheCreateObject )
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                break;
            }
        }
        hr = S_OK;
    } while( false );


    if ( FAILED(hr) )
    {
        if ( NULL != m_hModLog )
        {
            ::FreeLibrary( m_hModLog );
            m_hModLog = NULL;
        }
        m_pfnBKCacheCreateObject = NULL;
    }

    return hr;
}