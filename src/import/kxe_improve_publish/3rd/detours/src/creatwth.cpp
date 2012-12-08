//////////////////////////////////////////////////////////////////////////////
//
//  File:		creatwth.cpp
//	Module:		detours.lib
//
//	DetourCreateProcessWithDll.  Version 1.5 (Build 46)
//
//	Copyright 1995-2001, Microsoft Corporation
//

#include <stdio.h>
#include <stddef.h>
#include <windows.h>
#include <imagehlp.h>

#define DETOURS_INTERNAL
#include "detours.h"

////////////////////////////////////////////////////////////// Injection Code.
//
static BOOL InjectLibrary(HANDLE hProcess,
                          HANDLE hThread,
						  PBYTE pfLoadLibrary,
						  PBYTE pbData,
						  DWORD cbData)
{
	BOOL fSucceeded = FALSE;
    DWORD nProtect = 0;
    DWORD nWritten = 0;
    CONTEXT cxt;
    UINT32 nCodeBase;
    PBYTE pbCode;

    struct Code
    {
        BYTE rbCode[128];
        union
        {
            WCHAR   wzLibFile[512];
            CHAR    szLibFile[512];
        };
    } code;

    SuspendThread(hThread);
    
    ZeroMemory(&cxt, sizeof(cxt));
    cxt.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext(hThread, &cxt)) {
		goto finish;
    }

#if VERBOSE
    printf("  ContextFlags: %08x\n", cxt.ContextFlags);
    printf("  EIP=%04x:%08x ESP=%04x:%08x EBP=%08x\n",
           cxt.SegCs & 0xffff, cxt.Eip, cxt.SegSs & 0xffff, cxt.Esp, cxt.Ebp);
    printf("  DS=%04x ES=%04x FS=%04x GS=%04x EFL=%08x\n",
           cxt.SegDs & 0xffff, cxt.SegEs & 0xffff,
           cxt.SegFs & 0xffff, cxt.SegGs & 0xffff,
           cxt.EFlags);
    printf("  EAX=%08x EBX=%08x ECX=%08x EDX=%08x ESI=%08x EDI=%08x\n",
           cxt.Eax, cxt.Ebx, cxt.Ecx, cxt.Edx, cxt.Esi, cxt.Edi);
#endif    

    nCodeBase = (cxt.Esp - sizeof(code)) & ~0x1fu;        // Cache-line align.
    pbCode = code.rbCode;

    if (pbData) {
        CopyMemory(code.szLibFile, pbData, cbData);
        pbCode = DetourGenPush(pbCode, nCodeBase + offsetof(Code, szLibFile));
        pbCode = DetourGenCall(pbCode, pfLoadLibrary,
                               (PBYTE)nCodeBase + (pbCode - code.rbCode));
    }
    
    pbCode = DetourGenMovEax(pbCode, cxt.Eax);
    pbCode = DetourGenMovEbx(pbCode, cxt.Ebx);
    pbCode = DetourGenMovEcx(pbCode, cxt.Ecx);
    pbCode = DetourGenMovEdx(pbCode, cxt.Edx);
    pbCode = DetourGenMovEsi(pbCode, cxt.Esi);
    pbCode = DetourGenMovEdi(pbCode, cxt.Edi);
    pbCode = DetourGenMovEbp(pbCode, cxt.Ebp);
    pbCode = DetourGenMovEsp(pbCode, cxt.Esp);
    pbCode = DetourGenJmp(pbCode, (PBYTE)cxt.Eip,
                          (PBYTE)nCodeBase + (pbCode - code.rbCode));
    
    cxt.Esp = nCodeBase - 4;
    cxt.Eip = nCodeBase;

    if (!VirtualProtectEx(hProcess, (PBYTE)nCodeBase, sizeof(Code),
                          PAGE_EXECUTE_READWRITE, &nProtect)) {
        goto finish;
    }

#if VERBOSE    
    printf("VirtualProtectEx(%08x) -> %d\n", nProtect, b);
#endif    

    if (!WriteProcessMemory(hProcess, (PBYTE)nCodeBase, &code, sizeof(Code),
                            &nWritten)) {
        goto finish;
    }

#if VERBOSE    
    printf("code: %08x..%08x (WriteProcess: %d)\n",
           nCodeBase, nCodeBase + (pbCode - code.rbCode), nWritten);
#endif
    
    if (!FlushInstructionCache(hProcess, (PBYTE)nCodeBase, sizeof(Code))) {
        goto finish;
    }
                        
    if (!SetThreadContext(hThread, &cxt)) {
        goto finish;
    }

#if VERBOSE    
    ZeroMemory(&cxt, sizeof(cxt));
    cxt.ContextFlags = CONTEXT_FULL;
    b = GetThreadContext(hThread, &cxt);
    printf("  EIP=%04x:%08x ESP=%04x:%08x EBP=%08x\n",
           cxt.SegCs & 0xffff, cxt.Eip, cxt.SegSs & 0xffff, cxt.Esp, cxt.Ebp);
    printf("  DS=%04x ES=%04x FS=%04x GS=%04x EFL=%08x\n",
           cxt.SegDs & 0xffff, cxt.SegEs & 0xffff,
           cxt.SegFs & 0xffff, cxt.SegGs & 0xffff,
           cxt.EFlags);
    printf("  EAX=%08x EBX=%08x ECX=%08x EDX=%08x ESI=%08x EDI=%08x\n",
           cxt.Eax, cxt.Ebx, cxt.Ecx, cxt.Edx, cxt.Esi, cxt.Edi);
#endif    
    
	fSucceeded = TRUE;
	
  finish:
    ResumeThread(hThread);
	return fSucceeded;
}

/////////////////////////////////////////////////////////////// Injected Code.
//
#pragma check_stack(off)
#pragma pack(push, 8)

typedef HINSTANCE (WINAPI *PROCLOADLIBRARY)(PWCHAR);

typedef struct {
	PROCLOADLIBRARY 	fnLoadLibrary;
	WCHAR				wzLibFile[MAX_PATH];
} INJLIBINFO, *PINJLIBINFO;

// Calls to the stack-checking routine must be disabled.

static DWORD WINAPI ThreadFunc(PINJLIBINFO pInjLibInfo) {
	// There must be less than a page-worth of local
	// variables used in this function.
	HINSTANCE hinstLib;

	// Call LoadLibrary(A/W) to load the DLL.
	hinstLib = pInjLibInfo->fnLoadLibrary(pInjLibInfo->wzLibFile);
	return((DWORD) hinstLib);
}

#pragma warning(disable: 4702)

static DWORD WINAPI DeadThreadFunc(PINJLIBINFO pInjLibInfo)
{
    (void)pInjLibInfo;
  x:
    goto x;
    return 0;
}

// This function marks the memory address after ThreadFunc.
// ThreadFuncCodeSizeInBytes = (PBYTE) AfterThreadFunc - (PBYTE) ThreadFunc.
static void AfterThreadFunc (void) {
}

#pragma pack(pop)
#pragma check_stack 

////////////////////////////////////////////////////////////// Injection Code.
//
static BOOL InjectLibraryOld(HANDLE hProcess,
                             PBYTE pfLoadLibrary,
                             PBYTE pbData,
                             DWORD cbData)
{
	BOOL fSucceeded = FALSE;

	// Initialize the InjLibInfo structure here and then copy
	// it to memory in the remote process.
	INJLIBINFO InjLibInfo;
	InjLibInfo.fnLoadLibrary = (PROCLOADLIBRARY)pfLoadLibrary;

	// The address where code will be copied to in the remote process.
	PDWORD pdwCodeRemote = NULL;

	// Calculate the number of bytes in the ThreadFunc function.
	const int cbCodeSize = ((LPBYTE) AfterThreadFunc - (LPBYTE) ThreadFunc);

	// The address where InjLibInfo will be copied to in the remote process.
	PINJLIBINFO pInjLibInfoRemote = NULL;

	// The number of bytes written to the remote process.
	DWORD dwNumBytesXferred = 0;

	// The handle and Id of the thread executing the remote copy of ThreadFunc.
	DWORD dwThreadId = 0;
	const DWORD cbMemSize = cbCodeSize + sizeof(InjLibInfo) + 3;
	HANDLE hThread = NULL;

	DWORD dwOldProtect;

	// Finish initializing the InjLibInfo structure by copying the
	// desired DLL's pathname.
    if (pbData) {
        CopyMemory(InjLibInfo.wzLibFile, pbData, cbData);
    }
	
	// Allocate memory in the remote process's address space large 
	// enough to hold our ThreadFunc function and a InjLibInfo structure.
	pdwCodeRemote = (PDWORD)VirtualAllocEx(hProcess, NULL, cbMemSize,
										   MEM_COMMIT | MEM_TOP_DOWN,
                                           PAGE_EXECUTE_READWRITE);
	if (pdwCodeRemote == NULL) {
		goto finish;
	}
#if 0    
    printf("pdwCodeRemote: %08x / %08x\n", pdwCodeRemote, pbData);
#endif    
    if (pbData == NULL) {
        pdwCodeRemote += ((LPBYTE) DeadThreadFunc - (LPBYTE) ThreadFunc);
    }
#if 0    
    printf("pdwCodeRemote: %08x / %08x\n", pdwCodeRemote, pbData);
#endif    

	// Change the page protection of the allocated memory
	// to executable, read, and write.
	if (!VirtualProtectEx(hProcess, pdwCodeRemote, cbMemSize,
						  PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
		goto finish;
	}

	// Write a copy of ThreadFunc to the remote process.
	if (!WriteProcessMemory(hProcess, pdwCodeRemote,
							(LPVOID) ThreadFunc, cbCodeSize, &dwNumBytesXferred)) {
		goto finish;
	}

	// Write a copy of InjLibInfo to the remote process
	// (the structure MUST start on an even 32-bit bourdary).
	pInjLibInfoRemote = (PINJLIBINFO)(pdwCodeRemote + ((cbCodeSize + 4) & ~3));

	// Put InjLibInfo in remote thread's memory block.
	if (!WriteProcessMemory(hProcess, pInjLibInfoRemote,
							&InjLibInfo, sizeof(InjLibInfo), &dwNumBytesXferred)) {
		goto finish;
	}

#if 0    
    if (pbData) {
#endif        
        if ((hThread = CreateRemoteThread(hProcess, NULL, 65536, 
                                          (LPTHREAD_START_ROUTINE) pdwCodeRemote,
                                          pInjLibInfoRemote, 0, &dwThreadId)) == NULL) {
            goto finish;
        }
        
    if (pbData) {
        WaitForSingleObject(hThread, INFINITE);
    }
	
	fSucceeded = TRUE;
	
  finish:
	if (hThread != NULL)
		CloseHandle(hThread);

	return fSucceeded;
}

//////////////////////////////////////////////////////////////////////////////
//
static PBYTE GetLoadLibraryA(VOID)
{
    return (PBYTE)GetProcAddress(GetModuleHandleA("Kernel32"), "LoadLibraryA");
}

static PBYTE GetLoadLibraryW(VOID)
{
    return (PBYTE)GetProcAddress(GetModuleHandleA("Kernel32"), "LoadLibraryW");
}

BOOL WINAPI DetourContinueProcessWithDllA(HANDLE hProcess, LPCSTR lpDllName)
{
	if (lpDllName) {
		if (!InjectLibraryOld(hProcess, GetLoadLibraryA(),
                              (PBYTE)lpDllName, strlen(lpDllName) + 1)) {
			return FALSE;
		}
	}
	return TRUE;
}


BOOL WINAPI DetourContinueProcessWithDllW(HANDLE hProcess, LPCWSTR lpDllName)
{
	if (lpDllName) {
		if (!InjectLibraryOld(hProcess, GetLoadLibraryW(),
                              (PBYTE)lpDllName, 2 * wcslen(lpDllName) + 2)) {
			return FALSE;
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL WINAPI DetourCreateProcessWithDllA(LPCSTR lpApplicationName,
                                        LPSTR lpCommandLine,
                                        LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                        LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                        BOOL bInheritHandles,
                                        DWORD dwCreationFlags,
                                        LPVOID lpEnvironment,
                                        LPCSTR lpCurrentDirectory,
                                        LPSTARTUPINFOA lpStartupInfo,
                                        LPPROCESS_INFORMATION lpProcessInformation,
                                        LPCSTR lpDllName,
                                        PDETOUR_CREATE_PROCESS_ROUTINEA pfCreateProcessA)
{
	DWORD dwMyCreationFlags = (dwCreationFlags | CREATE_SUSPENDED);
	PROCESS_INFORMATION pi;
	
	if (pfCreateProcessA == NULL) {
		pfCreateProcessA = CreateProcessA;
	}
	
	if (!pfCreateProcessA(lpApplicationName,
						  lpCommandLine,
						  lpProcessAttributes,
						  lpThreadAttributes,
						  bInheritHandles,
						  dwMyCreationFlags,
						  lpEnvironment,
						  lpCurrentDirectory,
						  lpStartupInfo,
						  &pi)) {
		return FALSE;
	}

    if (!InjectLibrary(pi.hProcess, pi.hThread, GetLoadLibraryA(),
                       (PBYTE)lpDllName,
                       lpDllName ? strlen(lpDllName) + 1 : 0)) {
        return FALSE;
    }
	if (lpProcessInformation) {
		CopyMemory(lpProcessInformation, &pi, sizeof(pi));
	}
	if (!(dwCreationFlags & CREATE_SUSPENDED)) {
		ResumeThread(pi.hThread);
	}
	return TRUE;
}


BOOL WINAPI DetourCreateProcessWithDllW(LPCWSTR lpApplicationName,
                                        LPWSTR lpCommandLine,
                                        LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                        LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                        BOOL bInheritHandles,
                                        DWORD dwCreationFlags,
                                        LPVOID lpEnvironment,
                                        LPCWSTR lpCurrentDirectory,
                                        LPSTARTUPINFOW lpStartupInfo,
                                        LPPROCESS_INFORMATION lpProcessInformation,
                                        LPCWSTR lpDllName,
                                        PDETOUR_CREATE_PROCESS_ROUTINEW pfCreateProcessW)
{
	DWORD dwMyCreationFlags = (dwCreationFlags | CREATE_SUSPENDED);
	PROCESS_INFORMATION pi;

	if (pfCreateProcessW == NULL) {
		pfCreateProcessW = CreateProcessW;
	}
	
	if (!pfCreateProcessW(lpApplicationName,
						  lpCommandLine,
						  lpProcessAttributes,
						  lpThreadAttributes,
						  bInheritHandles,
						  dwMyCreationFlags,
						  lpEnvironment,
						  lpCurrentDirectory,
						  lpStartupInfo,
						  &pi)) {
		return FALSE;
	}

    if (!InjectLibrary(pi.hProcess, pi.hThread, GetLoadLibraryW(),
                       (PBYTE)lpDllName,
                       lpDllName ? 2 * wcslen(lpDllName) + 2 : 0)) {
        return FALSE;
    }
	if (lpProcessInformation) {
		CopyMemory(lpProcessInformation, &pi, sizeof(pi));
	}
	if (!(dwCreationFlags & CREATE_SUSPENDED)) {
		ResumeThread(pi.hThread);
	}
	return TRUE;
}
//
///////////////////////////////////////////////////////////////// End of File.





