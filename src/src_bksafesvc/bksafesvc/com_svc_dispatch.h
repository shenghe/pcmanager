#pragma once

class   CBkComRoot
{
public:
    CBkComRoot(){}
    ~CBkComRoot(){}
public:
    HRESULT Initialize( DWORD dwProcId, int nVersion )
    {
        HRESULT hr = S_OK;

        m_dwProcId = dwProcId;

        return hr;
    }

    HRESULT Uninitialize()
    {
        return S_OK;
    }

    HRESULT Call( int nMethod, BSTR bstrParams, BSTR* pbstrResponse )
    {
        return S_OK;
    }
protected:
    DWORD   m_dwProcId;
};

#define COM_METHOD_DISPATCH_BEGIN()  \
HRESULT Call( int nMethod, BSTR bstrParams, BSTR* pbstrResponse )\
{\
    HRESULT hr = E_NOTIMPL;\
    BkXDX::CBKJsonCursor    curin;\
    BkXDX::CBKJsonCursor    curout;\
    switch( nMethod )\
    {    
#define COM_METHOD_DISPATCH_NO_ARG_NO_OUT( module, method ) \
        case BKCOM_EXPORT_##module##_METHOD_##method:\
        {\
            hr = method();\
            break;\
        }
#define COM_METHOD_DISPATCH_NO_ARG_OUT( module, method, outtype ) \
        case BKCOM_EXPORT_##module##_METHOD_##method:\
        {\
            outtype    out;\
            hr = method( out );\
            if ( SUCCEEDED( hr ) )\
            {\
                curout.WriteData( out );\
                try\
                {\
                    *pbstrResponse = curout.m_doc.Dump().AllocSysString();\
                }\
                catch ( ... )\
                {\
                    *pbstrResponse = NULL;\
                    hr = E_OUTOFMEMORY;\
                }\
            }\
            break;\
        }
#define COM_METHOD_DISPATCH_NO_ARG_OUT_WITH_XDX( module, method, argo ) \
        case BKCOM_EXPORT_##module##_METHOD_##method:\
        {\
            BKCOM_PROXY_METHOD_XDX_STRUCT(method, ##module##Out)    out;\
            hr = method( out.##argo );\
            if ( SUCCEEDED( hr ) )\
            {\
                curout.WriteData( out );\
                try\
                {\
                    *pbstrResponse = curout.m_doc.Dump().AllocSysString();\
                }\
                catch ( ... )\
                {\
                    *pbstrResponse = NULL;\
                    hr = E_OUTOFMEMORY;\
                }\
            }\
            break;\
        }
#define COM_METHOD_DISPATCH_IN_ARG_1_NO_OUT( module, method, arg1 ) \
        case BKCOM_EXPORT_##module##_METHOD_##method:\
        {\
            BKCOM_PROXY_METHOD_XDX_STRUCT( method, module ) param;\
            curin.m_doc.LoadString( bstrParams );\
            curin.ReadData( param );\
            hr = method( param.arg1 );\
            break;\
        }
#define COM_METHOD_DISPATCH_IN_ARG_1_OUT(module, method, arg1, outtype)\
        case BKCOM_EXPORT_##module##_METHOD_##method:\
        {\
            outtype    out;\
            BKCOM_PROXY_METHOD_XDX_STRUCT( method, module ) param;\
            curin.m_doc.LoadString( bstrParams );\
            curin.ReadData( param );\
            hr = method( param.##arg1, out );\
            if ( SUCCEEDED( hr ) )\
            {\
                curout.WriteData( out );\
                try\
                {\
                    *pbstrResponse = curout.m_doc.Dump().AllocSysString();\
                }\
                catch ( ... )\
                {\
                    *pbstrResponse = NULL;\
                    hr = E_OUTOFMEMORY;\
                }\
            }\
            break;\
        }
#define COM_METHOD_DISPATCH_IN_ARG_1_OUT_WITH_XDX(module, method, arg1, argo)\
        case BKCOM_EXPORT_##module##_METHOD_##method:\
        {\
            BKCOM_PROXY_METHOD_XDX_STRUCT(method, ##module##Out)    out;\
            BKCOM_PROXY_METHOD_XDX_STRUCT( method, module ) param;\
            curin.m_doc.LoadString( bstrParams );\
            curin.ReadData( param );\
            hr = method( param.##arg1, out.##argo );\
            if ( SUCCEEDED( hr ) )\
            {\
                curout.WriteData( out );\
                try\
                {\
                    *pbstrResponse = curout.m_doc.Dump().AllocSysString();\
                }\
                catch ( ... )\
                {\
                    *pbstrResponse = NULL;\
                    hr = E_OUTOFMEMORY;\
                }\
            }\
            break;\
        }
#define COM_METHOD_DISPATCH_IN_ARG_1_WITH_XDX_NO_OUT( module, method, arg1type ) \
        case BKCOM_EXPORT_##module##_METHOD_##method:\
        {\
            arg1type  param;\
            curin.m_doc.LoadString( bstrParams );\
            curin.ReadData( param );\
            hr = method( param );\
            break;\
        }
#define COM_METHOD_DISPATCH_IN_ARG_2_NO_OUT( module, method, arg1, arg2 )  \
        case BKCOM_EXPORT_##module##_METHOD_##method:\
        {\
            BKCOM_PROXY_METHOD_XDX_STRUCT( method, module ) param;\
            curin.m_doc.LoadString( bstrParams );\
            curin.ReadData( param );\
            hr = method( param.##arg1, param.##arg2 );\
            break;\
        }
#define COM_METHOD_DISPATCH_IN_ARG_2_OUT( module, method, arg1, arg2, outtype )  \
        case BKCOM_EXPORT_##module##_METHOD_##method:\
        {\
            outtype    out;\
            BKCOM_PROXY_METHOD_XDX_STRUCT( method, module ) param;\
            curin.m_doc.LoadString( bstrParams );\
            curin.ReadData( param );\
            hr = method( param.##arg1, param.##arg2, out );\
            if ( SUCCEEDED( hr ) )\
            {\
                curout.WriteData( out );\
                try\
                {\
                    *pbstrResponse = curout.m_doc.Dump().AllocSysString();\
                }\
                catch ( ... )\
                {\
                    *pbstrResponse = NULL;\
                    hr = E_OUTOFMEMORY;\
                }\
            }\
            break;\
        }
#define COM_METHOD_DISPATCH_IN_ARG_3_OUT( module, method, arg1, arg2, arg3, outtype )  \
        case BKCOM_EXPORT_##module##_METHOD_##method:\
        {\
        outtype    out;\
        BKCOM_PROXY_METHOD_XDX_STRUCT( method, module ) param;\
        curin.m_doc.LoadString( bstrParams );\
        curin.ReadData( param );\
        hr = method( param.##arg1, param.##arg2, param.##arg3, out );\
        if ( SUCCEEDED( hr ) )\
            {\
            curout.WriteData( out );\
            try\
                {\
                *pbstrResponse = curout.m_doc.Dump().AllocSysString();\
                }\
                catch ( ... )\
                {\
                *pbstrResponse = NULL;\
                hr = E_OUTOFMEMORY;\
                }\
            }\
            break;\
        }
#define COM_METHOD_DISPATCH_IN_ARG_2_OUT_WITH_XDX( module, method, arg1, arg2, argo )  \
        case BKCOM_EXPORT_##module##_METHOD_##method:\
        {\
        BKCOM_PROXY_METHOD_XDX_STRUCT(method, ##module##Out)    out;\
        BKCOM_PROXY_METHOD_XDX_STRUCT( method, module ) param;\
            curin.m_doc.LoadString( bstrParams );\
            curin.ReadData( param );\
            hr = method( param.##arg1, param.##arg2, out.##argo );\
            if ( SUCCEEDED( hr ) )\
            {\
                curout.WriteData( out );\
                try\
                    {\
                    *pbstrResponse = curout.m_doc.Dump().AllocSysString();\
                    }\
                    catch ( ... )\
                    {\
                    *pbstrResponse = NULL;\
                    hr = E_OUTOFMEMORY;\
                    }\
            }\
            break;\
        }
#define COM_METHOD_DISPATCH_IN_ARG_3_NO_OUT( module, method, arg1, arg2, arg3 )  \
        case BKCOM_EXPORT_##module##_METHOD_##method:\
        {\
            BKCOM_PROXY_METHOD_XDX_STRUCT( method, module ) param;\
            curin.m_doc.LoadString( bstrParams );\
            curin.ReadData( param );\
            hr = method( param.##arg1, param.##arg2, param.##arg3 );\
            break;\
        }
#define COM_METHOD_DISPATCH_END   \
        default:\
        {\
            break;\
        }\
    }\
    return hr;\
}