/********************************************************************
	created:	2010/03/08
	created:	8:3:2010   9:43
	filename: 	ksclock.h
	author:		Jiang Fengbing
	
	purpose:	锁的封装
*********************************************************************/

#ifndef KSCLOCK_INC_
#define KSCLOCK_INC_

//////////////////////////////////////////////////////////////////////////

#include "kscbase.h"

//////////////////////////////////////////////////////////////////////////

class KLockHandle 
{
public:
    KLockHandle() : m_hMutex(NULL)
    {
    }

    KLockHandle(const KLockHandle& handle)
    {
        m_hMutex = handle;
    }

    ~KLockHandle()
    {
    }

    operator HANDLE() const
    {
        return m_hMutex;
    }

    void Create()
    {
        if (!m_hMutex)
            m_hMutex = CreateMutex(NULL, FALSE, NULL);
    }

    void Destory()
    {
        if (m_hMutex)
        {
            CloseHandle(m_hMutex);
            m_hMutex = NULL;
        }
    }

    void Acquire()
    {
        WaitForSingleObject(m_hMutex, INFINITE);
    }

    void Release()
    {
        ReleaseMutex(m_hMutex);
    }

private:
    HANDLE m_hMutex;
};

class KLock 
{
public:
	KLock()
	{
		m_lockHandle.Create();
	}

    KLock(KLockHandle handle) : m_lockHandle(handle)
    {
    }

	~KLock()
	{
        m_lockHandle.Destory();
	}

    operator KLockHandle() const
    {
        return m_lockHandle;
    }

	void Acquire()
	{
        m_lockHandle.Acquire();
	}

	void Release()
	{
        m_lockHandle.Release();
	}

private:
	KLockHandle m_lockHandle;
};

class KAutoLock 
{
public:
	explicit KAutoLock(KLock& lockImpl) : m_lockImpl(lockImpl)
	{
		m_lockImpl.Acquire();
	}

	~KAutoLock()
	{
		m_lockImpl.Release();
	}

private:
	KLock& m_lockImpl;
};

//////////////////////////////////////////////////////////////////////////
//
// 使用方法: 把KLock作为类的成员, 在需要使用锁的时候在作用域内放置一个
// KAutoLock lock;即可
//
//////////////////////////////////////////////////////////////////////////

#endif	// KSCLOCK_INC_
