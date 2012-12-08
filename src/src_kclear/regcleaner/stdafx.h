// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

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

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define _RICHEDIT_VER 0x0200

#define END_MSG_MAP_EX	END_MSG_MAP

//////////////////////////////////////////////////////////////////////////

#include <windows.h>

#include <set>
#include <algorithm>
#include <vector>
#include <list>

#include <ShellAPI.h>
#include <ShlObj.h>
#include <UserEnv.h>

#include <atlbase.h>
#include <atltypes.h>
#include <atlstr.h>
#include <atlwin.h>
#include <atlimage.h>

#include "atlapp.h"
#include "atlcrack.h"
#include "atltheme.h"
#include "atlctrls.h"
#include "atlctrlx.h"
#include "atlframe.h"

#include <tinyxml.h>
#include <kuiwin/kuilistview.h>

#include "common/kiscomobject.h"
#include "common/kisqueryinterface.h"
#include "common/kisreg.h"

#include "kispublic/kcalloem.h"
#include "kispublic/kopksafereg.h"
//////////////////////////////////////////////////////////////////////////

extern CAppModule _Module;


// TODO: reference additional headers your program requires here
