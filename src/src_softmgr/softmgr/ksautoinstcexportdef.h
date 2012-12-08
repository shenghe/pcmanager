//=================================================================
/**
* @file ksautoinstcexportdef.h
* @brief
* @author zhengyousheng <zhengyousheng@kingsoft.com>
* @data 2010-11-08 19:19
*/
//=================================================================

#pragma once

typedef HRESULT (__stdcall* KSAICreateModuleFunc)(IID iid, PVOID *pv);

/*
调用Demo如下

class CKSAutoInstCallBack : public IKSAutoInstCallBack
{
public:
	virtual HRESULT BeforeInstall(const S_KSAI_SOFT_INFO* pstruSoftInfo)
	{
		return S_OK;
	}

	virtual HRESULT OnProgress(const S_KSAI_STATUS* pstruStatus)
	{
		return S_OK;
	}

	virtual HRESULT AfterInstall(const S_KSAI_INST_RESULT* pstruInstResult)
	{
		return S_OK;
	}
};

struct S_TEST 
{
	IKSAutoInstClient* pIKSAutoInstClient;
	std::vector<std::string>* pvecXmlBuffers;
};

unsigned __stdcall RunThread( void* p )
{
	_ASSERT(NULL != p);
	S_TEST* pstruTest = (S_TEST*)p;
	_ASSERT(NULL != pstruTest);
	_ASSERT(NULL != pstruTest->pIKSAutoInstClient);
	_ASSERT(NULL != pstruTest->pvecXmlBuffers);

	HRESULT hrRetCode = E_FAIL;
	std::vector<std::wstring>::iterator IterStart = pstruTest->pvecXmlBuffers->begin();
	std::vector<std::wstring>::iterator IterEnd   = pstruTest->pvecXmlBuffers->end();
	for (; IterEnd != IterStart; ++IterStart)
	{
		hrRetCode = pstruTest->pIKSAutoInstClient->StartInstSoft(IterStart->c_str(), IterStart->size());
		if (FAILED(hrRetCode))
		{
			return 0;
		}
	}
	return 0;
}

void AutoInstSoft(LPCWSTR lpwszXmlCfg, std::vector<std::string> vecXmlBuffers)
{
	_ASSERT(NULL != lpwszXmlCfg);

	HMODULE hModule = ::LoadLibrary(L"ksautoinstc.dll");
	if (NULL == hModule)
	{
		return;
	}

	S_TEST struTest;
	HANDLE hRunThread = NULL;
	CKSAutoInstCallBack classCallBack;
	HRESULT hrRetCode = E_FAIL;
	IKSAutoInstClient* pIKSAutoInstClient = NULL;
	KSAICreateModuleFunc pfnKSAICreateModuleFunc =
		(KSAICreateModuleFunc)::GetProcAddress(hModule, "KSAICreateModule");
	if (NULL == pfnKSAICreateModuleFunc)
	{
		goto Exit0;
	}

	hrRetCode = pfnKSAICreateModuleFunc(__uuidof(IKSAutoInstClient), (LPVOID*)&pIKSAutoInstClient);
	if (FAILED(hrRetCode))
	{
		goto Exit0;
	}

	hrRetCode = pIKSAutoInstClient->Init(lpwszXmlCfg, NULL);
	if (FAILED(hrRetCode))
	{
		goto Exit0;
	}

	hrRetCode = pIKSAutoInstClient->SetCallBack(&classCallBack);
	if (FAILED(hrRetCode))
	{
		goto Exit0;
	}

	struTest.pIKSAutoInstClient = pIKSAutoInstClient;
	struTest.pvecXmlBuffers = &vecXmlBuffers;
	hRunThread = (HANDLE)_beginthreadex( NULL, 0, RunThread, (void*)&struTest, 0, NULL );
	if(NULL == hRunThread)
	{
		hrRetCode = E_FAIL;
		goto Exit0;
	}

	printf("please any key to stop\r\n");
	::getchar();
	hrRetCode = pIKSAutoInstClient->NotifyStop();
	if (FAILED(hrRetCode))
	{
		goto Exit0;
	}

Exit0:

	if (NULL != hRunThread)
	{
		if( WAIT_OBJECT_0 != ::WaitForSingleObject(hRunThread, KSAI_WAITSVRSTOP_TIMEOUT))
		{
			::TerminateThread(hRunThread, -1);
		}
		::CloseHandle( hRunThread );
		hRunThread = NULL;
	}

	if (NULL != pIKSAutoInstClient)
	{
		pIKSAutoInstClient->UnInit();
		pIKSAutoInstClient->Release();
		pIKSAutoInstClient = NULL;
	}

	if (NULL != hModule)
	{
		::FreeLibrary(hModule);
		hModule = NULL;
	}
}

{
    std::string strXmlBuffer = 
        "<version>1.0</version>"
        "<soft>"
            "<softinfo softid=\"1\" inst_pack=\"d:\\QQ2010SP2.2.exe\" >"
               "<inst_dir>d:\\qq\\qq</inst_dir>"
               "<user_define count=\"1\">"
                   "<item name=\"$ctrl_name_5\" value=\"0\" info=\"桌面\" />"
               "</user_define>"
            "</softinfo>"
        "</soft>";
    std::wstring wstrXmlBuffer = CA2W(strXmlBuffer.c_str());
    std::string strUTF8XmlBuffer = CW2A(wstrXmlBuffer.c_str());
    std::vector<std::string> vecXmlBuffers;
    vecXmlBuffers.push_back(strUTF8XmlBuffer);

	AutoInstSoft(L"d:\\QQ2010SP2.2.xml", vecXmlBuffers);
}
*/