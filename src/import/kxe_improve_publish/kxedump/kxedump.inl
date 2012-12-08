//////////////////////////////////////////////////////////////////////
///		@file		kxedump.inl
///		@author		luopeng
///		@date		2008-12-13 09:07:58
///
///		@brief		dump¼ÓÔØ½Ó¿Ú
//////////////////////////////////////////////////////////////////////

#pragma once
#include "kxedump.h"

inline int InitializeDump()
{
	typedef void (__stdcall *pfnKxEOpenDumpMonitor)();

	HMODULE hLib = LoadLibrary(_T("kxedump.dll"));
	if (hLib)
	{
		pfnKxEOpenDumpMonitor OpenKingsoftDump = 
			(pfnKxEOpenDumpMonitor)GetProcAddress(hLib, "KxEOpenDumpMonitor");

		if (OpenKingsoftDump)
		{
			OpenKingsoftDump();				

			return 0;
		}
		else
		{
			return GetLastError();
		}
	}	
	else
	{
		return GetLastError();
	}
}

