#pragma once
#include "comproxy/com_utility_interface.h"
#include "com_svc_dispatch.h"
#include "comproxy/com_utility_xdx.h"
#include "unkownrescan.h"

class CBkRescanImp
    : public CBkComRoot,
      public IBkReScan
{
public:
    CBkRescanImp(void);
    ~CBkRescanImp(void);
public:
    HRESULT Initialize( DWORD dwProcId, int nVersion );

    HRESULT Uninitialize();

    COM_METHOD_DISPATCH_BEGIN()
        COM_METHOD_DISPATCH_NO_ARG_OUT_WITH_XDX( CBkRescan, GetRescanResult, result )
    COM_METHOD_DISPATCH_END
public:
    virtual HRESULT GetRescanResult( CAtlArray<BK_FILE_RESULT>& result );
};
