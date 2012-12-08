#pragma once
#include "comproxy/com_scan_interface.h"
#include "com_svc_dispatch.h"
#include "comproxy/com_scan_xdx.h"
#include "scanwork.h"

class CBkScanSingleFileImp
    :   public CBkComRoot,
        public IBkScanSingleFile
{
public:
    CBkScanSingleFileImp(void);
    ~CBkScanSingleFileImp(void);
public:
    HRESULT Initialize( DWORD dwProcId, int nVersion );
    HRESULT Uninitialize();

    COM_METHOD_DISPATCH_BEGIN()
        COM_METHOD_DISPATCH_IN_ARG_3_OUT( CBkScanSingleFile, ScanFile, strFile, dwScanMask, dwScanPolicy, BK_FILE_RESULT )
        COM_METHOD_DISPATCH_IN_ARG_3_OUT( CBkScanSingleFile, CleanFile, strFile, dwScanMask, dwScanPolicy, BK_FILE_RESULT )
        COM_METHOD_DISPATCH_IN_ARG_2_OUT( CBkScanSingleFile, ForceClean, strFile, detail, BK_FILE_RESULT )
        //COM_METHOD_DISPATCH_IN_ARG_1_WITH_XDX_NO_OUT( BkScan, SetScanSetting, BK_SCAN_SETTING )
        //COM_METHOD_DISPATCH_NO_ARG_OUT( BkScanSingleFile, GetScanSetting, BK_SCAN_SETTING )
    COM_METHOD_DISPATCH_END
public:
    virtual HRESULT ScanFile( CString& strFile, DWORD dwScanMask, DWORD dwScanPolicy, BK_FILE_RESULT& fr );
    virtual HRESULT CleanFile( CString& strFile, DWORD dwScanMask, DWORD dwScanPolicy, BK_FILE_RESULT& fr );
    virtual HRESULT ForceClean( CString& strFile, BK_FORCE_CLEAN_DETAIL& detail, BK_FILE_RESULT& fr );
    //virtual HRESULT SetScanSetting( BK_SCAN_SETTING& setting );
    //virtual HRESULT GetScanSetting( BK_SCAN_SETTING& setting );
private:
    CBkFileScan*    m_pFileScan;
    CString         m_strFile;
    BOOL            m_bOnline;
    BK_SCAN_SETTING m_setting;
};
