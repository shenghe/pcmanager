#pragma once
#include "winmod/winrunnable.h"

class CResRoot
{
public:
    virtual ~CResRoot() = 0{};
    virtual HRESULT Destroy() = 0;
};

template<typename E>
class CResBase : public CResRoot
{
public:
    CResBase() : m_pi( NULL ){}
    virtual ~CResBase() = 0{}
    virtual HRESULT Destroy() = 0;
    HRESULT ThrowToRecycle( E* pi )
    {
        if ( pi )
        {
            m_pi = pi;

            return CInsRecycle::Instance().ThrowToRecycle( static_cast<CResRoot*>(this) );
        }

        return E_INVALIDARG;
    }
protected:
    E*  m_pi;
};

class CInsRecycle
    :   public  WinMod::AWinRunnable
{
public:
    CInsRecycle(void);
    ~CInsRecycle(void);
public:
    static CInsRecycle& Instance()
    {
        static  CInsRecycle _ins;

        return _ins;
    }

    HRESULT Initialize();
    HRESULT Uninitialize();

    HRESULT ThrowToRecycle( CResRoot* pi );

    virtual DWORD   STDMETHODCALLTYPE Run();

    void    NotifyStop()
    {
        m_hNotifyStop.Set();
    }

    void    RegisterActiveRes()
    {
        InterlockedIncrement(  &m_resCount );
    }

    void    UnregisterActiveRes()
    {
        InterlockedDecrement( &m_resCount );
    }

private:
    void    DoRecycle();
private:
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;

    ATL::CEvent             m_hNotifyStop;
    CObjLock                m_lock;
    CAtlList<CResRoot*>     m_resList;
    LONG                    m_resCount;
};
