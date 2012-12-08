// bkcomm_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "_idl_gen/bksafesvc.h"
#include "comproxy/bkscanfile.h"
#include "comproxy/bkservicecall.h"
#include "comproxy/bkutility.h"
#include "common/procprivilege.h"
#include "common/utility.h"


int _tmain(int argc, _TCHAR* argv[])
{
    HRESULT     hr = S_OK;
    CBkScan     bkscan;
    CBkScanSingleFile   bkscanfile;
    CBkServiceCall  bkcall;
    CBkRescan   bkrescan;
    CProcPrivilege  test;
    CBkUtility  bkutility;
    DWORD   dwUnknown = 0, dwVirus = 0, dwOperatedViurs = 0;
    CAtlArray<BK_FILE_RESULT>   rescanresult;
    CAtlArray<CString>  custom;
    CString path,path1;

    //CAppPath::Instance().GetLeidianAppdataFolderPath( path );
    //CAppPath::Instance().GetLeidianAppPath( path );
    //CAppPath::Instance().GetLeidianDataPath( path );
    //CAppPath::Instance().CreateFullPath( path );
    //CAppPath::Instance().GetLeidianQuarantinePath( path );
    //CAppPath::Instance().CreateLeidianQuarantinePath( path );
    //path1 = L"d:\\program files\\beike\\beikescan.exe";
    //CAppPath::Instance().GetLeidianRecyclePath( path1, path );
    //CAppPath::Instance().CreateLeidianRecyclePath( path1, path );
    //CAppPath::Instance().GetLeidianCachePath( path );
    //CAppPath::Instance().GetLeidianLogPath( path );
    //CAppPath::Instance().GetLeidianTempPath( path );
    //CAppPath::Instance().GetLeidianCfgPath( path );
    ::CoInitialize( NULL );

    //bkutility.Initialize();

    //bkutility.ReportFile();
    //bkutility.SettingChange();

    //bkutility.Uninitialize();
    ////bkrescan.Initialize();

    //bkrescan.GetRescanResult( rescanresult );

    //bkrescan.Uninitialize();
    //bkscanfile.Initialize();
    //BK_FILE_RESULT fr;

    //CString path = L"e:\\bk2\\bksafesvc\\trunk\\bin\\debug\\virus\\091023-1-1.exe1";

    //bkscanfile.ScanFile( path, 0, fr );

    //bkscanfile.CleanFile( path, SCAN_POLICY_AVE | SCAN_POLICY_CACHE, fr );

    //bkscanfile.Uninitialize();
 
    bkscan.Initialize();
    BK_SCAN_SETTING setting;

    setting.dwSize = sizeof(BK_SCAN_SETTING);
    setting.bAutoClean = FALSE;
    setting.nScanPolicy = SCAN_POLICY_AVE | SCAN_POLICY_CACHE;
    setting.bScanArchive = FALSE;
    setting.nScanFileType = BkScanFileTypeExe;

    bkscan.SetScanSetting( setting );
    bkscan.GetScanSetting( setting );

    path = L"c:\\windows\\system32";
    custom.Add(path);
    //path = L"E:\\bk2\\bksafesvc\\trunk\\bin\\debug\\virus";
    path = L"c:\\workspace\\virus";
    custom.Add(path);

    bkscan.Scan( BkScanModeCheckScan, FALSE, custom );
/*
    while( 1 )
    {
        BK_SCAN_INFO    scanInfo;
        BK_FILE_INFO    info;

        bkscan.QueryScanInfo( scanInfo );

        wprintf( L"%s\r\n", scanInfo.LastScanFileName );
        wprintf( L"%d secs Files: %u (%u%%)\r", ( scanInfo.ScanTime + 500 ) / 1000, scanInfo.ScanFileCount, scanInfo.Progress );

        for ( ; dwUnknown < scanInfo.UnkownCount; dwUnknown ++ )
        {
            bkscan.QueryFileInfo( BkQueryInfoFileTypeUnknown, dwUnknown, info );
            wprintf( 
                L"[bkcomm_test]{unkown id : %d|step : %d|name : %s\r\n", 
                info.Index, 
                info.FileResult.Track, 
                info.FileResult.FileName 
                );
        }

        for ( ; dwVirus < scanInfo.VirusCount; dwVirus++ )
        {
            bkscan.QueryFileInfo( BkQueryInfoFileTypeVirus, dwVirus, info );
            wprintf( 
                L"[bkcomm_test]{virus id : %d|step : %d|name : %s|virus : %s\r\n", 
                info.Index, 
                info.FileResult.Track, 
                info.FileResult.FileName,
                info.FileResult.VirusName
                );
        }

        if ( scanInfo.ScanFinished )
        {
            break;
        }

        Sleep( 100 );
    }
*/
    //BK_CLEAN_INFO   cleanInfo;
    //bkscan.QueryCleanInfo( cleanInfo );

    CAtlArray<DWORD>    testarr;
    for ( DWORD i = 0; i < dwVirus; i++ )
    {
        testarr.Add(i);
    }
    //testarr.Add(100);
    //bkscan.Clean(testarr);
    //bkscan.Pause();
    //bkscan.Resume();
    //bkscan.Stop();
    bkscan.Uninitialize();

    ::CoUninitialize();
//     hr = test.SetPri( TRUE, SE_DEBUG_NAME );
//     if ( FAILED( hr ) )
//     {
//         printf( "enable se_debug_name successful\nstart service call");
//         hr = ::CoInitialize( NULL );
//         if ( FAILED( hr ) )
//         {
//             printf( "[bkcomm_test]CoInitialize faild %08x\n", hr );
//         }
// 
//         bkcall.Initialize();
// 
//         //CString exe = L"c:\\windows\\system32\\taskmgr.exe";
//         //CString exe = L"c:\\windows\\regedit.exe";
//         //CString exe = L"c:\\windows\\system32\\cmd.exe";
//         CString exe = L"c:\\debug\\bkcomm_test.exe";
//         CString cmd = L"";
// 
//         bkcall.Execute( exe, cmd, TRUE );
// 
//         bkcall.Uninitialize();
// 
//         ::CoUninitialize();
//     }
//     else
//     {
//         printf( "enable se_debug_name successful\n");
//     }

    //system("pause");
   
	return 0;
}

