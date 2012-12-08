

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Thu Aug 16 15:55:58 2007
 */
/* Compiler settings for .\temp\KAESign.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __KAESign_h__
#define __KAESign_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IKAESign_FWD_DEFINED__
#define __IKAESign_FWD_DEFINED__
typedef interface IKAESign IKAESign;
#endif 	/* __IKAESign_FWD_DEFINED__ */


#ifndef __KAESign_FWD_DEFINED__
#define __KAESign_FWD_DEFINED__

#ifdef __cplusplus
typedef class KAESign KAESign;
#else
typedef struct KAESign KAESign;
#endif /* __cplusplus */

#endif 	/* __KAESign_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_KAESign_0000 */
/* [local] */ 

const char g_cszKAECopyRight[] = "Kingsoft AntiVirus Database.   Copyright (c) 1999, 2001 Kingsoft AntiVirus.\x1A";

typedef struct tagKAE_KAVSIGNDATA_ITEM
    {
    unsigned int uPos;
    unsigned int uLen;
    } 	KAE_KAVSIGNDATA_ITEM;

#define defMAXSIGNDATAITEM           0x20
#define defMZSCANBUFDATAITEM         0
#define defMZSCANBUFVIRNAMEITEM      1
#define defMACROSCRIPTDATAITEM       2
#define defMACROSCRIPTVIRNAMEITEM    3
#define defVBSSCRIPTDATAITEM         4
#define defVBSSCRIPTVIRNAMEITEM      5
#define defPESCRIPTDATAITEM          6
#define defPESCRIPTVIRNAMEITEM       7
#define defMZCLEANDATAITEM           8
#define defMZCLEANMEMIMAGE           9
#define defDRWEBVIRUSBASEITEM        10
#define defCOMSCANBUF2DATAITEM          11
#define defCOMSCANPOLYBUF2DATAITEM      12
#define defMZSCANBUF2DATAITEM           13
#define defMZSCANPOLYBUF2DATAITEM       14
#define defPESCANBUF2DATAITEM           15
#define defPESCANPOLYBUF2DATAITEM       16
#define defMACROSCANBUF2DATAITEM        17
#define defMACROSCANPOLYBUF2DATAITEM    18
#define defTEXTSCANBUF2DATAITEM       19
#define defTEXTSCANPOLYBUF2DATAITEM   20
#define defPETROJANSCANBUF2DATAITEM           21
#define defPETROJANSCANPOLYBUF2DATAITEM       22

#define defDAILYSCANPOLYBUF2DATAITEM    23
#define defBASESCANBUF2DATAITEM			24
#define defBASESCANPOLYBUF2DATAITEM		25
#define defRISKSCANBUF2DATAITEM			26
#define defRISKSCANPOLYBUF2DATAITEM		27
#define defFALSESCANBUF2DATAITEM		28
#define defFALSESCANPOLYBUF2DATAITEM	29
#define defMONTHSCANPOLYBUF2DATAITEM	30
#define defUNUSE						31
#define defNUMOFSIGNDATA                32

#define defKVSIGN_PACK_NORAMAL              0
#define defKVSIGN_PACK_APLIB                1
#define defKVSIGN_PACK_NRV2B                2
#define defKVSIGN_PACK_NRV2D                3
#define defKVSIGN_PACK_ZLIB                 4
#define defKVSIGN_ENCRYPT_NORMAL            0
#define defKVSIGN_ENCRYPT_RAND              1
typedef struct tagKAE_KVSIGN_HEADER
    {
    unsigned char szCopyRight[ 78 ];
    unsigned short wDateBuildNum;
    unsigned int uVersion;
    unsigned int uDateStamp;
    unsigned int uProcessVirusNum;
    unsigned short wPackMethod;
    unsigned short wEncryptMethod;
    unsigned int uCheckSum;
    unsigned int uDataAreaEntry;
    unsigned int uDataAreaLen;
    unsigned int uSignDataItemCount;
    KAE_KAVSIGNDATA_ITEM SignDataItem[ 32 ];
    } 	KAE_KVSIGN_HEADER;

typedef struct tagKAE_SIGN_VERSION
    {
    unsigned int uYear;
    unsigned int uMonth;
    unsigned int uDay;
    unsigned int uBuildNum;
    } 	KAE_SIGN_VERSION;



extern RPC_IF_HANDLE __MIDL_itf_KAESign_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_KAESign_0000_v0_0_s_ifspec;

#ifndef __IKAESign_INTERFACE_DEFINED__
#define __IKAESign_INTERFACE_DEFINED__

/* interface IKAESign */
/* [helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IKAESign;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("65CD0BBB-6FF0-4ad8-892D-C32E640CA6AD")
    IKAESign : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitSignBuffer( 
            /* [in] */ int nBufSize,
            /* [size_is][in] */ const byte *pcbyBuf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddSignBuffer( 
            /* [in] */ int nBufSize,
            /* [size_is][in] */ const byte *pcbyBuf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnInitSignBuffer( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitSignFileName( 
            /* [in] */ int nFileNameSize,
            /* [size_is][in] */ const unsigned char *pcszFileName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddSignFileName( 
            /* [in] */ int nFileNameSize,
            /* [size_is][in] */ const unsigned char *pcszFileName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnInitSignFileName( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSignVersion( 
            /* [out] */ int *pnSignVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVirusCount( 
            /* [out] */ int *pnVirusCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSignVersion2( 
            /* [out] */ KAE_SIGN_VERSION *SignVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitSignFileNameW( 
            /* [in] */ int nFileNameSize,
            /* [size_is][in] */ const WCHAR *pcwszFileName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddSignFileNameW( 
            /* [in] */ int nFileNameSize,
            /* [size_is][in] */ const WCHAR *pcwszFileName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IKAESignVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IKAESign * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IKAESign * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IKAESign * This);
        
        HRESULT ( STDMETHODCALLTYPE *InitSignBuffer )( 
            IKAESign * This,
            /* [in] */ int nBufSize,
            /* [size_is][in] */ const byte *pcbyBuf);
        
        HRESULT ( STDMETHODCALLTYPE *AddSignBuffer )( 
            IKAESign * This,
            /* [in] */ int nBufSize,
            /* [size_is][in] */ const byte *pcbyBuf);
        
        HRESULT ( STDMETHODCALLTYPE *UnInitSignBuffer )( 
            IKAESign * This);
        
        HRESULT ( STDMETHODCALLTYPE *InitSignFileName )( 
            IKAESign * This,
            /* [in] */ int nFileNameSize,
            /* [size_is][in] */ const unsigned char *pcszFileName);
        
        HRESULT ( STDMETHODCALLTYPE *AddSignFileName )( 
            IKAESign * This,
            /* [in] */ int nFileNameSize,
            /* [size_is][in] */ const unsigned char *pcszFileName);
        
        HRESULT ( STDMETHODCALLTYPE *UnInitSignFileName )( 
            IKAESign * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSignVersion )( 
            IKAESign * This,
            /* [out] */ int *pnSignVersion);
        
        HRESULT ( STDMETHODCALLTYPE *GetVirusCount )( 
            IKAESign * This,
            /* [out] */ int *pnVirusCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetSignVersion2 )( 
            IKAESign * This,
            /* [out] */ KAE_SIGN_VERSION *SignVersion);
        
        HRESULT ( STDMETHODCALLTYPE *InitSignFileNameW )( 
            IKAESign * This,
            /* [in] */ int nFileNameSize,
            /* [size_is][in] */ const WCHAR *pcwszFileName);
        
        HRESULT ( STDMETHODCALLTYPE *AddSignFileNameW )( 
            IKAESign * This,
            /* [in] */ int nFileNameSize,
            /* [size_is][in] */ const WCHAR *pcwszFileName);
        
        END_INTERFACE
    } IKAESignVtbl;

    interface IKAESign
    {
        CONST_VTBL struct IKAESignVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IKAESign_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IKAESign_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IKAESign_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IKAESign_InitSignBuffer(This,nBufSize,pcbyBuf)	\
    (This)->lpVtbl -> InitSignBuffer(This,nBufSize,pcbyBuf)

#define IKAESign_AddSignBuffer(This,nBufSize,pcbyBuf)	\
    (This)->lpVtbl -> AddSignBuffer(This,nBufSize,pcbyBuf)

#define IKAESign_UnInitSignBuffer(This)	\
    (This)->lpVtbl -> UnInitSignBuffer(This)

#define IKAESign_InitSignFileName(This,nFileNameSize,pcszFileName)	\
    (This)->lpVtbl -> InitSignFileName(This,nFileNameSize,pcszFileName)

#define IKAESign_AddSignFileName(This,nFileNameSize,pcszFileName)	\
    (This)->lpVtbl -> AddSignFileName(This,nFileNameSize,pcszFileName)

#define IKAESign_UnInitSignFileName(This)	\
    (This)->lpVtbl -> UnInitSignFileName(This)

#define IKAESign_GetSignVersion(This,pnSignVersion)	\
    (This)->lpVtbl -> GetSignVersion(This,pnSignVersion)

#define IKAESign_GetVirusCount(This,pnVirusCount)	\
    (This)->lpVtbl -> GetVirusCount(This,pnVirusCount)

#define IKAESign_GetSignVersion2(This,SignVersion)	\
    (This)->lpVtbl -> GetSignVersion2(This,SignVersion)

#define IKAESign_InitSignFileNameW(This,nFileNameSize,pcwszFileName)	\
    (This)->lpVtbl -> InitSignFileNameW(This,nFileNameSize,pcwszFileName)

#define IKAESign_AddSignFileNameW(This,nFileNameSize,pcwszFileName)	\
    (This)->lpVtbl -> AddSignFileNameW(This,nFileNameSize,pcwszFileName)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IKAESign_InitSignBuffer_Proxy( 
    IKAESign * This,
    /* [in] */ int nBufSize,
    /* [size_is][in] */ const byte *pcbyBuf);


void __RPC_STUB IKAESign_InitSignBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IKAESign_AddSignBuffer_Proxy( 
    IKAESign * This,
    /* [in] */ int nBufSize,
    /* [size_is][in] */ const byte *pcbyBuf);


void __RPC_STUB IKAESign_AddSignBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IKAESign_UnInitSignBuffer_Proxy( 
    IKAESign * This);


void __RPC_STUB IKAESign_UnInitSignBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IKAESign_InitSignFileName_Proxy( 
    IKAESign * This,
    /* [in] */ int nFileNameSize,
    /* [size_is][in] */ const unsigned char *pcszFileName);


void __RPC_STUB IKAESign_InitSignFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IKAESign_AddSignFileName_Proxy( 
    IKAESign * This,
    /* [in] */ int nFileNameSize,
    /* [size_is][in] */ const unsigned char *pcszFileName);


void __RPC_STUB IKAESign_AddSignFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IKAESign_UnInitSignFileName_Proxy( 
    IKAESign * This);


void __RPC_STUB IKAESign_UnInitSignFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IKAESign_GetSignVersion_Proxy( 
    IKAESign * This,
    /* [out] */ int *pnSignVersion);


void __RPC_STUB IKAESign_GetSignVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IKAESign_GetVirusCount_Proxy( 
    IKAESign * This,
    /* [out] */ int *pnVirusCount);


void __RPC_STUB IKAESign_GetVirusCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IKAESign_GetSignVersion2_Proxy( 
    IKAESign * This,
    /* [out] */ KAE_SIGN_VERSION *SignVersion);


void __RPC_STUB IKAESign_GetSignVersion2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IKAESign_InitSignFileNameW_Proxy( 
    IKAESign * This,
    /* [in] */ int nFileNameSize,
    /* [size_is][in] */ const WCHAR *pcwszFileName);


void __RPC_STUB IKAESign_InitSignFileNameW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IKAESign_AddSignFileNameW_Proxy( 
    IKAESign * This,
    /* [in] */ int nFileNameSize,
    /* [size_is][in] */ const WCHAR *pcwszFileName);


void __RPC_STUB IKAESign_AddSignFileNameW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IKAESign_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_KAESign_0010 */
/* [local] */ 

EXTERN_C const CLSID CLSID_KAESign;


extern RPC_IF_HANDLE __MIDL_itf_KAESign_0010_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_KAESign_0010_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


