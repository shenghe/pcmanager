// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif				

#include <WinSock2.h>
#include <windows.h>
#include "atlsync.h"
#include <time.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <fstream>
using namespace std;


#include "scom/scom/SCOMDef.h"
#include "scom/scom/SCOMBase.h"
#include "scom/scom/SCOM.h"
#include "scom/scom/ksdll.h"
#include "fwproxy_public.h"
#include "logmgr_public.h"
#include "kpfwlogdef.h"

// TODO: reference additional headers your program requires here
