// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

#pragma warning(disable: 4995)
#pragma warning(disable: 4996)
#pragma warning(disable: 4819)
#pragma warning(disable: 4018)
#pragma warning(disable: 4244)

#define WINVER          0x0500
#define _WIN32_WINNT    0x0501
#define _WIN32_IE       0x0600
#define _RICHEDIT_VER   0x0200

#define _SECURE_ATL 0

#ifdef _DEBUG

#   include <miniutil/bkconsole.h>

#   define DEBUG_TRACE bkconsole::printf

#else

#   define DEBUG_TRACE

#endif

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atltrace.h>

#define _WTL_NO_CSTRING


#include <exdisp.h>
#include <exdispid.h>
#include <mshtml.h>
#include <mshtmdid.h>
#include <math.h>

#include <wtl/atlapp.h>
#include <wtl/atlframe.h>
#include <wtl/atlctrls.h>
#include <wtl/atlctrlx.h>
#include <wtl/atldlgs.h>
#include <wtl/atlcrack.h>
#include <wtl/atlmisc.h>
#include <string.h>

extern DWORD g_dwMainThreadId;

/* 界面封装 */
#include "tinyxml/tinyxml.h"
#include "bkwin/bklistview.h"
#include "bkwin/bklistbox.h"
#include "bkwin/bkwnd.h"
#include "bkwin/bkwnddlg.h"
#include "bkwin/bkwndtabctrl.h"
#include "bkwin/bkwndnotify.h"
#include "bkwin/bkobject.h"
#include "bkwin/bkimage.h"
#include "bkwin/bkcolor.h"
#include "bkwin/bkskin.h"
#include "bkwin/bklistbox.h"
#include "bkwin/bkwndstyle.h"
#include "bkwin/bktheme.h"
#include "bkwin/bkatldefine.h"
#include "bkwin/bkwndcmnctrl.h"
#include "bkwin/bkwndrichtxt.h"
#include "bkwin/bkwndpanel.h"
#include "wtlhelper/whwindow.h"
#include "wtlhelper/whthreadnotify.h"
#include "wtlhelper/iidl.h"
#include "wtlhelper/bkfolderbrowsectrl.h"
#include "wtlhelper/wh3statecheckimglst.h"


#include <Shlwapi.h>


#include "kispublic/kcalloem.h"
#include "kispublic/kopksafereg.h"

//#include "kispublic/public.h"

#if defined _M_IX86
#   pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#   pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#   pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#   pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include "beikesafe.h"  /*  _Module */

/* 资源 */
#include "bkwinres.h"
#include "bkmsgdefine.h"

/* 配置 */
#include "bksafe/bksafeconfig.h"
#include "bksafe/bksafenavigatestring.h"


#ifndef KAV_PROCESS_ERROR
#define KAV_PROCESS_ERROR( x ) if ( !( x ) ) goto Exit0;
#endif

#ifndef KAV_COM_PROCESS_ERROR
#define KAV_COM_PROCESS_ERROR( x ) if ( FAILED( x ) ) goto Exit0;
#endif


