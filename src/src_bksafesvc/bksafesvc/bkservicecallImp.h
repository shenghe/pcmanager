#pragma once
#include "comproxy/com_sc_interface.h"
#include "com_svc_dispatch.h"
#include "comproxy/com_sc_xdx.h"

class CBkServiceCallImp
    : public    CBkComRoot,
      public    IBkServiceCall
{
public:
    CBkServiceCallImp(void);
    ~CBkServiceCallImp(void);
public:
    HRESULT Initialize( DWORD dwProcId, int nVersion );

    HRESULT Uninitialize();

    COM_METHOD_DISPATCH_BEGIN()
        COM_METHOD_DISPATCH_IN_ARG_3_NO_OUT( CBkServiceCall, Execute, strExePath, strCmdline, bShow )
    COM_METHOD_DISPATCH_END
public:
    virtual HRESULT Execute( 
        const CString& strExePath, 
        const CString& strCmdline, 
        BOOL bShow
        );
private:
    // 返回TRUE表示进程验证通过
    BOOL    VerifyProcess( DWORD dwProcId );
    // 返回TRUE表示路径验证通过
    BOOL    VerifyExePath( LPCWSTR lpwszExePath );
    // 返回TRUE表示签名验证通过
    BOOL    VerifyFileSign( LPCWSTR lpwszExePath );
private:
    HANDLE  m_hToken;
};
