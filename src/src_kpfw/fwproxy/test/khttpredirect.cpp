///////////////////////////////////////////////////////////////	
//	
// FileName  :  khttpredirect.cpp
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-08-14  16:39
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#include "StdAfx.h"
#include "khttpredirect.h"
/*
volatile LONG BridgeConnection::s_refCount = 0;

extern VOID WriteDateToFile( TCHAR *szFileName, PVOID pBuffer, DWORD dwLength );

KHttpRedirect::KHttpRedirect(void):m_uBindPort(80)
{
	m_tcpServer.SetRoot(this);
}

//////////////////////////////////////////////////////////////////////////

KHttpRedirect::~KHttpRedirect(void)
{
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(BOOL) KHttpRedirect::Init()
{
	m_tcpServer.run();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(BOOL) KHttpRedirect::UnInit()
{
	m_tcpServer.stop();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(USHORT) KHttpRedirect::NeedRedirect( 
	const WCHAR *szModlueName, 
	ULONG uLocalIP, USHORT uLocalPort, 
	ULONG uRemoteIP, USHORT uRemotePort )
{
	USHORT uPort = 0;
	ULARGE_INTEGER int64RemoteAddr = { uRemotePort, htonl(uRemoteIP)};

	do 
	{

		if( uRemotePort != m_uBindPort )
		{
			break;
		}

		//wprintf( L"-------------------------------\nPath:%s\nLocal:%08X[%d]\nRemote:RIP:%08X[%d]\n",
		//	szModlueName, uLocalIP, uLocalPort, uRemoteIP, uRemotePort );

		g_csMap.Enter();
		m_redirectMap[uLocalPort] = int64RemoteAddr.QuadPart;
		g_csMap.Leave();
		
		uPort = m_uBindPort;

	} while (FALSE);
	
	return uPort;
}

//////////////////////////////////////////////////////////////////////////

unsigned __int64 KHttpRedirect::FindRemoteAddr( USHORT uLocalPort )
{
	__int64 i64Return = 0;
	map< USHORT, unsigned __int64 >::iterator item;

	g_csMap.Enter();
	item = m_redirectMap.find( uLocalPort );
	if( item != m_redirectMap.end() )
	{
		i64Return = item->second;
		m_redirectMap.erase( item );
	}
	g_csMap.Leave();
	
	return i64Return;
}

//////////////////////////////////////////////////////////////////////////

void BridgeConnection::start( KHttpRedirect *root )
{
	//连接sina.com
	//
	ULARGE_INTEGER int64RemoteAddr = {0};
	
	boost::asio::ip::tcp::endpoint local_endpoint = m_local.remote_endpoint();
	
	int64RemoteAddr.QuadPart = root->FindRemoteAddr( local_endpoint.port() );
	if( int64RemoteAddr.QuadPart == 0 )
	{
		return ;
	}

	connect_web_server( int64RemoteAddr.HighPart, (USHORT)int64RemoteAddr.LowPart );
}

//////////////////////////////////////////////////////////////////////////

ULONG BridgeConnection::FilterSendData( char *pSendBuffer, ULONG uSendLen, ULONG uBufferLen )
{
	//WriteDateToFile( L"C:\\httpSend.txt", pSendBuffer, uSendLen );

	if( strstr(pSendBuffer, "www.baidu.com" ) != NULL )
	{
		m_strReply = "<html><head><title>金山网镖拦截</title></head><body><p>HoHo~~  这个操作已经被拦截了，请打开网镖设置</p></body></html>";
		return 0;
	}
	return uSendLen;
}

//////////////////////////////////////////////////////////////////////////

ULONG BridgeConnection::FilterRecvData( char *pRecvBuffer, ULONG uRecvLen, ULONG uBufferLen )
{
	//传回的网页一般使用zip压缩
	//WriteDateToFile( L"C:\\httpRecv.txt", pSendBuffer, uSendLen );
	return uRecvLen;
}

*/