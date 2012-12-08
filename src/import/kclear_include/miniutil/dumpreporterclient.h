#pragma once

#include <miniutil/fileversion.h>
#include <shellapi.h>
#include <DbgHelp.h>
#include <Psapi.h>

//#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")

typedef BOOL (__stdcall FnStackWalk64)(
            DWORD                             MachineType,
            HANDLE                            hProcess,
            HANDLE                            hThread,
            LPSTACKFRAME64                    StackFrame,
            PVOID                             ContextRecord,
            PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
            PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
            PGET_MODULE_BASE_ROUTINE64        GetModuleBaseRoutine,
            PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
            );


class CSZDumpReporterClient
{
public:
    CSZDumpReporterClient()
    {
        HMODULE hModDbgHelp = ::LoadLibrary(_T("dbghelp.dll"));

        if (!hModDbgHelp)
            goto Exit0;

        ms_pfnStackWalk64 = (FnStackWalk64 *)::GetProcAddress(hModDbgHelp, "StackWalk64");
        ms_pfnSymFunctionTableAccess64 = (PFUNCTION_TABLE_ACCESS_ROUTINE64)::GetProcAddress(hModDbgHelp, "SymFunctionTableAccess64");
        ms_pfnSymGetModuleBase64 = (PGET_MODULE_BASE_ROUTINE64)::GetProcAddress(hModDbgHelp, "SymGetModuleBase64");
        if (!ms_pfnStackWalk64 || !ms_pfnSymFunctionTableAccess64 || !ms_pfnSymGetModuleBase64)
            goto Exit0;

        ms_bReady = TRUE;

        ms_pInstance = this;

    Exit0:

        ::SetUnhandledExceptionFilter(_SZTopLevelExceptionFilter);
    }

    static void SetChannel(DWORD dwProductID, DWORD dwChannelID)
    {
        ms_dwProductID = dwProductID;
        ms_dwChannelID = dwChannelID;
    }

private:
    static BOOL ms_bReady;
    static FnStackWalk64 *ms_pfnStackWalk64;
    static PFUNCTION_TABLE_ACCESS_ROUTINE64 ms_pfnSymFunctionTableAccess64;
    static PGET_MODULE_BASE_ROUTINE64 ms_pfnSymGetModuleBase64;
    static CSZDumpReporterClient *ms_pInstance;
    static DWORD ms_dwProductID;
    static DWORD ms_dwChannelID;

    static LONG WINAPI _SZTopLevelExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo)
    {
        if (ExceptionInfo && ms_bReady)
        {
            _WalkTheCrushStack(ExceptionInfo->ExceptionRecord, ExceptionInfo->ContextRecord);
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }

    static void _WalkTheCrushStack(PEXCEPTION_RECORD ExceptionRecord, PCONTEXT ContextRecord)
    {
        STACKFRAME64 stframe = { 0 };
        HANDLE hProcess = ::GetCurrentProcess();
        HANDLE hThread = ::GetCurrentThread();
        CString strTrace, strFormat;
        CONTEXT context = *ContextRecord;
        BOOL bRet = FALSE;

        stframe.AddrPC.Offset    = context.Eip;
        stframe.AddrPC.Mode      = AddrModeFlat;
        stframe.AddrStack.Offset = context.Esp;
        stframe.AddrStack.Mode   = AddrModeFlat;
        stframe.AddrFrame.Offset = context.Ebp;
        stframe.AddrFrame.Mode   = AddrModeFlat;

        strTrace = _T("[System]\n");

        GetSystemInfomation(strFormat);

        strTrace += strFormat;
        strFormat.Format(_T("Code:%08X\nAddr:%08X\n"), ExceptionRecord->ExceptionCode, ExceptionRecord->ExceptionAddress);
        strTrace += strFormat;
        strTrace += _T("[Module]\n");

        GetModuleList(hProcess, strFormat);

        strTrace += strFormat;
        strTrace += _T("[Stack]\n");

        for (int i = 0; i < 20; i ++)
        {
            bRet = ms_pfnStackWalk64(
                IMAGE_FILE_MACHINE_I386, 
                hProcess, hThread, &stframe, &context, NULL, 
                ms_pfnSymFunctionTableAccess64, ms_pfnSymGetModuleBase64, NULL
                );

            if (!bRet)
                break;

            GetAddressDetails(hProcess, (LPVOID)stframe.AddrPC.Offset, strFormat);

            strTrace += strFormat;
        }

        {
            CString strTempPath, strFileName;
            ::GetTempPath(MAX_PATH, strTempPath.GetBuffer(MAX_PATH + 1));
            strTempPath.ReleaseBuffer();

            if (strTempPath.IsEmpty())
                goto Exit0;

            if (strTempPath[strTempPath.GetLength() - 1] != _T('\\'))
                strTempPath += _T('\\');

            SYSTEMTIME st;
            ::GetLocalTime(&st);

            strFileName.Format(
                _T("SZDump%4d%02d%02d%02d%02d%02d.LOG"), 
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond
                );

            HANDLE hFile = ::CreateFile(
                strTempPath + strFileName, 
                GENERIC_ALL, NULL, NULL, 
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
                );
            if (INVALID_HANDLE_VALUE != hFile)
            {
                DWORD dwWrite = 0;
                CStringA strDump;
                {
                    strDump = CT2A(strTrace, CP_UTF8);
                }
                ::WriteFile(hFile, strDump, strDump.GetLength(), &dwWrite, NULL);
                ::SetEndOfFile(hFile);
                ::CloseHandle(hFile);

#ifdef _SZ_DUMP_REPORTER_
                CSZDumpReporter::ReportDump(strTempPath + strFileName, ms_dwProductID, ms_dwChannelID);
#else
                ShellBugReporter(strFileName, ms_dwProductID, ms_dwChannelID);
#endif
            }
        }

Exit0:

        if (hThread)
        {
            ::CloseHandle(hThread);
            hThread = NULL;
        }

        if (hProcess)
        {
            ::CloseHandle(hProcess);
            hProcess = NULL;
        }
    }

#ifndef _SZ_DUMP_REPORTER_

    // 这个函数现在是坏的
    static void ShellBugReporter(LPCTSTR lpszReportFileName, DWORD dwProductID, DWORD dwChannelID)
    {
        CString strBugReporterPath;
        HKEY hKey = NULL;
        LONG lRet = ERROR_SUCCESS;
        CString strPath;
        DWORD dwType = REG_SZ, dwSize = MAX_PATH * sizeof(TCHAR);

        // Get BugReport Module Path

        lRet = ::RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, 
            _T("SOFTWARE\\S-Zone\\module\\update"), 
            0, KEY_QUERY_VALUE, &hKey
            );
        if (ERROR_SUCCESS != lRet)
            goto Exit0;

        lRet = ::RegQueryValueEx(
            hKey, _T("InstallPath"), NULL, &dwType, 
            (LPBYTE)(LPCTSTR)strBugReporterPath.GetBuffer(MAX_PATH + 1), &dwSize
            );
        strBugReporterPath.ReleaseBuffer();
        if (ERROR_SUCCESS != lRet)
            goto Exit0;

        if (strBugReporterPath[strBugReporterPath.GetLength() - 1] != _T('\\'))
            strBugReporterPath += _T('\\');

        strBugReporterPath += _T("szbugrep.exe");

        ::ShellExecute(NULL, _T("open"), strBugReporterPath, lpszReportFileName, NULL, SW_SHOWNORMAL);

    Exit0:

        if (hKey)
        {
            ::RegCloseKey(hKey);
            hKey = NULL;
        }
    }
#endif

    static BOOL GetSystemInfomation(CString &strSysInfo)
    {
        BOOL bResult = TRUE, bRet = FALSE;
        CString strText;
        OSVERSIONINFOEX osvi = { sizeof(OSVERSIONINFOEX) };
        DWORD dwProductType = 0;
        HKEY hKey = NULL;
        LONG lRet = ERROR_SUCCESS;
        CString strVersion;
        DWORD dwType = REG_SZ, dwSize = MAX_PATH * sizeof(TCHAR);


        { // Get Windows Version
            bRet = ::GetVersionEx((LPOSVERSIONINFO)&osvi);
            if (!bRet)
                goto Exit0;

            dwProductType = osvi.wProductType;

            if (osvi.dwMajorVersion >= 6)
            {
                typedef BOOL (WINAPI *FnGetProductInfo)(DWORD ,DWORD ,DWORD ,DWORD ,PDWORD);

                FnGetProductInfo GetProductInfo = (FnGetProductInfo)::GetProcAddress(
                    ::GetModuleHandle(_T("kernel32.dll")), "GetProductInfo"
                    );
                if (GetProductInfo)
                {
                    GetProductInfo(
                        osvi.dwMajorVersion, 
                        osvi.dwMinorVersion, 
                        osvi.wServicePackMajor, 
                        osvi.wServicePackMinor, 
                        &dwProductType
                        );
                }
            }

            strText.Format(
                _T("Win:%d.%d.%d.%d.%X\n"), 
                osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.wServicePackMajor, osvi.dwBuildNumber, dwProductType
                );

            strSysInfo += strText;
        }

        { // Get IE Version (3.0 later)
            lRet = ::RegOpenKeyEx(
                HKEY_LOCAL_MACHINE, 
                _T("Software\\Microsoft\\Internet Explorer"), 
                0, KEY_QUERY_VALUE, &hKey
                );
            if (ERROR_SUCCESS != lRet)
                goto Exit0;

            lRet = ::RegQueryValueEx(hKey, _T("Version"), NULL, &dwType, (LPBYTE)(LPCTSTR)strVersion.GetBuffer(MAX_PATH + 1), &dwSize);
            strVersion.ReleaseBuffer();
            if (ERROR_SUCCESS != lRet)
                goto Exit0;

            strText.Format(_T("IE:%s\n"), strVersion);

            strSysInfo += strText;
        }

        bResult = TRUE;

Exit0:

        if (hKey)
        {
            ::RegCloseKey(hKey);
            hKey = NULL;
        }

        return bResult;
    }

    static BOOL GetModuleList(HANDLE hProcess, CString &strModList)
    {
        BOOL bResult = TRUE, bRet = FALSE;
        HMODULE *hMods = NULL;
        DWORD dwModuleCount = 0;
        CString strModInfo;
        MODULEINFO mi = { 0 };

        strModList.Empty();

        bRet = ::EnumProcessModules(hProcess, NULL, 0, &dwModuleCount);
        if (!bRet)
            goto Exit0;

        hMods = new HMODULE[dwModuleCount / sizeof(HMODULE)];

        bRet = ::EnumProcessModules(hProcess, hMods, dwModuleCount, &dwModuleCount);
        if (!bRet)
            goto Exit0;

        dwModuleCount /= sizeof(HMODULE);

        for (DWORD dwIndex = 0; dwIndex < dwModuleCount; dwIndex ++)
        {
            TCHAR pszModuleBaseName[MAX_PATH + 1] = { 0 }, pszModule[MAX_PATH + 1] = { 0 };
            ULARGE_INTEGER ullVersion = { 0 };
            ::GetModuleBaseName(hProcess, hMods[dwIndex], pszModuleBaseName, MAX_PATH);
            ::GetModuleFileName(hMods[dwIndex], pszModule, MAX_PATH);
            ::GetModuleInformation(hProcess, hMods[dwIndex], &mi, sizeof(MODULEINFO));
            ::GetPEFileVersion(pszModule, &ullVersion);

            strModInfo.Format(
                _T("%08X-%08X %s\n%s(%d.%d.%d.%d)\n"), 
                mi.lpBaseOfDll, 
                (DWORD_PTR)mi.lpBaseOfDll + mi.SizeOfImage, 
                pszModuleBaseName, pszModule, 
                HIWORD(ullVersion.HighPart), LOWORD(ullVersion.HighPart), 
                HIWORD(ullVersion.LowPart), LOWORD(ullVersion.LowPart)
                );
            strModList += strModInfo;
        }

        bResult = TRUE;

    Exit0:

        if (hMods)
        {
            delete[] hMods;
            hMods = NULL;
        }

        return bResult;
    }

    static BOOL GetAddressDetails(HANDLE hProcess, void* pAddress, CString& strAddressDetail)
    {
        MEMORY_BASIC_INFORMATION MemInfo;
        if (!VirtualQuery(pAddress, &MemInfo, sizeof(MEMORY_BASIC_INFORMATION)))
            return FALSE;

        HMODULE hModule = (HMODULE) MemInfo.AllocationBase;
        TCHAR pszModule[MAX_PATH + 1] = { 0 };

        if (::GetModuleBaseName(hProcess, hModule, pszModule, MAX_PATH))
        {
            strAddressDetail.Format(
                _T("%s+%X\n"), 
                pszModule, (INT_PTR)pAddress - (INT_PTR)MemInfo.AllocationBase
                );
            return TRUE;
        }
        else
        {
            strAddressDetail.Format(_T("Unknown Module+%X\n"), pAddress);
            return FALSE;
        }
    }
};

BOOL CSZDumpReporterClient::ms_bReady = FALSE;
FnStackWalk64 *CSZDumpReporterClient::ms_pfnStackWalk64 = NULL;
PFUNCTION_TABLE_ACCESS_ROUTINE64 CSZDumpReporterClient::ms_pfnSymFunctionTableAccess64 = NULL;
PGET_MODULE_BASE_ROUTINE64 CSZDumpReporterClient::ms_pfnSymGetModuleBase64 = NULL;
CSZDumpReporterClient* CSZDumpReporterClient::ms_pInstance = NULL;
DWORD CSZDumpReporterClient::ms_dwProductID = 0;
DWORD CSZDumpReporterClient::ms_dwChannelID = 0;

CSZDumpReporterClient dumpReporterClient;
