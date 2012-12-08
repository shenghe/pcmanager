/**
* @file    fileversionex.h
* @brief   ...
* @author  zhangrui
* @date    2008-12-28  03:21
*/

#ifndef FILEVERSIONEX_H
#define FILEVERSIONEX_H

#include <atlbase.h>
#include <atlstr.h>

////////////////////////////////////////////////////////////////
// 1998 Microsoft Systems Journal
//
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//

//////////////////
// CModuleVersion version info about a module.
// To use:
//
// CModuleVersion ver
// if (ver.GetFileVersionInfo("_T("mymodule))) {
//    // info is in ver, you can call GetValue to get variable info like
//    CString s = ver.GetValue(_T("CompanyName"));
// }
//
// You can also call the static fn DllGetVersion to get DLLVERSIONINFO.
//
class CModuleVersion : public VS_FIXEDFILEINFO
{
protected:
    CAutoVectorPtr<BYTE> m_atpVersionInfo;   // all version info

    struct TRANSLATION {
        WORD langID;         // language ID
        WORD charset;        // character set (code page)
    } m_translation;

public:
    CModuleVersion();
    virtual ~CModuleVersion();

    BOOL        GetFileVersionInfo(LPCTSTR modulename);
    BOOL        GetDLLVersionInfo(LPCTSTR filename);
    CString     GetValue(LPCTSTR lpKeyName);
};


#endif//FILEVERSIONEX_H