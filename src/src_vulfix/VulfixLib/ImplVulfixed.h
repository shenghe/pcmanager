#pragma once
#include "vulfix/Defines.h"
#include "LeakDB.h"

class CImplVulfixed : public IVulfixed
{
public:
	CImplVulfixed();
	~CImplVulfixed(void);
	
public:
	virtual void SetObserver(IVulfixObserver *pObserver);
	virtual HRESULT Scan(DWORD dwFlags);
	virtual HRESULT Cancel();
	virtual int Count();
	virtual const CSimpleArray<TItemFixed*>& GetResults();
	virtual const CSimpleArray<TReplacedUpdate*>& GetReplacedUpdates();

protected:
	void Reset();
	
protected:
	DBFilterBase *m_pFilterOS;
	IVulfixObserver *m_pObserver;

	BOOL m_Canceled;

	CDBOSLeak m_db;
	CSimpleArray<TItemFixed*> m_arrInstalledPatches;
	CSimpleArray<TReplacedUpdate*> m_arrReplacedUpdates;
};
