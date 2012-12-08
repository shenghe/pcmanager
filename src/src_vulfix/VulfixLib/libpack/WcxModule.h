#pragma once
#include <string>
#include <vector>
#pragma warning(disable : 4996)
#include "wcxhead.h"

typedef HANDLE (__stdcall *PLUGINOPENARCHIVE)( tOpenArchiveData* ArchiveData );
typedef int    (__stdcall *PLUGINCLOSEARCHIVE)( HANDLE hArcData );
typedef int    (__stdcall *PLUGINREADHEADER)( HANDLE hArcData, tHeaderData* HeaderData );
typedef int    (__stdcall *PLUGINPROCESSFILE)( HANDLE hArcData, int Operation, char* DestPath, char* DestName );
typedef int    (__stdcall *PLUGINPACKFILES)( char* PackedFile, char* SubPath, char* SrcPath, char* AddList, int Flags );
typedef int    (__stdcall *PLUGINDELETEFILES)( char* PackedFile, char* DeleteList );
typedef void   (__stdcall *PLUGINSETCHANGEVOLPROC)( HANDLE hArcData, tChangeVolProc pChangeVolProc1 );
typedef void   (__stdcall *PLUGINSETPROCESSDATAPROC)( HANDLE hArcData, tProcessDataProc pProcessDataProc );
typedef void   (__stdcall *PLUGINCONFIGUREPACKER)( HWND Parent, HINSTANCE DllInstance );
typedef int    (__stdcall *PLUGINGETPACKERCAPS)();
typedef BOOL   (__stdcall *PLUGINCANYOUHANDLETHISFILE)( const char* FileName );
typedef void   (__stdcall *PLUGINPACKSETDEFAULTPARAMS)( PackDefaultParamStruct* dps );

class WcxModule {

public:
	WcxModule ();
	~WcxModule ();

	bool Initialize( LPCTSTR );
	bool LoadedOK();	

public:
	PLUGINOPENARCHIVE m_pfnOpenArchive;
	PLUGINCLOSEARCHIVE m_pfnCloseArchive;
	PLUGINREADHEADER m_pfnReadHeader;
	PLUGINPROCESSFILE m_pfnProcessFile;
	PLUGINPACKFILES m_pfnPackFiles;
	PLUGINDELETEFILES m_pfnDeleteFiles;
	PLUGINSETCHANGEVOLPROC m_pfnSetChangeVolProc;
	PLUGINSETPROCESSDATAPROC m_pfnSetProcessDataProc;
	PLUGINCONFIGUREPACKER m_pfnConfigurePacker;
	PLUGINGETPACKERCAPS m_pfnGetPackerCaps;
	PLUGINCANYOUHANDLETHISFILE m_pfnCanYouHandleThisFile;
	PLUGINPACKSETDEFAULTPARAMS m_pfnPackSetDefaultParams;	
	
public:
	HMODULE m_hModule;
	DWORD m_dwCRC;
};


class WcxArchive
{
public:
	WcxArchive(WcxModule *pModule) : m_pModule(pModule)
	{	
	}
	bool pOpenArchive (const char *lpFileName, int nOpMode);
	void pCloseArchive ();
	bool pExtract(const char *lpDestPath);	
	
public:
	std::vector<std::string> m_files;

protected:		
	WcxModule *m_pModule;
	HANDLE m_hArchive;

};
