#ifndef  _KGetAntivirusProductInfo_h
#define  _KGetAntivirusProductInfo_h 
 
#include <comdef.h>   
#include <Wbemidl.h>   

#pragma comment(lib, "wbemuuid.lib")   

typedef struct _ANTIVIRUSPRODUCTINFO
{
	std::wstring    strProductName;
	std::wstring	strCompanyName;
}ANTIVIRUSPRODUCTINFO;


BOOL IsVistaOrLater(void)
{
	OSVERSIONINFOEXW Info;
	Info.dwOSVersionInfoSize = sizeof(Info);
	BOOL RetVal = GetVersionExW((LPOSVERSIONINFOW)&Info);
	if (RetVal && (Info.dwMajorVersion >= 6))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


BOOL GetAntivirusProductInfo(ANTIVIRUSPRODUCTINFO& struAntiVirusProductInfo)
{   
	BOOL bResult = FALSE;
	HRESULT hResult = E_FAIL;
	IWbemLocator *pLoc = NULL;   
	IWbemServices *pSvc = NULL;   
	IEnumWbemClassObject* pEnumerator = NULL;  
	IWbemClassObject *pclsObj;   
	ULONG uReturn = 0; 
 
	hResult =  CoInitializeEx(0, COINIT_MULTITHREADED);    
	if (FAILED(hResult))   
	{   
		//goto Exit0;
	}   

	hResult =  CoInitializeSecurity(NULL,    
		-1,
		NULL,  
    	NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT,  
		RPC_C_IMP_LEVEL_IMPERSONATE,    
		NULL,  
		EOAC_NONE,   
		NULL
		);             
	if (FAILED(hResult))   
	{   
		// 不要 goto 因为本进程中其他地方 CoInitializeSecurity 过了这里也会 FAILED 但是却是可以用的。
		//goto Exit0;
	}   

	hResult = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &pLoc);   
	if (FAILED(hResult))   
	{   
		goto Exit0;
	}   
 
	hResult = pLoc->ConnectServer(   
		(IsVistaOrLater() ? _bstr_t(L"ROOT\\SecurityCenter2") : _bstr_t(L"ROOT\\SecurityCenter")),   
		NULL,   
		NULL,
		0, 
		NULL,
		0,
		0,
		&pSvc 
		);   
	if (FAILED(hResult))   
	{   
		goto Exit0;
	}   

	hResult = CoSetProxyBlanket(   
		pSvc, 
		RPC_C_AUTHN_WINNT, 
		RPC_C_AUTHZ_NONE, 
		NULL,   
		RPC_C_AUTHN_LEVEL_CALL,  
		RPC_C_IMP_LEVEL_IMPERSONATE,  
		NULL, 
		EOAC_NONE    
		);   
	if (FAILED(hResult))   
	{   
		goto Exit0;
	} 
 
	hResult = pSvc->ExecQuery(   
		bstr_t("WQL"),    
		bstr_t("SELECT * FROM AntiVirusProduct"),   
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,    
		NULL,   
		&pEnumerator);   

	if (FAILED(hResult))   
	{   
		goto Exit0;
	}   
  
	while (pEnumerator)   
	{   
		VARIANT vtProp;  

		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);   
		if(0 == uReturn)   
		{   
			break;   
		}   

		hr = pclsObj->Get(L"displayName", 0, &vtProp, 0, 0); 
		if (SUCCEEDED(hr))
		{
			struAntiVirusProductInfo.strProductName = vtProp.bstrVal;
		}

		hr = pclsObj->Get(L"companyName", 0, &vtProp, 0, 0);
		if (SUCCEEDED(hr))
		{
			struAntiVirusProductInfo.strCompanyName = vtProp.bstrVal;
		}             

		VariantClear(&vtProp);    

		pclsObj->Release();   
	}   

	bResult = TRUE;

Exit0:

	if (pEnumerator)
	{
		pEnumerator->Release();
	}

	if (pSvc)
	{
		pSvc->Release(); 
	}
	  
	if (pLoc)
	{
		pLoc->Release();
	}
	
	CoUninitialize();   

	return bResult;
}  

#endif  // end of _KGetAntivirusProductInfo_h