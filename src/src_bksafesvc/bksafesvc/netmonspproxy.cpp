#include "StdAfx.h"
#include "common/utility.h"
#include "netmonspproxy.h"

#if defined(__KNetmonSp_DEBUG__)
void __KNetmonSpDbgInfoA(char *format, ...);
#define KNetmonSpDbgInfo __KNetmonSpDbgInfoA
void __KNetmonSpAssert( const char * exp, const char * src_file, int src_line);
#define KNetmonSpAssert(_exp_)  if(!(_exp_)){ __BPAssert(#_exp_, __FILE__, __LINE__);DebugBreak();}
#else
#define KNetmonSpDbgInfo(...)
#define KNetmonSpAssert(_exp_)
#endif


KNetmonSp KNetmonSp::s_Instance;
KNetmonSp::KNetmonSp(void)
{
	m_bStarted = FALSE;
}

KNetmonSp::~KNetmonSp(void)
{
}

HRESULT KNetmonSp::Start()
{
	HRESULT hrRetCode = E_FAIL;
	HRESULT hrResult = E_FAIL;
	CString sPath;
	CAppPath::Instance().GetLeidianAppPath(sPath);
	sPath += L"\\netstat.dll";



	hrRetCode = m_spIKwsServiceProvider.LoadInterface( sPath );
	if( FAILED(hrRetCode) || m_spIKwsServiceProvider == NULL )
	{
		KNetmonSpDbgInfo("Load netstat failed: %ws\n", sPath.GetBuffer() );
		return hrRetCode;
	}

	if( 0 != m_spIKwsServiceProvider->InitializeService(  ))
	{
		KNetmonSpDbgInfo("InitializeService netstat failed: %ws\n", sPath.GetBuffer() );
		return E_FAIL;
	}

	if( 0 !=m_spIKwsServiceProvider->StartService() )
	{
		KNetmonSpDbgInfo("StartService netstat failed: %ws\n", sPath.GetBuffer() );
		m_spIKwsServiceProvider->UninitializeService();
		return E_FAIL;
	}
	KNetmonSpDbgInfo( "Start succeed\n" );

	m_bStarted = TRUE;
	return S_OK;
}

HRESULT KNetmonSp::Stop()
{
	if(m_spIKwsServiceProvider == NULL || !m_bStarted )
	{
		KNetmonSpDbgInfo("Stop failed: Not need\n");
		return E_UNEXPECTED;
	}

	m_spIKwsServiceProvider->StopService();
	m_spIKwsServiceProvider->UninitializeService();
	m_bStarted = FALSE;

	KNetmonSpDbgInfo("Stop succeed\n" );
	return S_OK;
}


#if defined(__KNetmonSp_DEBUG__)
void __KNetmonSpDbgInfoA(char *format, ...)
{
	int Length;
	char *Buffer;
	va_list ap;
	char StaticBuffer[512];
	va_start(ap, format);
	Length = _vscprintf(format, ap);
	va_end(ap);
	int len0;

	Length += 64;

	if( Length >= sizeof(StaticBuffer) )
		Buffer = new char[Length];
	else
		Buffer = StaticBuffer;
	wsprintfA(Buffer, "[KNetmonSp][%06d]", GetCurrentProcessId());
	len0 = strlen(Buffer);

	va_start(ap, format);
	Length = vsnprintf(Buffer + len0, Length, format, ap);
	va_end(ap);
	if( Length > 0 )
	{
		Buffer[len0 + Length] = 0;
	}
	else
	{
		strcpy(Buffer + len0, "KNetmonSp vsnprintf error");
	}

	OutputDebugStringA(Buffer);

	if( Buffer != StaticBuffer )
		delete [] Buffer;
}
void __KNetmonSpAssert( const char * exp, const char * src_file, int src_line)
{
	CHAR Buffer[512];
	const char *msg_format = "[KNetmonSpAssert]\r\n"
		"exp:  \"%s\"\r\n"
		"file: \"%s\"\r\n"
		"line: %d\r\n";

	wsprintfA(Buffer, msg_format, exp, src_file, src_line);
	::MessageBoxA(NULL, Buffer, "KNetmonSpAssert", MB_OK|MB_ICONSTOP);
}

#endif
