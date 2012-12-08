#include "stdafx.h"
#include "KAccessControl.h"
using namespace std;

bool KAccessControl::SetPrivilege( HANDLE hToken, const wstring& privilegeName, bool enablePrivilege )
{
    // code from http://msdn.microsoft.com/en-us/library/aa446619(VS.85).aspx
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if ( !LookupPrivilegeValue( 
        NULL,            // lookup privilege on local system
        privilegeName.c_str(),   // privilege to lookup 
        &luid ) )        // receives LUID of privilege
    {
        return false; 
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (enablePrivilege)
    {
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    }
    else
    {
        tp.Privileges[0].Attributes = 0;
    }

    // Enable the privilege or disable all privileges.

    if ( !AdjustTokenPrivileges(
        hToken, 
        FALSE, 
        &tp, 
        sizeof(TOKEN_PRIVILEGES), 
        (PTOKEN_PRIVILEGES) NULL, 
        (PDWORD) NULL) )
    { 
        return false; 
    } 

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

    {
        // The token does not have the specified privilege.
        return false;
    } 

    return true;
}

bool KAccessControl::SetCurrentProcessPrivilege( const wstring& privilegeName, bool enablePrivilege /*= true */ )
{
    // 当前进程的HANDLE为-1，不过MSDN上建议用这个API取
    HANDLE handle = GetCurrentProcess();
    HANDLE hToken = NULL;

    OpenProcessToken(handle,(TOKEN_ADJUST_PRIVILEGES |TOKEN_QUERY),&hToken);
    
    bool result = KAccessControl::SetPrivilege(hToken, privilegeName, true);
    
    CloseHandle(hToken);
    CloseHandle(handle);

    return result;
}
