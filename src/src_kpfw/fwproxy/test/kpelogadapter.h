///////////////////////////////////////////////////////////////	
//	
// FileName  :  kpelogadapter.h
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-12-28  15:31
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#pragma once

class KPeLogAdapter
{
public:
	KPeLogAdapter( 
		ULONG uModuleId,
		WCHAR *wsPePath,
		char *szUrl
		);
	~KPeLogAdapter(void);

	BOOL Do( ILogMgr *logMgr );

private:
	PE_LOG m_peLog;
};


class KPeCreateAdapter
{
public:
	KPeCreateAdapter(
		ULONG uCreatorModuleId,
		WCHAR *wsPePath
		);
	~KPeCreateAdapter(void);

	BOOL Do( ILogMgr *logMgr );

private:
	PE_LOG m_peLog;
};
