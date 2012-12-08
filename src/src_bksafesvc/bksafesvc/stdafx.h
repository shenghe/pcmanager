// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

//#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <atlsync.h>
#include <atlpath.h>
#include <atltime.h>

#include <Psapi.h>
#include <shellapi.h>

#include "kispublic/kcalloem.h"
#include "kispublic/kopksafereg.h"

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "libeay32.lib")

#pragma warning( disable : 4996 )
using namespace ATL;

#include "skylark2\bkeng.h"

#ifndef KAV_PROCESS_ERROR
#define KAV_PROCESS_ERROR( x ) if ( !( x ) ) goto Exit0;
#endif

#ifndef KAV_COM_PROCESS_ERROR
#define KAV_COM_PROCESS_ERROR( x ) if ( FAILED( x ) ) goto Exit0;
#endif