#include "StdAfx.h"
#include "insrecycle.h"
#include "common/runtimelog.h"

CInsRecycle::CInsRecycle(void)
    : m_resCount( 0 )
{
}

CInsRecycle::~CInsRecycle(void)
{
}

HRESULT CInsRecycle::Initialize()
{
    HRESULT hr = S_OK;

    if ( NULL == m_hNotifyStop )
    {
        hr = m_hNotifyStop.Create( NULL, TRUE, FALSE, NULL );
    }

    return hr;
}

HRESULT CInsRecycle::Uninitialize()
{
    HRESULT hr = S_OK;

    if ( m_hNotifyStop )
    {
        m_hNotifyStop.Set();

        AWinRunnable::WaitExit(INFINITE);

        m_hNotifyStop.Close();
    }

    return hr;
}

HRESULT CInsRecycle::ThrowToRecycle( CResRoot* pi )
{
    if ( NULL == AWinRunnable::m_hThread )
    {
        return E_FAIL;
    }

    if ( pi )
    {
        CObjGuard   guard( m_lock );

        m_resList.AddTail( pi );

        return S_OK;
    }
    else
    {
        return  E_INVALIDARG;
    }
}

void    CInsRecycle::DoRecycle()
{
    CAtlList<CResRoot*> tmplist;
    {
        CObjGuard   guard( m_lock );

        if ( m_resList.GetCount() )
        {
            tmplist.AddTailList( &m_resList );
            m_resList.RemoveAll();
        }
    }

    while( tmplist.GetCount() )
    {
        CResRoot*   pi = tmplist.RemoveHead();

        if ( pi )
        {
            pi->Destroy();

            delete pi;
        }
    }
}

DWORD   STDMETHODCALLTYPE CInsRecycle::Run()
{
    CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[CInsRecycle] recycle thread start ok" );

    while( WAIT_TIMEOUT == WaitForSingleObject( m_hNotifyStop, 1000 ) )
    {
        DoRecycle();
    }

    // todo: 待发布时，这里的无限循环需要去掉
    while( InterlockedCompareExchange( &m_resCount, 0, 0 ) )
    {
        Sleep( 1000 );
    }

    DoRecycle();

    CRunTimeLog::WriteLog(WINMOD_LLVL_DEBUG, L"[CInsRecycle] recycle thread exit" );
    return 0;
}