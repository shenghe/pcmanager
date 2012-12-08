#pragma once

#include "winmod/winosver.h"
#include <miniutil/bkcmdline.h>

#ifdef _DEBUG
#include <miniutil/bkconsole.h>


inline BOOL _Debug_Trace(const TCHAR *format, ...)
{
    CString strTrace;
    va_list args;

    va_start(args, format);
    strTrace.FormatV(format, args);
    va_end(args);

    BOOL bRet = bkconsole::printf(L"%s", strTrace);
    ATLTRACE(L"%s", strTrace);

    return bRet;
}

#define BKSVC_DEBUG_TRACE _Debug_Trace
#else
#define BKSVC_DEBUG_TRACE
#endif

#define SVC_ORDERGROUP_1        L"SchedulerGroup"
#define SVC_ORDERGROUP_2        L"SpoolerGroup"

#define CMDLINE_PARAM_NAME_INSTALL       L"install"
#define CMDLINE_PARAM_NAME_UNINSTALL     L"uninstall"
#define CMDLINE_PARAM_NAME_DEBUG         L"debug"
#define CMDLINE_PARAM_DEBUG_BKGROUND     L"b"
#define CMDLINE_PARAM_NAME_START         L"start"
#define CMDLINE_PARAM_NAME_RESTART       L"restart"
#define CMDLINE_PARAM_NAME_STOP          L"stop"
#define CMDLINE_PARAM_NAME_SEVICE        L"svc"
#define CMDLINE_PARAM_NAME_REG           L"regserver"
#define CMDLINE_PARAM_NAME_UNREG         L"unregserver"

/* Overidable Functions
*
*   void OnStart();                         服务启动过程
*   void OnStop();                          收到停止通知
*   void OnPause();                         收到暂停通知
*   void OnContinue();                      收到继续通知
*   void OnShutdown();                      收到关机通知
*   void OnFinal();                         最后的操作
*   HRESULT OnInitializeSecurity();         初始化安全描述
*   void OnCustomRequest(DWORD dwOpcode);   收到自定义通知
*   BOOL Install();                         安装
*   BOOL Uninstall();                       卸载
*   BOOL ServiceStart();                    启动
*   BOOL ServiceStop();                     停止
*   HRESULT Start(int nShowCmd);            服务方式运行
*   HRESULT Run(int nShowCmd);              服务或调试方式运行
*/

template <class T, UINT t_nServiceNameID, UINT t_nDisplayNameID = 0, UINT t_nDescriptionID = 0>
class CBkServiceModuleT : public CAtlServiceModuleT<T, t_nServiceNameID>
{
public:

    CBkServiceModuleT() throw()
    {
        if (0 != t_nDisplayNameID)
            ::LoadString(
                _AtlBaseModule.GetModuleInstance(), 
                t_nDisplayNameID, m_szDisplayName, 
                sizeof(m_szDisplayName) / sizeof(TCHAR));

        if (0 != t_nDescriptionID)
            ::LoadString(
                _AtlBaseModule.GetModuleInstance(), 
                t_nDescriptionID, m_szDescription, 
                sizeof(m_szDescription) / sizeof(TCHAR));
    }

    int WinMain(int nShowCmd) throw()
    {
        if (CAtlBaseModule::m_bInitFailed)
        {
            ATLASSERT(0);
            return -1;
        }

        T* pT = static_cast<T*>(this);
        HRESULT hr = S_OK;

        _ParseCommandLine(nShowCmd, hr);

#ifdef _DEBUG
        _AtlWinModule.Term();
#endif    // _DEBUG

        pT->OnFinal();

        return hr;
    }

    HRESULT PreMessageLoop(int nShowCmd) throw()
    {
        HRESULT hRet = __super::PreMessageLoop(nShowCmd);
        if (FAILED(hRet))
            return hRet;

        T* pT = static_cast<T*>(this);

        pT->OnStart();

        return hRet;
    }

    HRESULT PostMessageLoop() throw()
    {
        T* pT = static_cast<T*>(this);

        pT->OnStop();

        return __super::PostMessageLoop();
    }

    HRESULT InitializeSecurity() throw()
    {
        T* pT = static_cast<T*>(this);

        return pT->OnInitializeSecurity();
    }

    void OnUnknownRequest(DWORD dwOpcode) throw()
    {
        T* pT = static_cast<T*>(this);

        pT->OnCustomRequest(dwOpcode);
    }

    void Handler(DWORD dwOpcode) throw()
    {
        T* pT = static_cast<T*>(this);

        if (SERVICE_CONTROL_STOP == dwOpcode)
        {
            SetServiceStatus(SERVICE_STOP_PENDING);
            pT->OnStop();
            PostThreadMessage(m_dwThreadID, WM_QUIT, 0, 0);
        }
        else
            __super::Handler(dwOpcode);
    }

    HRESULT Start(int nShowCmd) throw()
    {
        // 仅用于service方式启动
        T* pT = static_cast<T*>(this);
        
        CRegKey keyAppID;
        LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_READ);
        if (lRes != ERROR_SUCCESS)
        {
            m_status.dwWin32ExitCode = lRes;
            return HRESULT_FROM_WIN32 ( m_status.dwWin32ExitCode );
        }

        CRegKey key;
        lRes = key.Open(keyAppID, pT->GetAppIdT(), KEY_READ);
        if (lRes != ERROR_SUCCESS)
        {
            m_status.dwWin32ExitCode = lRes;
            return HRESULT_FROM_WIN32 ( m_status.dwWin32ExitCode );
        }

        TCHAR szValue[MAX_PATH];
        DWORD dwLen = MAX_PATH;
        lRes = key.QueryStringValue(_T("LocalService"), szValue, &dwLen);

        if (lRes != ERROR_SUCCESS)
        {
            HRESULT hRet = pT->RegisterAppId(TRUE);
            if (FAILED(hRet))
                return hRet;

            hRet = pT->RegisterServer(FALSE);
            if (FAILED(hRet))
                return hRet;

            hRet = _RegisterTypeLib();
            if (FAILED(hRet))
                return hRet;
        }

        SERVICE_TABLE_ENTRY st[] =
        {
            { m_szServiceName, _ServiceMain },
            { NULL, NULL }
        };

        if (::StartServiceCtrlDispatcher(st) == 0)
            m_status.dwWin32ExitCode = GetLastError();
        return HRESULT_FROM_WIN32 ( m_status.dwWin32ExitCode );
    }
protected:

    BOOL _ParseCommandLine(int nShowCmd, HRESULT &hRet)
    {
        CBkCmdLine _CmdLine;
        BOOL bRet = _CmdLine.Analyze(::GetCommandLine());

        if (!bRet)
            return FALSE;

        T* pT = static_cast<T*>(this);

        if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_INSTALL))
        {
            hRet = pT->Install() ? S_OK : E_FAIL;

            return FALSE;
        }
        else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_UNINSTALL))
        {
            hRet = pT->Uninstall() ? S_OK : E_FAIL;

            return FALSE;
        }
        else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_SEVICE))
        {
            pT->Start(nShowCmd);
            return FALSE;
        }
        else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_START))
        {
            pT->StartService();
            return FALSE;
        }
        else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_RESTART))
        {
            pT->StopService();
            pT->StartService();
            return FALSE;
        }
        else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_STOP))
        {
            pT->StopService();
            return FALSE;
        }
        else if (_CmdLine.HasParam(CMDLINE_PARAM_NAME_DEBUG))
        {
            m_bService = FALSE;

            m_dwThreadID = ::GetCurrentThreadId();

            pT->RegisterAppId(FALSE);

#ifdef _DEBUG

            if (0 != _CmdLine[CMDLINE_PARAM_NAME_DEBUG].String().CompareNoCase(CMDLINE_PARAM_DEBUG_BKGROUND))
            {
                HANDLE hThread = ::CreateThread(NULL, 0, _DebugThread, (LPVOID)(DWORD_PTR)m_dwThreadID, 0, NULL);

                ::CloseHandle(hThread);
            }
#endif
            pT->Run(nShowCmd);

            m_bService = TRUE;
            pT->RegisterAppId(TRUE);

            return FALSE;
        }
        else if ( _CmdLine.HasParam( CMDLINE_PARAM_NAME_REG ) )
        {
            hRet = pT->RegisterAppId();
            if (SUCCEEDED(hRet))
                hRet = pT->RegisterServer(TRUE);
            return FALSE;
        }
        else if ( _CmdLine.HasParam( CMDLINE_PARAM_NAME_UNREG ) )
        {
            hRet = pT->UnregisterServer(TRUE);
            if (SUCCEEDED( hRet ))
                hRet = pT->UnregisterAppId();
            return FALSE;
        }
        else
        {
            return pT->OnParseCommandLine(_CmdLine, nShowCmd, hRet);
        }

        return FALSE;
    }

    BOOL OnParseCommandLine(CBkCmdLine &_CmdLine, int nShowCmd, HRESULT &hRet)
    {
        return FALSE;
    }

    inline HRESULT RegisterAppId(bool bService = false) throw()
    {
        HRESULT hr = T::UpdateRegistryAppId(TRUE);
        if (FAILED(hr))
            return hr;

        CRegKey keyAppID;
        LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
        if (lRes != ERROR_SUCCESS)
            return AtlHresultFromWin32(lRes);

        CRegKey key;

        lRes = key.Create(keyAppID, T::GetAppIdT());
        if (lRes != ERROR_SUCCESS)
            return AtlHresultFromWin32(lRes);

        key.DeleteValue(_T("LocalService"));

        if (!bService)
            return S_OK;

        key.SetStringValue(_T("LocalService"), m_szServiceName);

        return S_OK;
    }

    HRESULT _RegisterTypeLib()
    {
        CComBSTR bstrPath;
        CComPtr<ITypeLib> pTypeLib;
        HRESULT hr = AtlLoadTypeLib((HINSTANCE)&__ImageBase, NULL, &bstrPath, &pTypeLib);
        if (SUCCEEDED(hr))
        {
            hr = ::RegisterTypeLib(pTypeLib, bstrPath, NULL);
        }
        return hr;
    }

    BOOL Install() throw()
    {
        CString strSvcCmdLine;
        LPTSTR lpszCmdLine = strSvcCmdLine.GetBuffer(MAX_PATH + 3);

        DWORD dwRet = ::GetModuleFileName(NULL, lpszCmdLine + 1, MAX_PATH);
        if( dwRet == 0 || dwRet == MAX_PATH )
        {
            strSvcCmdLine.ReleaseBuffer(0);
            return FALSE;
        }

        // Quote the FilePath before calling CreateService
        lpszCmdLine[0] = _T('\"');
        lpszCmdLine[dwRet + 1] = _T('\"');

        strSvcCmdLine.ReleaseBuffer(dwRet + 2);

        strSvcCmdLine += L" -" CMDLINE_PARAM_NAME_SEVICE;

        if (IsInstalled())
        {
            // change config
            BOOL bResult = TRUE;

            SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

            if (hSCM != NULL)
            {
                SC_HANDLE hService = ::OpenService(
                    hSCM, 
                    m_szServiceName, 
                    SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG
                    );
                if (hService != NULL)
                {
                    ChangeServiceConfig( 
                        hService,
                        SERVICE_WIN32_OWN_PROCESS,
                        SERVICE_DEMAND_START,
                        SERVICE_ERROR_NORMAL,
                        strSvcCmdLine,
                        WinMod::CWinOSVer::IsWinXPOrLater() ? SVC_ORDERGROUP_1 : SVC_ORDERGROUP_2,
                        NULL,
                        _T("RPCSS\0"),
                        NULL,
                        NULL,
                        (0 != t_nDisplayNameID) ? m_szDisplayName : m_szServiceName
                        );
                    ::CloseServiceHandle(hService);
                }
                ::CloseServiceHandle(hSCM);

                return StartService();
            }
            return bResult;
        }

        T* pT = static_cast<T*>(this);

        HRESULT hRet = pT->RegisterAppId(TRUE);
        if (FAILED(hRet))
            return FALSE;

        hRet = pT->RegisterServer(FALSE);
        if (FAILED(hRet))
            return FALSE;

        hRet = _RegisterTypeLib();
        if (FAILED(hRet))
            return FALSE;

        SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (hSCM == NULL)
            return FALSE;

        SC_HANDLE hService = ::CreateService(
            hSCM, m_szServiceName, 
            (0 != t_nDisplayNameID) ? m_szDisplayName : m_szServiceName,
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_DEMAND_START, 
            SERVICE_ERROR_NORMAL,
            strSvcCmdLine,
            WinMod::CWinOSVer::IsWinXPOrLater() ? SVC_ORDERGROUP_1 : SVC_ORDERGROUP_2,
            NULL, 
            _T("RPCSS\0"), 
            NULL, 
            NULL
            );
        if (NULL == hService)
        {
            ::CloseServiceHandle(hSCM);
            return FALSE;
        }

        if (0 != t_nDescriptionID)
        {
            SERVICE_DESCRIPTION sd = {m_szDescription};

            ::ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, (LPVOID)&sd);
        }

        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hSCM);

        return StartService();
    }

    HRESULT UnregisterAppId() throw()
    {
        CRegKey keyAppID;
        LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
        if (lRes != ERROR_SUCCESS)
            return AtlHresultFromWin32(lRes);

        CRegKey key;
        lRes = key.Open(keyAppID, T::GetAppIdT(), KEY_WRITE);
        if (lRes != ERROR_SUCCESS)
            return AtlHresultFromWin32(lRes);
        key.DeleteValue(_T("LocalService"));

        return T::UpdateRegistryAppId(FALSE);
    }

    BOOL Uninstall() throw()
    {
        if (!IsInstalled())
            return TRUE;

        T* pT = static_cast<T*>(this);

        HRESULT hRet = E_FAIL;
        
        hRet = pT->UnregisterServer(TRUE);
        hRet = pT->UnregisterAppId();

        BOOL bRet = StopService();

        SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (hSCM == NULL)
            return FALSE;

        SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_STOP | DELETE);
        if (hService == NULL)
        {
            ::CloseServiceHandle(hSCM);

            return FALSE;
        }

        bRet = ::DeleteService(hService);

        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hSCM);

        return bRet;
    }

    BOOL StartService()
    {
        BOOL bRet = FALSE;

        SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (hSCM == NULL)
            goto Exit0;

        SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_START | SERVICE_QUERY_STATUS);
        if (hService == NULL)
            goto Exit0;

        DWORD dwTickCount = ::GetTickCount();

        SERVICE_STATUS ss;

        do 
        {
            bRet = ::QueryServiceStatus(hService, &ss);
            if (!bRet)
                goto Exit0;

            switch (ss.dwCurrentState)
            {
            case SERVICE_RUNNING:

                goto Exit1;

            case SERVICE_STOPPED:

                bRet = ::StartService(hService, 0, NULL);
                if (!bRet)
                    goto Exit0;

                break;

            case SERVICE_START_PENDING:
            case SERVICE_STOP_PENDING:

                ::Sleep(ss.dwWaitHint);

                break;

            default:

                goto Exit0;
            }

            if (::GetTickCount() - dwTickCount > 10000) // 最多10秒
                goto Exit0;

        } while (TRUE);

    Exit1:

        bRet = TRUE;

    Exit0:

        if (hService)
            ::CloseServiceHandle(hService);
        if (hSCM)
            ::CloseServiceHandle(hSCM);

        return bRet;
    }

    BOOL StopService()
    {
        BOOL bRet = FALSE;

        SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (hSCM == NULL)
            goto Exit0;

        SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS);
        if (hService == NULL)
            goto Exit0;

        DWORD dwTickCount = ::GetTickCount();

        SERVICE_STATUS ss;

        do 
        {
            bRet = ::QueryServiceStatus(hService, &ss);
            if (!bRet)
                goto Exit0;

            switch (ss.dwCurrentState)
            {
            case SERVICE_RUNNING:

                bRet = ::ControlService(hService, SERVICE_CONTROL_STOP, &ss);
                if (!bRet)
                    goto Exit0;

                break;

            case SERVICE_STOPPED:

                goto Exit1;

            case SERVICE_START_PENDING:
            case SERVICE_STOP_PENDING:

                ::Sleep(ss.dwWaitHint);

                break;

            default:

                goto Exit0;
            }

            if (::GetTickCount() - dwTickCount > 10000) // 最多10秒
                goto Exit0;

        } while (TRUE);

    Exit1:

        bRet = TRUE;

    Exit0:

        if (hService)
            ::CloseServiceHandle(hService);
        if (hSCM)
            ::CloseServiceHandle(hSCM);

        return bRet;
    }

    void OnStart() throw()
    {

    }

    void OnCustomRequest(DWORD /*dwOpcode*/) throw()
    {
    }

    void OnFinal()
    {

    }

    void OnStop() throw()
    {
    }

protected:

    TCHAR m_szDisplayName[256];
    TCHAR m_szDescription[256];

    static DWORD WINAPI _DebugThread(LPVOID pvParam)
    {
        DWORD dwThreadID = (DWORD)(DWORD_PTR)pvParam;

        bkconsole the_console;

        the_console.printf(L"press any key to quit...\r\n");
        the_console.getch();

        ::PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);

        return 0;
    }
};
