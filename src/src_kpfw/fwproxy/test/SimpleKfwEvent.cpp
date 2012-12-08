#include "StdAfx.h"
#include "SimpleKfwEvent.h"
#include "kpcaprecorder.h"
#include "dnsrecorder.h"
#include "md5.h"
#include <ios>
#include "kapplogadapter.h"
#include "klanmanlogadapter.h"
#include "kmodulelogadapter.h"
#include "khttplogadapter.h"
#include "kpelogadapter.h"
#include "atlconv.h"
#include "psapi.h"
#include "shlobj.h"
using namespace std;
#pragma comment(lib, "Psapi.lib")

extern IFwProxy *g_pFwProxy;
extern list<IRedirect*> g_listRedirect;

CCriticalSection g_csOutput;
KPcapFileRecorder g_pcapRecorder;	//可以为一个连接分配多个记录器
KDnsRecorder g_dnsRecorder;


VOID WriteDateToFile( TCHAR *szFileName, PVOID pBuffer, DWORD dwLength )
{
	HANDLE hFile = CreateFile( szFileName, GENERIC_WRITE, FILE_SHARE_READ,
		0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
	if( hFile != INVALID_HANDLE_VALUE )
	{
		const char szLine[] = "\r\n-------------------------------------------\r\n-------------------------------------------\r\n";
		DWORD dwWrt = 0;
		LARGE_INTEGER i64Pos = {0};
		SetFilePointerEx( hFile, i64Pos, 0, FILE_END);
		WriteFile( hFile, pBuffer, dwLength, &dwWrt, 0 );
		WriteFile( hFile, szLine, sizeof(szLine)-1, &dwWrt, 0 );
		CloseHandle( hFile );	
	}
}

void ConvertAnsi(const wchar_t *wsUncodeString, string &sString)
{
	int nSize = WideCharToMultiByte(CP_ACP, 0, wsUncodeString, -1, NULL, 0, NULL, NULL);
	nSize --;
	sString.resize(nSize);
	WideCharToMultiByte(CP_ACP, 0, wsUncodeString, -1, &sString[0], nSize, NULL, NULL);  
}

void ConvertUncode( const string &sString, wstring &wsString )
{
	int nSize = MultiByteToWideChar( CP_ACP, 0, sString.c_str(), -1, NULL, 0 );
	nSize --;
	wsString.resize( nSize );
	MultiByteToWideChar( CP_ACP, 0, sString.c_str(), -1, &wsString[0], nSize );
}


SimpleKfwEvent::SimpleKfwEvent(void):m_logMgr(NULL)
{
}

SimpleKfwEvent::~SimpleKfwEvent(void)
{
}

extern ostream& GetPeInfo();

BOOL SimpleKfwEvent::GetStackModule( ULONG *uModuleID, ULONG *uRetAddr, string &strModule, wstring &strExePath )
{
	vector <ModuleInfo> vecModule;
	for( int i=0; i<MAX_STACK_CHECK_NUM; i++ )
	{
		ModuleInfo moduleInfo;
		memset( (char*)&moduleInfo, 0, sizeof(ModuleInfo) );
		if( uModuleID[i] == 0 )
			break;
		moduleInfo.dwModuleID = uModuleID[i];

		vecModule.push_back( moduleInfo );
	}

	if( vecModule.size() )
	{
		if( g_pFwProxy->GetModuleInfo( &vecModule[0], (ULONG)vecModule.size() ) == S_OK )
		{
			int i=0;
			for( vector <ModuleInfo>::iterator item = vecModule.begin(); item != vecModule.end(); item ++, i++ )
			{
				string strTemp;
				char hexBuf[16] = {0};
				ConvertAnsi( item->wsModulePath, strTemp);
				strModule += "\"";
				strModule += strTemp;
				//strModule += ":";
				//for( int i=0; i<16; i++ )
				//{
				//	sprintf(hexBuf, "%02X", item->byMd5[i] );
				//	strModule += hexBuf;
				//}
				
				if( uRetAddr != NULL )
				{
					sprintf(hexBuf, "%08X", uRetAddr[i] );
					strModule += "\tRetAddress:";
					strModule += hexBuf;
				}
				strModule += "\"";
				strModule += "\n";

				KModuleLogAdapter moduleLog( item->dwModuleID, item->wsModulePath, item->byMd5 );
				//moduleLog.Do( m_logMgr );
			}
		}

		strExePath = vecModule[0].wsModulePath;
	}
	

	return vecModule.size() != 0;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP SimpleKfwEvent::OnApplicationRequest( RESPONSE_APP_REQUEST_INFO  *pAppRequestInfo )
{
	WAIT_RESPONSE_INFO *waitResponseInfo = new WAIT_RESPONSE_INFO;
	if( waitResponseInfo == NULL )
		return E_FAIL;

	memcpy((char*)&waitResponseInfo->appResponseInfo, (char*)pAppRequestInfo, sizeof(RESPONSE_APP_REQUEST_INFO) );
	waitResponseInfo->pThis = (PVOID)this;

	QueueUserWorkItem( WaitResponseThread, (LPVOID)waitResponseInfo, WT_EXECUTELONGFUNCTION );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP SimpleKfwEvent::OnApplicationNotify( APP_NOTIFY_INFO  *pAppNotifyInfo )
{
	APP_NOTIFY_INFO_CONTEXT *notifyInfoContext = new APP_NOTIFY_INFO_CONTEXT;
	if( notifyInfoContext == NULL )
		return E_FAIL;

	memcpy((char*)&notifyInfoContext->appNotifyInfo, (char*)pAppNotifyInfo, sizeof(APP_NOTIFY_INFO) );
	notifyInfoContext->pThis = (PVOID)this;

	QueueUserWorkItem( AppNotifyThread, (LPVOID)notifyInfoContext, WT_EXECUTELONGFUNCTION );
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////

DWORD SimpleKfwEvent::WaitResponseThread( LPVOID pParam )
{
	ULONG dwResponse = Pass;
	WAIT_RESPONSE_INFO *waitInfo = (WAIT_RESPONSE_INFO *)pParam;
	if ( pParam == NULL )
		return 0;
	
	SimpleKfwEvent* pThis = (SimpleKfwEvent*)waitInfo->pThis;
	RESPONSE_APP_REQUEST_INFO* pRequestInfo = (RESPONSE_APP_REQUEST_INFO*)&waitInfo->appResponseInfo;
	
	pThis->OnApplicationRequest_( &pRequestInfo->appRequestInfo ,&dwResponse );

	if( pRequestInfo->pResponseContext != NULL )
		g_pFwProxy->SendResponse( pRequestInfo->pResponseContext, dwResponse );

	delete waitInfo;
	return 0;	
}

//////////////////////////////////////////////////////////////////////////

DWORD SimpleKfwEvent::AppNotifyThread( LPVOID pParam )
{
	ULONG dwResponse = Pass;
	APP_NOTIFY_INFO_CONTEXT *notifyInfo = (APP_NOTIFY_INFO_CONTEXT *)pParam;
	if ( pParam == NULL )
		return 0;

	SimpleKfwEvent* pThis = (SimpleKfwEvent*)notifyInfo->pThis;
	APP_NOTIFY_INFO* pRequestInfo = (APP_NOTIFY_INFO*)&notifyInfo->appNotifyInfo;

	pThis->OnApplicationNotify_( &notifyInfo->appNotifyInfo );

	delete notifyInfo;
	return 0;	
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP SimpleKfwEvent::OnApplicationRequest_( APP_REQUEST_INFO  *pAppRequestInfo, PULONG pAllow )
{
	DWORD dwResult = 0;
	DWORD dwResponseCode = Pass;
	char szCmdLine[4096] = {0};
	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFOA StartupInfo;
	string szRequestType;
	string szRequestProtocol;
	string sModulePath;
	wstring wsExePath;
	int iret = 0;
	const int DenyCode = 'Deny';
	const int AlwaysDenyCode = 'alDy';
	const int AlwaysPassCode = 'alPs';
	const int PassRecordCode = 'Psrc';
	const int AlwaysPassRecordCode = 'alPr';

	if ( pAppRequestInfo == NULL || pAllow == NULL )
		return E_FAIL;
	
	switch( pAppRequestInfo->nRequestProtocol )
	{
	case enumPT_TCP:
		szRequestProtocol = "TCP";
		break;
	case enumPT_UDP:
		szRequestProtocol = "UDP";
		break;
	case enumPT_RAWIP:
		szRequestProtocol = "RAWIP";
		break;
	case enumPT_LANMAN:
		szRequestProtocol = "LAN";
		break;
	case enumPT_HTTP:
		szRequestProtocol = "HTTP";
		break;
	case enumPT_FILE_MON:
		szRequestProtocol = "FILE_MON";
		break;
	}

	switch( pAppRequestInfo->nRequestType )
	{
	case ART_Connect:
		szRequestType = "ART_Connect";	
		break;
	case ART_Listen:
		szRequestType = "ART_Listen";	
		break;
	case ART_Accept:
		szRequestType = "ART_Accept";	
		break;
	case ART_Bind:
		szRequestType = "ART_Bind";	
		break;
	case ART_Close:
		szRequestType = "ART_Close";	
		break;
	case ART_Create:
		szRequestType = "ART_Create";	
		break;
	case ART_TrustUrl:
		szRequestType = "ART_TrustUrl";
		break;
	case ART_FileChanged:
		szRequestType = "ART_FileChanged";
		break;
	case ART_PacketRecord:
		szRequestType = "ART_PacketRecord";
		break;
	default:
		szRequestType = "Unknown Request";
	}
		

	if( pAppRequestInfo->nRequestType == ART_PacketRecord )
	{
		DWORD lip = pAppRequestInfo->Parameters.TDI.dwLocalAddress;
		SHORT lport = pAppRequestInfo->Parameters.TDI.wLocalPort;
		
		DWORD rip = pAppRequestInfo->Parameters.TDI.dwRemoteAddress;
		SHORT rport = pAppRequestInfo->Parameters.TDI.wRemotePort;
		g_csOutput.Enter();
		printf( "-------------------------------------\n" );
		printf( "Direction:%s\n", pAppRequestInfo->Parameters.TDI.uPacketDir == enumPD_Send ? "Send" : "Recv"  );
		printf( "ART_PacketRecord: PackSize:%d\n", pAppRequestInfo->Parameters.TDI.uDataLen );
		printf( "LocalAddr:%02d.%02d.%02d.%02d:%d\n", 
			*(((BYTE*)&lip)+0), *(((BYTE*)&lip)+1), *(((BYTE*)&lip)+2), *(((BYTE*)&lip)+3), lport );
		printf( "RemoteAddr:%02d.%02d.%02d.%02d:%d\n", 
			*(((BYTE*)&rip)+0), *(((BYTE*)&rip)+1), *(((BYTE*)&rip)+2), *(((BYTE*)&rip)+3), rport );
		
		
		g_pcapRecorder.Record( TEXT("c:\\dns.pcap"), 
				(PROTO_TYPE)pAppRequestInfo->nRequestProtocol,
				(PACKET_DIR)pAppRequestInfo->Parameters.TDI.uPacketDir,
				lip, lport,
				rip, rport,
				pAppRequestInfo->Parameters.TDI.bData,
				pAppRequestInfo->Parameters.TDI.uDataLen );
		
		//g_dnsRecorder.Record( 
		//	(PROTO_TYPE)pAppRequestInfo->nRequestProtocol,
		//	(PACKET_DIR)pAppRequestInfo->Parameters.TDI.uPacketDir,
		//	lip, lport,
		//	rip, rport,
		//	pAppRequestInfo->Parameters.TDI.bData,
		//	pAppRequestInfo->Parameters.TDI.uDataLen );

		g_csOutput.Leave();

		return S_OK;
	}

	//获取堆栈模块
	sModulePath = "Parents:\n";
	if( !GetStackModule( pAppRequestInfo->dwParentModules, NULL, sModulePath, wsExePath ) )
	{
		//sModulePath = "NoFoundModle";
	}
	
	sModulePath += "StackModules:\n";
	//获取堆栈模块
	if( !GetStackModule( pAppRequestInfo->dwStackModules, pAppRequestInfo->dwStackRetAddr, sModulePath, wsExePath ) )
	{
		sModulePath = "NoFoundModle";
	}

	if( pAppRequestInfo->nRequestProtocol == enumPT_LANMAN )
	{
		string sPath;
		
		ConvertAnsi( pAppRequestInfo->Parameters.LanMan.wsPath, sPath );
		sprintf_s(szCmdLine, "fakepop.exe --ptype %s --t %s --pid %d --lan %s --m \n%s",
			szRequestProtocol.c_str(),
			szRequestType.c_str(), 
			pAppRequestInfo->dwProcessId,
			sPath.c_str(),
			sModulePath.c_str()
			);
	}
	else if( pAppRequestInfo->nRequestProtocol == enumPT_HTTP ) 
	{
		if( pAppRequestInfo->nRequestType == ART_TrustUrl )
		{
			//sprintf_s(szCmdLine, "fakepop.exe --ptype %s --t %s --pid %d --host %s --url %s --m \n%s",
			//	szRequestProtocol.c_str(),
			//	szRequestType.c_str(), 
			//	pAppRequestInfo->dwProcessId,
			//	pAppRequestInfo->Parameters.TrustUrl.szHost,
			//	pAppRequestInfo->Parameters.TrustUrl.szUrl,
			//	sModulePath.c_str()
			//	);
			USES_CONVERSION;

			ModuleInfo info = {0};
			info.dwModuleID = pAppRequestInfo->dwStackModules[0];

			if (g_pFwProxy)
				g_pFwProxy->GetModuleInfo( &info, 1 );

			char* pOpString = "UnkownOp";
			if (pAppRequestInfo->Parameters.TrustUrl.nOp == TRUST_URL_OP_HTTP_GET)
				pOpString = "Get";
			else if (pAppRequestInfo->Parameters.TrustUrl.nOp == TRUST_URL_OP_HTTP_POST)
				pOpString = "Post";

			sprintf_s( szCmdLine, "%s %s%s", 
				pOpString,
				pAppRequestInfo->Parameters.TrustUrl.szHost, 
				pAppRequestInfo->Parameters.TrustUrl.szUrl );
			printf( "find http request: %s:\n\t%s\n", W2A(info.wsModulePath), szCmdLine );

			/*
			if (_stricmp(pAppRequestInfo->Parameters.TrustUrl.szHost, "www.baidu.com") == 0)
			{
				printf("deny baidu:\n");
				*pAllow = Deny;
			}*/

			return S_OK;
		}
	}
	else 
	{
		if( pAppRequestInfo->nRequestType == ART_Connect )
		{
			USHORT localPort = 0;
			for( list<IRedirect*>::iterator item = g_listRedirect.begin(); 
						item != g_listRedirect.end();
						item ++ )
			{
				localPort = (*item)->NeedRedirect( wsExePath.c_str(), 
					pAppRequestInfo->Parameters.TDI.dwLocalAddress, pAppRequestInfo->Parameters.TDI.wLocalPort,
					pAppRequestInfo->Parameters.TDI.dwRemoteAddress, pAppRequestInfo->Parameters.TDI.wRemotePort );

				if( localPort != 0 )
				{
					*pAllow = MAKELONG( KfwRedirect, localPort );
					return S_OK;
				}
			}
		}

		sprintf_s(szCmdLine, "fakepop.exe --ptype %s --t %s --pid %d --lip %d --lport %d --rip %d --rport %d --m %s",
			szRequestProtocol.c_str(),
			szRequestType.c_str(), 
			pAppRequestInfo->dwProcessId,
			pAppRequestInfo->Parameters.TDI.dwLocalAddress, pAppRequestInfo->Parameters.TDI.wLocalPort,
			pAppRequestInfo->Parameters.TDI.dwRemoteAddress, pAppRequestInfo->Parameters.TDI.wRemotePort,
			sModulePath.c_str() );
	}


	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof StartupInfo ;

	if(CreateProcessA(NULL,szCmdLine,
		NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,
		NULL,&StartupInfo,&ProcessInfo))
	{	
		//printf( "\r\r\r\r\rPopRequest:%s\n", szCmdLine );
		WaitForSingleObject(ProcessInfo.hProcess,INFINITE);
		GetExitCodeProcess( ProcessInfo.hProcess, &dwResult );

		CloseHandle(ProcessInfo.hThread);
		CloseHandle(ProcessInfo.hProcess);
	}
	else
	{
		//printf( "\r\r\r\r\rPopRequest:%s Error:%d\n", szCmdLine, GetLastError() );
	}
	//printf("KFW>>");
	
	if ( dwResult == DenyCode )
		dwResponseCode = Deny;
	else if( dwResult == AlwaysDenyCode )
		dwResponseCode = AlwaysDeny;
	else if( dwResult == AlwaysPassCode )
		dwResponseCode = AlwaysPass;

	*pAllow = dwResponseCode;

	if( pAppRequestInfo->nRequestProtocol == enumPT_TCP 
		|| pAppRequestInfo->nRequestProtocol == enumPT_UDP
		|| pAppRequestInfo->nRequestProtocol == enumPT_RAWIP )
	{
		KAppLogAdapter appLog( 
			pAppRequestInfo->dwStackModules[0], 
			pAppRequestInfo->nRequestProtocol,
			pAppRequestInfo->nRequestType,
			pAppRequestInfo->Parameters.TDI.dwLocalAddress, pAppRequestInfo->Parameters.TDI.wLocalPort,
			pAppRequestInfo->Parameters.TDI.dwRemoteAddress, pAppRequestInfo->Parameters.TDI.wRemotePort,
			GUID_NULL, 0, dwResponseCode, 0, 0 );

		//appLog.Do( m_logMgr );
	}
	else if(  pAppRequestInfo->nRequestProtocol == enumPT_LANMAN )
	{
		KLanmanLogAdapter lanmanLog( 
			pAppRequestInfo->dwStackModules[0], 
			pAppRequestInfo->Parameters.LanMan.wsPath,
			0, 
			dwResponseCode, 
			0, 
			0 );

		//lanmanLog.Do( m_logMgr );
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////

BOOL IsSamePath(LPCWSTR dosPath, LPCWSTR strNormanPath)
{
	INT nLen1 = wcslen(dosPath);
	INT nLen2 = wcslen(strNormanPath);
	while (nLen1 && nLen2)
	{
		if (tolower(dosPath[nLen1-1]) == tolower(strNormanPath[nLen2-1]))
		{
			nLen1--,nLen2--;
			continue;
		}
		else
			break;
	}
	if (nLen2 == 0)
		return TRUE;
	return FALSE;
}

INT FixDosPathToNormalPath(LPCWSTR dosPath, LPCWSTR pathProcess, WCHAR path[MAX_PATH])
{
	WCHAR pathSystem[MAX_PATH];
	::SHGetFolderPath(NULL, CSIDL_WINDOWS, NULL, SHGFP_TYPE_CURRENT, pathSystem);

	// 先看看是否以特殊的头开始
	if (_wcsnicmp(dosPath, SYSTEM_ROOT_PREFIX, wcslen(SYSTEM_ROOT_PREFIX)) == 0)
	{
		wcscpy_s(path, MAX_PATH, pathSystem);
		wcscat_s(path , MAX_PATH - 2, dosPath + wcslen(SYSTEM_ROOT_PREFIX)-1);
		if (::PathFileExists(path))
			return 1;
	}
	else if (_wcsnicmp(dosPath, WINDOWS_FOLDER_PREFIX, wcslen(WINDOWS_FOLDER_PREFIX)) == 0)
	{
		wcsncpy_s(path, MAX_PATH, pathSystem, 2);
		wcscpy_s(path+2 , MAX_PATH - 2, dosPath);
		if (::PathFileExists(path))
			return 2;
	}

	// 先看看是否与本进程在同一个盘下面
	WCHAR cbProcess = tolower(pathProcess[0]);
	wcsncpy_s(path, MAX_PATH, pathProcess, 2);
	wcscpy_s(path+2 , MAX_PATH - 2, dosPath);
	if (::PathFileExists(path))
		return 3;

	// 看是否在系统盘下面
	WCHAR cbSystem = tolower(pathSystem[0]);
	path[0] = pathSystem[0];
	if (::PathFileExists(path))
		return 4;

	// 枚举盘符
	for (WCHAR cb = L'c'; cb <= L'z'; cb++)
	{
		if ((cb != cbSystem) && (cb != cbProcess))
		{
			path[0] = cb;
			if (::PathFileExists(path))
				return 5;
		}
	}
	return 0;
}

BOOL GetModuleNameFromProcess(HANDLE hProcess, PVOID ImageBase, LPCWSTR strDosPath, WCHAR path[MAX_PATH])
{
	BOOL bFindModule = FALSE;

	WCHAR szModName[MAX_PATH];
	if ( GetModuleFileNameEx( hProcess, (HMODULE)ImageBase, szModName,
		MAX_PATH))
	{
		if (IsSamePath(strDosPath, szModName+2))
		{
			wprintf(L"find module same path direct with imagebase: %s\n", szModName);
			wcscpy_s(path, MAX_PATH, szModName);
			bFindModule = TRUE;
			return bFindModule;
		}
	}/*
	else
	{
		HMODULE hModules[1024] = {0};
		DWORD nSizeModule = 0;
		::EnumProcessModules(hProcess, hModules, sizeof(hModules), &nSizeModule);
		for (int i = 0; i < nSizeModule / sizeof(HMODULE); i++)
		{
			WCHAR szModName[MAX_PATH];

			if ( GetModuleFileNameEx( hProcess, hModules[i], szModName,
				MAX_PATH))
			{
				if (IsSamePath(strDosPath, szModName+2))
				{
					wprintf(L"find module same path: %s\n", szModName);
					wcscpy_s(path, MAX_PATH, szModName);
					bFindModule = TRUE;
				}
			}

			if (bFindModule)
			{
				if (hModules[i] == ImageBase)
				{
					printf("find module handle equal module base\n");
				}
				else
				{
					printf("find module handle not equal module base base: %08x, handle:%08x\n", ImageBase, hModules[i] );
				}
				break;
			}
		}
	}*/
	
	return bFindModule;
}

STDMETHODIMP SimpleKfwEvent::OnApplicationNotify_( APP_NOTIFY_INFO  *pAppNotifyInfo )
{
	//if( pAppNotifyInfo->nRequestProtocol == enumPT_FILE_MON )
	//{
	//	string sPath;
	//	ConvertAnsi( pAppNotifyInfo->Parameters.fileChangedInfo.wsModulePath, sPath );

	//	sprintf_s(szCmdLine, "fakepop.exe --ptype %s --t %s --pid %d --cf \"%s\" --m \n%s",
	//		szRequestProtocol.c_str(),
	//		szRequestType.c_str(), 
	//		pAppNotifyInfo->dwProcessId,
	//		sPath.c_str(),
	//		sModulePath.c_str()
	//		);
	//}
	//
	char szUrl[1024] = {0};
	string sModulePath;
	wstring wsExePath;

	//获取堆栈模块
	if( !GetStackModule( pAppNotifyInfo->dwStackModules, NULL, sModulePath, wsExePath ) )
	{
		sModulePath = "NoFoundModle";
	}

	if( pAppNotifyInfo->nRequestType == ART_LoadImage )
	{
		// printf( "--------------ART_LoadImage\n" );
		// printf("image base: %08x\n", pAppNotifyInfo->Parameters.imageLoadInfo.m_imageInfo.ImageBase);
		// wprintf( L"ImagePath:%s\n\n", pAppNotifyInfo->Parameters.imageLoadInfo.m_strImagePath);

		ModuleInfo info = {0};
		WCHAR pathModule [MAX_PATH] = {0};
		BOOL bFindModule = FALSE;
		/*if (pAppNotifyInfo->dwStackModules[0] != 0)
		{
			info.dwModuleID = pAppNotifyInfo->dwStackModules[0];
			g_pFwProxy->GetModuleInfo(&info, 1);
		}
		else*/
		{
			if (m_DeviceNameMgr.FixDosPathToNormalPath(pAppNotifyInfo->Parameters.imageLoadInfo.m_strImagePath,
				pathModule));
			{
				bFindModule = TRUE;
			}

			HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, pAppNotifyInfo->dwProcessId);
			if (hProcess)
			{
				if (!GetModuleFileNameEx(hProcess, NULL, info.wsModulePath, MAX_PATH))
				{
					printf("GetModuleFileNameEx GetProcess: %d failed: %d", pAppNotifyInfo->dwProcessId, ::GetLastError());
				}
				else
				{
					wprintf( L"ProcessID: %d ProcessPath:%s\n",  pAppNotifyInfo->dwProcessId, info.wsModulePath);
					if (bFindModule)
						wprintf(L"find module same path from driver: %s\n", pathModule);
					else
					{
						// 尝试通过ImageBase或者枚举的方法直接获取路径
						BOOL bFindModule = GetModuleNameFromProcess(hProcess,
							pAppNotifyInfo->Parameters.imageLoadInfo.m_imageInfo.ImageBase,
							pAppNotifyInfo->Parameters.imageLoadInfo.m_strImagePath,
							pathModule);

						// 不能从枚举中查找到，尝试使用强行添加盘符的方法进行猜测

						if (!bFindModule)
						{
							WCHAR pathModule[MAX_PATH] = {0};
							INT nFindSpecail = FixDosPathToNormalPath(pAppNotifyInfo->Parameters.imageLoadInfo.m_strImagePath, 
								info.wsModulePath, pathModule);
							if (nFindSpecail)
							{
								wprintf(L"find module same path with guess method: %d: %s\n", nFindSpecail, pathModule);
								bFindModule = TRUE;
							}
							else
							{
								printf("******cannot find module path with enum_process_modules or guess path\n");
							}
						}
					}
				}
				::CloseHandle(hProcess);
			}
			else
			{
				printf("OpenProcess :%d failed: %d\n", pAppNotifyInfo->dwProcessId, ::GetLastError());
				if (bFindModule)
					wprintf(L"find module same path from driver: %s\n", pathModule);
			}
		}
	}

	if( pAppNotifyInfo->nRequestType == ART_DownloadPeFile )
	{
		printf( "--------------ART_DownloadPeFile\n" );
		printf( "ProcessPath:%s\n", sModulePath.c_str() );
		wprintf( L"SavePath:%s\n", pAppNotifyInfo->Parameters.peFileInfo.m_strSavePath );
		printf( "Url:\t%s\n", pAppNotifyInfo->Parameters.peFileInfo.m_strUrl );
		printf( "Host:\t%s\n", pAppNotifyInfo->Parameters.peFileInfo.m_strHost);
		printf("KFW>>");
		
		sprintf_s( szUrl, "%s%s", 
			pAppNotifyInfo->Parameters.peFileInfo.m_strHost, 
			pAppNotifyInfo->Parameters.peFileInfo.m_strUrl );

		KPeLogAdapter peLog( 
			pAppNotifyInfo->dwStackModules[0], 
			pAppNotifyInfo->Parameters.peFileInfo.m_strSavePath,
			szUrl
			);
		//peLog.Do( m_logMgr );
		return S_OK;
	}
	

	if( pAppNotifyInfo->nRequestType == ART_CreatePeFile )
	{		
		printf( "--------------ART_CreatePeFile\n" );
		printf( "ProcessPath:%s\n", sModulePath.c_str() );
		wprintf( L"SavePath:%s\n", pAppNotifyInfo->Parameters.peFileInfo.m_strSavePath );
		printf("KFW>>");

		KPeCreateAdapter peLog( 
			pAppNotifyInfo->dwStackModules[0],
			pAppNotifyInfo->Parameters.peFileInfo.m_strSavePath
			);
		//peLog.Do( m_logMgr );
	}

	if( pAppNotifyInfo->nRequestType == ART_RenameFile )
	{
		printf( "--------------ART_RenameFile\n" );
		printf( "ProcessPath:%s\n", sModulePath.c_str() );
		wprintf( L"SrcPath:%s\n", pAppNotifyInfo->Parameters.renameFileInfo.m_strSrcPath );
		wprintf( L"DesPath:%s\n", pAppNotifyInfo->Parameters.renameFileInfo.m_strDesPath );
		printf("KFW>>");
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP SimpleKfwEvent::Initialize()
{
	/*
	if( !LoadLogMgr( CLSID_ILogMgrImp,  (void**)&m_logMgr ) )
	{
		printf( __FUNCTION__" failed\n" );
	}
	else
	{
		if( !m_logMgr->Init( L"c:\\kpfw.log" ) )
		{
			printf( __FUNCTION__" LogMgrInit failed\n" );
		}
	}*/
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////


STDMETHODIMP SimpleKfwEvent::UnInitialize()
{
	if( m_logMgr != NULL )
	{
		m_logMgr->UnInit();
		m_logMgr->Release();
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////

BOOL SimpleKfwEvent::LoadLogMgr(const GUID& iid,  void** pInterface )
{
	static KSDll logDll;
	static BOOL b = FALSE;

	if (!b)
	{
		WCHAR wszProxy[MAX_PATH + 1] = { 0 };

		GetModuleFileName(NULL, wszProxy, MAX_PATH);
		*(wcsrchr(wszProxy, L'\\')) = 0L;
		wcscat_s(wszProxy, MAX_PATH, L"\\kislog.dll");

		if (S_OK == logDll.Open(wszProxy))
			b = TRUE;
	}

	if (!b)
		return FALSE;

	return logDll.GetClassObject(iid, iid, pInterface) == S_OK;
}
