#pragma  once

class IScanFileCallBack
{
public:
    virtual HRESULT AfterScanFile(        
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag ) = 0;
    virtual HRESULT BeforeCleanFile(
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag,
        /* [in ] */ UINT64*             pulBackId = NULL ) = 0;
};