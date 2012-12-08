///////////////////////////////////////////////////////////////	
//	
// FileName  :  khttpredirect.h
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-08-14  16:40
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#pragma once
#include <iostream>
#include <string>
/*
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>*/
#include <map>

using namespace std;

class KHttpRedirect;
/*
class BridgeConnection:
	public boost::enable_shared_from_this<BridgeConnection>,
	private boost::noncopyable
{
public:

	BridgeConnection(boost::asio::io_service& io_service)
		: m_local(io_service), m_remote( io_service )
	{
		InterlockedIncrement( &s_refCount );
		cout << s_refCount << " ";
	}

	~BridgeConnection()
	{
		InterlockedDecrement( &s_refCount );
		cout << s_refCount << " ";
	}

	tcp::socket& socket()
	{
		return m_local;
	}

	void start( KHttpRedirect *m_root );
	static volatile LONG s_refCount;
private:

	void connect_web_server( const unsigned int uIp, const unsigned short sPort )
	{
		boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::address_v4(uIp), sPort );
		m_remote.async_connect(endpoint,
			boost::bind(&BridgeConnection::handle_connect, shared_from_this(),
			boost::asio::placeholders::error ) );
	}
	
	void handle_connect( const boost::system::error_code& error )
	{
		if(!error)
		{
			m_local.set_option( boost::asio::socket_base::receive_buffer_size(8192) );
			m_remote.set_option( boost::asio::socket_base::receive_buffer_size(8192) );
			m_local.set_option( boost::asio::socket_base::send_buffer_size(8192) );
			m_remote.set_option( boost::asio::socket_base::send_buffer_size(8192) );

			m_local.async_read_some( boost::asio::buffer(m_bufLocalRead),
				boost::bind(&BridgeConnection::handle_local_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));

			m_remote.async_read_some( boost::asio::buffer(m_bufRemoteRead),
				boost::bind(&BridgeConnection::handle_remote_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}


	void handle_remote_read( const boost::system::error_code& error, size_t bytes_transferred )
	{
		if( !error )
		{
			//过滤接收信息
			ULONG uLen = FilterRecvData( m_bufRemoteRead.data(), bytes_transferred, m_bufRemoteRead.size() );
			
			boost::asio::async_write( m_local, boost::asio::buffer(m_bufRemoteRead, uLen),
				boost::bind(&BridgeConnection::handle_remote_write, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			boost::system::error_code ignored_ec;
			m_local.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		}
	}

	void handle_remote_write( const boost::system::error_code& error, size_t bytes_transferred )
	{
		if( !error )
		{
			if( m_strReply.size() )
			{
				boost::system::error_code ignored_ec;
				m_local.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
				m_remote.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
				return ;
			}

			m_remote.async_read_some( boost::asio::buffer(m_bufRemoteRead),
				boost::bind(&BridgeConnection::handle_remote_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}

	void handle_local_write( const boost::system::error_code& error, size_t bytes_transferred )
	{
		if( !error )
		{
			m_local.async_read_some( boost::asio::buffer(m_bufLocalRead),
				boost::bind(&BridgeConnection::handle_local_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}

	void handle_local_read( const boost::system::error_code& error, size_t bytes_transferred )
	{
		if( !error )
		{
			//过滤发送信息
			ULONG uLen = FilterSendData( m_bufLocalRead.data(), bytes_transferred, m_bufLocalRead.size() );
			if( !uLen )
			{
				boost::asio::async_write( m_local, boost::asio::buffer(m_strReply.c_str(), m_strReply.length()),
					boost::bind(&BridgeConnection::handle_remote_write, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
				return ;
			}

			boost::asio::async_write( m_remote, boost::asio::buffer(m_bufLocalRead, bytes_transferred),
				boost::bind(&BridgeConnection::handle_local_write, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			boost::system::error_code ignored_ec;
			m_remote.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		}
	}
	
	ULONG FilterSendData( char *pSendBuffer, ULONG uSendLen, ULONG uBufferLen );
	ULONG FilterRecvData( char *pRecvBuffer, ULONG uRecvLen, ULONG uBufferLen );
	
	boost::array<char, 8193> m_bufRemoteRead;
	boost::array<char, 8193> m_bufLocalRead;
	string m_strReply;
	tcp::socket m_local;
	tcp::socket m_remote;

};

typedef boost::shared_ptr<BridgeConnection> connection_ptr;

class tcp_server
{
public:
	tcp_server()
		: acceptor_(m_io_service),
		m_root(NULL),
		new_connection_(new BridgeConnection( m_io_service ) )
	{
		tcp::endpoint localListenEndpoint( boost::asio::ip::address::from_string("127.0.0.1"), 80);
		acceptor_.open( localListenEndpoint.protocol() );
		acceptor_.set_option( boost::asio::ip::tcp::acceptor::reuse_address(true) );
		acceptor_.bind( localListenEndpoint );
		acceptor_.listen();
		acceptor_.async_accept( new_connection_->socket(),
			boost::bind(&tcp_server::handle_accept, this,
			boost::asio::placeholders::error) );
	}

	void SetRoot( KHttpRedirect *root )
	{
		m_root = root;
	}

	bool run()
	{
		for( int i=0; i<4; i++)
			CreateThread( 0, 0, ThreadListen, (PVOID)this, 0, 0 );
		return 0;
	}

	void stop()
	{
		m_io_service.stop();
	}

private:
	
	void handle_accept(	const boost::system::error_code& error )
	{
		if (!error)
		{
			new_connection_->start( m_root );
			new_connection_.reset( new BridgeConnection(acceptor_.io_service()) );
			acceptor_.async_accept(new_connection_->socket(),
				boost::bind(&tcp_server::handle_accept, this,
				boost::asio::placeholders::error ));
		}
	}

	static DWORD WINAPI ThreadListen( PVOID pParam )
	{
		tcp_server* pThis = (tcp_server*)pParam;
		pThis->m_io_service.run();
		return 0;
	}


	boost::asio::io_service m_io_service;
	tcp::acceptor acceptor_;
	connection_ptr new_connection_;
	KHttpRedirect *m_root;
};



class KHttpRedirect:public IRedirect
{
public:
	KHttpRedirect(void);
	~KHttpRedirect(void);
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		 REFIID riid,
		 void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		return S_OK;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef( void) 
	{
		return S_OK;
	}

	virtual ULONG STDMETHODCALLTYPE Release( void)
	{
		return S_OK;
	}

	virtual STDMETHODIMP_(BOOL) Init();
	virtual STDMETHODIMP_(BOOL) UnInit();
	virtual STDMETHODIMP_(USHORT) NeedRedirect( 
		const WCHAR *szModlueName, 
		ULONG uLocalIP, USHORT uLocalPort, 
		ULONG uRemoteIP, USHORT uRemotePort );

	unsigned __int64 FindRemoteAddr( USHORT uPort );

private:
	tcp_server m_tcpServer;
	map< USHORT, unsigned __int64> m_redirectMap;
	CCriticalSection g_csMap;
	USHORT m_uBindPort;
	
};
*/