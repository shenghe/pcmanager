// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER		0x0500
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0200

#pragma warning(disable: 4996)

#include <atlbase.h>
#include <atlstr.h>
#include <wtl/atlapp.h>

//#define _WTL_USE_CSTRING

extern CAppModule _Module;

#include <atlwin.h>
//#include <atlmisc.h>
#include <wtl/atlframe.h>
#include <tchar.h>
#include <conio.h>
#include <wtl/atldlgs.h>
#include <wtl/atlcrack.h>
#include <wtl/atlctrls.h>
#include "kispublic/kcalloem.h"
