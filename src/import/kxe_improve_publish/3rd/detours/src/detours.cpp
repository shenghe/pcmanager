//////////////////////////////////////////////////////////////////////////////
//
//  File:       detours.cpp
//  Module:     detours.lib
//
//  Detours for binary functions.  Version 1.5 (Build 46)
//
//  Copyright 1995-2001, Microsoft Corporation
//

#include <ole2.h>
#include <imagehlp.h>

#define DETOURS_INTERNAL
#include "detours.h"

//////////////////////////////////////////////////////////////////////////////
//
enum {
    OP_PRE_ES       = 0x26,
    OP_PRE_CS       = 0x2e,
    OP_PRE_SS       = 0x36,
    OP_PRE_DS       = 0x3e,
    OP_PRE_FS       = 0x64,
    OP_PRE_GS       = 0x65,
    OP_JMP_SEG      = 0x25,
    
    OP_JA           = 0x77,
    OP_NOP          = 0x90,
    OP_CALL         = 0xe8,
    OP_JMP          = 0xe9,
    OP_PREFIX       = 0xff,
    OP_MOV_EAX      = 0xa1,
    OP_SET_EAX      = 0xb8,
    OP_JMP_EAX      = 0xe0,
    OP_RET_POP      = 0xc2,
    OP_RET          = 0xc3,
    OP_BRK          = 0xcc,

    SIZE_OF_JMP     = 5,
    SIZE_OF_NOP     = 1,
    SIZE_OF_BRK     = 1,
    SIZE_OF_TRP_OPS = SIZE_OF_JMP /* + SIZE_OF_BRK */,
};

//////////////////////////////////////////////////////////////////////////////
//
static BOOL detour_insert_jump(PBYTE pbCode, PBYTE pbDest, LONG cbCode)
{
    if (cbCode < SIZE_OF_JMP)
        return FALSE;

    pbCode = DetourGenJmp(pbCode, pbDest);
    for (cbCode -= SIZE_OF_JMP; cbCode > 0; cbCode--) {
        pbCode = DetourGenBreak(pbCode);
    }
    return TRUE;
}

static BOOL detour_insert_detour(PBYTE pbTarget,
                                 PBYTE pbTrampoline,
                                 PBYTE pbDetour)
{
    PBYTE pbCont = pbTarget;
    for (LONG cbTarget = 0; cbTarget < SIZE_OF_TRP_OPS;) {
        PBYTE pbOp = pbCont;
        BYTE bOp = *pbOp;
        pbCont = DetourCopyInstruction(NULL, pbCont, NULL);
        cbTarget = pbCont - pbTarget;

        if (bOp == OP_JMP ||
            bOp == OP_JMP_EAX ||
            bOp == OP_RET_POP ||
            bOp == OP_RET) {

            break;
        }
        if (bOp == OP_PREFIX && pbOp[1] == OP_JMP_SEG) {
            break;
        }
        if ((bOp == OP_PRE_ES ||
             bOp == OP_PRE_CS ||
             bOp == OP_PRE_SS ||
             bOp == OP_PRE_DS ||
             bOp == OP_PRE_FS ||
             bOp == OP_PRE_GS) &&
            pbOp[1] == OP_PREFIX &&
            pbOp[2] == OP_JMP_SEG) {
            break;
        }
    }
    if (cbTarget  < SIZE_OF_TRP_OPS) {
        // Too few instructions.
        return FALSE;
    }
    if (cbTarget > (DETOUR_TRAMPOLINE_SIZE - SIZE_OF_JMP - 1)) {
        // Too many instructions.
        return FALSE;
    }

    //////////////////////////////////////////////////////// Finalize Reroute.
    //
    CDetourEnableWriteOnCodePage ewTrampoline(pbTrampoline, DETOUR_TRAMPOLINE_SIZE);
    CDetourEnableWriteOnCodePage ewTarget(pbTarget, cbTarget);
    if (!ewTrampoline.SetPermission(PAGE_EXECUTE_READWRITE))
        return FALSE;
    if (!ewTarget.IsValid())
        return FALSE;
    
    PBYTE pbSrc = pbTarget;
    PBYTE pbDst = pbTrampoline;
    for (LONG cbCopy = 0; cbCopy < cbTarget;) {
        pbSrc = DetourCopyInstruction(pbDst, pbSrc, NULL);
        cbCopy = pbSrc - pbTarget;
        pbDst = pbTrampoline + cbCopy;
    }
    if (cbCopy != cbTarget)                             // Count came out different!
        return FALSE;

    if (!detour_insert_jump(pbDst, pbTarget + cbTarget, SIZE_OF_JMP))
        return FALSE;

    pbTrampoline[DETOUR_TRAMPOLINE_SIZE-1] = (BYTE)cbTarget;

    if (!detour_insert_jump(pbTarget, pbDetour, cbTarget))
        return FALSE;
    
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
BOOL WINAPI DetourRemove(PBYTE pbTrampoline,
                                       PBYTE pbDetour)
{
    pbTrampoline = DetourGetFinalCode(pbTrampoline, TRUE);
    pbDetour = DetourGetFinalCode(pbDetour, FALSE);

    ////////////////////////////////////// Verify that Trampoline is in place.
    //
    LONG cbTarget = pbTrampoline[DETOUR_TRAMPOLINE_SIZE-1];
    if (cbTarget == 0 || cbTarget >= DETOUR_TRAMPOLINE_SIZE - 1) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (pbTrampoline[cbTarget] != OP_JMP) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
        
    LONG offset = *((PDWORD)&pbTrampoline[cbTarget + 1]);
    PBYTE pbTarget = pbTrampoline + cbTarget + SIZE_OF_JMP + offset - cbTarget;

    if (pbTarget[0] != OP_JMP) {                        // Missing detour.
        SetLastError(ERROR_INVALID_BLOCK);
        return FALSE;
    }

    offset = *((PDWORD)&pbTarget[1]);
    PBYTE pbTargetDetour = pbTarget + SIZE_OF_JMP + offset;
    if (pbTargetDetour != pbDetour) {
        SetLastError(ERROR_INVALID_ACCESS);
        return FALSE;
    }

    /////////////////////////////////////////////////////// Remove the Detour.
    CDetourEnableWriteOnCodePage ewTarget(pbTarget, cbTarget);
    
    PBYTE pbSrc = pbTrampoline;
    PBYTE pbDst = pbTarget;
    for (LONG cbCopy = 0; cbCopy < cbTarget; pbDst = pbTarget + cbCopy) {
        pbSrc = DetourCopyInstruction(pbDst, pbSrc, NULL);
        cbCopy = pbSrc - pbTrampoline;
    }
    if (cbCopy != cbTarget) {                           // Count came out different!
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }
    return TRUE;
}

PBYTE WINAPI DetourFunction(PBYTE pbTarget,
                            PBYTE pbDetour)
{
    PBYTE pbTrampoline = new BYTE [DETOUR_TRAMPOLINE_SIZE];
    if (pbTrampoline == NULL)
        return NULL;

    pbTarget = DetourGetFinalCode(pbTarget, FALSE);
    pbDetour = DetourGetFinalCode(pbDetour, FALSE);

    if (detour_insert_detour(pbTarget, pbTrampoline, pbDetour))
        return pbTrampoline;

    delete[] pbTrampoline;
    return NULL;
}

BOOL WINAPI DetourFunctionWithEmptyTrampoline(PBYTE pbTrampoline,
                                              PBYTE pbTarget,
                                              PBYTE pbDetour)
{
    return DetourFunctionWithEmptyTrampolineEx(pbTrampoline, pbTarget, pbDetour,
                                               NULL, NULL, NULL);
}

BOOL WINAPI DetourFunctionWithEmptyTrampolineEx(PBYTE pbTrampoline,
                                                PBYTE pbTarget,
                                                PBYTE pbDetour,
                                                PBYTE *ppbRealTrampoline,
                                                PBYTE *ppbRealTarget,
                                                PBYTE *ppbRealDetour)
{
    pbTrampoline = DetourGetFinalCode(pbTrampoline, TRUE);
    pbTarget = DetourGetFinalCode(pbTarget, FALSE);
    pbDetour = DetourGetFinalCode(pbDetour, FALSE);
    
    if (ppbRealTrampoline)
        *ppbRealTrampoline = pbTrampoline;
    if (ppbRealTarget)
        *ppbRealTarget = pbTarget;
    if (ppbRealDetour)
        *ppbRealDetour = pbDetour;
    
    if (pbTrampoline == NULL || pbDetour == NULL || pbTarget == NULL)
        return FALSE;
    
    if (pbTrampoline[0] != OP_NOP ||
        pbTrampoline[1] != OP_NOP) {
        
        return FALSE;
    }
    
    return detour_insert_detour(pbTarget, pbTrampoline, pbDetour);
}

BOOL WINAPI DetourFunctionWithTrampoline(PBYTE pbTrampoline,
                                         PBYTE pbDetour)
{
    return DetourFunctionWithTrampolineEx(pbTrampoline, pbDetour, NULL, NULL);
}

BOOL WINAPI DetourFunctionWithTrampolineEx(PBYTE pbTrampoline,
                                           PBYTE pbDetour,
                                           PBYTE *ppbRealTrampoline,
                                           PBYTE *ppbRealTarget)
{
    PBYTE pbTarget = NULL;

    pbTrampoline = DetourGetFinalCode(pbTrampoline, TRUE);
    pbDetour = DetourGetFinalCode(pbDetour, FALSE);
    
    if (ppbRealTrampoline)
        *ppbRealTrampoline = pbTrampoline;
    if (ppbRealTarget)
        *ppbRealTarget = NULL;
    
    if (pbTrampoline == NULL || pbDetour == NULL)
        return FALSE;

    if (pbTrampoline[0] != OP_NOP   ||
        pbTrampoline[1] != OP_NOP   ||
        pbTrampoline[2] != OP_CALL  ||
        pbTrampoline[7] != OP_PREFIX    ||
        pbTrampoline[8] != OP_JMP_EAX) {
        
        return FALSE;
    }

    PVOID (__fastcall * pfAddr)(VOID);

    pfAddr = (PVOID (__fastcall *)(VOID))(pbTrampoline +
                                          SIZE_OF_NOP + SIZE_OF_NOP + SIZE_OF_JMP +
                                          *(LONG *)&pbTrampoline[3]);

    pbTarget = DetourGetFinalCode((PBYTE)(*pfAddr)(), FALSE);
    if (ppbRealTarget)
        *ppbRealTarget = pbTarget;

    return detour_insert_detour(pbTarget, pbTrampoline, pbDetour);
}

//////////////////////////////////////////////////////////////////////////////

PDETOUR_SYM_INFO DetourLoadImageHlp(VOID)
{
    static DETOUR_SYM_INFO symInfo;
    static PDETOUR_SYM_INFO pSymInfo= NULL;

    if (pSymInfo != NULL) {
        return pSymInfo;
    }

    pSymInfo = &symInfo;
    ZeroMemory(&symInfo, sizeof(symInfo));
    symInfo.hProcess = GetCurrentProcess();

    symInfo.hImageHlp = LoadLibraryA("imagehlp.dll");
    if (symInfo.hImageHlp == NULL) {
        return NULL;
    }

    symInfo.pfImagehlpApiVersionEx
        = (PF_ImagehlpApiVersionEx)GetProcAddress(symInfo.hImageHlp,
                                                  "ImagehlpApiVersionEx");
    symInfo.pfSymInitialize
        = (PF_SymInitialize)GetProcAddress(symInfo.hImageHlp, "SymInitialize");
    symInfo.pfSymSetOptions
        = (PF_SymSetOptions)GetProcAddress(symInfo.hImageHlp, "SymSetOptions");
    symInfo.pfSymGetOptions
        = (PF_SymGetOptions)GetProcAddress(symInfo.hImageHlp, "SymGetOptions");
    symInfo.pfSymLoadModule
        = (PF_SymLoadModule)GetProcAddress(symInfo.hImageHlp, "SymLoadModule");
    symInfo.pfSymGetModuleInfo
        = (PF_SymGetModuleInfo)GetProcAddress(symInfo.hImageHlp, "SymGetModuleInfo");
    symInfo.pfSymGetSymFromName
        = (PF_SymGetSymFromName)GetProcAddress(symInfo.hImageHlp, "SymGetSymFromName");
    symInfo.pfBindImage
        = (PF_BindImage)GetProcAddress(symInfo.hImageHlp, "BindImage");

    API_VERSION av;
    ZeroMemory(&av, sizeof(av));
    av.MajorVersion = API_VERSION_NUMBER;
            
    if (symInfo.pfImagehlpApiVersionEx) {
        (*symInfo.pfImagehlpApiVersionEx)(&av);
    }

    if (symInfo.pfImagehlpApiVersionEx == NULL || av.MajorVersion < API_VERSION_NUMBER) {
        FreeLibrary(symInfo.hImageHlp);
        symInfo.hImageHlp = NULL;
        return NULL;
    }
        
    if (symInfo.pfSymInitialize) {
        (*symInfo.pfSymInitialize)(symInfo.hProcess, NULL, FALSE);
    }
        
    if (symInfo.pfSymGetOptions && symInfo.pfSymSetOptions) {
        DWORD dw = (*symInfo.pfSymGetOptions)();
        dw &= (SYMOPT_CASE_INSENSITIVE | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
        (*symInfo.pfSymSetOptions)(dw);
    }
        
    return pSymInfo;
}

//////////////////////////////////////////////////////////////////////////////
//
PBYTE WINAPI DetourGetFinalCode(PBYTE pbCode, BOOL fSkipJmp)
{
    if (pbCode == NULL) {
        return NULL;
    }
    
    if (pbCode[0] == OP_PREFIX && pbCode[1] == OP_JMP_SEG) {
        // Looks like an import alias jump, then get the code it points to.
        pbCode = *(PBYTE *)&pbCode[2];
        pbCode = *(PBYTE *)pbCode;
    }
    else if (pbCode[0] == OP_JMP && fSkipJmp) {         // Reference passed (for tramp).
        // Looks like a reference passed from an incremental-link build.
        // We only skip these for trampolines.
        pbCode = pbCode + SIZE_OF_JMP + *(LONG *)&pbCode[1];
    }
    return pbCode;
}

PBYTE WINAPI DetourFindFunction(PCHAR pszModule, PCHAR pszFunction)
{
    /////////////////////////////////////////////// First, Try GetProcAddress.
    //
    HMODULE hModule = LoadLibraryA(pszModule);
    if (hModule == NULL) {
        return NULL;
    }

    PBYTE pbCode = (PBYTE)GetProcAddress(hModule, pszFunction);
    if (pbCode) {
        return pbCode;
    }

    ////////////////////////////////////////////////////// Then Try ImageHelp.
    //
    PDETOUR_SYM_INFO pSymInfo = DetourLoadImageHlp();
    if (pSymInfo == NULL || 
        pSymInfo->pfSymLoadModule == NULL ||
        pSymInfo->pfSymGetModuleInfo == NULL ||
        pSymInfo->pfSymGetSymFromName == NULL) {

        return NULL;
    }
    
    (*pSymInfo->pfSymLoadModule)(pSymInfo->hProcess, NULL, pszModule, NULL, (DWORD)hModule, 0);

    IMAGEHLP_MODULE modinfo;
    ZeroMemory(&modinfo, sizeof(modinfo));
    if (!(*pSymInfo->pfSymGetModuleInfo)(pSymInfo->hProcess, (DWORD)hModule, &modinfo)) {
        return NULL;
    }

    CHAR szFullName[512];
    strcpy(szFullName, modinfo.ModuleName);
    strcat(szFullName, "!");
    strcat(szFullName, pszFunction);
    
    struct CFullSymbol : IMAGEHLP_SYMBOL {
        CHAR szRestOfName[512];
    } symbol;
    ZeroMemory(&symbol, sizeof(symbol));
    symbol.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
    symbol.MaxNameLength = sizeof(symbol.szRestOfName)/sizeof(0);

    if (!(*pSymInfo->pfSymGetSymFromName)(pSymInfo->hProcess, szFullName, &symbol)) {
        return NULL;
    }

    return (PBYTE)symbol.Address;
}

//////////////////////////////////////////////////// Module Image Functions.
//
HMODULE WINAPI DetourEnumerateModules(HMODULE hModuleLast)
{
    PBYTE pbLast;
    
    if (hModuleLast == NULL) {
        pbLast = (PBYTE)0x10000;
    }
    else {
        pbLast = (PBYTE)hModuleLast + 0x10000;
    }

    MEMORY_BASIC_INFORMATION mbi;
    ZeroMemory(&mbi, sizeof(mbi));

    // Find the next memory region that contains a mapped PE image.
    //
    for (;; pbLast = (PBYTE)mbi.BaseAddress + mbi.RegionSize) {
        if (VirtualQuery((PVOID)pbLast, &mbi, sizeof(mbi)) <= 0) {
            break;
        }

        // Skip uncommitted regions and guard pages.
        //
        if ((mbi.State != MEM_COMMIT) || (mbi.Protect & PAGE_GUARD)) {
            continue;
        }
        
        __try {
            PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pbLast;
            if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
                continue;
            }

            PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDosHeader +
                                                              pDosHeader->e_lfanew);
            if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) {
                continue;
            }

            return (HMODULE)pDosHeader;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            /* nothing. */
        }
    }
    return NULL;
}

PBYTE WINAPI DetourGetEntryPoint(HMODULE hModule)
{
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (hModule == NULL) {
        pDosHeader = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
    }
    
    __try {
        if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            SetLastError(ERROR_BAD_EXE_FORMAT);
            return NULL;
        }
        
        PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDosHeader +
                                                          pDosHeader->e_lfanew);
        if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) {
            SetLastError(ERROR_INVALID_EXE_SIGNATURE);
            return NULL;
        }
        if (pNtHeader->FileHeader.SizeOfOptionalHeader == 0) {
            SetLastError(ERROR_EXE_MARKED_INVALID);
            return NULL;
        }
        SetLastError(NO_ERROR);
        return (PBYTE)pNtHeader->OptionalHeader.AddressOfEntryPoint +
            pNtHeader->OptionalHeader.ImageBase;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }
    SetLastError(ERROR_EXE_MARKED_INVALID);
    
    return NULL;
}

static inline PBYTE RvaAdjust(HMODULE hModule, DWORD raddr)
{
    if (raddr != NULL) {
        return (PBYTE)hModule + raddr;
    }
    return NULL;
}

BOOL WINAPI DetourEnumerateExports(HMODULE hModule,
                                   PVOID pContext,
                                   PF_DETOUR_BINARY_EXPORT_CALLBACK pfExport)
{
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (hModule == NULL) {
        pDosHeader = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
    }
    
    __try {
        if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            SetLastError(ERROR_BAD_EXE_FORMAT);
            return NULL;
        }
        
        PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDosHeader +
                                                          pDosHeader->e_lfanew);
        if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) {
            SetLastError(ERROR_INVALID_EXE_SIGNATURE);
            return FALSE;
        }
        if (pNtHeader->FileHeader.SizeOfOptionalHeader == 0) {
            SetLastError(ERROR_EXE_MARKED_INVALID);
            return FALSE;
        }

        PIMAGE_EXPORT_DIRECTORY pExportDir
            = (PIMAGE_EXPORT_DIRECTORY)
            RvaAdjust(hModule,
                      pNtHeader->OptionalHeader
                      .DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
        
        if (pExportDir == NULL) {
            SetLastError(ERROR_EXE_MARKED_INVALID);
            return FALSE;
        }

        PDWORD pdwFunctions = (PDWORD)RvaAdjust(hModule, pExportDir->AddressOfFunctions);
        PDWORD pdwNames = (PDWORD)RvaAdjust(hModule, pExportDir->AddressOfNames);
        PWORD pwOrdinals = (PWORD)RvaAdjust(hModule, pExportDir->AddressOfNameOrdinals);

        for (DWORD nFunc = 0; nFunc < pExportDir->NumberOfFunctions; nFunc++) {
            PBYTE pbCode = (PBYTE)RvaAdjust(hModule, pdwFunctions[nFunc]);
            PCHAR pszName = (nFunc < pExportDir->NumberOfNames) ?
                (PCHAR)RvaAdjust(hModule, pdwNames[nFunc]) : NULL;
            ULONG nOrdinal = pExportDir->Base + pwOrdinals[nFunc];

            if (!(*pfExport)(pContext, nOrdinal, pszName, pbCode)) {
                break;
            }
        }
        SetLastError(NO_ERROR);
        return TRUE;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }
    SetLastError(ERROR_EXE_MARKED_INVALID);
    return FALSE;
}

static PDETOUR_LOADED_BINARY WINAPI GetPayloadSectionFromModule(HMODULE hModule)
{
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (hModule == NULL) {
        pDosHeader = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
    }
    
    __try {
        if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            SetLastError(ERROR_BAD_EXE_FORMAT);
            return NULL;
        }
        
        PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDosHeader +
                                                          pDosHeader->e_lfanew);
        if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) {
            SetLastError(ERROR_INVALID_EXE_SIGNATURE);
            return NULL;
        }
        if (pNtHeader->FileHeader.SizeOfOptionalHeader == 0) {
            SetLastError(ERROR_EXE_MARKED_INVALID);
            return NULL;
        }
        
        PIMAGE_SECTION_HEADER pSectionHeaders
            = (PIMAGE_SECTION_HEADER)((PBYTE)pNtHeader
                                      + sizeof(pNtHeader->Signature)
                                      + sizeof(pNtHeader->FileHeader)
                                      + pNtHeader->FileHeader.SizeOfOptionalHeader);

        for (DWORD n = 0; n < pNtHeader->FileHeader.NumberOfSections; n++) {
            if (strcmp((PCHAR)pSectionHeaders[n].Name, ".detour") == 0) {
                if (pSectionHeaders[n].VirtualAddress == 0 ||
                    pSectionHeaders[n].SizeOfRawData == 0) {

                    break;
                }
                    
                PBYTE pbData = (PBYTE)pDosHeader + pSectionHeaders[n].VirtualAddress;
                DETOUR_SECTION_HEADER *pHeader = (DETOUR_SECTION_HEADER *)pbData;
                if (pHeader->cbHeaderSize < sizeof(DETOUR_SECTION_HEADER) ||
                    pHeader->nSignature != DETOUR_SECTION_HEADER_SIGNATURE) {
                    
                    break;
                }

                if (pHeader->nDataOffset == 0) {
                    pHeader->nDataOffset = pHeader->cbHeaderSize;
                }
                SetLastError(NO_ERROR);
                return (PBYTE)pHeader;
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }
    SetLastError(ERROR_EXE_MARKED_INVALID);
    
    return NULL;
}

DWORD WINAPI DetourGetSizeOfPayloads(HMODULE hModule)
{
    PDETOUR_LOADED_BINARY pBinary = GetPayloadSectionFromModule(hModule);
    
    __try {
        DETOUR_SECTION_HEADER *pHeader = (DETOUR_SECTION_HEADER *)pBinary;
        if (pHeader->cbHeaderSize < sizeof(DETOUR_SECTION_HEADER) ||
            pHeader->nSignature != DETOUR_SECTION_HEADER_SIGNATURE) {
            
            SetLastError(ERROR_INVALID_HANDLE);
            return 0;
        }
        SetLastError(NO_ERROR);
        return pHeader->cbDataSize;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
    SetLastError(ERROR_INVALID_HANDLE);
    return 0;
}

PBYTE WINAPI DetourFindPayload(HMODULE hModule, REFGUID rguid, DWORD * pcbData)
{
    PBYTE pbData = NULL;
    if (pcbData) {
        *pcbData = 0;
    }

    PDETOUR_LOADED_BINARY pBinary = GetPayloadSectionFromModule(hModule);
    
    __try {
        DETOUR_SECTION_HEADER *pHeader = (DETOUR_SECTION_HEADER *)pBinary;
        if (pHeader->cbHeaderSize < sizeof(DETOUR_SECTION_HEADER) ||
            pHeader->nSignature != DETOUR_SECTION_HEADER_SIGNATURE) {

            SetLastError(ERROR_INVALID_EXE_SIGNATURE);
            return NULL;
        }
        
        PBYTE pbBeg = ((PBYTE)pHeader) + pHeader->nDataOffset;
        PBYTE pbEnd = ((PBYTE)pHeader) + pHeader->cbDataSize;
        
        for (pbData = pbBeg; pbData < pbEnd;) {
            DETOUR_SECTION_RECORD *pSection = (DETOUR_SECTION_RECORD *)pbData;
            
            if (pSection->guid == rguid) {
                if (pcbData) {
                    *pcbData = pSection->cbBytes - sizeof(*pSection);
                    SetLastError(NO_ERROR);
                    return (PBYTE)(pSection + 1);
                }
            }
            
            pbData = (PBYTE)pSection + pSection->cbBytes;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_HANDLE);
        return NULL;
    }
    SetLastError(ERROR_INVALID_HANDLE);
    return NULL;
}

//  End of File
