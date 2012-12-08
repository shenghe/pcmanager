///////////////////////////////////////////////////////////////	
//	
// FileName  :  khttplogadapter.h
// Creator   :  yangzhenhui@kingsoft.com
// Date    	 :  2009-12-25  09:07
// Comment   :  
//
///////////////////////////////////////////////////////////////	
#pragma once

class KHttpLogAdapter
{
public:
	KHttpLogAdapter( 
		ULONG uModuleId, 
		char *szUrl
		);
	~KHttpLogAdapter(void);

	BOOL Do( ILogMgr *logMgr );

private:
	HTTP_LOG m_httpLog;
};
