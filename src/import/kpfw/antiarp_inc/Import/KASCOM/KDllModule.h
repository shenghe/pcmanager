///////////////////////////////////////////////////////////////
//
//	Filename: 	KDllModule.h
//	Creator:	lichenglin  <lichenglin@kingsoft.net>
//	Date:		2007-8-25   12:44
//	Comment:	
//
///////////////////////////////////////////////////////////////

#ifndef _KDllModule_h_
#define _KDllModule_h_
#include "KClassFactoryManagement.h"
#include <assert.h>
//.-----------------------------------------------------------------------------
// KDllModule Decleration
//.-----------------------------------------------------------------------------

class KDllModule
{
private:
	KDllModule(void);
	~KDllModule(void);

public:
	static KDllModule* GetInstance();

	ULONG Lock();
	ULONG UnLock();

	HRESULT DllCanUnloadNow(void);
	HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);

private:
	KClassFactoryManagement* m_pCfmgt;
	ULONG  m_ulLockCount;
};




//.-----------------------------------------------------------------------------
// KDllModule Implementation
//.-----------------------------------------------------------------------------

inline
KDllModule* KDllModule::GetInstance()
{
	static KDllModule _DllModule;
	return &_DllModule;
}


inline
KDllModule::KDllModule(void)
{
	m_ulLockCount = 0;
	m_pCfmgt = KClassFactoryManagement::GetInstance();
}


inline
KDllModule::~KDllModule(void)
{
}


inline
ULONG KDllModule::Lock()
{
	return ++m_ulLockCount;
}


inline
ULONG KDllModule::UnLock()
{
	return --m_ulLockCount;
}


inline
HRESULT KDllModule::DllCanUnloadNow(void)
{
	// 仅剩下类厂对象
	return m_ulLockCount == m_pCfmgt->GetCount() ? S_OK : E_FAIL;
}


inline
HRESULT KDllModule::DllGetClassObject( REFCLSID rclsid, REFIID riid, LPVOID* ppv )
{
	return m_pCfmgt->GetClassObject( rclsid, riid, ppv );
}




//.-----------------------------------------------------------------------------
// A Only one instance of KDllModule
//.-----------------------------------------------------------------------------

inline
KDllModule* KASGetDllModule()
{
	return KDllModule::GetInstance();
}


#endif // _KDllModule_h_