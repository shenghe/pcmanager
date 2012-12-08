//////////////////////////////////////////////////////////////////////////
//   File Name: bkprocprivilege.h
// Description: Raise Process Privilege
//     Creator: Zhang Xiaoxuan
//     Version: 2009.5.26 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

class CBkProcPrivilege
{
public:
    CBkProcPrivilege()
    {
    }

    BOOL SetDebug(BOOL bSet)
    {
        BOOL bRet  = FALSE;
        HANDLE hTokenProc = NULL;

        bRet = ::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hTokenProc);
        if (!bRet)
            goto Exit0;

        bRet = _SetPrivilege(hTokenProc, SE_DEBUG_NAME, bSet);
        if (!bRet)
            goto Exit0;

    Exit0:

        if (hTokenProc)
        {
            ::CloseHandle(hTokenProc);
            hTokenProc = NULL;
        }

        return bRet;
    }

    BOOL EnableShutdown()
    {
        BOOL bRet  = FALSE;
        HANDLE hTokenProc = NULL;

        bRet = ::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hTokenProc);
        if (!bRet)
            goto Exit0;

        bRet = _SetPrivilege(hTokenProc, SE_SHUTDOWN_NAME, TRUE);
        if (!bRet)
            goto Exit0;

Exit0:

        if (hTokenProc)
        {
            ::CloseHandle(hTokenProc);
            hTokenProc = NULL;
        }

        return bRet;
    }

private:

    BOOL _SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
    {
        BOOL RetVal = FALSE;
        BOOL bResult  = FALSE;

        TOKEN_PRIVILEGES tp;
        LUID luid;

        bResult = ::LookupPrivilegeValue(NULL, lpszPrivilege, &luid);
        if (!bResult)
        {
            goto Exit0;
        }

        tp.PrivilegeCount       = 1;
        tp.Privileges[0].Luid   = luid;
        if (bEnablePrivilege)
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        else
            tp.Privileges[0].Attributes = SE_PRIVILEGE_REMOVED;

        bResult = ::AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tp,
            sizeof(TOKEN_PRIVILEGES),
            (PTOKEN_PRIVILEGES)NULL,
            (PDWORD)NULL
            );
        if (!bResult)
            goto Exit0;

        RetVal = TRUE;

    Exit0:

        return RetVal;
    }
};