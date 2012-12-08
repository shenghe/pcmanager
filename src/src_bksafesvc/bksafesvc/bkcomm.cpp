// bkcomm.cpp : Implementation of Cbkcomm

#include "stdafx.h"
#include "bkcomm.h"
//#include "bkscanimp.h"
//#include "bkscansinglefileimp.h"
#include "bkservicecallImp.h"
//#include "bkrescanimp.h"
//#include "bkutilityimp.h"
//#include "bkechoimp.h"
//#include "bkscanmultifileimp.h"
#include "comproxy/export_module_def.h"

#define BEGIN_COM_IMPL_INTITIALIZE() \
    switch ( m_nModuleId )\
    {
#define COM_IMPL_INITIALIZE(module, implclass)\
        case BKCOM_EXPORT_MODULE_##module:\
        {\
            implclass* pi = new implclass();\
            if ( pi )\
            {\
                hr = pi->Initialize( m_dwProcId, nVersion );\
                if ( SUCCEEDED( hr ) )\
                {\
                    assert(!m_piRoot);\
                    m_piRoot = static_cast<CBkComRoot*>(pi);\
                }\
                else\
                {\
                    delete pi;\
                }\
            }\
            else\
            {\
                hr = E_OUTOFMEMORY;\
            }\
            break;\
        }
#define END_COM_IMPL_INITIALIZE()    \
        default:\
        {\
            hr = E_NOTIMPL;\
            break;\
        }\
    }

#define BEGIN_COM_IMPL_UNINIT() \
    switch ( m_nModuleId )\
    {
#define COM_IMPL_UNINIT(module, implclass)\
        case BKCOM_EXPORT_MODULE_##module:\
        {\
            implclass* pi = static_cast<implclass*>(m_piRoot);\
            pi->Uninitialize();\
            delete pi;\
            m_piRoot = NULL;\
            break;\
        }
#define END_COM_IMPL_UNINIT()    \
        default:\
        {\
            break;\
        }\
    }

#define BEGIN_COM_IMPL_CALL() \
    switch ( m_nModuleId )\
    {
#define COM_IMPL_CALL(module, implclass)\
        case BKCOM_EXPORT_MODULE_##module:\
        {\
            implclass* pi = static_cast<implclass*>(m_piRoot);\
            hr = pi->Call( nMethod, bstrParams, pbstrResponse );\
            break;\
        }
#define END_COM_IMPL_CALL()    \
        default:\
        {\
            hr = E_NOTIMPL;\
            break;\
        }\
    }
// Cbkcomm
STDMETHODIMP Cbkcomm::Initialize(int nModule, ULONG ulProcId, int nVersion)
{
    HRESULT hr = S_OK;
    m_dwProcId = ( DWORD )ulProcId;
    m_nModuleId = nModule;

    BEGIN_COM_IMPL_INTITIALIZE()
//         COM_IMPL_INITIALIZE( CBkScan, CBkScanImp )
//         COM_IMPL_INITIALIZE( CBkScanSingleFile, CBkScanSingleFileImp )
        COM_IMPL_INITIALIZE( CBkServiceCall, CBkServiceCallImp )
//         COM_IMPL_INITIALIZE( CBkRescan, CBkRescanImp )
//         COM_IMPL_INITIALIZE( CBkUtility, CBkUtilityImp )
//         COM_IMPL_INITIALIZE( CBkEcho, CBkEchoImp )
//         COM_IMPL_INITIALIZE( CBkPluginReport, CBkPluginReportImp )
//         COM_IMPL_INITIALIZE( CBkScanMultiFile, CBkScanMultiFileImp )
    END_COM_IMPL_INITIALIZE()
    //switch ( m_nModuleId )
    //{
    //case BKCOM_EXPORT_MODULE_BkScan:
    //    {
    //        CBkScanImp* pi = new CBkScanImp();
    //        if ( pi )
    //        {
    //            hr = pi->Initialize( m_dwProcId );
    //            if ( SUCCEEDED( hr ) )
    //            {
    //                m_piRoot = static_cast<CBkComRoot*>(pi);
    //            }
    //        }
    //        else
    //        {
    //            hr = E_OUTOFMEMORY;
    //        }
    //        break;
    //    }
    //default:
    //    {
    //        hr = E_NOTIMPL;
    //        break;
    //    }
    //}

    return hr;
}

STDMETHODIMP Cbkcomm::Call(int nMethod, BSTR bstrParams, BSTR* pbstrResponse)
{
    HRESULT hr = E_NOINTERFACE;

    if ( NULL == m_piRoot )
    {
        goto Exit0;
    }

    BEGIN_COM_IMPL_CALL()
//         COM_IMPL_CALL( CBkScan, CBkScanImp )
//         COM_IMPL_CALL( CBkScanSingleFile, CBkScanSingleFileImp )
        COM_IMPL_CALL( CBkServiceCall, CBkServiceCallImp )
//         COM_IMPL_CALL( CBkRescan, CBkRescanImp )
//         COM_IMPL_CALL( CBkUtility, CBkUtilityImp )
//         COM_IMPL_CALL( CBkEcho, CBkEchoImp )
//         COM_IMPL_CALL( CBkPluginReport, CBkPluginReportImp )
//         COM_IMPL_CALL( CBkScanMultiFile, CBkScanMultiFileImp )
    END_COM_IMPL_CALL()
    //switch( m_nModuleId )
    //{
    //case BKCOM_EXPORT_MODULE_BkScan:
    //    {
    //        CBkScanImp* pi = static_cast<CBkScanImp*>(m_piRoot);

    //        hr = pi->Call( nMethod, bstrParams, pbstrResponse );
    //        break;
    //    }
    //default:
    //    {
    //        hr = E_NOTIMPL;
    //        break;
    //    }
    //}
Exit0:
    return hr;
}

STDMETHODIMP Cbkcomm::Uninitialize(void)
{
    if ( m_piRoot )
    {
         BEGIN_COM_IMPL_UNINIT()
//             COM_IMPL_UNINIT( CBkScan, CBkScanImp )
//             COM_IMPL_UNINIT( CBkScanSingleFile, CBkScanSingleFileImp )
             COM_IMPL_UNINIT( CBkServiceCall, CBkServiceCallImp )
//             COM_IMPL_UNINIT( CBkRescan, CBkRescanImp )
//             COM_IMPL_UNINIT( CBkUtility, CBkUtilityImp )
//             COM_IMPL_UNINIT( CBkEcho, CBkEchoImp )
//             COM_IMPL_UNINIT( CBkPluginReport, CBkPluginReportImp )
//             COM_IMPL_UNINIT( CBkScanMultiFile, CBkScanMultiFileImp )
         END_COM_IMPL_UNINIT()
        //switch( m_nModuleId )
        //{
        //case BKCOM_EXPORT_MODULE_BkScan:
        //    {
        //        CBkScanImp* pi = static_cast<CBkScanImp*>(m_piRoot);

        //        pi->Uninitialize();
        //        delete pi;
        //        m_piRoot = NULL;
        //        break;
        //    }
        //default:
        //    {
        //        break;
        //    }
        //}
    }

    return S_OK;
}

HRESULT Cbkcomm::OnFinalConstruct()
{
    return S_OK;
}

void Cbkcomm::OnFinalRelease()
{
    if ( m_piRoot )
    {
         BEGIN_COM_IMPL_UNINIT()
//             COM_IMPL_UNINIT( CBkScan, CBkScanImp )
//             COM_IMPL_UNINIT( CBkScanSingleFile, CBkScanSingleFileImp )
             COM_IMPL_UNINIT( CBkServiceCall, CBkServiceCallImp )
//             COM_IMPL_UNINIT( CBkRescan, CBkRescanImp )
//             COM_IMPL_UNINIT( CBkUtility, CBkUtilityImp )
//             COM_IMPL_UNINIT( CBkEcho, CBkEchoImp )
//             COM_IMPL_UNINIT( CBkPluginReport, CBkPluginReportImp )
//             COM_IMPL_UNINIT( CBkScanMultiFile, CBkScanMultiFileImp )
         END_COM_IMPL_UNINIT()
    }
}