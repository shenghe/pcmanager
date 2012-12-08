////////////////////////////////////////////////////////////////////////////////
//      
//      Kingsoft File for kpfwfrm 
//      
//      File      : kpfwfrm.h
//      Comment   : kpfwfrm保留的rpc接口
//      
//      Create at : 2008-07-18
//      Create by : chenguicheng
//      
////////////////////////////////////////////////////////////////////////////////

#pragma once 

#include "SCOM/SCOM/SCOMBase.h"
#include "SCOM/SCOM/SCOMDef.h"

interface IKKpfwfrmService
{
	STDPROC_(HRESULT)		DoCommand(ATL::CString)		PURE;
	STDPROC_(HRESULT)		DoQuit()		PURE;
};

RCF_CLASS_BEGIN(IKKpfwfrmService)
	RCF_METHOD_R_1(HRESULT, DoCommand,		ATL::CString)
	RCF_METHOD_R_0(HRESULT, DoQuit)
RCF_CLASS_END

#define KPFWFRM_SINGLETEN_MUTEX		TEXT("kpfwfrm_singleten_BD9D8BD8-E282-4ac1-9222-4E1DF2BB206B")

#define KPFWFRM_RPC_NAME		TEXT("\\\\.\\pipe\\kpfwfrm_rpc_35D19EBE-1648-4486-AE59-743DD4BCFD6B")