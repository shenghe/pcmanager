/********************************************************************
* CreatedOn: 2007-8-16   15:18
* FileName:  KISCOM_AdvRegistery.h
* CreatedBy: zhangrui <zhangrui@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL$
* Purpose:
*********************************************************************/

#ifndef KISCOM_AdvRegistery_H_
#define KISCOM_AdvRegistery_H_


// KISCOM_REG_IID   support both CKISUseComDll and CKISUseComDll_v2
// KISCOM_REG_CLSID support CKISUseComDll_v2 only


#define KISCOM_REG_BEGIN()                                          \
STDAPI DllGetClassObject(                                           \
    /* [in ] */ REFCLSID rclsid,                                    \
    /* [in ] */ REFIID riid,                                        \
    /* [out, iid_is(riid)] */ LPVOID* ppv                           \
)                                                                   \
{                                                                   \
    try                                                             \
    {                                                               \
	    HRESULT hr = E_FAIL;                                        \
	    if (NULL == ppv)                                            \
	    {                                                           \
	        return E_POINTER;
	    
	
#define KISCOM_REG_CLSID(class__)                                   \
	    }                                                           \
        else if (IsEqualCLSID(rclsid, class__::GetCLSID()))         \
	    {                                                           \
	        class__* po = new class__();                            \
	        if (NULL == po)                                         \
	        {                                                       \
	            return E_OUTOFMEMORY;                               \
	        }                                                       \
	        hr = po->QueryInterface(riid, ppv);                     \
	        po->Release();                                          \
	        return hr;

#define KISCOM_REG_CLSID_EX(class__)                                \
        }                                                           \
        else if (IsEqualCLSID(rclsid, __uuidof(class__)))           \
        {                                                           \
        class__* po = new class__();                                \
        if (NULL == po)                                             \
            {                                                       \
            return E_OUTOFMEMORY;                                   \
            }                                                       \
            hr = po->QueryInterface(riid, ppv);                     \
            po->Release();                                          \
            return hr;

#define KISCOM_REG_CLSID_EX2(class__)                               \
        }                                                           \
        else if (IsEqualCLSID(rclsid,  class__::GetCLSID()))        \
        {                                                           \
        static class__* po = new class__();                         \
        if (NULL == po)                                             \
            {                                                       \
            return E_OUTOFMEMORY;                                   \
            }                                                       \
            hr = po->QueryInterface(riid, ppv);                     \
            po->Release();                                          \
            return hr;

#define KISCOM_REG_CLSID_EX3(class__)                               \
        }                                                           \
        else if (IsEqualCLSID(rclsid,  class__::GetCLSID()))        \
        {                                                           \
        static class__* po = &class__::Instance();                  \
        if (NULL == po)                                             \
            {                                                       \
            return E_OUTOFMEMORY;                                   \
            }                                                       \
            hr = po->QueryInterface(riid, ppv);                     \
            return hr;


	
#define KISCOM_REG_IID(interface__, class__)                        \
	    }                                                           \
	    else if (IsEqualIID(riid, __uuidof(interface__)))           \
	    {                                                           \
	        class__* po = new class__();                            \
	        if (NULL == po)                                         \
	        {                                                       \
	            return E_OUTOFMEMORY;                               \
	        }                                                       \
	        hr = po->QueryInterface(riid, ppv);                     \
	        po->Release();                                          \
	        return hr;
	
#define KISCOM_REG_END()                                            \
	    }                                                           \
	    else                                                        \
	    {                                                           \
	        return REGDB_E_CLASSNOTREG;                             \
	    }                                                           \
    }                                                               \
    catch (ATL::CAtlException& e)                                   \
    {                                                               \
    	return e;                                                   \
    }                                                               \
    catch (...)                                                     \
    {                                                               \
    	return E_FAIL;                                              \
    }                                                               \
}










#define KISCOM_PROG_BEGIN()                                         \
STDAPI DllGetCLSID(                                                 \
    /* [in ] */ LPCWSTR lpProgid,                                   \
    /* [out] */ LPCLSID lpClsid                                     \
)                                                                   \
{                                                                   \
    try                                                             \
    {                                                               \
	    if (NULL == lpProgid || NULL == lpClsid)                    \
	    {                                                           \
	        return E_POINTER;
	
	
	
#define KISCOM_PROGID(progid__, class__)                            \
	    }                                                           \
	    else if (progid__ && 0 == _wcsicmp(progid__, lpProgid))     \
	    {                                                           \
	        *lpClsid = __uuidof(class__);                           \
	        return S_OK;
	
	
	
#define KISCOM_PROG_END()                                           \
	    }                                                           \
	    else                                                        \
	    {                                                           \
	        return REGDB_E_CLASSNOTREG;                             \
	    }                                                           \
    }                                                               \
    catch (ATL::CAtlException& e)                                   \
    {                                                               \
    	return e;                                                   \
    }                                                               \
    catch (...)                                                     \
    {                                                               \
    	return E_FAIL;                                              \
    }                                                               \
}







#endif//KISCOM_AdvRegistery_H_
