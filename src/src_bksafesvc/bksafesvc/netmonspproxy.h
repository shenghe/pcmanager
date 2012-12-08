#pragma once


#include "kasesc/KComInterfacePtr.h"
#include "kpfw/netmonsp.h"

class KNetmonSp
{
	static KNetmonSp s_Instance;
public:
	static inline KNetmonSp & Instance()
	{
		return s_Instance;
	}
public:
	KNetmonSp(void);
	~KNetmonSp(void);
	HRESULT Start();
	HRESULT Stop();

private:
	KComInterfacePtr<INetMonServiceProvider> m_spIKwsServiceProvider;
	BOOL m_bStarted;

};
