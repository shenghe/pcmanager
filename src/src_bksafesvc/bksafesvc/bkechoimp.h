#pragma once
#include "winmod/winthread.h"
#include "comproxy/com_utility_interface.h"
#include "com_svc_dispatch.h"
#include "comproxy/com_utility_xdx.h"
#include "bkengex.h"

class CBkEchoInner
    :   public WinMod::IWinRunnable
{
public:
    CBkEchoInner() : m_pi( NULL ), m_hEcho( S_FALSE )
    { 
        m_result.dwEchoStatus = 0; 
        m_result.hEchoCode = 0;
    }
public:
    HRESULT Initialize();
    HRESULT Uninitialize();
    HRESULT Echo( CAtlArray<CString>& FileList );
    HRESULT GetEchoResult( BK_ECHO_RESULT& result );

    virtual DWORD STDMETHODCALLTYPE  Run(){ return this->EchoThread(); }

    void    Stop()
    {
        if ( m_pi )
        {
            m_pi->Interrupt();
        }
    }

    void    WaitExit( DWORD dwWait = INFINITE )
    {
        m_thread.WaitExit( dwWait );
    }
private:
    DWORD   EchoThread();
private:
    WinMod::CWinThread  m_thread;
    Skylark::IBKEngineEcho*  m_pi;
    BK_ECHO_RESULT  m_result;
    HRESULT         m_hEcho;
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;

    CObjLock    m_Lock;
};

class CBkEchoImp
    : public IBkEcho,
      public CBkComRoot
{
public:
    CBkEchoImp(void);
    ~CBkEchoImp(void);
public:
    HRESULT Initialize( DWORD dwProcId, int nVersion );

    HRESULT Uninitialize();

    COM_METHOD_DISPATCH_BEGIN()
        COM_METHOD_DISPATCH_IN_ARG_1_NO_OUT( CBkEcho, Echo, FileList )
        COM_METHOD_DISPATCH_NO_ARG_OUT( CBkEcho, GetEchoResult, BK_ECHO_RESULT )
        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkEcho, Stop )
    COM_METHOD_DISPATCH_END
public:
    virtual HRESULT Echo( CAtlArray<CString>& FileList );
    virtual HRESULT GetEchoResult( BK_ECHO_RESULT& result );
    virtual HRESULT Stop();

private:
    HRESULT CheckInstance()
    {
        CObjGuard   guard( m_InsLock );

        return ( m_pi ? S_OK : E_NOINTERFACE );
    }
private:
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;

    CObjLock    m_Lock;
    CObjLock    m_InsLock;
    CBkEchoInner*   m_pi;
};
