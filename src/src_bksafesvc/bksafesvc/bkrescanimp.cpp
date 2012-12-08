#include "StdAfx.h"
#include "bkrescanimp.h"
#include "miniutil/bksvcmdl.h"

CBkRescanImp::CBkRescanImp(void)
{
}

CBkRescanImp::~CBkRescanImp(void)
{
}

HRESULT CBkRescanImp::Initialize( DWORD dwProcId, int nVersion )
{
    HRESULT hr = S_OK;

    if ( BKCOM_EXPORT_MODULE_CBkRescan_VER != nVersion )
    {
        hr = E_NOTIMPL;
        goto Exit0;
    }

    BKSVC_DEBUG_TRACE( L"[CBkRescanImp] Initialize\n" );
    hr = __super::Initialize( dwProcId, nVersion );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }
Exit0:
    return hr;
}

HRESULT CBkRescanImp::Uninitialize()
{
    HRESULT hr = S_OK;

    BKSVC_DEBUG_TRACE( L"[CBkRescanImp] Uninitialize\n" );
    hr = __super::Uninitialize();

    return hr;
}

HRESULT CBkRescanImp::GetRescanResult( CAtlArray<BK_FILE_RESULT>& result )
{
    CUnkownRescan::Instance().GetRescanResult( result );

    return S_OK;
}