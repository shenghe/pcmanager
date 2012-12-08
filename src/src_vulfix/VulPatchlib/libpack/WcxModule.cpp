#include "stdafx.h"
#include "WcxModule.h"
#include <string>
#include <vulfix\\BeikeUtils.h>

WcxModule::WcxModule()
	: m_pfnOpenArchive(NULL)
	,m_pfnCloseArchive(NULL)
	,m_pfnReadHeader(NULL)
	,m_pfnProcessFile(NULL)
	,m_pfnPackFiles(NULL)
	,m_pfnDeleteFiles(NULL)
	,m_pfnSetChangeVolProc(NULL)
	,m_pfnSetProcessDataProc(NULL)
	,m_pfnConfigurePacker(NULL)
	,m_pfnGetPackerCaps(NULL)
	,m_pfnCanYouHandleThisFile(NULL)
	,m_pfnPackSetDefaultParams(NULL)
{
	m_hModule = NULL;
}

bool WcxModule::LoadedOK ()
{
	if ( !(m_hModule && m_pfnOpenArchive && m_pfnCloseArchive && m_pfnReadHeader && m_pfnProcessFile) )
		return false;
	
	return true;
}

WcxModule::~WcxModule ()
{
	FreeLibrary (m_hModule);
	m_hModule = NULL;
}

bool WcxModule::Initialize( LPCTSTR lpFileName )
{
	m_hModule = LoadLibraryEx (
		lpFileName,
		NULL,
		LOAD_WITH_ALTERED_SEARCH_PATH
		);
	
	if ( m_hModule )
	{
		m_pfnOpenArchive=(PLUGINOPENARCHIVE)GetProcAddress(m_hModule,"OpenArchive");
		m_pfnCloseArchive=(PLUGINCLOSEARCHIVE)GetProcAddress(m_hModule,"CloseArchive");
		m_pfnReadHeader=(PLUGINREADHEADER)GetProcAddress(m_hModule,"ReadHeader");
		m_pfnProcessFile=(PLUGINPROCESSFILE)GetProcAddress(m_hModule,"ProcessFile");
		m_pfnPackFiles=(PLUGINPACKFILES)GetProcAddress(m_hModule,"PackFiles");
		m_pfnDeleteFiles=(PLUGINDELETEFILES)GetProcAddress(m_hModule,"DeleteFiles");
		m_pfnSetChangeVolProc=(PLUGINSETCHANGEVOLPROC)GetProcAddress(m_hModule,"SetChangeVolProc");
		m_pfnSetProcessDataProc=(PLUGINSETPROCESSDATAPROC)GetProcAddress(m_hModule,"SetProcessDataProc");
		m_pfnConfigurePacker=(PLUGINCONFIGUREPACKER)GetProcAddress(m_hModule,"ConfigurePacker");
		m_pfnGetPackerCaps=(PLUGINGETPACKERCAPS)GetProcAddress(m_hModule,"GetPackerCaps");
		m_pfnCanYouHandleThisFile=(PLUGINCANYOUHANDLETHISFILE)GetProcAddress(m_hModule,"CanYouHandleThisFile");
		m_pfnPackSetDefaultParams=(PLUGINPACKSETDEFAULTPARAMS)GetProcAddress(m_hModule,"PackSetDefaultParams");
	}
	
	if ( LoadedOK() )
	{
		if (m_pfnPackSetDefaultParams)
		{
			PackDefaultParamStruct dps;

			dps.size = sizeof(dps);
			dps.PluginInterfaceVersionLow = 10;
			dps.PluginInterfaceVersionHi = 2;
			strcpy(dps.DefaultIniName, "");

			m_pfnPackSetDefaultParams(&dps);
		}
	}
	return LoadedOK();
}

int __stdcall ChangeVolProc(char *ArcName,int Mode)
{
	//ATLTRACE("ChangeVolProc: %s %d \n", ArcName, Mode);
	return Mode;
}

int __stdcall ProcessDataProc(char *FileName,int Size)
{
	//ATLTRACE("ProcessDataProc: %s %d \n", FileName, Size);
	return Size;
}

bool WcxArchive::pOpenArchive( const char *lpFileName, int nOpMode )
{
	char szFilename[255] = {0};
	strcpy(szFilename, lpFileName);

	tOpenArchiveData OpenArchiveData = {0};
	OpenArchiveData.ArcName = szFilename;
	OpenArchiveData.OpenMode = (nOpMode == 0)?PK_OM_LIST:PK_OM_EXTRACT;

	SetFileApisToANSI();
	m_hArchive = m_pModule->m_pfnOpenArchive (&OpenArchiveData);
	SetFileApisToOEM();

	if(m_hArchive)
	{
		if (m_pModule->m_pfnSetProcessDataProc)
			m_pModule->m_pfnSetProcessDataProc (m_hArchive, ProcessDataProc);
		
		if (m_pModule->m_pfnSetChangeVolProc)
			m_pModule->m_pfnSetChangeVolProc (m_hArchive, ChangeVolProc);
	}

	return m_hArchive!=NULL;
}

void WcxArchive::pCloseArchive()
{
	m_pModule->m_pfnCloseArchive (m_hArchive);
}

bool WcxArchive::pExtract( const char *lpDestPath )
{
	int nProcessed = 0;
	int nResult = 0;
	
	m_files.clear();
	while ( nResult == 0 )
	{
		tHeaderData HeaderData;
		memset (&HeaderData, 0, sizeof (HeaderData));

		nResult = m_pModule->m_pfnReadHeader (m_hArchive, &HeaderData);
		if( nResult!=0 )
			continue;
		
		char szDestPath[MAX_PATH] = {0};
		strcpy(szDestPath, lpDestPath);
		PathAppendA(szDestPath, HeaderData.FileName);
				
		int nProcessResult = 0;
		if ( (HeaderData.FileAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
		{
			CreateDirEx (szDestPath);
			nProcessResult = m_pModule->m_pfnProcessFile (m_hArchive, PK_SKIP, NULL, NULL);
			ATLASSERT(FALSE);
		}
		else
		{
			CreateDirs( szDestPath );

			SetFileApisToANSI();
			nProcessResult = m_pModule->m_pfnProcessFile (m_hArchive, PK_EXTRACT, NULL, szDestPath);
			SetFileApisToOEM();
			
			if(!nProcessResult)
				m_files.push_back( szDestPath );
		}
		
		if ( !nProcessResult  )
			nProcessed++;
	}
	return nProcessed!=0;
}