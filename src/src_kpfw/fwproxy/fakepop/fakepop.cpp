// FakePop.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <conio.h>
#include <Psapi.h>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
namespace po = boost::program_options;

#pragma comment (lib, "psapi.lib")

#include <iostream>
#include <iterator>
using namespace std;

BOOL GetProcessNameFromPid(DWORD pid, char *szProcessName );
const int DenyCode = 'Deny';
const int AlwaysDenyCode = 'alDy';
const int AlwaysPassCode = 'alPs';


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


int _tmain(int argc, _TCHAR* argv[])
{
	UpPrivilege();

	int nResult = 0;
	int c = 'y';
	cout.imbue(locale("chs"));

	try 
	{
		po::options_description desc("TestKfw");
		desc.add_options()
			("ptype", po::value<string>(), "协议类型")
			("t", po::value<string>(), "请求类型")
			("pid",po::value<DWORD>(), "进程ID")
			("lip", po::value<DWORD>(),"本地IP")
			("lport", po::value<DWORD>(),"本地端口")
			("rip", po::value<DWORD>(),"远程IP")
			("rport", po::value<DWORD>(),"远程端口")
			("lan", po::value<string>(),"局域网访问")
			("host", po::value<string>(),"网址认证-主机名")
			("url", po::value<string>(),"网址认证-URL")
			("m", po::value<string>(),"模块名")
			("cf", po::value<string>(),"文件改变路径")
			;
		if( argc == 1)
		{
			cout << desc <<endl;
			return -1;
		}

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
		
		char szLocalAddr[32] = {0};
		char szRemoteAddr[32] = {0};
		char szProcessName[MAX_PATH] = {0};
		string requestProtocal = vm["ptype"].as<string>();
		string requestType = vm["t"].as<string>();
		string strModulePath ;

		po::variables_map::iterator opItem = vm.find( "m" );
		if( opItem != vm.end() )
		{
			strModulePath = vm["m"].as<string>();
		}
		
		DWORD pId = vm["pid" ].as<DWORD>();
		
		if( !GetProcessNameFromPid( pId, szProcessName ) )
		{
			sprintf_s( szProcessName, MAX_PATH, "GetProcessName Failed:%d", GetLastError() );
		}

		cout << "Protocal:\t"<< requestProtocal <<endl <<
			"RequestType:\t"<< requestType << endl <<
			"ProcessID:\t"<< boost::lexical_cast<string>(pId) <<endl <<
			"ModulePath:\t\n"<< strModulePath << endl <<
			"ProcessPath:\t"<< szProcessName << endl;
			
		if( requestProtocal == "FILE_MON" )
		{
			string strFilePath = vm["cf"].as<string>();	
			cout << "FilePath:\t" << strFilePath <<endl;
			system("pause");
			return 0;
		}
		else if( requestProtocal == "LAN" )
		{
			string strLanPath = vm["lan"].as<string>();	
			cout << "LanPath:\t" << strLanPath <<endl;
		}
		else if( requestProtocal == "HTTP" )
		{
			string strHost = vm["host"].as<string>();	
			cout << "Host:\t" << strHost <<endl;

			string strUrl = vm["url"].as<string>();	
			cout << "Url:\t" << strUrl <<endl;
		}
		else
		{
			DWORD lip = vm[ "lip" ].as<DWORD>();
			DWORD lport = vm[ "lport" ].as<DWORD>();
			DWORD rip = vm[ "rip" ].as<DWORD>();
			DWORD rport = vm[ "rport" ].as<DWORD>();
			sprintf_s(szLocalAddr, 32, "%02d.%02d.%02d.%02d\t:%d", *(((BYTE*)&lip)+0), *(((BYTE*)&lip)+1), *(((BYTE*)&lip)+2), *(((BYTE*)&lip)+3), lport );
			sprintf_s(szRemoteAddr, 32, "%02d.%02d.%02d.%02d\t:%d", *(((BYTE*)&rip)+0), *(((BYTE*)&rip)+1), *(((BYTE*)&rip)+2), *(((BYTE*)&rip)+3), rport );

			cout <<	"LocalAddr:\t"<< szLocalAddr << endl <<
				"RemoteAddr:\t"<< szRemoteAddr << endl;
		}

		cout << "Press 'n' or 'N' to deny this Request..."<<endl <<
			"Press 'd' or 'D' to deny all of Requests..."<<endl <<
			"Press 'p' or 'P' to pass all of Requests..."<<endl;
			
		c = _getch();

	}
	catch(exception& e) {
		cerr << "error: " << e.what() << "\n";

		for( int i=0; i<argc; i++ )
		{
			cout << argv[i] <<endl;
		}
		system("pause");
		return 0;
	}
	catch(...) {
		cerr << "Exception of unknown type!\n";
		system("pause");
		return 0;
	}

	int iRetCode = 0;

	if( c == 'n' || c == 'N' )
		iRetCode = DenyCode;
	else if( c == 'd' || c == 'D' )
		iRetCode = AlwaysDenyCode;
	else if( c == 'p' || c == 'P' )
		iRetCode = AlwaysPassCode;
	
	return iRetCode;
}

//////////////////////////////////////////////////////////////////////////
BOOL GetProcessNameFromPid(DWORD pid, char *szProcessName )
{
	if ( pid == 4 || pid == 8 )
	{
		strncpy_s( szProcessName, MAX_PATH, "System", 7 );
		return TRUE;
	}

	HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, 0, pid );
	if( hProcess == NULL )
		return FALSE;

	return GetModuleFileNameExA( hProcess, 0, szProcessName, MAX_PATH ) != 0;
}