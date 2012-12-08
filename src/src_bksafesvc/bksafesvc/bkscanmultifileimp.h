#pragma once
#include "comproxy/com_scan_interface.h"
#include "com_svc_dispatch.h"
#include "comproxy/com_scan_xdx.h"
#include "scanwork.h"

class CBkScanMultiFileImp
    :   public CBkComRoot,
        public IBkScanMultiFile
{
public:
    CBkScanMultiFileImp(void);
    ~CBkScanMultiFileImp(void);

public:
    HRESULT Initialize( DWORD dwProcId, int nVersion );
    HRESULT Uninitialize();

    COM_METHOD_DISPATCH_BEGIN()
        COM_METHOD_DISPATCH_IN_ARG_2_OUT_WITH_XDX( CBkScanMultiFile, ScanHash, hashArray, dwScanPolicy, frArray )
        COM_METHOD_DISPATCH_IN_ARG_2_OUT_WITH_XDX( CBkScanMultiFile, ScanFile, fileArray, dwScanPolicy, frArray )
    COM_METHOD_DISPATCH_END
public:
    virtual HRESULT ScanHash( CAtlArray<CString>& hashArray, DWORD dwScanPolicy, CAtlArray<BK_FILE_RESULT>& frArray );
    virtual HRESULT ScanFile( CAtlArray<CString>& fileArray, DWORD dwScanPolicy, CAtlArray<BK_FILE_RESULT>& frArray );
private:
    CBkMultiFileScan*   m_pFileScan;
    BK_SCAN_SETTING m_setting;
};
