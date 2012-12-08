#pragma once

#include "SCOM/SCOM/SCOMBase.h"
#include "KisRpc/Rfcdef.h"

interface IRemoteSCOMMgr
{
	virtual HRESULT STDMETHODCALLTYPE CreateRemoteInstance(IN KSCLSID clsid, IN KSCLSID callerClsid, IN DWORD callerPID
		) = 0;
};


//RPC接口定义
RCF_CLASS_BEGIN_WITH_COM(IRemoteSCOMMgr)
RCF_METHOD_R_3(HRESULT, CreateRemoteInstance, KSCLSID, KSCLSID, DWORD)
RCF_CLASS_END

