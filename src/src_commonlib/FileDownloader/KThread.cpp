/***************************************************************
 * @date:   2007-10-19
 * @author: BrucePeng
 * @brief:  The encapsulation of thread operation
 *          The base class of other thread class
 *          *Windows OS only*
 */

#include "KThread.h"

bool CThread::Start(void)
{
    if(enumTHREADS_EXECUTING == GetState())
        return false;

    if(enumTHREADS_FINISHED == GetState())
    {
        Stop();
    }

    m_hThread = (HANDLE)_beginthreadex(NULL, 
        0, 
        ThreadFun, 
        (LPVOID)this, 
        0, 
        &m_uThreadID
    );
    return (NULL != m_hThread) ? true : false;
}

bool CThread::Stop(void)
{
    DWORD dwRetCode = 0;

    if(NULL == m_hThread)
        return true;

    dwRetCode = WaitForSingleObject(m_hThread, INFINITE);
 
    CloseHandle(m_hThread);
    m_hThread = NULL;

    return (WAIT_OBJECT_0 == dwRetCode) ? true : false;
}

unsigned __stdcall CThread::ThreadFun(void* params)
{
    CThread* pThread = (CThread* )params;
    if(NULL != pThread)
    {
        pThread->SetState(enumTHREADS_EXECUTING);
        pThread->Run();
        pThread->SetState(enumTHREADS_FINISHED);
    }
    return 0;
}

