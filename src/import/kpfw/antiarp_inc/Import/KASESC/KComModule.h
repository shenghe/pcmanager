/********************************************************************
* CreatedOn: 2007-8-24   17:59
* FileName:  KComModule.h
* CreatedBy: lidengwang <lidengwang@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL:  $
* Purpose:
*********************************************************************/

#ifndef __KAS_KCOMMODULE_H__
#define __KAS_KCOMMODULE_H__

#include "KModuleHandle.h"

template<bool t_bManaged>
class KComModuleT : public KModuleHandleT<t_bManaged>
{
public:
    
    KComModuleT( HMODULE hModule = NULL ) :
        m_pfnDllCanUnloadNow(NULL),
        m_pfnDllGetClassObject(NULL),
        KModuleHandleT<t_bManaged>(hModule)
    {
        _ComInitFuncs();
    }

    KComModuleT( LPCTSTR pszPathFile ) :
        m_pfnDllCanUnloadNow(NULL),
        m_pfnDllGetClassObject(NULL)
    {
        if (pszPathFile != NULL)
            LoadLibrary(pszPathFile);
    }

public:

    HMODULE LoadLibrary(LPCTSTR pszPathFile)
    {
        m_hModule = KModuleHandleT<t_bManaged>::LoadLibrary(pszPathFile);
        
        HRESULT hRet = _ComInitFuncs();

        return m_hModule;
    }

    //////////////////////////////////////////////////////////////////////////

    HRESULT DllGetClassObject(
        REFCLSID rclsid,
        REFIID riid,
        LPVOID* ppv)
    {
        if (m_pfnDllGetClassObject == NULL || m_hModule == NULL)
            return E_FAIL;

        return m_pfnDllGetClassObject(rclsid, riid, ppv);
    }

    HRESULT DllCanUnloadNow()
    {
        if (m_pfnDllCanUnloadNow == NULL || m_hModule == NULL)
            return S_OK;

        return m_pfnDllCanUnloadNow();
    }

protected:

    #define IDS_PFN_DLLGETCLASSOBJECT  ("DllGetClassObject")
    #define IDS_PFN_DLLCANUNLOADNOW    ("DllCanUnloadNow"  )

    typedef HRESULT STDAPICALLTYPE pfnDllCanUnloadNow(void);        
    typedef HRESULT STDAPICALLTYPE pfnDllGetClassObject(
        REFCLSID rclsid, 
        REFIID riid, 
        LPVOID* ppv);

    HRESULT _ComInitFuncs()
    {
        HRESULT hResult = E_FAIL;

        if (Handle() == NULL)
            return E_FAIL;
        
        m_pfnDllCanUnloadNow = 
            (pfnDllCanUnloadNow*)GetProcAddress(IDS_PFN_DLLCANUNLOADNOW);
        m_pfnDllGetClassObject = 
            (pfnDllGetClassObject*)GetProcAddress(IDS_PFN_DLLGETCLASSOBJECT);
        
        if (m_pfnDllCanUnloadNow != NULL || m_pfnDllGetClassObject != NULL)
            hResult = S_OK;

        return hResult;
    }

private:
    
    pfnDllCanUnloadNow*   m_pfnDllCanUnloadNow;
    pfnDllGetClassObject* m_pfnDllGetClassObject;
};

typedef KComModuleT<true>  KComModule;
typedef KComModuleT<false> KComModuleHandle;

#endif