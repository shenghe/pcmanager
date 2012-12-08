#pragma once

#include <safemon\safemonitor.h>
#include <safemon\safetrayshell.h>
#include "../../../import/kclear_include/miniutil/bkcmdline.h"

typedef CWinTraits<WS_POPUP, 0> CBkSafeProtectionNotifyWndTraits;

#define BKSAFE_PROTECTION_NOTIFY_WND_CLASS L"{AA4D6C0D-4658-46a8-80FA-0CDB62B000AA}"

class KFlowTray
    : public CWindowImpl<KFlowTray, ATL::CWindow, CBkSafeProtectionNotifyWndTraits>
{
public:
    DECLARE_WND_CLASS_EX(BKSAFE_PROTECTION_NOTIFY_WND_CLASS, CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

public:
    KFlowTray(BOOL bNoQuitSvc = FALSE)
    {
    }

    ~KFlowTray()
    {
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

    BOOL m_bMonitorOn[6];
	BOOL m_bKWSEnable;
	BOOL m_bTrayAutorun;
    BOOL m_bUpdateCalled;
    BOOL m_bNoQuitSvc;

    void _ShellTray()
    {
        if (::IsWindow(m_MonitorShell.GetWnd()))
            return;

		WCHAR szFilePath[MAX_PATH + 1] = {0};
		DWORD dwRet = ::GetModuleFileName(NULL, szFilePath, MAX_PATH);
        CString strPath = szFilePath;
		strPath.Truncate(strPath.ReverseFind(L'\\') + 1);
		       
        strPath += L"KSafeTray.exe";

        CBkCmdLine cmdLine;

        cmdLine.Execute(strPath, FALSE, FALSE);
    }

    void _Refresh()
    {
 
    }

    void OnTimer(UINT_PTR nIDEvent)
    {
        if (1 == nIDEvent)
        {
            _Refresh();
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

    BEGIN_MSG_MAP_EX(KFlowTray)
        MSG_WM_TIMER(OnTimer)
    END_MSG_MAP()
};