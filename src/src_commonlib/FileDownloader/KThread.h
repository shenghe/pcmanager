/***************************************************************
 * @date:   2007-10-19
 * @author: BrucePeng
 * @brief:  The encapsulation of thread operation
 *          The base class of other thread class
 *          *Windows OS only*
 */

#ifndef K_THREAD_H_
#define K_THREAD_H_

#include <windows.h>
#include <process.h>

/**
 * @enum ENUM_THREAD_STATE
 * Enumerate the state of a thread
 */
enum ENUM_THREAD_STATE
{
    enumTHREADS_READY           = 0x01,
    enumTHREADS_EXECUTING       = 0x02,
    enumTHREADS_FINISHED        = 0x03
};

class CThread
{
public:
    // The default constructor
    CThread(void)
    : m_hThread(NULL), m_uThreadID(0), m_nState(enumTHREADS_READY){}

    // The default destructor
    virtual ~CThread(void) {}

public:
    // Start the thread
    // It will create the underlying thread
    bool Start(void);

    // Stop the thread
    // It will wait until The thread function returns
    bool Stop(void);

    // Get the state of thread
    // see: ENUM_THREAD_STATE
    inline int  GetState(void) const;

    // Is the thread alive.
    inline bool IsAlive(void) const;

protected:
    virtual void Run(void) = 0;

private:
    // The thread handle
    HANDLE          m_hThread;

    // The thread ID
    unsigned        m_uThreadID;

    // The thread state
    int             m_nState;

private:
    // Not support copy constructor and "operator="
    CThread(const CThread&);
    CThread& operator=(const CThread&);

    // Set the state of thread
    // see: ENUM_THREAD_STATE
    inline void SetState(int nState);

    static unsigned __stdcall ThreadFun(void* params);
};

inline bool CThread::IsAlive(void) const
{
    return (NULL == m_hThread);
}

inline int  CThread::GetState(void) const
{
    return m_nState;
}

inline void CThread::SetState(int nState)
{
    m_nState = nState;
}

#endif // K_THREAD_H_

