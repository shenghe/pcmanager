///////////////////////////////////////////////////////////////	
//	
// FileName  :  klanmanlogadapter.h
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-12-21  17:35
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#pragma once

class KModuleLogAdapter
{
public:
	KModuleLogAdapter( 
		ULONG uModuleId, 
		WCHAR *wsModulePath,
		UCHAR *byMd5
		);
	~KModuleLogAdapter(void);

	BOOL Do( ILogMgr *logMgr );

private:
	MODULE_LOG m_moduleLog;
};
