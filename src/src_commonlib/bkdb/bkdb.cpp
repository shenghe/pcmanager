// bkdb.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

static CBKDebugIniter g_dbgIniter;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}