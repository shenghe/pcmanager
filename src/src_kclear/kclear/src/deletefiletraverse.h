// DeleteFileTraverse.h: interface for the CDeleteFileTraverse class.
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "TraverseFile.h"


class CFileDelete
{
    typedef BOOL (WINAPI * PFN_MoveFileExA)(LPCSTR, LPCSTR, DWORD);

public:
    
    CFileDelete();
    ~CFileDelete();

    int TrashFileDelete(LPCTSTR pszFullName);


    BOOL MoveFileEx(
        LPCSTR lpExistingFileName,
        LPCSTR lpNewFileName,
        DWORD dwFlags);

private:
    HMODULE m_hModule;
    PFN_MoveFileExA m_pfnMoveFileExA;
};


typedef void (__stdcall * TraverseProcessCallback)(unsigned long, unsigned long);


