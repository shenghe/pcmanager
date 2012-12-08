// bkscan2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "scantest.h"
#include "winmod\winpath.h"


#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


int _tmain(int argc, _TCHAR* argv[])
{
    HRESULT hr = E_FAIL;

    {
        WCHAR AA[5] ;
        wcsncpy_s( AA, 5, L"1234567", 2 );

        int a = 3;
    }

    BKInitDebugOutput();
    //CTestCallBack test;
    //CFastScanPolicy FastScanPolicy;
    //FastScanPolicy.StartScan( &test );

    hr = BkScanInitialize();
    if ( FAILED(hr) )
    {
        return 0;
    }



    // TestRemoveDisk();
    //TestFileScan();
    TestScanMode();
    //TestRelateDir();
    //TestAutoRun();
    //TestScan();


    

    //TestMutiFileScan();
   
    //TestWhileScanMode();
    Sleep( 2000 );

 BkScanUninitialize();
    _CrtDumpMemoryLeaks();


	return 0;
}

