#pragma once
#include "comproxy/com_scan_interface.h"
#include "com_svc_dispatch.h"
#include "comproxy/com_scan_xdx.h"
#include "scanwork.h"
#include "svccallback.h"
#include "kbasicserviceprovider.h"

class CScanWorkProxy;

class CBkScanImp
    : public CBkComRoot,
      public IBkScan,
	  public IBKUnknownFileReport
{
public:
    CBkScanImp(void);
    ~CBkScanImp(void);
public:
    HRESULT Initialize( DWORD dwProcId, int nVersion );

    HRESULT Uninitialize();

    COM_METHOD_DISPATCH_BEGIN()
        COM_METHOD_DISPATCH_IN_ARG_3_NO_OUT( CBkScan, Scan, nScanMode, bBackGround, lpParam )
        COM_METHOD_DISPATCH_IN_ARG_3_NO_OUT( CBkScan, Scan2, nScanMode, bBackGround, lpParam )
        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkScan, Pause )
        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkScan, Resume )
        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkScan, Stop )
        COM_METHOD_DISPATCH_NO_ARG_OUT( CBkScan, QueryScanInfo, BK_SCAN_INFO )
        COM_METHOD_DISPATCH_NO_ARG_OUT( CBkScan, QueryCleanInfo, BK_CLEAN_INFO )
        COM_METHOD_DISPATCH_IN_ARG_2_OUT( CBkScan, QueryFileInfo, nType, dwIndex, BK_FILE_INFO )
        COM_METHOD_DISPATCH_IN_ARG_1_NO_OUT( CBkScan, Clean, fileIndex )
        COM_METHOD_DISPATCH_IN_ARG_1_OUT( CBkScan, QueryLastScanInfo, nScanMode, BK_SCAN_INFO )
        COM_METHOD_DISPATCH_IN_ARG_1_WITH_XDX_NO_OUT( CBkScan, SetScanSetting, BK_SCAN_SETTING )
        COM_METHOD_DISPATCH_NO_ARG_OUT( CBkScan, GetScanSetting, BK_SCAN_SETTING )
        COM_METHOD_DISPATCH_IN_ARG_1_OUT_WITH_XDX( CBkScan, QuerySteps, nScanMode, steps )
        COM_METHOD_DISPATCH_NO_ARG_NO_OUT( CBkScan, ReportFile )
    COM_METHOD_DISPATCH_END
public:
    virtual HRESULT Scan( int nScanMode, BOOL bBackGround, CAtlArray<CString>& lpParam );
    virtual HRESULT Scan2( int nScanMode, BOOL bBackGround, CAtlArray<BK_SCAN_ADDITIONAL_PATH>& lpParam );
    virtual HRESULT QuerySteps( int nScanMode, CAtlArray<int>& steps );
    virtual HRESULT Pause();
    virtual HRESULT Resume();
    virtual HRESULT Stop();
    virtual HRESULT QueryScanInfo( BK_SCAN_INFO& scanInfo );
    virtual HRESULT QueryCleanInfo( BK_CLEAN_INFO& cleanInfo );
    virtual HRESULT QueryFileInfo( int nType, DWORD dwIndex, BK_FILE_INFO& fileInfo );
    virtual HRESULT Clean( CAtlArray<DWORD>& fileIndex );
    virtual HRESULT QueryLastScanInfo( int nScanMode, BK_SCAN_INFO& scanInfo );
    virtual HRESULT SetScanSetting( BK_SCAN_SETTING& setting );
    virtual HRESULT GetScanSetting( BK_SCAN_SETTING& setting );
    virtual HRESULT ReportFile();

public:  // IBKUnknownFileReport
	/**
	* @brief        添加未知文件至上传列表
	* @param[in]    lpszFilePath    原始文件路径
	* @return       非0为失败
	*/
	virtual HRESULT STDMETHODCALLTYPE AddUnknownFile(
		LPCWSTR             lpszFilePath
		);

	/**
	* @brief        将文件上传至服务端
	* @return       非0为失败
	*/
	virtual HRESULT STDMETHODCALLTYPE Report();

	/**
	* @brief        是否为自动上传至服务端
	* @return       非0为失败
	*/
	virtual HRESULT STDMETHODCALLTYPE IsAutoReport(
		BOOL& bIsAutoReport
		);
private:
    HRESULT CheckInstance()
    {
        CObjGuard   guard( m_InsLock );

        return ( m_pScanIns ? S_OK : E_NOINTERFACE );
    }
private:
    typedef CComAutoCriticalSection                     CObjLock;
    typedef CComCritSecLock<CComAutoCriticalSection>    CObjGuard;

    CObjLock        m_InsLock;
    CScanWorkProxy* m_pScanIns;
    BK_SCAN_SETTING m_setting;
};


// HRESULT CBkScanImp::Call( int nMethod, BSTR bstrParams, BSTR* pbstrResponse )
// {
//     HRESULT hr = E_NOTIMPL;
//     BkXDX::CBKJsonCursor    curin;
//     BkXDX::CBKJsonCursor    curout;
// 
//     switch( nMethod )
//     {
//     case BKCOM_EXPORT_BkScan_METHOD_Scan:
//         {
//             BKCOM_PROXY_METHOD_XDX_STRUCT( Scan, 0 ) param;
//             curin.m_doc.LoadString( bstrParams );
//             curin.ReadData( param );
//             hr = Scan( param.nScanMode, param.lpParam );
//             break;
//         }
//     case BKCOM_EXPORT_BkScan_METHOD_Pause:
//         {
//             hr = Pause();
//             break;
//         }
//     case BKCOM_EXPORT_BkScan_METHOD_Resume:
//         {
//             hr = Resume();
//             break;
//         }
//     case BKCOM_EXPORT_BkScan_METHOD_Stop:
//         {
//             hr = Stop();
//             break;
//         }
//     case BKCOM_EXPORT_BkScan_METHOD_QueryScanInfo:
//         {
//             BK_SCAN_INFO    out;
//             hr = QueryScanInfo( out );
//             if ( SUCCEEDED( hr ) )
//             {
//                 curout.WriteData( out );
//                 try
//                 {
//                     *pbstrResponse = curout.m_doc.Dump().AllocSysString();
//                 }
//                 catch ( ... )
//                 {
//                     *pbstrResponse = NULL;
//                     hr = E_OUTOFMEMORY;
//                 }
//             }
//             break;
//         }
//     case BKCOM_EXPORT_BkScan_METHOD_QueryCleanInfo:
//         {
//             BK_CLEAN_INFO   out;
//             hr = QueryCleanInfo( out );
//             if ( SUCCEEDED( hr ) )
//             {
//                 curout.WriteData( out );
//                 try
//                 {
//                     *pbstrResponse = curout.m_doc.Dump().AllocSysString();
//                 }
//                 catch ( ... )
//                 {
//                     *pbstrResponse = NULL;
//                     hr = E_OUTOFMEMORY;
//                 }
//             }
//             break;
//         }
//     case BKCOM_EXPORT_BkScan_METHOD_QueryFileInfo:
//         {
//             BK_FILE_INFO    out;
//             BKCOM_PROXY_METHOD_XDX_STRUCT( QueryFileInfo, 0 ) param;
//             curin.m_doc.LoadString( bstrParams );
//             curin.ReadData( param );
//             hr = QueryFileInfo( param.nType, param.dwIndex, out );
//             if ( SUCCEEDED( hr ) )
//             {
//                 curout.WriteData( out );
//                 try
//                 {
//                     *pbstrResponse = curout.m_doc.Dump().AllocSysString();
//                 }
//                 catch ( ... )
//                 {
//                     *pbstrResponse = NULL;
//                     hr = E_OUTOFMEMORY;
//                 }
//             }
//             break;
//         }
//     case BKCOM_EXPORT_BkScan_METHOD_Clean:
//         {
//             BKCOM_PROXY_METHOD_XDX_STRUCT( Clean, 0 ) param;
//             curin.m_doc.LoadString( bstrParams );
//             curin.ReadData( param );
//             Clean( param.fileIndex );
//             break;
//         }
//     case BKCOM_EXPORT_BkScan_METHOD_QueryLastScanInfo:
//         {
//             BK_SCAN_INFO    out;
//             hr = QueryLastScanInfo( out );
//             if ( SUCCEEDED( hr ) )
//             {
//                 curout.WriteData( out );
//                 try
//                 {
//                     *pbstrResponse = curout.m_doc.Dump().AllocSysString();
//                 }
//                 catch ( ... )
//                 {
//                     *pbstrResponse = NULL;
//                     hr = E_OUTOFMEMORY;
//                 }
//             }
//             break;
//         }
//     case BKCOM_EXPORT_BkScan_METHOD_SetScanSetting:
//         {
//             BK_SCAN_SETTING param;
//             curin.m_doc.LoadString( bstrParams );
//             curin.ReadData( param );
//             hr = SetScanSetting( param );
//             break;
//         }
//     case BKCOM_EXPORT_BkScan_METHOD_GetScanSetting:
//         {
//             BK_SCAN_SETTING out;
//             hr = GetScanSetting( out );
//             if ( SUCCEEDED( hr ) )
//             {
//                 curout.WriteData( out );
//                 try
//                 {
//                     *pbstrResponse = curout.m_doc.Dump().AllocSysString();
//                 }
//                 catch ( ... )
//                 {
//                     *pbstrResponse = NULL;
//                     hr = E_OUTOFMEMORY;
//                 }
//             }
//             break;
//         }
//     default:
//         {
//             break;
//         }
//     }
// Exit0:
//     return hr;
// }