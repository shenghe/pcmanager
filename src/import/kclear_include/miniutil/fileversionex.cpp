/**
* @file    fileversionex.cpp
* @brief   ...
* @author  zhangrui
* @date    2008-12-28  03:21
*/

#include "stdafx.h"
#include "fileversionex.h"

#pragma comment(lib, "version.lib")

////////////////////////////////////////////////////////////////
// 1998 Microsoft Systems Journal
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
// CModuleVersion provides an easy way to get version info
// for a module.(DLL or EXE).
//
CModuleVersion::CModuleVersion()
{
}

//////////////////
// Destroy: delete version info
//
CModuleVersion::~CModuleVersion()
{
}

//////////////////
// Get file version info for a given module
// Allocates storage for all info, fills "this" with
// VS_FIXEDFILEINFO, and sets codepage.
//
BOOL CModuleVersion::GetFileVersionInfo(LPCTSTR modulename)
{
    m_translation.charset = 1252;    // default = ANSI code page
    memset((VS_FIXEDFILEINFO*)this, 0, sizeof(VS_FIXEDFILEINFO));

    // get module handle
    TCHAR filename[_MAX_PATH];
    HMODULE hModule = ::GetModuleHandle(modulename);
    if (hModule==NULL && modulename!=NULL)
        return FALSE;

    // get module file name
    DWORD len = GetModuleFileName(hModule, filename, sizeof(filename)/sizeof(filename[0]));
    if (len <= 0)
        return FALSE;

    // read file version info
    DWORD dwDummyHandle; // will always be set to zero
    len = GetFileVersionInfoSize(filename, &dwDummyHandle);
    if (len <= 0)
        return FALSE;

    BYTE* pbData = new BYTE[len]; // allocate version info
    if (!pbData)
        return FALSE;

    m_atpVersionInfo.Attach(pbData);
    if (!::GetFileVersionInfo(filename, 0, len, m_atpVersionInfo))
        return FALSE;

    LPVOID lpvi;
    UINT iLen;
    if (!VerQueryValue(m_atpVersionInfo, _T("\\"), &lpvi, &iLen))
        return FALSE;

    // copy fixed info to myself, which am derived from VS_FIXEDFILEINFO
    *(VS_FIXEDFILEINFO*)this = *(VS_FIXEDFILEINFO*)lpvi;

    // Get translation info
    if (VerQueryValue(m_atpVersionInfo,
        _T("\\VarFileInfo\\Translation"), &lpvi, &iLen) && iLen >= 4) {
        m_translation = *(TRANSLATION*)lpvi;
    }

    return dwSignature == VS_FFI_SIGNATURE;
}

BOOL CModuleVersion::GetDLLVersionInfo(LPCTSTR filename)
{
    m_translation.charset = 1252;    // default = ANSI code page
    memset((VS_FIXEDFILEINFO*)this, 0, sizeof(VS_FIXEDFILEINFO));

    // get module handle
    //HMODULE hModule = LoadLibrary(filename);
    //if (hModule==NULL)
    //    return FALSE;

    // some times, LoadLibrary may fail , --- by bbcallen
    HMODULE hModule = LoadLibraryEx(filename, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hModule==NULL)
        return FALSE;

    // read file version info
    DWORD dwDummyHandle; // will always be set to zero
    DWORD len = GetFileVersionInfoSize(filename, &dwDummyHandle);
    if (len <= 0)
    {
        FreeLibrary(hModule);
        return FALSE;
    }

    BYTE* pbData = new BYTE[len]; // allocate version info
    if (!pbData)
    {
        FreeLibrary(hModule);
        return FALSE;
    }

    m_atpVersionInfo.Attach(pbData);
    if (!::GetFileVersionInfo(filename, 0, len, m_atpVersionInfo))
    {
        FreeLibrary(hModule);
        return FALSE;
    }

    LPVOID lpvi;
    UINT iLen;
    if (!VerQueryValue(m_atpVersionInfo, _T("\\"), &lpvi, &iLen))
    {
        FreeLibrary(hModule);
        return FALSE;
    }

    // copy fixed info to myself, which am derived from VS_FIXEDFILEINFO
    *(VS_FIXEDFILEINFO*)this = *(VS_FIXEDFILEINFO*)lpvi;

    // Get translation info
    if (VerQueryValue(m_atpVersionInfo,
        _T("\\VarFileInfo\\Translation"), &lpvi, &iLen) && iLen >= 4) {
        m_translation = *(TRANSLATION*)lpvi;
    }
    FreeLibrary(hModule);
    return dwSignature == VS_FFI_SIGNATURE;
}

//////////////////
// Get string file info.
// Key name is something like "CompanyName".
// returns the value as a CString.
//
CString CModuleVersion::GetValue(LPCTSTR lpKeyName)
{
    CString sVal;
    if (m_atpVersionInfo) {

        // To get a string value must pass query in the form
        //
        //    "\StringFileInfo\<langID><codepage>\keyname"
        //
        // where <langID><codepage> is the languageID concatenated with the
        // code page, in hex. Wow.
        //
        CString query;
        query.Format(_T("\\StringFileInfo\\%04x%04x\\%s"),
                   m_translation.langID,
                   m_translation.charset,
                   lpKeyName);

        LPCTSTR pVal;
        UINT iLenVal;
        if (VerQueryValue(m_atpVersionInfo, (LPTSTR)(LPCTSTR)query,
            (LPVOID*)&pVal, &iLenVal)) {

            sVal = pVal;
        }
    }
    return sVal;
}
