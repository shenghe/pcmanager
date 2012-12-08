////////////////////////////////////////////////////////////////////////////////
//      
//      File for netmon
//      
//      File      : netmonsp.h
//      Version   : 1.0
//      Comment   : 网络监控模块
//      
//      Create at : 2011-1-11
//      Create by : liupeng
//		Motify by : 
//      
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Unknwn.h>


//////////////////////////////////////////////////////////////////////////
//
// 服务调用模块
//
//////////////////////////////////////////////////////////////////////////

#define UIDDEF_INETMONSERVICEPROVIDER __declspec( uuid( "4F7E33D0-3455-4e7f-B0A0-F8CEA887D673" ) )
struct UIDDEF_INETMONSERVICEPROVIDER INetMonServiceProvider : public IUnknown
{
	virtual int __stdcall InitializeService( ) = 0;

	virtual int __stdcall UninitializeService() = 0;

	virtual int __stdcall StartService() = 0;

	virtual int __stdcall StopService() = 0;	
};
