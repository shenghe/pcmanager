#pragma once

#pragma warning(disable: 4996)

#define WINVER          0x0500
#define _WIN32_WINNT    0x0501
#define _WIN32_IE       0x0501
#define _RICHEDIT_VER   0x0200

// WTL : 
#define _SECURE_ATL 0
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#define _WTL_NO_CSTRING

#include <atlbase.h>
#include <atlwin.h>
#include <atltrace.h>
#include <atlstr.h>

#include "kispublic/kcalloem.h"
#include "kispublic/kopksafereg.h"

#include <wtl/atlapp.h>
#include <wtl/atlctrls.h>
#include <wtl/atlctrlw.h>
