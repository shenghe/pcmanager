

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Wed Mar 16 17:07:26 2011
 */
/* Compiler settings for ..\..\publish\idl\bksafesvc.idl:
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

#ifndef __bksafesvc_h__
#define __bksafesvc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __Ibkcomm_FWD_DEFINED__
#define __Ibkcomm_FWD_DEFINED__
typedef interface Ibkcomm Ibkcomm;
#endif 	/* __Ibkcomm_FWD_DEFINED__ */


#ifndef __bkcomm_FWD_DEFINED__
#define __bkcomm_FWD_DEFINED__

#ifdef __cplusplus
typedef class bkcomm bkcomm;
#else
typedef struct bkcomm bkcomm;
#endif /* __cplusplus */

#endif 	/* __bkcomm_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __Ibkcomm_INTERFACE_DEFINED__
#define __Ibkcomm_INTERFACE_DEFINED__

/* interface Ibkcomm */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_Ibkcomm;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("25407239-B26F-40DF-97FF-1652E76BF999")
    Ibkcomm : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ int nModule,
            /* [in] */ ULONG ulProcId,
            /* [in] */ int nVersion) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Call( 
            /* [in] */ int nMethod,
            /* [in] */ BSTR bstrParams,
            /* [retval][out] */ BSTR *pbstrResponse) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Uninitialize( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IbkcommVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Ibkcomm * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Ibkcomm * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Ibkcomm * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Ibkcomm * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Ibkcomm * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Ibkcomm * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Ibkcomm * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            Ibkcomm * This,
            /* [in] */ int nModule,
            /* [in] */ ULONG ulProcId,
            /* [in] */ int nVersion);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Call )( 
            Ibkcomm * This,
            /* [in] */ int nMethod,
            /* [in] */ BSTR bstrParams,
            /* [retval][out] */ BSTR *pbstrResponse);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Uninitialize )( 
            Ibkcomm * This);
        
        END_INTERFACE
    } IbkcommVtbl;

    interface Ibkcomm
    {
        CONST_VTBL struct IbkcommVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Ibkcomm_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define Ibkcomm_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define Ibkcomm_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define Ibkcomm_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define Ibkcomm_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define Ibkcomm_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define Ibkcomm_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define Ibkcomm_Initialize(This,nModule,ulProcId,nVersion)	\
    ( (This)->lpVtbl -> Initialize(This,nModule,ulProcId,nVersion) ) 

#define Ibkcomm_Call(This,nMethod,bstrParams,pbstrResponse)	\
    ( (This)->lpVtbl -> Call(This,nMethod,bstrParams,pbstrResponse) ) 

#define Ibkcomm_Uninitialize(This)	\
    ( (This)->lpVtbl -> Uninitialize(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __Ibkcomm_INTERFACE_DEFINED__ */



#ifndef __bksafesvcLib_LIBRARY_DEFINED__
#define __bksafesvcLib_LIBRARY_DEFINED__

/* library bksafesvcLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_bksafesvcLib;

EXTERN_C const CLSID CLSID_bkcomm;

#ifdef __cplusplus

class DECLSPEC_UUID("C313E554-97AB-49F9-988F-04DF64CD0451")
bkcomm;
#endif
#endif /* __bksafesvcLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


