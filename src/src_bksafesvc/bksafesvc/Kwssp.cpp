#include "StdAfx.h"
#include "common/utility.h"
#include "Kwssp.h"

#if defined(__KWSSP_DEBUG__)
void __KwsspDbgInfoA(char *format, ...);
#define KwsspDbgInfo __KwsspDbgInfoA
void __KwsspAssert( const char * exp, const char * src_file, int src_line);
#define KwsspAssert(_exp_)  if(!(_exp_)){ __BPAssert(#_exp_, __FILE__, __LINE__);DebugBreak();}
#else
#define KwsspDbgInfo(...)
#define KwsspAssert(_exp_)
#endif


Kwssp Kwssp::s_Instance;
Kwssp::Kwssp(void)
{
	m_bStarted = FALSE;
}

Kwssp::~Kwssp(void)
{
}

HRESULT Kwssp::Start()
{
	HRESULT hrRetCode = E_FAIL;
	HRESULT hrResult = E_FAIL;
	CString sPath;
	CAppPath::Instance().GetLeidianAppPath(sPath);
	sPath += L"\\kwssp.dll";



	hrRetCode = m_spIKwsServiceProvider.LoadInterface( sPath );
	if( FAILED(hrRetCode) || m_spIKwsServiceProvider == NULL )
	{
		KwsspDbgInfo("Load failed: %ws\n", sPath.GetBuffer() );
		return hrRetCode;
	}

	if( 0 != m_spIKwsServiceProvider->InitializeService( this ))
	{
		KwsspDbgInfo("InitializeService failed: %ws\n", sPath.GetBuffer() );
		return E_FAIL;
	}

	if( 0 !=m_spIKwsServiceProvider->StartService() )
	{
		KwsspDbgInfo("StartService failed: %ws\n", sPath.GetBuffer() );
		m_spIKwsServiceProvider->UninitializeService();
		return E_FAIL;
	}
	KwsspDbgInfo( "Start succeed\n" );

	m_bStarted = TRUE;
	return S_OK;
}

HRESULT Kwssp::Stop()
{
	if(m_spIKwsServiceProvider == NULL || !m_bStarted )
	{
		KwsspDbgInfo("Stop failed: Not need\n");
		return E_UNEXPECTED;
	}

	m_spIKwsServiceProvider->StopService();
	m_spIKwsServiceProvider->UninitializeService();
	m_bStarted = FALSE;

	KwsspDbgInfo("Stop succeed\n" );
	return S_OK;
}



HRESULT __stdcall Kwssp::QueryBasicInterface( BASICINTERFACE niid, void** p )
{
	return E_NOTIMPL;
}



#if defined(__KWSSP_DEBUG__)
void __KwsspDbgInfoA(char *format, ...)
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
	wsprintfA(Buffer, "[Kwssp][%06d]", GetCurrentProcessId());
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
		strcpy(Buffer + len0, "Kwssp vsnprintf error");
	}

	OutputDebugStringA(Buffer);

	if( Buffer != StaticBuffer )
		delete [] Buffer;
}
void __KwsspAssert( const char * exp, const char * src_file, int src_line)
{
	CHAR Buffer[512];
	const char *msg_format = "[KwsspAssert]\r\n"
	"exp:  \"%s\"\r\n"
		"file: \"%s\"\r\n"
		"line: %d\r\n";

	wsprintfA(Buffer, msg_format, exp, src_file, src_line);
	::MessageBoxA(NULL, Buffer, "KwsspAssert", MB_OK|MB_ICONSTOP);
}

#endif
