/*++

Copyright (C) 2007-2008 Kingsoft Corporation

Creator:

    Leo Jiang <JiangFengbing@kingsoft.com>

Abstract:

    A mini support framework for advanced packet filter module.
    If you want to change the interface please notice Leo Jiang!!!
    The object file 'memcmp_for_cf.obj' is associated with this 
    interface!!!

Environment:

    Kernel driver

--*/

#ifndef ACF_SUPPORT_INC_
#define ACF_SUPPORT_INC_

#include "acf_unknwn.h"

//////////////////////////////////////////////////////////////////////////

[
    uuid(6f888df7-ad1d-4e64-adf5-2aebfcf152ba)
]
struct IFiltSupport : FwUnknown
{
    virtual void* __cdecl malloc(size_t nSize) = 0;
    virtual void __cdecl free(void* pMem) = 0;
    virtual int __cdecl memcmp(const void * ptr1, const void * ptr2, size_t num) = 0;
};

//////////////////////////////////////////////////////////////////////////

#define FILT_MODULE_EXPORT1         "KsGetFilter"
#define FILT_MODULE_EXPORT2         "KsSupport"
#define FILT_MODULE_EXPORT3			"KsScanBuffer"
typedef long (__cdecl *TKsGetFilter)(REFIID, void**);
typedef int (__stdcall *TKsScanBuffer)(unsigned long dwBufferSize,
				 void* pBuffer,
				 bool Outbound,
				 PCF_SCAN_RESULT pScanResult);

//////////////////////////////////////////////////////////////////////////


#endif  // !ACF_SUPPORT_INC_

