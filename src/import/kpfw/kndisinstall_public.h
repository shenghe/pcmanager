/*++

Copyright (C) Kingsoft Corporation 2009

Creator:

Chen Liang <chenliang@kingsoft.com>

Description:

interface of ndis setup

--*/

#ifndef _INSTALLNDISPUBLIC_H_
#define _INSTALLNDISPUBLIC_H_

#include <Unknwn.h>

//=========================================================================s
[
	uuid("7101801C-2847-42b9-A571-A262A40ADA29")
]
interface INdisInstall : public IUnknown
{
	virtual STDMETHODIMP_(BOOL) InstallNdisDriver(
		LPCTSTR lpcszInfPath,
		ULONG uTimeOut, 
		BOOL* bTimeOut, 
		BOOL* bNeedReboot, 
		BOOL* bCanceled
		) PURE;

	virtual STDMETHODIMP_(BOOL) UnInstallNdisDriver(
		ULONG uTimeOut, 
		BOOL* bNeedReboot, 
		BOOL* bCanceled
		) PURE;

};

#endif