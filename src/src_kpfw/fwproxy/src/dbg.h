// dbg.h: The debug header file.
#ifndef __DBG_H__
#define __DBG_H__

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef _DEBUG_LOG
static void __dbg_printf (const char * format,...)
{
	#define MAX_DBG_MSG_LEN (512)
	char buf[MAX_DBG_MSG_LEN];
	va_list ap;

	va_start(ap, format);

	vsnprintf_s(buf, 512, 512, format, ap);
	OutputDebugStringA(buf);

	va_end(ap);
}
#else
static void __dbg_printf (const char * format,...) {}
#endif

#define DBG __dbg_printf

//#ifdef _DEBUG
static void __dbg_wprintf (const wchar_t * format,...)
{
	#define MAX_DBG_MSG_LEN (512)
	wchar_t buf[MAX_DBG_MSG_LEN];
	va_list ap;

	va_start(ap, format);

	_vsnwprintf_s(buf, 512, 512, format, ap);
	OutputDebugStringW(buf);

	va_end(ap);
}
#define DBGW __dbg_wprintf
//#else
//static void __dbg_printf (const char * format,...) {}
//#define DBG  1?((void)(NULL)):__dbg_printf


#endif