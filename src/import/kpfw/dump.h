////////////////////////////////////////////////////////////////////////////////
//      
//      File for kpfw
//      
//      File      : dump.h
//      Version   : 1.0
//      Comment   :	提供给所有kpfw产品dump功能支持
//      
//      Create at : 2009-3-3
//      Create by : liupeng
//      
////////////////////////////////////////////////////////////////////////////////
#pragma once 

#define DUMP_REPORT_ADDR		"http://info.duba.net/pagetracer/kpfw20/dump/__utm.gif"

inline void InitDump(BOOL bUi = TRUE)
{
	typedef void (__cdecl *pfnOpenKingsoftDump)();
	typedef void (__cdecl *pfnOpenKingsoftDumpNoGui)();
	typedef int (__cdecl *pfnSetPostServerAddress)(const char*);
	typedef void (__cdecl *pSetExternFilter)(LPTOP_LEVEL_EXCEPTION_FILTER);

	HMODULE hLib = LoadLibrary(L"dump.DLL");
	if (hLib)
	{
		pfnSetPostServerAddress SetPostServerAddress = 
			(pfnSetPostServerAddress)GetProcAddress(hLib, "SetPostServerAddress");
		if (SetPostServerAddress)
			SetPostServerAddress(DUMP_REPORT_ADDR);

		if (bUi)
		{
			pfnOpenKingsoftDump OpenKingsoftDump = 
				(pfnOpenKingsoftDump)GetProcAddress(hLib, "OpenKingsoftDump");
			if (OpenKingsoftDump)
				OpenKingsoftDump(); 
		}
		else
		{
			pfnOpenKingsoftDumpNoGui OpenKingsoftDumpNoGUI = 
				(pfnOpenKingsoftDumpNoGui)GetProcAddress(hLib, "OpenKingsoftDumpNoGUI");
			if (OpenKingsoftDumpNoGUI)
				OpenKingsoftDumpNoGUI(); 
		}
	}
	else
	{
		// to do: echo error message
	}	
}