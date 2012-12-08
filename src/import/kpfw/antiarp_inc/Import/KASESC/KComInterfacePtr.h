/********************************************************************
* CreatedOn: 2007-8-24   17:59
* FileName:  KComInterfacePtr.h
* CreatedBy: lidengwang <lidengwang@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL:  $
* Purpose:
*********************************************************************/


#ifndef __KAS_KCOMINTERFACEPTR_H__
#define __KAS_KCOMINTERFACEPTR_H__

#include "KComAutoPtr.h"
#include "KComModule.h"


template<class T, const CLSID* pclsid = NULL, const IID* piid = &__uuidof(T)>
class KComInterfacePtr : public KComAutoPtr<T>
{
public:
    
    KComInterfacePtr(bool bInit = true) :
        m_hComDll(HMODULE(0)),
        KComAutoPtr<T>(NULL)
    {
        if (bInit)
            LoadSysInterface();
    }

    KComInterfacePtr(LPCTSTR pszComDllFile)
    {
        LoadInterface(pszComDllFile);
    }

    KComInterfacePtr(HMODULE hModule) :
        KComModuleHandle(hModule)
    {
        LoadInterface(hModule);
    }

    ~KComInterfacePtr()
    {
        Free();
    }

public:

    BOOL LoadSysInterface()
    {
        assert(m_hComDll.Handle() == NULL);

        CLSID clsid = { 0 };

        if (pclsid != NULL)
            clsid = (*pclsid);

        HRESULT hr = KComAutoPtr<T>::CoCreateInstance(clsid);

        return SUCCEEDED(hr);
    }
    
    BOOL LoadInterface(LPCTSTR pszComDllFile)
    {
        assert(m_hComDll.Handle() == NULL);

        m_hComDll.LoadLibrary(pszComDllFile);

        HRESULT hr = CoCreateInstance();

        return SUCCEEDED(hr);
    }

    BOOL LoadInterface(HMODULE hModule)
    {
        assert(m_hComDll.Handle() == NULL);

        HRESULT hr =CoCreateInstance();

        return SUCCEEDED(hr);
    }

    void Free()
    {
        Release();
        
        // 
        //assert(m_hComDll.DllCanUnloadNow() == S_OK); 

        //if (SUCCEEDED(m_hComDll.DllCanUnloadNow()))
        {
            m_hComDll.FreeLibrary();
        }
    }

protected:

    HRESULT CoCreateInstance()
    {
        HRESULT hResult  = E_FAIL;
        HRESULT hRetCode = E_FAIL;


        CLSID clsid = { 0 };
        IID   iid   = { 0 };

        if (pclsid != NULL)
            clsid = (*pclsid);

        if (piid != NULL)
            iid = (*piid);

        KComAutoPtr<IClassFactory> spClassFactory;


        hRetCode = m_hComDll.DllGetClassObject(clsid, 
            IID_IClassFactory, (void**)&spClassFactory);
        if (SUCCEEDED(hRetCode) && spClassFactory != NULL )
		{
			hRetCode = spClassFactory->CreateInstance(NULL, iid, (void**)&m_pT);
			if (FAILED(hRetCode))
				return hRetCode;
		}
		else
		{
			hRetCode = m_hComDll.DllGetClassObject(clsid, 
				iid, (void**)&m_pT);
			if (FAILED(hRetCode))
				return hRetCode;
		}

        //spClassFactory->LockServer(TRUE);

        hResult = S_OK;

        return hResult;
    }

    KComInterfacePtr<T, pclsid, piid>& operator = (const KComInterfacePtr<T, pclsid, piid>&);

private:

    KComModuleHandle m_hComDll;

};


#endif