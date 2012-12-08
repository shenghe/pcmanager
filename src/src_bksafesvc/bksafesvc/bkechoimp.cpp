#include "StdAfx.h"
#include "bkechoimp.h"
#include "insrecycle.h"

class   CBkEchoImpRes  : public CResBase<CBkEchoInner>
{
public:
    virtual ~CBkEchoImpRes(){}
    virtual HRESULT Destroy()
    {
        if ( m_pi )
        {
            m_pi->Uninitialize();
            delete m_pi;
            m_pi = NULL;
        }
        return S_OK;
    }
};

//////////////////////////////////////////////////////////////////////////

HRESULT CBkEchoInner::Initialize()
{
    HRESULT hr = S_OK;

    if ( NULL == m_pi )
    {
        hr = CScanEngineProxy::Instance().BKEngCreateEngine( 
            __uuidof( Skylark::IBKEngineEcho ), 
            (void**)&m_pi
            );
        if ( FAILED( hr ) )
        {
            goto Exit0;
        }

        hr = m_pi->Initialize();
        if ( FAILED( hr ) )
        {
            goto Exit0;
        }
    }
Exit0:
    return  S_OK;
}

HRESULT CBkEchoInner::Uninitialize()
{
    if ( m_pi )
    {
        Stop();

        WaitExit();

        m_pi->Uninitialize();
        m_pi->Release();
        m_pi = NULL;
    }

    return S_OK;
}

HRESULT CBkEchoInner::Echo( CAtlArray<CString>& FileList )
{
    HRESULT hr = S_OK;

    if ( !m_thread.IsExit() )
    {
        goto Exit0;
    }

    if ( m_pi )
    {
        for ( size_t i = 0; i < FileList.GetCount(); i++ )
        {
            hr = m_pi->AppendFile( FileList[ i ], ( void* )( ULONG_PTR )i );
            if ( FAILED( hr ) )
            {
                goto Exit0;
            }
        }

        hr = m_thread.Create( static_cast<WinMod::IWinRunnable*>(this) );
    }
Exit0:
    return hr;
}

DWORD   CBkEchoInner::EchoThread()
{
    if ( m_pi )
    {
        HRESULT hr = S_OK;
        Skylark::BKENG_ECHO_RESULT   result;

        Skylark::BKENG_INIT( &result );

        hr = m_pi->Echo( &result );

        {
            CObjGuard   guard( m_Lock );

            m_hEcho = hr;
            m_result.dwEchoStatus   = result.uEchoStatus;
            m_result.hEchoCode      = result.hEchoCode;
            m_result.bTrustProgram  = result.bTrustProgram;
        }
    }

    return 0;
}

HRESULT CBkEchoInner::GetEchoResult( BK_ECHO_RESULT& result )
{
    CObjGuard   guard( m_Lock );

    result.dwEchoStatus     = m_result.dwEchoStatus;
    result.hEchoCode        = m_result.hEchoCode;
    result.bTrustProgram    = m_result.bTrustProgram;

    return m_hEcho;
}

//////////////////////////////////////////////////////////////////////////

CBkEchoImp::CBkEchoImp(void)
    : m_pi( NULL )
{
}

CBkEchoImp::~CBkEchoImp(void)
{
}

HRESULT CBkEchoImp::Initialize( DWORD dwProcId, int nVersion )
{
    HRESULT hr = S_OK;

    if ( BKCOM_EXPORT_MODULE_CBkEcho_VER != nVersion )
    {
        hr = E_NOTIMPL;
        goto Exit0;
    }

    hr = __super::Initialize( dwProcId, nVersion );
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    {
        CObjGuard   guard( m_InsLock );

        if ( NULL == m_pi )
        {
            m_pi = new CBkEchoInner;
            if ( NULL == m_pi )
            {
                hr = E_OUTOFMEMORY;
                goto Exit0;
            }
        }
        else
        {
            goto Exit0;
        }
    }

    hr = m_pi->Initialize();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    CInsRecycle::Instance().RegisterActiveRes();
Exit0:
    return hr;
}

HRESULT CBkEchoImp::Uninitialize()
{
    HRESULT hr = S_OK;

    {
        CObjGuard   guard( m_InsLock );
        if ( m_pi )
        {
            CBkEchoImpRes* pRes = new CBkEchoImpRes();
            if ( pRes )
            {
                hr = pRes->ThrowToRecycle( m_pi );
                if ( SUCCEEDED( hr ) )
                {
                    m_pi = NULL;
                    goto Exit0;
                }
            }
        }
        else
        {
            goto Exit0;
        }
    }

    if ( m_pi )
    {
        m_pi->Uninitialize();
        delete m_pi;
        m_pi = NULL;
    }

Exit0:

    CInsRecycle::Instance().UnregisterActiveRes();

    hr = __super::Uninitialize();

    return hr;
}

HRESULT CBkEchoImp::Echo( CAtlArray<CString>& FileList )
{
    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_pi->Echo( FileList );
Exit0:
    return hr;
}

HRESULT CBkEchoImp::Stop()
{
    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    m_pi->Stop();
Exit0:
    return hr;
}

HRESULT CBkEchoImp::GetEchoResult( BK_ECHO_RESULT& result )
{
    HRESULT hr = S_OK;

    hr = CheckInstance();
    if ( FAILED( hr ) )
    {
        goto Exit0;
    }

    hr = m_pi->GetEchoResult( result );
Exit0:
    return hr;
}