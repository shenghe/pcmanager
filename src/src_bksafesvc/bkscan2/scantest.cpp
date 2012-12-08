#include "stdafx.h"
#include "scantest.h"



class CSvcCB
    : public IScanFileCallBack
{
    virtual HRESULT AfterScanFile(        
        /* [in ] */ LPCWSTR             lpszFilePath,
        /* [in ] */ const Skylark::BKENG_RESULT* pResult,
        /* [in ] */ void*               pvFlag )
    {
        return S_OK;
    }
};
CSvcCB g_cb;

void TestRemoveDisk()
{
    WCHAR szRootPath[] = L"C:\\";
    for ( int i = 0; i < 24; ++i, ++szRootPath[0] )
    {
        UINT uDriveType = ::GetDriveType( szRootPath );
        if ( (DRIVE_CDROM == uDriveType)
            || (DRIVE_UNKNOWN == uDriveType)
            || (DRIVE_NO_ROOT_DIR == uDriveType) )
        {
            continue;
        }
    }
}


void TestFileScan()
{
    HRESULT hr = E_FAIL;

    CBkFileScan FileScan;


    BK_FILE_RESULT fileResult;
    hr = FileScan.Initialize( 1 );
    if ( FAILED(hr) )
    {
        return ;
    }

    BK_SCAN_SETTING seting;
    seting.Reset();
    seting.bAutoClean = TRUE;
    FileScan.SetScanSetting( seting );

    CString namepath = L"E:\\virus\\新建文件夹\\53.ex_";
    hr = FileScan.Scan( namepath,fileResult );
    if ( FAILED(hr) )
    {
        return ;
    }

    //CString namepath2 = L"E:\\virus\\新建文件夹\\53.dl_";
    //hr = FileScan.Scan( namepath2,fileResult, FALSE );
    //if ( FAILED(hr) )
    //{
    //    return ;
    //}

    FileScan.Uninitialize();

}

static unsigned int __stdcall MutifaileScanThread( void * lpParameter )
{
     CBkMultiFileScan * pThis = (CBkMultiFileScan*)lpParameter;
     Sleep( 500 );
     pThis->Stop();
     return 0;
}

void TestMutiFileScan()
{
    HRESULT hr = E_FAIL;

    CBkMultiFileScan MutiFileScan;

    CString namepath1 = L"E:\\virus\\新建文件夹\\50.dl_";
    CString namepath2 = L"E:\\virus\\新建文件夹\\52.dl_";
    CString namepath3 = L"E:\\virus\\新建文件夹\\53.dl_";
   
    CAtlArray<CString> FilePathArray;
    FilePathArray.Add( namepath1 );
    FilePathArray.Add( namepath2 );
    FilePathArray.Add( namepath3 );
   
    BK_SCAN_SETTING set;
    set.Reset();
    set.nScanPolicy = SCAN_POLICY_CLOUD;
    MutiFileScan.SetScanSetting( set );
    hr = MutiFileScan.Initialize();
    if ( FAILED(hr) )
    {
        return ;
    }
    //::_beginthreadex( NULL, 0, MutifaileScanThread, &MutiFileScan, 0, NULL );
    CAtlArray<BK_FILE_RESULT> FileResultArray;
    hr = MutiFileScan.Scan( FilePathArray, FileResultArray );
    if ( FAILED(hr) )
    {
        return ;
    }
    MutiFileScan.Uninitialize();
}


CEvent ExitEvent;
static unsigned int __stdcall ModeScanThread( void * lpParameter )
{
    _ASSERT( NULL != lpParameter );

      
    int NumWhile = 0;
    DWORD Ret = 0;
    CScanWork * pThis = static_cast<CScanWork*>(lpParameter);
    while ( TRUE )
    {
        Sleep( 1000 );


        HRESULT hr;
        BKDebugOutput(BKDBG_LVL_DEBUG, L"*********************请求扫描信息**************************\n");
        BK_SCAN_INFO ScanInfo;
        hr = pThis->QueryScanInfo( ScanInfo );
        if ( FAILED(hr) )
        {
            BKDebugOutput(BKDBG_LVL_DEBUG,L"***QueryScanInfo failed !!!!!!!!\n");
            break;
        }
        BKDebugOutput(BKDBG_LVL_DEBUG,L"***扫描文件 %s  文件个数 %d  !!!!!!!!\n", ScanInfo.LastScanFileName, ScanInfo.ScanFileCount );
        BKDebugOutput( BKDBG_LVL_DEBUG, L"***QueryScanInfo \n");
        BKDebugOutput( BKDBG_LVL_DEBUG, L"ScanInfo.CantCleanVirus %d \n ScanInfo.CleanedVirus %d \n ScanInfo.CleanFailedCount %d \n ScanInfo.LastScanFileName %s \n", ScanInfo.CantCleanVirus, ScanInfo.CleanedVirus, ScanInfo.CleanFailedCount, ScanInfo.LastScanFileName );
        BKDebugOutput( BKDBG_LVL_DEBUG, L"ScanInfo.Progress %d \n ScanInfo.QueryFailedCount %d \n ScanInfo.RebootCount %d \n ScanInfo.SafeCount %d \n ScanInfo.ScanFileCount %d \n ", ScanInfo.Progress, ScanInfo.QueryFailedCount, ScanInfo.RebootCount, ScanInfo.SafeCount, ScanInfo.ScanFileCount);
        BKDebugOutput( BKDBG_LVL_DEBUG, L"ScanInfo.ScanFinished %d \n ScanInfo.ScanMode %d \n ScanInfo.ScanTime %d \n ScanInfo.Step %d \n ScanInfo.TrustCount %d \n ScanInfo.TrustCount %d \n ScanInfo.UnkownCount %d \n ScanInfo.VirusCount %d \n ", ScanInfo.ScanFinished, ScanInfo.ScanMode, ScanInfo.ScanTime, ScanInfo.Step, ScanInfo.TrustCount, ScanInfo.TrustCount, ScanInfo.UnkownCount, ScanInfo.VirusCount );
        BKDebugOutput(BKDBG_LVL_DEBUG, L"***********************************************\n\n");


        if ( 1 == ScanInfo.ScanFinished )
        {
            {
                Sleep( 1000 );
                //BKDebugOutput(BKDBG_LVL_DEBUG, L"扫描结束 11111");
                ExitEvent.Set();
                break;
            }
           
        }  

        //BKDebugOutput( BKDBG_LVL_DEBUG, L"&&&&&&&&&&&&&&&&&&请求清除信息&&&&&&&&&&&&&&&&&&&&&&&&\n");
        //BK_CLEAN_INFO CleanInfo;
        //hr = pThis->QueryCleanInfo( CleanInfo );
        //if ( FAILED(hr) )
        //{
        //    BKDebugOutput( BKDBG_LVL_DEBUG, L"&&&QueryScanInfo failed !!!!!!!!\n");
        //    break;
        //}
        //BKDebugOutput( BKDBG_LVL_DEBUG, L"&&&QueryScanInfo \n");
        //BKDebugOutput( BKDBG_LVL_DEBUG, L"CleanInfo.CleanFinished %d \n CleanInfo.LastCleanFileName %s \n CleanInfo.OperatedVirus %d \n", CleanInfo.CleanFinished, CleanInfo.LastCleanFileName, CleanInfo.OperatedVirus );
        //BKDebugOutput( BKDBG_LVL_DEBUG, L"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n\n");

 


/*
        BKDebugOutput( BKDBG_LVL_DEBUG, L"==================请求文件信息BkQueryInfoFileTypeVirus===========================\n");
        
        while ( true )
        {
            BK_FILE_INFO fileinfo;
            hr = pThis->QueryFileInfo( BkQueryInfoFileTypeVirus, i++, fileinfo );
            if ( FAILED(hr) )
            {
                 BKDebugOutput( BKDBG_LVL_DEBUG, L"===QueryFileInfo failed\n");
                break;
            }
            BKDebugOutput( BKDBG_LVL_DEBUG, L"===QueryFileInfo \n");
            BKDebugOutput( BKDBG_LVL_DEBUG, L"===fileinfo.Index %d \n fileinfo.FileResult.CRC %d \n fileinfo.FileResult.FileName %s \n fileinfo.FileResult.MD5 %s \n fileinfo.FileResult.Status %d \n fileinfo.FileResult.Step %d \n fileinfo.FileResult.Type %d \n fileinfo.FileResult.VirusName %s \n fileinfo.FileResult.VirusType %d \n ", fileinfo.Index, fileinfo.FileResult.CRC, fileinfo.FileResult.FileName, fileinfo.FileResult.MD5, fileinfo.FileResult.Status, fileinfo.FileResult.Step, fileinfo.FileResult.Type, fileinfo.FileResult.VirusName, fileinfo.FileResult.VirusType );
        }
        BKDebugOutput( BKDBG_LVL_DEBUG, L"=============================================\n\n");

        BKDebugOutput( BKDBG_LVL_DEBUG, L"==================请求文件信息BkQueryInfoFileTypeUnknown===========================\n");
        
        while ( true )
        {
            BK_FILE_INFO fileinfo;
            hr = pThis->QueryFileInfo( BkQueryInfoFileTypeUnknown, i++, fileinfo );
            if ( FAILED(hr) )
            {
                BKDebugOutput( BKDBG_LVL_DEBUG, L"===QueryFileInfo failed\n");
                break;
            }
            BKDebugOutput( BKDBG_LVL_DEBUG, L"===QueryFileInfo \n");
            BKDebugOutput( BKDBG_LVL_DEBUG, L"===fileinfo.Index %d \n fileinfo.FileResult.CRC %d \n fileinfo.FileResult.FileName %s \n fileinfo.FileResult.MD5 %s \n fileinfo.FileResult.Status %d \n fileinfo.FileResult.Step %d \n fileinfo.FileResult.Type %d \n fileinfo.FileResult.VirusName %s \n fileinfo.FileResult.VirusType %d \n ", fileinfo.Index, fileinfo.FileResult.CRC, fileinfo.FileResult.FileName, fileinfo.FileResult.MD5, fileinfo.FileResult.Status, fileinfo.FileResult.Step, fileinfo.FileResult.Type, fileinfo.FileResult.VirusName, fileinfo.FileResult.VirusType );
        }
        BKDebugOutput( BKDBG_LVL_DEBUG, L"=============================================\n\n");
*/

        //BKDebugOutput(L"------------------暂停 回复 %d-------------------------\n", NumWhile  );
        //if ( 10 == NumWhile++ )
        //{
        //    pThis->Pause();
        //    BKDebugOutput( BKDBG_LVL_DEBUG, L"---------------pause begin \n");
        //    Sleep( 3000 );
        //    pThis->Resume();
        //    BKDebugOutput( BKDBG_LVL_DEBUG, L"---------------pause end \n");
        //    NumWhile = 0;
        //}
        //BKDebugOutput(L"-------------------------------------------\n\n");
    
        //BKDebugOutput(L"++++++++++++++++++请求最后扫描信息+++++++++++++++++++++++++++\n");
        //hr = pThis->QueryLastScanInfo( ScanInfo );
        //if ( FAILED(hr) )
        //{
        //     BKDebugOutput(L"+++QueryLastScanInfo failed !!!!!!!!!!\n");
        //     break;
        //}
        //BKDebugOutput(L"+++QueryLastScanInfo successed\n");
        //BKDebugOutput(L"+++++++++++++++++++++++++++++++++++++++++++++\n\n");


        //static int j = 0;
        //j++;
        //if ( j == 3 )
        //{
        //    pThis->Stop();
        //}

       
    }
    return Ret;
}




  
void TestScanMode()
 {
    ExitEvent.Create( NULL, TRUE, FALSE, NULL );

    
        CScanWork scanwork;
        HRESULT hr = S_OK;
        hr = scanwork.Initialize( 0, &g_cb );
        if ( FAILED(hr) )
        {
            return;
        }


        CAtlArray<CString> CustomParam;
        CustomParam.Add( L"E:\\virus\\virus" );

        BK_SCAN_SETTING seting;
        seting.Reset();
        seting.bAutoClean = TRUE;
        scanwork.SetScanSetting( seting );


        scanwork.Scan( BkScanModeCheckScan, FALSE, CustomParam );
        HANDLE hThread = (HANDLE)::_beginthreadex( NULL, 0, ModeScanThread, &scanwork, 0, NULL );
        ::CloseHandle( hThread );
        hThread = 0;
        ::WaitForSingleObject( ExitEvent, INFINITE );

        ExitEvent.Reset();

        scanwork.Uninitialize();
    
    ExitEvent.Close();
   
}


void TestMutiScan()
{
    ExitEvent.Create( NULL, TRUE, FALSE, NULL );

    {
        CTestCallBack callback;
        CScanWork ScanWork1;
        CScanWork ScanWork2;
        HRESULT hr;
        hr = ScanWork1.Initialize( 0, &g_cb );
        if ( FAILED(hr) )
        {
            return;
        }
        hr = ScanWork2.Initialize( 0, &g_cb );
        if ( FAILED(hr) )
        {
            return;
        }


        CAtlArray<CString> CustomParam;
        CustomParam.Add( L"E:\\virus\\virus" );




        ScanWork1.Scan( BkScanModeCheckScan, FALSE, CustomParam );
        ScanWork2.Scan( BkScanModeCheckScan, FALSE, CustomParam );
        //::_beginthreadex( NULL, 0, ModeScanThread, &scanwork, 0, NULL );
        ::WaitForSingleObject( ExitEvent, INFINITE );
        ExitEvent.Reset();
        ::WaitForSingleObject( ExitEvent, INFINITE );
        ScanWork1.Uninitialize();
        ScanWork2.Uninitialize();
    }
}


void TestWhileScanMode()
{
    ExitEvent.Create( NULL, TRUE, FALSE, NULL );

    int i = 0;
    while ( ++i<10 )
    {
        ExitEvent.Reset();
        CTestCallBack callback;
        CScanWork scanwork;
        HRESULT hr;
        hr = scanwork.Initialize( 0, &g_cb );
        if ( FAILED(hr) )
        {
            return;
        }


        CAtlArray<CString> CustomParam;
        CustomParam.Add( L"E:\\virus\\virus" );


        scanwork.Scan( BkScanModeCheckScan, FALSE, CustomParam );
        ::_beginthreadex( NULL, 0, ModeScanThread, &scanwork, 0, NULL );
        ::WaitForSingleObject( ExitEvent, INFINITE );
        scanwork.Uninitialize();
    }

}