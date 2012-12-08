#pragma once

#include <mstask.h>

#pragma comment(lib, "mstask.lib")

class CBkScheduledTask
{
public:
    CBkScheduledTask(LPCTSTR lpszTaskName)
        : m_strTaskName(lpszTaskName)
    {
        ::ZeroMemory(&m_TaskTrigger, sizeof(TASK_TRIGGER));
        m_TaskTrigger.cbTriggerSize = sizeof(TASK_TRIGGER);
    }

    HRESULT Exist(LPCWSTR lpszCheckAppPath = NULL)
    {
        HRESULT hRet = E_FAIL;
        CComPtr<ITaskScheduler> piScheduler = NULL;

        hRet = ::CoCreateInstance(
            CLSID_CTaskScheduler,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_ITaskScheduler,
            (void **) &piScheduler);
        if (FAILED(hRet))
            return hRet;

        CComPtr<ITask> piTask = NULL;

        hRet = piScheduler->Activate(m_strTaskName,
            IID_ITask,
            (IUnknown**)&piTask);
        if (SUCCEEDED(hRet))
        {
            if (NULL == lpszCheckAppPath)
                hRet = S_OK;
            else
            {
                LPWSTR pwszApplicationName = NULL;

                hRet = piTask->GetApplicationName(&pwszApplicationName);

                if (SUCCEEDED(hRet))
                {
                    CString strAppPath = pwszApplicationName;

                    if (0 == strAppPath.CompareNoCase(lpszCheckAppPath))
                        hRet = S_OK;
                    else
                        hRet = S_FALSE;

                    ::CoTaskMemFree(pwszApplicationName);
                }
            }
        }
        else
            hRet = S_FALSE;

        return hRet;
    }

    HRESULT Delete()
    {
        HRESULT hRet = E_FAIL;
        CComPtr<ITaskScheduler> piScheduler = NULL;

        hRet = ::CoCreateInstance(
            CLSID_CTaskScheduler,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_ITaskScheduler,
            (void **) &piScheduler);
        if (FAILED(hRet))
            return hRet;

        CComPtr<ITask> piTask = NULL;

        hRet = piScheduler->Activate(m_strTaskName,
            IID_ITask,
            (IUnknown**)&piTask);
        if (SUCCEEDED(hRet))
        {
            hRet = piScheduler->Delete(m_strTaskName);
        }

        return hRet;
    }

    HRESULT SetDailyTask(WORD wStartHour, WORD wStartMinute)
    {
        m_TaskTrigger.TriggerType = TASK_TIME_TRIGGER_DAILY;
        m_TaskTrigger.Type.Daily.DaysInterval = 1;

        m_TaskTrigger.wStartHour    = wStartHour;
        m_TaskTrigger.wStartMinute  = wStartMinute;

        return S_OK;
    }

    HRESULT SetLogonTask()
    {
        m_TaskTrigger.TriggerType = TASK_EVENT_TRIGGER_AT_LOGON;

        return S_OK;
    }

    HRESULT Save(LPCTSTR lpszTaskAppPath, LPCTSTR lpszTaskCmdParam = L"")
    {
        HRESULT hRet = E_FAIL;
        CComPtr<ITaskScheduler> piScheduler = NULL;

        hRet = ::CoCreateInstance(
            CLSID_CTaskScheduler,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_ITaskScheduler,
            (void **) &piScheduler);
        if (FAILED(hRet))
            return hRet;

        Delete();

        CComPtr<ITask> piTask = NULL;
        hRet = piScheduler->NewWorkItem(m_strTaskName,
            CLSID_CTask,
            IID_ITask,
            (IUnknown**)&piTask);
        if (FAILED(hRet))
            return hRet;

        hRet = piTask->SetApplicationName(lpszTaskAppPath);
        if (FAILED(hRet))
            return hRet;

        hRet = piTask->SetParameters(lpszTaskCmdParam);
        if (FAILED(hRet))
            return hRet;

        hRet = piTask->SetWorkingDirectory(L"");
        if (FAILED(hRet))
            return hRet;

        hRet = piTask->SetAccountInformation(L"", NULL);
        if (FAILED(hRet))
            return hRet;

        hRet = piTask->SetFlags(TASK_FLAG_INTERACTIVE);
        if (FAILED(hRet))
            return hRet;

        CComPtr<ITaskTrigger> piTaskTrigger = NULL;
        WORD wTrigNumber = 0;
        hRet = piTask->CreateTrigger(&wTrigNumber, &piTaskTrigger);
        if (FAILED(hRet))
            return hRet;

        SYSTEMTIME lt;

        ::GetLocalTime(&lt);

        m_TaskTrigger.wBeginYear    = lt.wYear;
        m_TaskTrigger.wBeginMonth   = lt.wMonth;
        m_TaskTrigger.wBeginDay     = lt.wDay;

        hRet = piTaskTrigger->SetTrigger(&m_TaskTrigger);
        if (FAILED(hRet))
            return hRet;

        CComPtr<IPersistFile> piPersistFile = NULL;
        hRet = piTask->QueryInterface(IID_IPersistFile, (LPVOID*)&piPersistFile);
        if (FAILED(hRet))
            return hRet;

        hRet = piPersistFile->Save(NULL, TRUE);
        if (FAILED(hRet))
            return hRet;

        return hRet;
    }

    static BOOL StartSvc()
    {
        BOOL bResult = FALSE;
        SC_HANDLE hSvcMgr = NULL, hSvc = NULL;
        LPQUERY_SERVICE_CONFIG pqsc = NULL;

        if (RunTimeHelper::IsVista())
            return TRUE;

        hSvcMgr = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (NULL == hSvcMgr)
            goto Exit0;

        hSvc = ::OpenService(hSvcMgr, L"Schedule", SERVICE_ALL_ACCESS);
        if (NULL == hSvc)
            goto Exit0;

        DWORD dwSize = 0;
        BOOL bRet = ::QueryServiceConfig(
            hSvc, NULL, 0, &dwSize
            );
        if (bRet)
            goto Exit0;

        if(ERROR_INSUFFICIENT_BUFFER != ::GetLastError())
            goto Exit0;

        pqsc = (LPQUERY_SERVICE_CONFIG)::LocalAlloc(LMEM_FIXED, dwSize);
        if (!pqsc)
            goto Exit0;

        bRet = ::QueryServiceConfig(
            hSvc, pqsc, dwSize, &dwSize
            );

        if (SERVICE_AUTO_START != pqsc->dwStartType)
        {
            bRet = ::ChangeServiceConfig(
                hSvc,                   // handle of service
                SERVICE_NO_CHANGE,      // service type: no change
                SERVICE_AUTO_START,     // service start type
                SERVICE_NO_CHANGE,      // error control: no change
                NULL,                   // binary path: no change
                NULL,                   // load order group: no change
                NULL,                   // tag ID: no change
                NULL,                   // dependencies: no change
                NULL,                   // account name: no change
                NULL,                   // password: no change
                NULL);
            if (!bRet)
                goto Exit0;
        }

        bRet = ::StartService(hSvc, NULL, NULL);
        if (!bRet)
        {
            if (ERROR_SERVICE_ALREADY_RUNNING != ::GetLastError())
                goto Exit0;
        }

        bResult = TRUE;

    Exit0:

        if (pqsc)
        {
            ::LocalFree(pqsc);
            pqsc = NULL;
        }

        if (hSvc)
        {
            ::CloseServiceHandle(hSvc);
            hSvc = NULL;
        }
        if (hSvc)
        {
            ::CloseServiceHandle(hSvcMgr);
            hSvcMgr = NULL;
        }

        return bResult;
    }

protected:
    CString m_strTaskName;
    TASK_TRIGGER m_TaskTrigger;
};