// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Psapi.h>
#pragma comment (lib, "psapi.lib")

#include "fwproxy_public.h"
#include "SimpleKfwEvent.h"
#include "dnsrecorder.h"
#include "kpcaprecorder.h"
#include "khttpredirect.h"
#include "ProgramTrafficMgr.h"
#include <strstream>
#include <string>
#include <vector>
#include <sstream>
#include "Psapi.h"
using namespace std;
/*
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;
using namespace boost::algorithm;
*/


UCHAR gsToUpperTable[256 + 4] =
{
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,123,124,125,126,127,
	128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
	144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
	160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
	176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
	192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
	208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
	224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
	240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
	0, 0, 0, 0
};

BOOL LoadInterface(const GUID& iid,  void** pInterface );

BOOL CreateKpfwService();
BOOL UnInitKpfwService();
BOOL InstallTdiDriver( DWORD dwEnable, DWORD dwRequest );
BOOL UninstallTdiDriver();
BOOL InstallDriver( DWORD dwEnable, DWORD dwRequest );
BOOL UninstallDriver();
BOOL Start();
BOOL Stop();
BOOL GetFlux();
BOOL GetConnectInfo();
BOOL GetProcessFlux();
BOOL GetProgramFlux();
BOOL EnableNet( BOOL bEnable );
BOOL EnableKfw( BOOL bEnable );
BOOL GetKfwState();
VOID Usage();
BOOL GetProcessNameFromPid(DWORD pid, char *szProcessName );
BOOL UpPrivilege();
VOID ClearResultCache();
BOOL SetProtectFile();
BOOL GetModuleList();
BOOL UnitTest();
BOOL SetRecorder();
BOOL OpenPort( USHORT uPort );
BOOL ClosePort( USHORT uPort );
BOOL ShowOpenPort();
BOOL LoadRedirect();
BOOL SetProcessSpeed( string &strCmdLine );
BOOL SetRefreshCacheTimer( int nTimer );
BOOL GetProgramTrafficFromFile();

IFwProxy *g_pFwProxy = NULL;
IFwEvent *g_pFwEvent = NULL;


list<IRedirect*> g_listRedirect;

ofstream* g_peInfo = NULL;

CProgramTrafficMgr g_ProgramTrafficMgr;

ostream& GetPeInfo()
{
	if (g_peInfo)
		return *g_peInfo;
	else
		return cout;
}

#define  TDI_SVR_NAME	TEXT("ktdifilt")

BOOL _StartDriverSvr(LPCTSTR lpcszSvrName)
{
	if ( !lpcszSvrName )
		return FALSE;

	BOOL bRetCode = FALSE;
	SC_HANDLE hSvrMgr = NULL;
	SC_HANDLE hSvrSpe = NULL;
	SERVICE_STATUS_PROCESS ssStatus; 
	DWORD dwOldCheckPoint; 
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;

	SetLastError(0);

	hSvrMgr = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if ( !hSvrMgr )
	{
		goto Exit0;
	}

	hSvrSpe = ::OpenService(hSvrMgr, lpcszSvrName, SERVICE_ALL_ACCESS);
	if ( !hSvrSpe )
	{
		goto Exit0;
	}

	if ( !::StartService(hSvrSpe, 0, NULL) && GetLastError() != ERROR_SERVICE_ALREADY_RUNNING )
	{
		goto Exit0;
	}

	if (!QueryServiceStatusEx( 
		hSvrSpe,             // handle to service 
		SC_STATUS_PROCESS_INFO, // info level
		(LPBYTE)&ssStatus,              // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded ) )              // if buffer too small
	{
		goto Exit0;
	}

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING) 
	{ 
		// Do not wait longer than the wait hint. A good interval is 
		// one tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if( dwWaitTime < 1000 )
			dwWaitTime = 1000;
		else if ( dwWaitTime > 10000 )
			dwWaitTime = 10000;

		Sleep( dwWaitTime );


		if (!QueryServiceStatusEx( 
			hSvrSpe,             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE)&ssStatus,              // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded ) )              // if buffer too small
			break;

		if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
		{
			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if( GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint )
			{
				break;
			}
		}
	} 

	bRetCode = (ssStatus.dwCurrentState == SERVICE_RUNNING);

Exit0:
	if ( hSvrMgr )
	{
		CloseServiceHandle(hSvrMgr);
		hSvrMgr = NULL;
	}

	if ( hSvrSpe )
	{
		CloseServiceHandle( hSvrSpe );
		hSvrSpe = NULL;
	}

	return bRetCode;
}

int _tmain(int argc, _TCHAR* argv[])
{
	//return UnitTest();
	wcout.imbue(locale("chs"));
	setlocale(LC_CTYPE, "chs");

	KSCoInitialize();

	if (argc == 1)
	{				
		do 
		{
			if ( !LoadInterface(CLSID_IFwProxyImp, (VOID**)&g_pFwProxy) )
			{
				printf("LoadInterface failed\n");
				break;
			}
			
			g_pFwEvent = new SimpleKfwEvent();
			if( g_pFwEvent == NULL )
				break;
			((SimpleKfwEvent*)g_pFwEvent)->Initialize();
			
			if ( g_pFwProxy->Initialize( g_pFwEvent ) != S_OK )
			{
				printf("Initialize failed 1\n");

				if ( !_StartDriverSvr(TDI_SVR_NAME) )
					break;

				if (FAILED(g_pFwProxy->Initialize(g_pFwEvent)))	
				{
					printf("Initialize failed 2 \n");
					break;
				}
				
			}

			g_pFwProxy->EnableFirewall();
			g_pFwProxy->SetRequestFlag(REQUEST_TRUST_URL | REQUEST_DOWNLOAD_PE | ENABLE_TRAFFIC_CONTRL);
			
			// g_pFwProxy->SetRequestFlag( 0xffffffff );
			// g_pFwProxy->SetRequestFlag( 0xffffffff - REQUEST_TCP );

			Usage();
			
			UpPrivilege();

			string strCmdLine;
			while(true)
			{
				cout << "KFW>>" ;
				getline( cin, strCmdLine );

				if ( strCmdLine.length() == 0 )
					continue;
				
				if ( strCmdLine == "?" || strCmdLine == "h" )
				{
					Usage();
					continue;
				}
				
				if ( strCmdLine == "q" )
				{
					g_pFwProxy->UnInitialize();
					g_pFwProxy->Release();
					g_pFwProxy = NULL;

					if( g_pFwEvent != NULL )
					{
						((SimpleKfwEvent*)g_pFwEvent)->UnInitialize();
						delete g_pFwEvent;
						g_pFwEvent = NULL;
					}

					break;
				}
							
				if ( strCmdLine == "f" ) 
				{
					GetFlux();
					continue;
				}
				
				if ( strCmdLine == "c") 
				{
					GetConnectInfo();
					continue;
				}

				if ( strCmdLine == "p") 
				{
					GetProcessFlux();
					continue;
				}

				if ( strCmdLine == "pp") 
				{
					GetProgramFlux();
					continue;
				}

				if ( strCmdLine == "pc" ) 
				{
					if( !g_ProgramTrafficMgr.Init() )
					{
						g_ProgramTrafficMgr.Uninit();
					}
					continue;
				}

				if ( strCmdLine.find("spc ") != string::npos && strCmdLine.length() > 3)
				{
					SetRefreshCacheTimer( atoi(&strCmdLine.c_str()[3]) );
					continue;
				}

				if ( strCmdLine == "ppf" )
				{
					GetProgramTrafficFromFile();
					continue;
				}

				if ( strCmdLine == "en" )
				{
					EnableNet( TRUE );
					continue;
				}

				if ( strCmdLine == "dn" ) 
				{
					EnableNet( FALSE );
					continue;
				}

				if ( strCmdLine == "ef" ) 
				{
					EnableKfw( TRUE );
					continue;
				}

				if ( strCmdLine == "df" ) 
				{
					EnableKfw( FALSE );
					continue;
				}

				//if ( strCmdLine == "d" ) 
				//{
				//	__asm int 3;
				//	continue;
				//}
				
				if ( strCmdLine == "r" ) 
				{
					ClearResultCache();
					continue;
				}

				if ( strCmdLine == "s" ) 
				{
					GetKfwState();
					continue;
				}

				//if ( strCmdLine == "sp" )
				//{
				//	SetProtectFile();
				//	continue;
				//}

				//if (strCmdLine == "peinfo")
				//{
				//	string peInfoFile;
				//	cin >> peInfoFile;
				//	if (g_peInfo)
				//	{
				//		delete g_peInfo;
				//		g_peInfo = NULL;
				//	}
				//	if (peInfoFile.length() > 0)
				//	{
				//		g_peInfo = new ofstream(peInfoFile.c_str());
				//	}
				//	continue;
				//}

				if (strCmdLine == "m")
				{
					GetModuleList();
					continue;
				}
				
				//if( strCmdLine == "sr" )
				//{
				//	SetRecorder();
				//	continue;
				//}

				if ( strCmdLine.find("op ") != string::npos && strCmdLine.length() > 3)
				{
					OpenPort( atoi(&strCmdLine.c_str()[3]) );
					continue;
				}

				if ( strCmdLine.find("cp ") != string::npos && strCmdLine.length() > 3)
				{
					ClosePort( atoi(&strCmdLine.c_str()[3]) );
					continue;
				}

				if( strCmdLine == "lp" )
				{
					ShowOpenPort();
					continue;
				}

				if( strCmdLine == "lr" )
				{
					LoadRedirect();
					continue;
				}

				if( strCmdLine.find("ss ") != string::npos )
				{
					SetProcessSpeed( strCmdLine );
					continue;
				}

				Usage();
			}
		} while (FALSE);

		KSCoUninitialize();

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////

	try {
		if (argc < 2)
			return 0;

		TCHAR* pArg1 = argv[1];
		
		do 
		{
			if (_tcsicmp(pArg1, TEXT("/ipe")) == 0)
			{
				InstallTdiDriver( ENABLE_TCP, REQUEST_DOWNLOAD_PE );
				break;
			}

			if (_tcsicmp(pArg1, TEXT("/u")) == 0)
			{
				UninstallTdiDriver();
				break;
			}

			if (_tcsicmp(pArg1, TEXT("/i")) == 0)
			{
				//CreateKpfwService();
				InstallDriver( ENABLE_ALL, REQUEST_ALL );
				break;
			}
			
		} while (FALSE);

	}
	catch(exception& e) {
		cerr << "error: " << e.what() << "\n";
		return 1;
	}
	catch(...) {
		cerr << "Exception of unknown type!\n";
	}
	

	KSCoUninitialize();
	system("pause");

	return 0;
}

//////////////////////////////////////////////////////////////////////////

BOOL CreateKpfwService()
{
	int	nRetCode = 0;
	WCHAR wszModulePath[MAX_PATH] = { 0 };
	STARTUPINFO StartInfo = {0};
	PROCESS_INFORMATION ProcessInfo = {0};

	GetModuleFileName(NULL, wszModulePath, sizeof(wszModulePath) / sizeof(wszModulePath[0]));

	*(wcsrchr(wszModulePath, _T('\\')) + 1) = '\0';

	wcscat_s(wszModulePath, MAX_PATH, _T("kiserv.exe"));
	
	StartInfo.cb = sizeof(STARTUPINFO);
	nRetCode = ::CreateProcess(wszModulePath,
							   _T(" /install"), 
							   NULL,
							   NULL,
							   FALSE,
							   NORMAL_PRIORITY_CLASS,
							   NULL,
							   NULL,
							   &StartInfo,
							   &ProcessInfo);
	if ( nRetCode )
	{
		::WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
		CloseHandle(ProcessInfo.hThread);
		CloseHandle(ProcessInfo.hProcess);
		printf("Create Kiserv Success!\n");
	}
	else
		printf("Create Kiserv Failed! %d\n", GetLastError());


	return TRUE;
}

BOOL UnInitKpfwService()
{
	int	nRetCode = 0;
	WCHAR wszModulePath[MAX_PATH] = { 0 };
	STARTUPINFO StartInfo = {0};
	PROCESS_INFORMATION ProcessInfo = {0};

	GetModuleFileName(NULL, wszModulePath, sizeof(wszModulePath) / sizeof(wszModulePath[0]));

	*(wcsrchr(wszModulePath, _T('\\')) + 1) = '\0';

	wcscat_s(wszModulePath, MAX_PATH, _T("kiserv.exe"));

	nRetCode = ::CreateProcess(wszModulePath, 
		_T(" /uninstall"), 
		NULL,
		NULL,
		FALSE,
		NORMAL_PRIORITY_CLASS,
		NULL,
		NULL,
		&StartInfo,
		&ProcessInfo);
	if ( nRetCode )
	{
		::WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
		printf("UnInit Kiserv Success!\n");
	}
	else
		printf("UnInit Kiserv Failed! %d\n", GetLastError());

	return TRUE;

}

BOOL InstallDriver( DWORD dwEnable, DWORD dwRequest )
{
	ULONG status = E_FAIL;
	IFwInstaller *FwInstaller = NULL;
	if( LoadInterface(__uuidof(IFwInstaller), (VOID**)&FwInstaller) )
	{
		status = FwInstaller->Install();
		if( status == S_OK )
		{
			FwInstaller->SetConfig( dwEnable, dwRequest );
		}
	}
	else
	{
		printf( "无法获取安装接口\n" );
	}
	
	printf( "%s:%08X\n", status == S_OK ? "安装成功" : "安装失败", status );
	if( FwInstaller != NULL )
		FwInstaller->Release();
	return status == S_OK;
}

//////////////////////////////////////////////////////////////////////////

BOOL InstallTdiDriver( DWORD dwEnable, DWORD dwRequest )
{
	ULONG status = E_FAIL;
	IFwInstaller *FwInstaller = NULL;
	if( LoadInterface(__uuidof(IFwInstaller), (VOID**)&FwInstaller) )
	{
		status = FwInstaller->Install();
		if( status == S_OK )
		{
			FwInstaller->SetConfig( dwEnable, dwRequest );
		}
	}
	else
	{
		printf( "无法获取安装接口\n" );
	}

	printf( "%s:%08X\n", status == S_OK ? "安装成功" : "安装失败", status );
	if( FwInstaller != NULL )
		FwInstaller->Release();
	return status == S_OK;
}


//////////////////////////////////////////////////////////////////////////

BOOL UninstallDriver()
{
	ULONG status = E_FAIL;
	BOOL bResult = FALSE;
	IFwInstaller *FwInstaller = NULL;
	if( LoadInterface(__uuidof(IFwInstaller), (VOID**)&FwInstaller) )
	{
		status = FwInstaller->UnInstall();
	}
	else
	{
		printf( "无法获取安装接口\n" );
	}

	printf("%s\n", status == S_OK ? "卸载成功" : "卸载失败" );

	if( FwInstaller != NULL )
		FwInstaller->Release();

	return status == S_OK;
}

//////////////////////////////////////////////////////////////////////////

BOOL UninstallTdiDriver()
{
	ULONG status = E_FAIL;
	BOOL bResult = FALSE;
	IFwInstaller *FwInstaller = NULL;
	if( LoadInterface(__uuidof(IFwInstaller), (VOID**)&FwInstaller) )
	{
		status = FwInstaller->UnInstall();
	}
	else
	{
		printf( "无法获取安装接口\n" );
	}

	printf("%s\n", status == S_OK ? "卸载成功" : "卸载失败" );

	if( FwInstaller != NULL )
		FwInstaller->Release();

	return status == S_OK;
}


//////////////////////////////////////////////////////////////////////////

BOOL GetFlux()
{
	ULONG status = E_FAIL;
	BOOL bResult = FALSE;
	
	ULARGE_INTEGER int64FluxIn = {0};
	ULARGE_INTEGER int64FluxOut = {0};
	ULARGE_INTEGER int64FluxInLocal = {0};
	ULARGE_INTEGER int64FluxOutLocal = {0};

	status = g_pFwProxy->GetFlux( &int64FluxIn.QuadPart, &int64FluxOut.QuadPart ,
		&int64FluxInLocal.QuadPart, &int64FluxOutLocal.QuadPart);
	if( status == S_OK )
	{

		float fInFlux = int64FluxIn.QuadPart / float(1024);
		float fOutFlux = int64FluxOut.QuadPart / float(1024);
		float fCountFlux = fInFlux + fOutFlux;

		float fInFluxM = int64FluxIn.QuadPart / float(1024) / float(1024);
		float fOutFluxM = int64FluxOut.QuadPart / float(1024) / float(1024);
		float fCountFluxM = fInFluxM + fOutFluxM;

		printf("\t\tBytes\t\tKB\t\tMB\n");

		printf("IN   %16I64d%16.2f% 16.2f\n", 
			int64FluxIn.QuadPart, fInFlux, fInFluxM );

		printf("OUT  %16I64d%16.2f% 16.2f\n", 
			int64FluxOut.QuadPart, fOutFlux, fOutFluxM );

		printf("COUNT%16I64d%16.2f% 16.2f\n", 
			int64FluxOut.QuadPart + int64FluxIn.QuadPart, fCountFlux, fCountFluxM );
				
		
		bResult = TRUE;
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////

BOOL GetConnectInfo()
{
	ULONG retCount = 0;
	ULONG needSize = 1;
	char  szProcessPath[MAX_PATH] = {0};
	CONN_INFO *connInfoList = NULL;
	map<int ,string > mapConnectionStatus;
	
	mapConnectionStatus[TDI_STATUS_TCP_CONNECT] = "TDI_STATUS_TCP_CONNECT";
	mapConnectionStatus[TDI_STATUS_TCP_CONNECT_COMPLETE] = "TDI_STATUS_TCP_CONNECT_COMPLETE";
	mapConnectionStatus[TDI_STATUS_TCP_LISTEN] = "TDI_STATUS_TCP_LISTEN";
	mapConnectionStatus[TDI_STATUS_TCP_BE_CONNECTED] = "TDI_STATUS_TCP_BE_CONNECTED";
	mapConnectionStatus[TDI_STATUS_TCP_SEND] = "TDI_STATUS_TCP_SEND";
	mapConnectionStatus[TDI_STATUS_TCP_RECV] = "TDI_STATUS_TCP_RECV";
	mapConnectionStatus[TDI_STATUS_UDP_LISTEN] = "TDI_STATUS_UDP_LISTEN";
	mapConnectionStatus[TDI_STATUS_UDP_SEND_DATAGRAM] = "TDI_STATUS_UDP_SEND_DATAGRAM";
	mapConnectionStatus[TDI_STATUS_UDP_RECV_DATAGRAM] = "TDI_STATUS_UDP_RECV_DATAGRAM";

	while( TRUE )
	{
		retCount = 0;

		if( connInfoList != NULL )
			delete []connInfoList;
		
		connInfoList = new CONN_INFO[needSize];
		if ( connInfoList == NULL )
			break;

		memset((char*)connInfoList, 0, needSize * sizeof(CONN_INFO) );

		if( g_pFwProxy->EnumConnect( connInfoList, needSize, &retCount ) == S_OK )
			break;

		if( GetLastError() != ERROR_BAD_LENGTH )
			break;

		needSize *= 2;
	}

	
	for( ULONG i=0; i<retCount; i++ )
	{
		printf( "#%i--------------\n", i );
		
		printf( "Status:\n%s\n", mapConnectionStatus[connInfoList[i].State].c_str() );		
		printf( "ProcessId:\t%d\n", connInfoList[i].ProcessId );
		
		if( !GetProcessNameFromPid( connInfoList[i].ProcessId, szProcessPath ) )
		{
			printf( "ProcessPath:\tErrCode:%d\n", GetLastError() );
		}
		else
		{
			printf( "ProcessPath:\t%s\n", szProcessPath );
		}
		printf( "ThreadID:\t%d\n", connInfoList[i].ThreadId );
		
		printf("LocalAddr:\t%02d.%02d.%02d.%02d\t:%d\n", 
			*(((BYTE*)&connInfoList[i].LocalAddr)+0), 
			*(((BYTE*)&connInfoList[i].LocalAddr)+1), 
			*(((BYTE*)&connInfoList[i].LocalAddr)+2), 
			*(((BYTE*)&connInfoList[i].LocalAddr)+3), 
			connInfoList[i].LocalPort );

		printf("RemoteAddr:\t%02d.%02d.%02d.%02d\t:%d\n", 
			*(((BYTE*)&connInfoList[i].RemoteAddr)+0), 
			*(((BYTE*)&connInfoList[i].RemoteAddr)+1), 
			*(((BYTE*)&connInfoList[i].RemoteAddr)+2), 
			*(((BYTE*)&connInfoList[i].RemoteAddr)+3), 
			connInfoList[i].RemotePort );
		
		printf("Host:\t%s\n", connInfoList[i].m_strHost );
		printf("Url:\t%s\n", connInfoList[i].m_strUrl );
		printf("InBytes:\t%I64d\n", connInfoList[i].BytesIn);
		printf("OutBytes:\t%I64d\n", connInfoList[i].BytesOut);
		printf("ModuleID:\t%08X\n", connInfoList[i].dwModuleID );
		
	}

	if ( connInfoList != NULL )
	{
		delete []connInfoList;
		connInfoList = NULL;
	}

	return retCount != 0;
}

//////////////////////////////////////////////////////////////////////////
BOOL GetProcessFlux()
{	
	ULONG retCount = 0;
	ULONG needSize = 1;
	char  szProcessPath[MAX_PATH] = {0};
	PROCESS_TRAFFIC *pProcessTraffic = NULL;

	while( TRUE )
	{
		retCount = 0;

		if( pProcessTraffic != NULL )
			delete []pProcessTraffic;

		pProcessTraffic = new PROCESS_TRAFFIC[needSize];
		if ( pProcessTraffic == NULL )
			break;

		memset((char*)pProcessTraffic, 0, needSize * sizeof(PROCESS_TRAFFIC) );

		if( g_pFwProxy->EnumProcessTraffic( pProcessTraffic, needSize, &retCount ) == S_OK )
			break;

		if( GetLastError() != ERROR_BAD_LENGTH )
			break;

		needSize *= 2;
	}
	
	for( ULONG i=0; i<retCount; i++ )
	{
		//FileTimeToLocalFileTime( (FILETIME*)&pProcessTraffic[i].ulCreateTime, &filetimeLocal );
		//FileTimeToSystemTime( &filetimeLocal, &sysTime );

		printf( "#%i---------------------------------------\n", i );
		printf( "ProcessID:\t%d\n", pProcessTraffic[i].uProcessID );
		if( !GetProcessNameFromPid( pProcessTraffic[i].uProcessID, szProcessPath ) )
		{
			printf( "ProcessPath:\tErrCode:%d\n", GetLastError() );
		}
		else
		{
			printf( "ProcessPath:\t%s\n", szProcessPath );
		}
		//printf( "CreateTime:\t%02d-%02d %02d:%02d:%02d\n", sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond );
		printf( "InBytes:\t%I64d\n", pProcessTraffic[i].ulTrafficIn.QuadPart );
		printf( "OutBytes:\t%I64d\n", pProcessTraffic[i].ulTrafficOut.QuadPart );
		printf( "SendSpeedLimit:\t%d [KB/S]\n", pProcessTraffic[i].ulSendLimit );
		printf( "RecvSpeedLimit:\t%d [KB/S]\n", pProcessTraffic[i].ulRecvLimit );

	}

	if ( pProcessTraffic != NULL )
	{
		delete []pProcessTraffic;
		pProcessTraffic = NULL;
	}

	return retCount != 0;
}

//////////////////////////////////////////////////////////////////////////

BOOL EnableNet( BOOL bEnable )
{
	HRESULT hResult = E_FAIL;
	if( bEnable )
		hResult = g_pFwProxy->EnableNetwork();
	else
		hResult = g_pFwProxy->DisableNetwork();

	return hResult == S_OK;
}

//////////////////////////////////////////////////////////////////////////

BOOL EnableKfw( BOOL bEnable )
{
	HRESULT hResult = E_FAIL;
	if( bEnable )
		hResult = g_pFwProxy->EnableFirewall();
	else
		hResult = g_pFwProxy->DisableFirewall();

	return hResult == S_OK;
}

//////////////////////////////////////////////////////////////////////////
BOOL GetKfwState()
{
	HRESULT hResult = E_FAIL;
	KFW_STATE kfwState;

	hResult = g_pFwProxy->GetKfwState( &kfwState );
	if ( hResult == S_OK )
	{
		printf( "\tKfwVer:\t\t%d.%d\n", kfwState.usMajorVer, kfwState.usMinorVer );
		printf( "\tEnableKfw:\t%s\n", kfwState.bEnableKfw ? "TRUE" : "FALSE" );
		printf( "\tEnableNet:\t%s\n", kfwState.bEnableNet ? "TRUE" : "FALSE" );
	}

	return hResult == S_OK;
}

//////////////////////////////////////////////////////////////////////////



BOOL LoadInterface(const GUID& iid,  void** pInterface )
{
	static KSDll dll;
	static BOOL b = FALSE;

	if (!b)
	{
		WCHAR wszProxy[MAX_PATH + 1] = { 0 };

		GetModuleFileName(NULL, wszProxy, MAX_PATH);
		*(wcsrchr(wszProxy, L'\\')) = 0L;
		wcscat_s(wszProxy, MAX_PATH, L"\\fwproxy.dll");

		if (S_OK == dll.Open(wszProxy))
			b = TRUE;
	}
	
	if (!b)
		return FALSE;

	return dll.GetClassObject(iid, iid, pInterface) == S_OK;
	/*
	typedef HRESULT  (__stdcall *TDllGetClassObject)(REFCLSID, REFIID, LPVOID*);
	ULONG status = S_OK;
	TDllGetClassObject _DllGetClassObject = NULL;
	WCHAR wszProxy[MAX_PATH + 1] = { 0 };
	CLSID nilClsId;

	GetModuleFileName(NULL, wszProxy, MAX_PATH);
	*(wcsrchr(wszProxy, L'\\')) = 0L;
	wcscat_s(wszProxy, MAX_PATH, L"\\fwproxy.dll");

	HINSTANCE m_hFwProxy = LoadLibraryW(wszProxy);
	if ( !m_hFwProxy )
	{
		wprintf(L"LoadLibrary(%s) Failed:%d\n", wszProxy, GetLastError());
		return FALSE;
	}

	_DllGetClassObject = (TDllGetClassObject)GetProcAddress( m_hFwProxy, "DllGetClassObject" );
	if (!_DllGetClassObject)
	{
		wprintf(L"Get 'DllGetClassObject' Address Failed\n");
		return FALSE;
	}
	
	status = _DllGetClassObject( nilClsId, iid, (void**)pInterface );
	if( S_OK != status )
	{
		wprintf(L"DllGetClassObject failed:%d\n", status);
		return FALSE;
	}

	return TRUE;*/

	/*
	HRESULT hRes = E_FAIL;
	static BOOL s_bLocalSCOMInited = FALSE;
	if (!s_bLocalSCOMInited)
	{
		//进程内创建SCOM组件
		s_bLocalSCOMInited = TRUE;
		hRes = KSCoCreateInstance(iid, iid , (void**)pInterface);

	}
	return hRes == S_OK;*/
}


VOID Usage()
{
	cout << "KPFW_Test--TDI. "<< __DATE__ <<"  " << __TIME__<<endl;
	cout << "\ts\t" << "Status"<<endl;
	cout << "\tf\t" << "Flux" <<endl;
	cout << "\tc\t" << "Connection" <<endl;
	cout << "\tp\t" << "Process Flux" <<endl;
	cout << "\tpp\t" << "Program Flux" <<endl;
	//cout << "\tpc\t" << "从Cache中读取程序流量"<<endl;
	//cout << "\tspc\t" << "刷新Cache的时间阀值"<<endl;
	//cout << "\tppf\t" << "从文件中读取程序流量"<<endl;
	cout << "\ten\t" << "Enable Network" <<endl;
	cout << "\tdn\t" << "Disable Network" <<endl;
	cout << "\tef\t" << "Enable KPFW" <<endl;
	cout << "\tdf\t" << "Disable KPFW" <<endl;
	cout << "\tr\t" << "Clear Cache" <<endl;
	//cout << "\tpeinfo FileName\t" << "设置pe信息保存的文件" <<endl;
	//cout << "\tsp\t" << "保护文件列表.当前目录protect.txt" <<endl;
	//cout << "\tm\t" << "查看当前保护文件列表" <<endl;
	//cout << "\tsr\t" << "数据记录列表.当前目录recorder.txt" <<endl;
	cout << "\top\t" << "[port]Open Port" <<endl;
	cout << "\tcp\t" << "[port]Close Port" <<endl;
	cout << "\tlp\t" << "Show Opened Port" <<endl;
	//cout << "\tlr\t" << "加载重定向模块" <<endl;
	cout << "\tss\t" << "[pid] [send speed] [recv speed] KB/s"<<endl;
	cout << "\th\t" << "Help" <<endl;
	cout << "\tq\t" << "Quit" <<endl;
}

BOOL GetProcessNameFromPid(DWORD pid, char *szProcessName )
{
	BOOL bResult = FALSE;
	if ( pid == 4 || pid == 8)
	{
		strncpy_s( szProcessName, MAX_PATH, "System", MAX_PATH );
		return TRUE;
	}
	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , 0, pid );
	if( hProcess == NULL )
		return FALSE;
	WCHAR buf[MAX_PATH] = {0};
	WCHAR bufWIN32[MAX_PATH] = {0};
	if (GetProcessImageFileNameW(hProcess, buf, MAX_PATH))
	{
		((SimpleKfwEvent*)g_pFwEvent)->GetDeviceNameMgr().FixDosPathToNormalPath(buf, bufWIN32);
		USES_CONVERSION;
		strcpy_s(szProcessName, MAX_PATH, W2A(bufWIN32));
		bResult = TRUE;
	}
	if (hProcess)
	{
		::CloseHandle(hProcess);
		hProcess = NULL;
	}
	return bResult;
	// return GetProcessImageFileName ( hProcess, 0, szProcessName, MAX_PATH ) != 0;
}

//////////////////////////////////////////////////////////////////////////

BOOL UpPrivilege()
{   
	HANDLE   hToken;                        
	TOKEN_PRIVILEGES   tkp;
	BOOL result = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken);   
	if ( !result )
		return result;   

	result = LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid );   
	if(!result)         //查看进程权限错误   
		return   result;   

	tkp.PrivilegeCount   =   1;     //   one   privilege   to   set   
	tkp.Privileges[0].Attributes   =   SE_PRIVILEGE_ENABLED;   

	result = AdjustTokenPrivileges( hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL );

	return  result;   
}

//////////////////////////////////////////////////////////////////////////

VOID ClearResultCache()
{
	g_pFwProxy->ClearResultCache();
}

//////////////////////////////////////////////////////////////////////////

BOOL SetProtectFile()
{
	vector<ModuleInfo> vecModuleInfo;
	wstring wsLine;
	string strLine;
	ModuleInfo moduleInfo;

	ifstream fileProtect("protect.txt");
	if( !fileProtect )
	{
		cout << "无法打开文件" << GetLastError() <<endl;
		return FALSE;
	}
	
	while( getline( fileProtect, strLine ) )
	{
		cout << strLine <<endl;
		ConvertUncode( strLine, wsLine );	
		memset( (char*)&moduleInfo, 0, sizeof(moduleInfo) );
		wcscpy( moduleInfo.wsModulePath, wsLine.c_str() );
		vecModuleInfo.push_back( moduleInfo );
	}
	fileProtect.close();

	if( vecModuleInfo.size() )
		g_pFwProxy->SetModuleInfo( &vecModuleInfo[0], (ULONG)vecModuleInfo.size() );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

ULONG GetStringHashWZ( WCHAR *wsSrc )
{
	ULONG ulLoopCount = 0;
	ULONG ulResult = 0;

	if ( wsSrc == NULL)
		return 0;

	while( wsSrc[ulLoopCount] )
	{
		ulResult = (ulResult * 31) +  gsToUpperTable[wsSrc[ulLoopCount] >> 8];
		ulResult = (ulResult * 31) +  gsToUpperTable[wsSrc[ulLoopCount] & 0x00ff];
		ulLoopCount++;
	}

	return ulResult;
}

//////////////////////////////////////////////////////////////////////////

ULONG GetStringHash( const char *szSrc )
{
	ULONG ulLoopCount = 0;
	ULONG ulResult = 0;

	if ( szSrc == NULL)
		return 0;

	while( szSrc[ulLoopCount] )
	{
		ulResult = (ulResult * 31);
		ulResult = (ulResult * 31) +  gsToUpperTable[(BYTE)szSrc[ulLoopCount]];
		ulLoopCount++;
	}

	return ulResult;
}

//////////////////////////////////////////////////////////////////////////

BOOL SetRecorder()
{/*
	vector<DATA_RECORDER> vecRecorder;
	wstring wsLine;
	string strLine;
	DATA_RECORDER Recorder;
	vector<string> sliptList;
	wstring wsPath;
	ifstream fileProtect("recorder.txt");
	if( !fileProtect )
	{
		cout << "无法打开文件" << GetLastError() <<endl;
		return FALSE;
	}

	while( getline( fileProtect, strLine ) )
	{
		cout << strLine <<endl;
		memset( (char*)&Recorder, 0, sizeof(Recorder) );
		
		split( sliptList, strLine, is_any_of("?" ) );
		if( sliptList.size() != 3 )
			continue;
		
		Recorder.dwModlueId = GetStringHash( sliptList[0].c_str() );
		Recorder.dwLocalPort = boost::lexical_cast<short>( sliptList[1] );
		Recorder.dwRemotePort = boost::lexical_cast<short>( sliptList[2] );

		vecRecorder.push_back( Recorder );
	}
	fileProtect.close();

	if( vecRecorder.size() )
		g_pFwProxy->AddRecorder( &vecRecorder[0], (ULONG)vecRecorder.size() );
*/
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

BOOL GetModuleList()
{
	ULONG retCount = 0;
	ULONG needSize = 1;
	ModuleInfo *moduleInfoList = NULL;

	while( TRUE )
	{
		retCount = 0;

		if( moduleInfoList != NULL )
			delete []moduleInfoList;

		moduleInfoList = new ModuleInfo[needSize];
		if ( moduleInfoList == NULL )
			break;

		memset((char*)moduleInfoList, 0, needSize * sizeof(ModuleInfo) );

		if( g_pFwProxy->GetModuleList( moduleInfoList, needSize, &retCount ) == S_OK )
			break;

		if( GetLastError() != ERROR_BAD_LENGTH )
			break;

		needSize *= 2;
	}


	for( ULONG i=0; i<retCount; i++ )
	{
		DWORD ws = 0;
		cout << i <<"\t----------------------------------" <<endl;
		//wprintf( L"FilePath:%s\n", moduleInfoList[i].wsModulePath );
		WriteConsoleW( GetStdHandle(STD_OUTPUT_HANDLE), moduleInfoList[i].wsModulePath ,
				wcslen(moduleInfoList[i].wsModulePath),&ws,NULL );
		cout << endl;
		cout << "Changed:"<< (moduleInfoList[i].bChanged ? "TRUE" : "FALSE") <<endl;
		cout << "ModuleID:"<< moduleInfoList[i].dwModuleID <<endl;
		cout << "MD5:";
		for( int j=0; j<16; j++)
			printf( "%02X", moduleInfoList[i].byMd5[j] );
		cout <<endl;
	}

	if ( moduleInfoList != NULL )
	{
		delete []moduleInfoList;
		moduleInfoList = NULL;
	}

	return retCount != 0;
}

//////////////////////////////////////////////////////////////////////////

BOOL OpenPort( USHORT uPort )
{
	if( uPort == 0 )
		return FALSE;

	return g_pFwProxy->OpenPort( enumPT_TCP, uPort ) 
		&& g_pFwProxy->OpenPort( enumPT_UDP, uPort ) ;
}

//////////////////////////////////////////////////////////////////////////

BOOL ClosePort( USHORT uPort )
{
	if( uPort == 0 )
		return FALSE;

	return g_pFwProxy->ClosePort( enumPT_TCP, uPort ) 
		&& g_pFwProxy->ClosePort( enumPT_UDP, uPort ) ;
}

//////////////////////////////////////////////////////////////////////////

BOOL ShowOpenPort()
{
	PVOID* pObjArr = new PVOID[65536*2];
	memset( (char*)pObjArr, 0, 65536*sizeof(PVOID)*2 );

	if( g_pFwProxy->GetOpenPort( pObjArr, 65536*sizeof(PVOID)*2 ) )
	{
		int iCount = 1;
		cout << "TcpPort:"<<endl;
		for( int i=0;i<65536; i++ )
		{
			if( pObjArr[i] != NULL )
			{
				cout << i << "\t";
				if( (iCount++) % 8 == 0 )
					cout <<endl;
			}
		}

		iCount = 1;
		cout <<endl<< "UdpPort:" << endl;
		for( int i=0;i<65536; i++ )
		{
			if( pObjArr[i+65536] != NULL )
			{
				cout << i << "\t";
				if( (iCount++) % 8 == 0 )
					cout <<endl;
			}
		}
		cout <<endl;
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////

BOOL UnitTest()
{
	
	const DWORD dwReadSize = 0x1e;
	DWORD dwRead = 0;
	char pPackBuffer[dwReadSize] = {0};
	HANDLE hFile = CreateFile( L"c:\\dns.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0 );
	if( hFile == INVALID_HANDLE_VALUE )
		return TRUE;

	//if( !SetFilePointer( hFile, 0x2e6, 0, FILE_BEGIN ) )
	//	return TRUE;

	if( !ReadFile( hFile, pPackBuffer, dwReadSize, &dwRead, 0 ) )
		return TRUE;

	CloseHandle( hFile );
	

	KPcapFileRecorder pcapRecorder;
	
	pcapRecorder.Record( L"C:\\dns.pcap", enumPT_UDP, enumPD_Send, 0x11223344, 0x3300, 0x11223344, 0x4400, pPackBuffer, dwReadSize );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

BOOL LoadRedirect()
{/*
	//TODO:遍历redirect目录, 加载所有dll
	
	IRedirect *pRedirect = NULL;

	if( g_listRedirect.size() )
		return FALSE;
	
	pRedirect = new KHttpRedirect();
	if( pRedirect == NULL )
		return FALSE;

	if( !pRedirect->Init() )
		return FALSE;

	g_listRedirect.push_back( pRedirect );
*/
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

BOOL SetProcessSpeed( string &strCmdLine )
{
	vector<string> optVec;
	istringstream str(strCmdLine);

	string strCmd;
	str >> strCmd;
	if (strCmd != "ss")
		return FALSE;

	ULONG nPid = 0;
	str >> nPid;

	ULONG nSendSpeed = 0;
	str >> nSendSpeed;

	ULONG nRecvSpeed = 0;
	str >> nRecvSpeed;

	return g_pFwProxy->SetProcessSpeed( nPid, nSendSpeed*1024, nRecvSpeed*1024 , FALSE) == S_OK;
}

//////////////////////////////////////////////////////////////////////////

VOID OutputProgramTrafficItem( PROGRAM_TRAFFIC *pProgramTraffic )
{
	WCHAR  szProcessPath[MAX_PATH] = {0};	
	ModuleInfo moduleInfo;
	SYSTEMTIME sysTime;
	FILETIME filetimeLocal;
	memset( (char*)&moduleInfo, 0, sizeof(ModuleInfo) );
	FileTimeToLocalFileTime( (FILETIME*)&pProgramTraffic->ulTime, &filetimeLocal );
	FileTimeToSystemTime( &filetimeLocal, &sysTime );

	moduleInfo.dwModuleID = pProgramTraffic->dwModuleID;
	if( S_OK == g_pFwProxy->GetModuleInfo( &moduleInfo, 1 ) )
	{
		wcout << L"ProgramPath:" << moduleInfo.wsModulePath<<endl;
	}
	printf( "CreateTime:\t%02d-%02d %02d:%02d:%02d\n", sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond );
	printf( "InBytes:\t%I64d\n", pProgramTraffic->ulTrafficIn.QuadPart );
	printf( "OutBytes:\t%I64d\n", pProgramTraffic->ulTrafficOut.QuadPart );
}


//////////////////////////////////////////////////////////////////////////

BOOL GetProgramFlux()
{
	ULONG retCount = 0;
	ULONG needSize = 1;
	PROGRAM_TRAFFIC *pProgramTraffic = NULL;

	while( TRUE )
	{
		retCount = 0;

		if( pProgramTraffic != NULL )
			delete []pProgramTraffic;

		pProgramTraffic = new PROGRAM_TRAFFIC[needSize];
		if ( pProgramTraffic == NULL )
			break;

		memset((char*)pProgramTraffic, 0, needSize * sizeof(PROGRAM_TRAFFIC) );

		if( g_pFwProxy->EnumProgramTraffic( pProgramTraffic, needSize, &retCount ) == S_OK )
			break;

		if( GetLastError() != ERROR_BAD_LENGTH )
			break;

		needSize *= 2;
	}

	for( ULONG i=0; i<retCount; i++ )
	{
		printf( "#%d---------------------------------------\n", i );
		OutputProgramTrafficItem( &pProgramTraffic[i] );
	}

	if ( pProgramTraffic != NULL )
	{
		delete []pProgramTraffic;
		pProgramTraffic = NULL;
	}

	return retCount != 0;
}

//////////////////////////////////////////////////////////////////////////

BOOL SetRefreshCacheTimer( int nTimer )
{
	return g_pFwProxy->SetProgramTrafficTimer( (ULONG)nTimer ) == S_OK;
}

//////////////////////////////////////////////////////////////////////////

BOOL GetProgramTrafficFromFile()
{
	PROGRAM_TRAFFIC_FROM_FILE pProgramTraffic[200];
	ULONG dwRetCount = 0;
	ULONG dwOffset = 0;
	ULONG dwCount = 0;
	for(;;)
	{
		dwRetCount = 0;
		g_pFwProxy->EnumProgramTrafficFromFile( pProgramTraffic, 200, dwOffset, &dwRetCount );
		if( dwRetCount == 0 )
			break;

		for( DWORD i=0; i<dwRetCount; i++ )
		{
			printf( "\n#%d---------------------------------------\n", dwCount++ );
			OutputProgramTrafficItem( (PROGRAM_TRAFFIC*)&pProgramTraffic[i] );
			wcout<< L"ProgramPath:\t" << pProgramTraffic[i].wsFilePath<<endl;
		}
		dwOffset += dwRetCount;
	}
	return FALSE;
}