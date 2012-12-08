#include "stdafx.h"
#include "common.h"
#include "autorunex.h"

#define AUTORUN_DLL_NAME    L"katrun.dll"

HRESULT CAutorunProxy::Initialize()
{
    HRESULT hr = E_FAIL;

    //
    WinMod::CWinPath    Path;

    hr = CAppPath::Instance().GetLeidianAppPath( Path.m_strPath ); 
    if ( SUCCEEDED(hr) )
    {   
        Path.Append( AUTORUN_DLL_NAME );
        hr = _LoadAutorun( Path.m_strPath );
        if ( SUCCEEDED(hr) )
        {
            //成功返回
            return hr;
        }
    }
    //执行到这说名之前失败。
    hr = Path.GetModuleFileName(NULL);
    if ( SUCCEEDED(hr) )
    {
        Path.RemoveFileSpec();
        Path.Append( AUTORUN_DLL_NAME );
        hr = _LoadAutorun( Path.m_strPath );
        if ( SUCCEEDED(hr) )
        {
            //成功返回
            return hr;
        }
    }
    //
    return hr;
}

HRESULT CAutorunProxy::Uninitialize()
{
    if ( NULL != m_hModAutorun )
    {
        ::FreeLibrary( m_hModAutorun );
        m_hModAutorun = NULL;
    }
    m_pfnBKAtrunCreateObject = NULL;

    return S_OK;
}

HRESULT CAutorunProxy::BkCreateAutorunHandle( REFIID riid, void** ppvObj, DWORD dwBKEngVer )
{
    if ( NULL == ppvObj )
    {
        return E_INVALIDARG;
    }

    if ( FALSE == _CheckOpeateData() )
    {
        return E_FAIL;
    }
    return m_pfnBKAtrunCreateObject( riid, ppvObj, dwBKEngVer );
}



HRESULT CAutorunProxy::_LoadAutorun( LPCTSTR pPathDll )
{
    if ( NULL == pPathDll )
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;


    do 
    {
        if ( NULL == m_hModAutorun )
        {
            m_hModAutorun = ::LoadLibrary( pPathDll );
            if ( NULL == m_hModAutorun )
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                break;
            }
        }
        if ( NULL == m_pfnBKAtrunCreateObject )
        {
            m_pfnBKAtrunCreateObject = (PFN_BKAtrunCreateObject)::GetProcAddress( m_hModAutorun, FN_BKAtrunCreateObject );
            if ( NULL == m_pfnBKAtrunCreateObject )
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                break;
            }
        }

        hr = S_OK;
    } while ( false );


    if ( FAILED(hr) )
    {
        if ( NULL != m_hModAutorun )
        {
            ::FreeLibrary( m_hModAutorun );
            m_hModAutorun = NULL;
        }   
        m_pfnBKAtrunCreateObject = NULL;
    }

    return hr;
}


