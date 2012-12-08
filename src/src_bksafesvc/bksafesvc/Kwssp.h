#pragma once


#include "kasesc/KComInterfacePtr.h"
#include "kws/kwssp.h"
class Kwssp: public IKwsServiceBase 
{
	static Kwssp s_Instance;
public:
	static inline Kwssp & Instance()
	{
		return s_Instance;
	}
public:
	Kwssp(void);
	~Kwssp(void);
	HRESULT Start();
	HRESULT Stop();

	virtual HRESULT __stdcall QueryBasicInterface( BASICINTERFACE niid, void** p );

private:
	KComInterfacePtr<IKwsServiceProvider> m_spIKwsServiceProvider;
	BOOL m_bStarted;

};
