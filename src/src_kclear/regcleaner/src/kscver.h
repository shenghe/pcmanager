#pragma once


//////////////////////////////////////////////////////////////////////////

//#include <miniutil/bkcmdline.h>

//////////////////////////////////////////////////////////////////////////

#define KSC_DDE_WND_CLASS L"{A866E23E-2789-44a7-AA60-8E728E108B93}"

//////////////////////////////////////////////////////////////////////////

class CKscVersion
{
public:
    CKscVersion()
    {
    }

    HRESULT Init(HINSTANCE hInstance);
    VOID UnInit();
    LPCTSTR GetAppVersion();

protected:
    CString m_strAppFileName;
    CString m_strAppPath;
    CString m_strAppVersion;
    LONG    m_lAppVersionNumber;

    void _GetFileInfo(BYTE *pbyInfo, LPCWSTR lpszFileInfoKey, LPWSTR lpszFileInfoRet);
    BOOL _GetPEProductVersion(LPCWSTR lpszFileName, CString &strProductVersion);
    BOOL GetProductVersion();
};

//////////////////////////////////////////////////////////////////////////
