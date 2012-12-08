#pragma once
#include "vulfix/Defines.h"
#include "LeakDB.h"

class CImplSoftVulfix : public ISoftVulfix
{
public:
	CImplSoftVulfix();
	~CImplSoftVulfix(void);
	
public:
	// ISoftLeakScan
	virtual void SetObserver(IVulfixObserver *pObserver);
	virtual HRESULT Scan(DWORD dwFlags);
	virtual HRESULT Cancel();
	virtual int Count();
	virtual const CSimpleArray<LPTVulSoft>& GetResults();
	virtual HRESULT EnableVulCOM(int nId, BOOL bEnable=FALSE);

protected:
	void _Init();
	void _Reset();
	int _FindLeak( int nId );
	void _Notify(TVulFixEventType evt, int nKbId, DWORD dwParam1, DWORD dwParam2);

protected:
	IVulfixObserver *m_Observer;
	BOOL _Inited;
	CDBSoftLeak m_db;
	CSimpleArray<LPTVulSoft> m_arrLeaks;
};
