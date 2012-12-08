////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for kpfwtray
//      
//      File      : kpfwtrayservice.h
//      Comment   : kpfwtray保留的rpc接口
//      
//      Create at : 2008-07-18
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 

#include "SCOM/SCOM/SCOMBase.h"
#include "SCOM/SCOM/SCOMDef.h"

interface IKKpfwtrayService
{
	STDPROC_(HRESULT)		DoQuit()		PURE;
};

RCF_CLASS_BEGIN(IKKpfwtrayService)
	RCF_METHOD_R_0(HRESULT, DoQuit)
RCF_CLASS_END

#define KPFWTRAY_SINGLETEM_MUTEX		TEXT("kpfwtray_singleten_81B51423-CCB6-45c9-9B7D-B30635486BCB")

#define KPFWTRAY_APP				TEXT("kpfwtray.exe")

#define KPFWFRM_NAME				TEXT("kpfwfrm.exe")

#define KPFWTRAY_RPC_NAME		TEXT("\\\\.\\pipe\\kpfwtray_rpc_38A619E9-7E2F-4085-9C85-F78650F97DF9")