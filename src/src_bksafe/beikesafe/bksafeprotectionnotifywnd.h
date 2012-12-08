#pragma once

#include <safemon\safemonitor.h>
#include <safemon\safetrayshell.h>
#include "trayruncommmon.h"
#include "kpfw/arpsetting_public.h"
#include "beikesafearpdlg.h"
#include "arpinstallcheck.h"
#include "runoptimize/restorerunitem.h"  /* for CRestoreRunner */

typedef CWinTraits<WS_POPUP, 0> CBkSafeProtectionNotifyWndTraits;

#define BKSAFE_PROTECTION_NOTIFY_WND_CLASS L"{AA4D6C0D-4658-46a8-80FA-0CDB62B000AA}"


class CBkSafeProtectionNotifyWindow
    : public CWindowImpl<CBkSafeProtectionNotifyWindow, ATL::CWindow, CBkSafeProtectionNotifyWndTraits>
{
public:
    DECLARE_WND_CLASS_EX(BKSAFE_PROTECTION_NOTIFY_WND_CLASS, CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

public:
    CBkSafeProtectionNotifyWindow(BOOL bNoQuitSvc = FALSE)
        : m_hWndNotify(NULL)
        , m_uMsgNotifyChange(0)
        , m_bUpdateCalled1(FALSE)
        , m_bUpdateCalled2(FALSE)
        , m_bNoQuitSvc(bNoQuitSvc)
    {
		m_bTrayAutorun1 = FALSE;
        m_bTrayAutorun2 = FALSE;
        ::ZeroMemory(m_bMonitorOn, sizeof(m_bMonitorOn));
        ::ZeroMemory(m_bNetMonitorOn, sizeof(m_bNetMonitorOn));
		m_bKWSEnable = FALSE;
        m_bKArpRun = FALSE;
    }

    ~CBkSafeProtectionNotifyWindow()
    {
        if (IsWindow())
            DestroyWindow();

        if (!m_bNoQuitSvc)
        {
            BOOL bAnyMonitorOn = FALSE;
            BOOL _bAnyMonitorOn = FALSE;

            for (int i = 0; i < ARRAYSIZE(m_bMonitorOn); i ++)
                bAnyMonitorOn |= m_bMonitorOn[i];

            for (int j = 0; j < ARRAYSIZE(m_bNetMonitorOn); j++)
                _bAnyMonitorOn |= m_bNetMonitorOn[j];
         
            if (!bAnyMonitorOn || !_bAnyMonitorOn)
            {
                SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
                if (hSCM == NULL)
                    goto Exit0;

                SC_HANDLE hService = ::OpenService(hSCM, L"KSafeSvc", SERVICE_STOP | SERVICE_QUERY_STATUS);
                if (hService == NULL)
                    goto Exit0;

                SERVICE_STATUS ss;

                ::ControlService(hService, SERVICE_CONTROL_STOP, &ss);
Exit0:

                if (hService)
                    ::CloseServiceHandle(hService);
                if (hSCM)
                    ::CloseServiceHandle(hSCM);
            }
        }
    }

    HWND Create(HWND hWndNotify, UINT uMsgNotifyChange)
    {
        HWND hWnd = __super::Create(NULL);
        if (NULL == hWnd)
            return NULL;

        m_hWndNotify = hWndNotify;
        m_uMsgNotifyChange = uMsgNotifyChange;

        _ShellTray();

        _Refresh1();
        _Refresh2();

        SetTimer(1, 1000, NULL);
        SetTimer(2, 3000, NULL);

        return hWnd;
    }
	HWND SetNotifyHwnd(HWND hNtoifyWnd)
	{
		HWND hWnd = m_hWndNotify;
		m_hWndNotify = hNtoifyWnd;

		return hWnd;
	}

	DWORD SetNotifyMsg(DWORD dwMsg)
	{
		DWORD msg = m_uMsgNotifyChange;
		m_uMsgNotifyChange = dwMsg;
		return msg;
	}

	void TurnSwitch(DWORD dwMonitorID)
	{
		BOOL bRun = m_MonitorShell.GetMonitorRun(dwMonitorID);
		m_MonitorShell.SetMonitorRun(dwMonitorID, !bRun);

		_Refresh1();
        _Refresh2();
	}

	BOOL GetTurnStatus(DWORD dwMonitorID)
	{
		return m_MonitorShell.GetMonitorRun(dwMonitorID);
	}

	/** 
	*  设置开机启动tray 
	*/
	void OnStartupRun()
	{
		CRestoreRunner* pLogRunner = new CRestoreRunner;
		pLogRunner->DeleteItemFromLog(_T("KSafeTray"), KSRUN_TYPE_STARTUP, 590);
		delete pLogRunner;
		
        m_MonitorShell.SetAutorunTray(TRUE);

		if (m_hWndNotify && ::IsWindow(m_hWndNotify))
			::PostMessage(m_hWndNotify, MSG_SYSOPT_REFRESH_ITEMLIST, 0, 0); /* 刷新开机启动列表 */
	}

	void TurnOnAll()
	{
		_Module.Exam.SetAllMonitorRun();
		CTrayRunCommon::GetPtr()->SetChange();

		_Refresh1();
	}

    void _TurnOnAll()
    {
        _Module.Exam._SetAllMonitorRun();
        CTrayRunCommon::GetPtr()->SetChange();
        _Refresh2();
    }

    static BOOL NotifyTrayRestart()
    {
        HWND hWndDDE = ::FindWindow(BKSAFE_PROTECTION_NOTIFY_WND_CLASS, NULL);
        if (NULL == hWndDDE)
            return FALSE;

        return ::PostMessage(hWndDDE, WM_APP, NULL, NULL);
    }

	int GetMonitorStatus()
	{
		DWORD dwMonitorIDList[] = {
			SM_ID_RISK, 
			SM_ID_PROCESS, 
			SM_ID_UDISK, 
			SM_ID_LEAK,			
		};

		int t = 0;
		for ( int i = 0; i < ARRAYSIZE(dwMonitorIDList); i++ )
		{
			/*
			if (TRUE == _Module.Exam.IsWin64())
			{
				if (SM_ID_PROCESS == dwMonitorIDList[i])
					t++;
			}
			*/
			if ( m_bMonitorOn[i] )
			{
				t++;
			}
		}

		if ( t == ARRAYSIZE(dwMonitorIDList) )
		{
			return m_bTrayAutorun1 ? 0 : 1;
		}

		return (t == 0) ? 3 : 2;
	}

    int _GetMonitorStatus()
    {
        DWORD dwMonitorIDList[] = {
			SM_ID_KWS,
			SM_ID_KWS_SAFE_DOWN,
		};

		int t = 0;
		for ( int i = 0; i < ARRAYSIZE(dwMonitorIDList); i++ )
		{
			/*
			if (TRUE == _Module.Exam.IsWin64())
			{
				if (SM_ID_PROCESS == dwMonitorIDList[i])
					t++;
			}
			*/
			if ( m_bNetMonitorOn[i] )
			{
				t++;
			}
		}

		if ( t == ARRAYSIZE(dwMonitorIDList) )
		{
			return m_bTrayAutorun2 ? 0 : 1;
		}

		return (t == 0) ? 3 : 2;

    }

    void ShellTray()
    {
        _ShellTray();

        m_MonitorShell.CallUpdate(1);
        m_MonitorShell.CallUpdate(2);
    }

	void TryOpenShell()
	{
		_ShellTray();
	}

protected:

    CSafeMonitorTrayShell m_MonitorShell;

    HWND m_hWndNotify;
    UINT m_uMsgNotifyChange;

    BOOL m_bMonitorOn[4];
    BOOL m_bNetMonitorOn[2];
	BOOL m_bKWSEnable;
    BOOL m_bKArpRun;
	BOOL m_bTrayAutorun1;
    BOOL m_bTrayAutorun2;
    BOOL m_bUpdateCalled1;
    BOOL m_bUpdateCalled2;
    BOOL m_bNoQuitSvc;

    void _ShellTray()
    {
        if (::IsWindow(m_MonitorShell.GetWnd()))
            return;

        CString strPath = _Module.GetAppDirPath(), strCmdLine;
        
        strPath += L"KSafeTray.exe";

        CBkCmdLine cmdLine;

//         strCmdLine.Format(L"0x%08X", m_hWnd);
// 
//         cmdLine.SetParam(L"mainwnd", strCmdLine);

        cmdLine.Execute(strPath, FALSE, FALSE);
    }

    void _Refresh1()
    {
        BOOL bAllMonitorOn = TRUE, bAllMonitorOnOld = TRUE;
        BOOL bAnyMonitorOn = FALSE, bAnyMonitorOnOld = FALSE, bAnyMonitorSettingOn = FALSE;
        BOOL bValue = FALSE;
        DWORD dwMonitorIDList[] = {
            SM_ID_RISK, 
            SM_ID_PROCESS, 
            SM_ID_UDISK, 
            SM_ID_LEAK,			
        };

        for (int i = 0; i < ARRAYSIZE(dwMonitorIDList); i ++)
        {
            bAllMonitorOnOld &= m_bMonitorOn[i];
            bAnyMonitorOnOld |= m_bMonitorOn[i];
        }

        BOOL bTrayRunning = ::IsWindow(m_MonitorShell.GetWnd());

        if (bTrayRunning && !m_bUpdateCalled1)
        {
            m_MonitorShell.CallUpdate(1);
            m_MonitorShell.CallUpdate(2);

            m_bUpdateCalled1 = TRUE;
        }

        for (int i = 0; i < ARRAYSIZE(dwMonitorIDList); i ++)
        {
			bValue = m_MonitorShell.GetMonitorRun(dwMonitorIDList[i]);

            bAnyMonitorSettingOn |= bValue;

            bValue &= bTrayRunning;

            bAllMonitorOn &= bValue;
            bAnyMonitorOn |= bValue;

            if (m_bMonitorOn[i] != bValue)
            {
                m_bMonitorOn[i] = bValue;

                if (m_hWndNotify && ::IsWindow(m_hWndNotify))
                    ::PostMessage(m_hWndNotify, m_uMsgNotifyChange, dwMonitorIDList[i], bValue);
            }
        }

		BOOL bTrayAutorun = m_MonitorShell.GetAutorunTray();
		BOOL bFlag = FALSE;
		if (TRUE == _Module.Exam.IsWin64())
		{
			bFlag = 0 == _Module.Exam.IsSafeMonitorAllRun();
		}
		else
		{
			bFlag = bAllMonitorOn && !bAllMonitorOnOld;
		}

        if ( bFlag || bTrayAutorun != m_bTrayAutorun1 )
		{
			m_bTrayAutorun1 = bTrayAutorun;

            if (m_hWndNotify && ::IsWindow(m_hWndNotify))
                ::PostMessage(m_hWndNotify, m_uMsgNotifyChange, SM_ID_INVAILD, TRUE);
		}

        if (bAnyMonitorOn != bAnyMonitorOnOld)
        {
			/*m_MonitorShell.SetAutorunTray(bAnyMonitorSettingOn);*/
			CTrayRunCommon::GetPtr()->SetChange();
        }
    }

    void _Refresh2()
    {

        BOOL bAllMonitorOn = TRUE, bAllMonitorOnOld = TRUE;
        BOOL bAnyMonitorOn = FALSE, bAnyMonitorOnOld = FALSE, bAnyMonitorSettingOn = FALSE;
        BOOL bValue = FALSE;
        DWORD dwMonitorIDList[] = {
            SM_ID_KWS_SAFE_DOWN,
            SM_ID_KWS,
        };

        for (int i = 0; i < ARRAYSIZE(dwMonitorIDList); i ++)
        {
            bAllMonitorOnOld &= m_bNetMonitorOn[i];
            bAnyMonitorOnOld |= m_bNetMonitorOn[i];
        }

        BOOL bTrayRunning = ::IsWindow(m_MonitorShell.GetWnd());

        if (bTrayRunning && !m_bUpdateCalled2)
        {
            m_MonitorShell.CallUpdate(1);
            m_MonitorShell.CallUpdate(2);

            m_bUpdateCalled2 = TRUE;
        }

        for (int i = 0; i < ARRAYSIZE(dwMonitorIDList); i ++)
        {
            bValue = m_MonitorShell.GetMonitorRun(dwMonitorIDList[i]);

            bAnyMonitorSettingOn |= bValue;

            bValue &= bTrayRunning;

            bAllMonitorOn &= bValue;
            bAnyMonitorOn |= bValue;

            if (m_bNetMonitorOn[i] != bValue)
            {
                m_bNetMonitorOn[i] = bValue;

                if (m_hWndNotify && ::IsWindow(m_hWndNotify))
                    ::PostMessage(m_hWndNotify, m_uMsgNotifyChange, dwMonitorIDList[i], bValue);
            }   
        }
       
        if (IskArpInstalled())
        {
            IArpFwSetting* Iprpsetting = CArpSetting::Instance().GetPtr();
            if (NULL != Iprpsetting)
            {
                Iprpsetting->IsArpFwEnable(&bValue);
            } 
        }


        if (bValue != m_bKArpRun)
        {
            m_bKArpRun = bValue;
            if (m_hWndNotify && ::IsWindow(m_hWndNotify))
                ::PostMessage(m_hWndNotify, m_uMsgNotifyChange, SM_ID_ARP, bValue);
        }
          
        BOOL bTrayAutorun = m_MonitorShell.GetAutorunTray();
        BOOL bFlag = FALSE;
        if (TRUE == _Module.Exam.IsWin64())
        {
            bFlag = 0 == _Module.Exam._IsSafeMonitorAllRun();
        }
        else
        {
            bFlag = bAllMonitorOn && !bAllMonitorOnOld;
        }

        if ( bFlag || bTrayAutorun != m_bTrayAutorun2 )
        {
            m_bTrayAutorun2 = bTrayAutorun;

            if (m_hWndNotify && ::IsWindow(m_hWndNotify))
                ::PostMessage(m_hWndNotify, m_uMsgNotifyChange, SM_ID_INVAILD, TRUE);
        }

        if (bAnyMonitorOn != bAnyMonitorOnOld)
        {
            /*m_MonitorShell.SetAutorunTray(bAnyMonitorSettingOn);*/
            CTrayRunCommon::GetPtr()->SetChange();
        }

    }

   


    LRESULT OnApp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
    {
        _Refresh1();
        _Refresh2();

        return 0;
    }

    void OnTimer(UINT_PTR nIDEvent)
    {
        if (1 == nIDEvent)
        {
            _Refresh1();
            _Refresh2();
            return;
        }
        else if (2 == nIDEvent)
        {
            _ShellTray();
            KillTimer(2);
        }

        SetMsgHandled(FALSE);
    }

public:

    BEGIN_MSG_MAP_EX(CBkSafeProtectionNotifyWindow)
        MESSAGE_HANDLER_EX(WM_APP, OnApp)
        MSG_WM_TIMER(OnTimer)
    END_MSG_MAP()
};